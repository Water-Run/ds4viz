r"""
纯工具函数，无项目内部依赖

:file: src/utils.py
:author: WaterRun
:time: 2026-01-29
"""

import hashlib
import uuid
from datetime import UTC, datetime, timedelta
from typing import Any

import bcrypt
import jwt
from fastapi import Request


def hash_password(password: str, rounds: int = 12) -> str:
    r"""
    使用bcrypt哈希密码

    :param password: 明文密码
    :param rounds: bcrypt轮数
    :return str: 哈希后的密码字符串
    """
    salt: bytes = bcrypt.gensalt(rounds=rounds)
    hashed: bytes = bcrypt.hashpw(password.encode("utf-8"), salt)
    return hashed.decode("utf-8")


def verify_password(password: str, hashed: str) -> bool:
    r"""
    验证密码是否匹配

    :param password: 明文密码
    :param hashed: 已哈希的密码
    :return bool: 是否匹配
    """
    return bcrypt.checkpw(password.encode("utf-8"), hashed.encode("utf-8"))


def compute_hash(content: str) -> str:
    r"""
    计算字符串的SHA256哈希，用于代码缓存

    :param content: 待哈希内容
    :return str: 64位十六进制哈希字符串
    """
    return hashlib.sha256(content.encode("utf-8")).hexdigest()


def get_utc_now() -> datetime:
    r"""
    获取当前UTC时间（带时区）

    :return datetime: 带时区的UTC时间
    """
    return datetime.now(UTC)


def get_client_ip(request: Request) -> str:
    r"""
    从FastAPI请求中提取客户端IP

    :param request: FastAPI请求对象
    :return str: 客户端IP地址
    """
    forwarded: str | None = request.headers.get("X-Forwarded-For")
    if forwarded:
        return forwarded.split(",")[0].strip()
    return request.client.host if request.client else "unknown"


def generate_uuid() -> str:
    r"""
    生成唯一标识符

    :return str: UUID十六进制字符串
    """
    return uuid.uuid4().hex


def generate_request_id() -> str:
    r"""
    生成请求追踪ID

    :return str: 请求ID字符串
    """
    return uuid.uuid4().hex[:16]


def create_jwt_token(
    user_id: int,
    secret: str,
    algorithm: str = "HS256",
    expire_hours: int = 24,
) -> tuple[str, datetime]:
    r"""
    创建JWT令牌

    :param user_id: 用户ID
    :param secret: JWT密钥
    :param algorithm: 签名算法
    :param expire_hours: 过期时间（小时）
    :return tuple[str, datetime]: (JWT令牌, 过期时间UTC)
    """
    expires_at: datetime = get_utc_now() + timedelta(hours=expire_hours)
    payload: dict[str, Any] = {
        "sub": str(user_id),
        "exp": expires_at,
        "iat": get_utc_now(),
        "jti": generate_uuid(),
    }
    token: str = jwt.encode(payload, secret, algorithm=algorithm)
    return token, expires_at


def decode_jwt_token(
    token: str,
    secret: str,
    algorithm: str = "HS256",
) -> dict[str, Any] | None:
    r"""
    解码并验证JWT令牌

    :param token: JWT令牌
    :param secret: JWT密钥
    :param algorithm: 签名算法
    :return dict | None: 解码后的payload，验证失败返回None
    """
    try:
        payload: dict[str, Any] = jwt.decode(
            token, secret, algorithms=[algorithm])
        return payload
    except jwt.ExpiredSignatureError:
        return None
    except jwt.InvalidTokenError:
        return None
