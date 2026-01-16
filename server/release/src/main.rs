//! ds4viz 服务器入口
//!
//! 本文件是应用程序的主入口点，负责初始化日志、加载配置、
//! 连接数据库并启动 HTTP 服务器。
//!
//! # 运行方式
//!
//! ```bash
//! # 使用默认配置文件
//! cargo run
//!
//! # 指定配置文件
//! cargo run -- --config configs/production.toml
//! ```
//!
//! 作者: WaterRun
//! 日期: 2026-01-16

use std::env;

use poem::{Server, listener::TcpListener};
use tracing::{error, info};
use tracing_subscriber::{layer::SubscriberExt, util::SubscriberInitExt};

use ds4viz_server::app::{AppState, create_app};
use ds4viz_server::auth::JwtManager;
use ds4viz_server::config::Config;
use ds4viz_server::db::Database;

#[tokio::main]
async fn main() {
    init_logging();

    if let Err(e) = run().await {
        error!("服务器启动失败: {}", e);
        std::process::exit(1);
    }
}

fn init_logging() {
    tracing_subscriber::registry()
        .with(
            tracing_subscriber::EnvFilter::try_from_default_env().unwrap_or_else(|_| {
                "ds4viz_server=debug,poem=info"
                    .parse()
                    .expect("默认日志过滤器应有效")
            }),
        )
        .with(tracing_subscriber::fmt::layer())
        .init();
}

async fn run() -> Result<(), Box<dyn std::error::Error>> {
    let config_path = env::args()
        .nth(2)
        .filter(|_| env::args().nth(1).is_some_and(|arg| arg == "--config"))
        .unwrap_or_else(|| "configs/default.toml".to_string());

    info!("加载配置文件: {}", config_path);
    let config = Config::from_file(&config_path)?;

    info!("连接数据库: {}", config.database.url);
    let db = Database::connect(&config.database).await?;

    let jwt_manager = JwtManager::new(&config.jwt);

    info!("初始化执行服务...");
    let execution_service =
        ds4viz_server::executions::ExecutionService::new(db.clone(), config.clone());
    let quota_manager = std::sync::Arc::new(ds4viz_server::quota::QuotaManager::new(
        db.clone(),
        config.quota.clone(),
    ));

    let bind_addr = format!("{}:{}", config.server.host, config.server.port);
    info!("服务器监听地址: {}", bind_addr);

    let state = AppState {
        db,
        config,
        jwt_manager,
        execution_service,
        quota_manager,
    };

    let app = create_app(state);

    Server::new(TcpListener::bind(&bind_addr)).run(app).await?;

    Ok(())
}
