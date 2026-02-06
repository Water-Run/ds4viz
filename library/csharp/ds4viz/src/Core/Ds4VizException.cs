/*
 * ds4viz 异常类型定义
 * 提供库专用的异常类型，携带错误类型与行号信息以支持 IR 错误块生成
 * 
 * @author: WaterRun
 * @file: src/Core/Ds4VizException.cs
 * @date: 2026-02-06
 */

#nullable enable

namespace Ds4Viz.Core;

/// <summary>ds4viz 库的基础异常类，携带错误分类与源码位置信息。</summary>
/// <remarks>
/// 不变量：Message 不为 null；ErrorType 为有效枚举值。
/// 线程安全：异常对象创建后不可变，线程安全。
/// 副作用：无。
/// </remarks>
public class Ds4VizException : Exception
{
    /// <summary>错误类型分类。</summary>
    /// <value>表示错误的类别，默认为 <see cref="ErrorType.Runtime"/>，不允许 null。</value>
    public ErrorType ErrorType { get; }

    /// <summary>发生错误的源码行号。</summary>
    /// <value>正整数表示具体行号，null 表示行号未知。</value>
    public int? Line { get; }

    /// <summary>创建带有完整错误信息的异常实例。</summary>
    /// <param name="message">错误消息，不允许 null 或空字符串。</param>
    /// <param name="errorType">错误类型分类，默认为 Runtime。</param>
    /// <param name="line">源码行号，null 表示未知。</param>
    /// <exception cref="ArgumentNullException">当 message 为 null 时抛出。</exception>
    public Ds4VizException(string message, ErrorType errorType = ErrorType.Runtime, int? line = null)
        : base(message)
    {
        ArgumentNullException.ThrowIfNull(message);
        ErrorType = errorType;
        Line = line;
    }

    /// <summary>创建带有内部异常的异常实例。</summary>
    /// <param name="message">错误消息，不允许 null 或空字符串。</param>
    /// <param name="innerException">导致当前异常的内部异常，允许 null。</param>
    /// <param name="errorType">错误类型分类，默认为 Runtime。</param>
    /// <param name="line">源码行号，null 表示未知。</param>
    /// <exception cref="ArgumentNullException">当 message 为 null 时抛出。</exception>
    public Ds4VizException(
        string message,
        Exception? innerException,
        ErrorType errorType = ErrorType.Runtime,
        int? line = null)
        : base(message, innerException)
    {
        ArgumentNullException.ThrowIfNull(message);
        ErrorType = errorType;
        Line = line;
    }
}

/// <summary>数据结构操作错误，表示对数据结构执行非法操作时抛出的异常。</summary>
/// <remarks>
/// 不变量：始终使用 ErrorType.Runtime 作为错误类型。
/// 线程安全：异常对象创建后不可变，线程安全。
/// 副作用：无。
/// </remarks>
public sealed class StructureException : Ds4VizException
{
    /// <summary>创建数据结构操作异常。</summary>
    /// <param name="message">错误消息，描述具体的操作错误，不允许 null。</param>
    /// <param name="line">发生错误的源码行号，null 表示未知。</param>
    /// <exception cref="ArgumentNullException">当 message 为 null 时抛出。</exception>
    public StructureException(string message, int? line = null)
        : base(message, ErrorType.Runtime, line)
    {
    }
}

/// <summary>数据验证错误，表示输入数据不符合约束条件时抛出的异常。</summary>
/// <remarks>
/// 不变量：始终使用 ErrorType.Validation 作为错误类型。
/// 线程安全：异常对象创建后不可变，线程安全。
/// 副作用：无。
/// </remarks>
public sealed class ValidationException : Ds4VizException
{
    /// <summary>创建数据验证异常。</summary>
    /// <param name="message">错误消息，描述具体的验证失败原因，不允许 null。</param>
    /// <param name="line">发生错误的源码行号，null 表示未知。</param>
    /// <exception cref="ArgumentNullException">当 message 为 null 时抛出。</exception>
    public ValidationException(string message, int? line = null)
        : base(message, ErrorType.Validation, line)
    {
    }
}