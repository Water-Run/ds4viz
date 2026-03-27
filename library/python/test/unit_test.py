r"""
单元测试模块

:file: test/unit_test.py
:author: WaterRun
:time: 2026-03-27
"""

import sys
from pathlib import Path
from typing import Any
import inspect
import pytest

from ds4viz.exception import DS4VizError, StructureError, ValidationError, ErrorType
from ds4viz.trace import (
    Meta, Package, Remarks, Object, State, Step, CodeLocation,
    Result, Commit, Error, Trace
)
from ds4viz.writer import TomlWriter
from ds4viz.session import (
    GlobalConfig, config, get_config, get_python_version,
    get_caller_line, Session
)
from ds4viz.structures.linear import Stack, Queue, SingleLinkedList, DoubleLinkedList
from ds4viz.structures.tree import BinaryTree, BinarySearchTree
from ds4viz.structures.graph import GraphUndirected, GraphDirected, GraphWeighted
import ds4viz


class TestExceptionModule:
    r"""异常模块单元测试"""

    def test_error_type_enum_values(self) -> None:
        r"""测试 ErrorType 枚举值与 IR 定义 error.type 取值集合一致"""
        assert ErrorType.RUNTIME.value == "runtime"
        assert ErrorType.TIMEOUT.value == "timeout"
        assert ErrorType.VALIDATION.value == "validation"
        assert ErrorType.SANDBOX.value == "sandbox"
        assert ErrorType.UNKNOWN.value == "unknown"

    def test_error_type_enum_count(self) -> None:
        r"""测试 ErrorType 枚举数量与 IR 定义 error.type 五种取值一致"""
        assert len(ErrorType) == 5

    def test_ds4viz_error_basic(self) -> None:
        r"""测试 DS4VizError 基本功能, 默认类型为 RUNTIME"""
        error = DS4VizError("test message")
        assert error.message == "test message"
        assert error.error_type == ErrorType.RUNTIME
        assert error.line is None
        assert str(error) == "test message"

    def test_ds4viz_error_with_all_params(self) -> None:
        r"""测试 DS4VizError 带所有参数"""
        error = DS4VizError("error msg", ErrorType.VALIDATION, 42)
        assert error.message == "error msg"
        assert error.error_type == ErrorType.VALIDATION
        assert error.line == 42

    def test_ds4viz_error_is_runtime_error(self) -> None:
        r"""测试 DS4VizError 继承 RuntimeError"""
        error = DS4VizError("test")
        assert isinstance(error, RuntimeError)

    def test_structure_error(self) -> None:
        r"""测试 StructureError 带行号"""
        error = StructureError("structure error", 10)
        assert error.message == "structure error"
        assert error.error_type == ErrorType.RUNTIME
        assert error.line == 10
        assert isinstance(error, RuntimeError)

    def test_structure_error_default_line_none(self) -> None:
        r"""测试 StructureError 不传行号时默认为 None"""
        error = StructureError("msg")
        assert error.line is None
        assert error.error_type == ErrorType.RUNTIME

    def test_validation_error(self) -> None:
        r"""测试 ValidationError 带行号"""
        error = ValidationError("validation error", 20)
        assert error.message == "validation error"
        assert error.error_type == ErrorType.VALIDATION
        assert error.line == 20

    def test_validation_error_default_line_none(self) -> None:
        r"""测试 ValidationError 不传行号时默认为 None"""
        error = ValidationError("msg")
        assert error.line is None
        assert error.error_type == ErrorType.VALIDATION

    def test_error_inheritance(self) -> None:
        r"""测试异常继承关系"""
        assert issubclass(DS4VizError, RuntimeError)
        assert issubclass(StructureError, DS4VizError)
        assert issubclass(ValidationError, DS4VizError)

    def test_error_catchable_as_runtime_error(self) -> None:
        r"""测试 StructureError 可被 RuntimeError 捕获"""
        with pytest.raises(RuntimeError):
            raise StructureError("test", 5)


class TestTraceDataclasses:
    r"""Trace 数据类单元测试"""

    def test_meta_creation(self) -> None:
        r"""测试 Meta 创建"""
        meta = Meta(created_at="2026-03-23T00:00:00Z",
                    lang="python", lang_version="3.12.0")
        assert meta.created_at == "2026-03-23T00:00:00Z"
        assert meta.lang == "python"
        assert meta.lang_version == "3.12.0"

    def test_meta_defaults(self) -> None:
        r"""测试 Meta 默认值, lang 默认为 python"""
        meta = Meta(created_at="2026-03-23T00:00:00Z")
        assert meta.lang == "python"
        assert meta.lang_version == ""

    def test_package_defaults(self) -> None:
        r"""测试 Package 默认值与 IR 定义的包信息一致"""
        package = Package()
        assert package.name == "ds4viz"
        assert package.lang == "python"
        assert package.version == "0.1.0"

    def test_package_lang_matches_meta_lang(self) -> None:
        r"""测试 Package.lang 默认与 Meta.lang 一致, 符合 IR 约束"""
        meta = Meta(created_at="2026-03-23T00:00:00Z")
        package = Package()
        assert meta.lang == package.lang

    def test_remarks_is_empty(self) -> None:
        r"""测试 Remarks.is_empty 各种组合"""
        assert Remarks().is_empty() is True
        assert Remarks(title="T").is_empty() is False
        assert Remarks(author="A").is_empty() is False
        assert Remarks(comment="C").is_empty() is False

    def test_remarks_all_fields(self) -> None:
        r"""测试 Remarks 全部字段非空时 is_empty 为 False"""
        remarks = Remarks(title="T", author="A", comment="C")
        assert remarks.is_empty() is False
        assert remarks.title == "T"
        assert remarks.author == "A"
        assert remarks.comment == "C"

    def test_object_all_kinds(self) -> None:
        r"""测试 IR 定义中所有合法的 object.kind"""
        kinds: list[str] = [
            "stack", "queue", "slist", "dlist", "binary_tree",
            "bst", "graph_undirected", "graph_directed", "graph_weighted"
        ]
        for kind in kinds:
            obj = Object(kind=kind)
            assert obj.kind == kind

    def test_object_label_optional(self) -> None:
        r"""测试 Object.label 默认为空字符串"""
        obj = Object(kind="stack")
        assert obj.label == ""

    def test_state_with_complex_data(self) -> None:
        r"""测试 State 携带树结构数据"""
        data: dict[str, Any] = {
            "root": 0,
            "nodes": [
                {"id": 0, "alias": "root", "value": 10, "left": 1, "right": -1},
                {"id": 1, "value": 5, "left": -1, "right": -1}
            ]
        }
        state = State(id=0, data=data)
        assert state.id == 0
        assert len(state.data["nodes"]) == 2

    def test_step_with_phase_and_highlights(self) -> None:
        r"""测试 Step 包含 phase 和 highlights"""
        step = Step(
            id=0, op="observe", before=0, after=0, args={},
            phase="build",
            highlights=[{"kind": "node", "target": 0,
                         "style": "focus", "level": 1}]
        )
        assert step.phase == "build"
        assert step.highlights is not None
        assert len(step.highlights) == 1

    def test_step_defaults(self) -> None:
        r"""测试 Step 可选字段默认值均为 None"""
        step = Step(id=0, op="push", before=0)
        assert step.phase is None
        assert step.highlights is None
        assert step.after is None
        assert step.ret is None
        assert step.note is None
        assert step.code is None

    def test_code_location_defaults(self) -> None:
        r"""测试 CodeLocation 的 col 可选, 默认 None"""
        code = CodeLocation(line=10)
        assert code.line == 10
        assert code.col is None

    def test_code_location_with_col(self) -> None:
        r"""测试 CodeLocation 带 col"""
        code = CodeLocation(line=10, col=5)
        assert code.line == 10
        assert code.col == 5

    def test_commit_creation(self) -> None:
        r"""测试 Commit 创建"""
        commit = Commit(op="commit", line=100)
        assert commit.op == "commit"
        assert commit.line == 100

    def test_result_creation(self) -> None:
        r"""测试 Result 创建"""
        result = Result(final_state=5, commit=Commit(op="commit", line=50))
        assert result.final_state == 5
        assert result.commit is not None

    def test_result_without_commit(self) -> None:
        r"""测试 Result 不带 commit, commit 为 IR 可选项"""
        result = Result(final_state=3)
        assert result.final_state == 3
        assert result.commit is None

    def test_error_creation(self) -> None:
        r"""测试 Error 创建, 包含所有字段"""
        error = Error(type="runtime", message="test error",
                      line=30, step=2, last_state=1)
        assert error.type == "runtime"
        assert error.message == "test error"
        assert error.line == 30
        assert error.step == 2
        assert error.last_state == 1

    def test_error_optional_fields(self) -> None:
        r"""测试 Error 仅必填字段, line/step/last_state 均为 IR 可选"""
        error = Error(type="unknown", message="something failed")
        assert error.type == "unknown"
        assert error.line is None
        assert error.step is None
        assert error.last_state is None

    def test_trace_creation(self) -> None:
        r"""测试 Trace 成功场景创建"""
        trace = Trace(
            meta=Meta(created_at="2026-03-23T00:00:00Z"),
            package=Package(),
            remarks=Remarks(),
            object=Object(kind="stack"),
            states=[State(id=0, data={"items": [], "top": -1})],
            result=Result(final_state=0)
        )
        assert trace.object.kind == "stack"
        assert trace.error is None
        assert trace.result is not None

    def test_trace_with_error_no_result(self) -> None:
        r"""测试 Trace 失败场景: 有 error 无 result, 符合 IR 顶层结局互斥约束"""
        trace = Trace(
            meta=Meta(created_at="2026-03-23T00:00:00Z"),
            package=Package(),
            remarks=Remarks(),
            object=Object(kind="stack"),
            states=[State(id=0, data={"items": [], "top": -1})],
            error=Error(type="runtime", message="fail")
        )
        assert trace.result is None
        assert trace.error is not None
        assert trace.error.type == "runtime"


