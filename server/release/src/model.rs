//! 数据模型模块
//!
//! 本模块定义应用的数据模型，包含两部分：
//!
//! - 数据库表映射结构，用于与 PostgreSQL 交互
//! - API 请求/响应结构，用于 HTTP 接口
//!
//! # 当前实现范围
//!
//! - 用户相关: User, UserLevel, UserStatus
//! - 代码执行: Language, ExecutionSubmission, ExecutionCache, ExecutionStatus
//! - 配额管理: UserQuotaUsage
//! - API 请求/响应: 认证、执行、配额等
//!
//! 作者: WaterRun
//! 日期: 2026-01-16

use chrono::{DateTime, Utc};
use serde::{Deserialize, Serialize};
use uuid::Uuid;
use validator::Validate;

// ============================================================================
// 数据库表映射模型
// ============================================================================

/// 用户表映射
///
/// 对应数据库 `users` 表，包含用户的完整信息。
///
/// # 示例
///
/// ```
/// use ds4viz_server::model::{User, UserLevel, UserStatus};
/// use chrono::Utc;
/// use uuid::Uuid;
///
/// let user = User {
///     id: Uuid::new_v4(),
///     username: "testuser".to_string(),
///     email: "test@example.com".to_string(),
///     password_hash: "hashed_password".to_string(),
///     avatar: None,
///     avatar_mime_type: None,
///     bio: Some("Hello, World!".to_string()),
///     level: UserLevel::Standard,
///     status: UserStatus::Active,
///     banned_reason: None,
///     banned_until: None,
///     created_at: Utc::now(),
///     updated_at: Utc::now(),
/// };
///
/// assert_eq!(user.username, "testuser");
/// assert_eq!(user.level, UserLevel::Standard);
/// ```
#[derive(Debug, Clone, sqlx::FromRow)]
pub struct User {
    /// 用户唯一标识
    pub id: Uuid,
    /// 用户名，唯一，4-64 字符
    pub username: String,
    /// 邮箱地址，唯一
    pub email: String,
    /// 密码哈希（Argon2）
    pub password_hash: String,
    /// 头像二进制数据
    pub avatar: Option<Vec<u8>>,
    /// 头像 MIME 类型
    pub avatar_mime_type: Option<String>,
    /// 个人简介
    pub bio: Option<String>,
    /// 用户等级
    pub level: UserLevel,
    /// 账户状态
    pub status: UserStatus,
    /// 封禁原因
    pub banned_reason: Option<String>,
    /// 封禁截止时间，None 表示永久封禁
    pub banned_until: Option<DateTime<Utc>>,
    /// 注册时间
    pub created_at: DateTime<Utc>,
    /// 最后更新时间
    pub updated_at: DateTime<Utc>,
}

/// 用户等级
///
/// 定义用户的权限等级，影响可用功能和资源配额。
///
/// # 示例
///
/// ```
/// use ds4viz_server::model::UserLevel;
///
/// let level = UserLevel::Standard;
/// assert_eq!(level.as_str(), "standard");
///
/// let level = UserLevel::Enhanced;
/// assert!(level.is_enhanced());
/// ```
#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize, sqlx::Type)]
#[sqlx(type_name = "varchar", rename_all = "lowercase")]
#[serde(rename_all = "lowercase")]
pub enum UserLevel {
    /// 标准用户
    ///
    /// 受以下限制：
    /// - 只能使用解释型语言
    /// - 每周期有 CPU 时间配额限制
    /// - 单次提交代码大小限制
    Standard,

    /// 增强用户
    ///
    /// 无资源限制，可使用所有语言。
    Enhanced,
}

/// 用户状态
///
/// 定义用户账户的当前状态。
///
/// # 示例
///
/// ```
/// use ds4viz_server::model::UserStatus;
///
/// let status = UserStatus::Active;
/// assert_eq!(status.as_str(), "active");
/// assert!(status.is_active());
///
/// let status = UserStatus::Banned;
/// assert!(!status.is_active());
/// ```
#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize, sqlx::Type)]
#[sqlx(type_name = "varchar", rename_all = "lowercase")]
#[serde(rename_all = "lowercase")]
pub enum UserStatus {
    /// 正常状态
    Active,
    /// 被封禁
    Banned,
    /// 被暂停
    Suspended,
}

impl UserLevel {
    /// 获取等级的字符串表示
    ///
    /// # 示例
    ///
    /// ```
    /// use ds4viz_server::model::UserLevel;
    ///
    /// assert_eq!(UserLevel::Standard.as_str(), "standard");
    /// assert_eq!(UserLevel::Enhanced.as_str(), "enhanced");
    /// ```
    #[must_use]
    pub const fn as_str(&self) -> &'static str {
        match self {
            Self::Standard => "standard",
            Self::Enhanced => "enhanced",
        }
    }

    /// 是否为增强用户
    ///
    /// # 示例
    ///
    /// ```
    /// use ds4viz_server::model::UserLevel;
    ///
    /// assert!(!UserLevel::Standard.is_enhanced());
    /// assert!(UserLevel::Enhanced.is_enhanced());
    /// ```
    #[must_use]
    pub const fn is_enhanced(&self) -> bool {
        matches!(self, Self::Enhanced)
    }
}

