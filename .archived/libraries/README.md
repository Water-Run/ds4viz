# 归档语言库

已从 `library/` 移出的 `.toml` IR 生成库, 包含未实现 / 旧版 / 暂停支持的语言实现.

当前活跃库见 [`library/`](../../library/).

## 目录

| 目录 | 语言 | 归档原因 | 备注 |
|------|------|----------|------|
| [`csharp/`](./csharp/) | C# (`Ds4Viz`) | 未测试 | 存在完整实现, 但未与 IR 规范同步测试 |
| [`fsharp/`](./fsharp/) | F# (`Ds4Viz`) | 占位 | 仅含 F# 项目模板, 无实质实现 |
| [`java/`](./java/) | Java | 占位 | 仅含 `todo` 标记 |
| [`javascript/`](./javascript/) | JavaScript | 旧版 | 被 `library/typescript` 替代 |
| [`lua/`](./lua/) | Lua (`ds4viz`) | 旧版 | 仅供 `server/demo` 兼容使用 |
| [`nim/`](./nim/) | Nim | 未实现 | 仅含基础结构 |
| [`php/`](./php/) | PHP (`ds4viz`) | 旧版 | 暂停支持 |
| [`rust/`](./rust/) | Rust (`ds4viz`) | 旧版 | 暂停支持 |
| [`typescript/`](./typescript/) | TypeScript (`ds4viz`) | 旧版 | 已不再与 IR 规范同步 |
| [`zig/`](./zig/) | Zig | 占位 | 仅含 `todo` 标记 |

## 恢复

如需恢复某个库: 将对应目录 `git mv` 回 `library/` 根目录, 同步更新 [`README.md`](../../README.md) 的"支持状态"表, 并在 [`AGENTS.md`](../../AGENTS.md) 中补充对应的工具链说明.
