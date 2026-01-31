--[[
ds4viz 后台管理TUI

一个基于ltui的后台管理终端界面，用于直接操作ds4viz项目的PostgreSQL数据库。
支持用户管理、会话管理、模板管理、执行记录、缓存管理、系统日志等功能。
通过配置文件控制写操作和危险操作的权限。

Author:
    WaterRun
File:
    ds4viz/server/runtime/admin-tui/admin-tui.lua
Date:
    2026-01-31
Updated:
    2026-01-31
]]

local ltui = require("ltui")
local pgmoon = require("pgmoon")

--@description: 应用程序名称
--@const: APP_NAME
local APP_NAME = "ds4viz 后台管理"

--@description: 版本号
--@const: VERSION
local VERSION = "1.0.0"

--@description: 配置文件路径
--@const: CONFIG_FILE
local CONFIG_FILE = "admin-tui.ini"

--@description: 日志文件路径
--@const: LOG_FILE
local LOG_FILE = "admin-tui.log"

--[[
日志级别枚举

Enum:
    LogLevel
Values:
    DEBUG: 调试信息
    INFO: 一般信息
    WARN: 警告信息
    ERROR: 错误信息
]]
local LogLevel = {
    DEBUG = "DEBUG",
    INFO = "INFO",
    WARN = "WARN",
    ERROR = "ERROR"
}

--[[
用户状态枚举

Enum:
    UserStatus
Values:
    ACTIVE: 正常状态
    BANNED: 已封禁
    SUSPENDED: 已暂停
]]
local UserStatus = {
    ACTIVE = "Active",
    BANNED = "Banned",
    SUSPENDED = "Suspended"
}

--@description: 全局配置表
--@field allow_write: boolean - 是否允许写操作
--@field allow_danger: boolean - 是否允许危险操作
--@field check_password: boolean - 是否检查密码
--@field database: table - 数据库配置
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
--@type: pgmoon|nil
local g_db = nil

--@description: 当前使用的数据库名称
--@type: string
local g_current_db = ""

--@description: 写入日志到文件
--@param level: string - 日志级别
--@param message: string - 日志内容
--@return: nil
local function writeLog(level, message)
    local file = io.open(LOG_FILE, "a")
    if file then
        local timestamp = os.date("%Y-%m-%d %H:%M:%S")
        file:write(string.format("[%s] [%s] %s\n", timestamp, level, message))
        file:close()
    end
end

--@description: 去除字符串两端空白
--@param s: string - 输入字符串
--@return: string - 去除空白后的字符串
local function trim(s)
    return s:match("^%s*(.-)%s*$")
end

--@description: 去除字符串两端引号
--@param s: string - 输入字符串
--@return: string - 去除引号后的字符串
local function unquote(s)
    s = trim(s)
    if (s:sub(1, 1) == '"' and s:sub(-1) == '"') or
        (s:sub(1, 1) == "'" and s:sub(-1) == "'") then
        return s:sub(2, -2)
    end
    return s
end

--@description: 解析INI配置文件
--@param filepath: string - 配置文件路径
--@return: table|nil - 解析后的配置表，失败返回nil
--@return: string|nil - 错误信息
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

--@description: 连接数据库
--@param use_prod: boolean - 是否使用生产数据库
--@return: boolean - 是否连接成功
--@return: string|nil - 错误信息
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
    writeLog(LogLevel.INFO, "成功连接数据库: " .. db_name)
    return true, nil
end

--@description: 断开数据库连接
--@return: nil
local function disconnectDatabase()
    if g_db then
        g_db:disconnect()
        g_db = nil
        writeLog(LogLevel.INFO, "数据库连接已断开")
    end
end

--@description: 执行SQL查询
--@param sql: string - SQL语句
--@return: table|nil - 查询结果
--@return: string|nil - 错误信息
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

--@description: 转义SQL字符串，防止注入
--@param s: string - 输入字符串
--@return: string - 转义后的字符串
local function escapeString(s)
    if not s then return "NULL" end
    if type(s) ~= "string" then s = tostring(s) end
    return "'" .. s:gsub("'", "''") .. "'"
end

--@description: 格式化时间戳显示
--@param timestamp: string|nil - 时间戳字符串
--@return: string - 格式化后的时间
local function formatTimestamp(timestamp)
    if not timestamp then return "-" end
    return timestamp:sub(1, 19):gsub("T", " ")
end

--@description: 截断字符串
--@param s: string|nil - 输入字符串
--@param max_len: number - 最大长度
--@return: string - 截断后的字符串
local function truncateString(s, max_len)
    if not s then return "-" end
    if #s <= max_len then return s end
    return s:sub(1, max_len - 3) .. "..."
end

--[[
应用程序类

基于ltui.application的扩展，实现ds4viz后台管理TUI的主要逻辑。

Class:
    AdminApp
Fields:
    _config: table - 配置信息
    _current_view: string - 当前视图名称
]]
local AdminApp = ltui.application()

--@description: 初始化应用程序
--@param self: AdminApp - 应用实例
--@return: nil
function AdminApp:init()
    ltui.application.init(self, APP_NAME)
    self._config = g_config
    self._current_view = "main"
    self._selected_db = "prod"

    writeLog(LogLevel.INFO, "应用程序初始化完成")
end

