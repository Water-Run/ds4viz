//! 数据库连接池与用户数据操作模块
//!
//! 本模块提供 PostgreSQL 数据库连接池的创建与管理，
//! 以及用户相关的数据库操作方法。
//!
//! # 主要功能
//!
//! - 用户管理: 创建、查询用户
//! - 语言配置: 查询支持的编程语言
//! - 执行缓存: 代码执行结果的缓存读写
//! - 代码提交: 提交记录的增删改查
//! - 配额追踪: 用户配额使用情况管理
//!
//! 作者: WaterRun
//! 日期: 2026-01-16

use sqlx::PgPool;
use sqlx::postgres::PgPoolOptions;
use uuid::Uuid;

use crate::config::DatabaseConfig;
use crate::error::AppError;
use crate::model::User;

/// 数据库连接池包装
///
/// 封装 SQLx 的 PostgreSQL 连接池，提供统一的访问接口和用户操作方法。
///
/// # 示例
///
/// ```rust,no_run
/// use ds4viz_server::config::DatabaseConfig;
/// use ds4viz_server::db::Database;
///
/// # async fn example() -> Result<(), Box<dyn std::error::Error>> {
/// let config = DatabaseConfig {
///     url: "postgres://localhost/ds4viz".to_string(),
///     max_connections: 10,
/// };
/// let db = Database::connect(&config).await?;
///
/// // 查找用户
/// let user = db.find_user_by_username("testuser").await?;
/// # Ok(())
/// # }
/// ```
#[derive(Debug, Clone)]
pub struct Database {
    pool: PgPool,
}

impl Database {
    /// 从配置创建数据库连接池
    ///
    /// # 参数
    ///
    /// - `config`: 数据库配置，包含连接字符串和连接池大小
    ///
    /// # 返回值
    ///
    /// 成功返回 [`Database`] 实例，失败返回 [`AppError::Database`]。
    ///
    /// # 错误
    ///
    /// - 连接字符串无效或数据库不可达时返回错误
    ///
    /// # 示例
    ///
    /// ```rust,no_run
    /// use ds4viz_server::config::DatabaseConfig;
    /// use ds4viz_server::db::Database;
    ///
    /// # async fn example() -> Result<(), Box<dyn std::error::Error>> {
    /// let config = DatabaseConfig {
    ///     url: "postgres://user:pass@localhost/db".to_string(),
    ///     max_connections: 5,
    /// };
    /// let db = Database::connect(&config).await?;
    /// # Ok(())
    /// # }
    /// ```
    pub async fn connect(config: &DatabaseConfig) -> Result<Self, AppError> {
        let pool = PgPoolOptions::new()
            .max_connections(config.max_connections)
            .connect(&config.url)
            .await
            .map_err(|e| AppError::Database(format!("数据库连接失败: {e}")))?;

        Ok(Self { pool })
    }

    /// 获取连接池引用
    ///
    /// 返回内部 [`PgPool`] 的引用，供其他模块执行数据库操作。
    ///
    /// # 示例
    ///
    /// ```rust,no_run
    /// use ds4viz_server::config::DatabaseConfig;
    /// use ds4viz_server::db::Database;
    ///
    /// # async fn example() -> Result<(), Box<dyn std::error::Error>> {
    /// let config = DatabaseConfig {
    ///     url: "postgres://localhost/ds4viz".to_string(),
    ///     max_connections: 10,
    /// };
    /// let db = Database::connect(&config).await?;
    /// let pool = db.pool();
    /// // 使用 pool 执行查询...
    /// # Ok(())
    /// # }
    /// ```
    #[must_use]
    pub fn pool(&self) -> &PgPool {
        &self.pool
    }

