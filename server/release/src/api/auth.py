r"""
认证相关API

:file: src/api/auth.py
:author: WaterRun
:time: 2026-01-29
"""

from fastapi import APIRouter, Request, Depends

from api import get_current_user, get_token
from model import UserCreate, UserLogin, UserResponse, UserWithToken
from service import create_user, authenticate_user, get_user_by_id, invalidate_session
from utils import get_client_ip

router = APIRouter(prefix="/api/auth", tags=["认证"])


@router.post("/register", response_model=UserResponse)
def register(data: UserCreate) -> UserResponse:
    r"""
    用户注册

    :param data: 注册信息
    :return UserResponse: 创建的用户信息
    """
    return create_user(data)


@router.post("/login", response_model=UserWithToken)
def login(data: UserLogin, request: Request) -> UserWithToken:
    r"""
    用户登录

    :param data: 登录信息
    :param request: FastAPI请求对象
    :return UserWithToken: 包含令牌和用户信息的响应
    """
    ip_address: str = get_client_ip(request)
    return authenticate_user(data, ip_address)


@router.post("/logout")
def logout(
    token: str = Depends(get_token),
    _: int = Depends(get_current_user),
) -> dict[str, str]:
    r"""
    用户登出

    :param token: 当前会话令牌
    :param _: 当前用户ID（用于验证认证）
    :return dict: 登出成功消息
    """
    invalidate_session(token)
    return {"message": "Logged out successfully"}


@router.get("/me", response_model=UserResponse)
def get_me(user_id: int = Depends(get_current_user)) -> UserResponse:
    r"""
    获取当前用户信息

    :param user_id: 当前用户ID
    :return UserResponse: 用户信息
    """
    return get_user_by_id(user_id)
