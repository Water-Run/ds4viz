/*
 * Trace 数据模型定义
 * 定义 IR 规范中的所有数据结构，用于构建完整的操作追踪记录
 * 
 * @author: WaterRun
 * @file: src/Core/Trace.cs
 * @date: 2026-02-06
 */

#nullable enable

namespace Ds4Viz.Core;

/// <summary>元数据信息，记录追踪文件的生成上下文。</summary>
/// <remarks>
/// 不变量：所有字符串字段不为 null。
/// 线程安全：record 类型不可变，线程安全。
/// 副作用：无。
/// </remarks>
/// <param name="CreatedAt">创建时间，RFC3339 格式 UTC 时间戳。</param>
/// <param name="Lang">生成追踪的编程语言标识，固定为 "csharp"。</param>
/// <param name="LangVersion">语言版本字符串。</param>
internal sealed record Meta(string CreatedAt, string Lang, string LangVersion);

/// <summary>包信息，描述生成追踪的 ds4viz 库版本。</summary>
/// <remarks>
/// 不变量：所有字符串字段不为 null。
/// 线程安全：record 类型不可变，线程安全。
/// 副作用：无。
/// </remarks>
/// <param name="Name">包名称，固定为 "ds4viz"。</param>
/// <param name="Lang">包所属语言，固定为 "csharp"。</param>
/// <param name="Version">包版本号。</param>
internal sealed record Package(string Name = "ds4viz", string Lang = "csharp", string Version = "0.1.0");

/// <summary>备注信息，提供可视化展示用的描述性元数据。</summary>
/// <remarks>
/// 不变量：所有字符串字段不为 null。
/// 线程安全：record 类型不可变，线程安全。
/// 副作用：无。
/// </remarks>
/// <param name="Title">可视化标题。</param>
/// <param name="Author">作者信息。</param>
/// <param name="Comment">注释说明。</param>
internal sealed record Remarks(string Title = "", string Author = "", string Comment = "")
{
    /// <summary>检查备注是否全部为空。</summary>
    /// <returns>当所有字段都为空字符串时返回 true。</returns>
    public bool IsEmpty() => string.IsNullOrEmpty(Title) && string.IsNullOrEmpty(Author) && string.IsNullOrEmpty(Comment);
}

/// <summary>数据结构对象描述，标识追踪记录对应的数据结构类型。</summary>
/// <remarks>
/// 不变量：Kind 必须是 IR 规范定义的有效类型标识。
/// 线程安全：record 类型不可变，线程安全。
/// 副作用：无。
/// </remarks>
/// <param name="Kind">数据结构类型标识。</param>
/// <param name="Label">数据结构实例标签。</param>
internal sealed record ObjectInfo(string Kind, string Label = "");

/// <summary>状态快照，记录数据结构在某一时刻的完整状态。</summary>
/// <remarks>
/// 不变量：Id 非负且在文档内唯一；Data 不为 null。
/// 线程安全：Data 字典内容可能被修改，不保证线程安全。
/// 副作用：无。
/// </remarks>
/// <param name="Id">状态唯一标识，从 0 开始连续递增。</param>
/// <param name="Data">状态数据字典，结构由数据结构类型决定。</param>
internal sealed record State(int Id, Dictionary<string, object> Data);

/// <summary>代码位置信息，记录操作对应的源码位置。</summary>
/// <remarks>
/// 不变量：Line 为正整数；Col 若存在也为正整数。
/// 线程安全：record 类型不可变，线程安全。
/// 副作用：无。
/// </remarks>
/// <param name="Line">代码行号，从 1 开始。</param>
/// <param name="Col">代码列号，从 1 开始，可选。</param>
internal sealed record CodeLocation(int Line, int? Col = null);

/// <summary>操作步骤，记录对数据结构的单次操作及其影响。</summary>
/// <remarks>
/// 不变量：Id 非负且在文档内唯一；Before 引用有效的 State.Id。
/// 线程安全：Args 字典内容可能被修改，不保证线程安全。
/// 副作用：无。
/// </remarks>
/// <param name="Id">步骤唯一标识，从 0 开始连续递增。</param>
/// <param name="Op">操作名称标识。</param>
/// <param name="Before">操作前的状态 Id。</param>
/// <param name="After">操作后的状态 Id，失败时为 null。</param>
/// <param name="Args">操作参数字典。</param>
/// <param name="Code">源码位置信息，可选。</param>
/// <param name="Ret">操作返回值，可选。</param>
/// <param name="Note">操作备注，可选。</param>
internal sealed record Step(
    int Id,
    string Op,
    int Before,
    int? After,
    Dictionary<string, object> Args,
    CodeLocation? Code = null,
    object? Ret = null,
    string? Note = null);

/// <summary>提交信息，记录成功完成时的最终操作位置。</summary>
/// <remarks>
/// 不变量：Op 不为 null；Line 为正整数。
/// 线程安全：record 类型不可变，线程安全。
/// 副作用：无。
/// </remarks>
/// <param name="Op">操作标识。</param>
/// <param name="Line">提交操作的源码行号。</param>
internal sealed record Commit(string Op, int Line);

/// <summary>成功结果，表示追踪正常完成。</summary>
/// <remarks>
/// 不变量：FinalState 引用有效的 State.Id。
/// 线程安全：record 类型不可变，线程安全。
/// 副作用：无。
/// </remarks>
/// <param name="FinalState">最终状态的 Id。</param>
/// <param name="CommitInfo">提交信息，可选。</param>
internal sealed record Result(int FinalState, Commit? CommitInfo = null);

/// <summary>错误信息，记录追踪过程中发生的错误详情。</summary>
/// <remarks>
/// 不变量：Type 和 Message 不为 null。
/// 线程安全：record 类型不可变，线程安全。
/// 副作用：无。
/// </remarks>
/// <param name="Type">错误类型标识字符串。</param>
/// <param name="Message">错误消息。</param>
/// <param name="Line">错误发生的源码行号，可选。</param>
/// <param name="StepId">发生错误的步骤 Id，可选。</param>
/// <param name="LastState">错误前最后一个有效状态的 Id，可选。</param>
internal sealed record Error(
    string Type,
    string Message,
    int? Line = null,
    int? StepId = null,
    int? LastState = null);

/// <summary>完整的操作追踪记录，包含所有 IR 规范要求的数据块。</summary>
/// <remarks>
/// 不变量：Meta、Package、ObjectInfo 不为 null；Result 和 Error 互斥存在。
/// 线程安全：States 和 Steps 列表可能被修改，不保证线程安全。
/// 副作用：无。
/// </remarks>
internal sealed class Trace
{
    /// <summary>元数据信息。</summary>
    public required Meta Meta { get; init; }

    /// <summary>包信息。</summary>
    public required Package Package { get; init; }

    /// <summary>备注信息。</summary>
    public required Remarks Remarks { get; init; }

    /// <summary>数据结构对象描述。</summary>
    public required ObjectInfo ObjectInfo { get; init; }

    /// <summary>状态快照列表。</summary>
    public required List<State> States { get; init; }

    /// <summary>操作步骤列表。</summary>
    public required List<Step> Steps { get; init; }

    /// <summary>成功结果，与 Error 互斥。</summary>
    public Result? Result { get; set; }

    /// <summary>错误信息，与 Result 互斥。</summary>
    public Error? Error { get; set; }
}