# `ds4viz`: Release 服务器

`ds4viz` 生产服务器,提供安全的远程代码执行服务,返回 TOML IR 用于数据结构可视化。

> 部署环境: `Fedora 43` | 作者: WaterRun | 更新: 2026-01-16

## 技术栈

- **Web框架**: Rust + Poem 3.x
- **数据库**: PostgreSQL 18 (用户、执行记录、缓存)
- **沙箱**: systemd-run (cgroup 资源隔离)
- **认证**: JWT (HttpOnly Cookie)
- **缓存**: 双层缓存 (LRU 内存 + PostgreSQL 持久化)

## 核心功能

✅ **用户认证**
- 注册/登录/登出
- JWT 令牌管理 (Access + Refresh Token)
- 密码哈希 (Argon2)

✅ **代码执行**
- 支持语言: Python 3.14, Lua 5.4, Rust 1.75
- 沙箱隔离: systemd cgroup (CPU/内存/进程数限制)
- 超时控制: 30秒(解释型) / 60秒(编译型)
- 执行缓存: SHA256(language_id + code)

✅ **配额管理**
- Standard 用户: 10秒 CPU/小时
- Enhanced 用户: 无限制
- 自动周期重置

✅ **安全措施**
- 文件系统隔离 (ProtectSystem, PrivateTmp)
- 禁止提权 (NoNewPrivileges)
- 资源限制 (256MB 内存, 64 进程)
- 只读系统目录

## 架构设计

```
客户端
  ↓ POST /api/v1/executions
  ↓ { language_id: "python", code: "..." }
服务器
  ├─ 1. 验证请求 (代码大小、语言)
  ├─ 2. 计算哈希 SHA256(language + code)
  ├─ 3. 检查缓存 (L1:LRU + L2:PostgreSQL)
  │    └─ 命中 → 返回缓存结果 (200 OK)
  ├─ 4. 检查配额 (Standard: 10s/h)
  ├─ 5. 创建提交记录 (status=pending)
  ├─ 6. 异步执行 (202 Accepted)
  │    ├─ 创建临时文件 (/tmp/ds4viz/exec_{uuid}.py)
  │    ├─ systemd-run 沙箱执行
  │    ├─ 读取 trace.toml
  │    ├─ 清理临时文件
  │    ├─ 写入缓存
  │    └─ 更新配额
  └─ 7. 客户端轮询 GET /executions/{id}
```

## 编译和测试

### 编译

```bash
# 开发模式
cargo build

# 生产模式 (优化)
cargo build --release

# 类型检查 (快速)
cargo check
```

### 测试

```bash
# 运行所有测试
cargo test

# 仅单元测试
cargo test --lib

# 包含 systemd 集成测试 (需要权限)
cargo test -- --ignored --test-threads=1

# 代码格式检查
cargo fmt --check

# Clippy 检查
cargo clippy -- -D warnings
```

## 配置项和说明

配置文件: `configs/default.toml`

### 核心配置

| 配置项 | 默认值 | 说明 |
|--------|--------|------|
| `server.host` | `127.0.0.1` | 监听地址 |
| `server.port` | `8080` | 监听端口 |
| `database.url` | `postgres://...` | PostgreSQL 连接字符串 |
| `database.max_connections` | `10` | 连接池大小 |
| `jwt.secret` | `your-secret-key...` | JWT 签名密钥 ⚠️ 生产必改 |
| `jwt.access_token_ttl_secs` | `3600` | Access Token 有效期 (1小时) |
| `jwt.refresh_token_ttl_secs` | `604800` | Refresh Token 有效期 (7天) |
| `quota.period_minutes` | `60` | 配额周期 (分钟) |
| `quota.standard_cpu_time_ms` | `10000` | Standard 用户 CPU 配额 (10秒) |
| `quota.standard_max_code_bytes` | `16384` | Standard 用户代码大小限制 (16KB) |
| `execution.default_timeout_ms` | `30000` | 默认超时 (30秒) |
| `execution.temp_dir` | `/tmp/ds4viz` | 临时文件目录 |

## 启动服务

### 1. 数据库初始化

```bash
# 创建数据库
createdb ds4viz

# 初始化表结构
psql ds4viz < init.sql

# 验证
psql ds4viz -c "SELECT * FROM languages;"
```

### 2. 配置文件

```bash
# 复制并修改配置
cp configs/default.toml configs/production.toml

# ⚠️ 必须修改:
# - jwt.secret: 使用强随机密钥
# - database.url: 生产数据库连接
```

### 3. 系统依赖

```bash
# Fedora/RHEL
sudo dnf install systemd python3 lua rustc

# 验证 systemd-run 可用
systemd-run --user --scope echo "test"

# 创建临时目录
mkdir -p /tmp/ds4viz
chmod 755 /tmp/ds4viz
```

### 4. 启动服务

```bash
# 开发模式
cargo run

# 生产模式 (指定配置)
cargo run --release -- --config configs/production.toml

# 后台运行
nohup cargo run --release &> server.log &
```

## 后台管理  

后台管理系统以部署在服务器本机上的TUI提供.  

### 技术栈  

- `lua`  
- `ltui`  
- `sqlite`  

### 启动  

```bash
lua main.lua
```

### 配置定义和修改  

|配置项|说明|
|---|---|
|`password`|进入口令, 用于在部署在公网上等情况时添加最小的验证|