impl UserStatus {
    /// 获取状态的字符串表示
    ///
    /// # 示例
    ///
    /// ```
    /// use ds4viz_server::model::UserStatus;
    ///
    /// assert_eq!(UserStatus::Active.as_str(), "active");
    /// assert_eq!(UserStatus::Banned.as_str(), "banned");
    /// assert_eq!(UserStatus::Suspended.as_str(), "suspended");
    /// ```
    #[must_use]
    pub const fn as_str(&self) -> &'static str {
        match self {
            Self::Active => "active",
            Self::Banned => "banned",
            Self::Suspended => "suspended",
        }
    }

    /// 账户是否处于正常状态
    ///
    /// # 示例
    ///
    /// ```
    /// use ds4viz_server::model::UserStatus;
    ///
    /// assert!(UserStatus::Active.is_active());
    /// assert!(!UserStatus::Banned.is_active());
    /// assert!(!UserStatus::Suspended.is_active());
    /// ```
    #[must_use]
    pub const fn is_active(&self) -> bool {
        matches!(self, Self::Active)
    }
}

// ============================================================================
// API 请求/响应模型
// ============================================================================

/// 用户注册请求
///
/// # 验证规则
///
/// - `username`: 4-64 字符
/// - `email`: 有效的邮箱格式
/// - `password`: 8-128 字符
///
/// # 示例
///
/// ```
/// use ds4viz_server::model::RegisterRequest;
/// use validator::Validate;
///
/// let request = RegisterRequest {
///     username: "testuser".to_string(),
///     email: "test@example.com".to_string(),
///     password: "password123".to_string(),
/// };
///
/// assert!(request.validate().is_ok());
/// ```
#[derive(Debug, Clone, Deserialize, Validate)]
pub struct RegisterRequest {
    /// 用户名，4-64 字符，仅字母数字下划线
    #[validate(length(min = 4, max = 64, message = "用户名长度需在 4-64 字符之间"))]
    pub username: String,

    /// 邮箱地址
    #[validate(email(message = "无效的邮箱地址"))]
    pub email: String,

    /// 密码，8-128 字符
    #[validate(length(min = 8, max = 128, message = "密码长度需在 8-128 字符之间"))]
    pub password: String,
}

/// 用户登录请求
///
/// # 示例
///
/// ```
/// use ds4viz_server::model::LoginRequest;
///
/// let request = LoginRequest {
///     username: "testuser".to_string(),
///     password: "password123".to_string(),
/// };
///
/// assert_eq!(request.username, "testuser");
/// ```
#[derive(Debug, Clone, Deserialize)]
pub struct LoginRequest {
    /// 用户名或邮箱
    pub username: String,

    /// 密码
    pub password: String,
}

/// 刷新令牌请求
///
/// # 示例
///
/// ```
/// use ds4viz_server::model::RefreshTokenRequest;
///
/// let request = RefreshTokenRequest {
///     refresh_token: Some("token".to_string()),
/// };
///
/// assert!(request.refresh_token.is_some());
/// ```
#[derive(Debug, Clone, Deserialize)]
pub struct RefreshTokenRequest {
    /// 刷新令牌，如使用 Cookie 则可不传
    pub refresh_token: Option<String>,
}

/// 更新用户信息请求
///
/// # 验证规则
///
/// - `bio`: 最大 5000 字符
/// - `email`: 如提供，需为有效邮箱格式
///
/// # 示例
///
/// ```
/// use ds4viz_server::model::UpdateUserRequest;
/// use validator::Validate;
///
/// let request = UpdateUserRequest {
///     bio: Some("Hello, World!".to_string()),
///     email: None,
/// };
///
/// assert!(request.validate().is_ok());
/// ```
#[derive(Debug, Clone, Deserialize, Validate)]
pub struct UpdateUserRequest {
    /// 个人简介，最大 5000 字符
    #[validate(length(max = 5000, message = "个人简介最多 5000 字符"))]
    pub bio: Option<String>,

    /// 新邮箱地址
    #[validate(email(message = "无效的邮箱地址"))]
    pub email: Option<String>,
}

/// 令牌信息响应
///
/// # 示例
///
/// ```
/// use ds4viz_server::model::TokenResponse;
///
/// let response = TokenResponse {
///     access_token: "jwt_token".to_string(),
///     expires_in: 3600,
/// };
///
/// assert_eq!(response.expires_in, 3600);
/// ```
#[derive(Debug, Clone, Serialize)]
pub struct TokenResponse {
    /// JWT 访问令牌
    pub access_token: String,

    /// 有效期（秒）
    pub expires_in: u64,
}

/// 用户信息响应（完整，用于登录/注册后返回）
///
/// # 示例
///
/// ```
/// use ds4viz_server::model::UserResponse;
/// use chrono::Utc;
/// use uuid::Uuid;
///
/// let response = UserResponse {
///     id: Uuid::new_v4(),
///     username: "testuser".to_string(),
///     email: "test@example.com".to_string(),
///     avatar_url: None,
///     bio: None,
///     level: "standard".to_string(),
///     status: "active".to_string(),
///     created_at: Utc::now(),
///     updated_at: None,
/// };
///
/// assert_eq!(response.username, "testuser");
/// ```
#[derive(Debug, Clone, Serialize)]
pub struct UserResponse {
    /// 用户 ID
    pub id: Uuid,

    /// 用户名
    pub username: String,

