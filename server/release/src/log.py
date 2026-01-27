r"""
日志模块，日志写入数据库

:file: src/log.py
:author: WaterRun
:time: 2026-01-27
"""

import sys
import traceback
from datetime import UTC, datetime
from enum import IntEnum

from database import get_connection


class LogLevel(IntEnum):
    r"""
    日志级别
    """

    DEBUG = 10
    INFO = 20
    WARNING = 30
    ERROR = 40


_min_level: LogLevel = LogLevel.INFO
_initialized: bool = False


def init_logger(level: LogLevel = LogLevel.INFO) -> None:
    r"""
    初始化日志模块
    :param level: 最低日志级别
    """
    global _min_level, _initialized
    _min_level = level
    _initialized = True


def get_logger(module: str) -> "Logger":
    r"""
    获取绑定模块名的日志记录器
    :param module: 模块名
    :return Logger: 日志记录器实例
    :raise RuntimeError: 日志模块未初始化
    """
    if not _initialized:
        raise RuntimeError("日志模块未初始化")
    return Logger(module)


class Logger:
    r"""
    日志记录器
    """

    def __init__(self, module: str) -> None:
        r"""
        初始化日志记录器
        :param module: 模块名
        """
        self._module = module

    def _log(self, level: LogLevel, message: str) -> None:
        r"""
        记录日志
        :param level: 日志级别
        :param message: 日志内容
        """
        if level < _min_level:
            return
        timestamp = datetime.now(UTC)
        level_name = level.name
        # 控制台输出
        print(
            f"[{timestamp.isoformat()}] [{level_name}] [{self._module}] {message}",
            file=sys.stderr,
        )
        # 写入数据库
        try:
            with get_connection() as conn:
                with conn.cursor() as cur:
                    cur.execute(
                        "INSERT INTO logs (timestamp, level, module, message) VALUES (%s, %s, %s, %s)",
                        (timestamp, level_name, self._module, message),
                    )
                conn.commit()
        except Exception as e:
            print(f"[LOG ERROR] 写入数据库失败: {e}", file=sys.stderr)

    def debug(self, message: str) -> None:
        r"""
        记录DEBUG级别日志
        :param message: 日志内容
        """
        self._log(LogLevel.DEBUG, message)

    def info(self, message: str) -> None:
        r"""
        记录INFO级别日志
        :param message: 日志内容
        """
        self._log(LogLevel.INFO, message)

    def warning(self, message: str) -> None:
        r"""
        记录WARNING级别日志
        :param message: 日志内容
        """
        self._log(LogLevel.WARNING, message)

    def error(self, message: str, exc: Exception | None = None) -> None:
        r"""
        记录ERROR级别日志
        :param message: 日志内容
        :param exc: 可选的异常对象，会附加堆栈信息
        """
        if exc:
            tb = "".join(traceback.format_exception(type(exc), exc, exc.__traceback__))
            message = f"{message}\n{tb}"
        self._log(LogLevel.ERROR, message)
