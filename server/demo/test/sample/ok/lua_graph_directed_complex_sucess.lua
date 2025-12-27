local ds4viz = require("ds4viz")

local OUT = "/dev/stdout"

ds4viz.config({
    output_path = OUT,
    title = "Lua Directed Graph (complex)",
    author = "WaterRun",
    comment = "add/remove/update edges/nodes with cycles"
})

ds4viz.withContext(ds4viz.graphDirected("lua_digraph_complex", OUT), function(g)
    -- nodes
    g:addNode(0, "Start")
    g:addNode(1, "A")
    g:addNode(2, "B")
    g:addNode(3, "C")
    g:addNode(4, "D")
    g:addNode(5, "End")

    -- edges (a main path + extra branches)
    g:addEdge(0, 1)
    g:addEdge(1, 2)
    g:addEdge(2, 3)
    g:addEdge(3, 5)

    g:addEdge(1, 4)
    g:addEdge(4, 3)

    -- introduce a cycle then break it
    g:addEdge(3, 1)  -- cycle: 1->2->3->1
    g:removeEdge(3, 1) -- break cycle

    -- update label
    g:updateNodeLabel(2, "B_updated")

    -- remove a node and its incident edges
    g:removeNode(4)

    -- add an alternative direct jump
    g:addEdge(0, 2)
    g:addEdge(2, 5)
end)
