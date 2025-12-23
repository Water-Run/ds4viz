r"""
单元测试模块

:file: test/unit_test.py
:author: WaterRun
:time: 2025-12-23
"""

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


class TestExceptionModule:
    r"""异常模块单元测试"""

    def test_error_type_enum_values(self) -> None:
        r"""测试 ErrorType 枚举值"""
        assert ErrorType.RUNTIME.value == "runtime"
        assert ErrorType.TIMEOUT.value == "timeout"
        assert ErrorType.VALIDATION.value == "validation"
        assert ErrorType.SANDBOX.value == "sandbox"
        assert ErrorType.UNKNOWN.value == "unknown"

    def test_ds4viz_error_basic(self) -> None:
        r"""测试 DS4VizError 基本功能"""
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

    def test_structure_error(self) -> None:
        r"""测试 StructureError"""
        error = StructureError("structure error", 10)
        assert error.message == "structure error"
        assert error.error_type == ErrorType.RUNTIME
        assert error.line == 10

    def test_validation_error(self) -> None:
        r"""测试 ValidationError"""
        error = ValidationError("validation error", 20)
        assert error.message == "validation error"
        assert error.error_type == ErrorType.VALIDATION
        assert error.line == 20

    def test_error_inheritance(self) -> None:
        r"""测试异常继承关系"""
        assert issubclass(DS4VizError, Exception)
        assert issubclass(StructureError, DS4VizError)
        assert issubclass(ValidationError, DS4VizError)


class TestTraceDataclasses:
    r"""Trace 数据类单元测试"""

    def test_meta_creation(self) -> None:
        r"""测试 Meta 创建"""
        meta = Meta(created_at="2025-12-23T00:00:00Z", lang="python", lang_version="3.12.0")
        assert meta.created_at == "2025-12-23T00:00:00Z"
        assert meta.lang == "python"
        assert meta.lang_version == "3.12.0"

    def test_meta_default_values(self) -> None:
        r"""测试 Meta 默认值"""
        meta = Meta(created_at="2025-12-23T00:00:00Z")
        assert meta.lang == "python"
        assert meta.lang_version == ""

    def test_package_creation(self) -> None:
        r"""测试 Package 创建"""
        package = Package()
        assert package.name == "ds4viz"
        assert package.lang == "python"
        assert package.version == "1.0.0"

    def test_remarks_is_empty(self) -> None:
        r"""测试 Remarks.is_empty 方法"""
        empty_remarks = Remarks()
        assert empty_remarks.is_empty() is True

        remarks_with_title = Remarks(title="Test")
        assert remarks_with_title.is_empty() is False

        remarks_with_author = Remarks(author="Author")
        assert remarks_with_author.is_empty() is False

        remarks_with_comment = Remarks(comment="Comment")
        assert remarks_with_comment.is_empty() is False

        full_remarks = Remarks(title="T", author="A", comment="C")
        assert full_remarks.is_empty() is False

    def test_object_creation(self) -> None:
        r"""测试 Object 创建"""
        obj = Object(kind="stack", label="my_stack")
        assert obj.kind == "stack"
        assert obj.label == "my_stack"

    def test_state_creation(self) -> None:
        r"""测试 State 创建"""
        state = State(id=0, data={"items": [1, 2, 3], "top": 2})
        assert state.id == 0
        assert state.data == {"items": [1, 2, 3], "top": 2}

    def test_state_default_data(self) -> None:
        r"""测试 State 默认数据"""
        state = State(id=1)
        assert state.data == {}

    def test_code_location(self) -> None:
        r"""测试 CodeLocation 创建"""
        loc = CodeLocation(line=10, col=5)
        assert loc.line == 10
        assert loc.col == 5

        loc_no_col = CodeLocation(line=20)
        assert loc_no_col.line == 20
        assert loc_no_col.col is None

    def test_step_creation(self) -> None:
        r"""测试 Step 创建"""
        step = Step(
            id=0,
            op="push",
            before=0,
            after=1,
            args={"value": 10},
            code=CodeLocation(line=5),
            ret=None,
            note="test note"
        )
        assert step.id == 0
        assert step.op == "push"
        assert step.before == 0
        assert step.after == 1
        assert step.args == {"value": 10}
        assert step.code is not None
        assert step.code.line == 5
        assert step.note == "test note"

    def test_step_minimal(self) -> None:
        r"""测试 Step 最小创建"""
        step = Step(id=0, op="clear", before=0)
        assert step.after is None
        assert step.args == {}
        assert step.code is None
        assert step.ret is None
        assert step.note is None

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
        assert result.commit.line == 50

    def test_error_creation(self) -> None:
        r"""测试 Error 创建"""
        error = Error(
            type="runtime",
            message="test error",
            line=30,
            step=2,
            last_state=1
        )
        assert error.type == "runtime"
        assert error.message == "test error"
        assert error.line == 30
        assert error.step == 2
        assert error.last_state == 1

    def test_trace_creation(self) -> None:
        r"""测试 Trace 创建"""
        trace = Trace(
            meta=Meta(created_at="2025-12-23T00:00:00Z"),
            package=Package(),
            remarks=Remarks(),
            object=Object(kind="stack"),
            states=[State(id=0, data={"items": [], "top": -1})],
            steps=[],
            result=Result(final_state=0),
            error=None
        )
        assert trace.meta.created_at == "2025-12-23T00:00:00Z"
        assert trace.object.kind == "stack"
        assert len(trace.states) == 1
        assert trace.result is not None
        assert trace.error is None


