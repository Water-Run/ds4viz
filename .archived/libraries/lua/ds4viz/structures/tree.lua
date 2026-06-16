--[[
树形数据结构实现

提供二叉树、二叉搜索树、堆的实现

Author:
    WaterRun
File:
    ds4viz/structures/tree.lua
Date:
    2025-12-24
Updated:
    2026-01-02
]]

local session = require("ds4viz.session")

local M = {}

--[[
BinaryTree二叉树数据结构

Class:
    BinaryTree
Fields:
    nodes: table - 节点字典
    root: number - 根节点ID
    next_id: number - 下一个可用节点ID
]]
local BinaryTree = setmetatable({}, { __index = session.BaseStructure })
BinaryTree.__index = BinaryTree

--@description: 创建二叉树实例
--@param label: string|nil - 二叉树的标签，默认为"binary_tree"
--@param output: string|nil - 输出文件路径
--@return: BinaryTree - 二叉树实例
function BinaryTree.new(label, output)
    local base = session.BaseStructure.new("binary_tree", label or "binary_tree", output)
    local self = setmetatable(base, BinaryTree)
    self.nodes = {}
    self.root = -1
    self.next_id = 0
    return self
end

--@description: 构建二叉树的状态数据
--@param self: BinaryTree - BinaryTree实例
--@return: table - 状态数据字典
function BinaryTree:buildData()
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
            left = data.left,
            right = data.right
        }
    end
    return { root = self.root, nodes = nodes_list }
end

--@description: 查找指定节点的父节点ID
--@param self: BinaryTree - BinaryTree实例
--@param node_id: number - 目标节点ID
--@return: number - 父节点ID，若为根节点或不存在则返回-1
--@local: true
function BinaryTree:findParent(node_id)
    for nid, data in pairs(self.nodes) do
        if data.left == node_id or data.right == node_id then
            return nid
        end
    end
    return -1
end

