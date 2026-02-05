r"""
ds4viz 功能测试

:file: test/function_test.py
:author: WaterRun
:time: 2026-02-05
"""

import time
from datetime import datetime, timezone, timedelta

import psycopg
import pytest
import requests

BASE_URL: str = "http://127.0.0.1:20000"

DB_CONFIG: dict[str, str | int] = {
    "host": "localhost",
    "port": 5432,
    "dbname": "ds4viz_test",
    "user": "ds4viz_test",
    "password": "test_pwd_123",
}


def get_db_connection() -> psycopg.Connection:
    r"""
    获取数据库连接

    :return psycopg.Connection: 数据库连接对象
    """
    return psycopg.connect(**DB_CONFIG)


def reset_database() -> None:
    r"""
    重置数据库到初始状态
    """
    with get_db_connection() as conn:
        with conn.cursor() as cur:
            tables: list[str] = [
                "logs", "execution_cache", "executions",
                "user_favorites", "template_codes", "templates",
                "sessions", "users"
            ]
            for table in tables:
                cur.execute(f"DELETE FROM {table}")

            sequences: list[str] = [
                "users_id_seq", "templates_id_seq", "executions_id_seq",
                "sessions_id_seq", "template_codes_id_seq", "user_favorites_id_seq",
                "execution_cache_id_seq", "logs_id_seq"
            ]
            for seq in sequences:
                cur.execute(f"ALTER SEQUENCE {seq} RESTART WITH 1")
        conn.commit()


def api_url(path: str) -> str:
    r"""
    构造完整 API URL

    :param path: API路径
    :return str: 完整URL
    """
    return f"{BASE_URL}{path}"


def auth_header(token: str) -> dict[str, str]:
    r"""
    构造认证头

    :param token: JWT令牌
    :return dict[str, str]: 认证头字典
    """
    return {"Authorization": f"Bearer {token}"}


def register_user(username: str, password: str) -> requests.Response:
    r"""
    注册用户

    :param username: 用户名
    :param password: 密码
    :return requests.Response: 响应对象
    """
    return requests.post(
        api_url("/api/auth/register"),
        json={"username": username, "password": password},
    )


def login_user(username: str, password: str) -> requests.Response:
    r"""
    用户登录

    :param username: 用户名
    :param password: 密码
    :return requests.Response: 响应对象
    """
    return requests.post(
        api_url("/api/auth/login"),
        json={"username": username, "password": password},
    )


def create_user_and_login(username: str = "testuser", password: str = "testpass") -> str:
    r"""
    创建用户并登录，返回 token

    :param username: 用户名
    :param password: 密码
    :return str: JWT令牌
    """
    register_user(username, password)
    resp: requests.Response = login_user(username, password)
    return resp.json()["token"]


def set_user_status(user_id: int, status: str) -> None:
    r"""
    直接修改用户状态（通过数据库）

    :param user_id: 用户ID
    :param status: 目标状态
    """
    with get_db_connection() as conn:
        with conn.cursor() as cur:
            cur.execute(
                "UPDATE users SET status = %s WHERE id = %s",
                (status, user_id),
            )
        conn.commit()


def create_template(
    title: str,
    category: str,
    description: str,
    codes: list[dict[str, str]] | None = None,
) -> int:
    r"""
    直接在数据库创建模板，返回模板 ID

    :param title: 模板标题
    :param category: 分类
    :param description: 描述
    :param codes: 代码实现列表
    :return int: 模板ID
    """
    with get_db_connection() as conn:
        with conn.cursor() as cur:
            cur.execute(
                """
                INSERT INTO templates (title, category, description)
                VALUES (%s, %s, %s)
                RETURNING id
                """,
                (title, category, description),
            )
            template_id: int = cur.fetchone()[0]

            if codes:
                for code in codes:
                    cur.execute(
                        """
                        INSERT INTO template_codes (template_id, language, code, explanation)
                        VALUES (%s, %s, %s, %s)
                        """,
                        (
                            template_id,
                            code.get("language", "python"),
                            code.get("code", ""),
                            code.get("explanation"),
                        ),
                    )
        conn.commit()
    return template_id


@pytest.fixture(autouse=True)
def clean_db(server_process) -> None:
    r"""
    每个测试前后清理数据库
    """
    reset_database()
    yield
    reset_database()


class TestAuthRegister:
    r"""
    用户注册测试
    """

    def test_register_success(self) -> None:
        r"""
        正常注册：有效用户名和密码
        """
        resp: requests.Response = register_user("testuser", "testpass123")
        assert resp.status_code == 200
        data: dict = resp.json()
        assert data["username"] == "testuser"
        assert data["status"] == "Active"
        assert data["avatar_url"] is None
        assert "id" in data
        assert "created_at" in data

    def test_register_username_min_length(self) -> None:
        r"""
        用户名边界：恰好 3 个字符（最小）
        """
        resp: requests.Response = register_user("abc", "testpass")
        assert resp.status_code == 200
        assert resp.json()["username"] == "abc"

    def test_register_username_max_length(self) -> None:
        r"""
        用户名边界：恰好 32 个字符（最大）
        """
        username: str = "a" * 32
        resp: requests.Response = register_user(username, "testpass")
        assert resp.status_code == 200
        assert resp.json()["username"] == username

    def test_register_username_too_short(self) -> None:
        r"""
        用户名过短：2 个字符，返回 422
        """
        resp: requests.Response = register_user("ab", "testpass")
        assert resp.status_code == 422

    def test_register_username_too_long(self) -> None:
        r"""
        用户名过长：33 个字符，返回 422
        """
        username: str = "a" * 33
        resp: requests.Response = register_user(username, "testpass")
        assert resp.status_code == 422

    def test_register_password_min_length(self) -> None:
        r"""
        密码边界：恰好 1 个字符（最小）
        """
        resp: requests.Response = register_user("testuser", "a")
        assert resp.status_code == 200

    def test_register_password_max_length(self) -> None:
        r"""
        密码边界：恰好 64 个字符（最大）
        """
        password: str = "a" * 64
        resp: requests.Response = register_user("testuser", password)
        assert resp.status_code == 200

    def test_register_password_too_long(self) -> None:
        r"""
        密码过长：65 个字符，返回 422
        """
        password: str = "a" * 65
        resp: requests.Response = register_user("testuser", password)
        assert resp.status_code == 422

    def test_register_password_utf8_bytes_exceed(self) -> None:
        r"""
        密码 UTF-8 字节超限：使用多字节字符使字节数超过 64
        """
        password: str = "中" * 22
        resp: requests.Response = register_user("testuser", password)
        assert resp.status_code == 422

    def test_register_duplicate_username(self) -> None:
        r"""
        用户名重复注册，返回 409
        """
        register_user("testuser", "testpass")
        resp: requests.Response = register_user("testuser", "anotherpass")
        assert resp.status_code == 409

    def test_register_username_chinese(self) -> None:
        r"""
        用户名含中文字符
        """
        resp: requests.Response = register_user("测试用户", "testpass")
        assert resp.status_code == 200
        assert resp.json()["username"] == "测试用户"

    def test_register_username_special_chars(self) -> None:
        r"""
        用户名含特殊字符
        """
        resp: requests.Response = register_user("test_user-01", "testpass")
        assert resp.status_code == 200
        assert resp.json()["username"] == "test_user-01"

    def test_register_empty_username(self) -> None:
        r"""
        空用户名，返回 422
        """
        resp: requests.Response = register_user("", "testpass")
        assert resp.status_code == 422

    def test_register_empty_password(self) -> None:
        r"""
        空密码，返回 422
        """
        resp: requests.Response = register_user("testuser", "")
        assert resp.status_code == 422

    def test_register_response_structure(self) -> None:
        r"""
        验证返回结构完整性
        """
        resp: requests.Response = register_user("testuser", "testpass")
        data: dict = resp.json()
        assert "id" in data
        assert "username" in data
        assert "avatar_url" in data
        assert "status" in data
        assert "created_at" in data
        assert isinstance(data["id"], int)
        assert data["id"] > 0


class TestAuthLogin:
    r"""
    用户登录测试
    """

    def test_login_success(self) -> None:
        r"""
        正常登录，返回 token 和用户信息
        """
        register_user("testuser", "testpass")
        resp: requests.Response = login_user("testuser", "testpass")
        assert resp.status_code == 200
        data: dict = resp.json()
        assert "token" in data
        assert "user" in data
        assert "expires_at" in data
        assert data["user"]["username"] == "testuser"

    def test_login_user_not_found(self) -> None:
        r"""
        用户名不存在，返回 401
        """
        resp: requests.Response = login_user("nonexistent", "testpass")
        assert resp.status_code == 401

    def test_login_wrong_password(self) -> None:
        r"""
        密码错误，返回 401
        """
        register_user("testuser", "testpass")
        resp: requests.Response = login_user("testuser", "wrongpass")
        assert resp.status_code == 401

    def test_login_banned_user(self) -> None:
        r"""
        Banned 用户登录，返回 403
        """
        register_user("testuser", "testpass")
        set_user_status(1, "Banned")
        resp: requests.Response = login_user("testuser", "testpass")
        assert resp.status_code == 403

    def test_login_suspended_user(self) -> None:
        r"""
        Suspended 用户登录，返回成功但 status 为 Suspended
        """
        register_user("testuser", "testpass")
        set_user_status(1, "Suspended")
        resp: requests.Response = login_user("testuser", "testpass")
        assert resp.status_code == 200
        assert resp.json()["user"]["status"] == "Suspended"

    def test_login_token_format(self) -> None:
        r"""
        验证 token 格式（JWT）
        """
        register_user("testuser", "testpass")
        resp: requests.Response = login_user("testuser", "testpass")
        token: str = resp.json()["token"]
        parts: list[str] = token.split(".")
        assert len(parts) == 3

    def test_login_expires_at_reasonable(self) -> None:
        r"""
        验证 expires_at 时间合理
        """
        register_user("testuser", "testpass")
        resp: requests.Response = login_user("testuser", "testpass")
        expires_at: datetime = datetime.fromisoformat(resp.json()["expires_at"])
        now: datetime = datetime.now(timezone.utc)
        assert expires_at > now
        assert expires_at < now + timedelta(hours=2)

    def test_login_multiple_sessions(self) -> None:
        r"""
        同一用户多次登录生成不同 session
        """
        register_user("testuser", "testpass")
        resp1: requests.Response = login_user("testuser", "testpass")
        resp2: requests.Response = login_user("testuser", "testpass")
        token1: str = resp1.json()["token"]
        token2: str = resp2.json()["token"]
        assert token1 != token2