class TestGlobalConfig:
    r"""全局配置单元测试"""

    def test_default_config(self) -> None:
        r"""测试默认配置"""
        cfg = GlobalConfig()
        assert cfg.output_path == "trace.toml"
        assert cfg.title == ""
        assert cfg.author == ""
        assert cfg.comment == ""

    def test_config_function(self) -> None:
        r"""测试 config 函数"""
        config(
            output_path="custom.toml",
            title="Test Title",
            author="Test Author",
            comment="Test Comment"
        )
        cfg = get_config()
        assert cfg.output_path == "custom.toml"
        assert cfg.title == "Test Title"
        assert cfg.author == "Test Author"
        assert cfg.comment == "Test Comment"

        # 恢复默认配置
        config()

    def test_get_python_version(self) -> None:
        r"""测试获取 Python 版本"""
        version = get_python_version()
        parts = version.split(".")
        assert len(parts) == 3
        assert all(part.isdigit() for part in parts)

    def test_get_caller_line(self) -> None:
        r"""测试获取调用行号"""
        line = get_caller_line(1)
        assert isinstance(line, int)
        assert line > 0


class TestSession:
    r"""Session 单元测试"""

    def test_session_init(self) -> None:
        r"""测试 Session 初始化"""
        session = Session(kind="stack", label="test_stack", output="test.toml")
        assert session.step_counter == 0
        assert session.failed_step_id is None
        assert session.get_last_state_id() == -1

    def test_session_add_state(self) -> None:
        r"""测试添加状态"""
        session = Session(kind="stack", label="test")
        state_id = session.add_state({"items": [], "top": -1})
        assert state_id == 0
        assert session.get_last_state_id() == 0

        state_id2 = session.add_state({"items": [10], "top": 0})
        assert state_id2 == 1
        assert session.get_last_state_id() == 1

    def test_session_add_step(self) -> None:
        r"""测试添加步骤"""
        session = Session(kind="stack", label="test")
        session.add_state({"items": [], "top": -1})
        session.add_state({"items": [10], "top": 0})

        step_id = session.add_step(
            op="push",
            before=0,
            after=1,
            args={"value": 10},
            ret=None,
            note="pushed 10",
            line=5
        )
        assert step_id == 0
        assert session.step_counter == 1

    def test_session_set_error(self) -> None:
        r"""测试设置错误"""
        session = Session(kind="stack", label="test")
        session.add_state({"items": [], "top": -1})
        session.set_error(
            error_type=ErrorType.RUNTIME,
            message="test error",
            line=10,
            step_id=0
        )
        assert session.failed_step_id == 0

    def test_session_set_lines(self) -> None:
        r"""测试设置入口和退出行号"""
        session = Session(kind="stack", label="test")
        session.set_entry_line(5)
        session.set_exit_line(20)
        # 这些值在 build_trace 时会被使用

    def test_session_build_trace_success(self) -> None:
        r"""测试构建成功的 Trace"""
        session = Session(kind="stack", label="test_stack")
        session.set_entry_line(1)
        session.set_exit_line(10)
        session.add_state({"items": [], "top": -1})
        session.add_state({"items": [10], "top": 0})
        session.add_step(op="push", before=0, after=1, args={"value": 10}, line=5)

        trace = session.build_trace()
        assert trace.object.kind == "stack"
        assert trace.object.label == "test_stack"
        assert len(trace.states) == 2
        assert len(trace.steps) == 1
        assert trace.result is not None
        assert trace.result.final_state == 1
        assert trace.error is None

    def test_session_build_trace_error(self) -> None:
        r"""测试构建带错误的 Trace"""
        session = Session(kind="stack", label="test_stack")
        session.add_state({"items": [], "top": -1})
        session.set_error(
            error_type=ErrorType.RUNTIME,
            message="Cannot pop from empty stack",
            line=10,
            step_id=None
        )

        trace = session.build_trace()
        assert trace.result is None
        assert trace.error is not None
        assert trace.error.type == "runtime"
        assert trace.error.message == "Cannot pop from empty stack"


