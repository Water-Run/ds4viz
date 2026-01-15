//! 认证模块

use std::sync::Arc;

use argon2::password_hash::rand_core::OsRng;
use argon2::password_hash::SaltString;
use argon2::{Argon2, PasswordHash, PasswordHasher, PasswordVerifier};
use chrono::{Duration, Utc};
use jsonwebtoken::{decode, encode, DecodingKey, EncodingKey, Header, Validation};
use poem::http::header::AUTHORIZATION;
use poem::web::cookie::{Cookie, CookieJar};
use poem::web::{Data, Json};
use poem::{handler, Endpoint, IntoResponse, Middleware, Request, Response, Result, Route};
use serde::{Deserialize, Serialize};
use uuid::Uuid;

use crate::app::AppState;
use crate::config::JwtConfig;
use crate::db::DbPool;
use crate::error::{ApiError, ErrorCode};
use crate::model::{User, UserPrivateProfile, UserStatus};

/// JWT 令牌声明
#[derive(Debug, Serialize, Deserialize)]
pub struct TokenClaims {
    pub sub: Uuid,
    pub exp: i64,
    pub iat: i64,
    pub token_type: TokenType,
}

/// 令牌类型
#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
#[serde(rename_all = "snake_case")]
pub enum TokenType {
    Access,
    Refresh,
}

/// 令牌响应
#[derive(Debug, Serialize)]
pub struct TokenResponse {
    pub access_token: String,
    pub expires_in: u64,
}

/// 注册请求
#[derive(Debug, Deserialize)]
pub struct RegisterRequest {
    pub username: String,
    pub email: String,
    pub password: String,
}

/// 登录请求
#[derive(Debug, Deserialize)]
pub struct LoginRequest {
    pub username: String,
    pub password: String,
}

/// 刷新令牌请求
#[derive(Debug, Deserialize)]
pub struct RefreshRequest {
    pub refresh_token: Option<String>,
}

/// 注册响应
#[derive(Debug, Serialize)]
pub struct AuthResponse {
    pub user: UserPrivateProfile,
    pub token: TokenResponse,
}

/// 已认证用户（用于中间件注入）
#[derive(Debug, Clone)]
pub struct AuthenticatedUser(pub User);

/// 认证服务
pub struct AuthService {
    pool: DbPool,
    jwt_config: Arc<JwtConfig>,
    argon2: Argon2<'static>,
}

impl AuthService {
    #[must_use]
    pub fn new(pool: DbPool, jwt_config: Arc<JwtConfig>) -> Self {
        Self {
            pool,
            jwt_config,
            argon2: Argon2::default(),
        }
    }

    pub async fn register(&self, req: RegisterRequest) -> Result<User, ApiError> {
        Self::validate_username(&req.username)?;
        Self::validate_email(&req.email)?;
        Self::validate_password(&req.password)?;

        let password_hash = self.hash_password(&req.password)?;

        let user = sqlx::query_as::<_, User>(
            r#"
            INSERT INTO users (id, username, email, password_hash, level, status, created_at, updated_at)
            VALUES ($1, $2, $3, $4, 'standard', 'active', NOW(), NOW())
            RETURNING *
            "#,
        )
            .bind(Uuid::new_v4())
            .bind(&req.username)
            .bind(&req.email)
            .bind(&password_hash)
            .fetch_one(&self.pool)
            .await?;

        tracing::info!(user_id = %user.id, username = %user.username, "用户注册成功");

        Ok(user)
    }

    pub async fn login(&self, req: LoginRequest) -> Result<User, ApiError> {
        let user = sqlx::query_as::<_, User>(
            "SELECT * FROM users WHERE username = $1 OR email = $1",
        )
            .bind(&req.username)
            .fetch_optional(&self.pool)
            .await?
            .ok_or_else(|| ApiError::new(ErrorCode::InvalidCredentials, "用户名或密码错误"))?;

        self.verify_password(&req.password, &user.password_hash)?;

        match user.status_enum() {
            UserStatus::Active => {}
            UserStatus::Suspended => {
                return Err(ApiError::new(ErrorCode::AccountSuspended, "账户已暂停"));
            }
            UserStatus::Banned => {
                let mut error = ApiError::new(ErrorCode::AccountBanned, "账户已被封禁");
                if user.banned_reason.is_some() || user.banned_until.is_some() {
                    error = error.with_details(serde_json::json!({
                        "reason": user.banned_reason,
                        "banned_until": user.banned_until
                    }));
                }
                return Err(error);
            }
        }

        tracing::info!(user_id = %user.id, "用户登录成功");

        Ok(user)
    }

