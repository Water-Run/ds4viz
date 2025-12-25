//! 数据结构模块
//!
//! 提供各种数据结构的实现
//!
//! # 作者
//!
//! WaterRun
//!
//! # 文件路径
//!
//! src/structures/mod.rs
//!
//! # 日期
//!
//! 2025-12-25

mod base;
mod graph;
mod linear;
mod tree;

pub use base::BaseStructure;
pub use graph::{GraphDirected, GraphUndirected, GraphWeighted};
pub use linear::{DoubleLinkedList, Queue, SingleLinkedList, Stack};
pub use tree::{BinarySearchTree, BinaryTree, Heap, HeapType};