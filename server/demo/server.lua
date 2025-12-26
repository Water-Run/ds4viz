#!/usr/bin/env lua
--[[
ds4viz Demo Server
执行用户提交的多语言代码，返回标准输出（TOML IR）

运行环境需求：
- Lua 5.4+ 或 LuaJIT 2.1+
- Pegasus HTTP 框架
- SQLite3 数据库
- cjson JSON 解析库

支持语言：
- Lua, Python, JavaScript, TypeScript, C, Rust, PHP

Author:
    WaterRun
File:
    ds4viz/server/demo_server.lua
Date:
    2025-12-26
Updated:
    2025-12-26
]]

local pegasus = require("pegasus")
local sqlite3 = require("lsqlite3")
local cjson = require("cjson")

--------------------------------------------------------------------------------
-- 配置
--------------------------------------------------------------------------------

--@description: 服务器配置表
--@field host: string - 服务器监听地址
--@field port: number - 服务器监听端口
--@field default_timeout_ms: number - 默认执行超时时间（毫秒）
--@field min_timeout_ms: number - 最小超时时间
--@field max_timeout_ms: number - 最大超时时间
--@field db_path: string - SQLite数据库文件路径
--@field temp_dir: string - 临时文件目录
--@field languages: table - 支持的语言配置
local CONFIG = {
    host = os.getenv("DS4VIZ_HOST") or "localhost",
    port = tonumber(os.getenv("DS4VIZ_PORT")) or 9090,
    default_timeout_ms = tonumber(os.getenv("DS4VIZ_TIMEOUT_MS")) or 5000,
    min_timeout_ms = 100,
    max_timeout_ms = 30000,
    db_path = "ds4viz.db",
    temp_dir = "/tmp/ds4viz",

    languages = {
        lua = {
            version_cmd = "lua -v 2>&1",
            run_cmd = function(filepath, timeout_sec)
                return string.format("timeout %d lua %s 2>&1", timeout_sec, filepath)
            end,
            extension = ".lua"
        },
        python = {
            version_cmd = "python3 --version 2>&1",
            run_cmd = function(filepath, timeout_sec)
                return string.format("timeout %d python3 -u %s 2>&1", timeout_sec, filepath)
            end,
            extension = ".py"
        },
        javascript = {
            version_cmd = "node --version 2>&1",
            run_cmd = function(filepath, timeout_sec)
                return string.format("timeout %d node %s 2>&1", timeout_sec, filepath)
            end,
            extension = ".js"
        },
        typescript = {
            version_cmd = "npx ts-node --version 2>&1",
            run_cmd = function(filepath, timeout_sec)
                return string.format("timeout %d npx ts-node --transpile-only %s 2>&1", timeout_sec, filepath)
            end,
            extension = ".ts"
        },
        c = {
            version_cmd = "gcc --version 2>&1 | head -1",
            compile = true,
            compile_cmd = function(src, out, timeout_sec)
                return string.format("timeout %d gcc -o %s %s -lm 2>&1", timeout_sec, out, src)
            end,
            run_cmd = function(filepath, timeout_sec)
                return string.format("timeout %d %s 2>&1", timeout_sec, filepath)
            end,
            extension = ".c"
        },
        rust = {
            version_cmd = "rustc --version 2>&1",
            compile = true,
            compile_cmd = function(src, out, timeout_sec)
                return string.format("timeout %d rustc -o %s %s 2>&1", timeout_sec, out, src)
            end,
            run_cmd = function(filepath, timeout_sec)
                return string.format("timeout %d %s 2>&1", timeout_sec, filepath)
            end,
            extension = ".rs"
        },
        php = {
            version_cmd = "php --version 2>&1 | head -1",
            run_cmd = function(filepath, timeout_sec)
                return string.format("timeout %d php %s 2>&1", timeout_sec, filepath)
            end,
            extension = ".php"
        }
    }
}