    /// 邮箱（仅自己可见）
    pub email: String,

    /// 头像 URL
    pub avatar_url: Option<String>,

    /// 个人简介
    pub bio: Option<String>,

    /// 用户等级
    pub level: String,

    /// 账户状态
    pub status: String,

    /// 注册时间
    pub created_at: DateTime<Utc>,

    /// 更新时间
    pub updated_at: Option<DateTime<Utc>>,
}

/// 用户公开信息响应（用于查看他人信息）
///
/// # 示例
///
/// ```
/// use ds4viz_server::model::PublicUserResponse;
/// use chrono::Utc;
/// use uuid::Uuid;
///
/// let response = PublicUserResponse {
///     id: Uuid::new_v4(),
///     username: "publicuser".to_string(),
///     avatar_url: None,
///     bio: None,
///     level: "standard".to_string(),
///     created_at: Utc::now(),
/// };
///
/// assert_eq!(response.username, "publicuser");
/// ```
#[derive(Debug, Clone, Serialize)]
pub struct PublicUserResponse {
    /// 用户 ID
    pub id: Uuid,

    /// 用户名
    pub username: String,

    /// 头像 URL
    pub avatar_url: Option<String>,

    /// 个人简介
    pub bio: Option<String>,

    /// 用户等级
    pub level: String,

    /// 注册时间
    pub created_at: DateTime<Utc>,
}

/// 认证成功响应（登录/注册）
///
/// # 示例
///
/// ```
/// use ds4viz_server::model::{AuthResponse, UserResponse, TokenResponse};
/// use chrono::Utc;
/// use uuid::Uuid;
///
/// let response = AuthResponse {
///     user: UserResponse {
///         id: Uuid::new_v4(),
///         username: "testuser".to_string(),
///         email: "test@example.com".to_string(),
///         avatar_url: None,
///         bio: None,
///         level: "standard".to_string(),
///         status: "active".to_string(),
///         created_at: Utc::now(),
///         updated_at: None,
///     },
///     token: TokenResponse {
///         access_token: "jwt".to_string(),
///         expires_in: 3600,
///     },
/// };
///
/// assert_eq!(response.token.expires_in, 3600);
/// ```
#[derive(Debug, Clone, Serialize)]
pub struct AuthResponse {
    /// 用户信息
    pub user: UserResponse,

    /// 令牌信息
    pub token: TokenResponse,
}

impl UserResponse {
    /// 从数据库用户模型创建响应
    ///
    /// # 参数
    ///
    /// - `user`: 数据库用户模型
    ///
    /// # 示例
    ///
    /// ```
    /// use ds4viz_server::model::{UserResponse, User, UserLevel, UserStatus};
    /// use chrono::Utc;
    /// use uuid::Uuid;
    ///
    /// let user = User {
    ///     id: Uuid::new_v4(),
    ///     username: "testuser".to_string(),
    ///     email: "test@example.com".to_string(),
    ///     password_hash: "hash".to_string(),
    ///     avatar: None,
    ///     avatar_mime_type: None,
    ///     bio: None,
    ///     level: UserLevel::Standard,
    ///     status: UserStatus::Active,
    ///     banned_reason: None,
    ///     banned_until: None,
    ///     created_at: Utc::now(),
    ///     updated_at: Utc::now(),
    /// };
    ///
    /// let response = UserResponse::from_user(&user);
    /// assert_eq!(response.username, "testuser");
    /// ```
    #[must_use]
    pub fn from_user(user: &User) -> Self {
        Self {
            id: user.id,
            username: user.username.clone(),
            email: user.email.clone(),
            avatar_url: user
                .avatar
                .as_ref()
                .map(|_| format!("/api/v1/users/{}/avatar", user.id)),
            bio: user.bio.clone(),
            level: user.level.as_str().to_string(),
            status: user.status.as_str().to_string(),
            created_at: user.created_at,
            updated_at: Some(user.updated_at),
        }
    }
}

impl PublicUserResponse {
    /// 从数据库用户模型创建公开信息响应
    ///
    /// # 参数
    ///
    /// - `user`: 数据库用户模型
    ///
    /// # 示例
    ///
    /// ```
    /// use ds4viz_server::model::{PublicUserResponse, User, UserLevel, UserStatus};
    /// use chrono::Utc;
    /// use uuid::Uuid;
    ///
    /// let user = User {
    ///     id: Uuid::new_v4(),
    ///     username: "publicuser".to_string(),
    ///     email: "public@example.com".to_string(),
    ///     password_hash: "hash".to_string(),
    ///     avatar: None,
    ///     avatar_mime_type: None,
    ///     bio: Some("bio".to_string()),
    ///     level: UserLevel::Enhanced,
    ///     status: UserStatus::Active,
    ///     banned_reason: None,
    ///     banned_until: None,
    ///     created_at: Utc::now(),
    ///     updated_at: Utc::now(),
    /// };
    ///
    /// let response = PublicUserResponse::from_user(&user);
    /// assert_eq!(response.level, "enhanced");
    /// ```
    #[must_use]
    pub fn from_user(user: &User) -> Self {
        Self {
            id: user.id,
            username: user.username.clone(),
            avatar_url: user
                .avatar
                .as_ref()
                .map(|_| format!("/api/v1/users/{}/avatar", user.id)),
            bio: user.bio.clone(),
            level: user.level.as_str().to_string(),
            created_at: user.created_at,
        }
    }
}

