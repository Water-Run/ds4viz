/*
 * 单链表数据结构实现
 * 提供单链表的可视化追踪功能，支持插入、删除、反转操作
 * 
 * @author: WaterRun
 * @file: src/Structures/SingleLinkedList.cs
 * @date: 2026-02-06
 */

#nullable enable

using Ds4Viz.Core;

namespace Ds4Viz.Structures;

/// <summary>单链表数据结构，提供单向链接节点的存储与追踪功能。</summary>
/// <remarks>
/// 不变量：节点 Id 从 0 开始连续分配；_head 为 -1 表示空链表。
/// 线程安全：不保证线程安全。
/// 副作用：所有操作方法都会记录追踪步骤。
/// </remarks>
public sealed class SingleLinkedList : BaseStructure
{
    /// <summary>单链表节点内部表示。</summary>
    private sealed class Node
    {
        /// <summary>节点存储的值。</summary>
        public object Value { get; set; }

        /// <summary>下一个节点的 Id，-1 表示无后继。</summary>
        public int Next { get; set; }

        /// <summary>创建节点实例。</summary>
        /// <param name="value">节点值。</param>
        /// <param name="next">下一个节点 Id。</param>
        public Node(object value, int next = -1)
        {
            Value = value;
            Next = next;
        }
    }

    /// <summary>节点存储字典，键为节点 Id。</summary>
    private readonly Dictionary<int, Node> _nodes = [];

    /// <summary>头节点 Id，-1 表示空链表。</summary>
    private int _head = -1;

    /// <summary>下一个可用的节点 Id。</summary>
    private int _nextId;

    /// <summary>创建单链表实例。</summary>
    /// <param name="label">单链表的标签，默认为 "slist"。</param>
    /// <param name="output">TOML 输出路径，null 表示使用全局配置。</param>
    internal SingleLinkedList(string label = "slist", string? output = null)
        : base("slist", label, output)
    {
    }

    /// <summary>构建单链表的当前状态数据。</summary>
    /// <returns>包含 head 和 nodes 字段的状态数据字典。</returns>
    protected override Dictionary<string, object> BuildData()
    {
        var nodesList = _nodes
            .OrderBy(kv => kv.Key)
            .Select(kv => new Dictionary<string, object>
            {
                ["id"] = kv.Key,
                ["value"] = kv.Value.Value,
                ["next"] = kv.Value.Next
            })
            .Cast<object>()
            .ToList();

        return new Dictionary<string, object>
        {
            ["head"] = _head,
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
        _nodes[newId] = new Node(value, _head);
        _head = newId;
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
        _nodes[newId] = new Node(value);

        if (_head == -1)
        {
            _head = newId;
        }
        else
        {
            var tail = FindTail();
            _nodes[tail].Next = newId;
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
        _nodes[newId] = new Node(value, oldNext);
        _nodes[nodeId].Next = newId;
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
        var deletedValue = _nodes[nodeId].Value;

        if (_head == nodeId)
        {
            _head = _nodes[nodeId].Next;
        }
        else
        {
            var prev = FindPrev(nodeId);
            if (prev != -1)
            {
                _nodes[prev].Next = _nodes[nodeId].Next;
            }
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

        var before = _session.GetLastStateId();
        var oldHead = _head;
        var deletedValue = _nodes[oldHead].Value;
        _head = _nodes[oldHead].Next;
        _nodes.Remove(oldHead);
        var after = _session.AddState(BuildData());

        _session.AddStep(
            "delete_head",
            before,
            after,
            [],
            ret: deletedValue,
            line: callerLine);
    }

    /// <summary>反转链表。</summary>
    /// <param name="callerLine">调用者行号。</param>
    public void Reverse([CallerLineNumber] int callerLine = 0)
    {
        var before = _session.GetLastStateId();

        var prev = -1;
        var current = _head;

        while (current != -1)
        {
            var next = _nodes[current].Next;
            _nodes[current].Next = prev;
            prev = current;
            current = next;
        }

        _head = prev;
        var after = _session.AddState(BuildData());

        _session.AddStep(
            "reverse",
            before,
            after,
            [],
            line: callerLine);
    }

    /// <summary>查找尾节点 Id。</summary>
    /// <returns>尾节点 Id，链表为空时返回 -1。</returns>
    private int FindTail()
    {
        if (_head == -1)
        {
            return -1;
        }

        var current = _head;
        while (_nodes[current].Next != -1)
        {
            current = _nodes[current].Next;
        }
        return current;
    }

    /// <summary>查找指定节点的前驱节点 Id。</summary>
    /// <param name="nodeId">目标节点 Id。</param>
    /// <returns>前驱节点 Id，若为头节点或不存在则返回 -1。</returns>
    private int FindPrev(int nodeId)
    {
        if (_head == -1 || _head == nodeId)
        {
            return -1;
        }

        var current = _head;
        while (current != -1)
        {
            if (_nodes[current].Next == nodeId)
            {
                return current;
            }
            current = _nodes[current].Next;
        }
        return -1;
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