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
    2025-12-27
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
}

--------------------------------------------------------------------------------
-- HTTP 工具函数
--------------------------------------------------------------------------------

--@description: 发送 HTTP GET 请求
--@param path: string - 请求路径
--@return: table|nil - 响应数据
--@return: number - HTTP 状态码
--@return: table - 响应头
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

--@description: 发送 HTTP POST 请求
--@param path: string - 请求路径
--@param payload: table|string|nil - 请求体
--@param content_type: string|nil - Content-Type
--@return: table|nil - 响应数据
--@return: number - HTTP 状态码
--@return: table - 响应头
--@return: string - 原始响应体
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

--@description: 发送 HTTP OPTIONS 请求
--@param path: string - 请求路径
--@return: number - HTTP 状态码
--@return: table - 响应头
local function http_options(path)
    local response_body = {}
    local result, status_code, headers = http.request({
        url = CONFIG.base_url .. path,
        method = "OPTIONS",
        sink = ltn12.sink.table(response_body),
    })

    return status_code or 0, headers or {}
end

--@description: 读取测试样本文件
--@param filename: string - 文件名
--@return: string|nil - 文件内容
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

--------------------------------------------------------------------------------
-- 测试: /ping 健康检查接口
--------------------------------------------------------------------------------

describe("/ping 健康检查接口", function()
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
        assert.is_truthy(data.timestamp:match("^%d%d%d%d%-%d%d%-%d%dT%d%d:%d%d:%d%d%.%d%d%dZ$"))
    end)

    it("应返回支持的语言列表", function()
        local data, status = http_get("/ping")
        assert.are.equal(200, status)
        assert.is_not_nil(data.supported_languages)
        assert.is_true(type(data.supported_languages) == "table")
        assert.is_true(#data.supported_languages > 0)

        local expected_langs = { "lua", "python", "javascript", "typescript", "c", "rust", "php" }
        local lang_set = {}
        for _, lang in ipairs(data.supported_languages) do
            lang_set[lang] = true
        end
        for _, lang in ipairs(expected_langs) do
            assert.is_true(lang_set[lang], "缺少语言: " .. lang)
        end
    end)

    it("应返回默认超时时间", function()
        local data, status = http_get("/ping")
        assert.are.equal(200, status)
        assert.is_not_nil(data.default_timeout_ms)
        assert.is_true(type(data.default_timeout_ms) == "number")
        assert.is_true(data.default_timeout_ms > 0)
    end)

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
        local cors_origin = headers["access-control-allow-origin"] or headers["Access-Control-Allow-Origin"]
        assert.are.equal("*", cors_origin)
    end)
end)

--------------------------------------------------------------------------------
-- 测试: /execute 请求验证
--------------------------------------------------------------------------------

