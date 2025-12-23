r"""
TOML 文件写入器，将 Trace 对象序列化为 TOML 格式

:file: ds4viz/writer.py
:author: WaterRun
:time: 2025-12-23
"""

from typing import Any

from ds4viz.trace import Trace, State, Step


class TomlWriter:
    r"""
    TOML 格式写入器
    """

    def __init__(self, trace: Trace) -> None:
        r"""
        初始化写入器

        :param trace: 要序列化的 Trace 对象
        :return None: 无返回值
        """
        self._trace: Trace = trace

    def write(self, path: str) -> None:
        r"""
        将 Trace 写入指定路径的 TOML 文件

        :param path: 输出文件路径
        :return None: 无返回值
        """
        content: str = self._serialize()
        with open(path, "w", encoding="utf-8") as f:
            f.write(content)

    def _serialize(self) -> str:
        r"""
        将 Trace 序列化为 TOML 字符串

        :return str: TOML 格式的字符串
        """
        lines: list[str] = []
        lines.extend(self._serialize_meta())
        lines.append("")
        lines.extend(self._serialize_package())
        if not self._trace.remarks.is_empty():
            lines.append("")
            lines.extend(self._serialize_remarks())
        lines.append("")
        lines.extend(self._serialize_object())
        for state in self._trace.states:
            lines.append("")
            lines.extend(self._serialize_state(state))
        for step in self._trace.steps:
            lines.append("")
            lines.extend(self._serialize_step(step))
        if self._trace.result is not None:
            lines.append("")
            lines.extend(self._serialize_result())
        if self._trace.error is not None:
            lines.append("")
            lines.extend(self._serialize_error())
        return "\n".join(lines)

    def _serialize_meta(self) -> list[str]:
        r"""
        序列化 meta 部分

        :return list[str]: TOML 行列表
        """
        return [
            "[meta]",
            f'created_at = "{self._trace.meta.created_at}"',
            f'lang = "{self._trace.meta.lang}"',
            f'lang_version = "{self._trace.meta.lang_version}"',
        ]

    def _serialize_package(self) -> list[str]:
        r"""
        序列化 package 部分

        :return list[str]: TOML 行列表
        """
        return [
            "[package]",
            f'name = "{self._trace.package.name}"',
            f'lang = "{self._trace.package.lang}"',
            f'version = "{self._trace.package.version}"',
        ]

    def _serialize_remarks(self) -> list[str]:
        r"""
        序列化 remarks 部分

        :return list[str]: TOML 行列表
        """
        lines: list[str] = ["[remarks]"]
        if self._trace.remarks.title:
            lines.append(f'title = "{self._escape_string(self._trace.remarks.title)}"')
        if self._trace.remarks.author:
            lines.append(f'author = "{self._escape_string(self._trace.remarks.author)}"')
        if self._trace.remarks.comment:
            lines.append(f'comment = "{self._escape_string(self._trace.remarks.comment)}"')
        return lines

    def _serialize_object(self) -> list[str]:
        r"""
        序列化 object 部分

        :return list[str]: TOML 行列表
        """
        lines: list[str] = [
            "[object]",
            f'kind = "{self._trace.object.kind}"',
        ]
        if self._trace.object.label:
            lines.append(f'label = "{self._escape_string(self._trace.object.label)}"')
        return lines

    def _serialize_state(self, state: State) -> list[str]:
        r"""
        序列化单个 state

        :param state: 状态对象
        :return list[str]: TOML 行列表
        """
        lines: list[str] = [
            "[[states]]",
            f"id = {state.id}",
            "",
            "[states.data]",
        ]
        lines.extend(self._serialize_state_data(state.data))
        return lines

    def _serialize_state_data(self, data: dict[str, Any]) -> list[str]:
        r"""
        序列化 state.data 部分

        :param data: 状态数据字典
        :return list[str]: TOML 行列表
        """
        lines: list[str] = []
        for key, value in data.items():
            if key in ("nodes", "edges"):
                lines.append(f"{key} = {self._serialize_inline_table_array(value)}")
            elif key == "items":
                lines.append(f"{key} = {self._serialize_simple_array(value)}")
            else:
                lines.append(f"{key} = {self._serialize_value(value)}")
        return lines

    def _serialize_inline_table_array(self, arr: list[dict[str, Any]]) -> str:
        r"""
        序列化 inline table 数组

        :param arr: 字典数组
        :return str: TOML 格式的 inline table 数组字符串
        """
        if not arr:
            return "[]"
        items: list[str] = []
        for item in arr:
            pairs: list[str] = [f"{k} = {self._serialize_value(v)}" for k, v in item.items()]
            items.append("{ " + ", ".join(pairs) + " }")
        return "[\n  " + ",\n  ".join(items) + "\n]"

    def _serialize_simple_array(self, arr: list[Any]) -> str:
        r"""
        序列化简单数组

        :param arr: 值数组
        :return str: TOML 格式的数组字符串
        """
        if not arr:
            return "[]"
        items: list[str] = [self._serialize_value(v) for v in arr]
        return "[" + ", ".join(items) + "]"

    def _serialize_step(self, step: Step) -> list[str]:
        r"""
        序列化单个 step

        :param step: 步骤对象
        :return list[str]: TOML 行列表
        """
        lines: list[str] = [
            "[[steps]]",
            f"id = {step.id}",
            f'op = "{step.op}"',
            f"before = {step.before}",
        ]
        if step.after is not None:
            lines.append(f"after = {step.after}")
        if step.ret is not None:
            lines.append(f"ret = {self._serialize_value(step.ret)}")
        if step.note:
            lines.append(f'note = "{self._escape_string(step.note)}"')

        lines.append("")
        lines.append("[steps.args]")

        if step.args:
            for key, value in step.args.items():
                lines.append(f"{key} = {self._serialize_value(value)}")

        if step.code is not None:
            lines.append("")
            lines.append("[steps.code]")
            lines.append(f"line = {step.code.line}")
            if step.code.col is not None:
                lines.append(f"col = {step.code.col}")

        return lines

    def _serialize_result(self) -> list[str]:
        r"""
        序列化 result 部分

        :return list[str]: TOML 行列表
        """
        result = self._trace.result
        if result is None:
            return []
        lines: list[str] = [
            "[result]",
            f"final_state = {result.final_state}",
        ]
        if result.commit is not None:
            lines.append("")
            lines.append("[result.commit]")
            lines.append(f'op = "{result.commit.op}"')
            lines.append(f"line = {result.commit.line}")
        return lines

    def _serialize_error(self) -> list[str]:
        r"""
        序列化 error 部分

        :return list[str]: TOML 行列表
        """
        error = self._trace.error
        if error is None:
            return []
        lines: list[str] = [
            "[error]",
            f'type = "{error.type}"',
            f'message = "{self._escape_string(error.message)}"',
        ]
        if error.line is not None:
            lines.append(f"line = {error.line}")
        if error.step is not None:
            lines.append(f"step = {error.step}")
        if error.last_state is not None:
            lines.append(f"last_state = {error.last_state}")
        return lines

    def _serialize_value(self, value: Any) -> str:
        r"""
        序列化单个值

        :param value: 要序列化的值
        :return str: TOML 格式的值字符串
        """
        match value:
            case bool():
                return "true" if value else "false"
            case int():
                return str(value)
            case float():
                return str(value)
            case str():
                return f'"{self._escape_string(value)}"'
            case list():
                return self._serialize_simple_array(value)
            case dict():
                pairs = [f"{k} = {self._serialize_value(v)}" for k, v in value.items()]
                return "{ " + ", ".join(pairs) + " }"
            case _:
                return f'"{self._escape_string(str(value))}"'

    @staticmethod
    def _escape_string(s: str) -> str:
        r"""
        转义字符串中的特殊字符

        :param s: 原始字符串
        :return str: 转义后的字符串
        """
        return s.replace("\\", "\\\\").replace('"', '\\"').replace("\n", "\\n").replace("\r", "\\r")
    