r"""
ds4viz 功能测试

:file: test/function_test.py
"""

import time
from typing import Any

import psycopg
import pytest
import requests

# =============================================================================
# 测试配置
# =============================================================================

BASE_URL = "http://127.0.0.1:20000"

DB_CONFIG = {
    "host": "localhost",
    "port": 5432,
    "dbname": "ds4viz_test",
    "user": "ds4viz_test",
    "password": "test_pwd_123",
}


# =============================================================================
# 辅助函数
# =============================================================================


def get_db_connection() -> psycopg.Connection:
    """获取数据库连接"""
    return psycopg.connect(**DB_CONFIG)


def reset_database() -> None:
    """重置数据库到初始状态"""
    with get_db_connection() as conn:
        with conn.cursor() as cur:
            cur.execute("DELETE FROM logs")
            cur.execute("DELETE FROM execution_cache")
            cur.execute("DELETE FROM executions")
            cur.execute("DELETE FROM user_favorites")
            cur.execute("DELETE FROM template_codes")
            cur.execute("DELETE FROM templates")
            cur.execute("DELETE FROM sessions")
            cur.execute("DELETE FROM users")
            cur.execute("ALTER SEQUENCE users_id_seq RESTART WITH 1")
            cur.execute("ALTER SEQUENCE templates_id_seq RESTART WITH 1")
            cur.execute("ALTER SEQUENCE executions_id_seq RESTART WITH 1")
            cur.execute("ALTER SEQUENCE sessions_id_seq RESTART WITH 1")
            cur.execute("ALTER SEQUENCE template_codes_id_seq RESTART WITH 1")
            cur.execute("ALTER SEQUENCE user_favorites_id_seq RESTART WITH 1")
            cur.execute("ALTER SEQUENCE execution_cache_id_seq RESTART WITH 1")
            cur.execute("ALTER SEQUENCE logs_id_seq RESTART WITH 1")
        conn.commit()


def api_url(path: str) -> str:
    """构造完整 API URL"""
    return f"{BASE_URL}{path}"


def auth_header(token: str) -> dict[str, str]:
    """构造认证头"""
    return {"Authorization": f"Bearer {token}"}


def register_user(username: str, password: str) -> requests.Response:
    """注册用户"""
    return requests.post(
        api_url("/api/auth/register"),
        json={"username": username, "password": password},
    )


def login_user(username: str, password: str) -> requests.Response:
    """用户登录"""
    return requests.post(
        api_url("/api/auth/login"),
        json={"username": username, "password": password},
    )


def create_user_and_login(username: str = "testuser", password: str = "testpass") -> str:
    """创建用户并登录，返回 token"""
    register_user(username, password)
    resp = login_user(username, password)
    return resp.json()["token"]


def set_user_status(user_id: int, status: str) -> None:
    """直接修改用户状态（通过数据库）"""
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
    """直接在数据库创建模板，返回模板 ID"""
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
            template_id = cur.fetchone()[0]

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


# =============================================================================
# Fixtures
# =============================================================================


@pytest.fixture(autouse=True)
def clean_db(server_process):
    """每个测试前后清理数据库"""
    reset_database()
    yield
    reset_database()


# =============================================================================
# 1. 认证相关 API 测试
# =============================================================================


class TestAuthRegister:
    """用户注册测试"""

    def test_register_success(self):
        """正常注册：有效用户名和密码"""
        resp = register_user("testuser", "testpass123")
        assert resp.status_code == 200
        data = resp.json()
        assert data["username"] == "testuser"
        assert data["status"] == "Active"
        assert data["avatar_url"] is None
        assert "id" in data
        assert "created_at" in data

    def test_register_username_min_length(self):
        """用户名边界：恰好 3 个字符（最小）"""
        resp = register_user("abc", "testpass")
        assert resp.status_code == 200
        assert resp.json()["username"] == "abc"

    def test_register_username_max_length(self):
        """用户名边界：恰好 32 个字符（最大）"""
        username = "a" * 32
        resp = register_user(username, "testpass")
        assert resp.status_code == 200
        assert resp.json()["username"] == username

    def test_register_username_too_short(self):
        """用户名过短：2 个字符，返回 422"""
        resp = register_user("ab", "testpass")
        assert resp.status_code == 422

    def test_register_username_too_long(self):
        """用户名过长：33 个字符，返回 422"""
        username = "a" * 33
        resp = register_user(username, "testpass")
        assert resp.status_code == 422

    def test_register_password_min_length(self):
        """密码边界：恰好 1 个字符（最小）"""
        resp = register_user("testuser", "a")
        assert resp.status_code == 200

    def test_register_password_max_length(self):
        """密码边界：恰好 64 个字符（最大）"""
        password = "a" * 64
        resp = register_user("testuser", password)
        assert resp.status_code == 200

    def test_register_password_too_long(self):
        """密码过长：65 个字符，返回 422"""
        password = "a" * 65
        resp = register_user("testuser", password)
        assert resp.status_code == 422

    def test_register_password_utf8_bytes_exceed(self):
        """密码 UTF-8 字节超限：使用多字节字符使字节数超过 64"""
        # 每个中文字符 UTF-8 编码占 3 字节，22 个中文字符 = 66 字节
        password = "中" * 22
        resp = register_user("testuser", password)
        assert resp.status_code == 422

    def test_register_duplicate_username(self):
        """用户名重复注册，返回 409"""
        register_user("testuser", "testpass")
        resp = register_user("testuser", "anotherpass")
        assert resp.status_code == 409

    def test_register_username_chinese(self):
        """用户名含中文字符"""
        resp = register_user("测试用户", "testpass")
        assert resp.status_code == 200
        assert resp.json()["username"] == "测试用户"

    def test_register_username_special_chars(self):
        """用户名含特殊字符"""
        resp = register_user("test_user-01", "testpass")
        assert resp.status_code == 200
        assert resp.json()["username"] == "test_user-01"

    def test_register_empty_username(self):
        """空用户名，返回 422"""
        resp = register_user("", "testpass")
        assert resp.status_code == 422

    def test_register_empty_password(self):
        """空密码，返回 422"""
        resp = register_user("testuser", "")
        assert resp.status_code == 422

    def test_register_response_structure(self):
        """验证返回结构完整性"""
        resp = register_user("testuser", "testpass")
        data = resp.json()
        assert "id" in data
        assert "username" in data
        assert "avatar_url" in data
        assert "status" in data
        assert "created_at" in data
        assert isinstance(data["id"], int)
        assert data["id"] > 0


class TestAuthLogin:
    """用户登录测试"""

    def test_login_success(self):
        """正常登录，返回 token 和用户信息"""
        register_user("testuser", "testpass")
        resp = login_user("testuser", "testpass")
        assert resp.status_code == 200
        data = resp.json()
        assert "token" in data
        assert "user" in data
        assert "expires_at" in data
        assert data["user"]["username"] == "testuser"

    def test_login_user_not_found(self):
        """用户名不存在，返回 401"""
        resp = login_user("nonexistent", "testpass")
        assert resp.status_code == 401

    def test_login_wrong_password(self):
        """密码错误，返回 401"""
        register_user("testuser", "testpass")
        resp = login_user("testuser", "wrongpass")
        assert resp.status_code == 401

    def test_login_banned_user(self):
        """Banned 用户登录，返回 403"""
        register_user("testuser", "testpass")
        set_user_status(1, "Banned")
        resp = login_user("testuser", "testpass")
        assert resp.status_code == 403

    def test_login_suspended_user(self):
        """Suspended 用户登录，返回成功但 status 为 Suspended"""
        register_user("testuser", "testpass")
        set_user_status(1, "Suspended")
        resp = login_user("testuser", "testpass")
        assert resp.status_code == 200
        assert resp.json()["user"]["status"] == "Suspended"

    def test_login_token_format(self):
        """验证 token 格式（JWT）"""
        register_user("testuser", "testpass")
        resp = login_user("testuser", "testpass")
        token = resp.json()["token"]
        # JWT 格式: header.payload.signature
        parts = token.split(".")
        assert len(parts) == 3

    def test_login_expires_at_reasonable(self):
        """验证 expires_at 时间合理"""
        register_user("testuser", "testpass")
        resp = login_user("testuser", "testpass")
        from datetime import datetime, timedelta

        expires_at = datetime.fromisoformat(resp.json()["expires_at"])
        now = datetime.utcnow()
        # 测试配置为 1 小时，允许一些误差
        assert expires_at > now
        assert expires_at < now + timedelta(hours=2)

    def test_login_multiple_sessions(self):
        """同一用户多次登录生成不同 session"""
        register_user("testuser", "testpass")
        resp1 = login_user("testuser", "testpass")
        resp2 = login_user("testuser", "testpass")
        token1 = resp1.json()["token"]
        token2 = resp2.json()["token"]
        # 两个 token 应该不同
        assert token1 != token2


