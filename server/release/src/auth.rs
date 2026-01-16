//! JWT 认证、密码哈希与用户提取器模块
//!
//! 本模块提供以下功能：
//!
//! - `JwtManager`: JWT 令牌的生成与验证
//! - `Claims`: JWT 令牌载荷结构
//! - `TokenType`: 令牌类型（访问令牌/刷新令牌）
//! - `hash_password` / `verify_password`: 密码哈希与验证
//! - `CurrentUser`: Poem 提取器，从请求中提取当前用户
//!
//! # 密码哈希
//!
//! 使用 Argon2id 算法，这是 OWASP 推荐的密码哈希算法，
//! 具有良好的抗 GPU/ASIC 攻击能力。
//!
//! # 认证流程
//!
//! 1. 用户登录时，验证密码后生成 access_token 和 refresh_token
//! 2. access_token 存储在 HttpOnly Cookie 中，用于 API 认证
//! 3. refresh_token 用于在 access_token 过期后获取新令牌
//! 4. [`CurrentUser`] 提取器从请求中自动提取并验证令牌
//!
//! 作者: WaterRun
//! 日期: 2026-01-16

use std::sync::Arc;

use argon2::{Argon2, PasswordHash, PasswordHasher, PasswordVerifier, password_hash::SaltString};
use chrono::{Duration, Utc};
use jsonwebtoken::{DecodingKey, EncodingKey, Header, Validation, decode, encode};
use poem::{FromRequest, Request, RequestBody};
use rand::rngs::OsRng;
use serde::{Deserialize, Serialize};
use uuid::Uuid;

use crate::app::AppState;
use crate::config::JwtConfig;
use crate::error::{AppError, AuthErrorKind};

// ============================================================================
// 密码哈希
// ============================================================================

/// 对密码进行哈希处理
///
/// 使用 Argon2id 算法生成密码哈希，每次调用生成不同的盐值。
///
/// # 参数
///
/// - `password`: 明文密码
///
/// # 返回值
///
/// 成功返回 PHC 格式的密码哈希字符串，失败返回 [`AppError`]。
///
/// # 示例
///
/// ```
/// use ds4viz_server::auth::hash_password;
///
/// let hash = hash_password("my_secure_password").unwrap();
/// assert!(hash.starts_with("$argon2id$"));
/// ```
pub fn hash_password(password: &str) -> Result<String, AppError> {
    let salt = SaltString::generate(&mut OsRng);
    let argon2 = Argon2::default();

    argon2
        .hash_password(password.as_bytes(), &salt)
        .map(|hash| hash.to_string())
        .map_err(|e| AppError::Internal(format!("密码哈希失败: {e}")))
}

/// 验证密码是否匹配哈希
///
/// # 参数
///
/// - `password`: 明文密码
/// - `hash`: PHC 格式的密码哈希字符串
///
/// # 返回值
///
/// 密码匹配返回 `Ok(true)`，不匹配返回 `Ok(false)`，
/// 哈希格式无效返回 [`AppError`]。
///
/// # 示例
///
/// ```
/// use ds4viz_server::auth::{hash_password, verify_password};
///
/// let hash = hash_password("my_password").unwrap();
/// assert!(verify_password("my_password", &hash).unwrap());
/// assert!(!verify_password("wrong_password", &hash).unwrap());
/// ```
pub fn verify_password(password: &str, hash: &str) -> Result<bool, AppError> {
    let parsed_hash = PasswordHash::new(hash)
        .map_err(|e| AppError::Internal(format!("密码哈希格式无效: {e}")))?;

    Ok(Argon2::default()
        .verify_password(password.as_bytes(), &parsed_hash)
        .is_ok())
}

// ============================================================================
// JWT 令牌
// ============================================================================

/// JWT 令牌类型
///
/// # 示例
///
/// ```
/// use ds4viz_server::auth::TokenType;
///
/// let token_type = TokenType::Access;
/// assert_eq!(token_type.as_str(), "access");
///
/// let token_type = TokenType::Refresh;
/// assert_eq!(token_type.as_str(), "refresh");
/// ```
#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
#[serde(rename_all = "lowercase")]
pub enum TokenType {
    /// 访问令牌，用于 API 认证
    Access,
    /// 刷新令牌，用于获取新的访问令牌
    Refresh,
}

