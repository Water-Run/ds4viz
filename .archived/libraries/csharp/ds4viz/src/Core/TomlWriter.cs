/*
 * TOML 文件写入器
 * 将 Trace 对象序列化为符合 IR 规范的 TOML 格式文件
 * 
 * @author: WaterRun
 * @file: src/Core/TomlWriter.cs
 * @date: 2026-02-06
 */

#nullable enable

namespace Ds4Viz.Core;

/// <summary>TOML 格式序列化写入器，负责将追踪数据输出为 TOML 文件。</summary>
/// <remarks>
/// 不变量：_trace 引用在实例生命周期内不变。
/// 线程安全：实例方法不保证线程安全，不应并发调用。
/// 副作用：Write 方法会创建或覆盖文件。
/// </remarks>
internal sealed class TomlWriter
{
    /// <summary>要序列化的追踪数据。</summary>
    private readonly Trace _trace;

    /// <summary>创建 TOML 写入器实例。</summary>
    /// <param name="trace">要序列化的追踪数据，不允许 null。</param>
    /// <exception cref="ArgumentNullException">当 trace 为 null 时抛出。</exception>
    public TomlWriter(Trace trace)
    {
        ArgumentNullException.ThrowIfNull(trace);
        _trace = trace;
    }

    /// <summary>将追踪数据写入指定路径的 TOML 文件。</summary>
    /// <param name="path">输出文件路径，不允许 null 或空字符串。</param>
    /// <exception cref="ArgumentNullException">当 path 为 null 时抛出。</exception>
    /// <exception cref="ArgumentException">当 path 为空字符串时抛出。</exception>
    /// <exception cref="IOException">当文件写入失败时抛出。</exception>
    public void Write(string path)
    {
        ArgumentNullException.ThrowIfNull(path);
        if (string.IsNullOrWhiteSpace(path))
        {
            throw new ArgumentException("文件路径不能为空", nameof(path));
        }

        var content = Serialize();
        File.WriteAllText(path, content, Encoding.UTF8);
    }

    /// <summary>将追踪数据序列化为 TOML 格式字符串。</summary>
    /// <returns>完整的 TOML 格式字符串。</returns>
    private string Serialize()
    {
        var sb = new StringBuilder();

        AppendMeta(sb);
        sb.AppendLine();
        AppendPackage(sb);

        if (!_trace.Remarks.IsEmpty())
        {
            sb.AppendLine();
            AppendRemarks(sb);
        }

        sb.AppendLine();
        AppendObject(sb);

        foreach (var state in _trace.States)
        {
            sb.AppendLine();
            AppendState(sb, state);
        }

        foreach (var step in _trace.Steps)
        {
            sb.AppendLine();
            AppendStep(sb, step);
        }

        if (_trace.Result is not null)
        {
            sb.AppendLine();
            AppendResult(sb);
        }

        if (_trace.Error is not null)
        {
            sb.AppendLine();
            AppendError(sb);
        }

        return sb.ToString();
    }

    /// <summary>追加 [meta] 块。</summary>
    /// <param name="sb">字符串构建器。</param>
    private void AppendMeta(StringBuilder sb)
    {
        sb.AppendLine("[meta]");
        sb.AppendLine($"created_at = \"{_trace.Meta.CreatedAt}\"");
        sb.AppendLine($"lang = \"{_trace.Meta.Lang}\"");
        sb.AppendLine($"lang_version = \"{_trace.Meta.LangVersion}\"");
    }

    /// <summary>追加 [package] 块。</summary>
    /// <param name="sb">字符串构建器。</param>
    private void AppendPackage(StringBuilder sb)
    {
        sb.AppendLine("[package]");
        sb.AppendLine($"name = \"{_trace.Package.Name}\"");
        sb.AppendLine($"lang = \"{_trace.Package.Lang}\"");
        sb.AppendLine($"version = \"{_trace.Package.Version}\"");
    }

    /// <summary>追加 [remarks] 块。</summary>
    /// <param name="sb">字符串构建器。</param>
    private void AppendRemarks(StringBuilder sb)
    {
        sb.AppendLine("[remarks]");
        if (!string.IsNullOrEmpty(_trace.Remarks.Title))
        {
            sb.AppendLine($"title = \"{EscapeString(_trace.Remarks.Title)}\"");
        }
        if (!string.IsNullOrEmpty(_trace.Remarks.Author))
        {
            sb.AppendLine($"author = \"{EscapeString(_trace.Remarks.Author)}\"");
        }
        if (!string.IsNullOrEmpty(_trace.Remarks.Comment))
        {
            sb.AppendLine($"comment = \"{EscapeString(_trace.Remarks.Comment)}\"");
        }
    }

    /// <summary>追加 [object] 块。</summary>
    /// <param name="sb">字符串构建器。</param>
    private void AppendObject(StringBuilder sb)
    {
        sb.AppendLine("[object]");
        sb.AppendLine($"kind = \"{_trace.ObjectInfo.Kind}\"");
        if (!string.IsNullOrEmpty(_trace.ObjectInfo.Label))
        {
            sb.AppendLine($"label = \"{EscapeString(_trace.ObjectInfo.Label)}\"");
        }
    }

    /// <summary>追加单个 [[states]] 块。</summary>
    /// <param name="sb">字符串构建器。</param>
    /// <param name="state">状态快照数据。</param>
    private void AppendState(StringBuilder sb, State state)
    {
        sb.AppendLine("[[states]]");
        sb.AppendLine($"id = {state.Id}");
        sb.AppendLine();
        sb.AppendLine("[states.data]");

        foreach (var (key, value) in state.Data)
        {
            sb.AppendLine($"{key} = {SerializeValue(value, key)}");
        }
    }