    pub async fn authenticate(&self, token: &str) -> Result<User, ApiError> {
        let claims = self.verify_token(token)?;

        if claims.token_type != TokenType::Access {
            return Err(ApiError::new(ErrorCode::InvalidToken, "无效的访问令牌"));
        }

        let user = sqlx::query_as::<_, User>("SELECT * FROM users WHERE id = $1")
            .bind(claims.sub)
            .fetch_optional(&self.pool)
            .await?
            .ok_or_else(|| ApiError::new(ErrorCode::UserNotFound, "用户不存在"))?;

        if !user.status_enum().can_login() {
            return Err(ApiError::new(ErrorCode::Forbidden, "账户状态异常"));
        }

        Ok(user)
    }

    pub async fn refresh(&self, refresh_token: &str) -> Result<TokenResponse, ApiError> {
        let claims = self.verify_token(refresh_token)?;

        if claims.token_type != TokenType::Refresh {
            return Err(ApiError::new(ErrorCode::InvalidToken, "无效的刷新令牌"));
        }

        let user = sqlx::query_as::<_, User>("SELECT * FROM users WHERE id = $1")
            .bind(claims.sub)
            .fetch_optional(&self.pool)
            .await?
            .ok_or_else(|| ApiError::new(ErrorCode::UserNotFound, "用户不存在"))?;

        if !user.status_enum().can_login() {
            return Err(ApiError::new(ErrorCode::Forbidden, "账户状态异常"));
        }

        self.generate_access_token(&user)
    }

    pub fn generate_tokens(&self, user: &User) -> Result<(TokenResponse, String), ApiError> {
        let access = self.generate_access_token(user)?;
        let refresh = self.generate_refresh_token(user)?;
        Ok((access, refresh))
    }

    fn generate_access_token(&self, user: &User) -> Result<TokenResponse, ApiError> {
        let now = Utc::now();
        let claims = TokenClaims {
            sub: user.id,
            exp: (now + Duration::seconds(self.jwt_config.access_token_expires_secs as i64))
                .timestamp(),
            iat: now.timestamp(),
            token_type: TokenType::Access,
        };

        let token = encode(
            &Header::default(),
            &claims,
            &EncodingKey::from_secret(self.jwt_config.secret.as_bytes()),
        )
            .map_err(|e| {
                tracing::error!(error = %e, "生成访问令牌失败");
                ApiError::internal("令牌生成失败")
            })?;

        Ok(TokenResponse {
            access_token: token,
            expires_in: self.jwt_config.access_token_expires_secs,
        })
    }

    fn generate_refresh_token(&self, user: &User) -> Result<String, ApiError> {
        let now = Utc::now();
        let claims = TokenClaims {
            sub: user.id,
            exp: (now + Duration::seconds(self.jwt_config.refresh_token_expires_secs as i64))
                .timestamp(),
            iat: now.timestamp(),
            token_type: TokenType::Refresh,
        };

        encode(
            &Header::default(),
            &claims,
            &EncodingKey::from_secret(self.jwt_config.secret.as_bytes()),
        )
            .map_err(|e| {
                tracing::error!(error = %e, "生成刷新令牌失败");
                ApiError::internal("令牌生成失败")
            })
    }

    fn verify_token(&self, token: &str) -> Result<TokenClaims, ApiError> {
        decode::<TokenClaims>(
            token,
            &DecodingKey::from_secret(self.jwt_config.secret.as_bytes()),
            &Validation::default(),
        )
            .map(|data| data.claims)
            .map_err(|e| {
                use jsonwebtoken::errors::ErrorKind;
                match e.kind() {
                    ErrorKind::ExpiredSignature => {
                        ApiError::new(ErrorCode::TokenExpired, "令牌已过期")
                    }
                    _ => ApiError::new(ErrorCode::InvalidToken, "令牌无效"),
                }
            })
    }

    fn validate_username(username: &str) -> Result<(), ApiError> {
        if username.len() < 4 || username.len() > 64 {
            return Err(ApiError::new(
                ErrorCode::InvalidUsername,
                "用户名长度必须在 4-64 字符之间",
            ));
        }

        if !username
            .chars()
            .all(|c| c.is_ascii_alphanumeric() || c == '_')
        {
            return Err(ApiError::new(
                ErrorCode::InvalidUsername,
                "用户名只能包含字母、数字和下划线",
            ));
        }

        Ok(())
    }

    fn validate_email(email: &str) -> Result<(), ApiError> {
        if !email.contains('@') || email.len() < 5 || email.len() > 255 {
            return Err(ApiError::new(ErrorCode::InvalidEmail, "邮箱格式无效"));
        }

        Ok(())
    }

