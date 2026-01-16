//! Rust 语言支持
//!
//! 构造 Rust 代码的编译和执行命令。
//!
//! # 注意
//!
//! Rust 是编译型语言,执行时间包含编译时间,可能较长(10-60秒)。
//! Standard 用户默认不允许使用 Rust。
//!
//! 作者: WaterRun
//! 日期: 2026-01-16

use std::path::Path;

use crate::error::AppError;

/// 构造 Rust 执行命令
///
/// 使用 `rustc` 编译代码并执行生成的二进制文件。
///
/// # 参数
///
/// - `code_file`: 代码文件路径
/// - `_toml_file`: TOML 输出路径（Rust 库自动处理）
///
/// # 返回值
///
/// 返回命令及参数列表,例如:
/// ```text
/// ["sh", "-c", "rustc /tmp/exec_123.rs -o /tmp/exec_123 && /tmp/exec_123"]
/// ```
///
/// # 示例
///
/// ```rust,no_run
/// use std::path::Path;
/// use ds4viz_server::sandbox::languages::rust::build_command;
///
/// # fn example() -> Result<(), Box<dyn std::error::Error>> {
/// let code_file = Path::new("/tmp/ds4viz/exec_123.rs");
/// let toml_file = Path::new("/tmp/ds4viz/exec_123_trace.toml");
/// let cmd = build_command(code_file, toml_file)?;
/// assert_eq!(cmd[0], "sh");
/// # Ok(())
/// # }
/// ```
pub fn build_command(code_file: &Path, _toml_file: &Path) -> Result<Vec<String>, AppError> {
    // 生成二进制文件路径（去掉.rs扩展名）
    let binary_path = code_file.with_extension("");

    // 构造编译+执行的 shell 命令
    let shell_cmd = format!(
        "rustc {} -o {} && {}",
        code_file.to_string_lossy(),
        binary_path.to_string_lossy(),
        binary_path.to_string_lossy()
    );

    Ok(vec!["sh".to_string(), "-c".to_string(), shell_cmd])
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_build_command_返回正确格式() {
        let code_file = Path::new("/tmp/test.rs");
        let toml_file = Path::new("/tmp/trace.toml");

        let cmd = build_command(code_file, toml_file).unwrap();

        assert_eq!(cmd.len(), 3);
        assert_eq!(cmd[0], "sh");
        assert_eq!(cmd[1], "-c");
        assert!(cmd[2].contains("rustc"));
        assert!(cmd[2].contains("&&"));
    }

    #[test]
    fn test_build_command_二进制路径正确() {
        let code_file = Path::new("/tmp/ds4viz/exec_abc.rs");
        let toml_file = Path::new("/tmp/trace.toml");

        let cmd = build_command(code_file, toml_file).unwrap();

        assert!(cmd[2].contains("/tmp/ds4viz/exec_abc"));
        assert!(!cmd[2].contains(".rs &&")); // 二进制文件不应有.rs扩展名
    }
}
