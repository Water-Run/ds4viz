//! 配置加载与管理模块
//!
//! 本模块负责从配置文件加载应用配置，支持默认值与自定义配置文件路径。
//!
//! # 配置文件格式
//!
//! 使用 TOML 格式，完整配置结构参见 [`Config`]。
//!
//! # 限流说明
//!
//! [`RateLimitConfig`] 定义了各类接口的限流参数，限流中间件的实际实现
//! 将在后续迭代中完成。当前仅定义配置结构。
//!
//! 作者: WaterRun
//! 日期: 2026-01-16

use serde::Deserialize;
use std::path::Path;

use crate::error::{AppError, ConfigErrorKind};

/// 应用完整配置
///
/// 包含服务器、数据库、JWT、配额、执行和限流等所有配置项。
///
/// # 示例
///
/// ```rust,no_run
/// use ds4viz_server::config::Config;
///
/// let config = Config::from_file("configs/default.toml").unwrap();
/// assert_eq!(config.server.port, 8080);
/// ```
#[derive(Debug, Clone, Deserialize)]
pub struct Config {
    /// 服务器配置
    pub server: ServerConfig,
    /// 数据库配置
    pub database: DatabaseConfig,
    /// JWT 配置
    pub jwt: JwtConfig,
    /// 配额配置
    pub quota: QuotaConfig,
    /// 执行配置
    pub execution: ExecutionConfig,
    /// 限流配置
    pub rate_limit: RateLimitConfig,
}

/// 服务器配置
///
/// # 示例
///
/// ```
/// use ds4viz_server::config::ServerConfig;
///
/// let config = ServerConfig::default();
/// assert_eq!(config.host, "127.0.0.1");
/// assert_eq!(config.port, 8080);
/// ```
#[derive(Debug, Clone, Deserialize)]
pub struct ServerConfig {
    /// 监听地址，默认 "127.0.0.1"
    #[serde(default = "default_host")]
    pub host: String,
    /// 监听端口，默认 8080
    #[serde(default = "default_port")]
    pub port: u16,
}

/// 数据库配置
///
/// # 示例
///
/// ```
/// use ds4viz_server::config::DatabaseConfig;
///
/// let config = DatabaseConfig::default();
/// assert_eq!(config.max_connections, 10);
/// ```
#[derive(Debug, Clone, Deserialize)]
pub struct DatabaseConfig {
    /// PostgreSQL 连接字符串
    pub url: String,
    /// 连接池最大连接数，默认 10
    #[serde(default = "default_max_connections")]
    pub max_connections: u32,
}

/// JWT 配置
///
/// # 示例
///
/// ```
/// use ds4viz_server::config::JwtConfig;
///
/// let config = JwtConfig {
///     secret: "test_secret".to_string(),
///     access_token_ttl_secs: 3600,
///     refresh_token_ttl_secs: 604800,
/// };
/// assert_eq!(config.access_token_ttl_secs, 3600);
/// ```
#[derive(Debug, Clone, Deserialize)]
pub struct JwtConfig {
    /// JWT 签名密钥
    pub secret: String,
    /// Access Token 有效期（秒），默认 3600（1小时）
    #[serde(default = "default_access_token_ttl")]
    pub access_token_ttl_secs: u64,
    /// Refresh Token 有效期（秒），默认 604800（7天）
    #[serde(default = "default_refresh_token_ttl")]
    pub refresh_token_ttl_secs: u64,
}

/// 配额配置
///
/// 定义 Standard 用户的资源限制参数。Enhanced 用户无限制。
///
/// # 示例
///
/// ```
/// use ds4viz_server::config::QuotaConfig;
///
/// let config = QuotaConfig::default();
/// assert_eq!(config.period_minutes, 60);
/// assert_eq!(config.standard_cpu_time_ms, 10000);
/// ```
#[derive(Debug, Clone, Deserialize)]
pub struct QuotaConfig {
    /// 配额周期（分钟），默认 60
    #[serde(default = "default_period_minutes")]
    pub period_minutes: u64,
    /// Standard 用户每周期可用 CPU 时间（毫秒），默认 10000
    #[serde(default = "default_standard_cpu_time_ms")]
    pub standard_cpu_time_ms: u64,
    /// Standard 用户单次提交代码最大字节数，默认 16384（16KB）
    #[serde(default = "default_standard_max_code_bytes")]
    pub standard_max_code_bytes: usize,
}

/// 执行配置
///
/// # 示例
///
/// ```
/// use ds4viz_server::config::ExecutionConfig;
///
/// let config = ExecutionConfig::default();
/// assert_eq!(config.default_timeout_ms, 30000);
/// ```
#[derive(Debug, Clone, Deserialize)]
pub struct ExecutionConfig {
    /// 默认执行超时时间（毫秒），默认 30000
    #[serde(default = "default_timeout_ms")]
    pub default_timeout_ms: u64,
    /// 临时文件目录，默认 "/tmp/ds4viz"
    #[serde(default = "default_temp_dir")]
    pub temp_dir: String,
}

