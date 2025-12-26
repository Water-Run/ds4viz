#!/usr/bin/env lua
-- ds4viz Demo Server
-- Lua 5.4 + Pegasus + SQLite3
-- 职责：执行代码，返回标准输出（TOML IR 由各语言 ds4viz 库生成）

local pegasus = require("pegasus")
local sqlite3 = require("lsqlite3")
local cjson = require("cjson")

--------------------------------------------------------------------------------
-- 配置
--------------------------------------------------------------------------------

local CONFIG = {
    host = os.getenv("DS4VIZ_HOST") or "localhost",
    port = tonumber(os.getenv("DS4VIZ_PORT")) or 9090,
    default_timeout_ms = tonumber(os.getenv("DS4VIZ_TIMEOUT_MS")) or 5000,
    min_timeout_ms = 100,
    max_timeout_ms = 30000,
    db_path = "ds4viz.db",
    temp_dir = "/tmp/ds4viz",
    slib_dir = "./slib", -- ds4viz 库文件目录

    -- 支持的语言配置
    languages = {
        lua = {
            version_cmd = "lua -v 2>&1",
            -- 需要将 slib 目录加入 package.path
            run_cmd = function(filepath, slib_dir, timeout_sec)
                local lua_path = string.format("%s/?.lua;%s/?/init.lua", slib_dir, slib_dir)
                return string.format(
                    "timeout %d lua -e 'package.path=\"%s;\"..package.path' %s 2>&1",
                    timeout_sec, lua_path, filepath
                )
            end,
            extension = ".lua"
        },
        python = {
            version_cmd = "python3 --version 2>&1",
            run_cmd = function(filepath, slib_dir, timeout_sec)
                return string.format(
                    "timeout %d python3 -u %s 2>&1",
                    timeout_sec, filepath
                )
            end,
            extension = ".py",
            -- Python 需要设置 PYTHONPATH
            env_setup = function(slib_dir)
                return string.format("PYTHONPATH=%s:$PYTHONPATH ", slib_dir)
            end
        },
        javascript = {
            version_cmd = "node --version 2>&1",
            run_cmd = function(filepath, slib_dir, timeout_sec)
                return string.format(
                    "timeout %d node %s 2>&1",
                    timeout_sec, filepath
                )
            end,
            extension = ".js",
            env_setup = function(slib_dir)
                return string.format("NODE_PATH=%s:$NODE_PATH ", slib_dir)
            end
        },
        typescript = {
            version_cmd = "npx ts-node --version 2>&1",
            run_cmd = function(filepath, slib_dir, timeout_sec)
                return string.format(
                    "timeout %d npx ts-node --transpile-only %s 2>&1",
                    timeout_sec, filepath
                )
            end,
            extension = ".ts",
            env_setup = function(slib_dir)
                return string.format("NODE_PATH=%s:$NODE_PATH ", slib_dir)
            end
        },
        c = {
            version_cmd = "gcc --version 2>&1 | head -1",
            -- C 需要先编译再运行
            compile = true,
            compile_cmd = function(src, out, slib_dir, timeout_sec)
                return string.format(
                    "timeout %d gcc -o %s %s -I%s -L%s -lm 2>&1",
                    timeout_sec, out, src, slib_dir, slib_dir
                )
            end,
            run_cmd = function(filepath, slib_dir, timeout_sec)
                return string.format("timeout %d %s 2>&1", timeout_sec, filepath)
            end,
            extension = ".c"
        },
        rust = {
            version_cmd = "rustc --version 2>&1",
            -- Rust 需要先编译再运行
            compile = true,
            compile_cmd = function(src, out, slib_dir, timeout_sec)
                return string.format(
                    "timeout %d rustc -o %s %s 2>&1",
                    timeout_sec, out, src
                )
            end,
            run_cmd = function(filepath, slib_dir, timeout_sec)
                return string.format("timeout %d %s 2>&1", timeout_sec, filepath)
            end,
            extension = ".rs"
        },
        php = {
            version_cmd = "php --version 2>&1 | head -1",
            run_cmd = function(filepath, slib_dir, timeout_sec)
                return string.format(
                    "timeout %d php -d include_path=%s %s 2>&1",
                    timeout_sec, slib_dir, filepath
                )
            end,
            extension = ".php"
        }
    }
}