class TestTomlWriter:
    r"""TomlWriter 单元测试"""

    def test_escape_string(self) -> None:
        r"""测试字符串转义"""
        assert TomlWriter._escape_string('hello') == 'hello'
        assert TomlWriter._escape_string('hello"world') == 'hello\\"world'
        assert TomlWriter._escape_string('hello\\world') == 'hello\\\\world'
        assert TomlWriter._escape_string('hello\nworld') == 'hello\\nworld'
        assert TomlWriter._escape_string('hello\rworld') == 'hello\\rworld'

    def test_serialize_value_bool(self) -> None:
        r"""测试序列化布尔值"""
        trace = Trace(
            meta=Meta(created_at="2025-12-23T00:00:00Z"),
            package=Package(),
            remarks=Remarks(),
            object=Object(kind="stack"),
            states=[],
            steps=[]
        )
        writer = TomlWriter(trace)
        assert writer._serialize_value(True) == "true"
        assert writer._serialize_value(False) == "false"

    def test_serialize_value_numbers(self) -> None:
        r"""测试序列化数字"""
        trace = Trace(
            meta=Meta(created_at="2025-12-23T00:00:00Z"),
            package=Package(),
            remarks=Remarks(),
            object=Object(kind="stack"),
            states=[],
            steps=[]
        )
        writer = TomlWriter(trace)
        assert writer._serialize_value(42) == "42"
        assert writer._serialize_value(-10) == "-10"
        assert writer._serialize_value(3.14) == "3.14"

    def test_serialize_value_string(self) -> None:
        r"""测试序列化字符串"""
        trace = Trace(
            meta=Meta(created_at="2025-12-23T00:00:00Z"),
            package=Package(),
            remarks=Remarks(),
            object=Object(kind="stack"),
            states=[],
            steps=[]
        )
        writer = TomlWriter(trace)
        assert writer._serialize_value("hello") == '"hello"'
        assert writer._serialize_value('say "hi"') == '"say \\"hi\\""'

    def test_serialize_simple_array(self) -> None:
        r"""测试序列化简单数组"""
        trace = Trace(
            meta=Meta(created_at="2025-12-23T00:00:00Z"),
            package=Package(),
            remarks=Remarks(),
            object=Object(kind="stack"),
            states=[],
            steps=[]
        )
        writer = TomlWriter(trace)
        assert writer._serialize_simple_array([]) == "[]"
        assert writer._serialize_simple_array([1, 2, 3]) == "[1, 2, 3]"
        assert writer._serialize_simple_array(["a", "b"]) == '["a", "b"]'
        assert writer._serialize_simple_array([True, False]) == "[true, false]"

    def test_serialize_inline_table_array(self) -> None:
        r"""测试序列化 inline table 数组"""
        trace = Trace(
            meta=Meta(created_at="2025-12-23T00:00:00Z"),
            package=Package(),
            remarks=Remarks(),
            object=Object(kind="stack"),
            states=[],
            steps=[]
        )
        writer = TomlWriter(trace)
        assert writer._serialize_inline_table_array([]) == "[]"

        result = writer._serialize_inline_table_array([{"id": 0, "value": 10}])
        assert "id = 0" in result
        assert "value = 10" in result

    def test_serialize_meta(self) -> None:
        r"""测试序列化 meta 部分"""
        trace = Trace(
            meta=Meta(created_at="2025-12-23T00:00:00Z", lang="python", lang_version="3.12.0"),
            package=Package(),
            remarks=Remarks(),
            object=Object(kind="stack"),
            states=[],
            steps=[]
        )
        writer = TomlWriter(trace)
        lines = writer._serialize_meta()
        assert "[meta]" in lines
        assert 'created_at = "2025-12-23T00:00:00Z"' in lines
        assert 'lang = "python"' in lines
        assert 'lang_version = "3.12.0"' in lines

    def test_serialize_package(self) -> None:
        r"""测试序列化 package 部分"""
        trace = Trace(
            meta=Meta(created_at="2025-12-23T00:00:00Z"),
            package=Package(),
            remarks=Remarks(),
            object=Object(kind="stack"),
            states=[],
            steps=[]
        )
        writer = TomlWriter(trace)
        lines = writer._serialize_package()
        assert "[package]" in lines
        assert 'name = "ds4viz"' in lines
        assert 'lang = "python"' in lines
        assert 'version = "1.0.0"' in lines

    def test_serialize_remarks_empty(self) -> None:
        r"""测试序列化空 remarks"""
        trace = Trace(
            meta=Meta(created_at="2025-12-23T00:00:00Z"),
            package=Package(),
            remarks=Remarks(),
            object=Object(kind="stack"),
            states=[],
            steps=[]
        )
        writer = TomlWriter(trace)
        content = writer._serialize()
        assert "[remarks]" not in content

    def test_serialize_remarks_with_content(self) -> None:
        r"""测试序列化有内容的 remarks"""
        trace = Trace(
            meta=Meta(created_at="2025-12-23T00:00:00Z"),
            package=Package(),
            remarks=Remarks(title="Test", author="Author", comment="Comment"),
            object=Object(kind="stack"),
            states=[],
            steps=[]
        )
        writer = TomlWriter(trace)
        lines = writer._serialize_remarks()
        assert "[remarks]" in lines
        assert 'title = "Test"' in lines
        assert 'author = "Author"' in lines
        assert 'comment = "Comment"' in lines

    def test_serialize_object(self) -> None:
        r"""测试序列化 object 部分"""
        trace = Trace(
            meta=Meta(created_at="2025-12-23T00:00:00Z"),
            package=Package(),
            remarks=Remarks(),
            object=Object(kind="stack", label="my_stack"),
            states=[],
            steps=[]
        )
        writer = TomlWriter(trace)
        lines = writer._serialize_object()
        assert "[object]" in lines
        assert 'kind = "stack"' in lines
        assert 'label = "my_stack"' in lines

    def test_serialize_state(self) -> None:
        r"""测试序列化 state"""
        trace = Trace(
            meta=Meta(created_at="2025-12-23T00:00:00Z"),
            package=Package(),
            remarks=Remarks(),
            object=Object(kind="stack"),
            states=[],
            steps=[]
        )
        writer = TomlWriter(trace)
        state = State(id=0, data={"items": [1, 2, 3], "top": 2})
        lines = writer._serialize_state(state)
        assert "[[states]]" in lines
        assert "id = 0" in lines
        assert "[states.data]" in lines

    def test_serialize_step_full(self) -> None:
        r"""测试序列化完整 step"""
        trace = Trace(
            meta=Meta(created_at="2025-12-23T00:00:00Z"),
            package=Package(),
            remarks=Remarks(),
            object=Object(kind="stack"),
            states=[],
            steps=[]
        )
        writer = TomlWriter(trace)
        step = Step(
            id=0,
            op="push",
            before=0,
            after=1,
            args={"value": 10},
            code=CodeLocation(line=5, col=1),
            ret=10,
            note="test note"
        )
        lines = writer._serialize_step(step)
        assert "[[steps]]" in lines
        assert "id = 0" in lines
        assert 'op = "push"' in lines
        assert "before = 0" in lines
        assert "after = 1" in lines
        assert "ret = 10" in lines
        assert 'note = "test note"' in lines
        assert "[steps.args]" in lines
        assert "value = 10" in lines
        assert "[steps.code]" in lines
        assert "line = 5" in lines
        assert "col = 1" in lines

    def test_serialize_step_minimal(self) -> None:
        r"""测试序列化最小 step"""
        trace = Trace(
            meta=Meta(created_at="2025-12-23T00:00:00Z"),
            package=Package(),
            remarks=Remarks(),
            object=Object(kind="stack"),
            states=[],
            steps=[]
        )
        writer = TomlWriter(trace)
        step = Step(id=0, op="clear", before=0, args={})
        lines = writer._serialize_step(step)
        assert "[[steps]]" in lines
        assert "after" not in "\n".join(lines)
        assert "ret" not in "\n".join(lines)
        assert "note" not in "\n".join(lines)

    def test_serialize_result(self) -> None:
        r"""测试序列化 result"""
        trace = Trace(
            meta=Meta(created_at="2025-12-23T00:00:00Z"),
            package=Package(),
            remarks=Remarks(),
            object=Object(kind="stack"),
            states=[],
            steps=[],
            result=Result(final_state=5, commit=Commit(op="commit", line=10))
        )
        writer = TomlWriter(trace)
        lines = writer._serialize_result()
        assert "[result]" in lines
        assert "final_state = 5" in lines
        assert "[result.commit]" in lines
        assert 'op = "commit"' in lines
        assert "line = 10" in lines

    def test_serialize_error(self) -> None:
        r"""测试序列化 error"""
        trace = Trace(
            meta=Meta(created_at="2025-12-23T00:00:00Z"),
            package=Package(),
            remarks=Remarks(),
            object=Object(kind="stack"),
            states=[],
            steps=[],
            error=Error(
                type="runtime",
                message="test error",
                line=20,
                step=1,
                last_state=0
            )
        )
        writer = TomlWriter(trace)
        lines = writer._serialize_error()
        assert "[error]" in lines
        assert 'type = "runtime"' in lines
        assert 'message = "test error"' in lines
        assert "line = 20" in lines
        assert "step = 1" in lines
        assert "last_state = 0" in lines