impl TokenType {
    /// 获取令牌类型的字符串表示
    ///
    /// # 示例
    ///
    /// ```
    /// use ds4viz_server::auth::TokenType;
    ///
    /// assert_eq!(TokenType::Access.as_str(), "access");
    /// assert_eq!(TokenType::Refresh.as_str(), "refresh");
    /// ```
    #[must_use]
    pub const fn as_str(&self) -> &'static str {
        match self {
            Self::Access => "access",
            Self::Refresh => "refresh",
        }
    }
}

/// JWT 令牌载荷
///
/// 包含令牌的所有声明信息。
///
/// # 示例
///
/// ```
/// use ds4viz_server::auth::{Claims, TokenType};
/// use uuid::Uuid;
/// use chrono::Utc;
///
/// let claims = Claims {
///     sub: Uuid::new_v4(),
///     exp: Utc::now().timestamp() as u64 + 3600,
///     iat: Utc::now().timestamp() as u64,
///     token_type: TokenType::Access,
/// };
///
/// assert_eq!(claims.token_type, TokenType::Access);
/// ```
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Claims {
    /// 主题（用户 ID）
    pub sub: Uuid,
    /// 过期时间（Unix 时间戳）
    pub exp: u64,
    /// 签发时间（Unix 时间戳）
    pub iat: u64,
    /// 令牌类型
    pub token_type: TokenType,
}

/// JWT 令牌管理器
///
/// 负责生成和验证 JWT 令牌。
///
/// # 示例
///
/// ```
/// use ds4viz_server::auth::JwtManager;
/// use ds4viz_server::config::JwtConfig;
/// use uuid::Uuid;
///
/// let config = JwtConfig {
///     secret: "test_secret_key_32_bytes_long!!!".to_string(),
///     access_token_ttl_secs: 3600,
///     refresh_token_ttl_secs: 604800,
/// };
///
/// let manager = JwtManager::new(&config);
/// let user_id = Uuid::new_v4();
///
/// let (access_token, _) = manager.generate_access_token(user_id).unwrap();
/// let claims = manager.verify_token(&access_token).unwrap();
///
/// assert_eq!(claims.sub, user_id);
/// ```
pub struct JwtManager {
    encoding_key: EncodingKey,
    decoding_key: DecodingKey,
    access_token_ttl_secs: u64,
    refresh_token_ttl_secs: u64,
}

impl Clone for JwtManager {
    fn clone(&self) -> Self {
        // EncodingKey 和 DecodingKey 不实现 Clone,需要从原始密钥重新创建
        // 这里假设使用 HMAC-SHA256,从原始密钥克隆
        // 注意: 这是一个简化实现,实际应该存储原始密钥
        Self {
            encoding_key: self.encoding_key.clone(),
            decoding_key: self.decoding_key.clone(),
            access_token_ttl_secs: self.access_token_ttl_secs,
            refresh_token_ttl_secs: self.refresh_token_ttl_secs,
        }
    }
}

impl std::fmt::Debug for JwtManager {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.debug_struct("JwtManager")
            .field("access_token_ttl_secs", &self.access_token_ttl_secs)
            .field("refresh_token_ttl_secs", &self.refresh_token_ttl_secs)
            .finish()
    }
}

impl JwtManager {
    /// 创建新的 JWT 管理器
    ///
    /// # 参数
    ///
    /// - `config`: JWT 配置，包含密钥和令牌有效期
    ///
    /// # 示例
    ///
    /// ```
    /// use ds4viz_server::auth::JwtManager;
    /// use ds4viz_server::config::JwtConfig;
    ///
    /// let config = JwtConfig {
    ///     secret: "my_secret_key".to_string(),
    ///     access_token_ttl_secs: 3600,
    ///     refresh_token_ttl_secs: 604800,
    /// };
    ///
    /// let manager = JwtManager::new(&config);
    /// ```
    #[must_use]
    pub fn new(config: &JwtConfig) -> Self {
        Self {
            encoding_key: EncodingKey::from_secret(config.secret.as_bytes()),
            decoding_key: DecodingKey::from_secret(config.secret.as_bytes()),
            access_token_ttl_secs: config.access_token_ttl_secs,
            refresh_token_ttl_secs: config.refresh_token_ttl_secs,
        }
    }

