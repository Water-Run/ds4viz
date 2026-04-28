# `ds4viz`

以下文档描述 `ds4viz` 项目 Python 库的有关实现.

参见: [GitHub](https://github.com/Water-Run/ds4viz)

## 安装

`ds4viz` (计划)发布在 [PyPI](https://pypi.org/project/ds4viz/) 上:

```bash
pip install ds4viz
```

当前, 你可以在项目目录下:  

```bash
pip install .
```

## 测试

在项目目录下运行:

```bash
pytest -v
```

## 快速上手

以下示例演示如何构建一棵二叉树并记录中序遍历过程:

```python
import ds4viz as dv

dv.config(title="二叉树中序遍历", author="WaterRun")

with dv.binary_tree() as tree:
    root = tree.insert_root(10)
    left = tree.insert_left(root, 5)
    right = tree.insert_right(root, 15)

    tree.step(note="访问节点 5", highlights=[dv.node(left, "focus")])
    tree.step(note="访问节点 10", highlights=[
        dv.node(left, "visited"), dv.node(root, "focus")
    ])
    tree.step(note="访问节点 15", highlights=[
        dv.node(left, "visited"), dv.node(root, "visited"),
        dv.node(right, "focus")
    ])

print("已生成 trace.toml")
```

使用上下文管理器 (`with`) 确保无论成功或失败都会生成 `.toml` 文件: 正常退出时包含 `[result]`, 发生异常时包含 `[error]`.

---

## API 参考

### 全局配置

#### `dv.config()`

```python
def config(
    output_path: str = "trace.toml",
    title: str = "",
    author: str = "",
    comment: str = ""
) -> None:
    r"""
    配置全局参数.

    :param output_path: 输出文件路径, 默认为 "trace.toml"
    :param title: 可视化标题 (对应 IR [remarks].title)
    :param author: 作者信息 (对应 IR [remarks].author)
    :param comment: 注释说明 (对应 IR [remarks].comment)
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

### 高亮标记辅助函数

用于构造 `highlights` 数组中的元素, 供 `step()` 和 `amend()` 使用.

#### `dv.node()`

```python
def node(
    target: int,
    style: str = "focus",
    level: int = 1
) -> dict:
    r"""
    创建节点高亮标记.
    适用于含 nodes 的结构: slist, dlist, binary_tree, bst,
    graph_undirected, graph_directed, graph_weighted.

    :param target: 节点 id, 必须引用关联 state 中存在的节点
    :param style: 高亮样式, 仅允许小写字母/数字/下划线/连字符, 长度 1..32
    :param level: 视觉强度等级, 1..9, 默认为 1, 数值越大越醒目
    :return dict: 高亮标记字典
    """
```

#### `dv.item()`

```python
def item(
    target: int,
    style: str = "focus",
    level: int = 1
) -> dict:
    r"""
    创建元素高亮标记.
    仅适用于 stack 和 queue.

    :param target: 元素在 items 数组中的有效索引
    :param style: 高亮样式, 仅允许小写字母/数字/下划线/连字符, 长度 1..32
    :param level: 视觉强度等级, 1..9, 默认为 1, 数值越大越醒目
    :return dict: 高亮标记字典
    """
```

#### `dv.edge()`

```python
def edge(
    from_id: int,
    to_id: int,
    style: str = "focus",
    level: int = 1
) -> dict:
    r"""
    创建边高亮标记.
    仅适用于图结构: graph_undirected, graph_directed, graph_weighted.
    (from_id, to_id) 必须匹配关联 state 中存在的一条边.
    无向图时必须满足 from_id < to_id.

    :param from_id: 起始节点 id
    :param to_id: 终止节点 id
    :param style: 高亮样式, 仅允许小写字母/数字/下划线/连字符, 长度 1..32
    :param level: 视觉强度等级, 1..9, 默认为 1, 数值越大越醒目
    :return dict: 高亮标记字典
    """
```

#### 预定义 `style` 值

渲染器必须支持以下预定义样式. 自定义 `style` 值会回退为 `"focus"` 的表现.

| style       | 语义         |
|-------------|--------------|
| `focus`     | 当前焦点     |
| `visited`   | 已访问       |
| `active`    | 参与当前计算 |
| `comparing` | 正在比较     |
| `found`     | 查找命中     |
| `error`     | 异常状态     |

渲染器看到 `style = "focus"` 时会自动将画布定位到该元素.

---

### 通用实例方法

所有数据结构实例均支持以下方法.

#### `step()`

```python
def step(
    self,
    note: str | None = None,
    highlights: list | None = None,
    line_offset: int = 0
) -> None:
    r"""
    记录一次观察步骤, 不改变数据结构的状态.
    在 IR 中生成 op = "observe"、before == after 的 step.
    适用于遍历、查找等需要逐步展示但不修改结构的场景.

    :param note: 步骤说明, 长度 1..256
    :param highlights: 高亮标记列表, 由 dv.node() / dv.item() / dv.edge() 构造
    :param line_offset: 用户调用栈偏移量.
                        0 表示记录 step() 调用处;
                        1 表示记录 step() 外层封装函数调用处
    :return None: 无返回值
    """
```

#### `amend()`

```python
def amend(
    self,
    note: str | None = None,
    highlights: list | None = None
) -> None:
    r"""
    修改上一步的展示属性. 传入 None 的参数不会修改对应字段.
    适用于在变更操作后补充高亮或说明.

    :param note: 步骤说明, 传入 None 表示不修改
    :param highlights: 高亮标记列表, 传入 None 表示不修改; 传入 [] 表示清除
    :return None: 无返回值
    :raise RuntimeError: 当不存在上一步时抛出异常
    """
```

#### `phase()`

```python
def phase(self, name: str) -> ContextManager:
    r"""
    返回阶段上下文管理器 (可选).
    上下文内产生的所有步骤 (包括变更操作自动产生的步骤和手动调用 step() 产生的步骤)
    都会标记为指定阶段 (对应 IR steps.phase).
    支持嵌套, 内层 phase 优先.

    phase 不是必须的. 不使用 phase 时, 步骤不会携带阶段标记.
    当操作流程存在明确的逻辑划分时, 推荐使用 phase 以便渲染器分段展示.

    :param name: 阶段名称, 长度 1..64
    :return ContextManager: 上下文管理器
    """
```

#### `alias()` (仅含 `nodes` 的结构)

适用于: `SingleLinkedList`, `DoubleLinkedList`, `BinaryTree`, `BinarySearchTree`, `GraphUndirected`, `GraphDirected`, `GraphWeighted`.

```python
def alias(self, node_id: int, name: str | None) -> None:
    r"""
    设置或清除节点别名.
    别名会出现在后续的状态快照中, 供渲染器展示.
    不产生新的步骤.

    :param node_id: 节点 id
    :param name: 别名, 传入 None 表示清除;
                 长度 1..64, 仅允许: a-z, A-Z, 0-9, _, -; 同一 state 内唯一
    :return None: 无返回值
    :raise RuntimeError: 当节点不存在时抛出异常
    """
```

---

### 线性结构

#### `dv.stack()` → `Stack`

```python
def stack(
    label: str = "stack",
    output: str | None = None
) -> Stack:
    r"""
    创建栈实例.

    :param label: 栈的标签 (对应 IR [object].label)
    :param output: 输出文件路径, 若为 None 则使用全局配置
    :return Stack: 栈实例 (上下文管理器)
    """
```

**Stack 类方法:**

```python
class Stack:
    def push(self, value: int | float | str | bool) -> None:
        r"""
        压栈操作.

        :param value: 要压入的值
        :return None: 无返回值
        """

    def pop(self) -> int | float | str | bool:
        r"""
        弹栈操作.

        :return: 弹出的栈顶元素值
        :raise RuntimeError: 当栈为空时抛出异常
        """

    def clear(self) -> None:
        r"""
        清空栈.

        :return None: 无返回值
        """
```

**示例:**

```python
import ds4viz as dv

dv.config(title="栈操作")

with dv.stack(label="演示栈") as s:
    s.push(10)
    s.amend(note="压入 10", highlights=[dv.item(0, "focus", level=3)])

    s.push(20)
    s.amend(note="压入 20", highlights=[
        dv.item(0, "visited"),
        dv.item(1, "focus", level=3)
    ])

    s.push(30)
    s.amend(highlights=[
        dv.item(0, "visited"),
        dv.item(1, "visited"),
        dv.item(2, "focus", level=3)
    ])

    val = s.pop()   # val = 30
    s.amend(note=f"弹出 {val}")

    val = s.pop()   # val = 20
    s.amend(note=f"弹出 {val}", highlights=[
        dv.item(0, "focus", level=3)
    ])
```

#### `dv.queue()` → `Queue`

```python
def queue(
    label: str = "queue",
    output: str | None = None
) -> Queue:
    r"""
    创建队列实例.

    :param label: 队列的标签 (对应 IR [object].label)
    :param output: 输出文件路径, 若为 None 则使用全局配置
    :return Queue: 队列实例 (上下文管理器)
    """
```

**Queue 类方法:**

```python
class Queue:
    def enqueue(self, value: int | float | str | bool) -> None:
        r"""
        入队操作.

        :param value: 要入队的值
        :return None: 无返回值
        """

    def dequeue(self) -> int | float | str | bool:
        r"""
        出队操作.

        :return: 出队的队首元素值
        :raise RuntimeError: 当队列为空时抛出异常
        """

    def clear(self) -> None:
        r"""
        清空队列.

        :return None: 无返回值
        """
```

**示例:**

```python
import ds4viz as dv

dv.config(title="队列操作")

with dv.queue(label="演示队列") as q:
    with q.phase("入队"):
        q.enqueue("甲")
        q.enqueue("乙")
        q.enqueue("丙")

    with q.phase("处理"):
        val = q.dequeue()   # val = "甲"
        q.amend(note=f"处理 {val}", highlights=[dv.item(0, "focus", level=3)])

        val = q.dequeue()   # val = "乙"
        q.amend(note=f"处理 {val}", highlights=[dv.item(0, "focus", level=3)])

        q.enqueue("丁")
        q.amend(note="新任务入队", highlights=[
            dv.item(0, "visited"),
            dv.item(1, "focus", level=3)
        ])
```

#### `dv.single_linked_list()` → `SingleLinkedList`

```python
def single_linked_list(
    label: str = "slist",
    output: str | None = None
) -> SingleLinkedList:
    r"""
    创建单链表实例.

    :param label: 单链表的标签 (对应 IR [object].label)
    :param output: 输出文件路径, 若为 None 则使用全局配置
    :return SingleLinkedList: 单链表实例 (上下文管理器)
    """
```

**SingleLinkedList 类方法:**

```python
class SingleLinkedList:
    def insert_head(self, value: int | float | str | bool) -> int:
        r"""
        在链表头部插入节点.

        :param value: 要插入的值
        :return int: 新插入节点的 id
        """

    def insert_tail(self, value: int | float | str | bool) -> int:
        r"""
        在链表尾部插入节点.

        :param value: 要插入的值
        :return int: 新插入节点的 id
        """

    def insert_after(self, node_id: int, value: int | float | str | bool) -> int:
        r"""
        在指定节点后插入新节点.

        :param node_id: 目标节点的 id
        :param value: 要插入的值
        :return int: 新插入节点的 id
        :raise RuntimeError: 当指定节点不存在时抛出异常
        """

    def delete(self, node_id: int) -> None:
        r"""
        删除指定节点.

        :param node_id: 要删除的节点 id
        :return None: 无返回值
        :raise RuntimeError: 当指定节点不存在时抛出异常
        """

    def delete_head(self) -> None:
        r"""
        删除头节点.

        :return None: 无返回值
        :raise RuntimeError: 当链表为空时抛出异常
        """

    def reverse(self) -> None:
        r"""
        反转链表.

        :return None: 无返回值
        """
```

**示例:**

```python
import ds4viz as dv

dv.config(title="单链表操作")

with dv.single_linked_list(label="演示单链表") as slist:
    a = slist.insert_head(10)
    slist.alias(a, "head")
    b = slist.insert_tail(20)
    c = slist.insert_tail(30)
    slist.amend(highlights=[dv.node(c, "focus", level=3)])

    d = slist.insert_after(a, 15)
    slist.amend(note="在 10 后插入 15", highlights=[
        dv.node(a, "active"),
        dv.node(d, "focus", level=3)
    ])

    slist.delete(c)
    slist.amend(note="删除节点 30")

    slist.reverse()
    slist.amend(note="反转链表")
```

#### `dv.double_linked_list()` → `DoubleLinkedList`

```python
def double_linked_list(
    label: str = "dlist",
    output: str | None = None
) -> DoubleLinkedList:
    r"""
    创建双向链表实例.

    :param label: 双向链表的标签 (对应 IR [object].label)
    :param output: 输出文件路径, 若为 None 则使用全局配置
    :return DoubleLinkedList: 双向链表实例 (上下文管理器)
    """
```

**DoubleLinkedList 类方法:**

```python
class DoubleLinkedList:
    def insert_head(self, value: int | float | str | bool) -> int:
        r"""
        在链表头部插入节点.

        :param value: 要插入的值
        :return int: 新插入节点的 id
        """

    def insert_tail(self, value: int | float | str | bool) -> int:
        r"""
        在链表尾部插入节点.

        :param value: 要插入的值
        :return int: 新插入节点的 id
        """

    def insert_before(self, node_id: int, value: int | float | str | bool) -> int:
        r"""
        在指定节点前插入新节点.

        :param node_id: 目标节点的 id
        :param value: 要插入的值
        :return int: 新插入节点的 id
        :raise RuntimeError: 当指定节点不存在时抛出异常
        """

    def insert_after(self, node_id: int, value: int | float | str | bool) -> int:
        r"""
        在指定节点后插入新节点.

        :param node_id: 目标节点的 id
        :param value: 要插入的值
        :return int: 新插入节点的 id
        :raise RuntimeError: 当指定节点不存在时抛出异常
        """

    def delete(self, node_id: int) -> None:
        r"""
        删除指定节点.

        :param node_id: 要删除的节点 id
        :return None: 无返回值
        :raise RuntimeError: 当指定节点不存在时抛出异常
        """

    def delete_head(self) -> None:
        r"""
        删除头节点.

        :return None: 无返回值
        :raise RuntimeError: 当链表为空时抛出异常
        """

    def delete_tail(self) -> None:
        r"""
        删除尾节点.

        :return None: 无返回值
        :raise RuntimeError: 当链表为空时抛出异常
        """

    def reverse(self) -> None:
        r"""
        反转链表.

        :return None: 无返回值
        """
```

**示例:**

```python
import ds4viz as dv

with dv.double_linked_list(label="演示双链表") as dlist:
    with dlist.phase("构建"):
        a = dlist.insert_head(10)
        dlist.alias(a, "head")
        b = dlist.insert_tail(30)
        c = dlist.insert_before(b, 20)
        dlist.amend(note="在 30 前插入 20", highlights=[
            dv.node(c, "focus", level=3),
            dv.node(b, "active")
        ])

    with dlist.phase("修改"):
        dlist.insert_after(a, 15)
        dlist.delete_tail()
        dlist.reverse()
```

---

### 树结构

#### `dv.binary_tree()` → `BinaryTree`

```python
def binary_tree(
    label: str = "binary_tree",
    output: str | None = None
) -> BinaryTree:
    r"""
    创建二叉树实例.

    :param label: 二叉树的标签 (对应 IR [object].label)
    :param output: 输出文件路径, 若为 None 则使用全局配置
    :return BinaryTree: 二叉树实例 (上下文管理器)
    """
```

**BinaryTree 类方法:**

```python
class BinaryTree:
    def insert_root(self, value: int | float | str | bool) -> int:
        r"""
        插入根节点.

        :param value: 根节点的值
        :return int: 根节点的 id
        :raise RuntimeError: 当根节点已存在时抛出异常
        """

    def insert_left(self, parent_id: int, value: int | float | str | bool) -> int:
        r"""
        在指定父节点的左侧插入子节点.

        :param parent_id: 父节点的 id
        :param value: 要插入的子节点值
        :return int: 新插入节点的 id
        :raise RuntimeError: 当父节点不存在或左子节点已存在时抛出异常
        """

    def insert_right(self, parent_id: int, value: int | float | str | bool) -> int:
        r"""
        在指定父节点的右侧插入子节点.

        :param parent_id: 父节点的 id
        :param value: 要插入的子节点值
        :return int: 新插入节点的 id
        :raise RuntimeError: 当父节点不存在或右子节点已存在时抛出异常
        """

    def delete(self, node_id: int) -> None:
        r"""
        删除指定节点及其所有子树.

        :param node_id: 要删除的节点 id
        :return None: 无返回值
        :raise RuntimeError: 当节点不存在时抛出异常
        """

    def update_value(self, node_id: int, value: int | float | str | bool) -> None:
        r"""
        更新节点的值.

        :param node_id: 节点 id
        :param value: 新的值
        :return None: 无返回值
        :raise RuntimeError: 当节点不存在时抛出异常
        """
```

**示例:**

```python
import ds4viz as dv

dv.config(title="二叉树构建与修改")

with dv.binary_tree(label="演示二叉树") as tree:
    root = tree.insert_root(10)
    tree.alias(root, "root")
    left = tree.insert_left(root, 5)
    right = tree.insert_right(root, 15)
    ll = tree.insert_left(left, 3)
    lr = tree.insert_right(left, 7)

    tree.update_value(right, 20)
    tree.amend(note="将 15 更新为 20", highlights=[
        dv.node(right, "focus", level=3)
    ])

    tree.delete(left)
    tree.amend(note="删除节点 5 及其子树", highlights=[
        dv.node(root, "active")
    ])
```

#### `dv.binary_search_tree()` → `BinarySearchTree`

```python
def binary_search_tree(
    label: str = "bst",
    output: str | None = None
) -> BinarySearchTree:
    r"""
    创建二叉搜索树实例.

    :param label: 二叉搜索树的标签 (对应 IR [object].label)
    :param output: 输出文件路径, 若为 None 则使用全局配置
    :return BinarySearchTree: 二叉搜索树实例 (上下文管理器)
    """
```

**BinarySearchTree 类方法:**

```python
class BinarySearchTree:
    def insert(self, value: int | float) -> int:
        r"""
        插入节点, 自动维护二叉搜索树性质.

        :param value: 要插入的值
        :return int: 新插入节点的 id
        """

    def delete(self, value: int | float) -> None:
        r"""
        删除节点, 自动维护二叉搜索树性质.

        :param value: 要删除的节点值
        :return None: 无返回值
        :raise RuntimeError: 当值不存在时抛出异常
        """
```

**示例:**

```python
import ds4viz as dv

dv.config(title="BST 插入与删除")

with dv.binary_search_tree(label="演示BST") as bst:
    with bst.phase("插入"):
        n10 = bst.insert(10)
        bst.alias(n10, "root")
        n5 = bst.insert(5)
        n15 = bst.insert(15)
        n3 = bst.insert(3)
        n7 = bst.insert(7)

    with bst.phase("删除"):
        bst.delete(5)
        bst.amend(note="删除 5, 用中序后继替代")

        n6 = bst.insert(6)
        bst.amend(note="插入 6", highlights=[
            dv.node(n6, "focus", level=3)
        ])
```

### 图结构

#### `dv.graph_undirected()` → `GraphUndirected`

```python
def graph_undirected(
    label: str = "graph",
    output: str | None = None
) -> GraphUndirected:
    r"""
    创建无向图实例.
    边在内部自动规范化为 from < to.

    :param label: 无向图的标签 (对应 IR [object].label)
    :param output: 输出文件路径, 若为 None 则使用全局配置
    :return GraphUndirected: 无向图实例 (上下文管理器)
    """
```

**GraphUndirected 类方法:**

```python
class GraphUndirected:
    def add_node(self, node_id: int, label: str) -> None:
        r"""
        添加节点.

        :param node_id: 节点 id, 非负整数
        :param label: 节点标签, 长度 1..32
        :return None: 无返回值
        :raise RuntimeError: 当节点已存在时抛出异常
        """

    def add_edge(self, from_id: int, to_id: int) -> None:
        r"""
        添加无向边. 内部自动规范化为 from_id < to_id.

        :param from_id: 节点 id
        :param to_id: 节点 id
        :return None: 无返回值
        :raise RuntimeError: 当节点不存在、边已存在或形成自环时抛出异常
        """

    def remove_node(self, node_id: int) -> None:
        r"""
        删除节点及其所有关联边.

        :param node_id: 要删除的节点 id
        :return None: 无返回值
        :raise RuntimeError: 当节点不存在时抛出异常
        """

    def remove_edge(self, from_id: int, to_id: int) -> None:
        r"""
        删除边.

        :param from_id: 节点 id
        :param to_id: 节点 id
        :return None: 无返回值
        :raise RuntimeError: 当边不存在时抛出异常
        """

    def update_node_label(self, node_id: int, label: str) -> None:
        r"""
        更新节点标签.

        :param node_id: 节点 id
        :param label: 新标签, 长度 1..32
        :return None: 无返回值
        :raise RuntimeError: 当节点不存在时抛出异常
        """
```

**示例:**

```python
import ds4viz as dv

with dv.graph_undirected(label="演示无向图") as g:
    g.add_node(0, "甲")
    g.add_node(1, "乙")
    g.add_node(2, "丙")
    g.add_node(3, "丁")

    g.add_edge(0, 1)
    g.add_edge(0, 2)
    g.add_edge(1, 3)
    g.add_edge(2, 3)

    g.update_node_label(1, "乙*")
    g.remove_edge(0, 1)
    g.remove_node(3)
```

#### `dv.graph_directed()` → `GraphDirected`

```python
def graph_directed(
    label: str = "graph",
    output: str | None = None
) -> GraphDirected:
    r"""
    创建有向图实例.

    :param label: 有向图的标签 (对应 IR [object].label)
    :param output: 输出文件路径, 若为 None 则使用全局配置
    :return GraphDirected: 有向图实例 (上下文管理器)
    """
```

**GraphDirected 类方法:**

```python
class GraphDirected:
    def add_node(self, node_id: int, label: str) -> None:
        r"""
        添加节点.

        :param node_id: 节点 id, 非负整数
        :param label: 节点标签, 长度 1..32
        :return None: 无返回值
        :raise RuntimeError: 当节点已存在时抛出异常
        """

    def add_edge(self, from_id: int, to_id: int) -> None:
        r"""
        添加有向边.

        :param from_id: 起始节点 id
        :param to_id: 终止节点 id
        :return None: 无返回值
        :raise RuntimeError: 当节点不存在、边已存在或形成自环时抛出异常
        """

    def remove_node(self, node_id: int) -> None:
        r"""
        删除节点及其所有关联边 (入边与出边).

        :param node_id: 要删除的节点 id
        :return None: 无返回值
        :raise RuntimeError: 当节点不存在时抛出异常
        """

    def remove_edge(self, from_id: int, to_id: int) -> None:
        r"""
        删除有向边.

        :param from_id: 起始节点 id
        :param to_id: 终止节点 id
        :return None: 无返回值
        :raise RuntimeError: 当边不存在时抛出异常
        """

    def update_node_label(self, node_id: int, label: str) -> None:
        r"""
        更新节点标签.

        :param node_id: 节点 id
        :param label: 新标签, 长度 1..32
        :return None: 无返回值
        :raise RuntimeError: 当节点不存在时抛出异常
        """
```

**示例:**

```python
import ds4viz as dv

with dv.graph_directed(label="演示有向图") as g:
    with g.phase("构建"):
        g.add_node(0, "甲")
        g.add_node(1, "乙")
        g.add_node(2, "丙")

        g.add_edge(0, 1)
        g.add_edge(1, 2)
        g.add_edge(2, 0)

    with g.phase("修改"):
        g.remove_edge(2, 0)
        g.amend(note="断开 丙→甲 的环", highlights=[
            dv.node(0, "active"),
            dv.node(2, "active")
        ])
```

#### `dv.graph_weighted()` → `GraphWeighted`

```python
def graph_weighted(
    label: str = "graph",
    output: str | None = None
) -> GraphWeighted:
    r"""
    创建带权有向图实例.

    :param label: 带权图的标签 (对应 IR [object].label)
    :param output: 输出文件路径, 若为 None 则使用全局配置
    :return GraphWeighted: 带权图实例 (上下文管理器)
    """
```

**GraphWeighted 类方法:**

```python
class GraphWeighted:
    def add_node(self, node_id: int, label: str) -> None:
        r"""
        添加节点.

        :param node_id: 节点 id, 非负整数
        :param label: 节点标签, 长度 1..32
        :return None: 无返回值
        :raise RuntimeError: 当节点已存在时抛出异常
        """

    def add_edge(self, from_id: int, to_id: int, weight: int | float) -> None:
        r"""
        添加带权有向边.

        :param from_id: 起始节点 id
        :param to_id: 终止节点 id
        :param weight: 边的权重
        :return None: 无返回值
        :raise RuntimeError: 当节点不存在、边已存在或形成自环时抛出异常
        """

    def remove_node(self, node_id: int) -> None:
        r"""
        删除节点及其所有关联边 (入边与出边).

        :param node_id: 要删除的节点 id
        :return None: 无返回值
        :raise RuntimeError: 当节点不存在时抛出异常
        """

    def remove_edge(self, from_id: int, to_id: int) -> None:
        r"""
        删除边.

        :param from_id: 起始节点 id
        :param to_id: 终止节点 id
        :return None: 无返回值
        :raise RuntimeError: 当边不存在时抛出异常
        """

    def update_weight(self, from_id: int, to_id: int, weight: int | float) -> None:
        r"""
        更新边的权重.

        :param from_id: 起始节点 id
        :param to_id: 终止节点 id
        :param weight: 新的权重值
        :return None: 无返回值
        :raise RuntimeError: 当边不存在时抛出异常
        """

    def update_node_label(self, node_id: int, label: str) -> None:
        r"""
        更新节点标签.

        :param node_id: 节点 id
        :param label: 新标签, 长度 1..32
        :return None: 无返回值
        :raise RuntimeError: 当节点不存在时抛出异常
        """
```

**示例:**

```python
import ds4viz as dv

dv.config(title="带权图构建")

with dv.graph_weighted(label="演示带权图") as g:
    g.add_node(0, "甲")
    g.alias(0, "source")
    g.add_node(1, "乙")
    g.add_node(2, "丙")

    g.add_edge(0, 1, 3.5)
    g.amend(highlights=[
        dv.node(0, "active"),
        dv.node(1, "active"),
        dv.edge(0, 1, "focus", level=3)
    ])

    g.add_edge(1, 2, 2.0)
    g.add_edge(0, 2, 10.0)

    g.update_weight(0, 2, 5.5)
    g.amend(note="松弛 甲→丙 的权重: 10.0 → 5.5", highlights=[
        dv.edge(0, 2, "focus", level=3)
    ])
```

---

### 异常处理

所有数据结构在遇到非法操作时抛出 `RuntimeError`. 上下文管理器会捕获异常并在退出时自动生成包含 `[error]` 的 `.toml` 文件.

```python
import ds4viz as dv

try:
    with dv.binary_tree() as tree:
        root = tree.insert_root(10)
        tree.insert_left(999, 5)    # 父节点不存在, 抛出 RuntimeError
except RuntimeError as e:
    print(f"发生错误: {e}")

# trace.toml 已生成, 包含:
# [error]
# type = "runtime"
# message = "Parent node not found: 999"
# step = 1
# last_state = 1
```

`[error].type` 的可能取值: `runtime`, `timeout`, `validation`, `sandbox`, `unknown`.

---

### 类型约束

`value` 字段 (用于 stack, queue, slist, dlist, binary_tree, bst) 仅支持 TOML 标量类型: `int`, `float`, `str`, `bool`.

`label` 字段 (用于 graph_undirected, graph_directed, graph_weighted 的节点) 为 `str` 类型, 长度 1..32.

`weight` 字段 (用于 graph_weighted 的边) 为 `int | float` 类型.

BST 的 `insert` / `delete` / `extract` 仅接受 `int | float`, 因为需要维护有序性.

---

## 使用示例

### 二叉树: 前序遍历

构建二叉树后, 使用 `step()` 逐步记录前序遍历 (根→左→右) 的访问过程.

```python
import ds4viz as dv

dv.config(title="二叉树前序遍历", author="WaterRun")

with dv.binary_tree(label="前序遍历") as tree:
    with tree.phase("构建"):
        root = tree.insert_root(10)
        tree.alias(root, "root")
        n5  = tree.insert_left(root, 5)
        n15 = tree.insert_right(root, 15)
        n3  = tree.insert_left(n5, 3)
        n7  = tree.insert_right(n5, 7)
        n12 = tree.insert_left(n15, 12)

    visited = []

    def visit(node_id, label):
        tree.step(
            note=f"访问节点 {label}",
            highlights=[
                *[dv.node(v, "visited") for v in visited],
                dv.node(node_id, "focus", level=3)
            ]
        )
        visited.append(node_id)

    with tree.phase("前序遍历"):
        visit(root, 10)     # 根
        visit(n5, 5)        # 左子树根
        visit(n3, 3)        # 左左
        visit(n7, 7)        # 左右
        visit(n15, 15)      # 右子树根
        visit(n12, 12)      # 右左
```

### 二叉树: 中序遍历

中序遍历 (左→根→右) 对 BST 可得到有序序列.

```python
import ds4viz as dv

dv.config(title="BST 中序遍历")

with dv.binary_search_tree(label="中序遍历") as bst:
    n8  = bst.insert(8)
    bst.alias(n8, "root")
    n3  = bst.insert(3)
    n10 = bst.insert(10)
    n1  = bst.insert(1)
    n6  = bst.insert(6)
    n14 = bst.insert(14)
    n4  = bst.insert(4)
    n7  = bst.insert(7)

    # 中序: 1, 3, 4, 6, 7, 8, 10, 14
    order = [
        (n1, 1), (n3, 3), (n4, 4), (n6, 6),
        (n7, 7), (n8, 8), (n10, 10), (n14, 14)
    ]
    visited = []

    for node_id, val in order:
        bst.step(
            note=f"访问 {val}",
            highlights=[
                *[dv.node(v, "visited") for v in visited],
                dv.node(node_id, "focus", level=3)
            ]
        )
        visited.append(node_id)
```

### 二叉树: 后序遍历

后序遍历 (左→右→根) 常用于释放树节点或计算子树属性.

```python
import ds4viz as dv

dv.config(title="二叉树后序遍历")

with dv.binary_tree(label="后序遍历") as tree:
    root = tree.insert_root("A")
    tree.alias(root, "root")
    b = tree.insert_left(root, "B")
    c = tree.insert_right(root, "C")
    d = tree.insert_left(b, "D")
    e = tree.insert_right(b, "E")

    # 后序: D, E, B, C, A
    visited = []

    def visit(node_id, label):
        tree.step(
            note=f"访问节点 {label}",
            highlights=[
                *[dv.node(v, "visited") for v in visited],
                dv.node(node_id, "focus", level=3)
            ]
        )
        visited.append(node_id)

    visit(d, "D")
    visit(e, "E")
    visit(b, "B")
    visit(c, "C")
    visit(root, "A")
```

### 二叉树: 层序遍历

逐层从左到右访问所有节点.

```python
import ds4viz as dv

dv.config(title="二叉树层序遍历")

with dv.binary_tree(label="层序遍历") as tree:
    with tree.phase("构建"):
        root = tree.insert_root(1)
        n2 = tree.insert_left(root, 2)
        n3 = tree.insert_right(root, 3)
        n4 = tree.insert_left(n2, 4)
        n5 = tree.insert_right(n2, 5)
        n6 = tree.insert_left(n3, 6)

    visited = []
    levels = [
        ([root], "第 1 层: [1]"),
        ([n2, n3], "第 2 层: [2, 3]"),
        ([n4, n5, n6], "第 3 层: [4, 5, 6]"),
    ]

    with tree.phase("层序遍历"):
        for level_nodes, desc in levels:
            tree.step(
                note=desc,
                highlights=[
                    *[dv.node(v, "visited") for v in visited],
                    *[dv.node(n, "focus", level=3) for n in level_nodes]
                ]
            )
            visited.extend(level_nodes)
```

### 二叉搜索树: 查找过程

用 `step()` 记录在 BST 中查找目标值的比较路径.

```python
import ds4viz as dv

dv.config(title="BST 查找 7")

with dv.binary_search_tree(label="BST查找") as bst:
    with bst.phase("构建"):
        n10 = bst.insert(10)
        bst.alias(n10, "root")
        n5  = bst.insert(5)
        n15 = bst.insert(15)
        n3  = bst.insert(3)
        n7  = bst.insert(7)
        n12 = bst.insert(12)
        n20 = bst.insert(20)

    target = 7

    with bst.phase("查找"):
        bst.step(note=f"{target} < 10, 向左", highlights=[
            dv.node(n10, "comparing", level=3)
        ])

        bst.step(note=f"{target} > 5, 向右", highlights=[
            dv.node(n10, "visited"),
            dv.node(n5, "comparing", level=3)
        ])

        bst.step(note=f"{target} == 7, 找到!", highlights=[
            dv.node(n10, "visited"),
            dv.node(n5, "visited"),
            dv.node(n7, "found", level=3)
        ])
```

### 栈: 括号匹配检查

演示使用栈检查括号字符串 `"(())"` 是否合法.

```python
import ds4viz as dv

dv.config(title="括号匹配: (())")

with dv.stack(label="括号匹配") as s:
    with s.phase("扫描"):
        s.push("(")
        s.amend(note="读入 '(', 压栈", highlights=[dv.item(0, "focus", level=3)])

        s.push("(")
        s.amend(note="读入 '(', 压栈", highlights=[
            dv.item(0, "visited"),
            dv.item(1, "focus", level=3)
        ])

        val = s.pop()
        s.amend(note=f"读入 ')', 弹出 '{val}' 匹配成功", highlights=[
            dv.item(0, "focus", level=3)
        ])

        val = s.pop()
        s.amend(note=f"读入 ')', 弹出 '{val}' 匹配成功")

    s.step(note="栈为空, 括号合法")
```

### 队列: 任务调度

演示简单的先进先出任务调度.

```python
import ds4viz as dv

dv.config(title="任务调度")

with dv.queue(label="任务队列") as q:
    q.enqueue("下载文件")
    q.enqueue("解压缩")
    q.enqueue("编译")

    val = q.dequeue()
    q.amend(note=f"执行: {val}", highlights=[dv.item(0, "focus", level=3)])

    q.enqueue("测试")
    q.amend(note="新增任务: 测试")

    val = q.dequeue()
    q.amend(note=f"执行: {val}", highlights=[dv.item(0, "focus", level=3)])

    val = q.dequeue()
    q.amend(note=f"执行: {val}", highlights=[dv.item(0, "focus", level=3)])

    val = q.dequeue()
    q.amend(note=f"执行: {val}")

    q.step(note="所有任务已完成")
```

### 单链表: 构建与遍历

```python
import ds4viz as dv

dv.config(title="单链表遍历")

with dv.single_linked_list(label="遍历演示") as slist:
    with slist.phase("构建"):
        a = slist.insert_head("甲")
        b = slist.insert_tail("乙")
        c = slist.insert_tail("丙")
        d = slist.insert_tail("丁")

    visited = []
    nodes = [(a, "甲"), (b, "乙"), (c, "丙"), (d, "丁")]

    with slist.phase("遍历"):
        for node_id, label in nodes:
            slist.step(
                note=f"访问 {label}",
                highlights=[
                    *[dv.node(v, "visited") for v in visited],
                    dv.node(node_id, "focus", level=3)
                ]
            )
            visited.append(node_id)
```

### 无向图: BFS 遍历

从节点 甲 出发, 广度优先遍历无向图.

```python
import ds4viz as dv

dv.config(title="无向图 BFS")

with dv.graph_undirected(label="BFS演示") as g:
    g.add_node(0, "甲")
    g.add_node(1, "乙")
    g.add_node(2, "丙")
    g.add_node(3, "丁")
    g.add_node(4, "戊")

    g.add_edge(0, 1)    # 甲-乙
    g.add_edge(0, 2)    # 甲-丙
    g.add_edge(1, 3)    # 乙-丁
    g.add_edge(2, 3)    # 丙-丁
    g.add_edge(3, 4)    # 丁-戊

    visited = []

    # 起点 甲
    visited.append(0)
    g.step(note="起点 甲 入队", highlights=[
        dv.node(0, "focus", level=3)
    ])

    # 出队 甲, 发现邻居 乙, 丙
    g.step(note="出队 甲, 发现邻居 乙, 丙", highlights=[
        dv.node(0, "visited"),
        dv.node(1, "focus", level=2),
        dv.node(2, "focus", level=2),
        dv.edge(0, 1, "active"),
        dv.edge(0, 2, "active")
    ])
    visited.extend([1, 2])

    # 出队 乙, 发现邻居 丁
    g.step(note="出队 乙, 发现邻居 丁", highlights=[
        *[dv.node(v, "visited") for v in [0, 1]],
        dv.node(2, "active"),
        dv.node(3, "focus", level=2),
        dv.edge(1, 3, "active")
    ])
    visited.append(3)

    # 出队 丙, 邻居 丁 已访问
    g.step(note="出队 丙, 邻居 丁 已访问", highlights=[
        *[dv.node(v, "visited") for v in [0, 1, 2]],
        dv.node(3, "active")
    ])

    # 出队 丁, 发现邻居 戊
    g.step(note="出队 丁, 发现邻居 戊", highlights=[
        *[dv.node(v, "visited") for v in [0, 1, 2, 3]],
        dv.node(4, "focus", level=2),
        dv.edge(3, 4, "active")
    ])
    visited.append(4)

    # 出队 戊, BFS 结束
    g.step(note="出队 戊, BFS 完成", highlights=[
        *[dv.node(v, "visited") for v in visited]
    ])
```

### 有向图: DFS 遍历

从节点 甲 出发, 深度优先遍历有向图.

```python
import ds4viz as dv

dv.config(title="有向图 DFS")

with dv.graph_directed(label="DFS演示") as g:
    g.add_node(0, "甲")
    g.add_node(1, "乙")
    g.add_node(2, "丙")
    g.add_node(3, "丁")

    g.add_edge(0, 1)    # 甲→乙
    g.add_edge(0, 2)    # 甲→丙
    g.add_edge(1, 3)    # 乙→丁
    g.add_edge(2, 3)    # 丙→丁

    visited = []

    def dfs_visit(node_id, label):
        g.step(
            note=f"进入 {label}",
            highlights=[
                *[dv.node(v, "visited") for v in visited],
                dv.node(node_id, "focus", level=3)
            ]
        )
        visited.append(node_id)

    # DFS: 甲 → 乙 → 丁 → (回溯) → 丙 (丁已访问)
    dfs_visit(0, "甲")
    dfs_visit(1, "乙")
    dfs_visit(3, "丁")

    g.step(note="丁 无出边, 回溯到 甲", highlights=[
        *[dv.node(v, "visited") for v in visited],
        dv.node(0, "active", level=2)
    ])

    dfs_visit(2, "丙")

    g.step(note="丙→丁 已访问, DFS 完成", highlights=[
        *[dv.node(v, "visited") for v in visited]
    ])
```

### 带权图: 最短路径松弛

演示 Dijkstra 风格的边松弛过程.

```python
import ds4viz as dv

dv.config(title="Dijkstra 松弛过程")

with dv.graph_weighted(label="最短路径") as g:
    with g.phase("构建"):
        g.add_node(0, "起点")
        g.alias(0, "source")
        g.add_node(1, "甲")
        g.add_node(2, "乙")
        g.add_node(3, "终点")

        g.add_edge(0, 1, 1)      # 起点→甲: 1
        g.add_edge(0, 2, 4)      # 起点→乙: 4
        g.add_edge(1, 2, 2)      # 甲→乙: 2
        g.add_edge(1, 3, 6)      # 甲→终点: 6
        g.add_edge(2, 3, 3)      # 乙→终点: 3

    with g.phase("松弛"):
        # 从 起点 出发
        g.step(note="从 起点 松弛: dist[甲]=1, dist[乙]=4", highlights=[
            dv.node(0, "visited"),
            dv.node(1, "focus", level=2),
            dv.node(2, "focus", level=2),
            dv.edge(0, 1, "active", level=3),
            dv.edge(0, 2, "active")
        ])

        # 取 甲
        g.step(note="从 甲 松弛: dist[乙]=4→3, dist[终点]=7", highlights=[
            dv.node(0, "visited"),
            dv.node(1, "visited"),
            dv.node(2, "focus", level=2),
            dv.node(3, "focus", level=2),
            dv.edge(1, 2, "active", level=3),
            dv.edge(1, 3, "active")
        ])

        # 取 乙
        g.step(note="从 乙 松弛: dist[终点]=7→6", highlights=[
            dv.node(0, "visited"),
            dv.node(1, "visited"),
            dv.node(2, "visited"),
            dv.node(3, "focus", level=3),
            dv.edge(2, 3, "active", level=3)
        ])

        # 完成
        g.step(note="完成, 最短路径 起点→甲→乙→终点, 距离=6", highlights=[
            dv.node(0, "found"), dv.node(1, "found"),
            dv.node(2, "found"), dv.node(3, "found"),
            dv.edge(0, 1, "found", level=3),
            dv.edge(1, 2, "found", level=3),
            dv.edge(2, 3, "found", level=3)
        ])
```

### 错误处理示例

异常发生后, 上下文管理器自动将已记录的状态和步骤写入 `.toml`, 并附带 `[error]`.

```python
import ds4viz as dv

dv.config(title="错误处理演示")

try:
    with dv.stack(label="错误演示") as s:
        s.push(10)
        s.push(20)
        s.pop()
        s.pop()
        s.pop()     # 栈已空, 抛出 RuntimeError
except RuntimeError as e:
    print(f"捕获错误: {e}")

# trace.toml 已生成, 包含前 4 步的正常记录 + [error] 信息
```

```python
import ds4viz as dv

dv.config(title="图错误演示")

try:
    with dv.graph_directed(label="错误图") as g:
        g.add_node(0, "甲")
        g.add_node(1, "乙")
        g.add_edge(0, 1)
        g.add_edge(0, 1)    # 重复边, 抛出 RuntimeError
except RuntimeError as e:
    print(f"捕获错误: {e}")
```