class TestTomlWriterFullSerialization:
    r"""TomlWriter 完整序列化测试"""

    def test_full_trace_serialization(self) -> None:
        r"""测试完整 trace 序列化"""
        trace = Trace(
            meta=Meta(created_at="2025-12-23T00:00:00Z", lang="python", lang_version="3.12.0"),
            package=Package(),
            remarks=Remarks(title="Test", author="Tester"),
            object=Object(kind="stack", label="test_stack"),
            states=[
                State(id=0, data={"items": [], "top": -1}),
                State(id=1, data={"items": [10], "top": 0}),
            ],
            steps=[
                Step(id=0, op="push", before=0, after=1, args={"value": 10}, code=CodeLocation(line=5))
            ],
            result=Result(final_state=1, commit=Commit(op="commit", line=10))
        )
        writer = TomlWriter(trace)
        content = writer._serialize()

        # 验证各部分存在
        assert "[meta]" in content
        assert "[package]" in content
        assert "[remarks]" in content
        assert "[object]" in content
        assert "[[states]]" in content
        assert "[[steps]]" in content
        assert "[result]" in content
        assert "[error]" not in content

    def test_trace_with_error_serialization(self) -> None:
        r"""测试带错误的 trace 序列化"""
        trace = Trace(
            meta=Meta(created_at="2025-12-23T00:00:00Z", lang="python", lang_version="3.12.0"),
            package=Package(),
            remarks=Remarks(),
            object=Object(kind="stack"),
            states=[State(id=0, data={"items": [], "top": -1})],
            steps=[],
            error=Error(type="runtime", message="Error occurred", line=5)
        )
        writer = TomlWriter(trace)
        content = writer._serialize()

        assert "[error]" in content
        assert "[result]" not in content