    /// 创建新用户
    ///
    /// 在数据库中插入新用户记录。
    ///
    /// # 参数
    ///
    /// - `username`: 用户名
    /// - `email`: 邮箱地址
    /// - `password_hash`: 密码哈希（应使用 [`crate::auth::hash_password`] 生成）
    ///
    /// # 返回值
    ///
    /// 成功返回创建的 [`User`]，失败返回 [`AppError`]。
    ///
    /// # 错误
    ///
    /// - 用户名或邮箱已存在时返回数据库错误
    ///
    /// # 示例
    ///
    /// ```rust,no_run
    /// use ds4viz_server::db::Database;
    /// use ds4viz_server::auth::hash_password;
    /// use ds4viz_server::config::DatabaseConfig;
    ///
    /// # async fn example() -> Result<(), Box<dyn std::error::Error>> {
    /// # let config = DatabaseConfig { url: "".to_string(), max_connections: 5 };
    /// # let db = Database::connect(&config).await?;
    /// let password_hash = hash_password("my_password")?;
    /// let user = db.create_user("testuser", "test@example.com", &password_hash).await?;
    /// println!("Created user: {}", user.id);
    /// # Ok(())
    /// # }
    /// ```
    pub async fn create_user(
        &self,
        username: &str,
        email: &str,
        password_hash: &str,
    ) -> Result<User, AppError> {
        let user = sqlx::query_as::<_, User>(
            r#"
            INSERT INTO users (username, email, password_hash)
            VALUES ($1, $2, $3)
            RETURNING *
            "#,
        )
        .bind(username)
        .bind(email)
        .bind(password_hash)
        .fetch_one(&self.pool)
        .await?;

        Ok(user)
    }

    /// 按 ID 查找用户
    ///
    /// # 参数
    ///
    /// - `id`: 用户 UUID
    ///
    /// # 返回值
    ///
    /// 找到返回 `Some(User)`，未找到返回 `None`。
    ///
    /// # 示例
    ///
    /// ```rust,no_run
    /// use ds4viz_server::db::Database;
    /// use ds4viz_server::config::DatabaseConfig;
    /// use uuid::Uuid;
    ///
    /// # async fn example() -> Result<(), Box<dyn std::error::Error>> {
    /// # let config = DatabaseConfig { url: "".to_string(), max_connections: 5 };
    /// # let db = Database::connect(&config).await?;
    /// let user_id = Uuid::new_v4();
    /// if let Some(user) = db.find_user_by_id(user_id).await? {
    ///     println!("Found user: {}", user.username);
    /// }
    /// # Ok(())
    /// # }
    /// ```
    pub async fn find_user_by_id(&self, id: Uuid) -> Result<Option<User>, AppError> {
        let user = sqlx::query_as::<_, User>("SELECT * FROM users WHERE id = $1")
            .bind(id)
            .fetch_optional(&self.pool)
            .await?;

        Ok(user)
    }

    /// 按用户名查找用户
    ///
    /// # 参数
    ///
    /// - `username`: 用户名
    ///
    /// # 返回值
    ///
    /// 找到返回 `Some(User)`，未找到返回 `None`。
    ///
    /// # 示例
    ///
    /// ```rust,no_run
    /// use ds4viz_server::db::Database;
    /// use ds4viz_server::config::DatabaseConfig;
    ///
    /// # async fn example() -> Result<(), Box<dyn std::error::Error>> {
    /// # let config = DatabaseConfig { url: "".to_string(), max_connections: 5 };
    /// # let db = Database::connect(&config).await?;
    /// if let Some(user) = db.find_user_by_username("testuser").await? {
    ///     println!("Found user: {}", user.email);
    /// }
    /// # Ok(())
    /// # }
    /// ```
    pub async fn find_user_by_username(&self, username: &str) -> Result<Option<User>, AppError> {
        let user = sqlx::query_as::<_, User>("SELECT * FROM users WHERE username = $1")
            .bind(username)
            .fetch_optional(&self.pool)
            .await?;

        Ok(user)
    }

