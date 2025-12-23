r"""
Trace 数据结构定义，用于记录操作轨迹

:file: ds4viz/trace.py
:author: WaterRun
:time: 2025-12-23
"""

from dataclasses import dataclass, field
from typing import Any


@dataclass
class Meta:
    r"""
    元数据信息
    """
    created_at: str
    lang: str = "python"
    lang_version: str = ""


@dataclass
class Package:
    r"""
    包信息
    """
    name: str = "ds4viz"
    lang: str = "python"
    version: str = "1.0.0"


@dataclass
class Remarks:
    r"""
    备注信息
    """
    title: str = ""
    author: str = ""
    comment: str = ""

    def is_empty(self) -> bool:
        r"""
        检查备注是否为空

        :return bool: 如果所有字段都为空则返回 True
        """
        return not self.title and not self.author and not self.comment


@dataclass
class Object:
    r"""
    数据结构对象描述
    """
    kind: str
    label: str = ""


@dataclass
class State:
    r"""
    状态快照
    """
    id: int
    data: dict[str, Any] = field(default_factory=dict)


@dataclass
class CodeLocation:
    r"""
    代码位置信息
    """
    line: int
    col: int | None = None


@dataclass
class Step:
    r"""
    操作步骤
    """
    id: int
    op: str
    before: int
    after: int | None = None
    args: dict[str, Any] = field(default_factory=dict)
    code: CodeLocation | None = None
    ret: Any = None
    note: str | None = None


@dataclass
class Commit:
    r"""
    提交信息
    """
    op: str
    line: int


@dataclass
class Result:
    r"""
    成功结果
    """
    final_state: int
    commit: Commit | None = None


@dataclass
class Error:
    r"""
    错误信息
    """
    type: str
    message: str
    line: int | None = None
    step: int | None = None
    last_state: int | None = None


@dataclass
class Trace:
    r"""
    完整的操作轨迹记录
    """
    meta: Meta
    package: Package
    remarks: Remarks
    object: Object
    states: list[State] = field(default_factory=list)
    steps: list[Step] = field(default_factory=list)
    result: Result | None = None
    error: Error | None = None
