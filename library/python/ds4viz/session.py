r"""
会话管理模块, 提供上下文管理和全局配置

:file: ds4viz/session.py
:author: WaterRun
:time: 2026-03-23
"""

import inspect
import sys
from dataclasses import dataclass
from datetime import datetime, timezone
from types import TracebackType
from typing import Any

from ds4viz.exception import DS4VizError, ErrorType
from ds4viz.trace import (
    Trace, Meta, Package, Remarks, Object,
    State, Step, CodeLocation, Result, Commit, Error
)
from ds4viz.writer import TomlWriter


@dataclass
class GlobalConfig:
    r"""
    全局配置
    """
    output_path: str = "trace.toml"
    title: str = ""
    author: str = ""
    comment: str = ""


_global_config: GlobalConfig = GlobalConfig()


def config(
    output_path: str = "trace.toml",
    title: str = "",
    author: str = "",
    comment: str = ""
) -> None:
    r"""
    配置全局参数

    :param output_path: 输出文件路径, 默认为 "trace.toml"
    :param title: 可视化标题
    :param author: 作者信息
    :param comment: 注释说明
    :return None: 无返回值
    """
    global _global_config
    _global_config = GlobalConfig(
        output_path=output_path,
        title=title,
        author=author,
        comment=comment
    )


def get_config() -> GlobalConfig:
    r"""
    获取全局配置

    :return GlobalConfig: 当前全局配置
    """
    return _global_config


def get_python_version() -> str:
    r"""
    获取当前 Python 版本字符串

    :return str: Python 版本字符串
    """
    return f"{sys.version_info.major}.{sys.version_info.minor}.{sys.version_info.micro}"


def get_caller_line(depth: int = 2) -> int:
    r"""
    获取调用者的代码行号

    :param depth: 调用栈深度
    :return int: 调用者所在行号
    """
    frame = inspect.currentframe()
    try:
        for _ in range(depth):
            if frame is not None:
                frame = frame.f_back
        return frame.f_lineno if frame is not None else 1
    finally:
        del frame