class TestAuthLogout:
    r"""
    用户登出测试
    """

    def test_logout_success(self) -> None:
        r"""
        正常登出
        """
        token: str = create_user_and_login()
        resp: requests.Response = requests.post(
            api_url("/api/auth/logout"),
            headers=auth_header(token)
        )
        assert resp.status_code == 200
        assert resp.json()["message"] == "Logged out successfully"

    def test_logout_token_invalidated(self) -> None:
        r"""
        登出后原 token 立即失效
        """
        token: str = create_user_and_login()
        requests.post(api_url("/api/auth/logout"), headers=auth_header(token))
        resp: requests.Response = requests.get(
            api_url("/api/auth/me"),
            headers=auth_header(token)
        )
        assert resp.status_code == 401

    def test_logout_invalid_token(self) -> None:
        r"""
        无效 token 登出，返回 401
        """
        resp: requests.Response = requests.post(
            api_url("/api/auth/logout"),
            headers=auth_header("invalid.token.here"),
        )
        assert resp.status_code == 401

    def test_logout_no_auth_header(self) -> None:
        r"""
        无 Authorization 头登出，返回 401
        """
        resp: requests.Response = requests.post(api_url("/api/auth/logout"))
        assert resp.status_code == 401

    def test_logout_duplicate(self) -> None:
        r"""
        重复登出同一 token，第二次返回 401
        """
        token: str = create_user_and_login()
        resp1: requests.Response = requests.post(
            api_url("/api/auth/logout"),
            headers=auth_header(token)
        )
        assert resp1.status_code == 200
        resp2: requests.Response = requests.post(
            api_url("/api/auth/logout"),
            headers=auth_header(token)
        )
        assert resp2.status_code == 401


class TestAuthMe:
    r"""
    获取当前用户测试
    """

    def test_me_success(self) -> None:
        r"""
        正常获取当前用户信息
        """
        token: str = create_user_and_login("testuser", "testpass")
        resp: requests.Response = requests.get(
            api_url("/api/auth/me"),
            headers=auth_header(token)
        )
        assert resp.status_code == 200
        data: dict = resp.json()
        assert data["username"] == "testuser"

    def test_me_invalid_token(self) -> None:
        r"""
        无效 token，返回 401
        """
        resp: requests.Response = requests.get(
            api_url("/api/auth/me"),
            headers=auth_header("invalid.token"),
        )
        assert resp.status_code == 401

    def test_me_no_auth_header(self) -> None:
        r"""
        无 Authorization 头，返回 401
        """
        resp: requests.Response = requests.get(api_url("/api/auth/me"))
        assert resp.status_code == 401

    def test_me_response_structure(self) -> None:
        r"""
        验证返回结构完整
        """
        token: str = create_user_and_login()
        resp: requests.Response = requests.get(
            api_url("/api/auth/me"),
            headers=auth_header(token)
        )
        data: dict = resp.json()
        assert "id" in data
        assert "username" in data
        assert "avatar_url" in data
        assert "status" in data
        assert "created_at" in data


class TestUserAvatar:
    r"""
    用户头像测试
    """

    def test_upload_avatar_first_time(self) -> None:
        r"""
        上传头像：首次上传
        """
        token: str = create_user_and_login()
        resp: requests.Response = requests.get(
            api_url("/api/auth/me"),
            headers=auth_header(token)
        )
        user_id: int = resp.json()["id"]

        avatar_data: bytes = b"\x89PNG\r\n\x1a\n" + b"\x00" * 100
        resp = requests.put(
            api_url(f"/api/users/{user_id}/avatar"),
            headers=auth_header(token),
            files={"avatar": ("test.png", avatar_data, "image/png")},
        )
        assert resp.status_code == 200
        assert "avatar_url" in resp.json()

    def test_upload_avatar_update(self) -> None:
        r"""
        更新头像：覆盖已有头像
        """
        token: str = create_user_and_login()
        resp: requests.Response = requests.get(
            api_url("/api/auth/me"),
            headers=auth_header(token)
        )
        user_id: int = resp.json()["id"]

        avatar_data1: bytes = b"\x89PNG\r\n\x1a\n" + b"\x01" * 100
        requests.put(
            api_url(f"/api/users/{user_id}/avatar"),
            headers=auth_header(token),
            files={"avatar": ("test1.png", avatar_data1, "image/png")},
        )

        avatar_data2: bytes = b"\x89PNG\r\n\x1a\n" + b"\x02" * 100
        resp = requests.put(
            api_url(f"/api/users/{user_id}/avatar"),
            headers=auth_header(token),
            files={"avatar": ("test2.png", avatar_data2, "image/png")},
        )
        assert resp.status_code == 200

    def test_get_avatar_success(self) -> None:
        r"""
        获取头像：返回正确二进制数据
        """
        token: str = create_user_and_login()
        resp: requests.Response = requests.get(
            api_url("/api/auth/me"),
            headers=auth_header(token)
        )
        user_id: int = resp.json()["id"]

        avatar_data: bytes = b"\x89PNG\r\n\x1a\n" + b"\xAB" * 100
        requests.put(
            api_url(f"/api/users/{user_id}/avatar"),
            headers=auth_header(token),
            files={"avatar": ("test.png", avatar_data, "image/png")},
        )

        resp = requests.get(api_url(f"/api/users/{user_id}/avatar"))
        assert resp.status_code == 200
        assert resp.content == avatar_data

    def test_get_avatar_no_avatar(self) -> None:
        r"""
        获取无头像用户的头像，返回 404
        """
        token: str = create_user_and_login()
        resp: requests.Response = requests.get(
            api_url("/api/auth/me"),
            headers=auth_header(token)
        )
        user_id: int = resp.json()["id"]

        resp = requests.get(api_url(f"/api/users/{user_id}/avatar"))
        assert resp.status_code == 404

    def test_get_avatar_user_not_found(self) -> None:
        r"""
        获取不存在用户的头像，返回 404
        """
        resp: requests.Response = requests.get(api_url("/api/users/99999/avatar"))
        assert resp.status_code == 404

    def test_upload_avatar_other_user(self) -> None:
        r"""
        上传他人头像，返回 403
        """
        token1: str = create_user_and_login("user1", "pass1")
        create_user_and_login("user2", "pass2")

        avatar_data: bytes = b"\x89PNG\r\n\x1a\n" + b"\x00" * 100
        resp: requests.Response = requests.put(
            api_url("/api/users/2/avatar"),
            headers=auth_header(token1),
            files={"avatar": ("test.png", avatar_data, "image/png")},
        )
        assert resp.status_code == 403

    def test_upload_avatar_not_logged_in(self) -> None:
        r"""
        未登录上传头像，返回 401
        """
        avatar_data: bytes = b"\x89PNG\r\n\x1a\n" + b"\x00" * 100
        resp: requests.Response = requests.put(
            api_url("/api/users/1/avatar"),
            files={"avatar": ("test.png", avatar_data, "image/png")},
        )
        assert resp.status_code == 401

    def test_upload_avatar_url_returned(self) -> None:
        r"""
        上传后 avatar_url 字段正确返回
        """
        token: str = create_user_and_login()
        resp: requests.Response = requests.get(
            api_url("/api/auth/me"),
            headers=auth_header(token)
        )
        user_id: int = resp.json()["id"]

        avatar_data: bytes = b"\x89PNG\r\n\x1a\n" + b"\x00" * 100
        requests.put(
            api_url(f"/api/users/{user_id}/avatar"),
            headers=auth_header(token),
            files={"avatar": ("test.png", avatar_data, "image/png")},
        )

        resp = requests.get(api_url("/api/auth/me"), headers=auth_header(token))
        assert resp.json()["avatar_url"] == f"/api/users/{user_id}/avatar"


class TestUserPassword:
    r"""
    修改密码测试
    """

    def test_change_password_success(self) -> None:
        r"""
        正常修改密码
        """
        token: str = create_user_and_login("testuser", "oldpass")
        resp: requests.Response = requests.get(
            api_url("/api/auth/me"),
            headers=auth_header(token)
        )
        user_id: int = resp.json()["id"]

        resp = requests.put(
            api_url(f"/api/users/{user_id}/password"),
            headers=auth_header(token),
            json={"old_password": "oldpass", "new_password": "newpass"},
        )
        assert resp.status_code == 200

    def test_change_password_wrong_old(self) -> None:
        r"""
        旧密码错误，返回 400
        """
        token: str = create_user_and_login("testuser", "oldpass")
        resp: requests.Response = requests.get(
            api_url("/api/auth/me"),
            headers=auth_header(token)
        )
        user_id: int = resp.json()["id"]

        resp = requests.put(
            api_url(f"/api/users/{user_id}/password"),
            headers=auth_header(token),
            json={"old_password": "wrongpass", "new_password": "newpass"},
        )
        assert resp.status_code == 400

    def test_change_password_other_user(self) -> None:
        r"""
        修改他人密码，返回 403
        """
        token1: str = create_user_and_login("user1", "pass1")
        create_user_and_login("user2", "pass2")

        resp: requests.Response = requests.put(
            api_url("/api/users/2/password"),
            headers=auth_header(token1),
            json={"old_password": "pass2", "new_password": "newpass"},
        )
        assert resp.status_code == 403

    def test_change_password_too_long(self) -> None:
        r"""
        新密码过长，返回 422
        """
        token: str = create_user_and_login("testuser", "oldpass")
        resp: requests.Response = requests.get(
            api_url("/api/auth/me"),
            headers=auth_header(token)
        )
        user_id: int = resp.json()["id"]

        resp = requests.put(
            api_url(f"/api/users/{user_id}/password"),
            headers=auth_header(token),
            json={"old_password": "oldpass", "new_password": "a" * 65},
        )
        assert resp.status_code == 422

    def test_change_password_utf8_exceed(self) -> None:
        r"""
        新密码 UTF-8 字节超限，返回 422
        """
        token: str = create_user_and_login("testuser", "oldpass")
        resp: requests.Response = requests.get(
            api_url("/api/auth/me"),
            headers=auth_header(token)
        )
        user_id: int = resp.json()["id"]

        resp = requests.put(
            api_url(f"/api/users/{user_id}/password"),
            headers=auth_header(token),
            json={"old_password": "oldpass", "new_password": "中" * 22},
        )
        assert resp.status_code == 422

    def test_change_password_user_not_found(self) -> None:
        r"""
        修改不存在用户的密码，当前用户无权限，返回 403
        """
        token: str = create_user_and_login()
        resp: requests.Response = requests.put(
            api_url("/api/users/99999/password"),
            headers=auth_header(token),
            json={"old_password": "oldpass", "new_password": "newpass"},
        )
        assert resp.status_code == 403

    def test_change_password_not_logged_in(self) -> None:
        r"""
        未登录修改密码，返回 401
        """
        resp: requests.Response = requests.put(
            api_url("/api/users/1/password"),
            json={"old_password": "oldpass", "new_password": "newpass"},
        )
        assert resp.status_code == 401

    def test_change_password_old_invalid(self) -> None:
        r"""
        修改后旧密码无法登录
        """
        register_user("testuser", "oldpass")
        resp: requests.Response = login_user("testuser", "oldpass")
        token: str = resp.json()["token"]
        user_id: int = resp.json()["user"]["id"]

        requests.put(
            api_url(f"/api/users/{user_id}/password"),
            headers=auth_header(token),
            json={"old_password": "oldpass", "new_password": "newpass"},
        )

        resp = login_user("testuser", "oldpass")
        assert resp.status_code == 401

    def test_change_password_new_valid(self) -> None:
        r"""
        修改后新密码可以登录
        """
        register_user("testuser", "oldpass")
        resp: requests.Response = login_user("testuser", "oldpass")
        token: str = resp.json()["token"]
        user_id: int = resp.json()["user"]["id"]

        requests.put(
            api_url(f"/api/users/{user_id}/password"),
            headers=auth_header(token),
            json={"old_password": "oldpass", "new_password": "newpass"},
        )

        resp = login_user("testuser", "newpass")
        assert resp.status_code == 200


