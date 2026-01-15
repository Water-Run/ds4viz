//! 数据模型定义

use chrono::{DateTime, Utc};
use serde::{Deserialize, Serialize};
use sqlx::FromRow;
use uuid::Uuid;

/// 用户等级
#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize, sqlx::Type)]
#[sqlx(type_name = "VARCHAR", rename_all = "lowercase")]
#[serde(rename_all = "lowercase")]
pub enum UserLevel {
    Standard,
    Enhanced,
}

impl UserLevel {
    #[must_use]
    pub const fn is_enhanced(self) -> bool {
        matches!(self, Self::Enhanced)
    }
}

impl Default for UserLevel {
    fn default() -> Self {
        Self::Standard
    }
}

/// 用户状态
#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize, sqlx::Type)]
#[sqlx(type_name = "VARCHAR", rename_all = "lowercase")]
#[serde(rename_all = "lowercase")]
pub enum UserStatus {
    Active,
    Suspended,
    Banned,
}

impl UserStatus {
    #[must_use]
    pub const fn can_login(self) -> bool {
        matches!(self, Self::Active)
    }
}

impl Default for UserStatus {
    fn default() -> Self {
        Self::Active
    }
}

/// 用户实体
#[derive(Debug, Clone, FromRow)]
pub struct User {
    pub id: Uuid,
    pub username: String,
    pub email: String,
    pub password_hash: String,
    #[sqlx(default)]
    pub avatar: Option<Vec<u8>>,
    #[sqlx(default)]
    pub avatar_mime_type: Option<String>,
    #[sqlx(default)]
    pub bio: Option<String>,
    pub level: String,
    pub status: String,
    #[sqlx(default)]
    pub banned_reason: Option<String>,
    #[sqlx(default)]
    pub banned_until: Option<DateTime<Utc>>,
    pub created_at: DateTime<Utc>,
    pub updated_at: DateTime<Utc>,
}

impl User {
    #[must_use]
    pub fn level_enum(&self) -> UserLevel {
        match self.level.as_str() {
            "enhanced" => UserLevel::Enhanced,
            _ => UserLevel::Standard,
        }
    }

    #[must_use]
    pub fn status_enum(&self) -> UserStatus {
        match self.status.as_str() {
            "banned" => UserStatus::Banned,
            "suspended" => UserStatus::Suspended,
            _ => UserStatus::Active,
        }
    }
}

/// 用户公开信息（用于 API 响应）
#[derive(Debug, Clone, Serialize)]
pub struct UserPublicProfile {
    pub id: Uuid,
    pub username: String,
    pub avatar_url: Option<String>,
    pub bio: Option<String>,
    pub level: UserLevel,
    pub created_at: DateTime<Utc>,
}

impl From<&User> for UserPublicProfile {
    fn from(user: &User) -> Self {
        let avatar_url = if user.avatar.is_some() {
            Some(format!("/api/v1/users/{}/avatar", user.id))
        } else {
            None
        };

        Self {
            id: user.id,
            username: user.username.clone(),
            avatar_url,
            bio: user.bio.clone(),
            level: user.level_enum(),
            created_at: user.created_at,
        }
    }
}

/// 用户私有信息（仅本人可见）
#[derive(Debug, Clone, Serialize)]
pub struct UserPrivateProfile {
    pub id: Uuid,
    pub username: String,
    pub email: String,
    pub avatar_url: Option<String>,
    pub bio: Option<String>,
    pub level: UserLevel,
    pub status: UserStatus,
    pub created_at: DateTime<Utc>,
    pub updated_at: DateTime<Utc>,
}

impl From<&User> for UserPrivateProfile {
    fn from(user: &User) -> Self {
        let avatar_url = if user.avatar.is_some() {
            Some(format!("/api/v1/users/{}/avatar", user.id))
        } else {
            None
        };

        Self {
            id: user.id,
            username: user.username.clone(),
            email: user.email.clone(),
            avatar_url,
            bio: user.bio.clone(),
            level: user.level_enum(),
            status: user.status_enum(),
            created_at: user.created_at,
            updated_at: user.updated_at,
        }
    }
}

/// 语言配置
#[derive(Debug, Clone, FromRow, Serialize)]
pub struct Language {
    pub id: String,
    pub display_name: String,
    pub version: Option<String>,
    pub is_interpreted: bool,
    pub file_extension: String,
    pub enabled: bool,
    pub timeout_ms: i32,
    pub sort_order: i32,
}

