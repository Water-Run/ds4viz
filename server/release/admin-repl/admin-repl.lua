--[[
ds4viz 后台管理工具

一个简单的命令行交互式后台管理工具，用于直接操作ds4viz项目的PostgreSQL数据库。

Author:
    WaterRun
File:
    ds4viz/server/release/admin-repl/admin-repl.lua
Date:
    2026-02-04
]]

os.setlocale("", "all")

local pgmoon = require("pgmoon")

--@description: 应用程序名称
local APP_NAME = "ds4viz 后台管理工具"

--@description: 版本号
local VERSION = "2.0.0"

--@description: 配置文件路径
local CONFIG_FILE = "admin-repl.ini"

--@description: 日志文件路径
local LOG_FILE = "admin-repl.log"

--@description: 日志级别
local LogLevel = {
    DEBUG = "DEBUG",
    INFO = "INFO",
    WARN = "WARN",
    ERROR = "ERROR"
}

--@description: 用户状态
local UserStatus = {
    ACTIVE = "Active",
    BANNED = "Banned",
    SUSPENDED = "Suspended"
}

--@description: 全局配置表
local g_config = {
    allow_write = false,
    allow_danger = false,
    check_password = true,
    database = {
        host = "localhost",
        port = 5432,
        usr = "",
        pwd = "",
        prod_db = "",
        test_db = ""
    }
}

--@description: 全局数据库连接
local g_db = nil

--@description: 当前使用的数据库名称
local g_current_db = ""

--@description: 是否运行中
local g_running = true

-- ========== 工具函数 ==========

--@description: 写入日志
local function writeLog(level, message)
    local file = io.open(LOG_FILE, "a")
    if file then
        local timestamp = os.date("%Y-%m-%d %H:%M:%S")
        file:write(string.format("[%s] [%s] %s\n", timestamp, level, message))
        file:close()
    end
end

--@description: 去除字符串两端空白
local function trim(s)
    if not s then return "" end
    return s:match("^%s*(.-)%s*$")
end

--@description: 去除字符串两端引号
local function unquote(s)
    s = trim(s)
    if (s:sub(1, 1) == '"' and s:sub(-1) == '"') or
        (s:sub(1, 1) == "'" and s:sub(-1) == "'") then
        return s:sub(2, -2)
    end
    return s
end

--@description: 打印带颜色的文本（ANSI转义码）
local function printColor(color, text)
    local colors = {
        red = "\27[31m",
        green = "\27[32m",
        yellow = "\27[33m",
        blue = "\27[34m",
        magenta = "\27[35m",
        cyan = "\27[36m",
        reset = "\27[0m"
    }
    io.write((colors[color] or "") .. text .. colors.reset)
end

--@description: 打印一行带颜色的文本
local function printlnColor(color, text)
    printColor(color, text .. "\n")
end

--@description: 打印信息
local function printInfo(text)
    printlnColor("cyan", "[信息] " .. text)
end

--@description: 打印成功
local function printSuccess(text)
    printlnColor("green", "[成功] " .. text)
end

--@description: 打印警告
local function printWarn(text)
    printlnColor("yellow", "[警告] " .. text)
end

--@description: 打印错误
local function printError(text)
    printlnColor("red", "[错误] " .. text)
end

--@description: 打印分隔线
local function printSeparator(char, length)
    char = char or "-"
    length = length or 60
    print(string.rep(char, length))
end

--@description: 格式化时间戳
local function formatTimestamp(timestamp)
    if not timestamp then return "-" end
    local str = tostring(timestamp)
    return str:sub(1, 19):gsub("T", " ")
end

--@description: 截断字符串
local function truncateString(s, max_len)
    if not s then return "-" end
    s = tostring(s)
    if #s <= max_len then return s end
    return s:sub(1, max_len - 3) .. "..."
end

--@description: 读取用户输入
local function readInput(prompt)
    if prompt then
        io.write(prompt)
    end
    io.flush()
    local input = io.read("*l")
    return trim(input or "")
end

--@description: 读取密码输入（不回显）
local function readPassword(prompt)
    if prompt then
        io.write(prompt)
    end
    io.flush()
    -- 尝试使用stty隐藏输入
    os.execute("stty -echo 2>/dev/null")
    local input = io.read("*l")
    os.execute("stty echo 2>/dev/null")
    print() -- 换行
    return trim(input or "")
end

--@description: 确认操作
local function confirm(message)
    io.write(message .. " (y/n): ")
    io.flush()
    local input = io.read("*l")
    input = trim(input or ""):lower()
    return input == "y" or input == "yes"
end

-- ========== 配置文件解析 ==========