class TestGlobalConfig:
    r"""全局配置单元测试"""

    def test_default_config(self) -> None:
        r"""测试 GlobalConfig 默认值"""
        cfg = GlobalConfig()
        assert cfg.output_path == "trace.toml"
        assert cfg.title == ""
        assert cfg.author == ""
        assert cfg.comment == ""

    def test_config_function(self) -> None:
        r"""测试 config 函数设置全局配置"""
        config(output_path="custom.toml", title="Test Title",
               author="Author", comment="Comment")
        cfg = get_config()
        assert cfg.output_path == "custom.toml"
        assert cfg.title == "Test Title"
        assert cfg.author == "Author"
        assert cfg.comment == "Comment"
        config()

    def test_config_reset_to_defaults(self) -> None:
        r"""测试 config() 无参数时重置为默认值"""
        config(title="T", author="A")
        config()
        cfg = get_config()
        assert cfg.output_path == "trace.toml"
        assert cfg.title == ""
        assert cfg.author == ""

    def test_config_partial(self) -> None:
        r"""测试 config 仅设置部分字段, 其余保持默认"""
        config(title="Only Title")
        cfg = get_config()
        assert cfg.title == "Only Title"
        assert cfg.author == ""
        assert cfg.output_path == "trace.toml"
        config()

    def test_get_python_version(self) -> None:
        r"""测试获取 Python 版本格式为 MAJOR.MINOR.MICRO"""
        version: str = get_python_version()
        parts: list[str] = version.split(".")
        assert len(parts) == 3
        assert version == f"{sys.version_info.major}.{sys.version_info.minor}.{sys.version_info.micro}"

    def test_get_caller_line(self) -> None:
        r"""测试获取调用行号返回正整数"""
        line: int = get_caller_line(1)
        assert isinstance(line, int)
        assert line > 0


class TestSession:
    r"""Session 单元测试"""

    def test_session_add_state(self) -> None:
        r"""测试添加状态返回从 0 开始的 ID"""
        session = Session(kind="stack", label="test")
        sid: int = session.add_state({"items": [], "top": -1})
        assert sid == 0
        assert session.get_last_state_id() == 0

    def test_session_add_multiple_states_consecutive(self) -> None:
        r"""测试多次添加状态 ID 从 0 连续递增, 符合 IR states.id 约束"""
        session = Session(kind="stack", label="test")
        ids: list[int] = [
            session.add_state({"items": [], "top": -1}),
            session.add_state({"items": [1], "top": 0}),
            session.add_state({"items": [1, 2], "top": 1}),
        ]
        assert ids == [0, 1, 2]

    def test_session_add_step_with_phase(self) -> None:
        r"""测试添加带 phase 和 highlights 的步骤"""
        session = Session(kind="stack", label="test")
        session.add_state({"items": [], "top": -1})
        session.add_state({"items": [10], "top": 0})
        step_id: int = session.add_step(
            op="push", before=0, after=1, args={"value": 10},
            phase="build", highlights=[{"kind": "item", "target": 0, "style": "focus", "level": 3}]
        )
        assert step_id == 0
        last: Step | None = session.get_last_step()
        assert last is not None
        assert last.phase == "build"
        assert last.highlights is not None

    def test_session_step_ids_consecutive(self) -> None:
        r"""测试步骤 ID 从 0 连续递增, 符合 IR steps.id 约束"""
        session = Session(kind="stack", label="test")
        session.add_state({"items": [], "top": -1})
        session.add_state({"items": [1], "top": 0})
        session.add_state({"items": [1, 2], "top": 1})
        s0: int = session.add_step(
            op="push", before=0, after=1, args={"value": 1})
        s1: int = session.add_step(
            op="push", before=1, after=2, args={"value": 2})
        assert s0 == 0
        assert s1 == 1

    def test_session_get_last_step_empty(self) -> None:
        r"""测试空 session 获取最后步骤返回 None"""
        session = Session(kind="stack", label="test")
        assert session.get_last_step() is None

    def test_session_step_counter_property(self) -> None:
        r"""测试 step_counter 属性与实际步骤数一致"""
        session = Session(kind="stack", label="test")
        assert session.step_counter == 0
        session.add_state({"items": [], "top": -1})
        session.add_state({"items": [1], "top": 0})
        session.add_step(op="push", before=0, after=1, args={})
        assert session.step_counter == 1

    def test_session_failed_step_id_default(self) -> None:
        r"""测试 failed_step_id 初始为 None"""
        session = Session(kind="stack", label="test")
        assert session.failed_step_id is None

    def test_session_get_last_state_id_empty(self) -> None:
        r"""测试无状态时 get_last_state_id 返回 -1"""
        session = Session(kind="stack", label="test")
        assert session.get_last_state_id() == -1

    def test_session_build_trace_success(self) -> None:
        r"""测试构建成功的 Trace, 包含 result 不包含 error"""
        session = Session(kind="stack", label="test_stack")
        session.set_entry_line(1)
        session.set_exit_line(10)
        session.add_state({"items": [], "top": -1})
        trace: Trace = session.build_trace()
        assert trace.result is not None
        assert trace.error is None
        assert trace.result.final_state == 0

    def test_session_build_trace_error(self) -> None:
        r"""测试构建带错误的 Trace, 包含 error 不包含 result"""
        session = Session(kind="stack", label="test")
        session.add_state({"items": [], "top": -1})
        session.set_error(ErrorType.RUNTIME,
                          "Cannot pop from empty stack", line=10, step_id=0)
        trace: Trace = session.build_trace()
        assert trace.result is None
        assert trace.error is not None
        assert trace.error.type == "runtime"
        assert trace.error.last_state == 0

    def test_session_set_error_truncates_message(self) -> None:
        r"""测试 set_error 截断超长消息至 512 字符, 符合 IR error.message 长度约束"""
        session = Session(kind="stack", label="test")
        session.add_state({"items": [], "top": -1})
        long_msg: str = "x" * 1000
        session.set_error(ErrorType.RUNTIME, long_msg)
        trace: Trace = session.build_trace()
        assert trace.error is not None
        assert len(trace.error.message) == 512


