/*
 * 双向链表数据结构实现
 * 提供双向链表的可视化追踪功能，支持双向插入、删除、反转操作
 * 
 * @author: WaterRun
 * @file: src/Structures/DoubleLinkedList.cs
 * @date: 2026-02-06
 */

#nullable enable

using Ds4Viz.Core;

namespace Ds4Viz.Structures;

/// <summary>双向链表数据结构，提供双向链接节点的存储与追踪功能。</summary>
/// <remarks>
/// 不变量：节点 Id 从 0 开始连续分配；_head 和 _tail 同时为 -1 或同时不为 -1。
/// 线程安全：不保证线程安全。
/// 副作用：所有操作方法都会记录追踪步骤。
/// </remarks>
public sealed class DoubleLinkedList : BaseStructure
{
    /// <summary>双向链表节点内部表示。</summary>
    private sealed class Node
    {
        /// <summary>节点存储的值。</summary>
        public object Value { get; set; }

        /// <summary>前一个节点的 Id，-1 表示无前驱。</summary>
        public int Prev { get; set; }

        /// <summary>下一个节点的 Id，-1 表示无后继。</summary>
        public int Next { get; set; }

        /// <summary>创建节点实例。</summary>
        /// <param name="value">节点值。</param>
        /// <param name="prev">前一个节点 Id。</param>
        /// <param name="next">下一个节点 Id。</param>
        public Node(object value, int prev = -1, int next = -1)
        {
            Value = value;
            Prev = prev;
            Next = next;
        }
    }

    /// <summary>节点存储字典，键为节点 Id。</summary>
    private readonly Dictionary<int, Node> _nodes = [];

    /// <summary>头节点 Id，-1 表示空链表。</summary>
    private int _head = -1;

    /// <summary>尾节点 Id，-1 表示空链表。</summary>
    private int _tail = -1;

    /// <summary>下一个可用的节点 Id。</summary>
    private int _nextId;

    /// <summary>创建双向链表实例。</summary>
    /// <param name="label">双向链表的标签，默认为 "dlist"。</param>
    /// <param name="output">TOML 输出路径，null 表示使用全局配置。</param>
    internal DoubleLinkedList(string label = "dlist", string? output = null)
        : base("dlist", label, output)
    {
    }

    /// <summary>构建双向链表的当前状态数据。</summary>
    /// <returns>包含 head、tail 和 nodes 字段的状态数据字典。</returns>
    protected override Dictionary<string, object> BuildData()
    {
        var nodesList = _nodes
            .OrderBy(kv => kv.Key)
            .Select(kv => new Dictionary<string, object>
            {
                ["id"] = kv.Key,
                ["value"] = kv.Value.Value,
                ["prev"] = kv.Value.Prev,
                ["next"] = kv.Value.Next
            })
            .Cast<object>()
            .ToList();

        return new Dictionary<string, object>
        {
            ["head"] = _head,
            ["tail"] = _tail,
            ["nodes"] = nodesList
        };
    }

    /// <summary>在链表头部插入节点。</summary>
    /// <param name="value">要插入的值。</param>
    /// <param name="callerLine">调用者行号。</param>
    /// <returns>新插入节点的 Id。</returns>
    public int InsertHead(object value, [CallerLineNumber] int callerLine = 0)
    {
        ValidateValue(value);

        var before = _session.GetLastStateId();
        var newId = _nextId++;
        _nodes[newId] = new Node(value, -1, _head);

        if (_head != -1)
        {
            _nodes[_head].Prev = newId;
        }

        _head = newId;

        if (_tail == -1)
        {
            _tail = newId;
        }

        var after = _session.AddState(BuildData());

        _session.AddStep(
            "insert_head",
            before,
            after,
            new Dictionary<string, object> { ["value"] = value },
            ret: newId,
            line: callerLine);

        return newId;
    }

    /// <summary>在链表尾部插入节点。</summary>
    /// <param name="value">要插入的值。</param>
    /// <param name="callerLine">调用者行号。</param>
    /// <returns>新插入节点的 Id。</returns>
    public int InsertTail(object value, [CallerLineNumber] int callerLine = 0)
    {
        ValidateValue(value);

        var before = _session.GetLastStateId();
        var newId = _nextId++;
        _nodes[newId] = new Node(value, _tail, -1);

        if (_tail != -1)
        {
            _nodes[_tail].Next = newId;
        }

        _tail = newId;

        if (_head == -1)
        {
            _head = newId;
        }

        var after = _session.AddState(BuildData());

        _session.AddStep(
            "insert_tail",
            before,
            after,
            new Dictionary<string, object> { ["value"] = value },
            ret: newId,
            line: callerLine);

        return newId;
    }

