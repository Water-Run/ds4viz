//! 错误类型定义
//!
//! 定义服务器所有错误类型，支持结构化错误响应。

use poem::http::StatusCode;
use poem::{IntoResponse, Response};
use serde::Serialize;
use thiserror::Error;

/// 配置加载错误
#[derive(Debug, Error)]
pub enum ConfigError {
    #[error("无法读取配置文件 '{path}': {source}")]
    FileRead {
        path: String,
        source: std::io::Error,
    },

    #[error("配置文件解析失败: {source}")]
    Parse { source: toml::de::Error },

    #[error("配置缺少必需字段: {field}")]
    MissingField { field: String },

    #[error("配置字段 '{field}' 值无效 ('{value}'): {reason}")]
    InvalidValue {
        field: String,
        value: String,
        reason: String,
    },
}

/// API 错误码
#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize)]
#[serde(rename_all = "SCREAMING_SNAKE_CASE")]
pub enum ErrorCode {
    // 通用错误
    InvalidRequest,
    Unauthorized,
    Forbidden,
    NotFound,
    Conflict,
    RateLimitExceeded,
    InternalError,

    // 认证相关
    InvalidCredentials,
    InvalidToken,
    TokenExpired,
    AccountBanned,
    AccountSuspended,

    // 用户相关
    UsernameExists,
    EmailExists,
    UserNotFound,
    InvalidUsername,
    InvalidEmail,
    WeakPassword,
    InvalidFileType,
    FileTooLarge,

    // 执行相关
    InvalidLanguage,
    CodeTooLarge,
    QuotaExceeded,
    LanguageNotAllowed,
    ExecutionNotFound,

    // 模板相关
    TemplateNotFound,
    LanguageNotAvailable,
    AlreadyLiked,
    NotLiked,
    AlreadyFavorited,

    // 升级申请
    PendingRequestExists,
    AlreadyEnhanced,
}

impl ErrorCode {
    #[must_use]
    pub const fn status_code(self) -> StatusCode {
        match self {
            Self::InvalidRequest
            | Self::InvalidUsername
            | Self::InvalidEmail
            | Self::WeakPassword
            | Self::InvalidLanguage
            | Self::CodeTooLarge
            | Self::InvalidFileType
            | Self::FileTooLarge => StatusCode::BAD_REQUEST,

            Self::Unauthorized
            | Self::InvalidCredentials
            | Self::InvalidToken
            | Self::TokenExpired => StatusCode::UNAUTHORIZED,

            Self::Forbidden
            | Self::AccountBanned
            | Self::AccountSuspended
            | Self::QuotaExceeded
            | Self::LanguageNotAllowed => StatusCode::FORBIDDEN,

            Self::NotFound
            | Self::UserNotFound
            | Self::ExecutionNotFound
            | Self::TemplateNotFound
            | Self::LanguageNotAvailable
            | Self::NotLiked => StatusCode::NOT_FOUND,

            Self::Conflict
            | Self::UsernameExists
            | Self::EmailExists
            | Self::AlreadyLiked
            | Self::AlreadyFavorited
            | Self::PendingRequestExists
            | Self::AlreadyEnhanced => StatusCode::CONFLICT,

            Self::RateLimitExceeded => StatusCode::TOO_MANY_REQUESTS,

            Self::InternalError => StatusCode::INTERNAL_SERVER_ERROR,
        }
    }
}

/// API 错误响应体
#[derive(Debug, Serialize)]
pub struct ApiErrorResponse {
    pub error: ApiErrorBody,
}

/// API 错误详情
#[derive(Debug, Serialize)]
pub struct ApiErrorBody {
    pub code: ErrorCode,
    pub message: String,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub details: Option<serde_json::Value>,
}

/// API 错误
#[derive(Debug)]
pub struct ApiError {
    pub code: ErrorCode,
    pub message: String,
    pub details: Option<serde_json::Value>,
}

impl ApiError {
    #[must_use]
    pub fn new(code: ErrorCode, message: impl Into<String>) -> Self {
        Self {
            code,
            message: message.into(),
            details: None,
        }
    }

