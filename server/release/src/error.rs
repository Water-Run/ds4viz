//! 统一错误类型定义模块
//!
//! 本模块定义应用的所有错误类型，并提供到 HTTP 响应的转换。
//!
//! # 错误分类
//!
//! - 认证错误 (401): 未认证或认证过期
//! - 权限错误 (403): 无权限访问、账户被封禁、配额超限等
//! - 请求错误 (400): 请求参数无效
//! - 资源不存在 (404): 请求的资源不存在
//! - 资源冲突 (409): 资源已存在（如用户名重复）
//! - 限流错误 (429): 请求频率超限
//! - 配置错误: 配置加载或解析失败
//! - 数据库错误: 数据库操作失败
//! - 内部错误 (500): 服务器内部错误
//!
//! 作者: WaterRun
//! 日期: 2026-01-16

use chrono::{DateTime, Utc};
use poem::{IntoResponse, Response, error::ResponseError, http::StatusCode};
use serde::Serialize;

/// 应用统一错误类型
///
/// 所有业务逻辑错误都通过此类型表达，并可转换为 HTTP 响应。
///
/// # 示例
///
/// ```
/// use ds4viz_server::error::{AppError, AuthErrorKind};
///
/// let error = AppError::Unauthorized(AuthErrorKind::InvalidCredentials);
/// assert_eq!(error.http_status_code(), 401);
/// assert_eq!(error.error_code(), "INVALID_CREDENTIALS");
/// ```
#[derive(Debug, thiserror::Error)]
pub enum AppError {
    /// 认证错误 (HTTP 401)
    #[error("认证失败: {0}")]
    Unauthorized(AuthErrorKind),

    /// 权限错误 (HTTP 403)
    #[error("权限不足: {0}")]
    Forbidden(ForbiddenKind),

    /// 请求参数错误 (HTTP 400)
    #[error("请求参数错误: {0}")]
    InvalidRequest(InvalidRequestKind),

    /// 资源不存在 (HTTP 404)
    #[error("资源不存在: {0}")]
    NotFound(NotFoundKind),

    /// 资源冲突 (HTTP 409)
    #[error("资源冲突: {0}")]
    Conflict(ConflictKind),

    /// 请求频率超限 (HTTP 429)
    #[error("请求频率超限")]
    RateLimitExceeded,

    /// 配置错误
    #[error("配置错误: {0}")]
    Config(ConfigErrorKind),

    /// 数据库错误 (HTTP 500)
    #[error("数据库错误: {0}")]
    Database(String),

    /// 内部服务器错误 (HTTP 500)
    #[error("服务器内部错误: {0}")]
    Internal(String),
}

/// 认证错误详情
///
/// # 示例
///
/// ```
/// use ds4viz_server::error::AuthErrorKind;
///
/// let kind = AuthErrorKind::TokenExpired;
/// assert_eq!(kind.to_string(), "令牌已过期");
/// ```
#[derive(Debug, Clone, thiserror::Error)]
pub enum AuthErrorKind {
    /// 用户名或密码错误
    #[error("用户名或密码错误")]
    InvalidCredentials,

    /// 无效的令牌
    #[error("无效的令牌")]
    InvalidToken,

    /// 令牌已过期
    #[error("令牌已过期")]
    TokenExpired,
}

/// 权限错误详情
///
/// # 示例
///
/// ```
/// use ds4viz_server::error::ForbiddenKind;
/// use chrono::Utc;
///
/// let kind = ForbiddenKind::AccountBanned {
///     reason: "违规操作".to_string(),
///     until: Some(Utc::now()),
/// };
/// assert!(kind.to_string().contains("账户已被封禁"));
/// ```
#[derive(Debug, Clone, thiserror::Error)]
pub enum ForbiddenKind {
    /// 账户被封禁
    #[error("账户已被封禁: {reason}")]
    AccountBanned {
        /// 封禁原因
        reason: String,
        /// 封禁截止时间，None 表示永久封禁
        until: Option<DateTime<Utc>>,
    },