class TestAuthLogout:
    """用户登出测试"""

    def test_logout_success(self):
        """正常登出"""
        token = create_user_and_login()
        resp = requests.post(api_url("/api/auth/logout"), headers=auth_header(token))
        assert resp.status_code == 200
        assert resp.json()["message"] == "Logged out successfully"

    def test_logout_token_invalidated(self):
        """登出后原 token 立即失效"""
        token = create_user_and_login()
        requests.post(api_url("/api/auth/logout"), headers=auth_header(token))
        # 尝试使用已登出的 token
        resp = requests.get(api_url("/api/auth/me"), headers=auth_header(token))
        assert resp.status_code == 401

    def test_logout_invalid_token(self):
        """无效 token 登出，返回 401"""
        resp = requests.post(
            api_url("/api/auth/logout"),
            headers=auth_header("invalid.token.here"),
        )
        assert resp.status_code == 401

    def test_logout_no_auth_header(self):
        """无 Authorization 头登出，返回 401"""
        resp = requests.post(api_url("/api/auth/logout"))
        assert resp.status_code == 401

    def test_logout_duplicate(self):
        """重复登出同一 token，第二次返回 401"""
        token = create_user_and_login()
        resp1 = requests.post(api_url("/api/auth/logout"), headers=auth_header(token))
        assert resp1.status_code == 200
        resp2 = requests.post(api_url("/api/auth/logout"), headers=auth_header(token))
        assert resp2.status_code == 401


class TestAuthMe:
    """获取当前用户测试"""

    def test_me_success(self):
        """正常获取当前用户信息"""
        token = create_user_and_login("testuser", "testpass")
        resp = requests.get(api_url("/api/auth/me"), headers=auth_header(token))
        assert resp.status_code == 200
        data = resp.json()
        assert data["username"] == "testuser"

    def test_me_invalid_token(self):
        """无效 token，返回 401"""
        resp = requests.get(
            api_url("/api/auth/me"),
            headers=auth_header("invalid.token"),
        )
        assert resp.status_code == 401

    def test_me_no_auth_header(self):
        """无 Authorization 头，返回 401"""
        resp = requests.get(api_url("/api/auth/me"))
        assert resp.status_code == 401

    def test_me_response_structure(self):
        """验证返回结构完整"""
        token = create_user_and_login()
        resp = requests.get(api_url("/api/auth/me"), headers=auth_header(token))
        data = resp.json()
        assert "id" in data
        assert "username" in data
        assert "avatar_url" in data
        assert "status" in data
        assert "created_at" in data


# =============================================================================
# 2. 用户相关 API 测试
# =============================================================================


class TestUserAvatar:
    """用户头像测试"""

    def test_upload_avatar_first_time(self):
        """上传头像：首次上传"""
        token = create_user_and_login()
        resp = requests.get(api_url("/api/auth/me"), headers=auth_header(token))
        user_id = resp.json()["id"]

        avatar_data = b"\x89PNG\r\n\x1a\n" + b"\x00" * 100  # 模拟 PNG 头
        resp = requests.put(
            api_url(f"/api/users/{user_id}/avatar"),
            headers=auth_header(token),
            files={"avatar": ("test.png", avatar_data, "image/png")},
        )
        assert resp.status_code == 200
        assert "avatar_url" in resp.json()

    def test_upload_avatar_update(self):
        """更新头像：覆盖已有头像"""
        token = create_user_and_login()
        resp = requests.get(api_url("/api/auth/me"), headers=auth_header(token))
        user_id = resp.json()["id"]

        # 首次上传
        avatar_data1 = b"\x89PNG\r\n\x1a\n" + b"\x01" * 100
        requests.put(
            api_url(f"/api/users/{user_id}/avatar"),
            headers=auth_header(token),
            files={"avatar": ("test1.png", avatar_data1, "image/png")},
        )

        # 更新
        avatar_data2 = b"\x89PNG\r\n\x1a\n" + b"\x02" * 100
        resp = requests.put(
            api_url(f"/api/users/{user_id}/avatar"),
            headers=auth_header(token),
            files={"avatar": ("test2.png", avatar_data2, "image/png")},
        )
        assert resp.status_code == 200

    def test_get_avatar_success(self):
        """获取头像：返回正确二进制数据"""
        token = create_user_and_login()
        resp = requests.get(api_url("/api/auth/me"), headers=auth_header(token))
        user_id = resp.json()["id"]

        avatar_data = b"\x89PNG\r\n\x1a\n" + b"\xAB" * 100
        requests.put(
            api_url(f"/api/users/{user_id}/avatar"),
            headers=auth_header(token),
            files={"avatar": ("test.png", avatar_data, "image/png")},
        )

        resp = requests.get(api_url(f"/api/users/{user_id}/avatar"))
        assert resp.status_code == 200
        assert resp.content == avatar_data

    def test_get_avatar_no_avatar(self):
        """获取无头像用户的头像，返回 404"""
        token = create_user_and_login()
        resp = requests.get(api_url("/api/auth/me"), headers=auth_header(token))
        user_id = resp.json()["id"]

        resp = requests.get(api_url(f"/api/users/{user_id}/avatar"))
        assert resp.status_code == 404

    def test_get_avatar_user_not_found(self):
        """获取不存在用户的头像，返回 404"""
        resp = requests.get(api_url("/api/users/99999/avatar"))
        assert resp.status_code == 404

    def test_upload_avatar_other_user(self):
        """上传他人头像，返回 403"""
        token1 = create_user_and_login("user1", "pass1")
        create_user_and_login("user2", "pass2")

        avatar_data = b"\x89PNG\r\n\x1a\n" + b"\x00" * 100
        resp = requests.put(
            api_url("/api/users/2/avatar"),
            headers=auth_header(token1),
            files={"avatar": ("test.png", avatar_data, "image/png")},
        )
        assert resp.status_code == 403

    def test_upload_avatar_not_logged_in(self):
        """未登录上传头像，返回 401"""
        avatar_data = b"\x89PNG\r\n\x1a\n" + b"\x00" * 100
        resp = requests.put(
            api_url("/api/users/1/avatar"),
            files={"avatar": ("test.png", avatar_data, "image/png")},
        )
        assert resp.status_code == 401

    def test_upload_avatar_url_returned(self):
        """上传后 avatar_url 字段正确返回"""
        token = create_user_and_login()
        resp = requests.get(api_url("/api/auth/me"), headers=auth_header(token))
        user_id = resp.json()["id"]

        avatar_data = b"\x89PNG\r\n\x1a\n" + b"\x00" * 100
        requests.put(
            api_url(f"/api/users/{user_id}/avatar"),
            headers=auth_header(token),
            files={"avatar": ("test.png", avatar_data, "image/png")},
        )

        resp = requests.get(api_url("/api/auth/me"), headers=auth_header(token))
        assert resp.json()["avatar_url"] == f"/api/users/{user_id}/avatar"