    /// 生成访问令牌
    ///
    /// # 参数
    ///
    /// - `user_id`: 用户 ID
    ///
    /// # 返回值
    ///
    /// 成功返回 `(令牌字符串, 有效期秒数)`，失败返回 [`AppError`]。
    ///
    /// # 示例
    ///
    /// ```
    /// use ds4viz_server::auth::JwtManager;
    /// use ds4viz_server::config::JwtConfig;
    /// use uuid::Uuid;
    ///
    /// let config = JwtConfig {
    ///     secret: "test_secret".to_string(),
    ///     access_token_ttl_secs: 3600,
    ///     refresh_token_ttl_secs: 604800,
    /// };
    ///
    /// let manager = JwtManager::new(&config);
    /// let (token, expires_in) = manager.generate_access_token(Uuid::new_v4()).unwrap();
    ///
    /// assert!(!token.is_empty());
    /// assert_eq!(expires_in, 3600);
    /// ```
    pub fn generate_access_token(&self, user_id: Uuid) -> Result<(String, u64), AppError> {
        self.generate_token(user_id, TokenType::Access, self.access_token_ttl_secs)
    }

    /// 生成刷新令牌
    ///
    /// # 参数
    ///
    /// - `user_id`: 用户 ID
    ///
    /// # 返回值
    ///
    /// 成功返回 `(令牌字符串, 有效期秒数)`，失败返回 [`AppError`]。
    ///
    /// # 示例
    ///
    /// ```
    /// use ds4viz_server::auth::JwtManager;
    /// use ds4viz_server::config::JwtConfig;
    /// use uuid::Uuid;
    ///
    /// let config = JwtConfig {
    ///     secret: "test_secret".to_string(),
    ///     access_token_ttl_secs: 3600,
    ///     refresh_token_ttl_secs: 604800,
    /// };
    ///
    /// let manager = JwtManager::new(&config);
    /// let (token, expires_in) = manager.generate_refresh_token(Uuid::new_v4()).unwrap();
    ///
    /// assert!(!token.is_empty());
    /// assert_eq!(expires_in, 604800);
    /// ```
    pub fn generate_refresh_token(&self, user_id: Uuid) -> Result<(String, u64), AppError> {
        self.generate_token(user_id, TokenType::Refresh, self.refresh_token_ttl_secs)
    }

    /// 验证令牌并提取载荷
    ///
    /// # 参数
    ///
    /// - `token`: JWT 令牌字符串
    ///
    /// # 返回值
    ///
    /// 成功返回 [`Claims`]，失败返回 [`AppError`]。
    ///
    /// # 错误
    ///
    /// - 令牌格式无效：[`AuthErrorKind::InvalidToken`]
    /// - 令牌已过期：[`AuthErrorKind::TokenExpired`]
    ///
    /// # 示例
    ///
    /// ```
    /// use ds4viz_server::auth::JwtManager;
    /// use ds4viz_server::config::JwtConfig;
    /// use uuid::Uuid;
    ///
    /// let config = JwtConfig {
    ///     secret: "test_secret".to_string(),
    ///     access_token_ttl_secs: 3600,
    ///     refresh_token_ttl_secs: 604800,
    /// };
    ///
    /// let manager = JwtManager::new(&config);
    /// let user_id = Uuid::new_v4();
    /// let (token, _) = manager.generate_access_token(user_id).unwrap();
    ///
    /// let claims = manager.verify_token(&token).unwrap();
    /// assert_eq!(claims.sub, user_id);
    /// ```
    pub fn verify_token(&self, token: &str) -> Result<Claims, AppError> {
        let validation = Validation::default();

        decode::<Claims>(token, &self.decoding_key, &validation)
            .map(|data| data.claims)
            .map_err(|e| {
                if e.kind() == &jsonwebtoken::errors::ErrorKind::ExpiredSignature {
                    AppError::Unauthorized(AuthErrorKind::TokenExpired)
                } else {
                    AppError::Unauthorized(AuthErrorKind::InvalidToken)
                }
            })
    }

