r"""
用户相关模型定义

:file: src/model/user.py
:author: WaterRun
:time: 2026-02-28
"""

import re
from datetime import datetime
from enum import StrEnum

from pydantic import BaseModel, Field, field_validator

# C# 标识符规则：以字母或下划线开头，后跟字母、数字、下划线（支持 Unicode 字母），不超过 64 字符
_USERNAME_PATTERN: re.Pattern[str] = re.compile(
    r'^[^\W\d]\w{0,63}$', re.UNICODE)

# 密码允许的特殊字符（全部可打印 ASCII 非字母数字字符）
_SPECIAL_CHARS: frozenset[str] = frozenset(
    '!"#$%&\'()*+,-./:;<=>?@[\\]^_`{|}~')


def _validate_username_format(username: str) -> str:
    r"""
    校验用户名符合 C# 标识符规则，且不超过 64 个字符

    规则：以字母或下划线开头，后续字符仅含字母、数字、下划线；
    支持 Unicode 字母（如中文），不允许连字符、空格、emoji 等。

    :param username: 用户名
    :return str: 原始用户名
    :raise ValueError: 格式不合法
    """
    if not _USERNAME_PATTERN.match(username):
        raise ValueError(
            "用户名须以字母或下划线开头，仅含字母、数字、下划线，且不超过64个字符"
        )
    return username


def _validate_password_strength(password: str) -> str:
    r"""
    校验密码强度：8-32 位，须同时包含大写字母、小写字母、数字和特殊字符

    :param password: 明文密码
    :return str: 原始密码
    :raise ValueError: 不满足强度要求
    """
    if not (8 <= len(password) <= 32):
        raise ValueError("密码长度须在8到32个字符之间")
    if not any(c.isupper() for c in password):
        raise ValueError("密码须包含至少一个大写字母")
    if not any(c.islower() for c in password):
        raise ValueError("密码须包含至少一个小写字母")
    if not any(c.isdigit() for c in password):
        raise ValueError("密码须包含至少一个数字")
    if not any(c in _SPECIAL_CHARS for c in password):
        raise ValueError("密码须包含至少一个特殊字符")
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

    username: str = Field(min_length=1, max_length=64, description="用户名")
    password: str = Field(min_length=8, max_length=32, description="密码")

    @field_validator("username")
    @classmethod
    def validate_username(cls, value: str) -> str:
        r"""
        校验用户名格式（C# 标识符规则）

        :param value: 用户名
        :return str: 校验后的用户名
        """
        return _validate_username_format(value)

    @field_validator("password")
    @classmethod
    def validate_password(cls, value: str) -> str:
        r"""
        校验密码强度

        :param value: 密码
        :return str: 校验后的密码
        """
        return _validate_password_strength(value)


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
    new_password: str = Field(min_length=8, max_length=32, description="新密码")

    @field_validator("new_password")
    @classmethod
    def validate_new_password(cls, value: str) -> str:
        r"""
        校验新密码强度

        :param value: 新密码
        :return str: 校验后的新密码
        """
        return _validate_password_strength(value)


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
