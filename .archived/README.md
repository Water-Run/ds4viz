# `.archived` 目录

存放已被归档、暂不维护的子项目, 包括:

- 早期实现或仅作占位的多语言 `.toml` IR 生成库
- 未完成 / 已弃用的渲染器与编译器
- 仅作历史验证用的 demo 服务

> 归档内容**不参与当前构建与发布**, 保留仅为历史参考与未来参考实现.
> 若需要恢复某个归档项目, 直接将其移回原位置即可 (`git mv` 保留历史).

## 当前归档内容

| 类别 | 路径 | 归档原因 |
|------|------|----------|
| 语言库 | [`.archived/libraries/`](./libraries/README.md) | 未实现 / 旧版 / 暂停支持 |
| 渲染器 | [`.archived/renderers/`](./renderers/README.md) | 未实现 / 旧版 demo / 已弃用 |
| 编译器 | [`.archived/compilers/`](./compilers/README.md) | 全部未实现 |
| 服务 | [`.archived/server/`](./server/README.md) | demo 实现, 已被 `server/release` 取代 |

## 当前活跃项目

位于仓库根目录的 `library/`, `render/`, `server/`:

- **语言库**: `library/python`, `library/c`
- **渲染器**: `render/vue`
- **服务**: `server/release`

详见根目录 [`README.md`](../README.md).
