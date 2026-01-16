//! 执行结果缓存模块
//!
//! 使用双层缓存架构: 内存LRU + PostgreSQL持久化。
//!
//! # 缓存策略
//!
//! - L1: LRU内存缓存（1000条），快速访问
//! - L2: PostgreSQL持久化，跨进程共享
//! - 缓存键: SHA256(language_id + code)
//!
//! 作者: WaterRun
//! 日期: 2026-01-16

use std::num::NonZeroUsize;
use std::sync::{Arc, Mutex};

use lru::LruCache;
use sha2::{Digest, Sha256};

use crate::db::Database;
use crate::error::AppError;
use crate::model::ExecutionCache;

/// 内存缓存大小
const MEMORY_CACHE_SIZE: usize = 1000;

/// 缓存的执行结果
///
/// # 示例
///
/// ```
/// use ds4viz_server::cache::CachedResult;
///
/// let result = CachedResult {
///     stdout: "hello\n".to_string(),
///     stderr: String::new(),
///     exit_code: 0,
///     cpu_time_ms: 123,
/// };
///
/// assert_eq!(result.exit_code, 0);
/// ```
#[derive(Debug, Clone)]
pub struct CachedResult {
    /// 标准输出
    pub stdout: String,
    /// 标准错误
    pub stderr: String,
    /// 退出码
    pub exit_code: i32,
    /// CPU耗时（毫秒）
    pub cpu_time_ms: i64,
}

/// 执行结果缓存管理器
///
/// # 示例
///
/// ```rust,no_run
/// use ds4viz_server::cache::ExecutionCacheManager;
/// use ds4viz_server::db::Database;
///
/// # async fn example() -> Result<(), Box<dyn std::error::Error>> {
/// # let db = Database::connect(&Default::default()).await?;
/// let cache_manager = ExecutionCacheManager::new(db);
///
/// // 查询缓存
/// let code_hash = "abc123...";
/// if let Some(result) = cache_manager.get(code_hash).await? {
///     println!("Cache hit! Output: {}", result.stdout);
/// }
/// # Ok(())
/// # }
/// ```
pub struct ExecutionCacheManager {
    memory_cache: Arc<Mutex<LruCache<String, CachedResult>>>,
    db: Database,
}

impl ExecutionCacheManager {
    /// 创建新的缓存管理器
    ///
    /// # 参数
    ///
    /// - `db`: 数据库连接
    ///
    /// # 示例
    ///
    /// ```rust,no_run
    /// # use ds4viz_server::cache::ExecutionCacheManager;
    /// # use ds4viz_server::db::Database;
    /// # async fn example() -> Result<(), Box<dyn std::error::Error>> {
    /// # let db = Database::connect(&Default::default()).await?;
    /// let manager = ExecutionCacheManager::new(db);
    /// # Ok(())
    /// # }
    /// ```
    #[must_use]
    pub fn new(db: Database) -> Self {
        Self {
            memory_cache: Arc::new(Mutex::new(LruCache::new(
                NonZeroUsize::new(MEMORY_CACHE_SIZE).unwrap(),
            ))),
            db,
        }
    }

    /// 获取缓存的执行结果
    ///
    /// 先查内存缓存,未命中则查数据库。查到后会写入内存缓存。
    ///
    /// # 参数
    ///
    /// - `code_hash`: 代码哈希值
    ///
    /// # 返回值
    ///
    /// - 缓存存在返回 `Some(CachedResult)`
    /// - 缓存不存在返回 `None`
    ///
    /// # 示例
    ///
    /// ```rust,no_run
    /// # use ds4viz_server::cache::ExecutionCacheManager;
    /// # use ds4viz_server::db::Database;
    /// # async fn example() -> Result<(), Box<dyn std::error::Error>> {
    /// # let db = Database::connect(&Default::default()).await?;
    /// # let manager = ExecutionCacheManager::new(db);
    /// let result = manager.get("abc123...").await?;
    /// match result {
    ///     Some(r) => println!("Cache hit: {}", r.stdout),
    ///     None => println!("Cache miss"),
    /// }
    /// # Ok(())
    /// # }
    /// ```
    pub async fn get(&self, code_hash: &str) -> Result<Option<CachedResult>, AppError> {
        // 1. 先查内存缓存
        if let Some(result) = self.memory_cache.lock().unwrap().get(code_hash) {
            return Ok(Some(result.clone()));
        }

        // 2. 查数据库
        if let Some(cache) = self.db.get_execution_cache(code_hash).await? {
            let result = CachedResult {
                stdout: cache.stdout.unwrap_or_default(),
                stderr: cache.stderr.unwrap_or_default(),
                exit_code: cache.exit_code.unwrap_or(-1),
                cpu_time_ms: cache.cpu_time_ms.unwrap_or(0),
            };

            // 3. 写入内存缓存
            self.memory_cache
                .lock()
                .unwrap()
                .put(code_hash.to_string(), result.clone());

            return Ok(Some(result));
        }

        Ok(None)
    }

