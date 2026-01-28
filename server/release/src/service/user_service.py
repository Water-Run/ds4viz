r"""
用户服务，处理用户注册、登录、信息管理与收藏

:file: src/service/user_service.py
:author: WaterRun
:time: 2026-01-28
"""

from database import Database
from model import (
    UserCreate,
    UserLogin,
    UserResponse,
    UserStatus,
    PasswordChange,
    FavoriteListResponse,
    FavoriteItem,
)
from service.auth_service import create_session
from utils import hash_password, verify_password, get_timestamp


class UserServiceError(Exception):
    r"""
    用户服务异常基类
    """
    ...


class UserNotFoundError(UserServiceError):
    r"""
    用户不存在异常
    """
    ...


class UserAlreadyExistsError(UserServiceError):
    r"""
    用户已存在异常
    """
    ...


class InvalidCredentialsError(UserServiceError):
    r"""
    凭证无效异常（用户名或密码错误）
    """
    ...


class UserBannedError(UserServiceError):
    r"""
    用户已被封禁异常
    """
    ...


class PasswordMismatchError(UserServiceError):
    r"""
    密码不匹配异常
    """
    ...


class TemplateNotFoundError(UserServiceError):
    r"""
    模板不存在异常
    """
    ...


class FavoriteAlreadyExistsError(UserServiceError):
    r"""
    收藏已存在异常
    """
    ...


class FavoriteNotFoundError(UserServiceError):
    r"""
    收藏不存在异常
    """
    ...


def create_user(data: UserCreate) -> UserResponse:
    r"""
    创建新用户

    :param data: 用户创建数据
    :return UserResponse: 创建的用户信息
    :raise UserAlreadyExistsError: 用户名已存在
    """
    password_hashed: str = hash_password(data.password)

    with Database.get_connection() as conn:
        with conn.cursor() as cur:
            cur.execute(
                """
                SELECT id FROM users WHERE username = %s
                """,
                (data.username,),
            )
            if cur.fetchone() is not None:
                raise UserAlreadyExistsError(f"用户名已存在: {data.username}")

            cur.execute(
                """
                INSERT INTO users (username, password_hash)
                VALUES (%s, %s)
                RETURNING id, created_at
                """,
                (data.username, password_hashed),
            )
            row: tuple[int, ...] = cur.fetchone()
            user_id, created_at = row[0], row[1]
        conn.commit()

    return UserResponse(
        id=user_id,
        username=data.username,
        status=UserStatus.ACTIVE,
        created_at=created_at,
    )


def authenticate_user(data: UserLogin, ip_address: str) -> tuple[UserResponse, str, str]:
    r"""
    用户登录认证

    :param data: 登录数据
    :param ip_address: 客户端IP地址
    :return tuple[UserResponse, str, str]: (用户信息, 令牌, 过期时间ISO格式)
    :raise InvalidCredentialsError: 用户名或密码错误
    :raise UserBannedError: 用户已被封禁
    """
    with Database.get_connection() as conn:
        with conn.cursor() as cur:
            cur.execute(
                """
                SELECT id, username, password_hash, status, created_at
                FROM users WHERE username = %s
                """,
                (data.username,),
            )
            row: tuple | None = cur.fetchone()

            if row is None:
                raise InvalidCredentialsError("用户名或密码错误")

            user_id, username, password_hash, status, created_at = row

            if not verify_password(data.password, password_hash):
                raise InvalidCredentialsError("用户名或密码错误")

            user_status: UserStatus = UserStatus(status)
            if user_status == UserStatus.BANNED:
                raise UserBannedError(f"用户已被封禁: {username}")

    token, expires_at = create_session(user_id, ip_address)

    user_response: UserResponse = UserResponse(
        id=user_id,
        username=username,
        status=user_status,
        created_at=created_at,
    )

    return user_response, token, expires_at.isoformat()


def get_user_by_id(user_id: int) -> UserResponse:
    r"""
    通过ID获取用户信息

    :param user_id: 用户ID
    :return UserResponse: 用户信息
    :raise UserNotFoundError: 用户不存在
    """
    with Database.get_connection() as conn:
        with conn.cursor() as cur:
            cur.execute(
                """
                SELECT id, username, status, created_at
                FROM users WHERE id = %s
                """,
                (user_id,),
            )
            row: tuple | None = cur.fetchone()

            if row is None:
                raise UserNotFoundError(f"用户不存在: {user_id}")

            return UserResponse(
                id=row[0],
                username=row[1],
                status=UserStatus(row[2]),
                created_at=row[3],
            )


def update_avatar(user_id: int, avatar: bytes) -> None:
    r"""
    更新用户头像

    :param user_id: 用户ID
    :param avatar: 头像二进制数据
    :return None: 无返回值
    :raise UserNotFoundError: 用户不存在
    """
    with Database.get_connection() as conn:
        with conn.cursor() as cur:
            cur.execute(
                """
                UPDATE users SET avatar = %s WHERE id = %s
                """,
                (avatar, user_id),
            )
            if cur.rowcount == 0:
                raise UserNotFoundError(f"用户不存在: {user_id}")
        conn.commit()


