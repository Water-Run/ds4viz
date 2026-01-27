r"""
自定义异常类型

:file: src/exceptions.py
:author: WaterRun
:time: 2026-01-27
"""


class Ds4VizException(Exception):
    r"""
    异常基类
    """

    status_code: int = 500
    message: str = "服务器内部错误"

    def __init__(self, message: str | None = None) -> None:
        r"""
        初始化异常
        :param message: 错误信息，None则使用默认值
        """
        self.message = message or self.__class__.message
        super().__init__(self.message)


class AuthenticationError(Ds4VizException):
    r"""
    认证失败，如token无效、密码错误
    """

    status_code = 401
    message = "认证失败"


class AuthorizationError(Ds4VizException):
    r"""
    权限不足
    """

    status_code = 403
    message = "权限不足"


class NotFoundError(Ds4VizException):
    r"""
    资源不存在
    """

    status_code = 404
    message = "资源不存在"


class ValidationError(Ds4VizException):
    r"""
    请求参数校验失败
    """

    status_code = 400
    message = "参数校验失败"


class ExecutionError(Ds4VizException):
    r"""
    代码执行失败
    """

    status_code = 500
    message = "代码执行失败"


class TimeoutError(Ds4VizException):
    r"""
    执行超时
    """

    status_code = 408
    message = "执行超时"