    /// 设置缓存
    ///
    /// 同时写入数据库和内存缓存。
    ///
    /// # 参数
    ///
    /// - `code_hash`: 代码哈希值
    /// - `language_id`: 语言ID
    /// - `code`: 原始代码
    /// - `result`: 执行结果
    ///
    /// # 示例
    ///
    /// ```rust,no_run
    /// # use ds4viz_server::cache::{ExecutionCacheManager, CachedResult};
    /// # use ds4viz_server::db::Database;
    /// # async fn example() -> Result<(), Box<dyn std::error::Error>> {
    /// # let db = Database::connect(&Default::default()).await?;
    /// # let manager = ExecutionCacheManager::new(db);
    /// let result = CachedResult {
    ///     stdout: "hello\n".to_string(),
    ///     stderr: String::new(),
    ///     exit_code: 0,
    ///     cpu_time_ms: 123,
    /// };
    ///
    /// manager.set(
    ///     "abc123...".to_string(),
    ///     "python".to_string(),
    ///     "print('hello')".to_string(),
    ///     result
    /// ).await?;
    /// # Ok(())
    /// # }
    /// ```
    pub async fn set(
        &self,
        code_hash: String,
        language_id: String,
        code: String,
        result: CachedResult,
    ) -> Result<(), AppError> {
        // 1. 写入数据库
        let cache = ExecutionCache {
            code_hash: code_hash.clone(),
            language_id,
            code,
            stdout: Some(result.stdout.clone()),
            stderr: Some(result.stderr.clone()),
            exit_code: Some(result.exit_code),
            cpu_time_ms: Some(result.cpu_time_ms),
            created_at: chrono::Utc::now(),
        };
        self.db.insert_execution_cache(&cache).await?;

        // 2. 写入内存缓存
        self.memory_cache.lock().unwrap().put(code_hash, result);

        Ok(())
    }
}

/// 计算代码哈希
///
/// 使用 SHA256(language_id + code) 作为缓存键。
///
/// # 参数
///
/// - `language_id`: 语言ID
/// - `code`: 代码内容
///
/// # 返回值
///
/// 返回64字符的十六进制哈希字符串。
///
/// # 示例
///
/// ```
/// use ds4viz_server::cache::compute_code_hash;
///
/// let hash1 = compute_code_hash("python", "print('hello')");
/// let hash2 = compute_code_hash("python", "print('hello')");
/// let hash3 = compute_code_hash("python", "print('world')");
///
/// assert_eq!(hash1, hash2, "相同代码应产生相同哈希");
/// assert_ne!(hash1, hash3, "不同代码应产生不同哈希");
/// assert_eq!(hash1.len(), 64, "SHA256哈希应为64字符");
/// ```
#[must_use]
pub fn compute_code_hash(language_id: &str, code: &str) -> String {
    let mut hasher = Sha256::new();
    hasher.update(language_id.as_bytes());
    hasher.update(code.as_bytes());
    hex::encode(hasher.finalize())
}

// ============================================================================
// 单元测试
// ============================================================================

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_compute_code_hash_确定性() {
        let hash1 = compute_code_hash("python", "print('hello')");
        let hash2 = compute_code_hash("python", "print('hello')");

        assert_eq!(hash1, hash2, "相同输入应产生相同哈希");
    }

    #[test]
    fn test_compute_code_hash_不同代码() {
        let hash1 = compute_code_hash("python", "print('hello')");
        let hash2 = compute_code_hash("python", "print('world')");

        assert_ne!(hash1, hash2, "不同代码应产生不同哈希");
    }

    #[test]
    fn test_compute_code_hash_不同语言() {
        let hash1 = compute_code_hash("python", "print('test')");
        let hash2 = compute_code_hash("lua", "print('test')");

        assert_ne!(hash1, hash2, "不同语言应产生不同哈希");
    }

    #[test]
    fn test_compute_code_hash_长度() {
        let hash = compute_code_hash("python", "test");

        assert_eq!(hash.len(), 64, "SHA256哈希应为64字符");
    }

    #[test]
    fn test_cached_result_creation() {
        let result = CachedResult {
            stdout: "output".to_string(),
            stderr: "error".to_string(),
            exit_code: 1,
            cpu_time_ms: 500,
        };

        assert_eq!(result.stdout, "output");
        assert_eq!(result.stderr, "error");
        assert_eq!(result.exit_code, 1);
        assert_eq!(result.cpu_time_ms, 500);
    }
}
