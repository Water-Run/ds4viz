/*
 * 二叉搜索树数据结构实现
 * 提供二叉搜索树的可视化追踪功能，自动维护 BST 性质
 * 
 * @author: WaterRun
 * @file: src/Structures/BinarySearchTree.cs
 * @date: 2026-02-06
 */

#nullable enable

using Ds4Viz.Core;

namespace Ds4Viz.Structures;

/// <summary>二叉搜索树数据结构，提供有序树形节点的存储与追踪功能。</summary>
/// <remarks>
/// 不变量：对于任意节点，左子树所有值小于该节点，右子树所有值大于等于该节点。
/// 线程安全：不保证线程安全。
/// 副作用：所有操作方法都会记录追踪步骤。
/// </remarks>
public sealed class BinarySearchTree : BaseStructure
{
    /// <summary>二叉搜索树节点内部表示。</summary>
    private sealed class BstNode
    {
        /// <summary>节点存储的值。</summary>
        public double Value { get; set; }

        /// <summary>左子节点的 Id，-1 表示无左子。</summary>
        public int Left { get; set; }

        /// <summary>右子节点的 Id，-1 表示无右子。</summary>
        public int Right { get; set; }

        /// <summary>创建节点实例。</summary>
        /// <param name="value">节点值。</param>
        public BstNode(double value)
        {
            Value = value;
            Left = -1;
            Right = -1;
        }
    }

    /// <summary>节点存储字典，键为节点 Id。</summary>
    private readonly Dictionary<int, BstNode> _nodes = [];

    /// <summary>根节点 Id，-1 表示空树。</summary>
    private int _root = -1;

    /// <summary>下一个可用的节点 Id。</summary>
    private int _nextId;

    /// <summary>创建二叉搜索树实例。</summary>
    /// <param name="label">二叉搜索树的标签，默认为 "bst"。</param>
    /// <param name="output">TOML 输出路径，null 表示使用全局配置。</param>
    internal BinarySearchTree(string label = "bst", string? output = null)
        : base("bst", label, output)
    {
    }

    /// <summary>构建二叉搜索树的当前状态数据。</summary>
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

    /// <summary>插入节点，自动维护二叉搜索树性质。</summary>
    /// <param name="value">要插入的值。</param>
    /// <param name="callerLine">调用者行号。</param>
    /// <returns>新插入节点的 Id。</returns>
    public int Insert(double value, [CallerLineNumber] int callerLine = 0)
    {
        var before = _session.GetLastStateId();
        var newId = _nextId++;
        _nodes[newId] = new BstNode(value);

        if (_root == -1)
        {
            _root = newId;
        }
        else
        {
            var current = _root;
            while (true)
            {
                var currentValue = _nodes[current].Value;
                if (value < currentValue)
                {
                    if (_nodes[current].Left == -1)
                    {
                        _nodes[current].Left = newId;
                        break;
                    }
                    current = _nodes[current].Left;
                }
                else
                {
                    if (_nodes[current].Right == -1)
                    {
                        _nodes[current].Right = newId;
                        break;
                    }
                    current = _nodes[current].Right;
                }
            }
        }

        var after = _session.AddState(BuildData());

        _session.AddStep(
            "insert",
            before,
            after,
            new Dictionary<string, object> { ["value"] = value },
            ret: newId,
            line: callerLine);

        return newId;
    }

    /// <summary>删除节点，自动维护二叉搜索树性质。</summary>
    /// <param name="value">要删除的节点值。</param>
    /// <param name="callerLine">调用者行号。</param>
    /// <exception cref="StructureException">当节点不存在时抛出。</exception>
    public void Delete(double value, [CallerLineNumber] int callerLine = 0)
    {
        var nodeId = FindNodeByValue(value);
        if (nodeId == -1)
        {
            RaiseError($"Node with value {value} not found", callerLine);
        }

        var before = _session.GetLastStateId();
        DeleteNode(nodeId);
        var after = _session.AddState(BuildData());

        _session.AddStep(
            "delete",
            before,
            after,
            new Dictionary<string, object> { ["value"] = value },
            line: callerLine);
    }

    /// <summary>根据值查找节点 Id。</summary>
    /// <param name="value">要查找的值。</param>
    /// <returns>节点 Id，若不存在则返回 -1。</returns>
    private int FindNodeByValue(double value)
    {
        foreach (var (id, node) in _nodes)
        {
            if (Math.Abs(node.Value - value) < double.Epsilon)
            {
                return id;
            }
        }
        return -1;
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

    /// <summary>查找以指定节点为根的子树中的最小值节点。</summary>
    /// <param name="nodeId">子树根节点 Id。</param>
    /// <returns>最小值节点 Id。</returns>
    private int FindMinNode(int nodeId)
    {
        var current = nodeId;
        while (_nodes[current].Left != -1)
        {
            current = _nodes[current].Left;
        }
        return current;
    }

    /// <summary>删除指定节点并维护 BST 性质。</summary>
    /// <param name="nodeId">要删除的节点 Id。</param>
    private void DeleteNode(int nodeId)
    {
        var node = _nodes[nodeId];
        var parentId = FindParent(nodeId);
        var leftChild = node.Left;
        var rightChild = node.Right;

        if (leftChild == -1 && rightChild == -1)
        {
            ReplaceInParent(parentId, nodeId, -1);
            _nodes.Remove(nodeId);
        }
        else if (leftChild == -1)
        {
            ReplaceInParent(parentId, nodeId, rightChild);
            _nodes.Remove(nodeId);
        }
        else if (rightChild == -1)
        {
            ReplaceInParent(parentId, nodeId, leftChild);
            _nodes.Remove(nodeId);
        }
        else
        {
            var successorId = FindMinNode(rightChild);
            var successorValue = _nodes[successorId].Value;
            DeleteNode(successorId);
            _nodes[nodeId].Value = successorValue;
        }
    }

    /// <summary>在父节点中替换子节点引用。</summary>
    /// <param name="parentId">父节点 Id。</param>
    /// <param name="oldChild">旧子节点 Id。</param>
    /// <param name="newChild">新子节点 Id。</param>
    private void ReplaceInParent(int parentId, int oldChild, int newChild)
    {
        if (parentId == -1)
        {
            _root = newChild;
        }
        else if (_nodes[parentId].Left == oldChild)
        {
            _nodes[parentId].Left = newChild;
        }
        else
        {
            _nodes[parentId].Right = newChild;
        }
    }
}