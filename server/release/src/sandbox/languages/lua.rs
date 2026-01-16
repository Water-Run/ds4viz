//! Lua 语言支持
//!
//! 构造 Lua 代码的执行命令。
//!
//! 作者: WaterRun
//! 日期: 2026-01-16

use std::path::Path;

use crate::error::AppError;

/// 构造 Lua 执行命令
///
/// Lua 客户端库会自动生成 `trace.toml` 文件到配置的输出路径。
///
/// # 参数
///
/// - `code_file`: 代码文件路径
/// - `_toml_file`: TOML 输出路径（Lua 库自动处理,此参数未使用）
///
/// # 返回值
///
/// 返回命令及参数列表,例如 `["lua", "/tmp/ds4viz/exec_xxx.lua"]`
///
/// # 示例
///
/// ```rust,no_run
/// use std::path::Path;
/// use ds4viz_server::sandbox::languages::lua::build_command;
///
/// # fn example() -> Result<(), Box<dyn std::error::Error>> {
/// let code_file = Path::new("/tmp/ds4viz/exec_123.lua");
/// let toml_file = Path::new("/tmp/ds4viz/exec_123_trace.toml");
/// let cmd = build_command(code_file, toml_file)?;
/// assert_eq!(cmd[0], "lua");
/// # Ok(())
/// # }
/// ```
pub fn build_command(code_file: &Path, _toml_file: &Path) -> Result<Vec<String>, AppError> {
    Ok(vec![
        "lua".to_string(),
        code_file.to_string_lossy().to_string(),
    ])
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_build_command_返回正确格式() {
        let code_file = Path::new("/tmp/test.lua");
        let toml_file = Path::new("/tmp/trace.toml");

        let cmd = build_command(code_file, toml_file).unwrap();

        assert_eq!(cmd.len(), 2);
        assert_eq!(cmd[0], "lua");
        assert_eq!(cmd[1], "/tmp/test.lua");
    }
}