--@description: 显示启动动画
--@param self: AdminApp - 应用实例
--@param callback: function - 完成后的回调函数
--@return: nil
function AdminApp:showStartupAnimation(callback)
    local dialog = self:insert(ltui.boxdialog:new("startup_dialog", ltui.rect { 0, 0, 60, 20 }, "启动中"))
    dialog:frame():style_set("border", true)

    local logo_lines = {
        "    ██████╗ ███████╗██╗  ██╗██╗   ██╗██╗███████╗",
        "    ██╔══██╗██╔════╝██║  ██║██║   ██║██║╚══███╔╝",
        "    ██║  ██║███████╗███████║██║   ██║██║  ███╔╝ ",
        "    ██║  ██║╚════██║╚════██║╚██╗ ██╔╝██║ ███╔╝  ",
        "    ██████╔╝███████║     ██║ ╚████╔╝ ██║███████╗",
        "    ╚═════╝ ╚══════╝     ╚═╝  ╚═══╝  ╚═╝╚══════╝",
        "",
        "              后台管理系统 v" .. VERSION
    }

    local status_messages = {
        "正在加载配置文件...",
        "正在初始化界面...",
        "正在连接数据库...",
        "启动完成!"
    }

    local text_content = table.concat(logo_lines, "\n") .. "\n\n"
    local text = dialog:text()
    text:text_set(text_content)
    dialog:center(self:bounds())

    local step = 0
    local timer_id

    local function updateStatus()
        step = step + 1
        if step <= #status_messages then
            local new_content = table.concat(logo_lines, "\n") .. "\n\n"
            for i = 1, step do
                local prefix = (i == step) and "► " or "✓ "
                new_content = new_content .. prefix .. status_messages[i] .. "\n"
            end
            text:text_set(new_content)
            self:invalidate()

            if step < #status_messages then
                timer_id = self:timer_start(500, updateStatus)
            else
                timer_id = self:timer_start(800, function()
                    self:remove(dialog)
                    if callback then callback() end
                end)
            end
        end
    end

    timer_id = self:timer_start(300, updateStatus)
end

--@description: 显示密码验证对话框
--@param self: AdminApp - 应用实例
--@param callback: function - 验证成功后的回调
--@return: nil
function AdminApp:showPasswordDialog(callback)
    local dialog = self:insert(ltui.inputdialog:new(
        "pwd_dialog",
        ltui.rect { 0, 0, 50, 8 },
        "安全验证"
    ))
    dialog:frame():style_set("border", true)
    dialog:text():text_set("请输入数据库密码:")
    dialog:center(self:bounds())

    dialog:button_add("confirm", "确认", function()
        local input_pwd = dialog:textedit():text()
        if input_pwd == g_config.database.pwd then
            writeLog(LogLevel.INFO, "密码验证成功")
            self:remove(dialog)
            if callback then callback() end
        else
            writeLog(LogLevel.WARN, "密码验证失败")
            dialog:text():text_set("密码错误，请重试:")
            dialog:textedit():text_set("")
        end
    end)

    dialog:button_add("cancel", "退出", function()
        writeLog(LogLevel.INFO, "用户取消登录")
        self:quit()
    end)
end

--@description: 显示数据库选择对话框
--@param self: AdminApp - 应用实例
--@param callback: function - 选择后的回调
--@return: nil
function AdminApp:showDatabaseSelectDialog(callback)
    local dialog = self:insert(ltui.choicedialog:new(
        "db_select_dialog",
        ltui.rect { 0, 0, 45, 12 },
        "选择数据库"
    ))
    dialog:frame():style_set("border", true)

    local choices = {
        { g_config.database.prod_db, "生产数据库" },
        { g_config.database.test_db, "测试数据库" }
    }
    dialog:choices():load(choices)
    dialog:center(self:bounds())

    dialog:button_add("confirm", "确认", function()
        local selected = dialog:choices():current()
        if selected then
            local use_prod = (selected == g_config.database.prod_db)
            self._selected_db = use_prod and "prod" or "test"

            local ok, err = connectDatabase(use_prod)
            if ok then
                self:remove(dialog)
                if callback then callback() end
            else
                self:showMessageBox("连接失败", err or "未知错误", function()
                    self:quit()
                end)
            end
        end
    end)

    dialog:button_add("cancel", "退出", function()
        self:quit()
    end)
end

--@description: 显示消息框
--@param self: AdminApp - 应用实例
--@param title: string - 标题
--@param message: string - 消息内容
--@param callback: function|nil - 关闭后的回调
--@return: nil
function AdminApp:showMessageBox(title, message, callback)
    local dialog = self:insert(ltui.boxdialog:new(
        "msg_dialog",
        ltui.rect { 0, 0, 50, 10 },
        title
    ))
    dialog:frame():style_set("border", true)
    dialog:text():text_set(message)
    dialog:center(self:bounds())

    dialog:button_add("ok", "确定", function()
        self:remove(dialog)
        if callback then callback() end
    end)
end

--@description: 显示确认对话框
--@param self: AdminApp - 应用实例
--@param title: string - 标题
--@param message: string - 消息内容
--@param on_confirm: function - 确认回调
--@param on_cancel: function|nil - 取消回调
--@return: nil
function AdminApp:showConfirmDialog(title, message, on_confirm, on_cancel)
    local dialog = self:insert(ltui.boxdialog:new(
        "confirm_dialog",
        ltui.rect { 0, 0, 50, 10 },
        title
    ))
    dialog:frame():style_set("border", true)
    dialog:text():text_set(message)
    dialog:center(self:bounds())

    dialog:button_add("confirm", "确认", function()
        self:remove(dialog)
        if on_confirm then on_confirm() end
    end)

    dialog:button_add("cancel", "取消", function()
        self:remove(dialog)
        if on_cancel then on_cancel() end
    end)
end

--@description: 检查写权限
--@param self: AdminApp - 应用实例
--@return: boolean - 是否有写权限
function AdminApp:checkWritePermission()
    if not self._config.allow_write then
        self:showMessageBox("权限不足", "写操作已禁用\n请检查配置文件中的ALLOW_WRITE设置")
        return false
    end
    return true
end

--@description: 检查危险操作权限
--@param self: AdminApp - 应用实例
--@return: boolean - 是否有危险操作权限
function AdminApp:checkDangerPermission()
    if not self._config.allow_danger then
        self:showMessageBox("权限不足", "危险操作已禁用\n请检查配置文件中的ALLOW_DANGER设置")
        return false
    end
    return true
end

