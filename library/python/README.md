# `ds4viz`  

`ds4viz`是一个`Python`库, 提供了`Python`语言的`ds4viz`支持.  
`ds4viz`是一个可视化数据结构工具. 参见: [GitHub](https://github.com/Water-Run/ds4viz)  

## 安装  

`ds4viz`发布在[PyPi](https://pypi.org/project/ds4viz/)上:  

```bash
pip install ds4viz
```

## 测试  

在目录下运行:  

```bash
pytest -v
```

## 快速上手  

以下脚本提供了一个快速使用示例, 演示如何使用`ds4viz`生成`trace.toml`, 并在发生异常时仍然产出包含`[error]`的`.toml`IR文件:  

```python
import ds4viz as dv  # pip install ds4viz

def main() -> None:
    # 使用上下文: 确保无论成功/失败, 都会生成 trace.toml (成功 => [result], 异常 => [error])
    with dv.graph_undirected() as graph:
        # 添加节点
        graph.add_node(0, "A")
        graph.add_node(1, "B")
        graph.add_node(2, "C")

        # 添加无向边(内部会规范化为 from < to)
        graph.add_edge(0, 1)
        graph.add_edge(1, 2)

        # 也可以在这里继续调用更多 API, 每次操作都会记录到 trace 中

    print("已生成 trace.toml")

if __name__ == "__main__":
    main()
```

## API参考  

### 全局配置

#### `config`

```python
def config(
    output_path: str = "trace.toml",
    title: str = "",
    author: str = "",
    comment: str = ""
) -> None:
    r"""
    配置全局参数
    
    :param output_path: 输出文件路径, 默认为 "trace.toml"
    :param title: 可视化标题
    :param author: 作者信息
    :param comment: 注释说明
    :return None: 无返回值
    """
```

**示例:**

```python
import ds4viz as dv

dv.config(
    output_path="my_trace.toml",
    title="栈操作演示",
    author="WaterRun",
    comment="演示基本的栈操作"
)
```

---

### 线性结构

#### `stack`

```python
def stack(
    label: str = "stack",
    output: str | None = None
) -> Stack:
    r"""
    创建栈实例
    
    :param label: 栈的标签, 默认为 "stack"
    :param output: 输出文件路径, 若为 None 则使用全局配置
    :return Stack: 栈实例
    """
```

**Stack 类方法:**

```python
class Stack:
    r"""
    栈数据结构
    """
    
    def push(self, value: int | float | str | bool) -> None:
        r"""
        压栈操作
        
        :param value: 要压入的值
        :return None: 无返回值
        """
    
    def pop(self) -> None:
        r"""
        弹栈操作
        
        :return None: 无返回值
        :raise RuntimeError: 当栈为空时抛出异常
        """
    
    def clear(self) -> None:
        r"""
        清空栈
        
        :return None: 无返回值
        """
```

**示例:**

```python
import ds4viz as dv

with dv.stack(label="demo_stack") as s:
    s.push(10)
    s.push(20)
    s.push(30)
    
    s.pop()
    s.pop()
```

#### `queue`

```python
def queue(
    label: str = "queue",
    output: str | None = None
) -> Queue:
    r"""
    创建队列实例
    
    :param label: 队列的标签, 默认为 "queue"
    :param output: 输出文件路径, 若为 None 则使用全局配置
    :return Queue: 队列实例
    """
```

**Queue 类方法:**

```python
class Queue:
    r"""
    队列数据结构
    """
    
    def enqueue(self, value: int | float | str | bool) -> None:
        r"""
        入队操作
        
        :param value: 要入队的值
        :return None: 无返回值
        """
    
    def dequeue(self) -> None:
        r"""
        出队操作
        
        :return None: 无返回值
        :raise RuntimeError: 当队列为空时抛出异常
        """
    
    def clear(self) -> None:
        r"""
        清空队列
        
        :return None: 无返回值
        """
```

**示例:**

```python
import ds4viz as dv

with dv.queue(label="demo_queue") as q:
    q.enqueue(10)
    q.enqueue(20)
    q.enqueue(30)
    
    q.dequeue()
    q.dequeue()
```

#### `single_linked_list`

```python
def single_linked_list(
    label: str = "slist",
    output: str | None = None
) -> SingleLinkedList:
    r"""
    创建单链表实例
    
    :param label: 单链表的标签, 默认为 "slist"
    :param output: 输出文件路径, 若为 None 则使用全局配置
    :return SingleLinkedList: 单链表实例
    """
```

**SingleLinkedList 类方法:**

```python
class SingleLinkedList:
    r"""
    单链表数据结构
    """
    
    def insert_head(self, value: int | float | str | bool) -> int:
        r"""
        在链表头部插入节点
        
        :param value: 要插入的值
        :return int: 新插入节点的 id
        """
    
    def insert_tail(self, value: int | float | str | bool) -> int:
        r"""
        在链表尾部插入节点
        
        :param value: 要插入的值
        :return int: 新插入节点的 id
        """
    
    def insert_after(self, node_id: int, value: int | float | str | bool) -> int:
        r"""
        在指定节点后插入新节点
        
        :param node_id: 目标节点的 id
        :param value: 要插入的值
        :return int: 新插入节点的 id
        :raise RuntimeError: 当指定节点不存在时抛出异常
        """
    
    def delete(self, node_id: int) -> None:
        r"""
        删除指定节点
        
        :param node_id: 要删除的节点 id
        :return None: 无返回值
        :raise RuntimeError: 当指定节点不存在时抛出异常
        """
    
    def delete_head(self) -> None:
        r"""
        删除头节点
        
        :return None: 无返回值
        :raise RuntimeError: 当链表为空时抛出异常
        """
    
    def reverse(self) -> None:
        r"""
        反转链表
        
        :return None: 无返回值
        """
```

**示例:**

```python
import ds4viz as dv

with dv.single_linked_list(label="demo_slist") as slist:
    node_a = slist.insert_head(10)
    slist.insert_tail(20)
    node_c = slist.insert_tail(30)
    
    slist.insert_after(node_a, 15)
    
    slist.delete(node_c)
    slist.reverse()
```

#### `double_linked_list`

```python
def double_linked_list(
    label: str = "dlist",
    output: str | None = None
) -> DoubleLinkedList:
    r"""
    创建双向链表实例
    
    :param label: 双向链表的标签, 默认为 "dlist"
    :param output: 输出文件路径, 若为 None 则使用全局配置
    :return DoubleLinkedList: 双向链表实例
    """
```

**DoubleLinkedList 类方法:**

```python
class DoubleLinkedList:
    r"""
    双向链表数据结构
    """
    
    def insert_head(self, value: int | float | str | bool) -> int:
        r"""
        在链表头部插入节点
        
        :param value: 要插入的值
        :return int: 新插入节点的 id
        """
    
    def insert_tail(self, value: int | float | str | bool) -> int:
        r"""
        在链表尾部插入节点
        
        :param value: 要插入的值
        :return int: 新插入节点的 id
        """
    
    def insert_before(self, node_id: int, value: int | float | str | bool) -> int:
        r"""
        在指定节点前插入新节点
        
        :param node_id: 目标节点的 id
        :param value: 要插入的值
        :return int: 新插入节点的 id
        :raise RuntimeError: 当指定节点不存在时抛出异常
        """
    
    def insert_after(self, node_id: int, value: int | float | str | bool) -> int:
        r"""
        在指定节点后插入新节点
        
        :param node_id: 目标节点的 id
        :param value: 要插入的值
        :return int: 新插入节点的 id
        :raise RuntimeError: 当指定节点不存在时抛出异常
        """
    
    def delete(self, node_id: int) -> None:
        r"""
        删除指定节点
        
        :param node_id: 要删除的节点 id
        :return None: 无返回值
        :raise RuntimeError: 当指定节点不存在时抛出异常
        """
    
    def delete_head(self) -> None:
        r"""
        删除头节点
        
        :return None: 无返回值
        :raise RuntimeError: 当链表为空时抛出异常
        """
    
    def delete_tail(self) -> None:
        r"""
        删除尾节点
        
        :return None: 无返回值
        :raise RuntimeError: 当链表为空时抛出异常
        """
    
    def reverse(self) -> None:
        r"""
        反转链表
        
        :return None: 无返回值
        """
```

**示例:**

```python
import ds4viz as dv

with dv.double_linked_list(label="demo_dlist") as dlist:
    node_a = dlist.insert_head(10)
    dlist.insert_tail(30)
    
    dlist.insert_after(node_a, 20)
    
    dlist.delete_tail()
    dlist.reverse()
```

---

### 树结构

#### `binary_tree`

```python
def binary_tree(
    label: str = "binary_tree",
    output: str | None = None
) -> BinaryTree:
    r"""
    创建二叉树实例
    
    :param label: 二叉树的标签, 默认为 "binary_tree"
    :param output: 输出文件路径, 若为 None 则使用全局配置
    :return BinaryTree: 二叉树实例
    """
```

**BinaryTree 类方法:**

```python
class BinaryTree:
    r"""
    二叉树数据结构
    """
    
    def insert_root(self, value: int | float | str | bool) -> int:
        r"""
        插入根节点
        
        :param value: 根节点的值
        :return int: 根节点的 id
        :raise RuntimeError: 当根节点已存在时抛出异常
        """
    
    def insert_left(self, parent_id: int, value: int | float | str | bool) -> int:
        r"""
        在指定父节点的左侧插入子节点
        
        :param parent_id: 父节点的 id
        :param value: 要插入的子节点值
        :return int: 新插入节点的 id
        :raise RuntimeError: 当父节点不存在或左子节点已存在时抛出异常
        """
    
    def insert_right(self, parent_id: int, value: int | float | str | bool) -> int:
        r"""
        在指定父节点的右侧插入子节点
        
        :param parent_id: 父节点的 id
        :param value: 要插入的子节点值
        :return int: 新插入节点的 id
        :raise RuntimeError: 当父节点不存在或右子节点已存在时抛出异常
        """
    
    def delete(self, node_id: int) -> None:
        r"""
        删除指定节点及其所有子树
        
        :param node_id: 要删除的节点 id
        :return None: 无返回值
        :raise RuntimeError: 当节点不存在时抛出异常
        """
    
    def update_value(self, node_id: int, value: int | float | str | bool) -> None:
        r"""
        更新节点的值
        
        :param node_id: 节点 id
        :param value: 新的值
        :return None: 无返回值
        :raise RuntimeError: 当节点不存在时抛出异常
        """
```

**示例:**

```python
import ds4viz as dv

with dv.binary_tree(label="demo_tree") as tree:
    root = tree.insert_root(10)
    left = tree.insert_left(root, 5)
    right = tree.insert_right(root, 15)
    
    tree.insert_left(left, 3)
    tree.insert_right(left, 7)
    
    tree.update_value(right, 20)
    tree.delete(left)
```

#### `binary_search_tree`

```python
def binary_search_tree(
    label: str = "bst",
    output: str | None = None
) -> BinarySearchTree:
    r"""
    创建二叉搜索树实例
    
    :param label: 二叉搜索树的标签, 默认为 "bst"
    :param output: 输出文件路径, 若为 None 则使用全局配置
    :return BinarySearchTree: 二叉搜索树实例
    """
```

**BinarySearchTree 类方法:**

```python
class BinarySearchTree:
    r"""
    二叉搜索树数据结构
    """
    
    def insert(self, value: int | float) -> int:
        r"""
        插入节点, 自动维护二叉搜索树性质
        
        :param value: 要插入的值
        :return int: 新插入节点的 id
        """
    
    def delete(self, value: int | float) -> None:
        r"""
        删除节点, 自动维护二叉搜索树性质
        
        :param value: 要删除的节点值
        :return None: 无返回值
        :raise RuntimeError: 当节点不存在时抛出异常
        """
```

**示例:**

```python
import ds4viz as dv

with dv.binary_search_tree(label="demo_bst") as bst:
    bst.insert(10)
    bst.insert(5)
    bst.insert(15)
    bst.insert(3)
    bst.insert(7)
    
    bst.delete(5)
    bst.insert(6)
```

#### `heap`

```python
def heap(
    label: str = "heap",
    heap_type: str = "min",
    output: str | None = None
) -> Heap:
    r"""
    创建堆实例
    
    :param label: 堆的标签, 默认为 "heap"
    :param heap_type: 堆类型, "min" 表示最小堆, "max" 表示最大堆, 默认为 "min"
    :param output: 输出文件路径, 若为 None 则使用全局配置
    :return Heap: 堆实例
    :raise ValueError: 当 heap_type 不是 "min" 或 "max" 时抛出异常
    """
```

**Heap 类方法:**

```python
class Heap:
    r"""
    堆数据结构
    """
    
    def insert(self, value: int | float) -> None:
        r"""
        插入元素, 自动维护堆性质
        
        :param value: 要插入的值
        :return None: 无返回值
        """
    
    def extract(self) -> None:
        r"""
        提取堆顶元素, 自动维护堆性质
        
        :return None: 无返回值
        :raise RuntimeError: 当堆为空时抛出异常
        """
    
    def clear(self) -> None:
        r"""
        清空堆
        
        :return None: 无返回值
        """
```

**示例:**

```python
import ds4viz as dv

# 最小堆
with dv.heap(label="demo_min_heap", heap_type="min") as min_heap:
    min_heap.insert(10)
    min_heap.insert(5)
    min_heap.insert(15)
    min_heap.insert(3)
    
    min_heap.extract()  # 提取最小值 3

# 最大堆
with dv.heap(label="demo_max_heap", heap_type="max") as max_heap:
    max_heap.insert(10)
    max_heap.insert(5)
    max_heap.insert(15)
    max_heap.insert(3)
    
    max_heap.extract()  # 提取最大值 15
```

---

### 图结构

#### `graph_undirected`

```python
def graph_undirected(
    label: str = "graph",
    output: str | None = None
) -> GraphUndirected:
    r"""
    创建无向图实例
    
    :param label: 无向图的标签, 默认为 "graph"
    :param output: 输出文件路径, 若为 None 则使用全局配置
    :return GraphUndirected: 无向图实例
    """
```

**GraphUndirected 类方法:**

```python
class GraphUndirected:
    r"""
    无向图数据结构
    """
    
    def add_node(self, node_id: int, label: str) -> None:
        r"""
        添加节点
        
        :param node_id: 节点 id
        :param label: 节点标签, 长度限制为 1-32 字符
        :return None: 无返回值
        :raise RuntimeError: 当节点已存在时抛出异常
        """
    
    def add_edge(self, from_id: int, to_id: int) -> None:
        r"""
        添加无向边, 内部自动规范化为 from_id < to_id
        
        :param from_id: 起始节点 id
        :param to_id: 终止节点 id
        :return None: 无返回值
        :raise RuntimeError: 当节点不存在, 边已存在或形成自环时抛出异常
        """
    
    def remove_node(self, node_id: int) -> None:
        r"""
        删除节点及其相关的所有边
        
        :param node_id: 要删除的节点 id
        :return None: 无返回值
        :raise RuntimeError: 当节点不存在时抛出异常
        """
    
    def remove_edge(self, from_id: int, to_id: int) -> None:
        r"""
        删除边
        
        :param from_id: 起始节点 id
        :param to_id: 终止节点 id
        :return None: 无返回值
        :raise RuntimeError: 当边不存在时抛出异常
        """
    
    def update_node_label(self, node_id: int, label: str) -> None:
        r"""
        更新节点标签
        
        :param node_id: 节点 id
        :param label: 新的节点标签, 长度限制为 1-32 字符
        :return None: 无返回值
        :raise RuntimeError: 当节点不存在时抛出异常
        """
```

**示例:**

```python
import ds4viz as dv

with dv.graph_undirected(label="demo_graph") as graph:
    graph.add_node(0, "A")
    graph.add_node(1, "B")
    graph.add_node(2, "C")
    
    graph.add_edge(0, 1)
    graph.add_edge(1, 2)
    
    graph.update_node_label(1, "B_updated")
    graph.remove_edge(0, 1)
```

#### `graph_directed`

```python
def graph_directed(
    label: str = "graph",
    output: str | None = None
) -> GraphDirected:
    r"""
    创建有向图实例
    
    :param label: 有向图的标签, 默认为 "graph"
    :param output: 输出文件路径, 若为 None 则使用全局配置
    :return GraphDirected: 有向图实例
    """
```

**GraphDirected 类方法:**

```python
class GraphDirected:
    r"""
    有向图数据结构
    """
    
    def add_node(self, node_id: int, label: str) -> None:
        r"""
        添加节点
        
        :param node_id: 节点 id
        :param label: 节点标签, 长度限制为 1-32 字符
        :return None: 无返回值
        :raise RuntimeError: 当节点已存在时抛出异常
        """
    
    def add_edge(self, from_id: int, to_id: int) -> None:
        r"""
        添加有向边
        
        :param from_id: 起始节点 id
        :param to_id: 终止节点 id
        :return None: 无返回值
        :raise RuntimeError: 当节点不存在, 边已存在或形成自环时抛出异常
        """
    
    def remove_node(self, node_id: int) -> None:
        r"""
        删除节点及其相关的所有边
        
        :param node_id: 要删除的节点 id
        :return None: 无返回值
        :raise RuntimeError: 当节点不存在时抛出异常
        """
    
    def remove_edge(self, from_id: int, to_id: int) -> None:
        r"""
        删除边
        
        :param from_id: 起始节点 id
        :param to_id: 终止节点 id
        :return None: 无返回值
        :raise RuntimeError: 当边不存在时抛出异常
        """
    
    def update_node_label(self, node_id: int, label: str) -> None:
        r"""
        更新节点标签
        
        :param node_id: 节点 id
        :param label: 新的节点标签, 长度限制为 1-32 字符
        :return None: 无返回值
        :raise RuntimeError: 当节点不存在时抛出异常
        """
```

**示例:**

```python
import ds4viz as dv

with dv.graph_directed(label="demo_digraph") as graph:
    graph.add_node(0, "A")
    graph.add_node(1, "B")
    graph.add_node(2, "C")
    
    graph.add_edge(0, 1)
    graph.add_edge(1, 2)
    graph.add_edge(2, 0)
    
    graph.update_node_label(1, "B_updated")
    graph.remove_edge(2, 0)
```

#### `graph_weighted`

```python
def graph_weighted(
    label: str = "graph",
    directed: bool = False,
    output: str | None = None
) -> GraphWeighted:
    r"""
    创建带权图实例
    
    :param label: 带权图的标签, 默认为 "graph"
    :param directed: 是否为有向图, 默认为 False (无向图)
    :param output: 输出文件路径, 若为 None 则使用全局配置
    :return GraphWeighted: 带权图实例
    """
```

**GraphWeighted 类方法:**

```python
class GraphWeighted:
    r"""
    带权图数据结构
    """
    
    def add_node(self, node_id: int, label: str) -> None:
        r"""
        添加节点
        
        :param node_id: 节点 id
        :param label: 节点标签, 长度限制为 1-32 字符
        :return None: 无返回值
        :raise RuntimeError: 当节点已存在时抛出异常
        """
    
    def add_edge(self, from_id: int, to_id: int, weight: float) -> None:
        r"""
        添加带权边
        
        :param from_id: 起始节点 id
        :param to_id: 终止节点 id
        :param weight: 边的权重
        :return None: 无返回值
        :raise RuntimeError: 当节点不存在, 边已存在或形成自环时抛出异常
        """
    
    def remove_node(self, node_id: int) -> None:
        r"""
        删除节点及其相关的所有边
        
        :param node_id: 要删除的节点 id
        :return None: 无返回值
        :raise RuntimeError: 当节点不存在时抛出异常
        """
    
    def remove_edge(self, from_id: int, to_id: int) -> None:
        r"""
        删除边
        
        :param from_id: 起始节点 id
        :param to_id: 终止节点 id
        :return None: 无返回值
        :raise RuntimeError: 当边不存在时抛出异常
        """
    
    def update_weight(self, from_id: int, to_id: int, weight: float) -> None:
        r"""
        更新边的权重
        
        :param from_id: 起始节点 id
        :param to_id: 终止节点 id
        :param weight: 新的权重值
        :return None: 无返回值
        :raise RuntimeError: 当边不存在时抛出异常
        """
    
    def update_node_label(self, node_id: int, label: str) -> None:
        r"""
        更新节点标签
        
        :param node_id: 节点 id
        :param label: 新的节点标签, 长度限制为 1-32 字符
        :return None: 无返回值
        :raise RuntimeError: 当节点不存在时抛出异常
        """
```

**示例:**

```python
import ds4viz as dv

# 无向带权图
with dv.graph_weighted(label="demo_weighted_graph", directed=False) as graph:
    graph.add_node(0, "A")
    graph.add_node(1, "B")
    graph.add_node(2, "C")
    
    graph.add_edge(0, 1, 3.5)
    graph.add_edge(1, 2, 2.0)
    graph.add_edge(2, 0, 4.2)
    
    graph.update_weight(0, 1, 5.0)
    graph.remove_edge(2, 0)

# 有向带权图
with dv.graph_weighted(label="demo_directed_weighted", directed=True) as graph:
    graph.add_node(0, "Start")
    graph.add_node(1, "Middle")
    graph.add_node(2, "End")
    
    graph.add_edge(0, 1, 1.5)
    graph.add_edge(1, 2, 2.5)
```

---

### 异常处理

所有数据结构在遇到错误时都会抛出 `RuntimeError`, 并在退出上下文时自动生成包含 `[error]` 部分的 `.toml` 文件.

**示例:**

```python
import ds4viz as dv

try:
    with dv.binary_tree() as tree:
        root = tree.insert_root(10)
        tree.insert_left(999, 5)  # 父节点不存在, 抛出异常
except RuntimeError as e:
    print(f"发生错误: {e}")

# 即使发生异常, trace.toml 仍会生成, 包含 [error] 部分
```

生成的 `trace.toml` 将包含:

```toml
[error]
type = "runtime"
message = "Parent node not found: 999"
line = 18
step = 1
last_state = 1
```

---

## 类型约束

根据 IR 定义, `value` 字段仅支持以下类型:

- `int`: 整数
- `float`: 浮点数
- `str`: 字符串
- `bool`: 布尔值

所有 API 在类型注解中都严格遵循此约束.