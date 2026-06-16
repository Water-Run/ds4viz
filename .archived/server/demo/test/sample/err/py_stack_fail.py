# py_stack_failure.py
import ds4viz as dv

dv.config(
    output_path="trace.toml",
    title="Python Stack Failure",
    author="WaterRun",
    comment="Intentionally pop from empty stack to trigger error"
)

try:
    with dv.stack() as s:
        s.pop()
except RuntimeError:
    pass
