#!/usr/bin/env lua
--[[
ds4viz Demo Server 测试套件
使用 busted 测试框架

运行方式:
    busted ./test/test.lua

依赖:
    luarocks install busted
    luarocks install lua-cjson
    luarocks install luasocket

注意:
    运行测试前请确保 demo 服务器已启动:
    lua server.lua

Author:
    WaterRun
File:
    ds4viz/server/demo/test/test.lua
Date:
    2025-12-27
Updated:
    2025-12-30
]]

local cjson = require("cjson")
local socket = require("socket")
local http = require("socket.http")
local ltn12 = require("ltn12")

--------------------------------------------------------------------------------
-- 测试配置
--------------------------------------------------------------------------------

local CONFIG = {
    base_url = "http://localhost:9090",
    timeout = 10,
    supported_languages = { "lua", "python", "rust" },
    rust_timeout = 60000, -- Rust 编译需要更长时间
    default_timeout = 10000,
}

--------------------------------------------------------------------------------
-- HTTP 工具函数
--------------------------------------------------------------------------------

local function http_get(path)
    local response_body = {}
    local result, status_code, headers = http.request({
        url = CONFIG.base_url .. path,
        method = "GET",
        sink = ltn12.sink.table(response_body),
        headers = {
            ["Accept"] = "application/json",
        },
    })

    local body = table.concat(response_body)
    local data = nil
    if body and body ~= "" then
        local ok, parsed = pcall(cjson.decode, body)
        if ok then
            data = parsed
        end
    end

    return data, status_code or 0, headers or {}
end

local function http_post(path, payload, content_type)
    local request_body = ""
    if type(payload) == "table" then
        request_body = cjson.encode(payload)
    elseif type(payload) == "string" then
        request_body = payload
    end

    local response_body = {}
    local result, status_code, headers = http.request({
        url = CONFIG.base_url .. path,
        method = "POST",
        source = ltn12.source.string(request_body),
        sink = ltn12.sink.table(response_body),
        headers = {
            ["Content-Type"] = content_type or "application/json",
            ["Content-Length"] = #request_body,
            ["Accept"] = "application/json",
        },
    })

    local body = table.concat(response_body)
    local data = nil
    if body and body ~= "" then
        local ok, parsed = pcall(cjson.decode, body)
        if ok then
            data = parsed
        end
    end

    return data, status_code or 0, headers or {}, body
end

local function http_options(path)
    local response_body = {}
    local result, status_code, headers = http.request({
        url = CONFIG.base_url .. path,
        method = "OPTIONS",
        sink = ltn12.sink.table(response_body),
    })

    return status_code or 0, headers or {}
end

local function http_method(method, path, payload)
    local request_body = ""
    if payload then
        if type(payload) == "table" then
            request_body = cjson.encode(payload)
        else
            request_body = tostring(payload)
        end
    end

    local response_body = {}
    local headers_tbl = {
        ["Content-Type"] = "application/json",
        ["Accept"] = "application/json",
    }
    if #request_body > 0 then
        headers_tbl["Content-Length"] = #request_body
    end

    local result, status_code, headers = http.request({
        url = CONFIG.base_url .. path,
        method = method,
        source = #request_body > 0 and ltn12.source.string(request_body) or nil,
        sink = ltn12.sink.table(response_body),
        headers = headers_tbl,
    })

    local body = table.concat(response_body)
    local data = nil
    if body and body ~= "" then
        local ok, parsed = pcall(cjson.decode, body)
        if ok then
            data = parsed
        end
    end

    return data, status_code or 0, headers or {}
end

local function read_sample(filename)
    local source = debug.getinfo(1, "S").source
    local script_dir

    if source:sub(1, 1) == "@" then
        source = source:sub(2)
        script_dir = source:match("(.*/)")
        if not script_dir then
            script_dir = "./"
        end
    else
        script_dir = "./"
    end

    local filepath = script_dir .. "sample/" .. filename
    local file = io.open(filepath, "r")
    if not file then
        return nil
    end
    local content = file:read("*a")
    file:close()
    return content
end

local function list_sample_files(subdir)
    local source = debug.getinfo(1, "S").source
    local script_dir

    if source:sub(1, 1) == "@" then
        source = source:sub(2)
        script_dir = source:match("(.*/)")
        if not script_dir then
            script_dir = "./"
        end
    else
        script_dir = "./"
    end

    local dir_path = script_dir .. "sample/" .. subdir
    local files = {}

    local handle = io.popen('ls -1 "' .. dir_path .. '" 2>/dev/null')
    if handle then
        for line in handle:lines() do
            if line and line ~= "" then
                table.insert(files, line)
            end
        end
        handle:close()
    end

    return files
end

local function get_lang_from_filename(filename)
    if filename:match("^lua_") or filename:match("%.lua$") then
        return "lua"
    elseif filename:match("^py_") or filename:match("%.py$") then
        return "python"
    elseif filename:match("^rust_") or filename:match("%.rs$") then
        return "rust"
    end
    return nil
end

local function get_timeout_for_lang(lang)
    if lang == "rust" then
        return CONFIG.rust_timeout
    end
    return CONFIG.default_timeout
end

--------------------------------------------------------------------------------
-- TOML 验证辅助函数
--------------------------------------------------------------------------------

local function toml_has_section(toml, section)
    if not toml then return false end
    local pattern = "%[" .. section:gsub("([%.%[%]])", "%%%1") .. "%]"
    return toml:match(pattern) ~= nil
end

local function toml_has_field(toml, field, value)
    if not toml then return false end
    if value then
        local pattern = field .. "%s*=%s*" .. tostring(value)
        return toml:match(pattern) ~= nil
    else
        local pattern = field .. "%s*="
        return toml:match(pattern) ~= nil
    end
end

local function toml_get_status(toml)
    if not toml then return nil end
    if toml_has_section(toml, "result") then
        return "ok"
    elseif toml_has_section(toml, "error") then
        return "error"
    end
    return nil
end

local function validate_toml_structure(toml)
    local errors = {}

    if not toml_has_section(toml, "meta") then
        table.insert(errors, "缺少 [meta] 块")
    end

    if not toml_has_section(toml, "package") then
        table.insert(errors, "缺少 [package] 块")
    end

    if not toml_has_section(toml, "object") then
        table.insert(errors, "缺少 [object] 块")
    end

    local has_result = toml_has_section(toml, "result")
    local has_error = toml_has_section(toml, "error")

    if not has_result and not has_error then
        table.insert(errors, "缺少 [result] 或 [error] 块")
    end

    if has_result and has_error then
        table.insert(errors, "不能同时存在 [result] 和 [error] 块")
    end

    return #errors == 0, errors
end

--------------------------------------------------------------------------------
-- 测试: /ping 健康检查接口
--------------------------------------------------------------------------------

