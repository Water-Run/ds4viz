# `ds4viz`  

`ds4viz`是一个`Python`库, 提供了`Python`语言的`ds4viz`支持.  
`ds4viz`是一个可视化数据结构工具. 参见: [GitHub](https://github.com/Water-Run/ds4viz)  

## 安装  

`ds4viz`发布在[PyPi](https://pypi.org/project/luainstaller/)上:  

```bash
pip install ds4viz
```

## 快速上手  

以下脚本提供了一个快速使用示例, 演示如何使用`ds4viz`生成`trace.toml`, 并在发生异常时仍然产出包含`[error]`的`.toml`IR文件:  

```python
import ds4viz as dv  # pip install ds4viz

def main() -> None:
    # 使用上下文: 确保无论成功/失败, 都会生成 trace.toml (成功 => [result], 异常 => [error])
    with dv.graph_undirected() as graph:
        # 添加节点
        graph.add_node(0, "A")
        graph.add_node(1, "B")
        graph.add_node(2, "C")

        # 添加无向边(内部会规范化为 from < to)
        graph.add_edge(0, 1)
        graph.add_edge(1, 2)

        # 也可以在这里继续调用更多 API, 每次操作都会记录到 trace 中

    print("已生成 trace.toml")

if __name__ == "__main__":
    main()
```

## API参考  
