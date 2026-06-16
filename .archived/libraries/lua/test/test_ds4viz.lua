--[[
ds4viz 集成测试模块

使用 busted 测试框架对 ds4viz Lua 库进行完整的端到端测试。
涵盖各种数据结构的基础使用、边缘情况和错误处理。

Author:
    WaterRun
File:
    test/test_ds4viz.lua
Date:
    2025-12-24
]]

local ds4viz = require("ds4viz")

-- 辅助函数：创建临时文件路径
local function tempFile()
    return os.tmpname() .. ".toml"
end

-- 辅助函数：读取文件内容
local function readFile(path)
    local file = io.open(path, "r")
    if not file then
        return nil
    end
    local content = file:read("*a")
    file:close()
    return content
end

-- 辅助函数：删除文件
local function removeFile(path)
    os.remove(path)
end

-- 辅助函数：检查文件是否存在
local function fileExists(path)
    local file = io.open(path, "r")
    if file then
        file:close()
        return true
    end
    return false
end

-- TOML 验证器
local TomlValidator = {}

function TomlValidator.validate(content)
    local errors = {}

    -- 检查必需的部分
    if not content:find("%[meta%]") then
        errors[#errors + 1] = "Missing [meta] section"
    end
    if not content:find("%[package%]") then
        errors[#errors + 1] = "Missing [package] section"
    end
    if not content:find("%[object%]") then
        errors[#errors + 1] = "Missing [object] section"
    end
    if not content:find("%[%[states%]%]") then
        errors[#errors + 1] = "Missing [[states]] section"
    end

    -- 检查 result 和 error 互斥
    local hasResult = content:find("%[result%]") ~= nil
    local hasError = content:find("%[error%]") ~= nil
    if hasResult and hasError then
        errors[#errors + 1] = "Document cannot have both [result] and [error]"
    end
    if not hasResult and not hasError then
        errors[#errors + 1] = "Document must have either [result] or [error]"
    end

    return #errors == 0, errors
end

function TomlValidator.hasSection(content, section)
    return content:find("%[" .. section .. "%]") ~= nil
end

function TomlValidator.hasArraySection(content, section)
    return content:find("%[%[" .. section .. "%]%]") ~= nil
end

function TomlValidator.getFieldValue(content, field)
    local pattern = field .. '%s*=%s*"([^"]*)"'
    return content:match(pattern)
end

function TomlValidator.getFieldNumber(content, field)
    local pattern = field .. "%s*=%s*([%d%-%.]+)"
    local value = content:match(pattern)
    return value and tonumber(value)
end

-- ============================================================================
-- Stack 测试
-- ============================================================================

describe("Stack", function()
    describe("基础操作", function()
        it("应该能够执行 push 和 pop 操作", function()
            local path = tempFile()
            local ok, err = ds4viz.withContext(ds4viz.stack("test_stack", path), function(s)
                s:push(10)
                s:push(20)
                s:push(30)
                s:pop()
            end)

            assert.is_true(ok)
            assert.is_true(fileExists(path))

            local content = readFile(path)
            local valid, errors = TomlValidator.validate(content)
            assert.is_true(valid, table.concat(errors, ", "))
            assert.is_true(content:find('kind = "stack"') ~= nil)

            removeFile(path)
        end)

        it("应该能够执行 clear 操作", function()
            local path = tempFile()
            local ok = ds4viz.withContext(ds4viz.stack(nil, path), function(s)
                s:push(1)
                s:push(2)
                s:push(3)
                s:clear()
            end)

            assert.is_true(ok)
            local content = readFile(path)
            assert.is_true(content:find("items = %[%]") ~= nil)

            removeFile(path)
        end)

        it("空栈 pop 应该产生错误", function()
            local path = tempFile()
            local ok, err = ds4viz.withContext(ds4viz.stack(nil, path), function(s)
                s:pop()
            end)

            assert.is_false(ok)
            local content = readFile(path)
            assert.is_true(content:find("%[error%]") ~= nil)
            assert.is_true(content:find('type = "runtime"') ~= nil)

            removeFile(path)
        end)
    end)

    describe("各种值类型", function()
        it("应该支持整数", function()
            local path = tempFile()
            ds4viz.withContext(ds4viz.stack(nil, path), function(s)
                s:push(42)
                s:push(-10)
                s:push(0)
            end)

            local content = readFile(path)
            local valid = TomlValidator.validate(content)
            assert.is_true(valid)

            removeFile(path)
        end)

        it("应该支持浮点数", function()
            local path = tempFile()
            ds4viz.withContext(ds4viz.stack(nil, path), function(s)
                s:push(3.14)
                s:push(-2.718)
                s:push(0.0)
            end)

            local content = readFile(path)
            local valid = TomlValidator.validate(content)
            assert.is_true(valid)

            removeFile(path)
        end)

        it("应该支持字符串", function()
            local path = tempFile()
            ds4viz.withContext(ds4viz.stack(nil, path), function(s)
                s:push("hello")
                s:push("world")
            end)

            local content = readFile(path)
            local valid = TomlValidator.validate(content)
            assert.is_true(valid)

            removeFile(path)
        end)

        it("应该支持布尔值", function()
            local path = tempFile()
            ds4viz.withContext(ds4viz.stack(nil, path), function(s)
                s:push(true)
                s:push(false)
            end)

            local content = readFile(path)
            local valid = TomlValidator.validate(content)
            assert.is_true(valid)

            removeFile(path)
        end)

        it("应该支持混合类型", function()
            local path = tempFile()
            ds4viz.withContext(ds4viz.stack(nil, path), function(s)
                s:push(1)
                s:push(2.5)
                s:push("text")
                s:push(true)
            end)

            local content = readFile(path)
            local valid = TomlValidator.validate(content)
            assert.is_true(valid)

            removeFile(path)
        end)
    end)

    describe("边缘情况", function()
        it("空栈应该生成有效的 TOML", function()
            local path = tempFile()
            ds4viz.withContext(ds4viz.stack(nil, path), function(s)
                -- 不执行任何操作
            end)

            local content = readFile(path)
            local valid = TomlValidator.validate(content)
            assert.is_true(valid)

            removeFile(path)
        end)

        it("单元素栈", function()
            local path = tempFile()
            ds4viz.withContext(ds4viz.stack(nil, path), function(s)
                s:push(100)
                s:pop()
            end)

            local content = readFile(path)
            local valid = TomlValidator.validate(content)
            assert.is_true(valid)

            removeFile(path)
        end)

        it("clear 后再 push", function()
            local path = tempFile()
            ds4viz.withContext(ds4viz.stack(nil, path), function(s)
                s:push(1)
                s:push(2)
                s:clear()
                s:push(10)
                s:push(20)
            end)

            local content = readFile(path)
            local valid = TomlValidator.validate(content)
            assert.is_true(valid)

            removeFile(path)
        end)
    end)
end)

-- ============================================================================
-- Queue 测试
-- ============================================================================

describe("Queue", function()
    describe("基础操作", function()
        it("应该能够执行 enqueue 和 dequeue 操作", function()
            local path = tempFile()
            local ok = ds4viz.withContext(ds4viz.queue("test_queue", path), function(q)
                q:enqueue(10)
                q:enqueue(20)
                q:enqueue(30)
                q:dequeue()
            end)

            assert.is_true(ok)
            local content = readFile(path)
            local valid = TomlValidator.validate(content)
            assert.is_true(valid)
            assert.is_true(content:find('kind = "queue"') ~= nil)

            removeFile(path)
        end)

        it("应该能够执行 clear 操作", function()
            local path = tempFile()
            ds4viz.withContext(ds4viz.queue(nil, path), function(q)
                q:enqueue(1)
                q:enqueue(2)
                q:clear()
            end)

            local content = readFile(path)
            assert.is_true(content:find("items = %[%]") ~= nil)

            removeFile(path)
        end)

        it("空队列 dequeue 应该产生错误", function()
            local path = tempFile()
            local ok = ds4viz.withContext(ds4viz.queue(nil, path), function(q)
                q:dequeue()
            end)

            assert.is_false(ok)
            local content = readFile(path)
            assert.is_true(content:find("%[error%]") ~= nil)

            removeFile(path)
        end)
    end)

    describe("边缘情况", function()
        it("交替入队出队", function()
            local path = tempFile()
            ds4viz.withContext(ds4viz.queue(nil, path), function(q)
                q:enqueue(1)
                q:enqueue(2)
                q:dequeue()
                q:enqueue(3)
                q:dequeue()
                q:enqueue(4)
            end)

            local content = readFile(path)
            local valid = TomlValidator.validate(content)
            assert.is_true(valid)

            removeFile(path)
        end)

        it("出队直到空", function()
            local path = tempFile()
            ds4viz.withContext(ds4viz.queue(nil, path), function(q)
                for i = 1, 3 do
                    q:enqueue(i)
                end
                for _ = 1, 3 do
                    q:dequeue()
                end
            end)

            local content = readFile(path)
            local valid = TomlValidator.validate(content)
            assert.is_true(valid)

            removeFile(path)
        end)
    end)
end)

-- ============================================================================
-- SingleLinkedList 测试
-- ============================================================================

describe("SingleLinkedList", function()
    describe("基础操作", function()
        it("应该能够执行插入操作", function()
            local path = tempFile()
            ds4viz.withContext(ds4viz.singleLinkedList("test_slist", path), function(slist)
                local node_a = slist:insertHead(10)
                slist:insertTail(20)
                slist:insertAfter(node_a, 15)
            end)

            local content = readFile(path)
            local valid = TomlValidator.validate(content)
            assert.is_true(valid)
            assert.is_true(content:find('kind = "slist"') ~= nil)

            removeFile(path)
        end)

        it("应该能够执行删除操作", function()
            local path = tempFile()
            ds4viz.withContext(ds4viz.singleLinkedList(nil, path), function(slist)
                local node_a = slist:insertTail(1)
                local node_b = slist:insertTail(2)
                local node_c = slist:insertTail(3)
                slist:delete(node_b)
            end)

            local content = readFile(path)
            local valid = TomlValidator.validate(content)
            assert.is_true(valid)

            removeFile(path)
        end)

        it("应该能够执行反转操作", function()
            local path = tempFile()
            ds4viz.withContext(ds4viz.singleLinkedList(nil, path), function(slist)
                slist:insertTail(1)
                slist:insertTail(2)
                slist:insertTail(3)
                slist:reverse()
            end)

            local content = readFile(path)
            local valid = TomlValidator.validate(content)
            assert.is_true(valid)

            removeFile(path)
        end)
    end)

    describe("错误处理", function()
        it("删除不存在的节点应该产生错误", function()
            local path = tempFile()
            local ok = ds4viz.withContext(ds4viz.singleLinkedList(nil, path), function(slist)
                slist:insertHead(10)
                slist:delete(999)
            end)

            assert.is_false(ok)
            local content = readFile(path)
            assert.is_true(content:find("%[error%]") ~= nil)

            removeFile(path)
        end)

        it("空链表删除头节点应该产生错误", function()
            local path = tempFile()
            local ok = ds4viz.withContext(ds4viz.singleLinkedList(nil, path), function(slist)
                slist:deleteHead()
            end)

            assert.is_false(ok)
            local content = readFile(path)
            assert.is_true(content:find("%[error%]") ~= nil)

            removeFile(path)
        end)

        it("在不存在的节点后插入应该产生错误", function()
            local path = tempFile()
            local ok = ds4viz.withContext(ds4viz.singleLinkedList(nil, path), function(slist)
                slist:insertHead(10)
                slist:insertAfter(999, 20)
            end)

            assert.is_false(ok)
            local content = readFile(path)
            assert.is_true(content:find("%[error%]") ~= nil)

            removeFile(path)
        end)
    end)

    describe("边缘情况", function()
        it("反转空链表", function()
            local path = tempFile()
            ds4viz.withContext(ds4viz.singleLinkedList(nil, path), function(slist)
                slist:reverse()
            end)

            local content = readFile(path)
            local valid = TomlValidator.validate(content)
            assert.is_true(valid)

            removeFile(path)
        end)

        it("反转单节点链表", function()
            local path = tempFile()
            ds4viz.withContext(ds4viz.singleLinkedList(nil, path), function(slist)
                slist:insertHead(1)
                slist:reverse()
            end)

            local content = readFile(path)
            local valid = TomlValidator.validate(content)
            assert.is_true(valid)

            removeFile(path)
        end)
    end)
end)

-- ============================================================================
-- DoubleLinkedList 测试
-- ============================================================================

describe("DoubleLinkedList", function()
    describe("基础操作", function()
        it("应该能够执行插入操作", function()
            local path = tempFile()
            ds4viz.withContext(ds4viz.doubleLinkedList("test_dlist", path), function(dlist)
                local node_a = dlist:insertHead(10)
                local node_b = dlist:insertTail(30)
                dlist:insertBefore(node_b, 20)
                dlist:insertAfter(node_a, 15)
            end)

            local content = readFile(path)
            local valid = TomlValidator.validate(content)
            assert.is_true(valid)
            assert.is_true(content:find('kind = "dlist"') ~= nil)

            removeFile(path)
        end)

        it("应该能够执行删除操作", function()
            local path = tempFile()
            ds4viz.withContext(ds4viz.doubleLinkedList(nil, path), function(dlist)
                dlist:insertHead(1)
                dlist:insertTail(2)
                dlist:insertTail(3)
                dlist:deleteHead()
                dlist:deleteTail()
            end)

            local content = readFile(path)
            local valid = TomlValidator.validate(content)
            assert.is_true(valid)

            removeFile(path)
        end)

        it("应该能够执行反转操作", function()
            local path = tempFile()
            ds4viz.withContext(ds4viz.doubleLinkedList(nil, path), function(dlist)
                dlist:insertTail(1)
                dlist:insertTail(2)
                dlist:insertTail(3)
                dlist:reverse()
            end)

            local content = readFile(path)
            local valid = TomlValidator.validate(content)
            assert.is_true(valid)

            removeFile(path)
        end)
    end)

    describe("错误处理", function()
        it("空链表删除头节点应该产生错误", function()
            local path = tempFile()
            local ok = ds4viz.withContext(ds4viz.doubleLinkedList(nil, path), function(dlist)
                dlist:deleteHead()
            end)

            assert.is_false(ok)
            local content = readFile(path)
            assert.is_true(content:find("%[error%]") ~= nil)

            removeFile(path)
        end)

        it("空链表删除尾节点应该产生错误", function()
            local path = tempFile()
            local ok = ds4viz.withContext(ds4viz.doubleLinkedList(nil, path), function(dlist)
                dlist:deleteTail()
            end)

            assert.is_false(ok)
            local content = readFile(path)
            assert.is_true(content:find("%[error%]") ~= nil)

            removeFile(path)
        end)

        it("在不存在的节点前插入应该产生错误", function()
            local path = tempFile()
            local ok = ds4viz.withContext(ds4viz.doubleLinkedList(nil, path), function(dlist)
                dlist:insertHead(10)
                dlist:insertBefore(999, 20)
            end)

            assert.is_false(ok)
            local content = readFile(path)
            assert.is_true(content:find("%[error%]") ~= nil)

            removeFile(path)
        end)
    end)
end)

-- ============================================================================
-- BinaryTree 测试
-- ============================================================================

describe("BinaryTree", function()
    describe("基础操作", function()
        it("应该能够插入节点", function()
            local path = tempFile()
            ds4viz.withContext(ds4viz.binaryTree("test_btree", path), function(tree)
                local root = tree:insertRoot(10)
                local left = tree:insertLeft(root, 5)
                local right = tree:insertRight(root, 15)
                tree:insertLeft(left, 3)
                tree:insertRight(left, 7)
            end)

            local content = readFile(path)
            local valid = TomlValidator.validate(content)
            assert.is_true(valid)
            assert.is_true(content:find('kind = "binary_tree"') ~= nil)

            removeFile(path)
        end)

        it("应该能够删除子树", function()
            local path = tempFile()
            ds4viz.withContext(ds4viz.binaryTree(nil, path), function(tree)
                local root = tree:insertRoot(10)
                local left = tree:insertLeft(root, 5)
                tree:insertLeft(left, 3)
                tree:insertRight(left, 7)
                tree:delete(left)
            end)

            local content = readFile(path)
            local valid = TomlValidator.validate(content)
            assert.is_true(valid)

            removeFile(path)
        end)

        it("应该能够更新节点值", function()
            local path = tempFile()
            ds4viz.withContext(ds4viz.binaryTree(nil, path), function(tree)
                local root = tree:insertRoot(10)
                tree:updateValue(root, 100)
            end)

            local content = readFile(path)
            local valid = TomlValidator.validate(content)
            assert.is_true(valid)

            removeFile(path)
        end)
    end)

    describe("错误处理", function()
        it("重复插入根节点应该产生错误", function()
            local path = tempFile()
            local ok = ds4viz.withContext(ds4viz.binaryTree(nil, path), function(tree)
                tree:insertRoot(10)
                tree:insertRoot(20)
            end)

            assert.is_false(ok)
            local content = readFile(path)
            assert.is_true(content:find("%[error%]") ~= nil)

            removeFile(path)
        end)

        it("重复插入左子节点应该产生错误", function()
            local path = tempFile()
            local ok = ds4viz.withContext(ds4viz.binaryTree(nil, path), function(tree)
                local root = tree:insertRoot(10)
                tree:insertLeft(root, 5)
                tree:insertLeft(root, 6)
            end)

            assert.is_false(ok)
            local content = readFile(path)
            assert.is_true(content:find("%[error%]") ~= nil)

            removeFile(path)
        end)

        it("在不存在的父节点插入应该产生错误", function()
            local path = tempFile()
            local ok = ds4viz.withContext(ds4viz.binaryTree(nil, path), function(tree)
                tree:insertRoot(10)
                tree:insertLeft(999, 5)
            end)

            assert.is_false(ok)
            local content = readFile(path)
            assert.is_true(content:find("%[error%]") ~= nil)

            removeFile(path)
        end)

        it("删除不存在的节点应该产生错误", function()
            local path = tempFile()
            local ok = ds4viz.withContext(ds4viz.binaryTree(nil, path), function(tree)
                tree:insertRoot(10)
                tree:delete(999)
            end)

            assert.is_false(ok)
            local content = readFile(path)
            assert.is_true(content:find("%[error%]") ~= nil)

            removeFile(path)
        end)
    end)
end)

-- ============================================================================
-- BinarySearchTree 测试
-- ============================================================================

describe("BinarySearchTree", function()
    describe("基础操作", function()
        it("应该能够插入节点", function()
            local path = tempFile()
            ds4viz.withContext(ds4viz.binarySearchTree("test_bst", path), function(bst)
                bst:insert(10)
                bst:insert(5)
                bst:insert(15)
                bst:insert(3)
                bst:insert(7)
            end)

            local content = readFile(path)
            local valid = TomlValidator.validate(content)
            assert.is_true(valid)
            assert.is_true(content:find('kind = "bst"') ~= nil)

            removeFile(path)
        end)

        it("应该能够删除叶节点", function()
            local path = tempFile()
            ds4viz.withContext(ds4viz.binarySearchTree(nil, path), function(bst)
                bst:insert(10)
                bst:insert(5)
                bst:insert(15)
                bst:delete(5)
            end)

            local content = readFile(path)
            local valid = TomlValidator.validate(content)
            assert.is_true(valid)

            removeFile(path)
        end)

        it("应该能够删除有两个子节点的节点", function()
            local path = tempFile()
            ds4viz.withContext(ds4viz.binarySearchTree(nil, path), function(bst)
                bst:insert(10)
                bst:insert(5)
                bst:insert(15)
                bst:insert(3)
                bst:insert(7)
                bst:delete(5)
            end)

            local content = readFile(path)
            local valid = TomlValidator.validate(content)
            assert.is_true(valid)

            removeFile(path)
        end)

        it("应该能够删除根节点", function()
            local path = tempFile()
            ds4viz.withContext(ds4viz.binarySearchTree(nil, path), function(bst)
                bst:insert(10)
                bst:insert(5)
                bst:insert(15)
                bst:delete(10)
            end)

            local content = readFile(path)
            local valid = TomlValidator.validate(content)
            assert.is_true(valid)

            removeFile(path)
        end)
    end)

    describe("错误处理", function()
        it("删除不存在的节点应该产生错误", function()
            local path = tempFile()
            local ok = ds4viz.withContext(ds4viz.binarySearchTree(nil, path), function(bst)
                bst:insert(10)
                bst:delete(999)
            end)

            assert.is_false(ok)
            local content = readFile(path)
            assert.is_true(content:find("%[error%]") ~= nil)

            removeFile(path)
        end)
    end)

    describe("边缘情况", function()
        it("左偏树", function()
            local path = tempFile()
            ds4viz.withContext(ds4viz.binarySearchTree(nil, path), function(bst)
                for i = 5, 1, -1 do
                    bst:insert(i)
                end
            end)

            local content = readFile(path)
            local valid = TomlValidator.validate(content)
            assert.is_true(valid)

            removeFile(path)
        end)

        it("右偏树", function()
            local path = tempFile()
            ds4viz.withContext(ds4viz.binarySearchTree(nil, path), function(bst)
                for i = 1, 5 do
                    bst:insert(i)
                end
            end)

            local content = readFile(path)
            local valid = TomlValidator.validate(content)
            assert.is_true(valid)

            removeFile(path)
        end)
    end)
end)

-- ============================================================================
-- Heap 测试
-- ============================================================================

describe("Heap", function()
    describe("基础操作", function()
        it("最小堆基本操作", function()
            local path = tempFile()
            ds4viz.withContext(ds4viz.heap("test_heap", "min", path), function(h)
                h:insert(10)
                h:insert(5)
                h:insert(15)
                h:insert(3)
                h:extract()
            end)

            local content = readFile(path)
            local valid = TomlValidator.validate(content)
            assert.is_true(valid)
            assert.is_true(content:find('kind = "binary_tree"') ~= nil)

            removeFile(path)
        end)

        it("最大堆基本操作", function()
            local path = tempFile()
            ds4viz.withContext(ds4viz.heap(nil, "max", path), function(h)
                h:insert(10)
                h:insert(20)
                h:insert(5)
                h:extract()
            end)

            local content = readFile(path)
            local valid = TomlValidator.validate(content)
            assert.is_true(valid)

            removeFile(path)
        end)

        it("应该能够执行 clear 操作", function()
            local path = tempFile()
            ds4viz.withContext(ds4viz.heap(nil, "min", path), function(h)
                h:insert(1)
                h:insert(2)
                h:insert(3)
                h:clear()
            end)

            local content = readFile(path)
            local valid = TomlValidator.validate(content)
            assert.is_true(valid)

            removeFile(path)
        end)
    end)

    describe("错误处理", function()
        it("空堆 extract 应该产生错误", function()
            local path = tempFile()
            local ok = ds4viz.withContext(ds4viz.heap(nil, "min", path), function(h)
                h:extract()
            end)

            assert.is_false(ok)
            local content = readFile(path)
            assert.is_true(content:find("%[error%]") ~= nil)

            removeFile(path)
        end)

        it("无效的堆类型应该抛出错误", function()
            assert.has_error(function()
                ds4viz.heap(nil, "invalid")
            end)
        end)
    end)

    describe("边缘情况", function()
        it("多次 extract", function()
            local path = tempFile()
            ds4viz.withContext(ds4viz.heap(nil, "min", path), function(h)
                h:insert(10)
                h:insert(5)
                h:insert(15)
                h:insert(3)
                h:extract()
                h:extract()
            end)

            local content = readFile(path)
            local valid = TomlValidator.validate(content)
            assert.is_true(valid)

            removeFile(path)
        end)

        it("单元素堆", function()
            local path = tempFile()
            ds4viz.withContext(ds4viz.heap(nil, "min", path), function(h)
                h:insert(100)
                h:extract()
            end)

            local content = readFile(path)
            local valid = TomlValidator.validate(content)
            assert.is_true(valid)

            removeFile(path)
        end)
    end)
end)

-- ============================================================================
-- GraphUndirected 测试
-- ============================================================================

describe("GraphUndirected", function()
    describe("基础操作", function()
        it("应该能够添加节点和边", function()
            local path = tempFile()
            ds4viz.withContext(ds4viz.graphUndirected("test_graph", path), function(g)
                g:addNode(0, "A")
                g:addNode(1, "B")
                g:addNode(2, "C")
                g:addEdge(0, 1)
                g:addEdge(1, 2)
            end)

            local content = readFile(path)
            local valid = TomlValidator.validate(content)
            assert.is_true(valid)
            assert.is_true(content:find('kind = "graph_undirected"') ~= nil)

            removeFile(path)
        end)

        it("边应该被规范化（from < to）", function()
            local path = tempFile()
            ds4viz.withContext(ds4viz.graphUndirected(nil, path), function(g)
                g:addNode(0, "A")
                g:addNode(1, "B")
                g:addEdge(1, 0) -- 应该被规范化为 (0, 1)
            end)

            local content = readFile(path)
            assert.is_true(content:find("from = 0") ~= nil)
            assert.is_true(content:find("to = 1") ~= nil)

            removeFile(path)
        end)

        it("应该能够删除节点", function()
            local path = tempFile()
            ds4viz.withContext(ds4viz.graphUndirected(nil, path), function(g)
                g:addNode(0, "A")
                g:addNode(1, "B")
                g:addNode(2, "C")
                g:addEdge(0, 1)
                g:addEdge(1, 2)
                g:removeNode(1)
            end)

            local content = readFile(path)
            local valid = TomlValidator.validate(content)
            assert.is_true(valid)

            removeFile(path)
        end)

        it("应该能够删除边", function()
            local path = tempFile()
            ds4viz.withContext(ds4viz.graphUndirected(nil, path), function(g)
                g:addNode(0, "A")
                g:addNode(1, "B")
                g:addEdge(0, 1)
                g:removeEdge(0, 1)
            end)

            local content = readFile(path)
            local valid = TomlValidator.validate(content)
            assert.is_true(valid)

            removeFile(path)
        end)

        it("应该能够更新节点标签", function()
            local path = tempFile()
            ds4viz.withContext(ds4viz.graphUndirected(nil, path), function(g)
                g:addNode(0, "A")
                g:updateNodeLabel(0, "X")
            end)

            local content = readFile(path)
            local valid = TomlValidator.validate(content)
            assert.is_true(valid)

            removeFile(path)
        end)
    end)

    describe("错误处理", function()
        it("自环应该产生错误", function()
            local path = tempFile()
            local ok = ds4viz.withContext(ds4viz.graphUndirected(nil, path), function(g)
                g:addNode(0, "A")
                g:addEdge(0, 0)
            end)

            assert.is_false(ok)
            local content = readFile(path)
            assert.is_true(content:find("%[error%]") ~= nil)

            removeFile(path)
        end)

        it("重复边应该产生错误", function()
            local path = tempFile()
            local ok = ds4viz.withContext(ds4viz.graphUndirected(nil, path), function(g)
                g:addNode(0, "A")
                g:addNode(1, "B")
                g:addEdge(0, 1)
                g:addEdge(0, 1)
            end)

            assert.is_false(ok)
            local content = readFile(path)
            assert.is_true(content:find("%[error%]") ~= nil)

            removeFile(path)
        end)

        it("重复节点应该产生错误", function()
            local path = tempFile()
            local ok = ds4viz.withContext(ds4viz.graphUndirected(nil, path), function(g)
                g:addNode(0, "A")
                g:addNode(0, "B")
            end)

            assert.is_false(ok)
            local content = readFile(path)
            assert.is_true(content:find("%[error%]") ~= nil)

            removeFile(path)
        end)

        it("标签过长应该产生错误", function()
            local path = tempFile()
            local ok = ds4viz.withContext(ds4viz.graphUndirected(nil, path), function(g)
                g:addNode(0, string.rep("A", 33))
            end)

            assert.is_false(ok)
            local content = readFile(path)
            assert.is_true(content:find("%[error%]") ~= nil)

            removeFile(path)
        end)

        it("空标签应该产生错误", function()
            local path = tempFile()
            local ok = ds4viz.withContext(ds4viz.graphUndirected(nil, path), function(g)
                g:addNode(0, "")
            end)

            assert.is_false(ok)
            local content = readFile(path)
            assert.is_true(content:find("%[error%]") ~= nil)

            removeFile(path)
        end)
    end)
end)

-- ============================================================================
-- GraphDirected 测试
-- ============================================================================

describe("GraphDirected", function()
    describe("基础操作", function()
        it("应该能够添加节点和有向边", function()
            local path = tempFile()
            ds4viz.withContext(ds4viz.graphDirected("test_digraph", path), function(g)
                g:addNode(0, "A")
                g:addNode(1, "B")
                g:addEdge(0, 1)
                g:addEdge(1, 0)
            end)

            local content = readFile(path)
            local valid = TomlValidator.validate(content)
            assert.is_true(valid)
            assert.is_true(content:find('kind = "graph_directed"') ~= nil)

            removeFile(path)
        end)

        it("有向边不应该被规范化", function()
            local path = tempFile()
            ds4viz.withContext(ds4viz.graphDirected(nil, path), function(g)
                g:addNode(0, "A")
                g:addNode(1, "B")
                g:addEdge(1, 0)
            end)

            local content = readFile(path)
            assert.is_true(content:find("from = 1") ~= nil)
            assert.is_true(content:find("to = 0") ~= nil)

            removeFile(path)
        end)
    end)

    describe("错误处理", function()
        it("自环应该产生错误", function()
            local path = tempFile()
            local ok = ds4viz.withContext(ds4viz.graphDirected(nil, path), function(g)
                g:addNode(0, "A")
                g:addEdge(0, 0)
            end)

            assert.is_false(ok)
            local content = readFile(path)
            assert.is_true(content:find("%[error%]") ~= nil)

            removeFile(path)
        end)
    end)
end)

-- ============================================================================
-- GraphWeighted 测试
-- ============================================================================

describe("GraphWeighted", function()
    describe("基础操作", function()
        it("无向带权图", function()
            local path = tempFile()
            ds4viz.withContext(ds4viz.graphWeighted("test_wgraph", false, path), function(g)
                g:addNode(0, "A")
                g:addNode(1, "B")
                g:addEdge(0, 1, 3.5)
            end)

            local content = readFile(path)
            local valid = TomlValidator.validate(content)
            assert.is_true(valid)
            assert.is_true(content:find('kind = "graph_weighted"') ~= nil)
            assert.is_true(content:find("weight = 3.5") ~= nil)

            removeFile(path)
        end)

        it("有向带权图", function()
            local path = tempFile()
            ds4viz.withContext(ds4viz.graphWeighted(nil, true, path), function(g)
                g:addNode(0, "A")
                g:addNode(1, "B")
                g:addEdge(0, 1, 2.0)
                g:addEdge(1, 0, 3.0)
            end)

            local content = readFile(path)
            local valid = TomlValidator.validate(content)
            assert.is_true(valid)

            removeFile(path)
        end)

        it("应该能够更新权重", function()
            local path = tempFile()
            ds4viz.withContext(ds4viz.graphWeighted(nil, false, path), function(g)
                g:addNode(0, "A")
                g:addNode(1, "B")
                g:addEdge(0, 1, 1.0)
                g:updateWeight(0, 1, 5.0)
            end)

            local content = readFile(path)
            local valid = TomlValidator.validate(content)
            assert.is_true(valid)

            removeFile(path)
        end)
    end)

    describe("边缘情况", function()
        it("负权重", function()
            local path = tempFile()
            ds4viz.withContext(ds4viz.graphWeighted(nil, false, path), function(g)
                g:addNode(0, "A")
                g:addNode(1, "B")
                g:addEdge(0, 1, -5.5)
            end)

            local content = readFile(path)
            local valid = TomlValidator.validate(content)
            assert.is_true(valid)

            removeFile(path)
        end)

        it("零权重", function()
            local path = tempFile()
            ds4viz.withContext(ds4viz.graphWeighted(nil, false, path), function(g)
                g:addNode(0, "A")
                g:addNode(1, "B")
                g:addEdge(0, 1, 0.0)
            end)

            local content = readFile(path)
            local valid = TomlValidator.validate(content)
            assert.is_true(valid)

            removeFile(path)
        end)
    end)

    describe("错误处理", function()
        it("更新不存在的边权重应该产生错误", function()
            local path = tempFile()
            local ok = ds4viz.withContext(ds4viz.graphWeighted(nil, false, path), function(g)
                g:addNode(0, "A")
                g:addNode(1, "B")
                g:updateWeight(0, 1, 5.0)
            end)

            assert.is_false(ok)
            local content = readFile(path)
            assert.is_true(content:find("%[error%]") ~= nil)

            removeFile(path)
        end)
    end)
end)

-- ============================================================================
-- 全局配置测试
-- ============================================================================

describe("GlobalConfig", function()
    it("应该能够配置 remarks", function()
        local path = tempFile()
        ds4viz.config({
            output_path = path,
            title = "Test Title",
            author = "Test Author",
            comment = "Test Comment"
        })

        ds4viz.withContext(ds4viz.stack(), function(s)
            s:push(1)
        end)

        local content = readFile(path)
        assert.is_true(content:find("%[remarks%]") ~= nil)
        assert.is_true(content:find('title = "Test Title"') ~= nil)
        assert.is_true(content:find('author = "Test Author"') ~= nil)
        assert.is_true(content:find('comment = "Test Comment"') ~= nil)

        -- 重置配置
        ds4viz.config({
            output_path = "trace.toml",
            title = "",
            author = "",
            comment = ""
        })

        removeFile(path)
    end)

    it("只配置 title", function()
        local path = tempFile()
        ds4viz.config({
            output_path = path,
            title = "Only Title"
        })

        ds4viz.withContext(ds4viz.stack(), function(s)
            s:push(1)
        end)

        local content = readFile(path)
        assert.is_true(content:find('title = "Only Title"') ~= nil)

        ds4viz.config({
            output_path = "trace.toml",
            title = "",
            author = "",
            comment = ""
        })

        removeFile(path)
    end)
end)

-- ============================================================================
-- API 一致性测试
-- ============================================================================

describe("API 一致性", function()
    it("所有工厂函数应该存在", function()
        assert.is_function(ds4viz.stack)
        assert.is_function(ds4viz.queue)
        assert.is_function(ds4viz.singleLinkedList)
        assert.is_function(ds4viz.doubleLinkedList)
        assert.is_function(ds4viz.binaryTree)
        assert.is_function(ds4viz.binarySearchTree)
        assert.is_function(ds4viz.heap)
        assert.is_function(ds4viz.graphUndirected)
        assert.is_function(ds4viz.graphDirected)
        assert.is_function(ds4viz.graphWeighted)
        assert.is_function(ds4viz.config)
        assert.is_function(ds4viz.withContext)
    end)

    it("所有类应该被导出", function()
        assert.is_table(ds4viz.Stack)
        assert.is_table(ds4viz.Queue)
        assert.is_table(ds4viz.SingleLinkedList)
        assert.is_table(ds4viz.DoubleLinkedList)
        assert.is_table(ds4viz.BinaryTree)
        assert.is_table(ds4viz.BinarySearchTree)
        assert.is_table(ds4viz.Heap)
        assert.is_table(ds4viz.GraphUndirected)
        assert.is_table(ds4viz.GraphDirected)
        assert.is_table(ds4viz.GraphWeighted)
    end)

    it("版本信息应该存在", function()
        assert.is_string(ds4viz.VERSION)
        assert.is_true(ds4viz.VERSION:match("^%d+%.%d+%.%d+$") ~= nil)
    end)

    it("作者信息应该存在", function()
        assert.is_string(ds4viz.AUTHOR)
        assert.is_true(ds4viz.AUTHOR:find("WaterRun") ~= nil)
    end)
end)

-- ============================================================================
-- 边缘情况测试
-- ============================================================================

describe("边缘情况", function()
    it("空数据结构应该生成有效的 TOML", function()
        local path = tempFile()
        ds4viz.withContext(ds4viz.stack(nil, path), function(s)
            -- 不执行任何操作
        end)

        local content = readFile(path)
        local valid = TomlValidator.validate(content)
        assert.is_true(valid)

        -- 应该只有一个状态
        local state_count = select(2, content:gsub("%[%[states%]%]", ""))
        assert.equals(1, state_count)

        removeFile(path)
    end)

    it("特殊字符串应该被正确转义", function()
        local path = tempFile()
        ds4viz.withContext(ds4viz.stack(nil, path), function(s)
            s:push('hello"world')
            s:push("line1\\nline2")
        end)

        local content = readFile(path)
        local valid = TomlValidator.validate(content)
        assert.is_true(valid)

        removeFile(path)
    end)

    it("大数字应该正确处理", function()
        local path = tempFile()
        ds4viz.withContext(ds4viz.stack(nil, path), function(s)
            s:push(10 ^ 15)
            s:push(-10 ^ 15)
        end)

        local content = readFile(path)
        local valid = TomlValidator.validate(content)
        assert.is_true(valid)

        removeFile(path)
    end)

    it("大量操作应该正确处理", function()
        local path = tempFile()
        ds4viz.withContext(ds4viz.stack(nil, path), function(s)
            for i = 1, 50 do
                s:push(i)
            end
            for _ = 1, 25 do
                s:pop()
            end
        end)

        local content = readFile(path)
        local valid = TomlValidator.validate(content)
        assert.is_true(valid)

        -- 应该有 76 个状态 (1 初始 + 50 push + 25 pop)
        local state_count = select(2, content:gsub("%[%[states%]%]", ""))
        assert.equals(76, state_count)

        -- 应该有 75 个步骤
        local step_count = select(2, content:gsub("%[%[steps%]%]", ""))
        assert.equals(75, step_count)

        removeFile(path)
    end)

    it("最大长度标签", function()
        local path = tempFile()
        ds4viz.withContext(ds4viz.graphUndirected(nil, path), function(g)
            local max_label = string.rep("X", 32)
            g:addNode(0, max_label)
        end)

        local content = readFile(path)
        local valid = TomlValidator.validate(content)
        assert.is_true(valid)

        removeFile(path)
    end)

    it("Unicode 字符串", function()
        local path = tempFile()
        ds4viz.withContext(ds4viz.stack(nil, path), function(s)
            s:push("你好")
            s:push("世界")
        end)

        local content = readFile(path)
        local valid = TomlValidator.validate(content)
        assert.is_true(valid)

        removeFile(path)
    end)
end)

-- ============================================================================
-- 复杂场景测试
-- ============================================================================

describe("复杂场景", function()
    it("BST 复杂删除", function()
        local path = tempFile()
        ds4viz.withContext(ds4viz.binarySearchTree(nil, path), function(bst)
            local values = { 50, 30, 70, 20, 40, 60, 80, 35, 45 }
            for _, v in ipairs(values) do
                bst:insert(v)
            end
            bst:delete(30)
            bst:delete(50)
        end)

        local content = readFile(path)
        local valid = TomlValidator.validate(content)
        assert.is_true(valid)

        removeFile(path)
    end)

    it("图的复杂操作", function()
        local path = tempFile()
        ds4viz.withContext(ds4viz.graphWeighted(nil, true, path), function(g)
            for i = 0, 4 do
                g:addNode(i, string.char(65 + i))
            end

            g:addEdge(0, 1, 1.0)
            g:addEdge(0, 2, 2.0)
            g:addEdge(1, 2, 1.5)
            g:addEdge(2, 3, 2.5)
            g:addEdge(3, 4, 1.0)

            g:updateWeight(0, 1, 10.0)
            g:removeNode(2)
            g:updateNodeLabel(0, "Start")
        end)

        local content = readFile(path)
        local valid = TomlValidator.validate(content)
        assert.is_true(valid)

        removeFile(path)
    end)

    it("链表复杂操作", function()
        local path = tempFile()
        ds4viz.withContext(ds4viz.doubleLinkedList(nil, path), function(dlist)
            local nodes = {}
            for i = 0, 4 do
                nodes[#nodes + 1] = dlist:insertTail(i)
            end

            dlist:insertAfter(nodes[3], 100)
            dlist:insertBefore(nodes[3], 200)
            dlist:deleteHead()
            dlist:deleteTail()
            dlist:reverse()
        end)

        local content = readFile(path)
        local valid = TomlValidator.validate(content)
        assert.is_true(valid)

        removeFile(path)
    end)

    it("堆排序模拟", function()
        local path = tempFile()
        ds4viz.withContext(ds4viz.heap(nil, "min", path), function(h)
            local values = { 5, 2, 8, 1, 9, 3 }
            for _, v in ipairs(values) do
                h:insert(v)
            end
            for _ = 1, #values do
                h:extract()
            end
        end)

        local content = readFile(path)
        local valid = TomlValidator.validate(content)
        assert.is_true(valid)

        removeFile(path)
    end)
end)
