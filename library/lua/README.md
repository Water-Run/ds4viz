# ds4viz

`ds4viz` 是一个 Lua 库，提供了 Lua 语言的 `ds4viz` 支持。  
`ds4viz` 是一个可视化数据结构工具。参见：[GitHub](https://github.com/Water-Run/ds4viz)

## 安装

```bash
luarocks install ds4viz
```

## 测试  

```bash
busted ./test/test_ds4viz.lua
```

## 快速上手

以下脚本提供了一个快速使用示例，演示如何使用 `ds4viz` 生成 `trace.toml`：

```lua
local ds4viz = require("ds4viz")

-- 配置全局参数（可选）
ds4viz.config({
    output_path = "trace.toml",
    title = "Graph Demo",
    author = "WaterRun",
    comment = "无向图示例"
})

-- 使用上下文管理器
ds4viz.withContext(ds4viz.graphUndirected(), function(graph)
    -- 添加节点
    graph:addNode(0, "A")
    graph:addNode(1, "B")
    graph:addNode(2, "C")

    -- 添加无向边
    graph:addEdge(0, 1)
    graph:addEdge(1, 2)
end)

print("已生成 trace.toml")
```

## API 参考

### 全局配置

#### `config`

```lua
ds4viz.config(options)
```

配置全局参数。

**参数:**
- `options`: table - 配置选项
  - `output_path`: string - 输出文件路径，默认为 "trace.toml"
  - `title`: string - 可视化标题
  - `author`: string - 作者信息
  - `comment`: string - 注释说明

**示例:**

```lua
ds4viz.config({
    output_path = "my_trace.toml",
    title = "栈操作演示",
    author = "WaterRun",
    comment = "演示基本的栈操作"
})
```

### 上下文管理

#### `withContext`

```lua
ds4viz.withContext(structure, fn)
```

使用上下文执行函数，模拟 Python 的 with 语句。确保无论成功/失败，都会生成 trace.toml。

**参数:**
- `structure`: BaseStructure - 数据结构实例
- `fn`: function - 要执行的函数，接收数据结构实例作为参数

**返回:**
- `boolean` - 是否成功执行
- `any` - 错误信息（如果有）

**示例:**

```lua
local ok, err = ds4viz.withContext(ds4viz.stack(), function(s)
    s:push(10)
    s:push(20)
    s:pop()
end)
```

---

### 线性结构

#### `stack`

```lua
ds4viz.stack(label, output)
```

创建栈实例。

**参数:**
- `label`: string|nil - 栈的标签，默认为 "stack"
- `output`: string|nil - 输出文件路径

**Stack 方法:**

```lua
Stack:push(value)      -- 压栈操作
Stack:pop()            -- 弹栈操作（栈为空时抛出异常）
Stack:clear()          -- 清空栈
```

**示例:**

```lua
ds4viz.withContext(ds4viz.stack("demo_stack"), function(s)
    s:push(10)
    s:push(20)
    s:push(30)
    s:pop()
    s:pop()
end)
```

#### `queue`

```lua
ds4viz.queue(label, output)
```

创建队列实例。

**Queue 方法:**

```lua
Queue:enqueue(value)   -- 入队操作
Queue:dequeue()        -- 出队操作（队列为空时抛出异常）
Queue:clear()          -- 清空队列
```

**示例:**

```lua
ds4viz.withContext(ds4viz.queue("demo_queue"), function(q)
    q:enqueue(10)
    q:enqueue(20)
    q:enqueue(30)
    q:dequeue()
    q:dequeue()
end)
```

#### `singleLinkedList`

```lua
ds4viz.singleLinkedList(label, output)
```

创建单链表实例。

**SingleLinkedList 方法:**

```lua
SingleLinkedList:insertHead(value)           -- 在头部插入，返回节点 id
SingleLinkedList:insertTail(value)           -- 在尾部插入，返回节点 id
SingleLinkedList:insertAfter(node_id, value) -- 在指定节点后插入，返回节点 id
SingleLinkedList:delete(node_id)             -- 删除指定节点
SingleLinkedList:deleteHead()                -- 删除头节点
SingleLinkedList:reverse()                   -- 反转链表
```

**示例:**

```lua
ds4viz.withContext(ds4viz.singleLinkedList("demo_slist"), function(slist)
    local node_a = slist:insertHead(10)
    slist:insertTail(20)
    local node_c = slist:insertTail(30)
    slist:insertAfter(node_a, 15)
    slist:delete(node_c)
    slist:reverse()
end)
```

#### `doubleLinkedList`

```lua
ds4viz.doubleLinkedList(label, output)
```

创建双向链表实例。

**DoubleLinkedList 方法:**

```lua
DoubleLinkedList:insertHead(value)            -- 在头部插入
DoubleLinkedList:insertTail(value)            -- 在尾部插入
DoubleLinkedList:insertBefore(node_id, value) -- 在指定节点前插入
DoubleLinkedList:insertAfter(node_id, value)  -- 在指定节点后插入
DoubleLinkedList:delete(node_id)              -- 删除指定节点
DoubleLinkedList:deleteHead()                 -- 删除头节点
DoubleLinkedList:deleteTail()                 -- 删除尾节点
DoubleLinkedList:reverse()                    -- 反转链表
```

**示例:**

```lua
ds4viz.withContext(ds4viz.doubleLinkedList("demo_dlist"), function(dlist)
    local node_a = dlist:insertHead(10)
    dlist:insertTail(30)
    dlist:insertAfter(node_a, 20)
    dlist:deleteTail()
    dlist:reverse()
end)
```

---

### 树结构

#### `binaryTree`

```lua
ds4viz.binaryTree(label, output)
```

创建二叉树实例。

**BinaryTree 方法:**

```lua
BinaryTree:insertRoot(value)             -- 插入根节点，返回节点 id
BinaryTree:insertLeft(parent_id, value)  -- 在左侧插入子节点，返回节点 id
BinaryTree:insertRight(parent_id, value) -- 在右侧插入子节点，返回节点 id
BinaryTree:delete(node_id)               -- 删除节点及其所有子树
BinaryTree:updateValue(node_id, value)   -- 更新节点的值
```

**示例:**

```lua
ds4viz.withContext(ds4viz.binaryTree("demo_tree"), function(tree)
    local root = tree:insertRoot(10)
    local left = tree:insertLeft(root, 5)
    local right = tree:insertRight(root, 15)
    tree:insertLeft(left, 3)
    tree:insertRight(left, 7)
    tree:updateValue(right, 20)
    tree:delete(left)
end)
```

#### `binarySearchTree`

```lua
ds4viz.binarySearchTree(label, output)
```

创建二叉搜索树实例。

**BinarySearchTree 方法:**

```lua
BinarySearchTree:insert(value) -- 插入节点，自动维护 BST 性质
BinarySearchTree:delete(value) -- 删除节点，自动维护 BST 性质
```

**示例:**

```lua
ds4viz.withContext(ds4viz.binarySearchTree("demo_bst"), function(bst)
    bst:insert(10)
    bst:insert(5)
    bst:insert(15)
    bst:insert(3)
    bst:insert(7)
    bst:delete(5)
    bst:insert(6)
end)
```

#### `heap`

```lua
ds4viz.heap(label, heap_type, output)
```

创建堆实例。

**参数:**
- `label`: string|nil - 堆的标签，默认为 "heap"
- `heap_type`: string|nil - 堆类型，"min" 表示最小堆，"max" 表示最大堆，默认为 "min"
- `output`: string|nil - 输出文件路径

**Heap 方法:**

```lua
Heap:insert(value) -- 插入元素，自动维护堆性质
Heap:extract()     -- 提取堆顶元素，自动维护堆性质
Heap:clear()       -- 清空堆
```

**示例:**

```lua
-- 最小堆
ds4viz.withContext(ds4viz.heap("demo_min_heap", "min"), function(min_heap)
    min_heap:insert(10)
    min_heap:insert(5)
    min_heap:insert(15)
    min_heap:insert(3)
    min_heap:extract()  -- 提取最小值 3
end)

-- 最大堆
ds4viz.withContext(ds4viz.heap("demo_max_heap", "max"), function(max_heap)
    max_heap:insert(10)
    max_heap:insert(5)
    max_heap:insert(15)
    max_heap:insert(3)
    max_heap:extract()  -- 提取最大值 15
end)
```

---

### 图结构

#### `graphUndirected`

```lua
ds4viz.graphUndirected(label, output)
```

创建无向图实例。

**GraphUndirected 方法:**

```lua
GraphUndirected:addNode(node_id, label)          -- 添加节点
GraphUndirected:addEdge(from_id, to_id)          -- 添加无向边
GraphUndirected:removeNode(node_id)              -- 删除节点及其相关边
GraphUndirected:removeEdge(from_id, to_id)       -- 删除边
GraphUndirected:updateNodeLabel(node_id, label)  -- 更新节点标签
```

**示例:**

```lua
ds4viz.withContext(ds4viz.graphUndirected("demo_graph"), function(graph)
    graph:addNode(0, "A")
    graph:addNode(1, "B")
    graph:addNode(2, "C")
    graph:addEdge(0, 1)
    graph:addEdge(1, 2)
    graph:updateNodeLabel(1, "B_updated")
    graph:removeEdge(0, 1)
end)
```

#### `graphDirected`

```lua
ds4viz.graphDirected(label, output)
```

创建有向图实例。

**GraphDirected 方法:**

```lua
GraphDirected:addNode(node_id, label)          -- 添加节点
GraphDirected:addEdge(from_id, to_id)          -- 添加有向边
GraphDirected:removeNode(node_id)              -- 删除节点及其相关边
GraphDirected:removeEdge(from_id, to_id)       -- 删除边
GraphDirected:updateNodeLabel(node_id, label)  -- 更新节点标签
```

**示例:**

```lua
ds4viz.withContext(ds4viz.graphDirected("demo_digraph"), function(graph)
    graph:addNode(0, "A")
    graph:addNode(1, "B")
    graph:addNode(2, "C")
    graph:addEdge(0, 1)
    graph:addEdge(1, 2)
    graph:addEdge(2, 0)
    graph:updateNodeLabel(1, "B_updated")
    graph:removeEdge(2, 0)
end)
```

#### `graphWeighted`

```lua
ds4viz.graphWeighted(label, directed, output)
```

创建带权图实例。

**参数:**
- `label`: string|nil - 带权图的标签，默认为 "graph"
- `directed`: boolean|nil - 是否为有向图，默认为 false（无向图）
- `output`: string|nil - 输出文件路径

**GraphWeighted 方法:**

```lua
GraphWeighted:addNode(node_id, label)               -- 添加节点
GraphWeighted:addEdge(from_id, to_id, weight)       -- 添加带权边
GraphWeighted:removeNode(node_id)                   -- 删除节点及其相关边
GraphWeighted:removeEdge(from_id, to_id)            -- 删除边
GraphWeighted:updateWeight(from_id, to_id, weight)  -- 更新边权重
GraphWeighted:updateNodeLabel(node_id, label)       -- 更新节点标签
```

**示例:**

```lua
-- 无向带权图
ds4viz.withContext(ds4viz.graphWeighted("demo_weighted", false), function(graph)
    graph:addNode(0, "A")
    graph:addNode(1, "B")
    graph:addNode(2, "C")
    graph:addEdge(0, 1, 3.5)
    graph:addEdge(1, 2, 2.0)
    graph:addEdge(2, 0, 4.2)
    graph:updateWeight(0, 1, 5.0)
    graph:removeEdge(2, 0)
end)

-- 有向带权图
ds4viz.withContext(ds4viz.graphWeighted("demo_directed_weighted", true), function(graph)
    graph:addNode(0, "Start")
    graph:addNode(1, "Middle")
    graph:addNode(2, "End")
    graph:addEdge(0, 1, 1.5)
    graph:addEdge(1, 2, 2.5)
end)
```

---

### 异常处理

所有数据结构在遇到错误时都会抛出异常，使用 `withContext` 可以自动捕获异常并在退出时生成包含 `[error]` 部分的 `.toml` 文件。

**示例:**

```lua
local ok, err = ds4viz.withContext(ds4viz.binaryTree(), function(tree)
    local root = tree:insertRoot(10)
    tree:insertLeft(999, 5)  -- 父节点不存在，抛出异常
end)

if not ok then
    print("发生错误: " .. tostring(err))
end

-- 即使发生异常，trace.toml 仍会生成，包含 [error] 部分
```

---

## 类型约束

根据 IR 定义，`value` 字段仅支持以下类型：

- `number`: 整数或浮点数
- `string`: 字符串
- `boolean`: 布尔值
