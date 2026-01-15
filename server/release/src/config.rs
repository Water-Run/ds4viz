//! 配置模块
//!
//! 负责加载和管理服务器配置。支持 TOML 文件配置，环境变量可覆盖。

use std::env;
use std::fs;
use std::net::SocketAddr;
use std::path::Path;

use serde::Deserialize;

use crate::error::ConfigError;

/// 服务器完整配置
#[derive(Debug, Clone, Deserialize)]
pub struct Config {
    /// 数据库配置
    pub database: DatabaseConfig,
    /// 服务器配置
    pub server: ServerConfig,
    /// JWT 认证配置
    pub jwt: JwtConfig,
    /// 配额配置
    pub quota: QuotaConfig,
    /// 执行配置
    pub execution: ExecutionConfig,
}

/// 数据库配置
#[derive(Debug, Clone, Deserialize)]
pub struct DatabaseConfig {
    /// PostgreSQL 连接 URL
    pub url: String,
    /// 连接池最大连接数
    #[serde(default = "default_max_connections")]
    pub max_connections: u32,
    /// 连接池最小连接数
    #[serde(default = "default_min_connections")]
    pub min_connections: u32,
}

/// 服务器配置
#[derive(Debug, Clone, Deserialize)]
pub struct ServerConfig {
    /// 监听地址
    #[serde(default = "default_listen")]
    pub listen: SocketAddr,
}

/// JWT 认证配置
#[derive(Debug, Clone, Deserialize)]
pub struct JwtConfig {
    /// 签名密钥
    pub secret: String,
    /// Access Token 过期时间（秒）
    #[serde(default = "default_access_token_expires")]
    pub access_token_expires_secs: u64,
    /// Refresh Token 过期时间（秒）
    #[serde(default = "default_refresh_token_expires")]
    pub refresh_token_expires_secs: u64,
}

/// 配额配置
#[derive(Debug, Clone, Deserialize)]
pub struct QuotaConfig {
    /// 配额刷新周期（分钟）
    #[serde(default = "default_quota_period_minutes")]
    pub period_minutes: u32,
    /// Standard 用户 CPU 时间限制（毫秒）
    #[serde(default = "default_quota_cpu_time_ms")]
    pub cpu_time_ms: u64,
    /// Standard 用户代码大小限制（字节）
    #[serde(default = "default_max_code_size")]
    pub max_code_size_bytes: usize,
}

/// 执行配置
#[derive(Debug, Clone, Deserialize)]
pub struct ExecutionConfig {
    /// 最大并发执行数
    #[serde(default = "default_max_concurrent_executions")]
    pub max_concurrent: usize,
    /// 临时文件目录
    #[serde(default = "default_temp_dir")]
    pub temp_dir: String,
}

const fn default_max_connections() -> u32 {
    10
}

const fn default_min_connections() -> u32 {
    2
}

fn default_listen() -> SocketAddr {
    "127.0.0.1:3000".parse().unwrap()
}

const fn default_access_token_expires() -> u64 {
    3600
}

const fn default_refresh_token_expires() -> u64 {
    604800
}

const fn default_quota_period_minutes() -> u32 {
    60
}

const fn default_quota_cpu_time_ms() -> u64 {
    10000
}

const fn default_max_code_size() -> usize {
    16384
}

const fn default_max_concurrent_executions() -> usize {
    10
}

fn default_temp_dir() -> String {
    "/tmp/ds4viz".to_owned()
}

impl Config {
    /// 从 TOML 文件加载配置
    ///
    /// 环境变量可覆盖以下配置：
    /// - `DATABASE_URL`: 数据库连接 URL
    /// - `JWT_SECRET`: JWT 签名密钥
    /// - `LISTEN_ADDR`: 服务器监听地址
    pub fn load<P: AsRef<Path>>(path: P) -> Result<Self, ConfigError> {
        let content = fs::read_to_string(path.as_ref()).map_err(|e| ConfigError::FileRead {
            path: path.as_ref().to_string_lossy().into_owned(),
            source: e,
        })?;

        let mut config: Config =
            toml::from_str(&content).map_err(|e| ConfigError::Parse { source: e })?;

        if let Ok(url) = env::var("DATABASE_URL") {
            config.database.url = url;
        }

        if let Ok(secret) = env::var("JWT_SECRET") {
            config.jwt.secret = secret;
        }

        if let Ok(addr) = env::var("LISTEN_ADDR") {
            config.server.listen = addr.parse().map_err(|_| ConfigError::InvalidValue {
                field: "LISTEN_ADDR".to_owned(),
                value: addr,
                reason: "无效的套接字地址格式".to_owned(),
            })?;
        }

        config.validate()?;

        Ok(config)
    }

    fn validate(&self) -> Result<(), ConfigError> {
        if self.jwt.secret.len() < 32 {
            return Err(ConfigError::InvalidValue {
                field: "jwt.secret".to_owned(),
                value: "[REDACTED]".to_owned(),
                reason: "JWT 密钥长度必须至少 32 字符".to_owned(),
            });
        }

        if self.database.url.is_empty() {
            return Err(ConfigError::MissingField {
                field: "database.url".to_owned(),
            });
        }

        Ok(())
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::io::Write;
    use tempfile::NamedTempFile;

    fn create_temp_config(content: &str) -> NamedTempFile {
        let mut file = NamedTempFile::new().unwrap();
        file.write_all(content.as_bytes()).unwrap();
        file
    }

    #[test]
    fn test_load_valid_config() {
        let content = r#"
[database]
url = "postgres://user:pass@localhost/db"

[server]
listen = "127.0.0.1:8080"

[jwt]
secret = "this-is-a-very-long-secret-key-for-testing-purposes"

[quota]
period_minutes = 30

[execution]
max_concurrent = 5
"#;
        let file = create_temp_config(content);
        let config = Config::load(file.path()).unwrap();

        assert_eq!(config.server.listen.port(), 8080);
        assert_eq!(config.quota.period_minutes, 30);
        assert_eq!(config.execution.max_concurrent, 5);
    }

    #[test]
    fn test_load_nonexistent_file() {
        let result = Config::load("/nonexistent/path/config.toml");
        assert!(matches!(result, Err(ConfigError::FileRead { .. })));
    }

    #[test]
    fn test_reject_short_jwt_secret() {
        let content = r#"
[database]
url = "postgres://user:pass@localhost/db"

[server]

[jwt]
secret = "short"

[quota]

[execution]
"#;
        let file = create_temp_config(content);
        let result = Config::load(file.path());

        assert!(
            matches!(result, Err(ConfigError::InvalidValue { field, .. }) if field == "jwt.secret")
        );
    }

    #[test]
    fn test_reject_empty_database_url() {
        let content = r#"
[database]
url = ""

[server]

[jwt]
secret = "this-is-a-very-long-secret-key-for-testing-purposes"

[quota]

[execution]
"#;
        let file = create_temp_config(content);
        let result = Config::load(file.path());

        assert!(
            matches!(result, Err(ConfigError::MissingField { field }) if field == "database.url")
        );
    }

    #[test]
    fn test_use_default_values() {
        let content = r#"
[database]
url = "postgres://user:pass@localhost/db"

[server]

[jwt]
secret = "this-is-a-very-long-secret-key-for-testing-purposes"

[quota]

[execution]
"#;
        let file = create_temp_config(content);
        let config = Config::load(file.path()).unwrap();

        assert_eq!(config.database.max_connections, 10);
        assert_eq!(config.quota.period_minutes, 60);
        assert_eq!(config.quota.cpu_time_ms, 10000);
    }
}