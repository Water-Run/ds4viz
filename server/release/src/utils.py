r"""
纯工具函数，无项目内部依赖

:file: src/utils.py
:author: WaterRun
:time: 2026-01-27
"""

import hashlib
from datetime import UTC, datetime, timedelta

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
    salt = bcrypt.gensalt(rounds=rounds)
    return bcrypt.hashpw(password.encode(), salt).decode()


def verify_password(password: str, hashed: str) -> bool:
    r"""
    验证密码是否匹配
    :param password: 明文密码
    :param hashed: 已哈希的密码
    :return bool: 是否匹配
    """
    return bcrypt.checkpw(password.encode(), hashed.encode())


def sha256_hash(content: str) -> str:
    r"""
    计算字符串的SHA256哈希，用于代码缓存
    :param content: 待哈希内容
    :return str: 十六进制哈希字符串
    """
    return hashlib.sha256(content.encode()).hexdigest()


def create_jwt(
    payload: dict,
    secret: str,
    algorithm: str = "HS256",
    expires_delta: timedelta | None = None,
) -> str:
    r"""
    创建JWT令牌
    :param payload: 载荷数据
    :param secret: 签名密钥
    :param algorithm: 签名算法
    :param expires_delta: 过期时间增量，None则不设过期
    :return str: JWT字符串
    """
    to_encode = payload.copy()
    if expires_delta:
        to_encode["exp"] = datetime.now(UTC) + expires_delta
    return jwt.encode(to_encode, secret, algorithm=algorithm)


def decode_jwt(token: str, secret: str, algorithm: str = "HS256") -> dict:
    r"""
    解析JWT令牌
    :param token: JWT字符串
    :param secret: 签名密钥
    :param algorithm: 签名算法
    :return dict: 载荷数据
    :raise jwt.InvalidTokenError: token无效或过期
    """
    return jwt.decode(token, secret, algorithms=[algorithm])


def utc_now() -> datetime:
    r"""
    获取当前UTC时间
    :return datetime: 带时区的UTC时间
    """
    return datetime.now(UTC)


def get_client_ip(request: Request) -> str:
    r"""
    从FastAPI请求中提取客户端IP
    :param request: FastAPI请求对象
    :return str: 客户端IP地址
    """
    forwarded = request.headers.get("X-Forwarded-For")
    if forwarded:
        return forwarded.split(",")[0].strip()
    return request.client.host if request.client else "unknown"
