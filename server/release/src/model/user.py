r"""
用户相关模型定义

:file: src/model/user.py
:author: WaterRun
:time: 2026-01-31
"""

from datetime import datetime
from enum import StrEnum

from pydantic import BaseModel, Field, field_validator

from exceptions import PasswordTooLongError


def _validate_password_bytes(password: str) -> str:
    r"""
    校验密码UTF-8字节长度不超过64

    :param password: 明文密码
    :return str: 原始密码
    :raise PasswordTooLongError: 密码过长
    """
    if len(password.encode("utf-8")) > 64:
        raise PasswordTooLongError("密码长度不能超过64字节")
    return password


class UserStatus(StrEnum):
    r"""
    用户状态枚举
    """

    ACTIVE = "Active"
    BANNED = "Banned"
    SUSPENDED = "Suspended"


class UserCreate(BaseModel):
    r"""
    用户注册请求体
    """

    username: str = Field(min_length=3, max_length=32, description="用户名")
    password: str = Field(min_length=1, max_length=64, description="密码")

    @field_validator("password")
    @classmethod
    def validate_password(cls, value: str) -> str:
        r"""
        校验密码长度

        :param value: 密码
        :return str: 校验后的密码
        """
        return _validate_password_bytes(value)


class UserLogin(BaseModel):
    r"""
    用户登录请求体
    """

    username: str = Field(description="用户名")
    password: str = Field(description="密码")


class UserResponse(BaseModel):
    r"""
    用户信息响应
    """

    id: int = Field(description="用户ID")
    username: str = Field(description="用户名")
    avatar_url: str | None = Field(default=None, description="头像URL")
    status: UserStatus = Field(description="用户状态")
    created_at: datetime = Field(description="创建时间")


class UserWithToken(BaseModel):
    r"""
    登录成功响应，包含令牌和用户信息
    """

    token: str = Field(description="会话令牌")
    user: UserResponse = Field(description="用户信息")
    expires_at: datetime = Field(description="令牌过期时间")


class PasswordChange(BaseModel):
    r"""
    修改密码请求体
    """

    old_password: str = Field(description="原密码")
    new_password: str = Field(min_length=1, max_length=64, description="新密码")

    @field_validator("new_password")
    @classmethod
    def validate_new_password(cls, value: str) -> str:
        r"""
        校验新密码长度

        :param value: 新密码
        :return str: 校验后的新密码
        """
        return _validate_password_bytes(value)


class FavoriteItem(BaseModel):
    r"""
    收藏项响应
    """

    template_id: int = Field(description="模板ID")
    title: str = Field(description="模板标题")
    category: str = Field(description="模板分类")
    description: str = Field(description="模板描述")
    favorite_count: int = Field(description="收藏数")
    favorited_at: datetime = Field(description="收藏时间")


class FavoriteListResponse(BaseModel):
    r"""
    用户收藏列表响应
    """

    items: list[FavoriteItem] = Field(description="收藏列表")
    total: int = Field(description="总数")
    page: int = Field(description="当前页码")
    limit: int = Field(description="每页数量")
