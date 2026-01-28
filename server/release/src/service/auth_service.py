r"""
鉴权服务，处理JWT生成验证与会话管理

:file: src/service/auth_service.py
:author: WaterRun
:time: 2026-01-28
"""

import secrets
from datetime import datetime, timedelta

from database import Database
from utils import get_timestamp


def create_session(user_id: int, ip_address: str, expires_hours: int = 24) -> tuple[str, datetime]:
    r"""
    创建用户会话

    :param user_id: 用户ID
    :param ip_address: 客户端IP地址
    :param expires_hours: 会话有效期（小时）
    :return tuple[str, datetime]: (会话令牌, 过期时间)
    """
    token: str = secrets.token_urlsafe(64)
    expires_at: datetime = get_timestamp() + timedelta(hours=expires_hours)

    with Database.get_connection() as conn:
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

    :param token: 会话令牌
    :return int | None: 有效则返回user_id，无效或过期返回None
    """
    with Database.get_connection() as conn:
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

            user_id, expires_at = row
            if expires_at < get_timestamp():
                _delete_session(cur, token)
                conn.commit()
                return None

            return user_id


def invalidate_session(token: str) -> bool:
    r"""
    注销指定会话

    :param token: 会话令牌
    :return bool: 是否成功注销（会话存在则为True）
    """
    with Database.get_connection() as conn:
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
    with Database.get_connection() as conn:
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
    with Database.get_connection() as conn:
        with conn.cursor() as cur:
            cur.execute(
                """
                DELETE FROM sessions WHERE expires_at < %s
                """,
                (get_timestamp(),),
            )
            deleted: int = cur.rowcount
        conn.commit()

    return deleted


def _delete_session(cur, token: str) -> None:
    r"""
    内部函数：删除指定会话（使用现有游标）

    :param cur: 数据库游标
    :param token: 会话令牌
    :return None: 无返回值
    """
    cur.execute(
        """
        DELETE FROM sessions WHERE token = %s
        """,
        (token,),
    )
