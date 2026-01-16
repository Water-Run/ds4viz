//! 语言特定的执行逻辑
//!
//! 为每种支持的编程语言提供执行命令构造器。
//!
//! 作者: WaterRun
//! 日期: 2026-01-16

use std::path::Path;

use crate::error::{AppError, InvalidRequestKind};

pub mod lua;
pub mod python;
pub mod rust;

/// 构造语言特定的执行命令
///
/// 根据语言ID选择对应的命令构造器。
///
/// # 参数
///
/// - `language_id`: 语言标识符（python, lua, rust）
/// - `code_file`: 代码文件路径
/// - `toml_file`: TOML 输出文件路径
///
/// # 返回值
///
/// 返回命令及参数列表,可直接传递给 `systemd::execute_with_systemd`。
///
/// # 错误
///
/// 不支持的语言ID会返回 `InvalidRequestKind::InvalidLanguage`。
///
/// # 示例
///
/// ```rust,no_run
/// use std::path::Path;
/// use ds4viz_server::sandbox::languages::build_command;
///
/// # fn example() -> Result<(), Box<dyn std::error::Error>> {
/// let code_file = Path::new("/tmp/ds4viz/exec_123.py");
/// let toml_file = Path::new("/tmp/ds4viz/exec_123_trace.toml");
///
/// let cmd = build_command("python", code_file, toml_file)?;
/// assert_eq!(cmd[0], "python3");
/// # Ok(())
/// # }
/// ```
pub fn build_command(
    language_id: &str,
    code_file: &Path,
    toml_file: &Path,
) -> Result<Vec<String>, AppError> {
    match language_id {
        "python" => python::build_command(code_file, toml_file),
        "lua" => lua::build_command(code_file, toml_file),
        "rust" => rust::build_command(code_file, toml_file),
        _ => Err(AppError::InvalidRequest(
            InvalidRequestKind::InvalidLanguage {
                language_id: language_id.to_string(),
            },
        )),
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_build_command_python() {
        let cmd = build_command(
            "python",
            Path::new("/tmp/test.py"),
            Path::new("/tmp/trace.toml"),
        )
        .unwrap();

        assert_eq!(cmd[0], "python3");
    }

    #[test]
    fn test_build_command_lua() {
        let cmd = build_command(
            "lua",
            Path::new("/tmp/test.lua"),
            Path::new("/tmp/trace.toml"),
        )
        .unwrap();

        assert_eq!(cmd[0], "lua");
    }

    #[test]
    fn test_build_command_rust() {
        let cmd = build_command(
            "rust",
            Path::new("/tmp/test.rs"),
            Path::new("/tmp/trace.toml"),
        )
        .unwrap();

        assert_eq!(cmd[0], "sh");
    }

    #[test]
    fn test_build_command_invalid_language() {
        let result = build_command(
            "javascript",
            Path::new("/tmp/test.js"),
            Path::new("/tmp/trace.toml"),
        );

        assert!(result.is_err(), "不支持的语言应返回错误");
    }
}
