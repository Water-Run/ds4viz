//! 配额管理模块
//!
//! 负责用户配额的检查、更新和重置逻辑。
//!
//! # 配额策略
//!
//! - Standard 用户: 每小时 10000ms CPU时间
//! - Enhanced 用户: 无限制
//! - 缓存命中不消耗配额
//!
//! 作者: WaterRun
//! 日期: 2026-01-16

use chrono::{DateTime, Timelike, Utc};
use uuid::Uuid;

use crate::config::QuotaConfig;
use crate::db::Database;
use crate::error::{AppError, ForbiddenKind};
use crate::model::UserLevel;

/// 配额管理器
///
/// # 示例
///
/// ```rust,no_run
/// use ds4viz_server::config::QuotaConfig;
/// use ds4viz_server::db::Database;
/// use ds4viz_server::quota::QuotaManager;
/// use ds4viz_server::model::UserLevel;
/// use uuid::Uuid;
///
/// # async fn example() -> Result<(), Box<dyn std::error::Error>> {
/// # let db = Database::connect(&Default::default()).await?;
/// # let config = QuotaConfig::default();
/// let manager = QuotaManager::new(db, config);
///
/// let user_id = Uuid::new_v4();
/// manager.check_quota(user_id, UserLevel::Standard, 5000).await?;
/// # Ok(())
/// # }
/// ```
pub struct QuotaManager {
    db: Database,
    config: QuotaConfig,
}

impl QuotaManager {
    /// 创建新的配额管理器
    ///
    /// # 参数
    ///
    /// - `db`: 数据库连接
    /// - `config`: 配额配置
    ///
    /// # 示例
    ///
    /// ```rust,no_run
    /// # use ds4viz_server::config::QuotaConfig;
    /// # use ds4viz_server::db::Database;
    /// # use ds4viz_server::quota::QuotaManager;
    /// # async fn example() -> Result<(), Box<dyn std::error::Error>> {
    /// # let db = Database::connect(&Default::default()).await?;
    /// let config = QuotaConfig::default();
    /// let manager = QuotaManager::new(db, config);
    /// # Ok(())
    /// # }
    /// ```
    #[must_use]
    pub fn new(db: Database, config: QuotaConfig) -> Self {
        Self { db, config }
    }

    /// 检查用户配额是否充足
    ///
    /// Enhanced 用户始终通过检查。
    ///
    /// # 参数
    ///
    /// - `user_id`: 用户ID
    /// - `user_level`: 用户等级
    /// - `estimated_cpu_ms`: 预估的CPU时间（毫秒）
    ///
    /// # 返回值
    ///
    /// 配额充足返回 `Ok(())`,否则返回 `Err(AppError::Forbidden(QuotaExceeded))`。
    ///
    /// # 示例
    ///
    /// ```rust,no_run
    /// # use ds4viz_server::config::QuotaConfig;
    /// # use ds4viz_server::db::Database;
    /// # use ds4viz_server::quota::QuotaManager;
    /// # use ds4viz_server::model::UserLevel;
    /// # use uuid::Uuid;
    /// # async fn example() -> Result<(), Box<dyn std::error::Error>> {
    /// # let db = Database::connect(&Default::default()).await?;
    /// # let config = QuotaConfig::default();
    /// # let manager = QuotaManager::new(db, config);
    /// let user_id = Uuid::new_v4();
    ///
    /// // 检查是否可以执行需要5秒CPU时间的任务
    /// manager.check_quota(user_id, UserLevel::Standard, 5000).await?;
    /// # Ok(())
    /// # }
    /// ```
    pub async fn check_quota(
        &self,
        user_id: Uuid,
        user_level: UserLevel,
        estimated_cpu_ms: i64,
    ) -> Result<(), AppError> {
        // Enhanced 用户无限制
        if user_level == UserLevel::Enhanced {
            return Ok(());
        }

        // 获取当前周期
        let period_start = get_current_period_start(self.config.period_minutes);

        // 获取或创建配额记录
        let usage = self
            .db
            .get_or_create_quota_usage(user_id, period_start)
            .await?;

        // 检查配额
        let limit = self.config.standard_cpu_time_ms as i64;
        let new_usage = usage.cpu_time_used_ms + estimated_cpu_ms;

        if new_usage > limit {
            let _period_end =
                period_start + chrono::Duration::minutes(self.config.period_minutes as i64);
            return Err(AppError::Forbidden(ForbiddenKind::QuotaExceeded {
                period_start,
                period_end: _period_end,
                quota_limit_ms: limit as u64,
                quota_used_ms: usage.cpu_time_used_ms as u64,
            }));
        }

        Ok(())
    }

