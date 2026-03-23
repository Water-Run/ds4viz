r"""
数据结构基类定义

:file: ds4viz/structures/_base.py
:author: WaterRun
:time: 2026-03-23
"""

import re
from abc import ABC, abstractmethod
from types import TracebackType
from typing import Any

from ds4viz.session import ContextManager, get_caller_line
from ds4viz.exception import StructureError

ValueType = int | float | str | bool

_ALIAS_RE: re.Pattern[str] = re.compile(r"^[a-zA-Z0-9_-]+$")


class PhaseContext:
    r"""
    阶段上下文管理器
    """

    def __init__(self, structure: "BaseStructure", name: str) -> None:
        r"""
        初始化阶段上下文

        :param structure: 所属数据结构实例
        :param name: 阶段名称
        :return None: 无返回值
        """
        self._structure: BaseStructure = structure
        self._name: str = name

    def __enter__(self) -> "PhaseContext":
        r"""
        进入阶段上下文

        :return PhaseContext: 自身实例
        """
        self._structure._phase_stack.append(self._name)
        return self

    def __exit__(
        self,
        exc_type: type[BaseException] | None,
        exc_val: BaseException | None,
        exc_tb: TracebackType | None
    ) -> bool:
        r"""
        退出阶段上下文

        :param exc_type: 异常类型
        :param exc_val: 异常值
        :param exc_tb: 异常追踪信息
        :return bool: 不抑制异常
        """
        self._structure._phase_stack.pop()
        return False


class BaseStructure(ContextManager, ABC):
    r"""
    数据结构抽象基类
    """

    def __init__(
        self,
        kind: str,
        label: str,
        output: str | None = None
    ) -> None:
        r"""
        初始化数据结构

        :param kind: 数据结构类型
        :param label: 数据结构标签
        :param output: 输出文件路径
        :return None: 无返回值
        """
        super().__init__(kind, label, output)
        self._aliases: dict[int, str] = {}
        self._phase_stack: list[str] = []

    @property
    def _current_phase(self) -> str | None:
        r"""
        获取当前阶段名称

        :return str | None: 当前阶段名称, 无阶段时为 None
        """
        return self._phase_stack[-1] if self._phase_stack else None

    @abstractmethod
    def _build_data(self) -> dict[str, Any]:
        r"""
        构建当前状态的数据字典

        :return dict[str, Any]: 状态数据字典
        """
        ...

    def _node_exists(self, node_id: int) -> bool:
        r"""
        检查节点是否存在, 非节点结构默认返回 False

        :param node_id: 节点 ID
        :return bool: 节点是否存在
        """
        return False

    def _initialize(self) -> None:
        r"""
        初始化数据结构, 添加初始状态

        :return None: 无返回值
        """
        self._session.add_state(self._build_data())

    def _record_step(self, op: str, args: dict[str, Any], ret: Any = None) -> None:
        r"""
        记录操作步骤并添加新状态

        :param op: 操作名称
        :param args: 操作参数
        :param ret: 返回值
        :return None: 无返回值
        """
        before: int = self._session.get_last_state_id()
        line: int = get_caller_line(3)
        after: int = self._session.add_state(self._build_data())
        self._session.add_step(
            op=op,
            before=before,
            after=after,
            args=args,
            ret=ret,
            line=line,
            phase=self._current_phase
        )

    def _raise_error(self, message: str) -> None:
        r"""
        抛出数据结构错误

        :param message: 错误消息
        :return None: 无返回值
        :raise StructureError: 始终抛出
        """
        line: int = get_caller_line(3)
        step_id: int = self._session.step_counter
        self._session._failed_step_id = step_id
        raise StructureError(message, line)

    def step(
        self,
        note: str | None = None,
        highlights: list[dict[str, Any]] | None = None
    ) -> None:
        r"""
        记录一次观察步骤, 不改变数据结构的状态, 但会创建新的状态快照
        以反映别名等展示属性的变化

        :param note: 步骤说明, 长度 1..256
        :param highlights: 高亮标记列表
        :return None: 无返回值
        """
        before: int = self._session.get_last_state_id()
        line: int = get_caller_line(2)
        after: int = self._session.add_state(self._build_data())
        self._session.add_step(
            op="observe",
            before=before,
            after=after,
            args={},
            line=line,
            note=note,
            phase=self._current_phase,
            highlights=highlights
        )

    def amend(
        self,
        note: str | None = None,
        highlights: list[dict[str, Any]] | None = None
    ) -> None:
        r"""
        修改上一步的展示属性, 传入 None 的参数不会修改对应字段

        :param note: 步骤说明, 传入 None 表示不修改
        :param highlights: 高亮标记列表, 传入 None 表示不修改, 传入 [] 表示清除
        :return None: 无返回值
        :raise RuntimeError: 当不存在上一步时抛出异常
        """
        from ds4viz.trace import Step
        last_step: Step | None = self._session.get_last_step()
        if last_step is None:
            raise RuntimeError("No previous step to amend")
        if note is not None:
            last_step.note = note
        if highlights is not None:
            last_step.highlights = highlights

    def phase(self, name: str) -> PhaseContext:
        r"""
        返回阶段上下文管理器

        :param name: 阶段名称, 长度 1..64
        :return PhaseContext: 阶段上下文管理器
        """
        return PhaseContext(self, name)

    def alias(self, node_id: int, name: str | None) -> None:
        r"""
        设置或清除节点别名, 不产生新的步骤

        :param node_id: 节点 id
        :param name: 别名, 传入 None 表示清除
        :return None: 无返回值
        :raise RuntimeError: 当节点不存在时抛出异常
        """
        if not self._node_exists(node_id):
            self._raise_error(f"Node not found: {node_id}")
        if name is None:
            self._aliases.pop(node_id, None)
        else:
            if not (1 <= len(name) <= 64):
                self._raise_error(
                    f"Alias length must be 1-64, got {len(name)}")
            if not _ALIAS_RE.match(name):
                self._raise_error(f"Alias contains invalid characters: {name}")
            for nid, existing in self._aliases.items():
                if nid != node_id and existing == name:
                    self._raise_error(f"Alias already in use: {name}")
            self._aliases[node_id] = name