    /// 按邮箱查找用户
    ///
    /// # 参数
    ///
    /// - `email`: 邮箱地址
    ///
    /// # 返回值
    ///
    /// 找到返回 `Some(User)`，未找到返回 `None`。
    ///
    /// # 示例
    ///
    /// ```rust,no_run
    /// use ds4viz_server::db::Database;
    /// use ds4viz_server::config::DatabaseConfig;
    ///
    /// # async fn example() -> Result<(), Box<dyn std::error::Error>> {
    /// # let config = DatabaseConfig { url: "".to_string(), max_connections: 5 };
    /// # let db = Database::connect(&config).await?;
    /// if let Some(user) = db.find_user_by_email("test@example.com").await? {
    ///     println!("Found user: {}", user.username);
    /// }
    /// # Ok(())
    /// # }
    /// ```
    pub async fn find_user_by_email(&self, email: &str) -> Result<Option<User>, AppError> {
        let user = sqlx::query_as::<_, User>("SELECT * FROM users WHERE email = $1")
            .bind(email)
            .fetch_optional(&self.pool)
            .await?;

        Ok(user)
    }

    /// 按用户名或邮箱查找用户
    ///
    /// 用于登录场景，支持用户使用用户名或邮箱登录。
    ///
    /// # 参数
    ///
    /// - `identifier`: 用户名或邮箱
    ///
    /// # 返回值
    ///
    /// 找到返回 `Some(User)`，未找到返回 `None`。
    ///
    /// # 示例
    ///
    /// ```rust,no_run
    /// use ds4viz_server::db::Database;
    /// use ds4viz_server::config::DatabaseConfig;
    ///
    /// # async fn example() -> Result<(), Box<dyn std::error::Error>> {
    /// # let config = DatabaseConfig { url: "".to_string(), max_connections: 5 };
    /// # let db = Database::connect(&config).await?;
    /// // 支持用户名或邮箱登录
    /// let user = db.find_user_by_username_or_email("testuser").await?;
    /// let user = db.find_user_by_username_or_email("test@example.com").await?;
    /// # Ok(())
    /// # }
    /// ```
    pub async fn find_user_by_username_or_email(
        &self,
        identifier: &str,
    ) -> Result<Option<User>, AppError> {
        let user =
            sqlx::query_as::<_, User>("SELECT * FROM users WHERE username = $1 OR email = $1")
                .bind(identifier)
                .fetch_optional(&self.pool)
                .await?;

        Ok(user)
    }

    /// 检查用户名是否已存在
    ///
    /// # 参数
    ///
    /// - `username`: 用户名
    ///
    /// # 返回值
    ///
    /// 存在返回 `true`，不存在返回 `false`。
    ///
    /// # 示例
    ///
    /// ```rust,no_run
    /// use ds4viz_server::db::Database;
    /// use ds4viz_server::config::DatabaseConfig;
    ///
    /// # async fn example() -> Result<(), Box<dyn std::error::Error>> {
    /// # let config = DatabaseConfig { url: "".to_string(), max_connections: 5 };
    /// # let db = Database::connect(&config).await?;
    /// if db.username_exists("testuser").await? {
    ///     println!("Username already taken");
    /// }
    /// # Ok(())
    /// # }
    /// ```
    pub async fn username_exists(&self, username: &str) -> Result<bool, AppError> {
        let exists: (bool,) =
            sqlx::query_as("SELECT EXISTS(SELECT 1 FROM users WHERE username = $1)")
                .bind(username)
                .fetch_one(&self.pool)
                .await?;

        Ok(exists.0)
    }

    /// 检查邮箱是否已存在
    ///
    /// # 参数
    ///
    /// - `email`: 邮箱地址
    ///
    /// # 返回值
    ///
    /// 存在返回 `true`，不存在返回 `false`。
    ///
    /// # 示例
    ///
    /// ```rust,no_run
    /// use ds4viz_server::db::Database;
    /// use ds4viz_server::config::DatabaseConfig;
    ///
    /// # async fn example() -> Result<(), Box<dyn std::error::Error>> {
    /// # let config = DatabaseConfig { url: "".to_string(), max_connections: 5 };
    /// # let db = Database::connect(&config).await?;
    /// if db.email_exists("test@example.com").await? {
    ///     println!("Email already registered");
    /// }
    /// # Ok(())
    /// # }
    /// ```
    pub async fn email_exists(&self, email: &str) -> Result<bool, AppError> {
        let exists: (bool,) = sqlx::query_as("SELECT EXISTS(SELECT 1 FROM users WHERE email = $1)")
            .bind(email)
            .fetch_one(&self.pool)
            .await?;

        Ok(exists.0)
    }

