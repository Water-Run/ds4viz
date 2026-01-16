//! 代码执行模块
//!
//! 负责协调沙箱执行、缓存管理和配额追踪。
//!
//! # 执行流程
//!
//! 1. 验证请求(代码大小、语言有效性)
//! 2. 计算代码哈希
//! 3. 检查缓存
//! 4. 检查配额
//! 5. 创建提交记录
//! 6. 异步执行代码
//! 7. 更新结果和配额
//!
//! 作者: WaterRun
//! 日期: 2026-01-16

use std::sync::Arc;
use uuid::Uuid;

use crate::cache::{CachedResult, ExecutionCacheManager, compute_code_hash};
use crate::config::Config;
use crate::db::Database;
use crate::error::{AppError, ForbiddenKind, InvalidRequestKind, NotFoundKind};
use crate::model::{CodeSubmission, ExecutionStatus, Language, User, UserLevel};
use crate::quota::QuotaManager;
use crate::sandbox::SandboxExecutor;

/// 执行服务
///
/// 协调沙箱执行、缓存、配额等所有执行相关逻辑。
///
/// # 示例
///
/// ```rust,no_run
/// use ds4viz_server::executions::ExecutionService;
/// use ds4viz_server::config::Config;
/// use ds4viz_server::db::Database;
///
/// # async fn example() -> Result<(), Box<dyn std::error::Error>> {
/// let config = Config::from_file("configs/default.toml")?;
/// let db = Database::connect(&config.database).await?;
/// let service = ExecutionService::new(db, config);
/// # Ok(())
/// # }
/// ```
#[derive(Clone)]
pub struct ExecutionService {
    db: Database,
    pub(crate) cache: Arc<ExecutionCacheManager>,
    quota: Arc<QuotaManager>,
    sandbox: Arc<SandboxExecutor>,
    config: Config,
}

impl ExecutionService {
    /// 创建新的执行服务
    ///
    /// # 参数
    ///
    /// - `db`: 数据库连接
    /// - `config`: 应用配置
    ///
    /// # 示例
    ///
    /// ```rust,no_run
    /// # use ds4viz_server::executions::ExecutionService;
    /// # use ds4viz_server::config::Config;
    /// # use ds4viz_server::db::Database;
    /// # async fn example() -> Result<(), Box<dyn std::error::Error>> {
    /// # let config = Config::from_file("configs/default.toml")?;
    /// # let db = Database::connect(&config.database).await?;
    /// let service = ExecutionService::new(db, config);
    /// # Ok(())
    /// # }
    /// ```
    #[must_use]
    pub fn new(db: Database, config: Config) -> Self {
        let cache = Arc::new(ExecutionCacheManager::new(db.clone()));
        let quota = Arc::new(QuotaManager::new(db.clone(), config.quota.clone()));
        let sandbox = Arc::new(SandboxExecutor::new(
            config.execution.temp_dir.clone().into(),
            config.execution.default_timeout_ms,
        ));

        Self {
            db,
            cache,
            quota,
            sandbox,
            config,
        }
    }

    /// 提交代码执行
    ///
    /// # 参数
    ///
    /// - `user`: 提交用户
    /// - `language_id`: 语言ID
    /// - `code`: 代码内容
    ///
    /// # 返回值
    ///
    /// 返回创建的提交记录。如果缓存命中,状态为Success;否则为Pending。
    ///
    /// # 错误
    ///
    /// - 代码过大
    /// - 语言无效或未启用
    /// - Standard用户使用编译型语言
    /// - 配额不足
    pub async fn submit_execution(
        &self,
        user: &User,
        language_id: String,
        code: String,
    ) -> Result<CodeSubmission, AppError> {
        // 1. 验证代码大小
        if user.level == UserLevel::Standard
            && code.len() > self.config.quota.standard_max_code_bytes
        {
            return Err(AppError::InvalidRequest(InvalidRequestKind::CodeTooLarge {
                max_bytes: self.config.quota.standard_max_code_bytes,
                actual_bytes: code.len(),
            }));
        }

        // 2. 验证语言
        let language = self
            .db
            .get_language_by_id(&language_id)
            .await?
            .ok_or_else(|| {
                AppError::InvalidRequest(InvalidRequestKind::InvalidLanguage {
                    language_id: language_id.clone(),
                })
            })?;

        if !language.enabled {
            return Err(AppError::NotFound(NotFoundKind::LanguageNotAvailable {
                language_id,
            }));
        }

        // 3. Standard用户只能使用解释型语言
        if user.level == UserLevel::Standard && !language.is_interpreted {
            return Err(AppError::Forbidden(ForbiddenKind::LanguageNotAllowed));
        }

        // 4. 计算代码哈希
        let code_hash = compute_code_hash(&language_id, &code);

        // 5. 检查缓存
        if let Some(cached) = self.cache.get(&code_hash).await? {
            // 缓存命中,创建提交记录但不执行
            let submission = CodeSubmission {
                id: Uuid::new_v4(),
                user_id: user.id,
                language_id,
                code_hash,
                status: ExecutionStatus::Success,
                cache_hit: true,
                cpu_time_ms: Some(cached.cpu_time_ms),
                created_at: chrono::Utc::now(),
            };

            return self.db.create_submission(&submission).await;
        }

        // 6. 检查配额 (预估5秒)
        self.quota.check_quota(user.id, user.level, 5000).await?;

        // 7. 先创建 execution_cache 占位记录 (满足外键约束)
        let cache_placeholder = crate::model::ExecutionCache {
            code_hash: code_hash.clone(),
            language_id: language_id.clone(),
            code: code.clone(),
            stdout: None,
            stderr: None,
            exit_code: None,
            cpu_time_ms: None,
            created_at: chrono::Utc::now(),
        };
        self.db.insert_execution_cache(&cache_placeholder).await?;

        // 8. 创建待执行的提交记录
        let submission = CodeSubmission {
            id: Uuid::new_v4(),
            user_id: user.id,
            language_id: language_id.clone(),
            code_hash: code_hash.clone(),
            status: ExecutionStatus::Pending,
            cache_hit: false,
            cpu_time_ms: None,
            created_at: chrono::Utc::now(),
        };

        let created = self.db.create_submission(&submission).await?;

        // 9. 异步执行代码
        let execution_id = created.id;
        let timeout = language.timeout_ms as u64;
        let user_id = user.id;
        let user_level = user.level;

        let db = self.db.clone();
        let cache = self.cache.clone();
        let quota = self.quota.clone();
        let sandbox = self.sandbox.clone();

        tokio::spawn(async move {
            Self::execute_code_async(
                db,
                cache,
                quota,
                sandbox,
                execution_id,
                language_id,
                code,
                code_hash,
                timeout,
                user_id,
                user_level,
            )
            .await;
        });

        Ok(created)
    }

