r"""
日志模块，日志写入数据库

:file: src/log.py
:author: WaterRun
:time: 2026-01-29
"""

import sys
import traceback
from contextvars import ContextVar
from dataclasses import dataclass
from datetime import datetime
from enum import IntEnum

from database import get_connection
from utils import get_utc_now

_request_context: ContextVar["RequestContext | None"] = ContextVar(
    "request_context", default=None
)


class LogLevel(IntEnum):
    r"""
    日志级别
    """

    DEBUG = 10
    INFO = 20
    WARNING = 30
    ERROR = 40


@dataclass
class RequestContext:
    r"""
    请求上下文信息
    """

    request_id: str | None = None
    user_id: int | None = None
    ip_address: str | None = None


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


def set_request_context(
    request_id: str | None = None,
    user_id: int | None = None,
    ip_address: str | None = None,
) -> None:
    r"""
    设置当前请求的上下文信息

    :param request_id: 请求追踪ID
    :param user_id: 用户ID
    :param ip_address: 客户端IP
    """
    ctx = RequestContext(
        request_id=request_id,
        user_id=user_id,
        ip_address=ip_address,
    )
    _request_context.set(ctx)


def clear_request_context() -> None:
    r"""
    清除当前请求的上下文信息
    """
    _request_context.set(None)


def get_request_context() -> RequestContext | None:
    r"""
    获取当前请求的上下文信息

    :return RequestContext | None: 上下文信息
    """
    return _request_context.get()


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

    def _log(
        self,
        level: LogLevel,
        message: str,
        user_id: int | None = None,
        ip_address: str | None = None,
        request_id: str | None = None,
    ) -> None:
        r"""
        记录日志

        :param level: 日志级别
        :param message: 日志内容
        :param user_id: 用户ID（可选，覆盖上下文）
        :param ip_address: IP地址（可选，覆盖上下文）
        :param request_id: 请求ID（可选，覆盖上下文）
        """
        if level < _min_level:
            return

        ctx: RequestContext | None = get_request_context()

        final_user_id: int | None = user_id if user_id is not None else (
            ctx.user_id if ctx else None)
        final_ip: str | None = ip_address if ip_address is not None else (
            ctx.ip_address if ctx else None)
        final_request_id: str | None = request_id if request_id is not None else (
            ctx.request_id if ctx else None)

        timestamp: datetime = get_utc_now()
        level_name: str = level.name

        context_parts: list[str] = []
        if final_request_id:
            context_parts.append(f"req={final_request_id}")
        if final_user_id:
            context_parts.append(f"user={final_user_id}")
        if final_ip:
            context_parts.append(f"ip={final_ip}")

        context_str: str = f" [{', '.join(context_parts)}]" if context_parts else ""

        print(
            f"[{timestamp.isoformat()}] [{level_name}] [{self._module}]{context_str} {message}",
            file=sys.stderr,
        )

        try:
            with get_connection() as conn:
                with conn.cursor() as cur:
                    cur.execute(
                        """
                        INSERT INTO logs (level, module, message, user_id, ip_address, request_id)
                        VALUES (%s, %s, %s, %s, %s, %s)
                        """,
                        (level_name, self._module, message,
                         final_user_id, final_ip, final_request_id),
                    )
                conn.commit()
        except Exception as db_err:
            print(
                f"[LOG ERROR] 写入数据库失败: {db_err!r}",
                file=sys.stderr,
            )

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

    def error(self, message: str, exc: BaseException | None = None) -> None:
        r"""
        记录ERROR级别日志

        :param message: 日志内容
        :param exc: 可选的异常对象，会附加堆栈信息
        """
        if exc is not None:
            tb: str = "".join(
                traceback.format_exception(type(exc), exc, exc.__traceback__)
            )
            message = f"{message}\n{tb}"
        self._log(LogLevel.ERROR, message)

    def with_context(
        self,
        user_id: int | None = None,
        ip_address: str | None = None,
        request_id: str | None = None,
    ) -> "BoundLogger":
        r"""
        创建绑定上下文的日志记录器

        :param user_id: 用户ID
        :param ip_address: IP地址
        :param request_id: 请求ID
        :return BoundLogger: 绑定上下文的日志记录器
        """
        return BoundLogger(
            logger=self,
            user_id=user_id,
            ip_address=ip_address,
            request_id=request_id,
        )


class BoundLogger:
    r"""
    绑定上下文的日志记录器
    """

    def __init__(
        self,
        logger: Logger,
        user_id: int | None = None,
        ip_address: str | None = None,
        request_id: str | None = None,
    ) -> None:
        r"""
        初始化绑定上下文的日志记录器

        :param logger: 原始日志记录器
        :param user_id: 用户ID
        :param ip_address: IP地址
        :param request_id: 请求ID
        """
        self._logger = logger
        self._user_id = user_id
        self._ip_address = ip_address
        self._request_id = request_id

    def debug(self, message: str) -> None:
        r"""
        记录DEBUG级别日志

        :param message: 日志内容
        """
        self._logger._log(
            LogLevel.DEBUG, message,
            user_id=self._user_id,
            ip_address=self._ip_address,
            request_id=self._request_id,
        )

    def info(self, message: str) -> None:
        r"""
        记录INFO级别日志

        :param message: 日志内容
        """
        self._logger._log(
            LogLevel.INFO, message,
            user_id=self._user_id,
            ip_address=self._ip_address,
            request_id=self._request_id,
        )

    def warning(self, message: str) -> None:
        r"""
        记录WARNING级别日志

        :param message: 日志内容
        """
        self._logger._log(
            LogLevel.WARNING, message,
            user_id=self._user_id,
            ip_address=self._ip_address,
            request_id=self._request_id,
        )

    def error(self, message: str, exc: BaseException | None = None) -> None:
        r"""
        记录ERROR级别日志

        :param message: 日志内容
        :param exc: 可选的异常对象
        """
        if exc is not None:
            tb: str = "".join(
                traceback.format_exception(type(exc), exc, exc.__traceback__)
            )
            message = f"{message}\n{tb}"
        self._logger._log(
            LogLevel.ERROR, message,
            user_id=self._user_id,
            ip_address=self._ip_address,
            request_id=self._request_id,
        )
