# AGENTS.md

> 本文件给接盘此项目的 AI Agent / 开发者使用, 列出仓库地图、工具链、命令速查与代码规范.
>
> 推荐先读 [`document/二开快速上手.md`](./document/二开快速上手.md) —— 它是面向"动手改 1 个文件"的工作流手册; 本文件是命令与规范的索引.
>
> 所有子项目的 `prompt/编码规范.md` 提供该子项目内部的更细规则 (Python / C / Vue / 服务端).

## 仓库地图

| 路径 | 作用 | 测试命令 |
|---|---|---|
| `library/python` | Python 库 (可编辑安装, `pip install -e ".[dev]"`) | `pytest -v` |
| `library/c` | C 头文件库 (单文件, C23) | `make` (默认目标即 `test`) |
| `render/vue` | Vue 3 + TS SPA (在线服务前端) | `pnpm test:unit` |
| `server/release` | FastAPI 后端 + PostgreSQL 18 | `pytest -v` |
| `server/release/admin-repl` | Lua 5.4 管理 REPL | (无自动化测试) |
| `document/` | IR 规范 / 部署 / 二开手册 | - |
| `docker/` | Dockerfile + `manage.sh` 一键脚本 | - |
| `.archived/` | 旧版 / 未实现 / demo 子项目, **不参与构建** | - |

## 构建 / 测试 / 静态检查速查

### Python 库 (`library/python`)

```bash
python -m pip install -e ".[dev]"
pytest -v                                 # 全量
pytest -v test/integration_test.py        # 仅集成
pytest -v test/unit_test.py               # 仅单元
pytest -v -k "stack"                      # 按关键字
pytest --cov                              # 覆盖率
ruff check .                              # lint
ruff format .                             # format
mypy ds4viz                              # 类型检查 (strict)
```

### C 库 (`library/c`)

```bash
make                  # 编译 + 跑 test.c (默认)
make clean            # 清理 test binary 与 t_*.toml 临时输出
```

C 库无 `make test` / `make analyze` / `make memcheck` 目标 (项目未配置);
手动 valgrind:

```bash
gcc -std=c2x -Wall -Wextra -Wpedantic -Werror -fsanitize=address,undefined -g test.c -o test -lm
./test
# 或 valgrind --leak-check=full ./test
```

### Vue 渲染器 (`render/vue`)

```bash
pnpm install                              # 或 npm / bun
pnpm dev                                  # http://localhost:5173
pnpm build                                # vue-tsc 严格检查 + Vite 打包
pnpm preview                              # 预览生产产物
pnpm test:unit                            # vitest
pnpm test:unit -- -t "name"               # 按名
pnpm test:unit -- src/__tests__/App.spec.ts  # 按文件
pnpm type-check                           # vue-tsc, 必跑
pnpm lint                                 # oxlint + eslint (--fix)
pnpm format                               # prettier --write
```

### 后端 (`server/release`)

```bash
python -m pip install bcrypt pyjwt psycopg[pool] pyyaml fastapi uvicorn python-multipart
pytest -v                                            # 自动启动测试服务器 (conftest.py)
pytest -v test/function_test.py                      # 仅功能测试
ruff check .
ruff format .
mypy .
python src/main.py --test                            # 测试配置启动
python src/main.py --prod                            # 生产配置启动
python src/main.py --docker                          # Docker 容器内启动
```

### admin-repl (`server/release/admin-repl`)

```bash
# Fedora
sudo dnf install lua-luaossl
sudo luarocks install pgmoon

cd server/release/admin-repl
lua admin-repl.lua                # 必须从此目录启动, 日志路径相对此目录
```

### Docker 一键 (推荐)

```bash
cp docker/.env.example .env
./docker/manage.sh up             # postgres + backend + frontend
./docker/manage.sh logs           # 跟踪日志
./docker/manage.sh restart backend
./docker/manage.sh rebuild
./docker/manage.sh down
./docker/manage.sh clean          # 含数据卷清理
```

## IR 契约 (硬约束)

`.toml` 是跨语言 / 跨组件的硬契约. 改 IR 必读 [`document/IR定义.md`](./document/IR定义.md).

关键不变量:

- 顶层块: `[meta]` `[package]` `[remarks]?` `[object]` `[[states]]` `[[steps]]?` `[result]`/`[error]`
- `meta.lang` ∈ {`python`, `c`}, 与 `package.lang` 一致
- `object.kind` ∈ {`stack`, `queue`, `slist`, `dlist`, `binary_tree`, `bst`, `graph_undirected`, `graph_directed`, `graph_weighted`}
- `states.id` / `steps.id` 从 0 连续递增
- `value` 字段仅允许 `int | float | str | bool`
- 空指针用 `-1`
- 无向图边必须 `from < to`
- `result` 与 `error` **二选一**, 禁止共存