class TestTomlWriter:
    r"""TomlWriter 单元测试"""

    def test_escape_string(self) -> None:
        r"""测试字符串转义: 双引号、反斜杠、换行符、回车符"""
        assert TomlWriter._escape_string('hello"world') == 'hello\\"world'
        assert TomlWriter._escape_string('hello\\world') == 'hello\\\\world'
        assert TomlWriter._escape_string('hello\nworld') == 'hello\\nworld'
        assert TomlWriter._escape_string('hello\rworld') == 'hello\\rworld'

    def test_escape_string_no_escape_needed(self) -> None:
        r"""测试无需转义的字符串保持不变"""
        assert TomlWriter._escape_string("hello world") == "hello world"
        assert TomlWriter._escape_string("") == ""

    def test_serialize_value_types(self) -> None:
        r"""测试序列化 IR 允许的各种 TOML 标量类型"""
        trace = Trace(
            meta=Meta(created_at="t"), package=Package(),
            remarks=Remarks(), object=Object(kind="stack")
        )
        w = TomlWriter(trace)
        assert w._serialize_value(True) == "true"
        assert w._serialize_value(False) == "false"
        assert w._serialize_value(42) == "42"
        assert w._serialize_value(0) == "0"
        assert w._serialize_value(-1) == "-1"
        assert w._serialize_value(3.14) == "3.14"
        assert w._serialize_value("hello") == '"hello"'
        assert w._serialize_value([1, 2]) == "[1, 2]"
        assert w._serialize_value([]) == "[]"

    def test_serialize_bool_before_int(self) -> None:
        r"""测试 bool 值序列化为 true/false 而非 1/0, match-case 顺序正确"""
        trace = Trace(
            meta=Meta(created_at="t"), package=Package(),
            remarks=Remarks(), object=Object(kind="stack")
        )
        w = TomlWriter(trace)
        assert w._serialize_value(True) == "true"
        assert w._serialize_value(False) == "false"
        assert w._serialize_value(1) == "1"
        assert w._serialize_value(0) == "0"

    def test_serialize_dict_as_inline_table(self) -> None:
        r"""测试字典序列化为 TOML inline table"""
        trace = Trace(
            meta=Meta(created_at="t"), package=Package(),
            remarks=Remarks(), object=Object(kind="stack")
        )
        w = TomlWriter(trace)
        result: str = w._serialize_value({"a": 1, "b": "x"})
        assert "a = 1" in result
        assert 'b = "x"' in result

    def test_serialize_inline_table_array_empty(self) -> None:
        r"""测试空 inline table 数组序列化为 []"""
        trace = Trace(
            meta=Meta(created_at="t"), package=Package(),
            remarks=Remarks(), object=Object(kind="stack")
        )
        w = TomlWriter(trace)
        assert w._serialize_inline_table_array([]) == "[]"

    def test_serialize_step_with_phase_and_highlights(self) -> None:
        r"""测试序列化带 phase 和 highlights 的 step"""
        trace = Trace(
            meta=Meta(created_at="t"), package=Package(),
            remarks=Remarks(), object=Object(kind="stack")
        )
        w = TomlWriter(trace)
        step = Step(
            id=0, op="push", before=0, after=1, args={"value": 10},
            phase="build",
            highlights=[{"kind": "item", "target": 0,
                         "style": "focus", "level": 3}]
        )
        lines: list[str] = w._serialize_step(step)
        content: str = "\n".join(lines)
        assert 'phase = "build"' in content
        assert "highlights" in content
        assert 'kind = "item"' in content

    def test_serialize_step_without_phase(self) -> None:
        r"""测试序列化无 phase 的 step, phase 字段不出现"""
        trace = Trace(
            meta=Meta(created_at="t"), package=Package(),
            remarks=Remarks(), object=Object(kind="stack")
        )
        w = TomlWriter(trace)
        step = Step(id=0, op="push", before=0, after=1, args={})
        lines: list[str] = w._serialize_step(step)
        content: str = "\n".join(lines)
        assert "phase" not in content

    def test_serialize_step_empty_args_header(self) -> None:
        r"""测试步骤 args 为空时仍输出 [steps.args] 表头, 符合 IR 必需约束"""
        trace = Trace(
            meta=Meta(created_at="t"), package=Package(),
            remarks=Remarks(), object=Object(kind="stack")
        )
        w = TomlWriter(trace)
        step = Step(id=0, op="pop", before=0, after=1, args={})
        lines: list[str] = w._serialize_step(step)
        content: str = "\n".join(lines)
        assert "[steps.args]" in content

    def test_serialize_empty_highlights(self) -> None:
        r"""测试空 highlights 列表序列化为 highlights = []"""
        trace = Trace(
            meta=Meta(created_at="t"), package=Package(),
            remarks=Remarks(), object=Object(kind="stack")
        )
        w = TomlWriter(trace)
        step = Step(id=0, op="observe", before=0,
                    after=0, args={}, highlights=[])
        lines: list[str] = w._serialize_step(step)
        content: str = "\n".join(lines)
        assert "highlights = []" in content

    def test_serialize_remarks_partial(self) -> None:
        r"""测试仅设置部分 remarks 字段时, 只输出非空字段"""
        trace = Trace(
            meta=Meta(created_at="t"), package=Package(),
            remarks=Remarks(title="Title Only"),
            object=Object(kind="stack")
        )
        w = TomlWriter(trace)
        lines: list[str] = w._serialize_remarks()
        content: str = "\n".join(lines)
        assert 'title = "Title Only"' in content
        assert "author" not in content
        assert "comment" not in content

    def test_serialize_remarks_empty_omitted(self) -> None:
        r"""测试 remarks 全部为空时不出现在序列化输出中"""
        trace = Trace(
            meta=Meta(created_at="2026-03-23T00:00:00Z"), package=Package(),
            remarks=Remarks(), object=Object(kind="stack"),
            states=[State(id=0, data={"items": [], "top": -1})],
            result=Result(final_state=0)
        )
        w = TomlWriter(trace)
        content: str = w._serialize()
        assert "[remarks]" not in content

    def test_serialize_error_section(self) -> None:
        r"""测试 error 部分序列化, 包含所有可选字段"""
        trace = Trace(
            meta=Meta(created_at="t"), package=Package(),
            remarks=Remarks(), object=Object(kind="stack"),
            states=[State(id=0, data={"items": [], "top": -1})],
            error=Error(type="runtime", message="fail",
                        line=10, step=0, last_state=0)
        )
        w = TomlWriter(trace)
        lines: list[str] = w._serialize_error()
        content: str = "\n".join(lines)
        assert '[error]' in content
        assert 'type = "runtime"' in content
        assert 'message = "fail"' in content
        assert "line = 10" in content
        assert "step = 0" in content
        assert "last_state = 0" in content

    def test_serialize_error_minimal(self) -> None:
        r"""测试 error 部分仅必填字段时, 可选字段不出现"""
        trace = Trace(
            meta=Meta(created_at="t"), package=Package(),
            remarks=Remarks(), object=Object(kind="stack"),
            states=[State(id=0, data={"items": [], "top": -1})],
            error=Error(type="unknown", message="oops")
        )
        w = TomlWriter(trace)
        lines: list[str] = w._serialize_error()
        content: str = "\n".join(lines)
        assert 'type = "unknown"' in content
        assert "line =" not in content
        assert "step =" not in content
        assert "last_state =" not in content

    def test_write_to_file(self, tmp_path: Path) -> None:
        r"""测试完整 Trace 写入文件, result 与 error 互斥"""
        trace = Trace(
            meta=Meta(created_at="2026-03-23T00:00:00Z"),
            package=Package(), remarks=Remarks(),
            object=Object(kind="stack"),
            states=[State(id=0, data={"items": [], "top": -1})],
            result=Result(final_state=0)
        )
        w = TomlWriter(trace)
        out: Path = tmp_path / "test.toml"
        w.write(str(out))
        content: str = out.read_text(encoding="utf-8")
        assert "[meta]" in content
        assert "[package]" in content
        assert 'version = "0.1.0"' in content
        assert "[result]" in content
        assert "[error]" not in content


