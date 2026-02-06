/*
 * 有向图数据结构实现
 * 提供有向图的可视化追踪功能，支持节点和有向边的增删操作
 * 
 * @author: WaterRun
 * @file: src/Structures/GraphDirected.cs
 * @date: 2026-02-06
 */

#nullable enable

using Ds4Viz.Core;

namespace Ds4Viz.Structures;

/// <summary>有向图数据结构，提供有向边连接的节点存储与追踪功能。</summary>
/// <remarks>
/// 不变量：不允许自环和重边；边的方向有意义。
/// 线程安全：不保证线程安全。
/// 副作用：所有操作方法都会记录追踪步骤。
/// </remarks>
public sealed class GraphDirected : BaseStructure
{
    /// <summary>节点存储字典，键为节点 Id，值为标签。</summary>
    private readonly Dictionary<int, string> _nodes = [];

    /// <summary>有向边集合，存储为 (from, to) 元组。</summary>
    private readonly HashSet<(int From, int To)> _edges = [];

    /// <summary>创建有向图实例。</summary>
    /// <param name="label">有向图的标签，默认为 "graph"。</param>
    /// <param name="output">TOML 输出路径，null 表示使用全局配置。</param>
    internal GraphDirected(string label = "graph", string? output = null)
        : base("graph_directed", label, output)
    {
    }

    /// <summary>构建有向图的当前状态数据。</summary>
    /// <returns>包含 nodes 和 edges 字段的状态数据字典。</returns>
    protected override Dictionary<string, object> BuildData()
    {
        var nodesList = _nodes
            .OrderBy(kv => kv.Key)
            .Select(kv => new Dictionary<string, object>
            {
                ["id"] = kv.Key,
                ["label"] = kv.Value
            })
            .Cast<object>()
            .ToList();

        var edgesList = _edges
            .OrderBy(e => e.From)
            .ThenBy(e => e.To)
            .Select(e => new Dictionary<string, object>
            {
                ["from"] = e.From,
                ["to"] = e.To
            })
            .Cast<object>()
            .ToList();

        return new Dictionary<string, object>
        {
            ["nodes"] = nodesList,
            ["edges"] = edgesList
        };
    }

    /// <summary>添加节点。</summary>
    /// <param name="nodeId">节点 Id。</param>
    /// <param name="label">节点标签，长度限制 1-32 字符。</param>
    /// <param name="callerLine">调用者行号。</param>
    /// <exception cref="StructureException">当节点已存在时抛出。</exception>
    /// <exception cref="ArgumentException">当标签长度不合法时抛出。</exception>
    public void AddNode(int nodeId, string label, [CallerLineNumber] int callerLine = 0)
    {
        ValidateLabel(label);

        if (_nodes.ContainsKey(nodeId))
        {
            RaiseError($"Node already exists: {nodeId}", callerLine);
        }

        var before = _session.GetLastStateId();
        _nodes[nodeId] = label;
        var after = _session.AddState(BuildData());

        _session.AddStep(
            "add_node",
            before,
            after,
            new Dictionary<string, object> { ["id"] = nodeId, ["label"] = label },
            line: callerLine);
    }

    /// <summary>添加有向边。</summary>
    /// <param name="fromId">起始节点 Id。</param>
    /// <param name="toId">终止节点 Id。</param>
    /// <param name="callerLine">调用者行号。</param>
    /// <exception cref="StructureException">当节点不存在、边已存在或形成自环时抛出。</exception>
    public void AddEdge(int fromId, int toId, [CallerLineNumber] int callerLine = 0)
    {
        if (!_nodes.ContainsKey(fromId))
        {
            RaiseError($"Node not found: {fromId}", callerLine);
        }
        if (!_nodes.ContainsKey(toId))
        {
            RaiseError($"Node not found: {toId}", callerLine);
        }
        if (fromId == toId)
        {
            RaiseError($"Self-loop not allowed: {fromId}", callerLine);
        }

        var edge = (fromId, toId);
        if (_edges.Contains(edge))
        {
            RaiseError($"Edge already exists: ({fromId}, {toId})", callerLine);
        }

        var before = _session.GetLastStateId();
        _edges.Add(edge);
        var after = _session.AddState(BuildData());

        _session.AddStep(
            "add_edge",
            before,
            after,
            new Dictionary<string, object> { ["from"] = fromId, ["to"] = toId },
            line: callerLine);
    }

    /// <summary>删除节点及其相关的所有边。</summary>
    /// <param name="nodeId">要删除的节点 Id。</param>
    /// <param name="callerLine">调用者行号。</param>
    /// <exception cref="StructureException">当节点不存在时抛出。</exception>
    public void RemoveNode(int nodeId, [CallerLineNumber] int callerLine = 0)
    {
        if (!_nodes.ContainsKey(nodeId))
        {
            RaiseError($"Node not found: {nodeId}", callerLine);
        }

        var before = _session.GetLastStateId();

        _edges.RemoveWhere(e => e.From == nodeId || e.To == nodeId);
        _nodes.Remove(nodeId);

        var after = _session.AddState(BuildData());

        _session.AddStep(
            "remove_node",
            before,
            after,
            new Dictionary<string, object> { ["node_id"] = nodeId },
            line: callerLine);
    }

    /// <summary>删除边。</summary>
    /// <param name="fromId">起始节点 Id。</param>
    /// <param name="toId">终止节点 Id。</param>
    /// <param name="callerLine">调用者行号。</param>
    /// <exception cref="StructureException">当边不存在时抛出。</exception>
    public void RemoveEdge(int fromId, int toId, [CallerLineNumber] int callerLine = 0)
    {
        var edge = (fromId, toId);
        if (!_edges.Contains(edge))
        {
            RaiseError($"Edge not found: ({fromId}, {toId})", callerLine);
        }

        var before = _session.GetLastStateId();
        _edges.Remove(edge);
        var after = _session.AddState(BuildData());

        _session.AddStep(
            "remove_edge",
            before,
            after,
            new Dictionary<string, object> { ["from"] = fromId, ["to"] = toId },
            line: callerLine);
    }

    /// <summary>更新节点标签。</summary>
    /// <param name="nodeId">节点 Id。</param>
    /// <param name="label">新的节点标签，长度限制 1-32 字符。</param>
    /// <param name="callerLine">调用者行号。</param>
    /// <exception cref="StructureException">当节点不存在时抛出。</exception>
    /// <exception cref="ArgumentException">当标签长度不合法时抛出。</exception>
    public void UpdateNodeLabel(int nodeId, string label, [CallerLineNumber] int callerLine = 0)
    {
        ValidateLabel(label);

        if (!_nodes.ContainsKey(nodeId))
        {
            RaiseError($"Node not found: {nodeId}", callerLine);
        }

        var before = _session.GetLastStateId();
        var oldLabel = _nodes[nodeId];
        _nodes[nodeId] = label;
        var after = _session.AddState(BuildData());

        _session.AddStep(
            "update_node_label",
            before,
            after,
            new Dictionary<string, object> { ["node_id"] = nodeId, ["label"] = label },
            ret: oldLabel,
            line: callerLine);
    }

    /// <summary>验证标签长度。</summary>
    /// <param name="label">要验证的标签。</param>
    private static void ValidateLabel(string label)
    {
        ArgumentNullException.ThrowIfNull(label);
        if (label.Length < 1 || label.Length > 32)
        {
            throw new ArgumentException(
                $"标签长度必须在 1-32 之间，收到: {label.Length}",
                nameof(label));
        }
    }
}