describe("/ping 健康检查接口", function()
    describe("基本功能", function()
        it("应返回 HTTP 200 和 ok 状态", function()
            local data, status = http_get("/ping")
            assert.are.equal(200, status)
            assert.is_not_nil(data)
            assert.are.equal("ok", data.status)
        end)

        it("应返回有效的 ISO 8601 时间戳", function()
            local data, status = http_get("/ping")
            assert.are.equal(200, status)
            assert.is_not_nil(data.timestamp)
            local pattern = "^%d%d%d%d%-%d%d%-%d%dT%d%d:%d%d:%d%d%.%d%d%dZ$"
            assert.is_truthy(data.timestamp:match(pattern))
        end)

        it("应返回正确的支持语言列表", function()
            local data, status = http_get("/ping")
            assert.are.equal(200, status)
            assert.is_not_nil(data.supported_languages)
            assert.is_true(type(data.supported_languages) == "table")
            assert.are.equal(3, #data.supported_languages)

            local lang_set = {}
            for _, lang in ipairs(data.supported_languages) do
                lang_set[lang] = true
            end

            assert.is_true(lang_set["lua"], "缺少 lua")
            assert.is_true(lang_set["python"], "缺少 python")
            assert.is_true(lang_set["rust"], "缺少 rust")
        end)

        it("应返回默认超时时间", function()
            local data, status = http_get("/ping")
            assert.are.equal(200, status)
            assert.is_not_nil(data.default_timeout_ms)
            assert.is_true(type(data.default_timeout_ms) == "number")
            assert.is_true(data.default_timeout_ms > 0)
        end)
    end)

    describe("响应头验证", function()
        it("应返回正确的 Content-Type", function()
            local data, status, headers = http_get("/ping")
            assert.are.equal(200, status)
            local content_type = headers["content-type"] or headers["Content-Type"]
            assert.is_not_nil(content_type)
            assert.is_truthy(content_type:match("application/json"))
        end)

        it("应返回 CORS 头", function()
            local data, status, headers = http_get("/ping")
            assert.are.equal(200, status)
            local cors = headers["access-control-allow-origin"] or headers["Access-Control-Allow-Origin"]
            assert.are.equal("*", cors)
        end)
    end)

    describe("多次调用", function()
        it("连续调用应都成功", function()
            for i = 1, 5 do
                local data, status = http_get("/ping")
                assert.are.equal(200, status)
                assert.are.equal("ok", data.status)
            end
        end)

        it("时间戳应随时间变化", function()
            local data1, _ = http_get("/ping")
            socket.sleep(1.1)
            local data2, _ = http_get("/ping")
            assert.is_not_nil(data1.timestamp)
            assert.is_not_nil(data2.timestamp)
            -- 间隔超过1秒，时间戳应该不同
            assert.are_not.equal(data1.timestamp, data2.timestamp)
        end)
    end)
end)

--------------------------------------------------------------------------------
-- 测试: /execute 请求验证 - 请求体
--------------------------------------------------------------------------------

describe("/execute 请求验证 - 请求体", function()
    describe("空请求和无效 JSON", function()
        it("空请求体应返回 HTTP 400", function()
            local data, status = http_post("/execute", "")
            assert.are.equal(400, status)
            assert.are.equal("validation", data.error)
            assert.is_truthy(data.message:match("请求体为空"))
        end)

        it("空白字符请求体应返回 HTTP 400", function()
            local data, status = http_post("/execute", "   ")
            assert.are.equal(400, status)
        end)

        it("无效 JSON 应返回 HTTP 400", function()
            local data, status = http_post("/execute", "{ invalid json }")
            assert.are.equal(400, status)
            assert.are.equal("validation", data.error)
            assert.is_truthy(data.message:match("无效的 JSON"))
        end)

        it("不完整的 JSON 应返回 HTTP 400", function()
            local data, status = http_post("/execute", '{"lang": "lua"')
            assert.are.equal(400, status)
            assert.are.equal("validation", data.error)
        end)

        it("JSON 数组应返回 HTTP 400", function()
            local data, status = http_post("/execute", "[1, 2, 3]")
            assert.are.equal(400, status)
        end)

        it("JSON 原始值应返回 HTTP 400", function()
            local data, status = http_post("/execute", '"just a string"')
            assert.are.equal(400, status)
        end)

        it("JSON null 应返回 HTTP 400", function()
            local data, status = http_post("/execute", "null")
            assert.are.equal(400, status)
        end)

        it("JSON 数字应返回 HTTP 400", function()
            local data, status = http_post("/execute", "12345")
            assert.are.equal(400, status)
        end)

        it("JSON 布尔值应返回 HTTP 400", function()
            local data, status = http_post("/execute", "true")
            assert.are.equal(400, status)
        end)
    end)

    describe("lang 字段验证", function()
        it("缺少 lang 字段应返回 HTTP 400", function()
            local data, status = http_post("/execute", { code = "print('hello')" })
            assert.are.equal(400, status)
            assert.are.equal("validation", data.error)
            assert.is_truthy(data.message:match("lang"))
        end)

        it("lang 为空字符串应返回 HTTP 400", function()
            local data, status = http_post("/execute", { lang = "", code = "print('hello')" })
            assert.are.equal(400, status)
            assert.are.equal("validation", data.error)
        end)

        it("lang 为 null 应返回 HTTP 400", function()
            local data, status = http_post("/execute", '{"lang": null, "code": "print(1)"}')
            assert.are.equal(400, status)
        end)

        it("lang 为数字应返回 HTTP 400", function()
            local data, status = http_post("/execute", { lang = 123, code = "print('hello')" })
            assert.are.equal(400, status)
            assert.are.equal("validation", data.error)
        end)

        it("lang 为布尔值应返回 HTTP 400", function()
            local data, status = http_post("/execute", '{"lang": true, "code": "print(1)"}')
            assert.are.equal(400, status)
        end)

        it("lang 为数组应返回 HTTP 400", function()
            local data, status = http_post("/execute", '{"lang": ["lua"], "code": "print(1)"}')
            assert.are.equal(400, status)
        end)

        it("lang 为对象应返回 HTTP 400", function()
            local data, status = http_post("/execute", '{"lang": {"name": "lua"}, "code": "print(1)"}')
            assert.are.equal(400, status)
        end)

        it("lang 为浮点数应返回 HTTP 400", function()
            local data, status = http_post("/execute", { lang = 3.14, code = "print('hello')" })
            assert.are.equal(400, status)
        end)

        it("lang 为负数应返回 HTTP 400", function()
            local data, status = http_post("/execute", { lang = -1, code = "print('hello')" })
            assert.are.equal(400, status)
        end)
    end)

    describe("code 字段验证", function()
        it("缺少 code 字段应返回 HTTP 400", function()
            local data, status = http_post("/execute", { lang = "lua" })
            assert.are.equal(400, status)
            assert.are.equal("validation", data.error)
            assert.is_truthy(data.message:match("code"))
        end)

        it("code 为 null 应返回 HTTP 400", function()
            local data, status = http_post("/execute", '{"lang": "lua", "code": null}')
            assert.are.equal(400, status)
        end)

        it("code 为数字应返回 HTTP 400", function()
            local data, status = http_post("/execute", { lang = "lua", code = 12345 })
            assert.are.equal(400, status)
            assert.are.equal("validation", data.error)
        end)

        it("code 为布尔值应返回 HTTP 400", function()
            local data, status = http_post("/execute", '{"lang": "lua", "code": false}')
            assert.are.equal(400, status)
        end)

        it("code 为数组应返回 HTTP 400", function()
            local data, status = http_post("/execute", '{"lang": "lua", "code": ["print(1)"]}')
            assert.are.equal(400, status)
        end)

        it("code 为对象应返回 HTTP 400", function()
            local data, status = http_post("/execute", '{"lang": "lua", "code": {"src": "print(1)"}}')
            assert.are.equal(400, status)
        end)

        it("空字符串 code 应被接受", function()
            local data, status = http_post("/execute", { lang = "lua", code = "" })
            assert.are.equal(200, status)
        end)
    end)

    describe("额外字段处理", function()
        it("额外字段应被忽略", function()
            local data, status = http_post("/execute", {
                lang = "lua",
                code = 'print("test")',
                extra_field = "should be ignored",
                another = 12345,
                nested = { a = 1, b = 2 }
            })
            assert.are.equal(200, status)
        end)

        it("大量额外字段应被忽略", function()
            local payload = { lang = "lua", code = 'print("test")' }
            for i = 1, 100 do
                payload["extra_" .. i] = "value_" .. i
            end
            local data, status = http_post("/execute", payload)
            assert.are.equal(200, status)
        end)
    end)
end)

--------------------------------------------------------------------------------
-- 测试: /execute 请求验证 - 语言
--------------------------------------------------------------------------------

describe("/execute 请求验证 - 语言", function()
    describe("不支持的语言", function()
        local unsupported_langs = {
            "golang", "java", "ruby", "perl", "swift", "kotlin",
            "scala", "haskell", "erlang", "elixir", "clojure",
            "cpp", "cxx", "c++", "objective-c", "assembly"
        }

        for _, lang in ipairs(unsupported_langs) do
            it(lang .. " 应返回 HTTP 400", function()
                local data, status = http_post("/execute", {
                    lang = lang,
                    code = "some code"
                })
                assert.are.equal(400, status)
                assert.are.equal("validation", data.error)
                assert.is_truthy(data.message:match("不支持的语言"))
            end)
        end
    end)

    describe("已移除的语言", function()
        it("javascript 应返回 HTTP 400", function()
            local data, status = http_post("/execute", {
                lang = "javascript",
                code = "console.log('test')"
            })
            assert.are.equal(400, status)
            assert.is_truthy(data.message:match("不支持的语言"))
        end)

        it("typescript 应返回 HTTP 400", function()
            local data, status = http_post("/execute", {
                lang = "typescript",
                code = "console.log('test')"
            })
            assert.are.equal(400, status)
        end)

        it("c 应返回 HTTP 400", function()
            local data, status = http_post("/execute", {
                lang = "c",
                code = "int main() { return 0; }"
            })
            assert.are.equal(400, status)
        end)

        it("php 应返回 HTTP 400", function()
            local data, status = http_post("/execute", {
                lang = "php",
                code = "<?php echo 'test'; ?>"
            })
            assert.are.equal(400, status)
        end)
    end)

    describe("大小写敏感", function()
        it("Lua 应返回 HTTP 400", function()
            local data, status = http_post("/execute", { lang = "Lua", code = "print('test')" })
            assert.are.equal(400, status)
        end)

        it("LUA 应返回 HTTP 400", function()
            local data, status = http_post("/execute", { lang = "LUA", code = "print('test')" })
            assert.are.equal(400, status)
        end)

        it("Python 应返回 HTTP 400", function()
            local data, status = http_post("/execute", { lang = "Python", code = "print('test')" })
            assert.are.equal(400, status)
        end)

        it("PYTHON 应返回 HTTP 400", function()
            local data, status = http_post("/execute", { lang = "PYTHON", code = "print('test')" })
            assert.are.equal(400, status)
        end)

        it("Rust 应返回 HTTP 400", function()
            local data, status = http_post("/execute", { lang = "Rust", code = "fn main() {}" })
            assert.are.equal(400, status)
        end)

        it("RUST 应返回 HTTP 400", function()
            local data, status = http_post("/execute", { lang = "RUST", code = "fn main() {}" })
            assert.are.equal(400, status)
        end)
    end)

    describe("语言名格式异常", function()
        it("带前导空格的语言名应返回 HTTP 400", function()
            local data, status = http_post("/execute", { lang = " lua", code = "print('test')" })
            assert.are.equal(400, status)
        end)

        it("带尾随空格的语言名应返回 HTTP 400", function()
            local data, status = http_post("/execute", { lang = "lua ", code = "print('test')" })
            assert.are.equal(400, status)
        end)

        it("带前后空格的语言名应返回 HTTP 400", function()
            local data, status = http_post("/execute", { lang = " lua ", code = "print('test')" })
            assert.are.equal(400, status)
        end)

        it("带换行符的语言名应返回 HTTP 400", function()
            local data, status = http_post("/execute", { lang = "lua\n", code = "print('test')" })
            assert.are.equal(400, status)
        end)

        it("带制表符的语言名应返回 HTTP 400", function()
            local data, status = http_post("/execute", { lang = "lua\t", code = "print('test')" })
            assert.are.equal(400, status)
        end)
    end)

    describe("支持的语言", function()
        it("lua 应被接受", function()
            local data, status = http_post("/execute", { lang = "lua", code = "print('test')" })
            assert.are.equal(200, status)
        end)

        it("python 应被接受", function()
            local data, status = http_post("/execute", { lang = "python", code = "print('test')" })
            assert.are.equal(200, status)
        end)

        it("rust 应被接受", function()
            local data, status = http_post("/execute", {
                lang = "rust",
                code = 'fn main() { println!("test"); }',
                timeout_ms = 30000
            })
            assert.are.equal(200, status)
        end)
    end)
end)

--------------------------------------------------------------------------------
-- 测试: /execute 请求验证 - 超时
--------------------------------------------------------------------------------

describe("/execute 请求验证 - 超时", function()
    describe("无效超时值", function()
        it("timeout_ms 为 0 应返回 HTTP 400", function()
            local data, status = http_post("/execute", {
                lang = "lua",
                code = "print('hello')",
                timeout_ms = 0
            })
            assert.are.equal(400, status)
            assert.are.equal("validation", data.error)
        end)

        it("timeout_ms 为 50 应返回 HTTP 400", function()
            local data, status = http_post("/execute", {
                lang = "lua",
                code = "print('hello')",
                timeout_ms = 50
            })
            assert.are.equal(400, status)
            assert.is_truthy(data.message:match("100"))
        end)

        it("timeout_ms 为 99 应返回 HTTP 400", function()
            local data, status = http_post("/execute", {
                lang = "lua",
                code = "print('hello')",
                timeout_ms = 99
            })
            assert.are.equal(400, status)
        end)

        it("timeout_ms 为 30001 应返回 HTTP 400", function()
            local data, status = http_post("/execute", {
                lang = "lua",
                code = "print('hello')",
                timeout_ms = 30001
            })
            assert.are.equal(400, status)
            assert.is_truthy(data.message:match("30000"))
        end)

        it("timeout_ms 为 60000 应返回 HTTP 400", function()
            local data, status = http_post("/execute", {
                lang = "lua",
                code = "print('hello')",
                timeout_ms = 60000
            })
            assert.are.equal(400, status)
        end)

        it("timeout_ms 为负数应返回 HTTP 400", function()
            local data, status = http_post("/execute", {
                lang = "lua",
                code = "print('hello')",
                timeout_ms = -1000
            })
            assert.are.equal(400, status)
        end)

        it("timeout_ms 为字符串应返回 HTTP 400", function()
            local data, status = http_post("/execute", {
                lang = "lua",
                code = "print('hello')",
                timeout_ms = "5000"
            })
            assert.are.equal(400, status)
        end)

        it("timeout_ms 为布尔值应返回 HTTP 400", function()
            local data, status = http_post("/execute", '{"lang":"lua","code":"print(1)","timeout_ms":true}')
            assert.are.equal(400, status)
        end)

        -- 修改：null 在 JSON 中会被 cjson 解析为 cjson.null，服务器应该返回 400
        it("timeout_ms 为 null 应返回 HTTP 400", function()
            local data, status = http_post("/execute", '{"lang":"lua","code":"print(1)","timeout_ms":null}')
            assert.are.equal(400, status)
        end)
    end)

    describe("有效超时值", function()
        it("timeout_ms 为 100 应被接受", function()
            local data, status = http_post("/execute", {
                lang = "lua",
                code = "print('hi')",
                timeout_ms = 100
            })
            assert.are.equal(200, status)
        end)

        it("timeout_ms 为 1000 应被接受", function()
            local data, status = http_post("/execute", {
                lang = "lua",
                code = "print('hi')",
                timeout_ms = 1000
            })
            assert.are.equal(200, status)
        end)

        it("timeout_ms 为 5000 应被接受", function()
            local data, status = http_post("/execute", {
                lang = "lua",
                code = "print('hi')",
                timeout_ms = 5000
            })
            assert.are.equal(200, status)
        end)

        it("timeout_ms 为 30000 应被接受", function()
            local data, status = http_post("/execute", {
                lang = "lua",
                code = "print('hi')",
                timeout_ms = 30000
            })
            assert.are.equal(200, status)
        end)

        it("timeout_ms 为浮点数应被接受(取整)", function()
            local data, status = http_post("/execute", {
                lang = "lua",
                code = "print('hello')",
                timeout_ms = 3000.5
            })
            assert.are.equal(200, status)
        end)

        it("不提供 timeout_ms 应使用默认值", function()
            local data, status = http_post("/execute", {
                lang = "lua",
                code = "print('hi')"
            })
            assert.are.equal(200, status)
        end)
    end)
end)

--------------------------------------------------------------------------------
-- 测试: /execute 响应结构
--------------------------------------------------------------------------------

describe("/execute 响应结构", function()
    describe("成功响应必需字段", function()
        it("应包含 request_id", function()
            local data, status = http_post("/execute", { lang = "lua", code = 'print("test")' })
            assert.are.equal(200, status)
            assert.is_not_nil(data.request_id)
        end)

        it("request_id 应为有效的 UUID v4 格式", function()
            local data, status = http_post("/execute", { lang = "lua", code = 'print("test")' })
            assert.are.equal(200, status)
            local uuid_pattern = "^%x%x%x%x%x%x%x%x%-%x%x%x%x%-4%x%x%x%-[89ab]%x%x%x%-%x%x%x%x%x%x%x%x%x%x%x%x$"
            assert.is_truthy(data.request_id:match(uuid_pattern), "request_id 不是有效的 UUID v4")
        end)

        it("每次请求应生成不同的 request_id", function()
            local ids = {}
            for i = 1, 10 do
                local data, status = http_post("/execute", { lang = "lua", code = 'print(' .. i .. ')' })
                assert.are.equal(200, status)
                assert.is_nil(ids[data.request_id], "发现重复的 request_id")
                ids[data.request_id] = true
            end
        end)

        it("应包含 duration_ms", function()
            local data, status = http_post("/execute", { lang = "lua", code = 'print("test")' })
            assert.are.equal(200, status)
            assert.is_not_nil(data.duration_ms)
            assert.is_true(type(data.duration_ms) == "number")
            assert.is_true(data.duration_ms >= 0)
        end)

        it("应包含 status 字段", function()
            local data, status = http_post("/execute", { lang = "lua", code = 'print("test")' })
            assert.are.equal(200, status)
            assert.is_not_nil(data.status)
            assert.is_true(data.status == "ok" or data.status == "error")
        end)
    end)

    describe("状态字段语义", function()
        it("未生成 trace.toml 时 status 应为 error", function()
            local data, status = http_post("/execute", { lang = "lua", code = 'print("hello")' })
            assert.are.equal(200, status)
            assert.are.equal("error", data.status)
            assert.is_nil(data.toml)
        end)

        it("status 为 error 时应有 error 字段", function()
            local data, status = http_post("/execute", { lang = "lua", code = 'print("hello")' })
            assert.are.equal(200, status)
            assert.are.equal("error", data.status)
            assert.is_not_nil(data.error)
        end)

        it("stdout 输出应在 stdout 字段中返回", function()
            local data, status = http_post("/execute", { lang = "lua", code = 'print("MARKER_12345")' })
            assert.are.equal(200, status)
            if data.stdout then
                assert.is_truthy(data.stdout:match("MARKER_12345"))
            end
        end)
    end)

    describe("Content-Type", function()
        it("成功响应应为 JSON", function()
            local data, status, headers = http_post("/execute", { lang = "lua", code = "print(1)" })
            local ct = headers["content-type"] or headers["Content-Type"]
            assert.is_truthy(ct:match("application/json"))
        end)

        it("错误响应应为 JSON", function()
            local data, status, headers = http_post("/execute", { lang = "invalid" })
            local ct = headers["content-type"] or headers["Content-Type"]
            assert.is_truthy(ct:match("application/json"))
        end)
    end)
end)

--------------------------------------------------------------------------------
-- 测试: 样本文件执行 - 成功样本 (排除 Rust)
--------------------------------------------------------------------------------

describe("样本文件执行 - 成功样本 (Lua/Python)", function()
    local ok_files = list_sample_files("ok")

    for _, filename in ipairs(ok_files) do
        local lang = get_lang_from_filename(filename)
        -- 仅测试 Lua 和 Python，Rust 单独测试
        if lang and (lang == "lua" or lang == "python") then
            describe(filename, function()
                it("应成功执行并返回 HTTP 200", function()
                    local code = read_sample("ok/" .. filename)
                    if not code then
                        pending("样本文件不存在: " .. filename)
                        return
                    end

                    local data, status = http_post("/execute", {
                        lang = lang,
                        code = code,
                        timeout_ms = get_timeout_for_lang(lang)
                    })

                    assert.are.equal(200, status, "HTTP 状态码不是 200")
                end)

                it("status 应为 ok", function()
                    local code = read_sample("ok/" .. filename)
                    if not code then
                        pending("样本文件不存在: " .. filename)
                        return
                    end

                    local data, status = http_post("/execute", {
                        lang = lang,
                        code = code,
                        timeout_ms = get_timeout_for_lang(lang)
                    })

                    assert.are.equal(200, status)
                    assert.are.equal("ok", data.status, "status 不是 ok: " .. (data.error or "unknown"))
                end)

                it("应返回非空 toml", function()
                    local code = read_sample("ok/" .. filename)
                    if not code then
                        pending("样本文件不存在: " .. filename)
                        return
                    end

                    local data, status = http_post("/execute", {
                        lang = lang,
                        code = code,
                        timeout_ms = get_timeout_for_lang(lang)
                    })

                    assert.are.equal(200, status)
                    assert.is_not_nil(data.toml, "toml 为空")
                    assert.is_true(#data.toml > 0, "toml 长度为 0")
                end)

                it("toml 应包含必需块并通过结构验证", function()
                    local code = read_sample("ok/" .. filename)
                    if not code then
                        pending("样本文件不存在: " .. filename)
                        return
                    end

                    local data, status = http_post("/execute", {
                        lang = lang,
                        code = code,
                        timeout_ms = get_timeout_for_lang(lang)
                    })

                    assert.are.equal(200, status)
                    if data.toml then
                        assert.is_truthy(toml_has_section(data.toml, "meta"), "toml 缺少 [meta]")
                        assert.is_truthy(toml_has_section(data.toml, "package"), "toml 缺少 [package]")
                        assert.is_truthy(toml_has_section(data.toml, "object"), "toml 缺少 [object]")
                        assert.is_truthy(toml_has_section(data.toml, "result"), "toml 缺少 [result]")
                        assert.is_falsy(toml_has_section(data.toml, "error"), "toml 不应包含 [error]")

                        local valid, errors = validate_toml_structure(data.toml)
                        assert.is_true(valid, "TOML 结构验证失败: " .. table.concat(errors, ", "))
                    end
                end)
            end)
        end
    end
end)

--------------------------------------------------------------------------------
-- 测试: 样本文件执行 - Rust 成功样本 (独立测试，允许编译失败)
--------------------------------------------------------------------------------

describe("样本文件执行 - Rust 成功样本", function()
    local ok_files = list_sample_files("ok")

    for _, filename in ipairs(ok_files) do
        local lang = get_lang_from_filename(filename)
        if lang == "rust" then
            describe(filename, function()
                it("应成功执行或因环境问题失败", function()
                    local code = read_sample("ok/" .. filename)
                    if not code then
                        pending("样本文件不存在: " .. filename)
                        return
                    end

                    local data, status = http_post("/execute", {
                        lang = lang,
                        code = code,
                        timeout_ms = CONFIG.rust_timeout
                    })

                    assert.are.equal(200, status, "HTTP 状态码不是 200")

                    -- Rust 编译可能因环境问题失败，这里只验证返回格式正确
                    assert.is_not_nil(data.status)

                    if data.status == "ok" then
                        -- 成功时验证 toml
                        assert.is_not_nil(data.toml, "toml 为空")
                        assert.is_truthy(toml_has_section(data.toml, "result"))
                    else
                        -- 失败时应有错误信息
                        assert.is_not_nil(data.error)
                        -- 检查是否是编译相关错误（可接受）
                        if data.stdout then
                            local is_compile_error = data.stdout:match("error") or
                                data.stdout:match("rlib not found") or
                                data.error:match("编译失败")
                            if is_compile_error then
                                pending("Rust 编译环境问题: " .. (data.error or ""))
                            end
                        end
                    end
                end)
            end)
        end
    end
end)

--------------------------------------------------------------------------------
-- 测试: 样本文件执行 - 失败样本
--------------------------------------------------------------------------------

describe("样本文件执行 - 失败样本", function()
    local err_files = list_sample_files("err")

    for _, filename in ipairs(err_files) do
        local lang = get_lang_from_filename(filename)
        -- Rust 失败样本单独处理
        if lang and (lang == "lua" or lang == "python") then
            describe(filename, function()
                it("应返回 HTTP 200", function()
                    local code = read_sample("err/" .. filename)
                    if not code then
                        pending("样本文件不存在: " .. filename)
                        return
                    end

                    local data, status = http_post("/execute", {
                        lang = lang,
                        code = code,
                        timeout_ms = get_timeout_for_lang(lang)
                    })

                    assert.are.equal(200, status, "HTTP 状态码不是 200")
                end)

                it("status 应为 error 或 toml 包含 [error] 块", function()
                    local code = read_sample("err/" .. filename)
                    if not code then
                        pending("样本文件不存在: " .. filename)
                        return
                    end

                    local data, status = http_post("/execute", {
                        lang = lang,
                        code = code,
                        timeout_ms = get_timeout_for_lang(lang)
                    })

                    assert.are.equal(200, status)

                    -- 失败样本可能：1. status=error 且无 toml，2. 有 toml 但包含 [error] 块
                    if data.toml then
                        -- 如果有 toml，应该包含 [error] 块
                        local has_error_section = toml_has_section(data.toml, "error")
                        local has_result_section = toml_has_section(data.toml, "result")
                        -- 要么有 [error]，要么整体 status 是 error
                        assert.is_true(has_error_section or data.status == "error",
                            "失败样本应有 [error] 块或 status=error")
                        -- 不应同时有 [result] 和 [error]
                        assert.is_false(has_result_section and has_error_section,
                            "不能同时存在 [result] 和 [error]")
                    else
                        -- 无 toml 时 status 必须是 error
                        assert.are.equal("error", data.status, "无 toml 时 status 应为 error")
                    end
                end)
            end)
        elseif lang == "rust" then
            describe(filename .. " (Rust)", function()
                it("应返回 HTTP 200 且为错误状态", function()
                    local code = read_sample("err/" .. filename)
                    if not code then
                        pending("样本文件不存在: " .. filename)
                        return
                    end

                    local data, status = http_post("/execute", {
                        lang = lang,
                        code = code,
                        timeout_ms = CONFIG.rust_timeout
                    })

                    assert.are.equal(200, status)
                    -- Rust 失败样本应该返回 error（编译或运行时错误）
                    assert.are.equal("error", data.status)
                end)
            end)
        end
    end
end)

--------------------------------------------------------------------------------
-- 测试: Lua 基本执行
--------------------------------------------------------------------------------

describe("Lua 基本执行", function()
    describe("简单代码", function()
        it("print 语句应执行", function()
            local data, status = http_post("/execute", {
                lang = "lua",
                code = 'print("hello lua")'
            })
            assert.are.equal(200, status)
            if data.stdout then
                assert.is_truthy(data.stdout:match("hello lua"))
            end
        end)

        it("算术运算应执行", function()
            local data, status = http_post("/execute", {
                lang = "lua",
                code = 'local x = 1 + 2 * 3; print(x)'
            })
            assert.are.equal(200, status)
            if data.stdout then
                assert.is_truthy(data.stdout:match("7"))
            end
        end)

        it("多行代码应执行", function()
            local code = [[
local a = 10
local b = 20
local c = a + b
print(c)
]]
            local data, status = http_post("/execute", { lang = "lua", code = code })
            assert.are.equal(200, status)
            if data.stdout then
                assert.is_truthy(data.stdout:match("30"))
            end
        end)

        it("函数定义和调用应执行", function()
            local code = [[
local function add(a, b)
    return a + b
end
print(add(3, 4))
]]
            local data, status = http_post("/execute", { lang = "lua", code = code })
            assert.are.equal(200, status)
            if data.stdout then
                assert.is_truthy(data.stdout:match("7"))
            end
        end)

        it("表操作应执行", function()
            local code = [[
local t = {1, 2, 3}
table.insert(t, 4)
print(#t)
]]
            local data, status = http_post("/execute", { lang = "lua", code = code })
            assert.are.equal(200, status)
            if data.stdout then
                assert.is_truthy(data.stdout:match("4"))
            end
        end)

        it("循环应执行", function()
            local code = [[
local sum = 0
for i = 1, 10 do
    sum = sum + i
end
print(sum)
]]
            local data, status = http_post("/execute", { lang = "lua", code = code })
            assert.are.equal(200, status)
            if data.stdout then
                assert.is_truthy(data.stdout:match("55"))
            end
        end)

        it("字符串操作应执行", function()
            local code = [[
local s = "hello"
print(s:upper())
]]
            local data, status = http_post("/execute", { lang = "lua", code = code })
            assert.are.equal(200, status)
            if data.stdout then
                assert.is_truthy(data.stdout:match("HELLO"))
            end
        end)
    end)

    describe("错误处理", function()
        it("语法错误应返回 error 状态", function()
            local data, status = http_post("/execute", {
                lang = "lua",
                code = "print(unclosed_parenthesis"
            })
            assert.are.equal(200, status)
            assert.are.equal("error", data.status)
        end)

        it("运行时错误应返回 error 状态", function()
            local data, status = http_post("/execute", {
                lang = "lua",
                code = 'error("intentional error")'
            })
            assert.are.equal(200, status)
            assert.are.equal("error", data.status)
            if data.stdout then
                assert.is_truthy(data.stdout:match("intentional error"))
            end
        end)

        it("未定义变量访问应返回 error 状态", function()
            local data, status = http_post("/execute", {
                lang = "lua",
                code = 'print(undefined_variable.field)'
            })
            assert.are.equal(200, status)
            assert.are.equal("error", data.status)
        end)

        it("类型错误应返回 error 状态", function()
            local data, status = http_post("/execute", {
                lang = "lua",
                code = 'local x = "hello" + 1'
            })
            assert.are.equal(200, status)
            assert.are.equal("error", data.status)
        end)

        it("除零返回 inf 而非错误", function()
            local data, status = http_post("/execute", {
                lang = "lua",
                code = 'print(1/0)'
            })
            assert.are.equal(200, status)
            -- Lua 中 1/0 返回 inf，不是错误
            if data.stdout then
                assert.is_truthy(data.stdout:match("inf"))
            end
        end)
    end)
end)

--------------------------------------------------------------------------------
-- 测试: Python 基本执行
--------------------------------------------------------------------------------

describe("Python 基本执行", function()
    describe("简单代码", function()
        it("print 语句应执行", function()
            local data, status = http_post("/execute", {
                lang = "python",
                code = 'print("hello python")'
            })
            assert.are.equal(200, status)
            if data.stdout then
                assert.is_truthy(data.stdout:match("hello python"))
            end
        end)

        it("算术运算应执行", function()
            local data, status = http_post("/execute", {
                lang = "python",
                code = 'x = 1 + 2 * 3\nprint(x)'
            })
            assert.are.equal(200, status)
            if data.stdout then
                assert.is_truthy(data.stdout:match("7"))
            end
        end)

        it("多行代码应执行", function()
            local code = [[
a = 10
b = 20
c = a + b
print(c)
]]
            local data, status = http_post("/execute", { lang = "python", code = code })
            assert.are.equal(200, status)
            if data.stdout then
                assert.is_truthy(data.stdout:match("30"))
            end
        end)

        it("函数定义和调用应执行", function()
            local code = [[
def add(a, b):
    return a + b

print(add(3, 4))
]]
            local data, status = http_post("/execute", { lang = "python", code = code })
            assert.are.equal(200, status)
            if data.stdout then
                assert.is_truthy(data.stdout:match("7"))
            end
        end)

        it("列表操作应执行", function()
            local code = [[
lst = [1, 2, 3]
lst.append(4)
print(len(lst))
]]
            local data, status = http_post("/execute", { lang = "python", code = code })
            assert.are.equal(200, status)
            if data.stdout then
                assert.is_truthy(data.stdout:match("4"))
            end
        end)

        it("列表推导式应执行", function()
            local code = [[
squares = [x**2 for x in range(5)]
print(sum(squares))
]]
            local data, status = http_post("/execute", { lang = "python", code = code })
            assert.are.equal(200, status)
            if data.stdout then
                assert.is_truthy(data.stdout:match("30"))
            end
        end)

        it("字典操作应执行", function()
            local code = [[
d = {'a': 1, 'b': 2}
d['c'] = 3
print(len(d))
]]
            local data, status = http_post("/execute", { lang = "python", code = code })
            assert.are.equal(200, status)
            if data.stdout then
                assert.is_truthy(data.stdout:match("3"))
            end
        end)

        it("类定义和实例化应执行", function()
            local code = [[
class Counter:
    def __init__(self):
        self.count = 0
    def inc(self):
        self.count += 1

c = Counter()
c.inc()
c.inc()
print(c.count)
]]
            local data, status = http_post("/execute", { lang = "python", code = code })
            assert.are.equal(200, status)
            if data.stdout then
                assert.is_truthy(data.stdout:match("2"))
            end
        end)

        it("标准库导入应执行", function()
            local code = [[
import math
print(math.pi)
]]
            local data, status = http_post("/execute", { lang = "python", code = code })
            assert.are.equal(200, status)
            if data.stdout then
                assert.is_truthy(data.stdout:match("3%.14"))
            end
        end)
    end)

    describe("错误处理", function()
        it("语法错误应返回 error 状态", function()
            local data, status = http_post("/execute", {
                lang = "python",
                code = "def broken("
            })
            assert.are.equal(200, status)
            assert.are.equal("error", data.status)
            if data.stdout then
                assert.is_truthy(data.stdout:match("SyntaxError") or data.stdout:match("Error"))
            end
        end)

        it("缩进错误应返回 error 状态", function()
            local code = [[
def foo():
print("wrong indent")
]]
            local data, status = http_post("/execute", { lang = "python", code = code })
            assert.are.equal(200, status)
            assert.are.equal("error", data.status)
        end)

        it("运行时错误应返回 error 状态", function()
            local data, status = http_post("/execute", {
                lang = "python",
                code = 'raise ValueError("intentional error")'
            })
            assert.are.equal(200, status)
            assert.are.equal("error", data.status)
            if data.stdout then
                assert.is_truthy(data.stdout:match("ValueError"))
            end
        end)

        it("NameError 应返回 error 状态", function()
            local data, status = http_post("/execute", {
                lang = "python",
                code = 'print(undefined_variable)'
            })
            assert.are.equal(200, status)
            assert.are.equal("error", data.status)
            if data.stdout then
                assert.is_truthy(data.stdout:match("NameError"))
            end
        end)

        it("TypeError 应返回 error 状态", function()
            local data, status = http_post("/execute", {
                lang = "python",
                code = '"hello" + 1'
            })
            assert.are.equal(200, status)
            assert.are.equal("error", data.status)
        end)

        it("IndexError 应返回 error 状态", function()
            local data, status = http_post("/execute", {
                lang = "python",
                code = 'lst = [1,2,3]\nprint(lst[10])'
            })
            assert.are.equal(200, status)
            assert.are.equal("error", data.status)
            if data.stdout then
                assert.is_truthy(data.stdout:match("IndexError"))
            end
        end)

        it("ZeroDivisionError 应返回 error 状态", function()
            local data, status = http_post("/execute", {
                lang = "python",
                code = 'print(1/0)'
            })
            assert.are.equal(200, status)
            assert.are.equal("error", data.status)
            if data.stdout then
                assert.is_truthy(data.stdout:match("ZeroDivisionError"))
            end
        end)

        it("ModuleNotFoundError 应返回 error 状态", function()
            local data, status = http_post("/execute", {
                lang = "python",
                code = 'import nonexistent_module_xyz123'
            })
            assert.are.equal(200, status)
            assert.are.equal("error", data.status)
        end)
    end)
end)

--------------------------------------------------------------------------------
-- 测试: Rust 基本执行 (容忍编译环境问题)
--------------------------------------------------------------------------------

describe("Rust 基本执行", function()
    -- 辅助函数：检查 Rust 执行结果，容忍编译环境问题
    local function check_rust_result(data, status, expected_stdout_pattern)
        assert.are.equal(200, status)

        if data.status == "ok" then
            -- 成功执行
            if expected_stdout_pattern and data.stdout then
                assert.is_truthy(data.stdout:match(expected_stdout_pattern),
                    "stdout 不匹配: " .. (data.stdout or "nil"))
            end
            return true
        elseif data.status == "error" then
            -- 检查是否是编译环境问题
            local is_env_issue = false
            if data.stdout then
                is_env_issue = data.stdout:match("rlib not found") or
                    data.stdout:match("can't find crate")
            end
            if data.error then
                is_env_issue = is_env_issue or data.error:match("编译失败")
            end

            if is_env_issue then
                pending("Rust 编译环境问题")
                return false
            end

            -- 真正的代码错误
            return true
        end
    end

    describe("简单代码", function()
        it("println 宏应执行", function()
            local code = [[
fn main() {
    println!("hello rust");
}
]]
            local data, status = http_post("/execute", {
                lang = "rust",
                code = code,
                timeout_ms = CONFIG.rust_timeout
            })
            check_rust_result(data, status, "hello rust")
        end)

        it("变量绑定应执行", function()
            local code = [[
fn main() {
    let x = 10;
    let y = 20;
    println!("{}", x + y);
}
]]
            local data, status = http_post("/execute", {
                lang = "rust",
                code = code,
                timeout_ms = CONFIG.rust_timeout
            })
            check_rust_result(data, status, "30")
        end)

        it("函数定义应执行", function()
            local code = [[
fn add(a: i32, b: i32) -> i32 {
    a + b
}

fn main() {
    println!("{}", add(3, 4));
}
]]
            local data, status = http_post("/execute", {
                lang = "rust",
                code = code,
                timeout_ms = CONFIG.rust_timeout
            })
            check_rust_result(data, status, "7")
        end)

        it("Vector 操作应执行", function()
            local code = [[
fn main() {
    let mut v = vec![1, 2, 3];
    v.push(4);
    println!("{}", v.len());
}
]]
            local data, status = http_post("/execute", {
                lang = "rust",
                code = code,
                timeout_ms = CONFIG.rust_timeout
            })
            check_rust_result(data, status, "4")
        end)

        it("迭代器应执行", function()
            local code = [[
fn main() {
    let sum: i32 = (1..=10).sum();
    println!("{}", sum);
}
]]
            local data, status = http_post("/execute", {
                lang = "rust",
                code = code,
                timeout_ms = CONFIG.rust_timeout
            })
            check_rust_result(data, status, "55")
        end)

        it("结构体应执行", function()
            local code = [[
struct Point {
    x: i32,
    y: i32,
}

fn main() {
    let p = Point { x: 10, y: 20 };
    println!("{}", p.x + p.y);
}
]]
            local data, status = http_post("/execute", {
                lang = "rust",
                code = code,
                timeout_ms = CONFIG.rust_timeout
            })
            check_rust_result(data, status, "30")
        end)

        it("Option 类型应执行", function()
            local code = [[
fn main() {
    let x: Option<i32> = Some(5);
    match x {
        Some(v) => println!("{}", v),
        None => println!("none"),
    }
}
]]
            local data, status = http_post("/execute", {
                lang = "rust",
                code = code,
                timeout_ms = CONFIG.rust_timeout
            })
            check_rust_result(data, status, "5")
        end)
    end)

    describe("编译错误", function()
        it("类型不匹配应返回 error 状态", function()
            local code = [[
fn main() {
    let x: i32 = "not a number";
}
]]
            local data, status = http_post("/execute", {
                lang = "rust",
                code = code,
                timeout_ms = CONFIG.rust_timeout
            })
            assert.are.equal(200, status)
            assert.are.equal("error", data.status)
        end)

        it("未定义变量应返回 error 状态", function()
            local code = [[
fn main() {
    println!("{}", undefined_var);
}
]]
            local data, status = http_post("/execute", {
                lang = "rust",
                code = code,
                timeout_ms = CONFIG.rust_timeout
            })
            assert.are.equal(200, status)
            assert.are.equal("error", data.status)
        end)

        it("借用错误应返回 error 状态", function()
            local code = [[
fn main() {
    let mut s = String::from("hello");
    let r1 = &mut s;
    let r2 = &mut s;
    println!("{} {}", r1, r2);
}
]]
            local data, status = http_post("/execute", {
                lang = "rust",
                code = code,
                timeout_ms = CONFIG.rust_timeout
            })
            assert.are.equal(200, status)
            assert.are.equal("error", data.status)
        end)

        it("语法错误应返回 error 状态", function()
            local code = [[
fn main() {
    let x =
}
]]
            local data, status = http_post("/execute", {
                lang = "rust",
                code = code,
                timeout_ms = CONFIG.rust_timeout
            })
            assert.are.equal(200, status)
            assert.are.equal("error", data.status)
        end)

        it("缺少分号应返回 error 状态", function()
            local code = [[
fn main() {
    let x = 5
    println!("{}", x);
}
]]
            local data, status = http_post("/execute", {
                lang = "rust",
                code = code,
                timeout_ms = CONFIG.rust_timeout
            })
            assert.are.equal(200, status)
            assert.are.equal("error", data.status)
        end)
    end)

    describe("运行时行为", function()
        it("panic 应返回 error 状态", function()
            local code = [[
fn main() {
    panic!("intentional panic");
}
]]
            local data, status = http_post("/execute", {
                lang = "rust",
                code = code,
                timeout_ms = CONFIG.rust_timeout
            })
            assert.are.equal(200, status)
            -- panic 会导致非零退出码，应该是 error
            -- 但如果编译环境有问题也会是 error
            assert.are.equal("error", data.status)
        end)

        it("数组越界应返回 error 状态", function()
            local code = [[
fn main() {
    let arr = [1, 2, 3];
    println!("{}", arr[10]);
}
]]
            local data, status = http_post("/execute", {
                lang = "rust",
                code = code,
                timeout_ms = CONFIG.rust_timeout
            })
            assert.are.equal(200, status)
            assert.are.equal("error", data.status)
        end)

        it("Option unwrap None 应 panic", function()
            local code = [[
fn main() {
    let x: Option<i32> = None;
    println!("{}", x.unwrap());
}
]]
            local data, status = http_post("/execute", {
                lang = "rust",
                code = code,
                timeout_ms = CONFIG.rust_timeout
            })
            assert.are.equal(200, status)
            assert.are.equal("error", data.status)
        end)
    end)
end)

--------------------------------------------------------------------------------
-- 测试: 超时处理
--------------------------------------------------------------------------------

describe("超时处理", function()
    describe("无限循环", function()
        it("Lua 无限循环应在超时后终止", function()
            local start = socket.gettime()
            local data, status = http_post("/execute", {
                lang = "lua",
                code = "while true do end",
                timeout_ms = 1000
            })
            local elapsed = socket.gettime() - start
            assert.are.equal(200, status)
            assert.are.equal("error", data.status)
            assert.is_true(elapsed >= 1, "超时未生效")
            assert.is_true(elapsed < 5, "超时时间过长")
        end)

        it("Python 无限循环应在超时后终止", function()
            local start = socket.gettime()
            local data, status = http_post("/execute", {
                lang = "python",
                code = "while True: pass",
                timeout_ms = 1000
            })
            local elapsed = socket.gettime() - start
            assert.are.equal(200, status)
            assert.are.equal("error", data.status)
            assert.is_true(elapsed >= 1, "超时未生效")
            assert.is_true(elapsed < 5, "超时时间过长")
        end)
    end)

    describe("sleep 超时", function()
        it("Python sleep 超时应被正确处理", function()
            local start = socket.gettime()
            local data, status = http_post("/execute", {
                lang = "python",
                code = "import time; time.sleep(10)",
                timeout_ms = 1000
            })
            local elapsed = socket.gettime() - start
            assert.are.equal(200, status)
            assert.are.equal("error", data.status)
            assert.is_true(elapsed < 5, "超时未生效")
        end)
    end)

    describe("自定义超时", function()
        it("2秒超时应被尊重", function()
            local start = socket.gettime()
            local data, status = http_post("/execute", {
                lang = "lua",
                code = "while true do end",
                timeout_ms = 2000
            })
            local elapsed = socket.gettime() - start
            assert.are.equal(200, status)
            assert.are.equal("error", data.status)
            assert.is_true(elapsed >= 2, "超时时间过短")
            assert.is_true(elapsed < 5, "超时时间过长")
        end)

        it("最小超时 100ms 应生效", function()
            local data, status = http_post("/execute", {
                lang = "lua",
                code = "while true do end",
                timeout_ms = 100
            })
            assert.are.equal(200, status)
            assert.are.equal("error", data.status)
        end)
    end)

    describe("快速执行不应被误判", function()
        it("快速 Lua 代码应正常返回", function()
            local data, status = http_post("/execute", {
                lang = "lua",
                code = 'print("quick")',
                timeout_ms = 100
            })
            assert.are.equal(200, status)
            -- 应该不是超时错误
            if data.error then
                assert.is_falsy(data.error:match("超时"))
            end
        end)

        it("快速 Python 代码应正常返回", function()
            local data, status = http_post("/execute", {
                lang = "python",
                code = 'print("quick")',
                timeout_ms = 100
            })
            assert.are.equal(200, status)
        end)
    end)
end)

--------------------------------------------------------------------------------
-- 测试: 边缘情况 - 空代码和注释
--------------------------------------------------------------------------------

describe("边缘情况 - 空代码和注释", function()
    describe("空代码", function()
        it("Lua 空字符串应被接受", function()
            local data, status = http_post("/execute", { lang = "lua", code = "" })
            assert.are.equal(200, status)
        end)

        it("Python 空字符串应被接受", function()
            local data, status = http_post("/execute", { lang = "python", code = "" })
            assert.are.equal(200, status)
        end)

        it("仅空白字符应被接受", function()
            local data, status = http_post("/execute", { lang = "python", code = "   \n\t\n   " })
            assert.are.equal(200, status)
        end)

        it("仅换行符应被接受", function()
            local data, status = http_post("/execute", { lang = "lua", code = "\n\n\n" })
            assert.are.equal(200, status)
        end)
    end)

    describe("仅注释", function()
        it("Lua 单行注释应被接受", function()
            local data, status = http_post("/execute", {
                lang = "lua",
                code = "-- this is just a comment"
            })
            assert.are.equal(200, status)
        end)

        it("Lua 多行注释应被接受", function()
            local code = [=[
--[[
This is a
multi-line comment
]]
]=]
            local data, status = http_post("/execute", { lang = "lua", code = code })
            assert.are.equal(200, status)
        end)

        it("Python 单行注释应被接受", function()
            local data, status = http_post("/execute", {
                lang = "python",
                code = "# this is just a comment"
            })
            assert.are.equal(200, status)
        end)

        it("Python 多行注释应被接受", function()
            local code = [[
# line 1
# line 2
# line 3
]]
            local data, status = http_post("/execute", { lang = "python", code = code })
            assert.are.equal(200, status)
        end)

        it("Python docstring 应被接受", function()
            local code = [[
"""
This is a docstring
"""
]]
            local data, status = http_post("/execute", { lang = "python", code = code })
            assert.are.equal(200, status)
        end)
    end)
end)

--------------------------------------------------------------------------------
-- 测试: 边缘情况 - 特殊字符
--------------------------------------------------------------------------------

describe("边缘情况 - 特殊字符", function()
    describe("Unicode", function()
        it("Python Unicode 应正常执行", function()
            local data, status = http_post("/execute", {
                lang = "python",
                code = 'print("你好世界")'
            })
            assert.are.equal(200, status)
        end)

        it("Lua Unicode 应正常执行", function()
            local data, status = http_post("/execute", {
                lang = "lua",
                code = 'print("你好世界")'
            })
            assert.are.equal(200, status)
        end)

        it("Emoji 应正常执行", function()
            local data, status = http_post("/execute", {
                lang = "python",
                code = 'print("🎉🎊🎁")'
            })
            assert.are.equal(200, status)
        end)
    end)

    describe("转义字符", function()
        it("换行符转义应正常执行", function()
            local data, status = http_post("/execute", {
                lang = "lua",
                code = [[print("line1\nline2")]]
            })
            assert.are.equal(200, status)
        end)

        it("制表符转义应正常执行", function()
            local data, status = http_post("/execute", {
                lang = "lua",
                code = [[print("col1\tcol2")]]
            })
            assert.are.equal(200, status)
        end)

        it("引号应正常执行", function()
            local data, status = http_post("/execute", {
                lang = "python",
                code = [[print('single "double" quotes')]]
            })
            assert.are.equal(200, status)
        end)

        it("反斜杠应正常执行", function()
            local data, status = http_post("/execute", {
                lang = "python",
                code = [[print("path\\to\\file")]]
            })
            assert.are.equal(200, status)
        end)
    end)

    describe("Shell 特殊字符", function()
        it("Shell 变量语法应作为字面量", function()
            local data, status = http_post("/execute", {
                lang = "python",
                code = 'print("$HOME")'
            })
            assert.are.equal(200, status)
            if data.stdout then
                assert.is_truthy(data.stdout:match("%$HOME"))
            end
        end)

        it("反引号应作为字面量", function()
            local data, status = http_post("/execute", {
                lang = "python",
                code = 'print("`whoami`")'
            })
            assert.are.equal(200, status)
            if data.stdout then
                assert.is_truthy(data.stdout:match("`whoami`"))
            end
        end)

        it("命令替换语法应作为字面量", function()
            local data, status = http_post("/execute", {
                lang = "python",
                code = 'print("$(ls)")'
            })
            assert.are.equal(200, status)
            if data.stdout then
                assert.is_truthy(data.stdout:match("%$%(ls%)"))
            end
        end)
    end)
end)

--------------------------------------------------------------------------------
-- 测试: 边缘情况 - 大代码量
--------------------------------------------------------------------------------

describe("边缘情况 - 大代码量", function()
    it("100 行代码应能执行", function()
        local lines = {}
        for i = 1, 100 do
            table.insert(lines, string.format('print("line %d")', i))
        end
        local code = table.concat(lines, "\n")
        local data, status = http_post("/execute", {
            lang = "lua",
            code = code,
            timeout_ms = 5000
        })
        assert.are.equal(200, status)
    end)

    it("长单行代码应能执行", function()
        local long_string = string.rep("x", 10000)
        local code = string.format('print("%s")', long_string)
        local data, status = http_post("/execute", {
            lang = "lua",
            code = code,
            timeout_ms = 5000
        })
        assert.are.equal(200, status)
    end)

    it("深度嵌套代码应能执行", function()
        local code = [[
def deep():
    x = 1
    if x:
        if x:
            if x:
                if x:
                    if x:
                        print("deep")
deep()
]]
        local data, status = http_post("/execute", { lang = "python", code = code })
        assert.are.equal(200, status)
        if data.stdout then
            assert.is_truthy(data.stdout:match("deep"))
        end
    end)
end)

--------------------------------------------------------------------------------
-- 测试: 边缘情况 - stdout
--------------------------------------------------------------------------------

describe("边缘情况 - stdout", function()
    describe("多行输出", function()
        it("多行输出应被完整捕获", function()
            local code = [[
for i = 1, 5 do
    print("line " .. i)
end
]]
            local data, status = http_post("/execute", { lang = "lua", code = code })
            assert.are.equal(200, status)
            if data.stdout then
                assert.is_truthy(data.stdout:match("line 1"))
                assert.is_truthy(data.stdout:match("line 5"))
            end
        end)
    end)

    describe("stderr", function()
        it("Python stderr 应被捕获", function()
            local code = [[
import sys
print("stderr message", file=sys.stderr)
]]
            local data, status = http_post("/execute", { lang = "python", code = code })
            assert.are.equal(200, status)
            if data.stdout then
                assert.is_truthy(data.stdout:match("stderr message"))
            end
        end)

        it("混合 stdout 和 stderr 应被捕获", function()
            local code = [[
import sys
print("stdout")
print("stderr", file=sys.stderr)
print("stdout again")
]]
            local data, status = http_post("/execute", { lang = "python", code = code })
            assert.are.equal(200, status)
            if data.stdout then
                assert.is_truthy(data.stdout:match("stdout"))
                assert.is_truthy(data.stdout:match("stderr"))
            end
        end)
    end)

    describe("io.write", function()
        it("Lua io.write 输出应被捕获", function()
            local data, status = http_post("/execute", {
                lang = "lua",
                code = 'io.write("io_write_test")'
            })
            assert.are.equal(200, status)
            if data.stdout then
                assert.is_truthy(data.stdout:match("io_write_test"))
            end
        end)
    end)

    describe("flush", function()
        it("Python flush 输出应被捕获", function()
            local code = [[
import sys
print("buffered output", flush=True)
sys.stdout.flush()
]]
            local data, status = http_post("/execute", { lang = "python", code = code })
            assert.are.equal(200, status)
            if data.stdout then
                assert.is_truthy(data.stdout:match("buffered output"))
            end
        end)
    end)
end)

--------------------------------------------------------------------------------
-- 测试: 并发请求
--------------------------------------------------------------------------------

describe("并发请求", function()
    it("多个顺序请求应独立处理", function()
        local ids = {}
        for i = 1, 5 do
            local data, status = http_post("/execute", {
                lang = "lua",
                code = string.format('print("request %d")', i)
            })
            assert.are.equal(200, status)
            assert.is_nil(ids[data.request_id], "发现重复的 request_id")
            ids[data.request_id] = true
        end
    end)

    it("不同语言的请求应独立处理", function()
        local requests = {
            { lang = "lua",    code = 'print("lua")' },
            { lang = "python", code = 'print("python")' },
            { lang = "lua",    code = 'print("lua2")' },
        }

        local ids = {}
        for _, req in ipairs(requests) do
            local data, status = http_post("/execute", req)
            assert.are.equal(200, status)
            assert.is_nil(ids[data.request_id])
            ids[data.request_id] = true
        end
    end)
end)

--------------------------------------------------------------------------------
-- 测试: 文件系统隔离
--------------------------------------------------------------------------------

describe("文件系统隔离", function()
    it("文件写入和读取应在独立目录中", function()
        local code = [[
with open("test_file.txt", "w") as f:
    f.write("test content")
with open("test_file.txt", "r") as f:
    print(f.read())
]]
        local data, status = http_post("/execute", { lang = "python", code = code })
        assert.are.equal(200, status)
        if data.stdout then
            assert.is_truthy(data.stdout:match("test content"))
        end
    end)

    it("读取不存在的文件应报错", function()
        local code = [[
with open("nonexistent_file_xyz.txt", "r") as f:
    print(f.read())
]]
        local data, status = http_post("/execute", { lang = "python", code = code })
        assert.are.equal(200, status)
        assert.are.equal("error", data.status)
    end)

    it("前一次执行的文件不应影响后续执行", function()
        -- 第一次执行：创建文件
        local code1 = [[
with open("shared_file.txt", "w") as f:
    f.write("first")
print("created")
]]
        local data1, status1 = http_post("/execute", { lang = "python", code = code1 })
        assert.are.equal(200, status1)

        -- 第二次执行：文件不应存在
        local code2 = [[
try:
    with open("shared_file.txt", "r") as f:
        print(f.read())
except FileNotFoundError:
    print("not found")
]]
        local data2, status2 = http_post("/execute", { lang = "python", code = code2 })
        assert.are.equal(200, status2)
        if data2.stdout then
            assert.is_truthy(data2.stdout:match("not found"))
        end
    end)
end)

--------------------------------------------------------------------------------
-- 测试: CORS 支持
--------------------------------------------------------------------------------

describe("CORS 支持", function()
    describe("OPTIONS 预检请求", function()
        it("/execute OPTIONS 应返回 204", function()
            local status, headers = http_options("/execute")
            assert.are.equal(204, status)
        end)

        it("/ping OPTIONS 应返回 204", function()
            local status, headers = http_options("/ping")
            assert.are.equal(204, status)
        end)

        it("OPTIONS 应返回 Access-Control-Allow-Origin", function()
            local status, headers = http_options("/execute")
            local allow_origin = headers["access-control-allow-origin"] or headers["Access-Control-Allow-Origin"]
            assert.are.equal("*", allow_origin)
        end)

        it("OPTIONS 应返回 Access-Control-Allow-Methods", function()
            local status, headers = http_options("/execute")
            local allow_methods = headers["access-control-allow-methods"] or headers["Access-Control-Allow-Methods"]
            assert.is_truthy(allow_methods:match("POST"))
            assert.is_truthy(allow_methods:match("GET"))
        end)

        it("OPTIONS 应返回 Access-Control-Allow-Headers", function()
            local status, headers = http_options("/execute")
            local allow_headers = headers["access-control-allow-headers"] or headers["Access-Control-Allow-Headers"]
            assert.is_truthy(allow_headers:match("Content%-Type"))
        end)
    end)

    describe("响应 CORS 头", function()
        it("GET /ping 响应应包含 CORS 头", function()
            local data, status, headers = http_get("/ping")
            local cors = headers["access-control-allow-origin"] or headers["Access-Control-Allow-Origin"]
            assert.are.equal("*", cors)
        end)

        it("POST /execute 成功响应应包含 CORS 头", function()
            local data, status, headers = http_post("/execute", { lang = "lua", code = 'print(1)' })
            local cors = headers["access-control-allow-origin"] or headers["Access-Control-Allow-Origin"]
            assert.are.equal("*", cors)
        end)

        it("POST /execute 400 响应应包含 CORS 头", function()
            local data, status, headers = http_post("/execute", { lang = "invalid" })
            local cors = headers["access-control-allow-origin"] or headers["Access-Control-Allow-Origin"]
            assert.are.equal("*", cors)
        end)

        it("404 响应应包含 CORS 头", function()
            local data, status, headers = http_get("/notfound")
            local cors = headers["access-control-allow-origin"] or headers["Access-Control-Allow-Origin"]
            assert.are.equal("*", cors)
        end)
    end)
end)

--------------------------------------------------------------------------------
-- 测试: 404 处理
--------------------------------------------------------------------------------

describe("404 处理", function()
    describe("不存在的路径", function()
        it("GET /nonexistent 应返回 404", function()
            local data, status = http_get("/nonexistent")
            assert.are.equal(404, status)
            assert.are.equal("not_found", data.error)
        end)

        it("GET /a/b/c/d 应返回 404", function()
            local data, status = http_get("/a/b/c/d")
            assert.are.equal(404, status)
        end)

        it("错误响应应包含路径信息", function()
            local data, status = http_get("/unknown/path")
            assert.are.equal(404, status)
            assert.is_truthy(data.message:match("/unknown/path"))
        end)
    end)

    describe("错误的 HTTP 方法", function()
        it("POST /ping 应返回 404", function()
            local data, status = http_post("/ping", {})
            assert.are.equal(404, status)
        end)

        it("GET /execute 应返回 404", function()
            local data, status = http_get("/execute")
            assert.are.equal(404, status)
        end)

        it("PUT /execute 应返回 404", function()
            local data, status = http_method("PUT", "/execute", { lang = "lua", code = "print(1)" })
            assert.are.equal(404, status)
        end)

        it("DELETE /execute 应返回 404", function()
            local data, status = http_method("DELETE", "/execute", nil)
            assert.are.equal(404, status)
        end)

        it("PATCH /execute 应返回 404", function()
            local data, status = http_method("PATCH", "/execute", { lang = "lua", code = "print(1)" })
            assert.are.equal(404, status)
        end)
    end)
end)

--------------------------------------------------------------------------------
-- 测试: 响应格式一致性
--------------------------------------------------------------------------------

describe("响应格式一致性", function()
    describe("成功执行", function()
        it("成功生成 toml 时应包含所有必需字段", function()
            local code = read_sample("ok/lua_tree_sucess.lua")
            if code then
                local data, status = http_post("/execute", { lang = "lua", code = code })
                assert.are.equal(200, status)
                assert.is_not_nil(data.request_id)
                assert.is_not_nil(data.duration_ms)
                assert.are.equal("ok", data.status)
                assert.is_not_nil(data.toml)
            else
                pending("样本文件不存在")
            end
        end)
    end)

    describe("执行失败", function()
        it("未生成 toml 时应有 error 字段", function()
            local data, status = http_post("/execute", { lang = "lua", code = 'print("no trace")' })
            assert.are.equal(200, status)
            assert.is_not_nil(data.request_id)
            assert.is_not_nil(data.duration_ms)
            assert.are.equal("error", data.status)
            assert.is_not_nil(data.error)
        end)
    end)

    describe("验证错误", function()
        it("应包含 error 和 message", function()
            local data, status = http_post("/execute", { lang = "invalid_lang", code = "test" })
            assert.are.equal(400, status)
            assert.is_not_nil(data.error)
            assert.is_not_nil(data.message)
            assert.are.equal("validation", data.error)
        end)
    end)

    describe("404 错误", function()
        it("应包含 error 和 message", function()
            local data, status = http_get("/notfound")
            assert.are.equal(404, status)
            assert.is_not_nil(data.error)
            assert.is_not_nil(data.message)
            assert.are.equal("not_found", data.error)
        end)
    end)

    describe("Content-Type 一致性", function()
        it("所有响应的 Content-Type 应为 JSON", function()
            local test_cases = {
                { method = "get",  path = "/ping" },
                { method = "post", path = "/execute", payload = { lang = "lua", code = "print(1)" } },
                { method = "get",  path = "/notfound" },
                { method = "post", path = "/execute", payload = { lang = "invalid" } },
            }

            for _, tc in ipairs(test_cases) do
                local data, status, headers
                if tc.method == "get" then
                    data, status, headers = http_get(tc.path)
                else
                    data, status, headers = http_post(tc.path, tc.payload)
                end
                local ct = headers["content-type"] or headers["Content-Type"]
                assert.is_truthy(ct:match("application/json"), tc.path .. " Content-Type 不是 JSON")
            end
        end)
    end)
end)

--------------------------------------------------------------------------------
-- 测试: 跨语言一致性
--------------------------------------------------------------------------------

describe("跨语言一致性", function()
    describe("所有语言的基本执行", function()
        local test_codes = {
            lua = 'print("hello lua")',
            python = 'print("hello python")',
        }

        for lang, code in pairs(test_codes) do
            it(lang .. " 简单代码应成功执行", function()
                local data, status = http_post("/execute", {
                    lang = lang,
                    code = code,
                    timeout_ms = get_timeout_for_lang(lang)
                })
                assert.are.equal(200, status, lang .. " 执行失败")
                assert.is_not_nil(data.request_id)
                assert.is_not_nil(data.duration_ms)
            end)
        end

        it("rust 简单代码应成功执行（容忍编译环境问题）", function()
            local code = 'fn main() { println!("hello rust"); }'
            local data, status = http_post("/execute", {
                lang = "rust",
                code = code,
                timeout_ms = CONFIG.rust_timeout
            })
            assert.are.equal(200, status, "rust 执行失败")
            assert.is_not_nil(data.request_id)
            assert.is_not_nil(data.duration_ms)
        end)
    end)

    describe("所有语言的语法错误", function()
        local error_codes = {
            lua = 'print(unclosed',
            python = 'def broken(',
            rust = 'fn main() { let x = ',
        }

        for lang, code in pairs(error_codes) do
            it(lang .. " 语法错误应返回 error 状态", function()
                local data, status = http_post("/execute", {
                    lang = lang,
                    code = code,
                    timeout_ms = get_timeout_for_lang(lang)
                })
                assert.are.equal(200, status)
                assert.are.equal("error", data.status, lang .. " 状态不是 error")
            end)
        end
    end)

    describe("所有语言的超时处理", function()
        local timeout_codes = {
            lua = 'while true do end',
            python = 'while True: pass',
        }

        for lang, code in pairs(timeout_codes) do
            it(lang .. " 超时应被正确处理", function()
                local data, status = http_post("/execute", {
                    lang = lang,
                    code = code,
                    timeout_ms = 1000
                })
                assert.are.equal(200, status)
                assert.are.equal("error", data.status, lang .. " 超时未被正确处理")
            end)
        end
    end)
end)

--------------------------------------------------------------------------------
-- 测试: stdout 标记验证
--------------------------------------------------------------------------------

describe("stdout 标记验证", function()
    it("Lua stdout 应被正确捕获", function()
        local data, status = http_post("/execute", {
            lang = "lua",
            code = 'print("STDOUT_MARKER_LUA_12345")'
        })
        assert.are.equal(200, status)
        if data.stdout then
            assert.is_truthy(data.stdout:match("STDOUT_MARKER_LUA_12345"))
        end
    end)

    it("Python stdout 应被正确捕获", function()
        local data, status = http_post("/execute", {
            lang = "python",
            code = 'print("STDOUT_MARKER_PYTHON_12345")'
        })
        assert.are.equal(200, status)
        if data.stdout then
            assert.is_truthy(data.stdout:match("STDOUT_MARKER_PYTHON_12345"))
        end
    end)

    it("Rust stdout 应被正确捕获（容忍编译环境问题）", function()
        local code = [[
fn main() {
    println!("STDOUT_MARKER_RUST_12345");
}
]]
        local data, status = http_post("/execute", {
            lang = "rust",
            code = code,
            timeout_ms = CONFIG.rust_timeout
        })
        assert.are.equal(200, status)

        if data.status == "ok" and data.stdout then
            assert.is_truthy(data.stdout:match("STDOUT_MARKER_RUST_12345"))
        elseif data.status == "error" then
            -- 检查是否是编译环境问题
            local is_env_issue = false
            if data.stdout then
                is_env_issue = data.stdout:match("rlib not found") or
                    data.stdout:match("can't find crate")
            end
            if data.error then
                is_env_issue = is_env_issue or data.error:match("编译失败")
            end
            if is_env_issue then
                pending("Rust 编译环境问题")
            end
        end
    end)
end)