    /// 获取访问令牌有效期
    ///
    /// # 示例
    ///
    /// ```
    /// use ds4viz_server::auth::JwtManager;
    /// use ds4viz_server::config::JwtConfig;
    ///
    /// let config = JwtConfig {
    ///     secret: "test".to_string(),
    ///     access_token_ttl_secs: 7200,
    ///     refresh_token_ttl_secs: 604800,
    /// };
    ///
    /// let manager = JwtManager::new(&config);
    /// assert_eq!(manager.access_token_ttl(), 7200);
    /// ```
    #[must_use]
    pub const fn access_token_ttl(&self) -> u64 {
        self.access_token_ttl_secs
    }

    fn generate_token(
        &self,
        user_id: Uuid,
        token_type: TokenType,
        ttl_secs: u64,
    ) -> Result<(String, u64), AppError> {
        let now = Utc::now();
        let exp = now + Duration::seconds(i64::try_from(ttl_secs).unwrap_or(i64::MAX));

        let claims = Claims {
            sub: user_id,
            exp: exp.timestamp() as u64,
            iat: now.timestamp() as u64,
            token_type,
        };

        let token = encode(&Header::default(), &claims, &self.encoding_key)
            .map_err(|e| AppError::Internal(format!("JWT 编码失败: {e}")))?;

        Ok((token, ttl_secs))
    }
}

// ============================================================================
// 用户提取器
// ============================================================================

/// 当前用户提取器
///
/// 从请求中提取并验证 JWT 令牌，返回当前用户 ID。
///
/// # 令牌来源
///
/// 按优先级顺序尝试以下来源：
/// 1. Cookie 中的 `access_token`
/// 2. `Authorization` 头中的 `Bearer` 令牌
///
/// # 示例
///
/// ```rust,no_run
/// use ds4viz_server::auth::CurrentUser;
/// use poem::{handler, web::Data};
///
/// #[handler]
/// async fn protected_endpoint(user: CurrentUser) -> String {
///     format!("Hello, user {}", user.id)
/// }
/// ```
#[derive(Debug, Clone)]
pub struct CurrentUser {
    /// 用户 ID
    pub id: Uuid,
}

impl<'a> FromRequest<'a> for CurrentUser {
    async fn from_request(req: &'a Request, _body: &mut RequestBody) -> poem::Result<Self> {
        let state = req
            .extensions()
            .get::<Arc<AppState>>()
            .ok_or_else(|| AppError::Internal("应用状态未找到".to_string()))?;

        let token = extract_token_from_cookie(req)
            .or_else(|| extract_token_from_header(req))
            .ok_or(AppError::Unauthorized(AuthErrorKind::InvalidToken))?;

        let claims = state.jwt_manager.verify_token(&token)?;

        if claims.token_type != TokenType::Access {
            return Err(AppError::Unauthorized(AuthErrorKind::InvalidToken).into());
        }

        Ok(Self { id: claims.sub })
    }
}

/// 从 Cookie 中提取令牌
fn extract_token_from_cookie(req: &Request) -> Option<String> {
    // 从 Cookie 头中手动解析
    req.header("Cookie").and_then(|cookie_str| {
        cookie_str
            .split(';')
            .filter_map(|pair| {
                let mut parts = pair.trim().splitn(2, '=');
                let name = parts.next()?;
                let value = parts.next()?;
                if name == "access_token" {
                    // 移除可能的URL编码和引号
                    let decoded = urlencoding::decode(value).ok()?;
                    Some(decoded.trim_matches('"').to_string())
                } else {
                    None
                }
            })
            .next()
    })
}

/// 从 Authorization 头中提取 Bearer 令牌
fn extract_token_from_header(req: &Request) -> Option<String> {
    req.header("Authorization")
        .and_then(|h| h.strip_prefix("Bearer "))
        .map(|s| s.to_string())
}

// ============================================================================
// 单元测试
// ============================================================================

#[cfg(test)]
mod tests {
    use super::*;

    fn create_test_manager() -> JwtManager {
        let config = JwtConfig {
            secret: "test_secret_key_for_testing_purposes".to_string(),
            access_token_ttl_secs: 3600,
            refresh_token_ttl_secs: 604800,
        };
        JwtManager::new(&config)
    }