class TestUserPassword:
    """修改密码测试"""

    def test_change_password_success(self):
        """正常修改密码"""
        token = create_user_and_login("testuser", "oldpass")
        resp = requests.get(api_url("/api/auth/me"), headers=auth_header(token))
        user_id = resp.json()["id"]

        resp = requests.put(
            api_url(f"/api/users/{user_id}/password"),
            headers=auth_header(token),
            json={"old_password": "oldpass", "new_password": "newpass"},
        )
        assert resp.status_code == 200

    def test_change_password_wrong_old(self):
        """旧密码错误，返回 400"""
        token = create_user_and_login("testuser", "oldpass")
        resp = requests.get(api_url("/api/auth/me"), headers=auth_header(token))
        user_id = resp.json()["id"]

        resp = requests.put(
            api_url(f"/api/users/{user_id}/password"),
            headers=auth_header(token),
            json={"old_password": "wrongpass", "new_password": "newpass"},
        )
        assert resp.status_code == 400

    def test_change_password_other_user(self):
        """修改他人密码，返回 403"""
        token1 = create_user_and_login("user1", "pass1")
        create_user_and_login("user2", "pass2")

        resp = requests.put(
            api_url("/api/users/2/password"),
            headers=auth_header(token1),
            json={"old_password": "pass2", "new_password": "newpass"},
        )
        assert resp.status_code == 403

    def test_change_password_too_long(self):
        """新密码过长，返回 422"""
        token = create_user_and_login("testuser", "oldpass")
        resp = requests.get(api_url("/api/auth/me"), headers=auth_header(token))
        user_id = resp.json()["id"]

        resp = requests.put(
            api_url(f"/api/users/{user_id}/password"),
            headers=auth_header(token),
            json={"old_password": "oldpass", "new_password": "a" * 65},
        )
        assert resp.status_code == 422

    def test_change_password_utf8_exceed(self):
        """新密码 UTF-8 字节超限，返回 422"""
        token = create_user_and_login("testuser", "oldpass")
        resp = requests.get(api_url("/api/auth/me"), headers=auth_header(token))
        user_id = resp.json()["id"]

        resp = requests.put(
            api_url(f"/api/users/{user_id}/password"),
            headers=auth_header(token),
            json={"old_password": "oldpass", "new_password": "中" * 22},
        )
        assert resp.status_code == 422

    def test_change_password_user_not_found(self):
        """用户不存在，返回 404"""
        token = create_user_and_login()
        resp = requests.put(
            api_url("/api/users/99999/password"),
            headers=auth_header(token),
            json={"old_password": "oldpass", "new_password": "newpass"},
        )
        assert resp.status_code == 404

    def test_change_password_not_logged_in(self):
        """未登录修改密码，返回 401"""
        resp = requests.put(
            api_url("/api/users/1/password"),
            json={"old_password": "oldpass", "new_password": "newpass"},
        )
        assert resp.status_code == 401

    def test_change_password_old_invalid(self):
        """修改后旧密码无法登录"""
        register_user("testuser", "oldpass")
        resp = login_user("testuser", "oldpass")
        token = resp.json()["token"]
        user_id = resp.json()["user"]["id"]

        requests.put(
            api_url(f"/api/users/{user_id}/password"),
            headers=auth_header(token),
            json={"old_password": "oldpass", "new_password": "newpass"},
        )

        resp = login_user("testuser", "oldpass")
        assert resp.status_code == 401

    def test_change_password_new_valid(self):
        """修改后新密码可以登录"""
        register_user("testuser", "oldpass")
        resp = login_user("testuser", "oldpass")
        token = resp.json()["token"]
        user_id = resp.json()["user"]["id"]

        requests.put(
            api_url(f"/api/users/{user_id}/password"),
            headers=auth_header(token),
            json={"old_password": "oldpass", "new_password": "newpass"},
        )

        resp = login_user("testuser", "newpass")
        assert resp.status_code == 200


class TestUserFavorites:
    """用户收藏列表测试"""

    def test_get_favorites_with_items(self):
        """正常获取收藏列表（有收藏）"""
        token = create_user_and_login()
        resp = requests.get(api_url("/api/auth/me"), headers=auth_header(token))
        user_id = resp.json()["id"]

        template_id = create_template("测试模板", "测试分类", "测试描述")
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
        data = resp.json()
        assert data["total"] == 1
        assert len(data["items"]) == 1
        assert data["items"][0]["template_id"] == template_id

    def test_get_favorites_empty(self):
        """空收藏列表"""
        token = create_user_and_login()
        resp = requests.get(api_url("/api/auth/me"), headers=auth_header(token))
        user_id = resp.json()["id"]

        resp = requests.get(
            api_url(f"/api/users/{user_id}/favorites"),
            headers=auth_header(token),
        )
        assert resp.status_code == 200
        assert resp.json()["total"] == 0
        assert resp.json()["items"] == []

    def test_get_favorites_pagination(self):
        """分页：page=1, limit=10"""
        token = create_user_and_login()
        resp = requests.get(api_url("/api/auth/me"), headers=auth_header(token))
        user_id = resp.json()["id"]

        # 创建 15 个模板并收藏
        for i in range(15):
            template_id = create_template(f"模板{i}", "分类", "描述")
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
        data = resp.json()
        assert data["total"] == 15
        assert len(data["items"]) == 10
        assert data["page"] == 1
        assert data["limit"] == 10

    def test_get_favorites_page_2(self):
        """分页：page=2"""
        token = create_user_and_login()
        resp = requests.get(api_url("/api/auth/me"), headers=auth_header(token))
        user_id = resp.json()["id"]

        for i in range(15):
            template_id = create_template(f"模板{i}", "分类", "描述")
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

    def test_get_favorites_limit_1(self):
        """limit 边界：limit=1"""
        token = create_user_and_login()
        resp = requests.get(api_url("/api/auth/me"), headers=auth_header(token))
        user_id = resp.json()["id"]

        for i in range(3):
            template_id = create_template(f"模板{i}", "分类", "描述")
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

    def test_get_favorites_limit_100(self):
        """limit 边界：limit=100"""
        token = create_user_and_login()
        resp = requests.get(api_url("/api/auth/me"), headers=auth_header(token))
        user_id = resp.json()["id"]

        resp = requests.get(
            api_url(f"/api/users/{user_id}/favorites"),
            headers=auth_header(token),
            params={"limit": 100},
        )
        assert resp.status_code == 200

    def test_get_favorites_other_user(self):
        """查看他人收藏，返回 403"""
        token1 = create_user_and_login("user1", "pass1")
        create_user_and_login("user2", "pass2")

        resp = requests.get(
            api_url("/api/users/2/favorites"),
            headers=auth_header(token1),
        )
        assert resp.status_code == 403

    def test_get_favorites_user_not_found(self):
        """用户不存在，返回 404"""
        token = create_user_and_login()
        resp = requests.get(
            api_url("/api/users/99999/favorites"),
            headers=auth_header(token),
        )
        assert resp.status_code == 404

    def test_get_favorites_not_logged_in(self):
        """未登录获取，返回 401"""
        resp = requests.get(api_url("/api/users/1/favorites"))
        assert resp.status_code == 401

    def test_get_favorites_order_by_time_desc(self):
        """验证按收藏时间倒序排列"""
        token = create_user_and_login()
        resp = requests.get(api_url("/api/auth/me"), headers=auth_header(token))
        user_id = resp.json()["id"]

        template_id1 = create_template("模板1", "分类", "描述")
        requests.post(
            api_url("/api/favorites"),
            headers=auth_header(token),
            json={"template_id": template_id1},
        )
        time.sleep(0.1)
        template_id2 = create_template("模板2", "分类", "描述")
        requests.post(
            api_url("/api/favorites"),
            headers=auth_header(token),
            json={"template_id": template_id2},
        )

        resp = requests.get(
            api_url(f"/api/users/{user_id}/favorites"),
            headers=auth_header(token),
        )
        items = resp.json()["items"]
        # 最新收藏的在前面
        assert items[0]["template_id"] == template_id2
        assert items[1]["template_id"] == template_id1


