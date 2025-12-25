# `ds4viz`

`WaterRun` `2025-12-24`

毕业论文项目. 一个可扩展的数据结构可视化教学平台.  

`ds4viz` -> `datastructure for visualizaion` -> `数据结构可视化`.  

## 核心架构

采用"代码 → 中间语言 → 渲染"三层解耦架构:

```plaintext
[编写代码]              [执行生成]                [可视化渲染]
调用ds4viz库  ->  生成统一.toml描述文件  ->  渲染器解析并交互展示
```

**关键特性**

* **语言无关**: 通过`.toml`作为统一中间表示(IR), 支持任意语言扩展
* **渲染无关**: 同一份`.toml`可在Web/CLI/桌面/移动端多平台渲染
* **教学友好**: 参考[algorithm-visualizer](https://github.com/algorithm-visualizer/algorithm-visualizer)设计, 代码即文档, 统一且易上手

## `.toml` IR生成库

按语言实现ds4viz库, 运行时生成标准化的`.toml`中间文件.

### 库设计原则

* 代码即文档: API简洁一致, 无需额外学习成本
* 上下文管理: 自动捕获执行流程, 确保成功/失败均生成有效`.toml`
* 语义统一: 不同语言库生成的`.toml`结构完全一致

### 各语言支持状态

| 语言 | 安装 | 文档                                          | 状态   |
|---|---|---------------------------------------------|------|
| `Python` | `pip install ds4viz` | [python-ds4viz](./library/python/README.md) | 已编写  |
| `C` | 前往[Release](https://github.com/Water-Run/ds4viz/releases/tag/lib-0.1.0)下载`c-ds4viz`,解压获取`ds4viz.h` | [c-ds4viz](./library/c/README.md)                                           | 已编写 |
| `C#` | - | -                                           | 规划中 |
| `Java` | - | -                                           | 规划中 |
| `JavaScript` | - | -                                           | 规划中 |
| `TypeScript` | - | -                                           | 规划中 |
| `Lua` | `luarocks install ds4viz` | [lua-ds4viz](./library/lua/README.md)                                           | 已编写 |
| `PHP` | `composer require ds4viz/ds4viz` | [php-ds4viz](./library/php/README.md)                                          | 已编写 |
| `Nim` | - | -                                           | 规划中 |
| `Rust` | `cargo add ds4viz` | [rust-ds4viz](./library/rust/README.md)                                           | 已编写 |
| `Zig` | - | -                                           | 规划中 |

## 渲染器

解析`.toml`IR并生成交互式可视化界面, 支持多平台部署.

| 渲染器 | 平台 | 下载 | 文档 | 状态 |
|---|---|---|---|---|
| `simp-web` | Web (本地单HTML, 对应Demo服务器) | - | - | 开发中 |
| `vue` | Web (SPA) | - | - | 开发中 |
| `cli` | 命令行 | - | - | 规划中 |
| `winui3` | Windows桌面 | - | - | 规划中 |
| `flutter` | Android | - | - | 规划中 |

## 在线服务(Vue Web)

提供在线代码编辑 + 远程执行 + 实时可视化的一站式体验.

**功能特性**

* 左侧编辑器: 支持多语言高亮/补全, 提供预设模板
* 右侧可视化: 实时渲染`.toml`, 支持步进/回退/跳转
* 远程执行: 后端沙箱运行代码, 带超时/资源限制保护
* 直接上传: 也可本地生成`.toml`后直接上传渲染

**部署架构**

```plaintext
前端(Vue3+TS) <-> 后端(FastAPI+PostgreSQL)
    |                    |
  编辑器              沙箱执行引擎
  渲染器              TOML验证器
```

通过`systemd-run`瞬态单元做到每次请求对应一次性临时沙箱安全运行.  
完整版本提供:  

- 可注册, 登陆, 历史  
- 基础缓存机制, 相同的代码直接返回, 减少资源消耗  
- 用户每日CPU时间资源限制  

> Demo版本先采用简化服务端的实现, 仅包含运行代码的功能: 使用`lua`+`pegasus`+`sqlite`

## 文档参考

| 文档                       |
|--------------------------|
| [IR定义](./prompt/IR定义.md) |
