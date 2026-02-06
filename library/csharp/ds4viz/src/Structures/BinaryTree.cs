/*
 * 二叉树数据结构实现
 * 提供二叉树的可视化追踪功能，支持节点插入、删除、值更新操作
 * 
 * @author: WaterRun
 * @file: src/Structures/BinaryTree.cs
 * @date: 2026-02-06
 */

#nullable enable

using Ds4Viz.Core;

namespace Ds4Viz.Structures;

/// <summary>二叉树数据结构，提供树形节点的存储与追踪功能。</summary>
/// <remarks>
/// 不变量：节点 Id 从 0 开始连续分配；_root 为 -1 表示空树。
/// 线程安全：不保证线程安全。
/// 副作用：所有操作方法都会记录追踪步骤。
/// </remarks>
public sealed class BinaryTree : BaseStructure
{
    /// <summary>二叉树节点内部表示。</summary>
    private sealed class TreeNode
    {
        /// <summary>节点存储的值。</summary>
        public object Value { get; set; }

        /// <summary>左子节点的 Id，-1 表示无左子。</summary>
        public int Left { get; set; }

        /// <summary>右子节点的 Id，-1 表示无右子。</summary>
        public int Right { get; set; }

        /// <summary>创建节点实例。</summary>
        /// <param name="value">节点值。</param>
        public TreeNode(object value)
        {
            Value = value;
            Left = -1;
            Right = -1;
        }
    }

    /// <summary>节点存储字典，键为节点 Id。</summary>
    private readonly Dictionary<int, TreeNode> _nodes = [];

    /// <summary>根节点 Id，-1 表示空树。</summary>
    private int _root = -1;

    /// <summary>下一个可用的节点 Id。</summary>
    private int _nextId;

    /// <summary>创建二叉树实例。</summary>
    /// <param name="label">二叉树的标签，默认为 "binary_tree"。</param>
    /// <param name="output">TOML 输出路径，null 表示使用全局配置。</param>
    internal BinaryTree(string label = "binary_tree", string? output = null)
        : base("binary_tree", label, output)
    {
    }

    /// <summary>构建二叉树的当前状态数据。</summary>
    /// <returns>包含 root 和 nodes 字段的状态数据字典。</returns>
    protected override Dictionary<string, object> BuildData()
    {
        var nodesList = _nodes
            .OrderBy(kv => kv.Key)
            .Select(kv => new Dictionary<string, object>
            {
                ["id"] = kv.Key,
                ["value"] = kv.Value.Value,
                ["left"] = kv.Value.Left,
                ["right"] = kv.Value.Right
            })
            .Cast<object>()
            .ToList();

        return new Dictionary<string, object>
        {
            ["root"] = _root,
            ["nodes"] = nodesList
        };
    }

    /// <summary>插入根节点。</summary>
    /// <param name="value">根节点的值。</param>
    /// <param name="callerLine">调用者行号。</param>
    /// <returns>根节点的 Id。</returns>
    /// <exception cref="StructureException">当根节点已存在时抛出。</exception>
    public int InsertRoot(object value, [CallerLineNumber] int callerLine = 0)
    {
        ValidateValue(value);

        if (_root != -1)
        {
            RaiseError("Root node already exists", callerLine);
        }

        var before = _session.GetLastStateId();
        var newId = _nextId++;
        _nodes[newId] = new TreeNode(value);
        _root = newId;
        var after = _session.AddState(BuildData());

        _session.AddStep(
            "insert_root",
            before,
            after,
            new Dictionary<string, object> { ["value"] = value },
            ret: newId,
            line: callerLine);

        return newId;
    }

    /// <summary>在指定父节点的左侧插入子节点。</summary>
    /// <param name="parentId">父节点的 Id。</param>
    /// <param name="value">要插入的子节点值。</param>
    /// <param name="callerLine">调用者行号。</param>
    /// <returns>新插入节点的 Id。</returns>
    /// <exception cref="StructureException">当父节点不存在或左子节点已存在时抛出。</exception>
    public int InsertLeft(int parentId, object value, [CallerLineNumber] int callerLine = 0)
    {
        ValidateValue(value);

        if (!_nodes.ContainsKey(parentId))
        {
            RaiseError($"Parent node not found: {parentId}", callerLine);
        }

        if (_nodes[parentId].Left != -1)
        {
            RaiseError($"Left child already exists for node: {parentId}", callerLine);
        }

        var before = _session.GetLastStateId();
        var newId = _nextId++;
        _nodes[newId] = new TreeNode(value);
        _nodes[parentId].Left = newId;
        var after = _session.AddState(BuildData());

        _session.AddStep(
            "insert_left",
            before,
            after,
            new Dictionary<string, object> { ["parent"] = parentId, ["value"] = value },
            ret: newId,
            line: callerLine);

        return newId;
    }

