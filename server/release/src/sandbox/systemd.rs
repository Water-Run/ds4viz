//! systemd-run 封装
//!
//! 使用 systemd cgroup 进行资源隔离和限制,确保代码执行的安全性。
//!
//! 作者: WaterRun
//! 日期: 2026-01-16

use std::time::Instant;
use tokio::process::Command;

use crate::error::AppError;

/// systemd 执行结果
///
/// # 示例
///
/// ```
/// use ds4viz_server::sandbox::systemd::SystemdResult;
///
/// let result = SystemdResult {
///     stdout: "hello\n".to_string(),
///     stderr: String::new(),
///     exit_code: 0,
///     cpu_time_ms: 123,
/// };
///
/// assert_eq!(result.exit_code, 0);
/// ```
#[derive(Debug, Clone)]
pub struct SystemdResult {
    /// 标准输出
    pub stdout: String,
    /// 标准错误
    pub stderr: String,
    /// 退出码
    pub exit_code: i32,
    /// CPU耗时（毫秒）
    pub cpu_time_ms: i64,
}

/// 使用 systemd-run 执行命令
///
/// 通过 systemd cgroup 实现以下安全措施:
/// - CPU配额限制: 100%
/// - 内存限制: 256MB
/// - 进程数限制: 64
/// - 隔离临时目录
/// - 禁止提权
/// - 只读系统目录
/// - 禁止访问 /home
///
/// # 参数
///
/// - `cmd`: 要执行的命令及参数（例如 `["python3", "/tmp/code.py"]`）
/// - `timeout_ms`: 超时时间（毫秒）
///
/// # 返回值
///
/// 返回 `SystemdResult` 包含执行结果。
///
/// # 错误
///
/// - systemd-run 命令不存在
/// - 执行超时
/// - 系统权限不足
///
/// # 示例
///
/// ```rust,no_run
/// use ds4viz_server::sandbox::systemd::execute_with_systemd;
///
/// # async fn example() -> Result<(), Box<dyn std::error::Error>> {
/// let cmd = vec!["python3".to_string(), "-c".to_string(), "print('hello')".to_string()];
/// let result = execute_with_systemd(cmd, 30000).await?;
/// println!("Output: {}", result.stdout);
/// println!("Exit code: {}", result.exit_code);
/// # Ok(())
/// # }
/// ```
pub async fn execute_with_systemd(
    cmd: Vec<String>,
    timeout_ms: u64,
) -> Result<SystemdResult, AppError> {
    if cmd.is_empty() {
        return Err(AppError::Internal("执行命令不能为空".to_string()));
    }

    let timeout_secs = (timeout_ms / 1000).max(1); // 至少1秒

    // 构造 systemd-run 命令
    let mut systemd_cmd = Command::new("systemd-run");
    systemd_cmd
        .arg("--user") // 用户级 systemd
        .arg("--scope") // 临时作用域
        .arg("--quiet") // 不输出 systemd 自己的日志
        // 资源限制
        .arg("--property=CPUQuota=100%") // CPU 限制为 100%
        .arg("--property=MemoryMax=256M") // 内存限制 256MB
        .arg("--property=TasksMax=64") // 进程数限制 64
        // 安全隔离
        .arg("--property=PrivateTmp=yes") // 隔离临时目录
        .arg("--property=NoNewPrivileges=yes") // 禁止提权
        .arg("--property=ProtectSystem=strict") // 只读系统目录
        .arg("--property=ProtectHome=yes") // 禁止访问 /home
        .arg("--property=ReadWritePaths=/tmp/ds4viz") // 仅允许写入沙箱目录
        // 超时控制
        .arg("--")
        .arg("timeout")
        .arg(format!("{}s", timeout_secs))
        .args(&cmd);

    // 执行并计时
    let start = Instant::now();
    let output = systemd_cmd.output().await.map_err(|e| {
        AppError::Internal(format!(
            "systemd-run 执行失败: {}。请确保systemd已安装并且有权限使用--user模式",
            e
        ))
    })?;
    let elapsed = start.elapsed().as_millis() as i64;

    // timeout 命令返回值:
    // - 0: 命令成功完成
    // - 124: 超时
    // - 其他: 命令自身的退出码
    let exit_code = output.status.code().unwrap_or(-1);

    Ok(SystemdResult {
        stdout: String::from_utf8_lossy(&output.stdout).to_string(),
        stderr: String::from_utf8_lossy(&output.stderr).to_string(),
        exit_code,
        cpu_time_ms: elapsed,
    })
}

// ============================================================================
// 单元测试
// ============================================================================

#[cfg(test)]
mod tests {
    use super::*;

    #[tokio::test]
    #[ignore] // 需要 systemd --user 权限,CI环境可能不可用
    async fn test_execute_with_systemd_成功执行() {
        let cmd = vec!["echo".to_string(), "hello world".to_string()];

        let result = execute_with_systemd(cmd, 5000).await.unwrap();

        assert_eq!(result.exit_code, 0, "退出码应为0");
        assert!(
            result.stdout.contains("hello world"),
            "输出应包含 'hello world'"
        );
    }

    #[tokio::test]
    #[ignore] // 需要 systemd --user 权限
    async fn test_execute_with_systemd_超时控制() {
        let cmd = vec!["sleep".to_string(), "10".to_string()];

        let result = execute_with_systemd(cmd, 2000).await.unwrap();

        // timeout 命令超时时返回 124
        assert_eq!(result.exit_code, 124, "超时应返回退出码124");
    }

    #[tokio::test]
    #[ignore] // 需要 systemd --user 权限
    async fn test_execute_with_systemd_错误退出码() {
        let cmd = vec!["sh".to_string(), "-c".to_string(), "exit 42".to_string()];

        let result = execute_with_systemd(cmd, 5000).await.unwrap();

        assert_eq!(result.exit_code, 42, "应返回命令的退出码");
    }

    #[tokio::test]
    async fn test_execute_with_systemd_空命令() {
        let cmd = vec![];

        let result = execute_with_systemd(cmd, 5000).await;

        assert!(result.is_err(), "空命令应返回错误");
    }

    #[tokio::test]
    #[ignore] // 需要 systemd --user 权限
    async fn test_execute_with_systemd_捕获stderr() {
        let cmd = vec![
            "sh".to_string(),
            "-c".to_string(),
            "echo error >&2".to_string(),
        ];

        let result = execute_with_systemd(cmd, 5000).await.unwrap();

        assert!(result.stderr.contains("error"), "stderr应包含错误信息");
    }

    #[tokio::test]
    #[ignore] // 需要 systemd --user 权限
    async fn test_execute_with_systemd_cpu时间记录() {
        let cmd = vec!["echo".to_string(), "test".to_string()];

        let result = execute_with_systemd(cmd, 5000).await.unwrap();

        assert!(result.cpu_time_ms > 0, "CPU时间应大于0");
        assert!(result.cpu_time_ms < 5000, "CPU时间应小于超时时间");
    }
}