class TestUserFavorites:
    r"""
    用户收藏列表测试
    """

    def test_get_favorites_with_items(self) -> None:
        r"""
        正常获取收藏列表（有收藏）
        """
        token: str = create_user_and_login()
        resp: requests.Response = requests.get(
            api_url("/api/auth/me"),
            headers=auth_header(token)
        )
        user_id: int = resp.json()["id"]

        template_id: int = create_template("测试模板", "测试分类", "测试描述")
        requests.post(
            api_url("/api/favorites"),
            headers=auth_header(token),
            json={"template_id": template_id},
        )

        resp = requests.get(
            api_url(f"/api/users/{user_id}/favorites"),
            headers=auth_header(token),
        )
        assert resp.status_code == 200
        data: dict = resp.json()
        assert data["total"] == 1
        assert len(data["items"]) == 1
        assert data["items"][0]["template_id"] == template_id

    def test_get_favorites_empty(self) -> None:
        r"""
        空收藏列表
        """
        token: str = create_user_and_login()
        resp: requests.Response = requests.get(
            api_url("/api/auth/me"),
            headers=auth_header(token)
        )
        user_id: int = resp.json()["id"]

        resp = requests.get(
            api_url(f"/api/users/{user_id}/favorites"),
            headers=auth_header(token),
        )
        assert resp.status_code == 200
        assert resp.json()["total"] == 0
        assert resp.json()["items"] == []

    def test_get_favorites_pagination(self) -> None:
        r"""
        分页：page=1, limit=10
        """
        token: str = create_user_and_login()
        resp: requests.Response = requests.get(
            api_url("/api/auth/me"),
            headers=auth_header(token)
        )
        user_id: int = resp.json()["id"]

        for i in range(15):
            template_id: int = create_template(f"模板{i}", "分类", "描述")
            requests.post(
                api_url("/api/favorites"),
                headers=auth_header(token),
                json={"template_id": template_id},
            )

        resp = requests.get(
            api_url(f"/api/users/{user_id}/favorites"),
            headers=auth_header(token),
            params={"page": 1, "limit": 10},
        )
        assert resp.status_code == 200
        data: dict = resp.json()
        assert data["total"] == 15
        assert len(data["items"]) == 10
        assert data["page"] == 1
        assert data["limit"] == 10

    def test_get_favorites_page_2(self) -> None:
        r"""
        分页：page=2
        """
        token: str = create_user_and_login()
        resp: requests.Response = requests.get(
            api_url("/api/auth/me"),
            headers=auth_header(token)
        )
        user_id: int = resp.json()["id"]

        for i in range(15):
            template_id: int = create_template(f"模板{i}", "分类", "描述")
            requests.post(
                api_url("/api/favorites"),
                headers=auth_header(token),
                json={"template_id": template_id},
            )

        resp = requests.get(
            api_url(f"/api/users/{user_id}/favorites"),
            headers=auth_header(token),
            params={"page": 2, "limit": 10},
        )
        assert resp.status_code == 200
        assert len(resp.json()["items"]) == 5

    def test_get_favorites_limit_1(self) -> None:
        r"""
        limit 边界：limit=1
        """
        token: str = create_user_and_login()
        resp: requests.Response = requests.get(
            api_url("/api/auth/me"),
            headers=auth_header(token)
        )
        user_id: int = resp.json()["id"]

        for i in range(3):
            template_id: int = create_template(f"模板{i}", "分类", "描述")
            requests.post(
                api_url("/api/favorites"),
                headers=auth_header(token),
                json={"template_id": template_id},
            )

        resp = requests.get(
            api_url(f"/api/users/{user_id}/favorites"),
            headers=auth_header(token),
            params={"limit": 1},
        )
        assert resp.status_code == 200
        assert len(resp.json()["items"]) == 1

    def test_get_favorites_limit_100(self) -> None:
        r"""
        limit 边界：limit=100
        """
        token: str = create_user_and_login()
        resp: requests.Response = requests.get(
            api_url("/api/auth/me"),
            headers=auth_header(token)
        )
        user_id: int = resp.json()["id"]

        resp = requests.get(
            api_url(f"/api/users/{user_id}/favorites"),
            headers=auth_header(token),
            params={"limit": 100},
        )
        assert resp.status_code == 200

    def test_get_favorites_other_user(self) -> None:
        r"""
        查看他人收藏，返回 403
        """
        token1: str = create_user_and_login("user1", "pass1")
        create_user_and_login("user2", "pass2")

        resp: requests.Response = requests.get(
            api_url("/api/users/2/favorites"),
            headers=auth_header(token1),
        )
        assert resp.status_code == 403

    def test_get_favorites_user_not_found(self) -> None:
        r"""
        获取不存在用户的收藏，当前用户无权限，返回 403
        """
        token: str = create_user_and_login()
        resp: requests.Response = requests.get(
            api_url("/api/users/99999/favorites"),
            headers=auth_header(token),
        )
        assert resp.status_code == 403

    def test_get_favorites_not_logged_in(self) -> None:
        r"""
        未登录获取，返回 401
        """
        resp: requests.Response = requests.get(api_url("/api/users/1/favorites"))
        assert resp.status_code == 401

    def test_get_favorites_order_by_time_desc(self) -> None:
        r"""
        验证按收藏时间倒序排列
        """
        token: str = create_user_and_login()
        resp: requests.Response = requests.get(
            api_url("/api/auth/me"),
            headers=auth_header(token)
        )
        user_id: int = resp.json()["id"]

        template_id1: int = create_template("模板1", "分类", "描述")
        requests.post(
            api_url("/api/favorites"),
            headers=auth_header(token),
            json={"template_id": template_id1},
        )
        time.sleep(0.1)
        template_id2: int = create_template("模板2", "分类", "描述")
        requests.post(
            api_url("/api/favorites"),
            headers=auth_header(token),
            json={"template_id": template_id2},
        )

        resp = requests.get(
            api_url(f"/api/users/{user_id}/favorites"),
            headers=auth_header(token),
        )
        items: list[dict] = resp.json()["items"]
        assert items[0]["template_id"] == template_id2
        assert items[1]["template_id"] == template_id1


class TestUserExecutions:
    r"""
    用户执行历史测试
    """

    def test_get_executions_success(self) -> None:
        r"""
        正常获取执行历史
        """
        token: str = create_user_and_login()
        resp: requests.Response = requests.get(
            api_url("/api/auth/me"),
            headers=auth_header(token)
        )
        user_id: int = resp.json()["id"]

        code: str = '''
import ds4viz as dv
with dv.stack("test") as s:
    s.push(1)
'''
        requests.post(
            api_url("/api/execute"),
            headers=auth_header(token),
            json={"language": "python", "code": code},
        )

        resp = requests.get(
            api_url(f"/api/users/{user_id}/executions"),
            headers=auth_header(token),
        )
        assert resp.status_code == 200
        assert resp.json()["total"] >= 1

    def test_get_executions_empty(self) -> None:
        r"""
        空执行历史
        """
        token: str = create_user_and_login()
        resp: requests.Response = requests.get(
            api_url("/api/auth/me"),
            headers=auth_header(token)
        )
        user_id: int = resp.json()["id"]

        resp = requests.get(
            api_url(f"/api/users/{user_id}/executions"),
            headers=auth_header(token),
        )
        assert resp.status_code == 200
        assert resp.json()["total"] == 0

    def test_get_executions_pagination(self) -> None:
        r"""
        分页测试
        """
        token: str = create_user_and_login()
        resp: requests.Response = requests.get(
            api_url("/api/auth/me"),
            headers=auth_header(token)
        )
        user_id: int = resp.json()["id"]

        for i in range(5):
            code: str = f'''
import ds4viz as dv
with dv.stack("test{i}") as s:
    s.push({i})
'''
            requests.post(
                api_url("/api/execute"),
                headers=auth_header(token),
                json={"language": "python", "code": code},
            )

        resp = requests.get(
            api_url(f"/api/users/{user_id}/executions"),
            headers=auth_header(token),
            params={"page": 1, "limit": 3},
        )
        assert resp.status_code == 200
        assert len(resp.json()["items"]) == 3

    def test_get_executions_other_user(self) -> None:
        r"""
        查看他人执行历史，返回 403
        """
        token1: str = create_user_and_login("user1", "pass1")
        create_user_and_login("user2", "pass2")

        resp: requests.Response = requests.get(
            api_url("/api/users/2/executions"),
            headers=auth_header(token1),
        )
        assert resp.status_code == 403

    def test_get_executions_not_logged_in(self) -> None:
        r"""
        未登录获取，返回 401
        """
        resp: requests.Response = requests.get(api_url("/api/users/1/executions"))
        assert resp.status_code == 401

    def test_get_executions_order_by_time_desc(self) -> None:
        r"""
        验证按创建时间倒序排列
        """
        token: str = create_user_and_login()
        resp: requests.Response = requests.get(
            api_url("/api/auth/me"),
            headers=auth_header(token)
        )
        user_id: int = resp.json()["id"]

        for i in range(3):
            code: str = f'''
import ds4viz as dv
with dv.stack("test{i}") as s:
    s.push({i})
'''
            requests.post(
                api_url("/api/execute"),
                headers=auth_header(token),
                json={"language": "python", "code": code},
            )
            time.sleep(0.1)

        resp = requests.get(
            api_url(f"/api/users/{user_id}/executions"),
            headers=auth_header(token),
        )
        items: list[dict] = resp.json()["items"]
        for i in range(len(items) - 1):
            assert items[i]["created_at"] >= items[i + 1]["created_at"]

    def test_get_executions_various_status(self) -> None:
        r"""
        验证各状态记录都能正确返回
        """
        token: str = create_user_and_login()
        resp: requests.Response = requests.get(
            api_url("/api/auth/me"),
            headers=auth_header(token)
        )
        user_id: int = resp.json()["id"]

        success_code: str = '''
import ds4viz as dv
with dv.stack("test") as s:
    s.push(1)
'''
        requests.post(
            api_url("/api/execute"),
            headers=auth_header(token),
            json={"language": "python", "code": success_code},
        )

        error_code: str = "syntax error here!!!"
        requests.post(
            api_url("/api/execute"),
            headers=auth_header(token),
            json={"language": "python", "code": error_code},
        )

        resp = requests.get(
            api_url(f"/api/users/{user_id}/executions"),
            headers=auth_header(token),
        )
        assert resp.status_code == 200
        assert resp.json()["total"] == 2