--@description: 显示主菜单
--@param self: AdminApp - 应用实例
--@return: nil
function AdminApp:showMainMenu()
    self._current_view = "main"

    local existing = self:view("main_menu")
    if existing then
        self:remove(existing)
    end

    local bounds = self:bounds()
    local menu = self:insert(ltui.menuconf:new(
        "main_menu",
        ltui.rect { 0, 0, bounds:width(), bounds:height() },
        APP_NAME .. " - " .. g_current_db
    ))
    menu:frame():style_set("border", true)

    local status_line = string.format(
        "写操作: %s | 危险操作: %s | 数据库: %s",
        self._config.allow_write and "允许" or "禁止",
        self._config.allow_danger and "允许" or "禁止",
        g_current_db
    )

    menu:load({
        { description = "用户管理", key = "1", action = function() self:showUserManagement() end },
        { description = "会话管理", key = "2", action = function() self:showSessionManagement() end },
        { description = "模板管理", key = "3", action = function() self:showTemplateManagement() end },
        { description = "执行记录", key = "4", action = function() self:showExecutionHistory() end },
        { description = "缓存管理", key = "5", action = function() self:showCacheManagement() end },
        { description = "系统日志", key = "6", action = function() self:showSystemLogs() end },
        { description = "切换数据库", key = "7", action = function() self:switchDatabase() end },
        { description = "退出", key = "q", action = function() self:confirmQuit() end }
    })

    menu:text():text_set(status_line)
    menu:select(menu:first())
end

--@description: 切换数据库
--@param self: AdminApp - 应用实例
--@return: nil
function AdminApp:switchDatabase()
    disconnectDatabase()
    self:showDatabaseSelectDialog(function()
        self:showMainMenu()
    end)
end

--@description: 确认退出
--@param self: AdminApp - 应用实例
--@return: nil
function AdminApp:confirmQuit()
    self:showConfirmDialog("确认退出", "是否确定退出管理系统?", function()
        writeLog(LogLevel.INFO, "用户退出系统")
        disconnectDatabase()
        self:quit()
    end)
end

--@description: 显示用户管理界面
--@param self: AdminApp - 应用实例
--@return: nil
function AdminApp:showUserManagement()
    self._current_view = "users"
    writeLog(LogLevel.INFO, "进入用户管理")

    local result, err = executeQuery([[
        SELECT id, username, status, created_at, updated_at
        FROM users ORDER BY id
    ]])

    if not result then
        self:showMessageBox("查询失败", err or "未知错误", function()
            self:showMainMenu()
        end)
        return
    end

    local existing = self:view("main_menu")
    if existing then self:remove(existing) end

    local bounds = self:bounds()
    local menu = self:insert(ltui.menuconf:new(
        "user_menu",
        ltui.rect { 0, 0, bounds:width(), bounds:height() },
        "用户管理"
    ))
    menu:frame():style_set("border", true)

    local items = {}

    table.insert(items, {
        description = string.format("%-6s %-20s %-12s %-20s", "ID", "用户名", "状态", "创建时间"),
        key = "",
        action = function() end
    })

    table.insert(items, {
        description = string.rep("-", 70),
        key = "",
        action = function() end
    })

    if result and #result > 0 then
        for _, user in ipairs(result) do
            local desc = string.format("%-6s %-20s %-12s %-20s",
                tostring(user.id),
                truncateString(user.username, 18),
                user.status,
                formatTimestamp(user.created_at)
            )
            table.insert(items, {
                description = desc,
                action = function() self:showUserDetail(user.id) end
            })
        end
    else
        table.insert(items, { description = "暂无用户数据", action = function() end })
    end

    table.insert(items, { description = string.rep("-", 70), action = function() end })
    table.insert(items, {
        description = "返回主菜单",
        key = "b",
        action = function()
            self:remove(menu)
            self:showMainMenu()
        end
    })

    menu:load(items)
    menu:select(menu:first())
end

--@description: 显示用户详情
--@param self: AdminApp - 应用实例
--@param user_id: number - 用户ID
--@return: nil
function AdminApp:showUserDetail(user_id)
    local result, err = executeQuery(string.format([[
        SELECT id, username, status, created_at, updated_at
        FROM users WHERE id = %d
    ]], user_id))

    if not result or #result == 0 then
        self:showMessageBox("查询失败", err or "用户不存在")
        return
    end

    local user = result[1]

    local existing = self:view("user_menu")
    if existing then self:remove(existing) end

    local bounds = self:bounds()
    local menu = self:insert(ltui.menuconf:new(
        "user_detail_menu",
        ltui.rect { 0, 0, bounds:width(), bounds:height() },
        "用户详情 - " .. user.username
    ))
    menu:frame():style_set("border", true)

    local detail_text = string.format([[
用户ID: %s
用户名: %s
状态: %s
创建时间: %s
更新时间: %s
]],
        user.id,
        user.username,
        user.status,
        formatTimestamp(user.created_at),
        formatTimestamp(user.updated_at)
    )

    local items = {
        { description = "--- 用户信息 ---", action = function() end }
    }

    for line in detail_text:gmatch("[^\n]+") do
        table.insert(items, { description = line, action = function() end })
    end

    table.insert(items, { description = "", action = function() end })
    table.insert(items, { description = "--- 操作 ---", action = function() end })

    if user.status == UserStatus.ACTIVE then
        table.insert(items, {
            description = "封禁用户",
            key = "b",
            action = function()
                if not self:checkWritePermission() then return end
                self:showConfirmDialog("确认封禁", "是否封禁用户 " .. user.username .. "?", function()
                    self:updateUserStatus(user_id, UserStatus.BANNED)
                end)
            end
        })
    else
        table.insert(items, {
            description = "解封用户",
            key = "u",
            action = function()
                if not self:checkWritePermission() then return end
                self:showConfirmDialog("确认解封", "是否解封用户 " .. user.username .. "?", function()
                    self:updateUserStatus(user_id, UserStatus.ACTIVE)
                end)
            end
        })
    end

    table.insert(items, {
        description = "删除用户",
        key = "d",
        action = function()
            if not self:checkWritePermission() then return end
            if not self:checkDangerPermission() then return end
            self:showConfirmDialog(
                "⚠ 危险操作",
                "确定要删除用户 " .. user.username .. " 吗?\n此操作不可恢复!",
                function()
                    self:deleteUser(user_id)
                end
            )
        end
    })

    table.insert(items, { description = "", action = function() end })
    table.insert(items, {
        description = "返回用户列表",
        key = "r",
        action = function()
            self:remove(menu)
            self:showUserManagement()
        end
    })

    menu:load(items)
    menu:select(menu:first())