--------------------------------------------------------------------------------
-- 工具函数
--------------------------------------------------------------------------------

--@description: 生成 UUID v4 格式的唯一标识符
--@return: string - UUID 字符串
--@usage: local id = uuid()
local function uuid()
    local template = "xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx"
    return (template:gsub("[xy]", function(c)
        local v = (c == "x") and math.random(0, 15) or math.random(8, 11)
        return string.format("%x", v)
    end))
end

--@description: 获取 ISO 8601 格式的当前 UTC 时间戳
--@return: string - ISO 8601 时间戳字符串
--@usage: local timestamp = iso8601_now()
local function iso8601_now()
    return os.date("!%Y-%m-%dT%H:%M:%S") .. ".000Z"
end

--@description: 获取毫秒级时间戳
--@return: number - 毫秒时间戳
--@usage: local ms = get_time_ms()
local function get_time_ms()
    local ok, socket = pcall(require, "socket")
    if ok and socket.gettime then
        return math.floor(socket.gettime() * 1000)
    end
    return os.time() * 1000
end

--@description: 发送 JSON 格式的 HTTP 响应
--@param res: table - Pegasus 响应对象
--@param status: number - HTTP 状态码
--@param data: table - 响应数据表
--@usage: json_response(res, 200, {status = "ok"})
local function json_response(res, status, data)
    res:statusCode(status)
    res:addHeader("Content-Type", "application/json; charset=utf-8")
    res:addHeader("Access-Control-Allow-Origin", "*")
    res:addHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS")
    res:addHeader("Access-Control-Allow-Headers", "Content-Type")
    res:write(cjson.encode(data))
end

--@description: 读取 HTTP 请求体
--@param req: table - Pegasus 请求对象
--@return: string|nil - 请求体内容或 nil
--@usage: local body = read_body(req)
local function read_body(req)
    local body = req:post()
    if type(body) == "string" then
        return body
    end
    return nil
end

--@description: 写入文件内容
--@param path: string - 文件路径
--@param content: string - 文件内容
--@return: boolean - 是否成功
--@return: string|nil - 错误信息
--@usage: local ok, err = write_file("/tmp/test.txt", "content")
local function write_file(path, content)
    local file = io.open(path, "w")
    if not file then
        return false, "无法创建文件: " .. path
    end
    file:write(content)
    file:close()
    return true
end

--@description: 删除文件（静默失败）
--@param path: string - 文件路径
--@usage: remove_file("/tmp/test.txt")
local function remove_file(path)
    os.remove(path)
end

--@description: 确保目录存在，不存在则创建
--@param path: string - 目录路径
--@usage: ensure_dir("/tmp/ds4viz")
local function ensure_dir(path)
    os.execute(string.format("mkdir -p '%s' 2>/dev/null", path))
end

--@description: 执行 shell 命令并获取输出
--@param cmd: string - shell 命令
--@return: string|nil - 命令输出
--@return: number|nil - 退出码
--@usage: local output, code = exec_shell("ls -la")
local function exec_shell(cmd)
    local handle = io.popen(cmd, "r")
    if not handle then
        return nil, "无法执行命令"
    end
    local output = handle:read("*a")
    local success, exit_type, exit_code = handle:close()
    return output, exit_code
end

--@description: 获取编程语言的版本信息
--@param lang_config: table - 语言配置表
--@return: string - 版本信息字符串
--@usage: local version = get_lang_version(CONFIG.languages.python)
local function get_lang_version(lang_config)
    local output, _ = exec_shell(lang_config.version_cmd)
    if output then
        local first_line = output:match("^([^\n]*)")
        if first_line then
            return first_line:match("^%s*(.-)%s*$")
        end
    end
    return "unknown"
end

--@description: 获取支持的语言列表
--@return: table - 语言名称数组
--@usage: local langs = get_supported_languages()
local function get_supported_languages()
    local langs = {}
    for lang, _ in pairs(CONFIG.languages) do
        table.insert(langs, lang)
    end
    table.sort(langs)
    return langs