class Session:
    r"""
    会话管理器, 用于管理数据结构的操作轨迹记录
    """

    def __init__(
        self,
        kind: str,
        label: str,
        output: str | None = None
    ) -> None:
        r"""
        初始化会话

        :param kind: 数据结构类型
        :param label: 数据结构标签
        :param output: 输出文件路径, 若为 None 则使用全局配置
        :return None: 无返回值
        """
        self._kind: str = kind
        self._label: str = label
        self._output: str = output if output is not None else _global_config.output_path
        self._states: list[State] = []
        self._steps: list[Step] = []
        self._state_counter: int = 0
        self._step_counter: int = 0
        self._error: Error | None = None
        self._entry_line: int = 1
        self._exit_line: int = 1
        self._failed_step_id: int | None = None

    @property
    def failed_step_id(self) -> int | None:
        r"""
        获取失败步骤 ID

        :return int | None: 失败步骤 ID, 如果没有失败则为 None
        """
        return self._failed_step_id

    @property
    def step_counter(self) -> int:
        r"""
        获取当前步骤计数器

        :return int: 当前步骤计数
        """
        return self._step_counter

    def add_state(self, data: dict[str, Any]) -> int:
        r"""
        添加状态快照

        :param data: 状态数据
        :return int: 新状态的 ID
        """
        state_id: int = self._state_counter
        self._states.append(State(id=state_id, data=data))
        self._state_counter += 1
        return state_id

    def add_step(
        self,
        op: str,
        before: int,
        after: int | None,
        args: dict[str, Any],
        ret: Any = None,
        note: str | None = None,
        line: int | None = None,
        phase: str | None = None,
        highlights: list[dict[str, Any]] | None = None
    ) -> int:
        r"""
        添加操作步骤

        :param op: 操作名称
        :param before: 操作前状态 ID
        :param after: 操作后状态 ID, 失败时为 None
        :param args: 操作参数
        :param ret: 返回值
        :param note: 备注
        :param line: 代码行号
        :param phase: 阶段标记
        :param highlights: 高亮标记列表
        :return int: 新步骤的 ID
        """
        step_id: int = self._step_counter
        code_loc: CodeLocation | None = CodeLocation(
            line=line) if line is not None else None
        self._steps.append(Step(
            id=step_id,
            op=op,
            before=before,
            after=after,
            args=args,
            code=code_loc,
            ret=ret,
            note=note,
            phase=phase,
            highlights=highlights
        ))
        self._step_counter += 1
        return step_id

    def get_last_step(self) -> Step | None:
        r"""
        获取最后一个步骤

        :return Step | None: 最后一个步骤, 若无步骤则返回 None
        """
        return self._steps[-1] if self._steps else None

    def set_error(
        self,
        error_type: ErrorType,
        message: str,
        line: int | None = None,
        step_id: int | None = None
    ) -> None:
        r"""
        设置错误信息

        :param error_type: 错误类型
        :param message: 错误消息
        :param line: 错误发生的代码行号
        :param step_id: 发生错误的步骤 ID
        :return None: 无返回值
        """
        last_state: int | None = self._states[-1].id if self._states else None
        self._error = Error(
            type=error_type.value,
            message=message[:512],
            line=line,
            step=step_id,
            last_state=last_state
        )
        self._failed_step_id = step_id

    def set_entry_line(self, line: int) -> None:
        r"""
        设置上下文入口行号

        :param line: 入口行号
        :return None: 无返回值
        """
        self._entry_line = line

    def set_exit_line(self, line: int) -> None:
        r"""
        设置上下文退出行号

        :param line: 退出行号
        :return None: 无返回值
        """
        self._exit_line = line

    def get_last_state_id(self) -> int:
        r"""
        获取最后一个状态的 ID

        :return int: 最后一个状态的 ID, 若无状态则返回 -1
        """
        return self._states[-1].id if self._states else -1

    def build_trace(self) -> Trace:
        r"""
        构建完整的 Trace 对象

        :return Trace: 完整的 Trace 对象
        """
        now: str = datetime.now(timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ")
        meta = Meta(
            created_at=now,
            lang="python",
            lang_version=get_python_version()
        )
        package = Package()
        remarks = Remarks(
            title=_global_config.title,
            author=_global_config.author,
            comment=_global_config.comment
        )
        obj = Object(kind=self._kind, label=self._label)
        result: Result | None = None
        error: Error | None = None
        if self._error is not None:
            error = self._error
        else:
            final_state_id: int = self._states[-1].id if self._states else 0
            result = Result(
                final_state=final_state_id,
                commit=Commit(op="commit", line=self._exit_line)
            )
        return Trace(
            meta=meta,
            package=package,
            remarks=remarks,
            object=obj,
            states=self._states,
            steps=self._steps,
            result=result,
            error=error
        )

    def write(self) -> None:
        r"""
        将 Trace 写入文件

        :return None: 无返回值
        """
        trace: Trace = self.build_trace()
        writer = TomlWriter(trace)
        writer.write(self._output)


class ContextManager:
    r"""
    上下文管理器基类, 用于管理数据结构的生命周期
    """

    def __init__(
        self,
        kind: str,
        label: str,
        output: str | None = None
    ) -> None:
        r"""
        初始化上下文管理器

        :param kind: 数据结构类型
        :param label: 数据结构标签
        :param output: 输出文件路径
        :return None: 无返回值
        """
        self._session: Session = Session(kind, label, output)
        self._entered: bool = False

    @property
    def session(self) -> Session:
        r"""
        获取会话对象

        :return Session: 会话对象
        """
        return self._session

    @property
    def step_counter(self) -> int:
        r"""
        获取当前步骤计数器

        :return int: 当前步骤计数
        """
        return self._session.step_counter

    def __enter__(self) -> "ContextManager":
        r"""
        进入上下文

        :return ContextManager: 自身实例
        """
        self._entered = True
        self._session.set_entry_line(get_caller_line(2))
        self._initialize()
        return self

    def __exit__(
        self,
        exc_type: type[BaseException] | None,
        exc_val: BaseException | None,
        exc_tb: TracebackType | None
    ) -> bool:
        r"""
        退出上下文

        :param exc_type: 异常类型
        :param exc_val: 异常值
        :param exc_tb: 异常追踪信息
        :return bool: 是否抑制异常
        """
        self._session.set_exit_line(get_caller_line(2))
        if exc_val is not None:
            error_line: int | None = None
            if exc_tb is not None:
                error_line = exc_tb.tb_lineno
            error_type: ErrorType = ErrorType.UNKNOWN
            message: str = str(exc_val)
            if isinstance(exc_val, DS4VizError):
                error_type = exc_val.error_type
                message = exc_val.message
                if exc_val.line is not None:
                    error_line = exc_val.line
            self._session.set_error(
                error_type=error_type,
                message=message,
                line=error_line,
                step_id=self._session.failed_step_id
            )
        self._session.write()
        return False

    def _initialize(self) -> None:
        r"""
        初始化数据结构, 子类需要重写此方法添加初始状态

        :return None: 无返回值
        """
        ...
