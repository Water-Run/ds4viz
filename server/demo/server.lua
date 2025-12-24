#!/usr/bin/env lua
-- ds4viz Demo Server
-- Lua 5.4 + Pegasus + SQLite3

local pegasus = require("pegasus")
local sqlite3 = require("lsqlite3")
local cjson = require("cjson")

--------------------------------------------------------------------------------
-- Configuration
--------------------------------------------------------------------------------

local CONFIG = {
    host = os.getenv("DS4VIZ_HOST") or "localhost",
    port = tonumber(os.getenv("DS4VIZ_PORT")) or 9090,
    default_timeout_ms = tonumber(os.getenv("DS4VIZ_TIMEOUT_MS")) or 5000,
    min_timeout_ms = 100,
    max_timeout_ms = 30000,
    db_path = "ds4viz.db",
    supported_languages = { lua = true, python = true }
}

--------------------------------------------------------------------------------
-- Utilities
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
        return math.floor(socket.gettime() * 1000)
    end
    return os.time() * 1000
end

local function json_response(res, status, data)
    res:statusCode(status)
    res:addHeader("Content-Type", "application/json")
    res:addHeader("Access-Control-Allow-Origin", "*")
    res:addHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS")
    res:addHeader("Access-Control-Allow-Headers", "Content-Type")
    res:write(cjson.encode(data))
end

local function read_body(req)
    local body = req:post()
    if type(body) == "string" then
        return body
    end
    return nil
end

--------------------------------------------------------------------------------
-- Database
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

local function log_execution(params)
    local stmt = db:prepare([[
        INSERT INTO exec_runs (request_id, lang, lang_version, code, status, duration_ms, timeout_ms, client_ip, toml, http_error)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
    ]])
    stmt:bind_values(
        params.request_id,
        params.lang,
        params.lang_version or "",
        params.code,
        params.status,
        params.duration_ms,
        params.timeout_ms,
        params.client_ip,
        params.toml,
        params.http_error
    )
    stmt:step()
    stmt:finalize()
end

--------------------------------------------------------------------------------
-- TOML IR Generator
--------------------------------------------------------------------------------

local function escape_toml_string(s)
    if type(s) ~= "string" then return tostring(s) end
    return s:gsub("\\", "\\\\"):gsub('"', '\\"'):gsub("\n", "\\n"):gsub("\r", "\\r"):gsub("\t", "\\t")
end

local function format_toml_value(v)
    local t = type(v)
    if t == "string" then
        return '"' .. escape_toml_string(v) .. '"'
    elseif t == "number" then
        if v == math.floor(v) then
            return tostring(math.floor(v))
        else
            return tostring(v)
        end
    elseif t == "boolean" then
        return v and "true" or "false"
    elseif t == "table" then
        -- inline table for arrays
        if #v > 0 then
            local items = {}
            for _, item in ipairs(v) do
                table.insert(items, format_toml_value(item))
            end
            return "[" .. table.concat(items, ", ") .. "]"
        else
            -- inline table for objects
            local pairs_list = {}
            for k, val in pairs(v) do
                table.insert(pairs_list, k .. " = " .. format_toml_value(val))
            end
            return "{ " .. table.concat(pairs_list, ", ") .. " }"
        end
    end
    return '""'
end

