r"""
用户服务，处理用户注册、登录、信息管理与收藏

:file: src/service/user_service.py
:author: WaterRun
:time: 2026-01-29
"""

from datetime import datetime

from config import get_config
from database import get_connection
from exceptions import (
    UserNotFoundError,
    UserAlreadyExistsError,
    InvalidCredentialsError,
    UserBannedError,
    PasswordMismatchError,
    TemplateNotFoundError,
    FavoriteAlreadyExistsError,
    FavoriteNotFoundError,
    AuthorizationError,
)
from model import (
    UserCreate,
    UserLogin,
    UserResponse,
    UserStatus,
    UserWithToken,
    PasswordChange,
    FavoriteListResponse,
    FavoriteItem,
)
from service.auth_service import create_session
from utils import hash_password, verify_password


class UserSuspendedError(AuthorizationError):
    r"""
    用户已被暂停，无法执行该操作
    """

    message = "用户已被暂停，无法执行该操作"


def _build_avatar_url(user_id: int, has_avatar: bool) -> str | None:
    r"""
    构建用户头像URL

    :param user_id: 用户ID
    :param has_avatar: 是否有头像
    :return str | None: 头像URL，无头像返回None
    """
    return f"/api/users/{user_id}/avatar" if has_avatar else None


def create_user(data: UserCreate) -> UserResponse:
    r"""
    创建新用户

    :param data: 用户创建数据
    :return UserResponse: 创建的用户信息
    :raise UserAlreadyExistsError: 用户名已存在
    """
    config = get_config()
    password_hashed: str = hash_password(
        data.password, rounds=config.security.bcrypt_rounds)

    with get_connection() as conn:
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
            row: tuple[int, datetime] = cur.fetchone()
            user_id, created_at = row[0], row[1]
        conn.commit()

    return UserResponse(
        id=user_id,
        username=data.username,
        avatar_url=None,
        status=UserStatus.ACTIVE,
        created_at=created_at,
    )


def authenticate_user(data: UserLogin, ip_address: str) -> UserWithToken:
    r"""
    用户登录认证

    Suspended状态用户可以登录，但后续操作（执行代码、收藏）会被拦截

    :param data: 登录数据
    :param ip_address: 客户端IP地址
    :return UserWithToken: 包含令牌和用户信息的响应
    :raise InvalidCredentialsError: 用户名或密码错误
    :raise UserBannedError: 用户已被封禁
    """
    config = get_config()
    expire_hours: int = config.security.access_token_expire_hours

    with get_connection() as conn:
        with conn.cursor() as cur:
            cur.execute(
                """
                SELECT id, username, password_hash, avatar IS NOT NULL, status, created_at
                FROM users WHERE username = %s
                """,
                (data.username,),
            )
            row: tuple | None = cur.fetchone()

            if row is None:
                raise InvalidCredentialsError("用户名或密码错误")

            user_id, username, password_hash, has_avatar, status, created_at = row

            if not verify_password(data.password, password_hash):
                raise InvalidCredentialsError("用户名或密码错误")

            user_status: UserStatus = UserStatus(status)
            if user_status == UserStatus.BANNED:
                raise UserBannedError(f"用户已被封禁: {username}")

    token, expires_at = create_session(user_id, ip_address, expire_hours)

    user_response: UserResponse = UserResponse(
        id=user_id,
        username=username,
        avatar_url=_build_avatar_url(user_id, has_avatar),
        status=user_status,
        created_at=created_at,
    )

    return UserWithToken(
        token=token,
        user=user_response,
        expires_at=expires_at,
    )


def get_user_by_id(user_id: int) -> UserResponse:
    r"""
    通过ID获取用户信息

    :param user_id: 用户ID
    :return UserResponse: 用户信息
    :raise UserNotFoundError: 用户不存在
    """
    with get_connection() as conn:
        with conn.cursor() as cur:
            cur.execute(
                """
                SELECT id, username, avatar IS NOT NULL, status, created_at
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
                avatar_url=_build_avatar_url(row[0], row[2]),
                status=UserStatus(row[3]),
                created_at=row[4],
            )


def update_avatar(user_id: int, avatar: bytes) -> None:
    r"""
    更新用户头像

    :param user_id: 用户ID
    :param avatar: 头像二进制数据
    :raise UserNotFoundError: 用户不存在
    """
    with get_connection() as conn:
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
    with get_connection() as conn:
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
    :raise UserNotFoundError: 用户不存在
    :raise PasswordMismatchError: 原密码错误
    """
    config = get_config()

    with get_connection() as conn:
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

            new_hash: str = hash_password(
                data.new_password, rounds=config.security.bcrypt_rounds)
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
    with get_connection() as conn:
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


def require_active_status(user_id: int) -> None:
    r"""
    要求用户状态为Active，否则抛出异常

    用于需要Active状态的操作（如执行代码、收藏模板）

    :param user_id: 用户ID
    :raise UserNotFoundError: 用户不存在
    :raise UserBannedError: 用户已被封禁
    :raise UserSuspendedError: 用户已被暂停
    """
    status: UserStatus = check_user_status(user_id)
    match status:
        case UserStatus.BANNED:
            raise UserBannedError("用户已被封禁")
        case UserStatus.SUSPENDED:
            raise UserSuspendedError("用户已被暂停，无法执行该操作")
        case UserStatus.ACTIVE:
            ...


def add_favorite(user_id: int, template_id: int) -> None:
    r"""
    添加模板收藏

    :param user_id: 用户ID
    :param template_id: 模板ID
    :raise UserNotFoundError: 用户不存在
    :raise UserBannedError: 用户已被封禁
    :raise UserSuspendedError: 用户已被暂停
    :raise TemplateNotFoundError: 模板不存在
    :raise FavoriteAlreadyExistsError: 已收藏该模板
    """
    require_active_status(user_id)

    with get_connection() as conn:
        with conn.cursor() as cur:
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
    :raise FavoriteNotFoundError: 未收藏该模板
    """
    with get_connection() as conn:
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

    with get_connection() as conn:
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