end

--@description: 更新用户状态
--@param self: AdminApp - 应用实例
--@param user_id: number - 用户ID
--@param new_status: string - 新状态
--@return: nil
function AdminApp:updateUserStatus(user_id, new_status)
    local sql = string.format(
        "UPDATE users SET status = %s WHERE id = %d",
        escapeString(new_status),
        user_id
    )

    local result, err = executeQuery(sql)
    if result then
        writeLog(LogLevel.INFO, string.format("用户 %d 状态更新为 %s", user_id, new_status))
        self:showMessageBox("操作成功", "用户状态已更新", function()
            self:showUserDetail(user_id)
        end)
    else
        self:showMessageBox("操作失败", err or "未知错误")
    end
end

--@description: 删除用户
--@param self: AdminApp - 应用实例
--@param user_id: number - 用户ID
--@return: nil
function AdminApp:deleteUser(user_id)
    local sql = string.format("DELETE FROM users WHERE id = %d", user_id)

    local result, err = executeQuery(sql)
    if result then
        writeLog(LogLevel.WARN, string.format("删除用户 id=%d", user_id))
        self:showMessageBox("操作成功", "用户已删除", function()
            self:showUserManagement()
        end)
    else
        self:showMessageBox("操作失败", err or "未知错误")
    end
end

--@description: 显示会话管理界面
--@param self: AdminApp - 应用实例
--@return: nil
function AdminApp:showSessionManagement()
    self._current_view = "sessions"
    writeLog(LogLevel.INFO, "进入会话管理")

    local result, err = executeQuery([[
        SELECT s.id, s.user_id, u.username, s.ip_address, s.expires_at, s.created_at
        FROM sessions s
        LEFT JOIN users u ON s.user_id = u.id
        ORDER BY s.created_at DESC
        LIMIT 100
    ]])

    if not result then
        self:showMessageBox("查询失败", err or "未知错误", function()
            self:showMainMenu()
        end)
        return
    end

    local existing = self:view("main_menu") or self:view("user_menu") or self:view("user_detail_menu")
    if existing then self:remove(existing) end

    local bounds = self:bounds()
    local menu = self:insert(ltui.menuconf:new(
        "session_menu",
        ltui.rect { 0, 0, bounds:width(), bounds:height() },
        "会话管理"
    ))
    menu:frame():style_set("border", true)

    local items = {}

    table.insert(items, {
        description = string.format("%-6s %-15s %-18s %-20s", "ID", "用户", "IP地址", "过期时间"),
        action = function() end
    })
    table.insert(items, { description = string.rep("-", 70), action = function() end })

    if result and #result > 0 then
        for _, session in ipairs(result) do
            local desc = string.format("%-6s %-15s %-18s %-20s",
                tostring(session.id),
                truncateString(session.username or "未知", 13),
                truncateString(session.ip_address, 16),
                formatTimestamp(session.expires_at)
            )
            table.insert(items, {
                description = desc,
                action = function()
                    if not self:checkWritePermission() then return end
                    self:showConfirmDialog(
                        "强制下线",
                        string.format("是否强制下线会话 %d (%s)?", session.id, session.username or "未知"),
                        function()
                            self:deleteSession(session.id)
                        end
                    )
                end
            })
        end
    else
        table.insert(items, { description = "暂无活跃会话", action = function() end })
    end

    table.insert(items, { description = string.rep("-", 70), action = function() end })

    table.insert(items, {
        description = "清空所有会话",
        key = "c",
        action = function()
            if not self:checkWritePermission() then return end
            if not self:checkDangerPermission() then return end
            self:showConfirmDialog(
                "⚠ 危险操作",
                "确定要清空所有会话吗?\n所有用户将被强制下线!",
                function()
                    self:clearAllSessions()
                end
            )
        end
    })

    table.insert(items, {
        description = "返回主菜单",
        key = "b",
        action = function()
            self:remove(menu)
            self:showMainMenu()
        end
    })

    menu:load(items)
    menu:select(menu:first())
end

--@description: 删除会话
--@param self: AdminApp - 应用实例
--@param session_id: number - 会话ID
--@return: nil
function AdminApp:deleteSession(session_id)
    local sql = string.format("DELETE FROM sessions WHERE id = %d", session_id)
    local result, err = executeQuery(sql)

    if result then
        writeLog(LogLevel.INFO, string.format("删除会话 id=%d", session_id))
        self:showMessageBox("操作成功", "会话已终止", function()
            self:showSessionManagement()
        end)
    else
        self:showMessageBox("操作失败", err or "未知错误")
    end
end

--@description: 清空所有会话
--@param self: AdminApp - 应用实例
--@return: nil
function AdminApp:clearAllSessions()
    local result, err = executeQuery("DELETE FROM sessions")

    if result then
        writeLog(LogLevel.WARN, "清空所有会话")
        self:showMessageBox("操作成功", "所有会话已清空", function()
            self:showSessionManagement()
        end)
    else
        self:showMessageBox("操作失败", err or "未知错误")
    end
end

--@description: 显示模板管理界面
--@param self: AdminApp - 应用实例
--@return: nil
function AdminApp:showTemplateManagement()
    self._current_view = "templates"
    writeLog(LogLevel.INFO, "进入模板管理")

    local result, err = executeQuery([[
        SELECT id, title, category, favorite_count, created_at
        FROM templates ORDER BY id
    ]])

    if not result then
        self:showMessageBox("查询失败", err or "未知错误", function()
            self:showMainMenu()
        end)
        return
    end

    local existing = self:view("main_menu") or self:view("session_menu")
    if existing then self:remove(existing) end

    local bounds = self:bounds()
    local menu = self:insert(ltui.menuconf:new(
        "template_menu",
        ltui.rect { 0, 0, bounds:width(), bounds:height() },
        "模板管理"
    ))
    menu:frame():style_set("border", true)

    local items = {}

    table.insert(items, {
        description = string.format("%-5s %-25s %-15s %-8s", "ID", "标题", "分类", "收藏"),
        action = function() end
    })
    table.insert(items, { description = string.rep("-", 70), action = function() end })

    if result and #result > 0 then
        for _, tpl in ipairs(result) do
            local desc = string.format("%-5s %-25s %-15s %-8s",
                tostring(tpl.id),
                truncateString(tpl.title, 23),
                truncateString(tpl.category, 13),
                tostring(tpl.favorite_count)
            )
            table.insert(items, {
                description = desc,
                action = function() self:showTemplateDetail(tpl.id) end
            })
        end
    else
        table.insert(items, { description = "暂无模板数据", action = function() end })
    end

    table.insert(items, { description = string.rep("-", 70), action = function() end })

    table.insert(items, {
        description = "新增模板",
        key = "n",
        action = function()
            if not self:checkWritePermission() then return end
            self:showAddTemplateDialog()
        end
    })

    table.insert(items, {
        description = "返回主菜单",
        key = "b",
        action = function()
            self:remove(menu)
            self:showMainMenu()
        end
    })

    menu:load(items)
    menu:select(menu:first())