class TestValueTypeSerialization:
    r"""值类型序列化测试"""

    def test_mixed_value_types_in_items(self) -> None:
        r"""测试 items 中混合值类型"""
        trace = Trace(
            meta=Meta(created_at="2025-12-23T00:00:00Z"),
            package=Package(),
            remarks=Remarks(),
            object=Object(kind="stack"),
            states=[],
            steps=[]
        )
        writer = TomlWriter(trace)

        # 整数
        assert writer._serialize_simple_array([1, 2, 3]) == "[1, 2, 3]"

        # 浮点数
        assert writer._serialize_simple_array([1.5, 2.5]) == "[1.5, 2.5]"

        # 字符串
        assert writer._serialize_simple_array(["a", "b"]) == '["a", "b"]'

        # 布尔值
        assert writer._serialize_simple_array([True, False, True]) == "[true, false, true]"

    def test_negative_numbers(self) -> None:
        r"""测试负数序列化"""
        trace = Trace(
            meta=Meta(created_at="2025-12-23T00:00:00Z"),
            package=Package(),
            remarks=Remarks(),
            object=Object(kind="stack"),
            states=[],
            steps=[]
        )
        writer = TomlWriter(trace)

        assert writer._serialize_value(-1) == "-1"
        assert writer._serialize_value(-3.14) == "-3.14"

    def test_special_float_values(self) -> None:
        r"""测试特殊浮点数"""
        trace = Trace(
            meta=Meta(created_at="2025-12-23T00:00:00Z"),
            package=Package(),
            remarks=Remarks(),
            object=Object(kind="stack"),
            states=[],
            steps=[]
        )
        writer = TomlWriter(trace)

        assert writer._serialize_value(0.0) == "0.0"
        assert writer._serialize_value(1e10) == "10000000000.0"
