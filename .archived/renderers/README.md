# 归档渲染器

已从 `render/` 移出的渲染器, 包含未实现 / 旧版 demo / 已弃用的可视化前端.

当前活跃渲染器见 [`render/`](../../render/).

## 目录

| 目录 | 渲染器 | 归档原因 | 备注 |
|------|--------|----------|------|
| [`simp-web/`](./simp-web/) | 简易 Web (HTML/JS) | 旧版 demo | 原为 `server/demo` 配套前端, 已被 `render/vue` 取代 |
| [`tui/`](./tui/) | 终端 TUI (Rust) | 未实现 | 仅有入口占位 |
| [`winui3/`](./winui3/) | Windows 桌面 (WinUI 3) | 未实现 | 仅有项目模板, 内嵌 `MiniScript` 计划 |
| [`dart-flutter/`](./dart-flutter/) | Flutter (移动端) | 已弃用 | 目录为空, 不再维护 |

## 恢复

如需恢复某个渲染器: 将对应目录 `git mv` 回 `render/` 根目录, 同步更新 [`README.md`](../../README.md) 的"渲染器"表, 并在 [`AGENTS.md`](../../AGENTS.md) 中补充对应的工具链说明.
