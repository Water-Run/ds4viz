//! Trace 数据模型定义
//!
//! 定义 IR 规范中的所有数据结构
//!
//! # 作者
//!
//! WaterRun
//!
//! # 文件路径
//!
//! src/trace/model.rs
//!
//! # 日期
//!
//! 2025-12-25

use std::collections::HashMap;

/// 值类型枚举
///
/// 根据 IR 定义，value 字段仅支持以下类型
#[derive(Debug, Clone, PartialEq)]
pub enum Value {
    /// 整数
    Integer(i64),
    /// 浮点数
    Float(f64),
    /// 字符串
    String(String),
    /// 布尔值
    Boolean(bool),
    /// 数组
    Array(Vec<Value>),
    /// 表（用于内联表）
    Table(HashMap<String, Value>),
}

impl From<i64> for Value {
    fn from(v: i64) -> Self {
        Self::Integer(v)
    }
}

impl From<i32> for Value {
    fn from(v: i32) -> Self {
        Self::Integer(i64::from(v))
    }
}

impl From<f64> for Value {
    fn from(v: f64) -> Self {
        Self::Float(v)
    }
}

impl From<f32> for Value {
    fn from(v: f32) -> Self {
        Self::Float(f64::from(v))
    }
}

impl From<String> for Value {
    fn from(v: String) -> Self {
        Self::String(v)
    }
}

impl From<&str> for Value {
    fn from(v: &str) -> Self {
        Self::String(v.to_string())
    }
}

impl From<bool> for Value {
    fn from(v: bool) -> Self {
        Self::Boolean(v)
    }
}

impl<T: Into<Value>> From<Vec<T>> for Value {
    fn from(v: Vec<T>) -> Self {
        Self::Array(v.into_iter().map(Into::into).collect())
    }
}

/// 数值类型（用于 BST 和 Heap）
#[derive(Debug, Clone, PartialEq)]
pub enum NumericValue {
    /// 整数
    Integer(i64),
    /// 浮点数
    Float(f64),
}

impl NumericValue {
    /// 比较两个数值
    #[must_use]
    pub fn cmp_value(&self, other: &Self) -> std::cmp::Ordering {
        match (self, other) {
            (Self::Integer(a), Self::Integer(b)) => a.cmp(b),
            (Self::Float(a), Self::Float(b)) => a.partial_cmp(b).unwrap_or(std::cmp::Ordering::Equal),
            (Self::Integer(a), Self::Float(b)) => {
                (*a as f64).partial_cmp(b).unwrap_or(std::cmp::Ordering::Equal)
            }
            (Self::Float(a), Self::Integer(b)) => {
                a.partial_cmp(&(*b as f64)).unwrap_or(std::cmp::Ordering::Equal)
            }
        }
    }
}

impl From<i64> for NumericValue {
    fn from(v: i64) -> Self {
        Self::Integer(v)
    }
}

impl From<i32> for NumericValue {
    fn from(v: i32) -> Self {
        Self::Integer(i64::from(v))
    }
}

impl From<f64> for NumericValue {
    fn from(v: f64) -> Self {
        Self::Float(v)
    }
}

impl From<f32> for NumericValue {
    fn from(v: f32) -> Self {
        Self::Float(f64::from(v))
    }
}

impl From<NumericValue> for Value {
    fn from(v: NumericValue) -> Self {
        match v {
            NumericValue::Integer(i) => Self::Integer(i),
            NumericValue::Float(f) => Self::Float(f),
        }
    }
}

impl std::fmt::Display for NumericValue {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            Self::Integer(v) => write!(f, "{v}"),
            Self::Float(v) => write!(f, "{v}"),
        }
    }
}

/// 元数据信息
#[derive(Debug, Clone)]
pub struct Meta {
    /// 创建时间
    pub created_at: String,
    /// 语言
    pub lang: String,
    /// 语言版本
    pub lang_version: String,
}

/// 包信息
#[derive(Debug, Clone)]
pub struct Package {
    /// 包名称
    pub name: String,
    /// 语言
    pub lang: String,
    /// 版本
    pub version: String,
}

/// 备注信息
#[derive(Debug, Clone)]
pub struct Remarks {
    /// 标题
    pub title: Option<String>,
    /// 作者
    pub author: Option<String>,
    /// 注释
    pub comment: Option<String>,
}

impl Remarks {
    /// 检查备注是否为空
    #[must_use]
    pub fn is_empty(&self) -> bool {
        self.title.is_none() && self.author.is_none() && self.comment.is_none()
    }
}

/// 数据结构对象描述
#[derive(Debug, Clone)]
pub struct Object {
    /// 数据结构类型
    pub kind: String,
    /// 标签
    pub label: Option<String>,
}

/// 状态快照
#[derive(Debug, Clone)]
pub struct State {
    /// 状态 ID
    pub id: i64,
    /// 状态数据
    pub data: HashMap<String, Value>,
}

/// 代码位置信息
#[derive(Debug, Clone)]
pub struct CodeLocation {
    /// 行号
    pub line: i64,
    /// 列号
    pub col: Option<i64>,
}

/// 操作步骤
#[derive(Debug, Clone)]
pub struct Step {
    /// 步骤 ID
    pub id: i64,
    /// 操作名称
    pub op: String,
    /// 操作前状态 ID
    pub before: i64,
    /// 操作后状态 ID
    pub after: Option<i64>,
    /// 操作参数
    pub args: HashMap<String, Value>,
    /// 代码位置
    pub code: Option<CodeLocation>,
    /// 返回值
    pub ret: Option<Value>,
    /// 备注
    pub note: Option<String>,
}

/// 提交信息
#[derive(Debug, Clone)]
pub struct Commit {
    /// 操作名称
    pub op: String,
    /// 行号
    pub line: i64,
}

/// 成功结果
#[derive(Debug, Clone)]
pub struct Result {
    /// 最终状态 ID
    pub final_state: i64,
    /// 提交信息
    pub commit: Option<Commit>,
}

/// 错误信息
#[derive(Debug, Clone)]
pub struct Error {
    /// 错误类型
    pub error_type: String,
    /// 错误消息
    pub message: String,
    /// 行号
    pub line: Option<i64>,
    /// 步骤 ID
    pub step: Option<i64>,
    /// 最后状态 ID
    pub last_state: Option<i64>,
}

/// 完整的操作轨迹记录
#[derive(Debug, Clone)]
pub struct Trace {
    /// 元数据
    pub meta: Meta,
    /// 包信息
    pub package: Package,
    /// 备注
    pub remarks: Option<Remarks>,
    /// 对象描述
    pub object: Object,
    /// 状态列表
    pub states: Vec<State>,
    /// 步骤列表
    pub steps: Vec<Step>,
    /// 成功结果
    pub result: Option<Result>,
    /// 错误信息
    pub error: Option<Error>,
}