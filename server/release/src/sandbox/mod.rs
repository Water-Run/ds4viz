//! 沙箱执行模块
//!
//! 提供安全的代码执行环境,使用 systemd-run 进行资源隔离。
//!
//! # 安全措施
//!
//! - CPU/内存/进程数限制
//! - 文件系统隔离
//! - 超时控制
//! - 禁止网络访问(可选)
//!
//! 作者: WaterRun
//! 日期: 2026-01-16

use std::fs;
use std::path::PathBuf;
use uuid::Uuid;

use crate::error::AppError;

pub mod languages;
pub mod systemd;
pub mod utils;

/// 沙箱执行结果
///
/// # 示例
///
/// ```
/// use ds4viz_server::sandbox::SandboxResult;
///
/// let result = SandboxResult {
///     stdout: "hello\n".to_string(),
///     stderr: String::new(),
///     exit_code: 0,
///     cpu_time_ms: 123,
///     toml_output: Some("[data]\nvalue = 42".to_string()),
/// };
///
/// assert_eq!(result.exit_code, 0);
/// assert!(result.toml_output.is_some());
/// ```
#[derive(Debug, Clone)]
pub struct SandboxResult {
    /// 标准输出
    pub stdout: String,
    /// 标准错误
    pub stderr: String,
    /// 退出码
    pub exit_code: i32,
    /// CPU耗时（毫秒）
    pub cpu_time_ms: i64,
    /// TOML 输出内容（如果存在）
    pub toml_output: Option<String>,
}

/// 沙箱执行器
///
/// 负责准备执行环境、调用 systemd-run、收集结果并清理。
///
/// # 示例
///
/// ```rust,no_run
/// use std::path::PathBuf;
/// use ds4viz_server::sandbox::SandboxExecutor;
/// use uuid::Uuid;
///
/// # async fn example() -> Result<(), Box<dyn std::error::Error>> {
/// let executor = SandboxExecutor::new(PathBuf::from("/tmp/ds4viz"), 30000);
///
/// let execution_id = Uuid::new_v4();
/// let code = "print('hello world')";
/// let result = executor.execute(execution_id, "python", code, 30000).await?;
///
/// println!("Output: {}", result.stdout);
/// println!("Exit code: {}", result.exit_code);
/// # Ok(())
/// # }
/// ```
pub struct SandboxExecutor {
    temp_dir: PathBuf,
    default_timeout_ms: u64,
}

impl SandboxExecutor {
    /// 创建新的沙箱执行器
    ///
    /// # 参数
    ///
    /// - `temp_dir`: 临时文件目录
    /// - `default_timeout_ms`: 默认超时时间（毫秒）
    ///
    /// # 示例
    ///
    /// ```
    /// use std::path::PathBuf;
    /// use ds4viz_server::sandbox::SandboxExecutor;
    ///
    /// let executor = SandboxExecutor::new(PathBuf::from("/tmp/ds4viz"), 30000);
    /// ```
    #[must_use]
    pub fn new(temp_dir: PathBuf, default_timeout_ms: u64) -> Self {
        Self {
            temp_dir,
            default_timeout_ms,
        }
    }

    /// 执行代码
    ///
    /// 完整流程:
    /// 1. 确保临时目录存在
    /// 2. 创建代码和TOML文件
    /// 3. 写入代码内容
    /// 4. 构造执行命令
    /// 5. 通过 systemd-run 执行
    /// 6. 读取 TOML 输出（如果有）
    /// 7. 清理临时文件
    ///
    /// # 参数
    ///
    /// - `execution_id`: 执行ID（用于生成唯一文件名）
    /// - `language_id`: 语言标识符
    /// - `code`: 代码内容
    /// - `timeout_ms`: 超时时间（毫秒）,传0使用默认值
    ///
    /// # 返回值
    ///
    /// 返回 `SandboxResult` 包含执行结果和可选的TOML输出。
    ///
    /// # 错误
    ///
    /// - 临时目录创建失败
    /// - 文件写入失败
    /// - 执行超时
    /// - 不支持的语言
    ///
    /// # 示例
    ///
    /// ```rust,no_run
    /// # use std::path::PathBuf;
    /// # use ds4viz_server::sandbox::SandboxExecutor;
    /// # use uuid::Uuid;
    /// # async fn example() -> Result<(), Box<dyn std::error::Error>> {
    /// let executor = SandboxExecutor::new(PathBuf::from("/tmp/ds4viz"), 30000);
    ///
    /// let result = executor.execute(
    ///     Uuid::new_v4(),
    ///     "python",
    ///     "import ds4viz\nds4viz.stack().push(10)",
    ///     30000
    /// ).await?;
    ///
    /// if let Some(toml) = result.toml_output {
    ///     println!("Generated TOML: {}", toml);
    /// }
    /// # Ok(())
    /// # }
    /// ```
    pub async fn execute(
        &self,
        execution_id: Uuid,
        language_id: &str,
        code: &str,
        timeout_ms: u64,
    ) -> Result<SandboxResult, AppError> {
        // 使用默认超时或指定超时
        let timeout = if timeout_ms == 0 {
            self.default_timeout_ms
        } else {
            timeout_ms
        };

        // 1. 确保临时目录存在
        utils::ensure_temp_dir(&self.temp_dir)?;

        // 2. 准备文件
        let (code_file, toml_file) = self.prepare_files(execution_id, language_id, code)?;

        // 3. 构造执行命令
        let cmd = languages::build_command(language_id, &code_file, &toml_file)?;

        // 4. 通过 systemd 执行
        let result = systemd::execute_with_systemd(cmd, timeout).await?;

        // 5. 读取 TOML 输出
        let toml_output = utils::read_toml_if_exists(&toml_file)?;

        // 6. 清理临时文件
        utils::cleanup_files(&[code_file, toml_file])?;

        Ok(SandboxResult {
            stdout: result.stdout,
            stderr: result.stderr,
            exit_code: result.exit_code,
            cpu_time_ms: result.cpu_time_ms,
            toml_output,
        })
    }

