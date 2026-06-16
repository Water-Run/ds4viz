--[[
ds4viz - 可扩展的数据结构可视化教学平台 Lua 库

提供数据结构的可视化追踪功能，生成标准化的 TOML 中间表示文件。

Author:
    WaterRun
File:
    ds4viz/init.lua
Date:
    2025-12-24
Updated:
    2025-12-24
]]

local session = require("ds4viz.session")
local linear = require("ds4viz.structures.linear")
local tree = require("ds4viz.structures.tree")
local graph = require("ds4viz.structures.graph")

local M = {}

--@description: 库版本
--@const: VERSION
M.VERSION = "0.1.0"

--@description: 作者信息
--@const: AUTHOR
M.AUTHOR = "WaterRun <linzhuangrun49@gmail.com>"

--@description: 配置全局参数
--@param options: table|nil - 配置选项表
--@return: nil
--@usage: ds4viz.config({ output_path = "my_trace.toml", title = "Demo" })
function M.config(options)
    session.config(options)
end

--@description: 使用上下文执行函数（模拟Python的with语句）
--@param structure: BaseStructure - 数据结构实例
--@param fn: function - 要执行的函数
--@return: boolean - 是否成功执行
--@return: any - 错误信息（如果有）
--@usage: ds4viz.withContext(ds4viz.stack(), function(s) s:push(10) end)
function M.withContext(structure, fn)
    return session.withContext(structure, fn)
end

--@description: 创建栈实例
--@param label: string|nil - 栈的标签，默认为"stack"
--@param output: string|nil - 输出文件路径
--@return: Stack - 栈实例
function M.stack(label, output)
    return linear.Stack.new(label, output)
end

--@description: 创建队列实例
--@param label: string|nil - 队列的标签，默认为"queue"
--@param output: string|nil - 输出文件路径
--@return: Queue - 队列实例
function M.queue(label, output)
    return linear.Queue.new(label, output)
end

--@description: 创建单链表实例
--@param label: string|nil - 单链表的标签，默认为"slist"
--@param output: string|nil - 输出文件路径
--@return: SingleLinkedList - 单链表实例
function M.singleLinkedList(label, output)
    return linear.SingleLinkedList.new(label, output)
end

--@description: 创建双向链表实例
--@param label: string|nil - 双向链表的标签，默认为"dlist"
--@param output: string|nil - 输出文件路径
--@return: DoubleLinkedList - 双向链表实例
function M.doubleLinkedList(label, output)
    return linear.DoubleLinkedList.new(label, output)
end

--@description: 创建二叉树实例
--@param label: string|nil - 二叉树的标签，默认为"binary_tree"
--@param output: string|nil - 输出文件路径
--@return: BinaryTree - 二叉树实例
function M.binaryTree(label, output)
    return tree.BinaryTree.new(label, output)
end

--@description: 创建二叉搜索树实例
--@param label: string|nil - 二叉搜索树的标签，默认为"bst"
--@param output: string|nil - 输出文件路径
--@return: BinarySearchTree - 二叉搜索树实例
function M.binarySearchTree(label, output)
    return tree.BinarySearchTree.new(label, output)
end

--@description: 创建堆实例
--@param label: string|nil - 堆的标签，默认为"heap"
--@param heap_type: string|nil - 堆类型，"min"表示最小堆，"max"表示最大堆，默认为"min"
--@param output: string|nil - 输出文件路径
--@return: Heap - 堆实例
--@raise: error - 当heap_type不是"min"或"max"时抛出异常
function M.heap(label, heap_type, output)
    return tree.Heap.new(label, heap_type, output)
end

--@description: 创建无向图实例
--@param label: string|nil - 无向图的标签，默认为"graph"
--@param output: string|nil - 输出文件路径
--@return: GraphUndirected - 无向图实例
function M.graphUndirected(label, output)
    return graph.GraphUndirected.new(label, output)
end

--@description: 创建有向图实例
--@param label: string|nil - 有向图的标签，默认为"graph"
--@param output: string|nil - 输出文件路径
--@return: GraphDirected - 有向图实例
function M.graphDirected(label, output)
    return graph.GraphDirected.new(label, output)
end

--@description: 创建带权图实例
--@param label: string|nil - 带权图的标签，默认为"graph"
--@param directed: boolean|nil - 是否为有向图，默认为false（无向图）
--@param output: string|nil - 输出文件路径
--@return: GraphWeighted - 带权图实例
function M.graphWeighted(label, directed, output)
    return graph.GraphWeighted.new(label, directed, output)
end

--@description: 导出数据结构类
M.Stack = linear.Stack
M.Queue = linear.Queue
M.SingleLinkedList = linear.SingleLinkedList
M.DoubleLinkedList = linear.DoubleLinkedList
M.BinaryTree = tree.BinaryTree
M.BinarySearchTree = tree.BinarySearchTree
M.Heap = tree.Heap
M.GraphUndirected = graph.GraphUndirected
M.GraphDirected = graph.GraphDirected
M.GraphWeighted = graph.GraphWeighted

return M