    // ------------------------------------------------------------------------
    // 密码哈希测试
    // ------------------------------------------------------------------------

    #[test]
    fn test_hash_password_produces_valid_hash() {
        let password = "my_secure_password";
        let hash = hash_password(password).expect("哈希应成功");

        assert!(hash.starts_with("$argon2id$"), "应使用 argon2id 算法");
        assert!(hash.len() > 50, "哈希应有足够长度");
    }

    #[test]
    fn test_hash_password_produces_different_hashes() {
        let password = "same_password";
        let hash1 = hash_password(password).expect("哈希应成功");
        let hash2 = hash_password(password).expect("哈希应成功");

        assert_ne!(hash1, hash2, "相同密码应产生不同哈希（不同盐值）");
    }

    #[test]
    fn test_verify_password_correct() {
        let password = "correct_password";
        let hash = hash_password(password).expect("哈希应成功");

        let result = verify_password(password, &hash).expect("验证应成功");
        assert!(result, "正确密码应验证通过");
    }

    #[test]
    fn test_verify_password_incorrect() {
        let password = "correct_password";
        let hash = hash_password(password).expect("哈希应成功");

        let result = verify_password("wrong_password", &hash).expect("验证应成功");
        assert!(!result, "错误密码应验证失败");
    }

    #[test]
    fn test_verify_password_invalid_hash() {
        let result = verify_password("password", "invalid_hash_format");
        assert!(result.is_err(), "无效哈希格式应返回错误");
    }

    #[test]
    fn test_hash_password_empty_password() {
        let hash = hash_password("").expect("空密码也应能哈希");
        assert!(hash.starts_with("$argon2id$"), "空密码哈希应有效");
    }

    #[test]
    fn test_verify_password_empty_password() {
        let hash = hash_password("").expect("哈希应成功");

        assert!(
            verify_password("", &hash).expect("验证应成功"),
            "空密码应能验证"
        );
        assert!(
            !verify_password("not_empty", &hash).expect("验证应成功"),
            "非空密码不应匹配空密码哈希"
        );
    }

    // ------------------------------------------------------------------------
    // JWT 令牌测试
    // ------------------------------------------------------------------------

    #[test]
    fn test_token_type_as_str() {
        assert_eq!(
            TokenType::Access.as_str(),
            "access",
            "Access 类型字符串应为 'access'"
        );
        assert_eq!(
            TokenType::Refresh.as_str(),
            "refresh",
            "Refresh 类型字符串应为 'refresh'"
        );
    }

    #[test]
    fn test_generate_access_token() {
        let manager = create_test_manager();
        let user_id = Uuid::new_v4();

        let result = manager.generate_access_token(user_id);
        assert!(result.is_ok(), "生成访问令牌应成功");

        let (token, expires_in) = result.expect("已验证成功");
        assert!(!token.is_empty(), "令牌不应为空");
        assert_eq!(expires_in, 3600, "有效期应为 3600 秒");
    }

    #[test]
    fn test_generate_refresh_token() {
        let manager = create_test_manager();
        let user_id = Uuid::new_v4();

        let result = manager.generate_refresh_token(user_id);
        assert!(result.is_ok(), "生成刷新令牌应成功");

        let (token, expires_in) = result.expect("已验证成功");
        assert!(!token.is_empty(), "令牌不应为空");
        assert_eq!(expires_in, 604800, "有效期应为 604800 秒");
    }

    #[test]
    fn test_verify_valid_token() {
        let manager = create_test_manager();
        let user_id = Uuid::new_v4();

        let (token, _) = manager
            .generate_access_token(user_id)
            .expect("生成令牌应成功");

        let claims = manager.verify_token(&token);
        assert!(claims.is_ok(), "验证有效令牌应成功");

        let claims = claims.expect("已验证成功");
        assert_eq!(claims.sub, user_id, "用户 ID 应匹配");
        assert_eq!(claims.token_type, TokenType::Access, "令牌类型应为 Access");
    }