class TestHighlightHelpers:
    r"""高亮辅助函数测试"""

    def test_node_helper(self) -> None:
        r"""测试 node() 辅助函数"""
        h: dict[str, Any] = ds4viz.node(0, "focus", 3)
        assert h == {"kind": "node", "target": 0, "style": "focus", "level": 3}

    def test_node_helper_defaults(self) -> None:
        r"""测试 node() 默认参数: style="focus", level=1"""
        h: dict[str, Any] = ds4viz.node(5)
        assert h == {"kind": "node", "target": 5, "style": "focus", "level": 1}

    def test_item_helper(self) -> None:
        r"""测试 item() 辅助函数"""
        h: dict[str, Any] = ds4viz.item(1, "visited", 2)
        assert h == {"kind": "item", "target": 1,
                     "style": "visited", "level": 2}

    def test_item_helper_defaults(self) -> None:
        r"""测试 item() 默认参数: style="focus", level=1"""
        h: dict[str, Any] = ds4viz.item(0)
        assert h == {"kind": "item", "target": 0, "style": "focus", "level": 1}

    def test_edge_helper(self) -> None:
        r"""测试 edge() 辅助函数"""
        h: dict[str, Any] = ds4viz.edge(0, 1, "active", 3)
        assert h == {"kind": "edge", "from": 0,
                     "to": 1, "style": "active", "level": 3}

    def test_edge_helper_defaults(self) -> None:
        r"""测试 edge() 默认参数: style="focus", level=1"""
        h: dict[str, Any] = ds4viz.edge(2, 3)
        assert h == {"kind": "edge", "from": 2,
                     "to": 3, "style": "focus", "level": 1}

    def test_all_predefined_styles(self) -> None:
        r"""测试 IR 定义的所有预定义 style 值均可用于辅助函数"""
        styles: list[str] = ["focus", "visited",
                             "active", "comparing", "found", "error"]
        for style in styles:
            assert ds4viz.node(0, style)["style"] == style
            assert ds4viz.item(0, style)["style"] == style
            assert ds4viz.edge(0, 1, style)["style"] == style


class TestStackUnit:
    r"""Stack 结构单元测试"""

    def test_build_data_empty(self) -> None:
        r"""测试空栈 _build_data 符合 IR: items=[], top=-1"""
        s = Stack(output="dummy.toml")
        data: dict[str, Any] = s._build_data()
        assert data == {"items": [], "top": -1}

    def test_build_data_non_empty(self) -> None:
        r"""测试非空栈 _build_data 符合 IR: top=len(items)-1"""
        s = Stack(output="dummy.toml")
        s._items = [10, 20, 30]
        data: dict[str, Any] = s._build_data()
        assert data == {"items": [10, 20, 30], "top": 2}

    def test_build_data_single_item(self) -> None:
        r"""测试单元素栈 top=0"""
        s = Stack(output="dummy.toml")
        s._items = [42]
        data: dict[str, Any] = s._build_data()
        assert data["top"] == 0

    def test_push_pop_returns_value(self, tmp_path: Path) -> None:
        r"""测试 pop 返回弹出的值"""
        out: Path = tmp_path / "s.toml"
        with Stack(output=str(out)) as s:
            s.push(42)
            val = s.pop()
            assert val == 42

    def test_pop_empty_error(self, tmp_path: Path) -> None:
        r"""测试空栈弹出抛出 StructureError"""
        out: Path = tmp_path / "s.toml"
        with pytest.raises(StructureError):
            with Stack(output=str(out)) as s:
                s.pop()

    def test_push_various_types(self, tmp_path: Path) -> None:
        r"""测试栈支持 IR 定义的所有标量类型: int, float, str, bool"""
        out: Path = tmp_path / "s.toml"
        with Stack(output=str(out)) as s:
            s.push(42)
            s.push(3.14)
            s.push("text")
            s.push(True)
            s.push(False)
            assert len(s._items) == 5

    def test_clear_resets_to_empty(self, tmp_path: Path) -> None:
        r"""测试 clear 后栈为空"""
        out: Path = tmp_path / "s.toml"
        with Stack(output=str(out)) as s:
            s.push(1)
            s.push(2)
            s.clear()
            assert s._build_data() == {"items": [], "top": -1}


class TestQueueUnit:
    r"""Queue 结构单元测试"""

    def test_build_data_empty(self) -> None:
        r"""测试空队列 _build_data 符合 IR: front=-1, rear=-1"""
        q = Queue(output="dummy.toml")
        data: dict[str, Any] = q._build_data()
        assert data == {"items": [], "front": -1, "rear": -1}

    def test_build_data_non_empty(self) -> None:
        r"""测试非空队列 _build_data 符合 IR: front=0, rear=len-1"""
        q = Queue(output="dummy.toml")
        q._items = ["a", "b", "c"]
        data: dict[str, Any] = q._build_data()
        assert data == {"items": ["a", "b", "c"], "front": 0, "rear": 2}

    def test_build_data_single_item(self) -> None:
        r"""测试单元素队列 front=0, rear=0"""
        q = Queue(output="dummy.toml")
        q._items = [99]
        data: dict[str, Any] = q._build_data()
        assert data["front"] == 0
        assert data["rear"] == 0

    def test_dequeue_returns_value(self, tmp_path: Path) -> None:
        r"""测试 dequeue 返回队首元素"""
        out: Path = tmp_path / "q.toml"
        with Queue(output=str(out)) as q:
            q.enqueue("hello")
            val = q.dequeue()
            assert val == "hello"

    def test_dequeue_empty_error(self, tmp_path: Path) -> None:
        r"""测试空队列出队抛出 StructureError"""
        out: Path = tmp_path / "q.toml"
        with pytest.raises(StructureError):
            with Queue(output=str(out)) as q:
                q.dequeue()

    def test_fifo_order(self, tmp_path: Path) -> None:
        r"""测试队列先进先出顺序"""
        out: Path = tmp_path / "q.toml"
        with Queue(output=str(out)) as q:
            q.enqueue(1)
            q.enqueue(2)
            q.enqueue(3)
            assert q.dequeue() == 1
            assert q.dequeue() == 2
            assert q.dequeue() == 3

    def test_clear_resets_to_empty(self, tmp_path: Path) -> None:
        r"""测试 clear 后队列为空"""
        out: Path = tmp_path / "q.toml"
        with Queue(output=str(out)) as q:
            q.enqueue(1)
            q.enqueue(2)
            q.clear()
            assert q._build_data() == {"items": [], "front": -1, "rear": -1}


class TestSingleLinkedListUnit:
    r"""单链表结构单元测试"""

    def test_build_data_empty(self) -> None:
        r"""测试空链表 _build_data 符合 IR: head=-1, nodes=[]"""
        slist = SingleLinkedList(output="dummy.toml")
        data: dict[str, Any] = slist._build_data()
        assert data == {"head": -1, "nodes": []}

    def test_node_exists(self) -> None:
        r"""测试 _node_exists 对存在和不存在的节点"""
        slist = SingleLinkedList(output="dummy.toml")
        slist._nodes[0] = {"value": 10, "next": -1}
        assert slist._node_exists(0) is True
        assert slist._node_exists(999) is False

    def test_insert_head_returns_incremented_id(self, tmp_path: Path) -> None:
        r"""测试 insert_head 返回递增的节点 ID"""
        out: Path = tmp_path / "sl.toml"
        with SingleLinkedList(output=str(out)) as slist:
            a: int = slist.insert_head(10)
            b: int = slist.insert_head(20)
            assert a == 0
            assert b == 1
            assert slist._head == 1

    def test_insert_after_nonexistent_error(self, tmp_path: Path) -> None:
        r"""测试 insert_after 不存在的节点抛出 StructureError"""
        out: Path = tmp_path / "sl.toml"
        with pytest.raises(StructureError):
            with SingleLinkedList(output=str(out)) as slist:
                slist.insert_after(999, 10)

    def test_delete_head_empty_error(self, tmp_path: Path) -> None:
        r"""测试空链表删除头节点抛出 StructureError"""
        out: Path = tmp_path / "sl.toml"
        with pytest.raises(StructureError):
            with SingleLinkedList(output=str(out)) as slist:
                slist.delete_head()

    def test_delete_nonexistent_error(self, tmp_path: Path) -> None:
        r"""测试删除不存在的节点抛出 StructureError"""
        out: Path = tmp_path / "sl.toml"
        with pytest.raises(StructureError):
            with SingleLinkedList(output=str(out)) as slist:
                slist.insert_head(10)
                slist.delete(999)

    def test_reverse_empty(self, tmp_path: Path) -> None:
        r"""测试反转空链表不报错, head 仍为 -1"""
        out: Path = tmp_path / "sl.toml"
        with SingleLinkedList(output=str(out)) as slist:
            slist.reverse()
            assert slist._head == -1

    def test_build_data_includes_alias(self) -> None:
        r"""测试 _build_data 中包含 alias 字段"""
        slist = SingleLinkedList(output="dummy.toml")
        slist._nodes[0] = {"value": 10, "next": -1}
        slist._head = 0
        slist._aliases[0] = "head"
        data: dict[str, Any] = slist._build_data()
        assert data["nodes"][0]["alias"] == "head"