end

--------------------------------------------------------------------------------
-- 数据库
--------------------------------------------------------------------------------

--@description: SQLite 数据库连接对象
local db

--@description: 初始化 SQLite 数据库，创建必要的表和索引
--@usage: init_db()
local function init_db()
    db = sqlite3.open(CONFIG.db_path)
    db:exec([[
        CREATE TABLE IF NOT EXISTS exec_runs (
            id            INTEGER PRIMARY KEY AUTOINCREMENT,
            created_at    TEXT NOT NULL DEFAULT (strftime('%Y-%m-%dT%H:%M:%fZ','now')),
            request_id    TEXT NOT NULL,
            lang          TEXT NOT NULL,
            lang_version  TEXT NOT NULL,
            code          TEXT NOT NULL,
            status        TEXT NOT NULL,
            duration_ms   INTEGER NOT NULL,
            timeout_ms    INTEGER NULL,
            client_ip     TEXT NULL,
            toml          TEXT NULL,
            http_error    TEXT NULL
        );
        CREATE INDEX IF NOT EXISTS idx_exec_runs_created_at ON exec_runs(created_at);
        CREATE INDEX IF NOT EXISTS idx_exec_runs_status     ON exec_runs(status);
        CREATE INDEX IF NOT EXISTS idx_exec_runs_request_id ON exec_runs(request_id);
    ]])
end

--@description: 记录代码执行日志到数据库
--@param params: table - 执行参数表
--@field request_id: string - 请求ID
--@field lang: string - 编程语言
--@field lang_version: string - 语言版本
--@field code: string - 源代码
--@field status: string - 执行状态
--@field duration_ms: number - 执行耗时
--@field timeout_ms: number|nil - 超时设置
--@field client_ip: string|nil - 客户端IP
--@field toml: string|nil - TOML输出
--@field http_error: string|nil - HTTP错误信息
--@usage: log_execution({request_id = "xxx", lang = "python", ...})
local function log_execution(params)
    local stmt = db:prepare([[
        INSERT INTO exec_runs (request_id, lang, lang_version, code, status, duration_ms, timeout_ms, client_ip, toml, http_error)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
    ]])
    if stmt then
        stmt:bind_values(
            params.request_id or "",
            params.lang or "",
            params.lang_version or "",
            params.code or "",
            params.status or "error",
            params.duration_ms or 0,
            params.timeout_ms,
            params.client_ip,
            params.toml,
            params.http_error
        )
        stmt:step()
        stmt:finalize()
    end
end

--------------------------------------------------------------------------------
-- 代码执行器
--------------------------------------------------------------------------------

--@description: 执行用户提交的代码
--@param lang: string - 编程语言
--@param code: string - 源代码
--@param timeout_ms: number - 超时时间（毫秒）
--@return: string|nil - 标准输出内容
--@return: number - 执行耗时（毫秒）
--@return: string - 执行状态（"ok" 或 "error"）
--@return: string|nil - 错误信息
--@usage: local output, duration, status, err = execute_code("python", code, 5000)
local function execute_code(lang, code, timeout_ms)
    local lang_config = CONFIG.languages[lang]
    if not lang_config then
        return nil, 0, "error", "不支持的语言: " .. lang
    end

    local start_time = get_time_ms()
    local timeout_sec = math.ceil(timeout_ms / 1000)

    ensure_dir(CONFIG.temp_dir)

    local file_id = uuid()
    local src_path = CONFIG.temp_dir .. "/" .. file_id .. lang_config.extension
    local exe_path = CONFIG.temp_dir .. "/" .. file_id

    local ok, err = write_file(src_path, code)
    if not ok then
        return nil, get_time_ms() - start_time, "error", err
    end

    local output, exit_code
    local cleanup_files = { src_path }

    if lang_config.compile then
        local compile_cmd = lang_config.compile_cmd(src_path, exe_path, timeout_sec)
        local compile_output, compile_exit = exec_shell(compile_cmd)

        if compile_exit ~= 0 then
            for _, f in ipairs(cleanup_files) do remove_file(f) end
            local duration = get_time_ms() - start_time
            return compile_output or "编译失败", duration, "error", nil
        end

        table.insert(cleanup_files, exe_path)
    end

    local run_target = lang_config.compile and exe_path or src_path
    local run_cmd = lang_config.run_cmd(run_target, timeout_sec)

    output, exit_code = exec_shell(run_cmd)

    for _, f in ipairs(cleanup_files) do
        remove_file(f)
    end

    local duration = get_time_ms() - start_time

    if exit_code == 124 then
        return output or "", duration, "error", "执行超时"
    end

    local status = (exit_code == 0) and "ok" or "error"
    return output or "", duration, status, nil