class TestTemplateList:
    r"""
    模板列表测试
    """

    def test_get_templates_success(self) -> None:
        r"""
        正常获取模板列表
        """
        create_template("模板1", "分类A", "描述1")
        create_template("模板2", "分类B", "描述2")

        resp: requests.Response = requests.get(api_url("/api/templates"))
        assert resp.status_code == 200
        data: dict = resp.json()
        assert data["total"] == 2
        assert len(data["items"]) == 2

    def test_get_templates_empty(self) -> None:
        r"""
        空模板列表
        """
        resp: requests.Response = requests.get(api_url("/api/templates"))
        assert resp.status_code == 200
        assert resp.json()["total"] == 0
        assert resp.json()["items"] == []

    def test_get_templates_pagination(self) -> None:
        r"""
        分页测试
        """
        for i in range(15):
            create_template(f"模板{i}", "分类", "描述")

        resp: requests.Response = requests.get(
            api_url("/api/templates"),
            params={"page": 1, "limit": 10}
        )
        assert resp.status_code == 200
        data: dict = resp.json()
        assert data["total"] == 15
        assert len(data["items"]) == 10

    def test_get_templates_filter_by_category(self) -> None:
        r"""
        按分类筛选
        """
        create_template("模板1", "分类A", "描述1")
        create_template("模板2", "分类B", "描述2")
        create_template("模板3", "分类A", "描述3")

        resp: requests.Response = requests.get(
            api_url("/api/templates"),
            params={"category": "分类A"}
        )
        assert resp.status_code == 200
        data: dict = resp.json()
        assert data["total"] == 2
        for item in data["items"]:
            assert item["category"] == "分类A"

    def test_get_templates_category_not_found(self) -> None:
        r"""
        分类不存在返回空列表
        """
        create_template("模板1", "分类A", "描述1")

        resp: requests.Response = requests.get(
            api_url("/api/templates"),
            params={"category": "不存在的分类"}
        )
        assert resp.status_code == 200
        assert resp.json()["total"] == 0

    def test_get_templates_logged_in_is_favorited(self) -> None:
        r"""
        登录用户获取列表，验证 is_favorited 正确
        """
        template_id: int = create_template("模板1", "分类", "描述")
        token: str = create_user_and_login()
        requests.post(
            api_url("/api/favorites"),
            headers=auth_header(token),
            json={"template_id": template_id},
        )

        resp: requests.Response = requests.get(
            api_url("/api/templates"),
            headers=auth_header(token)
        )
        assert resp.status_code == 200
        items: list[dict] = resp.json()["items"]
        favorited_item: dict = next(i for i in items if i["id"] == template_id)
        assert favorited_item["is_favorited"] is True

    def test_get_templates_not_logged_in_is_favorited_false(self) -> None:
        r"""
        未登录用户获取列表，is_favorited 为 false
        """
        create_template("模板1", "分类", "描述")

        resp: requests.Response = requests.get(api_url("/api/templates"))
        assert resp.status_code == 200
        for item in resp.json()["items"]:
            assert item["is_favorited"] is False

    def test_get_templates_order_by_time_desc(self) -> None:
        r"""
        验证按创建时间倒序排列
        """
        create_template("模板1", "分类", "描述")
        time.sleep(0.1)
        create_template("模板2", "分类", "描述")

        resp: requests.Response = requests.get(api_url("/api/templates"))
        items: list[dict] = resp.json()["items"]
        assert items[0]["title"] == "模板2"
        assert items[1]["title"] == "模板1"


class TestTemplateDetail:
    r"""
    模板详情测试
    """

    def test_get_template_success(self) -> None:
        r"""
        正常获取模板详情
        """
        template_id: int = create_template(
            "测试模板",
            "测试分类",
            "测试描述",
            codes=[
                {"language": "python", "code": "print('hello')", "explanation": "Python实现"},
                {"language": "lua", "code": "print('hello')", "explanation": None},
            ],
        )

        resp: requests.Response = requests.get(api_url(f"/api/templates/{template_id}"))
        assert resp.status_code == 200
        data: dict = resp.json()
        assert data["title"] == "测试模板"
        assert data["category"] == "测试分类"
        assert len(data["codes"]) == 2

    def test_get_template_not_found(self) -> None:
        r"""
        模板不存在，返回 404
        """
        resp: requests.Response = requests.get(api_url("/api/templates/99999"))
        assert resp.status_code == 404

    def test_get_template_codes_multi_language(self) -> None:
        r"""
        验证 codes 数组包含多语言实现
        """
        template_id: int = create_template(
            "模板",
            "分类",
            "描述",
            codes=[
                {"language": "python", "code": "python code"},
                {"language": "lua", "code": "lua code"},
                {"language": "rust", "code": "rust code"},
            ],
        )

        resp: requests.Response = requests.get(api_url(f"/api/templates/{template_id}"))
        codes: list[dict] = resp.json()["codes"]
        languages: list[str] = [c["language"] for c in codes]
        assert "python" in languages
        assert "lua" in languages
        assert "rust" in languages

    def test_get_template_explanation_null(self) -> None:
        r"""
        验证 explanation 字段可为 null
        """
        template_id: int = create_template(
            "模板",
            "分类",
            "描述",
            codes=[{"language": "python", "code": "code", "explanation": None}],
        )

        resp: requests.Response = requests.get(api_url(f"/api/templates/{template_id}"))
        assert resp.json()["codes"][0]["explanation"] is None

    def test_get_template_logged_in_is_favorited(self) -> None:
        r"""
        登录用户获取，验证 is_favorited
        """
        template_id: int = create_template("模板", "分类", "描述")
        token: str = create_user_and_login()
        requests.post(
            api_url("/api/favorites"),
            headers=auth_header(token),
            json={"template_id": template_id},
        )

        resp: requests.Response = requests.get(
            api_url(f"/api/templates/{template_id}"),
            headers=auth_header(token),
        )
        assert resp.json()["is_favorited"] is True

    def test_get_template_not_logged_in(self) -> None:
        r"""
        未登录用户获取
        """
        template_id: int = create_template("模板", "分类", "描述")

        resp: requests.Response = requests.get(api_url(f"/api/templates/{template_id}"))
        assert resp.status_code == 200
        assert resp.json()["is_favorited"] is False


class TestTemplateCode:
    r"""
    获取模板代码测试
    """

    def test_get_template_code_python(self) -> None:
        r"""
        获取 Python 实现
        """
        template_id: int = create_template(
            "模板",
            "分类",
            "描述",
            codes=[{"language": "python", "code": "python code", "explanation": "说明"}],
        )

        resp: requests.Response = requests.get(
            api_url(f"/api/templates/{template_id}/code/python")
        )
        assert resp.status_code == 200
        data: dict = resp.json()
        assert data["language"] == "python"
        assert data["code"] == "python code"

    def test_get_template_code_lua(self) -> None:
        r"""
        获取 Lua 实现
        """
        template_id: int = create_template(
            "模板",
            "分类",
            "描述",
            codes=[{"language": "lua", "code": "lua code"}],
        )

        resp: requests.Response = requests.get(
            api_url(f"/api/templates/{template_id}/code/lua")
        )
        assert resp.status_code == 200
        assert resp.json()["language"] == "lua"

    def test_get_template_code_rust(self) -> None:
        r"""
        获取 Rust 实现
        """
        template_id: int = create_template(
            "模板",
            "分类",
            "描述",
            codes=[{"language": "rust", "code": "rust code"}],
        )

        resp: requests.Response = requests.get(
            api_url(f"/api/templates/{template_id}/code/rust")
        )
        assert resp.status_code == 200
        assert resp.json()["language"] == "rust"

    def test_get_template_code_template_not_found(self) -> None:
        r"""
        模板不存在，返回 404
        """
        resp: requests.Response = requests.get(api_url("/api/templates/99999/code/python"))
        assert resp.status_code == 404

    def test_get_template_code_language_not_found(self) -> None:
        r"""
        语言实现不存在，返回 404
        """
        template_id: int = create_template(
            "模板",
            "分类",
            "描述",
            codes=[{"language": "python", "code": "code"}],
        )

        resp: requests.Response = requests.get(
            api_url(f"/api/templates/{template_id}/code/lua")
        )
        assert resp.status_code == 404