class TestUserExecutions:
    """用户执行历史测试"""

    def test_get_executions_success(self):
        """正常获取执行历史"""
        token = create_user_and_login()
        resp = requests.get(api_url("/api/auth/me"), headers=auth_header(token))
        user_id = resp.json()["id"]

        # 执行一段代码
        code = '''
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

    def test_get_executions_empty(self):
        """空执行历史"""
        token = create_user_and_login()
        resp = requests.get(api_url("/api/auth/me"), headers=auth_header(token))
        user_id = resp.json()["id"]

        resp = requests.get(
            api_url(f"/api/users/{user_id}/executions"),
            headers=auth_header(token),
        )
        assert resp.status_code == 200
        assert resp.json()["total"] == 0

    def test_get_executions_pagination(self):
        """分页测试"""
        token = create_user_and_login()
        resp = requests.get(api_url("/api/auth/me"), headers=auth_header(token))
        user_id = resp.json()["id"]

        # 执行多段代码
        for i in range(5):
            code = f'''
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

    def test_get_executions_other_user(self):
        """查看他人执行历史，返回 403"""
        token1 = create_user_and_login("user1", "pass1")
        create_user_and_login("user2", "pass2")

        resp = requests.get(
            api_url("/api/users/2/executions"),
            headers=auth_header(token1),
        )
        assert resp.status_code == 403

    def test_get_executions_not_logged_in(self):
        """未登录获取，返回 401"""
        resp = requests.get(api_url("/api/users/1/executions"))
        assert resp.status_code == 401

    def test_get_executions_order_by_time_desc(self):
        """验证按创建时间倒序排列"""
        token = create_user_and_login()
        resp = requests.get(api_url("/api/auth/me"), headers=auth_header(token))
        user_id = resp.json()["id"]

        for i in range(3):
            code = f'''
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
        items = resp.json()["items"]
        # 验证时间倒序
        for i in range(len(items) - 1):
            assert items[i]["created_at"] >= items[i + 1]["created_at"]

    def test_get_executions_various_status(self):
        """验证各状态记录都能正确返回"""
        token = create_user_and_login()
        resp = requests.get(api_url("/api/auth/me"), headers=auth_header(token))
        user_id = resp.json()["id"]

        # 成功的执行
        success_code = '''
import ds4viz as dv
with dv.stack("test") as s:
    s.push(1)