    fn validate_password(password: &str) -> Result<(), ApiError> {
        if password.len() < 8 || password.len() > 128 {
            return Err(ApiError::new(
                ErrorCode::WeakPassword,
                "密码长度必须在 8-128 字符之间",
            ));
        }

        let has_letter = password.chars().any(|c| c.is_ascii_alphabetic());
        let has_digit = password.chars().any(|c| c.is_ascii_digit());

        if !has_letter || !has_digit {
            return Err(ApiError::new(
                ErrorCode::WeakPassword,
                "密码必须包含字母和数字",
            ));
        }

        Ok(())
    }

    fn hash_password(&self, password: &str) -> Result<String, ApiError> {
        let salt = SaltString::generate(&mut OsRng);
        self.argon2
            .hash_password(password.as_bytes(), &salt)
            .map(|h| h.to_string())
            .map_err(|e| {
                tracing::error!(error = %e, "密码哈希失败");
                ApiError::internal("密码处理失败")
            })
    }

    fn verify_password(&self, password: &str, hash: &str) -> Result<(), ApiError> {
        let parsed_hash =
            PasswordHash::new(hash).map_err(|_| ApiError::internal("密码哈希格式错误"))?;

        self.argon2
            .verify_password(password.as_bytes(), &parsed_hash)
            .map_err(|_| ApiError::new(ErrorCode::InvalidCredentials, "用户名或密码错误"))
    }
}

// ===== HTTP Handlers =====

#[handler]
async fn register_handler(
    state: Data<&AppState>,
    Json(req): Json<RegisterRequest>,
    cookie_jar: &CookieJar,
) -> Result<Response> {
    let auth_service = AuthService::new(state.pool.clone(), Arc::clone(&state.config.jwt));

    let user = auth_service.register(req).await?;
    let (token, refresh_token) = auth_service.generate_tokens(&user)?;

    let mut refresh_cookie = Cookie::new("refresh_token", refresh_token);
    refresh_cookie.set_http_only(true);
    refresh_cookie.set_path("/api/v1/auth");
    cookie_jar.add(refresh_cookie);

    let response = AuthResponse {
        user: UserPrivateProfile::from(&user),
        token,
    };

    Ok(Json(serde_json::json!({ "data": response }))
        .with_status(poem::http::StatusCode::CREATED)
        .into_response())
}

#[handler]
async fn login_handler(
    state: Data<&AppState>,
    Json(req): Json<LoginRequest>,
    cookie_jar: &CookieJar,
) -> Result<Json<serde_json::Value>> {
    let auth_service = AuthService::new(state.pool.clone(), Arc::clone(&state.config.jwt));

    let user = auth_service.login(req).await?;
    let (token, refresh_token) = auth_service.generate_tokens(&user)?;

    let mut refresh_cookie = Cookie::new("refresh_token", refresh_token);
    refresh_cookie.set_http_only(true);
    refresh_cookie.set_path("/api/v1/auth");
    cookie_jar.add(refresh_cookie);

    let response = AuthResponse {
        user: UserPrivateProfile::from(&user),
        token,
    };

    Ok(Json(serde_json::json!({ "data": response })))
}

#[handler]
async fn logout_handler(cookie_jar: &CookieJar) -> Response {
    cookie_jar.remove("refresh_token");
    Response::builder()
        .status(poem::http::StatusCode::NO_CONTENT)
        .finish()
}

#[handler]
async fn refresh_handler(
    state: Data<&AppState>,
    cookie_jar: &CookieJar,
    Json(req): Json<RefreshRequest>,
) -> Result<Json<serde_json::Value>> {
    let refresh_token = req
        .refresh_token
        .or_else(|| cookie_jar.get("refresh_token").map(|c| c.value_str().to_owned()))
        .ok_or_else(|| ApiError::new(ErrorCode::InvalidToken, "未提供刷新令牌"))?;

    let auth_service = AuthService::new(state.pool.clone(), Arc::clone(&state.config.jwt));
    let token = auth_service.refresh(&refresh_token).await?;

    Ok(Json(serde_json::json!({ "data": token })))
}

#[handler]
async fn me_handler(req: &Request) -> Result<Json<serde_json::Value>> {
    let user = req
        .extensions()
        .get::<AuthenticatedUser>()
        .ok_or_else(|| ApiError::unauthorized("未认证"))?;

    let profile = UserPrivateProfile::from(&user.0);
    Ok(Json(serde_json::json!({ "data": profile })))
}