end

--------------------------------------------------------------------------------
-- 请求处理器
--------------------------------------------------------------------------------

--@description: 处理健康检查请求
--@param req: table - Pegasus 请求对象
--@param res: table - Pegasus 响应对象
--@usage: handle_ping(req, res)
local function handle_ping(req, res)
    json_response(res, 200, {
        status = "ok",
        timestamp = iso8601_now(),
        supported_languages = get_supported_languages(),
        default_timeout_ms = CONFIG.default_timeout_ms
    })
end

--@description: 处理代码执行请求
--@param req: table - Pegasus 请求对象
--@param res: table - Pegasus 响应对象
--@usage: handle_execute(req, res)
local function handle_execute(req, res)
    local request_id = uuid()
    local start_time = get_time_ms()
    local client_ip = req:headers()["X-Forwarded-For"] or req:headers()["x-forwarded-for"] or "unknown"

    local body = read_body(req)
    if not body or body == "" then
        local duration = get_time_ms() - start_time
        log_execution({
            request_id = request_id,
            lang = "",
            lang_version = "",
            code = "",
            status = "error",
            duration_ms = duration,
            client_ip = client_ip,
            http_error = "请求体为空"
        })
        json_response(res, 400, {
            error = "validation",
            message = "请求体为空"
        })
        return
    end

    local ok, data = pcall(cjson.decode, body)
    if not ok then
        local duration = get_time_ms() - start_time
        log_execution({
            request_id = request_id,
            lang = "",
            lang_version = "",
            code = "",
            status = "error",
            duration_ms = duration,
            client_ip = client_ip,
            http_error = "无效的 JSON"
        })
        json_response(res, 400, {
            error = "validation",
            message = "无效的 JSON"
        })
        return
    end

    if not data.lang or type(data.lang) ~= "string" or data.lang == "" then
        local duration = get_time_ms() - start_time
        log_execution({
            request_id = request_id,
            lang = "",
            lang_version = "",
            code = data.code or "",
            status = "error",
            duration_ms = duration,
            timeout_ms = data.timeout_ms,
            client_ip = client_ip,
            http_error = "缺少必填字段: lang"
        })
        json_response(res, 400, {
            error = "validation",
            message = "缺少必填字段: lang"
        })
        return
    end

    if not data.code or type(data.code) ~= "string" then
        local duration = get_time_ms() - start_time
        log_execution({
            request_id = request_id,
            lang = data.lang,
            lang_version = "",
            code = "",
            status = "error",
            duration_ms = duration,
            timeout_ms = data.timeout_ms,
            client_ip = client_ip,
            http_error = "缺少必填字段: code"
        })
        json_response(res, 400, {
            error = "validation",
            message = "缺少必填字段: code"
        })
        return
    end

    local lang_config = CONFIG.languages[data.lang]
    if not lang_config then
        local duration = get_time_ms() - start_time
        log_execution({
            request_id = request_id,
            lang = data.lang,
            lang_version = "",
            code = data.code,
            status = "error",
            duration_ms = duration,
            timeout_ms = data.timeout_ms,
            client_ip = client_ip,
            http_error = "不支持的语言: " .. data.lang
        })
        json_response(res, 400, {
            error = "validation",
            message = "不支持的语言: " .. data.lang
        })
        return
    end

    local timeout_ms = data.timeout_ms or CONFIG.default_timeout_ms
    if type(timeout_ms) ~= "number" or timeout_ms < CONFIG.min_timeout_ms or timeout_ms > CONFIG.max_timeout_ms then
        local duration = get_time_ms() - start_time
        log_execution({
            request_id = request_id,
            lang = data.lang,
            lang_version = "",
            code = data.code,
            status = "error",
            duration_ms = duration,
            timeout_ms = data.timeout_ms,
            client_ip = client_ip,
            http_error = string.format("timeout_ms 必须在 %d 到 %d 之间", CONFIG.min_timeout_ms, CONFIG.max_timeout_ms)
        })
        json_response(res, 400, {
            error = "validation",
            message = string.format("timeout_ms 必须在 %d 到 %d 之间", CONFIG.min_timeout_ms, CONFIG.max_timeout_ms)
        })
        return
    end

    local lang_version = get_lang_version(lang_config)

    local output, exec_duration, status, exec_error = execute_code(data.lang, data.code, timeout_ms)

    local total_duration = get_time_ms() - start_time

    if exec_error and not output then
        log_execution({
            request_id = request_id,
            lang = data.lang,
            lang_version = lang_version,
            code = data.code,
            status = "error",
            duration_ms = total_duration,
            timeout_ms = timeout_ms,
            client_ip = client_ip,
            http_error = exec_error
        })
        json_response(res, 500, {
            error = "internal",
            message = exec_error
        })
        return
    end

    log_execution({
        request_id = request_id,
        lang = data.lang,
        lang_version = lang_version,
        code = data.code,
        status = status,
        duration_ms = total_duration,
        timeout_ms = timeout_ms,
        client_ip = client_ip,
        toml = output
    })

    json_response(res, 200, {
        request_id = request_id,
        duration_ms = exec_duration,
        toml = output
    })