    // ========================================================================
    // 语言配置操作
    // ========================================================================

    /// 获取所有语言配置
    ///
    /// # 参数
    ///
    /// - `enabled_only`: 是否只返回启用的语言
    ///
    /// # 返回值
    ///
    /// 返回语言列表,按 `sort_order` 升序排序。
    ///
    /// # 示例
    ///
    /// ```rust,no_run
    /// # use ds4viz_server::config::DatabaseConfig;
    /// # use ds4viz_server::db::Database;
    /// # async fn example() -> Result<(), Box<dyn std::error::Error>> {
    /// # let config = DatabaseConfig { url: "postgres://localhost/ds4viz".to_string(), max_connections: 10 };
    /// # let db = Database::connect(&config).await?;
    /// let languages = db.get_all_languages(true).await?;
    /// for lang in languages {
    ///     println!("{}: {}", lang.id, lang.display_name);
    /// }
    /// # Ok(())
    /// # }
    /// ```
    pub async fn get_all_languages(
        &self,
        enabled_only: bool,
    ) -> Result<Vec<crate::model::Language>, AppError> {
        let query = if enabled_only {
            "SELECT * FROM languages WHERE enabled = true ORDER BY sort_order"
        } else {
            "SELECT * FROM languages ORDER BY sort_order"
        };

        let languages = sqlx::query_as(query).fetch_all(&self.pool).await?;

        Ok(languages)
    }

    /// 根据ID查找语言
    ///
    /// # 参数
    ///
    /// - `id`: 语言ID
    ///
    /// # 返回值
    ///
    /// 找到返回 `Some(Language)`,否则返回 `None`。
    ///
    /// # 示例
    ///
    /// ```rust,no_run
    /// # use ds4viz_server::config::DatabaseConfig;
    /// # use ds4viz_server::db::Database;
    /// # async fn example() -> Result<(), Box<dyn std::error::Error>> {
    /// # let config = DatabaseConfig { url: "postgres://localhost/ds4viz".to_string(), max_connections: 10 };
    /// # let db = Database::connect(&config).await?;
    /// if let Some(lang) = db.get_language_by_id("python").await? {
    ///     println!("Python version: {}", lang.version);
    /// }
    /// # Ok(())
    /// # }
    /// ```
    pub async fn get_language_by_id(
        &self,
        id: &str,
    ) -> Result<Option<crate::model::Language>, AppError> {
        let lang = sqlx::query_as("SELECT * FROM languages WHERE id = $1")
            .bind(id)
            .fetch_optional(&self.pool)
            .await?;

        Ok(lang)
    }

    // ========================================================================
    // 执行缓存操作
    // ========================================================================

    /// 根据代码哈希查询缓存
    ///
    /// # 参数
    ///
    /// - `code_hash`: 代码哈希值
    ///
    /// # 返回值
    ///
    /// 找到返回 `Some(ExecutionCache)`,否则返回 `None`。
    ///
    /// # 示例
    ///
    /// ```rust,no_run
    /// # use ds4viz_server::config::DatabaseConfig;
    /// # use ds4viz_server::db::Database;
    /// # async fn example() -> Result<(), Box<dyn std::error::Error>> {
    /// # let config = DatabaseConfig { url: "postgres://localhost/ds4viz".to_string(), max_connections: 10 };
    /// # let db = Database::connect(&config).await?;
    /// let cache = db.get_execution_cache("abc123...").await?;
    /// if let Some(c) = cache {
    ///     println!("Cache hit! Exit code: {:?}", c.exit_code);
    /// }
    /// # Ok(())
    /// # }
    /// ```
    pub async fn get_execution_cache(
        &self,
        code_hash: &str,
    ) -> Result<Option<crate::model::ExecutionCache>, AppError> {
        let cache = sqlx::query_as("SELECT * FROM execution_cache WHERE code_hash = $1")
            .bind(code_hash)
            .fetch_optional(&self.pool)
            .await?;

        Ok(cache)
    }