end

--@description: 显示模板详情
--@param self: AdminApp - 应用实例
--@param template_id: number - 模板ID
--@return: nil
function AdminApp:showTemplateDetail(template_id)
    local result, err = executeQuery(string.format([[
        SELECT id, title, category, description, favorite_count, created_at, updated_at
        FROM templates WHERE id = %d
    ]], template_id))

    if not result or #result == 0 then
        self:showMessageBox("查询失败", err or "模板不存在")
        return
    end

    local tpl = result[1]

    local codes_result = executeQuery(string.format([[
        SELECT language, code FROM template_codes WHERE template_id = %d
    ]], template_id))

    local existing = self:view("template_menu")
    if existing then self:remove(existing) end

    local bounds = self:bounds()
    local menu = self:insert(ltui.menuconf:new(
        "template_detail_menu",
        ltui.rect { 0, 0, bounds:width(), bounds:height() },
        "模板详情 - " .. tpl.title
    ))
    menu:frame():style_set("border", true)

    local items = {
        { description = "--- 基本信息 ---", action = function() end },
        { description = "ID: " .. tpl.id, action = function() end },
        { description = "标题: " .. tpl.title, action = function() end },
        { description = "分类: " .. tpl.category, action = function() end },
        { description = "收藏数: " .. tpl.favorite_count, action = function() end },
        { description = "创建时间: " .. formatTimestamp(tpl.created_at), action = function() end },
        { description = "", action = function() end },
        { description = "--- 支持语言 ---", action = function() end }
    }

    if codes_result and #codes_result > 0 then
        for _, code in ipairs(codes_result) do
            table.insert(items, {
                description = "  " .. code.language,
                action = function()
                    self:showTemplateCode(tpl.title, code.language, code.code)
                end
            })
        end
    else
        table.insert(items, { description = "  暂无代码实现", action = function() end })
    end

    table.insert(items, { description = "", action = function() end })
    table.insert(items, { description = "--- 操作 ---", action = function() end })

    table.insert(items, {
        description = "编辑模板",
        key = "e",
        action = function()
            if not self:checkWritePermission() then return end
            self:showEditTemplateDialog(tpl)
        end
    })

    table.insert(items, {
        description = "删除模板",
        key = "d",
        action = function()
            if not self:checkWritePermission() then return end
            if not self:checkDangerPermission() then return end
            self:showConfirmDialog(
                "⚠ 危险操作",
                "确定要删除模板 " .. tpl.title .. " 吗?\n此操作不可恢复!",
                function()
                    self:deleteTemplate(template_id)
                end
            )
        end
    })

    table.insert(items, { description = "", action = function() end })
    table.insert(items, {
        description = "返回模板列表",
        key = "r",
        action = function()
            self:remove(menu)
            self:showTemplateManagement()
        end
    })

    menu:load(items)
    menu:select(menu:first())
end

--@description: 显示模板代码
--@param self: AdminApp - 应用实例
--@param title: string - 模板标题
--@param language: string - 编程语言
--@param code: string - 代码内容
--@return: nil
function AdminApp:showTemplateCode(title, language, code)
    local dialog = self:insert(ltui.textdialog:new(
        "code_dialog",
        ltui.rect { 0, 0, 70, 25 },
        title .. " - " .. language
    ))
    dialog:frame():style_set("border", true)
    dialog:text():text_set(code or "暂无代码")
    dialog:center(self:bounds())

    dialog:button_add("close", "关闭", function()
        self:remove(dialog)
    end)
end

--@description: 显示新增模板对话框
--@param self: AdminApp - 应用实例
--@return: nil
function AdminApp:showAddTemplateDialog()
    local dialog = self:insert(ltui.inputdialog:new(
        "add_tpl_dialog",
        ltui.rect { 0, 0, 55, 10 },
        "新增模板"
    ))
    dialog:frame():style_set("border", true)
    dialog:text():text_set("请输入模板标题:")
    dialog:center(self:bounds())

    dialog:button_add("next", "下一步", function()
        local title = dialog:textedit():text()
        if title and #trim(title) > 0 then
            self:remove(dialog)
            self:showAddTemplateCategoryDialog(trim(title))
        else
            dialog:text():text_set("标题不能为空，请重新输入:")
        end
    end)

    dialog:button_add("cancel", "取消", function()
        self:remove(dialog)
    end)
end

--@description: 显示新增模板分类对话框
--@param self: AdminApp - 应用实例
--@param title: string - 模板标题
--@return: nil
function AdminApp:showAddTemplateCategoryDialog(title)
    local dialog = self:insert(ltui.inputdialog:new(
        "add_tpl_cat_dialog",
        ltui.rect { 0, 0, 55, 10 },
        "新增模板 - 分类"
    ))
    dialog:frame():style_set("border", true)
    dialog:text():text_set("请输入模板分类 (如: 排序算法):")
    dialog:center(self:bounds())

    dialog:button_add("create", "创建", function()
        local category = dialog:textedit():text()
        if category and #trim(category) > 0 then
            self:remove(dialog)
            self:createTemplate(title, trim(category))
        else
            dialog:text():text_set("分类不能为空，请重新输入:")
        end
    end)

    dialog:button_add("cancel", "取消", function()
        self:remove(dialog)
    end)
