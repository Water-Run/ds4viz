//! 应用路由与状态组装模块
//!
//! 本模块负责创建 Poem 应用实例，组装路由和共享状态。
//!
//! # 路由结构
//!
//! ```text
//! /api/v1
//! ├── /auth
//! │   ├── POST /register     - 用户注册
//! │   ├── POST /login        - 用户登录
//! │   ├── POST /logout       - 用户登出
//! │   ├── POST /refresh      - 刷新令牌
//! │   └── GET /me            - 获取当前用户信息
//! ├── /users
//! │   ├── GET /search        - 搜索用户（待实现）
//! │   ├── GET /{user_id}     - 获取用户公开信息（待实现）
//! │   ├── PUT /me            - 更新用户信息（待实现）
//! │   ├── PUT /me/avatar     - 上传头像（待实现）
//! │   └── GET /{user_id}/avatar - 获取头像（待实现）
//! └── ...（其他模块待实现）
//! ```
//!
//! 作者: WaterRun
//! 日期: 2026-01-16

use std::sync::Arc;

use poem::http::StatusCode;
use poem::web::{Data, Json, cookie::Cookie};
use poem::{Endpoint, EndpointExt, IntoResponse, Response, Route, get, handler, post, put};
use serde::{Deserialize, Serialize};
use uuid::Uuid;
use validator::Validate;

use crate::auth::{CurrentUser, JwtManager, TokenType, hash_password, verify_password};
use crate::config::Config;
use crate::db::Database;
use crate::error::{AppError, AuthErrorKind, ConflictKind, ForbiddenKind};
use crate::model::{
    AuthResponse, LoginRequest, RefreshTokenRequest, RegisterRequest, TokenResponse, UserResponse,
};

// ============================================================================
// 应用状态
// ============================================================================

/// 应用共享状态
///
/// 包含数据库连接、配置和认证管理器等跨请求共享的资源。
///
/// # 示例
///
/// ```rust,no_run
/// use ds4viz_server::app::AppState;
/// use ds4viz_server::config::Config;
/// use ds4viz_server::db::Database;
/// use ds4viz_server::auth::JwtManager;
///
/// # async fn example() -> Result<(), Box<dyn std::error::Error>> {
/// let config = Config::from_file("configs/default.toml")?;
/// let db = Database::connect(&config.database).await?;
/// let jwt_manager = JwtManager::new(&config.jwt);
///
/// let state = AppState {
///     db,
///     config,
///     jwt_manager,
/// };
/// # Ok(())
/// # }
/// ```
#[derive(Clone)]
pub struct AppState {
    /// 数据库连接池
    pub db: Database,
    /// 应用配置
    pub config: Config,
    /// JWT 管理器
    pub jwt_manager: JwtManager,
    /// 执行服务
    pub execution_service: crate::executions::ExecutionService,
    /// 配额管理器
    pub quota_manager: std::sync::Arc<crate::quota::QuotaManager>,
}

// ============================================================================
// API 响应包装
// ============================================================================

/// API 成功响应包装
#[derive(Serialize)]
struct ApiResponse<T: Serialize> {
    data: T,
}

impl<T: Serialize> ApiResponse<T> {
    fn new(data: T) -> Self {
        Self { data }
    }
}

// ============================================================================
// 应用创建
// ============================================================================

/// 创建 Poem 应用实例
///
/// 组装所有路由并附加共享状态。
///
/// # 参数
///
/// - `state`: 应用共享状态
///
/// # 返回值
///
/// 返回配置完成的 Poem 应用端点。
///
/// # 示例
///
/// ```rust,no_run
/// use ds4viz_server::app::{create_app, AppState};
/// use ds4viz_server::config::Config;
/// use ds4viz_server::db::Database;
/// use ds4viz_server::auth::JwtManager;
///
/// # async fn example() -> Result<(), Box<dyn std::error::Error>> {
/// let config = Config::from_file("configs/default.toml")?;
/// let db = Database::connect(&config.database).await?;
/// let jwt_manager = JwtManager::new(&config.jwt);
///
/// let state = AppState { db, config, jwt_manager };
/// let app = create_app(state);
/// # Ok(())
/// # }
/// ```
#[must_use]
pub fn create_app(state: AppState) -> impl Endpoint {
    let api_routes = Route::new()
        .nest("/auth", auth_routes())
        .nest("/users", user_routes())
        .nest("/executions", execution_routes())
        .nest("/languages", language_routes())
        .nest("/quota", quota_routes());

    Route::new()
        .nest("/api/v1", api_routes)
        .data(Arc::new(state))
}