    /// 插入执行缓存
    ///
    /// # 参数
    ///
    /// - `cache`: 缓存数据
    ///
    /// # 示例
    ///
    /// ```rust,no_run
    /// # use ds4viz_server::config::DatabaseConfig;
    /// # use ds4viz_server::db::Database;
    /// # use ds4viz_server::model::ExecutionCache;
    /// # use chrono::Utc;
    /// # async fn example() -> Result<(), Box<dyn std::error::Error>> {
    /// # let config = DatabaseConfig { url: "postgres://localhost/ds4viz".to_string(), max_connections: 10 };
    /// # let db = Database::connect(&config).await?;
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
    /// db.insert_execution_cache(&cache).await?;
    /// # Ok(())
    /// # }
    /// ```
    pub async fn insert_execution_cache(
        &self,
        cache: &crate::model::ExecutionCache,
    ) -> Result<(), AppError> {
        sqlx::query(
            "INSERT INTO execution_cache (code_hash, language_id, code, stdout, stderr, exit_code, cpu_time_ms, created_at)
             VALUES ($1, $2, $3, $4, $5, $6, $7, $8)
             ON CONFLICT (code_hash) DO NOTHING"
        )
        .bind(&cache.code_hash)
        .bind(&cache.language_id)
        .bind(&cache.code)
        .bind(&cache.stdout)
        .bind(&cache.stderr)
        .bind(cache.exit_code)
        .bind(cache.cpu_time_ms)
        .bind(cache.created_at)
        .execute(&self.pool)
        .await?;

        Ok(())
    }

    // ========================================================================
    // 代码提交操作
    // ========================================================================

    /// 创建代码提交记录
    ///
    /// # 参数
    ///
    /// - `submission`: 提交数据
    ///
    /// # 返回值
    ///
    /// 返回创建的提交记录（包含数据库生成的ID）。
    ///
    /// # 示例
    ///
    /// ```rust,no_run
    /// # use ds4viz_server::config::DatabaseConfig;
    /// # use ds4viz_server::db::Database;
    /// # use ds4viz_server::model::{CodeSubmission, ExecutionStatus};
    /// # use chrono::Utc;
    /// # use uuid::Uuid;
    /// # async fn example() -> Result<(), Box<dyn std::error::Error>> {
    /// # let config = DatabaseConfig { url: "postgres://localhost/ds4viz".to_string(), max_connections: 10 };
    /// # let db = Database::connect(&config).await?;
    /// let submission = CodeSubmission {
    ///     id: Uuid::new_v4(),
    ///     user_id: Uuid::new_v4(),
    ///     language_id: "python".to_string(),
    ///     code_hash: "abc123...".to_string(),
    ///     status: ExecutionStatus::Pending,
    ///     cache_hit: false,
    ///     cpu_time_ms: None,
    ///     created_at: Utc::now(),
    /// };
    /// let created = db.create_submission(&submission).await?;
    /// println!("Created submission: {}", created.id);
    /// # Ok(())
    /// # }
    /// ```
    pub async fn create_submission(
        &self,
        submission: &crate::model::CodeSubmission,
    ) -> Result<crate::model::CodeSubmission, AppError> {
        let created = sqlx::query_as(
            "INSERT INTO code_submissions (id, user_id, language_id, code_hash, status, cache_hit, cpu_time_ms, created_at)
             VALUES ($1, $2, $3, $4, $5, $6, $7, $8)
             RETURNING *"
        )
        .bind(submission.id)
        .bind(submission.user_id)
        .bind(&submission.language_id)
        .bind(&submission.code_hash)
        .bind(submission.status)
        .bind(submission.cache_hit)
        .bind(submission.cpu_time_ms)
        .bind(submission.created_at)
        .fetch_one(&self.pool)
        .await?;

        Ok(created)
    }