'''
        requests.post(
            api_url("/api/execute"),
            headers=auth_header(token),
            json={"language": "python", "code": success_code},
        )

        # 错误的执行
        error_code = "syntax error here!!!"
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


# =============================================================================
# 3. 模板相关 API 测试
# =============================================================================


class TestTemplateList:
    """模板列表测试"""

    def test_get_templates_success(self):
        """正常获取模板列表"""
        create_template("模板1", "分类A", "描述1")
        create_template("模板2", "分类B", "描述2")

        resp = requests.get(api_url("/api/templates"))
        assert resp.status_code == 200
        data = resp.json()
        assert data["total"] == 2
        assert len(data["items"]) == 2

    def test_get_templates_empty(self):
        """空模板列表"""
        resp = requests.get(api_url("/api/templates"))
        assert resp.status_code == 200
        assert resp.json()["total"] == 0
        assert resp.json()["items"] == []

    def test_get_templates_pagination(self):
        """分页测试"""
        for i in range(15):
            create_template(f"模板{i}", "分类", "描述")

        resp = requests.get(api_url("/api/templates"), params={"page": 1, "limit": 10})
        assert resp.status_code == 200
        data = resp.json()
        assert data["total"] == 15
        assert len(data["items"]) == 10

    def test_get_templates_filter_by_category(self):
        """按分类筛选"""
        create_template("模板1", "分类A", "描述1")
        create_template("模板2", "分类B", "描述2")
        create_template("模板3", "分类A", "描述3")

        resp = requests.get(api_url("/api/templates"), params={"category": "分类A"})
        assert resp.status_code == 200
        data = resp.json()
        assert data["total"] == 2
        for item in data["items"]:
            assert item["category"] == "分类A"

    def test_get_templates_category_not_found(self):
        """分类不存在返回空列表"""
        create_template("模板1", "分类A", "描述1")

        resp = requests.get(api_url("/api/templates"), params={"category": "不存在的分类"})
        assert resp.status_code == 200
        assert resp.json()["total"] == 0

    def test_get_templates_logged_in_is_favorited(self):
        """登录用户获取列表，验证 is_favorited 正确"""
        template_id = create_template("模板1", "分类", "描述")
        token = create_user_and_login()
        requests.post(
            api_url("/api/favorites"),
            headers=auth_header(token),
            json={"template_id": template_id},
        )

        resp = requests.get(api_url("/api/templates"), headers=auth_header(token))
        assert resp.status_code == 200
        items = resp.json()["items"]
        favorited_item = next(i for i in items if i["id"] == template_id)
        assert favorited_item["is_favorited"] is True

    def test_get_templates_not_logged_in_is_favorited_false(self):
        """未登录用户获取列表，is_favorited 为 false"""
        create_template("模板1", "分类", "描述")

        resp = requests.get(api_url("/api/templates"))
        assert resp.status_code == 200
        for item in resp.json()["items"]:
            assert item["is_favorited"] is False

    def test_get_templates_order_by_time_desc(self):
        """验证按创建时间倒序排列"""
        create_template("模板1", "分类", "描述")
        time.sleep(0.1)
        create_template("模板2", "分类", "描述")

        resp = requests.get(api_url("/api/templates"))
        items = resp.json()["items"]
        assert items[0]["title"] == "模板2"
        assert items[1]["title"] == "模板1"


class TestTemplateDetail:
    """模板详情测试"""

    def test_get_template_success(self):
        """正常获取模板详情"""
        template_id = create_template(
            "测试模板",
            "测试分类",
            "测试描述",
            codes=[
                {"language": "python", "code": "print('hello')", "explanation": "Python实现"},
                {"language": "lua", "code": "print('hello')", "explanation": None},
            ],
        )

        resp = requests.get(api_url(f"/api/templates/{template_id}"))
        assert resp.status_code == 200
        data = resp.json()
        assert data["title"] == "测试模板"
        assert data["category"] == "测试分类"
        assert len(data["codes"]) == 2

    def test_get_template_not_found(self):
        """模板不存在，返回 404"""
        resp = requests.get(api_url("/api/templates/99999"))
        assert resp.status_code == 404

    def test_get_template_codes_multi_language(self):
        """验证 codes 数组包含多语言实现"""
        template_id = create_template(
            "模板",
            "分类",
            "描述",
            codes=[
                {"language": "python", "code": "python code"},
                {"language": "lua", "code": "lua code"},
                {"language": "rust", "code": "rust code"},
            ],
        )

        resp = requests.get(api_url(f"/api/templates/{template_id}"))
        codes = resp.json()["codes"]
        languages = [c["language"] for c in codes]
        assert "python" in languages
        assert "lua" in languages
        assert "rust" in languages

    def test_get_template_explanation_null(self):
        """验证 explanation 字段可为 null"""
        template_id = create_template(
            "模板",
            "分类",
            "描述",
            codes=[{"language": "python", "code": "code", "explanation": None}],
        )

        resp = requests.get(api_url(f"/api/templates/{template_id}"))
        assert resp.json()["codes"][0]["explanation"] is None

    def test_get_template_logged_in_is_favorited(self):
        """登录用户获取，验证 is_favorited"""
        template_id = create_template("模板", "分类", "描述")
        token = create_user_and_login()
        requests.post(
            api_url("/api/favorites"),
            headers=auth_header(token),
            json={"template_id": template_id},
        )

        resp = requests.get(
            api_url(f"/api/templates/{template_id}"),
            headers=auth_header(token),
        )
        assert resp.json()["is_favorited"] is True

    def test_get_template_not_logged_in(self):
        """未登录用户获取"""
        template_id = create_template("模板", "分类", "描述")

        resp = requests.get(api_url(f"/api/templates/{template_id}"))
        assert resp.status_code == 200
        assert resp.json()["is_favorited"] is False


class TestTemplateCode:
    """获取模板代码测试"""

    def test_get_template_code_python(self):
        """获取 Python 实现"""
        template_id = create_template(
            "模板",
            "分类",
            "描述",
            codes=[{"language": "python", "code": "python code", "explanation": "说明"}],
        )

        resp = requests.get(api_url(f"/api/templates/{template_id}/code/python"))
        assert resp.status_code == 200
        data = resp.json()
        assert data["language"] == "python"
        assert data["code"] == "python code"

    def test_get_template_code_lua(self):
        """获取 Lua 实现"""
        template_id = create_template(
            "模板",
            "分类",
            "描述",
            codes=[{"language": "lua", "code": "lua code"}],
        )

        resp = requests.get(api_url(f"/api/templates/{template_id}/code/lua"))
        assert resp.status_code == 200
        assert resp.json()["language"] == "lua"

    def test_get_template_code_rust(self):
        """获取 Rust 实现"""
        template_id = create_template(
            "模板",
            "分类",
            "描述",
            codes=[{"language": "rust", "code": "rust code"}],
        )

        resp = requests.get(api_url(f"/api/templates/{template_id}/code/rust"))
        assert resp.status_code == 200
        assert resp.json()["language"] == "rust"

    def test_get_template_code_template_not_found(self):
        """模板不存在，返回 404"""
        resp = requests.get(api_url("/api/templates/99999/code/python"))
        assert resp.status_code == 404

    def test_get_template_code_language_not_found(self):
        """语言实现不存在，返回 404"""
        template_id = create_template(
            "模板",
            "分类",
            "描述",
            codes=[{"language": "python", "code": "code"}],
        )

        resp = requests.get(api_url(f"/api/templates/{template_id}/code/lua"))
        assert resp.status_code == 404


class TestTemplateCategories:
    """模板分类列表测试"""

    def test_get_categories_success(self):
        """正常获取分类列表"""
        create_template("模板1", "排序算法", "描述")
        create_template("模板2", "图算法", "描述")
        create_template("模板3", "数据结构", "描述")

        resp = requests.get(api_url("/api/templates/categories"))
        assert resp.status_code == 200
        categories = resp.json()["items"]
        assert "排序算法" in categories
        assert "图算法" in categories
        assert "数据结构" in categories

    def test_get_categories_deduplicated(self):
        """验证分类已去重"""
        create_template("模板1", "分类A", "描述")
        create_template("模板2", "分类A", "描述")
        create_template("模板3", "分类B", "描述")

        resp = requests.get(api_url("/api/templates/categories"))
        categories = resp.json()["items"]
        assert categories.count("分类A") == 1

    def test_get_categories_sorted(self):
        """验证按字母顺序排列"""
        create_template("模板1", "C分类", "描述")
        create_template("模板2", "A分类", "描述")
        create_template("模板3", "B分类", "描述")

        resp = requests.get(api_url("/api/templates/categories"))
        categories = resp.json()["items"]
        assert categories == sorted(categories)

    def test_get_categories_empty(self):
        """无模板时返回空列表"""
        resp = requests.get(api_url("/api/templates/categories"))
        assert resp.status_code == 200
        assert resp.json()["items"] == []


class TestTemplateSearch:
    """搜索模板测试"""

    def test_search_by_title(self):
        """按标题搜索"""
        create_template("冒泡排序", "排序算法", "描述")
        create_template("快速排序", "排序算法", "描述")
        create_template("图遍历", "图算法", "描述")

        resp = requests.get(api_url("/api/templates/search"), params={"keyword": "排序"})
        assert resp.status_code == 200
        data = resp.json()
        assert data["total"] == 2

    def test_search_by_description(self):
        """按描述搜索"""
        create_template("模板1", "分类", "这是一个关于冒泡的描述")
        create_template("模板2", "分类", "这是一个关于快速的描述")

        resp = requests.get(api_url("/api/templates/search"), params={"keyword": "冒泡"})
        assert resp.status_code == 200
        assert resp.json()["total"] == 1

    def test_search_no_results(self):
        """搜索无结果返回空列表"""
        create_template("模板1", "分类", "描述")

        resp = requests.get(api_url("/api/templates/search"), params={"keyword": "不存在的关键词"})
        assert resp.status_code == 200
        assert resp.json()["total"] == 0
        assert resp.json()["items"] == []

    def test_search_case_insensitive(self):
        """大小写不敏感搜索"""
        create_template("BubbleSort", "分类", "描述")

        resp = requests.get(api_url("/api/templates/search"), params={"keyword": "bubblesort"})
        assert resp.status_code == 200
        assert resp.json()["total"] == 1

    def test_search_missing_keyword(self):
        """缺少 keyword 参数，返回 422"""
        resp = requests.get(api_url("/api/templates/search"))
        assert resp.status_code == 422

    def test_search_pagination(self):
        """搜索结果分页"""
        for i in range(15):
            create_template(f"排序算法{i}", "分类", "描述")

        resp = requests.get(
            api_url("/api/templates/search"),
            params={"keyword": "排序", "page": 1, "limit": 10},
        )
        assert resp.status_code == 200
        data = resp.json()
        assert data["total"] == 15
        assert len(data["items"]) == 10

    def test_search_order_by_favorite_count(self):
        """验证按收藏数降序排列"""
        id1 = create_template("排序1", "分类", "描述")
        id2 = create_template("排序2", "分类", "描述")

        # 为模板2增加收藏
        with get_db_connection() as conn:
            with conn.cursor() as cur:
                cur.execute(
                    "UPDATE templates SET favorite_count = 10 WHERE id = %s",
                    (id2,),
                )
            conn.commit()

        resp = requests.get(api_url("/api/templates/search"), params={"keyword": "排序"})
        items = resp.json()["items"]
        assert items[0]["id"] == id2  # 收藏数多的在前

    def test_search_partial_match(self):
        """部分匹配搜索"""
        create_template("冒泡排序算法", "分类", "描述")

        resp = requests.get(api_url("/api/templates/search"), params={"keyword": "冒泡"})
        assert resp.status_code == 200
        assert resp.json()["total"] == 1

    def test_search_logged_in_is_favorited(self):
        """登录用户搜索，验证 is_favorited"""
        template_id = create_template("排序算法", "分类", "描述")
        token = create_user_and_login()
        requests.post(
            api_url("/api/favorites"),
            headers=auth_header(token),
            json={"template_id": template_id},
        )

        resp = requests.get(
            api_url("/api/templates/search"),
            headers=auth_header(token),
            params={"keyword": "排序"},
        )
        assert resp.json()["items"][0]["is_favorited"] is True


# =============================================================================
# 4. 收藏相关 API 测试
# =============================================================================


class TestFavoriteAdd:
    """收藏模板测试"""

    def test_favorite_success(self):
        """正常收藏模板"""
        template_id = create_template("模板", "分类", "描述")
        token = create_user_and_login()

        resp = requests.post(
            api_url("/api/favorites"),
            headers=auth_header(token),
            json={"template_id": template_id},
        )
        assert resp.status_code == 200

    def test_favorite_template_not_found(self):
        """模板不存在，返回 404"""
        token = create_user_and_login()

        resp = requests.post(
            api_url("/api/favorites"),
            headers=auth_header(token),
            json={"template_id": 99999},
        )
        assert resp.status_code == 404

    def test_favorite_duplicate(self):
        """重复收藏，返回 409"""
        template_id = create_template("模板", "分类", "描述")
        token = create_user_and_login()

        requests.post(
            api_url("/api/favorites"),
            headers=auth_header(token),
            json={"template_id": template_id},
        )
        resp = requests.post(
            api_url("/api/favorites"),
            headers=auth_header(token),
            json={"template_id": template_id},
        )
        assert resp.status_code == 409

    def test_favorite_not_logged_in(self):
        """未登录收藏，返回 401"""
        template_id = create_template("模板", "分类", "描述")

        resp = requests.post(
            api_url("/api/favorites"),
            json={"template_id": template_id},
        )
        assert resp.status_code == 401

    def test_favorite_suspended_user(self):
        """Suspended 用户收藏，返回 403"""
        template_id = create_template("模板", "分类", "描述")
        token = create_user_and_login()
        set_user_status(1, "Suspended")

        resp = requests.post(
            api_url("/api/favorites"),
            headers=auth_header(token),
            json={"template_id": template_id},
        )
        assert resp.status_code == 403

    def test_favorite_count_increased(self):
        """收藏后 favorite_count 增加"""
        template_id = create_template("模板", "分类", "描述")
        token = create_user_and_login()

        resp = requests.get(api_url(f"/api/templates/{template_id}"))
        old_count = resp.json()["favorite_count"]

        requests.post(
            api_url("/api/favorites"),
            headers=auth_header(token),
            json={"template_id": template_id},
        )

        resp = requests.get(api_url(f"/api/templates/{template_id}"))
        new_count = resp.json()["favorite_count"]
        assert new_count == old_count + 1

    def test_favorite_is_favorited_true(self):
        """收藏后 is_favorited 变为 true"""
        template_id = create_template("模板", "分类", "描述")
        token = create_user_and_login()

        requests.post(
            api_url("/api/favorites"),
            headers=auth_header(token),
            json={"template_id": template_id},
        )

        resp = requests.get(
            api_url(f"/api/templates/{template_id}"),
            headers=auth_header(token),
        )
        assert resp.json()["is_favorited"] is True


class TestFavoriteRemove:
    """取消收藏测试"""

    def test_unfavorite_success(self):
        """正常取消收藏"""
        template_id = create_template("模板", "分类", "描述")
        token = create_user_and_login()

        requests.post(
            api_url("/api/favorites"),
            headers=auth_header(token),
            json={"template_id": template_id},
        )
        resp = requests.delete(
            api_url(f"/api/favorites/{template_id}"),
            headers=auth_header(token),
        )
        assert resp.status_code == 200

    def test_unfavorite_not_favorited(self):
        """未收藏的模板取消，返回 404"""
        template_id = create_template("模板", "分类", "描述")
        token = create_user_and_login()

        resp = requests.delete(
            api_url(f"/api/favorites/{template_id}"),
            headers=auth_header(token),
        )
        assert resp.status_code == 404

    def test_unfavorite_not_logged_in(self):
        """未登录取消，返回 401"""
        template_id = create_template("模板", "分类", "描述")

        resp = requests.delete(api_url(f"/api/favorites/{template_id}"))
        assert resp.status_code == 401

    def test_unfavorite_count_decreased(self):
        """取消后 favorite_count 减少"""
        template_id = create_template("模板", "分类", "描述")
        token = create_user_and_login()

        requests.post(
            api_url("/api/favorites"),
            headers=auth_header(token),
            json={"template_id": template_id},
        )

        resp = requests.get(api_url(f"/api/templates/{template_id}"))
        old_count = resp.json()["favorite_count"]

        requests.delete(
            api_url(f"/api/favorites/{template_id}"),
            headers=auth_header(token),
        )

        resp = requests.get(api_url(f"/api/templates/{template_id}"))
        new_count = resp.json()["favorite_count"]
        assert new_count == old_count - 1

    def test_unfavorite_is_favorited_false(self):
        """取消后 is_favorited 变为 false"""
        template_id = create_template("模板", "分类", "描述")
        token = create_user_and_login()

        requests.post(
            api_url("/api/favorites"),
            headers=auth_header(token),
            json={"template_id": template_id},
        )
        requests.delete(
            api_url(f"/api/favorites/{template_id}"),
            headers=auth_header(token),
        )

        resp = requests.get(
            api_url(f"/api/templates/{template_id}"),
            headers=auth_header(token),
        )
        assert resp.json()["is_favorited"] is False

    def test_unfavorite_suspended_user_allowed(self):
        """Suspended 用户取消收藏，应成功"""
        template_id = create_template("模板", "分类", "描述")
        token = create_user_and_login()

        requests.post(
            api_url("/api/favorites"),
            headers=auth_header(token),
            json={"template_id": template_id},
        )

        set_user_status(1, "Suspended")

        resp = requests.delete(
            api_url(f"/api/favorites/{template_id}"),
            headers=auth_header(token),
        )
        assert resp.status_code == 200


# =============================================================================
# 5. 代码执行相关 API 测试
# =============================================================================


class TestExecuteCode:
    """代码执行测试"""

    def test_execute_python_success(self):
        """Python 代码执行成功，返回 TOML"""
        token = create_user_and_login()
        code = '''