    /// 消耗配额
    ///
    /// Enhanced 用户不消耗配额。
    ///
    /// # 参数
    ///
    /// - `user_id`: 用户ID
    /// - `user_level`: 用户等级
    /// - `cpu_time_ms`: 实际消耗的CPU时间（毫秒）
    ///
    /// # 示例
    ///
    /// ```rust,no_run
    /// # use ds4viz_server::config::QuotaConfig;
    /// # use ds4viz_server::db::Database;
    /// # use ds4viz_server::quota::QuotaManager;
    /// # use ds4viz_server::model::UserLevel;
    /// # use uuid::Uuid;
    /// # async fn example() -> Result<(), Box<dyn std::error::Error>> {
    /// # let db = Database::connect(&Default::default()).await?;
    /// # let config = QuotaConfig::default();
    /// # let manager = QuotaManager::new(db, config);
    /// let user_id = Uuid::new_v4();
    ///
    /// // 执行完成后消耗配额
    /// manager.consume_quota(user_id, UserLevel::Standard, 1234).await?;
    /// # Ok(())
    /// # }
    /// ```
    pub async fn consume_quota(
        &self,
        user_id: Uuid,
        user_level: UserLevel,
        cpu_time_ms: i64,
    ) -> Result<(), AppError> {
        // Enhanced 用户不消耗配额
        if user_level == UserLevel::Enhanced {
            return Ok(());
        }

        let period_start = get_current_period_start(self.config.period_minutes);
        self.db
            .increment_quota_usage(user_id, period_start, cpu_time_ms)
            .await
    }

    /// 获取用户当前配额使用情况
    ///
    /// # 参数
    ///
    /// - `user_id`: 用户ID
    /// - `user_level`: 用户等级
    ///
    /// # 返回值
    ///
    /// 返回 `QuotaStatus` 枚举表示配额状态。
    ///
    /// # 示例
    ///
    /// ```rust,no_run
    /// # use ds4viz_server::config::QuotaConfig;
    /// # use ds4viz_server::db::Database;
    /// # use ds4viz_server::quota::{QuotaManager, QuotaStatus};
    /// # use ds4viz_server::model::UserLevel;
    /// # use uuid::Uuid;
    /// # async fn example() -> Result<(), Box<dyn std::error::Error>> {
    /// # let db = Database::connect(&Default::default()).await?;
    /// # let config = QuotaConfig::default();
    /// # let manager = QuotaManager::new(db, config);
    /// let user_id = Uuid::new_v4();
    ///
    /// let status = manager.get_quota_status(user_id, UserLevel::Standard).await?;
    /// match status {
    ///     QuotaStatus::Unlimited => println!("Unlimited quota"),
    ///     QuotaStatus::Limited { used_ms, limit_ms, .. } => {
    ///         println!("Used {} / {} ms", used_ms, limit_ms);
    ///     }
    /// }
    /// # Ok(())
    /// # }
    /// ```
    pub async fn get_quota_status(
        &self,
        user_id: Uuid,
        user_level: UserLevel,
    ) -> Result<QuotaStatus, AppError> {
        if user_level == UserLevel::Enhanced {
            return Ok(QuotaStatus::Unlimited);
        }

        let period_start = get_current_period_start(self.config.period_minutes);
        let usage = self
            .db
            .get_or_create_quota_usage(user_id, period_start)
            .await?;
        let limit = self.config.standard_cpu_time_ms as i64;

        let period_end =
            period_start + chrono::Duration::minutes(self.config.period_minutes as i64);

        Ok(QuotaStatus::Limited {
            period_start,
            period_end,
            limit_ms: limit,
            used_ms: usage.cpu_time_used_ms,
            remaining_ms: (limit - usage.cpu_time_used_ms).max(0),
            usage_percentage: (usage.cpu_time_used_ms as f64 / limit as f64 * 100.0).min(100.0),
        })
    }
}

