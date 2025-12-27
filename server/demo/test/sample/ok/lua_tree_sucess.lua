local ds4viz = require("ds4viz")

-- 写到 stdout，确保 Demo Server 能直接返回 TOML
ds4viz.config({
    output_path = "/dev/stdout",
    title = "Lua BinaryTree Complex Success",
    author = "WaterRun",
    comment = "覆盖 insertRoot/insertLeft/insertRight/updateValue/delete 多类操作"
})

ds4viz.withContext(ds4viz.binaryTree("demo_tree"), function(tree)
    -- 构造一个较完整的树结构
    local root  = tree:insertRoot("root")
    local left  = tree:insertLeft(root, 5)
    local right = tree:insertRight(root, 15)

    -- 左子树扩展（混合 value 类型：number/boolean/string）
    local ll    = tree:insertLeft(left, true)
    local lr    = tree:insertRight(left, "leaf")

    -- 右子树扩展
    local rl    = tree:insertLeft(right, 12)
    local rr    = tree:insertRight(right, 20)

    -- 更新节点值
    tree:updateValue(lr, "leaf_updated")
    tree:updateValue(rr, 99)

    -- 删除一些节点/子树，覆盖 delete 语义
    tree:delete(ll)   -- 删除一个叶子
    tree:delete(left) -- 删除整棵左子树（left, lr 已在子树内）
end)
