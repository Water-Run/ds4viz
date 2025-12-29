#!/usr/bin/env lua
--[[
ds4viz Demo Server
执行用户提交的多语言代码，读取生成的 trace.toml 文件返回

运行环境需求：
- Lua 5.4+ 或 LuaJIT 2.1+
- Pegasus HTTP 框架
- SQLite3 数据库
- cjson JSON 解析库

支持语言：
- Lua, Python, Rust

Author:
    WaterRun
File:
    ds4viz/server/demo/server.lua
Date:
    2025-12-26
Updated:
    2025-12-29
]]

local Pegasus = require("pegasus")
local sqlite3 = require("lsqlite3")
local cjson = require("cjson")

--------------------------------------------------------------------------------
-- 配置
--------------------------------------------------------------------------------

local CONFIG = {
    host = os.getenv("DS4VIZ_HOST") or "0.0.0.0",
    port = os.getenv("DS4VIZ_PORT") or "9090",
    default_timeout_ms = tonumber(os.getenv("DS4VIZ_TIMEOUT_MS")) or 5000,
    min_timeout_ms = 100,
    max_timeout_ms = 30000,
    db_path = "ds4viz.db",
    temp_dir = "/tmp/ds4viz",
    trace_filename = "trace.toml",

    languages = {
        lua = {
            version_cmd = "lua -v 2>&1",
            run_cmd = function(filepath, timeout_sec, work_dir)
                return string.format("cd '%s' && timeout %d lua '%s' 2>&1", work_dir, timeout_sec, filepath)
            end,
            extension = ".lua"
        },
        python = {
            version_cmd = "python3 --version 2>&1",
            run_cmd = function(filepath, timeout_sec, work_dir)
                return string.format("cd '%s' && timeout %d python3 -u '%s' 2>&1", work_dir, timeout_sec, filepath)
            end,
            extension = ".py"
        },
        rust = {
            version_cmd = "rustc --version 2>&1",
            compile = true,
            compile_cmd = function(src, out, timeout_sec, work_dir)
                local deps = "/home/waterrun/Coding/ds4viz/library/rust/target/debug/deps"
                return string.format([[
cd '%s' && timeout %d bash -lc '
DS4VIZ_DEPS="%s"
DS4VIZ_RLIB=$(ls %s/libds4viz-*.rlib 2>/dev/null | head -n 1)
if [ -z "$DS4VIZ_RLIB" ]; then
  echo "ds4viz rlib not found under %s"
  echo "Build once: (cd /home/waterrun/Coding/ds4viz/library/rust && cargo build)"
  exit 2
fi

cat > __ds4viz_runner.rs <<RS
mod user {
    use ds4viz::prelude::*;
    include!(r#"%s"#);
}
fn main() { user::main(); }
RS

rustc --edition=2021 -C debuginfo=0 -C opt-level=0 -C codegen-units=256 \
  -o "%s" __ds4viz_runner.rs \
  -L dependency="$DS4VIZ_DEPS" \
  --extern ds4viz="$DS4VIZ_RLIB"
' 2>&1
]], work_dir, timeout_sec, deps, deps, deps, src, out)
            end,
            run_cmd = function(filepath, timeout_sec, work_dir)
                return string.format("cd '%s' && timeout %d '%s' 2>&1", work_dir, timeout_sec, filepath)
            end,
            extension = ".rs"
        }
    }
}

--------------------------------------------------------------------------------
-- 日志工具
--------------------------------------------------------------------------------

local function log_request(method, path, status, duration_ms, extra)
    local timestamp = os.date("%Y-%m-%d %H:%M:%S")
    local extra_str = extra and (" | " .. extra) or ""
    print(string.format("[%s] %s %s -> %d (%.2fms)%s",
        timestamp, method, path, status, duration_ms, extra_str))
end

--------------------------------------------------------------------------------
-- 工具函数
--------------------------------------------------------------------------------

local function uuid()
    local template = "xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx"
    return (template:gsub("[xy]", function(c)
        local v = (c == "x") and math.random(0, 15) or math.random(8, 11)
        return string.format("%x", v)
    end))
end

local function iso8601_now()
    return os.date("!%Y-%m-%dT%H:%M:%S") .. ".000Z"
end

local function get_time_ms()
    local ok, socket = pcall(require, "socket")
    if ok and socket.gettime then
        return socket.gettime() * 1000
    end
    return os.time() * 1000
end

local function read_file(path)
    local file = io.open(path, "r")
    if not file then
        return nil
    end
    local content = file:read("*a")
    file:close()
    return content
end

local function write_file(path, content)
    local file = io.open(path, "w")
    if not file then
        return false, "无法创建文件: " .. path
    end
    file:write(content)
    file:close()
    return true
end

local function file_exists(path)
    local file = io.open(path, "r")
    if file then
        file:close()
        return true
    end
    return false
end

local function remove_dir(path)
    os.execute(string.format("rm -rf '%s' 2>/dev/null", path))
end

local function ensure_dir(path)
    os.execute(string.format("mkdir -p '%s' 2>/dev/null", path))
end

local function exec_shell(cmd)
    local handle = io.popen(cmd, "r")
    if not handle then
        return nil, -1
    end
    local output = handle:read("*a")
    local success, exit_type, exit_code = handle:close()
    return output, exit_code or 0
end

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

local db

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
            stdout        TEXT NULL,
            http_error    TEXT NULL
        );
        CREATE INDEX IF NOT EXISTS idx_exec_runs_created_at ON exec_runs(created_at);
        CREATE INDEX IF NOT EXISTS idx_exec_runs_status     ON exec_runs(status);
        CREATE INDEX IF NOT EXISTS idx_exec_runs_request_id ON exec_runs(request_id);
    ]])
