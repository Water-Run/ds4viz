r"""
自定义异常类型定义

:file: ds4viz/exception.py
:author: WaterRun
:time: 2025-12-23
"""

from enum import Enum


class ErrorType(Enum):
    r"""
    错误类型枚举，对应 IR 定义中的 error.type
    """
    RUNTIME = "runtime"
    TIMEOUT = "timeout"
    VALIDATION = "validation"
    SANDBOX = "sandbox"
    UNKNOWN = "unknown"


class DS4VizError(Exception):
    r"""
    ds4viz 库的基础异常类
    """

    def __init__(
        self,
        message: str,
        error_type: ErrorType = ErrorType.RUNTIME,
        line: int | None = None
    ) -> None:
        r"""
        初始化异常

        :param message: 错误消息
        :param error_type: 错误类型
        :param line: 发生错误的代码行号
        :return None: 无返回值
        """
        super().__init__(message)
        self.message: str = message
        self.error_type: ErrorType = error_type
        self.line: int | None = line


class StructureError(DS4VizError):
    r"""
    数据结构操作错误
    """

    def __init__(self, message: str, line: int | None = None) -> None:
        r"""
        初始化数据结构错误

        :param message: 错误消息
        :param line: 发生错误的代码行号
        :return None: 无返回值
        """
        super().__init__(message, ErrorType.RUNTIME, line)


class ValidationError(DS4VizError):
    r"""
    数据验证错误
    """

    def __init__(self, message: str, line: int | None = None) -> None:
        r"""
        初始化验证错误

        :param message: 错误消息
        :param line: 发生错误的代码行号
        :return None: 无返回值
        """
        super().__init__(message, ErrorType.VALIDATION, line)