class TestDoubleLinkedListUnit:
    r"""双向链表结构单元测试"""

    def test_build_data_empty(self) -> None:
        r"""测试空双向链表 _build_data 符合 IR: head=-1, tail=-1, nodes=[]"""
        dlist = DoubleLinkedList(output="dummy.toml")
        data: dict[str, Any] = dlist._build_data()
        assert data == {"head": -1, "tail": -1, "nodes": []}

    def test_head_tail_consistency(self, tmp_path: Path) -> None:
        r"""测试 head/tail 同时为 -1 或同时不为 -1, 符合 IR dlist 约束"""
        out: Path = tmp_path / "dl.toml"
        with DoubleLinkedList(output=str(out)) as dlist:
            assert dlist._head == -1 and dlist._tail == -1
            a: int = dlist.insert_head(10)
            assert dlist._head != -1 and dlist._tail != -1
            dlist.delete(a)
            assert dlist._head == -1 and dlist._tail == -1

    def test_delete_tail_empty_error(self, tmp_path: Path) -> None:
        r"""测试空链表删除尾节点抛出 StructureError"""
        out: Path = tmp_path / "dl.toml"
        with pytest.raises(StructureError):
            with DoubleLinkedList(output=str(out)) as dlist:
                dlist.delete_tail()

    def test_delete_head_empty_error(self, tmp_path: Path) -> None:
        r"""测试空链表删除头节点抛出 StructureError"""
        out: Path = tmp_path / "dl.toml"
        with pytest.raises(StructureError):
            with DoubleLinkedList(output=str(out)) as dlist:
                dlist.delete_head()

    def test_reverse_single_node(self, tmp_path: Path) -> None:
        r"""测试反转单节点链表, head/tail 不变"""
        out: Path = tmp_path / "dl.toml"
        with DoubleLinkedList(output=str(out)) as dlist:
            a: int = dlist.insert_head(10)
            dlist.reverse()
            assert dlist._head == a
            assert dlist._tail == a

    def test_insert_before_nonexistent_error(self, tmp_path: Path) -> None:
        r"""测试 insert_before 不存在的节点抛出 StructureError"""
        out: Path = tmp_path / "dl.toml"
        with pytest.raises(StructureError):
            with DoubleLinkedList(output=str(out)) as dlist:
                dlist.insert_before(999, 10)


class TestBinaryTreeUnit:
    r"""二叉树结构单元测试"""

    def test_build_data_empty(self) -> None:
        r"""测试空二叉树 _build_data 符合 IR: root=-1, nodes=[]"""
        bt = BinaryTree(output="dummy.toml")
        data: dict[str, Any] = bt._build_data()
        assert data == {"root": -1, "nodes": []}

    def test_insert_root_returns_id(self, tmp_path: Path) -> None:
        r"""测试 insert_root 返回节点 ID 0"""
        out: Path = tmp_path / "bt.toml"
        with BinaryTree(output=str(out)) as bt:
            root_id: int = bt.insert_root(10)
            assert root_id == 0
            assert bt._root == 0

    def test_insert_root_duplicate_error(self, tmp_path: Path) -> None:
        r"""测试重复插入根节点抛出 StructureError"""
        out: Path = tmp_path / "bt.toml"
        with pytest.raises(StructureError):
            with BinaryTree(output=str(out)) as bt:
                bt.insert_root(10)
                bt.insert_root(20)

    def test_insert_left_existing_child_error(self, tmp_path: Path) -> None:
        r"""测试左子节点已存在时插入抛出 StructureError"""
        out: Path = tmp_path / "bt.toml"
        with pytest.raises(StructureError):
            with BinaryTree(output=str(out)) as bt:
                root: int = bt.insert_root(10)
                bt.insert_left(root, 5)
                bt.insert_left(root, 3)

    def test_insert_right_existing_child_error(self, tmp_path: Path) -> None:
        r"""测试右子节点已存在时插入抛出 StructureError"""
        out: Path = tmp_path / "bt.toml"
        with pytest.raises(StructureError):
            with BinaryTree(output=str(out)) as bt:
                root: int = bt.insert_root(10)
                bt.insert_right(root, 15)
                bt.insert_right(root, 20)

    def test_insert_left_nonexistent_parent_error(self, tmp_path: Path) -> None:
        r"""测试父节点不存在时插入抛出 StructureError"""
        out: Path = tmp_path / "bt.toml"
        with pytest.raises(StructureError):
            with BinaryTree(output=str(out)) as bt:
                bt.insert_root(10)
                bt.insert_left(999, 5)

    def test_delete_root_makes_empty(self, tmp_path: Path) -> None:
        r"""测试删除根节点后树为空"""
        out: Path = tmp_path / "bt.toml"
        with BinaryTree(output=str(out)) as bt:
            root: int = bt.insert_root(10)
            bt.insert_left(root, 5)
            bt.delete(root)
            assert bt._root == -1
            assert len(bt._nodes) == 0

    def test_delete_subtree_removes_all_descendants(self, tmp_path: Path) -> None:
        r"""测试删除节点同时删除所有后代"""
        out: Path = tmp_path / "bt.toml"
        with BinaryTree(output=str(out)) as bt:
            root: int = bt.insert_root(10)
            left: int = bt.insert_left(root, 5)
            bt.insert_left(left, 3)
            bt.insert_right(left, 7)
            bt.delete(left)
            assert left not in bt._nodes
            assert len(bt._nodes) == 1

    def test_delete_nonexistent_error(self, tmp_path: Path) -> None:
        r"""测试删除不存在的节点抛出 StructureError"""
        out: Path = tmp_path / "bt.toml"
        with pytest.raises(StructureError):
            with BinaryTree(output=str(out)) as bt:
                bt.insert_root(10)
                bt.delete(999)

    def test_update_value_nonexistent_error(self, tmp_path: Path) -> None:
        r"""测试更新不存在节点的值抛出 StructureError"""
        out: Path = tmp_path / "bt.toml"
        with pytest.raises(StructureError):
            with BinaryTree(output=str(out)) as bt:
                bt.insert_root(10)
                bt.update_value(999, 100)


class TestBinarySearchTreeUnit:
    r"""二叉搜索树结构单元测试"""

    def test_build_data_empty(self) -> None:
        r"""测试空 BST _build_data 符合 IR: root=-1, nodes=[]"""
        bst = BinarySearchTree(output="dummy.toml")
        data: dict[str, Any] = bst._build_data()
        assert data == {"root": -1, "nodes": []}

    def test_insert_maintains_bst_property(self, tmp_path: Path) -> None:
        r"""测试插入后维持 BST 性质: 左小右大"""
        out: Path = tmp_path / "bst.toml"
        with BinarySearchTree(output=str(out)) as bst:
            root_id: int = bst.insert(10)
            left_id: int = bst.insert(5)
            right_id: int = bst.insert(15)
            assert bst._nodes[root_id]["left"] == left_id
            assert bst._nodes[root_id]["right"] == right_id
            assert bst._nodes[left_id]["value"] == 5
            assert bst._nodes[right_id]["value"] == 15

    def test_delete_nonexistent_value_error(self, tmp_path: Path) -> None:
        r"""测试删除不存在的值抛出 StructureError"""
        out: Path = tmp_path / "bst.toml"
        with pytest.raises(StructureError):
            with BinarySearchTree(output=str(out)) as bst:
                bst.insert(10)
                bst.delete(999)

    def test_delete_leaf_node(self, tmp_path: Path) -> None:
        r"""测试删除叶节点"""
        out: Path = tmp_path / "bst.toml"
        with BinarySearchTree(output=str(out)) as bst:
            bst.insert(10)
            bst.insert(5)
            bst.insert(15)
            bst.delete(5)
            assert bst._find_node_by_value(5) == -1

    def test_delete_node_with_two_children(self, tmp_path: Path) -> None:
        r"""测试删除有两个子节点的节点, 用中序后继替代"""
        out: Path = tmp_path / "bst.toml"
        with BinarySearchTree(output=str(out)) as bst:
            bst.insert(10)
            bst.insert(5)
            bst.insert(15)
            bst.insert(3)
            bst.insert(7)
            bst.delete(5)
            assert bst._find_node_by_value(5) == -1
            assert bst._find_node_by_value(7) != -1
            assert bst._find_node_by_value(3) != -1

    def test_delete_root(self, tmp_path: Path) -> None:
        r"""测试删除根节点"""
        out: Path = tmp_path / "bst.toml"
        with BinarySearchTree(output=str(out)) as bst:
            bst.insert(10)
            bst.insert(5)
            bst.insert(15)
            bst.delete(10)
            assert bst._find_node_by_value(10) == -1
            assert bst._root != -1

    def test_insert_equal_goes_right(self, tmp_path: Path) -> None:
        r"""测试插入相等值时放入右子树"""
        out: Path = tmp_path / "bst.toml"
        with BinarySearchTree(output=str(out)) as bst:
            root_id: int = bst.insert(10)
            dup_id: int = bst.insert(10)
            assert bst._nodes[root_id]["right"] == dup_id


