//! 预导入模块
//!
//! 提供常用类型和 trait 的便捷导入
//!
//! # 示例
//!
//! ```
//! use ds4viz::prelude::*;
//! ```
//!
//! # 作者
//!
//! WaterRun
//!
//! # 文件路径
//!
//! src/prelude.rs
//!
//! # 日期
//!
//! 2025-12-25

pub use crate::error::{Ds4VizError, Result, StructureError};
pub use crate::session::{config, Config};
pub use crate::structures::{
    BinarySearchTree, BinaryTree, DoubleLinkedList, GraphDirected, GraphUndirected, GraphWeighted,
    Heap, HeapType, Queue, SingleLinkedList, Stack,
};
pub use crate::trace::{NumericValue, Value};