// ============================================================================
// 认证路由
// ============================================================================

fn auth_routes() -> Route {
    Route::new()
        .at("/register", post(handle_register))
        .at("/login", post(handle_login))
        .at("/logout", post(handle_logout))
        .at("/refresh", post(handle_refresh))
        .at("/me", get(handle_me))
}

/// 用户注册处理
#[handler]
async fn handle_register(
    state: Data<&Arc<AppState>>,
    Json(req): Json<RegisterRequest>,
) -> poem::Result<Response> {
    // 验证请求
    req.validate().map_err(|e| {
        AppError::InvalidRequest(crate::error::InvalidRequestKind::ValidationFailed {
            message: e.to_string(),
        })
    })?;

    // 检查用户名是否已存在
    if state.db.username_exists(&req.username).await? {
        return Err(AppError::Conflict(ConflictKind::UsernameExists {
            username: req.username,
        })
        .into());
    }

    // 检查邮箱是否已存在
    if state.db.email_exists(&req.email).await? {
        return Err(AppError::Conflict(ConflictKind::EmailExists { email: req.email }).into());
    }

    // 哈希密码
    let password_hash = hash_password(&req.password)?;

    // 创建用户
    let user = state
        .db
        .create_user(&req.username, &req.email, &password_hash)
        .await?;

    // 生成令牌
    let (access_token, expires_in) = state.jwt_manager.generate_access_token(user.id)?;
    let (refresh_token, _) = state.jwt_manager.generate_refresh_token(user.id)?;

    // 构建响应
    let response = AuthResponse {
        user: UserResponse::from_user(&user),
        token: TokenResponse {
            access_token: access_token.clone(),
            expires_in,
        },
    };

    // 设置 Cookie
    let mut resp = Json(ApiResponse::new(response)).into_response();
    resp.set_status(StatusCode::CREATED);

    let mut access_cookie = Cookie::new("access_token", access_token);
    access_cookie.set_http_only(true);
    access_cookie.set_secure(true);
    access_cookie.set_path("/");

    let mut refresh_cookie = Cookie::new("refresh_token", refresh_token);
    refresh_cookie.set_http_only(true);
    refresh_cookie.set_secure(true);
    refresh_cookie.set_path("/api/v1/auth/refresh");

    resp.headers_mut()
        .append("Set-Cookie", access_cookie.to_string().parse().unwrap());
    resp.headers_mut()
        .append("Set-Cookie", refresh_cookie.to_string().parse().unwrap());

    Ok(resp)
}

