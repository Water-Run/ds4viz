/*
 * 队列数据结构实现
 * 提供队列的可视化追踪功能，支持 enqueue、dequeue、clear 操作
 * 
 * @author: WaterRun
 * @file: src/Structures/Queue.cs
 * @date: 2026-02-06
 */

#nullable enable

using Ds4Viz.Core;

namespace Ds4Viz.Structures;

/// <summary>队列数据结构，提供先进先出的元素存储与追踪功能。</summary>
/// <remarks>
/// 不变量：_items 中的元素类型仅限 int、double、string、bool。
/// 线程安全：不保证线程安全。
/// 副作用：所有操作方法都会记录追踪步骤。
/// </remarks>
public sealed class Queue : BaseStructure
{
    /// <summary>队列元素存储列表。</summary>
    private readonly List<object> _items = [];

    /// <summary>创建队列实例。</summary>
    /// <param name="label">队列的标签，默认为 "queue"。</param>
    /// <param name="output">TOML 输出路径，null 表示使用全局配置。</param>
    internal Queue(string label = "queue", string? output = null)
        : base("queue", label, output)
    {
    }

    /// <summary>构建队列的当前状态数据。</summary>
    /// <returns>包含 items、front、rear 字段的状态数据字典。</returns>
    protected override Dictionary<string, object> BuildData()
    {
        if (_items.Count == 0)
        {
            return new Dictionary<string, object>
            {
                ["items"] = new List<object>(),
                ["front"] = -1,
                ["rear"] = -1
            };
        }

        return new Dictionary<string, object>
        {
            ["items"] = _items.ToList<object>(),
            ["front"] = 0,
            ["rear"] = _items.Count - 1
        };
    }

    /// <summary>将元素加入队尾。</summary>
    /// <param name="value">要入队的值，仅支持 int、double、string、bool 类型。</param>
    /// <param name="callerLine">调用者行号，由编译器自动提供。</param>
    /// <exception cref="ArgumentException">当值类型不支持时抛出。</exception>
    public void Enqueue(object value, [CallerLineNumber] int callerLine = 0)
    {
        ValidateValue(value);

        var before = _session.GetLastStateId();
        _items.Add(value);
        var after = _session.AddState(BuildData());

        _session.AddStep(
            "enqueue",
            before,
            after,
            new Dictionary<string, object> { ["value"] = value },
            line: callerLine);
    }

    /// <summary>移除队首元素。</summary>
    /// <param name="callerLine">调用者行号，由编译器自动提供。</param>
    /// <exception cref="StructureException">当队列为空时抛出。</exception>
    public void Dequeue([CallerLineNumber] int callerLine = 0)
    {
        if (_items.Count == 0)
        {
            RaiseError("Cannot dequeue from empty queue", callerLine);
        }

        var before = _session.GetLastStateId();
        var dequeued = _items[0];
        _items.RemoveAt(0);
        var after = _session.AddState(BuildData());

        _session.AddStep(
            "dequeue",
            before,
            after,
            [],
            ret: dequeued,
            line: callerLine);
    }

    /// <summary>清空队列中所有元素。</summary>
    /// <param name="callerLine">调用者行号，由编译器自动提供。</param>
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

    /// <summary>验证值类型是否支持。</summary>
    /// <param name="value">要验证的值。</param>
    /// <exception cref="ArgumentException">当类型不支持时抛出。</exception>
    private static void ValidateValue(object value)
    {
        if (value is not (int or double or string or bool))
        {
            throw new ArgumentException(
                $"不支持的值类型: {value.GetType().Name}，仅支持 int、double、string、bool",
                nameof(value));
        }
    }
}