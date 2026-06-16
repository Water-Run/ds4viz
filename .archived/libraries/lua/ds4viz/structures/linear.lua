--[[
线性数据结构实现

提供栈、队列、单链表、双向链表的实现

Author:
    WaterRun
File:
    ds4viz/structures/linear.lua
Date:
    2025-12-24
Updated:
    2026-01-02
]]

local session = require("ds4viz.session")

local M = {}

--[[
Stack栈数据结构

Class:
    Stack
Fields:
    items: table - 栈元素列表
]]
local Stack = setmetatable({}, { __index = session.BaseStructure })
Stack.__index = Stack

--@description: 创建栈实例
--@param label: string|nil - 栈的标签，默认为"stack"
--@param output: string|nil - 输出文件路径
--@return: Stack - 栈实例
function Stack.new(label, output)
    local base = session.BaseStructure.new("stack", label or "stack", output)
    local self = setmetatable(base, Stack)
    self.items = {}
    return self
end

--@description: 构建栈的状态数据
--@param self: Stack - Stack实例
--@return: table - 状态数据字典
function Stack:buildData()
    local items_copy = {}
    for i, v in ipairs(self.items) do
        items_copy[i] = v
    end
    return {
        items = items_copy,
        top = #self.items > 0 and (#self.items - 1) or -1
    }
end

