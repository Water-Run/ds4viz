//! ds4viz - 可扩展的数据结构可视化教学平台 Rust 库
//!
//! 提供数据结构的可视化追踪功能，生成标准化的 TOML 中间表示文件。
//!
//! # 快速上手
//!
//! ```no_run
//! use ds4viz::prelude::*;
//!
//! fn main() -> ds4viz::Result<()> {
//!     ds4viz::stack("demo_stack", |s| {
//!         s.push(10)?;
//!         s.push(20)?;
//!         s.pop()?;
//!         Ok(())
//!     })
//! }
//! ```
//!
//! # 作者
//!
//! WaterRun
//!
//! # 文件路径
//!
//! src/lib.rs
//!
//! # 日期
//!
//! 2025-12-25

#![warn(missing_docs)]
#![warn(clippy::all)]
#![warn(clippy::pedantic)]
#![allow(clippy::module_name_repetitions)]

pub mod error;
pub mod prelude;
pub mod session;
pub mod structures;
pub mod trace;
pub mod writer;

pub use error::{Ds4VizError, Result, StructureError};
pub use session::{config, Config};
pub use structures::{
    BinarySearchTree, BinaryTree, DoubleLinkedList, GraphDirected, GraphUndirected, GraphWeighted,
    Heap, HeapType, Queue, SingleLinkedList, Stack,
};
pub use trace::{NumericValue, Value};

use session::Session;
use std::panic::{catch_unwind, AssertUnwindSafe};

/// 创建栈实例并在闭包中操作
///
/// # 参数
///
/// * `label` - 栈的标签
/// * `f` - 操作闭包，接收可变栈引用
///
/// # 返回
///
/// 成功返回 `Ok(())`，失败返回错误
///
/// # 示例
///
/// ```no_run
/// use ds4viz::prelude::*;
///
/// fn main() -> ds4viz::Result<()> {
///     ds4viz::stack("demo_stack", |s| {
///         s.push(10)?;
///         s.push(20)?;
///         s.pop()?;
///         Ok(())
///     })
/// }
/// ```
pub fn stack<F>(label: &str, f: F) -> Result<()>
where
    F: FnOnce(&mut Stack) -> Result<()>,
{
    stack_with_output(label, None, f)
}

/// 创建栈实例并指定输出路径
///
/// # 参数
///
/// * `label` - 栈的标签
/// * `output` - 输出文件路径，`None` 使用全局配置
/// * `f` - 操作闭包
pub fn stack_with_output<F>(label: &str, output: Option<&str>, f: F) -> Result<()>
where
    F: FnOnce(&mut Stack) -> Result<()>,
{
    run_with_session("stack", label, output, |session| {
        let mut structure = Stack::new(session);
        f(&mut structure)
    })
}

/// 创建队列实例并在闭包中操作
///
/// # 参数
///
/// * `label` - 队列的标签
/// * `f` - 操作闭包
pub fn queue<F>(label: &str, f: F) -> Result<()>
where
    F: FnOnce(&mut Queue) -> Result<()>,
{
    queue_with_output(label, None, f)
}

/// 创建队列实例并指定输出路径
pub fn queue_with_output<F>(label: &str, output: Option<&str>, f: F) -> Result<()>
where
    F: FnOnce(&mut Queue) -> Result<()>,
{
    run_with_session("queue", label, output, |session| {
        let mut structure = Queue::new(session);
        f(&mut structure)
    })
}

/// 创建单链表实例并在闭包中操作
///
/// # 参数
///
/// * `label` - 单链表的标签
/// * `f` - 操作闭包
pub fn single_linked_list<F>(label: &str, f: F) -> Result<()>
where
    F: FnOnce(&mut SingleLinkedList) -> Result<()>,
{
    single_linked_list_with_output(label, None, f)
}

/// 创建单链表实例并指定输出路径
pub fn single_linked_list_with_output<F>(label: &str, output: Option<&str>, f: F) -> Result<()>
where
    F: FnOnce(&mut SingleLinkedList) -> Result<()>,
{
    run_with_session("slist", label, output, |session| {
        let mut structure = SingleLinkedList::new(session);
        f(&mut structure)
    })
}

/// 创建双向链表实例并在闭包中操作
///
/// # 参数
///
/// * `label` - 双向链表的标签
/// * `f` - 操作闭包
pub fn double_linked_list<F>(label: &str, f: F) -> Result<()>
where
    F: FnOnce(&mut DoubleLinkedList) -> Result<()>,
{
    double_linked_list_with_output(label, None, f)
}

/// 创建双向链表实例并指定输出路径
pub fn double_linked_list_with_output<F>(label: &str, output: Option<&str>, f: F) -> Result<()>
where
    F: FnOnce(&mut DoubleLinkedList) -> Result<()>,
{
    run_with_session("dlist", label, output, |session| {
        let mut structure = DoubleLinkedList::new(session);
        f(&mut structure)
    })
}

/// 创建二叉树实例并在闭包中操作
///
/// # 参数
///
/// * `label` - 二叉树的标签
/// * `f` - 操作闭包
pub fn binary_tree<F>(label: &str, f: F) -> Result<()>
where
    F: FnOnce(&mut BinaryTree) -> Result<()>,
{
    binary_tree_with_output(label, None, f)
}

