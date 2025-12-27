local ds4viz = require("ds4viz")

-- 写到 stdout，确保 Demo Server 能直接返回 TOML（其中应包含 [error]）
ds4viz.config({
    output_path = "/dev/stdout",
    title = "Lua Stack Fail",
    author = "WaterRun",
    comment = "故意触发异常：对空栈执行 pop"
})

-- 预期：withContext 捕获异常并生成包含 [error] 的 TOML
ds4viz.withContext(ds4viz.stack("fail_stack"), function(s)
    s:pop() -- 空栈弹出：应抛异常
end)