/// 限流配置
///
/// 定义各类接口的请求频率限制。限流中间件实现将在后续迭代中完成。
///
/// # 示例
///
/// ```
/// use ds4viz_server::config::RateLimitConfig;
///
/// let config = RateLimitConfig::default();
/// assert_eq!(config.auth_per_minute, 5);
/// ```
#[derive(Debug, Clone, Deserialize)]
pub struct RateLimitConfig {
    /// 认证接口每分钟请求限制，默认 5
    #[serde(default = "default_auth_per_minute")]
    pub auth_per_minute: u32,
    /// Standard 用户执行接口每分钟请求限制，默认 10
    #[serde(default = "default_execution_standard_per_minute")]
    pub execution_standard_per_minute: u32,
    /// Enhanced 用户执行接口每分钟请求限制，默认 60
    #[serde(default = "default_execution_enhanced_per_minute")]
    pub execution_enhanced_per_minute: u32,
    /// 通用接口每分钟请求限制，默认 100
    #[serde(default = "default_general_per_minute")]
    pub general_per_minute: u32,
}

impl Config {
    /// 从指定路径加载配置文件
    ///
    /// # 参数
    ///
    /// - `path`: 配置文件路径
    ///
    /// # 返回值
    ///
    /// 成功返回 [`Config`]，失败返回 [`AppError`]。
    ///
    /// # 错误
    ///
    /// - 文件不存在或无法读取时返回 [`ConfigErrorKind::FileNotFound`]
    /// - 配置格式错误时返回 [`ConfigErrorKind::ParseError`]
    ///
    /// # 示例
    ///
    /// ```rust,no_run
    /// use ds4viz_server::config::Config;
    ///
    /// let config = Config::from_file("configs/default.toml").unwrap();
    /// ```
    pub fn from_file<P: AsRef<Path>>(path: P) -> Result<Self, AppError> {
        let path = path.as_ref();

        let content = std::fs::read_to_string(path).map_err(|e| {
            AppError::Config(ConfigErrorKind::FileNotFound {
                path: format!("{}: {}", path.display(), e),
            })
        })?;

        toml::from_str(&content).map_err(|e| {
            AppError::Config(ConfigErrorKind::ParseError {
                message: e.to_string(),
            })
        })
    }
}

impl Default for ServerConfig {
    fn default() -> Self {
        Self {
            host: default_host(),
            port: default_port(),
        }
    }
}

impl Default for DatabaseConfig {
    fn default() -> Self {
        Self {
            url: String::new(),
            max_connections: default_max_connections(),
        }
    }
}

impl Default for QuotaConfig {
    fn default() -> Self {
        Self {
            period_minutes: default_period_minutes(),
            standard_cpu_time_ms: default_standard_cpu_time_ms(),
            standard_max_code_bytes: default_standard_max_code_bytes(),
        }
    }
}

impl Default for ExecutionConfig {
    fn default() -> Self {
        Self {
            default_timeout_ms: default_timeout_ms(),
            temp_dir: default_temp_dir(),
        }
    }
}

impl Default for RateLimitConfig {
    fn default() -> Self {
        Self {
            auth_per_minute: default_auth_per_minute(),
            execution_standard_per_minute: default_execution_standard_per_minute(),
            execution_enhanced_per_minute: default_execution_enhanced_per_minute(),
            general_per_minute: default_general_per_minute(),
        }
    }
}

fn default_host() -> String {
    "127.0.0.1".to_string()
}

fn default_port() -> u16 {
    8080
}

fn default_max_connections() -> u32 {
    10
}

fn default_access_token_ttl() -> u64 {
    3600
}

fn default_refresh_token_ttl() -> u64 {
    604800
}

fn default_period_minutes() -> u64 {
    60
}

fn default_standard_cpu_time_ms() -> u64 {
    10000
}

fn default_standard_max_code_bytes() -> usize {
    16384
}

fn default_timeout_ms() -> u64 {
    30000
}

fn default_temp_dir() -> String {
    "/tmp/ds4viz".to_string()
}

fn default_auth_per_minute() -> u32 {
    5
}

fn default_execution_standard_per_minute() -> u32 {
    10
}

fn default_execution_enhanced_per_minute() -> u32 {
    60
}

