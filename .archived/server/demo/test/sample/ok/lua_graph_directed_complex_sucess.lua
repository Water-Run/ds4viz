local ds4viz = require("ds4viz")

local OUT = "trace.toml"

ds4viz.config({
    output_path = OUT,
    title = "Lua Directed Graph (complex)",
    author = "WaterRun",
    comment = "add/remove/update edges/nodes with cycles"
})

ds4viz.withContext(ds4viz.graphDirected("lua_digraph_complex", OUT), function(g)
    -- 添加节点
    g:addNode(0, "Start")
    g:addNode(1, "A")
    g:addNode(2, "B")
    g:addNode(3, "C")
    g:addNode(4, "D")
    g:addNode(5, "End")

    -- 构建主路径: Start -> A -> B -> C -> End
    g:addEdge(0, 1)
    g:addEdge(1, 2)
    g:addEdge(2, 3)
    g:addEdge(3, 5)

    -- 添加分支路径: A -> D -> C
    g:addEdge(1, 4)
    g:addEdge(4, 3)

    -- 创建循环: C -> A (形成 1->2->3->1 的环)
    g:addEdge(3, 1)

    -- 更新节点标签
    g:updateNodeLabel(2, "B_updated")

    -- 删除循环边
    g:removeEdge(3, 1)

    -- 删除节点 D 及其相关边 (这会自动删除 1->4 和 4->3)
    g:removeNode(4)

    -- 添加新的直达路径: Start -> B
    g:addEdge(0, 2)

    -- 添加新的直达路径: B -> End (注意：检查是否已存在)
    -- 由于之前的路径是 B->C->End，B 和 End 之间没有直接边，所以可以添加
    g:addEdge(2, 5)
end)