import ds4viz as dv
with dv.stack("test_stack") as s:
    s.push(1)
    s.push(2)
    s.push(3)
'''
        resp = requests.post(
            api_url("/api/execute"),
            headers=auth_header(token),
            json={"language": "python", "code": code},
        )
        assert resp.status_code == 200
        data = resp.json()
        assert data["status"] == "Success"
        assert data["toml_output"] is not None
        assert "[meta]" in data["toml_output"]

    def test_execute_lua_success(self):
        """Lua 代码执行成功"""
        token = create_user_and_login()
        code = '''
local ds4viz = require("ds4viz")

ds4viz.withContext(ds4viz.stack("test_stack"), function(s)
    s:push(1)
    s:push(2)
    s:push(3)
end)
'''
        resp = requests.post(
            api_url("/api/execute"),
            headers=auth_header(token),
            json={"language": "lua", "code": code},
        )
        assert resp.status_code == 200
        data = resp.json()
        assert data["status"] == "Success"
        assert data["toml_output"] is not None

    def test_execute_rust_success(self):
        """Rust 代码执行成功"""
        token = create_user_and_login()
        code = '''
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
        resp = requests.post(
            api_url("/api/execute"),
            headers=auth_header(token),
            json={"language": "rust", "code": code},
        )
        assert resp.status_code == 200
        data = resp.json()
        assert data["status"] == "Success"
        assert data["toml_output"] is not None

    def test_execute_python_syntax_error(self):
        """Python 语法错误，返回 Error"""
        token = create_user_and_login()
        code = "def invalid syntax here!!!"

        resp = requests.post(
            api_url("/api/execute"),
            headers=auth_header(token),
            json={"language": "python", "code": code},
        )
        assert resp.status_code == 200
        data = resp.json()
        assert data["status"] == "Error"
        assert data["error_message"] is not None

    def test_execute_python_runtime_error(self):
        """Python 运行时错误，返回 Error"""
        token = create_user_and_login()
        code = '''
import ds4viz as dv
with dv.stack("test") as s:
    s.pop()  # 空栈弹出
'''
        resp = requests.post(
            api_url("/api/execute"),
            headers=auth_header(token),
            json={"language": "python", "code": code},
        )
        assert resp.status_code == 200
        data = resp.json()
        # 即使有运行时错误，ds4viz 会生成包含 error 的 toml
        assert data["status"] in ["Success", "Error"]

    def test_execute_timeout(self):
        """代码执行超时，返回 Timeout"""
        token = create_user_and_login()
        code = '''
import time
time.sleep(100)  # 超过测试配置的 10 秒超时
'''
        resp = requests.post(
            api_url("/api/execute"),
            headers=auth_header(token),
            json={"language": "python", "code": code},
        )
        assert resp.status_code == 200
        data = resp.json()
        assert data["status"] == "Timeout"

    def test_execute_no_toml_output(self):
        """未生成 TOML 文件，返回 Error"""
        token = create_user_and_login()
        code = "print('hello world')"  # 没有使用 ds4viz

        resp = requests.post(
            api_url("/api/execute"),
            headers=auth_header(token),
            json={"language": "python", "code": code},
        )
        assert resp.status_code == 200
        data = resp.json()
        assert data["status"] == "Error"

    def test_execute_unsupported_language(self):
        """不支持的语言，返回 422"""
        token = create_user_and_login()

        resp = requests.post(
            api_url("/api/execute"),
            headers=auth_header(token),
            json={"language": "java", "code": "System.out.println('hello');"},
        )
        assert resp.status_code == 422

    def test_execute_not_logged_in(self):
        """未登录执行，返回 401"""
        resp = requests.post(
            api_url("/api/execute"),
            json={"language": "python", "code": "print('test')"},
        )
        assert resp.status_code == 401

    def test_execute_suspended_user(self):
        """Suspended 用户执行，返回 403"""
        token = create_user_and_login()
        set_user_status(1, "Suspended")

        resp = requests.post(
            api_url("/api/execute"),
            headers=auth_header(token),
            json={"language": "python", "code": "print('test')"},
        )
        assert resp.status_code == 403

    def test_execute_cache_hit(self):
        """缓存命中：相同代码第二次执行"""
        token = create_user_and_login()
        code = '''
import ds4viz as dv
with dv.stack("cache_test") as s:
    s.push(42)
'''
        # 第一次执行
        resp1 = requests.post(
            api_url("/api/execute"),
            headers=auth_header(token),
            json={"language": "python", "code": code},
        )
        assert resp1.json()["cached"] is False

        # 第二次执行（相同代码）
        resp2 = requests.post(
            api_url("/api/execute"),
            headers=auth_header(token),
            json={"language": "python", "code": code},
        )
        assert resp2.json()["cached"] is True

    def test_execute_cache_hit_execution_time_zero(self):
        """缓存命中时 cached 为 true，execution_time 为 0"""
        token = create_user_and_login()
        code = '''
import ds4viz as dv
with dv.stack("cache_test2") as s:
    s.push(100)
'''
        requests.post(
            api_url("/api/execute"),
            headers=auth_header(token),
            json={"language": "python", "code": code},
        )

        resp = requests.post(
            api_url("/api/execute"),
            headers=auth_header(token),
            json={"language": "python", "code": code},
        )
        data = resp.json()
        assert data["cached"] is True
        assert data["execution_time"] == 0

    def test_execute_record_saved(self):
        """执行记录正确保存到数据库"""
        token = create_user_and_login()
        resp = requests.get(api_url("/api/auth/me"), headers=auth_header(token))
        user_id = resp.json()["id"]

        code = '''
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
    """Python ds4viz 库执行测试"""

    def test_python_stack_operations(self):
        """栈操作：push、pop、clear"""
        token = create_user_and_login()
        code = '''
