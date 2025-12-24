--[[
会话管理模块

提供上下文管理、全局配置和会话管理功能。
管理数据结构的操作轨迹记录。

Author:
    WaterRun
File:
    ds4viz/session.lua
Date:
    2025-12-24
Updated:
    2025-12-24
]]

local trace_mod = require("ds4viz.trace")

local M = {}

--[[
错误类型枚举

Enum:
    ErrorType
Values:
    RUNTIME: 运行时错误
    TIMEOUT: 超时错误
    VALIDATION: 验证错误
    SANDBOX: 沙箱错误
    UNKNOWN: 未知错误
]]
M.ErrorType = {
    RUNTIME = "runtime",
    TIMEOUT = "timeout",
    VALIDATION = "validation",
    SANDBOX = "sandbox",
    UNKNOWN = "unknown",
}

--@description: 全局配置表
--@field output_path: string - 输出文件路径
--@field title: string - 可视化标题
--@field author: string - 作者信息
--@field comment: string - 注释说明
local global_config = {
    output_path = "trace.toml",
    title = "",
    author = "",
    comment = "",
}

--@description: 配置全局参数
--@param options: table|nil - 配置选项表
--@return: nil
--@usage: session.config({ output_path = "my_trace.toml", title = "Demo" })
function M.config(options)
    options = options or {}
    if options.output_path then
        global_config.output_path = options.output_path
    end
    if options.title then
        global_config.title = options.title
    end
    if options.author then
        global_config.author = options.author
    end
    if options.comment then
        global_config.comment = options.comment
    end
end

--@description: 获取全局配置
--@return: table - 当前全局配置
function M.getConfig()
    return {
        output_path = global_config.output_path,
        title = global_config.title,
        author = global_config.author,
        comment = global_config.comment,
    }
end

--@description: 获取当前Lua版本字符串
--@return: string - Lua版本字符串
function M.getLuaVersion()
    return _VERSION:match("Lua (.+)") or "5.4"
end

--@description: 获取调用者的代码行号
--@param level: number|nil - 调用栈层级，默认为3
--@return: number - 调用者所在行号
function M.getCallerLine(level)
    level = level or 3
    local info = debug.getinfo(level, "l")
    return info and info.currentline or 1
end

--@description: 获取当前时间的RFC3339格式字符串
--@return: string - RFC3339格式时间戳
function M.getCurrentTime()
    return os.date("!%Y-%m-%dT%H:%M:%SZ")
end

--[[
Session会话管理器类

管理数据结构的操作轨迹记录

Class:
    Session
Fields:
    kind: string - 数据结构类型
    label: string - 数据结构标签
    output: string - 输出文件路径
    states: table - 状态列表
    steps: table - 步骤列表
    state_counter: number - 状态计数器
    step_counter: number - 步骤计数器
    error: Error|nil - 错误信息
    entry_line: number - 入口行号
    exit_line: number - 退出行号
    failed_step_id: number|nil - 失败步骤ID
]]
local Session = {}
Session.__index = Session

--@description: 创建Session实例
--@param kind: string - 数据结构类型
--@param label: string - 数据结构标签
--@param output: string|nil - 输出文件路径
--@return: Session - Session实例
function Session.new(kind, label, output)
    local self = setmetatable({}, Session)
    self.kind = kind
    self.label = label
    self.output = output or global_config.output_path
    self.states = {}
    self.steps = {}
    self.state_counter = 0
    self.step_counter = 0
    self.error = nil
    self.entry_line = 1
    self.exit_line = 1
    self.failed_step_id = nil
    return self
end