    /// 更新提交状态
    ///
    /// # 参数
    ///
    /// - `id`: 提交ID
    /// - `status`: 新状态
    /// - `cpu_time_ms`: CPU耗时（可选）
    ///
    /// # 示例
    ///
    /// ```rust,no_run
    /// # use ds4viz_server::config::DatabaseConfig;
    /// # use ds4viz_server::db::Database;
    /// # use ds4viz_server::model::ExecutionStatus;
    /// # use uuid::Uuid;
    /// # async fn example() -> Result<(), Box<dyn std::error::Error>> {
    /// # let config = DatabaseConfig { url: "postgres://localhost/ds4viz".to_string(), max_connections: 10 };
    /// # let db = Database::connect(&config).await?;
    /// # let execution_id = Uuid::new_v4();
    /// db.update_submission_status(execution_id, ExecutionStatus::Success, Some(234)).await?;
    /// # Ok(())
    /// # }
    /// ```
    pub async fn update_submission_status(
        &self,
        id: Uuid,
        status: crate::model::ExecutionStatus,
        cpu_time_ms: Option<i64>,
    ) -> Result<(), AppError> {
        sqlx::query("UPDATE code_submissions SET status = $1, cpu_time_ms = $2 WHERE id = $3")
            .bind(status)
            .bind(cpu_time_ms)
            .bind(id)
            .execute(&self.pool)
            .await?;

        Ok(())
    }

    /// 根据ID查找提交记录
    ///
    /// # 参数
    ///
    /// - `id`: 提交ID
    ///
    /// # 返回值
    ///
    /// 找到返回 `Some(CodeSubmission)`,否则返回 `None`。
    ///
    /// # 示例
    ///
    /// ```rust,no_run
    /// # use ds4viz_server::config::DatabaseConfig;
    /// # use ds4viz_server::db::Database;
    /// # use uuid::Uuid;
    /// # async fn example() -> Result<(), Box<dyn std::error::Error>> {
    /// # let config = DatabaseConfig { url: "postgres://localhost/ds4viz".to_string(), max_connections: 10 };
    /// # let db = Database::connect(&config).await?;
    /// # let execution_id = Uuid::new_v4();
    /// let submission = db.get_submission_by_id(execution_id).await?;
    /// if let Some(s) = submission {
    ///     println!("Status: {}", s.status.as_str());
    /// }
    /// # Ok(())
    /// # }
    /// ```
    pub async fn get_submission_by_id(
        &self,
        id: Uuid,
    ) -> Result<Option<crate::model::CodeSubmission>, AppError> {
        let submission = sqlx::query_as("SELECT * FROM code_submissions WHERE id = $1")
            .bind(id)
            .fetch_optional(&self.pool)
            .await?;

        Ok(submission)
    }

    /// 获取用户的提交历史
    ///
    /// # 参数
    ///
    /// - `user_id`: 用户ID
    /// - `limit`: 返回数量
    /// - `offset`: 偏移量
    ///
    /// # 返回值
    ///
    /// 返回提交列表,按创建时间倒序排列。
    ///
    /// # 示例
    ///
    /// ```rust,no_run
    /// # use ds4viz_server::config::DatabaseConfig;
    /// # use ds4viz_server::db::Database;
    /// # use uuid::Uuid;
    /// # async fn example() -> Result<(), Box<dyn std::error::Error>> {
    /// # let config = DatabaseConfig { url: "postgres://localhost/ds4viz".to_string(), max_connections: 10 };
    /// # let db = Database::connect(&config).await?;
    /// # let user_id = Uuid::new_v4();
    /// let submissions = db.get_user_submissions(user_id, 20, 0).await?;
    /// for s in submissions {
    ///     println!("Submission {}: {}", s.id, s.status.as_str());
    /// }
    /// # Ok(())
    /// # }
    /// ```
    pub async fn get_user_submissions(
        &self,
        user_id: Uuid,
        limit: i64,
        offset: i64,
    ) -> Result<Vec<crate::model::CodeSubmission>, AppError> {
        let submissions = sqlx::query_as(
            "SELECT * FROM code_submissions WHERE user_id = $1 ORDER BY created_at DESC LIMIT $2 OFFSET $3"
        )
        .bind(user_id)
        .bind(limit)
        .bind(offset)
        .fetch_all(&self.pool)
        .await?;

        Ok(submissions)
    }