--------------------------------------------------------------------------------
-- 工具函数
--------------------------------------------------------------------------------

-- 生成 UUID
local function uuid()
    local template = "xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx"
    return (template:gsub("[xy]", function(c)
        local v = (c == "x") and math.random(0, 15) or math.random(8, 11)
        return string.format("%x", v)
    end))
end

-- 获取 ISO 8601 格式时间戳
local function iso8601_now()
    return os.date("!%Y-%m-%dT%H:%M:%S") .. ".000Z"
end

-- 获取毫秒级时间戳
local function get_time_ms()
    local ok, socket = pcall(require, "socket")
    if ok and socket.gettime then
        return math.floor(socket.gettime() * 1000)
    end
    return os.time() * 1000
end

-- JSON 响应
local function json_response(res, status, data)
    res:statusCode(status)
    res:addHeader("Content-Type", "application/json; charset=utf-8")
    res:addHeader("Access-Control-Allow-Origin", "*")
    res:addHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS")
    res:addHeader("Access-Control-Allow-Headers", "Content-Type")
    res:write(cjson.encode(data))
end

-- 读取请求体
local function read_body(req)
    local body = req:post()
    if type(body) == "string" then
        return body
    end
    return nil
end

-- 写入文件
local function write_file(path, content)
    local file = io.open(path, "w")
    if not file then
        return false, "无法创建文件: " .. path
    end
    file:write(content)
    file:close()
    return true
end

-- 删除文件（静默失败）
local function remove_file(path)
    os.remove(path)
end

-- 确保目录存在
local function ensure_dir(path)
    os.execute(string.format("mkdir -p '%s' 2>/dev/null", path))
end

-- 执行 shell 命令并获取输出
local function exec_shell(cmd)
    local handle = io.popen(cmd, "r")
    if not handle then
        return nil, "无法执行命令"
    end
    local output = handle:read("*a")
    local success, exit_type, exit_code = handle:close()
    return output, exit_code
end

-- 获取语言版本
local function get_lang_version(lang_config)
    local output, _ = exec_shell(lang_config.version_cmd)
    if output then
        -- 取第一行，去除首尾空白
        local first_line = output:match("^([^\n]*)")
        if first_line then
            return first_line:match("^%s*(.-)%s*$")
        end
    end
    return "unknown"
end

-- 获取支持的语言列表
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
            http_error    TEXT NULL
        );
        CREATE INDEX IF NOT EXISTS idx_exec_runs_created_at ON exec_runs(created_at);
        CREATE INDEX IF NOT EXISTS idx_exec_runs_status     ON exec_runs(status);
        CREATE INDEX IF NOT EXISTS idx_exec_runs_request_id ON exec_runs(request_id);
    ]])
end

-- 记录执行日志
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

