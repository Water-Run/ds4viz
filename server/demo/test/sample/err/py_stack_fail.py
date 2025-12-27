# ds4viz demo sample: failure (stack)
# Intentionally trigger an error (pop from empty stack).
# IMPORTANT: catch exceptions to avoid traceback polluting stdout (must output TOML only).

import os
import glob


def _find_trace_toml() -> str:
    if os.path.isfile("trace.toml"):
        return "trace.toml"

    candidates = []
    patterns = [
        "./*trace*.toml",
        "/tmp/ds4viz/*trace*.toml",
        "/tmp/*trace*.toml",
    ]
    for pat in patterns:
        candidates.extend(glob.glob(pat))

    candidates = [p for p in candidates if os.path.isfile(p)]
    if not candidates:
        return ""

    candidates.sort(key=lambda p: os.path.getmtime(p), reverse=True)
    return candidates[0]


def _print_trace_toml() -> None:
    path = _find_trace_toml()
    if not path:
        print('[error]\nmessage = "trace.toml not found"\n', end="")
        return

    with open(path, "r", encoding="utf-8", errors="replace") as f:
        print(f.read(), end="")


def main() -> None:
    # 清理同目录旧文件，降低误读旧 trace.toml 的概率
    try:
        if os.path.isfile("trace.toml"):
            os.remove("trace.toml")
    except Exception:
        pass

    from ds4viz import stack

    try:
        with stack() as s:
            # 空栈 pop：预期 ds4viz 产出 [error] 块
            s.pop()
    except Exception:
        # 避免异常堆栈输出污染 stdout（stdout 必须保持为 TOML）
        pass

    _print_trace_toml()


if __name__ == "__main__":
    main()