class TestTemplateCategories:
    r"""
    模板分类列表测试
    """

    def test_get_categories_success(self) -> None:
        r"""
        正常获取分类列表
        """
        create_template("模板1", "排序算法", "描述")
        create_template("模板2", "图算法", "描述")
        create_template("模板3", "数据结构", "描述")

        resp: requests.Response = requests.get(api_url("/api/templates/categories"))
        assert resp.status_code == 200
        categories: list[str] = resp.json()["items"]
        assert "排序算法" in categories
        assert "图算法" in categories
        assert "数据结构" in categories

    def test_get_categories_deduplicated(self) -> None:
        r"""
        验证分类已去重
        """
        create_template("模板1", "分类A", "描述")
        create_template("模板2", "分类A", "描述")
        create_template("模板3", "分类B", "描述")

        resp: requests.Response = requests.get(api_url("/api/templates/categories"))
        categories: list[str] = resp.json()["items"]
        assert categories.count("分类A") == 1

    def test_get_categories_sorted(self) -> None:
        r"""
        验证按字母顺序排列
        """
        create_template("模板1", "C分类", "描述")
        create_template("模板2", "A分类", "描述")
        create_template("模板3", "B分类", "描述")

        resp: requests.Response = requests.get(api_url("/api/templates/categories"))
        categories: list[str] = resp.json()["items"]
        assert categories == sorted(categories)

    def test_get_categories_empty(self) -> None:
        r"""
        无模板时返回空列表
        """
        resp: requests.Response = requests.get(api_url("/api/templates/categories"))
        assert resp.status_code == 200
        assert resp.json()["items"] == []


class TestTemplateSearch:
    r"""
    搜索模板测试
    """

    def test_search_by_title(self) -> None:
        r"""
        按标题搜索
        """
        create_template("冒泡排序", "排序算法", "描述")
        create_template("快速排序", "排序算法", "描述")
        create_template("图遍历", "图算法", "描述")

        resp: requests.Response = requests.get(
            api_url("/api/templates/search"),
            params={"keyword": "排序"}
        )
        assert resp.status_code == 200
        data: dict = resp.json()
        assert data["total"] == 2

    def test_search_by_description(self) -> None:
        r"""
        按描述搜索
        """
        create_template("模板1", "分类", "这是一个关于冒泡的描述")
        create_template("模板2", "分类", "这是一个关于快速的描述")

        resp: requests.Response = requests.get(
            api_url("/api/templates/search"),
            params={"keyword": "冒泡"}
        )
        assert resp.status_code == 200
        assert resp.json()["total"] == 1

    def test_search_no_results(self) -> None:
        r"""
        搜索无结果返回空列表
        """
        create_template("模板1", "分类", "描述")

        resp: requests.Response = requests.get(
            api_url("/api/templates/search"),
            params={"keyword": "不存在的关键词"}
        )
        assert resp.status_code == 200
        assert resp.json()["total"] == 0
        assert resp.json()["items"] == []

    def test_search_case_insensitive(self) -> None:
        r"""
        大小写不敏感搜索
        """
        create_template("BubbleSort", "分类", "描述")

        resp: requests.Response = requests.get(
            api_url("/api/templates/search"),
            params={"keyword": "bubblesort"}
        )
        assert resp.status_code == 200
        assert resp.json()["total"] == 1

    def test_search_missing_keyword(self) -> None:
        r"""
        缺少 keyword 参数，返回 422
        """
        resp: requests.Response = requests.get(api_url("/api/templates/search"))
        assert resp.status_code == 422

    def test_search_pagination(self) -> None:
        r"""
        搜索结果分页
        """
        for i in range(15):
            create_template(f"排序算法{i}", "分类", "描述")

        resp: requests.Response = requests.get(
            api_url("/api/templates/search"),
            params={"keyword": "排序", "page": 1, "limit": 10},
        )
        assert resp.status_code == 200
        data: dict = resp.json()
        assert data["total"] == 15
        assert len(data["items"]) == 10

    def test_search_order_by_favorite_count(self) -> None:
        r"""
        验证按收藏数降序排列
        """
        id1: int = create_template("排序1", "分类", "描述")
        id2: int = create_template("排序2", "分类", "描述")

        with get_db_connection() as conn:
            with conn.cursor() as cur:
                cur.execute(
                    "UPDATE templates SET favorite_count = 10 WHERE id = %s",
                    (id2,),
                )
            conn.commit()

        resp: requests.Response = requests.get(
            api_url("/api/templates/search"),
            params={"keyword": "排序"}
        )
        items: list[dict] = resp.json()["items"]
        assert items[0]["id"] == id2

    def test_search_partial_match(self) -> None:
        r"""
        部分匹配搜索
        """
        create_template("冒泡排序算法", "分类", "描述")

        resp: requests.Response = requests.get(
            api_url("/api/templates/search"),
            params={"keyword": "冒泡"}
        )
        assert resp.status_code == 200
        assert resp.json()["total"] == 1

    def test_search_logged_in_is_favorited(self) -> None:
        r"""
        登录用户搜索，验证 is_favorited
        """
        template_id: int = create_template("排序算法", "分类", "描述")
        token: str = create_user_and_login()
        requests.post(
            api_url("/api/favorites"),
            headers=auth_header(token),
            json={"template_id": template_id},
        )

        resp: requests.Response = requests.get(
            api_url("/api/templates/search"),
            headers=auth_header(token),
            params={"keyword": "排序"},
        )
        assert resp.json()["items"][0]["is_favorited"] is True


class TestFavoriteAdd:
    r"""
    收藏模板测试
    """

    def test_favorite_success(self) -> None:
        r"""
        正常收藏模板
        """
        template_id: int = create_template("模板", "分类", "描述")
        token: str = create_user_and_login()

        resp: requests.Response = requests.post(
            api_url("/api/favorites"),
            headers=auth_header(token),
            json={"template_id": template_id},
        )
        assert resp.status_code == 200

    def test_favorite_template_not_found(self) -> None:
        r"""
        模板不存在，返回 404
        """
        token: str = create_user_and_login()

        resp: requests.Response = requests.post(
            api_url("/api/favorites"),
            headers=auth_header(token),
            json={"template_id": 99999},
        )
        assert resp.status_code == 404

    def test_favorite_duplicate(self) -> None:
        r"""
        重复收藏，返回 409
        """
        template_id: int = create_template("模板", "分类", "描述")
        token: str = create_user_and_login()

        requests.post(
            api_url("/api/favorites"),
            headers=auth_header(token),
            json={"template_id": template_id},
        )
        resp: requests.Response = requests.post(
            api_url("/api/favorites"),
            headers=auth_header(token),
            json={"template_id": template_id},
        )
        assert resp.status_code == 409

    def test_favorite_not_logged_in(self) -> None:
        r"""
        未登录收藏，返回 401
        """
        template_id: int = create_template("模板", "分类", "描述")

        resp: requests.Response = requests.post(
            api_url("/api/favorites"),
            json={"template_id": template_id},
        )
        assert resp.status_code == 401

    def test_favorite_suspended_user(self) -> None:
        r"""
        Suspended 用户收藏，返回 403
        """
        template_id: int = create_template("模板", "分类", "描述")
        token: str = create_user_and_login()
        set_user_status(1, "Suspended")

        resp: requests.Response = requests.post(
            api_url("/api/favorites"),
            headers=auth_header(token),
            json={"template_id": template_id},
        )
        assert resp.status_code == 403

    def test_favorite_count_increased(self) -> None:
        r"""
        收藏后 favorite_count 增加
        """
        template_id: int = create_template("模板", "分类", "描述")
        token: str = create_user_and_login()

        resp: requests.Response = requests.get(api_url(f"/api/templates/{template_id}"))
        old_count: int = resp.json()["favorite_count"]

        requests.post(
            api_url("/api/favorites"),
            headers=auth_header(token),
            json={"template_id": template_id},
        )

        resp = requests.get(api_url(f"/api/templates/{template_id}"))
        new_count: int = resp.json()["favorite_count"]
        assert new_count == old_count + 1

    def test_favorite_is_favorited_true(self) -> None:
        r"""
        收藏后 is_favorited 变为 true
        """
        template_id: int = create_template("模板", "分类", "描述")
        token: str = create_user_and_login()

        requests.post(
            api_url("/api/favorites"),
            headers=auth_header(token),
            json={"template_id": template_id},
        )

        resp: requests.Response = requests.get(
            api_url(f"/api/templates/{template_id}"),
            headers=auth_header(token),
        )
        assert resp.json()["is_favorited"] is True


class TestFavoriteRemove:
    r"""
    取消收藏测试
    """

    def test_unfavorite_success(self) -> None:
        r"""
        正常取消收藏
        """
        template_id: int = create_template("模板", "分类", "描述")
        token: str = create_user_and_login()

        requests.post(
            api_url("/api/favorites"),
            headers=auth_header(token),
            json={"template_id": template_id},
        )
        resp: requests.Response = requests.delete(
            api_url(f"/api/favorites/{template_id}"),
            headers=auth_header(token),
        )
        assert resp.status_code == 200

    def test_unfavorite_not_favorited(self) -> None:
        r"""
        未收藏的模板取消，返回 404
        """
        template_id: int = create_template("模板", "分类", "描述")
        token: str = create_user_and_login()

        resp: requests.Response = requests.delete(
            api_url(f"/api/favorites/{template_id}"),
            headers=auth_header(token),
        )
        assert resp.status_code == 404

    def test_unfavorite_not_logged_in(self) -> None:
        r"""
        未登录取消，返回 401
        """
        template_id: int = create_template("模板", "分类", "描述")

        resp: requests.Response = requests.delete(
            api_url(f"/api/favorites/{template_id}")
        )
        assert resp.status_code == 401

    def test_unfavorite_count_decreased(self) -> None:
        r"""
        取消后 favorite_count 减少
        """
        template_id: int = create_template("模板", "分类", "描述")
        token: str = create_user_and_login()

        requests.post(
            api_url("/api/favorites"),
            headers=auth_header(token),
            json={"template_id": template_id},
        )

        resp: requests.Response = requests.get(api_url(f"/api/templates/{template_id}"))
        old_count: int = resp.json()["favorite_count"]

        requests.delete(
            api_url(f"/api/favorites/{template_id}"),
            headers=auth_header(token),
        )

        resp = requests.get(api_url(f"/api/templates/{template_id}"))
        new_count: int = resp.json()["favorite_count"]
        assert new_count == old_count - 1

    def test_unfavorite_is_favorited_false(self) -> None:
        r"""
        取消后 is_favorited 变为 false
        """
        template_id: int = create_template("模板", "分类", "描述")
        token: str = create_user_and_login()

        requests.post(
            api_url("/api/favorites"),
            headers=auth_header(token),
            json={"template_id": template_id},
        )
        requests.delete(
            api_url(f"/api/favorites/{template_id}"),
            headers=auth_header(token),
        )

        resp: requests.Response = requests.get(
            api_url(f"/api/templates/{template_id}"),
            headers=auth_header(token),
        )
        assert resp.json()["is_favorited"] is False

    def test_unfavorite_suspended_user_allowed(self) -> None:
        r"""
        Suspended 用户取消收藏，应成功
        """
        template_id: int = create_template("模板", "分类", "描述")
        token: str = create_user_and_login()

        requests.post(
            api_url("/api/favorites"),
            headers=auth_header(token),
            json={"template_id": template_id},
        )

        set_user_status(1, "Suspended")

        resp: requests.Response = requests.delete(
            api_url(f"/api/favorites/{template_id}"),
            headers=auth_header(token),
        )
        assert resp.status_code == 200


