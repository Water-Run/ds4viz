/*
 * 错误类型枚举
 * 定义 ds4viz IR 规范中支持的错误类型，用于标识追踪过程中发生的异常类别
 * 
 * @author: WaterRun
 * @file: src/Core/ErrorType.cs
 * @date: 2026-02-06
 */

#nullable enable

namespace Ds4Viz.Core;

/// <summary>错误类型枚举，对应 IR 定义中的 error.type 字段。</summary>
/// <remarks>
/// 不变量：枚举值与 IR 规范中定义的字符串标识一一对应。
/// 线程安全：枚举类型本身是不可变的，天然线程安全。
/// 副作用：无。
/// </remarks>
public enum ErrorType
{
    /// <summary>运行时错误，表示数据结构操作过程中发生的逻辑错误。</summary>
    Runtime,

    /// <summary>超时错误，表示操作执行时间超过限制。</summary>
    Timeout,

    /// <summary>验证错误，表示数据不符合 IR 规范约束。</summary>
    Validation,

    /// <summary>沙箱错误，表示代码执行环境限制触发的错误。</summary>
    Sandbox,

    /// <summary>未知错误，表示无法归类的其他错误。</summary>
    Unknown
}

/// <summary>提供 <see cref="ErrorType"/> 枚举的扩展方法。</summary>
/// <remarks>
/// 不变量：转换结果与 IR 规范保持一致。
/// 线程安全：所有方法均为纯函数，线程安全。
/// 副作用：无。
/// </remarks>
internal static class ErrorTypeExtensions
{
    /// <summary>将错误类型转换为 IR 规范定义的字符串表示。</summary>
    /// <param name="errorType">要转换的错误类型，不允许未定义的枚举值。</param>
    /// <returns>对应的小写字符串标识，如 "runtime"、"timeout" 等。</returns>
    internal static string ToIrString(this ErrorType errorType)
    {
        return errorType switch
        {
            ErrorType.Runtime => "runtime",
            ErrorType.Timeout => "timeout",
            ErrorType.Validation => "validation",
            ErrorType.Sandbox => "sandbox",
            ErrorType.Unknown => "unknown",
            _ => "unknown"
        };
    }
}