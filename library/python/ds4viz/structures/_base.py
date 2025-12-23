r"""
数据结构基类定义

:file: ds4viz/structures/_base.py
:author: WaterRun
:time: 2025-12-23
"""

from abc import ABC, abstractmethod
from typing import Any

from ds4viz.session import ContextManager, get_caller_line
from ds4viz.exception import StructureError


ValueType = int | float | str | bool


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

    @abstractmethod
    def _build_data(self) -> dict[str, Any]:
        r"""
        构建当前状态的数据字典

        :return dict[str, Any]: 状态数据字典
        """
        ...

    def _initialize(self) -> None:
        r"""
        初始化数据结构，添加初始状态

        :return None: 无返回值
        """
        self._session.add_state(self._build_data())

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