import ds4viz as dv
with dv.stack("python_stack") as s:
    s.push(10)
    s.push(20)
    s.push(30)
    s.pop()
    s.clear()
'''
        resp = requests.post(
            api_url("/api/execute"),
            headers=auth_header(token),
            json={"language": "python", "code": code},
        )
        assert resp.json()["status"] == "Success"

    def test_python_queue_operations(self):
        """队列操作：enqueue、dequeue"""
        token = create_user_and_login()
        code = '''
import ds4viz as dv
with dv.queue("python_queue") as q:
    q.enqueue(10)
    q.enqueue(20)
    q.enqueue(30)
    q.dequeue()
    q.dequeue()
'''
        resp = requests.post(
            api_url("/api/execute"),
            headers=auth_header(token),
            json={"language": "python", "code": code},
        )
        assert resp.json()["status"] == "Success"

    def test_python_single_linked_list(self):
        """单链表操作"""
        token = create_user_and_login()
        code = '''
import ds4viz as dv
with dv.single_linked_list("python_slist") as slist:
    node_a = slist.insert_head(10)
    slist.insert_tail(20)
    node_c = slist.insert_tail(30)
    slist.insert_after(node_a, 15)
    slist.delete(node_c)
    slist.reverse()
'''
        resp = requests.post(
            api_url("/api/execute"),
            headers=auth_header(token),
            json={"language": "python", "code": code},
        )
        assert resp.json()["status"] == "Success"

    def test_python_double_linked_list(self):
        """双向链表操作"""
        token = create_user_and_login()
        code = '''
import ds4viz as dv
with dv.double_linked_list("python_dlist") as dlist:
    node_a = dlist.insert_head(10)
    dlist.insert_tail(30)
    dlist.insert_after(node_a, 20)
    dlist.delete_tail()
    dlist.reverse()
'''
        resp = requests.post(
            api_url("/api/execute"),
            headers=auth_header(token),
            json={"language": "python", "code": code},
        )
        assert resp.json()["status"] == "Success"

    def test_python_binary_tree(self):
        """二叉树操作"""
        token = create_user_and_login()
        code = '''
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
        resp = requests.post(
            api_url("/api/execute"),
            headers=auth_header(token),
            json={"language": "python", "code": code},
        )
        assert resp.json()["status"] == "Success"

    def test_python_binary_search_tree(self):
        """二叉搜索树操作"""
        token = create_user_and_login()
        code = '''
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
        resp = requests.post(
            api_url("/api/execute"),
            headers=auth_header(token),
            json={"language": "python", "code": code},
        )
        assert resp.json()["status"] == "Success"

    def test_python_min_heap(self):
        """最小堆操作"""
        token = create_user_and_login()
        code = '''
import ds4viz as dv
with dv.heap("python_min_heap", heap_type="min") as h:
    h.insert(10)
    h.insert(5)
    h.insert(15)
    h.insert(3)
    h.extract()
'''
        resp = requests.post(
            api_url("/api/execute"),
            headers=auth_header(token),
            json={"language": "python", "code": code},
        )
        assert resp.json()["status"] == "Success"

    def test_python_max_heap(self):
        """最大堆操作"""
        token = create_user_and_login()
        code = '''
import ds4viz as dv
with dv.heap("python_max_heap", heap_type="max") as h:
    h.insert(10)
    h.insert(5)
    h.insert(15)
    h.insert(3)
    h.extract()
'''
        resp = requests.post(
            api_url("/api/execute"),
            headers=auth_header(token),
            json={"language": "python", "code": code},
        )
        assert resp.json()["status"] == "Success"

    def test_python_graph_undirected(self):
        """无向图操作"""
        token = create_user_and_login()
        code = '''
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
        resp = requests.post(
            api_url("/api/execute"),
            headers=auth_header(token),
            json={"language": "python", "code": code},
        )
        assert resp.json()["status"] == "Success"

    def test_python_graph_directed(self):
        """有向图操作"""
        token = create_user_and_login()
        code = '''
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
        resp = requests.post(
            api_url("/api/execute"),
            headers=auth_header(token),
            json={"language": "python", "code": code},
        )
        assert resp.json()["status"] == "Success"

    def test_python_graph_weighted(self):
        """带权图操作"""
        token = create_user_and_login()
        code = '''
import ds4viz as dv
with dv.graph_weighted("python_weighted", directed=False) as graph:
    graph.add_node(0, "A")
    graph.add_node(1, "B")
    graph.add_node(2, "C")
    graph.add_edge(0, 1, 3.5)
    graph.add_edge(1, 2, 2.0)
    graph.update_weight(0, 1, 5.0)
'''
        resp = requests.post(
            api_url("/api/execute"),
            headers=auth_header(token),
            json={"language": "python", "code": code},
        )
        assert resp.json()["status"] == "Success"


class TestExecuteLuaDs4viz:
    """Lua ds4viz 库执行测试"""

    def test_lua_stack_operations(self):
        """Lua 栈操作"""
        token = create_user_and_login()
        code = '''
local ds4viz = require("ds4viz")

ds4viz.withContext(ds4viz.stack("lua_stack"), function(s)
    s:push(10)
    s:push(20)
    s:push(30)
    s:pop()
end)
'''
        resp = requests.post(
            api_url("/api/execute"),
            headers=auth_header(token),
            json={"language": "lua", "code": code},
        )
        assert resp.json()["status"] == "Success"

    def test_lua_graph_operations(self):
        """Lua 图结构操作"""
        token = create_user_and_login()
        code = '''
local ds4viz = require("ds4viz")

ds4viz.withContext(ds4viz.graphUndirected("lua_graph"), function(graph)
    graph:addNode(0, "A")
    graph:addNode(1, "B")
    graph:addNode(2, "C")
    graph:addEdge(0, 1)
    graph:addEdge(1, 2)
end)
'''
        resp = requests.post(
            api_url("/api/execute"),
            headers=auth_header(token),
            json={"language": "lua", "code": code},
        )
        assert resp.json()["status"] == "Success"


class TestExecuteRustDs4viz:
    """Rust ds4viz 库执行测试"""

    def test_rust_basic_operations(self):
        """Rust 基本数据结构操作"""
        token = create_user_and_login()
        code = '''
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
        resp = requests.post(
            api_url("/api/execute"),
            headers=auth_header(token),
            json={"language": "rust", "code": code},
        )
        assert resp.json()["status"] == "Success"


class TestExecutionDetail:
    """获取执行详情测试"""

    def test_get_execution_success(self):
        """正常获取执行详情"""
        token = create_user_and_login()
        code = '''
import ds4viz as dv
with dv.stack("detail_test") as s:
    s.push(1)
'''
        resp = requests.post(
            api_url("/api/execute"),
            headers=auth_header(token),
            json={"language": "python", "code": code},
        )
        execution_id = resp.json()["execution_id"]

        resp = requests.get(
            api_url(f"/api/executions/{execution_id}"),
            headers=auth_header(token),
        )
        assert resp.status_code == 200
        data = resp.json()
        assert data["id"] == execution_id
        assert data["language"] == "python"

    def test_get_execution_not_found(self):
        """执行记录不存在，返回 404"""
        token = create_user_and_login()
        resp = requests.get(
            api_url("/api/executions/99999"),
            headers=auth_header(token),
        )
        assert resp.status_code == 404

    def test_get_execution_other_user(self):
        """查看他人执行记录，返回 403"""
        token1 = create_user_and_login("user1", "pass1")
        code = '''
import ds4viz as dv
with dv.stack("test") as s:
    s.push(1)
'''
        resp = requests.post(
            api_url("/api/execute"),
            headers=auth_header(token1),
            json={"language": "python", "code": code},
        )
        execution_id = resp.json()["execution_id"]

        token2 = create_user_and_login("user2", "pass2")
        resp = requests.get(
            api_url(f"/api/executions/{execution_id}"),
            headers=auth_header(token2),
        )
        assert resp.status_code == 403

    def test_get_execution_not_logged_in(self):
        """未登录获取，返回 401"""
        resp = requests.get(api_url("/api/executions/1"))
        assert resp.status_code == 401

    def test_get_execution_fields(self):
        """验证各字段正确返回"""
        token = create_user_and_login()
        code = '''
import ds4viz as dv
with dv.stack("fields_test") as s:
    s.push(1)
'''
        resp = requests.post(
            api_url("/api/execute"),
            headers=auth_header(token),
            json={"language": "python", "code": code},
        )
        execution_id = resp.json()["execution_id"]

        resp = requests.get(
            api_url(f"/api/executions/{execution_id}"),
            headers=auth_header(token),
        )
        data = resp.json()
        assert "id" in data
        assert "language" in data
        assert "code" in data
        assert "toml_output" in data
        assert "status" in data
        assert "error_message" in data
        assert "execution_time" in data
        assert "created_at" in data


