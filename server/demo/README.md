# `ds4viz` Demo服务器  

Demo服务器简单实现: 采用`lua`+`pegasus`+`sqlite`技术栈.  

`pegasus`安装:  

```bash
luarocks install pegasus
```

测试服务部署在`Fedora 43`, 采用`lua 5.4.8`.  

建表:  

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
  lang          TEXT NOT NULL,              -- e.g. lua/python/...
  lang_version  TEXT NOT NULL,

  -- user code (store full source as requested)
  code          TEXT NOT NULL,

  -- result summary
  status        TEXT NOT NULL,              -- ok | runtime | timeout | validation | sandbox | http | unknown
  duration_ms   INTEGER NOT NULL,

  -- optional config / client info
  timeout_ms    INTEGER NULL,
  client_ip     TEXT NULL,

  -- optional details for debugging
  http_status   INTEGER NULL,
  exit_code     INTEGER NULL,
  stdout        TEXT NULL,
  stderr        TEXT NULL,
  toml          TEXT NULL,                  -- store full trace.toml if you want; otherwise keep NULL and return only

  error_type    TEXT NULL,                  -- aligns with IR: runtime/timeout/validation/sandbox/unknown (+ http)
  error_message TEXT NULL
);

CREATE INDEX IF NOT EXISTS idx_exec_runs_created_at ON exec_runs(created_at);
CREATE INDEX IF NOT EXISTS idx_exec_runs_status     ON exec_runs(status);
CREATE INDEX IF NOT EXISTS idx_exec_runs_request_id ON exec_runs(request_id);
```

启动服务:  

```bash
lua server.lua
```
