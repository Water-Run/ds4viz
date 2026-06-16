# 归档服务

已从 `server/` 移出的历史服务实现, 已被 `server/release` 取代.

当前活跃服务见 [`server/`](../../server/).

## 目录

| 目录 | 服务 | 技术栈 | 归档原因 | 备注 |
|------|------|--------|----------|------|
| [`demo/`](./demo/) | Demo 在线服务 | `lua` + `pegasus` + `sqlite` | 旧版 demo | 仅作功能验证, 不检查安全性, 已由 `server/release` (FastAPI + PostgreSQL) 取代 |

## 恢复

如需恢复 demo 服务: 将 `demo/` `git mv` 回 `server/`, 同步更新 [`README.md`](../../README.md) 与 [`document/在线服务部署.md`](../../document/在线服务部署.md).
