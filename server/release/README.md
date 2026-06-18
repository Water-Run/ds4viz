# `ds4viz` 服务系统

`ds4viz` 的在线服务后端 (FastAPI + PostgreSQL).
运行于 `Fedora 43`, 协议 **GPL-3.0-or-later**.

- 仓库: <https://github.com/Water-Run/ds4viz>
- 文档: [`docs/`](./docs/) (表定义 / API参考 / 数据库环境准备)
- 配套渲染器: [`../../render/vue`](../../render/vue)
- 配套库: [`../../library/python`](../../library/python) (Py) / [`../../library/c`](../../library/c) (C)
- 管理 REPL: [`./admin-repl/`](./admin-repl/) (Lua 5.4 + pgmoon + luaossl)

## 目录结构

```text
server/release/
├── README.md
├── admin-repl/             # Lua 5.4 管理 REPL, 直连数据库
├── docs/                   # 项目文档
│   ├── API参考.md           # 全栈 API 文档
│   ├── 表定义.md            # 8 张表 + 触发器
│   └── 数据库环境准备.md    # PostgreSQL 18 部署
├── prompts/                # 后端开发规范与设计稿
│   ├── 编码规范.md
│   └── 服务系统.md
├── sql/
│   ├── init.sql            # 8 张表 + 触发器 (182 行)
│   └── templates.sql       # 50 模板 × 2 语言 = 100 条种子 (3474 行)
├── src/
│   ├── main.py             # 入口; --test / --prod / --docker 三套配置
│   ├── config.py           # yml 配置加载 (201 行)
│   ├── database.py         # psycopg 连接池 (85 行)
│   ├── exceptions.py       # 业务异常基类 (207 行)
│   ├── log.py              # 数据库日志 + ContextVar 请求上下文 (329 行)
│   ├── utils.py            # bcrypt/JWT/SHA256/IP/UUID (139 行)
│   ├── api/                # 路由层
│   │   ├── __init__.py     # get_current_user / get_optional_user 依赖
│   │   ├── auth.py         # /api/auth/* (注册/登录/登出/会话)
│   │   ├── user.py         # /api/users/* (个人资料/头像/收藏)
│   │   ├── template.py     # /api/templates/* (模板库/搜索/详情)
│   │   └── execution.py    # /api/execute + /api/executions/* + /api/favorites/*
│   ├── service/            # 业务层
│   │   ├── auth_service.py
│   │   ├── user_service.py
│   │   ├── template_service.py
│   │   ├── execution_service.py
│   │   └── sandbox_service.py    # 沙箱执行 (638 行, 核心安全模块)
│   ├── model/              # Pydantic 模型
│   │   ├── execution.py    # SupportedLanguage(StrEnum)
│   │   ├── user.py
│   │   └── template.py
│   └── config/             # yml 配置 (test/prod/docker)
└── test/                   # pytest
    ├── conftest.py         # 启动子进程服务器 + DB 重置 (250 行)
    └── function_test.py    # 全栈功能测试 (3379 行)
```

![后端架构图](../../assets/在线服务后端架构图.png)

## 启动

### 准备环境

```bash
python -m pip install bcrypt pyjwt psycopg[pool] pyyaml fastapi uvicorn python-multipart
```

详见 [`docs/数据库环境准备.md`](./docs/数据库环境准备.md).

### 跑测试

```bash
pytest -v                       # 全量 (conftest.py 自动启动测试服务器)
pytest -v test/function_test.py -k "auth"  # 单点
ruff check . && ruff format . && mypy .
```

### 启动服务 (三套配置)

```bash
python src/main.py --test        # 测试配置 (端口 10000, 短会话有效期)
python src/main.py --prod        # 生产配置
python src/main.py --docker      # Docker 容器内配置
```

启动后访问:
- API 根: `http://localhost:10000`
- Swagger: `http://localhost:10000/docs`
- 重新初始化数据库: 见 [`docs/数据库环境准备.md`](./docs/数据库环境准备.md)

### 一键 Docker 启动 (推荐)

仓库根目录提供 `./docker/manage.sh`, 包含 postgres + backend + frontend 一键拉起.

```bash
# 仓库根目录
cp docker/.env.example .env
./docker/manage.sh up
```

## 沙箱安全约束

后端在 `service/sandbox_service.py` 实现两层沙箱:

- **首选**: `systemd-run --user --scope --property=MemoryMax=...M --property=CPUQuota=50% -- timeout ... <cmd>`
- **降级**: `timeout --signal=KILL ... <cmd>` (Docker 容器内 systemd 不可用)

关键参数 (`config/test.yml` 与 `config/prod.yml`):

| 字段 | test | prod | 说明 |
|---|---|---|---|
| `sandbox.timeout_seconds` | 10 | 30 | 硬超时 (s) |
| `sandbox.max_memory_mb`   | 256 | 512 | systemd-run 模式生效 |
| `sandbox.temp_dir`        | `/tmp/ds4viz_test/exec` | `/tmp/ds4viz/exec` | 用户代码工作区 |

**生产必须**配 systemd + cgroup, 否则 fork/exec 出的孙子进程可逃逸 timeout.

## 文档

- [API 参考](./docs/API参考.md)
- [表定义](./docs/表定义.md)
- [数据库环境准备](./docs/数据库环境准备.md)
- [管理 REPL](./admin-repl/README.md)
