/*
 * 数据结构抽象基类
 * 提供所有数据结构共享的上下文管理与追踪记录功能
 * 
 * @author: WaterRun
 * @file: src/Structures/BaseStructure.cs
 * @date: 2026-02-06
 */

#nullable enable

using Ds4Viz.Core;

namespace Ds4Viz.Structures;

/// <summary>数据结构抽象基类，提供 IDisposable 实现与追踪记录基础设施。</summary>
/// <remarks>
/// 不变量：_session 在实例生命周期内不变；Dispose 后不应再调用任何操作方法。
/// 线程安全：不保证线程安全，同一实例不应被多线程并发访问。
/// 副作用：Dispose 时会写入 TOML 文件。
/// </remarks>
public abstract class BaseStructure : IDisposable
{
    /// <summary>会话管理器实例。</summary>
    private protected readonly Session _session;

    /// <summary>标识是否已进入上下文。</summary>
    private bool _entered;

    /// <summary>标识是否已释放资源。</summary>
    private bool _disposed;

    /// <summary>当前异常信息，用于 Dispose 时生成错误块。</summary>
    private Exception? _currentException;

    /// <summary>初始化数据结构基类。</summary>
    /// <param name="kind">数据结构类型标识，不允许 null。</param>
    /// <param name="label">数据结构实例标签，不允许 null。</param>
    /// <param name="output">TOML 输出路径，null 表示使用全局配置。</param>
    /// <exception cref="ArgumentNullException">当 kind 或 label 为 null 时抛出。</exception>
    protected BaseStructure(string kind, string label, string? output)
    {
        ArgumentNullException.ThrowIfNull(kind);
        ArgumentNullException.ThrowIfNull(label);

        _session = new Session(kind, label, output);
    }

    /// <summary>获取当前步骤计数器值。</summary>
    /// <value>下一个步骤将使用的 Id。</value>
    protected int StepCounter => _session.StepCounter;

    /// <summary>构建当前状态的数据字典，由具体数据结构实现。</summary>
    /// <returns>表示当前状态的数据字典。</returns>
    protected abstract Dictionary<string, object> BuildData();

    /// <summary>初始化数据结构，添加初始状态。</summary>
    protected virtual void Initialize()
    {
        _session.AddState(BuildData());
    }

    /// <summary>进入数据结构使用上下文。</summary>
    /// <param name="callerLine">调用者的源码行号，由编译器自动提供。</param>
    /// <returns>当前实例，支持 using 语句。</returns>
    public BaseStructure Enter([CallerLineNumber] int callerLine = 0)
    {
        if (_entered)
        {
            return this;
        }

        _entered = true;
        _session.SetEntryLine(callerLine);
        Initialize();
        return this;
    }

    /// <summary>记录操作过程中发生的异常。</summary>
    /// <param name="ex">发生的异常。</param>
    internal void RecordException(Exception ex)
    {
        _currentException = ex;
    }

    /// <summary>抛出数据结构操作错误并记录到会话。</summary>
    /// <param name="message">错误消息。</param>
    /// <param name="callerLine">调用者行号。</param>
    /// <exception cref="StructureException">始终抛出。</exception>
    protected void RaiseError(string message, [CallerLineNumber] int callerLine = 0)
    {
        var stepId = _session.StepCounter;
        _session.SetFailedStepId(stepId);
        var ex = new StructureException(message, callerLine);
        _currentException = ex;
        throw ex;
    }

    /// <summary>释放资源并写入追踪文件。</summary>
    /// <param name="callerLine">调用者的源码行号。</param>
    public void Dispose([CallerLineNumber] int callerLine = 0)
    {
        if (_disposed)
        {
            return;
        }

        _disposed = true;
        _session.SetExitLine(callerLine);

        if (_currentException is not null)
        {
            int? errorLine = null;
            var errorType = ErrorType.Unknown;
            var message = _currentException.Message;

            if (_currentException is Ds4VizException ds4VizEx)
            {
                errorType = ds4VizEx.ErrorType;
                errorLine = ds4VizEx.Line;
            }

            _session.SetError(
                errorType,
                message,
                errorLine,
                _session.FailedStepId);
        }

        _session.Write();
        GC.SuppressFinalize(this);
    }

    /// <summary>显式实现 IDisposable.Dispose。</summary>
    void IDisposable.Dispose()
    {
        Dispose();
    }
}