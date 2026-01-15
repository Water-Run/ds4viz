//! 数据库连接池模块

use sqlx::postgres::{PgPool, PgPoolOptions};

use crate::config::DatabaseConfig;
use crate::error::ApiError;

/// 数据库连接池类型别名
pub type DbPool = PgPool;

/// 创建数据库连接池
pub async fn create_pool(config: &DatabaseConfig) -> Result<DbPool, ApiError> {
    let pool = PgPoolOptions::new()
        .max_connections(config.max_connections)
        .min_connections(config.min_connections)
        .connect(&config.url)
        .await
        .map_err(|e| {
            tracing::error!(error = %e, "无法创建数据库连接池");
            ApiError::internal("无法连接到数据库")
        })?;

    sqlx::query("SELECT 1")
        .execute(&pool)
        .await
        .map_err(|e| {
            tracing::error!(error = %e, "数据库连接验证失败");
            ApiError::internal("数据库连接验证失败")
        })?;

    tracing::info!(
        max_connections = config.max_connections,
        min_connections = config.min_connections,
        "数据库连接池创建成功"
    );

    Ok(pool)
}