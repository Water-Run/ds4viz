--[[
图数据结构实现

提供无向图、有向图、带权图的实现

Author:
    WaterRun
File:
    ds4viz/structures/graph.lua
Date:
    2025-12-24
Updated:
    2026-01-02
]]

local session = require("ds4viz.session")

local M = {}

--[[
GraphUndirected无向图数据结构

Class:
    GraphUndirected
Fields:
    nodes: table - 节点字典 {id: label}
    edges: table - 边集合 {"{from},{to}": true}
]]
local GraphUndirected = setmetatable({}, { __index = session.BaseStructure })
GraphUndirected.__index = GraphUndirected

--@description: 创建无向图实例
--@param label: string|nil - 无向图的标签，默认为"graph"
--@param output: string|nil - 输出文件路径
--@return: GraphUndirected - 无向图实例
function GraphUndirected.new(label, output)
    local base = session.BaseStructure.new("graph_undirected", label or "graph", output)
    local self = setmetatable(base, GraphUndirected)
    self.nodes = {}
    self.edges = {}
    return self
end

--@description: 构建无向图的状态数据
--@param self: GraphUndirected - GraphUndirected实例
--@return: table - 状态数据字典
function GraphUndirected:buildData()
    local nodes_list = {}
    local ids = {}
    for id in pairs(self.nodes) do
        ids[#ids + 1] = id
    end
    table.sort(ids)
    for _, nid in ipairs(ids) do
        nodes_list[#nodes_list + 1] = { id = nid, label = self.nodes[nid] }
    end
    local edges_list = {}
    local edge_keys = {}
    for key in pairs(self.edges) do
        edge_keys[#edge_keys + 1] = key
    end
    table.sort(edge_keys)
    for _, key in ipairs(edge_keys) do
        local from_str, to_str = key:match("(%d+),(%d+)")
        edges_list[#edges_list + 1] = {
            from = tonumber(from_str),
            to = tonumber(to_str)
        }
    end
    return { nodes = nodes_list, edges = edges_list }
end

--@description: 规范化无向边，确保from < to
--@param from_id: number - 起始节点id
--@param to_id: number - 终止节点id
--@return: number - 规范化后的from
--@return: number - 规范化后的to
--@local: true
local function normalizeUndirectedEdge(from_id, to_id)
    if from_id < to_id then
        return from_id, to_id
    else
        return to_id, from_id
    end
end

--@description: 生成边的键
--@param from_id: number - 起始节点id
--@param to_id: number - 终止节点id
--@return: string - 边的键
--@local: true
local function edgeKey(from_id, to_id)
    return string.format("%d,%d", from_id, to_id)
end

--@description: 添加节点
--@param self: GraphUndirected - GraphUndirected实例
--@param node_id: number - 节点id
--@param node_label: string - 节点标签，长度限制为1-32字符
--@return: nil
--@raise: error - 当节点已存在时抛出异常
function GraphUndirected:addNode(node_id, node_label)
    if self.nodes[node_id] then
        self:raiseError(string.format("Node already exists: %d", node_id))
    end
    if #node_label < 1 or #node_label > 32 then
        self:raiseError(string.format("Label length must be 1-32, got %d", #node_label))
    end
    local before = self.session:getLastStateId()
    local line = session.getCallerLine(3)
    self.nodes[node_id] = node_label
    local after = self.session:addState(self:buildData())
    self.session:addStep("add_node", before, after, { id = node_id, label = node_label }, nil, nil, line)
end

--@description: 添加无向边，内部自动规范化为from_id < to_id
--@param self: GraphUndirected - GraphUndirected实例
--@param from_id: number - 起始节点id
--@param to_id: number - 终止节点id
--@return: nil
--@raise: error - 当节点不存在、边已存在或形成自环时抛出异常
function GraphUndirected:addEdge(from_id, to_id)
    if not self.nodes[from_id] then
        self:raiseError(string.format("Node not found: %d", from_id))
    end
    if not self.nodes[to_id] then
        self:raiseError(string.format("Node not found: %d", to_id))
    end
    if from_id == to_id then
        self:raiseError(string.format("Self-loop not allowed: %d", from_id))
    end
    local norm_from, norm_to = normalizeUndirectedEdge(from_id, to_id)
    local key = edgeKey(norm_from, norm_to)
    if self.edges[key] then
        self:raiseError(string.format("Edge already exists: (%d, %d)", norm_from, norm_to))
    end
    local before = self.session:getLastStateId()
    local line = session.getCallerLine(3)
    self.edges[key] = true
    local after = self.session:addState(self:buildData())
    self.session:addStep("add_edge", before, after, { from = from_id, to = to_id }, nil, nil, line)
end

--@description: 删除节点及其相关的所有边
--@param self: GraphUndirected - GraphUndirected实例
--@param node_id: number - 要删除的节点id
--@return: nil
--@raise: error - 当节点不存在时抛出异常
function GraphUndirected:removeNode(node_id)
    if not self.nodes[node_id] then
        self:raiseError(string.format("Node not found: %d", node_id))
    end
    local before = self.session:getLastStateId()
    local line = session.getCallerLine(3)
    local edges_to_remove = {}
    for key in pairs(self.edges) do
        local from_str, to_str = key:match("(%d+),(%d+)")
        local from_val = tonumber(from_str)
        local to_val = tonumber(to_str)
        if from_val == node_id or to_val == node_id then
            edges_to_remove[#edges_to_remove + 1] = key
        end
    end
    for _, key in ipairs(edges_to_remove) do
        self.edges[key] = nil
    end
    self.nodes[node_id] = nil
    local after = self.session:addState(self:buildData())
    self.session:addStep("remove_node", before, after, { node_id = node_id }, nil, nil, line)
end

--@description: 删除边
--@param self: GraphUndirected - GraphUndirected实例
--@param from_id: number - 起始节点id
--@param to_id: number - 终止节点id
--@return: nil
--@raise: error - 当边不存在时抛出异常
function GraphUndirected:removeEdge(from_id, to_id)
    local norm_from, norm_to = normalizeUndirectedEdge(from_id, to_id)
    local key = edgeKey(norm_from, norm_to)
    if not self.edges[key] then
        self:raiseError(string.format("Edge not found: (%d, %d)", norm_from, norm_to))
    end
    local before = self.session:getLastStateId()
    local line = session.getCallerLine(3)
    self.edges[key] = nil
    local after = self.session:addState(self:buildData())
    self.session:addStep("remove_edge", before, after, { from = from_id, to = to_id }, nil, nil, line)
end

--@description: 更新节点标签
--@param self: GraphUndirected - GraphUndirected实例
--@param node_id: number - 节点id
--@param node_label: string - 新的节点标签，长度限制为1-32字符
--@return: nil
--@raise: error - 当节点不存在时抛出异常
function GraphUndirected:updateNodeLabel(node_id, node_label)
    if not self.nodes[node_id] then
        self:raiseError(string.format("Node not found: %d", node_id))
    end
    if #node_label < 1 or #node_label > 32 then
        self:raiseError(string.format("Label length must be 1-32, got %d", #node_label))
    end
    local before = self.session:getLastStateId()
    local line = session.getCallerLine(3)
    local old_label = self.nodes[node_id]
    self.nodes[node_id] = node_label
    local after = self.session:addState(self:buildData())
    self.session:addStep("update_node_label", before, after, { node_id = node_id, label = node_label }, old_label, nil,
        line)
end

M.GraphUndirected = GraphUndirected

--[[
GraphDirected有向图数据结构

Class:
    GraphDirected
Fields:
    nodes: table - 节点字典 {id: label}
    edges: table - 边集合 {"{from},{to}": true}
]]
local GraphDirected = setmetatable({}, { __index = session.BaseStructure })
GraphDirected.__index = GraphDirected

--@description: 创建有向图实例
--@param label: string|nil - 有向图的标签，默认为"graph"
--@param output: string|nil - 输出文件路径
--@return: GraphDirected - 有向图实例
function GraphDirected.new(label, output)
    local base = session.BaseStructure.new("graph_directed", label or "graph", output)
    local self = setmetatable(base, GraphDirected)
    self.nodes = {}
    self.edges = {}
    return self
end

--@description: 构建有向图的状态数据
--@param self: GraphDirected - GraphDirected实例
--@return: table - 状态数据字典
function GraphDirected:buildData()
    local nodes_list = {}
    local ids = {}
    for id in pairs(self.nodes) do
        ids[#ids + 1] = id
    end
    table.sort(ids)
    for _, nid in ipairs(ids) do
        nodes_list[#nodes_list + 1] = { id = nid, label = self.nodes[nid] }
    end
    local edges_list = {}
    local edge_keys = {}
    for key in pairs(self.edges) do
        edge_keys[#edge_keys + 1] = key
    end
    table.sort(edge_keys)
    for _, key in ipairs(edge_keys) do
        local from_str, to_str = key:match("(%d+),(%d+)")
        edges_list[#edges_list + 1] = {
            from = tonumber(from_str),
            to = tonumber(to_str)
        }
    end
    return { nodes = nodes_list, edges = edges_list }
end

--@description: 添加节点
--@param self: GraphDirected - GraphDirected实例
--@param node_id: number - 节点id
--@param node_label: string - 节点标签，长度限制为1-32字符
--@return: nil
--@raise: error - 当节点已存在时抛出异常
function GraphDirected:addNode(node_id, node_label)
    if self.nodes[node_id] then
        self:raiseError(string.format("Node already exists: %d", node_id))
    end
    if #node_label < 1 or #node_label > 32 then
        self:raiseError(string.format("Label length must be 1-32, got %d", #node_label))
    end
    local before = self.session:getLastStateId()
    local line = session.getCallerLine(3)
    self.nodes[node_id] = node_label
    local after = self.session:addState(self:buildData())
    self.session:addStep("add_node", before, after, { id = node_id, label = node_label }, nil, nil, line)
end

--@description: 添加有向边
--@param self: GraphDirected - GraphDirected实例
--@param from_id: number - 起始节点id
--@param to_id: number - 终止节点id
--@return: nil
--@raise: error - 当节点不存在、边已存在或形成自环时抛出异常
function GraphDirected:addEdge(from_id, to_id)
    if not self.nodes[from_id] then
        self:raiseError(string.format("Node not found: %d", from_id))
    end
    if not self.nodes[to_id] then
        self:raiseError(string.format("Node not found: %d", to_id))
    end
    if from_id == to_id then
        self:raiseError(string.format("Self-loop not allowed: %d", from_id))
    end
    local key = edgeKey(from_id, to_id)
    if self.edges[key] then
        self:raiseError(string.format("Edge already exists: (%d, %d)", from_id, to_id))
    end
    local before = self.session:getLastStateId()
    local line = session.getCallerLine(3)
    self.edges[key] = true
    local after = self.session:addState(self:buildData())
    self.session:addStep("add_edge", before, after, { from = from_id, to = to_id }, nil, nil, line)
end

--@description: 删除节点及其相关的所有边
--@param self: GraphDirected - GraphDirected实例
--@param node_id: number - 要删除的节点id
--@return: nil
--@raise: error - 当节点不存在时抛出异常
function GraphDirected:removeNode(node_id)
    if not self.nodes[node_id] then
        self:raiseError(string.format("Node not found: %d", node_id))
    end
    local before = self.session:getLastStateId()
    local line = session.getCallerLine(3)
    local edges_to_remove = {}
    for key in pairs(self.edges) do
        local from_str, to_str = key:match("(%d+),(%d+)")
        local from_val = tonumber(from_str)
        local to_val = tonumber(to_str)
        if from_val == node_id or to_val == node_id then
            edges_to_remove[#edges_to_remove + 1] = key
        end
    end
    for _, key in ipairs(edges_to_remove) do
        self.edges[key] = nil
    end
    self.nodes[node_id] = nil
    local after = self.session:addState(self:buildData())
    self.session:addStep("remove_node", before, after, { node_id = node_id }, nil, nil, line)
end

--@description: 删除边
--@param self: GraphDirected - GraphDirected实例
--@param from_id: number - 起始节点id
--@param to_id: number - 终止节点id
--@return: nil
--@raise: error - 当边不存在时抛出异常
function GraphDirected:removeEdge(from_id, to_id)
    local key = edgeKey(from_id, to_id)
    if not self.edges[key] then
        self:raiseError(string.format("Edge not found: (%d, %d)", from_id, to_id))
    end
    local before = self.session:getLastStateId()
    local line = session.getCallerLine(3)
    self.edges[key] = nil
    local after = self.session:addState(self:buildData())
    self.session:addStep("remove_edge", before, after, { from = from_id, to = to_id }, nil, nil, line)
end

--@description: 更新节点标签
--@param self: GraphDirected - GraphDirected实例
--@param node_id: number - 节点id
--@param node_label: string - 新的节点标签，长度限制为1-32字符
--@return: nil
--@raise: error - 当节点不存在时抛出异常
function GraphDirected:updateNodeLabel(node_id, node_label)
    if not self.nodes[node_id] then
        self:raiseError(string.format("Node not found: %d", node_id))
    end
    if #node_label < 1 or #node_label > 32 then
        self:raiseError(string.format("Label length must be 1-32, got %d", #node_label))
    end
    local before = self.session:getLastStateId()
    local line = session.getCallerLine(3)
    local old_label = self.nodes[node_id]
    self.nodes[node_id] = node_label
    local after = self.session:addState(self:buildData())
    self.session:addStep("update_node_label", before, after, { node_id = node_id, label = node_label }, old_label, nil,
        line)
end

M.GraphDirected = GraphDirected

--[[
GraphWeighted带权图数据结构

Class:
    GraphWeighted
Fields:
    directed: boolean - 是否为有向图
    nodes: table - 节点字典 {id: label}
    edges: table - 边字典 {"{from},{to}": weight}
]]
local GraphWeighted = setmetatable({}, { __index = session.BaseStructure })
GraphWeighted.__index = GraphWeighted

--@description: 创建带权图实例
--@param label: string|nil - 带权图的标签，默认为"graph"
--@param directed: boolean|nil - 是否为有向图，默认为false（无向图）
--@param output: string|nil - 输出文件路径
--@return: GraphWeighted - 带权图实例
function GraphWeighted.new(label, directed, output)
    local base = session.BaseStructure.new("graph_weighted", label or "graph", output)
    local self = setmetatable(base, GraphWeighted)
    self.directed = directed or false
    self.nodes = {}
    self.edges = {}
    return self
end

--@description: 构建带权图的状态数据
--@param self: GraphWeighted - GraphWeighted实例
--@return: table - 状态数据字典
function GraphWeighted:buildData()
    local nodes_list = {}
    local ids = {}
    for id in pairs(self.nodes) do
        ids[#ids + 1] = id
    end
    table.sort(ids)
    for _, nid in ipairs(ids) do
        nodes_list[#nodes_list + 1] = { id = nid, label = self.nodes[nid] }
    end
    local edges_list = {}
    local edge_keys = {}
    for key in pairs(self.edges) do
        edge_keys[#edge_keys + 1] = key
    end
    table.sort(edge_keys)
    for _, key in ipairs(edge_keys) do
        local from_str, to_str = key:match("(%d+),(%d+)")
        edges_list[#edges_list + 1] = {
            from = tonumber(from_str),
            to = tonumber(to_str),
            weight = self.edges[key]
        }
    end
    return { nodes = nodes_list, edges = edges_list }
end

--@description: 规范化边（无向图时确保from < to）
--@param self: GraphWeighted - GraphWeighted实例
--@param from_id: number - 起始节点id
--@param to_id: number - 终止节点id
--@return: number - 规范化后的from
--@return: number - 规范化后的to
--@local: true
function GraphWeighted:normalizeEdge(from_id, to_id)
    if self.directed then
        return from_id, to_id
    else
        return normalizeUndirectedEdge(from_id, to_id)
    end
end

--@description: 添加节点
--@param self: GraphWeighted - GraphWeighted实例
--@param node_id: number - 节点id
--@param node_label: string - 节点标签，长度限制为1-32字符
--@return: nil
--@raise: error - 当节点已存在时抛出异常
function GraphWeighted:addNode(node_id, node_label)
    if self.nodes[node_id] then
        self:raiseError(string.format("Node already exists: %d", node_id))
    end
    if #node_label < 1 or #node_label > 32 then
        self:raiseError(string.format("Label length must be 1-32, got %d", #node_label))
    end
    local before = self.session:getLastStateId()
    local line = session.getCallerLine(3)
    self.nodes[node_id] = node_label
    local after = self.session:addState(self:buildData())
    self.session:addStep("add_node", before, after, { id = node_id, label = node_label }, nil, nil, line)
end

--@description: 添加带权边
--@param self: GraphWeighted - GraphWeighted实例
--@param from_id: number - 起始节点id
--@param to_id: number - 终止节点id
--@param weight: number - 边的权重
--@return: nil
--@raise: error - 当节点不存在、边已存在或形成自环时抛出异常
function GraphWeighted:addEdge(from_id, to_id, weight)
    if not self.nodes[from_id] then
        self:raiseError(string.format("Node not found: %d", from_id))
    end
    if not self.nodes[to_id] then
        self:raiseError(string.format("Node not found: %d", to_id))
    end
    if from_id == to_id then
        self:raiseError(string.format("Self-loop not allowed: %d", from_id))
    end
    local norm_from, norm_to = self:normalizeEdge(from_id, to_id)
    local key = edgeKey(norm_from, norm_to)
    if self.edges[key] then
        self:raiseError(string.format("Edge already exists: (%d, %d)", norm_from, norm_to))
    end
    local before = self.session:getLastStateId()
    local line = session.getCallerLine(3)
    self.edges[key] = weight
    local after = self.session:addState(self:buildData())
    self.session:addStep("add_edge", before, after, { from = from_id, to = to_id, weight = weight }, nil, nil, line)
end

--@description: 删除节点及其相关的所有边
--@param self: GraphWeighted - GraphWeighted实例
--@param node_id: number - 要删除的节点id
--@return: nil
--@raise: error - 当节点不存在时抛出异常
function GraphWeighted:removeNode(node_id)
    if not self.nodes[node_id] then
        self:raiseError(string.format("Node not found: %d", node_id))
    end
    local before = self.session:getLastStateId()
    local line = session.getCallerLine(3)
    local edges_to_remove = {}
    for key in pairs(self.edges) do
        local from_str, to_str = key:match("(%d+),(%d+)")
        local from_val = tonumber(from_str)
        local to_val = tonumber(to_str)
        if from_val == node_id or to_val == node_id then
            edges_to_remove[#edges_to_remove + 1] = key
        end
    end
    for _, key in ipairs(edges_to_remove) do
        self.edges[key] = nil
    end
    self.nodes[node_id] = nil
    local after = self.session:addState(self:buildData())
    self.session:addStep("remove_node", before, after, { node_id = node_id }, nil, nil, line)
end

--@description: 删除边
--@param self: GraphWeighted - GraphWeighted实例
--@param from_id: number - 起始节点id
--@param to_id: number - 终止节点id
--@return: nil
--@raise: error - 当边不存在时抛出异常
function GraphWeighted:removeEdge(from_id, to_id)
    local norm_from, norm_to = self:normalizeEdge(from_id, to_id)
    local key = edgeKey(norm_from, norm_to)
    if not self.edges[key] then
        self:raiseError(string.format("Edge not found: (%d, %d)", norm_from, norm_to))
    end
    local before = self.session:getLastStateId()
    local line = session.getCallerLine(3)
    self.edges[key] = nil
    local after = self.session:addState(self:buildData())
    self.session:addStep("remove_edge", before, after, { from = from_id, to = to_id }, nil, nil, line)
end

--@description: 更新边的权重
--@param self: GraphWeighted - GraphWeighted实例
--@param from_id: number - 起始节点id
--@param to_id: number - 终止节点id
--@param weight: number - 新的权重值
--@return: nil
--@raise: error - 当边不存在时抛出异常
function GraphWeighted:updateWeight(from_id, to_id, weight)
    local norm_from, norm_to = self:normalizeEdge(from_id, to_id)
    local key = edgeKey(norm_from, norm_to)
    if not self.edges[key] then
        self:raiseError(string.format("Edge not found: (%d, %d)", norm_from, norm_to))
    end
    local before = self.session:getLastStateId()
    local line = session.getCallerLine(3)
    local old_weight = self.edges[key]
    self.edges[key] = weight
    local after = self.session:addState(self:buildData())
    self.session:addStep("update_weight", before, after, { from = from_id, to = to_id, weight = weight }, old_weight, nil,
        line)
end

--@description: 更新节点标签
--@param self: GraphWeighted - GraphWeighted实例
--@param node_id: number - 节点id
--@param node_label: string - 新的节点标签，长度限制为1-32字符
--@return: nil
--@raise: error - 当节点不存在时抛出异常
function GraphWeighted:updateNodeLabel(node_id, node_label)
    if not self.nodes[node_id] then
        self:raiseError(string.format("Node not found: %d", node_id))
    end
    if #node_label < 1 or #node_label > 32 then
        self:raiseError(string.format("Label length must be 1-32, got %d", #node_label))
    end
    local before = self.session:getLastStateId()
    local line = session.getCallerLine(3)
    local old_label = self.nodes[node_id]
    self.nodes[node_id] = node_label
    local after = self.session:addState(self:buildData())
    self.session:addStep("update_node_label", before, after, { node_id = node_id, label = node_label }, old_label, nil,
        line)
end

M.GraphWeighted = GraphWeighted

return M
