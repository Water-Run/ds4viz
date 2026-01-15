//! ds4viz 服务器入口

#![forbid(unsafe_code)]
#![deny(warnings)]

use std::sync::Arc;

use poem::listener::TcpListener;
use poem::Server;
use tracing_subscriber::layer::SubscriberExt;
use tracing_subscriber::util::SubscriberInitExt;
use tracing_subscriber::EnvFilter;

use ds4viz_server::app::{build_app, AppConfig, AppState};
use ds4viz_server::config::Config;
use ds4viz_server::db;

#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error>> {
    tracing_subscriber::registry()
        .with(EnvFilter::try_from_default_env().unwrap_or_else(|_| EnvFilter::new("info")))
        .with(tracing_subscriber::fmt::layer())
        .init();

    let config_path = std::env::args()
        .nth(1)
        .unwrap_or_else(|| "configs/default.toml".to_owned());

    tracing::info!(path = %config_path, "加载配置文件");
    let config = Config::load(&config_path)?;

    let pool = db::create_pool(&config.database).await?;

    let state = AppState {
        pool,
        config: Arc::new(AppConfig::from(&config)),
    };

    let listen_addr = config.server.listen;
    let listener = TcpListener::bind(listen_addr);
    tracing::info!(address = %listen_addr, "服务器启动");

    Server::new(listener).run(build_app(state)).await?;

    Ok(())
}