class TestGraphUndirectedUnit:
    r"""无向图结构单元测试"""

    def test_build_data_empty(self) -> None:
        r"""测试空无向图 _build_data 符合 IR: nodes=[], edges=[]"""
        g = GraphUndirected(output="dummy.toml")
        data: dict[str, Any] = g._build_data()
        assert data == {"nodes": [], "edges": []}

    def test_edge_normalization(self) -> None:
        r"""测试无向边内部规范化为 from < to, 符合 IR 无向边约束"""
        assert GraphUndirected._normalize_edge(3, 1) == (1, 3)
        assert GraphUndirected._normalize_edge(1, 3) == (1, 3)
        assert GraphUndirected._normalize_edge(0, 5) == (0, 5)

    def test_self_loop_error(self, tmp_path: Path) -> None:
        r"""测试自环抛出 StructureError, 符合 IR 约束"""
        out: Path = tmp_path / "gu.toml"
        with pytest.raises(StructureError):
            with GraphUndirected(output=str(out)) as g:
                g.add_node(0, "A")
                g.add_edge(0, 0)

    def test_duplicate_node_error(self, tmp_path: Path) -> None:
        r"""测试重复添加节点抛出 StructureError"""
        out: Path = tmp_path / "gu.toml"
        with pytest.raises(StructureError):
            with GraphUndirected(output=str(out)) as g:
                g.add_node(0, "A")
                g.add_node(0, "B")

    def test_duplicate_edge_error(self, tmp_path: Path) -> None:
        r"""测试重复添加边抛出 StructureError, 符合 IR 多重边约束"""
        out: Path = tmp_path / "gu.toml"
        with pytest.raises(StructureError):
            with GraphUndirected(output=str(out)) as g:
                g.add_node(0, "A")
                g.add_node(1, "B")
                g.add_edge(0, 1)
                g.add_edge(0, 1)

    def test_duplicate_edge_reversed_error(self, tmp_path: Path) -> None:
        r"""测试反向重复添加无向边也抛出 StructureError"""
        out: Path = tmp_path / "gu.toml"
        with pytest.raises(StructureError):
            with GraphUndirected(output=str(out)) as g:
                g.add_node(0, "A")
                g.add_node(1, "B")
                g.add_edge(0, 1)
                g.add_edge(1, 0)

    def test_add_edge_nonexistent_node_error(self, tmp_path: Path) -> None:
        r"""测试对不存在的节点添加边抛出 StructureError"""
        out: Path = tmp_path / "gu.toml"
        with pytest.raises(StructureError):
            with GraphUndirected(output=str(out)) as g:
                g.add_node(0, "A")
                g.add_edge(0, 1)

    def test_remove_edge_nonexistent_error(self, tmp_path: Path) -> None:
        r"""测试删除不存在的边抛出 StructureError"""
        out: Path = tmp_path / "gu.toml"
        with pytest.raises(StructureError):
            with GraphUndirected(output=str(out)) as g:
                g.add_node(0, "A")
                g.add_node(1, "B")
                g.remove_edge(0, 1)

    def test_remove_node_nonexistent_error(self, tmp_path: Path) -> None:
        r"""测试删除不存在的节点抛出 StructureError"""
        out: Path = tmp_path / "gu.toml"
        with pytest.raises(StructureError):
            with GraphUndirected(output=str(out)) as g:
                g.remove_node(999)

    def test_update_label_nonexistent_error(self, tmp_path: Path) -> None:
        r"""测试更新不存在节点的标签抛出 StructureError"""
        out: Path = tmp_path / "gu.toml"
        with pytest.raises(StructureError):
            with GraphUndirected(output=str(out)) as g:
                g.update_node_label(999, "X")


class TestGraphDirectedUnit:
    r"""有向图结构单元测试"""

    def test_build_data_empty(self) -> None:
        r"""测试空有向图 _build_data"""
        g = GraphDirected(output="dummy.toml")
        data: dict[str, Any] = g._build_data()
        assert data == {"nodes": [], "edges": []}

    def test_directed_edges_no_normalization(self, tmp_path: Path) -> None:
        r"""测试有向图边保持方向, 不做规范化"""
        out: Path = tmp_path / "gd.toml"
        with GraphDirected(output=str(out)) as g:
            g.add_node(0, "A")
            g.add_node(1, "B")
            g.add_edge(1, 0)
            assert (1, 0) in g._edges
            assert (0, 1) not in g._edges

    def test_both_directions_allowed(self, tmp_path: Path) -> None:
        r"""测试有向图允许双向边"""
        out: Path = tmp_path / "gd.toml"
        with GraphDirected(output=str(out)) as g:
            g.add_node(0, "A")
            g.add_node(1, "B")
            g.add_edge(0, 1)
            g.add_edge(1, 0)
            assert (0, 1) in g._edges
            assert (1, 0) in g._edges

    def test_self_loop_error(self, tmp_path: Path) -> None:
        r"""测试有向图自环抛出 StructureError, 符合 IR 约束"""
        out: Path = tmp_path / "gd.toml"
        with pytest.raises(StructureError):
            with GraphDirected(output=str(out)) as g:
                g.add_node(0, "A")
                g.add_edge(0, 0)

    def test_duplicate_directed_edge_error(self, tmp_path: Path) -> None:
        r"""测试重复有向边抛出 StructureError, 符合 IR 多重边约束"""
        out: Path = tmp_path / "gd.toml"
        with pytest.raises(StructureError):
            with GraphDirected(output=str(out)) as g:
                g.add_node(0, "A")
                g.add_node(1, "B")
                g.add_edge(0, 1)
                g.add_edge(0, 1)