fn default_general_per_minute() -> u32 {
    100
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::io::Write;

    #[test]
    fn test_config_from_valid_file() {
        let temp_dir = std::env::temp_dir();
        let config_path = temp_dir.join("test_config_valid.toml");

        let config_content = r#"
[server]
host = "0.0.0.0"
port = 9000

[database]
url = "postgres://localhost/test"
max_connections = 20

[jwt]
secret = "test_secret_key"
access_token_ttl_secs = 7200
refresh_token_ttl_secs = 1209600

[quota]
period_minutes = 30
standard_cpu_time_ms = 5000
standard_max_code_bytes = 8192

[execution]
default_timeout_ms = 60000
temp_dir = "/var/tmp/ds4viz"

[rate_limit]
auth_per_minute = 10
execution_standard_per_minute = 20
execution_enhanced_per_minute = 120
general_per_minute = 200
"#;

        {
            let mut file = std::fs::File::create(&config_path).expect("创建测试配置文件失败");
            file.write_all(config_content.as_bytes())
                .expect("写入测试配置文件失败");
        }

        let config = Config::from_file(&config_path).expect("加载配置文件失败");

        assert_eq!(config.server.host, "0.0.0.0", "服务器地址应为 0.0.0.0");
        assert_eq!(config.server.port, 9000, "服务器端口应为 9000");
        assert_eq!(
            config.database.url, "postgres://localhost/test",
            "数据库 URL 不匹配"
        );
        assert_eq!(
            config.database.max_connections, 20,
            "数据库最大连接数应为 20"
        );
        assert_eq!(config.jwt.secret, "test_secret_key", "JWT 密钥不匹配");
        assert_eq!(
            config.jwt.access_token_ttl_secs, 7200,
            "Access Token TTL 应为 7200"
        );
        assert_eq!(config.quota.period_minutes, 30, "配额周期应为 30 分钟");
        assert_eq!(
            config.quota.standard_cpu_time_ms, 5000,
            "CPU 时间限制应为 5000ms"
        );
        assert_eq!(
            config.execution.default_timeout_ms, 60000,
            "默认超时应为 60000ms"
        );
        assert_eq!(
            config.rate_limit.auth_per_minute, 10,
            "认证限流应为 10/分钟"
        );

        let _ = std::fs::remove_file(&config_path);
    }

    #[test]
    fn test_config_file_not_found() {
        let result = Config::from_file("/nonexistent/path/config.toml");

        assert!(result.is_err(), "应返回错误");
        match result.unwrap_err() {
            AppError::Config(ConfigErrorKind::FileNotFound { path, .. }) => {
                assert!(path.contains("nonexistent"), "错误信息应包含文件路径");
            }
            other => panic!("期望 FileNotFound 错误，实际为: {:?}", other),
        }
    }

    #[test]
    fn test_config_parse_error() {
        let temp_dir = std::env::temp_dir();
        let config_path = temp_dir.join("test_config_invalid.toml");

        {
            let mut file = std::fs::File::create(&config_path).expect("创建测试配置文件失败");
            file.write_all(b"invalid toml content [[[")
                .expect("写入测试配置文件失败");
        }

        let result = Config::from_file(&config_path);

        assert!(result.is_err(), "应返回解析错误");
        match result.unwrap_err() {
            AppError::Config(ConfigErrorKind::ParseError { .. }) => {}
            other => panic!("期望 ParseError 错误，实际为: {:?}", other),
        }

        let _ = std::fs::remove_file(&config_path);
    }

    #[test]
    fn test_config_with_defaults() {
        let temp_dir = std::env::temp_dir();
        let config_path = temp_dir.join("test_config_minimal.toml");

        let config_content = r#"
[server]

[database]
url = "postgres://localhost/test"

[jwt]
secret = "minimal_secret"

[quota]

[execution]

[rate_limit]
"#;

        {
            let mut file = std::fs::File::create(&config_path).expect("创建测试配置文件失败");
            file.write_all(config_content.as_bytes())
                .expect("写入测试配置文件失败");
        }

        let config = Config::from_file(&config_path).expect("加载配置文件失败");

        assert_eq!(
            config.server.host, "127.0.0.1",
            "服务器地址默认值应为 127.0.0.1"
        );
        assert_eq!(config.server.port, 8080, "服务器端口默认值应为 8080");
        assert_eq!(
            config.database.max_connections, 10,
            "数据库最大连接数默认值应为 10"
        );
        assert_eq!(
            config.jwt.access_token_ttl_secs, 3600,
            "Access Token TTL 默认值应为 3600"
        );
        assert_eq!(
            config.quota.period_minutes, 60,
            "配额周期默认值应为 60 分钟"
        );
        assert_eq!(
            config.execution.default_timeout_ms, 30000,
            "默认超时默认值应为 30000ms"
        );
        assert_eq!(
            config.rate_limit.auth_per_minute, 5,
            "认证限流默认值应为 5/分钟"
        );

        let _ = std::fs::remove_file(&config_path);
    }

    #[test]
    fn test_server_config_default() {
        let config = ServerConfig::default();

        assert_eq!(config.host, "127.0.0.1");
        assert_eq!(config.port, 8080);
    }

    #[test]
    fn test_quota_config_default() {
        let config = QuotaConfig::default();

        assert_eq!(config.period_minutes, 60);
        assert_eq!(config.standard_cpu_time_ms, 10000);
        assert_eq!(config.standard_max_code_bytes, 16384);
    }

    #[test]
    fn test_execution_config_default() {
        let config = ExecutionConfig::default();

        assert_eq!(config.default_timeout_ms, 30000);
        assert_eq!(config.temp_dir, "/tmp/ds4viz");
    }

    #[test]
    fn test_rate_limit_config_default() {
        let config = RateLimitConfig::default();

        assert_eq!(config.auth_per_minute, 5);
        assert_eq!(config.execution_standard_per_minute, 10);
        assert_eq!(config.execution_enhanced_per_minute, 60);
        assert_eq!(config.general_per_minute, 100);
    }
}