def get_avatar(user_id: int) -> bytes | None:
    r"""
    获取用户头像

    :param user_id: 用户ID
    :return bytes | None: 头像二进制数据，无头像则返回None
    :raise UserNotFoundError: 用户不存在
    """
    with Database.get_connection() as conn:
        with conn.cursor() as cur:
            cur.execute(
                """
                SELECT avatar FROM users WHERE id = %s
                """,
                (user_id,),
            )
            row: tuple | None = cur.fetchone()

            if row is None:
                raise UserNotFoundError(f"用户不存在: {user_id}")

            avatar_data: bytes | None = row[0]
            return bytes(avatar_data) if avatar_data is not None else None


def change_password(user_id: int, data: PasswordChange) -> None:
    r"""
    修改用户密码

    :param user_id: 用户ID
    :param data: 密码修改数据
    :return None: 无返回值
    :raise UserNotFoundError: 用户不存在
    :raise PasswordMismatchError: 原密码错误
    """
    with Database.get_connection() as conn:
        with conn.cursor() as cur:
            cur.execute(
                """
                SELECT password_hash FROM users WHERE id = %s
                """,
                (user_id,),
            )
            row: tuple | None = cur.fetchone()

            if row is None:
                raise UserNotFoundError(f"用户不存在: {user_id}")

            current_hash: str = row[0]
            if not verify_password(data.old_password, current_hash):
                raise PasswordMismatchError("原密码错误")

            new_hash: str = hash_password(data.new_password)
            cur.execute(
                """
                UPDATE users SET password_hash = %s WHERE id = %s
                """,
                (new_hash, user_id),
            )
        conn.commit()


def check_user_status(user_id: int) -> UserStatus:
    r"""
    检查用户状态

    :param user_id: 用户ID
    :return UserStatus: 用户状态
    :raise UserNotFoundError: 用户不存在
    """
    with Database.get_connection() as conn:
        with conn.cursor() as cur:
            cur.execute(
                """
                SELECT status FROM users WHERE id = %s
                """,
                (user_id,),
            )
            row: tuple | None = cur.fetchone()

            if row is None:
                raise UserNotFoundError(f"用户不存在: {user_id}")

            return UserStatus(row[0])


def add_favorite(user_id: int, template_id: int) -> None:
    r"""
    添加模板收藏

    :param user_id: 用户ID
    :param template_id: 模板ID
    :return None: 无返回值
    :raise UserNotFoundError: 用户不存在
    :raise TemplateNotFoundError: 模板不存在
    :raise FavoriteAlreadyExistsError: 已收藏该模板
    """
    with Database.get_connection() as conn:
        with conn.cursor() as cur:
            cur.execute("SELECT id FROM users WHERE id = %s", (user_id,))
            if cur.fetchone() is None:
                raise UserNotFoundError(f"用户不存在: {user_id}")

            cur.execute("SELECT id FROM templates WHERE id = %s",
                        (template_id,))
            if cur.fetchone() is None:
                raise TemplateNotFoundError(f"模板不存在: {template_id}")

            cur.execute(
                """
                SELECT id FROM user_favorites
                WHERE user_id = %s AND template_id = %s
                """,
                (user_id, template_id),
            )
            if cur.fetchone() is not None:
                raise FavoriteAlreadyExistsError(f"已收藏该模板: {template_id}")

            cur.execute(
                """
                INSERT INTO user_favorites (user_id, template_id)
                VALUES (%s, %s)
                """,
                (user_id, template_id),
            )
        conn.commit()


def remove_favorite(user_id: int, template_id: int) -> None:
    r"""
    取消模板收藏

    :param user_id: 用户ID
    :param template_id: 模板ID
    :return None: 无返回值
    :raise FavoriteNotFoundError: 未收藏该模板
    """
    with Database.get_connection() as conn:
        with conn.cursor() as cur:
            cur.execute(
                """
                DELETE FROM user_favorites
                WHERE user_id = %s AND template_id = %s
                """,
                (user_id, template_id),
            )
            if cur.rowcount == 0:
                raise FavoriteNotFoundError(f"未收藏该模板: {template_id}")
        conn.commit()


def get_favorites(user_id: int, page: int = 1, limit: int = 20) -> FavoriteListResponse:
    r"""
    获取用户收藏列表

    :param user_id: 用户ID
    :param page: 页码（从1开始）
    :param limit: 每页数量
    :return FavoriteListResponse: 收藏列表响应
    :raise UserNotFoundError: 用户不存在
    """
    offset: int = (page - 1) * limit

    with Database.get_connection() as conn:
        with conn.cursor() as cur:
            cur.execute("SELECT id FROM users WHERE id = %s", (user_id,))
            if cur.fetchone() is None:
                raise UserNotFoundError(f"用户不存在: {user_id}")

            cur.execute(
                """
                SELECT COUNT(*) FROM user_favorites WHERE user_id = %s
                """,
                (user_id,),
            )
            total: int = cur.fetchone()[0]

            cur.execute(
                """
                SELECT t.id, t.title, t.category, t.description, t.favorite_count, uf.created_at
                FROM user_favorites uf
                JOIN templates t ON uf.template_id = t.id
                WHERE uf.user_id = %s
                ORDER BY uf.created_at DESC
                LIMIT %s OFFSET %s
                """,
                (user_id, limit, offset),
            )
            rows: list[tuple] = cur.fetchall()

            items: list[FavoriteItem] = [
                FavoriteItem(
                    template_id=row[0],
                    title=row[1],
                    category=row[2],
                    description=row[3],
                    favorite_count=row[4],
                    favorited_at=row[5],
                )
                for row in rows
            ]

    return FavoriteListResponse(
        items=items,
        total=total,
        page=page,
        limit=limit,
    )