class TestGraphWeightedUnit:
    r"""带权图结构单元测试"""

    def test_build_data_with_weight(self) -> None:
        r"""测试带权图 _build_data 包含 weight 字段"""
        g = GraphWeighted(output="dummy.toml")
        g._nodes[0] = "A"
        g._nodes[1] = "B"
        g._edges[(0, 1)] = 3.5
        data: dict[str, Any] = g._build_data()
        assert len(data["edges"]) == 1
        assert data["edges"][0]["weight"] == 3.5

    def test_self_loop_error(self, tmp_path: Path) -> None:
        r"""测试带权图自环抛出 StructureError, 符合 IR 约束"""
        out: Path = tmp_path / "gw.toml"
        with pytest.raises(StructureError):
            with GraphWeighted(output=str(out)) as g:
                g.add_node(0, "A")
                g.add_edge(0, 0, 1.0)

    def test_remove_edge_nonexistent_error(self, tmp_path: Path) -> None:
        r"""测试删除不存在的边抛出 StructureError"""
        out: Path = tmp_path / "gw.toml"
        with pytest.raises(StructureError):
            with GraphWeighted(output=str(out)) as g:
                g.add_node(0, "A")
                g.add_node(1, "B")
                g.remove_edge(0, 1)

    def test_update_weight_nonexistent_error(self, tmp_path: Path) -> None:
        r"""测试更新不存在边的权重抛出 StructureError"""
        out: Path = tmp_path / "gw.toml"
        with pytest.raises(StructureError):
            with GraphWeighted(output=str(out)) as g:
                g.add_node(0, "A")
                g.add_node(1, "B")
                g.update_weight(0, 1, 5.0)

    def test_label_boundary_valid(self, tmp_path: Path) -> None:
        r"""测试节点标签长度 1 和 32 均合法, 符合 IR label 长度 1..32"""
        out: Path = tmp_path / "gw.toml"
        with GraphWeighted(output=str(out)) as g:
            g.add_node(0, "A")
            g.add_node(1, "B" * 32)
            assert g._nodes[1] == "B" * 32

    def test_label_empty_error(self, tmp_path: Path) -> None:
        r"""测试空标签抛出 StructureError, 符合 IR label 长度 1..32 约束"""
        out: Path = tmp_path / "gw.toml"
        with pytest.raises(StructureError):
            with GraphWeighted(output=str(out)) as g:
                g.add_node(0, "")

    def test_label_too_long_error(self, tmp_path: Path) -> None:
        r"""测试标签超过 32 字符抛出 StructureError"""
        out: Path = tmp_path / "gw.toml"
        with pytest.raises(StructureError):
            with GraphWeighted(output=str(out)) as g:
                g.add_node(0, "A" * 33)

    def test_negative_weight_allowed(self, tmp_path: Path) -> None:
        r"""测试负权重合法, IR weight 仅要求为 number"""
        out: Path = tmp_path / "gw.toml"
        with GraphWeighted(output=str(out)) as g:
            g.add_node(0, "A")
            g.add_node(1, "B")
            g.add_edge(0, 1, -5.5)
            assert g._edges[(0, 1)] == -5.5

    def test_zero_weight_allowed(self, tmp_path: Path) -> None:
        r"""测试零权重合法"""
        out: Path = tmp_path / "gw.toml"
        with GraphWeighted(output=str(out)) as g:
            g.add_node(0, "A")
            g.add_node(1, "B")
            g.add_edge(0, 1, 0)
            assert g._edges[(0, 1)] == 0

    def test_integer_weight_allowed(self, tmp_path: Path) -> None:
        r"""测试整数权重合法"""
        out: Path = tmp_path / "gw.toml"
        with GraphWeighted(output=str(out)) as g:
            g.add_node(0, "A")
            g.add_node(1, "B")
            g.add_edge(0, 1, 10)
            assert g._edges[(0, 1)] == 10


class TestStepAmendPhase:
    r"""step / amend / phase 通用方法测试"""

    def test_step_creates_observe(self, tmp_path: Path) -> None:
        r"""测试 step() 创建 op="observe" 的步骤"""
        output: Path = tmp_path / "step.toml"
        with Stack(output=str(output)) as s:
            s.push(10)
            s.step(note="观察")
        content: str = output.read_text(encoding="utf-8")
        assert 'op = "observe"' in content
        assert 'note = "观察"' in content

    def test_step_without_note_or_highlights(self, tmp_path: Path) -> None:
        r"""测试 step() 不传 note 和 highlights 时, 对应字段为 None"""
        output: Path = tmp_path / "step_bare.toml"
        with Stack(output=str(output)) as s:
            s.push(10)
            s.step()
            last: Step | None = s.session.get_last_step()
            assert last is not None
            assert last.op == "observe"
            assert last.note is None
            assert last.highlights is None

    def test_amend_modifies_last_step(self, tmp_path: Path) -> None:
        r"""测试 amend() 修改上一步的 note 和 highlights"""
        output: Path = tmp_path / "amend.toml"
        with Stack(output=str(output)) as s:
            s.push(10)
            s.amend(note="已压入 10", highlights=[ds4viz.item(0, "focus", 3)])
        content: str = output.read_text(encoding="utf-8")
        assert 'note = "已压入 10"' in content
        assert "highlights" in content

    def test_amend_none_preserves_existing(self, tmp_path: Path) -> None:
        r"""测试 amend(None) 不修改对应字段"""
        output: Path = tmp_path / "amend_none.toml"
        with Stack(output=str(output)) as s:
            s.push(10)
            s.amend(note="first")
            s.amend(highlights=[ds4viz.item(0, "visited")])
        content: str = output.read_text(encoding="utf-8")
        assert 'note = "first"' in content
        assert "highlights" in content

    def test_amend_empty_highlights_clears(self, tmp_path: Path) -> None:
        r"""测试 amend(highlights=[]) 清除高亮"""
        output: Path = tmp_path / "amend_clear.toml"
        with Stack(output=str(output)) as s:
            s.push(10)
            s.amend(highlights=[ds4viz.item(0, "focus")])
            s.amend(highlights=[])
        content: str = output.read_text(encoding="utf-8")
        assert "highlights = []" in content

    def test_amend_no_previous_step_error(self, tmp_path: Path) -> None:
        r"""测试无前一步时 amend 抛出 RuntimeError"""
        output: Path = tmp_path / "amend_err.toml"
        with pytest.raises(RuntimeError, match="No previous step"):
            with Stack(output=str(output)) as s:
                s.amend(note="fail")

    def test_phase_context(self, tmp_path: Path) -> None:
        r"""测试 phase 上下文管理器标记步骤"""
        output: Path = tmp_path / "phase.toml"
        with Stack(output=str(output)) as s:
            with s.phase("build"):
                s.push(10)
                s.push(20)
            with s.phase("modify"):
                s.pop()
        content: str = output.read_text(encoding="utf-8")
        assert 'phase = "build"' in content
        assert 'phase = "modify"' in content

    def test_phase_nesting(self, tmp_path: Path) -> None:
        r"""测试 phase 嵌套时内层优先"""
        output: Path = tmp_path / "phase_nest.toml"
        with Stack(output=str(output)) as s:
            with s.phase("outer"):
                s.push(10)
                with s.phase("inner"):
                    s.push(20)
                s.push(30)
            steps: list[Step] = s._session._steps
            assert steps[0].phase == "outer"
            assert steps[1].phase == "inner"
            assert steps[2].phase == "outer"

    def test_no_phase_outside_context(self, tmp_path: Path) -> None:
        r"""测试 phase 上下文外的步骤不携带 phase"""
        output: Path = tmp_path / "no_phase.toml"
        with Stack(output=str(output)) as s:
            s.push(10)
            with s.phase("build"):
                s.push(20)
            s.push(30)
            steps: list[Step] = s._session._steps
            assert steps[0].phase is None
            assert steps[1].phase == "build"
            assert steps[2].phase is None

    def test_step_with_highlights(self, tmp_path: Path) -> None:
        r"""测试 step 带 highlights 在树结构上"""
        output: Path = tmp_path / "step_hl.toml"
        with BinaryTree(output=str(output)) as tree:
            root: int = tree.insert_root(10)
            left: int = tree.insert_left(root, 5)
            tree.step(note="观察", highlights=[
                ds4viz.node(root, "visited"),
                ds4viz.node(left, "focus", level=3)
            ])
        content: str = output.read_text(encoding="utf-8")
        assert 'op = "observe"' in content
        assert 'style = "visited"' in content
        assert 'style = "focus"' in content

    def test_step_does_not_produce_new_state(self, tmp_path: Path) -> None:
        r"""测试 step() 不改变数据也不产生新状态快照, before == after"""
        output: Path = tmp_path / "step_state.toml"
        with Stack(output=str(output)) as s:
            s.push(10)
            state_before: int = s._session.get_last_state_id()
            s.step(note="观察")
            state_after: int = s._session.get_last_state_id()
            assert state_after == state_before
            last_step: Step | None = s._session.get_last_step()
            assert last_step is not None
            assert last_step.before == last_step.after

    def test_step_records_caller_line_number(
        self, tmp_path: Path
    ) -> None:
        r"""测试 step() 记录的行号精确指向调用处源码行,
        确保前端可以正确高亮对应代码行"""
        output: Path = tmp_path / "line_check.toml"
        with Stack(output=str(output)) as s:
            s.push(42)
            expected_line: int = inspect.currentframe().f_lineno + 1
            s.step(note="检查行号")
            last_step: Step = s._session._steps[-1]
            assert last_step.code is not None, (
                "step 应记录 code 字段"
            )
            assert last_step.code.line == expected_line, (
                f"期望行号 {expected_line}, "
                f"实际 {last_step.code.line}"
            )
            assert last_step.before == last_step.after, (
                f"observe 步骤 before({last_step.before}) "
                f"应等于 after({last_step.after})"
            )