// ============================================================================
// 单元测试
// ============================================================================

#[cfg(test)]
mod tests {
    use super::*;

    // ------------------------------------------------------------------------
    // 数据库模型测试
    // ------------------------------------------------------------------------

    #[test]
    fn test_user_level_as_str() {
        assert_eq!(
            UserLevel::Standard.as_str(),
            "standard",
            "Standard 等级字符串应为 'standard'"
        );
        assert_eq!(
            UserLevel::Enhanced.as_str(),
            "enhanced",
            "Enhanced 等级字符串应为 'enhanced'"
        );
    }

    #[test]
    fn test_user_level_is_enhanced() {
        assert!(
            !UserLevel::Standard.is_enhanced(),
            "Standard 用户不应是增强用户"
        );
        assert!(
            UserLevel::Enhanced.is_enhanced(),
            "Enhanced 用户应是增强用户"
        );
    }

    #[test]
    fn test_user_status_as_str() {
        assert_eq!(
            UserStatus::Active.as_str(),
            "active",
            "Active 状态字符串应为 'active'"
        );
        assert_eq!(
            UserStatus::Banned.as_str(),
            "banned",
            "Banned 状态字符串应为 'banned'"
        );
        assert_eq!(
            UserStatus::Suspended.as_str(),
            "suspended",
            "Suspended 状态字符串应为 'suspended'"
        );
    }

    #[test]
    fn test_user_status_is_active() {
        assert!(UserStatus::Active.is_active(), "Active 状态应返回 true");
        assert!(!UserStatus::Banned.is_active(), "Banned 状态应返回 false");
        assert!(
            !UserStatus::Suspended.is_active(),
            "Suspended 状态应返回 false"
        );
    }

    #[test]
    fn test_user_level_serialize() {
        let level = UserLevel::Standard;
        let json = serde_json::to_string(&level).expect("序列化失败");
        assert_eq!(json, "\"standard\"", "Standard 应序列化为 'standard'");

        let level = UserLevel::Enhanced;
        let json = serde_json::to_string(&level).expect("序列化失败");
        assert_eq!(json, "\"enhanced\"", "Enhanced 应序列化为 'enhanced'");
    }

    #[test]
    fn test_user_level_deserialize() {
        let level: UserLevel = serde_json::from_str("\"standard\"").expect("反序列化失败");
        assert_eq!(level, UserLevel::Standard, "应反序列化为 Standard");

        let level: UserLevel = serde_json::from_str("\"enhanced\"").expect("反序列化失败");
        assert_eq!(level, UserLevel::Enhanced, "应反序列化为 Enhanced");
    }

    #[test]
    fn test_user_status_serialize() {
        let status = UserStatus::Active;
        let json = serde_json::to_string(&status).expect("序列化失败");
        assert_eq!(json, "\"active\"", "Active 应序列化为 'active'");

        let status = UserStatus::Banned;
        let json = serde_json::to_string(&status).expect("序列化失败");
        assert_eq!(json, "\"banned\"", "Banned 应序列化为 'banned'");
    }

    #[test]
    fn test_user_status_deserialize() {
        let status: UserStatus = serde_json::from_str("\"active\"").expect("反序列化失败");
        assert_eq!(status, UserStatus::Active, "应反序列化为 Active");

        let status: UserStatus = serde_json::from_str("\"banned\"").expect("反序列化失败");
        assert_eq!(status, UserStatus::Banned, "应反序列化为 Banned");

        let status: UserStatus = serde_json::from_str("\"suspended\"").expect("反序列化失败");
        assert_eq!(status, UserStatus::Suspended, "应反序列化为 Suspended");
    }

    #[test]
    fn test_user_struct_creation() {
        let user = User {
            id: Uuid::new_v4(),
            username: "testuser".to_string(),
            email: "test@example.com".to_string(),
            password_hash: "hash".to_string(),
            avatar: None,
            avatar_mime_type: None,
            bio: Some("test bio".to_string()),
            level: UserLevel::Standard,
            status: UserStatus::Active,
            banned_reason: None,
            banned_until: None,
            created_at: Utc::now(),
            updated_at: Utc::now(),
        };

        assert_eq!(user.username, "testuser", "用户名应正确");
        assert_eq!(user.email, "test@example.com", "邮箱应正确");
        assert_eq!(user.level, UserLevel::Standard, "等级应为 Standard");
        assert_eq!(user.status, UserStatus::Active, "状态应为 Active");
        assert!(user.bio.is_some(), "bio 应有值");
        assert!(user.avatar.is_none(), "avatar 应为空");
    }

    #[test]
    fn test_user_with_banned_status() {
        let banned_until = Utc::now();
        let user = User {
            id: Uuid::new_v4(),
            username: "banneduser".to_string(),
            email: "banned@example.com".to_string(),
            password_hash: "hash".to_string(),
            avatar: None,
            avatar_mime_type: None,
            bio: None,
            level: UserLevel::Standard,
            status: UserStatus::Banned,
            banned_reason: Some("违规操作".to_string()),
            banned_until: Some(banned_until),
            created_at: Utc::now(),
            updated_at: Utc::now(),
        };

        assert_eq!(user.status, UserStatus::Banned, "状态应为 Banned");
        assert!(user.banned_reason.is_some(), "封禁原因应有值");
        assert!(user.banned_until.is_some(), "封禁截止时间应有值");
        assert!(!user.status.is_active(), "被封禁的用户不应处于活跃状态");
    }