# =============================================================================
# 6. 健康检查测试
# =============================================================================


class TestHealthCheck:
    """健康检查测试"""

    def test_health_check_ok(self):
        """健康检查返回 status: ok"""
        resp = requests.get(api_url("/health"))
        assert resp.status_code == 200
        assert resp.json()["status"] == "ok"

    def test_health_check_no_auth_required(self):
        """健康检查无需认证"""
        resp = requests.get(api_url("/health"))
        assert resp.status_code == 200


# =============================================================================
# 7. 综合业务流程测试
# =============================================================================


class TestBusinessFlow:
    """综合业务流程测试"""

    def test_flow_register_login_execute_history(self):
        """完整流程：注册 → 登录 → 执行代码 → 查看历史"""
        # 注册
        resp = register_user("flowuser1", "flowpass1")
        assert resp.status_code == 200
        user_id = resp.json()["id"]

        # 登录
        resp = login_user("flowuser1", "flowpass1")
        assert resp.status_code == 200
        token = resp.json()["token"]

        # 执行代码
        code = '''
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

        # 查看历史
        resp = requests.get(
            api_url(f"/api/users/{user_id}/executions"),
            headers=auth_header(token),
        )
        assert resp.status_code == 200
        assert resp.json()["total"] >= 1

    def test_flow_register_login_favorite_list(self):
        """完整流程：注册 → 登录 → 收藏模板 → 查看收藏列表"""
        template_id = create_template("流程测试模板", "测试分类", "描述")

        # 注册
        resp = register_user("flowuser2", "flowpass2")
        user_id = resp.json()["id"]

        # 登录
        resp = login_user("flowuser2", "flowpass2")
        token = resp.json()["token"]

        # 收藏
        resp = requests.post(
            api_url("/api/favorites"),
            headers=auth_header(token),
            json={"template_id": template_id},
        )
        assert resp.status_code == 200

        # 查看收藏列表
        resp = requests.get(
            api_url(f"/api/users/{user_id}/favorites"),
            headers=auth_header(token),
        )
        assert resp.status_code == 200
        assert resp.json()["total"] == 1

    def test_cache_shared_across_users(self):
        """缓存跨用户共享：用户 A 执行后用户 B 执行相同代码命中缓存"""
        code = '''
import ds4viz as dv
with dv.stack("shared_cache_test") as s:
    s.push(999)
'''
        # 用户 A 执行
        token_a = create_user_and_login("userA", "passA")
        resp_a = requests.post(
            api_url("/api/execute"),
            headers=auth_header(token_a),
            json={"language": "python", "code": code},
        )
        assert resp_a.json()["cached"] is False

        # 用户 B 执行相同代码
        token_b = create_user_and_login("userB", "passB")
        resp_b = requests.post(
            api_url("/api/execute"),
            headers=auth_header(token_b),
            json={"language": "python", "code": code},
        )
        assert resp_b.json()["cached"] is True

    def test_user_status_change_permission(self):
        """用户状态变更后权限验证"""
        token = create_user_and_login("statususer", "statuspass")

        # Active 状态可以执行
        code = '''
import ds4viz as dv
with dv.stack("status_test") as s:
    s.push(1)
'''
        resp = requests.post(
            api_url("/api/execute"),
            headers=auth_header(token),
            json={"language": "python", "code": code},
        )
        assert resp.status_code == 200

        # 修改为 Suspended
        set_user_status(1, "Suspended")

        # Suspended 状态不能执行
        resp = requests.post(
            api_url("/api/execute"),
            headers=auth_header(token),
            json={"language": "python", "code": code},
        )
        assert resp.status_code == 403

    def test_logout_and_relogin(self):
        """登出后重新登录获取新 token"""
        register_user("reloginuser", "reloginpass")
        resp = login_user("reloginuser", "reloginpass")
        token1 = resp.json()["token"]

        # 登出
        requests.post(api_url("/api/auth/logout"), headers=auth_header(token1))

        # 重新登录
        resp = login_user("reloginuser", "reloginpass")
        token2 = resp.json()["token"]

        assert token1 != token2

        # 新 token 有效
        resp = requests.get(api_url("/api/auth/me"), headers=auth_header(token2))
        assert resp.status_code == 200

    def test_favorite_count_accuracy(self):
        """模板收藏计数准确性：多次收藏取消后计数正确"""
        template_id = create_template("计数测试模板", "分类", "描述")

        # 初始计数
        resp = requests.get(api_url(f"/api/templates/{template_id}"))
        initial_count = resp.json()["favorite_count"]

        # 多个用户收藏
        tokens = []
        for i in range(3):
            token = create_user_and_login(f"countuser{i}", f"countpass{i}")
            tokens.append(token)
            requests.post(
                api_url("/api/favorites"),
                headers=auth_header(token),
                json={"template_id": template_id},
            )

        resp = requests.get(api_url(f"/api/templates/{template_id}"))
        assert resp.json()["favorite_count"] == initial_count + 3

        # 2 个用户取消收藏
        for token in tokens[:2]:
            requests.delete(
                api_url(f"/api/favorites/{template_id}"),
                headers=auth_header(token),
            )

        resp = requests.get(api_url(f"/api/templates/{template_id}"))
        assert resp.json()["favorite_count"] == initial_count + 1

    def test_execution_history_matches_executions(self):
        """执行历史与实际执行对应"""
        token = create_user_and_login("histuser", "histpass")
        resp = requests.get(api_url("/api/auth/me"), headers=auth_header(token))
        user_id = resp.json()["id"]

        executed_codes = []
        for i in range(3):
            code = f'''
import ds4viz as dv
with dv.stack("hist_test_{i}") as s:
    s.push({i})
'''
            executed_codes.append(code)
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

    def test_pagination_boundary_total_equals_limit(self):
        """分页边界：总数恰好等于 limit 时的行为"""
        for i in range(10):
            create_template(f"边界模板{i}", "分类", "描述")

        resp = requests.get(api_url("/api/templates"), params={"page": 1, "limit": 10})
        data = resp.json()
        assert data["total"] == 10
        assert len(data["items"]) == 10
        assert data["page"] == 1


# =============================================================================
# 8. 错误处理和边缘情况测试
# =============================================================================


class TestErrorHandling:
    """错误处理和边缘情况测试"""

    def test_request_body_not_json(self):
        """请求体非 JSON 格式"""
        resp = requests.post(
            api_url("/api/auth/register"),
            data="not json data",
            headers={"Content-Type": "application/json"},
        )
        assert resp.status_code == 422

    def test_request_body_missing_fields(self):
        """请求体字段缺失"""
        resp = requests.post(
            api_url("/api/auth/register"),
            json={"username": "testuser"},  # 缺少 password
        )
        assert resp.status_code == 422

    def test_request_body_wrong_type(self):
        """请求体字段类型错误"""
        resp = requests.post(
            api_url("/api/auth/register"),
            json={"username": 123, "password": "testpass"},  # username 应为字符串
        )
        assert resp.status_code == 422

    def test_path_param_wrong_type(self):
        """路径参数类型错误"""
        resp = requests.get(api_url("/api/templates/not_a_number"))
        assert resp.status_code == 422

    def test_auth_header_wrong_format(self):
        """Authorization 头格式错误"""
        resp = requests.get(
            api_url("/api/auth/me"),
            headers={"Authorization": "Basic sometoken"},  # 应为 Bearer
        )
        assert resp.status_code == 401
        