--@description: 解析INI配置文件
local function parseIniFile(filepath)
    local file = io.open(filepath, "r")
    if not file then
        return nil, "无法打开配置文件: " .. filepath
    end

    local config = {
        allow_write = false,
        allow_danger = false,
        check_password = true,
        database = {}
    }

    local current_section = nil

    for line in file:lines() do
        line = trim(line)
        if line == "" or line:sub(1, 1) == ";" then
            goto continue
        end

        local section = line:match("^%[(.+)%]$")
        if section then
            current_section = section:upper()
            goto continue
        end

        local key, value = line:match("^([^=]+)=(.+)$")
        if key and value then
            key = trim(key):upper()
            value = trim(value)
            local comment_pos = value:find(";")
            if comment_pos then
                value = trim(value:sub(1, comment_pos - 1))
            end
            value = unquote(value)

            if current_section == "DATABASE" then
                local db_key = key:lower()
                if db_key == "port" then
                    config.database[db_key] = tonumber(value) or 5432
                else
                    config.database[db_key] = value
                end
            else
                if key == "ALLOW_WRITE" then
                    config.allow_write = (value == "1" or value:lower() == "true")
                elseif key == "ALLOW_DANGER" then
                    config.allow_danger = (value == "1" or value:lower() == "true")
                elseif key == "CHECK_PASSWORD" then
                    config.check_password = (value == "1" or value:lower() == "true")
                end
            end
        end

        ::continue::
    end

    file:close()
    return config, nil
end

-- ========== 数据库操作 ==========

--@description: 连接数据库
local function connectDatabase(use_prod)
    local db_name = use_prod and g_config.database.prod_db or g_config.database.test_db
    g_current_db = db_name

    local pg = pgmoon.new({
        host = g_config.database.host,
        port = g_config.database.port,
        database = db_name,
        user = g_config.database.usr,
        password = g_config.database.pwd
    })

    local ok, err = pg:connect()
    if not ok then
        writeLog(LogLevel.ERROR, "数据库连接失败: " .. tostring(err))
        return false, "数据库连接失败: " .. tostring(err)
    end

    g_db = pg
    writeLog(LogLevel.INFO, "成功连接到数据库: " .. db_name)
    return true, nil
end

--@description: 断开数据库连接
local function disconnectDatabase()
    if g_db then
        g_db:disconnect()
        g_db = nil
        writeLog(LogLevel.INFO, "数据库连接已关闭")
    end
end

--@description: 执行SQL查询
local function executeQuery(sql)
    if not g_db then
        return nil, "数据库未连接"
    end

    local result, err = g_db:query(sql)
    if not result then
        writeLog(LogLevel.ERROR, string.format("SQL执行失败: %s, 错误: %s", sql, tostring(err)))
        return nil, tostring(err)
    end

    return result, nil
end

--@description: 转义SQL字符串
local function escapeString(s)
    if not s then return "NULL" end
    if type(s) ~= "string" then s = tostring(s) end
    return "'" .. s:gsub("'", "''") .. "'"
end

-- ========== 权限检查 ==========

--@description: 检查写权限
local function checkWritePermission()
    if not g_config.allow_write then
        printError("写操作已禁用，请检查配置文件中的 ALLOW_WRITE 设置")
        return false
    end
    return true
end

--@description: 检查危险操作权限
local function checkDangerPermission()
    if not g_config.allow_danger then
        printError("危险操作已禁用，请检查配置文件中的 ALLOW_DANGER 设置")
        return false
    end
    return true
end

-- ========== 指令处理函数 ==========

--@description: 显示帮助信息
local function cmdHelp()
    print()
    printlnColor("cyan", "=== " .. APP_NAME .. " v" .. VERSION .. " ===")
    print()
    print("可用指令:")
    printSeparator()
    print("  help, h, ?         显示此帮助信息")
    print("  status, st         显示当前状态")
    print("  switch, sw         切换数据库")
    print()
    printlnColor("yellow", "用户管理:")
    print("  users, u           列出所有用户")
    print("  user <id>          查看用户详情")
    print("  ban <id>           封禁用户")
    print("  unban <id>         解封用户")
    print("  deluser <id>       删除用户 (危险)")
    print()
    printlnColor("yellow", "会话管理:")
    print("  sessions, s        列出所有会话")
    print("  kick <id>          踢出会话")
    print("  kickall            踢出所有会话 (危险)")
    print()
    printlnColor("yellow", "模板管理:")
    print("  templates, t       列出所有模板")
    print("  template <id>      查看模板详情")
    print("  addtpl             新增模板")
    print("  deltpl <id>        删除模板 (危险)")
    print()
    printlnColor("yellow", "执行记录:")
    print("  execs, e           列出执行记录")
    print("  exec <id>          查看执行详情")
    print("  clearexecs         清空执行记录 (危险)")
    print()
    printlnColor("yellow", "缓存管理:")
    print("  cache, c           查看缓存状态")
    print("  clearcache         清空缓存 (危险)")
    print()
    printlnColor("yellow", "日志查看:")
    print("  logs, l [级别]     查看系统日志")
    print("                     级别: debug/info/warn/error")
    print()
    printlnColor("yellow", "SQL查询:")
    print("  sql <语句>         执行只读SQL查询")
    print("  exec-sql <语句>    执行写入SQL语句 (需要写权限)")
    print()
    printlnColor("yellow", "其他:")
    print("  clear, cls         清屏")
    print("  exit, quit, q      退出程序")
    printSeparator()
    print()
end

