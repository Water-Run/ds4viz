# 归档编译器

已从 `compiler/` 移出的"非 `.toml` 直出"编译器, 全部为未实现状态.

> 编译器与渲染器不同, 直接生成可视化产物(文本/音视频/PPT), 不经过 `.toml` IR.
> 目前暂无活跃的编译器实现, 根目录的 `compiler/` 已被移除.

## 目录

| 目录 | 编译器 | 输出格式 | 技术栈 | 状态 |
|------|--------|----------|--------|------|
| [`text/`](./text/) | `text` | TXT / HTML | Nim | 仅含基础骨架 |
| [`media/`](./media/) | `media` | 音视频 | Python | 空目录 |
| [`powerpoint/`](./powerpoint/) | `powerpoint` | PowerPoint | Python | 空目录 |

## 恢复

如需恢复某个编译器: 在仓库根目录创建 `compiler/` 目录, 将对应目录 `git mv` 回 `compiler/`, 同步更新 [`README.md`](../../README.md) 的"编译器"表.