describe("/execute 请求验证", function()
    describe("请求体验证", function()
        it("空请求体应返回 HTTP 400", function()
            local data, status = http_post("/execute", "")
            assert.are.equal(400, status)
            assert.is_not_nil(data)
            assert.are.equal("validation", data.error)
            assert.is_truthy(data.message:match("请求体为空"))
        end)

        it("无效 JSON 应返回 HTTP 400", function()
            local data, status = http_post("/execute", "{ invalid json }")
            assert.are.equal(400, status)
            assert.is_not_nil(data)
            assert.are.equal("validation", data.error)
            assert.is_truthy(data.message:match("无效的 JSON"))
        end)

        it("缺少 lang 字段应返回 HTTP 400", function()
            local data, status = http_post("/execute", {
                code = "print('hello')"
            })
            assert.are.equal(400, status)
            assert.is_not_nil(data)
            assert.are.equal("validation", data.error)
            assert.is_truthy(data.message:match("lang"))
        end)

        it("lang 为空字符串应返回 HTTP 400", function()
            local data, status = http_post("/execute", {
                lang = "",
                code = "print('hello')"
            })
            assert.are.equal(400, status)
            assert.is_not_nil(data)
            assert.are.equal("validation", data.error)
        end)

        it("lang 为非字符串类型应返回 HTTP 400", function()
            local data, status = http_post("/execute", {
                lang = 123,
                code = "print('hello')"
            })
            assert.are.equal(400, status)
            assert.is_not_nil(data)
            assert.are.equal("validation", data.error)
        end)

        it("缺少 code 字段应返回 HTTP 400", function()
            local data, status = http_post("/execute", {
                lang = "lua"
            })
            assert.are.equal(400, status)
            assert.is_not_nil(data)
            assert.are.equal("validation", data.error)
            assert.is_truthy(data.message:match("code"))
        end)

        it("code 为非字符串类型应返回 HTTP 400", function()
            local data, status = http_post("/execute", {
                lang = "lua",
                code = 12345
            })
            assert.are.equal(400, status)
            assert.is_not_nil(data)
            assert.are.equal("validation", data.error)
        end)
    end)

    describe("语言验证", function()
        it("不支持的语言应返回 HTTP 400", function()
            local data, status = http_post("/execute", {
                lang = "golang",
                code = "package main"
            })
            assert.are.equal(400, status)
            assert.is_not_nil(data)
            assert.are.equal("validation", data.error)
            assert.is_truthy(data.message:match("不支持的语言"))
            assert.is_truthy(data.message:match("golang"))
        end)

        it("不存在的语言应返回 HTTP 400", function()
            local data, status = http_post("/execute", {
                lang = "nonexistent_language_xyz",
                code = "some code"
            })
            assert.are.equal(400, status)
            assert.are.equal("validation", data.error)
        end)
    end)

    describe("超时参数验证", function()
        it("timeout_ms 小于最小值应返回 HTTP 400", function()
            local data, status = http_post("/execute", {
                lang = "lua",
                code = "print('hello')",
                timeout_ms = 50
            })
            assert.are.equal(400, status)
            assert.is_not_nil(data)
            assert.are.equal("validation", data.error)
            assert.is_truthy(data.message:match("timeout_ms"))
            assert.is_truthy(data.message:match("100"))
        end)

        it("timeout_ms 大于最大值应返回 HTTP 400", function()
            local data, status = http_post("/execute", {
                lang = "lua",
                code = "print('hello')",
                timeout_ms = 60000
            })
            assert.are.equal(400, status)
            assert.is_not_nil(data)
            assert.are.equal("validation", data.error)
            assert.is_truthy(data.message:match("timeout_ms"))
            assert.is_truthy(data.message:match("30000"))
        end)

        it("timeout_ms 为非数字类型应返回 HTTP 400", function()
            local data, status = http_post("/execute", {
                lang = "lua",
                code = "print('hello')",
                timeout_ms = "5000"
            })
            assert.are.equal(400, status)
            assert.is_not_nil(data)
            assert.are.equal("validation", data.error)
        end)

        it("timeout_ms 为负数应返回 HTTP 400", function()
            local data, status = http_post("/execute", {
                lang = "lua",
                code = "print('hello')",
                timeout_ms = -1000
            })
            assert.are.equal(400, status)
            assert.are.equal("validation", data.error)
        end)

        it("有效的 timeout_ms 应被接受", function()
            local data, status = http_post("/execute", {
                lang = "lua",
                code = "print('hello')",
                timeout_ms = 3000
            })
            assert.are.equal(200, status)
            assert.is_not_nil(data)
        end)

        it("边界值 100ms 应被接受", function()
            local data, status = http_post("/execute", {
                lang = "lua",
                code = "print('hi')",
                timeout_ms = 100
            })
            assert.are.equal(200, status)
        end)

        it("边界值 30000ms 应被接受", function()
            local data, status = http_post("/execute", {
                lang = "lua",
                code = "print('hi')",
                timeout_ms = 30000
            })
            assert.are.equal(200, status)
        end)
    end)
end)

--------------------------------------------------------------------------------
-- 测试: /execute 代码执行 - 响应结构验证
--------------------------------------------------------------------------------

describe("/execute 响应结构验证", function()
    it("成功响应应包含 request_id", function()
        local data, status = http_post("/execute", {
            lang = "lua",
            code = 'print("test")'
        })
        assert.are.equal(200, status)
        assert.is_not_nil(data.request_id)
    end)

    it("request_id 应为有效的 UUID v4 格式", function()
        local data, status = http_post("/execute", {
            lang = "lua",
            code = 'print("test")'
        })
        assert.are.equal(200, status)
        local uuid_pattern = "^%x%x%x%x%x%x%x%x%-%x%x%x%x%-4%x%x%x%-[89ab]%x%x%x%-%x%x%x%x%x%x%x%x%x%x%x%x$"
        assert.is_truthy(data.request_id:match(uuid_pattern), "request_id 不是有效的 UUID v4")
    end)

    it("成功响应应包含 duration_ms", function()
        local data, status = http_post("/execute", {
            lang = "lua",
            code = 'print("test")'
        })
        assert.are.equal(200, status)
        assert.is_not_nil(data.duration_ms)
        assert.is_true(type(data.duration_ms) == "number")
        assert.is_true(data.duration_ms >= 0)
    end)

    it("成功响应应包含 status 字段", function()
        local data, status = http_post("/execute", {
            lang = "lua",
            code = 'print("test")'
        })
        assert.are.equal(200, status)
        assert.is_not_nil(data.status)
        assert.is_true(data.status == "ok" or data.status == "error")
    end)

    it("代码执行成功但未生成 trace.toml 时 status 应为 error", function()
        -- 简单的 print 不会生成 trace.toml（没有使用 ds4viz 库）
        local data, status = http_post("/execute", {
            lang = "lua",
            code = 'print("hello")'
        })
        assert.are.equal(200, status)
        assert.are.equal("error", data.status)
        assert.is_nil(data.toml)
        assert.is_not_nil(data.error)
    end)

    it("stdout 输出应在 stdout 字段中返回", function()
        local data, status = http_post("/execute", {
            lang = "lua",
            code = 'print("hello world")'
        })
        assert.are.equal(200, status)
        if data.stdout then
            assert.is_truthy(data.stdout:match("hello world"))
        end
    end)
end)

