# Pegasus HTTP 服务器框架使用指南

## 概述

Pegasus 是一个轻量级的 Lua HTTP 服务器框架，适用于构建简单的 Web 服务和 API。

## 安装

```bash
luarocks install pegasus
```

## 基础用法

### 创建服务器

```lua
local Pegasus = require("pegasus")

local server = Pegasus:new({
    host = "0.0.0.0",
    port = "8080"
})

server:start(function(req, resp)
    resp:write("Hello, World!")
end)
```

## Request 对象

### 获取请求信息

| 方法 | 说明 | 返回值 |
|------|------|--------|
| `req:method()` | 获取 HTTP 方法 | `"GET"`, `"POST"`, `"PUT"`, `"DELETE"` 等 |
| `req:path()` | 获取请求路径 | 字符串，如 `"/api/users"` |
| `req:headers()` | 获取所有请求头 | table（键名为小写） |
| `req:ip` | 获取客户端 IP | 字符串 |

### 获取请求头

```lua
local headers = req:headers()

-- 注意：Pegasus 将所有请求头键名转为小写
local content_type = headers["content-type"]
local content_length = headers["content-length"]
local auth = headers["authorization"]
local custom_header = headers["x-custom-header"]
```

### 读取请求体

```lua
-- 获取 Content-Length
local headers = req:headers()
local content_length = tonumber(headers["content-length"]) or 0

-- 读取原始请求体（用于 JSON 等）
local body = req:receiveBody(content_length)

-- 解析 JSON
local cjson = require("cjson")
local data = cjson.decode(body)
```

### 获取表单数据

```lua
-- 获取 URL 编码的表单数据 (application/x-www-form-urlencoded)
local post_data = req:post()
-- 返回 table: { field1 = "value1", field2 = "value2" }

local username = post_data["username"]
local password = post_data["password"]
```

### 获取查询参数

```lua
-- 对于 URL: /search?q=lua&page=1
local query_params = req:params()
-- 返回 table: { q = "lua", page = "1" }
```

## Response 对象

### 设置状态码

```lua
resp:statusCode(200)  -- OK
resp:statusCode(201)  -- Created
resp:statusCode(204)  -- No Content
resp:statusCode(400)  -- Bad Request
resp:statusCode(404)  -- Not Found
resp:statusCode(500)  -- Internal Server Error
```

### 设置响应头

```lua
resp:addHeader("Content-Type", "application/json; charset=utf-8")
resp:addHeader("Cache-Control", "no-cache")
resp:addHeader("X-Custom-Header", "custom-value")

-- CORS 相关
resp:addHeader("Access-Control-Allow-Origin", "*")
resp:addHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS")
resp:addHeader("Access-Control-Allow-Headers", "Content-Type")
```

### 写入响应体

```lua
-- 写入文本
resp:write("Hello, World!")

-- 写入 JSON
local cjson = require("cjson")
resp:write(cjson.encode({ status = "ok", data = result }))

-- 写入 HTML
resp:write("<html><body><h1>Welcome</h1></body></html>")
```

## 完整示例

### JSON API 服务器

```lua
local Pegasus = require("pegasus")
local cjson = require("cjson")

-- 辅助函数：读取 JSON 请求体
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
    
    local ok, data = pcall(cjson.decode, body)
    if not ok then
        return nil, "无效的 JSON"
    end
    
    return data, nil
end

-- 辅助函数：发送 JSON 响应
local function json_response(resp, status, data)
    resp:statusCode(status)
    resp:addHeader("Content-Type", "application/json; charset=utf-8")
    resp:addHeader("Access-Control-Allow-Origin", "*")
    resp:write(cjson.encode(data))
end

-- 创建服务器
local server = Pegasus:new({
    host = "0.0.0.0",
    port = "8080"
})

-- 启动服务器
server:start(function(req, resp)
    local method = req:method()
    local path = req:path()
    
    -- CORS 预检请求
    if method == "OPTIONS" then
        resp:statusCode(204)
        resp:addHeader("Access-Control-Allow-Origin", "*")
        resp:addHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS")
        resp:addHeader("Access-Control-Allow-Headers", "Content-Type")
        resp:write("")
        return
    end
    
    -- GET /ping - 健康检查
    if path == "/ping" and method == "GET" then
        json_response(resp, 200, {
            status = "ok",
            timestamp = os.date("!%Y-%m-%dT%H:%M:%SZ")
        })
        return
    end
    
    -- POST /api/echo - 回显 JSON
    if path == "/api/echo" and method == "POST" then
        local data, err = read_json_body(req)
        
        if not data then
            json_response(resp, 400, { error = err })
            return
        end
        
        json_response(resp, 200, {
            received = data,
            echoed_at = os.date("!%Y-%m-%dT%H:%M:%SZ")
        })
        return
    end
    
    -- GET /api/users?page=1 - 查询参数示例
    if path == "/api/users" and method == "GET" then
        local params = req:params() or {}
        local page = tonumber(params.page) or 1
        
        json_response(resp, 200, {
            page = page,
            users = { "alice", "bob", "charlie" }
        })
        return
    end
    
    -- 404 未找到
    json_response(resp, 404, {
        error = "not_found",
        message = "接口不存在: " .. method .. " " .. path
    })
end)
```