    // ------------------------------------------------------------------------
    // API 模型测试
    // ------------------------------------------------------------------------

    #[test]
    fn test_register_request_valid() {
        let request = RegisterRequest {
            username: "validuser".to_string(),
            email: "valid@example.com".to_string(),
            password: "password123".to_string(),
        };

        assert!(request.validate().is_ok(), "有效的注册请求应通过验证");
    }

    #[test]
    fn test_register_request_invalid_username_too_short() {
        let request = RegisterRequest {
            username: "ab".to_string(),
            email: "valid@example.com".to_string(),
            password: "password123".to_string(),
        };

        let result = request.validate();
        assert!(result.is_err(), "用户名太短应验证失败");

        let errors = result.unwrap_err();
        assert!(
            errors.field_errors().contains_key("username"),
            "应包含 username 字段错误"
        );
    }

    #[test]
    fn test_register_request_invalid_email() {
        let request = RegisterRequest {
            username: "validuser".to_string(),
            email: "invalid-email".to_string(),
            password: "password123".to_string(),
        };

        let result = request.validate();
        assert!(result.is_err(), "无效邮箱应验证失败");

        let errors = result.unwrap_err();
        assert!(
            errors.field_errors().contains_key("email"),
            "应包含 email 字段错误"
        );
    }

    #[test]
    fn test_register_request_invalid_password_too_short() {
        let request = RegisterRequest {
            username: "validuser".to_string(),
            email: "valid@example.com".to_string(),
            password: "short".to_string(),
        };

        let result = request.validate();
        assert!(result.is_err(), "密码太短应验证失败");

        let errors = result.unwrap_err();
        assert!(
            errors.field_errors().contains_key("password"),
            "应包含 password 字段错误"
        );
    }

    #[test]
    fn test_update_user_request_valid() {
        let request = UpdateUserRequest {
            bio: Some("This is my bio".to_string()),
            email: Some("new@example.com".to_string()),
        };

        assert!(request.validate().is_ok(), "有效的更新请求应通过验证");
    }

    #[test]
    fn test_update_user_request_bio_too_long() {
        let long_bio = "a".repeat(5001);
        let request = UpdateUserRequest {
            bio: Some(long_bio),
            email: None,
        };

        let result = request.validate();
        assert!(result.is_err(), "bio 过长应验证失败");
    }

    #[test]
    fn test_update_user_request_invalid_email() {
        let request = UpdateUserRequest {
            bio: None,
            email: Some("not-an-email".to_string()),
        };

        let result = request.validate();
        assert!(result.is_err(), "无效邮箱应验证失败");
    }

    #[test]
    fn test_user_response_from_user() {
        let user = User {
            id: Uuid::new_v4(),
            username: "testuser".to_string(),
            email: "test@example.com".to_string(),
            password_hash: "hash".to_string(),
            avatar: Some(vec![1, 2, 3]),
            avatar_mime_type: Some("image/png".to_string()),
            bio: Some("Hello".to_string()),
            level: UserLevel::Enhanced,
            status: UserStatus::Active,
            banned_reason: None,
            banned_until: None,
            created_at: Utc::now(),
            updated_at: Utc::now(),
        };

        let response = UserResponse::from_user(&user);

        assert_eq!(response.id, user.id, "ID 应匹配");
        assert_eq!(response.username, "testuser", "用户名应匹配");
        assert_eq!(response.email, "test@example.com", "邮箱应匹配");
        assert!(response.avatar_url.is_some(), "有头像时应返回 URL");
        assert!(
            response
                .avatar_url
                .as_ref()
                .is_some_and(|url| url.contains(&user.id.to_string())),
            "头像 URL 应包含用户 ID"
        );
        assert_eq!(response.level, "enhanced", "等级应为 enhanced");
        assert_eq!(response.status, "active", "状态应为 active");
    }

    #[test]
    fn test_user_response_from_user_without_avatar() {
        let user = User {
            id: Uuid::new_v4(),
            username: "noavatar".to_string(),
            email: "no@example.com".to_string(),
            password_hash: "hash".to_string(),
            avatar: None,
            avatar_mime_type: None,
            bio: None,
            level: UserLevel::Standard,
            status: UserStatus::Active,
            banned_reason: None,
            banned_until: None,
            created_at: Utc::now(),
            updated_at: Utc::now(),
        };

        let response = UserResponse::from_user(&user);

        assert!(response.avatar_url.is_none(), "无头像时应返回 None");
        assert!(response.bio.is_none(), "无 bio 时应返回 None");
    }

    #[test]
    fn test_public_user_response_from_user() {
        let user = User {
            id: Uuid::new_v4(),
            username: "publicuser".to_string(),
            email: "secret@example.com".to_string(),
            password_hash: "hash".to_string(),
            avatar: None,
            avatar_mime_type: None,
            bio: Some("Public bio".to_string()),
            level: UserLevel::Standard,
            status: UserStatus::Active,
            banned_reason: None,
            banned_until: None,
            created_at: Utc::now(),
            updated_at: Utc::now(),
        };

        let response = PublicUserResponse::from_user(&user);

        assert_eq!(response.username, "publicuser", "用户名应匹配");
        assert_eq!(response.level, "standard", "等级应为 standard");
        assert!(response.bio.is_some(), "bio 应存在");
    }

