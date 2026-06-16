local ds4viz = require("ds4viz")

ds4viz.config({
    output_path = "trace.toml",
    title = "Lua Stack Fail",
    author = "WaterRun",
    comment = "故意触发异常：对空栈执行 pop"
})

-- 预期：withContext 捕获异常并生成包含 [error] 的 TOML
ds4viz.withContext(ds4viz.stack("fail_stack"), function(s)
    s:pop() -- 空栈弹出：应抛异常
end)