### 静态文件服务器

```lua
local Pegasus = require("pegasus")

local MIME_TYPES = {
    html = "text/html",
    css = "text/css",
    js = "application/javascript",
    json = "application/json",
    png = "image/png",
    jpg = "image/jpeg",
    gif = "image/gif",
    svg = "image/svg+xml",
    ico = "image/x-icon"
}

local function get_mime_type(path)
    local ext = path:match("%.([^%.]+)$")
    return MIME_TYPES[ext] or "application/octet-stream"
end

local function read_file(path)
    local file = io.open(path, "rb")
    if not file then
        return nil
    end
    local content = file:read("*a")
    file:close()
    return content
end

local server = Pegasus:new({
    host = "0.0.0.0",
    port = "8080"
})

local STATIC_DIR = "./public"

server:start(function(req, resp)
    local path = req:path()
    
    -- 默认首页
    if path == "/" then
        path = "/index.html"
    end
    
    local file_path = STATIC_DIR .. path
    local content = read_file(file_path)
    
    if content then
        resp:statusCode(200)
        resp:addHeader("Content-Type", get_mime_type(path))
        resp:write(content)
    else
        resp:statusCode(404)
        resp:addHeader("Content-Type", "text/html")
        resp:write("<h1>404 Not Found</h1>")
    end
end)
```

## 常见模式

### 路由分发

```lua
local routes = {
    ["GET:/ping"] = handle_ping,
    ["GET:/api/users"] = handle_get_users,
    ["POST:/api/users"] = handle_create_user,
    ["GET:/api/users/:id"] = handle_get_user,
    ["DELETE:/api/users/:id"] = handle_delete_user
}

server:start(function(req, resp)
    local method = req:method()
    local path = req:path()
    local key = method .. ":" .. path
    
    local handler = routes[key]
    if handler then
        handler(req, resp)
    else
        handle_not_found(req, resp)
    end
end)
```

### 中间件模式

```lua
local function with_cors(handler)
    return function(req, resp)
        resp:addHeader("Access-Control-Allow-Origin", "*")
        resp:addHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS")
        resp:addHeader("Access-Control-Allow-Headers", "Content-Type")
        
        if req:method() == "OPTIONS" then
            resp:statusCode(204)
            resp:write("")
            return
        end
        
        handler(req, resp)
    end
end

local function with_logging(handler)
    return function(req, resp)
        local start = os.time()
        handler(req, resp)
        local duration = os.time() - start
        print(string.format("[%s] %s %s (%ds)",
            os.date("%Y-%m-%d %H:%M:%S"),
            req:method(),
            req:path(),
            duration
        ))
    end
end

-- 使用中间件
server:start(with_logging(with_cors(function(req, resp)
    -- 实际处理逻辑
end)))
```

### 错误处理

```lua
local function safe_handler(handler)
    return function(req, resp)
        local ok, err = pcall(handler, req, resp)
        if not ok then
            print("Error: " .. tostring(err))
            resp:statusCode(500)
            resp:addHeader("Content-Type", "application/json")
            resp:write(cjson.encode({
                error = "internal_error",
                message = "服务器内部错误"
            }))
        end
    end
end

server:start(safe_handler(function(req, resp)
    -- 可能抛出错误的代码
end))
```

## 注意事项

1. **请求头键名小写**：Pegasus 将所有请求头键名转换为小写，使用 `headers["content-type"]` 而非 `headers["Content-Type"]`

2. **JSON 请求体**：使用 `req:receiveBody(content_length)` 读取原始请求体，而非 `req:post()`

3. **表单数据**：`req:post()` 仅适用于 `application/x-www-form-urlencoded` 格式

4. **必须写入响应**：即使是 204 状态码，也建议调用 `resp:write("")`

5. **单线程模型**：Pegasus 是单线程的，长时间运行的操作会阻塞其他请求

6. **生产环境**：建议在生产环境中使用反向代理（如 Nginx）前置

## 依赖库

| 库 | 用途 | 安装命令 |
|----|------|----------|
| pegasus | HTTP 服务器 | `luarocks install pegasus` |
| cjson | JSON 解析 | `luarocks install lua-cjson` |
| luasocket | 网络通信（Pegasus 依赖） | `luarocks install luasocket` |

## API 速查表

```
Request 方法:
  req:method()              -> string     HTTP 方法
  req:path()                -> string     请求路径
  req:headers()             -> table      请求头（键小写）
  req:params()              -> table      查询参数
  req:post()                -> table      表单数据
  req:receiveBody(len)      -> string     原始请求体
  req.ip                    -> string     客户端 IP

Response 方法:
  resp:statusCode(code)     -> self       设置状态码
  resp:addHeader(k, v)      -> self       添加响应头
  resp:write(content)       -> nil        写入响应体
```