class TestAliasMethod:
    r"""alias 方法测试"""

    def test_alias_in_slist(self, tmp_path: Path) -> None:
        r"""测试单链表设置别名"""
        output: Path = tmp_path / "alias_slist.toml"
        with SingleLinkedList(output=str(output)) as slist:
            a: int = slist.insert_head(10)
            slist.alias(a, "head")
            slist.insert_tail(20)
        content: str = output.read_text(encoding="utf-8")
        assert 'alias = "head"' in content

    def test_alias_clear(self, tmp_path: Path) -> None:
        r"""测试传入 None 清除别名"""
        output: Path = tmp_path / "alias_clear.toml"
        with SingleLinkedList(output=str(output)) as slist:
            a: int = slist.insert_head(10)
            slist.alias(a, "head")
            slist.alias(a, None)
            slist.insert_tail(20)
        content: str = output.read_text(encoding="utf-8")
        last_state_section: str = content.split("[[states]]")[-1]
        assert 'alias = "head"' not in last_state_section

    def test_alias_nonexistent_node_error(self, tmp_path: Path) -> None:
        r"""测试对不存在节点设置别名抛出 StructureError"""
        output: Path = tmp_path / "alias_err.toml"
        with pytest.raises(StructureError):
            with SingleLinkedList(output=str(output)) as slist:
                slist.alias(999, "x")

    def test_alias_in_graph(self, tmp_path: Path) -> None:
        r"""测试图结构设置别名"""
        output: Path = tmp_path / "alias_graph.toml"
        with GraphWeighted(output=str(output)) as g:
            g.add_node(0, "A")
            g.alias(0, "source")
            g.add_node(1, "B")
        content: str = output.read_text(encoding="utf-8")
        assert 'alias = "source"' in content

    def test_alias_removed_on_delete(self, tmp_path: Path) -> None:
        r"""测试删除节点时别名被清除"""
        output: Path = tmp_path / "alias_del.toml"
        with BinaryTree(output=str(output)) as tree:
            root: int = tree.insert_root(10)
            tree.alias(root, "root")
            tree.delete(root)
        content: str = output.read_text(encoding="utf-8")
        last_state: str = content.split("[[states]]")[-1].split("[[steps]]")[0]
        assert 'alias = "root"' not in last_state

    def test_alias_invalid_characters_error(self, tmp_path: Path) -> None:
        r"""测试别名包含非法字符抛出 StructureError, 符合 IR 字符集约束 a-zA-Z0-9_-"""
        output: Path = tmp_path / "alias_inv.toml"
        with pytest.raises(StructureError):
            with SingleLinkedList(output=str(output)) as slist:
                a: int = slist.insert_head(10)
                slist.alias(a, "invalid name!")

    def test_alias_space_error(self, tmp_path: Path) -> None:
        r"""测试别名包含空格抛出 StructureError"""
        output: Path = tmp_path / "alias_space.toml"
        with pytest.raises(StructureError):
            with SingleLinkedList(output=str(output)) as slist:
                a: int = slist.insert_head(10)
                slist.alias(a, "a b")

    def test_alias_too_long_error(self, tmp_path: Path) -> None:
        r"""测试别名超过 64 字符抛出 StructureError, 符合 IR 长度 1..64 约束"""
        output: Path = tmp_path / "alias_long.toml"
        with pytest.raises(StructureError):
            with SingleLinkedList(output=str(output)) as slist:
                a: int = slist.insert_head(10)
                slist.alias(a, "a" * 65)

    def test_alias_duplicate_error(self, tmp_path: Path) -> None:
        r"""测试同一 state 内别名重复抛出 StructureError, 符合 IR 唯一性约束"""
        output: Path = tmp_path / "alias_dup.toml"
        with pytest.raises(StructureError):
            with SingleLinkedList(output=str(output)) as slist:
                a: int = slist.insert_head(10)
                b: int = slist.insert_tail(20)
                slist.alias(a, "same")
                slist.alias(b, "same")


    def test_alias_boundary_length(self, tmp_path: Path) -> None:
        r"""测试别名边界长度: 1 和 64 均合法"""
        output: Path = tmp_path / "alias_boundary.toml"
        with SingleLinkedList(output=str(output)) as slist:
            a: int = slist.insert_head(10)
            b: int = slist.insert_tail(20)
            slist.alias(a, "x")
            slist.alias(b, "a" * 64)
            slist.step(note="检查别名")
        content: str = output.read_text(encoding="utf-8")
        assert 'alias = "x"' in content

    def test_alias_reassign(self, tmp_path: Path) -> None:
        r"""测试别名重新赋值, 旧值被覆盖"""
        output: Path = tmp_path / "alias_reassign.toml"
        with SingleLinkedList(output=str(output)) as slist:
            a: int = slist.insert_head(10)
            slist.alias(a, "first")
            slist.alias(a, "second")
            slist.insert_tail(20)
        content: str = output.read_text(encoding="utf-8")
        last_state_section: str = content.split("[[states]]")[-1]
        assert 'alias = "second"' in last_state_section

    def test_alias_removed_on_graph_remove_node(self, tmp_path: Path) -> None:
        r"""测试图结构删除节点时别名被清除"""
        output: Path = tmp_path / "alias_graph_del.toml"
        with GraphDirected(output=str(output)) as g:
            g.add_node(0, "A")
            g.alias(0, "source")
            g.add_node(1, "B")
            g.remove_node(0)
            assert 0 not in g._aliases

    def test_alias_removed_on_slist_delete(self, tmp_path: Path) -> None:
        r"""测试单链表删除节点时别名被清除"""
        output: Path = tmp_path / "alias_slist_del.toml"
        with SingleLinkedList(output=str(output)) as slist:
            a: int = slist.insert_head(10)
            slist.alias(a, "head")
            slist.delete(a)
            assert a not in slist._aliases


class TestApiConsistency:
    r"""API 一致性测试"""

    def test_all_factory_functions_exist(self) -> None:
        r"""测试所有工厂函数存在且可调用"""
        for name in ["stack", "queue", "single_linked_list", "double_linked_list",
                     "binary_tree", "binary_search_tree",
                     "graph_undirected", "graph_directed", "graph_weighted",
                     "config", "node", "item", "edge"]:
            assert callable(getattr(ds4viz, name))

    def test_all_classes_exported(self) -> None:
        r"""测试所有数据结构类被导出"""
        for cls_name in ["Stack", "Queue", "SingleLinkedList", "DoubleLinkedList",
                         "BinaryTree", "BinarySearchTree",
                         "GraphUndirected", "GraphDirected", "GraphWeighted"]:
            assert hasattr(ds4viz, cls_name)

    def test_version_info(self) -> None:
        r"""测试版本信息格式为语义化版本, 符合 IR package.version 约束"""
        import re
        assert re.match(r"^\d+\.\d+\.\d+$", ds4viz.__version__)
        assert ds4viz.__version__ == "0.1.0"

    def test_no_heap_exported(self) -> None:
        r"""测试 Heap 未导出 (尚未实现)"""
        assert not hasattr(ds4viz, "Heap")
        assert not hasattr(ds4viz, "heap")

    def test_all_in_all_list(self) -> None:
        r"""测试 __all__ 中的所有名称均可访问"""
        for name in ds4viz.__all__:
            assert hasattr(
                ds4viz, name), f"{name} in __all__ but not accessible"

    def test_factory_returns_correct_type(self) -> None:
        r"""测试工厂函数返回正确的类型"""
        assert isinstance(ds4viz.stack(), Stack)
        assert isinstance(ds4viz.queue(), Queue)
        assert isinstance(ds4viz.single_linked_list(), SingleLinkedList)
        assert isinstance(ds4viz.double_linked_list(), DoubleLinkedList)
        assert isinstance(ds4viz.binary_tree(), BinaryTree)
        assert isinstance(ds4viz.binary_search_tree(), BinarySearchTree)
        assert isinstance(ds4viz.graph_undirected(), GraphUndirected)
        assert isinstance(ds4viz.graph_directed(), GraphDirected)
        assert isinstance(ds4viz.graph_weighted(), GraphWeighted)