    /// <summary>在指定父节点的右侧插入子节点。</summary>
    /// <param name="parentId">父节点的 Id。</param>
    /// <param name="value">要插入的子节点值。</param>
    /// <param name="callerLine">调用者行号。</param>
    /// <returns>新插入节点的 Id。</returns>
    /// <exception cref="StructureException">当父节点不存在或右子节点已存在时抛出。</exception>
    public int InsertRight(int parentId, object value, [CallerLineNumber] int callerLine = 0)
    {
        ValidateValue(value);

        if (!_nodes.ContainsKey(parentId))
        {
            RaiseError($"Parent node not found: {parentId}", callerLine);
        }

        if (_nodes[parentId].Right != -1)
        {
            RaiseError($"Right child already exists for node: {parentId}", callerLine);
        }

        var before = _session.GetLastStateId();
        var newId = _nextId++;
        _nodes[newId] = new TreeNode(value);
        _nodes[parentId].Right = newId;
        var after = _session.AddState(BuildData());

        _session.AddStep(
            "insert_right",
            before,
            after,
            new Dictionary<string, object> { ["parent"] = parentId, ["value"] = value },
            ret: newId,
            line: callerLine);

        return newId;
    }

    /// <summary>删除指定节点及其所有子树。</summary>
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
        var subtreeIds = CollectSubtree(nodeId);
        var parentId = FindParent(nodeId);

        if (parentId != -1)
        {
            if (_nodes[parentId].Left == nodeId)
            {
                _nodes[parentId].Left = -1;
            }
            else
            {
                _nodes[parentId].Right = -1;
            }
        }
        else
        {
            _root = -1;
        }

        foreach (var id in subtreeIds)
        {
            _nodes.Remove(id);
        }

        var after = _session.AddState(BuildData());

        _session.AddStep(
            "delete",
            before,
            after,
            new Dictionary<string, object> { ["node_id"] = nodeId },
            line: callerLine);
    }

    /// <summary>更新节点的值。</summary>
    /// <param name="nodeId">节点 Id。</param>
    /// <param name="value">新的值。</param>
    /// <param name="callerLine">调用者行号。</param>
    /// <exception cref="StructureException">当节点不存在时抛出。</exception>
    public void UpdateValue(int nodeId, object value, [CallerLineNumber] int callerLine = 0)
    {
        ValidateValue(value);

        if (!_nodes.ContainsKey(nodeId))
        {
            RaiseError($"Node not found: {nodeId}", callerLine);
        }

        var before = _session.GetLastStateId();
        var oldValue = _nodes[nodeId].Value;
        _nodes[nodeId].Value = value;
        var after = _session.AddState(BuildData());

        _session.AddStep(
            "update_value",
            before,
            after,
            new Dictionary<string, object> { ["node_id"] = nodeId, ["value"] = value },
            ret: oldValue,
            line: callerLine);
    }

    /// <summary>查找指定节点的父节点 Id。</summary>
    /// <param name="nodeId">目标节点 Id。</param>
    /// <returns>父节点 Id，若为根节点或不存在则返回 -1。</returns>
    private int FindParent(int nodeId)
    {
        foreach (var (id, node) in _nodes)
        {
            if (node.Left == nodeId || node.Right == nodeId)
            {
                return id;
            }
        }
        return -1;
    }

    /// <summary>收集以指定节点为根的子树中所有节点 Id。</summary>
    /// <param name="nodeId">根节点 Id。</param>
    /// <returns>子树中所有节点 Id 列表。</returns>
    private List<int> CollectSubtree(int nodeId)
    {
        if (nodeId == -1)
        {
            return [];
        }

        var result = new List<int> { nodeId };
        result.AddRange(CollectSubtree(_nodes[nodeId].Left));
        result.AddRange(CollectSubtree(_nodes[nodeId].Right));
        return result;
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