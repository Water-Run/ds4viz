//! 错误类型定义模块
//!
//! 定义 ds4viz 库使用的所有错误类型
//!
//! # 作者
//!
//! WaterRun
//!
//! # 文件路径
//!
//! src/error.rs
//!
//! # 日期
//!
//! 2025-12-25

use thiserror::Error;

/// ds4viz 库的结果类型别名
pub type Result<T> = std::result::Result<T, Ds4VizError>;

/// ds4viz 错误类型
///
/// 定义库中可能发生的所有错误
#[derive(Debug, Error)]
pub enum Ds4VizError {
    /// 数据结构操作错误
    #[error("Structure error: {0}")]
    Structure(#[from] StructureError),

    /// IO 错误
    #[error("IO error: {0}")]
    Io(#[from] std::io::Error),

    /// 验证错误
    #[error("Validation error: {0}")]
    Validation(String),
}

/// 数据结构操作错误
///
/// 定义数据结构操作过程中可能发生的错误
#[derive(Debug, Error, Clone)]
pub enum StructureError {
    /// 结构为空
    #[error("Structure is empty")]
    EmptyStructure,

    /// 节点未找到
    #[error("Node not found: {0}")]
    NodeNotFound(i64),

    /// 节点已存在
    #[error("Node already exists: {0}")]
    NodeExists(i64),

    /// 边未找到
    #[error("Edge not found: ({0}, {1})")]
    EdgeNotFound(i64, i64),

    /// 边已存在
    #[error("Edge already exists: ({0}, {1})")]
    EdgeExists(i64, i64),

    /// 不允许自环
    #[error("Self-loop not allowed: {0}")]
    SelfLoop(i64),

    /// 根节点已存在
    #[error("Root node already exists")]
    RootExists,

    /// 子节点已存在
    #[error("Child node already exists")]
    ChildExists,

    /// 值未找到
    #[error("Value not found: {0}")]
    ValueNotFound(String),

    /// 标签长度无效
    #[error("Invalid label length: expected 1-32, got {0}")]
    InvalidLabel(usize),

    /// 自定义错误
    #[error("{0}")]
    Custom(String),
}

/// 错误类型枚举，对应 IR 定义中的 error.type
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum ErrorType {
    /// 运行时错误
    Runtime,
    /// 超时错误
    Timeout,
    /// 验证错误
    Validation,
    /// 沙箱错误
    Sandbox,
    /// 未知错误
    Unknown,
}

impl ErrorType {
    /// 转换为 IR 定义的字符串
    #[must_use]
    pub const fn as_str(&self) -> &'static str {
        match self {
            Self::Runtime => "runtime",
            Self::Timeout => "timeout",
            Self::Validation => "validation",
            Self::Sandbox => "sandbox",
            Self::Unknown => "unknown",
        }
    }
}

impl From<&Ds4VizError> for ErrorType {
    fn from(err: &Ds4VizError) -> Self {
        match err {
            Ds4VizError::Structure(_) => Self::Runtime,
            Ds4VizError::Io(_) => Self::Runtime,
            Ds4VizError::Validation(_) => Self::Validation,
        }
    }
}