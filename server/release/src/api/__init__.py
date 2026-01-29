r"""
API层模块，提供HTTP接口

:file: src/api/__init__.py
:author: WaterRun
:time: 2026-01-29
"""

from fastapi import Request

from exceptions import AuthenticationError
from service import verify_session


def _extract_token(request: Request) -> str | None:
    r"""
    从请求头中提取Bearer令牌

    :param request: FastAPI请求对象
    :return str | None: 令牌字符串，不存在或格式错误返回None
    """
    auth_header: str | None = request.headers.get("Authorization")
    if auth_header is None:
        return None

    parts: list[str] = auth_header.split()
    if len(parts) != 2 or parts[0].lower() != "bearer":
        return None

    return parts[1]


def get_current_user(request: Request) -> int:
    r"""
    获取当前认证用户ID（必须认证）

    :param request: FastAPI请求对象
    :return int: 用户ID
    :raise AuthenticationError: 认证失败
    """
    token: str | None = _extract_token(request)
    if token is None:
        raise AuthenticationError("认证失败")

    user_id: int | None = verify_session(token)
    if user_id is None:
        raise AuthenticationError("认证失败")

    return user_id


def get_optional_user(request: Request) -> int | None:
    r"""
    获取当前认证用户ID（可选认证）

    :param request: FastAPI请求对象
    :return int | None: 用户ID，未认证返回None
    """
    token: str | None = _extract_token(request)
    if token is None:
        return None

    return verify_session(token)


def get_token(request: Request) -> str:
    r"""
    获取当前请求的令牌（必须存在）

    :param request: FastAPI请求对象
    :return str: 令牌字符串
    :raise AuthenticationError: 令牌不存在
    """
    token: str | None = _extract_token(request)
    if token is None:
        raise AuthenticationError("认证失败")

    return token