    /// <summary>在指定节点前插入新节点。</summary>
    /// <param name="nodeId">目标节点的 Id。</param>
    /// <param name="value">要插入的值。</param>
    /// <param name="callerLine">调用者行号。</param>
    /// <returns>新插入节点的 Id。</returns>
    /// <exception cref="StructureException">当目标节点不存在时抛出。</exception>
    public int InsertBefore(int nodeId, object value, [CallerLineNumber] int callerLine = 0)
    {
        ValidateValue(value);

        if (!_nodes.ContainsKey(nodeId))
        {
            RaiseError($"Node not found: {nodeId}", callerLine);
        }

        var before = _session.GetLastStateId();
        var newId = _nextId++;
        var oldPrev = _nodes[nodeId].Prev;
        _nodes[newId] = new Node(value, oldPrev, nodeId);
        _nodes[nodeId].Prev = newId;

        if (oldPrev != -1)
        {
            _nodes[oldPrev].Next = newId;
        }
        else
        {
            _head = newId;
        }

        var after = _session.AddState(BuildData());

        _session.AddStep(
            "insert_before",
            before,
            after,
            new Dictionary<string, object> { ["node_id"] = nodeId, ["value"] = value },
            ret: newId,
            line: callerLine);

        return newId;
    }

    /// <summary>在指定节点后插入新节点。</summary>
    /// <param name="nodeId">目标节点的 Id。</param>
    /// <param name="value">要插入的值。</param>
    /// <param name="callerLine">调用者行号。</param>
    /// <returns>新插入节点的 Id。</returns>
    /// <exception cref="StructureException">当目标节点不存在时抛出。</exception>
    public int InsertAfter(int nodeId, object value, [CallerLineNumber] int callerLine = 0)
    {
        ValidateValue(value);

        if (!_nodes.ContainsKey(nodeId))
        {
            RaiseError($"Node not found: {nodeId}", callerLine);
        }

        var before = _session.GetLastStateId();
        var newId = _nextId++;
        var oldNext = _nodes[nodeId].Next;
        _nodes[newId] = new Node(value, nodeId, oldNext);
        _nodes[nodeId].Next = newId;

        if (oldNext != -1)
        {
            _nodes[oldNext].Prev = newId;
        }
        else
        {
            _tail = newId;
        }

        var after = _session.AddState(BuildData());

        _session.AddStep(
            "insert_after",
            before,
            after,
            new Dictionary<string, object> { ["node_id"] = nodeId, ["value"] = value },
            ret: newId,
            line: callerLine);

        return newId;
    }

    /// <summary>删除指定节点。</summary>
    /// <param name="nodeId">要删除的节点 Id。</param>
    /// <param name="callerLine">调用者行号。</param>
    /// <exception cref="StructureException">当节点不存在时抛出。</exception>
    public void Delete(int nodeId, [CallerLineNumber] int callerLine = 0)
    {
        if (!_nodes.ContainsKey(nodeId))
        {
            RaiseError($"Node not found: {nodeId}", callerLine);
        }

        var before = _session.GetLastStateId();
        var node = _nodes[nodeId];
        var deletedValue = node.Value;

        if (node.Prev != -1)
        {
            _nodes[node.Prev].Next = node.Next;
        }
        else
        {
            _head = node.Next;
        }

        if (node.Next != -1)
        {
            _nodes[node.Next].Prev = node.Prev;
        }
        else
        {
            _tail = node.Prev;
        }

        _nodes.Remove(nodeId);
        var after = _session.AddState(BuildData());

        _session.AddStep(
            "delete",
            before,
            after,
            new Dictionary<string, object> { ["node_id"] = nodeId },
            ret: deletedValue,
            line: callerLine);
    }

    /// <summary>删除头节点。</summary>
    /// <param name="callerLine">调用者行号。</param>
    /// <exception cref="StructureException">当链表为空时抛出。</exception>
    public void DeleteHead([CallerLineNumber] int callerLine = 0)
    {
        if (_head == -1)
        {
            RaiseError("Cannot delete from empty list", callerLine);
        }

        Delete(_head, callerLine);
    }

    /// <summary>删除尾节点。</summary>
    /// <param name="callerLine">调用者行号。</param>
    /// <exception cref="StructureException">当链表为空时抛出。</exception>
    public void DeleteTail([CallerLineNumber] int callerLine = 0)
    {
        if (_tail == -1)
        {
            RaiseError("Cannot delete from empty list", callerLine);
        }

        Delete(_tail, callerLine);
    }

    /// <summary>反转链表。</summary>
    /// <param name="callerLine">调用者行号。</param>
    public void Reverse([CallerLineNumber] int callerLine = 0)
    {
        var before = _session.GetLastStateId();

        var current = _head;
        while (current != -1)
        {
            var node = _nodes[current];
            (node.Prev, node.Next) = (node.Next, node.Prev);
            current = node.Prev;
        }

        (_head, _tail) = (_tail, _head);
        var after = _session.AddState(BuildData());

        _session.AddStep(
            "reverse",
            before,
            after,
            [],
            line: callerLine);
    }

    /// <summary>验证值类型是否支持。</summary>
    /// <param name="value">要验证的值。</param>
    private static void ValidateValue(object value)
    {
        if (value is not (int or double or string or bool))
        {
            throw new ArgumentException(
                $"不支持的值类型: {value.GetType().Name}",
                nameof(value));
        }
    }
}