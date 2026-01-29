r"""
鉴权服务，处理JWT生成验证与会话管理

:file: src/service/auth_service.py
:author: WaterRun
:time: 2026-01-29
"""

from datetime import datetime

from config import get_config
from database import get_connection
from utils import get_utc_now, create_jwt_token, decode_jwt_token


def create_session(user_id: int, ip_address: str, expires_hours: int = 24) -> tuple[str, datetime]:
    r"""
    创建用户会话

    :param user_id: 用户ID
    :param ip_address: 客户端IP地址
    :param expires_hours: 会话有效期（小时）
    :return tuple[str, datetime]: (JWT令牌, 过期时间UTC)
    """
    config = get_config()
    token, expires_at = create_jwt_token(
        user_id=user_id,
        secret=config.security.jwt_secret,
        algorithm=config.security.jwt_algorithm,
        expire_hours=expires_hours,
    )

    with get_connection() as conn:
        with conn.cursor() as cur:
            cur.execute(
                """
                INSERT INTO sessions (user_id, token, ip_address, expires_at)
                VALUES (%s, %s, %s, %s)
                """,
                (user_id, token, ip_address, expires_at),
            )
        conn.commit()

    return token, expires_at


def verify_session(token: str) -> int | None:
    r"""
    验证会话有效性

    :param token: JWT令牌
    :return int | None: 有效则返回user_id，无效或过期返回None
    """
    config = get_config()
    payload = decode_jwt_token(
        token=token,
        secret=config.security.jwt_secret,
        algorithm=config.security.jwt_algorithm,
    )

    if payload is None:
        return None

    user_id_str: str | None = payload.get("sub")
    if user_id_str is None:
        return None

    try:
        user_id: int = int(user_id_str)
    except ValueError:
        return None

    with get_connection() as conn:
        with conn.cursor() as cur:
            cur.execute(
                """
                SELECT user_id, expires_at FROM sessions
                WHERE token = %s
                """,
                (token,),
            )
            row: tuple[int, datetime] | None = cur.fetchone()

            if row is None:
                return None

            db_user_id, expires_at = row
            if db_user_id != user_id:
                return None

            now: datetime = get_utc_now()
            expires_at_aware: datetime = expires_at.replace(
                tzinfo=now.tzinfo) if expires_at.tzinfo is None else expires_at
            if expires_at_aware < now:
                _delete_session(cur, token)
                conn.commit()
                return None

            return user_id


def invalidate_session(token: str) -> bool:
    r"""
    注销指定会话

    :param token: JWT令牌
    :return bool: 是否成功注销（会话存在则为True）
    """
    with get_connection() as conn:
        with conn.cursor() as cur:
            cur.execute(
                """
                DELETE FROM sessions WHERE token = %s
                """,
                (token,),
            )
            deleted: int = cur.rowcount
        conn.commit()

    return deleted > 0


def invalidate_all_sessions(user_id: int) -> int:
    r"""
    注销用户的所有会话

    :param user_id: 用户ID
    :return int: 注销的会话数量
    """
    with get_connection() as conn:
        with conn.cursor() as cur:
            cur.execute(
                """
                DELETE FROM sessions WHERE user_id = %s
                """,
                (user_id,),
            )
            deleted: int = cur.rowcount
        conn.commit()

    return deleted


def cleanup_expired_sessions() -> int:
    r"""
    清理所有过期会话

    :return int: 清理的会话数量
    """
    with get_connection() as conn:
        with conn.cursor() as cur:
            cur.execute(
                """
                DELETE FROM sessions WHERE expires_at < %s
                """,
                (get_utc_now(),),
            )
            deleted: int = cur.rowcount
        conn.commit()

    return deleted


def _delete_session(cur, token: str) -> None:
    r"""
    内部函数：删除指定会话（使用现有游标）

    :param cur: 数据库游标
    :param token: JWT令牌
    """
    cur.execute(
        """
        DELETE FROM sessions WHERE token = %s
        """,
        (token,),
    )
