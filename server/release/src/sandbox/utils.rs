//! 沙箱工具函数
//!
//! 提供临时文件管理、清理等辅助功能。
//!
//! 作者: WaterRun
//! 日期: 2026-01-16

use std::fs;
use std::path::Path;

use crate::error::AppError;

/// 读取 TOML 文件内容（如果存在）
///
/// 用于读取客户端库生成的 `trace.toml` 文件。
///
/// # 参数
///
/// - `path`: TOML 文件路径
///
/// # 返回值
///
/// - 文件存在时返回 `Some(String)` 包含文件内容
/// - 文件不存在时返回 `None`
/// - 读取失败时返回 `AppError`
///
/// # 示例
///
/// ```rust,no_run
/// use std::path::Path;
/// use ds4viz_server::sandbox::utils::read_toml_if_exists;
///
/// # async fn example() -> Result<(), Box<dyn std::error::Error>> {
/// let path = Path::new("/tmp/ds4viz/trace.toml");
/// if let Some(content) = read_toml_if_exists(path)? {
///     println!("TOML content: {}", content);
/// }
/// # Ok(())
/// # }
/// ```
pub fn read_toml_if_exists(path: &Path) -> Result<Option<String>, AppError> {
    if path.exists() {
        fs::read_to_string(path)
            .map(Some)
            .map_err(|e| AppError::Internal(format!("读取TOML文件失败: {}", e)))
    } else {
        Ok(None)
    }
}

/// 清理临时文件
///
/// 删除指定的文件列表，忽略不存在的文件。
///
/// # 参数
///
/// - `paths`: 文件路径列表
///
/// # 示例
///
/// ```rust,no_run
/// use std::path::Path;
/// use ds4viz_server::sandbox::utils::cleanup_files;
///
/// # fn example() -> Result<(), Box<dyn std::error::Error>> {
/// let files = vec![
///     Path::new("/tmp/ds4viz/code.py"),
///     Path::new("/tmp/ds4viz/trace.toml"),
/// ];
/// cleanup_files(&files)?;
/// # Ok(())
/// # }
/// ```
pub fn cleanup_files(paths: &[impl AsRef<Path>]) -> Result<(), AppError> {
    for path in paths {
        if path.as_ref().exists() {
            let _ = fs::remove_file(path);
        }
    }
    Ok(())
}

/// 确保临时目录存在
///
/// 如果目录不存在则创建，包括所有必要的父目录。
///
/// # 参数
///
/// - `dir`: 目录路径
///
/// # 示例
///
/// ```rust,no_run
/// use std::path::Path;
/// use ds4viz_server::sandbox::utils::ensure_temp_dir;
///
/// # fn example() -> Result<(), Box<dyn std::error::Error>> {
/// ensure_temp_dir(Path::new("/tmp/ds4viz"))?;
/// # Ok(())
/// # }
/// ```
pub fn ensure_temp_dir(dir: &Path) -> Result<(), AppError> {
    fs::create_dir_all(dir).map_err(|e| AppError::Internal(format!("创建临时目录失败: {}", e)))
}

// ============================================================================
// 单元测试
// ============================================================================

#[cfg(test)]
mod tests {
    use super::*;
    use std::io::Write;

    #[test]
    fn test_read_toml_if_exists_当文件存在时() {
        let temp_dir = std::env::temp_dir();
        let test_file = temp_dir.join("test_toml.toml");

        // 创建测试文件
        let mut file = fs::File::create(&test_file).unwrap();
        file.write_all(b"[test]\nvalue = 42").unwrap();

        // 读取文件
        let content = read_toml_if_exists(&test_file).unwrap();
        assert!(content.is_some(), "应返回文件内容");
        assert!(content.unwrap().contains("value = 42"));

        // 清理
        fs::remove_file(&test_file).unwrap();
    }

    #[test]
    fn test_read_toml_if_exists_当文件不存在时() {
        let test_file = std::env::temp_dir().join("nonexistent_file.toml");
        let content = read_toml_if_exists(&test_file).unwrap();
        assert!(content.is_none(), "不存在的文件应返回 None");
    }

    #[test]
    fn test_cleanup_files_成功清理() {
        let temp_dir = std::env::temp_dir();
        let test_file1 = temp_dir.join("cleanup_test1.txt");
        let test_file2 = temp_dir.join("cleanup_test2.txt");

        // 创建测试文件
        fs::File::create(&test_file1).unwrap();
        fs::File::create(&test_file2).unwrap();

        assert!(test_file1.exists());
        assert!(test_file2.exists());

        // 清理
        cleanup_files(&[&test_file1, &test_file2]).unwrap();

        assert!(!test_file1.exists());
        assert!(!test_file2.exists());
    }

    #[test]
    fn test_cleanup_files_忽略不存在的文件() {
        let nonexistent = std::env::temp_dir().join("nonexistent.txt");
        let result = cleanup_files(&[&nonexistent]);
        assert!(result.is_ok(), "清理不存在的文件应成功");
    }

    #[test]
    fn test_ensure_temp_dir_创建目录() {
        let temp_dir = std::env::temp_dir().join("ds4viz_test_dir");

        // 确保目录不存在
        let _ = fs::remove_dir_all(&temp_dir);

        ensure_temp_dir(&temp_dir).unwrap();
        assert!(temp_dir.exists(), "目录应被创建");
        assert!(temp_dir.is_dir(), "应该是一个目录");

        // 清理
        fs::remove_dir_all(&temp_dir).unwrap();
    }

    #[test]
    fn test_ensure_temp_dir_已存在时不报错() {
        let temp_dir = std::env::temp_dir();
        let result = ensure_temp_dir(&temp_dir);
        assert!(result.is_ok(), "已存在的目录应成功");
    }
}
