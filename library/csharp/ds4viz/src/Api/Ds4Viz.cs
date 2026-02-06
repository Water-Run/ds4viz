/*
 * ds4viz 公共 API 入口
 * 提供创建各种数据结构实例的工厂方法和全局配置功能
 * 
 * @author: WaterRun
 * @file: src/Api/Ds4Viz.cs
 * @date: 2026-02-06
 */

#nullable enable

using Ds4Viz.Core;
using Ds4Viz.Structures;

namespace Ds4Viz;

/// <summary>ds4viz 库的公共 API 入口类，提供数据结构创建和全局配置功能。</summary>
/// <remarks>
/// 不变量：所有工厂方法返回的实例都已调用 Enter() 进入上下文。
/// 线程安全：Config 方法修改全局状态，不保证线程安全；工厂方法本身是线程安全的。
/// 副作用：创建的数据结构实例在 Dispose 时会写入 TOML 文件。
/// </remarks>
public static class Api
{
    /// <summary>配置全局参数。</summary>
    /// <param name="outputPath">输出文件路径，默认为 "trace.toml"，不允许 null 或空。</param>
    /// <param name="title">可视化标题，默认为空字符串。</param>
    /// <param name="author">作者信息，默认为空字符串。</param>
    /// <param name="comment">注释说明，默认为空字符串。</param>
    /// <exception cref="ArgumentNullException">当任意参数为 null 时抛出。</exception>
    /// <exception cref="ArgumentException">当 outputPath 为空字符串时抛出。</exception>
    public static void Config(
        string outputPath = "trace.toml",
        string title = "",
        string author = "",
        string comment = "")
    {
        var config = GlobalConfigHolder.Instance;
        config.OutputPath = outputPath;
        config.Title = title;
        config.Author = author;
        config.Comment = comment;
    }

    /// <summary>创建栈实例。</summary>
    /// <param name="label">栈的标签，默认为 "stack"。</param>
    /// <param name="output">输出文件路径，null 表示使用全局配置。</param>
    /// <param name="callerLine">调用者行号，由编译器自动提供。</param>
    /// <returns>已初始化的栈实例。</returns>
    public static Structures.Stack Stack(
        string label = "stack",
        string? output = null,
        [CallerLineNumber] int callerLine = 0)
    {
        var stack = new Structures.Stack(label, output);
        stack.Enter(callerLine);
        return stack;
    }

    /// <summary>创建队列实例。</summary>
    /// <param name="label">队列的标签，默认为 "queue"。</param>
    /// <param name="output">输出文件路径，null 表示使用全局配置。</param>
    /// <param name="callerLine">调用者行号，由编译器自动提供。</param>
    /// <returns>已初始化的队列实例。</returns>
    public static Structures.Queue Queue(
        string label = "queue",
        string? output = null,
        [CallerLineNumber] int callerLine = 0)
    {
        var queue = new Structures.Queue(label, output);
        queue.Enter(callerLine);
        return queue;
    }

    /// <summary>创建单链表实例。</summary>
    /// <param name="label">单链表的标签，默认为 "slist"。</param>
    /// <param name="output">输出文件路径，null 表示使用全局配置。</param>
    /// <param name="callerLine">调用者行号，由编译器自动提供。</param>
    /// <returns>已初始化的单链表实例。</returns>
    public static SingleLinkedList SingleLinkedList(
        string label = "slist",
        string? output = null,
        [CallerLineNumber] int callerLine = 0)
    {
        var list = new SingleLinkedList(label, output);
        list.Enter(callerLine);
        return list;
    }

    /// <summary>创建双向链表实例。</summary>
    /// <param name="label">双向链表的标签，默认为 "dlist"。</param>
    /// <param name="output">输出文件路径，null 表示使用全局配置。</param>
    /// <param name="callerLine">调用者行号，由编译器自动提供。</param>
    /// <returns>已初始化的双向链表实例。</returns>
    public static DoubleLinkedList DoubleLinkedList(
        string label = "dlist",
        string? output = null,
        [CallerLineNumber] int callerLine = 0)
    {
        var list = new DoubleLinkedList(label, output);
        list.Enter(callerLine);
        return list;
    }