    #[must_use]
    pub fn with_details(mut self, details: serde_json::Value) -> Self {
        self.details = Some(details);
        self
    }

    #[must_use]
    pub fn internal(message: impl Into<String>) -> Self {
        Self::new(ErrorCode::InternalError, message)
    }

    #[must_use]
    pub fn unauthorized(message: impl Into<String>) -> Self {
        Self::new(ErrorCode::Unauthorized, message)
    }

    #[must_use]
    pub fn not_found(code: ErrorCode, message: impl Into<String>) -> Self {
        Self::new(code, message)
    }
}

impl std::fmt::Display for ApiError {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{:?}: {}", self.code, self.message)
    }
}

impl std::error::Error for ApiError {}

impl IntoResponse for ApiError {
    fn into_response(self) -> Response {
        let status = self.code.status_code();
        let body = ApiErrorResponse {
            error: ApiErrorBody {
                code: self.code,
                message: self.message,
                details: self.details,
            },
        };

        let json = serde_json::to_string(&body).unwrap_or_else(|_| {
            r#"{"error":{"code":"INTERNAL_ERROR","message":"序列化错误响应失败"}}"#.to_owned()
        });

        Response::builder()
            .status(status)
            .content_type("application/json")
            .body(json)
    }
}

impl From<sqlx::Error> for ApiError {
    fn from(err: sqlx::Error) -> Self {
        tracing::error!(error = %err, "数据库错误");

        match &err {
            sqlx::Error::RowNotFound => Self::new(ErrorCode::NotFound, "请求的资源不存在"),
            sqlx::Error::Database(db_err) => {
                if db_err.code().map_or(false, |c| c == "23505") {
                    let message = db_err.message();
                    if message.contains("username") {
                        return Self::new(ErrorCode::UsernameExists, "用户名已存在");
                    }
                    if message.contains("email") {
                        return Self::new(ErrorCode::EmailExists, "邮箱已存在");
                    }
                    return Self::new(ErrorCode::Conflict, "资源冲突");
                }
                Self::internal("数据库操作失败")
            }
            _ => Self::internal("数据库操作失败"),
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_error_code_status_mapping() {
        assert_eq!(
            ErrorCode::InvalidRequest.status_code(),
            StatusCode::BAD_REQUEST
        );
        assert_eq!(
            ErrorCode::Unauthorized.status_code(),
            StatusCode::UNAUTHORIZED
        );
        assert_eq!(ErrorCode::Forbidden.status_code(), StatusCode::FORBIDDEN);
        assert_eq!(ErrorCode::NotFound.status_code(), StatusCode::NOT_FOUND);
        assert_eq!(ErrorCode::Conflict.status_code(), StatusCode::CONFLICT);
        assert_eq!(
            ErrorCode::InternalError.status_code(),
            StatusCode::INTERNAL_SERVER_ERROR
        );
    }

    #[test]
    fn test_api_error_into_response() {
        let error = ApiError::new(ErrorCode::UserNotFound, "用户不存在");
        let response = error.into_response();

        assert_eq!(response.status(), StatusCode::NOT_FOUND);
    }

    #[test]
    fn test_api_error_with_details() {
        let error = ApiError::new(ErrorCode::QuotaExceeded, "配额已用完")
            .with_details(serde_json::json!({"remaining_ms": 0}));

        assert!(error.details.is_some());
    }

    #[test]
    fn test_api_error_new() {
        let error = ApiError::new(ErrorCode::InvalidCredentials, "凭证无效");

        assert_eq!(error.code, ErrorCode::InvalidCredentials);
        assert_eq!(error.message, "凭证无效");
        assert!(error.details.is_none());
    }

    #[test]
    fn test_api_error_internal() {
        let error = ApiError::internal("系统错误");
        assert_eq!(error.code, ErrorCode::InternalError);
    }

    #[test]
    fn test_api_error_unauthorized() {
        let error = ApiError::unauthorized("未登录");
        assert_eq!(error.code, ErrorCode::Unauthorized);
    }
}