-- 执行代码，返回 (output, duration_ms, status, error_msg)
local function execute_code(lang, code, timeout_ms)
    local lang_config = CONFIG.languages[lang]
    if not lang_config then
        return nil, 0, "error", "不支持的语言: " .. lang
    end

    local start_time = get_time_ms()
    local timeout_sec = math.ceil(timeout_ms / 1000)

    -- 确保临时目录存在
    ensure_dir(CONFIG.temp_dir)

    -- 生成唯一文件名
    local file_id = uuid()
    local src_path = CONFIG.temp_dir .. "/" .. file_id .. lang_config.extension
    local exe_path = CONFIG.temp_dir .. "/" .. file_id

    -- 写入源代码文件
    local ok, err = write_file(src_path, code)
    if not ok then
        return nil, get_time_ms() - start_time, "error", err
    end

    local output, exit_code
    local cleanup_files = { src_path }

    -- 如果是编译型语言，先编译
    if lang_config.compile then
        local compile_cmd = lang_config.compile_cmd(src_path, exe_path, CONFIG.slib_dir, timeout_sec)
        local compile_output, compile_exit = exec_shell(compile_cmd)

        if compile_exit ~= 0 then
            -- 编译失败
            for _, f in ipairs(cleanup_files) do remove_file(f) end
            local duration = get_time_ms() - start_time
            -- 返回编译错误信息作为输出
            return compile_output or "编译失败", duration, "error", nil
        end

        table.insert(cleanup_files, exe_path)
    end

    -- 构建运行命令
    local run_target = lang_config.compile and exe_path or src_path
    local run_cmd = lang_config.run_cmd(run_target, CONFIG.slib_dir, timeout_sec)

    -- 添加环境变量
    if lang_config.env_setup then
        run_cmd = lang_config.env_setup(CONFIG.slib_dir) .. run_cmd
    end

    -- 执行代码
    output, exit_code = exec_shell(run_cmd)

    -- 清理临时文件
    for _, f in ipairs(cleanup_files) do
        remove_file(f)
    end

    local duration = get_time_ms() - start_time

    -- 判断执行状态
    -- exit_code 124 是 timeout 命令的超时退出码
    if exit_code == 124 then
        return output or "", duration, "error", "执行超时"
    end

    -- 即使 exit_code 非零，也返回输出（可能包含运行时错误信息）
    -- 由 ds4viz 库生成的 TOML 中会包含错误信息
    local status = (exit_code == 0) and "ok" or "error"
    return output or "", duration, status, nil
end

--------------------------------------------------------------------------------
-- 请求处理器
--------------------------------------------------------------------------------

-- GET /ping - 健康检查
local function handle_ping(req, res)
    json_response(res, 200, {
        status = "ok",
        timestamp = iso8601_now(),
        supported_languages = get_supported_languages(),
        default_timeout_ms = CONFIG.default_timeout_ms
    })
end

-- POST /execute - 执行代码
local function handle_execute(req, res)
    local request_id = uuid()
    local start_time = get_time_ms()
    local client_ip = req:headers()["X-Forwarded-For"] or req:headers()["x-forwarded-for"] or "unknown"

    -- 读取请求体
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

    -- 解析 JSON
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

    -- 验证必填字段: lang
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

    -- 验证必填字段: code
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

    -- 验证语言是否支持
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

    -- 验证超时参数
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

    -- 获取语言版本
    local lang_version = get_lang_version(lang_config)

    -- 执行代码
    local output, exec_duration, status, exec_error = execute_code(data.lang, data.code, timeout_ms)

    local total_duration = get_time_ms() - start_time

    -- 如果执行过程本身出现错误（非代码运行时错误）
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

    -- 记录执行日志
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

    -- 返回结果
    -- output 就是代码的标准输出，应该是 ds4viz 库生成的 TOML IR
    json_response(res, 200, {
        request_id = request_id,
        duration_ms = exec_duration,
        toml = output
    })
end

-- OPTIONS - CORS 预检
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

local function router(req, res)
    local method = req:method()
    local path = req:path()

    -- CORS 预检请求
    if method == "OPTIONS" then
        handle_options(req, res)
        return
    end

    -- 路由匹配
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

-- 初始化随机数种子
math.randomseed(os.time())

-- 初始化数据库
init_db()

-- 确保临时目录存在
ensure_dir(CONFIG.temp_dir)

-- 打印启动信息
print(string.format("[ds4viz] Demo Server 启动于 %s:%d", CONFIG.host, CONFIG.port))
print("[ds4viz] 支持语言: " .. table.concat(get_supported_languages(), ", "))
print("[ds4viz] 默认超时: " .. CONFIG.default_timeout_ms .. "ms")
print("[ds4viz] 临时目录: " .. CONFIG.temp_dir)
print("[ds4viz] 库文件目录: " .. CONFIG.slib_dir)
print("[ds4viz] 按 Ctrl+C 停止服务")

-- 启动服务器
local server = pegasus:new({
    host = CONFIG.host,
    port = CONFIG.port
})

server:start(router)