/// 用户登录处理
#[handler]
async fn handle_login(
    state: Data<&Arc<AppState>>,
    Json(req): Json<LoginRequest>,
) -> poem::Result<Response> {
    // 查找用户
    let user = state
        .db
        .find_user_by_username_or_email(&req.username)
        .await?
        .ok_or(AppError::Unauthorized(AuthErrorKind::InvalidCredentials))?;

    // 验证密码
    let valid = verify_password(&req.password, &user.password_hash)?;
    if !valid {
        return Err(AppError::Unauthorized(AuthErrorKind::InvalidCredentials).into());
    }

    // 检查账户状态
    if !user.status.is_active() {
        return Err(match user.status {
            crate::model::UserStatus::Banned => AppError::Forbidden(ForbiddenKind::AccountBanned {
                reason: user
                    .banned_reason
                    .unwrap_or_else(|| "未提供原因".to_string()),
                until: user.banned_until,
            }),
            crate::model::UserStatus::Suspended => {
                AppError::Forbidden(ForbiddenKind::AccountSuspended)
            }
            crate::model::UserStatus::Active => unreachable!(),
        }
        .into());
    }

    // 生成令牌
    let (access_token, expires_in) = state.jwt_manager.generate_access_token(user.id)?;
    let (refresh_token, _) = state.jwt_manager.generate_refresh_token(user.id)?;

    // 构建响应
    let response = AuthResponse {
        user: UserResponse::from_user(&user),
        token: TokenResponse {
            access_token: access_token.clone(),
            expires_in,
        },
    };

    // 设置 Cookie
    let mut resp = Json(ApiResponse::new(response)).into_response();

    let mut access_cookie = Cookie::new("access_token", access_token);
    access_cookie.set_http_only(true);
    access_cookie.set_secure(true);
    access_cookie.set_path("/");

    let mut refresh_cookie = Cookie::new("refresh_token", refresh_token);
    refresh_cookie.set_http_only(true);
    refresh_cookie.set_secure(true);
    refresh_cookie.set_path("/api/v1/auth/refresh");

    resp.headers_mut()
        .append("Set-Cookie", access_cookie.to_string().parse().unwrap());
    resp.headers_mut()
        .append("Set-Cookie", refresh_cookie.to_string().parse().unwrap());

    Ok(resp)
}

/// 用户登出处理
#[handler]
async fn handle_logout(_user: CurrentUser) -> poem::Result<Response> {
    let mut resp = Response::builder().status(StatusCode::NO_CONTENT).finish();

    // 清除 Cookie
    let mut clear_access = Cookie::new("access_token", "");
    clear_access.set_http_only(true);
    clear_access.set_secure(true);
    clear_access.set_path("/");
    clear_access.set_max_age(std::time::Duration::ZERO);

    let mut clear_refresh = Cookie::new("refresh_token", "");
    clear_refresh.set_http_only(true);
    clear_refresh.set_secure(true);
    clear_refresh.set_path("/api/v1/auth/refresh");
    clear_refresh.set_max_age(std::time::Duration::ZERO);

    resp.headers_mut()
        .append("Set-Cookie", clear_access.to_string().parse().unwrap());
    resp.headers_mut()
        .append("Set-Cookie", clear_refresh.to_string().parse().unwrap());

    Ok(resp)
}

/// 刷新令牌处理
#[handler]
async fn handle_refresh(
    state: Data<&Arc<AppState>>,
    Json(body): Json<RefreshTokenRequest>,
) -> poem::Result<Response> {
    // 获取刷新令牌：从请求体获取
    let refresh_token = body
        .refresh_token
        .ok_or(AppError::Unauthorized(AuthErrorKind::InvalidToken))?;

    // 验证令牌
    let claims = state.jwt_manager.verify_token(&refresh_token)?;

    // 检查令牌类型
    if claims.token_type != TokenType::Refresh {
        return Err(AppError::Unauthorized(AuthErrorKind::InvalidToken).into());
    }

    // 生成新的访问令牌
    let (access_token, expires_in) = state.jwt_manager.generate_access_token(claims.sub)?;

    let response = TokenResponse {
        access_token: access_token.clone(),
        expires_in,
    };

    // 设置新的访问令牌 Cookie
    let mut resp = Json(ApiResponse::new(response)).into_response();

    let mut access_cookie = Cookie::new("access_token", access_token);
    access_cookie.set_http_only(true);
    access_cookie.set_secure(true);
    access_cookie.set_path("/");

    resp.headers_mut()
        .append("Set-Cookie", access_cookie.to_string().parse().unwrap());

    Ok(resp)
}

/// 获取当前用户信息
#[handler]
async fn handle_me(
    state: Data<&Arc<AppState>>,
    user: CurrentUser,
) -> poem::Result<Json<ApiResponse<UserResponse>>> {
    let user = state
        .db
        .find_user_by_id(user.id)
        .await?
        .ok_or_else(|| AppError::Internal("用户不存在".to_string()))?;

    Ok(Json(ApiResponse::new(UserResponse::from_user(&user))))
}