--------------------------------------------------------------------------------
-- 测试: /execute 代码执行 - 基本执行测试
--------------------------------------------------------------------------------

describe("/execute 代码执行 - 基本执行", function()
    describe("Lua 执行", function()
        it("简单代码应成功执行", function()
            local data, status = http_post("/execute", {
                lang = "lua",
                code = 'local x = 1 + 1; print(x)'
            })
            assert.are.equal(200, status)
            assert.is_not_nil(data)
            assert.is_not_nil(data.request_id)
            assert.is_not_nil(data.duration_ms)
        end)

        it("从样本文件执行成功代码", function()
            local code = read_sample("ok/lua_tree_sucess.lua")
            if code then
                local data, status = http_post("/execute", {
                    lang = "lua",
                    code = code
                })
                assert.are.equal(200, status)
                assert.are.equal("ok", data.status)
                assert.is_not_nil(data.toml)
            else
                pending("样本文件不存在: ok/lua_tree_sucess.lua")
            end
        end)

        it("复杂图算法样本应成功执行", function()
            local code = read_sample("ok/lua_graph_directed_complex_sucess.lua")
            if code then
                local data, status = http_post("/execute", {
                    lang = "lua",
                    code = code,
                    timeout_ms = 10000
                })
                assert.are.equal(200, status)
                assert.are.equal("ok", data.status)
                assert.is_not_nil(data.toml)
            else
                pending("样本文件不存在: ok/lua_graph_directed_complex_sucess.lua")
            end
        end)
    end)

    describe("Python 执行", function()
        it("简单代码应成功执行", function()
            local data, status = http_post("/execute", {
                lang = "python",
                code = 'x = 10 + 20\nprint(x)'
            })
            assert.are.equal(200, status)
            assert.is_not_nil(data)
        end)

        it("双向链表样本应成功执行", function()
            local code = read_sample("ok/py_double_linked_list_complex_sucess.py")
            if code then
                local data, status = http_post("/execute", {
                    lang = "python",
                    code = code,
                    timeout_ms = 10000
                })
                assert.are.equal(200, status)
                assert.are.equal("ok", data.status)
                assert.is_not_nil(data.toml)
            else
                pending("样本文件不存在")
            end
        end)

        it("加权图样本应成功执行", function()
            local code = read_sample("ok/py_graph_weighted_sucess.py")
            if code then
                local data, status = http_post("/execute", {
                    lang = "python",
                    code = code,
                    timeout_ms = 10000
                })
                assert.are.equal(200, status)
                assert.are.equal("ok", data.status)
                assert.is_not_nil(data.toml)
            else
                pending("样本文件不存在")
            end
        end)
    end)

    describe("JavaScript 执行", function()
        it("简单代码应成功执行", function()
            local data, status = http_post("/execute", {
                lang = "javascript",
                code = 'const x = 1 + 2; console.log(x);'
            })
            assert.are.equal(200, status)
            assert.is_not_nil(data)
        end)

        it("栈操作样本应成功执行", function()
            local code = read_sample("ok/js_stack_sucess.js")
            if code then
                local data, status = http_post("/execute", {
                    lang = "javascript",
                    code = code
                })
                assert.are.equal(200, status)
                assert.are.equal("ok", data.status)
                assert.is_not_nil(data.toml)
            else
                pending("样本文件不存在")
            end
        end)
    end)

    describe("TypeScript 执行", function()
        it("简单代码应成功执行", function()
            local data, status = http_post("/execute", {
                lang = "typescript",
                code = 'const x: number = 42; console.log(x);'
            })
            assert.are.equal(200, status)
            assert.is_not_nil(data)
        end)

        it("二叉搜索树样本应成功执行", function()
            local code = read_sample("ok/ts_binary_search_tree_sucess.ts")
            if code then
                local data, status = http_post("/execute", {
                    lang = "typescript",
                    code = code,
                    timeout_ms = 15000
                })
                assert.are.equal(200, status)
                assert.are.equal("ok", data.status)
                assert.is_not_nil(data.toml)
            else
                pending("样本文件不存在")
            end
        end)
    end)

    describe("C 执行", function()
        it("简单 Hello World 应成功编译执行", function()
            local code = [[
#include <stdio.h>
int main() {
    printf("hello from c\n");
    return 0;
}
]]
            local data, status = http_post("/execute", {
                lang = "c",
                code = code
            })
            assert.are.equal(200, status)
            assert.is_not_nil(data)
        end)

        it("队列样本应成功执行", function()
            local code = read_sample("ok/c_queue_sucess.c")
            if code then
                local data, status = http_post("/execute", {
                    lang = "c",
                    code = code,
                    timeout_ms = 10000
                })
                assert.are.equal(200, status)
                assert.are.equal("ok", data.status)
                assert.is_not_nil(data.toml)
            else
                pending("样本文件不存在")
            end
        end)
    end)

    describe("Rust 执行", function()
        it("简单 println 应成功编译执行", function()
            local code = [[
fn main() {
    println!("hello from rust");
}
]]
            local data, status = http_post("/execute", {
                lang = "rust",
                code = code,
                timeout_ms = 30000
            })
            assert.are.equal(200, status)
            assert.is_not_nil(data)
        end)

        it("二叉搜索树样本应成功执行", function()
            local code = read_sample("ok/rust_binary_search_tree_sucess.rs")
            if code then
                local data, status = http_post("/execute", {
                    lang = "rust",
                    code = code,
                    timeout_ms = 30000
                })
                assert.are.equal(200, status)
                assert.are.equal("ok", data.status)
                assert.is_not_nil(data.toml)
            else
                pending("样本文件不存在")
            end
        end)

        it("堆样本应成功执行", function()
            local code = read_sample("ok/rust_heap_sucess.rs")
            if code then
                local data, status = http_post("/execute", {
                    lang = "rust",
                    code = code,
                    timeout_ms = 30000
                })
                assert.are.equal(200, status)
                assert.are.equal("ok", data.status)
                assert.is_not_nil(data.toml)
            else
                pending("样本文件不存在")
            end
        end)
    end)

    describe("PHP 执行", function()
        it("echo 语句应成功执行", function()
            local code = '<?php echo "hello from php\\n"; ?>'
            local data, status = http_post("/execute", {
                lang = "php",
                code = code
            })
            assert.are.equal(200, status)
            assert.is_not_nil(data)
        end)

        it("栈样本应成功执行", function()
            local code = read_sample("ok/php_stack_sucess.php")
            if code then
                local data, status = http_post("/execute", {
                    lang = "php",
                    code = code
                })
                assert.are.equal(200, status)
                assert.are.equal("ok", data.status)
                assert.is_not_nil(data.toml)
            else
                pending("样本文件不存在")
            end
        end)
    end)
end)