    // ========================================================================
    // 配额管理操作
    // ========================================================================

    /// 获取或创建用户配额使用记录
    ///
    /// 如果指定周期的记录不存在,则创建一条初始记录（已用时间为0）。
    ///
    /// # 参数
    ///
    /// - `user_id`: 用户ID
    /// - `period_start`: 周期起始时间
    ///
    /// # 返回值
    ///
    /// 返回配额使用记录。
    ///
    /// # 示例
    ///
    /// ```rust,no_run
    /// # use ds4viz_server::config::DatabaseConfig;
    /// # use ds4viz_server::db::Database;
    /// # use chrono::Utc;
    /// # use uuid::Uuid;
    /// # async fn example() -> Result<(), Box<dyn std::error::Error>> {
    /// # let config = DatabaseConfig { url: "postgres://localhost/ds4viz".to_string(), max_connections: 10 };
    /// # let db = Database::connect(&config).await?;
    /// # let user_id = Uuid::new_v4();
    /// let period_start = Utc::now();
    /// let usage = db.get_or_create_quota_usage(user_id, period_start).await?;
    /// println!("Used: {} ms", usage.cpu_time_used_ms);
    /// # Ok(())
    /// # }
    /// ```
    pub async fn get_or_create_quota_usage(
        &self,
        user_id: Uuid,
        period_start: chrono::DateTime<chrono::Utc>,
    ) -> Result<crate::model::UserQuotaUsage, AppError> {
        let usage = sqlx::query_as(
            "INSERT INTO user_quota_usage (id, user_id, period_start, cpu_time_used_ms)
             VALUES (gen_random_uuid(), $1, $2, 0)
             ON CONFLICT (user_id, period_start) DO UPDATE SET user_id = user_quota_usage.user_id
             RETURNING *",
        )
        .bind(user_id)
        .bind(period_start)
        .fetch_one(&self.pool)
        .await?;

        Ok(usage)
    }

    /// 增加配额使用量
    ///
    /// # 参数
    ///
    /// - `user_id`: 用户ID
    /// - `period_start`: 周期起始时间
    /// - `cpu_time_ms`: 增加的CPU时间（毫秒）
    ///
    /// # 示例
    ///
    /// ```rust,no_run
    /// # use ds4viz_server::config::DatabaseConfig;
    /// # use ds4viz_server::db::Database;
    /// # use chrono::Utc;
    /// # use uuid::Uuid;
    /// # async fn example() -> Result<(), Box<dyn std::error::Error>> {
    /// # let config = DatabaseConfig { url: "postgres://localhost/ds4viz".to_string(), max_connections: 10 };
    /// # let db = Database::connect(&config).await?;
    /// # let user_id = Uuid::new_v4();
    /// let period_start = Utc::now();
    /// db.increment_quota_usage(user_id, period_start, 1000).await?;
    /// # Ok(())
    /// # }
    /// ```
    pub async fn increment_quota_usage(
        &self,
        user_id: Uuid,
        period_start: chrono::DateTime<chrono::Utc>,
        cpu_time_ms: i64,
    ) -> Result<(), AppError> {
        sqlx::query(
            "UPDATE user_quota_usage SET cpu_time_used_ms = cpu_time_used_ms + $1
             WHERE user_id = $2 AND period_start = $3",
        )
        .bind(cpu_time_ms)
        .bind(user_id)
        .bind(period_start)
        .execute(&self.pool)
        .await?;

        Ok(())
    }
}