    /// 账户被暂停
    #[error("账户已被暂停")]
    AccountSuspended,

    /// 配额超限
    #[error("配额超限")]
    QuotaExceeded {
        /// 周期开始时间
        period_start: DateTime<Utc>,
        /// 周期结束时间
        period_end: DateTime<Utc>,
        /// 配额限制（毫秒）
        quota_limit_ms: u64,
        /// 已使用配额（毫秒）
        quota_used_ms: u64,
    },

    /// 语言不允许（Standard 用户使用编译型语言）
    #[error("当前等级不允许使用此语言")]
    LanguageNotAllowed,

    /// 通用访问拒绝
    #[error("访问被拒绝")]
    AccessDenied,
}

/// 请求参数错误详情
///
/// # 示例
///
/// ```
/// use ds4viz_server::error::InvalidRequestKind;
///
/// let kind = InvalidRequestKind::InvalidUsername {
///     reason: "用户名太短".to_string(),
/// };
/// assert!(kind.to_string().contains("无效的用户名"));
/// ```
#[derive(Debug, Clone, thiserror::Error)]
pub enum InvalidRequestKind {
    /// 无效的用户名
    #[error("无效的用户名: {reason}")]
    InvalidUsername {
        /// 具体原因
        reason: String,
    },

    /// 无效的邮箱
    #[error("无效的邮箱: {reason}")]
    InvalidEmail {
        /// 具体原因
        reason: String,
    },

    /// 密码强度不足
    #[error("密码强度不足: {reason}")]
    WeakPassword {
        /// 具体原因
        reason: String,
    },

    /// 无效的语言标识
    #[error("无效的语言: {language_id}")]
    InvalidLanguage {
        /// 语言标识
        language_id: String,
    },

    /// 代码过大
    #[error("代码过大: 最大 {max_bytes} 字节，实际 {actual_bytes} 字节")]
    CodeTooLarge {
        /// 最大字节数
        max_bytes: usize,
        /// 实际字节数
        actual_bytes: usize,
    },

    /// 无效的文件类型
    #[error("无效的文件类型: {mime_type}")]
    InvalidFileType {
        /// 文件 MIME 类型
        mime_type: String,
    },

    /// 文件过大
    #[error("文件过大: 最大 {max_bytes} 字节")]
    FileTooLarge {
        /// 最大字节数
        max_bytes: usize,
    },

    /// 通用验证失败
    #[error("验证失败: {message}")]
    ValidationFailed {
        /// 错误消息
        message: String,
    },
}

/// 资源不存在错误详情
///
/// # 示例
///
/// ```
/// use ds4viz_server::error::NotFoundKind;
/// use uuid::Uuid;
///
/// let kind = NotFoundKind::UserNotFound { id: Uuid::new_v4() };
/// assert!(kind.to_string().contains("用户不存在"));
/// ```
#[derive(Debug, Clone, thiserror::Error)]
pub enum NotFoundKind {
    /// 用户不存在
    #[error("用户不存在: {id}")]
    UserNotFound {
        /// 用户 ID
        id: uuid::Uuid,
    },

    /// 执行记录不存在
    #[error("执行记录不存在: {id}")]
    ExecutionNotFound {
        /// 执行 ID
        id: uuid::Uuid,
    },

    /// 模板不存在
    #[error("模板不存在: {slug}")]
    TemplateNotFound {
        /// 模板 slug
        slug: String,
    },

    /// 语言版本不存在
    #[error("模板不支持此语言: {language_id}")]
    LanguageNotAvailable {
        /// 语言标识
        language_id: String,
    },

    /// 未点赞（取消点赞时）
    #[error("未点赞此模板")]
    NotLiked,
}

/// 资源冲突错误详情
///
/// # 示例
///
/// ```
/// use ds4viz_server::error::ConflictKind;
///
/// let kind = ConflictKind::UsernameExists {
///     username: "test".to_string(),
/// };
/// assert!(kind.to_string().contains("用户名已存在"));
/// ```
#[derive(Debug, Clone, thiserror::Error)]
pub enum ConflictKind {
    /// 用户名已存在
    #[error("用户名已存在: {username}")]
    UsernameExists {
        /// 用户名
        username: String,
    },

