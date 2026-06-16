# `ds4viz`

`ds4viz` 是一个 Rust 库，提供了 Rust 语言的 `ds4viz` 支持。
`ds4viz` 是一个可视化数据结构工具。参见：[GitHub](https://github.com/Water-Run/ds4viz)

## 安装

`ds4viz` 发布在 [crates.io](https://crates.io/crates/ds4viz) 上：

```toml
[dependencies]
ds4viz = "0.1.0"
```

或使用 cargo 命令：

```bash
cargo add ds4viz
```

## 测试

在目录下运行：

```bash
cargo test
```

运行 Clippy 检查：

```bash
cargo clippy -- -D warnings
```

## 快速上手

以下代码提供了一个快速使用示例，演示如何使用 `ds4viz` 生成 `trace.toml`，并在发生错误时仍然产出包含 `[error]` 的 `.toml` IR 文件：

```rust
use ds4viz::prelude::*;

fn main() -> ds4viz::Result<()> {
    // 使用闭包上下文：确保无论成功/失败，都会生成 trace.toml
    // 成功 => [result]，错误 => [error]
    ds4viz::graph_undirected("demo_graph", |graph| {
        // 添加节点
        graph.add_node(0, "A")?;
        graph.add_node(1, "B")?;
        graph.add_node(2, "C")?;

        // 添加无向边（内部会规范化为 from < to）
        graph.add_edge(0, 1)?;
        graph.add_edge(1, 2)?;

        // 也可以在这里继续调用更多 API，每次操作都会记录到 trace 中
        Ok(())
    })?;

    println!("已生成 trace.toml");
    Ok(())
}
```

## API 参考

### 全局配置

#### `config`

```rust
/// 配置全局参数
///
/// # 参数
///
/// * `output_path` - 输出文件路径，默认为 "trace.toml"
/// * `title` - 可视化标题
/// * `author` - 作者信息
/// * `comment` - 注释说明
pub fn config(settings: Config) -> ()
```

**示例：**

```rust
use ds4viz::{config, Config};

config(Config {
    output_path: "my_trace.toml".into(),
    title: "栈操作演示".into(),
    author: "WaterRun".into(),
    comment: "演示基本的栈操作".into(),
});
```

使用 Builder 模式：

```rust
use ds4viz::Config;

ds4viz::config(
    Config::builder()
        .output_path("my_trace.toml")
        .title("栈操作演示")
        .author("WaterRun")
        .comment("演示基本的栈操作")
        .build()
);
```

---

### 线性结构

#### `stack`

```rust
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
pub fn stack<F>(label: &str, f: F) -> Result<()>
where
    F: FnOnce(&mut Stack) -> Result<()>
```

**Stack 结构体方法：**

```rust
impl Stack {
    /// 压栈操作
    ///
    /// # 参数
    ///
    /// * `value` - 要压入的值，支持 i64、f64、String、bool
    pub fn push(&mut self, value: impl Into<Value>) -> Result<()>;

    /// 弹栈操作
    ///
    /// # 错误
    ///
    /// 当栈为空时返回 `StructureError::EmptyStructure`
    pub fn pop(&mut self) -> Result<()>;

    /// 清空栈
    pub fn clear(&mut self) -> Result<()>;
}
```

**示例：**

```rust
use ds4viz::prelude::*;

fn main() -> ds4viz::Result<()> {
    ds4viz::stack("demo_stack", |s| {
        s.push(10)?;
        s.push(20)?;
        s.push(30)?;

        s.pop()?;
        s.pop()?;
        Ok(())
    })
}
```

#### `queue`

```rust
/// 创建队列实例并在闭包中操作
///
/// # 参数
///
/// * `label` - 队列的标签
/// * `f` - 操作闭包
pub fn queue<F>(label: &str, f: F) -> Result<()>
where
    F: FnOnce(&mut Queue) -> Result<()>
```

**Queue 结构体方法：**

```rust
impl Queue {
    /// 入队操作
    ///
    /// # 参数
    ///
    /// * `value` - 要入队的值
    pub fn enqueue(&mut self, value: impl Into<Value>) -> Result<()>;

    /// 出队操作
    ///
    /// # 错误
    ///
    /// 当队列为空时返回 `StructureError::EmptyStructure`
    pub fn dequeue(&mut self) -> Result<()>;

    /// 清空队列
    pub fn clear(&mut self) -> Result<()>;
}
```

**示例：**

```rust
use ds4viz::prelude::*;

fn main() -> ds4viz::Result<()> {
    ds4viz::queue("demo_queue", |q| {
        q.enqueue(10)?;
        q.enqueue(20)?;
        q.enqueue(30)?;

        q.dequeue()?;
        q.dequeue()?;
        Ok(())
    })
}
```

#### `single_linked_list`

```rust
/// 创建单链表实例并在闭包中操作
///
/// # 参数
///
/// * `label` - 单链表的标签
/// * `f` - 操作闭包
pub fn single_linked_list<F>(label: &str, f: F) -> Result<()>
where
    F: FnOnce(&mut SingleLinkedList) -> Result<()>
```

**SingleLinkedList 结构体方法：**

```rust
impl SingleLinkedList {
    /// 在链表头部插入节点
    ///
    /// # 返回
    ///
    /// 返回新插入节点的 id
    pub fn insert_head(&mut self, value: impl Into<Value>) -> Result<i64>;

    /// 在链表尾部插入节点
    ///
    /// # 返回
    ///
    /// 返回新插入节点的 id
    pub fn insert_tail(&mut self, value: impl Into<Value>) -> Result<i64>;

    /// 在指定节点后插入新节点
    ///
    /// # 错误
    ///
    /// 当指定节点不存在时返回 `StructureError::NodeNotFound`
    pub fn insert_after(&mut self, node_id: i64, value: impl Into<Value>) -> Result<i64>;

    /// 删除指定节点
    ///
    /// # 错误
    ///
    /// 当指定节点不存在时返回 `StructureError::NodeNotFound`
    pub fn delete(&mut self, node_id: i64) -> Result<()>;

    /// 删除头节点
    ///
    /// # 错误
    ///
    /// 当链表为空时返回 `StructureError::EmptyStructure`
    pub fn delete_head(&mut self) -> Result<()>;

    /// 反转链表
    pub fn reverse(&mut self) -> Result<()>;
}
```

**示例：**

```rust
use ds4viz::prelude::*;

fn main() -> ds4viz::Result<()> {
    ds4viz::single_linked_list("demo_slist", |slist| {
        let node_a = slist.insert_head(10)?;
        slist.insert_tail(20)?;
        let node_c = slist.insert_tail(30)?;

        slist.insert_after(node_a, 15)?;

        slist.delete(node_c)?;
        slist.reverse()?;
        Ok(())
    })
}
```

#### `double_linked_list`

```rust
/// 创建双向链表实例并在闭包中操作
///
/// # 参数
///
/// * `label` - 双向链表的标签
/// * `f` - 操作闭包
pub fn double_linked_list<F>(label: &str, f: F) -> Result<()>
where
    F: FnOnce(&mut DoubleLinkedList) -> Result<()>
```

**DoubleLinkedList 结构体方法：**

```rust
impl DoubleLinkedList {
    /// 在链表头部插入节点
    pub fn insert_head(&mut self, value: impl Into<Value>) -> Result<i64>;

    /// 在链表尾部插入节点
    pub fn insert_tail(&mut self, value: impl Into<Value>) -> Result<i64>;

    /// 在指定节点前插入新节点
    ///
    /// # 错误
    ///
    /// 当指定节点不存在时返回 `StructureError::NodeNotFound`
    pub fn insert_before(&mut self, node_id: i64, value: impl Into<Value>) -> Result<i64>;

    /// 在指定节点后插入新节点
    ///
    /// # 错误
    ///
    /// 当指定节点不存在时返回 `StructureError::NodeNotFound`
    pub fn insert_after(&mut self, node_id: i64, value: impl Into<Value>) -> Result<i64>;

    /// 删除指定节点
    ///
    /// # 错误
    ///
    /// 当指定节点不存在时返回 `StructureError::NodeNotFound`
    pub fn delete(&mut self, node_id: i64) -> Result<()>;

    /// 删除头节点
    ///
    /// # 错误
    ///
    /// 当链表为空时返回 `StructureError::EmptyStructure`
    pub fn delete_head(&mut self) -> Result<()>;

    /// 删除尾节点
    ///
    /// # 错误
    ///
    /// 当链表为空时返回 `StructureError::EmptyStructure`
    pub fn delete_tail(&mut self) -> Result<()>;

    /// 反转链表
    pub fn reverse(&mut self) -> Result<()>;
}
```

**示例：**

```rust
use ds4viz::prelude::*;

fn main() -> ds4viz::Result<()> {
    ds4viz::double_linked_list("demo_dlist", |dlist| {
        let node_a = dlist.insert_head(10)?;
        dlist.insert_tail(30)?;

        dlist.insert_after(node_a, 20)?;

        dlist.delete_tail()?;
        dlist.reverse()?;
        Ok(())
    })
}
```

---

### 树结构

#### `binary_tree`

```rust
/// 创建二叉树实例并在闭包中操作
///
/// # 参数
///
/// * `label` - 二叉树的标签
/// * `f` - 操作闭包
pub fn binary_tree<F>(label: &str, f: F) -> Result<()>
where
    F: FnOnce(&mut BinaryTree) -> Result<()>
```

**BinaryTree 结构体方法：**

```rust
impl BinaryTree {
    /// 插入根节点
    ///
    /// # 错误
    ///
    /// 当根节点已存在时返回 `StructureError::RootExists`
    pub fn insert_root(&mut self, value: impl Into<Value>) -> Result<i64>;

    /// 在指定父节点的左侧插入子节点
    ///
    /// # 错误
    ///
    /// * 当父节点不存在时返回 `StructureError::NodeNotFound`
    /// * 当左子节点已存在时返回 `StructureError::ChildExists`
    pub fn insert_left(&mut self, parent_id: i64, value: impl Into<Value>) -> Result<i64>;

    /// 在指定父节点的右侧插入子节点
    ///
    /// # 错误
    ///
    /// * 当父节点不存在时返回 `StructureError::NodeNotFound`
    /// * 当右子节点已存在时返回 `StructureError::ChildExists`
    pub fn insert_right(&mut self, parent_id: i64, value: impl Into<Value>) -> Result<i64>;

    /// 删除指定节点及其所有子树
    ///
    /// # 错误
    ///
    /// 当节点不存在时返回 `StructureError::NodeNotFound`
    pub fn delete(&mut self, node_id: i64) -> Result<()>;

    /// 更新节点的值
    ///
    /// # 错误
    ///
    /// 当节点不存在时返回 `StructureError::NodeNotFound`
    pub fn update_value(&mut self, node_id: i64, value: impl Into<Value>) -> Result<()>;
}
```

**示例：**

```rust
use ds4viz::prelude::*;

fn main() -> ds4viz::Result<()> {
    ds4viz::binary_tree("demo_tree", |tree| {
        let root = tree.insert_root(10)?;
        let left = tree.insert_left(root, 5)?;
        let right = tree.insert_right(root, 15)?;

        tree.insert_left(left, 3)?;
        tree.insert_right(left, 7)?;

        tree.update_value(right, 20)?;
        tree.delete(left)?;
        Ok(())
    })
}
```

#### `binary_search_tree`

```rust
/// 创建二叉搜索树实例并在闭包中操作
///
/// # 参数
///
/// * `label` - 二叉搜索树的标签
/// * `f` - 操作闭包
pub fn binary_search_tree<F>(label: &str, f: F) -> Result<()>
where
    F: FnOnce(&mut BinarySearchTree) -> Result<()>
```

**BinarySearchTree 结构体方法：**

```rust
impl BinarySearchTree {
    /// 插入节点，自动维护二叉搜索树性质
    ///
    /// # 参数
    ///
    /// * `value` - 要插入的值，必须是可比较类型（i64 或 f64）
    ///
    /// # 返回
    ///
    /// 返回新插入节点的 id
    pub fn insert(&mut self, value: impl Into<NumericValue>) -> Result<i64>;

    /// 删除节点，自动维护二叉搜索树性质
    ///
    /// # 错误
    ///
    /// 当节点不存在时返回 `StructureError::ValueNotFound`
    pub fn delete(&mut self, value: impl Into<NumericValue>) -> Result<()>;
}
```

**示例：**

```rust
use ds4viz::prelude::*;

fn main() -> ds4viz::Result<()> {
    ds4viz::binary_search_tree("demo_bst", |bst| {
        bst.insert(10)?;
        bst.insert(5)?;
        bst.insert(15)?;
        bst.insert(3)?;
        bst.insert(7)?;

        bst.delete(5)?;
        bst.insert(6)?;
        Ok(())
    })
}
```

#### `heap`

```rust
/// 创建堆实例并在闭包中操作
///
/// # 参数
///
/// * `label` - 堆的标签
/// * `heap_type` - 堆类型
/// * `f` - 操作闭包
pub fn heap<F>(label: &str, heap_type: HeapType, f: F) -> Result<()>
where
    F: FnOnce(&mut Heap) -> Result<()>
```

**HeapType 枚举：**

```rust
/// 堆类型
pub enum HeapType {
    /// 最小堆
    Min,
    /// 最大堆
    Max,
}
```

**Heap 结构体方法：**

```rust
impl Heap {
    /// 插入元素，自动维护堆性质
    ///
    /// # 参数
    ///
    /// * `value` - 要插入的值，必须是可比较类型（i64 或 f64）
    pub fn insert(&mut self, value: impl Into<NumericValue>) -> Result<()>;

    /// 提取堆顶元素，自动维护堆性质
    ///
    /// # 错误
    ///
    /// 当堆为空时返回 `StructureError::EmptyStructure`
    pub fn extract(&mut self) -> Result<()>;

    /// 清空堆
    pub fn clear(&mut self) -> Result<()>;
}
```

**示例：**

```rust
use ds4viz::prelude::*;

fn main() -> ds4viz::Result<()> {
    // 最小堆
    ds4viz::heap("demo_min_heap", HeapType::Min, |min_heap| {
        min_heap.insert(10)?;
        min_heap.insert(5)?;
        min_heap.insert(15)?;
        min_heap.insert(3)?;

        min_heap.extract()?; // 提取最小值 3
        Ok(())
    })?;

    // 最大堆
    ds4viz::heap("demo_max_heap", HeapType::Max, |max_heap| {
        max_heap.insert(10)?;
        max_heap.insert(5)?;
        max_heap.insert(15)?;
        max_heap.insert(3)?;

        max_heap.extract()?; // 提取最大值 15
        Ok(())
    })
}
```

---

### 图结构

#### `graph_undirected`

```rust
/// 创建无向图实例并在闭包中操作
///
/// # 参数
///
/// * `label` - 无向图的标签
/// * `f` - 操作闭包
pub fn graph_undirected<F>(label: &str, f: F) -> Result<()>
where
    F: FnOnce(&mut GraphUndirected) -> Result<()>
```

**GraphUndirected 结构体方法：**

```rust
impl GraphUndirected {
    /// 添加节点
    ///
    /// # 参数
    ///
    /// * `node_id` - 节点 id
    /// * `label` - 节点标签，长度限制为 1-32 字符
    ///
    /// # 错误
    ///
    /// * 当节点已存在时返回 `StructureError::NodeExists`
    /// * 当标签长度无效时返回 `StructureError::InvalidLabel`
    pub fn add_node(&mut self, node_id: i64, label: &str) -> Result<()>;

    /// 添加无向边，内部自动规范化为 from_id < to_id
    ///
    /// # 错误
    ///
    /// * 当节点不存在时返回 `StructureError::NodeNotFound`
    /// * 当边已存在时返回 `StructureError::EdgeExists`
    /// * 当形成自环时返回 `StructureError::SelfLoop`
    pub fn add_edge(&mut self, from_id: i64, to_id: i64) -> Result<()>;

    /// 删除节点及其相关的所有边
    ///
    /// # 错误
    ///
    /// 当节点不存在时返回 `StructureError::NodeNotFound`
    pub fn remove_node(&mut self, node_id: i64) -> Result<()>;

    /// 删除边
    ///
    /// # 错误
    ///
    /// 当边不存在时返回 `StructureError::EdgeNotFound`
    pub fn remove_edge(&mut self, from_id: i64, to_id: i64) -> Result<()>;

    /// 更新节点标签
    ///
    /// # 错误
    ///
    /// * 当节点不存在时返回 `StructureError::NodeNotFound`
    /// * 当标签长度无效时返回 `StructureError::InvalidLabel`
    pub fn update_node_label(&mut self, node_id: i64, label: &str) -> Result<()>;
}
```

**示例：**

```rust
use ds4viz::prelude::*;

fn main() -> ds4viz::Result<()> {
    ds4viz::graph_undirected("demo_graph", |graph| {
        graph.add_node(0, "A")?;
        graph.add_node(1, "B")?;
        graph.add_node(2, "C")?;

        graph.add_edge(0, 1)?;
        graph.add_edge(1, 2)?;

        graph.update_node_label(1, "B_updated")?;
        graph.remove_edge(0, 1)?;
        Ok(())
    })
}
```

#### `graph_directed`

```rust
/// 创建有向图实例并在闭包中操作
///
/// # 参数
///
/// * `label` - 有向图的标签
/// * `f` - 操作闭包
pub fn graph_directed<F>(label: &str, f: F) -> Result<()>
where
    F: FnOnce(&mut GraphDirected) -> Result<()>
```

**GraphDirected 结构体方法：**

```rust
impl GraphDirected {
    /// 添加节点
    pub fn add_node(&mut self, node_id: i64, label: &str) -> Result<()>;

    /// 添加有向边
    ///
    /// # 错误
    ///
    /// * 当节点不存在时返回 `StructureError::NodeNotFound`
    /// * 当边已存在时返回 `StructureError::EdgeExists`
    /// * 当形成自环时返回 `StructureError::SelfLoop`
    pub fn add_edge(&mut self, from_id: i64, to_id: i64) -> Result<()>;

    /// 删除节点及其相关的所有边
    pub fn remove_node(&mut self, node_id: i64) -> Result<()>;

    /// 删除边
    pub fn remove_edge(&mut self, from_id: i64, to_id: i64) -> Result<()>;

    /// 更新节点标签
    pub fn update_node_label(&mut self, node_id: i64, label: &str) -> Result<()>;
}
```

**示例：**

```rust
use ds4viz::prelude::*;

fn main() -> ds4viz::Result<()> {
    ds4viz::graph_directed("demo_digraph", |graph| {
        graph.add_node(0, "A")?;
        graph.add_node(1, "B")?;
        graph.add_node(2, "C")?;

        graph.add_edge(0, 1)?;
        graph.add_edge(1, 2)?;
        graph.add_edge(2, 0)?;

        graph.update_node_label(1, "B_updated")?;
        graph.remove_edge(2, 0)?;
        Ok(())
    })
}
```

#### `graph_weighted`

```rust
/// 创建带权图实例并在闭包中操作
///
/// # 参数
///
/// * `label` - 带权图的标签
/// * `directed` - 是否为有向图，`false` 表示无向图
/// * `f` - 操作闭包
pub fn graph_weighted<F>(label: &str, directed: bool, f: F) -> Result<()>
where
    F: FnOnce(&mut GraphWeighted) -> Result<()>
```

**GraphWeighted 结构体方法：**

```rust
impl GraphWeighted {
    /// 添加节点
    pub fn add_node(&mut self, node_id: i64, label: &str) -> Result<()>;

    /// 添加带权边
    ///
    /// # 错误
    ///
    /// * 当节点不存在时返回 `StructureError::NodeNotFound`
    /// * 当边已存在时返回 `StructureError::EdgeExists`
    /// * 当形成自环时返回 `StructureError::SelfLoop`
    pub fn add_edge(&mut self, from_id: i64, to_id: i64, weight: f64) -> Result<()>;

    /// 删除节点及其相关的所有边
    pub fn remove_node(&mut self, node_id: i64) -> Result<()>;

    /// 删除边
    pub fn remove_edge(&mut self, from_id: i64, to_id: i64) -> Result<()>;

    /// 更新边的权重
    ///
    /// # 错误
    ///
    /// 当边不存在时返回 `StructureError::EdgeNotFound`
    pub fn update_weight(&mut self, from_id: i64, to_id: i64, weight: f64) -> Result<()>;

    /// 更新节点标签
    pub fn update_node_label(&mut self, node_id: i64, label: &str) -> Result<()>;
}
```

**示例：**

```rust
use ds4viz::prelude::*;

fn main() -> ds4viz::Result<()> {
    // 无向带权图
    ds4viz::graph_weighted("demo_weighted_graph", false, |graph| {
        graph.add_node(0, "A")?;
        graph.add_node(1, "B")?;
        graph.add_node(2, "C")?;

        graph.add_edge(0, 1, 3.5)?;
        graph.add_edge(1, 2, 2.0)?;
        graph.add_edge(2, 0, 4.2)?;

        graph.update_weight(0, 1, 5.0)?;
        graph.remove_edge(2, 0)?;
        Ok(())
    })?;

    // 有向带权图
    ds4viz::graph_weighted("demo_directed_weighted", true, |graph| {
        graph.add_node(0, "Start")?;
        graph.add_node(1, "Middle")?;
        graph.add_node(2, "End")?;

        graph.add_edge(0, 1, 1.5)?;
        graph.add_edge(1, 2, 2.5)?;
        Ok(())
    })
}
```

---

### 自定义输出路径

每个数据结构函数都有对应的 `_with_output` 变体，允许指定输出文件路径：

```rust
use ds4viz::prelude::*;

fn main() -> ds4viz::Result<()> {
    ds4viz::stack_with_output("demo_stack", "custom_output.toml", |s| {
        s.push(10)?;
        s.push(20)?;
        Ok(())
    })
}
```

---

### 异常处理

所有数据结构在遇到错误时都会返回 `Result<T, Ds4VizError>`，并在闭包退出时自动生成包含 `[error]` 部分的 `.toml` 文件。

**错误类型：**

```rust
/// ds4viz 错误类型
#[derive(Debug, thiserror::Error)]
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
#[derive(Debug, thiserror::Error)]
pub enum StructureError {
    #[error("Structure is empty")]
    EmptyStructure,

    #[error("Node not found: {0}")]
    NodeNotFound(i64),

    #[error("Node already exists: {0}")]
    NodeExists(i64),

    #[error("Edge not found: ({0}, {1})")]
    EdgeNotFound(i64, i64),

    #[error("Edge already exists: ({0}, {1})")]
    EdgeExists(i64, i64),

    #[error("Self-loop not allowed: {0}")]
    SelfLoop(i64),

    #[error("Root node already exists")]
    RootExists,

    #[error("Child node already exists")]
    ChildExists,

    #[error("Value not found: {0}")]
    ValueNotFound(String),

    #[error("Invalid label length: expected 1-32, got {0}")]
    InvalidLabel(usize),
}
```

**示例：**

```rust
use ds4viz::prelude::*;

fn main() {
    let result = ds4viz::binary_tree("demo", |tree| {
        let root = tree.insert_root(10)?;
        tree.insert_left(999, 5)?; // 父节点不存在，返回错误
        Ok(())
    });

    if let Err(e) = result {
        eprintln!("发生错误: {}", e);
    }

    // 即使发生错误，trace.toml 仍会生成，包含 [error] 部分
}
```

生成的 `trace.toml` 将包含：

```toml
[error]
type = "runtime"
message = "Node not found: 999"
line = 18
step = 1
last_state = 1
```

---

## 值类型

根据 IR 定义，`value` 字段支持以下类型：

```rust
/// 值类型枚举
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
}

/// 数值类型（用于 BST 和 Heap）
#[derive(Debug, Clone, PartialEq, PartialOrd)]
pub enum NumericValue {
    /// 整数
    Integer(i64),
    /// 浮点数
    Float(f64),
}
```

支持 `From` trait 自动转换：

```rust
use ds4viz::prelude::*;

ds4viz::stack("demo", |s| {
    s.push(42)?;           // i64
    s.push(3.14)?;         // f64
    s.push("hello")?;      // &str -> String
    s.push(true)?;         // bool
    Ok(())
});
```

---

## Cargo Features

```toml
[features]
default = []

# 启用 serde 支持
serde = ["dep:serde"]

# 启用异步 IO
async = ["dep:tokio"]
```