end

--@description: 处理 CORS 预检请求
--@param req: table - Pegasus 请求对象
--@param res: table - Pegasus 响应对象
--@usage: handle_options(req, res)
local function handle_options(req, res)
    res:statusCode(204)
    res:addHeader("Access-Control-Allow-Origin", "*")
    res:addHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS")
    res:addHeader("Access-Control-Allow-Headers", "Content-Type")
    res:write("")
end

--------------------------------------------------------------------------------
-- 路由
--------------------------------------------------------------------------------

--@description: HTTP 请求路由处理器
--@param req: table - Pegasus 请求对象
--@param res: table - Pegasus 响应对象
--@usage: router(req, res)
local function router(req, res)
    local method = req:method()
    local path = req:path()

    if method == "OPTIONS" then
        handle_options(req, res)
        return
    end

    if path == "/ping" and method == "GET" then
        handle_ping(req, res)
    elseif path == "/execute" and method == "POST" then
        handle_execute(req, res)
    else
        json_response(res, 404, {
            error = "not_found",
            message = "接口不存在: " .. method .. " " .. path
        })
    end
end

--------------------------------------------------------------------------------
-- 主程序
--------------------------------------------------------------------------------

math.randomseed(os.time())

init_db()

ensure_dir(CONFIG.temp_dir)

print(string.format("[ds4viz] Demo Server 启动于 %s:%d", CONFIG.host, CONFIG.port))
print("[ds4viz] 支持语言: " .. table.concat(get_supported_languages(), ", "))
print("[ds4viz] 默认超时: " .. CONFIG.default_timeout_ms .. "ms")
print("[ds4viz] 临时目录: " .. CONFIG.temp_dir)
print("[ds4viz] 按 Ctrl+C 停止服务")

local server = pegasus:new({
    host = CONFIG.host,
    port = CONFIG.port
})

server:start(router)