    /// 异步执行代码
    ///
    /// 在独立的 tokio 任务中执行,不阻塞API响应。
    #[allow(clippy::too_many_arguments)]
    async fn execute_code_async(
        db: Database,
        cache: Arc<ExecutionCacheManager>,
        quota: Arc<QuotaManager>,
        sandbox: Arc<SandboxExecutor>,
        execution_id: Uuid,
        language_id: String,
        code: String,
        code_hash: String,
        timeout_ms: u64,
        user_id: Uuid,
        user_level: UserLevel,
    ) {
        // 更新状态为执行中
        let _ = db
            .update_submission_status(execution_id, ExecutionStatus::Running, None)
            .await;

        // 执行沙箱
        match sandbox
            .execute(execution_id, &language_id, &code, timeout_ms)
            .await
        {
            Ok(result) => {
                // 判断执行状态
                let status = if result.exit_code == 0 {
                    ExecutionStatus::Success
                } else if result.exit_code == 124 {
                    // timeout 命令的超时退出码
                    ExecutionStatus::Timeout
                } else {
                    ExecutionStatus::Failed
                };

                // 更新提交状态
                let _ = db
                    .update_submission_status(execution_id, status, Some(result.cpu_time_ms))
                    .await;

                // 仅缓存成功的执行结果
                if status == ExecutionStatus::Success {
                    let cached = CachedResult {
                        stdout: result.stdout,
                        stderr: result.stderr,
                        exit_code: result.exit_code,
                        cpu_time_ms: result.cpu_time_ms,
                    };
                    let _ = cache.set(code_hash, language_id, code, cached).await;
                }

                // 消耗配额
                let _ = quota
                    .consume_quota(user_id, user_level, result.cpu_time_ms)
                    .await;
            }
            Err(e) => {
                // 执行失败
                tracing::error!("代码执行失败: execution_id={}, error={}", execution_id, e);
                let status = if e.to_string().contains("超时") {
                    ExecutionStatus::Timeout
                } else {
                    ExecutionStatus::Failed
                };
                let _ = db
                    .update_submission_status(execution_id, status, None)
                    .await;
            }
        }
    }

    /// 获取执行结果
    ///
    /// # 参数
    ///
    /// - `execution_id`: 执行ID
    /// - `requesting_user_id`: 请求用户ID
    ///
    /// # 返回值
    ///
    /// 返回提交记录和缓存的执行结果(如果已完成)。
    ///
    /// # 错误
    ///
    /// - 执行不存在
    /// - 无权访问(不能查看他人的执行)
    pub async fn get_execution(
        &self,
        execution_id: Uuid,
        requesting_user_id: Uuid,
    ) -> Result<(CodeSubmission, Option<Language>, Option<CachedResult>), AppError> {
        // 获取提交记录
        let submission =
            self.db
                .get_submission_by_id(execution_id)
                .await?
                .ok_or(AppError::NotFound(NotFoundKind::ExecutionNotFound {
                    id: execution_id,
                }))?;

        // 权限检查: 只能查看自己的执行
        if submission.user_id != requesting_user_id {
            return Err(AppError::Forbidden(ForbiddenKind::AccessDenied));
        }

        // 获取语言信息
        let language = self.db.get_language_by_id(&submission.language_id).await?;

        // 如果执行已完成,获取缓存结果
        let result = if submission.status.is_completed() {
            self.cache.get(&submission.code_hash).await?
        } else {
            None
        };

        Ok((submission, language, result))
    }

    /// 获取用户的执行历史
    ///
    /// # 参数
    ///
    /// - `user_id`: 用户ID
    /// - `limit`: 返回数量
    /// - `offset`: 偏移量
    ///
    /// # 返回值
    ///
    /// 返回提交记录列表,按创建时间倒序。
    pub async fn get_user_submissions(
        &self,
        user_id: Uuid,
        limit: i64,
        offset: i64,
    ) -> Result<Vec<CodeSubmission>, AppError> {
        self.db.get_user_submissions(user_id, limit, offset).await
    }

    /// 获取所有语言配置
    ///
    /// # 参数
    ///
    /// - `enabled_only`: 是否只返回启用的语言
    ///
    /// # 返回值
    ///
    /// 返回语言列表,按 sort_order 排序。
    pub async fn get_all_languages(&self, enabled_only: bool) -> Result<Vec<Language>, AppError> {
        self.db.get_all_languages(enabled_only).await
    }
}

// ============================================================================
// 单元测试
// ============================================================================

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_execution_service_clone() {
        // 验证 ExecutionService 可以克隆
        fn assert_clone<T: Clone>() {}
        assert_clone::<ExecutionService>();
    }
}
