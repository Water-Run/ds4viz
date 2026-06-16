/*
 * 堆数据结构实现
 * 提供堆的可视化追踪功能，以完全二叉树形式表示，支持最小堆和最大堆
 * 
 * @author: WaterRun
 * @file: src/Structures/Heap.cs
 * @date: 2026-02-06
 */

#nullable enable

using Ds4Viz.Core;

namespace Ds4Viz.Structures;

/// <summary>堆数据结构，以完全二叉树形式实现优先队列功能。</summary>
/// <remarks>
/// 不变量：最小堆中父节点值小于等于子节点；最大堆中父节点值大于等于子节点。
/// 线程安全：不保证线程安全。
/// 副作用：所有操作方法都会记录追踪步骤。
/// </remarks>
public sealed class Heap : BaseStructure
{
    /// <summary>堆元素存储列表。</summary>
    private readonly List<double> _items = [];

    /// <summary>堆类型，true 表示最小堆，false 表示最大堆。</summary>
    private readonly bool _isMinHeap;

    /// <summary>创建堆实例。</summary>
    /// <param name="label">堆的标签，默认为 "heap"。</param>
    /// <param name="heapType">堆类型，"min" 表示最小堆，"max" 表示最大堆。</param>
    /// <param name="output">TOML 输出路径，null 表示使用全局配置。</param>
    /// <exception cref="ArgumentException">当 heapType 不是 "min" 或 "max" 时抛出。</exception>
    internal Heap(string label = "heap", string heapType = "min", string? output = null)
        : base("binary_tree", label, output)
    {
        if (heapType is not ("min" or "max"))
        {
            throw new ArgumentException(
                $"heapType 必须是 'min' 或 'max'，收到: '{heapType}'",
                nameof(heapType));
        }

        _isMinHeap = heapType == "min";
    }

    /// <summary>构建堆的当前状态数据（以完全二叉树形式）。</summary>
    /// <returns>包含 root 和 nodes 字段的状态数据字典。</returns>
    protected override Dictionary<string, object> BuildData()
    {
        if (_items.Count == 0)
        {
            return new Dictionary<string, object>
            {
                ["root"] = -1,
                ["nodes"] = new List<object>()
            };
        }

        var nodesList = _items
            .Select((value, index) => new Dictionary<string, object>
            {
                ["id"] = index,
                ["value"] = value,
                ["left"] = 2 * index + 1 < _items.Count ? 2 * index + 1 : -1,
                ["right"] = 2 * index + 2 < _items.Count ? 2 * index + 2 : -1
            })
            .Cast<object>()
            .ToList();

        return new Dictionary<string, object>
        {
            ["root"] = 0,
            ["nodes"] = nodesList
        };
    }

    /// <summary>插入元素，自动维护堆性质。</summary>
    /// <param name="value">要插入的值。</param>
    /// <param name="callerLine">调用者行号。</param>
    public void Insert(double value, [CallerLineNumber] int callerLine = 0)
    {
        var before = _session.GetLastStateId();
        _items.Add(value);
        SiftUp(_items.Count - 1);
        var after = _session.AddState(BuildData());

        _session.AddStep(
            "insert",
            before,
            after,
            new Dictionary<string, object> { ["value"] = value },
            line: callerLine);
    }

    /// <summary>提取堆顶元素，自动维护堆性质。</summary>
    /// <param name="callerLine">调用者行号。</param>
    /// <exception cref="StructureException">当堆为空时抛出。</exception>
    public void Extract([CallerLineNumber] int callerLine = 0)
    {
        if (_items.Count == 0)
        {
            RaiseError("Cannot extract from empty heap", callerLine);
        }

        var before = _session.GetLastStateId();
        var extracted = _items[0];

        if (_items.Count == 1)
        {
            _items.RemoveAt(0);
        }
        else
        {
            _items[0] = _items[^1];
            _items.RemoveAt(_items.Count - 1);
            SiftDown(0);
        }

        var after = _session.AddState(BuildData());

        _session.AddStep(
            "extract",
            before,
            after,
            [],
            ret: extracted,
            line: callerLine);
    }

    /// <summary>清空堆中所有元素。</summary>
    /// <param name="callerLine">调用者行号。</param>
    public void Clear([CallerLineNumber] int callerLine = 0)
    {
        var before = _session.GetLastStateId();
        _items.Clear();
        var after = _session.AddState(BuildData());

        _session.AddStep(
            "clear",
            before,
            after,
            [],
            line: callerLine);
    }

    /// <summary>向上调整堆。</summary>
    /// <param name="index">开始调整的索引。</param>
    private void SiftUp(int index)
    {
        while (index > 0)
        {
            var parent = (index - 1) / 2;
            if (Compare(_items[index], _items[parent]))
            {
                (_items[index], _items[parent]) = (_items[parent], _items[index]);
                index = parent;
            }
            else
            {
                break;
            }
        }
    }

    /// <summary>向下调整堆。</summary>
    /// <param name="index">开始调整的索引。</param>
    private void SiftDown(int index)
    {
        var size = _items.Count;
        while (true)
        {
            var target = index;
            var left = 2 * index + 1;
            var right = 2 * index + 2;

            if (left < size && Compare(_items[left], _items[target]))
            {
                target = left;
            }
            if (right < size && Compare(_items[right], _items[target]))
            {
                target = right;
            }
            if (target == index)
            {
                break;
            }

            (_items[index], _items[target]) = (_items[target], _items[index]);
            index = target;
        }
    }

    /// <summary>比较两个值，根据堆类型决定比较方式。</summary>
    /// <param name="a">第一个值。</param>
    /// <param name="b">第二个值。</param>
    /// <returns>如果 a 应该在 b 之前（更接近堆顶）则返回 true。</returns>
    private bool Compare(double a, double b)
    {
        return _isMinHeap ? a < b : a > b;
    }
}