local function generate_toml_ir(trace, meta_info)
    local lines = {}

    -- [meta]
    table.insert(lines, "[meta]")
    table.insert(lines, 'created_at = "' .. meta_info.created_at .. '"')
    table.insert(lines, 'lang = "' .. meta_info.lang .. '"')
    table.insert(lines, 'lang_version = "' .. meta_info.lang_version .. '"')
    table.insert(lines, "")

    -- [package]
    table.insert(lines, "[package]")
    table.insert(lines, 'name = "ds4viz"')
    table.insert(lines, 'lang = "' .. meta_info.lang .. '"')
    table.insert(lines, 'version = "1.0.0"')
    table.insert(lines, "")

    -- [object]
    table.insert(lines, "[object]")
    table.insert(lines, 'kind = "' .. trace.kind .. '"')
    if trace.label then
        table.insert(lines, 'label = "' .. escape_toml_string(trace.label) .. '"')
    end
    table.insert(lines, "")

    -- [[states]]
    for _, state in ipairs(trace.states) do
        table.insert(lines, "[[states]]")
        table.insert(lines, "id = " .. state.id)
        table.insert(lines, "")
        table.insert(lines, "[states.data]")
        for k, v in pairs(state.data) do
            table.insert(lines, k .. " = " .. format_toml_value(v))
        end
        table.insert(lines, "")
    end

    -- [[steps]]
    for _, step in ipairs(trace.steps) do
        table.insert(lines, "[[steps]]")
        table.insert(lines, "id = " .. step.id)
        table.insert(lines, 'op = "' .. step.op .. '"')
        table.insert(lines, "before = " .. step.before)
        if step.after then
            table.insert(lines, "after = " .. step.after)
        end
        if step.ret ~= nil then
            table.insert(lines, "ret = " .. format_toml_value(step.ret))
        end
        table.insert(lines, "")
        table.insert(lines, "[steps.args]")
        if step.args then
            for k, v in pairs(step.args) do
                table.insert(lines, k .. " = " .. format_toml_value(v))
            end
        end
        table.insert(lines, "")
        if step.code_line then
            table.insert(lines, "[steps.code]")
            table.insert(lines, "line = " .. step.code_line)
            table.insert(lines, "")
        end
    end

    -- [result] or [error]
    if trace.error then
        table.insert(lines, "[error]")
        table.insert(lines, 'type = "' .. trace.error.type .. '"')
        table.insert(lines, 'message = "' .. escape_toml_string(trace.error.message) .. '"')
        if trace.error.line then
            table.insert(lines, "line = " .. trace.error.line)
        end
        if trace.error.step then
            table.insert(lines, "step = " .. trace.error.step)
        end
        if trace.error.last_state then
            table.insert(lines, "last_state = " .. trace.error.last_state)
        end
    else
        table.insert(lines, "[result]")
        table.insert(lines, "final_state = " .. trace.final_state)
    end

    return table.concat(lines, "\n")
end

--------------------------------------------------------------------------------
-- ds4viz Library (Lua implementation)
--------------------------------------------------------------------------------

local ds4viz = {}

-- Trace context for recording operations
local function create_trace_context(kind, label)
    return {
        kind = kind,
        label = label or kind,
        states = {},
        steps = {},
        current_state_id = -1,
        error = nil,
        final_state = nil
    }
end

local function add_state(ctx, data)
    ctx.current_state_id = ctx.current_state_id + 1
    table.insert(ctx.states, {
        id = ctx.current_state_id,
        data = data
    })
    return ctx.current_state_id
end

local function add_step(ctx, op, before, after, args, code_line, ret)
    table.insert(ctx.steps, {
        id = #ctx.steps,
        op = op,
        before = before,
        after = after,
        args = args,
        code_line = code_line,
        ret = ret
    })
end

local function set_error(ctx, err_type, message, line, step_id, last_state)
    ctx.error = {
        type = err_type,
        message = message,
        line = line,
        step = step_id,
        last_state = last_state
    }
end