--@description: 显示当前状态
local function cmdStatus()
    print()
    printlnColor("cyan", "=== 当前状态 ===")
    print()
    print("当前数据库: " .. (g_current_db ~= "" and g_current_db or "未连接"))
    print("数据库主机: " .. g_config.database.host .. ":" .. tostring(g_config.database.port))
    print("写操作权限: " .. (g_config.allow_write and "允许" or "禁止"))
    print("危险操作权限: " .. (g_config.allow_danger and "允许" or "禁止"))

    if g_db then
        local result = executeQuery("SELECT COUNT(*) as cnt FROM users")
        if result and result[1] then
            print("用户总数: " .. tostring(result[1].cnt))
        end
        result = executeQuery("SELECT COUNT(*) as cnt FROM sessions")
        if result and result[1] then
            print("活跃会话: " .. tostring(result[1].cnt))
        end
        result = executeQuery("SELECT COUNT(*) as cnt FROM templates")
        if result and result[1] then
            print("模板总数: " .. tostring(result[1].cnt))
        end
    end
    print()
end

--@description: 切换数据库
local function cmdSwitch()
    print()
    print("选择数据库:")
    print("  1. 生产环境 (" .. g_config.database.prod_db .. ")")
    print("  2. 测试环境 (" .. g_config.database.test_db .. ")")
    print()

    local choice = readInput("请选择 (1/2): ")

    if choice == "1" then
        disconnectDatabase()
        local ok, err = connectDatabase(true)
        if ok then
            printSuccess("已切换到生产数据库: " .. g_current_db)
        else
            printError(err)
        end
    elseif choice == "2" then
        disconnectDatabase()
        local ok, err = connectDatabase(false)
        if ok then
            printSuccess("已切换到测试数据库: " .. g_current_db)
        else
            printError(err)
        end
    else
        printWarn("已取消")
    end
end