改 IR 字段需要同时更新 (跨子项目级联):

1. `document/IR定义.md`
2. `library/python/ds4viz/writer.py` (序列化)
3. `library/c/ds4viz.h` (宏 `ds4viz_p_flush_` + 各结构 `ds4viz_p_*_ws_` 状态写入回调)
4. `render/vue/src/utils/ir.ts` (校验)
5. `render/vue/src/types/ir.ts` (TS 类型)
6. `library/python/test/integration_test.py` 的 `VALID_KINDS` / `VALID_LANGS`

## 代码规范速查

### 通用

- 修改前先读目标子项目 README + 本文件的工具链段, 不要从零探索
- 保持改动最小, 不做无关重排
- 改公共 API 必须同步更新对应子项目 README
- 配置文件不硬编码密码, 生产用环境变量 (参考 `server/release/src/config.py:142-146`)

### Python (`library/python`, `server/release`)

- ruff formatter, line-length 120
- mypy strict (无 `Optional[X]`, 用 `X | None`); 无未注解 def
- `snake_case` 函数/变量, `PascalCase` 类, `UPPER_SNAKE_CASE` 常量
- 错误: 用户态错误 → `RuntimeError`; 库内部 bug → `RuntimeError`; 校验失败 → 普通异常
- API `value` 字段类型仅 `int | float | str | bool`

### C (`library/c`)

- C23 (gcc `-std=c2x`); C17 兼容; `-Wall -Wextra -Wpedantic -Werror`
- `dv_` 短名 / `ds4viz` 长名 前缀
- 函数返回 `dv_error_t` 显式错误码
- 资源生命周期: create → operate → commit → destroy, commit 必须在 destroy 前

### Vue (`render/vue`)

- Prettier (`semi: false`, `singleQuote: true`, `printWidth: 100`)
- 2 空格缩进
- 优先 `<script setup>` + Composition API
- 避免 `any`; `vue-tsc` strict 必须绿
- VizPanel 用内联 SVG, **不要**改 canvas/D3

### 后端 (`server/release`)

- Pydantic 模型在 `model/<file>.py`
- 业务逻辑在 `service/<file>.py`, 路由在 `api/<file>.py`
- 抛 `Ds4VizException` 子类, 由 `main.py:113-128` 全局 handler 统一转 JSON
- 沙箱 `systemd-run` 优先, 降级 `timeout`; **生产必须**配 systemd
- 任何用户代码执行路径变更都要审视沙箱

## 强制 / 禁止行为

### 强制

1. 改前先读目标子项目 README + 本文件工具链段
2. 改 IR 必读 `document/IR定义.md`
3. 每次改后跑目标子项目测试
4. 改后端必看沙箱影响
5. 写新结构必同步跨 5 个子项目 (见 `document/二开快速上手.md` §11.1)
6. 配置文件不硬编码密码

### 禁止

1. 改 `.archived/` (不参与构建)
2. 在 `server/release/src/config/*.yml` 写明文密码
3. Python 库用 `Optional[X]` (3.10+ 新语法 `X | None`)
4. Vue 用 `any`
5. 用 canvas/D3 重写 VizPanel
6. 绕过沙箱执行用户代码
7. C 库 `realloc` 后不检查 NULL
8. 在 docker-compose 外手动启动
9. 单独维护 `SupportedLanguage` 副本 (后端模型是唯一来源)

## 紧急回滚

```bash
git checkout HEAD -- <file>             # 单文件回滚
git stash && git checkout <branch>      # 整体回退
```

## 链接

- 仓库: <https://github.com/Water-Run/ds4viz>
- 协议: **GPL-3.0-or-later** (见根 `LICENSE`)
- 二开手册: [`document/二开快速上手.md`](./document/二开快速上手.md)
- IR 规范: [`document/IR定义.md`](./document/IR定义.md)
- 部署指南: [`document/在线服务部署.md`](./document/在线服务部署.md)
- Python 库: [`library/python/README.md`](./library/python/README.md)
- C 库: [`library/c/README.md`](./library/c/README.md)
- Vue 渲染器: [`render/vue/README.md`](./render/vue/README.md)
- 后端: [`server/release/README.md`](./server/release/README.md)
