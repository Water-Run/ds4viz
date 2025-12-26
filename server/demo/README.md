# `ds4viz` Demo服务器  

Demo服务器简单实现: 采用`lua`+`pegasus`+`sqlite`技术栈.  

> Demo服务器仅验证功能, 不检查安全性  

## 依赖安装

```bash
luarocks install pegasus
luarocks install lsqlite3
luarocks install lua-cjson
```

测试服务部署在`Fedora 43`, 采用`lua 5.4.8`.  

## 数据库初始化

```sql
-- ds4viz demo: execution log
-- SQLite schema

CREATE TABLE IF NOT EXISTS exec_runs (
  id            INTEGER PRIMARY KEY AUTOINCREMENT,

  -- time
  created_at    TEXT NOT NULL DEFAULT (strftime('%Y-%m-%dT%H:%M:%fZ','now')),

  -- request correlation (recommended)
  request_id    TEXT NOT NULL,              -- UUID string

  -- execution context
  lang          TEXT NOT NULL,              -- e.g. lua/python/javascript/...
  lang_version  TEXT NOT NULL,

  -- user code (store full source as requested)
  code          TEXT NOT NULL,

  -- result summary
  status        TEXT NOT NULL,              -- ok | error
  duration_ms   INTEGER NOT NULL,

  -- optional config / client info
  timeout_ms    INTEGER NULL,
  client_ip     TEXT NULL,

  -- result
  toml          TEXT NULL,                  -- 生成的完整 trace.toml

  -- HTTP层错误(非业务错误)
  http_error    TEXT NULL                   -- 仅当请求本身失败时记录
);

CREATE INDEX IF NOT EXISTS idx_exec_runs_created_at ON exec_runs(created_at);
CREATE INDEX IF NOT EXISTS idx_exec_runs_status     ON exec_runs(status);
CREATE INDEX IF NOT EXISTS idx_exec_runs_request_id ON exec_runs(request_id);
```

## 启动服务

```bash
lua demo_server.lua
```

## 环境变量配置

| 变量 | 说明 | 默认值 |
|---|---|---|
| `DS4VIZ_HOST` | 服务器监听地址 | `localhost` |
| `DS4VIZ_PORT` | 服务器监听端口 | `9090` |
| `DS4VIZ_TIMEOUT_MS` | 代码执行超时时间(毫秒) | `5000` |

## 支持的编程语言

服务器当前支持以下编程语言:

- `lua`
- `python`
- `javascript`
- `typescript`
- `c`
- `rust`
- `php`

## API 接口

| 路径 | 方法 | 说明 |
|---|---|---|
| `/ping` | `GET` | 健康检查 |
| `/execute` | `POST` | 执行代码, 返回 TOML IR |

### `GET /ping`

健康检查接口.

**请求示例:**

```bash
curl http://localhost:9090/ping
```

**响应 (HTTP 200):**

```json
{
  "status": "ok",
  "timestamp": "2025-12-26T10:30:00.000Z",
  "supported_languages": ["lua", "python", "javascript", "typescript", "c", "rust", "php"],
  "default_timeout_ms": 5000
}
```

### `POST /execute`

执行用户代码, 返回生成的 TOML IR. 

代码执行的结果（包括运行时错误、超时等）均体现在返回的 TOML 中（`[result]` 或 `[error]` 块），HTTP 状态码仅反映请求本身是否有效.

**请求头:**

```
Content-Type: application/json
```

**请求体:**

| 字段 | 类型 | 必填 | 说明 |
|---|---|---|---|
| `lang` | `string` | 是 | 编程语言, 取值见上方"支持的编程语言"列表 |
| `code` | `string` | 是 | 待执行的源代码 |
| `timeout_ms` | `integer` | 否 | 超时时间(毫秒), 默认 5000, 范围 100-30000 |

**请求示例:**

```bash
curl -X POST http://localhost:9090/execute \
  -H "Content-Type: application/json" \
  -d '{
    "lang": "python",
    "code": "from ds4viz import stack\nwith stack() as s:\n    s.push(10)\n    s.push(20)",
    "timeout_ms": 3000
  }'
```

**成功响应 (HTTP 200):**

无论代码执行成功还是发生运行时错误, 只要请求有效且执行流程完成, 均返回 HTTP 200. 执行结果体现在 TOML 的 `[result]` 或 `[error]` 块中.

```json
{
  "request_id": "550e8400-e29b-41d4-a716-446655440000",
  "duration_ms": 150,
  "toml": "[meta]\ncreated_at = \"2025-12-26T10:30:00.000Z\"\nlang = \"python\"\nlang_version = \"3.12.0\"\n\n[package]\nname = \"ds4viz\"\nlang = \"python\"\nversion = \"1.0.0\"\n\n..."
}
```

**请求无效 (HTTP 400):**

请求参数校验失败时返回.

```json
{
  "error": "validation",
  "message": "缺少必填字段: code"
}
```

```json
{
  "error": "validation",
  "message": "不支持的语言: golang"
}
```

```json
{
  "error": "validation", 
  "message": "timeout_ms 必须在 100 到 30000 之间"
}
```

**服务器错误 (HTTP 500):**

服务器内部错误（非代码执行错误）时返回.

```json
{
  "error": "internal",
  "message": "执行器进程启动失败"
}
```

### 响应说明

`/execute` 的设计遵循以下原则:

1. **HTTP 状态码仅反映请求层面的状态**: 400 表示请求参数无效, 500 表示服务器内部错误, 200 表示请求已被正确处理
2. **代码执行结果完全由 TOML IR 表达**: 运行时错误、超时、沙箱错误等均记录在 TOML 的 `[error]` 块中, 符合 IR 规范定义
3. **前端只需解析 TOML**: 收到 HTTP 200 后, 前端直接解析 TOML, 根据是否存在 `[result]` 或 `[error]` 块判断执行结果

### CORS

服务器支持跨域请求:

```plaintext
Access-Control-Allow-Origin: *
Access-Control-Allow-Methods: GET, POST, OPTIONS
Access-Control-Allow-Headers: Content-Type
```