    /// 准备临时文件
    ///
    /// 创建代码文件和TOML输出文件,并写入代码内容。
    ///
    /// # 参数
    ///
    /// - `execution_id`: 执行ID
    /// - `language_id`: 语言ID
    /// - `code`: 代码内容
    ///
    /// # 返回值
    ///
    /// 返回 `(代码文件路径, TOML文件路径)`
    fn prepare_files(
        &self,
        execution_id: Uuid,
        language_id: &str,
        code: &str,
    ) -> Result<(PathBuf, PathBuf), AppError> {
        // 获取文件扩展名
        let ext = match language_id {
            "python" => "py",
            "lua" => "lua",
            "rust" => "rs",
            _ => return Err(AppError::Internal(format!("不支持的语言: {}", language_id))),
        };

        // 生成文件路径
        let code_file = self
            .temp_dir
            .join(format!("exec_{}_{}.{}", execution_id, language_id, ext));
        let toml_file = self
            .temp_dir
            .join(format!("exec_{}_trace.toml", execution_id));

        // 写入代码
        fs::write(&code_file, code)
            .map_err(|e| AppError::Internal(format!("写入代码文件失败: {}", e)))?;

        Ok((code_file, toml_file))
    }
}

// ============================================================================
// 单元测试
// ============================================================================

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_sandbox_result_creation() {
        let result = SandboxResult {
            stdout: "test output".to_string(),
            stderr: String::new(),
            exit_code: 0,
            cpu_time_ms: 100,
            toml_output: None,
        };

        assert_eq!(result.stdout, "test output");
        assert_eq!(result.exit_code, 0);
        assert!(result.toml_output.is_none());
    }

    #[test]
    fn test_sandbox_executor_creation() {
        let executor = SandboxExecutor::new(PathBuf::from("/tmp/test"), 5000);
        assert_eq!(executor.default_timeout_ms, 5000);
    }

    #[tokio::test]
    #[ignore] // 需要 systemd --user 权限
    async fn test_execute_simple_python_code() {
        let executor = SandboxExecutor::new(PathBuf::from("/tmp/ds4viz_test"), 5000);
        let code = "print('hello from test')";

        let result = executor.execute(Uuid::new_v4(), "python", code, 5000).await;

        assert!(result.is_ok(), "Python执行应成功");
        let result = result.unwrap();
        assert_eq!(result.exit_code, 0);
        assert!(result.stdout.contains("hello from test"));
    }

    #[tokio::test]
    #[ignore] // 需要 systemd --user 权限
    async fn test_execute_simple_lua_code() {
        let executor = SandboxExecutor::new(PathBuf::from("/tmp/ds4viz_test"), 5000);
        let code = "print('hello from lua')";

        let result = executor.execute(Uuid::new_v4(), "lua", code, 5000).await;

        assert!(result.is_ok(), "Lua执行应成功");
        let result = result.unwrap();
        assert_eq!(result.exit_code, 0);
        assert!(result.stdout.contains("hello from lua"));
    }

    #[tokio::test]
    async fn test_execute_invalid_language() {
        let executor = SandboxExecutor::new(PathBuf::from("/tmp/ds4viz_test"), 5000);
        let code = "console.log('hello')";

        let result = executor
            .execute(Uuid::new_v4(), "javascript", code, 5000)
            .await;

        assert!(result.is_err(), "不支持的语言应返回错误");
    }

    #[tokio::test]
    #[ignore] // 需要 systemd --user 权限
    async fn test_execute_code_with_error() {
        let executor = SandboxExecutor::new(PathBuf::from("/tmp/ds4viz_test"), 5000);
        let code = "import nonexistent_module";

        let result = executor.execute(Uuid::new_v4(), "python", code, 5000).await;

        assert!(result.is_ok(), "执行应返回结果（即使代码有错）");
        let result = result.unwrap();
        assert_ne!(result.exit_code, 0, "错误代码应有非零退出码");
        assert!(!result.stderr.is_empty(), "应有错误输出");
    }
}
