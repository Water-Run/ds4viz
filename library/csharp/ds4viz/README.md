# Ds4Viz - C# 数据结构可视化库

## 概述

`Ds4Viz` 是一个 C# 库，提供数据结构的可视化追踪功能，生成标准化的 TOML 中间表示文件。该库是 `ds4viz` 可视化平台的 C# 语言实现。 

## 安装

`Ds4Viz` 发布在 [NuGet](https://www.nuget.org/packages/Ds4Viz/) 上：

```bash
dotnet add package Ds4Viz
```

或通过 Package Manager Console：

```powershell
Install-Package Ds4Viz
```

## 快速上手

以下代码演示如何使用 `Ds4Viz` 生成 `trace.toml`，并在发生异常时仍然产出包含 `[error]` 的 TOML IR 文件：

```csharp
using Ds4Viz;

// 使用 using 语句：确保无论成功/失败，都会生成 trace.toml
// 成功 => [result]，异常 => [error]
using (var graph = Ds4Viz.Api.GraphUndirected())
{
    // 添加节点
    graph.AddNode(0, "A");
    graph.AddNode(1, "B");
    graph.AddNode(2, "C");

    // 添加无向边（内部会规范化为 from < to）
    graph.AddEdge(0, 1);
    graph.AddEdge(1, 2);

    // 可以继续调用更多 API，每次操作都会记录到 trace 中
}

Console.WriteLine("已生成 trace.toml");
```

## API 参考

### 全局配置

#### `Ds4Viz.Api.Config`

```csharp
/// <summary>
/// 配置全局参数
/// </summary>
/// <param name="outputPath">输出文件路径，默认为 "trace.toml"</param>
/// <param name="title">可视化标题</param>
/// <param name="author">作者信息</param>
/// <param name="comment">注释说明</param>
public static void Config(
    string outputPath = "trace.toml",
    string title = "",
    string author = "",
    string comment = ""
);
```

**示例：**

```csharp
using Ds4Viz;

Ds4Viz.Api.Config(
    outputPath: "my_trace.toml",
    title: "栈操作演示",
    author: "WaterRun",
    comment: "演示基本的栈操作"
);
```

---

### 线性结构

#### `Ds4Viz.Api.Stack`

```csharp
/// <summary>
/// 创建栈实例
/// </summary>
/// <param name="label">栈的标签，默认为 "stack"</param>
/// <param name="output">输出文件路径，若为 null 则使用全局配置</param>
/// <returns>栈实例</returns>
public static Stack Stack(string label = "stack", string? output = null);
```

**Stack 类方法：**

```csharp
public sealed class Stack : IDisposable
{
    /// <summary>
    /// 压栈操作
    /// </summary>
    /// <param name="value">要压入的值，支持 int、double、string、bool</param>
    public void Push(object value);

    /// <summary>
    /// 弹栈操作
    /// </summary>
    /// <exception cref="InvalidOperationException">当栈为空时抛出</exception>
    public void Pop();

    /// <summary>
    /// 清空栈
    /// </summary>
    public void Clear();
}
```

**示例：**

```csharp
using Ds4Viz;

using (var s = Ds4Viz.Api.Stack(label: "demo_stack"))
{
    s.Push(10);
    s.Push(20);
    s.Push(30);

    s.Pop();
    s.Pop();
}
```

#### `Ds4Viz.Api.Queue`

```csharp
/// <summary>
/// 创建队列实例
/// </summary>
/// <param name="label">队列的标签，默认为 "queue"</param>
/// <param name="output">输出文件路径，若为 null 则使用全局配置</param>
/// <returns>队列实例</returns>
public static Queue Queue(string label = "queue", string? output = null);
```

**Queue 类方法：**

```csharp
public sealed class Queue : IDisposable
{
    /// <summary>
    /// 入队操作
    /// </summary>
    /// <param name="value">要入队的值，支持 int、double、string、bool</param>
    public void Enqueue(object value);

    /// <summary>
    /// 出队操作
    /// </summary>
    /// <exception cref="InvalidOperationException">当队列为空时抛出</exception>
    public void Dequeue();

    /// <summary>
    /// 清空队列
    /// </summary>
    public void Clear();
}
```

**示例：**

```csharp
using Ds4Viz;

using (var q = Ds4Viz.Api.Queue(label: "demo_queue"))
{
    q.Enqueue(10);
    q.Enqueue(20);
    q.Enqueue(30);

    q.Dequeue();
    q.Dequeue();
}
```

#### `Ds4Viz.Api.SingleLinkedList`

```csharp
/// <summary>
/// 创建单链表实例
/// </summary>
/// <param name="label">单链表的标签，默认为 "slist"</param>
/// <param name="output">输出文件路径，若为 null 则使用全局配置</param>
/// <returns>单链表实例</returns>
public static SingleLinkedList SingleLinkedList(string label = "slist", string? output = null);
```

**SingleLinkedList 类方法：**

```csharp
public sealed class SingleLinkedList : IDisposable
{
    /// <summary>
    /// 在链表头部插入节点
    /// </summary>
    /// <param name="value">要插入的值</param>
    /// <returns>新插入节点的 id</returns>
    public int InsertHead(object value);

    /// <summary>
    /// 在链表尾部插入节点
    /// </summary>
    /// <param name="value">要插入的值</param>
    /// <returns>新插入节点的 id</returns>
    public int InsertTail(object value);

    /// <summary>
    /// 在指定节点后插入新节点
    /// </summary>
    /// <param name="nodeId">目标节点的 id</param>
    /// <param name="value">要插入的值</param>
    /// <returns>新插入节点的 id</returns>
    /// <exception cref="InvalidOperationException">当指定节点不存在时抛出</exception>
    public int InsertAfter(int nodeId, object value);

    /// <summary>
    /// 删除指定节点
    /// </summary>
    /// <param name="nodeId">要删除的节点 id</param>
    /// <exception cref="InvalidOperationException">当指定节点不存在时抛出</exception>
    public void Delete(int nodeId);

    /// <summary>
    /// 删除头节点
    /// </summary>
    /// <exception cref="InvalidOperationException">当链表为空时抛出</exception>
    public void DeleteHead();

    /// <summary>
    /// 反转链表
    /// </summary>
    public void Reverse();
}
```

**示例：**

```csharp
using Ds4Viz;

using (var slist = Ds4Viz.Api.SingleLinkedList(label: "demo_slist"))
{
    var nodeA = slist.InsertHead(10);
    slist.InsertTail(20);
    var nodeC = slist.InsertTail(30);

    slist.InsertAfter(nodeA, 15);

    slist.Delete(nodeC);
    slist.Reverse();
}
```

#### `Ds4Viz.Api.DoubleLinkedList`

```csharp
/// <summary>
/// 创建双向链表实例
/// </summary>
/// <param name="label">双向链表的标签，默认为 "dlist"</param>
/// <param name="output">输出文件路径，若为 null 则使用全局配置</param>
/// <returns>双向链表实例</returns>
public static DoubleLinkedList DoubleLinkedList(string label = "dlist", string? output = null);
```

**DoubleLinkedList 类方法：**

```csharp
public sealed class DoubleLinkedList : IDisposable
{
    /// <summary>
    /// 在链表头部插入节点
    /// </summary>
    /// <param name="value">要插入的值</param>
    /// <returns>新插入节点的 id</returns>
    public int InsertHead(object value);

    /// <summary>
    /// 在链表尾部插入节点
    /// </summary>
    /// <param name="value">要插入的值</param>
    /// <returns>新插入节点的 id</returns>
    public int InsertTail(object value);

    /// <summary>
    /// 在指定节点前插入新节点
    /// </summary>
    /// <param name="nodeId">目标节点的 id</param>
    /// <param name="value">要插入的值</param>
    /// <returns>新插入节点的 id</returns>
    /// <exception cref="InvalidOperationException">当指定节点不存在时抛出</exception>
    public int InsertBefore(int nodeId, object value);

    /// <summary>
    /// 在指定节点后插入新节点
    /// </summary>
    /// <param name="nodeId">目标节点的 id</param>
    /// <param name="value">要插入的值</param>
    /// <returns>新插入节点的 id</returns>
    /// <exception cref="InvalidOperationException">当指定节点不存在时抛出</exception>
    public int InsertAfter(int nodeId, object value);

    /// <summary>
    /// 删除指定节点
    /// </summary>
    /// <param name="nodeId">要删除的节点 id</param>
    /// <exception cref="InvalidOperationException">当指定节点不存在时抛出</exception>
    public void Delete(int nodeId);

    /// <summary>
    /// 删除头节点
    /// </summary>
    /// <exception cref="InvalidOperationException">当链表为空时抛出</exception>
    public void DeleteHead();

    /// <summary>
    /// 删除尾节点
    /// </summary>
    /// <exception cref="InvalidOperationException">当链表为空时抛出</exception>
    public void DeleteTail();

    /// <summary>
    /// 反转链表
    /// </summary>
    public void Reverse();
}
```

**示例：**

```csharp
using Ds4Viz;

using (var dlist = Ds4Viz.Api.DoubleLinkedList(label: "demo_dlist"))
{
    var nodeA = dlist.InsertHead(10);
    dlist.InsertTail(30);

    dlist.InsertAfter(nodeA, 20);

    dlist.DeleteTail();
    dlist.Reverse();
}
```

---

### 树结构

#### `Ds4Viz.Api.BinaryTree`

```csharp
/// <summary>
/// 创建二叉树实例
/// </summary>
/// <param name="label">二叉树的标签，默认为 "binary_tree"</param>
/// <param name="output">输出文件路径，若为 null 则使用全局配置</param>
/// <returns>二叉树实例</returns>
public static BinaryTree BinaryTree(string label = "binary_tree", string? output = null);
```

**BinaryTree 类方法：**

```csharp
public sealed class BinaryTree : IDisposable
{
    /// <summary>
    /// 插入根节点
    /// </summary>
    /// <param name="value">根节点的值</param>
    /// <returns>根节点的 id</returns>
    /// <exception cref="InvalidOperationException">当根节点已存在时抛出</exception>
    public int InsertRoot(object value);

    /// <summary>
    /// 在指定父节点的左侧插入子节点
    /// </summary>
    /// <param name="parentId">父节点的 id</param>
    /// <param name="value">要插入的子节点值</param>
    /// <returns>新插入节点的 id</returns>
    /// <exception cref="InvalidOperationException">当父节点不存在或左子节点已存在时抛出</exception>
    public int InsertLeft(int parentId, object value);

    /// <summary>
    /// 在指定父节点的右侧插入子节点
    /// </summary>
    /// <param name="parentId">父节点的 id</param>
    /// <param name="value">要插入的子节点值</param>
    /// <returns>新插入节点的 id</returns>
    /// <exception cref="InvalidOperationException">当父节点不存在或右子节点已存在时抛出</exception>
    public int InsertRight(int parentId, object value);

    /// <summary>
    /// 删除指定节点及其所有子树
    /// </summary>
    /// <param name="nodeId">要删除的节点 id</param>
    /// <exception cref="InvalidOperationException">当节点不存在时抛出</exception>
    public void Delete(int nodeId);

    /// <summary>
    /// 更新节点的值
    /// </summary>
    /// <param name="nodeId">节点 id</param>
    /// <param name="value">新的值</param>
    /// <exception cref="InvalidOperationException">当节点不存在时抛出</exception>
    public void UpdateValue(int nodeId, object value);
}
```

**示例：**

```csharp
using Ds4Viz;

using (var tree = Ds4Viz.Api.BinaryTree(label: "demo_tree"))
{
    var root = tree.InsertRoot(10);
    var left = tree.InsertLeft(root, 5);
    var right = tree.InsertRight(root, 15);

    tree.InsertLeft(left, 3);
    tree.InsertRight(left, 7);

    tree.UpdateValue(right, 20);
    tree.Delete(left);
}
```

#### `Ds4Viz.Api.BinarySearchTree`

```csharp
/// <summary>
/// 创建二叉搜索树实例
/// </summary>
/// <param name="label">二叉搜索树的标签，默认为 "bst"</param>
/// <param name="output">输出文件路径，若为 null 则使用全局配置</param>
/// <returns>二叉搜索树实例</returns>
public static BinarySearchTree BinarySearchTree(string label = "bst", string? output = null);
```

**BinarySearchTree 类方法：**

```csharp
public sealed class BinarySearchTree : IDisposable
{
    /// <summary>
    /// 插入节点，自动维护二叉搜索树性质
    /// </summary>
    /// <param name="value">要插入的值（仅支持 int 或 double）</param>
    /// <returns>新插入节点的 id</returns>
    public int Insert(double value);

    /// <summary>
    /// 删除节点，自动维护二叉搜索树性质
    /// </summary>
    /// <param name="value">要删除的节点值</param>
    /// <exception cref="InvalidOperationException">当节点不存在时抛出</exception>
    public void Delete(double value);
}
```

**示例：**

```csharp
using Ds4Viz;

using (var bst = Ds4Viz.Api.BinarySearchTree(label: "demo_bst"))
{
    bst.Insert(10);
    bst.Insert(5);
    bst.Insert(15);
    bst.Insert(3);
    bst.Insert(7);

    bst.Delete(5);
    bst.Insert(6);
}
```

#### `Ds4Viz.Api.Heap`

```csharp
/// <summary>
/// 创建堆实例
/// </summary>
/// <param name="label">堆的标签，默认为 "heap"</param>
/// <param name="heapType">堆类型，"min" 表示最小堆，"max" 表示最大堆，默认为 "min"</param>
/// <param name="output">输出文件路径，若为 null 则使用全局配置</param>
/// <returns>堆实例</returns>
/// <exception cref="ArgumentException">当 heapType 不是 "min" 或 "max" 时抛出</exception>
public static Heap Heap(string label = "heap", string heapType = "min", string? output = null);
```

**Heap 类方法：**

```csharp
public sealed class Heap : IDisposable
{
    /// <summary>
    /// 插入元素，自动维护堆性质
    /// </summary>
    /// <param name="value">要插入的值（仅支持 int 或 double）</param>
    public void Insert(double value);

    /// <summary>
    /// 提取堆顶元素，自动维护堆性质
    /// </summary>
    /// <exception cref="InvalidOperationException">当堆为空时抛出</exception>
    public void Extract();

    /// <summary>
    /// 清空堆
    /// </summary>
    public void Clear();
}
```

**示例：**

```csharp
using Ds4Viz;

// 最小堆
using (var minHeap = Ds4Viz.Api.Heap(label: "demo_min_heap", heapType: "min"))
{
    minHeap.Insert(10);
    minHeap.Insert(5);
    minHeap.Insert(15);
    minHeap.Insert(3);

    minHeap.Extract(); // 提取最小值 3
}

// 最大堆
using (var maxHeap = Ds4Viz.Api.Heap(label: "demo_max_heap", heapType: "max"))
{
    maxHeap.Insert(10);
    maxHeap.Insert(5);
    maxHeap.Insert(15);
    maxHeap.Insert(3);

    maxHeap.Extract(); // 提取最大值 15
}
```

---

### 图结构

#### `Ds4Viz.Api.GraphUndirected`

```csharp
/// <summary>
/// 创建无向图实例
/// </summary>
/// <param name="label">无向图的标签，默认为 "graph"</param>
/// <param name="output">输出文件路径，若为 null 则使用全局配置</param>
/// <returns>无向图实例</returns>
public static GraphUndirected GraphUndirected(string label = "graph", string? output = null);
```

**GraphUndirected 类方法：**

```csharp
public sealed class GraphUndirected : IDisposable
{
    /// <summary>
    /// 添加节点
    /// </summary>
    /// <param name="nodeId">节点 id</param>
    /// <param name="label">节点标签，长度限制为 1-32 字符</param>
    /// <exception cref="InvalidOperationException">当节点已存在时抛出</exception>
    /// <exception cref="ArgumentException">当标签长度不在 1-32 范围内时抛出</exception>
    public void AddNode(int nodeId, string label);

    /// <summary>
    /// 添加无向边，内部自动规范化为 fromId &lt; toId
    /// </summary>
    /// <param name="fromId">起始节点 id</param>
    /// <param name="toId">终止节点 id</param>
    /// <exception cref="InvalidOperationException">当节点不存在、边已存在或形成自环时抛出</exception>
    public void AddEdge(int fromId, int toId);

    /// <summary>
    /// 删除节点及其相关的所有边
    /// </summary>
    /// <param name="nodeId">要删除的节点 id</param>
    /// <exception cref="InvalidOperationException">当节点不存在时抛出</exception>
    public void RemoveNode(int nodeId);

    /// <summary>
    /// 删除边
    /// </summary>
    /// <param name="fromId">起始节点 id</param>
    /// <param name="toId">终止节点 id</param>
    /// <exception cref="InvalidOperationException">当边不存在时抛出</exception>
    public void RemoveEdge(int fromId, int toId);

    /// <summary>
    /// 更新节点标签
    /// </summary>
    /// <param name="nodeId">节点 id</param>
    /// <param name="label">新的节点标签，长度限制为 1-32 字符</param>
    /// <exception cref="InvalidOperationException">当节点不存在时抛出</exception>
    /// <exception cref="ArgumentException">当标签长度不在 1-32 范围内时抛出</exception>
    public void UpdateNodeLabel(int nodeId, string label);
}
```

**示例：**

```csharp
using Ds4Viz;

using (var graph = Ds4Viz.Api.GraphUndirected(label: "demo_graph"))
{
    graph.AddNode(0, "A");
    graph.AddNode(1, "B");
    graph.AddNode(2, "C");

    graph.AddEdge(0, 1);
    graph.AddEdge(1, 2);

    graph.UpdateNodeLabel(1, "B_updated");
    graph.RemoveEdge(0, 1);
}
```

#### `Ds4Viz.Api.GraphDirected`

```csharp
/// <summary>
/// 创建有向图实例
/// </summary>
/// <param name="label">有向图的标签，默认为 "graph"</param>
/// <param name="output">输出文件路径，若为 null 则使用全局配置</param>
/// <returns>有向图实例</returns>
public static GraphDirected GraphDirected(string label = "graph", string? output = null);
```

**GraphDirected 类方法：**

```csharp
public sealed class GraphDirected : IDisposable
{
    /// <summary>
    /// 添加节点
    /// </summary>
    /// <param name="nodeId">节点 id</param>
    /// <param name="label">节点标签，长度限制为 1-32 字符</param>
    /// <exception cref="InvalidOperationException">当节点已存在时抛出</exception>
    /// <exception cref="ArgumentException">当标签长度不在 1-32 范围内时抛出</exception>
    public void AddNode(int nodeId, string label);

    /// <summary>
    /// 添加有向边
    /// </summary>
    /// <param name="fromId">起始节点 id</param>
    /// <param name="toId">终止节点 id</param>
    /// <exception cref="InvalidOperationException">当节点不存在、边已存在或形成自环时抛出</exception>
    public void AddEdge(int fromId, int toId);

    /// <summary>
    /// 删除节点及其相关的所有边
    /// </summary>
    /// <param name="nodeId">要删除的节点 id</param>
    /// <exception cref="InvalidOperationException">当节点不存在时抛出</exception>
    public void RemoveNode(int nodeId);

    /// <summary>
    /// 删除边
    /// </summary>
    /// <param name="fromId">起始节点 id</param>
    /// <param name="toId">终止节点 id</param>
    /// <exception cref="InvalidOperationException">当边不存在时抛出</exception>
    public void RemoveEdge(int fromId, int toId);

    /// <summary>
    /// 更新节点标签
    /// </summary>
    /// <param name="nodeId">节点 id</param>
    /// <param name="label">新的节点标签，长度限制为 1-32 字符</param>
    /// <exception cref="InvalidOperationException">当节点不存在时抛出</exception>
    /// <exception cref="ArgumentException">当标签长度不在 1-32 范围内时抛出</exception>
    public void UpdateNodeLabel(int nodeId, string label);
}
```

**示例：**

```csharp
using Ds4Viz;

using (var graph = Ds4Viz.Api.GraphDirected(label: "demo_digraph"))
{
    graph.AddNode(0, "A");
    graph.AddNode(1, "B");
    graph.AddNode(2, "C");

    graph.AddEdge(0, 1);
    graph.AddEdge(1, 2);
    graph.AddEdge(2, 0);

    graph.UpdateNodeLabel(1, "B_updated");
    graph.RemoveEdge(2, 0);
}
```

#### `Ds4Viz.Api.GraphWeighted`

```csharp
/// <summary>
/// 创建带权图实例
/// </summary>
/// <param name="label">带权图的标签，默认为 "graph"</param>
/// <param name="directed">是否为有向图，默认为 false（无向图）</param>
/// <param name="output">输出文件路径，若为 null 则使用全局配置</param>
/// <returns>带权图实例</returns>
public static GraphWeighted GraphWeighted(
    string label = "graph",
    bool directed = false,
    string? output = null
);
```

**GraphWeighted 类方法：**

```csharp
public sealed class GraphWeighted : IDisposable
{
    /// <summary>
    /// 添加节点
    /// </summary>
    /// <param name="nodeId">节点 id</param>
    /// <param name="label">节点标签，长度限制为 1-32 字符</param>
    /// <exception cref="InvalidOperationException">当节点已存在时抛出</exception>
    /// <exception cref="ArgumentException">当标签长度不在 1-32 范围内时抛出</exception>
    public void AddNode(int nodeId, string label);

    /// <summary>
    /// 添加带权边
    /// </summary>
    /// <param name="fromId">起始节点 id</param>
    /// <param name="toId">终止节点 id</param>
    /// <param name="weight">边的权重</param>
    /// <exception cref="InvalidOperationException">当节点不存在、边已存在或形成自环时抛出</exception>
    public void AddEdge(int fromId, int toId, double weight);

    /// <summary>
    /// 删除节点及其相关的所有边
    /// </summary>
    /// <param name="nodeId">要删除的节点 id</param>
    /// <exception cref="InvalidOperationException">当节点不存在时抛出</exception>
    public void RemoveNode(int nodeId);

    /// <summary>
    /// 删除边
    /// </summary>
    /// <param name="fromId">起始节点 id</param>
    /// <param name="toId">终止节点 id</param>
    /// <exception cref="InvalidOperationException">当边不存在时抛出</exception>
    public void RemoveEdge(int fromId, int toId);

    /// <summary>
    /// 更新边的权重
    /// </summary>
    /// <param name="fromId">起始节点 id</param>
    /// <param name="toId">终止节点 id</param>
    /// <param name="weight">新的权重值</param>
    /// <exception cref="InvalidOperationException">当边不存在时抛出</exception>
    public void UpdateWeight(int fromId, int toId, double weight);

    /// <summary>
    /// 更新节点标签
    /// </summary>
    /// <param name="nodeId">节点 id</param>
    /// <param name="label">新的节点标签，长度限制为 1-32 字符</param>
    /// <exception cref="InvalidOperationException">当节点不存在时抛出</exception>
    /// <exception cref="ArgumentException">当标签长度不在 1-32 范围内时抛出</exception>
    public void UpdateNodeLabel(int nodeId, string label);
}
```

**示例：**

```csharp
using Ds4Viz;

// 无向带权图
using (var graph = Ds4Viz.Api.GraphWeighted(label: "demo_weighted_graph", directed: false))
{
    graph.AddNode(0, "A");
    graph.AddNode(1, "B");
    graph.AddNode(2, "C");

    graph.AddEdge(0, 1, 3.5);
    graph.AddEdge(1, 2, 2.0);
    graph.AddEdge(2, 0, 4.2);

    graph.UpdateWeight(0, 1, 5.0);
    graph.RemoveEdge(2, 0);
}

// 有向带权图
using (var graph = Ds4Viz.Api.GraphWeighted(label: "demo_directed_weighted", directed: true))
{
    graph.AddNode(0, "Start");
    graph.AddNode(1, "Middle");
    graph.AddNode(2, "End");

    graph.AddEdge(0, 1, 1.5);
    graph.AddEdge(1, 2, 2.5);
}
```

---

### 异常处理

所有数据结构在遇到错误时都会抛出 `InvalidOperationException` 或 `ArgumentException`，并在退出 `using` 作用域时自动生成包含 `[error]` 部分的 `.toml` 文件。

**示例：**

```csharp
using Ds4Viz;

try
{
    using (var tree = Ds4Viz.Api.BinaryTree())
    {
        var root = tree.InsertRoot(10);
        tree.InsertLeft(999, 5); // 父节点不存在，抛出异常
    }
}
catch (InvalidOperationException e)
{
    Console.WriteLine($"发生错误: {e.Message}");
}

// 即使发生异常，trace.toml 仍会生成，包含 [error] 部分
```

生成的 `trace.toml` 将包含：

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

根据 IR 定义，`value` 字段仅支持以下类型：

- `int`：整数
- `double`：浮点数（C# 中对应 Python 的 float）
- `string`：字符串
- `bool`：布尔值

所有 API 在参数中使用 `object` 类型接收值，内部会进行类型检查，仅接受上述四种类型。对于 BST 和 Heap，由于需要比较大小，仅支持数值类型（`int` 和 `double`）。