end

--@description: 创建模板
--@param self: AdminApp - 应用实例
--@param title: string - 模板标题
--@param category: string - 模板分类
--@return: nil
function AdminApp:createTemplate(title, category)
    local sql = string.format(
        "INSERT INTO templates (title, category, description) VALUES (%s, %s, '')",
        escapeString(title),
        escapeString(category)
    )

    local result, err = executeQuery(sql)
    if result then
        writeLog(LogLevel.INFO, string.format("创建模板: %s", title))
        self:showMessageBox("操作成功", "模板已创建", function()
            self:showTemplateManagement()
        end)
    else
        self:showMessageBox("操作失败", err or "未知错误")
    end
end

--@description: 显示编辑模板对话框
--@param self: AdminApp - 应用实例
--@param tpl: table - 模板信息
--@return: nil
function AdminApp:showEditTemplateDialog(tpl)
    local dialog = self:insert(ltui.inputdialog:new(
        "edit_tpl_dialog",
        ltui.rect { 0, 0, 55, 10 },
        "编辑模板"
    ))
    dialog:frame():style_set("border", true)
    dialog:text():text_set("编辑标题 (当前: " .. tpl.title .. "):")
    dialog:textedit():text_set(tpl.title)
    dialog:center(self:bounds())

    dialog:button_add("save", "保存", function()
        local new_title = dialog:textedit():text()
        if new_title and #trim(new_title) > 0 then
            self:remove(dialog)
            self:updateTemplate(tpl.id, trim(new_title), tpl.category)
        else
            dialog:text():text_set("标题不能为空:")
        end
    end)

    dialog:button_add("cancel", "取消", function()
        self:remove(dialog)
    end)
end

--@description: 更新模板
--@param self: AdminApp - 应用实例
--@param template_id: number - 模板ID
--@param title: string - 新标题
--@param category: string - 分类
--@return: nil
function AdminApp:updateTemplate(template_id, title, category)
    local sql = string.format(
        "UPDATE templates SET title = %s, category = %s WHERE id = %d",
        escapeString(title),
        escapeString(category),
        template_id
    )

    local result, err = executeQuery(sql)
    if result then
        writeLog(LogLevel.INFO, string.format("更新模板 id=%d", template_id))
        self:showMessageBox("操作成功", "模板已更新", function()
            self:showTemplateDetail(template_id)
        end)
    else
        self:showMessageBox("操作失败", err or "未知错误")
    end
end

--@description: 删除模板
--@param self: AdminApp - 应用实例
--@param template_id: number - 模板ID
--@return: nil
function AdminApp:deleteTemplate(template_id)
    local sql = string.format("DELETE FROM templates WHERE id = %d", template_id)

    local result, err = executeQuery(sql)
    if result then
        writeLog(LogLevel.WARN, string.format("删除模板 id=%d", template_id))
        self:showMessageBox("操作成功", "模板已删除", function()
            self:showTemplateManagement()
        end)
    else
        self:showMessageBox("操作失败", err or "未知错误")
    end
end

--@description: 显示执行记录界面
--@param self: AdminApp - 应用实例
--@return: nil
function AdminApp:showExecutionHistory()
    self._current_view = "executions"
    writeLog(LogLevel.INFO, "进入执行记录")

    local result, err = executeQuery([[
        SELECT e.id, e.user_id, u.username, e.language, e.status, e.execution_time, e.created_at
        FROM executions e
        LEFT JOIN users u ON e.user_id = u.id
        ORDER BY e.created_at DESC
        LIMIT 100
    ]])

    if not result then
        self:showMessageBox("查询失败", err or "未知错误", function()
            self:showMainMenu()
        end)
        return
    end

    local existing = self:view("main_menu") or self:view("template_menu") or self:view("template_detail_menu")
    if existing then self:remove(existing) end

    local bounds = self:bounds()
    local menu = self:insert(ltui.menuconf:new(
        "execution_menu",
        ltui.rect { 0, 0, bounds:width(), bounds:height() },
        "执行记录"
    ))
    menu:frame():style_set("border", true)

    local items = {}

    table.insert(items, {
        description = string.format("%-5s %-12s %-10s %-10s %-8s %-18s", "ID", "用户", "语言", "状态", "耗时", "时间"),
        action = function() end
    })
    table.insert(items, { description = string.rep("-", 75), action = function() end })

    if result and #result > 0 then
        for _, exec in ipairs(result) do
            local time_str = exec.execution_time and (exec.execution_time .. "ms") or "-"
            local desc = string.format("%-5s %-12s %-10s %-10s %-8s %-18s",
                tostring(exec.id),
                truncateString(exec.username or "未知", 10),
                exec.language,
                exec.status,
                time_str,
                formatTimestamp(exec.created_at):sub(1, 16)
            )
            table.insert(items, {
                description = desc,
                action = function() self:showExecutionDetail(exec.id) end
            })
        end
    else
        table.insert(items, { description = "暂无执行记录", action = function() end })
    end

    table.insert(items, { description = string.rep("-", 75), action = function() end })

    table.insert(items, {
        description = "清空所有记录",
        key = "c",
        action = function()
            if not self:checkWritePermission() then return end
            if not self:checkDangerPermission() then return end
            self:showConfirmDialog(
                "⚠ 危险操作",
                "确定要清空所有执行记录吗?\n此操作不可恢复!",
                function()
                    self:clearAllExecutions()
                end
            )
        end
    })

    table.insert(items, {
        description = "返回主菜单",
        key = "b",
        action = function()
            self:remove(menu)
            self:showMainMenu()
        end
    })

    menu:load(items)
    menu:select(menu:first())
end