    #[test]
    fn test_token_response_serialization() {
        let response = TokenResponse {
            access_token: "test_token".to_string(),
            expires_in: 3600,
        };

        let json = serde_json::to_string(&response).expect("序列化失败");

        assert!(json.contains("access_token"), "JSON 应包含 access_token");
        assert!(json.contains("expires_in"), "JSON 应包含 expires_in");
        assert!(json.contains("3600"), "JSON 应包含过期时间值");
    }

    #[test]
    fn test_auth_response_serialization() {
        let response = AuthResponse {
            user: UserResponse {
                id: Uuid::new_v4(),
                username: "authuser".to_string(),
                email: "auth@example.com".to_string(),
                avatar_url: None,
                bio: None,
                level: "standard".to_string(),
                status: "active".to_string(),
                created_at: Utc::now(),
                updated_at: None,
            },
            token: TokenResponse {
                access_token: "jwt".to_string(),
                expires_in: 3600,
            },
        };

        let json = serde_json::to_string(&response).expect("序列化失败");

        assert!(json.contains("user"), "JSON 应包含 user");
        assert!(json.contains("token"), "JSON 应包含 token");
        assert!(json.contains("authuser"), "JSON 应包含用户名");
    }

    #[test]
    fn test_login_request_deserialization() {
        let json = r#"{"username":"testuser","password":"testpass"}"#;
        let request: LoginRequest = serde_json::from_str(json).expect("反序列化失败");

        assert_eq!(request.username, "testuser", "用户名应匹配");
        assert_eq!(request.password, "testpass", "密码应匹配");
    }

    #[test]
    fn test_refresh_token_request_deserialization() {
        let json = r#"{"refresh_token":"some_token"}"#;
        let request: RefreshTokenRequest = serde_json::from_str(json).expect("反序列化失败");

        assert_eq!(
            request.refresh_token,
            Some("some_token".to_string()),
            "刷新令牌应匹配"
        );

        let json_empty = r#"{}"#;
        let request_empty: RefreshTokenRequest =
            serde_json::from_str(json_empty).expect("反序列化失败");

        assert!(request_empty.refresh_token.is_none(), "未提供时应为 None");
    }
}

// ============================================================================
// 语言配置模型
// ============================================================================

/// 语言表映射
///
/// 对应数据库 `languages` 表，存储支持的编程语言配置。
///
/// # 示例
///
/// ```
/// use ds4viz_server::model::Language;
///
/// let lang = Language {
///     id: "python".to_string(),
///     display_name: "Python".to_string(),
///     version: "3.14".to_string(),
///     is_interpreted: true,
///     file_extension: ".py".to_string(),
///     enabled: true,
///     timeout_ms: 30000,
///     sort_order: 1,
/// };
///
/// assert_eq!(lang.id, "python");
/// assert!(lang.is_interpreted);
/// ```
#[derive(Debug, Clone, sqlx::FromRow)]
pub struct Language {
    /// 语言唯一标识符
    pub id: String,
    /// 显示名称
    pub display_name: String,
    /// 当前版本号
    pub version: String,
    /// 是否为解释型语言
    pub is_interpreted: bool,
    /// 文件扩展名
    pub file_extension: String,
    /// 是否启用
    pub enabled: bool,
    /// 执行超时时间（毫秒）
    pub timeout_ms: i32,
    /// 显示排序
    pub sort_order: i32,
}

// ============================================================================
// 代码执行模型
// ============================================================================

/// 代码提交表映射
///
/// 对应数据库 `code_submissions` 表，记录用户的代码提交历史。
///
/// # 示例
///
/// ```
/// use ds4viz_server::model::{CodeSubmission, ExecutionStatus};
/// use chrono::Utc;
/// use uuid::Uuid;
///
/// let submission = CodeSubmission {
///     id: Uuid::new_v4(),
///     user_id: Uuid::new_v4(),
///     language_id: "python".to_string(),
///     code_hash: "abc123...".to_string(),
///     status: ExecutionStatus::Success,
///     cache_hit: false,
///     cpu_time_ms: Some(234),
///     created_at: Utc::now(),
/// };
///
/// assert_eq!(submission.language_id, "python");
/// assert!(!submission.cache_hit);
/// ```
#[derive(Debug, Clone, sqlx::FromRow)]
pub struct CodeSubmission {
    /// 提交唯一标识
    pub id: Uuid,
    /// 提交用户ID
    pub user_id: Uuid,
    /// 使用的语言ID
    pub language_id: String,
    /// 代码哈希值
    pub code_hash: String,
    /// 执行状态
    pub status: ExecutionStatus,
    /// 是否命中缓存
    pub cache_hit: bool,
    /// CPU耗时（毫秒），缓存命中时为None
    pub cpu_time_ms: Option<i64>,
    /// 提交时间
    pub created_at: DateTime<Utc>,
}