end

local function log_execution(params)
    local stmt = db:prepare([[
        INSERT INTO exec_runs (request_id, lang, lang_version, code, status, duration_ms, timeout_ms, client_ip, toml, stdout, http_error)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
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
            params.stdout,
            params.http_error
        )
        stmt:step()
        stmt:finalize()
    end
end

--------------------------------------------------------------------------------
-- 代码执行器
--------------------------------------------------------------------------------

local function execute_code(lang, code, timeout_ms)
    local lang_config = CONFIG.languages[lang]
    if not lang_config then
        return nil, nil, 0, "error", "不支持的语言: " .. lang
    end

    local start_time = get_time_ms()
    local timeout_sec = math.ceil(timeout_ms / 1000)

    local exec_id = uuid()
    local work_dir = CONFIG.temp_dir .. "/" .. exec_id
    ensure_dir(work_dir)

    local src_filename = "main" .. lang_config.extension
    local src_path = work_dir .. "/" .. src_filename
    local exe_path = work_dir .. "/main"
    local trace_path = work_dir .. "/" .. CONFIG.trace_filename

    local ok, err = write_file(src_path, code)
    if not ok then
        remove_dir(work_dir)
        return nil, nil, get_time_ms() - start_time, "error", err
    end

    local stdout, exit_code

    if lang_config.compile then
        local compile_cmd = lang_config.compile_cmd(src_path, exe_path, timeout_sec, work_dir)
        local compile_output, compile_exit = exec_shell(compile_cmd)

        if compile_exit ~= 0 then
            remove_dir(work_dir)
            local duration = get_time_ms() - start_time
            return nil, compile_output or "编译失败", duration, "error", "编译失败"
        end
    end

    local run_target = lang_config.compile and exe_path or src_path
    local run_cmd = lang_config.run_cmd(run_target, timeout_sec, work_dir)

    stdout, exit_code = exec_shell(run_cmd)

    local duration = get_time_ms() - start_time

    if exit_code == 124 then
        remove_dir(work_dir)
        return nil, stdout or "", duration, "error", "执行超时"
    end

    local toml_content = nil
    if file_exists(trace_path) then
        toml_content = read_file(trace_path)
    end

    remove_dir(work_dir)

    local status
    if exit_code ~= 0 then
        status = "error"
    elseif toml_content then
        status = "ok"
    else
        status = "error"
    end

    return toml_content, stdout or "", duration, status, nil
end

--------------------------------------------------------------------------------
-- 响应辅助函数
--------------------------------------------------------------------------------

local function add_cors_headers(resp)
    resp:addHeader("Access-Control-Allow-Origin", "*")
    resp:addHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS")
    resp:addHeader("Access-Control-Allow-Headers", "Content-Type")
end

local function json_response(resp, status, data)
    resp:statusCode(status)
    resp:addHeader("Content-Type", "application/json; charset=utf-8")
    add_cors_headers(resp)
    resp:write(cjson.encode(data))
end

--------------------------------------------------------------------------------
-- 读取 JSON 请求体
--------------------------------------------------------------------------------

local function read_json_body(req)
    local headers = req:headers()
    local content_length = tonumber(headers["content-length"]) or 0

    if content_length <= 0 then
        return nil, "请求体为空"
    end

    local body = req:receiveBody(content_length)

    if not body or body == "" then
        return nil, "请求体为空"
    end

    -- 去除首尾空白
    body = body:match("^%s*(.-)%s*$")
    if body == "" then
        return nil, "请求体为空"
    end

    local ok, data = pcall(cjson.decode, body)
    if not ok then
        return nil, "无效的 JSON: " .. tostring(data)
    end

    -- 确保解析结果是 table
    if type(data) ~= "table" then
        return nil, "请求体必须是 JSON 对象"
    end

    -- 检查是否为数组（数组的第一个键是数字）
    if data[1] ~= nil then
        return nil, "请求体必须是 JSON 对象，不能是数组"
    end

    return data, nil
end

--------------------------------------------------------------------------------
-- 请求处理
--------------------------------------------------------------------------------

local function handle_ping(req, resp)
    local start_time = get_time_ms()

    local response_data = {
        status = "ok",
        timestamp = iso8601_now(),
        supported_languages = get_supported_languages(),
        default_timeout_ms = CONFIG.default_timeout_ms
    }

    json_response(resp, 200, response_data)

    local duration = get_time_ms() - start_time
    log_request("GET", "/ping", 200, duration, "health check")
end

local function handle_options(req, resp)
    local start_time = get_time_ms()

    resp:statusCode(204)
    add_cors_headers(resp)
    resp:write("")

    local duration = get_time_ms() - start_time
    log_request("OPTIONS", req:path(), 204, duration, "preflight")
end

local function handle_execute(req, resp)
    local start_time = get_time_ms()
    local request_id = uuid()
    local headers = req:headers()
    local client_ip = headers["x-forwarded-for"] or req.ip or "unknown"

    local data, body_err = read_json_body(req)

    if not data then
        local duration = get_time_ms() - start_time
        log_execution({
            request_id = request_id,
            lang = "",
            lang_version = "",
            code = "",
            status = "error",
            duration_ms = duration,
            client_ip = client_ip,
            http_error = body_err
        })
        json_response(resp, 400, {
            error = "validation",
            message = body_err
        })
        log_request("POST", "/execute", 400, duration, body_err)
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
        json_response(resp, 400, {
            error = "validation",
            message = "缺少必填字段: lang"
        })
        log_request("POST", "/execute", 400, duration, "missing lang")
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
        json_response(resp, 400, {
            error = "validation",
            message = "缺少必填字段: code"
        })
        log_request("POST", "/execute", 400, duration, "missing code")
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
        json_response(resp, 400, {
            error = "validation",
            message = "不支持的语言: " .. data.lang
        })
        log_request("POST", "/execute", 400, duration, "unsupported lang: " .. data.lang)
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
        json_response(resp, 400, {
            error = "validation",
            message = string.format("timeout_ms 必须在 %d 到 %d 之间", CONFIG.min_timeout_ms, CONFIG.max_timeout_ms)
        })
        log_request("POST", "/execute", 400, duration, "invalid timeout")
        return
    end

    local lang_version = get_lang_version(lang_config)

    local toml_content, stdout, exec_duration, status, exec_error = execute_code(data.lang, data.code, timeout_ms)

    local total_duration = get_time_ms() - start_time

    if exec_error and not toml_content and not stdout then
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
        json_response(resp, 500, {
            error = "internal",
            message = exec_error
        })
        log_request("POST", "/execute", 500, total_duration, data.lang .. " | " .. exec_error)
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
        toml = toml_content,
        stdout = stdout
    })

    local response_data = {
        request_id = request_id,
        duration_ms = exec_duration,
        status = status
    }

    if toml_content then
        response_data.toml = toml_content
    end

    if stdout and stdout ~= "" then
        response_data.stdout = stdout
    end

    if status == "error" then
        if exec_error then
            response_data.error = exec_error
        elseif not toml_content then
            response_data.error = "未生成 trace.toml 文件"
        end
    end

    json_response(resp, 200, response_data)

    log_request("POST", "/execute", 200, total_duration,
        string.format("%s | %s | %.0fms exec | toml=%s",
            data.lang,
            status,
            exec_duration,
            toml_content and "yes" or "no"))
