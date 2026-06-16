--[[
Trace数据结构定义和TOML序列化

提供Trace、State、Step等数据结构的定义，
以及将Trace对象序列化为TOML格式的功能。

Author:
    WaterRun
File:
    ds4viz/trace.lua
Date:
    2025-12-24
Updated:
    2025-12-24
]]

local M = {}

--@description: 转义TOML字符串中的特殊字符
--@local: true
--@param s: string - 原始字符串
--@return: string - 转义后的字符串
local function escapeString(s)
    return s:gsub("\\", "\\\\")
        :gsub('"', '\\"')
        :gsub("\n", "\\n")
        :gsub("\r", "\\r")
end

--@description: 序列化单个值为TOML格式
--@local: true
--@param value: any - 要序列化的值
--@return: string - TOML格式的值字符串
local function serializeValue(value)
    local t = type(value)
    if t == "boolean" then
        return value and "true" or "false"
    elseif t == "number" then
        if value == math.floor(value) then
            return string.format("%d", value)
        else
            return string.format("%g", value)
        end
    elseif t == "string" then
        return string.format('"%s"', escapeString(value))
    elseif t == "table" then
        if #value > 0 or next(value) == nil then
            local items = {}
            for _, v in ipairs(value) do
                items[#items + 1] = serializeValue(v)
            end
            return "[" .. table.concat(items, ", ") .. "]"
        else
            local pairs_list = {}
            for k, v in pairs(value) do
                pairs_list[#pairs_list + 1] = string.format("%s = %s", k, serializeValue(v))
            end
            return "{ " .. table.concat(pairs_list, ", ") .. " }"
        end
    else
        return string.format('"%s"', escapeString(tostring(value)))
    end
end

--@description: 序列化简单数组为TOML格式
--@local: true
--@param arr: table - 值数组
--@return: string - TOML格式的数组字符串
local function serializeSimpleArray(arr)
    if #arr == 0 then
        return "[]"
    end
    local items = {}
    for _, v in ipairs(arr) do
        items[#items + 1] = serializeValue(v)
    end
    return "[" .. table.concat(items, ", ") .. "]"
end

--@description: 序列化inline table数组为TOML格式
--@local: true
--@param arr: table - 字典数组
--@return: string - TOML格式的inline table数组字符串
local function serializeInlineTableArray(arr)
    if #arr == 0 then
        return "[]"
    end
    local items = {}
    for _, item in ipairs(arr) do
        local pairs_list = {}
        local keys = {}
        for k in pairs(item) do
            keys[#keys + 1] = k
        end
        table.sort(keys)
        for _, k in ipairs(keys) do
            pairs_list[#pairs_list + 1] = string.format("%s = %s", k, serializeValue(item[k]))
        end
        items[#items + 1] = "{ " .. table.concat(pairs_list, ", ") .. " }"
    end
    return "[\n  " .. table.concat(items, ",\n  ") .. "\n]"
end

--[[
Meta元数据类

Class:
    Meta
Fields:
    created_at: string - 创建时间，RFC3339格式
    lang: string - 语言标识
    lang_version: string - 语言版本
]]
local Meta = {}
Meta.__index = Meta

--@description: 创建Meta实例
--@param created_at: string - 创建时间
--@param lang: string|nil - 语言标识，默认"lua"
--@param lang_version: string|nil - 语言版本
--@return: Meta - Meta实例
function Meta.new(created_at, lang, lang_version)
    local self = setmetatable({}, Meta)
    self.created_at = created_at
    self.lang = lang or "lua"
    self.lang_version = lang_version or _VERSION:match("Lua (.+)") or "5.4"
    return self
end

M.Meta = Meta

--[[
Package包信息类

Class:
    Package
Fields:
    name: string - 包名
    lang: string - 语言标识
    version: string - 包版本
]]
local Package = {}
Package.__index = Package

--@description: 创建Package实例
--@param name: string|nil - 包名，默认"ds4viz"
--@param lang: string|nil - 语言标识，默认"lua"
--@param version: string|nil - 包版本，默认"0.1.0"
--@return: Package - Package实例
function Package.new(name, lang, version)
    local self = setmetatable({}, Package)
    self.name = name or "ds4viz"
    self.lang = lang or "lua"
    self.version = version or "0.1.0"
    return self
end

M.Package = Package

--[[
Remarks备注信息类

Class:
    Remarks
Fields:
    title: string - 标题
    author: string - 作者
    comment: string - 注释
]]
local Remarks = {}
Remarks.__index = Remarks

--@description: 创建Remarks实例
--@param title: string|nil - 标题
--@param author: string|nil - 作者
--@param comment: string|nil - 注释
--@return: Remarks - Remarks实例
function Remarks.new(title, author, comment)
    local self = setmetatable({}, Remarks)
    self.title = title or ""
    self.author = author or ""
    self.comment = comment or ""
    return self
end

--@description: 检查备注是否为空
--@param self: Remarks - Remarks实例
--@return: boolean - 如果所有字段都为空则返回true
function Remarks:isEmpty()
    return self.title == "" and self.author == "" and self.comment == ""
end

M.Remarks = Remarks

--[[
Object数据结构对象描述类

Class:
    Object
Fields:
    kind: string - 数据结构类型
    label: string - 标签
]]
local Object = {}
Object.__index = Object

--@description: 创建Object实例
--@param kind: string - 数据结构类型
--@param label: string|nil - 标签
--@return: Object - Object实例
function Object.new(kind, label)
    local self = setmetatable({}, Object)
    self.kind = kind
    self.label = label or ""
    return self
end

M.Object = Object

--[[
State状态快照类

Class:
    State
Fields:
    id: number - 状态ID
    data: table - 状态数据
]]
local State = {}
State.__index = State

--@description: 创建State实例
--@param id: number - 状态ID
--@param data: table|nil - 状态数据
--@return: State - State实例
function State.new(id, data)
    local self = setmetatable({}, State)
    self.id = id
    self.data = data or {}
    return self
end

M.State = State

--[[
CodeLocation代码位置信息类

Class:
    CodeLocation
Fields:
    line: number - 行号
    col: number|nil - 列号
]]
local CodeLocation = {}
CodeLocation.__index = CodeLocation

--@description: 创建CodeLocation实例
--@param line: number - 行号
--@param col: number|nil - 列号
--@return: CodeLocation - CodeLocation实例
function CodeLocation.new(line, col)
    local self = setmetatable({}, CodeLocation)
    self.line = line
    self.col = col
    return self
end

M.CodeLocation = CodeLocation

--[[
Step操作步骤类

Class:
    Step
Fields:
    id: number - 步骤ID
    op: string - 操作名称
    before: number - 操作前状态ID
    after: number|nil - 操作后状态ID
    args: table - 操作参数
    code: CodeLocation|nil - 代码位置
    ret: any - 返回值
    note: string|nil - 备注
]]
local Step = {}
Step.__index = Step

--@description: 创建Step实例
--@param id: number - 步骤ID
--@param op: string - 操作名称
--@param before: number - 操作前状态ID
--@param after: number|nil - 操作后状态ID
--@param args: table|nil - 操作参数
--@param code: CodeLocation|nil - 代码位置
--@param ret: any - 返回值
--@param note: string|nil - 备注
--@return: Step - Step实例
function Step.new(id, op, before, after, args, code, ret, note)
    local self = setmetatable({}, Step)
    self.id = id
    self.op = op
    self.before = before
    self.after = after
    self.args = args or {}
    self.code = code
    self.ret = ret
    self.note = note
    return self
end

M.Step = Step

--[[
Commit提交信息类

Class:
    Commit
Fields:
    op: string - 操作名称
    line: number - 行号
]]
local Commit = {}
Commit.__index = Commit

--@description: 创建Commit实例
--@param op: string - 操作名称
--@param line: number - 行号
--@return: Commit - Commit实例
function Commit.new(op, line)
    local self = setmetatable({}, Commit)
    self.op = op
    self.line = line
    return self
end

M.Commit = Commit

--[[
Result成功结果类

Class:
    Result
Fields:
    final_state: number - 最终状态ID
    commit: Commit|nil - 提交信息
]]
local Result = {}
Result.__index = Result

--@description: 创建Result实例
--@param final_state: number - 最终状态ID
--@param commit: Commit|nil - 提交信息
--@return: Result - Result实例
function Result.new(final_state, commit)
    local self = setmetatable({}, Result)
    self.final_state = final_state
    self.commit = commit
    return self
end

M.Result = Result

--[[
Error错误信息类

Class:
    Error
Fields:
    type: string - 错误类型
    message: string - 错误消息
    line: number|nil - 行号
    step: number|nil - 步骤ID
    last_state: number|nil - 最后状态ID
]]
local Error = {}
Error.__index = Error

--@description: 创建Error实例
--@param err_type: string - 错误类型
--@param message: string - 错误消息
--@param line: number|nil - 行号
--@param step: number|nil - 步骤ID
--@param last_state: number|nil - 最后状态ID
--@return: Error - Error实例
function Error.new(err_type, message, line, step, last_state)
    local self = setmetatable({}, Error)
    self.type = err_type
    self.message = message:sub(1, 512)
    self.line = line
    self.step = step
    self.last_state = last_state
    return self
end

M.Error = Error

--[[
Trace完整操作轨迹记录类

Class:
    Trace
Fields:
    meta: Meta - 元数据
    package: Package - 包信息
    remarks: Remarks - 备注信息
    object: Object - 数据结构对象
    states: table - 状态列表
    steps: table - 步骤列表
    result: Result|nil - 成功结果
    error: Error|nil - 错误信息
]]
local Trace = {}
Trace.__index = Trace

--@description: 创建Trace实例
--@param meta: Meta - 元数据
--@param package: Package - 包信息
--@param remarks: Remarks - 备注信息
--@param object: Object - 数据结构对象
--@param states: table|nil - 状态列表
--@param steps: table|nil - 步骤列表
--@param result: Result|nil - 成功结果
--@param err: Error|nil - 错误信息
--@return: Trace - Trace实例
function Trace.new(meta, package, remarks, object, states, steps, result, err)
    local self = setmetatable({}, Trace)
    self.meta = meta
    self.package = package
    self.remarks = remarks
    self.object = object
    self.states = states or {}
    self.steps = steps or {}
    self.result = result
    self.error = err
    return self
end

M.Trace = Trace

--[[
TomlWriter TOML格式写入器类

Class:
    TomlWriter
Fields:
    trace: Trace - 要序列化的Trace对象
]]
local TomlWriter = {}
TomlWriter.__index = TomlWriter

--@description: 创建TomlWriter实例
--@param trace: Trace - 要序列化的Trace对象
--@return: TomlWriter - TomlWriter实例
function TomlWriter.new(trace)
    local self = setmetatable({}, TomlWriter)
    self.trace = trace
    return self
end

--@description: 序列化meta部分
--@param self: TomlWriter - TomlWriter实例
--@return: table - TOML行列表
function TomlWriter:serializeMeta()
    return {
        "[meta]",
        string.format('created_at = "%s"', self.trace.meta.created_at),
        string.format('lang = "%s"', self.trace.meta.lang),
        string.format('lang_version = "%s"', self.trace.meta.lang_version),
    }
end

--@description: 序列化package部分
--@param self: TomlWriter - TomlWriter实例
--@return: table - TOML行列表
function TomlWriter:serializePackage()
    return {
        "[package]",
        string.format('name = "%s"', self.trace.package.name),
        string.format('lang = "%s"', self.trace.package.lang),
        string.format('version = "%s"', self.trace.package.version),
    }
end

--@description: 序列化remarks部分
--@param self: TomlWriter - TomlWriter实例
--@return: table - TOML行列表
function TomlWriter:serializeRemarks()
    local lines = { "[remarks]" }
    if self.trace.remarks.title ~= "" then
        lines[#lines + 1] = string.format('title = "%s"', escapeString(self.trace.remarks.title))
    end
    if self.trace.remarks.author ~= "" then
        lines[#lines + 1] = string.format('author = "%s"', escapeString(self.trace.remarks.author))
    end
    if self.trace.remarks.comment ~= "" then
        lines[#lines + 1] = string.format('comment = "%s"', escapeString(self.trace.remarks.comment))
    end
    return lines
end

--@description: 序列化object部分
--@param self: TomlWriter - TomlWriter实例
--@return: table - TOML行列表
function TomlWriter:serializeObject()
    local lines = {
        "[object]",
        string.format('kind = "%s"', self.trace.object.kind),
    }
    if self.trace.object.label ~= "" then
        lines[#lines + 1] = string.format('label = "%s"', escapeString(self.trace.object.label))
    end
    return lines
end

--@description: 序列化state.data部分
--@param self: TomlWriter - TomlWriter实例
--@param data: table - 状态数据字典
--@return: table - TOML行列表
function TomlWriter:serializeStateData(data)
    local lines = {}
    local keys = {}
    for k in pairs(data) do
        keys[#keys + 1] = k
    end
    table.sort(keys)
    for _, key in ipairs(keys) do
        local value = data[key]
        if key == "nodes" or key == "edges" then
            lines[#lines + 1] = string.format("%s = %s", key, serializeInlineTableArray(value))
        elseif key == "items" then
            lines[#lines + 1] = string.format("%s = %s", key, serializeSimpleArray(value))
        else
            lines[#lines + 1] = string.format("%s = %s", key, serializeValue(value))
        end
    end
    return lines
end

--@description: 序列化单个state
--@param self: TomlWriter - TomlWriter实例
--@param state: State - 状态对象
--@return: table - TOML行列表
function TomlWriter:serializeState(state)
    local lines = {
        "[[states]]",
        string.format("id = %d", state.id),
        "",
        "[states.data]",
    }
    local data_lines = self:serializeStateData(state.data)
    for _, line in ipairs(data_lines) do
        lines[#lines + 1] = line
    end
    return lines
end

--@description: 序列化单个step
--@param self: TomlWriter - TomlWriter实例
--@param step: Step - 步骤对象
--@return: table - TOML行列表
function TomlWriter:serializeStep(step)
    local lines = {
        "[[steps]]",
        string.format("id = %d", step.id),
        string.format('op = "%s"', step.op),
        string.format("before = %d", step.before),
    }
    if step.after ~= nil then
        lines[#lines + 1] = string.format("after = %d", step.after)
    end
    if step.ret ~= nil then
        lines[#lines + 1] = string.format("ret = %s", serializeValue(step.ret))
    end
    if step.note then
        lines[#lines + 1] = string.format('note = "%s"', escapeString(step.note))
    end
    lines[#lines + 1] = ""
    lines[#lines + 1] = "[steps.args]"
    if next(step.args) then
        local keys = {}
        for k in pairs(step.args) do
            keys[#keys + 1] = k
        end
        table.sort(keys)
        for _, key in ipairs(keys) do
            lines[#lines + 1] = string.format("%s = %s", key, serializeValue(step.args[key]))
        end
    end
    if step.code then
        lines[#lines + 1] = ""
        lines[#lines + 1] = "[steps.code]"
        lines[#lines + 1] = string.format("line = %d", step.code.line)
        if step.code.col then
            lines[#lines + 1] = string.format("col = %d", step.code.col)
        end
    end
    return lines
end

--@description: 序列化result部分
--@param self: TomlWriter - TomlWriter实例
--@return: table - TOML行列表
function TomlWriter:serializeResult()
    if not self.trace.result then
        return {}
    end
    local lines = {
        "[result]",
        string.format("final_state = %d", self.trace.result.final_state),
    }
    if self.trace.result.commit then
        lines[#lines + 1] = ""
        lines[#lines + 1] = "[result.commit]"
        lines[#lines + 1] = string.format('op = "%s"', self.trace.result.commit.op)
        lines[#lines + 1] = string.format("line = %d", self.trace.result.commit.line)
    end
    return lines
end

--@description: 序列化error部分
--@param self: TomlWriter - TomlWriter实例
--@return: table - TOML行列表
function TomlWriter:serializeError()
    if not self.trace.error then
        return {}
    end
    local lines = {
        "[error]",
        string.format('type = "%s"', self.trace.error.type),
        string.format('message = "%s"', escapeString(self.trace.error.message)),
    }
    if self.trace.error.line then
        lines[#lines + 1] = string.format("line = %d", self.trace.error.line)
    end
    if self.trace.error.step then
        lines[#lines + 1] = string.format("step = %d", self.trace.error.step)
    end
    if self.trace.error.last_state then
        lines[#lines + 1] = string.format("last_state = %d", self.trace.error.last_state)
    end
    return lines
end

--@description: 将Trace序列化为TOML字符串
--@param self: TomlWriter - TomlWriter实例
--@return: string - TOML格式的字符串
function TomlWriter:serialize()
    local lines = {}
    local function appendLines(new_lines)
        for _, line in ipairs(new_lines) do
            lines[#lines + 1] = line
        end
    end
    appendLines(self:serializeMeta())
    lines[#lines + 1] = ""
    appendLines(self:serializePackage())
    if not self.trace.remarks:isEmpty() then
        lines[#lines + 1] = ""
        appendLines(self:serializeRemarks())
    end
    lines[#lines + 1] = ""
    appendLines(self:serializeObject())
    for _, state in ipairs(self.trace.states) do
        lines[#lines + 1] = ""
        appendLines(self:serializeState(state))
    end
    for _, step in ipairs(self.trace.steps) do
        lines[#lines + 1] = ""
        appendLines(self:serializeStep(step))
    end
    if self.trace.result then
        lines[#lines + 1] = ""
        appendLines(self:serializeResult())
    end
    if self.trace.error then
        lines[#lines + 1] = ""
        appendLines(self:serializeError())
    end
    return table.concat(lines, "\n")
end

--@description: 将Trace写入指定路径的TOML文件
--@param self: TomlWriter - TomlWriter实例
--@param path: string - 输出文件路径
--@return: nil
function TomlWriter:write(path)
    local content = self:serialize()
    local file = assert(io.open(path, "w"))
    file:write(content)
    file:close()
end

M.TomlWriter = TomlWriter

return M