--@description: 显示执行详情
--@param self: AdminApp - 应用实例
--@param exec_id: number - 执行ID
--@return: nil
function AdminApp:showExecutionDetail(exec_id)
    local result, err = executeQuery(string.format([[
        SELECT e.*, u.username
        FROM executions e
        LEFT JOIN users u ON e.user_id = u.id
        WHERE e.id = %d
    ]], exec_id))

    if not result or #result == 0 then
        self:showMessageBox("查询失败", err or "记录不存在")
        return
    end

    local exec = result[1]

    local existing = self:view("execution_menu")
    if existing then self:remove(existing) end

    local bounds = self:bounds()
    local menu = self:insert(ltui.menuconf:new(
        "exec_detail_menu",
        ltui.rect { 0, 0, bounds:width(), bounds:height() },
        "执行详情 #" .. exec_id
    ))
    menu:frame():style_set("border", true)

    local items = {
        { description = "--- 基本信息 ---", action = function() end },
        { description = "ID: " .. exec.id, action = function() end },
        { description = "用户: " .. (exec.username or "未知"), action = function() end },
        { description = "语言: " .. exec.language, action = function() end },
        { description = "状态: " .. exec.status, action = function() end },
        { description = "耗时: " .. (exec.execution_time or 0) .. "ms", action = function() end },
        { description = "IP: " .. (exec.ip_address or "-"), action = function() end },
        { description = "时间: " .. formatTimestamp(exec.created_at), action = function() end },
        { description = "", action = function() end }
    }

    if exec.error_message and #exec.error_message > 0 then
        table.insert(items, { description = "错误: " .. truncateString(exec.error_message, 50), action = function() end })
        table.insert(items, { description = "", action = function() end })
    end

    table.insert(items, { description = "--- 操作 ---", action = function() end })

    table.insert(items, {
        description = "查看源代码",
        key = "s",
        action = function()
            self:showTemplateCode("源代码", exec.language, exec.code)
        end
    })

    if exec.toml_output and #exec.toml_output > 0 then
        table.insert(items, {
            description = "查看TOML输出",
            key = "t",
            action = function()
                self:showTemplateCode("TOML输出", "toml", exec.toml_output)
            end
        })
    end

    table.insert(items, {
        description = "删除此记录",
        key = "d",
        action = function()
            if not self:checkWritePermission() then return end
            if not self:checkDangerPermission() then return end
            self:showConfirmDialog("确认删除", "是否删除此执行记录?", function()
                self:deleteExecution(exec_id)
            end)
        end
    })

    table.insert(items, { description = "", action = function() end })
    table.insert(items, {
        description = "返回记录列表",
        key = "r",
        action = function()
            self:remove(menu)
            self:showExecutionHistory()
        end
    })

    menu:load(items)
    menu:select(menu:first())
end

--@description: 删除执行记录
--@param self: AdminApp - 应用实例
--@param exec_id: number - 执行ID
--@return: nil
function AdminApp:deleteExecution(exec_id)
    local sql = string.format("DELETE FROM executions WHERE id = %d", exec_id)

    local result, err = executeQuery(sql)
    if result then
        writeLog(LogLevel.WARN, string.format("删除执行记录 id=%d", exec_id))
        self:showMessageBox("操作成功", "记录已删除", function()
            self:showExecutionHistory()
        end)
    else
        self:showMessageBox("操作失败", err or "未知错误")
    end
end

--@description: 清空所有执行记录
--@param self: AdminApp - 应用实例
--@return: nil
function AdminApp:clearAllExecutions()
    local result, err = executeQuery("DELETE FROM executions")

    if result then
        writeLog(LogLevel.WARN, "清空所有执行记录")
        self:showMessageBox("操作成功", "所有记录已清空", function()
            self:showExecutionHistory()
        end)
    else
        self:showMessageBox("操作失败", err or "未知错误")
    end
end

--@description: 显示缓存管理界面
--@param self: AdminApp - 应用实例
--@return: nil
function AdminApp:showCacheManagement()
    self._current_view = "cache"
    writeLog(LogLevel.INFO, "进入缓存管理")

    local stats_result = executeQuery([[
        SELECT
            COUNT(*) as total_count,
            COALESCE(SUM(hit_count), 0) as total_hits,
            COALESCE(SUM(LENGTH(toml_output)), 0) as total_size
        FROM execution_cache
    ]])

    local list_result = executeQuery([[
        SELECT id, language, hit_count, created_at, last_hit_at,
               LEFT(code_hash, 16) as hash_prefix
        FROM execution_cache
        ORDER BY last_hit_at DESC
        LIMIT 50
    ]])

    local existing = self:view("main_menu") or self:view("execution_menu") or self:view("exec_detail_menu")
    if existing then self:remove(existing) end

    local bounds = self:bounds()
    local menu = self:insert(ltui.menuconf:new(
        "cache_menu",
        ltui.rect { 0, 0, bounds:width(), bounds:height() },
        "缓存管理"
    ))
    menu:frame():style_set("border", true)

    local items = {}

    table.insert(items, { description = "--- 缓存统计 ---", action = function() end })

    if stats_result and #stats_result > 0 then
        local stats = stats_result[1]
        local size_kb = math.floor((tonumber(stats.total_size) or 0) / 1024)
        table.insert(items, {
            description = string.format("总缓存数: %s | 总命中: %s | 总大小: %dKB",
                stats.total_count, stats.total_hits, size_kb),
            action = function() end
        })
    end

    table.insert(items, { description = "", action = function() end })
    table.insert(items, { description = "--- 缓存列表 ---", action = function() end })
    table.insert(items, {
        description = string.format("%-5s %-18s %-10s %-8s %-20s", "ID", "哈希", "语言", "命中", "最后命中"),
        action = function() end
    })
    table.insert(items, { description = string.rep("-", 70), action = function() end })

    if list_result and #list_result > 0 then
        for _, cache in ipairs(list_result) do
            local desc = string.format("%-5s %-18s %-10s %-8s %-20s",
                tostring(cache.id),
                cache.hash_prefix .. "...",
                cache.language,
                tostring(cache.hit_count),
                formatTimestamp(cache.last_hit_at):sub(1, 16)
            )
            table.insert(items, { description = desc, action = function() end })
        end
    else
        table.insert(items, { description = "暂无缓存数据", action = function() end })
    end

    table.insert(items, { description = string.rep("-", 70), action = function() end })

    table.insert(items, {
        description = "清空所有缓存",
        key = "c",
        action = function()
            if not self:checkWritePermission() then return end
            if not self:checkDangerPermission() then return end
            self:showConfirmDialog(
                "⚠ 危险操作",
                "确定要清空所有缓存吗?",
                function()
                    self:clearAllCache()
                end
            )
        end
    })

    table.insert(items, {
        description = "返回主菜单",
        key = "b",
        action = function()
            self:remove(menu)
            self:showMainMenu()
        end
    })

    menu:load(items)
    menu:select(menu:first())