/// 构建认证路由
#[must_use]
pub fn auth_routes() -> Route {
    Route::new()
        .at("/register", poem::post(register_handler))
        .at("/login", poem::post(login_handler))
        .at("/logout", poem::post(logout_handler))
        .at("/refresh", poem::post(refresh_handler))
        .at("/me", poem::get(me_handler).with(AuthMiddleware))
}

// ===== Middleware =====

/// 认证中间件
pub struct AuthMiddleware;

impl<E: Endpoint> Middleware<E> for AuthMiddleware {
    type Output = AuthMiddlewareEndpoint<E>;

    fn transform(&self, ep: E) -> Self::Output {
        AuthMiddlewareEndpoint { inner: ep }
    }
}

pub struct AuthMiddlewareEndpoint<E> {
    inner: E,
}

impl<E: Endpoint> Endpoint for AuthMiddlewareEndpoint<E> {
    type Output = E::Output;

    async fn call(&self, mut req: Request) -> Result<Self::Output> {
        let state = req
            .data::<AppState>()
            .ok_or_else(|| ApiError::internal("应用状态未初始化"))?
            .clone();

        let token = extract_token(&req)?;
        let auth_service = AuthService::new(state.pool.clone(), Arc::clone(&state.config.jwt));
        let user = auth_service.authenticate(&token).await?;

        req.extensions_mut().insert(AuthenticatedUser(user));

        self.inner.call(req).await
    }
}

fn extract_token(req: &Request) -> Result<String, ApiError> {
    if let Some(auth_header) = req.headers().get(AUTHORIZATION) {
        let auth_str = auth_header
            .to_str()
            .map_err(|_| ApiError::new(ErrorCode::InvalidToken, "无效的 Authorization 头"))?;

        if let Some(token) = auth_str.strip_prefix("Bearer ") {
            return Ok(token.to_owned());
        }
    }

    if let Some(cookie) = req.cookie().get("access_token") {
        return Ok(cookie.value_str().to_owned());
    }

    Err(ApiError::unauthorized("未提供认证令牌"))
}

/// 可选认证中间件
pub struct OptionalAuthMiddleware;

impl<E: Endpoint> Middleware<E> for OptionalAuthMiddleware {
    type Output = OptionalAuthMiddlewareEndpoint<E>;

    fn transform(&self, ep: E) -> Self::Output {
        OptionalAuthMiddlewareEndpoint { inner: ep }
    }
}

pub struct OptionalAuthMiddlewareEndpoint<E> {
    inner: E,
}

impl<E: Endpoint> Endpoint for OptionalAuthMiddlewareEndpoint<E> {
    type Output = E::Output;

    async fn call(&self, mut req: Request) -> Result<Self::Output> {
        if let Ok(token) = extract_token(&req) {
            if let Some(state) = req.data::<AppState>().cloned() {
                let auth_service =
                    AuthService::new(state.pool.clone(), Arc::clone(&state.config.jwt));

                if let Ok(user) = auth_service.authenticate(&token).await {
                    req.extensions_mut().insert(AuthenticatedUser(user));
                }
            }
        }

        self.inner.call(req).await
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_validate_username_valid() {
        assert!(AuthService::validate_username("test_user").is_ok());
        assert!(AuthService::validate_username("User123").is_ok());
    }

    #[test]
    fn test_validate_username_invalid() {
        assert!(AuthService::validate_username("ab").is_err());
        assert!(AuthService::validate_username("user@name").is_err());
        assert!(AuthService::validate_username("user name").is_err());
    }

    #[test]
    fn test_validate_email_valid() {
        assert!(AuthService::validate_email("user@example.com").is_ok());
        assert!(AuthService::validate_email("a@b.co").is_ok());
    }

    #[test]
    fn test_validate_email_invalid() {
        assert!(AuthService::validate_email("invalid").is_err());
        assert!(AuthService::validate_email("@no-local").is_err());
    }

    #[test]
    fn test_validate_password_valid() {
        assert!(AuthService::validate_password("Password1").is_ok());
        assert!(AuthService::validate_password("Abcd1234").is_ok());
    }

    #[test]
    fn test_validate_password_invalid() {
        assert!(AuthService::validate_password("short1").is_err());
        assert!(AuthService::validate_password("noDIGITS").is_err());
        assert!(AuthService::validate_password("12345678").is_err());
    }

    #[test]
    fn test_token_type_serialization() {
        let access = serde_json::to_string(&TokenType::Access).unwrap();
        let refresh = serde_json::to_string(&TokenType::Refresh).unwrap();

        assert_eq!(access, "\"access\"");
        assert_eq!(refresh, "\"refresh\"");
    }
}