-- Stack implementation
function ds4viz.stack(label)
    local ctx = create_trace_context("stack", label)
    local items = {}

    -- Initial empty state
    add_state(ctx, { items = {}, top = -1 })

    local stack = {}
    stack._ctx = ctx

    function stack:push(value, code_line)
        local before = ctx.current_state_id
        table.insert(items, value)
        local items_copy = {}
        for i, v in ipairs(items) do items_copy[i] = v end
        local after = add_state(ctx, { items = items_copy, top = #items - 1 })
        add_step(ctx, "push", before, after, { value = value }, code_line)
    end

    function stack:pop(code_line)
        local before = ctx.current_state_id
        if #items == 0 then
            set_error(ctx, "runtime", "Stack underflow: cannot pop from empty stack", code_line, #ctx.steps, before)
            add_step(ctx, "pop", before, nil, {}, code_line)
            return nil
        end
        local value = table.remove(items)
        local items_copy = {}
        for i, v in ipairs(items) do items_copy[i] = v end
        local after = add_state(ctx, { items = items_copy, top = #items > 0 and #items - 1 or -1 })
        add_step(ctx, "pop", before, after, {}, code_line, value)
        return value
    end

    function stack:peek(code_line)
        if #items == 0 then
            return nil
        end
        return items[#items]
    end

    function stack:commit()
        if not ctx.error then
            ctx.final_state = ctx.current_state_id
        end
    end

    return stack
end

-- Queue implementation
function ds4viz.queue(label)
    local ctx = create_trace_context("queue", label)
    local items = {}

    -- Initial empty state
    add_state(ctx, { items = {}, front = -1, rear = -1 })

    local queue = {}
    queue._ctx = ctx

    function queue:enqueue(value, code_line)
        local before = ctx.current_state_id
        table.insert(items, value)
        local items_copy = {}
        for i, v in ipairs(items) do items_copy[i] = v end
        local after = add_state(ctx, { items = items_copy, front = 0, rear = #items - 1 })
        add_step(ctx, "enqueue", before, after, { value = value }, code_line)
    end

    function queue:dequeue(code_line)
        local before = ctx.current_state_id
        if #items == 0 then
            set_error(ctx, "runtime", "Queue underflow: cannot dequeue from empty queue", code_line, #ctx.steps, before)
            add_step(ctx, "dequeue", before, nil, {}, code_line)
            return nil
        end
        local value = table.remove(items, 1)
        local items_copy = {}
        for i, v in ipairs(items) do items_copy[i] = v end
        local front = #items > 0 and 0 or -1
        local rear = #items > 0 and #items - 1 or -1
        local after = add_state(ctx, { items = items_copy, front = front, rear = rear })
        add_step(ctx, "dequeue", before, after, {}, code_line, value)
        return value
    end

    function queue:commit()
        if not ctx.error then
            ctx.final_state = ctx.current_state_id
        end
    end

    return queue
end

-- Binary Tree implementation
function ds4viz.binary_tree(label)
    local ctx = create_trace_context("binary_tree", label)
    local nodes = {}
    local next_node_id = 0
    local root_id = -1

    -- Initial empty state
    add_state(ctx, { root = -1, nodes = {} })

    local tree = {}
    tree._ctx = ctx

    local function get_nodes_snapshot()
        local snapshot = {}
        for _, node in pairs(nodes) do
            table.insert(snapshot, {
                id = node.id,
                value = node.value,
                left = node.left,
                right = node.right
            })
        end
        table.sort(snapshot, function(a, b) return a.id < b.id end)
        return snapshot
    end

    local function find_node_by_value(value)
        for _, node in pairs(nodes) do
            if node.value == value then
                return node
            end
        end
        return nil
    end

    function tree:insert_root(value, code_line)
        local before = ctx.current_state_id
        if root_id ~= -1 then
            set_error(ctx, "runtime", "Root already exists", code_line, #ctx.steps, before)
            add_step(ctx, "insert_root", before, nil, { value = value }, code_line)
            return
        end
        local node_id = next_node_id
        next_node_id = next_node_id + 1
        nodes[node_id] = { id = node_id, value = value, left = -1, right = -1 }
        root_id = node_id
        local after = add_state(ctx, { root = root_id, nodes = get_nodes_snapshot() })
        add_step(ctx, "insert_root", before, after, { value = value }, code_line)
    end

    function tree:insert_left(parent_value, value, code_line)
        local before = ctx.current_state_id
        local parent = find_node_by_value(parent_value)
        if not parent then
            set_error(ctx, "runtime", "Parent node not found: " .. tostring(parent_value), code_line, #ctx.steps, before)
            add_step(ctx, "insert_left", before, nil, { parent = parent_value, value = value }, code_line)
            return
        end
        if parent.left ~= -1 then
            set_error(ctx, "runtime", "Left child already exists for node: " .. tostring(parent_value), code_line,
                #ctx.steps, before)
            add_step(ctx, "insert_left", before, nil, { parent = parent_value, value = value }, code_line)
            return
        end
        local node_id = next_node_id
        next_node_id = next_node_id + 1
        nodes[node_id] = { id = node_id, value = value, left = -1, right = -1 }
        parent.left = node_id
        local after = add_state(ctx, { root = root_id, nodes = get_nodes_snapshot() })
        add_step(ctx, "insert_left", before, after, { parent = parent_value, value = value }, code_line)
    end

    function tree:insert_right(parent_value, value, code_line)
        local before = ctx.current_state_id
        local parent = find_node_by_value(parent_value)
        if not parent then
            set_error(ctx, "runtime", "Parent node not found: " .. tostring(parent_value), code_line, #ctx.steps, before)
            add_step(ctx, "insert_right", before, nil, { parent = parent_value, value = value }, code_line)
            return
        end
        if parent.right ~= -1 then
            set_error(ctx, "runtime", "Right child already exists for node: " .. tostring(parent_value), code_line,
                #ctx.steps, before)
            add_step(ctx, "insert_right", before, nil, { parent = parent_value, value = value }, code_line)
            return
        end
        local node_id = next_node_id
        next_node_id = next_node_id + 1
        nodes[node_id] = { id = node_id, value = value, left = -1, right = -1 }
        parent.right = node_id
        local after = add_state(ctx, { root = root_id, nodes = get_nodes_snapshot() })
        add_step(ctx, "insert_right", before, after, { parent = parent_value, value = value }, code_line)
    end

    function tree:commit()
        if not ctx.error then
            ctx.final_state = ctx.current_state_id
        end
    end

    return tree
end

--------------------------------------------------------------------------------
-- Code Executor
--------------------------------------------------------------------------------

local function get_lua_version()
    return _VERSION:match("Lua (.+)") or "5.4"
end

local function execute_lua_code(code, timeout_ms)
    local start_time = get_time_ms()

    -- Create sandbox environment
    local sandbox_env = {
        ds4viz = ds4viz,
        print = function() end, -- suppress output
        tostring = tostring,
        tonumber = tonumber,
        type = type,
        pairs = pairs,
        ipairs = ipairs,
        math = math,
        string = string,
        table = table
    }

    -- Compile and execute
    local fn, compile_err = load(code, "user_code", "t", sandbox_env)
    if not fn then
        -- Compilation error: generate error TOML
        local trace = create_trace_context("stack", "error")
        add_state(trace, { items = {}, top = -1 })
        set_error(trace, "runtime", "Compilation error: " .. tostring(compile_err), 1, nil, 0)

        local duration = get_time_ms() - start_time
        local toml = generate_toml_ir(trace, {
            created_at = iso8601_now(),
            lang = "lua",
            lang_version = get_lua_version()
        })
        return toml, duration, "error"
    end

    -- Execute with pcall
    local ok, result = pcall(fn)
    local duration = get_time_ms() - start_time

    if not ok then
        -- Runtime error during execution
        local trace = create_trace_context("stack", "error")
        add_state(trace, { items = {}, top = -1 })
        set_error(trace, "runtime", tostring(result), nil, nil, 0)

        local toml = generate_toml_ir(trace, {
            created_at = iso8601_now(),
            lang = "lua",
            lang_version = get_lua_version()
        })
        return toml, duration, "error"
    end

    -- Check if result is a ds4viz object with context
    if type(result) == "table" and result._ctx then
        result:commit()
        local toml = generate_toml_ir(result._ctx, {
            created_at = iso8601_now(),
            lang = "lua",
            lang_version = get_lua_version()
        })
        local status = result._ctx.error and "error" or "ok"
        return toml, duration, status
    end

    -- No ds4viz object returned: create minimal success trace
    local trace = create_trace_context("stack", "default")
    add_state(trace, { items = {}, top = -1 })
    trace.final_state = 0

    local toml = generate_toml_ir(trace, {
        created_at = iso8601_now(),
        lang = "lua",
        lang_version = get_lua_version()
    })
    return toml, duration, "ok"
end

--------------------------------------------------------------------------------
-- Request Handlers
--------------------------------------------------------------------------------

local function handle_ping(req, res)
    json_response(res, 200, {
        status = "ok",
        timestamp = iso8601_now(),
        supported_languages = { "lua", "python" },
        default_timeout_ms = CONFIG.default_timeout_ms
    })
end

local function handle_execute(req, res)
    local request_id = uuid()
    local start_time = get_time_ms()
    local client_ip = req:headers()["X-Forwarded-For"] or "unknown"

    -- Read and parse body
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
            timeout_ms = nil,
            client_ip = client_ip,
            toml = nil,
            http_error = "Empty request body"
        })
        json_response(res, 400, {
            error = "validation",
            message = "Empty request body"
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
            timeout_ms = nil,
            client_ip = client_ip,
            toml = nil,
            http_error = "Invalid JSON"
        })
        json_response(res, 400, {
            error = "validation",
            message = "Invalid JSON"
        })
        return
    end

    -- Validate required fields
    if not data.lang then
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
            toml = nil,
            http_error = "Missing required field: lang"
        })
        json_response(res, 400, {
            error = "validation",
            message = "Missing required field: lang"
        })
        return
    end

    if not data.code then
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
            toml = nil,
            http_error = "Missing required field: code"
        })
        json_response(res, 400, {
            error = "validation",
            message = "Missing required field: code"
        })
        return
    end

    -- Validate language
    if not CONFIG.supported_languages[data.lang] then
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
            toml = nil,
            http_error = "Unsupported language: " .. tostring(data.lang)
        })
        json_response(res, 400, {
            error = "validation",
            message = "Unsupported language: " .. tostring(data.lang)
        })
        return
    end

    -- Validate timeout
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
            toml = nil,
            http_error = "timeout_ms must be between " .. CONFIG.min_timeout_ms .. " and " .. CONFIG.max_timeout_ms
        })
        json_response(res, 400, {
            error = "validation",
            message = "timeout_ms must be between " .. CONFIG.min_timeout_ms .. " and " .. CONFIG.max_timeout_ms
        })
        return
    end

    -- Execute code
    local toml, exec_duration, status

    if data.lang == "lua" then
        toml, exec_duration, status = execute_lua_code(data.code, timeout_ms)
    elseif data.lang == "python" then
        -- Python execution placeholder
        local trace = create_trace_context("stack", "python_placeholder")
        add_state(trace, { items = {}, top = -1 })
        set_error(trace, "runtime", "Python execution not yet implemented", nil, nil, 0)
        exec_duration = get_time_ms() - start_time
        toml = generate_toml_ir(trace, {
            created_at = iso8601_now(),
            lang = "python",
            lang_version = "3.12.0"
        })
        status = "error"
    end

    local total_duration = get_time_ms() - start_time

    -- Log execution
    log_execution({
        request_id = request_id,
        lang = data.lang,
        lang_version = data.lang == "lua" and get_lua_version() or "3.12.0",
        code = data.code,
        status = status,
        duration_ms = total_duration,
        timeout_ms = timeout_ms,
        client_ip = client_ip,
        toml = toml,
        http_error = nil
    })

    -- Return response
    json_response(res, 200, {
        request_id = request_id,
        duration_ms = exec_duration,
        toml = toml
    })
end

local function handle_options(req, res)
    res:statusCode(204)
    res:addHeader("Access-Control-Allow-Origin", "*")
    res:addHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS")
    res:addHeader("Access-Control-Allow-Headers", "Content-Type")
    res:write("")
end

--------------------------------------------------------------------------------
-- Router
--------------------------------------------------------------------------------

local function router(req, res)
    local method = req:method()
    local path = req:path()

    -- Handle CORS preflight
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
            message = "Endpoint not found: " .. method .. " " .. path
        })
    end
end

--------------------------------------------------------------------------------
-- Main
--------------------------------------------------------------------------------

math.randomseed(os.time())
init_db()

print(string.format("ds4viz Demo Server starting on %s:%d", CONFIG.host, CONFIG.port))
print("Supported languages: lua, python")
print("Press Ctrl+C to stop")

local server = pegasus:new({
    host = CONFIG.host,
    port = CONFIG.port
})

server:start(router)
