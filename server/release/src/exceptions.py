r"""
自定义异常类型

:file: src/exceptions.py
:author: WaterRun
:time: 2026-01-31
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


# ============================================
# 通用异常
# ============================================


class ValidationError(Ds4VizException):
    r"""
    请求参数校验失败
    """

    status_code = 400
    message = "参数校验失败"


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


class ConflictError(Ds4VizException):
    r"""
    资源冲突
    """

    status_code = 409
    message = "资源冲突"


class RequestTimeoutError(Ds4VizException):
    r"""
    请求超时
    """

    status_code = 408
    message = "请求超时"


# ============================================
# 用户相关异常
# ============================================


class UserNotFoundError(NotFoundError):
    r"""
    用户不存在
    """

    message = "用户不存在"


class UserAlreadyExistsError(ConflictError):
    r"""
    用户已存在
    """

    message = "用户名已存在"


class InvalidCredentialsError(AuthenticationError):
    r"""
    凭证无效，用户名或密码错误
    """

    message = "用户名或密码错误"


class UserBannedError(AuthorizationError):
    r"""
    用户已被封禁
    """

    message = "用户已被封禁"


class PasswordMismatchError(ValidationError):
    r"""
    密码不匹配
    """

    message = "原密码错误"
    

class PasswordTooLongError(ValueError, ValidationError):
    r"""
    密码过长（UTF-8字节长度超限）
    """

    message = "密码长度不能超过64字节"

    def __init__(self, message: str | None = None) -> None:
        r"""
        初始化异常

        :param message: 错误信息
        """
        final_message: str = message or self.__class__.message
        ValueError.__init__(self, final_message)
        self.message = final_message


# ============================================
# 模板相关异常
# ============================================


class TemplateNotFoundError(NotFoundError):
    r"""
    模板不存在
    """

    message = "模板不存在"


class TemplateCodeNotFoundError(NotFoundError):
    r"""
    模板语言实现不存在
    """

    message = "模板语言实现不存在"


# ============================================
# 收藏相关异常
# ============================================


class FavoriteAlreadyExistsError(ConflictError):
    r"""
    收藏已存在
    """

    message = "已收藏该模板"


class FavoriteNotFoundError(NotFoundError):
    r"""
    收藏不存在
    """

    message = "未收藏该模板"


# ============================================
# 执行相关异常
# ============================================


class ExecutionError(Ds4VizException):
    r"""
    代码执行失败
    """

    status_code = 500
    message = "代码执行失败"


class ExecutionNotFoundError(NotFoundError):
    r"""
    执行记录不存在
    """

    message = "执行记录不存在"


class ExecutionPermissionError(AuthorizationError):
    r"""
    执行记录访问权限不足
    """

    message = "无权访问该执行记录"


class ExecutionTimeoutError(RequestTimeoutError):
    r"""
    代码执行超时
    """

    message = "代码执行超时"