/// 执行缓存
#[derive(Debug, Clone, FromRow)]
pub struct ExecutionCache {
    pub code_hash: String,
    pub language_id: String,
    pub code: String,
    pub stdout: Option<String>,
    pub stderr: Option<String>,
    pub exit_code: Option<i32>,
    pub cpu_time_ms: Option<i64>,
    pub created_at: DateTime<Utc>,
}

/// 代码提交状态
#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
#[serde(rename_all = "lowercase")]
pub enum SubmissionStatus {
    Pending,
    Running,
    Success,
    Failed,
    Timeout,
}

impl SubmissionStatus {
    #[must_use]
    pub fn as_str(self) -> &'static str {
        match self {
            Self::Pending => "pending",
            Self::Running => "running",
            Self::Success => "success",
            Self::Failed => "failed",
            Self::Timeout => "timeout",
        }
    }
}

/// 代码提交记录
#[derive(Debug, Clone, FromRow)]
pub struct CodeSubmission {
    pub id: Uuid,
    pub user_id: Uuid,
    pub language_id: String,
    pub code_hash: String,
    pub status: String,
    pub cache_hit: bool,
    pub cpu_time_ms: Option<i64>,
    pub created_at: DateTime<Utc>,
}

impl CodeSubmission {
    #[must_use]
    pub fn status_enum(&self) -> SubmissionStatus {
        match self.status.as_str() {
            "running" => SubmissionStatus::Running,
            "success" => SubmissionStatus::Success,
            "failed" => SubmissionStatus::Failed,
            "timeout" => SubmissionStatus::Timeout,
            _ => SubmissionStatus::Pending,
        }
    }
}

/// 用户配额使用
#[derive(Debug, Clone, FromRow)]
pub struct UserQuotaUsage {
    pub id: Uuid,
    pub user_id: Uuid,
    pub period_start: DateTime<Utc>,
    pub cpu_time_used_ms: i64,
}

/// 升级申请状态
#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
#[serde(rename_all = "lowercase")]
pub enum UpgradeRequestStatus {
    Pending,
    Approved,
    Rejected,
}

/// 升级申请
#[derive(Debug, Clone, FromRow, Serialize)]
pub struct LevelUpgradeRequest {
    pub id: Uuid,
    pub user_id: Uuid,
    pub status: String,
    pub reason: Option<String>,
    pub reviewer_note: Option<String>,
    pub created_at: DateTime<Utc>,
    pub reviewed_at: Option<DateTime<Utc>>,
}

/// 模板
#[derive(Debug, Clone, FromRow)]
pub struct Template {
    pub id: Uuid,
    pub slug: String,
    pub title: String,
    pub description: String,
    pub category: Option<String>,
    pub like_count: i32,
    pub created_at: DateTime<Utc>,
    pub updated_at: DateTime<Utc>,
}

/// 模板代码
#[derive(Debug, Clone, FromRow)]
pub struct TemplateCode {
    pub id: Uuid,
    pub template_id: Uuid,
    pub language_id: String,
    pub code: String,
}

/// 模板评论
#[derive(Debug, Clone, FromRow)]
pub struct TemplateComment {
    pub id: Uuid,
    pub template_id: Uuid,
    pub user_id: Uuid,
    pub content: String,
    pub created_at: DateTime<Utc>,
    pub updated_at: Option<DateTime<Utc>>,
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_user_level_default() {
        assert_eq!(UserLevel::default(), UserLevel::Standard);
    }

    #[test]
    fn test_user_level_is_enhanced() {
        assert!(UserLevel::Enhanced.is_enhanced());
        assert!(!UserLevel::Standard.is_enhanced());
    }

    #[test]
    fn test_user_status_default() {
        assert_eq!(UserStatus::default(), UserStatus::Active);
    }

    #[test]
    fn test_user_status_can_login() {
        assert!(UserStatus::Active.can_login());
        assert!(!UserStatus::Suspended.can_login());
        assert!(!UserStatus::Banned.can_login());
    }

    #[test]
    fn test_submission_status_as_str() {
        assert_eq!(SubmissionStatus::Pending.as_str(), "pending");
        assert_eq!(SubmissionStatus::Running.as_str(), "running");
        assert_eq!(SubmissionStatus::Success.as_str(), "success");
        assert_eq!(SubmissionStatus::Failed.as_str(), "failed");
        assert_eq!(SubmissionStatus::Timeout.as_str(), "timeout");
    }
}