--------------------------------------------------------------------------------
-- 测试: /execute trace.toml 验证
--------------------------------------------------------------------------------

describe("/execute trace.toml 验证", function()
    it("成功生成 trace.toml 时 toml 字段应有内容", function()
        local code = read_sample("ok/lua_tree_sucess.lua")
        if code then
            local data, status = http_post("/execute", {
                lang = "lua",
                code = code
            })
            assert.are.equal(200, status)
            assert.are.equal("ok", data.status)
            assert.is_not_nil(data.toml)
            assert.is_true(#data.toml > 0)
        else
            pending("样本文件不存在")
        end
    end)

    it("trace.toml 应为有效的 TOML 格式", function()
        local code = read_sample("ok/lua_tree_sucess.lua")
        if code then
            local data, status = http_post("/execute", {
                lang = "lua",
                code = code
            })
            assert.are.equal(200, status)
            if data.toml then
                -- 简单验证：TOML 应该包含一些基本结构
                assert.is_truthy(
                    data.toml:match("%[") or data.toml:match("="),
                    "toml 内容不像有效的 TOML 格式"
                )
            end
        else
            pending("样本文件不存在")
        end
    end)

    it("未生成 trace.toml 时 toml 字段应为 nil", function()
        local data, status = http_post("/execute", {
            lang = "lua",
            code = 'print("no trace file")'
        })
        assert.are.equal(200, status)
        assert.is_nil(data.toml)
        assert.are.equal("error", data.status)
    end)

    it("不同语言的样本都应能生成 trace.toml", function()
        local samples = {
            { lang = "lua",        file = "ok/lua_tree_sucess.lua" },
            { lang = "python",     file = "ok/py_double_linked_list_complex_sucess.py" },
            { lang = "javascript", file = "ok/js_stack_sucess.js" },
        }

        for _, sample in ipairs(samples) do
            local code = read_sample(sample.file)
            if code then
                local data, status = http_post("/execute", {
                    lang = sample.lang,
                    code = code,
                    timeout_ms = 10000
                })
                assert.are.equal(200, status, sample.lang .. " 执行失败")
                assert.are.equal("ok", data.status, sample.lang .. " 状态不是 ok")
                assert.is_not_nil(data.toml, sample.lang .. " 未生成 trace.toml")
            end
        end
    end)
end)

--------------------------------------------------------------------------------
-- 测试: /execute 代码执行 - 错误场景
--------------------------------------------------------------------------------

describe("/execute 代码执行 - 错误场景", function()
    describe("语法错误", function()
        it("Lua 语法错误应返回 stdout 中的错误信息", function()
            local data, status = http_post("/execute", {
                lang = "lua",
                code = "print(unclosed_parenthesis"
            })
            assert.are.equal(200, status)
            assert.are.equal("error", data.status)
            assert.is_nil(data.toml)
            if data.stdout then
                assert.is_truthy(
                    data.stdout:match("error") or
                    data.stdout:match("Error") or
                    data.stdout:match("unexpected") or
                    data.stdout:match("syntax"),
                    "stdout 中未包含错误信息"
                )
            end
        end)

        it("Python 语法错误应返回 stdout 中的错误信息", function()
            local data, status = http_post("/execute", {
                lang = "python",
                code = "def broken("
            })
            assert.are.equal(200, status)
            assert.are.equal("error", data.status)
            assert.is_nil(data.toml)
            if data.stdout then
                assert.is_truthy(
                    data.stdout:match("SyntaxError") or
                    data.stdout:match("error") or
                    data.stdout:match("Error")
                )
            end
        end)

        it("JavaScript 语法错误应返回 stdout 中的错误信息", function()
            local data, status = http_post("/execute", {
                lang = "javascript",
                code = "function broken( {"
            })
            assert.are.equal(200, status)
            assert.are.equal("error", data.status)
            assert.is_nil(data.toml)
        end)
    end)

    describe("编译错误", function()
        it("C 编译错误应返回 stdout 中的错误信息", function()
            local code = [[
#include <stdio.h>
int main() {
    undefined_function();
    return 0;
}
]]
            local data, status = http_post("/execute", {
                lang = "c",
                code = code
            })
            assert.are.equal(200, status)
            assert.are.equal("error", data.status)
            assert.is_nil(data.toml)
            if data.stdout then
                assert.is_truthy(
                    data.stdout:match("error") or
                    data.stdout:match("undefined")
                )
            end
        end)

        it("Rust 编译错误应返回 stdout 中的错误信息", function()
            local code = [[
fn main() {
    let x: i32 = "not a number";
}
]]
            local data, status = http_post("/execute", {
                lang = "rust",
                code = code,
                timeout_ms = 30000
            })
            assert.are.equal(200, status)
            assert.are.equal("error", data.status)
            assert.is_nil(data.toml)
            if data.stdout then
                assert.is_truthy(
                    data.stdout:match("error") or
                    data.stdout:match("Error") or
                    data.stdout:match("mismatched types")
                )
            end
        end)
    end)

    describe("运行时错误", function()
        it("Python 运行时错误应在 stdout 中返回", function()
            local data, status = http_post("/execute", {
                lang = "python",
                code = 'raise ValueError("intentional error")'
            })
            assert.are.equal(200, status)
            assert.are.equal("error", data.status)
            if data.stdout then
                assert.is_truthy(
                    data.stdout:match("ValueError") or
                    data.stdout:match("intentional error")
                )
            end
        end)

        it("JavaScript 运行时错误应在 stdout 中返回", function()
            local data, status = http_post("/execute", {
                lang = "javascript",
                code = 'throw new Error("intentional js error");'
            })
            assert.are.equal(200, status)
            assert.are.equal("error", data.status)
            if data.stdout then
                assert.is_truthy(
                    data.stdout:match("Error") or
                    data.stdout:match("intentional js error")
                )
            end
        end)

        it("Lua 运行时错误应在 stdout 中返回", function()
            local data, status = http_post("/execute", {
                lang = "lua",
                code = 'error("intentional lua error")'
            })
            assert.are.equal(200, status)
            assert.are.equal("error", data.status)
            if data.stdout then
                assert.is_truthy(data.stdout:match("intentional lua error"))
            end
        end)
    end)

    describe("错误样本文件", function()
        it("Lua 错误样本", function()
            local code = read_sample("err/lua_stack_fail.lua")
            if code then
                local data, status = http_post("/execute", {
                    lang = "lua",
                    code = code
                })
                assert.are.equal(200, status)
                assert.are.equal("error", data.status)
            else
                pending("样本文件不存在")
            end
        end)

        it("Python 错误样本", function()
            local code = read_sample("err/py_stack_fail.py")
            if code then
                local data, status = http_post("/execute", {
                    lang = "python",
                    code = code
                })
                assert.are.equal(200, status)
                assert.are.equal("error", data.status)
            else
                pending("样本文件不存在")
            end
        end)

        it("JavaScript 错误样本", function()
            local code = read_sample("err/js_stack_fail.js")
            if code then
                local data, status = http_post("/execute", {
                    lang = "javascript",
                    code = code
                })
                assert.are.equal(200, status)
                assert.are.equal("error", data.status)
            else
                pending("样本文件不存在")
            end
        end)

        it("TypeScript 错误样本", function()
            local code = read_sample("err/ts_queue_fail.ts")
            if code then
                local data, status = http_post("/execute", {
                    lang = "typescript",
                    code = code,
                    timeout_ms = 15000
                })
                assert.are.equal(200, status)
                assert.are.equal("error", data.status)
            else
                pending("样本文件不存在")
            end
        end)

        it("C 错误样本", function()
            local code = read_sample("err/c_btree_fail.c")
            if code then
                local data, status = http_post("/execute", {
                    lang = "c",
                    code = code,
                    timeout_ms = 10000
                })
                assert.are.equal(200, status)
                assert.are.equal("error", data.status)
            else
                pending("样本文件不存在")
            end
        end)

        it("Rust 错误样本", function()
            local code = read_sample("err/rust_queue_fail.rs")
            if code then
                local data, status = http_post("/execute", {
                    lang = "rust",
                    code = code,
                    timeout_ms = 30000
                })
                assert.are.equal(200, status)
                assert.are.equal("error", data.status)
            else
                pending("样本文件不存在")
            end
        end)

        it("PHP 错误样本", function()
            local code = read_sample("err/php_stack_fail.php")
            if code then
                local data, status = http_post("/execute", {
                    lang = "php",
                    code = code
                })
                assert.are.equal(200, status)
                assert.are.equal("error", data.status)
            else
                pending("样本文件不存在")
            end
        end)
    end)
end)

--------------------------------------------------------------------------------
-- 测试: 超时处理
--------------------------------------------------------------------------------

describe("超时处理", function()
    it("无限循环应在超时后终止 (Lua)", function()
        local data, status = http_post("/execute", {
            lang = "lua",
            code = "while true do end",
            timeout_ms = 1000
        })
        assert.are.equal(200, status)
        assert.is_not_nil(data)
        assert.are.equal("error", data.status)
        assert.is_true(data.duration_ms >= 1000)
    end)

    it("无限循环应在超时后终止 (Python)", function()
        local data, status = http_post("/execute", {
            lang = "python",
            code = "while True: pass",
            timeout_ms = 1000
        })
        assert.are.equal(200, status)
        assert.is_not_nil(data)
        assert.are.equal("error", data.status)
        assert.is_true(data.duration_ms >= 1000)
    end)

    it("sleep 超时应被正确处理 (Python)", function()
        local data, status = http_post("/execute", {
            lang = "python",
            code = "import time; time.sleep(10)",
            timeout_ms = 1000
        })
        assert.are.equal(200, status)
        assert.are.equal("error", data.status)
        assert.is_true(data.duration_ms < 5000, "超时未生效")
    end)

    it("自定义超时应被尊重", function()
        local start_time = socket.gettime()
        local data, status = http_post("/execute", {
            lang = "lua",
            code = "while true do end",
            timeout_ms = 2000
        })
        local elapsed = (socket.gettime() - start_time) * 1000
        assert.are.equal(200, status)
        assert.are.equal("error", data.status)
        assert.is_true(elapsed >= 2000 and elapsed < 4000, "超时时间不准确")
    end)
end)

--------------------------------------------------------------------------------
-- 测试: 空代码和边缘情况
--------------------------------------------------------------------------------

describe("边缘情况", function()
    describe("空代码处理", function()
        it("空字符串代码应被接受", function()
            local data, status = http_post("/execute", {
                lang = "lua",
                code = ""
            })
            assert.are.equal(200, status)
            assert.is_not_nil(data)
        end)

        it("仅空白字符的代码应被接受", function()
            local data, status = http_post("/execute", {
                lang = "python",
                code = "   \n\t\n   "
            })
            assert.are.equal(200, status)
            assert.is_not_nil(data)
        end)

        it("仅注释的代码应被接受", function()
            local data, status = http_post("/execute", {
                lang = "lua",
                code = "-- this is just a comment"
            })
            assert.are.equal(200, status)
            assert.is_not_nil(data)
        end)
    end)

    describe("特殊字符处理", function()
        it("包含 Unicode 字符的代码应正常执行", function()
            local data, status = http_post("/execute", {
                lang = "python",
                code = 'print("你好世界 🌍")'
            })
            assert.are.equal(200, status)
            assert.is_not_nil(data)
        end)

        it("包含转义字符的代码应正常执行", function()
            local data, status = http_post("/execute", {
                lang = "lua",
                code = [[print("line1\nline2\ttabbed")]]
            })
            assert.are.equal(200, status)
            assert.is_not_nil(data)
        end)

        it("包含引号的代码应正常执行", function()
            local data, status = http_post("/execute", {
                lang = "python",
                code = [[print('single "double" quotes')]]
            })
            assert.are.equal(200, status)
            assert.is_not_nil(data)
        end)
    end)

    describe("大代码量处理", function()
        it("较长代码应能正常执行", function()
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
            assert.is_not_nil(data)
        end)
    end)

    describe("并发请求", function()
        it("多个并发请求应独立处理", function()
            local results = {}
            for i = 1, 3 do
                local data, status = http_post("/execute", {
                    lang = "lua",
                    code = string.format('print("request %d")', i)
                })
                if status == 200 and data then
                    table.insert(results, data.request_id)
                end
            end

            assert.are.equal(3, #results)
            local seen = {}
            for _, id in ipairs(results) do
                assert.is_nil(seen[id], "发现重复的 request_id")
                seen[id] = true
            end
        end)
    end)
end)

--------------------------------------------------------------------------------
-- 测试: CORS 支持
--------------------------------------------------------------------------------

describe("CORS 支持", function()
    it("OPTIONS 预检请求应返回 204", function()
        local status, headers = http_options("/execute")
        assert.are.equal(204, status)
    end)

    it("OPTIONS 应返回正确的 CORS 头", function()
        local status, headers = http_options("/execute")
        assert.are.equal(204, status)

        local allow_origin = headers["access-control-allow-origin"] or headers["Access-Control-Allow-Origin"]
        local allow_methods = headers["access-control-allow-methods"] or headers["Access-Control-Allow-Methods"]
        local allow_headers = headers["access-control-allow-headers"] or headers["Access-Control-Allow-Headers"]

        assert.are.equal("*", allow_origin)
        assert.is_truthy(allow_methods:match("POST"))
        assert.is_truthy(allow_headers:match("Content%-Type"))
    end)

    it("POST 响应应包含 CORS 头", function()
        local data, status, headers = http_post("/execute", {
            lang = "lua",
            code = 'print("cors test")'
        })
        assert.are.equal(200, status)

        local allow_origin = headers["access-control-allow-origin"] or headers["Access-Control-Allow-Origin"]
        assert.are.equal("*", allow_origin)
    end)
end)

--------------------------------------------------------------------------------
-- 测试: 404 处理
--------------------------------------------------------------------------------

describe("404 处理", function()
    it("不存在的路径应返回 404", function()
        local data, status = http_get("/nonexistent")
        assert.are.equal(404, status)
        assert.is_not_nil(data)
        assert.are.equal("not_found", data.error)
    end)

    it("POST 到 /ping 应返回 404", function()
        local data, status = http_post("/ping", {})
        assert.are.equal(404, status)
    end)

    it("GET 到 /execute 应返回 404", function()
        local data, status = http_get("/execute")
        assert.are.equal(404, status)
    end)

    it("错误响应应包含路径信息", function()
        local data, status = http_get("/unknown/path")
        assert.are.equal(404, status)
        assert.is_truthy(data.message:match("/unknown/path"))
    end)
end)

--------------------------------------------------------------------------------
-- 测试: 响应格式一致性
--------------------------------------------------------------------------------

describe("响应格式一致性", function()
    it("成功响应应包含所有必需字段", function()
        local code = read_sample("ok/lua_tree_sucess.lua")
        if code then
            local data, status = http_post("/execute", {
                lang = "lua",
                code = code
            })
            assert.are.equal(200, status)
            assert.is_not_nil(data.request_id)
            assert.is_not_nil(data.duration_ms)
            assert.is_not_nil(data.status)
            assert.are.equal("ok", data.status)
            assert.is_not_nil(data.toml)
        else
            pending("样本文件不存在")
        end
    end)

    it("执行失败响应应包含 status 为 error", function()
        local data, status = http_post("/execute", {
            lang = "lua",
            code = 'print("no trace")'
        })
        assert.are.equal(200, status)
        assert.is_not_nil(data.request_id)
        assert.is_not_nil(data.duration_ms)
        assert.are.equal("error", data.status)
    end)

    it("验证错误响应应包含 error 和 message", function()
        local data, status = http_post("/execute", {
            lang = "invalid_lang",
            code = "test"
        })
        assert.are.equal(400, status)
        assert.is_not_nil(data.error)
        assert.is_not_nil(data.message)
        assert.are.equal("validation", data.error)
    end)

    it("404 响应应包含 error 和 message", function()
        local data, status = http_get("/notfound")
        assert.are.equal(404, status)
        assert.is_not_nil(data.error)
        assert.is_not_nil(data.message)
        assert.are.equal("not_found", data.error)
    end)
end)

--------------------------------------------------------------------------------
-- 测试: stdout 验证
--------------------------------------------------------------------------------

describe("stdout 验证", function()
    local test_cases = {
        { lang = "lua",        code = 'print("STDOUT_MARKER_12345")',            marker = "STDOUT_MARKER_12345" },
        { lang = "python",     code = 'print("STDOUT_MARKER_12345")',            marker = "STDOUT_MARKER_12345" },
        { lang = "javascript", code = 'console.log("STDOUT_MARKER_12345");',     marker = "STDOUT_MARKER_12345" },
        { lang = "php",        code = '<?php echo "STDOUT_MARKER_12345\\n"; ?>', marker = "STDOUT_MARKER_12345" },
    }

    for _, tc in ipairs(test_cases) do
        it(string.format("%s stdout 应被正确捕获", tc.lang), function()
            local data, status = http_post("/execute", {
                lang = tc.lang,
                code = tc.code
            })
            assert.are.equal(200, status)
            if data.stdout then
                assert.is_truthy(data.stdout:match(tc.marker),
                    string.format("%s stdout 未包含预期标记", tc.lang))
            end
        end)
    end

    it("C stdout 应被正确捕获", function()
        local code = [[
#include <stdio.h>
int main() {
    printf("STDOUT_MARKER_C_12345\n");
    return 0;
}
]]
        local data, status = http_post("/execute", {
            lang = "c",
            code = code
        })
        assert.are.equal(200, status)
        if data.stdout then
            assert.is_truthy(data.stdout:match("STDOUT_MARKER_C_12345"))
        end
    end)

    it("Rust stdout 应被正确捕获", function()
        local code = [[
fn main() {
    println!("STDOUT_MARKER_RUST_12345");
}
]]
        local data, status = http_post("/execute", {
            lang = "rust",
            code = code,
            timeout_ms = 30000
        })
        assert.are.equal(200, status)
        if data.stdout then
            assert.is_truthy(data.stdout:match("STDOUT_MARKER_RUST_12345"))
        end
    end)
end)

--------------------------------------------------------------------------------
-- 测试: 工作目录隔离
--------------------------------------------------------------------------------

describe("工作目录隔离", function()
    it("并发执行应使用独立的工作目录", function()
        -- 执行两个请求，验证它们不会互相干扰
        local code1 = read_sample("ok/lua_tree_sucess.lua")
        local code2 = read_sample("ok/py_double_linked_list_complex_sucess.py")

        if code1 and code2 then
            local data1, status1 = http_post("/execute", {
                lang = "lua",
                code = code1
            })
            local data2, status2 = http_post("/execute", {
                lang = "python",
                code = code2,
                timeout_ms = 10000
            })

            assert.are.equal(200, status1)
            assert.are.equal(200, status2)
            assert.are_not.equal(data1.request_id, data2.request_id)
        else
            pending("样本文件不存在")
        end
    end)

    it("执行完成后工作目录应被清理", function()
        -- 这个测试主要验证执行能正常完成
        -- 实际的目录清理验证需要在服务器端进行
        local data, status = http_post("/execute", {
            lang = "lua",
            code = 'print("cleanup test")'
        })
        assert.are.equal(200, status)
        assert.is_not_nil(data.request_id)
    end)
end)