// ============================================================================
// 用户路由（占位）
// ============================================================================

fn user_routes() -> Route {
    Route::new()
        .at("/search", get(placeholder_handler))
        .at("/:user_id", get(placeholder_handler))
        .at("/me", put(placeholder_handler))
        .at("/me/avatar", put(placeholder_handler))
        .at("/:user_id/avatar", get(placeholder_handler))
}

#[handler]
async fn placeholder_handler() -> &'static str {
    r#"{"error":{"code":"NOT_IMPLEMENTED","message":"接口尚未实现"}}"#
}

// ============================================================================
// 代码执行路由
// ============================================================================

fn execution_routes() -> Route {
    Route::new()
        .at("/", post(handle_submit_execution))
        .at("/:execution_id", get(handle_get_execution))
        .at("/me", get(handle_my_executions))
}

/// 提交代码执行
#[handler]
async fn handle_submit_execution(
    state: Data<&Arc<AppState>>,
    user: CurrentUser,
    Json(req): Json<crate::model::ExecuteCodeRequest>,
) -> poem::Result<Response> {
    // 验证请求
    req.validate().map_err(|e| {
        AppError::InvalidRequest(crate::error::InvalidRequestKind::ValidationFailed {
            message: e.to_string(),
        })
    })?;

    // 获取用户信息
    let user_info = state
        .db
        .find_user_by_id(user.id)
        .await?
        .ok_or_else(|| AppError::Internal("用户不存在".to_string()))?;

    // 提交执行
    let submission = state
        .execution_service
        .submit_execution(&user_info, req.language_id.clone(), req.code.clone())
        .await?;

    // 获取语言信息
    let language = state
        .db
        .get_language_by_id(&req.language_id)
        .await?
        .ok_or_else(|| AppError::Internal("语言不存在".to_string()))?;

    // 构造响应
    let response = crate::model::ExecutionResponse {
        id: submission.id,
        status: submission.status,
        cache_hit: submission.cache_hit,
        language: crate::model::LanguageInfo::from_language(&language),
        code: None,   // 提交时不返回代码
        result: None, // 异步执行,提交时没有结果
        created_at: submission.created_at,
    };

    // 根据是否缓存命中返回不同状态码
    let mut resp = Json(ApiResponse::new(response)).into_response();
    if submission.cache_hit {
        resp.set_status(StatusCode::OK); // 200 缓存命中
    } else {
        resp.set_status(StatusCode::ACCEPTED); // 202 已接受,异步执行
    }

    Ok(resp)
}

/// 获取执行结果
#[handler]
async fn handle_get_execution(
    state: Data<&Arc<AppState>>,
    user: CurrentUser,
    poem::web::Path(execution_id): poem::web::Path<Uuid>,
) -> poem::Result<Json<ApiResponse<crate::model::ExecutionResponse>>> {
    // 获取执行结果
    let (submission, language, result) = state
        .execution_service
        .get_execution(execution_id, user.id)
        .await?;

    // 从缓存获取代码
    let code = if let Some(_cache) = state
        .execution_service
        .cache
        .get(&submission.code_hash)
        .await?
    {
        // TODO: 从 execution_cache 表获取完整代码
        None
    } else {
        None
    };

    // 构造响应
    let response = crate::model::ExecutionResponse {
        id: submission.id,
        status: submission.status,
        cache_hit: submission.cache_hit,
        language: crate::model::LanguageInfo::from_language(
            &language.ok_or_else(|| AppError::Internal("语言不存在".to_string()))?,
        ),
        code,
        result: result.map(|r| crate::model::ExecutionResult {
            stdout: r.stdout,
            stderr: r.stderr,
            exit_code: r.exit_code,
            cpu_time_ms: r.cpu_time_ms,
        }),
        created_at: submission.created_at,
    };

    Ok(Json(ApiResponse::new(response)))
}

