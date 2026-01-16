//! ds4viz 数据结构可视化平台后端服务库
//!
//! 本库提供 ds4viz 平台的核心后端功能，包括用户认证、代码执行、模板管理等模块。
//!
//! # 模块结构
//!
//! - `config`: 配置加载与管理
//! - `error`: 统一错误类型定义
//! - `db`: 数据库连接池管理
//! - `model`: 数据模型（数据库映射与 API 结构）
//! - `auth`: JWT 认证与用户提取器
//! - `app`: 应用路由与状态组装
//! - `executions`: 代码执行服务
//! - `sandbox`: 沙箱隔离执行
//! - `quota`: 配额管理
//! - `cache`: 执行结果缓存
//!
//! # 已实现模块
//!
//! - `cache`: 执行结果缓存管理（LRU + PostgreSQL）
//! - `executions`: 代码执行处理
//! - `quota`: 配额管理
//! - `sandbox`: 沙箱执行环境
//!
//! # 待实现模块
//!
//! - `templates`: 模板系统
//!
//! 作者: WaterRun
//! 日期: 2026-01-16

pub mod app;
pub mod auth;
pub mod cache;
pub mod config;
pub mod db;
pub mod error;
pub mod executions;
pub mod model;
pub mod quota;
pub mod sandbox;