--@description: 添加状态快照
--@param self: Session - Session实例
--@param data: table - 状态数据
--@return: number - 新状态的ID
function Session:addState(data)
    local state_id = self.state_counter
    self.states[#self.states + 1] = trace_mod.State.new(state_id, data)
    self.state_counter = self.state_counter + 1
    return state_id
end

--@description: 添加操作步骤
--@param self: Session - Session实例
--@param op: string - 操作名称
--@param before: number - 操作前状态ID
--@param after: number|nil - 操作后状态ID
--@param args: table - 操作参数
--@param ret: any - 返回值
--@param note: string|nil - 备注
--@param line: number|nil - 代码行号
--@return: number - 新步骤的ID
function Session:addStep(op, before, after, args, ret, note, line)
    local step_id = self.step_counter
    local code_loc = nil
    if line then
        code_loc = trace_mod.CodeLocation.new(line)
    end
    self.steps[#self.steps + 1] = trace_mod.Step.new(
        step_id, op, before, after, args or {}, code_loc, ret, note
    )
    self.step_counter = self.step_counter + 1
    return step_id
end

--@description: 设置错误信息
--@param self: Session - Session实例
--@param error_type: string - 错误类型
--@param message: string - 错误消息
--@param line: number|nil - 错误发生的代码行号
--@param step_id: number|nil - 发生错误的步骤ID
--@return: nil
function Session:setError(error_type, message, line, step_id)
    local last_state = nil
    if #self.states > 0 then
        last_state = self.states[#self.states].id
    end
    self.error = trace_mod.Error.new(error_type, message, line, step_id, last_state)
    self.failed_step_id = step_id
end

--@description: 设置上下文入口行号
--@param self: Session - Session实例
--@param line: number - 入口行号
--@return: nil
function Session:setEntryLine(line)
    self.entry_line = line
end

--@description: 设置上下文退出行号
--@param self: Session - Session实例
--@param line: number - 退出行号
--@return: nil
function Session:setExitLine(line)
    self.exit_line = line
end

--@description: 获取最后一个状态的ID
--@param self: Session - Session实例
--@return: number - 最后一个状态的ID，若无状态则返回-1
function Session:getLastStateId()
    if #self.states > 0 then
        return self.states[#self.states].id
    end
    return -1
end

--@description: 构建完整的Trace对象
--@param self: Session - Session实例
--@return: Trace - 完整的Trace对象
function Session:buildTrace()
    local now = M.getCurrentTime()
    local meta = trace_mod.Meta.new(now, "lua", M.getLuaVersion())
    local package = trace_mod.Package.new()
    local remarks = trace_mod.Remarks.new(
        global_config.title,
        global_config.author,
        global_config.comment
    )
    local obj = trace_mod.Object.new(self.kind, self.label)
    local result = nil
    local err = nil
    if self.error then
        err = self.error
    else
        local final_state_id = 0
        if #self.states > 0 then
            final_state_id = self.states[#self.states].id
        end
        result = trace_mod.Result.new(
            final_state_id,
            trace_mod.Commit.new("commit", self.exit_line)
        )
    end
    return trace_mod.Trace.new(
        meta, package, remarks, obj,
        self.states, self.steps, result, err
    )
end

--@description: 将Trace写入文件
--@param self: Session - Session实例
--@return: nil
function Session:write()
    local trace = self:buildTrace()
    local writer = trace_mod.TomlWriter.new(trace)
    writer:write(self.output)
end

M.Session = Session

--[[
BaseStructure数据结构基类

提供所有数据结构的公共功能

Class:
    BaseStructure
Fields:
    session: Session - 会话对象
    entered: boolean - 是否已进入上下文
]]
local BaseStructure = {}
BaseStructure.__index = BaseStructure

--@description: 创建BaseStructure实例
--@param kind: string - 数据结构类型
--@param label: string - 数据结构标签
--@param output: string|nil - 输出文件路径
--@return: BaseStructure - BaseStructure实例
function BaseStructure.new(kind, label, output)
    local self = setmetatable({}, BaseStructure)
    self.session = Session.new(kind, label, output)
    self.entered = false
    return self
end

--@description: 进入上下文（模拟with语句的开始）
--@param self: BaseStructure - BaseStructure实例
--@return: BaseStructure - 自身实例
function BaseStructure:enter()
    self.entered = true
    self.session:setEntryLine(M.getCallerLine(3))
    self:initialize()
    return self
end

--@description: 退出上下文（模拟with语句的结束）
--@param self: BaseStructure - BaseStructure实例
--@param err_msg: string|nil - 错误消息
--@param err_line: number|nil - 错误行号
--@return: nil
function BaseStructure:exit(err_msg, err_line)
    self.session:setExitLine(M.getCallerLine(3))
    if err_msg then
        self.session:setError(
            M.ErrorType.RUNTIME,
            err_msg,
            err_line,
            self.session.failed_step_id
        )
    end
    self.session:write()
end

--@description: 初始化数据结构，子类需要重写此方法添加初始状态
--@param self: BaseStructure - BaseStructure实例
--@return: nil
function BaseStructure:initialize()
    self.session:addState(self:buildData())
end

--@description: 构建当前状态的数据字典，子类需要重写此方法
--@param self: BaseStructure - BaseStructure实例
--@return: table - 状态数据字典
function BaseStructure:buildData()
    return {}
end

--@description: 抛出数据结构错误
--@param self: BaseStructure - BaseStructure实例
--@param message: string - 错误消息
--@return: nil
--@raise: error - 始终抛出错误
function BaseStructure:raiseError(message)
    local line = M.getCallerLine(3)
    local step_id = self.session.step_counter
    self.session.failed_step_id = step_id
    error({ message = message, line = line, is_ds4viz_error = true })
end

--@description: 记录操作步骤并添加新状态
--@param self: BaseStructure - BaseStructure实例
--@param op: string - 操作名称
--@param args: table - 操作参数
--@param ret: any - 返回值
--@param note: string|nil - 备注
--@return: nil
function BaseStructure:recordStep(op, args, ret, note)
    local before = self.session:getLastStateId()
    local line = M.getCallerLine(3)
    local new_data = self:buildData()
    local after = self.session:addState(new_data)
    self.session:addStep(op, before, after, args, ret, note, line)
end

M.BaseStructure = BaseStructure

--@description: 使用上下文执行函数（模拟Python的with语句）
--@param structure: BaseStructure - 数据结构实例
--@param fn: function - 要执行的函数
--@return: boolean - 是否成功执行
--@return: any - 错误信息（如果有）
function M.withContext(structure, fn)
    structure:enter()
    local ok, err = pcall(fn, structure)
    if ok then
        structure:exit()
    else
        local err_msg = err
        local err_line = nil
        if type(err) == "table" and err.is_ds4viz_error then
            err_msg = err.message
            err_line = err.line
        elseif type(err) == "string" then
            local line_match = err:match(":(%d+):")
            if line_match then
                err_line = tonumber(line_match)
            end
        end
        structure:exit(tostring(err_msg), err_line)
    end
    return ok, err
end

return M