    /// 邮箱已存在
    #[error("邮箱已存在: {email}")]
    EmailExists {
        /// 邮箱
        email: String,
    },

    /// 已有待审核的升级申请
    #[error("已有待审核的升级申请")]
    PendingRequestExists,

    /// 已是 Enhanced 级别
    #[error("已是 Enhanced 级别")]
    AlreadyEnhanced,

    /// 已点赞
    #[error("已点赞此模板")]
    AlreadyLiked,

    /// 已收藏
    #[error("已收藏此模板")]
    AlreadyFavorited,
}

/// 配置错误详情
///
/// # 示例
///
/// ```
/// use ds4viz_server::error::ConfigErrorKind;
///
/// let kind = ConfigErrorKind::FileNotFound {
///     path: "/etc/config.toml".to_string(),
///     source: "file not found".to_string(),
/// };
/// assert!(kind.to_string().contains("配置文件未找到"));
/// ```
#[derive(Debug, Clone, thiserror::Error)]
pub enum ConfigErrorKind {
    /// 配置文件未找到
    #[error("配置文件未找到: {path}")]
    FileNotFound {
        /// 文件路径
        path: String,
    },

    /// 配置解析错误
    #[error("配置解析错误: {message}")]
    ParseError {
        /// 错误信息
        message: String,
    },
}

/// API 错误响应结构
#[derive(Debug, Serialize)]
struct ErrorResponse {
    error: ErrorBody,
}

/// 错误响应体
#[derive(Debug, Serialize)]
struct ErrorBody {
    code: String,
    message: String,
    #[serde(skip_serializing_if = "Option::is_none")]
    details: Option<serde_json::Value>,
}

impl AppError {
    /// 获取 HTTP 状态码
    ///
    /// # 示例
    ///
    /// ```
    /// use ds4viz_server::error::{AppError, AuthErrorKind};
    ///
    /// let error = AppError::Unauthorized(AuthErrorKind::InvalidCredentials);
    /// assert_eq!(error.http_status_code(), 401);
    /// ```
    #[must_use]
    pub fn http_status_code(&self) -> u16 {
        match self {
            Self::Unauthorized(_) => 401,
            Self::Forbidden(_) => 403,
            Self::InvalidRequest(_) => 400,
            Self::NotFound(_) => 404,
            Self::Conflict(_) => 409,
            Self::RateLimitExceeded => 429,
            Self::Config(_) | Self::Database(_) | Self::Internal(_) => 500,
        }
    }