/// 创建二叉树实例并指定输出路径
pub fn binary_tree_with_output<F>(label: &str, output: Option<&str>, f: F) -> Result<()>
where
    F: FnOnce(&mut BinaryTree) -> Result<()>,
{
    run_with_session("binary_tree", label, output, |session| {
        let mut structure = BinaryTree::new(session);
        f(&mut structure)
    })
}

/// 创建二叉搜索树实例并在闭包中操作
///
/// # 参数
///
/// * `label` - 二叉搜索树的标签
/// * `f` - 操作闭包
pub fn binary_search_tree<F>(label: &str, f: F) -> Result<()>
where
    F: FnOnce(&mut BinarySearchTree) -> Result<()>,
{
    binary_search_tree_with_output(label, None, f)
}

/// 创建二叉搜索树实例并指定输出路径
pub fn binary_search_tree_with_output<F>(label: &str, output: Option<&str>, f: F) -> Result<()>
where
    F: FnOnce(&mut BinarySearchTree) -> Result<()>,
{
    run_with_session("bst", label, output, |session| {
        let mut structure = BinarySearchTree::new(session);
        f(&mut structure)
    })
}

/// 创建堆实例并在闭包中操作
///
/// # 参数
///
/// * `label` - 堆的标签
/// * `heap_type` - 堆类型（最小堆或最大堆）
/// * `f` - 操作闭包
pub fn heap<F>(label: &str, heap_type: HeapType, f: F) -> Result<()>
where
    F: FnOnce(&mut Heap) -> Result<()>,
{
    heap_with_output(label, heap_type, None, f)
}

/// 创建堆实例并指定输出路径
pub fn heap_with_output<F>(
    label: &str,
    heap_type: HeapType,
    output: Option<&str>,
    f: F,
) -> Result<()>
where
    F: FnOnce(&mut Heap) -> Result<()>,
{
    run_with_session("binary_tree", label, output, |session| {
        let mut structure = Heap::new(session, heap_type);
        f(&mut structure)
    })
}

/// 创建无向图实例并在闭包中操作
///
/// # 参数
///
/// * `label` - 无向图的标签
/// * `f` - 操作闭包
pub fn graph_undirected<F>(label: &str, f: F) -> Result<()>
where
    F: FnOnce(&mut GraphUndirected) -> Result<()>,
{
    graph_undirected_with_output(label, None, f)
}

/// 创建无向图实例并指定输出路径
pub fn graph_undirected_with_output<F>(label: &str, output: Option<&str>, f: F) -> Result<()>
where
    F: FnOnce(&mut GraphUndirected) -> Result<()>,
{
    run_with_session("graph_undirected", label, output, |session| {
        let mut structure = GraphUndirected::new(session);
        f(&mut structure)
    })
}

/// 创建有向图实例并在闭包中操作
///
/// # 参数
///
/// * `label` - 有向图的标签
/// * `f` - 操作闭包
pub fn graph_directed<F>(label: &str, f: F) -> Result<()>
where
    F: FnOnce(&mut GraphDirected) -> Result<()>,
{
    graph_directed_with_output(label, None, f)
}

/// 创建有向图实例并指定输出路径
pub fn graph_directed_with_output<F>(label: &str, output: Option<&str>, f: F) -> Result<()>
where
    F: FnOnce(&mut GraphDirected) -> Result<()>,
{
    run_with_session("graph_directed", label, output, |session| {
        let mut structure = GraphDirected::new(session);
        f(&mut structure)
    })
}

/// 创建带权图实例并在闭包中操作
///
/// # 参数
///
/// * `label` - 带权图的标签
/// * `directed` - 是否为有向图
/// * `f` - 操作闭包
pub fn graph_weighted<F>(label: &str, directed: bool, f: F) -> Result<()>
where
    F: FnOnce(&mut GraphWeighted) -> Result<()>,
{
    graph_weighted_with_output(label, directed, None, f)
}

/// 创建带权图实例并指定输出路径
pub fn graph_weighted_with_output<F>(
    label: &str,
    directed: bool,
    output: Option<&str>,
    f: F,
) -> Result<()>
where
    F: FnOnce(&mut GraphWeighted) -> Result<()>,
{
    run_with_session("graph_weighted", label, output, |session| {
        let mut structure = GraphWeighted::new(session, directed);
        f(&mut structure)
    })
}

/// 运行会话并处理结果
fn run_with_session<F>(kind: &str, label: &str, output: Option<&str>, f: F) -> Result<()>
where
    F: FnOnce(&mut Session) -> Result<()>,
{
    let mut session = Session::new(kind, label, output);

    // 使用 catch_unwind 捕获 panic
    let result = catch_unwind(AssertUnwindSafe(|| f(&mut session)));

    match result {
        Ok(Ok(())) => {
            session.finalize_success()?;
        }
        Ok(Err(e)) => {
            session.finalize_error(&e)?;
            return Err(e);
        }
        Err(panic_info) => {
            let message = if let Some(s) = panic_info.downcast_ref::<&str>() {
                (*s).to_string()
            } else if let Some(s) = panic_info.downcast_ref::<String>() {
                s.clone()
            } else {
                "Unknown panic".to_string()
            };
            let err = Ds4VizError::Structure(StructureError::Custom(message));
            session.finalize_error(&err)?;
            return Err(err);
        }
    }

    Ok(())
}