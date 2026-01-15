//! ds4viz 服务器库

#![forbid(unsafe_code)]
#![deny(warnings)]

pub mod app;
pub mod auth;
pub mod cache;
pub mod config;
pub mod db;
pub mod error;
pub mod executions;
pub mod model;
pub mod quota;
pub mod templates;

pub mod sandbox;

pub use app::AppState;
pub use config::Config;
pub use db::DbPool;
pub use error::{ApiError, ErrorCode};