/// 执行状态枚举
///
/// # 示例
///
/// ```
/// use ds4viz_server::model::ExecutionStatus;
///
/// let status = ExecutionStatus::Success;
/// assert_eq!(status.as_str(), "success");
/// assert!(status.is_completed());
/// ```
#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize, sqlx::Type)]
#[sqlx(type_name = "VARCHAR", rename_all = "lowercase")]
#[serde(rename_all = "lowercase")]
pub enum ExecutionStatus {
    /// 等待执行
    Pending,
    /// 执行中
    Running,
    /// 执行成功
    Success,
    /// 执行失败
    Failed,
    /// 执行超时
    Timeout,
}

impl ExecutionStatus {
    /// 转换为字符串表示
    #[must_use]
    pub fn as_str(&self) -> &'static str {
        match self {
            Self::Pending => "pending",
            Self::Running => "running",
            Self::Success => "success",
            Self::Failed => "failed",
            Self::Timeout => "timeout",
        }
    }

    /// 判断是否已完成（成功、失败或超时）
    #[must_use]
    pub fn is_completed(&self) -> bool {
        matches!(self, Self::Success | Self::Failed | Self::Timeout)
    }
}

/// 执行缓存表映射
///
/// 对应数据库 `execution_cache` 表，缓存代码执行结果。
///
/// # 示例
///
/// ```
/// use ds4viz_server::model::ExecutionCache;
/// use chrono::Utc;
///
/// let cache = ExecutionCache {
///     code_hash: "abc123...".to_string(),
///     language_id: "python".to_string(),
///     code: "print('hello')".to_string(),
///     stdout: Some("hello\n".to_string()),
///     stderr: Some("".to_string()),
///     exit_code: Some(0),
///     cpu_time_ms: Some(100),
///     created_at: Utc::now(),
/// };
///
/// assert_eq!(cache.exit_code, Some(0));
/// ```
#[derive(Debug, Clone, sqlx::FromRow)]
pub struct ExecutionCache {
    /// 代码哈希值（SHA256）
    pub code_hash: String,
    /// 语言ID
    pub language_id: String,
    /// 原始代码
    pub code: String,
    /// 标准输出
    pub stdout: Option<String>,
    /// 标准错误
    pub stderr: Option<String>,
    /// 退出码
    pub exit_code: Option<i32>,
    /// CPU耗时（毫秒）
    pub cpu_time_ms: Option<i64>,
    /// 首次执行时间
    pub created_at: DateTime<Utc>,
}

/// 用户配额使用表映射
///
/// 对应数据库 `user_quota_usage` 表，追踪用户在指定周期内的配额使用情况。
///
/// # 示例
///
/// ```
/// use ds4viz_server::model::UserQuotaUsage;
/// use chrono::Utc;
/// use uuid::Uuid;
///
/// let usage = UserQuotaUsage {
///     id: Uuid::new_v4(),
///     user_id: Uuid::new_v4(),
///     period_start: Utc::now(),
///     cpu_time_used_ms: 5000,
/// };
///
/// assert_eq!(usage.cpu_time_used_ms, 5000);
/// ```
#[derive(Debug, Clone, sqlx::FromRow)]
pub struct UserQuotaUsage {
    /// 记录唯一标识
    pub id: Uuid,
    /// 用户ID
    pub user_id: Uuid,
    /// 周期起始时间
    pub period_start: DateTime<Utc>,
    /// 已使用的CPU时间（毫秒）
    pub cpu_time_used_ms: i64,
}

// ============================================================================
// 执行相关 API 请求/响应模型
// ============================================================================

/// 代码执行请求
///
/// # 示例
///
/// ```
/// use ds4viz_server::model::ExecuteCodeRequest;
/// use validator::Validate;
///
/// let request = ExecuteCodeRequest {
///     language_id: "python".to_string(),
///     code: "print('hello')".to_string(),
/// };
///
/// assert!(request.validate().is_ok());
/// ```
#[derive(Debug, Deserialize, Validate)]
pub struct ExecuteCodeRequest {
    /// 语言ID
    #[validate(length(min = 1, max = 32))]
    pub language_id: String,
    /// 代码内容
    #[validate(length(min = 1))]
    pub code: String,
}

/// 语言信息响应
///
/// # 示例
///
/// ```
/// use ds4viz_server::model::LanguageInfo;
///
/// let lang = LanguageInfo {
///     id: "python".to_string(),
///     display_name: "Python".to_string(),
///     version: "3.14".to_string(),
///     file_extension: ".py".to_string(),
/// };
///
/// assert_eq!(lang.id, "python");
/// ```
#[derive(Debug, Serialize, Deserialize)]
pub struct LanguageInfo {
    /// 语言ID
    pub id: String,
    /// 显示名称
    pub display_name: String,
    /// 版本号
    pub version: String,
    /// 文件扩展名
    pub file_extension: String,
}

impl LanguageInfo {
    /// 从 Language 创建 LanguageInfo
    #[must_use]
    pub fn from_language(lang: &Language) -> Self {
        Self {
            id: lang.id.clone(),
            display_name: lang.display_name.clone(),
            version: lang.version.clone(),
            file_extension: lang.file_extension.clone(),
        }
    }
}