    /// <summary>追加单个 [[steps]] 块。</summary>
    /// <param name="sb">字符串构建器。</param>
    /// <param name="step">操作步骤数据。</param>
    private void AppendStep(StringBuilder sb, Step step)
    {
        sb.AppendLine("[[steps]]");
        sb.AppendLine($"id = {step.Id}");
        sb.AppendLine($"op = \"{step.Op}\"");
        sb.AppendLine($"before = {step.Before}");

        if (step.After.HasValue)
        {
            sb.AppendLine($"after = {step.After.Value}");
        }

        if (step.Ret is not null)
        {
            sb.AppendLine($"ret = {SerializeValue(step.Ret)}");
        }

        if (!string.IsNullOrEmpty(step.Note))
        {
            sb.AppendLine($"note = \"{EscapeString(step.Note)}\"");
        }

        sb.AppendLine();
        sb.AppendLine("[steps.args]");

        foreach (var (key, value) in step.Args)
        {
            sb.AppendLine($"{key} = {SerializeValue(value)}");
        }

        if (step.Code is not null)
        {
            sb.AppendLine();
            sb.AppendLine("[steps.code]");
            sb.AppendLine($"line = {step.Code.Line}");
            if (step.Code.Col.HasValue)
            {
                sb.AppendLine($"col = {step.Code.Col.Value}");
            }
        }
    }

    /// <summary>追加 [result] 块。</summary>
    /// <param name="sb">字符串构建器。</param>
    private void AppendResult(StringBuilder sb)
    {
        var result = _trace.Result!;
        sb.AppendLine("[result]");
        sb.AppendLine($"final_state = {result.FinalState}");

        if (result.CommitInfo is not null)
        {
            sb.AppendLine();
            sb.AppendLine("[result.commit]");
            sb.AppendLine($"op = \"{result.CommitInfo.Op}\"");
            sb.AppendLine($"line = {result.CommitInfo.Line}");
        }
    }

    /// <summary>追加 [error] 块。</summary>
    /// <param name="sb">字符串构建器。</param>
    private void AppendError(StringBuilder sb)
    {
        var error = _trace.Error!;
        sb.AppendLine("[error]");
        sb.AppendLine($"type = \"{error.Type}\"");
        sb.AppendLine($"message = \"{EscapeString(error.Message)}\"");

        if (error.Line.HasValue)
        {
            sb.AppendLine($"line = {error.Line.Value}");
        }
        if (error.StepId.HasValue)
        {
            sb.AppendLine($"step = {error.StepId.Value}");
        }
        if (error.LastState.HasValue)
        {
            sb.AppendLine($"last_state = {error.LastState.Value}");
        }
    }

    /// <summary>序列化单个值为 TOML 格式。</summary>
    /// <param name="value">要序列化的值。</param>
    /// <param name="key">键名，用于判断是否为特殊数组字段。</param>
    /// <returns>TOML 格式的值字符串。</returns>
    private string SerializeValue(object value, string? key = null)
    {
        return value switch
        {
            bool b => b ? "true" : "false",
            int i => i.ToString(),
            long l => l.ToString(),
            double d => SerializeDouble(d),
            float f => SerializeDouble(f),
            string s => $"\"{EscapeString(s)}\"",
            IList<object> list when key is "nodes" or "edges" => SerializeInlineTableArray(list),
            IList<object> list => SerializeSimpleArray(list),
            IDictionary<string, object> dict => SerializeInlineTable(dict),
            _ => $"\"{EscapeString(value.ToString() ?? "")}\"",
        };
    }

    /// <summary>序列化浮点数，确保输出有效的 TOML 数字格式。</summary>
    /// <param name="d">浮点数值。</param>
    /// <returns>TOML 格式的数字字符串。</returns>
    private static string SerializeDouble(double d)
    {
        var str = d.ToString("G", System.Globalization.CultureInfo.InvariantCulture);
        if (!str.Contains('.') && !str.Contains('E') && !str.Contains('e'))
        {
            str += ".0";
        }
        return str;
    }

    /// <summary>序列化内联表数组（用于 nodes 和 edges）。</summary>
    /// <param name="array">字典数组。</param>
    /// <returns>TOML 格式的内联表数组字符串。</returns>
    private string SerializeInlineTableArray(IList<object> array)
    {
        if (array.Count == 0)
        {
            return "[]";
        }

        var items = array
            .Cast<IDictionary<string, object>>()
            .Select(SerializeInlineTable);

        return "[\n  " + string.Join(",\n  ", items) + "\n]";
    }

    /// <summary>序列化简单数组。</summary>
    /// <param name="array">值数组。</param>
    /// <returns>TOML 格式的数组字符串。</returns>
    private string SerializeSimpleArray(IList<object> array)
    {
        if (array.Count == 0)
        {
            return "[]";
        }

        var items = array.Select(v => SerializeValue(v));
        return "[" + string.Join(", ", items) + "]";
    }

    /// <summary>序列化内联表。</summary>
    /// <param name="dict">键值对字典。</param>
    /// <returns>TOML 格式的内联表字符串。</returns>
    private string SerializeInlineTable(IDictionary<string, object> dict)
    {
        var pairs = dict.Select(kv => $"{kv.Key} = {SerializeValue(kv.Value)}");
        return "{ " + string.Join(", ", pairs) + " }";
    }

    /// <summary>转义字符串中的特殊字符。</summary>
    /// <param name="s">原始字符串。</param>
    /// <returns>转义后的字符串。</returns>
    private static string EscapeString(string s)
    {
        return s
            .Replace("\\", "\\\\")
            .Replace("\"", "\\\"")
            .Replace("\n", "\\n")
            .Replace("\r", "\\r");
    }
}