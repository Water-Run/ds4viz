/*
 * 栈数据结构实现
 * 提供栈的可视化追踪功能，支持 push、pop、clear 操作
 * 
 * @author: WaterRun
 * @file: src/Structures/Stack.cs
 * @date: 2026-02-06
 */

#nullable enable

using Ds4Viz.Core;

namespace Ds4Viz.Structures;

/// <summary>栈数据结构，提供后进先出的元素存储与追踪功能。</summary>
/// <remarks>
/// 不变量：_items 中的元素类型仅限 int、double、string、bool。
/// 线程安全：不保证线程安全。
/// 副作用：所有操作方法都会记录追踪步骤。
/// </remarks>
public sealed class Stack : BaseStructure
{
    /// <summary>栈元素存储列表。</summary>
    private readonly List<object> _items = [];

    /// <summary>创建栈实例。</summary>
    /// <param name="label">栈的标签，默认为 "stack"。</param>
    /// <param name="output">TOML 输出路径，null 表示使用全局配置。</param>
    internal Stack(string label = "stack", string? output = null)
        : base("stack", label, output)
    {
    }

    /// <summary>构建栈的当前状态数据。</summary>
    /// <returns>包含 items 和 top 字段的状态数据字典。</returns>
    protected override Dictionary<string, object> BuildData()
    {
        return new Dictionary<string, object>
        {
            ["items"] = _items.ToList<object>(),
            ["top"] = _items.Count > 0 ? _items.Count - 1 : -1
        };
    }

    /// <summary>将元素压入栈顶。</summary>
    /// <param name="value">要压入的值，仅支持 int、double、string、bool 类型。</param>
    /// <param name="callerLine">调用者行号，由编译器自动提供。</param>
    /// <exception cref="ArgumentException">当值类型不支持时抛出。</exception>
    public void Push(object value, [CallerLineNumber] int callerLine = 0)
    {
        ValidateValue(value);

        var before = _session.GetLastStateId();
        _items.Add(value);
        var after = _session.AddState(BuildData());

        _session.AddStep(
            "push",
            before,
            after,
            new Dictionary<string, object> { ["value"] = value },
            line: callerLine);
    }

    /// <summary>弹出栈顶元素。</summary>
    /// <param name="callerLine">调用者行号，由编译器自动提供。</param>
    /// <exception cref="StructureException">当栈为空时抛出。</exception>
    public void Pop([CallerLineNumber] int callerLine = 0)
    {
        if (_items.Count == 0)
        {
            RaiseError("Cannot pop from empty stack", callerLine);
        }

        var before = _session.GetLastStateId();
        var popped = _items[^1];
        _items.RemoveAt(_items.Count - 1);
        var after = _session.AddState(BuildData());

        _session.AddStep(
            "pop",
            before,
            after,
            [],
            ret: popped,
            line: callerLine);
    }

    /// <summary>清空栈中所有元素。</summary>
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