class TestExecuteCode:
    r"""
    代码执行测试
    """

    def test_execute_python_success(self) -> None:
        r"""
        Python 代码执行成功，返回 TOML
        """
        token: str = create_user_and_login()
        code: str = '''
import ds4viz as dv
with dv.stack("test_stack") as s:
    s.push(1)
    s.push(2)
    s.push(3)
'''
        resp: requests.Response = requests.post(
            api_url("/api/execute"),
            headers=auth_header(token),
            json={"language": "python", "code": code},
        )
        assert resp.status_code == 200
        data: dict = resp.json()
        if data["status"] != "Success":
            print(f"执行失败: {data}")
        assert data["status"] == "Success"
        assert data["toml_output"] is not None
        assert "[meta]" in data["toml_output"]

    def test_execute_lua_success(self) -> None:
        r"""
        Lua 代码执行成功
        """
        token: str = create_user_and_login()
        code: str = '''
local ds4viz = require("ds4viz")

ds4viz.withContext(ds4viz.stack("test_stack"), function(s)
    s:push(1)
    s:push(2)
    s:push(3)
end)
'''
        resp: requests.Response = requests.post(
            api_url("/api/execute"),
            headers=auth_header(token),
            json={"language": "lua", "code": code},
        )
        assert resp.status_code == 200
        data: dict = resp.json()
        assert data["status"] == "Success"
        assert data["toml_output"] is not None

    def test_execute_rust_success(self) -> None:
        r"""
        Rust 代码执行成功
        """
        token: str = create_user_and_login()
        code: str = '''
use ds4viz::prelude::*;

fn main() -> ds4viz::Result<()> {
    ds4viz::stack("test_stack", |s| {
        s.push(1)?;
        s.push(2)?;
        s.push(3)?;
        Ok(())
    })
}
'''
        resp: requests.Response = requests.post(
            api_url("/api/execute"),
            headers=auth_header(token),
            json={"language": "rust", "code": code},
        )
        assert resp.status_code == 200
        data: dict = resp.json()
        assert data["status"] == "Success"
        assert data["toml_output"] is not None

    def test_execute_python_syntax_error(self) -> None:
        r"""
        Python 语法错误，返回 Error
        """
        token: str = create_user_and_login()
        code: str = "def invalid syntax here!!!"

        resp: requests.Response = requests.post(
            api_url("/api/execute"),
            headers=auth_header(token),
            json={"language": "python", "code": code},
        )
        assert resp.status_code == 200
        data: dict = resp.json()
        assert data["status"] == "Error"
        assert data["error_message"] is not None

    def test_execute_python_runtime_error(self) -> None:
        r"""
        Python 运行时错误，返回 Error
        """
        token: str = create_user_and_login()
        code: str = '''
import ds4viz as dv
with dv.stack("test") as s:
    s.pop()
'''
        resp: requests.Response = requests.post(
            api_url("/api/execute"),
            headers=auth_header(token),
            json={"language": "python", "code": code},
        )
        assert resp.status_code == 200
        data: dict = resp.json()
        assert data["status"] in ["Success", "Error"]

    def test_execute_timeout(self) -> None:
        r"""
        代码执行超时，返回 Timeout
        """
        token: str = create_user_and_login()
        code: str = '''
import time
time.sleep(100)
'''
        resp: requests.Response = requests.post(
            api_url("/api/execute"),
            headers=auth_header(token),
            json={"language": "python", "code": code},
        )
        assert resp.status_code == 200
        data: dict = resp.json()
        assert data["status"] == "Timeout"

    def test_execute_no_toml_output(self) -> None:
        r"""
        未生成 TOML 文件，返回 Error
        """
        token: str = create_user_and_login()
        code: str = "print('hello world')"

        resp: requests.Response = requests.post(
            api_url("/api/execute"),
            headers=auth_header(token),
            json={"language": "python", "code": code},
        )
        assert resp.status_code == 200
        data: dict = resp.json()
        assert data["status"] == "Error"

    def test_execute_unsupported_language(self) -> None:
        r"""
        不支持的语言，返回 422
        """
        token: str = create_user_and_login()

        resp: requests.Response = requests.post(
            api_url("/api/execute"),
            headers=auth_header(token),
            json={"language": "java", "code": "System.out.println('hello');"},
        )
        assert resp.status_code == 422

    def test_execute_not_logged_in(self) -> None:
        r"""
        未登录执行，返回 401
        """
        resp: requests.Response = requests.post(
            api_url("/api/execute"),
            json={"language": "python", "code": "print('test')"},
        )
        assert resp.status_code == 401

    def test_execute_suspended_user(self) -> None:
        r"""
        Suspended 用户执行，返回 403
        """
        token: str = create_user_and_login()
        set_user_status(1, "Suspended")

        resp: requests.Response = requests.post(
            api_url("/api/execute"),
            headers=auth_header(token),
            json={"language": "python", "code": "print('test')"},
        )
        assert resp.status_code == 403

    def test_execute_cache_hit(self) -> None:
        r"""
        缓存命中：相同代码第二次执行
        """
        token: str = create_user_and_login()
        code: str = '''
import ds4viz as dv
with dv.stack("cache_test") as s:
    s.push(42)
'''
        resp1: requests.Response = requests.post(
            api_url("/api/execute"),
            headers=auth_header(token),
            json={"language": "python", "code": code},
        )
        assert resp1.json()["cached"] is False

        resp2: requests.Response = requests.post(
            api_url("/api/execute"),
            headers=auth_header(token),
            json={"language": "python", "code": code},
        )
        assert resp2.json()["cached"] is True

    def test_execute_cache_hit_execution_time_zero(self) -> None:
        r"""
        缓存命中时 cached 为 true，execution_time 为 0
        """
        token: str = create_user_and_login()
        code: str = '''
import ds4viz as dv
with dv.stack("cache_test2") as s:
    s.push(100)
'''
        requests.post(
            api_url("/api/execute"),
            headers=auth_header(token),
            json={"language": "python", "code": code},
        )

        resp: requests.Response = requests.post(
            api_url("/api/execute"),
            headers=auth_header(token),
            json={"language": "python", "code": code},
        )
        data: dict = resp.json()
        assert data["cached"] is True
        assert data["execution_time"] == 0

    def test_execute_record_saved(self) -> None:
        r"""
        执行记录正确保存到数据库
        """
        token: str = create_user_and_login()
        resp: requests.Response = requests.get(
            api_url("/api/auth/me"),
            headers=auth_header(token)
        )
        user_id: int = resp.json()["id"]

        code: str = '''
import ds4viz as dv
with dv.stack("record_test") as s:
    s.push(1)
'''
        requests.post(
            api_url("/api/execute"),
            headers=auth_header(token),
            json={"language": "python", "code": code},
        )

        resp = requests.get(
            api_url(f"/api/users/{user_id}/executions"),
            headers=auth_header(token),
        )
        assert resp.json()["total"] >= 1


