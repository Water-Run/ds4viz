package = "ds4viz"
version = "0.1.0-1"

source = {
    url = "git://github.com/Water-Run/ds4viz.git",
    tag = "v0.1.0"
}

description = {
    summary = "ds4viz的Lua支持: 可扩展的数据结构可视化教学平台 Lua 库",
    detailed = [[
        ds4viz 是一个可扩展的数据结构可视化教学平台。
        本库提供 Lua 语言支持，可以追踪数据结构的操作过程，
        并生成标准化的 TOML 中间表示文件，用于后续可视化渲染。
        
        支持的数据结构包括：
        - 线性结构：栈、队列、单链表、双向链表
        - 树结构：二叉树、二叉搜索树、堆
        - 图结构：无向图、有向图、带权图
    ]],
    homepage = "https://github.com/Water-Run/ds4viz",
    license = "MIT",
    maintainer = "WaterRun <linzhuangrun49@gmail.com>"
}

dependencies = {
    "lua >= 5.1"
}

build = {
    type = "builtin",
    modules = {
        ["ds4viz"] = "ds4viz/init.lua",
        ["ds4viz.session"] = "ds4viz/session.lua",
        ["ds4viz.trace"] = "ds4viz/trace.lua",
        ["ds4viz.structures.linear"] = "ds4viz/structures/linear.lua",
        ["ds4viz.structures.tree"] = "ds4viz/structures/tree.lua",
        ["ds4viz.structures.graph"] = "ds4viz/structures/graph.lua"
    }
}