end

--@description: 清空所有缓存
--@param self: AdminApp - 应用实例
--@return: nil
function AdminApp:clearAllCache()
    local result, err = executeQuery("DELETE FROM execution_cache")

    if result then
        writeLog(LogLevel.WARN, "清空所有缓存")
        self:showMessageBox("操作成功", "所有缓存已清空", function()
            self:showCacheManagement()
        end)
    else
        self:showMessageBox("操作失败", err or "未知错误")
    end
end

--@description: 显示系统日志界面
--@param self: AdminApp - 应用实例
--@return: nil
function AdminApp:showSystemLogs()
    self._current_view = "logs"
    writeLog(LogLevel.INFO, "进入系统日志")
    self:showLogsWithFilter(nil)
end

--@description: 显示带筛选的日志
--@param self: AdminApp - 应用实例
--@param level_filter: string|nil - 日志级别筛选
--@return: nil
function AdminApp:showLogsWithFilter(level_filter)
    local sql = [[
        SELECT id, level, module, message, created_at
        FROM logs
    ]]

    if level_filter then
        sql = sql .. string.format(" WHERE level = %s", escapeString(level_filter))
    end

    sql = sql .. " ORDER BY created_at DESC LIMIT 100"

    local result, err = executeQuery(sql)

    if not result then
        self:showMessageBox("查询失败", err or "未知错误", function()
            self:showMainMenu()
        end)
        return
    end

    local existing = self:view("main_menu") or self:view("cache_menu") or self:view("log_menu")
    if existing then self:remove(existing) end

    local bounds = self:bounds()
    local menu = self:insert(ltui.menuconf:new(
        "log_menu",
        ltui.rect { 0, 0, bounds:width(), bounds:height() },
        "系统日志" .. (level_filter and (" - " .. level_filter) or "")
    ))
    menu:frame():style_set("border", true)

    local items = {}

    table.insert(items, { description = "--- 筛选 ---", action = function() end })
    table.insert(items, {
        description = "全部",
        key = "0",
        action = function()
            self:remove(menu)
            self:showLogsWithFilter(nil)
        end
    })
    table.insert(items, {
        description = "DEBUG",
        key = "1",
        action = function()
            self:remove(menu)
            self:showLogsWithFilter("DEBUG")
        end
    })
    table.insert(items, {
        description = "INFO",
        key = "2",
        action = function()
            self:remove(menu)
            self:showLogsWithFilter("INFO")
        end
    })
    table.insert(items, {
        description = "WARNING",
        key = "3",
        action = function()
            self:remove(menu)
            self:showLogsWithFilter("WARNING")
        end
    })
    table.insert(items, {
        description = "ERROR",
        key = "4",
        action = function()
            self:remove(menu)
            self:showLogsWithFilter("ERROR")
        end
    })

    table.insert(items, { description = "", action = function() end })
    table.insert(items, { description = "--- 日志列表 ---", action = function() end })
    table.insert(items, {
        description = string.format("%-8s %-15s %-45s", "级别", "模块", "消息"),
        action = function() end
    })
    table.insert(items, { description = string.rep("-", 75), action = function() end })

    if result and #result > 0 then
        for _, log in ipairs(result) do
            local desc = string.format("%-8s %-15s %-45s",
                log.level,
                truncateString(log.module, 13),
                truncateString(log.message, 43)
            )
            table.insert(items, {
                description = desc,
                action = function()
                    self:showLogDetail(log)
                end
            })
        end
    else
        table.insert(items, { description = "暂无日志数据", action = function() end })
    end

    table.insert(items, { description = string.rep("-", 75), action = function() end })
    table.insert(items, {
        description = "返回主菜单",
        key = "b",
        action = function()
            self:remove(menu)
            self:showMainMenu()
        end
    })

    menu:load(items)
    menu:select(menu:first())
end

--@description: 显示日志详情
--@param self: AdminApp - 应用实例
--@param log: table - 日志信息
--@return: nil
function AdminApp:showLogDetail(log)
    local content = string.format([[
ID: %s
级别: %s
模块: %s
时间: %s

消息:
%s
]],
        log.id,
        log.level,
        log.module,
        formatTimestamp(log.created_at),
        log.message
    )

    local dialog = self:insert(ltui.textdialog:new(
        "log_detail_dialog",
        ltui.rect { 0, 0, 65, 18 },
        "日志详情"
    ))
    dialog:frame():style_set("border", true)
    dialog:text():text_set(content)
    dialog:center(self:bounds())

    dialog:button_add("close", "关闭", function()
        self:remove(dialog)
    end)
end

--@description: 运行应用程序
--@param self: AdminApp - 应用实例
--@return: nil
function AdminApp:run()
    ltui.application.run(self, function()
        self:showStartupAnimation(function()
            if self._config.check_password then
                self:showPasswordDialog(function()
                    self:showDatabaseSelectDialog(function()
                        self:showMainMenu()
                    end)
                end)
            else
                self:showDatabaseSelectDialog(function()
                    self:showMainMenu()
                end)
            end
        end)
    end)
end

--@description: 主函数
--@return: nil
local function main()
    writeLog(LogLevel.INFO, "启动管理TUI")

    local config, err = parseIniFile(CONFIG_FILE)
    if not config then
        print("错误: " .. (err or "配置文件解析失败"))
        writeLog(LogLevel.ERROR, err or "配置文件解析失败")
        os.exit(1)
    end

    g_config = config
    writeLog(LogLevel.INFO, string.format(
        "配置加载完成: allow_write=%s, allow_danger=%s, check_password=%s",
        tostring(config.allow_write),
        tostring(config.allow_danger),
        tostring(config.check_password)
    ))

    local app = AdminApp()
    app:run()
end

main()