    #[test]
    fn test_verify_invalid_token() {
        let manager = create_test_manager();

        let result = manager.verify_token("invalid.token.here");
        assert!(result.is_err(), "验证无效令牌应失败");

        match result.unwrap_err() {
            AppError::Unauthorized(AuthErrorKind::InvalidToken) => {}
            other => panic!("期望 InvalidToken 错误，实际为: {:?}", other),
        }
    }

    #[test]
    fn test_verify_token_with_wrong_secret() {
        let manager = create_test_manager();
        let user_id = Uuid::new_v4();

        let (token, _) = manager
            .generate_access_token(user_id)
            .expect("生成令牌应成功");

        let other_config = JwtConfig {
            secret: "different_secret_key_for_testing".to_string(),
            access_token_ttl_secs: 3600,
            refresh_token_ttl_secs: 604800,
        };
        let other_manager = JwtManager::new(&other_config);

        let result = other_manager.verify_token(&token);
        assert!(result.is_err(), "使用错误密钥验证应失败");
    }

    #[test]
    fn test_access_token_ttl() {
        let config = JwtConfig {
            secret: "test".to_string(),
            access_token_ttl_secs: 7200,
            refresh_token_ttl_secs: 604800,
        };

        let manager = JwtManager::new(&config);
        assert_eq!(manager.access_token_ttl(), 7200, "访问令牌有效期应为 7200");
    }

    #[test]
    fn test_claims_serialization() {
        let claims = Claims {
            sub: Uuid::new_v4(),
            exp: 1234567890,
            iat: 1234567800,
            token_type: TokenType::Access,
        };

        let json = serde_json::to_string(&claims).expect("序列化应成功");

        assert!(json.contains("sub"), "JSON 应包含 sub");
        assert!(json.contains("exp"), "JSON 应包含 exp");
        assert!(json.contains("iat"), "JSON 应包含 iat");
        assert!(json.contains("access"), "JSON 应包含 token_type");
    }

    #[test]
    fn test_claims_deserialization() {
        let user_id = Uuid::new_v4();
        let json = format!(
            r#"{{"sub":"{}","exp":1234567890,"iat":1234567800,"token_type":"refresh"}}"#,
            user_id
        );

        let claims: Claims = serde_json::from_str(&json).expect("反序列化应成功");

        assert_eq!(claims.sub, user_id, "用户 ID 应匹配");
        assert_eq!(claims.exp, 1234567890, "过期时间应匹配");
        assert_eq!(
            claims.token_type,
            TokenType::Refresh,
            "令牌类型应为 Refresh"
        );
    }

    #[test]
    fn test_different_users_get_different_tokens() {
        let manager = create_test_manager();
        let user1 = Uuid::new_v4();
        let user2 = Uuid::new_v4();

        let (token1, _) = manager
            .generate_access_token(user1)
            .expect("生成令牌应成功");
        let (token2, _) = manager
            .generate_access_token(user2)
            .expect("生成令牌应成功");

        assert_ne!(token1, token2, "不同用户的令牌应不同");

        let claims1 = manager.verify_token(&token1).expect("验证应成功");
        let claims2 = manager.verify_token(&token2).expect("验证应成功");

        assert_eq!(claims1.sub, user1, "用户 1 的 ID 应匹配");
        assert_eq!(claims2.sub, user2, "用户 2 的 ID 应匹配");
    }

    #[test]
    fn test_refresh_token_has_correct_type() {
        let manager = create_test_manager();
        let user_id = Uuid::new_v4();

        let (token, _) = manager
            .generate_refresh_token(user_id)
            .expect("生成令牌应成功");

        let claims = manager.verify_token(&token).expect("验证应成功");
        assert_eq!(
            claims.token_type,
            TokenType::Refresh,
            "刷新令牌类型应为 Refresh"
        );
    }

    // ------------------------------------------------------------------------
    // CurrentUser 提取器测试
    // ------------------------------------------------------------------------

    #[test]
    fn test_current_user_struct() {
        let user_id = Uuid::new_v4();
        let current_user = CurrentUser { id: user_id };

        assert_eq!(current_user.id, user_id, "用户 ID 应匹配");
    }

    #[test]
    fn test_current_user_is_clone() {
        let user = CurrentUser { id: Uuid::new_v4() };
        let cloned = user.clone();

        assert_eq!(user.id, cloned.id, "克隆后的 ID 应相同");
    }
}
