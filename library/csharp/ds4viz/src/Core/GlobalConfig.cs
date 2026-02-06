/*
 * 全局配置管理
 * 提供库级别的默认配置存储与访问，支持用户自定义输出路径与元数据
 * 
 * @author: WaterRun
 * @file: src/Core/GlobalConfig.cs
 * @date: 2026-02-06
 */

#nullable enable

namespace Ds4Viz.Core;

/// <summary>全局配置数据容器，存储 TOML 输出相关的默认设置。</summary>
/// <remarks>
/// 不变量：OutputPath 始终非 null 且非空。
/// 线程安全：不保证线程安全，建议在程序启动阶段配置后不再修改。
/// 副作用：无。
/// </remarks>
internal sealed class GlobalConfig
{
    /// <summary>TOML 文件的默认输出路径。</summary>
    private string _outputPath = "trace.toml";

    /// <summary>可视化标题。</summary>
    private string _title = "";

    /// <summary>作者信息。</summary>
    private string _author = "";

    /// <summary>注释说明。</summary>
    private string _comment = "";

    /// <summary>获取或设置 TOML 输出文件路径。</summary>
    /// <value>文件路径字符串，默认为 "trace.toml"，不允许 null 或空字符串。</value>
    /// <exception cref="ArgumentNullException">当设置值为 null 时抛出。</exception>
    /// <exception cref="ArgumentException">当设置值为空字符串时抛出。</exception>
    public string OutputPath
    {
        get => _outputPath;
        set
        {
            ArgumentNullException.ThrowIfNull(value);
            if (string.IsNullOrWhiteSpace(value))
            {
                throw new ArgumentException("输出路径不能为空", nameof(value));
            }
            _outputPath = value;
        }
    }

    /// <summary>获取或设置可视化标题。</summary>
    /// <value>标题字符串，默认为空字符串，不允许 null。</value>
    /// <exception cref="ArgumentNullException">当设置值为 null 时抛出。</exception>
    public string Title
    {
        get => _title;
        set
        {
            ArgumentNullException.ThrowIfNull(value);
            _title = value;
        }
    }

    /// <summary>获取或设置作者信息。</summary>
    /// <value>作者字符串，默认为空字符串，不允许 null。</value>
    /// <exception cref="ArgumentNullException">当设置值为 null 时抛出。</exception>
    public string Author
    {
        get => _author;
        set
        {
            ArgumentNullException.ThrowIfNull(value);
            _author = value;
        }
    }

    /// <summary>获取或设置注释说明。</summary>
    /// <value>注释字符串，默认为空字符串，不允许 null。</value>
    /// <exception cref="ArgumentNullException">当设置值为 null 时抛出。</exception>
    public string Comment
    {
        get => _comment;
        set
        {
            ArgumentNullException.ThrowIfNull(value);
            _comment = value;
        }
    }
}

/// <summary>全局配置单例访问器，提供配置的统一访问入口。</summary>
/// <remarks>
/// 不变量：Instance 始终指向同一个 GlobalConfig 实例。
/// 线程安全：实例引用不变，但配置内容修改不保证线程安全。
/// 副作用：无。
/// </remarks>
internal static class GlobalConfigHolder
{
    /// <summary>全局配置单例实例。</summary>
    private static readonly GlobalConfig _instance = new();

    /// <summary>获取全局配置实例。</summary>
    /// <value>全局共享的配置对象，不为 null。</value>
    public static GlobalConfig Instance => _instance;
}