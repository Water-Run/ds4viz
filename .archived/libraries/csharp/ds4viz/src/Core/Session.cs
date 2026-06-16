/*
 * 会话管理模块
 * 提供数据结构操作追踪的上下文管理与状态记录功能
 * 
 * @author: WaterRun
 * @file: src/Core/Session.cs
 * @date: 2026-02-06
 */

#nullable enable

using System.Runtime.InteropServices;

namespace Ds4Viz.Core;

/// <summary>会话管理器，负责管理数据结构操作的追踪记录生命周期。</summary>
/// <remarks>
/// 不变量：_states 和 _steps 中的 Id 从 0 开始连续递增。
/// 线程安全：不保证线程安全，同一实例不应被多线程并发访问。
/// 副作用：Write 方法会创建或覆盖文件。
/// </remarks>
internal sealed class Session
{
    /// <summary>数据结构类型标识。</summary>
    private readonly string _kind;

    /// <summary>数据结构实例标签。</summary>
    private readonly string _label;

    /// <summary>TOML 输出文件路径。</summary>
    private readonly string _outputPath;

    /// <summary>状态快照列表。</summary>
    private readonly List<State> _states = [];

    /// <summary>操作步骤列表。</summary>
    private readonly List<Step> _steps = [];

    /// <summary>状态 Id 计数器。</summary>
    private int _stateCounter;

    /// <summary>步骤 Id 计数器。</summary>
    private int _stepCounter;

    /// <summary>错误信息，若发生错误则不为 null。</summary>
    private Error? _error;

    /// <summary>上下文入口行号。</summary>
    private int _entryLine = 1;

    /// <summary>上下文退出行号。</summary>
    private int _exitLine = 1;

    /// <summary>失败步骤的 Id，若未失败则为 null。</summary>
    private int? _failedStepId;

    /// <summary>获取当前步骤计数器值。</summary>
    /// <value>下一个步骤将使用的 Id，非负整数。</value>
    public int StepCounter => _stepCounter;

    /// <summary>获取失败步骤 Id。</summary>
    /// <value>失败步骤的 Id，若未失败则为 null。</value>
    public int? FailedStepId => _failedStepId;

    /// <summary>创建会话实例。</summary>
    /// <param name="kind">数据结构类型标识，不允许 null。</param>
    /// <param name="label">数据结构实例标签，不允许 null。</param>
    /// <param name="outputPath">TOML 输出路径，若为 null 则使用全局配置。</param>
    /// <exception cref="ArgumentNullException">当 kind 或 label 为 null 时抛出。</exception>
    public Session(string kind, string label, string? outputPath = null)
    {
        ArgumentNullException.ThrowIfNull(kind);
        ArgumentNullException.ThrowIfNull(label);

        _kind = kind;
        _label = label;
        _outputPath = outputPath ?? GlobalConfigHolder.Instance.OutputPath;
    }

    /// <summary>添加状态快照。</summary>
    /// <param name="data">状态数据字典，不允许 null。</param>
    /// <returns>新状态的 Id。</returns>
    /// <exception cref="ArgumentNullException">当 data 为 null 时抛出。</exception>
    public int AddState(Dictionary<string, object> data)
    {
        ArgumentNullException.ThrowIfNull(data);

        var stateId = _stateCounter++;
        _states.Add(new State(stateId, data));
        return stateId;
    }

    /// <summary>添加操作步骤。</summary>
    /// <param name="op">操作名称，不允许 null。</param>
    /// <param name="before">操作前状态 Id，必须引用有效状态。</param>
    /// <param name="after">操作后状态 Id，失败时为 null。</param>
    /// <param name="args">操作参数字典，不允许 null。</param>
    /// <param name="ret">操作返回值，可选。</param>
    /// <param name="note">操作备注，可选。</param>
    /// <param name="line">源码行号，可选。</param>
    /// <returns>新步骤的 Id。</returns>
    /// <exception cref="ArgumentNullException">当 op 或 args 为 null 时抛出。</exception>
    public int AddStep(
        string op,
        int before,
        int? after,
        Dictionary<string, object> args,
        object? ret = null,
        string? note = null,
        int? line = null)
    {
        ArgumentNullException.ThrowIfNull(op);
        ArgumentNullException.ThrowIfNull(args);

        var stepId = _stepCounter++;
        var code = line.HasValue ? new CodeLocation(line.Value) : null;
        _steps.Add(new Step(stepId, op, before, after, args, code, ret, note));
        return stepId;
    }

    /// <summary>设置错误信息。</summary>
    /// <param name="errorType">错误类型。</param>
    /// <param name="message">错误消息，不允许 null，超过 512 字符将被截断。</param>
    /// <param name="line">错误发生的源码行号，可选。</param>
    /// <param name="stepId">发生错误的步骤 Id，可选。</param>
    /// <exception cref="ArgumentNullException">当 message 为 null 时抛出。</exception>
    public void SetError(ErrorType errorType, string message, int? line = null, int? stepId = null)
    {
        ArgumentNullException.ThrowIfNull(message);

        var truncatedMessage = message.Length > 512 ? message[..512] : message;
        var lastState = _states.Count > 0 ? _states[^1].Id : (int?)null;

        _error = new Error(
            errorType.ToIrString(),
            truncatedMessage,
            line,
            stepId,
            lastState);

        _failedStepId = stepId;
    }

    /// <summary>设置失败步骤 Id。</summary>
    /// <param name="stepId">失败步骤的 Id。</param>
    public void SetFailedStepId(int stepId)
    {
        _failedStepId = stepId;
    }

    /// <summary>设置上下文入口行号。</summary>
    /// <param name="line">入口行号，必须为正整数。</param>
    public void SetEntryLine(int line)
    {
        _entryLine = line;
    }

    /// <summary>设置上下文退出行号。</summary>
    /// <param name="line">退出行号，必须为正整数。</param>
    public void SetExitLine(int line)
    {
        _exitLine = line;
    }

    /// <summary>获取最后一个状态的 Id。</summary>
    /// <returns>最后一个状态的 Id，若无状态则返回 -1。</returns>
    public int GetLastStateId()
    {
        return _states.Count > 0 ? _states[^1].Id : -1;
    }

    /// <summary>构建完整的 Trace 对象。</summary>
    /// <returns>包含所有追踪数据的 Trace 实例。</returns>
    public Trace BuildTrace()
    {
        var now = DateTime.UtcNow.ToString("yyyy-MM-ddTHH:mm:ssZ");
        var langVersion = RuntimeInformation.FrameworkDescription;

        var config = GlobalConfigHolder.Instance;

        var trace = new Trace
        {
            Meta = new Meta(now, "csharp", langVersion),
            Package = new Package(),
            Remarks = new Remarks(config.Title, config.Author, config.Comment),
            ObjectInfo = new ObjectInfo(_kind, _label),
            States = _states,
            Steps = _steps
        };

        if (_error is not null)
        {
            trace.Error = _error;
        }
        else
        {
            var finalStateId = _states.Count > 0 ? _states[^1].Id : 0;
            trace.Result = new Result(finalStateId, new Commit("commit", _exitLine));
        }

        return trace;
    }

    /// <summary>将追踪数据写入 TOML 文件。</summary>
    public void Write()
    {
        var trace = BuildTrace();
        var writer = new TomlWriter(trace);
        writer.Write(_outputPath);
    }
}