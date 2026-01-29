r"""
用户相关API

:file: src/api/user.py
:author: WaterRun
:time: 2026-01-29
"""

from fastapi import APIRouter, Depends, UploadFile, File, Query
from fastapi.responses import Response

from api import get_current_user
from exceptions import AuthorizationError, NotFoundError
from model import PasswordChange, FavoriteListResponse, ExecutionListResponse
from service import (
    get_avatar,
    update_avatar,
    change_password,
    get_favorites,
    list_executions,
)

router = APIRouter(prefix="/api/users", tags=["用户"])


@router.get("/{user_id}/avatar")
def get_user_avatar(user_id: int) -> Response:
    r"""
    获取用户头像

    :param user_id: 用户ID
    :return Response: 头像二进制数据
    """
    avatar_data: bytes | None = get_avatar(user_id)
    if avatar_data is None:
        raise NotFoundError("头像不存在")

    return Response(content=avatar_data, media_type="application/octet-stream")


@router.put("/{user_id}/avatar")
async def upload_user_avatar(
    user_id: int,
    avatar: UploadFile = File(...),
    current_user_id: int = Depends(get_current_user),
) -> dict[str, str]:
    r"""
    上传/更新用户头像

    :param user_id: 用户ID
    :param avatar: 头像文件
    :param current_user_id: 当前用户ID
    :return dict: 上传成功消息
    """
    if current_user_id != user_id:
        raise AuthorizationError("权限不足")

    avatar_data: bytes = await avatar.read()
    update_avatar(user_id, avatar_data)

    return {
        "message": "Avatar uploaded successfully",
        "avatar_url": f"/api/users/{user_id}/avatar",
    }


@router.put("/{user_id}/password")
def update_user_password(
    user_id: int,
    data: PasswordChange,
    current_user_id: int = Depends(get_current_user),
) -> dict[str, str]:
    r"""
    修改用户密码

    :param user_id: 用户ID
    :param data: 密码修改信息
    :param current_user_id: 当前用户ID
    :return dict: 修改成功消息
    """
    if current_user_id != user_id:
        raise AuthorizationError("权限不足")

    change_password(user_id, data)
    return {"message": "Password updated successfully"}


@router.get("/{user_id}/favorites", response_model=FavoriteListResponse)
def get_user_favorites(
    user_id: int,
    page: int = Query(default=1, ge=1),
    limit: int = Query(default=20, ge=1, le=100),
    current_user_id: int = Depends(get_current_user),
) -> FavoriteListResponse:
    r"""
    获取用户收藏列表

    :param user_id: 用户ID
    :param page: 页码
    :param limit: 每页数量
    :param current_user_id: 当前用户ID
    :return FavoriteListResponse: 收藏列表
    """
    if current_user_id != user_id:
        raise AuthorizationError("权限不足")

    return get_favorites(user_id, page, limit)


@router.get("/{user_id}/executions", response_model=ExecutionListResponse)
def get_user_executions(
    user_id: int,
    page: int = Query(default=1, ge=1),
    limit: int = Query(default=20, ge=1, le=100),
    current_user_id: int = Depends(get_current_user),
) -> ExecutionListResponse:
    r"""
    获取用户执行历史

    :param user_id: 用户ID
    :param page: 页码
    :param limit: 每页数量
    :param current_user_id: 当前用户ID
    :return ExecutionListResponse: 执行历史列表
    """
    if current_user_id != user_id:
        raise AuthorizationError("权限不足")

    return list_executions(user_id, page, limit)