--@description: 收集以指定节点为根的子树中所有节点ID
--@param self: BinaryTree - BinaryTree实例
--@param node_id: number - 根节点ID
--@return: table - 子树中所有节点ID列表
--@local: true
function BinaryTree:collectSubtree(node_id)
    if node_id == -1 then
        return {}
    end
    local result = { node_id }
    local left_subtree = self:collectSubtree(self.nodes[node_id].left)
    local right_subtree = self:collectSubtree(self.nodes[node_id].right)
    for _, id in ipairs(left_subtree) do
        result[#result + 1] = id
    end
    for _, id in ipairs(right_subtree) do
        result[#result + 1] = id
    end
    return result
end

--@description: 插入根节点
--@param self: BinaryTree - BinaryTree实例
--@param value: number|string|boolean - 根节点的值
--@return: number - 根节点的id
--@raise: error - 当根节点已存在时抛出异常
function BinaryTree:insertRoot(value)
    if self.root ~= -1 then
        self:raiseError("Root node already exists")
    end
    local before = self.session:getLastStateId()
    local line = session.getCallerLine(3)
    local new_id = self.next_id
    self.next_id = self.next_id + 1
    self.nodes[new_id] = { value = value, left = -1, right = -1 }
    self.root = new_id
    local after = self.session:addState(self:buildData())
    self.session:addStep("insert_root", before, after, { value = value }, new_id, nil, line)
    return new_id
end

--@description: 在指定父节点的左侧插入子节点
--@param self: BinaryTree - BinaryTree实例
--@param parent_id: number - 父节点的id
--@param value: number|string|boolean - 要插入的子节点值
--@return: number - 新插入节点的id
--@raise: error - 当父节点不存在或左子节点已存在时抛出异常
function BinaryTree:insertLeft(parent_id, value)
    if not self.nodes[parent_id] then
        self:raiseError(string.format("Parent node not found: %d", parent_id))
    end
    if self.nodes[parent_id].left ~= -1 then
        self:raiseError(string.format("Left child already exists for node: %d", parent_id))
    end
    local before = self.session:getLastStateId()
    local line = session.getCallerLine(3)
    local new_id = self.next_id
    self.next_id = self.next_id + 1
    self.nodes[new_id] = { value = value, left = -1, right = -1 }
    self.nodes[parent_id].left = new_id
    local after = self.session:addState(self:buildData())
    self.session:addStep("insert_left", before, after, { parent = parent_id, value = value }, new_id, nil, line)
    return new_id
end

--@description: 在指定父节点的右侧插入子节点
--@param self: BinaryTree - BinaryTree实例
--@param parent_id: number - 父节点的id
--@param value: number|string|boolean - 要插入的子节点值
--@return: number - 新插入节点的id
--@raise: error - 当父节点不存在或右子节点已存在时抛出异常
function BinaryTree:insertRight(parent_id, value)
    if not self.nodes[parent_id] then
        self:raiseError(string.format("Parent node not found: %d", parent_id))
    end
    if self.nodes[parent_id].right ~= -1 then
        self:raiseError(string.format("Right child already exists for node: %d", parent_id))
    end
    local before = self.session:getLastStateId()
    local line = session.getCallerLine(3)
    local new_id = self.next_id
    self.next_id = self.next_id + 1
    self.nodes[new_id] = { value = value, left = -1, right = -1 }
    self.nodes[parent_id].right = new_id
    local after = self.session:addState(self:buildData())
    self.session:addStep("insert_right", before, after, { parent = parent_id, value = value }, new_id, nil, line)
    return new_id
end

--@description: 删除指定节点及其所有子树
--@param self: BinaryTree - BinaryTree实例
--@param node_id: number - 要删除的节点id
--@return: nil
--@raise: error - 当节点不存在时抛出异常
function BinaryTree:delete(node_id)
    if not self.nodes[node_id] then
        self:raiseError(string.format("Node not found: %d", node_id))
    end
    local before = self.session:getLastStateId()
    local line = session.getCallerLine(3)
    local subtree_ids = self:collectSubtree(node_id)
    local parent_id = self:findParent(node_id)
    if parent_id ~= -1 then
        if self.nodes[parent_id].left == node_id then
            self.nodes[parent_id].left = -1
        else
            self.nodes[parent_id].right = -1
        end
    else
        self.root = -1
    end
    for _, nid in ipairs(subtree_ids) do
        self.nodes[nid] = nil
    end
    local after = self.session:addState(self:buildData())
    self.session:addStep("delete", before, after, { node_id = node_id }, nil, nil, line)
end

--@description: 更新节点的值
--@param self: BinaryTree - BinaryTree实例
--@param node_id: number - 节点id
--@param value: number|string|boolean - 新的值
--@return: nil
--@raise: error - 当节点不存在时抛出异常
function BinaryTree:updateValue(node_id, value)
    if not self.nodes[node_id] then
        self:raiseError(string.format("Node not found: %d", node_id))
    end
    local before = self.session:getLastStateId()
    local line = session.getCallerLine(3)
    local old_value = self.nodes[node_id].value
    self.nodes[node_id].value = value
    local after = self.session:addState(self:buildData())
    self.session:addStep("update_value", before, after, { node_id = node_id, value = value }, old_value, nil, line)
end

M.BinaryTree = BinaryTree

--[[
BinarySearchTree二叉搜索树数据结构

Class:
    BinarySearchTree
Fields:
    nodes: table - 节点字典
    root: number - 根节点ID
    next_id: number - 下一个可用节点ID
]]
local BinarySearchTree = setmetatable({}, { __index = session.BaseStructure })
BinarySearchTree.__index = BinarySearchTree

--@description: 创建二叉搜索树实例
--@param label: string|nil - 二叉搜索树的标签，默认为"bst"
--@param output: string|nil - 输出文件路径
--@return: BinarySearchTree - 二叉搜索树实例
function BinarySearchTree.new(label, output)
    local base = session.BaseStructure.new("bst", label or "bst", output)
    local self = setmetatable(base, BinarySearchTree)
    self.nodes = {}
    self.root = -1
    self.next_id = 0
    return self
end

--@description: 构建二叉搜索树的状态数据
--@param self: BinarySearchTree - BinarySearchTree实例
--@return: table - 状态数据字典
function BinarySearchTree:buildData()
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
            left = data.left,
            right = data.right
        }
    end
    return { root = self.root, nodes = nodes_list }
end

--@description: 根据值查找节点ID
--@param self: BinarySearchTree - BinarySearchTree实例
--@param value: number - 要查找的值
--@return: number - 节点ID，若不存在则返回-1
--@local: true
function BinarySearchTree:findNodeByValue(value)
    for nid, data in pairs(self.nodes) do
        if data.value == value then
            return nid
        end
    end
    return -1
end

--@description: 查找指定节点的父节点ID
--@param self: BinarySearchTree - BinarySearchTree实例
--@param node_id: number - 目标节点ID
--@return: number - 父节点ID，若为根节点或不存在则返回-1
--@local: true
function BinarySearchTree:findParent(node_id)
    for nid, data in pairs(self.nodes) do
        if data.left == node_id or data.right == node_id then
            return nid
        end
    end
    return -1
end

--@description: 查找以指定节点为根的子树中的最小值节点
--@param self: BinarySearchTree - BinarySearchTree实例
--@param node_id: number - 子树根节点ID
--@return: number - 最小值节点ID
--@local: true
function BinarySearchTree:findMinNode(node_id)
    local current = node_id
    while self.nodes[current].left ~= -1 do
        current = self.nodes[current].left
    end
    return current
end

--@description: 插入节点，自动维护二叉搜索树性质
--@param self: BinarySearchTree - BinarySearchTree实例
--@param value: number - 要插入的值
--@return: number - 新插入节点的id
function BinarySearchTree:insert(value)
    local before = self.session:getLastStateId()
    local line = session.getCallerLine(3)
    local new_id = self.next_id
    self.next_id = self.next_id + 1
    self.nodes[new_id] = { value = value, left = -1, right = -1 }
    if self.root == -1 then
        self.root = new_id
    else
        local current = self.root
        while true do
            local current_value = self.nodes[current].value
            if value < current_value then
                if self.nodes[current].left == -1 then
                    self.nodes[current].left = new_id
                    break
                end
                current = self.nodes[current].left
            else
                if self.nodes[current].right == -1 then
                    self.nodes[current].right = new_id
                    break
                end
                current = self.nodes[current].right
            end
        end
    end
    local after = self.session:addState(self:buildData())
    self.session:addStep("insert", before, after, { value = value }, new_id, nil, line)
    return new_id
end

--@description: 在父节点中替换子节点引用
--@param self: BinarySearchTree - BinarySearchTree实例
--@param parent_id: number - 父节点ID
--@param old_child: number - 旧子节点ID
--@param new_child: number - 新子节点ID
--@return: nil
--@local: true
function BinarySearchTree:replaceInParent(parent_id, old_child, new_child)
    if parent_id == -1 then
        self.root = new_child
    elseif self.nodes[parent_id].left == old_child then
        self.nodes[parent_id].left = new_child
    else
        self.nodes[parent_id].right = new_child
    end
end

--@description: 删除指定节点并维护BST性质
--@param self: BinarySearchTree - BinarySearchTree实例
--@param node_id: number - 要删除的节点ID
--@return: nil
--@local: true
function BinarySearchTree:deleteNode(node_id)
    local node = self.nodes[node_id]
    local parent_id = self:findParent(node_id)
    local left_child = node.left
    local right_child = node.right
    if left_child == -1 and right_child == -1 then
        self:replaceInParent(parent_id, node_id, -1)
        self.nodes[node_id] = nil
    elseif left_child == -1 then
        self:replaceInParent(parent_id, node_id, right_child)
        self.nodes[node_id] = nil
    elseif right_child == -1 then
        self:replaceInParent(parent_id, node_id, left_child)
        self.nodes[node_id] = nil
    else
        local successor_id = self:findMinNode(right_child)
        local successor_value = self.nodes[successor_id].value
        self:deleteNode(successor_id)
        self.nodes[node_id].value = successor_value
    end
end

--@description: 删除节点，自动维护二叉搜索树性质
--@param self: BinarySearchTree - BinarySearchTree实例
--@param value: number - 要删除的节点值
--@return: nil
--@raise: error - 当节点不存在时抛出异常
function BinarySearchTree:delete(value)
    local node_id = self:findNodeByValue(value)
    if node_id == -1 then
        self:raiseError(string.format("Node with value %s not found", tostring(value)))
    end
    local before = self.session:getLastStateId()
    local line = session.getCallerLine(3)
    self:deleteNode(node_id)
    local after = self.session:addState(self:buildData())
    self.session:addStep("delete", before, after, { value = value }, nil, nil, line)
end

M.BinarySearchTree = BinarySearchTree

--[[
Heap堆数据结构（使用完全二叉树表示）

Class:
    Heap
Fields:
    heap_type: string - 堆类型，"min"或"max"
    items: table - 堆元素列表
]]
local Heap = setmetatable({}, { __index = session.BaseStructure })
Heap.__index = Heap

--@description: 创建堆实例
--@param label: string|nil - 堆的标签，默认为"heap"
--@param heap_type: string|nil - 堆类型，"min"表示最小堆，"max"表示最大堆，默认为"min"
--@param output: string|nil - 输出文件路径
--@return: Heap - 堆实例
--@raise: error - 当heap_type不是"min"或"max"时抛出异常
function Heap.new(label, heap_type, output)
    heap_type = heap_type or "min"
    if heap_type ~= "min" and heap_type ~= "max" then
        error(string.format("heap_type must be 'min' or 'max', got '%s'", heap_type))
    end
    local base = session.BaseStructure.new("binary_tree", label or "heap", output)
    local self = setmetatable(base, Heap)
    self.heap_type = heap_type
    self.items = {}
    return self
end

--@description: 构建堆的状态数据（以完全二叉树形式表示）
--@param self: Heap - Heap实例
--@return: table - 状态数据字典
function Heap:buildData()
    if #self.items == 0 then
        return { root = -1, nodes = {} }
    end
    local nodes_list = {}
    for i, value in ipairs(self.items) do
        local idx = i - 1
        local left = (2 * idx + 1 < #self.items) and (2 * idx + 1) or -1
        local right = (2 * idx + 2 < #self.items) and (2 * idx + 2) or -1
        nodes_list[#nodes_list + 1] = {
            id = idx,
            value = value,
            left = left,
            right = right
        }
    end
    return { root = 0, nodes = nodes_list }
end

--@description: 比较两个值，根据堆类型决定比较方式
--@param self: Heap - Heap实例
--@param a: number - 第一个值
--@param b: number - 第二个值
--@return: boolean - 如果a应该在b之前（更接近堆顶）则返回true
--@local: true
function Heap:compare(a, b)
    if self.heap_type == "min" then
        return a < b
    else
        return a > b
    end
end

--@description: 向上调整堆
--@param self: Heap - Heap实例
--@param index: number - 开始调整的索引（1-based）
--@return: nil
--@local: true
function Heap:siftUp(index)
    while index > 1 do
        local parent = math.floor(index / 2)
        if self:compare(self.items[index], self.items[parent]) then
            self.items[index], self.items[parent] = self.items[parent], self.items[index]
            index = parent
        else
            break
        end
    end
end

--@description: 向下调整堆
--@param self: Heap - Heap实例
--@param index: number - 开始调整的索引（1-based）
--@return: nil
--@local: true
function Heap:siftDown(index)
    local size = #self.items
    while true do
        local target = index
        local left = 2 * index
        local right = 2 * index + 1
        if left <= size and self:compare(self.items[left], self.items[target]) then
            target = left
        end
        if right <= size and self:compare(self.items[right], self.items[target]) then
            target = right
        end
        if target == index then
            break
        end
        self.items[index], self.items[target] = self.items[target], self.items[index]
        index = target
    end
end

--@description: 插入元素，自动维护堆性质
--@param self: Heap - Heap实例
--@param value: number - 要插入的值
--@return: nil
function Heap:insert(value)
    local before = self.session:getLastStateId()
    local line = session.getCallerLine(3)
    self.items[#self.items + 1] = value
    self:siftUp(#self.items)
    local after = self.session:addState(self:buildData())
    self.session:addStep("insert", before, after, { value = value }, nil, nil, line)
end

--@description: 提取堆顶元素，自动维护堆性质
--@param self: Heap - Heap实例
--@return: nil
--@raise: error - 当堆为空时抛出异常
function Heap:extract()
    if #self.items == 0 then
        self:raiseError("Cannot extract from empty heap")
    end
    local before = self.session:getLastStateId()
    local line = session.getCallerLine(3)
    local extracted = self.items[1]
    if #self.items == 1 then
        table.remove(self.items)
    else
        self.items[1] = table.remove(self.items)
        self:siftDown(1)
    end
    local after = self.session:addState(self:buildData())
    self.session:addStep("extract", before, after, {}, extracted, nil, line)
end

--@description: 清空堆
--@param self: Heap - Heap实例
--@return: nil
function Heap:clear()
    local before = self.session:getLastStateId()
    local line = session.getCallerLine(3)
    self.items = {}
    local after = self.session:addState(self:buildData())
    self.session:addStep("clear", before, after, {}, nil, nil, line)
end

M.Heap = Heap

return M