--@description: 列出用户
local function cmdUsers()
    local result, err = executeQuery([[
        SELECT id, username, status, created_at
        FROM users ORDER BY id
    ]])

    if not result then
        printError("查询失败: " .. (err or "未知错误"))
        return
    end

    print()
    printlnColor("cyan", "=== 用户列表 ===")
    print()
    print(string.format("%-6s %-20s %-12s %-20s", "ID", "用户名", "状态", "创建时间"))
    printSeparator()

    if #result == 0 then
        print("暂无用户数据")
    else
        for _, user in ipairs(result) do
            local status_color = user.status == "Active" and "green" or "red"
            io.write(string.format("%-6s %-20s ", tostring(user.id), truncateString(user.username, 18)))
            printColor(status_color, string.format("%-12s", user.status or "-"))
            print(" " .. formatTimestamp(user.created_at))
        end
    end
    printSeparator()
    print("共 " .. #result .. " 条记录")
    print()
end

--@description: 查看用户详情
local function cmdUserDetail(user_id)
    if not user_id or user_id == "" then
        printError("请指定用户ID，用法: user <id>")
        return
    end

    local id = tonumber(user_id)
    if not id then
        printError("无效的用户ID")
        return
    end

    local result, err = executeQuery(string.format([[
        SELECT id, username, status, created_at, updated_at
        FROM users WHERE id = %d
    ]], id))

    if not result or #result == 0 then
        printError("用户不存在")
        return
    end

    local user = result[1]

    print()
    printlnColor("cyan", "=== 用户详情 ===")
    print()
    print("用户ID:   " .. tostring(user.id))
    print("用户名:   " .. (user.username or "-"))
    print("状态:     " .. (user.status or "-"))
    print("创建时间: " .. formatTimestamp(user.created_at))
    print("更新时间: " .. formatTimestamp(user.updated_at))

    -- 统计信息
    local exec_result = executeQuery(string.format(
        "SELECT COUNT(*) as cnt FROM executions WHERE user_id = %d", id))
    if exec_result and exec_result[1] then
        print("执行记录: " .. tostring(exec_result[1].cnt) .. " 条")
    end

    local fav_result = executeQuery(string.format(
        "SELECT COUNT(*) as cnt FROM user_favorites WHERE user_id = %d", id))
    if fav_result and fav_result[1] then
        print("收藏模板: " .. tostring(fav_result[1].cnt) .. " 个")
    end

    print()
end

--@description: 封禁用户
local function cmdBanUser(user_id)
    if not checkWritePermission() then return end

    if not user_id or user_id == "" then
        printError("请指定用户ID，用法: ban <id>")
        return
    end

    local id = tonumber(user_id)
    if not id then
        printError("无效的用户ID")
        return
    end

    -- 检查用户是否存在
    local check = executeQuery(string.format("SELECT username, status FROM users WHERE id = %d", id))
    if not check or #check == 0 then
        printError("用户不存在")
        return
    end

    local user = check[1]
    if user.status == UserStatus.BANNED then
        printWarn("用户已处于封禁状态")
        return
    end

    if not confirm("确定要封禁用户 " .. user.username .. " 吗?") then
        printInfo("已取消")
        return
    end

    local sql = string.format(
        "UPDATE users SET status = %s, updated_at = NOW() WHERE id = %d",
        escapeString(UserStatus.BANNED), id)

    local result, err = executeQuery(sql)
    if result then
        writeLog(LogLevel.INFO, string.format("封禁用户 id=%d username=%s", id, user.username))
        printSuccess("用户 " .. user.username .. " 已被封禁")
    else
        printError("操作失败: " .. (err or "未知错误"))
    end
end

--@description: 解封用户
local function cmdUnbanUser(user_id)
    if not checkWritePermission() then return end

    if not user_id or user_id == "" then
        printError("请指定用户ID，用法: unban <id>")
        return
    end

    local id = tonumber(user_id)
    if not id then
        printError("无效的用户ID")
        return
    end

    local check = executeQuery(string.format("SELECT username, status FROM users WHERE id = %d", id))
    if not check or #check == 0 then
        printError("用户不存在")
        return
    end

    local user = check[1]
    if user.status == UserStatus.ACTIVE then
        printWarn("用户已处于正常状态")
        return
    end

    if not confirm("确定要解封用户 " .. user.username .. " 吗?") then
        printInfo("已取消")
        return
    end

    local sql = string.format(
        "UPDATE users SET status = %s, updated_at = NOW() WHERE id = %d",
        escapeString(UserStatus.ACTIVE), id)

    local result, err = executeQuery(sql)
    if result then
        writeLog(LogLevel.INFO, string.format("解封用户 id=%d username=%s", id, user.username))
        printSuccess("用户 " .. user.username .. " 已解封")
    else
        printError("操作失败: " .. (err or "未知错误"))
    end
end

--@description: 删除用户
local function cmdDeleteUser(user_id)
    if not checkWritePermission() then return end
    if not checkDangerPermission() then return end

    if not user_id or user_id == "" then
        printError("请指定用户ID，用法: deluser <id>")
        return
    end

    local id = tonumber(user_id)
    if not id then
        printError("无效的用户ID")
        return
    end

    local check = executeQuery(string.format("SELECT username FROM users WHERE id = %d", id))
    if not check or #check == 0 then
        printError("用户不存在")
        return
    end

    local username = check[1].username

    printlnColor("red", "!! 危险操作 !!")
    print("即将删除用户: " .. username)
    print("此操作不可撤销，关联的会话、执行记录、收藏等数据将被级联删除")

    if not confirm("确定要删除吗?") then
        printInfo("已取消")
        return
    end

    local result, err = executeQuery(string.format("DELETE FROM users WHERE id = %d", id))
    if result then
        writeLog(LogLevel.WARN, string.format("删除用户 id=%d username=%s", id, username))
        printSuccess("用户 " .. username .. " 已删除")
    else
        printError("操作失败: " .. (err or "未知错误"))
    end
end

--@description: 列出会话
local function cmdSessions()
    local result, err = executeQuery([[
        SELECT s.id, s.user_id, u.username, s.ip_address, s.expires_at, s.created_at
        FROM sessions s
        LEFT JOIN users u ON s.user_id = u.id
        ORDER BY s.created_at DESC
        LIMIT 50
    ]])

    if not result then
        printError("查询失败: " .. (err or "未知错误"))
        return
    end

    print()
    printlnColor("cyan", "=== 会话列表 ===")
    print()
    print(string.format("%-6s %-15s %-18s %-20s", "ID", "用户", "IP地址", "过期时间"))
    printSeparator()

    if #result == 0 then
        print("暂无活跃会话")
    else
        for _, session in ipairs(result) do
            print(string.format("%-6s %-15s %-18s %-20s",
                tostring(session.id),
                truncateString(session.username or "未知", 13),
                truncateString(session.ip_address or "-", 16),
                formatTimestamp(session.expires_at)
            ))
        end
    end
    printSeparator()
    print("共 " .. #result .. " 条记录")
    print()
end

--@description: 踢出会话
local function cmdKickSession(session_id)
    if not checkWritePermission() then return end

    if not session_id or session_id == "" then
        printError("请指定会话ID，用法: kick <id>")
        return
    end

    local id = tonumber(session_id)
    if not id then
        printError("无效的会话ID")
        return
    end

    local check = executeQuery(string.format([[
        SELECT s.id, u.username FROM sessions s
        LEFT JOIN users u ON s.user_id = u.id
        WHERE s.id = %d
    ]], id))

    if not check or #check == 0 then
        printError("会话不存在")
        return
    end

    local username = check[1].username or "未知"

    if not confirm("确定要踢出用户 " .. username .. " 的会话吗?") then
        printInfo("已取消")
        return
    end

    local result, err = executeQuery(string.format("DELETE FROM sessions WHERE id = %d", id))
    if result then
        writeLog(LogLevel.INFO, string.format("踢出会话 id=%d", id))
        printSuccess("会话已终止")
    else
        printError("操作失败: " .. (err or "未知错误"))
    end
end

--@description: 踢出所有会话
local function cmdKickAllSessions()
    if not checkWritePermission() then return end
    if not checkDangerPermission() then return end

    local count_result = executeQuery("SELECT COUNT(*) as cnt FROM sessions")
    local count = count_result and count_result[1] and count_result[1].cnt or 0

    printlnColor("red", "!! 危险操作 !!")
    print("即将清空所有 " .. tostring(count) .. " 个会话")
    print("所有用户将被强制下线")

    if not confirm("确定要继续吗?") then
        printInfo("已取消")
        return
    end

    local result, err = executeQuery("DELETE FROM sessions")
    if result then
        writeLog(LogLevel.WARN, "清空所有会话")
        printSuccess("所有会话已清空")
    else
        printError("操作失败: " .. (err or "未知错误"))
    end
end

--@description: 列出模板
local function cmdTemplates()
    local result, err = executeQuery([[
        SELECT id, title, category, favorite_count, created_at
        FROM templates ORDER BY id
    ]])

    if not result then
        printError("查询失败: " .. (err or "未知错误"))
        return
    end

    print()
    printlnColor("cyan", "=== 模板列表 ===")
    print()
    print(string.format("%-5s %-25s %-15s %-8s", "ID", "标题", "分类", "收藏数"))
    printSeparator()

    if #result == 0 then
        print("暂无模板数据")
    else
        for _, tpl in ipairs(result) do
            print(string.format("%-5s %-25s %-15s %-8s",
                tostring(tpl.id),
                truncateString(tpl.title, 23),
                truncateString(tpl.category or "-", 13),
                tostring(tpl.favorite_count or 0)
            ))
        end
    end
    printSeparator()
    print("共 " .. #result .. " 条记录")
    print()
end

--@description: 查看模板详情
local function cmdTemplateDetail(tpl_id)
    if not tpl_id or tpl_id == "" then
        printError("请指定模板ID，用法: template <id>")
        return
    end

    local id = tonumber(tpl_id)
    if not id then
        printError("无效的模板ID")
        return
    end

    local result, err = executeQuery(string.format([[
        SELECT id, title, category, description, favorite_count, created_at, updated_at
        FROM templates WHERE id = %d
    ]], id))

    if not result or #result == 0 then
        printError("模板不存在")
        return
    end

    local tpl = result[1]

    print()
    printlnColor("cyan", "=== 模板详情 ===")
    print()
    print("模板ID:   " .. tostring(tpl.id))
    print("标题:     " .. (tpl.title or "-"))
    print("分类:     " .. (tpl.category or "-"))
    print("收藏数:   " .. tostring(tpl.favorite_count or 0))
    print("创建时间: " .. formatTimestamp(tpl.created_at))
    print("更新时间: " .. formatTimestamp(tpl.updated_at))

    -- 获取代码实现
    local codes = executeQuery(string.format(
        "SELECT language FROM template_codes WHERE template_id = %d", id))

    if codes and #codes > 0 then
        print()
        print("支持的语言:")
        for _, code in ipairs(codes) do
            print("  - " .. (code.language or "未知"))
        end
    end

    if tpl.description and #tpl.description > 0 then
        print()
        print("描述:")
        print(truncateString(tpl.description, 200))
    end

    print()
end

--@description: 新增模板
local function cmdAddTemplate()
    if not checkWritePermission() then return end

    print()
    printlnColor("cyan", "=== 新增模板 ===")
    print()

    local title = readInput("模板标题: ")
    if title == "" then
        printWarn("已取消")
        return
    end

    local category = readInput("模板分类: ")
    if category == "" then
        printWarn("已取消")
        return
    end

    local description = readInput("模板描述 (可选): ")

    if not confirm("确定创建模板 \"" .. title .. "\" 吗?") then
        printInfo("已取消")
        return
    end

    local sql = string.format(
        "INSERT INTO templates (title, category, description, favorite_count, created_at) VALUES (%s, %s, %s, 0, NOW())",
        escapeString(title),
        escapeString(category),
        escapeString(description)
    )

    local result, err = executeQuery(sql)
    if result then
        writeLog(LogLevel.INFO, string.format("创建模板: %s", title))
        printSuccess("模板创建成功")
    else
        printError("创建失败: " .. (err or "未知错误"))
    end
end

--@description: 删除模板
local function cmdDeleteTemplate(tpl_id)
    if not checkWritePermission() then return end
    if not checkDangerPermission() then return end

    if not tpl_id or tpl_id == "" then
        printError("请指定模板ID，用法: deltpl <id>")
        return
    end

    local id = tonumber(tpl_id)
    if not id then
        printError("无效的模板ID")
        return
    end

    local check = executeQuery(string.format("SELECT title FROM templates WHERE id = %d", id))
    if not check or #check == 0 then
        printError("模板不存在")
        return
    end

    local title = check[1].title

    printlnColor("red", "!! 危险操作 !!")
    print("即将删除模板: " .. title)
    print("关联的代码实现将被级联删除")

    if not confirm("确定要删除吗?") then
        printInfo("已取消")
        return
    end

    -- 先删除关联代码
    executeQuery(string.format("DELETE FROM template_codes WHERE template_id = %d", id))

    local result, err = executeQuery(string.format("DELETE FROM templates WHERE id = %d", id))
    if result then
        writeLog(LogLevel.WARN, string.format("删除模板 id=%d title=%s", id, title))
        printSuccess("模板 " .. title .. " 已删除")
    else
        printError("删除失败: " .. (err or "未知错误"))
    end
end

--@description: 列出执行记录
local function cmdExecutions()
    local result, err = executeQuery([[
        SELECT e.id, u.username, e.language, e.status, e.execution_time, e.created_at
        FROM executions e
        LEFT JOIN users u ON e.user_id = u.id
        ORDER BY e.created_at DESC
        LIMIT 50
    ]])

    if not result then
        printError("查询失败: " .. (err or "未知错误"))
        return
    end

    print()
    printlnColor("cyan", "=== 执行记录 ===")
    print()
    print(string.format("%-5s %-12s %-10s %-10s %-8s %-16s", "ID", "用户", "语言", "状态", "耗时", "时间"))
    printSeparator()

    if #result == 0 then
        print("暂无执行记录")
    else
        for _, exec in ipairs(result) do
            local time_str = exec.execution_time and (tostring(exec.execution_time) .. "ms") or "-"
            local status_color = exec.status == "Success" and "green" or "red"

            io.write(string.format("%-5s %-12s %-10s ",
                tostring(exec.id),
                truncateString(exec.username or "未知", 10),
                exec.language or "-"
            ))
            printColor(status_color, string.format("%-10s", exec.status or "-"))
            print(string.format(" %-8s %-16s", time_str, formatTimestamp(exec.created_at):sub(1, 16)))
        end
    end
    printSeparator()
    print("共 " .. #result .. " 条记录")
    print()
end

--@description: 查看执行详情
local function cmdExecDetail(exec_id)
    if not exec_id or exec_id == "" then
        printError("请指定执行ID，用法: exec <id>")
        return
    end

    local id = tonumber(exec_id)
    if not id then
        printError("无效的执行ID")
        return
    end

    local result, err = executeQuery(string.format([[
        SELECT e.*, u.username
        FROM executions e
        LEFT JOIN users u ON e.user_id = u.id
        WHERE e.id = %d
    ]], id))

    if not result or #result == 0 then
        printError("记录不存在")
        return
    end

    local exec = result[1]

    print()
    printlnColor("cyan", "=== 执行详情 ===")
    print()
    print("记录ID:   " .. tostring(exec.id))
    print("用户:     " .. (exec.username or "未知"))
    print("语言:     " .. (exec.language or "-"))
    print("状态:     " .. (exec.status or "-"))
    print("耗时:     " .. tostring(exec.execution_time or 0) .. "ms")
    print("IP地址:   " .. (exec.ip_address or "-"))
    print("执行时间: " .. formatTimestamp(exec.created_at))

    if exec.error_message and #tostring(exec.error_message) > 0 then
        print()
        printlnColor("red", "错误信息:")
        print(exec.error_message)
    end

    if exec.code and #tostring(exec.code) > 0 then
        print()
        print("源代码 (前500字符):")
        printSeparator()
        print(truncateString(exec.code, 500))
        printSeparator()
    end

    print()
end

--@description: 清空执行记录
local function cmdClearExecutions()
    if not checkWritePermission() then return end
    if not checkDangerPermission() then return end

    local count_result = executeQuery("SELECT COUNT(*) as cnt FROM executions")
    local count = count_result and count_result[1] and count_result[1].cnt or 0

    printlnColor("red", "!! 危险操作 !!")
    print("即将清空所有 " .. tostring(count) .. " 条执行记录")

    if not confirm("确定要继续吗?") then
        printInfo("已取消")
        return
    end

    local result, err = executeQuery("DELETE FROM executions")
    if result then
        writeLog(LogLevel.WARN, "清空所有执行记录")
        printSuccess("执行记录已清空")
    else
        printError("操作失败: " .. (err or "未知错误"))
    end
end

--@description: 查看缓存状态
local function cmdCache()
    local stats = executeQuery([[
        SELECT
            COUNT(*) as total_count,
            COALESCE(SUM(hit_count), 0) as total_hits,
            COALESCE(SUM(LENGTH(toml_output)), 0) as total_size
        FROM execution_cache
    ]])

    print()
    printlnColor("cyan", "=== 缓存状态 ===")
    print()

    if stats and stats[1] then
        local s = stats[1]
        local size_kb = math.floor((tonumber(s.total_size) or 0) / 1024)
        print("缓存条目: " .. tostring(s.total_count or 0))
        print("命中次数: " .. tostring(s.total_hits or 0))
        print("占用空间: " .. tostring(size_kb) .. " KB")
    end

    -- 列出最近的缓存
    local list = executeQuery([[
        SELECT id, language, hit_count, created_at, last_hit_at,
               LEFT(code_hash, 16) as hash_prefix
        FROM execution_cache
        ORDER BY last_hit_at DESC NULLS LAST
        LIMIT 10
    ]])

    if list and #list > 0 then
        print()
        print("最近的缓存条目:")
        printSeparator()
        print(string.format("%-5s %-18s %-10s %-8s", "ID", "哈希", "语言", "命中"))
        printSeparator()
        for _, cache in ipairs(list) do
            print(string.format("%-5s %-18s %-10s %-8s",
                tostring(cache.id),
                (cache.hash_prefix or "") .. "...",
                cache.language or "-",
                tostring(cache.hit_count or 0)
            ))
        end
    end

    print()
end

--@description: 清空缓存
local function cmdClearCache()
    if not checkWritePermission() then return end
    if not checkDangerPermission() then return end

    local count_result = executeQuery("SELECT COUNT(*) as cnt FROM execution_cache")
    local count = count_result and count_result[1] and count_result[1].cnt or 0

    printlnColor("red", "!! 危险操作 !!")
    print("即将清空所有 " .. tostring(count) .. " 条缓存")

    if not confirm("确定要继续吗?") then
        printInfo("已取消")
        return
    end

    local result, err = executeQuery("DELETE FROM execution_cache")
    if result then
        writeLog(LogLevel.WARN, "清空所有缓存")
        printSuccess("缓存已清空")
    else
        printError("操作失败: " .. (err or "未知错误"))
    end
end

--@description: 查看日志
local function cmdLogs(level_filter)
    local sql = [[
        SELECT id, level, module, message, created_at
        FROM logs
    ]]

    if level_filter and level_filter ~= "" then
        local level_map = {
            debug = "DEBUG",
            info = "INFO",
            warn = "WARNING",
            warning = "WARNING",
            error = "ERROR"
        }
        local level = level_map[level_filter:lower()]
        if level then
            sql = sql .. string.format(" WHERE level = %s", escapeString(level))
        end
    end

    sql = sql .. " ORDER BY created_at DESC LIMIT 50"

    local result, err = executeQuery(sql)

    if not result then
        printError("查询失败: " .. (err or "未知错误"))
        return
    end

    print()
    local title = "=== 系统日志" ..
    (level_filter and level_filter ~= "" and (" [" .. level_filter:upper() .. "]") or "") .. " ==="
    printlnColor("cyan", title)
    print()
    print(string.format("%-8s %-15s %-35s", "级别", "模块", "消息"))
    printSeparator()

    if #result == 0 then
        print("暂无日志数据")
    else
        for _, log in ipairs(result) do
            local level_color = "reset"
            if log.level == "ERROR" then
                level_color = "red"
            elseif log.level == "WARNING" then
                level_color = "yellow"
            elseif log.level == "INFO" then
                level_color = "cyan"
            end

            printColor(level_color, string.format("%-8s", log.level or "-"))
            print(string.format(" %-15s %-35s",
                truncateString(log.module or "-", 13),
                truncateString(log.message or "-", 33)
            ))
        end
    end
    printSeparator()
    print("共 " .. #result .. " 条记录")
    print()
end

--@description: 执行只读SQL
local function cmdSQL(sql)
    if not sql or sql == "" then
        printError("请提供SQL语句，用法: sql <语句>")
        return
    end

    -- 检查是否为只读查询
    local lower_sql = sql:lower():gsub("^%s+", "")
    if not lower_sql:match("^select") and not lower_sql:match("^show") and not lower_sql:match("^describe") then
        printError("sql 指令仅支持只读查询 (SELECT/SHOW/DESCRIBE)")
        printInfo("如需执行写入操作，请使用 exec-sql 指令")
        return
    end

    local result, err = executeQuery(sql)

    if not result then
        printError("查询失败: " .. (err or "未知错误"))
        return
    end

    print()

    if type(result) == "table" and #result > 0 then
        -- 获取列名
        local columns = {}
        for k, _ in pairs(result[1]) do
            table.insert(columns, k)
        end
        table.sort(columns)

        -- 打印表头
        local header = ""
        for _, col in ipairs(columns) do
            header = header .. string.format("%-20s ", truncateString(col, 18))
        end
        print(header)
        printSeparator()

        -- 打印数据
        for _, row in ipairs(result) do
            local line = ""
            for _, col in ipairs(columns) do
                local val = row[col]
                if val == nil then
                    val = "NULL"
                else
                    val = tostring(val)
                end
                line = line .. string.format("%-20s ", truncateString(val, 18))
            end
            print(line)
        end
        printSeparator()
        print("共 " .. #result .. " 条记录")
    else
        print("查询执行成功，无返回数据")
    end

    print()
end

--@description: 执行写入SQL
local function cmdExecSQL(sql)
    if not checkWritePermission() then return end

    if not sql or sql == "" then
        printError("请提供SQL语句，用法: exec-sql <语句>")
        return
    end

    printWarn("即将执行SQL: " .. truncateString(sql, 60))

    if not confirm("确定要执行吗?") then
        printInfo("已取消")
        return
    end

    local result, err = executeQuery(sql)

    if result then
        writeLog(LogLevel.WARN, "执行SQL: " .. sql)
        printSuccess("SQL执行成功")

        if type(result) == "table" and result.affected_rows then
            print("影响行数: " .. tostring(result.affected_rows))
        end
    else
        printError("执行失败: " .. (err or "未知错误"))
    end
end

--@description: 清屏
local function cmdClear()
    os.execute("clear 2>/dev/null || cls")
end

-- ========== 主循环 ==========

--@description: 解析指令和参数
local function parseCommand(input)
    local parts = {}
    -- 简单分割，保留引号内的空格
    local in_quote = false
    local current = ""

    for i = 1, #input do
        local c = input:sub(i, i)
        if c == '"' or c == "'" then
            in_quote = not in_quote
        elseif c == " " and not in_quote then
            if #current > 0 then
                table.insert(parts, current)
                current = ""
            end
        else
            current = current .. c
        end
    end

    if #current > 0 then
        table.insert(parts, current)
    end

    local cmd = parts[1] or ""
    table.remove(parts, 1)
    local args = table.concat(parts, " ")

    return cmd:lower(), args
end

--@description: 处理指令
local function processCommand(input)
    local cmd, args = parseCommand(input)

    if cmd == "" then
        return
    end

    -- 指令映射
    if cmd == "help" or cmd == "h" or cmd == "?" then
        cmdHelp()
    elseif cmd == "status" or cmd == "st" then
        cmdStatus()
    elseif cmd == "switch" or cmd == "sw" then
        cmdSwitch()
    elseif cmd == "users" or cmd == "u" then
        cmdUsers()
    elseif cmd == "user" then
        cmdUserDetail(args)
    elseif cmd == "ban" then
        cmdBanUser(args)
    elseif cmd == "unban" then
        cmdUnbanUser(args)
    elseif cmd == "deluser" then
        cmdDeleteUser(args)
    elseif cmd == "sessions" or cmd == "s" then
        cmdSessions()
    elseif cmd == "kick" then
        cmdKickSession(args)
    elseif cmd == "kickall" then
        cmdKickAllSessions()
    elseif cmd == "templates" or cmd == "t" then
        cmdTemplates()
    elseif cmd == "template" then
        cmdTemplateDetail(args)
    elseif cmd == "addtpl" then
        cmdAddTemplate()
    elseif cmd == "deltpl" then
        cmdDeleteTemplate(args)
    elseif cmd == "execs" or cmd == "e" then
        cmdExecutions()
    elseif cmd == "exec" then
        cmdExecDetail(args)
    elseif cmd == "clearexecs" then
        cmdClearExecutions()
    elseif cmd == "cache" or cmd == "c" then
        cmdCache()
    elseif cmd == "clearcache" then
        cmdClearCache()
    elseif cmd == "logs" or cmd == "l" then
        cmdLogs(args)
    elseif cmd == "sql" then
        cmdSQL(args)
    elseif cmd == "exec-sql" then
        cmdExecSQL(args)
    elseif cmd == "clear" or cmd == "cls" then
        cmdClear()
    elseif cmd == "exit" or cmd == "quit" or cmd == "q" then
        if confirm("确定要退出吗?") then
            g_running = false
        end
    else
        printError("未知指令: " .. cmd)
        print("输入 help 查看帮助")
    end
end

--@description: 显示欢迎信息
local function showWelcome()
    cmdClear()
    print()
    printlnColor("cyan", "╔════════════════════════════════════════════════════════╗")
    printlnColor("cyan", "║                                                        ║")
    printlnColor("cyan", "║           " .. APP_NAME .. " v" .. VERSION .. "            ║")
    printlnColor("cyan", "║                                                        ║")
    printlnColor("cyan", "╚════════════════════════════════════════════════════════╝")
    print()
    print("输入 help 获取帮助，输入 exit 退出")
    print()
end

--@description: 主函数
local function main()
    writeLog(LogLevel.INFO, "========== 启动后台管理工具 ==========")

    -- 加载配置
    local config, err = parseIniFile(CONFIG_FILE)
    if not config then
        printError(err or "配置文件解析失败")
        os.exit(1)
    end

    g_config = config
    writeLog(LogLevel.INFO, string.format(
        "配置已加载: allow_write=%s, allow_danger=%s, check_password=%s",
        tostring(config.allow_write),
        tostring(config.allow_danger),
        tostring(config.check_password)
    ))

    showWelcome()

    -- 密码验证
    if g_config.check_password then
        printInfo("需要验证数据库密码")
        local pwd = readPassword("请输入数据库密码: ")
        if pwd == "" then
            printError("密码不能为空")
            os.exit(1)
        end
        g_config.database.pwd = pwd
    end

    -- 选择数据库
    print()
    print("请选择数据库:")
    print("  1. 生产环境 (" .. g_config.database.prod_db .. ")")
    print("  2. 测试环境 (" .. g_config.database.test_db .. ")")
    print()

    local db_choice = readInput("请选择 (1/2): ")
    local use_prod = (db_choice == "1")

    local ok, conn_err = connectDatabase(use_prod)
    if not ok then
        printError(conn_err)
        os.exit(1)
    end

    printSuccess("已连接到数据库: " .. g_current_db)
    print()

    -- 主循环
    while g_running do
        local prompt = string.format("[%s]>>> ", g_current_db)
        printColor("green", prompt)

        local input = io.read("*l")

        if input == nil then
            -- EOF (Ctrl+D)
            print()
            g_running = false
        else
            input = trim(input)
            processCommand(input)
        end
    end

    -- 清理
    disconnectDatabase()
    writeLog(LogLevel.INFO, "后台管理工具已退出")
    print()
    printInfo("再见！")
end

main()