    /// <summary>创建二叉树实例。</summary>
    /// <param name="label">二叉树的标签，默认为 "binary_tree"。</param>
    /// <param name="output">输出文件路径，null 表示使用全局配置。</param>
    /// <param name="callerLine">调用者行号，由编译器自动提供。</param>
    /// <returns>已初始化的二叉树实例。</returns>
    public static BinaryTree BinaryTree(
        string label = "binary_tree",
        string? output = null,
        [CallerLineNumber] int callerLine = 0)
    {
        var tree = new BinaryTree(label, output);
        tree.Enter(callerLine);
        return tree;
    }

    /// <summary>创建二叉搜索树实例。</summary>
    /// <param name="label">二叉搜索树的标签，默认为 "bst"。</param>
    /// <param name="output">输出文件路径，null 表示使用全局配置。</param>
    /// <param name="callerLine">调用者行号，由编译器自动提供。</param>
    /// <returns>已初始化的二叉搜索树实例。</returns>
    public static BinarySearchTree BinarySearchTree(
        string label = "bst",
        string? output = null,
        [CallerLineNumber] int callerLine = 0)
    {
        var bst = new BinarySearchTree(label, output);
        bst.Enter(callerLine);
        return bst;
    }

    /// <summary>创建堆实例。</summary>
    /// <param name="label">堆的标签，默认为 "heap"。</param>
    /// <param name="heapType">堆类型，"min" 表示最小堆，"max" 表示最大堆。</param>
    /// <param name="output">输出文件路径，null 表示使用全局配置。</param>
    /// <param name="callerLine">调用者行号，由编译器自动提供。</param>
    /// <returns>已初始化的堆实例。</returns>
    /// <exception cref="ArgumentException">当 heapType 不是 "min" 或 "max" 时抛出。</exception>
    public static Heap Heap(
        string label = "heap",
        string heapType = "min",
        string? output = null,
        [CallerLineNumber] int callerLine = 0)
    {
        var heap = new Heap(label, heapType, output);
        heap.Enter(callerLine);
        return heap;
    }

    /// <summary>创建无向图实例。</summary>
    /// <param name="label">无向图的标签，默认为 "graph"。</param>
    /// <param name="output">输出文件路径，null 表示使用全局配置。</param>
    /// <param name="callerLine">调用者行号，由编译器自动提供。</param>
    /// <returns>已初始化的无向图实例。</returns>
    public static GraphUndirected GraphUndirected(
        string label = "graph",
        string? output = null,
        [CallerLineNumber] int callerLine = 0)
    {
        var graph = new GraphUndirected(label, output);
        graph.Enter(callerLine);
        return graph;
    }

    /// <summary>创建有向图实例。</summary>
    /// <param name="label">有向图的标签，默认为 "graph"。</param>
    /// <param name="output">输出文件路径，null 表示使用全局配置。</param>
    /// <param name="callerLine">调用者行号，由编译器自动提供。</param>
    /// <returns>已初始化的有向图实例。</returns>
    public static GraphDirected GraphDirected(
        string label = "graph",
        string? output = null,
        [CallerLineNumber] int callerLine = 0)
    {
        var graph = new GraphDirected(label, output);
        graph.Enter(callerLine);
        return graph;
    }

    /// <summary>创建带权图实例。</summary>
    /// <param name="label">带权图的标签，默认为 "graph"。</param>
    /// <param name="directed">是否为有向图，默认为 false（无向图）。</param>
    /// <param name="output">输出文件路径，null 表示使用全局配置。</param>
    /// <param name="callerLine">调用者行号，由编译器自动提供。</param>
    /// <returns>已初始化的带权图实例。</returns>
    public static GraphWeighted GraphWeighted(
        string label = "graph",
        bool directed = false,
        string? output = null,
        [CallerLineNumber] int callerLine = 0)
    {
        var graph = new GraphWeighted(label, directed, output);
        graph.Enter(callerLine);
        return graph;
    }
}