    /// 获取错误码
    ///
    /// # 示例
    ///
    /// ```
    /// use ds4viz_server::error::{AppError, ConflictKind};
    ///
    /// let error = AppError::Conflict(ConflictKind::UsernameExists {
    ///     username: "test".to_string(),
    /// });
    /// assert_eq!(error.error_code(), "USERNAME_EXISTS");
    /// ```
    #[must_use]
    pub fn error_code(&self) -> &'static str {
        match self {
            Self::Unauthorized(kind) => match kind {
                AuthErrorKind::InvalidCredentials => "INVALID_CREDENTIALS",
                AuthErrorKind::InvalidToken => "INVALID_TOKEN",
                AuthErrorKind::TokenExpired => "TOKEN_EXPIRED",
            },
            Self::Forbidden(kind) => match kind {
                ForbiddenKind::AccountBanned { .. } => "ACCOUNT_BANNED",
                ForbiddenKind::AccountSuspended => "ACCOUNT_SUSPENDED",
                ForbiddenKind::QuotaExceeded { .. } => "QUOTA_EXCEEDED",
                ForbiddenKind::LanguageNotAllowed => "LANGUAGE_NOT_ALLOWED",
                ForbiddenKind::AccessDenied => "ACCESS_DENIED",
            },
            Self::InvalidRequest(kind) => match kind {
                InvalidRequestKind::InvalidUsername { .. } => "INVALID_USERNAME",
                InvalidRequestKind::InvalidEmail { .. } => "INVALID_EMAIL",
                InvalidRequestKind::WeakPassword { .. } => "WEAK_PASSWORD",
                InvalidRequestKind::InvalidLanguage { .. } => "INVALID_LANGUAGE",
                InvalidRequestKind::CodeTooLarge { .. } => "CODE_TOO_LARGE",
                InvalidRequestKind::InvalidFileType { .. } => "INVALID_FILE_TYPE",
                InvalidRequestKind::FileTooLarge { .. } => "FILE_TOO_LARGE",
                InvalidRequestKind::ValidationFailed { .. } => "VALIDATION_FAILED",
            },
            Self::NotFound(kind) => match kind {
                NotFoundKind::UserNotFound { .. } => "USER_NOT_FOUND",
                NotFoundKind::ExecutionNotFound { .. } => "EXECUTION_NOT_FOUND",
                NotFoundKind::TemplateNotFound { .. } => "TEMPLATE_NOT_FOUND",
                NotFoundKind::LanguageNotAvailable { .. } => "LANGUAGE_NOT_AVAILABLE",
                NotFoundKind::NotLiked => "NOT_LIKED",
            },
            Self::Conflict(kind) => match kind {
                ConflictKind::UsernameExists { .. } => "USERNAME_EXISTS",
                ConflictKind::EmailExists { .. } => "EMAIL_EXISTS",
                ConflictKind::PendingRequestExists => "PENDING_REQUEST_EXISTS",
                ConflictKind::AlreadyEnhanced => "ALREADY_ENHANCED",
                ConflictKind::AlreadyLiked => "ALREADY_LIKED",
                ConflictKind::AlreadyFavorited => "ALREADY_FAVORITED",
            },
            Self::RateLimitExceeded => "RATE_LIMIT_EXCEEDED",
            Self::Config(_) => "CONFIG_ERROR",
            Self::Database(_) => "DATABASE_ERROR",
            Self::Internal(_) => "INTERNAL_ERROR",
        }
    }

    /// 获取错误详情（可选）
    fn details(&self) -> Option<serde_json::Value> {
        match self {
            Self::Forbidden(ForbiddenKind::AccountBanned { reason, until }) => {
                Some(serde_json::json!({
                    "reason": reason,
                    "banned_until": until,
                }))
            }
            Self::Forbidden(ForbiddenKind::QuotaExceeded {
                period_start,
                period_end,
                quota_limit_ms,
                quota_used_ms,
            }) => Some(serde_json::json!({
                "period_start": period_start,
                "period_end": period_end,
                "quota_limit_ms": quota_limit_ms,
                "quota_used_ms": quota_used_ms,
                "reset_at": period_end,
            })),
            Self::InvalidRequest(InvalidRequestKind::CodeTooLarge {
                max_bytes,
                actual_bytes,
            }) => Some(serde_json::json!({
                "max_bytes": max_bytes,
                "actual_bytes": actual_bytes,
            })),
            _ => None,
        }
    }
}

impl IntoResponse for AppError {
    fn into_response(self) -> Response {
        let status = StatusCode::from_u16(self.http_status_code())
            .unwrap_or(StatusCode::INTERNAL_SERVER_ERROR);

        let body = ErrorResponse {
            error: ErrorBody {
                code: self.error_code().to_string(),
                message: self.to_string(),
                details: self.details(),
            },
        };

        let json = serde_json::to_string(&body).unwrap_or_else(|_| {
            r#"{"error":{"code":"INTERNAL_ERROR","message":"序列化错误"}}"#.to_string()
        });

        Response::builder()
            .status(status)
            .content_type("application/json")
            .body(json)
    }
}

impl From<sqlx::Error> for AppError {
    fn from(err: sqlx::Error) -> Self {
        Self::Database(err.to_string())
    }
}

impl ResponseError for AppError {
    fn status(&self) -> StatusCode {
        StatusCode::from_u16(self.http_status_code()).unwrap_or(StatusCode::INTERNAL_SERVER_ERROR)
    }

