//! 应用路由配置

use std::sync::Arc;

use poem::middleware::{Cors, Tracing};
use poem::{get, EndpointExt, Route};

use crate::auth::auth_routes;
use crate::config::{Config, JwtConfig};
use crate::db::DbPool;

/// 应用状态
#[derive(Clone)]
pub struct AppState {
    pub pool: DbPool,
    pub config: Arc<AppConfig>,
}

/// 应用配置（运行时需要的配置）
#[derive(Clone)]
pub struct AppConfig {
    pub jwt: Arc<JwtConfig>,
    pub quota_period_minutes: u32,
    pub quota_cpu_time_ms: u64,
    pub max_code_size_bytes: usize,
}

impl From<&Config> for AppConfig {
    fn from(config: &Config) -> Self {
        Self {
            jwt: Arc::new(config.jwt.clone()),
            quota_period_minutes: config.quota.period_minutes,
            quota_cpu_time_ms: config.quota.cpu_time_ms,
            max_code_size_bytes: config.quota.max_code_size_bytes,
        }
    }
}

#[poem::handler]
async fn health_check() -> &'static str {
    "OK"
}

/// 构建应用路由
#[must_use]
pub fn build_app(state: AppState) -> impl poem::Endpoint {
    let api_routes = Route::new()
        .at("/health", get(health_check))
        .nest("/auth", auth_routes());

    Route::new()
        .nest("/api/v1", api_routes)
        .with(Tracing)
        .with(Cors::new())
        .data(state)
}

#[cfg(test)]
mod tests {
    use super::*;

    #[tokio::test]
    async fn test_health_check() {
        let response = health_check().await;
        assert_eq!(response, "OK");
    }
}