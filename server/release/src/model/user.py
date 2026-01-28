r"""
用户相关模型定义

:file: src/model/user.py
:author: WaterRun
:time: 2026-01-28
"""

from datetime import datetime
from enum import StrEnum
from dataclasses import dataclass
from pydantic import BaseModel, Field


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
    password: str = Field(min_length=6, max_length=128, description="密码")


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

    token: str = Field(description="JWT令牌")
    user: UserResponse = Field(description="用户信息")
    expires_at: datetime = Field(description="令牌过期时间")


class PasswordChange(BaseModel):
    r"""
    修改密码请求体
    """

    old_password: str = Field(description="原密码")
    new_password: str = Field(min_length=6, max_length=128, description="新密码")


class FavoriteItem(BaseModel):
    r"""
    收藏项响应
    """

    id: int = Field(description="模板ID")
    title: str = Field(description="模板标题")
    category: str = Field(description="模板分类")
    favorited_at: datetime = Field(description="收藏时间")


class FavoriteListResponse(BaseModel):
    r"""
    用户收藏列表响应
    """

    favorites: list[FavoriteItem] = Field(description="收藏列表")

@dataclass
class FavoriteItem:
    r"""
    收藏项
    """
    template_id: int
    title: str
    category: str
    description: str
    favorite_count: int
    favorited_at: datetime


@dataclass
class FavoriteListResponse:
    r"""
    收藏列表响应
    """
    items: list[FavoriteItem]
    total: int
    page: int
    limit: int


@dataclass
class PasswordChange:
    r"""
    密码修改请求
    """
    old_password: str
    new_password: str