    fn as_response(&self) -> Response
    where
        Self: std::error::Error + Send + Sync + 'static,
    {
        self.clone().into_response()
    }
}

impl Clone for AppError {
    fn clone(&self) -> Self {
        match self {
            Self::Unauthorized(kind) => Self::Unauthorized(kind.clone()),
            Self::Forbidden(kind) => Self::Forbidden(kind.clone()),
            Self::InvalidRequest(kind) => Self::InvalidRequest(kind.clone()),
            Self::NotFound(kind) => Self::NotFound(kind.clone()),
            Self::Conflict(kind) => Self::Conflict(kind.clone()),
            Self::RateLimitExceeded => Self::RateLimitExceeded,
            Self::Config(kind) => Self::Config(kind.clone()),
            Self::Database(msg) => Self::Database(msg.clone()),
            Self::Internal(msg) => Self::Internal(msg.clone()),
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_unauthorized_error_code_and_status() {
        let error = AppError::Unauthorized(AuthErrorKind::InvalidCredentials);
        assert_eq!(error.http_status_code(), 401, "认证错误应返回 401");
        assert_eq!(
            error.error_code(),
            "INVALID_CREDENTIALS",
            "错误码应为 INVALID_CREDENTIALS"
        );

        let error = AppError::Unauthorized(AuthErrorKind::TokenExpired);
        assert_eq!(
            error.error_code(),
            "TOKEN_EXPIRED",
            "错误码应为 TOKEN_EXPIRED"
        );
    }

    #[test]
    fn test_forbidden_error_code_and_status() {
        let error = AppError::Forbidden(ForbiddenKind::AccountBanned {
            reason: "违规".to_string(),
            until: None,
        });
        assert_eq!(error.http_status_code(), 403, "权限错误应返回 403");
        assert_eq!(
            error.error_code(),
            "ACCOUNT_BANNED",
            "错误码应为 ACCOUNT_BANNED"
        );

        let error = AppError::Forbidden(ForbiddenKind::QuotaExceeded {
            period_start: Utc::now(),
            period_end: Utc::now(),
            quota_limit_ms: 10000,
            quota_used_ms: 12000,
        });
        assert_eq!(
            error.error_code(),
            "QUOTA_EXCEEDED",
            "错误码应为 QUOTA_EXCEEDED"
        );
    }

    #[test]
    fn test_invalid_request_error_code_and_status() {
        let error = AppError::InvalidRequest(InvalidRequestKind::InvalidUsername {
            reason: "太短".to_string(),
        });
        assert_eq!(error.http_status_code(), 400, "请求错误应返回 400");
        assert_eq!(
            error.error_code(),
            "INVALID_USERNAME",
            "错误码应为 INVALID_USERNAME"
        );

        let error = AppError::InvalidRequest(InvalidRequestKind::CodeTooLarge {
            max_bytes: 16384,
            actual_bytes: 20000,
        });
        assert_eq!(
            error.error_code(),
            "CODE_TOO_LARGE",
            "错误码应为 CODE_TOO_LARGE"
        );
    }

    #[test]
    fn test_not_found_error_code_and_status() {
        let error = AppError::NotFound(NotFoundKind::UserNotFound {
            id: uuid::Uuid::new_v4(),
        });
        assert_eq!(error.http_status_code(), 404, "不存在错误应返回 404");
        assert_eq!(
            error.error_code(),
            "USER_NOT_FOUND",
            "错误码应为 USER_NOT_FOUND"
        );
    }

    #[test]
    fn test_conflict_error_code_and_status() {
        let error = AppError::Conflict(ConflictKind::UsernameExists {
            username: "test".to_string(),
        });
        assert_eq!(error.http_status_code(), 409, "冲突错误应返回 409");
        assert_eq!(
            error.error_code(),
            "USERNAME_EXISTS",
            "错误码应为 USERNAME_EXISTS"
        );
    }

    #[test]
    fn test_rate_limit_error_code_and_status() {
        let error = AppError::RateLimitExceeded;
        assert_eq!(error.http_status_code(), 429, "限流错误应返回 429");
        assert_eq!(
            error.error_code(),
            "RATE_LIMIT_EXCEEDED",
            "错误码应为 RATE_LIMIT_EXCEEDED"
        );
    }

    #[test]
    fn test_internal_error_code_and_status() {
        let error = AppError::Internal("意外错误".to_string());
        assert_eq!(error.http_status_code(), 500, "内部错误应返回 500");
        assert_eq!(
            error.error_code(),
            "INTERNAL_ERROR",
            "错误码应为 INTERNAL_ERROR"
        );

        let error = AppError::Database("连接失败".to_string());
        assert_eq!(error.http_status_code(), 500, "数据库错误应返回 500");
        assert_eq!(
            error.error_code(),
            "DATABASE_ERROR",
            "错误码应为 DATABASE_ERROR"
        );
    }

    #[test]
    fn test_config_error_code_and_status() {
        let error = AppError::Config(ConfigErrorKind::FileNotFound {
            path: "/nonexistent: not found".to_string(),
        });
        assert_eq!(error.http_status_code(), 500, "配置错误应返回 500");
        assert_eq!(
            error.error_code(),
            "CONFIG_ERROR",
            "错误码应为 CONFIG_ERROR"
        );
    }

    #[test]
    fn test_error_details_for_account_banned() {
        let error = AppError::Forbidden(ForbiddenKind::AccountBanned {
            reason: "违规操作".to_string(),
            until: Some(Utc::now()),
        });

        let details = error.details();
        assert!(details.is_some(), "AccountBanned 应有详情");

        let details = details.expect("已验证存在");
        assert!(details.get("reason").is_some(), "详情应包含 reason 字段");
        assert!(
            details.get("banned_until").is_some(),
            "详情应包含 banned_until 字段"
        );
    }

    #[test]
    fn test_error_details_for_quota_exceeded() {
        let now = Utc::now();
        let error = AppError::Forbidden(ForbiddenKind::QuotaExceeded {
            period_start: now,
            period_end: now,
            quota_limit_ms: 10000,
            quota_used_ms: 12000,
        });

        let details = error.details();
        assert!(details.is_some(), "QuotaExceeded 应有详情");

        let details = details.expect("已验证存在");
        assert_eq!(
            details.get("quota_limit_ms"),
            Some(&serde_json::json!(10000)),
            "详情应包含正确的 quota_limit_ms"
        );
        assert_eq!(
            details.get("quota_used_ms"),
            Some(&serde_json::json!(12000)),
            "详情应包含正确的 quota_used_ms"
        );
    }

    #[test]
    fn test_error_details_for_code_too_large() {
        let error = AppError::InvalidRequest(InvalidRequestKind::CodeTooLarge {
            max_bytes: 16384,
            actual_bytes: 20000,
        });

        let details = error.details();
        assert!(details.is_some(), "CodeTooLarge 应有详情");

        let details = details.expect("已验证存在");
        assert_eq!(
            details.get("max_bytes"),
            Some(&serde_json::json!(16384)),
            "详情应包含正确的 max_bytes"
        );
        assert_eq!(
            details.get("actual_bytes"),
            Some(&serde_json::json!(20000)),
            "详情应包含正确的 actual_bytes"
        );
    }

    #[test]
    fn test_error_display_messages() {
        let error = AppError::Unauthorized(AuthErrorKind::InvalidCredentials);
        assert!(
            error.to_string().contains("用户名或密码错误"),
            "错误消息应包含具体描述"
        );

        let error = AppError::Forbidden(ForbiddenKind::AccountBanned {
            reason: "测试原因".to_string(),
            until: None,
        });
        assert!(
            error.to_string().contains("测试原因"),
            "错误消息应包含封禁原因"
        );
    }

    #[test]
    fn test_error_into_response() {
        let error = AppError::Unauthorized(AuthErrorKind::InvalidCredentials);
        let response = error.into_response();

        assert_eq!(
            response.status(),
            StatusCode::UNAUTHORIZED,
            "HTTP 状态码应为 401"
        );
    }
}