end

local function handle_not_found(req, resp)
    local start_time = get_time_ms()
    local method = req:method()
    local path = req:path()

    json_response(resp, 404, {
        error = "not_found",
        message = "接口不存在: " .. method .. " " .. path
    })

    local duration = get_time_ms() - start_time
    log_request(method, path, 404, duration, "not found")
end

--------------------------------------------------------------------------------
-- 主程序
--------------------------------------------------------------------------------

math.randomseed(os.time())

init_db()

ensure_dir(CONFIG.temp_dir)

print("================================================================================")
print("  ds4viz Demo Server")
print("================================================================================")
print(string.format("  监听地址: %s:%s", CONFIG.host, CONFIG.port))
print(string.format("  支持语言: %s", table.concat(get_supported_languages(), ", ")))
print(string.format("  默认超时: %dms", CONFIG.default_timeout_ms))
print(string.format("  临时目录: %s", CONFIG.temp_dir))
print(string.format("  数据库:   %s", CONFIG.db_path))
print(string.format("  trace文件: %s", CONFIG.trace_filename))
print("--------------------------------------------------------------------------------")
print("  按 Ctrl+C 停止服务")
print("================================================================================")
print("")

local server = Pegasus:new({
    host = CONFIG.host,
    port = CONFIG.port
})

server:start(function(req, resp)
    local method = req:method()
    local path = req:path()

    if method == "OPTIONS" then
        handle_options(req, resp)
        return
    end

    if path == "/ping" and method == "GET" then
        handle_ping(req, resp)
    elseif path == "/execute" and method == "POST" then
        handle_execute(req, resp)
    else
        handle_not_found(req, resp)
    end
end)