/// 获取执行历史
#[handler]
async fn handle_my_executions(
    state: Data<&Arc<AppState>>,
    user: CurrentUser,
    poem::web::Query(params): poem::web::Query<PaginationParams>,
) -> poem::Result<Json<ApiResponse<Vec<SubmissionSummary>>>> {
    let limit = params.limit.unwrap_or(20).min(100);
    let offset = params.offset.unwrap_or(0);

    let submissions = state
        .execution_service
        .get_user_submissions(user.id, limit, offset)
        .await?;

    // 构造简化的响应
    let summaries: Vec<SubmissionSummary> = submissions
        .into_iter()
        .map(|s| SubmissionSummary {
            id: s.id,
            language_id: s.language_id,
            status: s.status,
            cache_hit: s.cache_hit,
            cpu_time_ms: s.cpu_time_ms,
            created_at: s.created_at,
        })
        .collect();

    Ok(Json(ApiResponse::new(summaries)))
}

#[derive(Debug, Deserialize)]
struct PaginationParams {
    limit: Option<i64>,
    offset: Option<i64>,
}

#[derive(Debug, Serialize)]
struct SubmissionSummary {
    id: Uuid,
    language_id: String,
    status: crate::model::ExecutionStatus,
    cache_hit: bool,
    cpu_time_ms: Option<i64>,
    created_at: chrono::DateTime<chrono::Utc>,
}

// ============================================================================
// 语言配置路由
// ============================================================================

fn language_routes() -> Route {
    Route::new().at("/", get(handle_get_languages))
}

/// 获取语言列表
#[handler]
async fn handle_get_languages(
    state: Data<&Arc<AppState>>,
    poem::web::Query(params): poem::web::Query<LanguageQueryParams>,
) -> poem::Result<Json<ApiResponse<Vec<crate::model::LanguageResponse>>>> {
    let enabled_only = params.enabled_only.unwrap_or(true);

    let languages = state
        .execution_service
        .get_all_languages(enabled_only)
        .await?;

    let responses: Vec<crate::model::LanguageResponse> = languages
        .iter()
        .map(crate::model::LanguageResponse::from_language)
        .collect();

    Ok(Json(ApiResponse::new(responses)))
}

#[derive(Debug, Deserialize)]
struct LanguageQueryParams {
    enabled_only: Option<bool>,
}

// ============================================================================
// 配额查询路由
// ============================================================================

fn quota_routes() -> Route {
    Route::new().at("/me", get(handle_get_quota))
}

/// 获取当前用户配额
#[handler]
async fn handle_get_quota(
    state: Data<&Arc<AppState>>,
    user: CurrentUser,
) -> poem::Result<Json<ApiResponse<crate::model::QuotaResponse>>> {
    // 获取用户信息
    let user_info = state
        .db
        .find_user_by_id(user.id)
        .await?
        .ok_or_else(|| AppError::Internal("用户不存在".to_string()))?;

    // 获取配额状态
    let status = state
        .quota_manager
        .get_quota_status(user.id, user_info.level)
        .await?;

    let response = match status {
        crate::quota::QuotaStatus::Unlimited => crate::model::QuotaResponse::Unlimited {
            level: "enhanced".to_string(),
        },
        crate::quota::QuotaStatus::Limited {
            period_start,
            period_end,
            limit_ms,
            used_ms,
            remaining_ms,
            usage_percentage,
        } => crate::model::QuotaResponse::Limited {
            level: "standard".to_string(),
            quota: crate::model::QuotaStatusLimited {
                period_start,
                period_end,
                limit_ms,
                used_ms,
                remaining_ms,
                usage_percentage,
            },
        },
    };

    Ok(Json(ApiResponse::new(response)))
}

// ============================================================================
// 单元测试
// ============================================================================

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_app_state_is_clone() {
        fn assert_clone<T: Clone>() {}
        assert_clone::<AppState>();
    }

    #[test]
    fn test_api_response_serialization() {
        #[derive(Serialize)]
        struct TestData {
            value: i32,
        }

        let response = ApiResponse::new(TestData { value: 42 });
        let json = serde_json::to_string(&response).expect("序列化应成功");

        assert!(json.contains("data"), "JSON 应包含 data 字段");
        assert!(json.contains("42"), "JSON 应包含值");
    }
}