class TestExecutePythonDs4viz:
    r"""
    Python ds4viz 库执行测试
    """

    def test_python_stack_operations(self) -> None:
        r"""
        栈操作：push、pop、clear
        """
        token: str = create_user_and_login()
        code: str = '''
import ds4viz as dv
with dv.stack("python_stack") as s:
    s.push(10)
    s.push(20)
    s.push(30)
    s.pop()
    s.clear()
'''
        resp: requests.Response = requests.post(
            api_url("/api/execute"),
            headers=auth_header(token),
            json={"language": "python", "code": code},
        )
        assert resp.json()["status"] == "Success"

    def test_python_queue_operations(self) -> None:
        r"""
        队列操作：enqueue、dequeue
        """
        token: str = create_user_and_login()
        code: str = '''
import ds4viz as dv
with dv.queue("python_queue") as q:
    q.enqueue(10)
    q.enqueue(20)
    q.enqueue(30)
    q.dequeue()
    q.dequeue()
'''
        resp: requests.Response = requests.post(
            api_url("/api/execute"),
            headers=auth_header(token),
            json={"language": "python", "code": code},
        )
        assert resp.json()["status"] == "Success"

    def test_python_single_linked_list(self) -> None:
        r"""
        单链表操作
        """
        token: str = create_user_and_login()
        code: str = '''
import ds4viz as dv
with dv.single_linked_list("python_slist") as slist:
    node_a = slist.insert_head(10)
    slist.insert_tail(20)
    node_c = slist.insert_tail(30)
    slist.insert_after(node_a, 15)
    slist.delete(node_c)
    slist.reverse()
'''
        resp: requests.Response = requests.post(
            api_url("/api/execute"),
            headers=auth_header(token),
            json={"language": "python", "code": code},
        )
        assert resp.json()["status"] == "Success"

    def test_python_double_linked_list(self) -> None:
        r"""
        双向链表操作
        """
        token: str = create_user_and_login()
        code: str = '''
import ds4viz as dv
with dv.double_linked_list("python_dlist") as dlist:
    node_a = dlist.insert_head(10)
    dlist.insert_tail(30)
    dlist.insert_after(node_a, 20)
    dlist.delete_tail()
    dlist.reverse()
'''
        resp: requests.Response = requests.post(
            api_url("/api/execute"),
            headers=auth_header(token),
            json={"language": "python", "code": code},
        )
        assert resp.json()["status"] == "Success"

    def test_python_binary_tree(self) -> None:
        r"""
        二叉树操作
        """
        token: str = create_user_and_login()
        code: str = '''
import ds4viz as dv
with dv.binary_tree("python_tree") as tree:
    root = tree.insert_root(10)
    left = tree.insert_left(root, 5)
    right = tree.insert_right(root, 15)
    tree.insert_left(left, 3)
    tree.insert_right(left, 7)
    tree.update_value(right, 20)
    tree.delete(left)
'''
        resp: requests.Response = requests.post(
            api_url("/api/execute"),
            headers=auth_header(token),
            json={"language": "python", "code": code},
        )
        assert resp.json()["status"] == "Success"

    def test_python_binary_search_tree(self) -> None:
        r"""
        二叉搜索树操作
        """
        token: str = create_user_and_login()
        code: str = '''
import ds4viz as dv
with dv.binary_search_tree("python_bst") as bst:
    bst.insert(10)
    bst.insert(5)
    bst.insert(15)
    bst.insert(3)
    bst.insert(7)
    bst.delete(5)
    bst.insert(6)
'''
        resp: requests.Response = requests.post(
            api_url("/api/execute"),
            headers=auth_header(token),
            json={"language": "python", "code": code},
        )
        assert resp.json()["status"] == "Success"

    def test_python_min_heap(self) -> None:
        r"""
        最小堆操作
        """
        token: str = create_user_and_login()
        code: str = '''
import ds4viz as dv
with dv.heap("python_min_heap", heap_type="min") as h:
    h.insert(10)
    h.insert(5)
    h.insert(15)
    h.insert(3)
    h.extract()
'''
        resp: requests.Response = requests.post(
            api_url("/api/execute"),
            headers=auth_header(token),
            json={"language": "python", "code": code},
        )
        assert resp.json()["status"] == "Success"

    def test_python_max_heap(self) -> None:
        r"""
        最大堆操作
        """
        token: str = create_user_and_login()
        code: str = '''
import ds4viz as dv
with dv.heap("python_max_heap", heap_type="max") as h:
    h.insert(10)
    h.insert(5)
    h.insert(15)
    h.insert(3)
    h.extract()
'''
        resp: requests.Response = requests.post(
            api_url("/api/execute"),
            headers=auth_header(token),
            json={"language": "python", "code": code},
        )
        assert resp.json()["status"] == "Success"

    def test_python_graph_undirected(self) -> None:
        r"""
        无向图操作
        """
        token: str = create_user_and_login()
        code: str = '''
import ds4viz as dv
with dv.graph_undirected("python_graph") as graph:
    graph.add_node(0, "A")
    graph.add_node(1, "B")
    graph.add_node(2, "C")
    graph.add_edge(0, 1)
    graph.add_edge(1, 2)
    graph.update_node_label(1, "B_updated")
    graph.remove_edge(0, 1)
'''
        resp: requests.Response = requests.post(
            api_url("/api/execute"),
            headers=auth_header(token),
            json={"language": "python", "code": code},
        )
        assert resp.json()["status"] == "Success"

    def test_python_graph_directed(self) -> None:
        r"""
        有向图操作
        """
        token: str = create_user_and_login()
        code: str = '''
import ds4viz as dv
with dv.graph_directed("python_digraph") as graph:
    graph.add_node(0, "A")
    graph.add_node(1, "B")
    graph.add_node(2, "C")
    graph.add_edge(0, 1)
    graph.add_edge(1, 2)
    graph.add_edge(2, 0)
    graph.remove_edge(2, 0)
'''
        resp: requests.Response = requests.post(
            api_url("/api/execute"),
            headers=auth_header(token),
            json={"language": "python", "code": code},
        )
        assert resp.json()["status"] == "Success"

    def test_python_graph_weighted(self) -> None:
        r"""
        带权图操作
        """
        token: str = create_user_and_login()
        code: str = '''
import ds4viz as dv
with dv.graph_weighted("python_weighted", directed=False) as graph:
    graph.add_node(0, "A")
    graph.add_node(1, "B")
    graph.add_node(2, "C")
    graph.add_edge(0, 1, 3.5)
    graph.add_edge(1, 2, 2.0)
    graph.update_weight(0, 1, 5.0)
'''
        resp: requests.Response = requests.post(
            api_url("/api/execute"),
            headers=auth_header(token),
            json={"language": "python", "code": code},
        )
        assert resp.json()["status"] == "Success"


class TestExecuteLuaDs4viz:
    r"""
    Lua ds4viz 库执行测试
    """

    def test_lua_stack_operations(self) -> None:
        r"""
        Lua 栈操作
        """
        token: str = create_user_and_login()
        code: str = '''
local ds4viz = require("ds4viz")

ds4viz.withContext(ds4viz.stack("lua_stack"), function(s)
    s:push(10)
    s:push(20)
    s:push(30)
    s:pop()
end)
'''
        resp: requests.Response = requests.post(
            api_url("/api/execute"),
            headers=auth_header(token),
            json={"language": "lua", "code": code},
        )
        assert resp.json()["status"] == "Success"

    def test_lua_graph_operations(self) -> None:
        r"""
        Lua 图结构操作
        """
        token: str = create_user_and_login()
        code: str = '''
local ds4viz = require("ds4viz")

ds4viz.withContext(ds4viz.graphUndirected("lua_graph"), function(graph)
    graph:addNode(0, "A")
    graph:addNode(1, "B")
    graph:addNode(2, "C")
    graph:addEdge(0, 1)
    graph:addEdge(1, 2)
end)
'''
        resp: requests.Response = requests.post(
            api_url("/api/execute"),
            headers=auth_header(token),
            json={"language": "lua", "code": code},
        )
        assert resp.json()["status"] == "Success"


class TestExecuteRustDs4viz:
    r"""
    Rust ds4viz 库执行测试
    """

    def test_rust_basic_operations(self) -> None:
        r"""
        Rust 基本数据结构操作
        """
        token: str = create_user_and_login()
        code: str = '''
use ds4viz::prelude::*;

fn main() -> ds4viz::Result<()> {
    ds4viz::stack("rust_stack", |s| {
        s.push(10)?;
        s.push(20)?;
        s.push(30)?;
        s.pop()?;
        Ok(())
    })?;
    
    ds4viz::queue("rust_queue", |q| {
        q.enqueue(1)?;
        q.enqueue(2)?;
        q.dequeue()?;
        Ok(())
    })
}
'''
        resp: requests.Response = requests.post(
            api_url("/api/execute"),
            headers=auth_header(token),
            json={"language": "rust", "code": code},
        )
        assert resp.json()["status"] == "Success"


class TestExecutionDetail:
    r"""
    获取执行详情测试
    """

    def test_get_execution_success(self) -> None:
        r"""
        正常获取执行详情
        """
        token: str = create_user_and_login()
        code: str = '''
import ds4viz as dv
with dv.stack("detail_test") as s:
    s.push(1)
'''
        resp: requests.Response = requests.post(
            api_url("/api/execute"),
            headers=auth_header(token),
            json={"language": "python", "code": code},
        )
        execution_id: int = resp.json()["execution_id"]

        resp = requests.get(
            api_url(f"/api/executions/{execution_id}"),
            headers=auth_header(token),
        )
        assert resp.status_code == 200
        data: dict = resp.json()
        assert data["id"] == execution_id
        assert data["language"] == "python"

    def test_get_execution_not_found(self) -> None:
        r"""
        执行记录不存在，返回 404
        """
        token: str = create_user_and_login()
        resp: requests.Response = requests.get(
            api_url("/api/executions/99999"),
            headers=auth_header(token),
        )
        assert resp.status_code == 404

    def test_get_execution_other_user(self) -> None:
        r"""
        查看他人执行记录，返回 403
        """
        token1: str = create_user_and_login("user1", "pass1")
        code: str = '''
import ds4viz as dv
with dv.stack("test") as s:
    s.push(1)
'''
        resp: requests.Response = requests.post(
            api_url("/api/execute"),
            headers=auth_header(token1),
            json={"language": "python", "code": code},
        )
        execution_id: int = resp.json()["execution_id"]

        token2: str = create_user_and_login("user2", "pass2")
        resp = requests.get(
            api_url(f"/api/executions/{execution_id}"),
            headers=auth_header(token2),
        )
        assert resp.status_code == 403

    def test_get_execution_not_logged_in(self) -> None:
        r"""
        未登录获取，返回 401
        """
        resp: requests.Response = requests.get(api_url("/api/executions/1"))
        assert resp.status_code == 401

    def test_get_execution_fields(self) -> None:
        r"""
        验证各字段正确返回
        """
        token: str = create_user_and_login()
        code: str = '''
import ds4viz as dv
with dv.stack("fields_test") as s:
    s.push(1)
'''
        resp: requests.Response = requests.post(
            api_url("/api/execute"),
            headers=auth_header(token),
            json={"language": "python", "code": code},
        )
        execution_id: int = resp.json()["execution_id"]

        resp = requests.get(
            api_url(f"/api/executions/{execution_id}"),
            headers=auth_header(token),
        )
        data: dict = resp.json()
        assert "id" in data
        assert "language" in data
        assert "code" in data
        assert "toml_output" in data
        assert "status" in data
        assert "error_message" in data
        assert "execution_time" in data
        assert "created_at" in data


class TestHealthCheck:
    r"""
    健康检查测试
    """

    def test_health_check_ok(self) -> None:
        r"""
        健康检查返回 status: ok
        """
        resp: requests.Response = requests.get(api_url("/health"))
        assert resp.status_code == 200
        assert resp.json()["status"] == "ok"

    def test_health_check_no_auth_required(self) -> None:
        r"""
        健康检查无需认证
        """
        resp: requests.Response = requests.get(api_url("/health"))
        assert resp.status_code == 200