/// 配额状态
///
/// # 示例
///
/// ```
/// use ds4viz_server::quota::QuotaStatus;
/// use chrono::Utc;
///
/// let status = QuotaStatus::Limited {
///     period_start: Utc::now(),
///     period_end: Utc::now(),
///     limit_ms: 10000,
///     used_ms: 5000,
///     remaining_ms: 5000,
///     usage_percentage: 50.0,
/// };
///
/// match status {
///     QuotaStatus::Limited { used_ms, .. } => assert_eq!(used_ms, 5000),
///     _ => panic!("应该是 Limited"),
/// }
/// ```
#[derive(Debug, Clone)]
pub enum QuotaStatus {
    /// Enhanced 用户无限制
    Unlimited,
    /// Standard 用户有限制
    Limited {
        /// 周期起始时间
        period_start: DateTime<Utc>,
        /// 周期结束时间
        period_end: DateTime<Utc>,
        /// 配额限制（毫秒）
        limit_ms: i64,
        /// 已使用（毫秒）
        used_ms: i64,
        /// 剩余（毫秒）
        remaining_ms: i64,
        /// 使用百分比
        usage_percentage: f64,
    },
}

/// 获取当前周期的起始时间
///
/// 周期按小时边界划分。例如:
/// - period_minutes = 60: 每小时一个周期 (14:35 -> 14:00)
/// - period_minutes = 30: 每半小时一个周期 (14:35 -> 14:30)
///
/// # 参数
///
/// - `period_minutes`: 周期长度（分钟）
///
/// # 返回值
///
/// 返回当前周期的起始时间（秒和纳秒归零）。
///
/// # 示例
///
/// ```
/// use ds4viz_server::quota::get_current_period_start;
/// use chrono::Utc;
///
/// let period_start = get_current_period_start(60);
/// assert_eq!(period_start.minute(), 0);
/// assert_eq!(period_start.second(), 0);
/// ```
#[must_use]
pub fn get_current_period_start(period_minutes: u64) -> DateTime<Utc> {
    let now = Utc::now();
    let minute = (now.minute() / period_minutes as u32) * period_minutes as u32;
    now.with_minute(minute)
        .unwrap()
        .with_second(0)
        .unwrap()
        .with_nanosecond(0)
        .unwrap()
}

// ============================================================================
// 单元测试
// ============================================================================

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_get_current_period_start_每小时() {
        let period_start = get_current_period_start(60);
        assert_eq!(period_start.minute(), 0, "分钟应为0");
        assert_eq!(period_start.second(), 0, "秒应为0");
        assert_eq!(period_start.nanosecond(), 0, "纳秒应为0");
    }

    #[test]
    fn test_get_current_period_start_每半小时() {
        let period_start = get_current_period_start(30);
        assert!(
            period_start.minute() == 0 || period_start.minute() == 30,
            "分钟应为0或30"
        );
        assert_eq!(period_start.second(), 0);
    }

    #[test]
    fn test_quota_status_unlimited() {
        let status = QuotaStatus::Unlimited;
        matches!(status, QuotaStatus::Unlimited);
    }

    #[test]
    fn test_quota_status_limited() {
        let status = QuotaStatus::Limited {
            period_start: Utc::now(),
            period_end: Utc::now(),
            limit_ms: 10000,
            used_ms: 3000,
            remaining_ms: 7000,
            usage_percentage: 30.0,
        };

        if let QuotaStatus::Limited {
            used_ms,
            remaining_ms,
            usage_percentage,
            ..
        } = status
        {
            assert_eq!(used_ms, 3000);
            assert_eq!(remaining_ms, 7000);
            assert_eq!(usage_percentage, 30.0);
        } else {
            panic!("应该是 Limited");
        }
    }
}