/// 执行结果详情
///
/// # 示例
///
/// ```
/// use ds4viz_server::model::ExecutionResult;
///
/// let result = ExecutionResult {
///     stdout: "hello\n".to_string(),
///     stderr: String::new(),
///     exit_code: 0,
///     cpu_time_ms: 123,
/// };
///
/// assert_eq!(result.exit_code, 0);
/// ```
#[derive(Debug, Serialize, Deserialize)]
pub struct ExecutionResult {
    /// 标准输出
    pub stdout: String,
    /// 标准错误
    pub stderr: String,
    /// 退出码
    pub exit_code: i32,
    /// CPU耗时（毫秒）
    pub cpu_time_ms: i64,
}

/// 代码执行响应
///
/// # 示例
///
/// ```
/// use ds4viz_server::model::{ExecutionResponse, ExecutionStatus, LanguageInfo};
/// use chrono::Utc;
/// use uuid::Uuid;
///
/// let response = ExecutionResponse {
///     id: Uuid::new_v4(),
///     status: ExecutionStatus::Pending,
///     cache_hit: false,
///     language: LanguageInfo {
///         id: "python".to_string(),
///         display_name: "Python".to_string(),
///         version: "3.14".to_string(),
///         file_extension: ".py".to_string(),
///     },
///     code: None,
///     result: None,
///     created_at: Utc::now(),
/// };
///
/// assert_eq!(response.status, ExecutionStatus::Pending);
/// assert!(!response.cache_hit);
/// ```
#[derive(Debug, Serialize)]
pub struct ExecutionResponse {
    /// 执行ID
    pub id: Uuid,
    /// 执行状态
    pub status: ExecutionStatus,
    /// 是否命中缓存
    pub cache_hit: bool,
    /// 语言信息
    pub language: LanguageInfo,
    /// 代码内容（仅查询详情时返回）
    #[serde(skip_serializing_if = "Option::is_none")]
    pub code: Option<String>,
    /// 执行结果（完成后才有）
    #[serde(skip_serializing_if = "Option::is_none")]
    pub result: Option<ExecutionResult>,
    /// 提交时间
    pub created_at: DateTime<Utc>,
}

/// 语言列表响应
///
/// # 示例
///
/// ```
/// use ds4viz_server::model::LanguageResponse;
///
/// let response = LanguageResponse {
///     id: "python".to_string(),
///     display_name: "Python".to_string(),
///     version: "3.14".to_string(),
///     is_interpreted: true,
///     file_extension: ".py".to_string(),
///     enabled: true,
///     timeout_ms: 30000,
/// };
///
/// assert!(response.is_interpreted);
/// assert!(response.enabled);
/// ```
#[derive(Debug, Serialize)]
pub struct LanguageResponse {
    /// 语言ID
    pub id: String,
    /// 显示名称
    pub display_name: String,
    /// 版本号
    pub version: String,
    /// 是否为解释型语言
    pub is_interpreted: bool,
    /// 文件扩展名
    pub file_extension: String,
    /// 是否启用
    pub enabled: bool,
    /// 执行超时时间（毫秒）
    pub timeout_ms: i32,
}

impl LanguageResponse {
    /// 从 Language 创建响应
    #[must_use]
    pub fn from_language(lang: &Language) -> Self {
        Self {
            id: lang.id.clone(),
            display_name: lang.display_name.clone(),
            version: lang.version.clone(),
            is_interpreted: lang.is_interpreted,
            file_extension: lang.file_extension.clone(),
            enabled: lang.enabled,
            timeout_ms: lang.timeout_ms,
        }
    }
}

/// 配额状态响应（Standard用户）
///
/// # 示例
///
/// ```
/// use ds4viz_server::model::QuotaStatusLimited;
/// use chrono::Utc;
///
/// let status = QuotaStatusLimited {
///     period_start: Utc::now(),
///     period_end: Utc::now(),
///     limit_ms: 10000,
///     used_ms: 3456,
///     remaining_ms: 6544,
///     usage_percentage: 34.56,
/// };
///
/// assert_eq!(status.limit_ms, 10000);
/// assert_eq!(status.used_ms, 3456);
/// ```
#[derive(Debug, Serialize)]
pub struct QuotaStatusLimited {
    /// 周期起始时间
    pub period_start: DateTime<Utc>,
    /// 周期结束时间
    pub period_end: DateTime<Utc>,
    /// 配额限制（毫秒）
    pub limit_ms: i64,
    /// 已使用（毫秒）
    pub used_ms: i64,
    /// 剩余（毫秒）
    pub remaining_ms: i64,
    /// 使用百分比
    pub usage_percentage: f64,
}

/// 配额查询响应
///
/// # 示例
///
/// ```
/// use ds4viz_server::model::{QuotaResponse, QuotaStatusLimited};
/// use chrono::Utc;
///
/// let response = QuotaResponse::Limited {
///     level: "standard".to_string(),
///     quota: QuotaStatusLimited {
///         period_start: Utc::now(),
///         period_end: Utc::now(),
///         limit_ms: 10000,
///         used_ms: 5000,
///         remaining_ms: 5000,
///         usage_percentage: 50.0,
///     },
/// };
///
/// match response {
///     QuotaResponse::Limited { level, .. } => assert_eq!(level, "standard"),
///     _ => panic!("应该是 Limited"),
/// }
/// ```
#[derive(Debug, Serialize)]
#[serde(untagged)]
pub enum QuotaResponse {
    /// Enhanced 用户无限制
    Unlimited { level: String },
    /// Standard 用户有限制
    Limited {
        level: String,
        quota: QuotaStatusLimited,
    },
}