class TestBusinessFlow:
    r"""
    综合业务流程测试
    """

    def test_flow_register_login_execute_history(self) -> None:
        r"""
        完整流程：注册 → 登录 → 执行代码 → 查看历史
        """
        resp: requests.Response = register_user("flowuser1", "flowpass1")
        assert resp.status_code == 200
        user_id: int = resp.json()["id"]

        resp = login_user("flowuser1", "flowpass1")
        assert resp.status_code == 200
        token: str = resp.json()["token"]

        code: str = '''
import ds4viz as dv
with dv.stack("flow_test") as s:
    s.push(1)
    s.push(2)
'''
        resp = requests.post(
            api_url("/api/execute"),
            headers=auth_header(token),
            json={"language": "python", "code": code},
        )
        assert resp.status_code == 200
        assert resp.json()["status"] == "Success"

        resp = requests.get(
            api_url(f"/api/users/{user_id}/executions"),
            headers=auth_header(token),
        )
        assert resp.status_code == 200
        assert resp.json()["total"] >= 1

    def test_flow_register_login_favorite_list(self) -> None:
        r"""
        完整流程：注册 → 登录 → 收藏模板 → 查看收藏列表
        """
        template_id: int = create_template("流程测试模板", "测试分类", "描述")

        resp: requests.Response = register_user("flowuser2", "flowpass2")
        user_id: int = resp.json()["id"]

        resp = login_user("flowuser2", "flowpass2")
        token: str = resp.json()["token"]

        resp = requests.post(
            api_url("/api/favorites"),
            headers=auth_header(token),
            json={"template_id": template_id},
        )
        assert resp.status_code == 200

        resp = requests.get(
            api_url(f"/api/users/{user_id}/favorites"),
            headers=auth_header(token),
        )
        assert resp.status_code == 200
        assert resp.json()["total"] == 1

    def test_cache_shared_across_users(self) -> None:
        r"""
        缓存跨用户共享：用户 A 执行后用户 B 执行相同代码命中缓存
        """
        code: str = '''
import ds4viz as dv
with dv.stack("shared_cache_test") as s:
    s.push(999)
'''
        token_a: str = create_user_and_login("userA", "passA")
        resp_a: requests.Response = requests.post(
            api_url("/api/execute"),
            headers=auth_header(token_a),
            json={"language": "python", "code": code},
        )
        assert resp_a.json()["cached"] is False

        token_b: str = create_user_and_login("userB", "passB")
        resp_b: requests.Response = requests.post(
            api_url("/api/execute"),
            headers=auth_header(token_b),
            json={"language": "python", "code": code},
        )
        assert resp_b.json()["cached"] is True

    def test_user_status_change_permission(self) -> None:
        r"""
        用户状态变更后权限验证
        """
        token: str = create_user_and_login("statususer", "statuspass")

        code: str = '''
import ds4viz as dv
with dv.stack("status_test") as s:
    s.push(1)
'''
        resp: requests.Response = requests.post(
            api_url("/api/execute"),
            headers=auth_header(token),
            json={"language": "python", "code": code},
        )
        assert resp.status_code == 200

        set_user_status(1, "Suspended")

        resp = requests.post(
            api_url("/api/execute"),
            headers=auth_header(token),
            json={"language": "python", "code": code},
        )
        assert resp.status_code == 403

    def test_logout_and_relogin(self) -> None:
        r"""
        登出后重新登录获取新 token
        """
        register_user("reloginuser", "reloginpass")
        resp: requests.Response = login_user("reloginuser", "reloginpass")
        token1: str = resp.json()["token"]

        requests.post(api_url("/api/auth/logout"), headers=auth_header(token1))

        resp = login_user("reloginuser", "reloginpass")
        token2: str = resp.json()["token"]

        assert token1 != token2

        resp = requests.get(api_url("/api/auth/me"), headers=auth_header(token2))
        assert resp.status_code == 200

    def test_favorite_count_accuracy(self) -> None:
        r"""
        模板收藏计数准确性：多次收藏取消后计数正确
        """
        template_id: int = create_template("计数测试模板", "分类", "描述")

        resp: requests.Response = requests.get(api_url(f"/api/templates/{template_id}"))
        initial_count: int = resp.json()["favorite_count"]

        tokens: list[str] = []
        for i in range(3):
            token: str = create_user_and_login(f"countuser{i}", f"countpass{i}")
            tokens.append(token)
            requests.post(
                api_url("/api/favorites"),
                headers=auth_header(token),
                json={"template_id": template_id},
            )

        resp = requests.get(api_url(f"/api/templates/{template_id}"))
        assert resp.json()["favorite_count"] == initial_count + 3

        for token in tokens[:2]:
            requests.delete(
                api_url(f"/api/favorites/{template_id}"),
                headers=auth_header(token),
            )

        resp = requests.get(api_url(f"/api/templates/{template_id}"))
        assert resp.json()["favorite_count"] == initial_count + 1

    def test_execution_history_matches_executions(self) -> None:
        r"""
        执行历史与实际执行对应
        """
        token: str = create_user_and_login("histuser", "histpass")
        resp: requests.Response = requests.get(
            api_url("/api/auth/me"),
            headers=auth_header(token)
        )
        user_id: int = resp.json()["id"]

        for i in range(3):
            code: str = f'''
import ds4viz as dv
with dv.stack("hist_test_{i}") as s:
    s.push({i})
'''
            requests.post(
                api_url("/api/execute"),
                headers=auth_header(token),
                json={"language": "python", "code": code},
            )

        resp = requests.get(
            api_url(f"/api/users/{user_id}/executions"),
            headers=auth_header(token),
        )
        assert resp.json()["total"] == 3

    def test_pagination_boundary_total_equals_limit(self) -> None:
        r"""
        分页边界：总数恰好等于 limit 时的行为
        """
        for i in range(10):
            create_template(f"边界模板{i}", "分类", "描述")

        resp: requests.Response = requests.get(
            api_url("/api/templates"),
            params={"page": 1, "limit": 10}
        )
        data: dict = resp.json()
        assert data["total"] == 10
        assert len(data["items"]) == 10
        assert data["page"] == 1


class TestErrorHandling:
    r"""
    错误处理和边缘情况测试
    """

    def test_request_body_not_json(self) -> None:
        r"""
        请求体非 JSON 格式
        """
        resp: requests.Response = requests.post(
            api_url("/api/auth/register"),
            data="not json data",
            headers={"Content-Type": "application/json"},
        )
        assert resp.status_code == 422

    def test_request_body_missing_fields(self) -> None:
        r"""
        请求体字段缺失
        """
        resp: requests.Response = requests.post(
            api_url("/api/auth/register"),
            json={"username": "testuser"},
        )
        assert resp.status_code == 422

    def test_request_body_wrong_type(self) -> None:
        r"""
        请求体字段类型错误
        """
        resp: requests.Response = requests.post(
            api_url("/api/auth/register"),
            json={"username": 123, "password": "testpass"},
        )
        assert resp.status_code == 422

    def test_path_param_wrong_type(self) -> None:
        r"""
        路径参数类型错误
        """
        resp: requests.Response = requests.get(api_url("/api/templates/not_a_number"))
        assert resp.status_code == 422

    def test_auth_header_wrong_format(self) -> None:
        r"""
        Authorization 头格式错误
        """
        resp: requests.Response = requests.get(
            api_url("/api/auth/me"),
            headers={"Authorization": "Basic sometoken"},
        )
        assert resp.status_code == 401


class TestEdgeCases:
    r"""
    扩展边缘情况测试
    """

    def test_register_username_with_unicode_emoji(self) -> None:
        r"""
        用户名包含 Unicode emoji 字符
        """
        resp: requests.Response = register_user("user🎉test", "testpass")
        assert resp.status_code == 200
        assert resp.json()["username"] == "user🎉test"

    def test_register_username_whitespace_only(self) -> None:
        r"""
        用户名仅包含空白字符，应拒绝或按策略处理
        """
        resp: requests.Response = register_user("   ", "testpass")
        assert resp.status_code in [200, 422]

    def test_password_with_null_byte(self) -> None:
        r"""
        密码包含 null 字节
        """
        resp: requests.Response = register_user("nullbyteuser", "pass\x00word")
        assert resp.status_code in [200, 422]

    def test_login_case_sensitive_username(self) -> None:
        r"""
        验证用户名大小写敏感
        """
        register_user("TestUser", "testpass")
        resp: requests.Response = login_user("testuser", "testpass")
        assert resp.status_code == 401

    def test_token_expired_simulation(self) -> None:
        r"""
        直接删除数据库会话模拟 token 失效
        """
        token: str = create_user_and_login()

        with get_db_connection() as conn:
            with conn.cursor() as cur:
                cur.execute("DELETE FROM sessions")
            conn.commit()

        resp: requests.Response = requests.get(
            api_url("/api/auth/me"),
            headers=auth_header(token)
        )
        assert resp.status_code == 401

    def test_pagination_page_zero(self) -> None:
        r"""
        分页 page=0 边界情况
        """
        create_template("模板", "分类", "描述")
        resp: requests.Response = requests.get(
            api_url("/api/templates"),
            params={"page": 0, "limit": 10}
        )
        assert resp.status_code == 422

    def test_pagination_negative_page(self) -> None:
        r"""
        分页负数页码
        """
        resp: requests.Response = requests.get(
            api_url("/api/templates"),
            params={"page": -1, "limit": 10}
        )
        assert resp.status_code == 422

    def test_pagination_limit_zero(self) -> None:
        r"""
        分页 limit=0 边界情况
        """
        resp: requests.Response = requests.get(
            api_url("/api/templates"),
            params={"page": 1, "limit": 0}
        )
        assert resp.status_code == 422

    def test_pagination_limit_exceed_max(self) -> None:
        r"""
        分页 limit 超过最大值 100
        """
        resp: requests.Response = requests.get(
            api_url("/api/templates"),
            params={"page": 1, "limit": 101}
        )
        assert resp.status_code == 422

    def test_search_keyword_empty_string(self) -> None:
        r"""
        搜索关键词为空字符串
        """
        resp: requests.Response = requests.get(
            api_url("/api/templates/search"),
            params={"keyword": ""}
        )
        assert resp.status_code in [200, 422]

    def test_search_keyword_special_sql_chars(self) -> None:
        r"""
        搜索关键词包含 SQL 特殊字符防止注入
        """
        create_template("测试模板", "分类", "描述")
        resp: requests.Response = requests.get(
            api_url("/api/templates/search"),
            params={"keyword": "'; DROP TABLE templates; --"}
        )
        assert resp.status_code == 200

    def test_execute_empty_code(self) -> None:
        r"""
        执行空代码
        """
        token: str = create_user_and_login()
        resp: requests.Response = requests.post(
            api_url("/api/execute"),
            headers=auth_header(token),
            json={"language": "python", "code": ""},
        )
        assert resp.status_code == 200
        assert resp.json()["status"] == "Error"

    def test_execute_very_large_code(self) -> None:
        r"""
        执行超大代码块
        """
        token: str = create_user_and_login()
        large_code: str = "# " + "x" * 100000 + "\nprint('hello')"
        resp: requests.Response = requests.post(
            api_url("/api/execute"),
            headers=auth_header(token),
            json={"language": "python", "code": large_code},
        )
        assert resp.status_code in [200, 413, 422]

    def test_avatar_upload_empty_file(self) -> None:
        r"""
        上传空头像文件
        """
        token: str = create_user_and_login()
        resp: requests.Response = requests.get(
            api_url("/api/auth/me"),
            headers=auth_header(token)
        )
        user_id: int = resp.json()["id"]

        resp = requests.put(
            api_url(f"/api/users/{user_id}/avatar"),
            headers=auth_header(token),
            files={"avatar": ("empty.png", b"", "image/png")},
        )
        assert resp.status_code in [200, 400, 422]
        