--@description: 压栈操作
--@param self: Stack - Stack实例
--@param value: number|string|boolean - 要压入的值
--@return: nil
function Stack:push(value)
    local before = self.session:getLastStateId()
    local line = session.getCallerLine(3)
    self.items[#self.items + 1] = value
    local after = self.session:addState(self:buildData())
    self.session:addStep("push", before, after, { value = value }, nil, nil, line)
end

--@description: 弹栈操作
--@param self: Stack - Stack实例
--@return: nil
--@raise: error - 当栈为空时抛出异常
function Stack:pop()
    if #self.items == 0 then
        self:raiseError("Cannot pop from empty stack")
    end
    local before = self.session:getLastStateId()
    local line = session.getCallerLine(3)
    local popped = table.remove(self.items)
    local after = self.session:addState(self:buildData())
    self.session:addStep("pop", before, after, {}, popped, nil, line)
end

--@description: 清空栈
--@param self: Stack - Stack实例
--@return: nil
function Stack:clear()
    local before = self.session:getLastStateId()
    local line = session.getCallerLine(3)
    self.items = {}
    local after = self.session:addState(self:buildData())
    self.session:addStep("clear", before, after, {}, nil, nil, line)
end

M.Stack = Stack

--[[
Queue队列数据结构

Class:
    Queue
Fields:
    items: table - 队列元素列表
]]
local Queue = setmetatable({}, { __index = session.BaseStructure })
Queue.__index = Queue

--@description: 创建队列实例
--@param label: string|nil - 队列的标签，默认为"queue"
--@param output: string|nil - 输出文件路径
--@return: Queue - 队列实例
function Queue.new(label, output)
    local base = session.BaseStructure.new("queue", label or "queue", output)
    local self = setmetatable(base, Queue)
    self.items = {}
    return self
end

--@description: 构建队列的状态数据
--@param self: Queue - Queue实例
--@return: table - 状态数据字典
function Queue:buildData()
    local items_copy = {}
    for i, v in ipairs(self.items) do
        items_copy[i] = v
    end
    if #self.items == 0 then
        return { items = items_copy, front = -1, rear = -1 }
    end
    return {
        items = items_copy,
        front = 0,
        rear = #self.items - 1
    }
end

--@description: 入队操作
--@param self: Queue - Queue实例
--@param value: number|string|boolean - 要入队的值
--@return: nil
function Queue:enqueue(value)
    local before = self.session:getLastStateId()
    local line = session.getCallerLine(3)
    self.items[#self.items + 1] = value
    local after = self.session:addState(self:buildData())
    self.session:addStep("enqueue", before, after, { value = value }, nil, nil, line)
end

--@description: 出队操作
--@param self: Queue - Queue实例
--@return: nil
--@raise: error - 当队列为空时抛出异常
function Queue:dequeue()
    if #self.items == 0 then
        self:raiseError("Cannot dequeue from empty queue")
    end
    local before = self.session:getLastStateId()
    local line = session.getCallerLine(3)
    local dequeued = table.remove(self.items, 1)
    local after = self.session:addState(self:buildData())
    self.session:addStep("dequeue", before, after, {}, dequeued, nil, line)
end

--@description: 清空队列
--@param self: Queue - Queue实例
--@return: nil
function Queue:clear()
    local before = self.session:getLastStateId()
    local line = session.getCallerLine(3)
    self.items = {}
    local after = self.session:addState(self:buildData())
    self.session:addStep("clear", before, after, {}, nil, nil, line)
end

M.Queue = Queue

--[[
SingleLinkedList单链表数据结构

Class:
    SingleLinkedList
Fields:
    nodes: table - 节点字典
    head: number - 头节点ID
    next_id: number - 下一个可用节点ID
]]
local SingleLinkedList = setmetatable({}, { __index = session.BaseStructure })
SingleLinkedList.__index = SingleLinkedList

--@description: 创建单链表实例
--@param label: string|nil - 单链表的标签，默认为"slist"
--@param output: string|nil - 输出文件路径
--@return: SingleLinkedList - 单链表实例
function SingleLinkedList.new(label, output)
    local base = session.BaseStructure.new("slist", label or "slist", output)
    local self = setmetatable(base, SingleLinkedList)
    self.nodes = {}
    self.head = -1
    self.next_id = 0
    return self
end

--@description: 构建单链表的状态数据
--@param self: SingleLinkedList - SingleLinkedList实例
--@return: table - 状态数据字典
function SingleLinkedList:buildData()
    local nodes_list = {}
    local ids = {}
    for id in pairs(self.nodes) do
        ids[#ids + 1] = id
    end
    table.sort(ids)
    for _, nid in ipairs(ids) do
        local data = self.nodes[nid]
        nodes_list[#nodes_list + 1] = {
            id = nid,
            value = data.value,
            next = data.next
        }
    end
    return { head = self.head, nodes = nodes_list }
end

--@description: 查找尾节点ID
--@param self: SingleLinkedList - SingleLinkedList实例
--@return: number - 尾节点ID，若链表为空则返回-1
--@local: true
function SingleLinkedList:findTail()
    if self.head == -1 then
        return -1
    end
    local current = self.head
    while self.nodes[current].next ~= -1 do
        current = self.nodes[current].next
    end
    return current
end

--@description: 查找指定节点的前驱节点ID
--@param self: SingleLinkedList - SingleLinkedList实例
--@param node_id: number - 目标节点ID
--@return: number - 前驱节点ID，若为头节点或不存在则返回-1
--@local: true
function SingleLinkedList:findPrev(node_id)
    if self.head == -1 or self.head == node_id then
        return -1
    end
    local current = self.head
    while current ~= -1 do
        if self.nodes[current].next == node_id then
            return current
        end
        current = self.nodes[current].next
    end
    return -1
end

--@description: 在链表头部插入节点
--@param self: SingleLinkedList - SingleLinkedList实例
--@param value: number|string|boolean - 要插入的值
--@return: number - 新插入节点的id
function SingleLinkedList:insertHead(value)
    local before = self.session:getLastStateId()
    local line = session.getCallerLine(3)
    local new_id = self.next_id
    self.next_id = self.next_id + 1
    self.nodes[new_id] = { value = value, next = self.head }
    self.head = new_id
    local after = self.session:addState(self:buildData())
    self.session:addStep("insert_head", before, after, { value = value }, new_id, nil, line)
    return new_id
end

--@description: 在链表尾部插入节点
--@param self: SingleLinkedList - SingleLinkedList实例
--@param value: number|string|boolean - 要插入的值
--@return: number - 新插入节点的id
function SingleLinkedList:insertTail(value)
    local before = self.session:getLastStateId()
    local line = session.getCallerLine(3)
    local new_id = self.next_id
    self.next_id = self.next_id + 1
    self.nodes[new_id] = { value = value, next = -1 }
    if self.head == -1 then
        self.head = new_id
    else
        local tail = self:findTail()
        self.nodes[tail].next = new_id
    end
    local after = self.session:addState(self:buildData())
    self.session:addStep("insert_tail", before, after, { value = value }, new_id, nil, line)
    return new_id
end

--@description: 在指定节点后插入新节点
--@param self: SingleLinkedList - SingleLinkedList实例
--@param node_id: number - 目标节点的id
--@param value: number|string|boolean - 要插入的值
--@return: number - 新插入节点的id
--@raise: error - 当指定节点不存在时抛出异常
function SingleLinkedList:insertAfter(node_id, value)
    if not self.nodes[node_id] then
        self:raiseError(string.format("Node not found: %d", node_id))
    end
    local before = self.session:getLastStateId()
    local line = session.getCallerLine(3)
    local new_id = self.next_id
    self.next_id = self.next_id + 1
    local old_next = self.nodes[node_id].next
    self.nodes[new_id] = { value = value, next = old_next }
    self.nodes[node_id].next = new_id
    local after = self.session:addState(self:buildData())
    self.session:addStep("insert_after", before, after, { node_id = node_id, value = value }, new_id, nil, line)
    return new_id
end

--@description: 删除指定节点
--@param self: SingleLinkedList - SingleLinkedList实例
--@param node_id: number - 要删除的节点id
--@return: nil
--@raise: error - 当指定节点不存在时抛出异常
function SingleLinkedList:delete(node_id)
    if not self.nodes[node_id] then
        self:raiseError(string.format("Node not found: %d", node_id))
    end
    local before = self.session:getLastStateId()
    local line = session.getCallerLine(3)
    local deleted_value = self.nodes[node_id].value
    if self.head == node_id then
        self.head = self.nodes[node_id].next
    else
        local prev = self:findPrev(node_id)
        if prev ~= -1 then
            self.nodes[prev].next = self.nodes[node_id].next
        end
    end
    self.nodes[node_id] = nil
    local after = self.session:addState(self:buildData())
    self.session:addStep("delete", before, after, { node_id = node_id }, deleted_value, nil, line)
end

--@description: 删除头节点
--@param self: SingleLinkedList - SingleLinkedList实例
--@return: nil
--@raise: error - 当链表为空时抛出异常
function SingleLinkedList:deleteHead()
    if self.head == -1 then
        self:raiseError("Cannot delete from empty list")
    end
    local before = self.session:getLastStateId()
    local line = session.getCallerLine(3)
    local old_head = self.head
    local deleted_value = self.nodes[old_head].value
    self.head = self.nodes[old_head].next
    self.nodes[old_head] = nil
    local after = self.session:addState(self:buildData())
    self.session:addStep("delete_head", before, after, {}, deleted_value, nil, line)
end

--@description: 反转链表
--@param self: SingleLinkedList - SingleLinkedList实例
--@return: nil
function SingleLinkedList:reverse()
    local before = self.session:getLastStateId()
    local line = session.getCallerLine(3)
    local prev = -1
    local current = self.head
    while current ~= -1 do
        local next_node = self.nodes[current].next
        self.nodes[current].next = prev
        prev = current
        current = next_node
    end
    self.head = prev
    local after = self.session:addState(self:buildData())
    self.session:addStep("reverse", before, after, {}, nil, nil, line)
end

M.SingleLinkedList = SingleLinkedList

--[[
DoubleLinkedList双向链表数据结构

Class:
    DoubleLinkedList
Fields:
    nodes: table - 节点字典
    head: number - 头节点ID
    tail: number - 尾节点ID
    next_id: number - 下一个可用节点ID
]]
local DoubleLinkedList = setmetatable({}, { __index = session.BaseStructure })
DoubleLinkedList.__index = DoubleLinkedList

--@description: 创建双向链表实例
--@param label: string|nil - 双向链表的标签，默认为"dlist"
--@param output: string|nil - 输出文件路径
--@return: DoubleLinkedList - 双向链表实例
function DoubleLinkedList.new(label, output)
    local base = session.BaseStructure.new("dlist", label or "dlist", output)
    local self = setmetatable(base, DoubleLinkedList)
    self.nodes = {}
    self.head = -1
    self.tail = -1
    self.next_id = 0
    return self
end

--@description: 构建双向链表的状态数据
--@param self: DoubleLinkedList - DoubleLinkedList实例
--@return: table - 状态数据字典
function DoubleLinkedList:buildData()
    local nodes_list = {}
    local ids = {}
    for id in pairs(self.nodes) do
        ids[#ids + 1] = id
    end
    table.sort(ids)
    for _, nid in ipairs(ids) do
        local data = self.nodes[nid]
        nodes_list[#nodes_list + 1] = {
            id = nid,
            value = data.value,
            prev = data.prev,
            next = data.next
        }
    end
    return { head = self.head, tail = self.tail, nodes = nodes_list }
end

--@description: 在链表头部插入节点
--@param self: DoubleLinkedList - DoubleLinkedList实例
--@param value: number|string|boolean - 要插入的值
--@return: number - 新插入节点的id
function DoubleLinkedList:insertHead(value)
    local before = self.session:getLastStateId()
    local line = session.getCallerLine(3)
    local new_id = self.next_id
    self.next_id = self.next_id + 1
    self.nodes[new_id] = { value = value, prev = -1, next = self.head }
    if self.head ~= -1 then
        self.nodes[self.head].prev = new_id
    end
    self.head = new_id
    if self.tail == -1 then
        self.tail = new_id
    end
    local after = self.session:addState(self:buildData())
    self.session:addStep("insert_head", before, after, { value = value }, new_id, nil, line)
    return new_id
end

--@description: 在链表尾部插入节点
--@param self: DoubleLinkedList - DoubleLinkedList实例
--@param value: number|string|boolean - 要插入的值
--@return: number - 新插入节点的id
function DoubleLinkedList:insertTail(value)
    local before = self.session:getLastStateId()
    local line = session.getCallerLine(3)
    local new_id = self.next_id
    self.next_id = self.next_id + 1
    self.nodes[new_id] = { value = value, prev = self.tail, next = -1 }
    if self.tail ~= -1 then
        self.nodes[self.tail].next = new_id
    end
    self.tail = new_id
    if self.head == -1 then
        self.head = new_id
    end
    local after = self.session:addState(self:buildData())
    self.session:addStep("insert_tail", before, after, { value = value }, new_id, nil, line)
    return new_id
end

--@description: 在指定节点前插入新节点
--@param self: DoubleLinkedList - DoubleLinkedList实例
--@param node_id: number - 目标节点的id
--@param value: number|string|boolean - 要插入的值
--@return: number - 新插入节点的id
--@raise: error - 当指定节点不存在时抛出异常
function DoubleLinkedList:insertBefore(node_id, value)
    if not self.nodes[node_id] then
        self:raiseError(string.format("Node not found: %d", node_id))
    end
    local before = self.session:getLastStateId()
    local line = session.getCallerLine(3)
    local new_id = self.next_id
    self.next_id = self.next_id + 1
    local old_prev = self.nodes[node_id].prev
    self.nodes[new_id] = { value = value, prev = old_prev, next = node_id }
    self.nodes[node_id].prev = new_id
    if old_prev ~= -1 then
        self.nodes[old_prev].next = new_id
    else
        self.head = new_id
    end
    local after = self.session:addState(self:buildData())
    self.session:addStep("insert_before", before, after, { node_id = node_id, value = value }, new_id, nil, line)
    return new_id
end

--@description: 在指定节点后插入新节点
--@param self: DoubleLinkedList - DoubleLinkedList实例
--@param node_id: number - 目标节点的id
--@param value: number|string|boolean - 要插入的值
--@return: number - 新插入节点的id
--@raise: error - 当指定节点不存在时抛出异常
function DoubleLinkedList:insertAfter(node_id, value)
    if not self.nodes[node_id] then
        self:raiseError(string.format("Node not found: %d", node_id))
    end
    local before = self.session:getLastStateId()
    local line = session.getCallerLine(3)
    local new_id = self.next_id
    self.next_id = self.next_id + 1
    local old_next = self.nodes[node_id].next
    self.nodes[new_id] = { value = value, prev = node_id, next = old_next }
    self.nodes[node_id].next = new_id
    if old_next ~= -1 then
        self.nodes[old_next].prev = new_id
    else
        self.tail = new_id
    end
    local after = self.session:addState(self:buildData())
    self.session:addStep("insert_after", before, after, { node_id = node_id, value = value }, new_id, nil, line)
    return new_id
end

--@description: 删除指定节点
--@param self: DoubleLinkedList - DoubleLinkedList实例
--@param node_id: number - 要删除的节点id
--@return: nil
--@raise: error - 当指定节点不存在时抛出异常
function DoubleLinkedList:delete(node_id)
    if not self.nodes[node_id] then
        self:raiseError(string.format("Node not found: %d", node_id))
    end
    local before = self.session:getLastStateId()
    local line = session.getCallerLine(3)
    local deleted_value = self.nodes[node_id].value
    local prev_id = self.nodes[node_id].prev
    local next_id = self.nodes[node_id].next
    if prev_id ~= -1 then
        self.nodes[prev_id].next = next_id
    else
        self.head = next_id
    end
    if next_id ~= -1 then
        self.nodes[next_id].prev = prev_id
    else
        self.tail = prev_id
    end
    self.nodes[node_id] = nil
    local after = self.session:addState(self:buildData())
    self.session:addStep("delete", before, after, { node_id = node_id }, deleted_value, nil, line)
end

--@description: 删除头节点
--@param self: DoubleLinkedList - DoubleLinkedList实例
--@return: nil
--@raise: error - 当链表为空时抛出异常
function DoubleLinkedList:deleteHead()
    if self.head == -1 then
        self:raiseError("Cannot delete from empty list")
    end
    self:delete(self.head)
end

--@description: 删除尾节点
--@param self: DoubleLinkedList - DoubleLinkedList实例
--@return: nil
--@raise: error - 当链表为空时抛出异常
function DoubleLinkedList:deleteTail()
    if self.tail == -1 then
        self:raiseError("Cannot delete from empty list")
    end
    self:delete(self.tail)
end

--@description: 反转链表
--@param self: DoubleLinkedList - DoubleLinkedList实例
--@return: nil
function DoubleLinkedList:reverse()
    local before = self.session:getLastStateId()
    local line = session.getCallerLine(3)
    local current = self.head
    while current ~= -1 do
        local node = self.nodes[current]
        node.prev, node.next = node.next, node.prev
        current = node.prev
    end
    self.head, self.tail = self.tail, self.head
    local after = self.session:addState(self:buildData())
    self.session:addStep("reverse", before, after, {}, nil, nil, line)
end

M.DoubleLinkedList = DoubleLinkedList

return M
