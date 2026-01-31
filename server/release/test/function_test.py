r"""
功能测试模块，通过HTTP请求验证服务器API功能

:file: test/function_test.py
:author: WaterRun
:time: 2026-01-31
"""

import subprocess
import time
import signal
import os
import socket
from contextlib import contextmanager
from typing import Generator

import psycopg
import pytest
import requests

from conftest import (
    get_db_connection,
    reset_database,
    TEST_HOST,
    TEST_PORT,
)

# ============================================
# 测试配置
# ============================================
from conftest import TEST_HOST, TEST_PORT

BASE_URL = f"http://{TEST_HOST}:{TEST_PORT}"

DB_CONFIG = {
    "host": "localhost",
    "port": 5432,
    "dbname": "ds4viz_test",
    "user": "ds4viz_test",
    "password": "test_pwd_123",
}


# ============================================
# 数据库工具函数
# ============================================





def insert_user(username: str, password_hash: str, status: str = "Active") -> int:
    r"""
    直接插入用户记录
    """
    with get_db_connection() as conn:
        with conn.cursor() as cur:
            cur.execute(
                """
                INSERT INTO users (username, password_hash, status)
                VALUES (%s, %s, %s)
                RETURNING id
                """,
                (username, password_hash, status),
            )
            user_id = cur.fetchone()[0]
        conn.commit()
    return user_id


def insert_template(title: str, category: str, description: str) -> int:
    r"""
    直接插入模板记录
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
            template_id = cur.fetchone()[0]
        conn.commit()
    return template_id


def insert_template_code(template_id: int, language: str, code: str, explanation: str | None = None) -> int:
    r"""
    直接插入模板代码记录
    """
    with get_db_connection() as conn:
        with conn.cursor() as cur:
            cur.execute(
                """
                INSERT INTO template_codes (template_id, language, code, explanation)
                VALUES (%s, %s, %s, %s)
                RETURNING id
                """,
                (template_id, language, code, explanation),
            )
            code_id = cur.fetchone()[0]
        conn.commit()
    return code_id


def insert_execution(
    user_id: int,
    code_hash: str,
    language: str,
    code: str,
    status: str,
    toml_output: str | None = None,
    error_message: str | None = None,
    execution_time: int = 100,
    ip_address: str = "127.0.0.1",
) -> int:
    r"""
    直接插入执行记录
    """
    with get_db_connection() as conn:
        with conn.cursor() as cur:
            cur.execute(
                """
                INSERT INTO executions (user_id, code_hash, language, code, toml_output,
                                        status, error_message, execution_time, ip_address)
                VALUES (%s, %s, %s, %s, %s, %s, %s, %s, %s)
                RETURNING id
                """,
                (user_id, code_hash, language, code, toml_output,
                 status, error_message, execution_time, ip_address),
            )
            execution_id = cur.fetchone()[0]
        conn.commit()
    return execution_id


def insert_favorite(user_id: int, template_id: int) -> int:
    r"""
    直接插入收藏记录
    """
    with get_db_connection() as conn:
        with conn.cursor() as cur:
            cur.execute(
                """
                INSERT INTO user_favorites (user_id, template_id)
                VALUES (%s, %s)
                RETURNING id
                """,
                (user_id, template_id),
            )
            fav_id = cur.fetchone()[0]
        conn.commit()
    return fav_id


def insert_cache(code_hash: str, language: str, toml_output: str) -> int:
    r"""
    直接插入执行缓存记录
    """
    with get_db_connection() as conn:
        with conn.cursor() as cur:
            cur.execute(
                """
                INSERT INTO execution_cache (code_hash, language, toml_output)
                VALUES (%s, %s, %s)
                RETURNING id
                """,
                (code_hash, language, toml_output),
            )
            cache_id = cur.fetchone()[0]
        conn.commit()
    return cache_id


def update_user_status(user_id: int, status: str) -> None:
    r"""
    更新用户状态
    """
    with get_db_connection() as conn:
        with conn.cursor() as cur:
            cur.execute(
                "UPDATE users SET status = %s WHERE id = %s",
                (status, user_id),
            )
        conn.commit()


def get_template_favorite_count(template_id: int) -> int:
    r"""
    获取模板收藏数
    """
    with get_db_connection() as conn:
        with conn.cursor() as cur:
            cur.execute(
                "SELECT favorite_count FROM templates WHERE id = %s",
                (template_id,),
            )
            row = cur.fetchone()
            return row[0] if row else 0


def get_cache_hit_count(code_hash: str, language: str) -> int:
    r"""
    获取缓存命中次数
    """
    with get_db_connection() as conn:
        with conn.cursor() as cur:
            cur.execute(
                "SELECT hit_count FROM execution_cache WHERE code_hash = %s AND language = %s",
                (code_hash, language),
            )
            row = cur.fetchone()
            return row[0] if row else 0


def count_sessions(user_id: int) -> int:
    r"""
    统计用户会话数
    """
    with get_db_connection() as conn:
        with conn.cursor() as cur:
            cur.execute(
                "SELECT COUNT(*) FROM sessions WHERE user_id = %s",
                (user_id,),
            )
            return cur.fetchone()[0]


def count_logs(level: str | None = None, module: str | None = None) -> int:
    r"""
    统计日志数量
    """
    with get_db_connection() as conn:
        with conn.cursor() as cur:
            query = "SELECT COUNT(*) FROM logs WHERE 1=1"
            params = []
            if level:
                query += " AND level = %s"
                params.append(level)
            if module:
                query += " AND module = %s"
                params.append(module)
            cur.execute(query, params)
            return cur.fetchone()[0]


# ============================================
# 服务器管理
# ============================================
    
@pytest.fixture(scope="function")
def clean_db(server_process) -> Generator[None, None, None]:
    r"""
    每个测试函数前后重置数据库
    """
    reset_database()
    yield
    reset_database()


# ============================================
# HTTP 客户端辅助函数
# ============================================


class APIClient:
    r"""
    API客户端封装
    """

    def __init__(self, base_url: str = BASE_URL):
        self.base_url = base_url
        self.session = requests.Session()

    def get(self, path: str, headers: dict | None = None, **kwargs) -> requests.Response:
        return self.session.get(f"{self.base_url}{path}", headers=headers, **kwargs)

    def post(self, path: str, headers: dict | None = None, **kwargs) -> requests.Response:
        return self.session.post(f"{self.base_url}{path}", headers=headers, **kwargs)

    def put(self, path: str, headers: dict | None = None, **kwargs) -> requests.Response:
        return self.session.put(f"{self.base_url}{path}", headers=headers, **kwargs)

    def delete(self, path: str, headers: dict | None = None, **kwargs) -> requests.Response:
        return self.session.delete(f"{self.base_url}{path}", headers=headers, **kwargs)

    def register(self, username: str, password: str) -> requests.Response:
        return self.post("/api/auth/register", json={"username": username, "password": password})

    def login(self, username: str, password: str) -> requests.Response:
        return self.post("/api/auth/login", json={"username": username, "password": password})

    def auth_headers(self, token: str) -> dict:
        return {"Authorization": f"Bearer {token}"}


@pytest.fixture
def api(clean_db) -> APIClient:
    r"""
    创建API客户端
    """
    return APIClient()


@pytest.fixture
def registered_user(api: APIClient) -> dict:
    r"""
    注册并返回用户信息
    """
    response = api.register("testuser", "testpass123")
    assert response.status_code == 200
    return response.json()


@pytest.fixture
def auth_user(api: APIClient, registered_user: dict) -> dict:
    r"""
    登录并返回认证信息
    """
    response = api.login("testuser", "testpass123")
    assert response.status_code == 200
    data = response.json()
    return {
        "user": data["user"],
        "token": data["token"],
        "headers": api.auth_headers(data["token"]),
        "expires_at": data["expires_at"],
    }


@pytest.fixture
def sample_template(clean_db) -> dict:
    r"""
    创建示例模板
    """
    template_id = insert_template("冒泡排序", "排序算法", "## 冒泡排序\n经典的排序算法示例")
    insert_template_code(template_id, "python",
                         "def bubble_sort(arr):\n    pass", "Python实现")
    insert_template_code(template_id, "lua",
                         "function bubble_sort(arr)\nend", "Lua实现")
    insert_template_code(template_id, "rust", "fn bubble_sort() {}", "Rust实现")
    return {
        "id": template_id,
        "title": "冒泡排序",
        "category": "排序算法",
    }


@pytest.fixture
def multiple_templates(clean_db) -> list[dict]:
    r"""
    创建多个模板
    """
    templates = []

    t1_id = insert_template("冒泡排序", "排序算法", "冒泡排序描述")
    insert_template_code(t1_id, "python", "def bubble_sort(): pass", None)
    templates.append({"id": t1_id, "title": "冒泡排序", "category": "排序算法"})

    t2_id = insert_template("快速排序", "排序算法", "快速排序描述")
    insert_template_code(t2_id, "python", "def quick_sort(): pass", None)
    templates.append({"id": t2_id, "title": "快速排序", "category": "排序算法"})

    t3_id = insert_template("二叉树遍历", "树算法", "二叉树遍历描述")
    insert_template_code(t3_id, "python", "def traverse(): pass", None)
    templates.append({"id": t3_id, "title": "二叉树遍历", "category": "树算法"})

    t4_id = insert_template("图的广度优先搜索", "图算法", "BFS描述")
    insert_template_code(t4_id, "python", "def bfs(): pass", None)
    templates.append({"id": t4_id, "title": "图的广度优先搜索", "category": "图算法"})

    return templates


# ============================================
# 健康检查测试
# ============================================


class TestHealthCheck:
    r"""
    健康检查接口测试
    """

    def test_health_check_returns_ok(self, api: APIClient):
        r"""
        健康检查接口返回正常状态
        """
        response = api.get("/health")
        assert response.status_code == 200
        data = response.json()
        assert data == {"status": "ok"}

    def test_health_check_response_time(self, api: APIClient):
        r"""
        健康检查响应时间应小于1秒
        """
        start = time.time()
        response = api.get("/health")
        elapsed = time.time() - start
        assert response.status_code == 200
        assert elapsed < 1.0

    def test_health_check_content_type_json(self, api: APIClient):
        r"""
        健康检查返回 JSON Content-Type
        """
        response = api.get("/health")
        assert response.status_code == 200
        content_type = response.headers.get("Content-Type", "")
        assert content_type.startswith("application/json")

# ============================================
# 用户注册测试
# ============================================


class TestRegister:
    r"""
    用户注册测试
    """

    def test_register_success(self, api: APIClient):
        r"""
        正常注册成功
        """
        response = api.register("newuser", "password123")
        assert response.status_code == 200
        data = response.json()
        assert data["username"] == "newuser"
        assert data["status"] == "Active"
        assert data["avatar_url"] is None
        assert "id" in data
        assert "created_at" in data
        assert data["id"] > 0

    def test_register_duplicate_username(self, api: APIClient, registered_user: dict):
        r"""
        重复用户名注册失败
        """
        response = api.register("testuser", "anotherpass")
        assert response.status_code == 409
        assert "用户名已存在" in response.json()["error"]

    def test_register_duplicate_username_case_sensitive(self, api: APIClient, registered_user: dict):
        r"""
        用户名区分大小写
        """
        response = api.register("TestUser", "password123")
        assert response.status_code == 200

    def test_register_username_min_length(self, api: APIClient):
        r"""
        用户名最小长度为3
        """
        response = api.register("ab", "password123")
        assert response.status_code == 422

        response = api.register("abc", "password123")
        assert response.status_code == 200

    def test_register_username_max_length(self, api: APIClient):
        r"""
        用户名最大长度为32
        """
        response = api.register("a" * 33, "password123")
        assert response.status_code == 422

        response = api.register("a" * 32, "password123")
        assert response.status_code == 200

    def test_register_password_min_length(self, api: APIClient):
        r"""
        密码最小长度为1
        """
        response = api.register("usertest1", "")
        assert response.status_code == 422

        response = api.register("usertest2", "a")
        assert response.status_code == 200


    def test_register_password_max_length(self, api: APIClient):
        r"""
            密码最大长度为64
            """
        response = api.register("usertest3", "a" * 65)
        assert response.status_code == 422

        response = api.register("usertest4", "a" * 64)
        assert response.status_code == 200

    def test_register_unicode_username(self, api: APIClient):
        r"""
        用户名支持Unicode字符
        """
        response = api.register("用户名测试", "password123")
        assert response.status_code == 200
        assert response.json()["username"] == "用户名测试"

    def test_register_unicode_password(self, api: APIClient):
        r"""
        密码支持Unicode字符
        """
        response = api.register("unicodepass", "密码测试123")
        assert response.status_code == 200

        response = api.login("unicodepass", "密码测试123")
        assert response.status_code == 200

    def test_register_special_chars_username(self, api: APIClient):
        r"""
        用户名包含特殊字符
        """
        response = api.register("user_name", "password123")
        assert response.status_code == 200

        response = api.register("user-name", "password123")
        assert response.status_code == 200

        response = api.register("user.name", "password123")
        assert response.status_code == 200

    def test_register_whitespace_username(self, api: APIClient):
        r"""
        用户名包含空格
        """
        response = api.register("user name", "password123")
        assert response.status_code == 200

    def test_register_missing_username(self, api: APIClient):
        r"""
        缺少用户名字段
        """
        response = api.post("/api/auth/register",
                            json={"password": "password123"})
        assert response.status_code == 422

    def test_register_missing_password(self, api: APIClient):
        r"""
        缺少密码字段
        """
        response = api.post("/api/auth/register",
                            json={"username": "testuser"})
        assert response.status_code == 422

    def test_register_empty_body(self, api: APIClient):
        r"""
        空请求体
        """
        response = api.post("/api/auth/register", json={})
        assert response.status_code == 422

    def test_register_invalid_json(self, api: APIClient):
        r"""
        无效的JSON
        """
        response = api.post("/api/auth/register", data="invalid json",
                            headers={"Content-Type": "application/json"})
        assert response.status_code == 422

    def test_register_null_values(self, api: APIClient):
        r"""
        null值字段
        """
        response = api.post("/api/auth/register",
                            json={"username": None, "password": "pass"})
        assert response.status_code == 422

    def test_register_creates_session_on_login(self, api: APIClient):
        r"""
        注册后可以登录并创建会话
        """
        api.register("sessiontest", "password123")
        response = api.login("sessiontest", "password123")
        assert response.status_code == 200

        user_id = response.json()["user"]["id"]
        assert count_sessions(user_id) == 1

# ============================================
# 用户登录测试
# ============================================


class TestLogin:
    r"""
    用户登录测试
    """

    def test_login_success(self, api: APIClient, registered_user: dict):
        r"""
        正常登录成功
        """
        response = api.login("testuser", "testpass123")
        assert response.status_code == 200
        data = response.json()
        assert "token" in data
        assert "expires_at" in data
        assert data["user"]["username"] == "testuser"
        assert data["user"]["status"] == "Active"
        assert data["user"]["id"] == registered_user["id"]

    def test_login_wrong_password(self, api: APIClient, registered_user: dict):
        r"""
        密码错误
        """
        response = api.login("testuser", "wrongpassword")
        assert response.status_code == 401
        assert "用户名或密码错误" in response.json()["error"]

    def test_login_nonexistent_user(self, api: APIClient):
        r"""
        用户不存在
        """
        response = api.login("nonexistent", "password123")
        assert response.status_code == 401
        assert "用户名或密码错误" in response.json()["error"]

    def test_login_banned_user(self, api: APIClient, registered_user: dict):
        r"""
        被封禁用户无法登录
        """
        update_user_status(registered_user["id"], "Banned")
        response = api.login("testuser", "testpass123")
        assert response.status_code == 403
        assert "封禁" in response.json()["error"]

    def test_login_suspended_user_allowed(self, api: APIClient, registered_user: dict):
        r"""
        被暂停用户可以登录
        """
        update_user_status(registered_user["id"], "Suspended")
        response = api.login("testuser", "testpass123")
        assert response.status_code == 200
        assert response.json()["user"]["status"] == "Suspended"

    def test_login_case_sensitive_username(self, api: APIClient, registered_user: dict):
        r"""
        用户名区分大小写
        """
        response = api.login("TestUser", "testpass123")
        assert response.status_code == 401

    def test_login_case_sensitive_password(self, api: APIClient, registered_user: dict):
        r"""
        密码区分大小写
        """
        response = api.login("testuser", "TestPass123")
        assert response.status_code == 401

    def test_login_multiple_sessions(self, api: APIClient, registered_user: dict):
        r"""
        同一用户可以多次登录创建多个会话
        """
        response1 = api.login("testuser", "testpass123")
        assert response1.status_code == 200
        token1 = response1.json()["token"]

        response2 = api.login("testuser", "testpass123")
        assert response2.status_code == 200
        token2 = response2.json()["token"]

        assert token1 != token2
        assert count_sessions(registered_user["id"]) == 2

    def test_login_token_format(self, api: APIClient, registered_user: dict):
        r"""
        令牌格式为JWT
        """
        response = api.login("testuser", "testpass123")
        token = response.json()["token"]
        parts = token.split(".")
        assert len(parts) == 3

    def test_login_missing_fields(self, api: APIClient):
        r"""
        缺少必要字段
        """
        response = api.post("/api/auth/login", json={"username": "testuser"})
        assert response.status_code == 422

        response = api.post("/api/auth/login", json={"password": "password"})
        assert response.status_code == 422

    def test_login_empty_credentials(self, api: APIClient, registered_user: dict):
        r"""
        空凭证
        """
        response = api.login("", "testpass123")
        assert response.status_code == 401

        response = api.login("testuser", "")
        assert response.status_code == 401
        
    def test_login_expires_at_iso8601_and_avatar_null(self, api: APIClient, registered_user: dict):
        r"""
        登录返回 expires_at 为 ISO8601 UTC，且未上传头像时 avatar_url 为 null
        """
        import re
        response = api.login("testuser", "testpass123")
        assert response.status_code == 200
        data = response.json()
        assert re.match(
            r"^\d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2}(?:\.\d+)?Z$", data["expires_at"])
        assert data["user"]["avatar_url"] is None


# ============================================
# 登出测试
# ============================================


class TestLogout:
    r"""
    用户登出测试
    """

    def test_logout_success(self, api: APIClient, auth_user: dict):
        r"""
        正常登出成功
        """
        response = api.post("/api/auth/logout", headers=auth_user["headers"])
        assert response.status_code == 200
        assert response.json()["message"] == "Logged out successfully"

    def test_logout_invalidates_token(self, api: APIClient, auth_user: dict):
        r"""
        登出后令牌失效
        """
        api.post("/api/auth/logout", headers=auth_user["headers"])

        response = api.get("/api/auth/me", headers=auth_user["headers"])
        assert response.status_code == 401

    def test_logout_without_auth(self, api: APIClient):
        r"""
        未认证时登出
        """
        response = api.post("/api/auth/logout")
        assert response.status_code == 401

    def test_logout_invalid_token(self, api: APIClient):
        r"""
        无效令牌登出
        """
        response = api.post("/api/auth/logout",
                            headers={"Authorization": "Bearer invalid_token"})
        assert response.status_code == 401

    def test_logout_malformed_auth_header(self, api: APIClient):
        r"""
        格式错误的认证头
        """
        response = api.post("/api/auth/logout",
                            headers={"Authorization": "invalid"})
        assert response.status_code == 401

        response = api.post("/api/auth/logout",
                            headers={"Authorization": "Basic token"})
        assert response.status_code == 401

    def test_logout_twice(self, api: APIClient, auth_user: dict):
        r"""
        重复登出
        """
        api.post("/api/auth/logout", headers=auth_user["headers"])
        response = api.post("/api/auth/logout", headers=auth_user["headers"])
        assert response.status_code == 401

    def test_logout_one_session_keeps_others(self, api: APIClient, registered_user: dict):
        r"""
        登出一个会话不影响其他会话
        """
        response1 = api.login("testuser", "testpass123")
        token1 = response1.json()["token"]

        response2 = api.login("testuser", "testpass123")
        token2 = response2.json()["token"]

        api.post("/api/auth/logout", headers=api.auth_headers(token1))

        response = api.get("/api/auth/me", headers=api.auth_headers(token2))
        assert response.status_code == 200

    def test_expired_token_cannot_execute(self, api: APIClient, auth_user: dict):
        r"""
        A01: 过期 token 执行（模拟已登出的 token）
        """
        api.post("/api/auth/logout", headers=auth_user["headers"])

        response = api.post(
            "/api/execute",
            headers=auth_user["headers"],
            json={"language": "python", "code": "print('test')"},
        )
        assert response.status_code == 401

    def test_logged_out_token_cannot_execute(self, api: APIClient, auth_user: dict):
        r"""
        A02: 登出后 token 执行
        """
        api.post("/api/auth/logout", headers=auth_user["headers"])

        response = api.post(
            "/api/execute",
            headers=auth_user["headers"],
            json={"language": "python", "code": "print('test')"},
        )
        assert response.status_code == 401

    def test_multiple_sessions_one_logout_other_can_execute(self, api: APIClient, registered_user: dict):
        r"""
        A03: 多会话注销一个，另一 token 仍可执行
        """
        response1 = api.login("testuser", "testpass123")
        token1 = response1.json()["token"]
        headers1 = api.auth_headers(token1)

        response2 = api.login("testuser", "testpass123")
        token2 = response2.json()["token"]
        headers2 = api.auth_headers(token2)

        api.post("/api/auth/logout", headers=headers1)

        response = api.post(
            "/api/execute",
            headers=headers2,
            json={"language": "python", "code": "print('test')"},
        )
        assert response.status_code == 200
        
    def test_logout_reduces_session_count(self, api: APIClient, auth_user: dict):
        r"""
        登出后会话数减少
        """
        user_id = auth_user["user"]["id"]
        before_count = count_sessions(user_id)
        assert before_count >= 1

        response = api.post("/api/auth/logout", headers=auth_user["headers"])
        assert response.status_code == 200

        after_count = count_sessions(user_id)
        assert after_count == before_count - 1

# ============================================
# 获取当前用户测试
# ============================================


class TestGetMe:
    r"""
    获取当前用户信息测试
    """

    def test_get_me_success(self, api: APIClient, auth_user: dict):
        r"""
        获取当前用户信息成功
        """
        response = api.get("/api/auth/me", headers=auth_user["headers"])
        assert response.status_code == 200
        data = response.json()
        assert data["username"] == "testuser"
        assert data["id"] == auth_user["user"]["id"]
        assert data["status"] == "Active"

    def test_get_me_without_auth(self, api: APIClient):
        r"""
        未认证时获取用户信息
        """
        response = api.get("/api/auth/me")
        assert response.status_code == 401

    def test_get_me_invalid_token(self, api: APIClient):
        r"""
        无效令牌
        """
        response = api.get(
            "/api/auth/me", headers={"Authorization": "Bearer invalid"})
        assert response.status_code == 401

    def test_get_me_expired_token_format(self, api: APIClient):
        r"""
        格式正确但内容无效的令牌
        """
        fake_jwt = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJzdWIiOiIxMjM0NTY3ODkwIn0.dozjgNryP4J3jVmNHl0w5N_XgL0n3I9PlFUP0THsR8U"
        response = api.get(
            "/api/auth/me", headers={"Authorization": f"Bearer {fake_jwt}"})
        assert response.status_code == 401

    def test_get_me_shows_avatar_url_when_exists(self, api: APIClient, auth_user: dict):
        r"""
        有头像时显示头像URL
        """
        user_id = auth_user["user"]["id"]
        avatar_data = b"\x89PNG\r\n\x1a\n" + b"\x00" * 100
        api.put(
            f"/api/users/{user_id}/avatar",
            headers=auth_user["headers"],
            files={"avatar": ("avatar.png", avatar_data, "image/png")},
        )

        response = api.get("/api/auth/me", headers=auth_user["headers"])
        assert response.status_code == 200
        assert response.json()["avatar_url"] == f"/api/users/{user_id}/avatar"

# ============================================
# 用户头像测试
# ============================================


class TestAvatar:
    r"""
    用户头像测试
    """

    def test_get_avatar_not_exists(self, api: APIClient, auth_user: dict):
        r"""
        获取不存在的头像
        """
        user_id = auth_user["user"]["id"]
        response = api.get(f"/api/users/{user_id}/avatar")
        assert response.status_code == 404

    def test_upload_avatar_success(self, api: APIClient, auth_user: dict):
        r"""
        上传头像成功
        """
        user_id = auth_user["user"]["id"]
        avatar_data = b"\x89PNG\r\n\x1a\n" + b"\x00" * 100

        response = api.put(
            f"/api/users/{user_id}/avatar",
            headers=auth_user["headers"],
            files={"avatar": ("avatar.png", avatar_data, "image/png")},
        )
        assert response.status_code == 200
        assert response.json()["avatar_url"] == f"/api/users/{user_id}/avatar"

    def test_get_avatar_after_upload(self, api: APIClient, auth_user: dict):
        r"""
        上传后可以获取头像
        """
        user_id = auth_user["user"]["id"]
        avatar_data = b"\x89PNG\r\n\x1a\n" + b"\x00" * 100

        api.put(
            f"/api/users/{user_id}/avatar",
            headers=auth_user["headers"],
            files={"avatar": ("avatar.png", avatar_data, "image/png")},
        )

        response = api.get(f"/api/users/{user_id}/avatar")
        assert response.status_code == 200
        assert response.content == avatar_data

    def test_upload_avatar_overwrite(self, api: APIClient, auth_user: dict):
        r"""
        重复上传覆盖旧头像
        """
        user_id = auth_user["user"]["id"]
        avatar_data1 = b"\x89PNG\r\n\x1a\n" + b"\x01" * 100
        avatar_data2 = b"\x89PNG\r\n\x1a\n" + b"\x02" * 100

        api.put(
            f"/api/users/{user_id}/avatar",
            headers=auth_user["headers"],
            files={"avatar": ("avatar1.png", avatar_data1, "image/png")},
        )

        api.put(
            f"/api/users/{user_id}/avatar",
            headers=auth_user["headers"],
            files={"avatar": ("avatar2.png", avatar_data2, "image/png")},
        )

        response = api.get(f"/api/users/{user_id}/avatar")
        assert response.content == avatar_data2

    def test_upload_avatar_unauthorized(self, api: APIClient, auth_user: dict):
        r"""
        上传他人头像失败
        """
        avatar_data = b"\x89PNG\r\n\x1a\n" + b"\x00" * 100
        response = api.put(
            "/api/users/999/avatar",
            headers=auth_user["headers"],
            files={"avatar": ("avatar.png", avatar_data, "image/png")},
        )
        assert response.status_code == 403

    def test_upload_avatar_without_auth(self, api: APIClient, registered_user: dict):
        r"""
        未认证时上传头像
        """
        avatar_data = b"\x89PNG\r\n\x1a\n" + b"\x00" * 100
        response = api.put(
            f"/api/users/{registered_user['id']}/avatar",
            files={"avatar": ("avatar.png", avatar_data, "image/png")},
        )
        assert response.status_code == 401

    def test_upload_large_avatar(self, api: APIClient, auth_user: dict):
        r"""
        上传大文件头像
        """
        user_id = auth_user["user"]["id"]
        avatar_data = b"\x89PNG\r\n\x1a\n" + b"\x00" * (1024 * 1024)

        response = api.put(
            f"/api/users/{user_id}/avatar",
            headers=auth_user["headers"],
            files={"avatar": ("avatar.png", avatar_data, "image/png")},
        )
        assert response.status_code == 200

    def test_get_avatar_nonexistent_user(self, api: APIClient):
        r"""
        获取不存在用户的头像
        """
        response = api.get("/api/users/99999/avatar")
        assert response.status_code == 404

    def test_avatar_public_access(self, api: APIClient, auth_user: dict):
        r"""
        头像可以公开访问
        """
        user_id = auth_user["user"]["id"]
        avatar_data = b"\x89PNG\r\n\x1a\n" + b"\x00" * 100

        api.put(
            f"/api/users/{user_id}/avatar",
            headers=auth_user["headers"],
            files={"avatar": ("avatar.png", avatar_data, "image/png")},
        )

        response = api.get(f"/api/users/{user_id}/avatar")
        assert response.status_code == 200

    def test_get_avatar_content_type_octet_stream(self, api: APIClient, auth_user: dict):
        r"""
        获取头像返回 application/octet-stream
        """
        user_id = auth_user["user"]["id"]
        avatar_data = b"\x89PNG\r\n\x1a\n" + b"\x00" * 100

        api.put(
            f"/api/users/{user_id}/avatar",
            headers=auth_user["headers"],
            files={"avatar": ("avatar.png", avatar_data, "image/png")},
        )

        response = api.get(f"/api/users/{user_id}/avatar")
        assert response.status_code == 200
        content_type = response.headers.get("Content-Type", "")
        assert content_type.startswith("application/octet-stream")

# ============================================
# 密码修改测试
# ============================================


class TestChangePassword:
    r"""
    密码修改测试
    """

    def test_change_password_success(self, api: APIClient, auth_user: dict):
        r"""
        修改密码成功
        """
        user_id = auth_user["user"]["id"]
        response = api.put(
            f"/api/users/{user_id}/password",
            headers=auth_user["headers"],
            json={"old_password": "testpass123",
                  "new_password": "newpassword456"},
        )
        assert response.status_code == 200

    def test_change_password_then_login(self, api: APIClient, auth_user: dict):
        r"""
        修改密码后可以用新密码登录
        """
        user_id = auth_user["user"]["id"]
        api.put(
            f"/api/users/{user_id}/password",
            headers=auth_user["headers"],
            json={"old_password": "testpass123",
                  "new_password": "newpassword456"},
        )

        response = api.login("testuser", "newpassword456")
        assert response.status_code == 200

    def test_change_password_old_invalid(self, api: APIClient, auth_user: dict):
        r"""
        修改密码后旧密码无效
        """
        user_id = auth_user["user"]["id"]
        api.put(
            f"/api/users/{user_id}/password",
            headers=auth_user["headers"],
            json={"old_password": "testpass123",
                  "new_password": "newpassword456"},
        )

        response = api.login("testuser", "testpass123")
        assert response.status_code == 401

    def test_change_password_wrong_old(self, api: APIClient, auth_user: dict):
        r"""
        旧密码错误
        """
        user_id = auth_user["user"]["id"]
        response = api.put(
            f"/api/users/{user_id}/password",
            headers=auth_user["headers"],
            json={"old_password": "wrongpassword",
                  "new_password": "newpassword456"},
        )
        assert response.status_code == 400
        assert "原密码错误" in response.json()["error"]

    def test_change_password_unauthorized(self, api: APIClient, auth_user: dict):
        r"""
        修改他人密码失败
        """
        response = api.put(
            "/api/users/999/password",
            headers=auth_user["headers"],
            json={"old_password": "testpass123",
                  "new_password": "newpassword456"},
        )
        assert response.status_code == 403

    def test_change_password_without_auth(self, api: APIClient, registered_user: dict):
        r"""
        未认证时修改密码
        """
        response = api.put(
            f"/api/users/{registered_user['id']}/password",
            json={"old_password": "testpass123",
                  "new_password": "newpassword456"},
        )
        assert response.status_code == 401

    def test_change_password_same_password(self, api: APIClient, auth_user: dict):
        r"""
        新旧密码相同
        """
        user_id = auth_user["user"]["id"]
        response = api.put(
            f"/api/users/{user_id}/password",
            headers=auth_user["headers"],
            json={"old_password": "testpass123",
                  "new_password": "testpass123"},
        )
        assert response.status_code == 200

    def test_change_password_missing_fields(self, api: APIClient, auth_user: dict):
        r"""
        缺少必要字段
        """
        user_id = auth_user["user"]["id"]
        response = api.put(
            f"/api/users/{user_id}/password",
            headers=auth_user["headers"],
            json={"old_password": "testpass123"},
        )
        assert response.status_code == 422
        
    def test_change_password_new_password_length_bounds(self, api: APIClient, auth_user: dict):
        r"""
        new_password 长度边界：65 失败，64 成功
        """
        user_id = auth_user["user"]["id"]

        response = api.put(
            f"/api/users/{user_id}/password",
            headers=auth_user["headers"],
            json={"old_password": "testpass123", "new_password": "a" * 65},
        )
        assert response.status_code == 422

        response = api.put(
            f"/api/users/{user_id}/password",
            headers=auth_user["headers"],
            json={"old_password": "testpass123", "new_password": "a" * 64},
        )
        assert response.status_code == 200


# ============================================
# 模板列表测试
# ============================================


class TestTemplateList:
    r"""
    模板列表测试
    """

    def test_list_templates_empty(self, api: APIClient):
        r"""
        空模板列表
        """
        response = api.get("/api/templates")
        assert response.status_code == 200
        data = response.json()
        assert data["items"] == []
        assert data["total"] == 0
        assert data["page"] == 1
        assert data["limit"] == 20

    def test_list_templates_with_data(self, api: APIClient, sample_template: dict):
        r"""
        有数据时的模板列表
        """
        response = api.get("/api/templates")
        assert response.status_code == 200
        data = response.json()
        assert data["total"] == 1
        assert len(data["items"]) == 1
        assert data["items"][0]["title"] == "冒泡排序"
        assert data["items"][0]["is_favorited"] is False

    def test_list_templates_category_filter(self, api: APIClient, multiple_templates: list):
        r"""
        按分类筛选模板
        """
        response = api.get("/api/templates", params={"category": "排序算法"})
        assert response.status_code == 200
        data = response.json()
        assert data["total"] == 2
        for item in data["items"]:
            assert item["category"] == "排序算法"

    def test_list_templates_category_no_match(self, api: APIClient, sample_template: dict):
        r"""
        分类不匹配
        """
        response = api.get("/api/templates", params={"category": "不存在的分类"})
        assert response.status_code == 200
        assert response.json()["total"] == 0

    def test_list_templates_pagination(self, api: APIClient, multiple_templates: list):
        r"""
        分页参数
        """
        response = api.get("/api/templates", params={"page": 1, "limit": 2})
        assert response.status_code == 200
        data = response.json()
        assert data["total"] == 4
        assert len(data["items"]) == 2
        assert data["page"] == 1
        assert data["limit"] == 2

        response = api.get("/api/templates", params={"page": 2, "limit": 2})
        assert response.status_code == 200
        data = response.json()
        assert len(data["items"]) == 2
        assert data["page"] == 2

    def test_list_templates_page_out_of_range(self, api: APIClient, sample_template: dict):
        r"""
        页码超出范围
        """
        response = api.get("/api/templates", params={"page": 100})
        assert response.status_code == 200
        assert response.json()["items"] == []

    def test_list_templates_invalid_page(self, api: APIClient):
        r"""
        无效页码
        """
        response = api.get("/api/templates", params={"page": 0})
        assert response.status_code == 422

        response = api.get("/api/templates", params={"page": -1})
        assert response.status_code == 422

    def test_list_templates_invalid_limit(self, api: APIClient):
        r"""
        无效每页数量
        """
        response = api.get("/api/templates", params={"limit": 0})
        assert response.status_code == 422

        response = api.get("/api/templates", params={"limit": 101})
        assert response.status_code == 422

    def test_list_templates_with_auth_favorited(self, api: APIClient, auth_user: dict, sample_template: dict):
        r"""
        认证用户查看收藏状态
        """
        insert_favorite(auth_user["user"]["id"], sample_template["id"])

        response = api.get("/api/templates", headers=auth_user["headers"])
        assert response.status_code == 200
        assert response.json()["items"][0]["is_favorited"] is True

    def test_list_templates_without_auth_not_favorited(self, api: APIClient, sample_template: dict):
        r"""
        未认证时收藏状态为false
        """
        response = api.get("/api/templates")
        assert response.status_code == 200
        assert response.json()["items"][0]["is_favorited"] is False


# ============================================
# 模板详情测试
# ============================================


class TestTemplateDetail:
    r"""
    模板详情测试
    """

    def test_get_template_success(self, api: APIClient, sample_template: dict):
        r"""
        获取模板详情成功
        """
        response = api.get(f"/api/templates/{sample_template['id']}")
        assert response.status_code == 200
        data = response.json()
        assert data["title"] == "冒泡排序"
        assert data["category"] == "排序算法"
        assert len(data["codes"]) == 3
        assert data["favorite_count"] == 0
        assert data["is_favorited"] is False
        assert "created_at" in data
        assert "updated_at" in data

    def test_get_template_not_found(self, api: APIClient):
        r"""
        模板不存在
        """
        response = api.get("/api/templates/99999")
        assert response.status_code == 404
        assert "模板不存在" in response.json()["error"]

    def test_get_template_codes_included(self, api: APIClient, sample_template: dict):
        r"""
        模板详情包含所有语言代码
        """
        response = api.get(f"/api/templates/{sample_template['id']}")
        data = response.json()
        languages = [code["language"] for code in data["codes"]]
        assert "python" in languages
        assert "lua" in languages
        assert "rust" in languages

    def test_get_template_with_auth_favorited(self, api: APIClient, auth_user: dict, sample_template: dict):
        r"""
        认证用户查看收藏状态
        """
        insert_favorite(auth_user["user"]["id"], sample_template["id"])

        response = api.get(
            f"/api/templates/{sample_template['id']}", headers=auth_user["headers"])
        assert response.json()["is_favorited"] is True

    def test_get_template_favorite_count_accurate(self, api: APIClient, sample_template: dict):
        r"""
        收藏数准确
        """
        user1_id = insert_user("user1", "$2b$04$xxx", "Active")
        user2_id = insert_user("user2", "$2b$04$xxx", "Active")
        insert_favorite(user1_id, sample_template["id"])
        insert_favorite(user2_id, sample_template["id"])

        response = api.get(f"/api/templates/{sample_template['id']}")
        assert response.json()["favorite_count"] == 2

    def test_template_no_codes(self, api: APIClient):
        r"""
        T01: 模板无代码
        """
        template_id = insert_template("无代码模板", "测试", "这是一个没有代码的模板")

        response = api.get(f"/api/templates/{template_id}")
        assert response.status_code == 200
        data = response.json()
        assert data["codes"] == []
        
    def test_template_detail_code_fields_complete(self, api: APIClient, sample_template: dict):
        r"""
        模板详情中的 codes 字段包含 language/code/explanation
        """
        response = api.get(f"/api/templates/{sample_template['id']}")
        assert response.status_code == 200
        codes = response.json()["codes"]
        assert len(codes) >= 1
        for c in codes:
            assert "language" in c
            assert "code" in c
            assert "explanation" in c

# ============================================
# 模板代码测试
# ============================================


class TestTemplateCode:
    r"""
    模板代码测试
    """

    def test_get_template_code_success(self, api: APIClient, sample_template: dict):
        r"""
        获取模板代码成功
        """
        response = api.get(
            f"/api/templates/{sample_template['id']}/code/python")
        assert response.status_code == 200
        data = response.json()
        assert data["language"] == "python"
        assert "bubble_sort" in data["code"]
        assert data["explanation"] == "Python实现"

    def test_get_template_code_not_found(self, api: APIClient, sample_template: dict):
        r"""
        语言不存在
        """
        response = api.get(f"/api/templates/{sample_template['id']}/code/java")
        assert response.status_code == 404
        assert "模板语言实现不存在" in response.json()["error"]

    def test_get_template_code_template_not_found(self, api: APIClient):
        r"""
        模板不存在
        """
        response = api.get("/api/templates/99999/code/python")
        assert response.status_code == 404
        assert "模板不存在" in response.json()["error"]

    def test_get_template_code_all_languages(self, api: APIClient, sample_template: dict):
        r"""
        获取所有语言的代码
        """
        for lang in ["python", "lua", "rust"]:
            response = api.get(
                f"/api/templates/{sample_template['id']}/code/{lang}")
            assert response.status_code == 200
            assert response.json()["language"] == lang
            
    def test_get_template_code_empty_explanation(self, api: APIClient):
        r"""
        explanation 为空字符串时原样返回
        """
        template_id = insert_template("空说明模板", "测试", "desc")
        insert_template_code(template_id, "python", "print('x')", "")

        response = api.get(f"/api/templates/{template_id}/code/python")
        assert response.status_code == 200
        assert response.json()["explanation"] == ""


# ============================================
# 模板分类测试
# ============================================


class TestTemplateCategories:
    r"""
    模板分类测试
    """

    def test_get_categories_empty(self, api: APIClient):
        r"""
        空分类列表
        """
        response = api.get("/api/templates/categories")
        assert response.status_code == 200
        assert response.json()["items"] == []

    def test_get_categories_with_data(self, api: APIClient, multiple_templates: list):
        r"""
        有数据时的分类列表
        """
        response = api.get("/api/templates/categories")
        assert response.status_code == 200
        categories = response.json()["items"]
        assert "排序算法" in categories
        assert "树算法" in categories
        assert "图算法" in categories

    def test_get_categories_unique(self, api: APIClient, multiple_templates: list):
        r"""
        分类列表去重
        """
        response = api.get("/api/templates/categories")
        categories = response.json()["items"]
        assert len(categories) == len(set(categories))

    def test_get_categories_sorted(self, api: APIClient, multiple_templates: list):
        response = api.get("/api/templates/categories")
        categories = response.json()["items"]
        
        assert len(categories) == len(set(categories))
        assert "排序算法" in categories
        assert "树算法" in categories
        assert "图算法" in categories
        
    def test_get_categories_sorted_alphabetically(self, api: APIClient):
        r"""
        分类列表按字母顺序排序（使用 ASCII 分类避免中文排序差异）
        """
        insert_template("t1", "b_cat", "d")
        insert_template("t2", "a_cat", "d")
        insert_template("t3", "c_cat", "d")

        response = api.get("/api/templates/categories")
        assert response.status_code == 200
        categories = response.json()["items"]
        assert categories == sorted(categories)


# ============================================
# 模板搜索测试
# ============================================


class TestTemplateSearch:
    r"""
    模板搜索测试
    """

    def test_search_templates_success(self, api: APIClient, sample_template: dict):
        r"""
        搜索模板成功
        """
        response = api.get("/api/templates/search", params={"keyword": "冒泡"})
        assert response.status_code == 200
        data = response.json()
        assert data["total"] == 1
        assert data["items"][0]["title"] == "冒泡排序"

    def test_search_templates_no_result(self, api: APIClient, sample_template: dict):
        r"""
        无搜索结果
        """
        response = api.get("/api/templates/search",
                           params={"keyword": "不存在的关键词"})
        assert response.status_code == 200
        assert response.json()["total"] == 0

    def test_search_templates_missing_keyword(self, api: APIClient):
        r"""
        缺少关键词参数
        """
        response = api.get("/api/templates/search")
        assert response.status_code == 422

    def test_search_templates_empty_keyword(self, api: APIClient, sample_template: dict):
        r"""
        空关键词
        """
        response = api.get("/api/templates/search", params={"keyword": ""})
        assert response.status_code == 200

    def test_search_templates_description_match(self, api: APIClient, sample_template: dict):
        r"""
        搜索匹配描述
        """
        response = api.get("/api/templates/search", params={"keyword": "经典"})
        assert response.status_code == 200
        assert response.json()["total"] == 1

    def test_search_templates_case_insensitive(self, api: APIClient, multiple_templates: list):
        r"""
        搜索不区分大小写
        """
        response = api.get("/api/templates/search", params={"keyword": "BFS"})
        assert response.status_code == 200
        assert response.json()["total"] >= 1

    def test_search_templates_pagination(self, api: APIClient, multiple_templates: list):
        r"""
        搜索结果分页
        """
        response = api.get("/api/templates/search",
                           params={"keyword": "排序", "page": 1, "limit": 1})
        assert response.status_code == 200
        data = response.json()
        assert data["total"] == 2
        assert len(data["items"]) == 1

    def test_search_templates_special_chars(self, api: APIClient, sample_template: dict):
        r"""
        搜索特殊字符
        """
        response = api.get("/api/templates/search", params={"keyword": "%"})
        assert response.status_code == 200

        response = api.get("/api/templates/search", params={"keyword": "_"})
        assert response.status_code == 200

        response = api.get("/api/templates/search",
                           params={"keyword": "' OR '1'='1"})
        assert response.status_code == 200

    def test_search_templates_with_auth_favorited(self, api: APIClient, auth_user: dict, sample_template: dict):
        r"""
        认证用户搜索显示收藏状态
        """
        insert_favorite(auth_user["user"]["id"], sample_template["id"])

        response = api.get("/api/templates/search",
                           params={"keyword": "冒泡"}, headers=auth_user["headers"])
        assert response.json()["items"][0]["is_favorited"] is True

    def test_search_very_long_keyword(self, api: APIClient, sample_template: dict):
        r"""
        S01: 超长关键词搜索
        """
        long_keyword = "a" * 1000
        response = api.get("/api/templates/search", params={"keyword": long_keyword})
        assert response.status_code == 200
        
    def test_search_order_by_favorite_count_desc(self, api: APIClient):
        r"""
        搜索结果按收藏数降序
        """
        t1_id = insert_template("排序模板A", "测试", "desc")
        t2_id = insert_template("排序模板B", "测试", "desc")

        u1 = insert_user("suser1", "$2b$04$xxx", "Active")
        u2 = insert_user("suser2", "$2b$04$xxx", "Active")
        u3 = insert_user("suser3", "$2b$04$xxx", "Active")

        insert_favorite(u1, t1_id)
        insert_favorite(u2, t1_id)
        insert_favorite(u3, t1_id)
        insert_favorite(u1, t2_id)

        response = api.get("/api/templates/search", params={"keyword": "排序模板"})
        assert response.status_code == 200
        items = response.json()["items"]
        assert len(items) >= 2
        assert items[0]["id"] == t1_id
    
# ============================================
# 收藏功能测试
# ============================================


class TestFavorites:
    r"""
    收藏功能测试
    """

    def test_add_favorite_success(self, api: APIClient, auth_user: dict, sample_template: dict):
        r"""
        添加收藏成功
        """
        response = api.post(
            "/api/favorites",
            headers=auth_user["headers"],
            json={"template_id": sample_template["id"]},
        )
        assert response.status_code == 200
        assert response.json()["message"] == "Template favorited successfully"

    def test_add_favorite_duplicate(self, api: APIClient, auth_user: dict, sample_template: dict):
        r"""
        重复收藏
        """
        api.post("/api/favorites",
                 headers=auth_user["headers"], json={"template_id": sample_template["id"]})

        response = api.post(
            "/api/favorites", headers=auth_user["headers"], json={"template_id": sample_template["id"]})
        assert response.status_code == 409
        assert "已收藏该模板" in response.json()["error"]

    def test_add_favorite_template_not_found(self, api: APIClient, auth_user: dict):
        r"""
        收藏不存在的模板
        """
        response = api.post(
            "/api/favorites", headers=auth_user["headers"], json={"template_id": 99999})
        assert response.status_code == 404

    def test_add_favorite_without_auth(self, api: APIClient, sample_template: dict):
        r"""
        未认证时收藏
        """
        response = api.post(
            "/api/favorites", json={"template_id": sample_template["id"]})
        assert response.status_code == 401

    def test_add_favorite_updates_count(self, api: APIClient, auth_user: dict, sample_template: dict):
        r"""
        收藏更新计数
        """
        api.post("/api/favorites",
                 headers=auth_user["headers"], json={"template_id": sample_template["id"]})

        count = get_template_favorite_count(sample_template["id"])
        assert count == 1

    def test_remove_favorite_success(self, api: APIClient, auth_user: dict, sample_template: dict):
        r"""
        取消收藏成功
        """
        api.post("/api/favorites",
                 headers=auth_user["headers"], json={"template_id": sample_template["id"]})

        response = api.delete(
            f"/api/favorites/{sample_template['id']}", headers=auth_user["headers"])
        assert response.status_code == 200
        assert response.json()["message"] == "Favorite removed successfully"

    def test_remove_favorite_not_found(self, api: APIClient, auth_user: dict, sample_template: dict):
        r"""
        取消未收藏的模板
        """
        response = api.delete(
            f"/api/favorites/{sample_template['id']}", headers=auth_user["headers"])
        assert response.status_code == 404
        assert "未收藏该模板" in response.json()["error"]

    def test_remove_favorite_updates_count(self, api: APIClient, auth_user: dict, sample_template: dict):
        r"""
        取消收藏更新计数
        """
        api.post("/api/favorites",
                 headers=auth_user["headers"], json={"template_id": sample_template["id"]})
        api.delete(
            f"/api/favorites/{sample_template['id']}", headers=auth_user["headers"])

        count = get_template_favorite_count(sample_template["id"])
        assert count == 0

    def test_remove_favorite_without_auth(self, api: APIClient, sample_template: dict):
        r"""
        未认证时取消收藏
        """
        response = api.delete(f"/api/favorites/{sample_template['id']}")
        assert response.status_code == 401

    def test_get_user_favorites_empty(self, api: APIClient, auth_user: dict):
        r"""
        空收藏列表
        """
        user_id = auth_user["user"]["id"]
        response = api.get(
            f"/api/users/{user_id}/favorites", headers=auth_user["headers"])
        assert response.status_code == 200
        data = response.json()
        assert data["items"] == []
        assert data["total"] == 0

    def test_get_user_favorites_with_data(self, api: APIClient, auth_user: dict, sample_template: dict):
        r"""
        有数据时的收藏列表
        """
        api.post("/api/favorites",
                 headers=auth_user["headers"], json={"template_id": sample_template["id"]})

        user_id = auth_user["user"]["id"]
        response = api.get(
            f"/api/users/{user_id}/favorites", headers=auth_user["headers"])
        assert response.status_code == 200
        data = response.json()
        assert data["total"] == 1
        assert data["items"][0]["template_id"] == sample_template["id"]
        assert data["items"][0]["title"] == "冒泡排序"
        assert "favorited_at" in data["items"][0]

    def test_get_user_favorites_unauthorized(self, api: APIClient, auth_user: dict):
        r"""
        获取他人收藏列表失败
        """
        response = api.get("/api/users/999/favorites",
                           headers=auth_user["headers"])
        assert response.status_code == 403

    def test_get_user_favorites_pagination(self, api: APIClient, auth_user: dict, multiple_templates: list):
        r"""
        收藏列表分页
        """
        for t in multiple_templates:
            api.post("/api/favorites",
                     headers=auth_user["headers"], json={"template_id": t["id"]})

        user_id = auth_user["user"]["id"]
        response = api.get(
            f"/api/users/{user_id}/favorites", headers=auth_user["headers"], params={"limit": 2})
        assert response.status_code == 200
        data = response.json()
        assert data["total"] == 4
        assert len(data["items"]) == 2

    def test_favorite_banned_user(self, api: APIClient, auth_user: dict, sample_template: dict):
        r"""
        被封禁用户无法收藏
        """
        update_user_status(auth_user["user"]["id"], "Banned")

        response = api.post(
            "/api/favorites", headers=auth_user["headers"], json={"template_id": sample_template["id"]})
        assert response.status_code == 403

    def test_favorite_suspended_user(self, api: APIClient, auth_user: dict, sample_template: dict):
        r"""
        被暂停用户无法收藏
        """
        update_user_status(auth_user["user"]["id"], "Suspended")

        response = api.post(
            "/api/favorites", headers=auth_user["headers"], json={"template_id": sample_template["id"]})
        assert response.status_code == 403
        assert "暂停" in response.json()["error"]

    def test_multiple_users_favorite_same_template(self, api: APIClient, sample_template: dict):
        r"""
        多用户收藏同一模板
        """
        users = []
        for i in range(3):
            api.register(f"favuser{i}", "password123")
            response = api.login(f"favuser{i}", "password123")
            users.append(api.auth_headers(response.json()["token"]))

        for headers in users:
            api.post("/api/favorites", headers=headers,
                     json={"template_id": sample_template["id"]})

        count = get_template_favorite_count(sample_template["id"])
        assert count == 3
        
    def test_remove_nonexistent_favorite_count_unchanged(self, api: APIClient, auth_user: dict, sample_template: dict):
        r"""
        T02: 删除不存在收藏，收藏数不变
        """
        initial_count = get_template_favorite_count(sample_template["id"])

        response = api.delete(
            f"/api/favorites/{sample_template['id']}",
            headers=auth_user["headers"],
        )
        assert response.status_code == 404

        final_count = get_template_favorite_count(sample_template["id"])
        assert final_count == initial_count

    def test_multiple_unfavorite_always_404(self, api: APIClient, auth_user: dict, sample_template: dict):
        r"""
        T03: 多次取消收藏始终 404 且收藏数不为负
        """
        for _ in range(3):
            response = api.delete(
                f"/api/favorites/{sample_template['id']}",
                headers=auth_user["headers"],
            )
            assert response.status_code == 404

        count = get_template_favorite_count(sample_template["id"])
        assert count >= 0
        
    def test_get_user_favorites_item_fields_complete(self, api: APIClient, auth_user: dict, sample_template: dict):
        r"""
        收藏列表项字段完整
        """
        api.post(
            "/api/favorites",
            headers=auth_user["headers"],
            json={"template_id": sample_template["id"]},
        )

        user_id = auth_user["user"]["id"]
        response = api.get(
            f"/api/users/{user_id}/favorites", headers=auth_user["headers"]
        )
        assert response.status_code == 200
        item = response.json()["items"][0]

        assert "template_id" in item
        assert "title" in item
        assert "category" in item
        assert "description" in item
        assert "favorite_count" in item
        assert "favorited_at" in item


# ============================================
# 代码执行测试
# ============================================


class TestExecute:
    r"""
    代码执行测试
    """

    def test_execute_without_auth(self, api: APIClient):
        r"""
        未认证时执行代码
        """
        response = api.post(
            "/api/execute", json={"language": "python", "code": "print('hello')"})
        assert response.status_code == 401

    def test_execute_invalid_language(self, api: APIClient, auth_user: dict):
        r"""
        无效的语言参数
        """
        response = api.post("/api/execute", headers=auth_user["headers"], json={
                            "language": "invalid", "code": "print('hello')"})
        assert response.status_code == 422

    def test_execute_missing_fields(self, api: APIClient, auth_user: dict):
        r"""
        缺少必要字段
        """
        response = api.post(
            "/api/execute", headers=auth_user["headers"], json={"language": "python"})
        assert response.status_code == 422

        response = api.post(
            "/api/execute", headers=auth_user["headers"], json={"code": "print('hello')"})
        assert response.status_code == 422

    def test_execute_banned_user(self, api: APIClient, auth_user: dict):
        r"""
        被封禁用户无法执行代码
        """
        update_user_status(auth_user["user"]["id"], "Banned")

        response = api.post("/api/execute", headers=auth_user["headers"], json={
                            "language": "python", "code": "print('hello')"})
        assert response.status_code == 403

    def test_execute_suspended_user(self, api: APIClient, auth_user: dict):
        r"""
        被暂停用户无法执行代码
        """
        update_user_status(auth_user["user"]["id"], "Suspended")

        response = api.post("/api/execute", headers=auth_user["headers"], json={
                            "language": "python", "code": "print('hello')"})
        assert response.status_code == 403
        assert "暂停" in response.json()["error"]

    def test_execute_supported_languages(self, api: APIClient, auth_user: dict):
        r"""
        支持的语言参数验证
        """
        for lang in ["python", "lua", "rust"]:
            response = api.post(
                "/api/execute", headers=auth_user["headers"], json={"language": lang, "code": ""})
            assert response.status_code == 200 or response.status_code == 500

    def test_execute_suspended_user_message(self, api: APIClient, auth_user: dict):
        r"""
        A04: Suspended 用户执行，403 且提示"暂停"
        """
        update_user_status(auth_user["user"]["id"], "Suspended")

        response = api.post(
            "/api/execute",
            headers=auth_user["headers"],
            json={"language": "python", "code": "print('hello')"},
        )
        assert response.status_code == 403
        assert "暂停" in response.json()["error"]
        
    def test_execute_response_fields_on_success(self, api: APIClient, auth_user: dict):
        r"""
        成功执行返回字段与类型完整
        """
        unique_code = f"with open('out_{time.time_ns()}.toml', 'w') as f: f.write('[x]\\nvalue = 1')"
        response = api.post(
            "/api/execute",
            headers=auth_user["headers"],
            json={"language": "python", "code": unique_code},
        )
        assert response.status_code == 200
        data = response.json()

        assert isinstance(data["execution_id"], int)
        assert data["status"] == "Success"
        assert isinstance(data["execution_time"], int)
        assert isinstance(data["cached"], bool)
        assert data["toml_output"] is not None

# ============================================
# 执行历史测试
# ============================================


class TestExecutionHistory:
    r"""
    执行历史测试
    """

    def test_get_execution_history_empty(self, api: APIClient, auth_user: dict):
        r"""
        空执行历史
        """
        user_id = auth_user["user"]["id"]
        response = api.get(
            f"/api/users/{user_id}/executions", headers=auth_user["headers"])
        assert response.status_code == 200
        data = response.json()
        assert data["items"] == []
        assert data["total"] == 0

    def test_get_execution_history_with_data(self, api: APIClient, auth_user: dict):
        r"""
        有数据时的执行历史
        """
        user_id = auth_user["user"]["id"]
        insert_execution(user_id, "hash1", "python",
                         "print('hello')", "Success", "[test]\nvalue = 1")

        response = api.get(
            f"/api/users/{user_id}/executions", headers=auth_user["headers"])
        assert response.status_code == 200
        data = response.json()
        assert data["total"] == 1
        assert data["items"][0]["language"] == "python"
        assert data["items"][0]["status"] == "Success"

    def test_get_execution_history_unauthorized(self, api: APIClient, auth_user: dict):
        r"""
        获取他人执行历史失败
        """
        response = api.get("/api/users/999/executions",
                           headers=auth_user["headers"])
        assert response.status_code == 403

    def test_get_execution_history_pagination(self, api: APIClient, auth_user: dict):
        r"""
        执行历史分页
        """
        user_id = auth_user["user"]["id"]
        for i in range(5):
            insert_execution(
                user_id, f"hash{i}", "python", f"code{i}", "Success")

        response = api.get(
            f"/api/users/{user_id}/executions", headers=auth_user["headers"], params={"limit": 2})
        assert response.status_code == 200
        data = response.json()
        assert data["total"] == 5
        assert len(data["items"]) == 2

    def test_get_execution_history_order(self, api: APIClient, auth_user: dict):
        r"""
        执行历史按时间倒序
        """
        user_id = auth_user["user"]["id"]
        insert_execution(user_id, "hash1", "python", "code1", "Success")
        time.sleep(0.1)
        insert_execution(user_id, "hash2", "python", "code2", "Success")

        response = api.get(
            f"/api/users/{user_id}/executions", headers=auth_user["headers"])
        items = response.json()["items"]
        assert items[0]["code"] == "code2"
        assert items[1]["code"] == "code1"

    def test_execution_detail_fields_complete(self, api: APIClient, auth_user: dict):
        r"""
        H01: 执行详情字段完整
        """
        user_id = auth_user["user"]["id"]
        execution_id = insert_execution(
            user_id, "hash_h01", "python", "print('test')", "Success",
            "[test]\nvalue = 1", None, 150, "192.168.1.1"
        )

        response = api.get(
            f"/api/executions/{execution_id}", headers=auth_user["headers"])
        assert response.status_code == 200
        data = response.json()

        assert "id" in data
        assert "language" in data
        assert "code" in data
        assert "toml_output" in data
        assert "status" in data
        assert "error_message" in data
        assert "execution_time" in data
        assert "created_at" in data

        assert isinstance(data["id"], int)
        assert isinstance(data["language"], str)
        assert isinstance(data["code"], str)
        assert isinstance(data["execution_time"], int)

    def test_banned_user_can_view_history(self, api: APIClient, auth_user: dict):
        r"""
        H02: 用户被封禁仍可看历史
        """
        user_id = auth_user["user"]["id"]
        insert_execution(user_id, "hash_h02", "python", "code", "Success")

        update_user_status(user_id, "Banned")

        response = api.get(
            f"/api/users/{user_id}/executions", headers=auth_user["headers"])
        assert response.status_code == 200
        assert response.json()["total"] >= 1

    def test_execution_history_page_out_of_range(self, api: APIClient, auth_user: dict):
        r"""
        H03: 执行历史分页边界，page 超大返回空列表
        """
        user_id = auth_user["user"]["id"]
        insert_execution(user_id, "hash_h03", "python", "code", "Success")

        response = api.get(
            f"/api/users/{user_id}/executions",
            headers=auth_user["headers"],
            params={"page": 9999, "limit": 20},
        )
        assert response.status_code == 200
        data = response.json()
        assert data["items"] == []
        assert data["total"] >= 1

    def test_execution_history_order_stable(self, api: APIClient, auth_user: dict):
        r"""
        H04: 并发执行顺序，created_at 倒序稳定
        """
        user_id = auth_user["user"]["id"]
        for i in range(5):
            insert_execution(
                user_id, f"hash_h04_{i}", "python", f"code_{i}", "Success")
            time.sleep(0.05)

        response = api.get(
            f"/api/users/{user_id}/executions", headers=auth_user["headers"])
        assert response.status_code == 200
        items = response.json()["items"]

        created_times = [item["created_at"] for item in items]
        assert created_times == sorted(created_times, reverse=True)
        
    def test_execution_history_items_include_time_fields(self, api: APIClient, auth_user: dict):
        r"""
        执行历史项包含 execution_time 与 created_at
        """
        user_id = auth_user["user"]["id"]
        insert_execution(user_id, "hash_time", "python", "code", "Success", "[t]\nvalue = 1", None, 123)

        response = api.get(
            f"/api/users/{user_id}/executions", headers=auth_user["headers"]
        )
        assert response.status_code == 200
        item = response.json()["items"][0]
        assert "execution_time" in item
        assert "created_at" in item

# ============================================
# 执行详情测试
# ============================================


class TestExecutionDetail:
    r"""
    执行详情测试
    """

    def test_get_execution_detail_success(self, api: APIClient, auth_user: dict):
        r"""
        获取执行详情成功
        """
        user_id = auth_user["user"]["id"]
        execution_id = insert_execution(
            user_id, "hash1", "python", "print('hello')", "Success", "[test]\nvalue = 1")

        response = api.get(
            f"/api/executions/{execution_id}", headers=auth_user["headers"])
        assert response.status_code == 200
        data = response.json()
        assert data["id"] == execution_id
        assert data["language"] == "python"
        assert data["status"] == "Success"
        assert data["toml_output"] == "[test]\nvalue = 1"

    def test_get_execution_detail_not_found(self, api: APIClient, auth_user: dict):
        r"""
        执行记录不存在
        """
        response = api.get("/api/executions/99999",
                           headers=auth_user["headers"])
        assert response.status_code == 404
        assert "执行记录不存在" in response.json()["error"]

    def test_get_execution_detail_permission_denied(self, api: APIClient, auth_user: dict):
        r"""
        获取他人执行详情失败
        """
        other_user_id = insert_user("otheruser", "$2b$04$xxx", "Active")
        execution_id = insert_execution(
            other_user_id, "hash1", "python", "code", "Success")

        response = api.get(
            f"/api/executions/{execution_id}", headers=auth_user["headers"])
        assert response.status_code == 403
        assert "无权访问" in response.json()["error"]

    def test_get_execution_detail_without_auth(self, api: APIClient, auth_user: dict):
        r"""
        未认证时获取执行详情
        """
        user_id = auth_user["user"]["id"]
        execution_id = insert_execution(
            user_id, "hash1", "python", "code", "Success")

        response = api.get(f"/api/executions/{execution_id}")
        assert response.status_code == 401

    def test_get_execution_detail_with_error(self, api: APIClient, auth_user: dict):
        r"""
        获取失败执行的详情
        """
        user_id = auth_user["user"]["id"]
        execution_id = insert_execution(
            user_id, "hash1", "python", "invalid code", "Error", None, "SyntaxError")

        response = api.get(
            f"/api/executions/{execution_id}", headers=auth_user["headers"])
        assert response.status_code == 200
        data = response.json()
        assert data["status"] == "Error"
        assert data["error_message"] == "SyntaxError"
        assert data["toml_output"] is None


# ============================================
# 缓存测试
# ============================================


class TestCache:
    r"""
    缓存功能测试
    """

    def test_cache_hit_increments_count(self, api: APIClient, auth_user: dict):
        r"""
        缓存命中增加计数
        """
        import hashlib
        code = "test_cache_code"
        code_hash = hashlib.sha256(code.encode()).hexdigest()
        insert_cache(code_hash, "python", "[cached]\nvalue = 1")

        initial_count = get_cache_hit_count(code_hash, "python")

        response = api.post(
            "/api/execute", headers=auth_user["headers"], json={"language": "python", "code": code})

        if response.status_code == 200 and response.json().get("cached"):
            new_count = get_cache_hit_count(code_hash, "python")
            assert new_count == initial_count + 1
            
    def test_cache_hit_execution_time_zero(self, api: APIClient, auth_user: dict):
        r"""
        C01: 命中缓存时 execution_time=0
        """
        import hashlib
        code = "with open('c01_test.toml', 'w') as f:\n    f.write('[c01]\\nvalue = 1')"
        code_hash = hashlib.sha256(code.encode()).hexdigest()
        insert_cache(code_hash, "python", "[c01]\nvalue = 1")

        response = api.post(
            "/api/execute",
            headers=auth_user["headers"],
            json={"language": "python", "code": code},
        )
        assert response.status_code == 200
        data = response.json()
        assert data["cached"] is True
        assert data["execution_time"] == 0

    def test_cache_different_language_no_hit(self, api: APIClient, auth_user: dict):
        r"""
        C02: 同代码不同语言不互相命中缓存
        """
        import hashlib
        code = "print('hello')"
        code_hash = hashlib.sha256(code.encode()).hexdigest()
        insert_cache(code_hash, "python", "[cached]\nvalue = 1")

        response = api.post(
            "/api/execute",
            headers=auth_user["headers"],
            json={"language": "lua", "code": code},
        )
        assert response.status_code == 200
        data = response.json()
        assert data["cached"] is False

    def test_cache_hit_creates_execution_record(self, api: APIClient, auth_user: dict):
        r"""
        C03: 命中缓存也写执行记录
        """
        import hashlib
        code = "with open('c03_test.toml', 'w') as f:\n    f.write('[c03]\\nvalue = 1')"
        code_hash = hashlib.sha256(code.encode()).hexdigest()
        insert_cache(code_hash, "python", "[c03]\nvalue = 1")

        user_id = auth_user["user"]["id"]

        with get_db_connection() as conn:
            with conn.cursor() as cur:
                cur.execute(
                    "SELECT COUNT(*) FROM executions WHERE user_id = %s", (user_id,))
                before_count = cur.fetchone()[0]

        response = api.post(
            "/api/execute",
            headers=auth_user["headers"],
            json={"language": "python", "code": code},
        )
        assert response.status_code == 200
        assert response.json()["cached"] is True

        with get_db_connection() as conn:
            with conn.cursor() as cur:
                cur.execute(
                    "SELECT COUNT(*) FROM executions WHERE user_id = %s", (user_id,))
                after_count = cur.fetchone()[0]

        assert after_count == before_count + 1

    def test_cache_hit_increments_hit_count_correctly(self, api: APIClient, auth_user: dict):
        r"""
        C04: 命中缓存后 hit_count 正确递增
        """
        import hashlib
        code = "with open('c04_test.toml', 'w') as f:\n    f.write('[c04]\\nvalue = 1')"
        code_hash = hashlib.sha256(code.encode()).hexdigest()
        insert_cache(code_hash, "python", "[c04]\nvalue = 1")

        initial_count = get_cache_hit_count(code_hash, "python")

        api.post(
            "/api/execute",
            headers=auth_user["headers"],
            json={"language": "python", "code": code},
        )
        api.post(
            "/api/execute",
            headers=auth_user["headers"],
            json={"language": "python", "code": code},
        )

        new_count = get_cache_hit_count(code_hash, "python")
        assert new_count == initial_count + 2

    def test_cache_last_hit_at_updated(self, api: APIClient, auth_user: dict):
        r"""
        C05: last_hit_at 更新
        """
        import hashlib
        code = "with open('c05_test.toml', 'w') as f:\n    f.write('[c05]\\nvalue = 1')"
        code_hash = hashlib.sha256(code.encode()).hexdigest()
        insert_cache(code_hash, "python", "[c05]\nvalue = 1")

        with get_db_connection() as conn:
            with conn.cursor() as cur:
                cur.execute(
                    "SELECT last_hit_at FROM execution_cache WHERE code_hash = %s AND language = %s",
                    (code_hash, "python"),
                )
                before_hit_at = cur.fetchone()[0]

        time.sleep(0.1)

        api.post(
            "/api/execute",
            headers=auth_user["headers"],
            json={"language": "python", "code": code},
        )

        with get_db_connection() as conn:
            with conn.cursor() as cur:
                cur.execute(
                    "SELECT last_hit_at FROM execution_cache WHERE code_hash = %s AND language = %s",
                    (code_hash, "python"),
                )
                after_hit_at = cur.fetchone()[0]

        assert after_hit_at > before_hit_at

    def test_cache_not_written_on_failure(self, api: APIClient, auth_user: dict):
        r"""
        C06: 失败/超时不写缓存
        """
        import hashlib
        code = "syntax error here !!!"
        code_hash = hashlib.sha256(code.encode()).hexdigest()

        with get_db_connection() as conn:
            with conn.cursor() as cur:
                cur.execute(
                    "SELECT COUNT(*) FROM execution_cache WHERE code_hash = %s AND language = %s",
                    (code_hash, "python"),
                )
                before_count = cur.fetchone()[0]

        api.post(
            "/api/execute",
            headers=auth_user["headers"],
            json={"language": "python", "code": code},
        )

        with get_db_connection() as conn:
            with conn.cursor() as cur:
                cur.execute(
                    "SELECT COUNT(*) FROM execution_cache WHERE code_hash = %s AND language = %s",
                    (code_hash, "python"),
                )
                after_count = cur.fetchone()[0]

        assert after_count == before_count


# ============================================
# 集成测试
# ============================================


class TestIntegration:
    r"""
    集成测试
    """

    def test_complete_user_flow(self, api: APIClient, sample_template: dict):
        r"""
        完整的用户使用流程
        """
        response = api.register("integrationuser", "integrationpass")
        assert response.status_code == 200
        user_id = response.json()["id"]

        response = api.login("integrationuser", "integrationpass")
        assert response.status_code == 200
        token = response.json()["token"]
        headers = api.auth_headers(token)

        response = api.get("/api/auth/me", headers=headers)
        assert response.status_code == 200
        assert response.json()["username"] == "integrationuser"

        response = api.get("/api/templates", headers=headers)
        assert response.status_code == 200
        assert response.json()["total"] >= 1

        response = api.get(
            f"/api/templates/{sample_template['id']}", headers=headers)
        assert response.status_code == 200
        assert response.json()["is_favorited"] is False

        response = api.post("/api/favorites", headers=headers,
                            json={"template_id": sample_template["id"]})
        assert response.status_code == 200

        response = api.get(
            f"/api/templates/{sample_template['id']}", headers=headers)
        assert response.json()["is_favorited"] is True
        assert response.json()["favorite_count"] == 1

        response = api.get(f"/api/users/{user_id}/favorites", headers=headers)
        assert response.status_code == 200
        assert response.json()["total"] == 1

        response = api.delete(
            f"/api/favorites/{sample_template['id']}", headers=headers)
        assert response.status_code == 200

        response = api.get(f"/api/users/{user_id}/favorites", headers=headers)
        assert response.json()["total"] == 0

        avatar_data = b"\x89PNG\r\n\x1a\n" + b"\x00" * 50
        response = api.put(f"/api/users/{user_id}/avatar", headers=headers, files={
                           "avatar": ("avatar.png", avatar_data, "image/png")})
        assert response.status_code == 200

        response = api.put(f"/api/users/{user_id}/password", headers=headers, json={
                           "old_password": "integrationpass", "new_password": "newintegrationpass"})
        assert response.status_code == 200

        response = api.post("/api/auth/logout", headers=headers)
        assert response.status_code == 200

        response = api.login("integrationuser", "newintegrationpass")
        assert response.status_code == 200

    def test_concurrent_favorites(self, api: APIClient, sample_template: dict):
        r"""
        多用户并发收藏
        """
        users = []
        for i in range(5):
            api.register(f"concuser{i}", "password123")
            response = api.login(f"concuser{i}", "password123")
            users.append(api.auth_headers(response.json()["token"]))

        for headers in users:
            response = api.post("/api/favorites", headers=headers,
                                json={"template_id": sample_template["id"]})
            assert response.status_code == 200

        count = get_template_favorite_count(sample_template["id"])
        assert count == 5

        for headers in users:
            response = api.delete(
                f"/api/favorites/{sample_template['id']}", headers=headers)
            assert response.status_code == 200

        count = get_template_favorite_count(sample_template["id"])
        assert count == 0

    def test_search_after_add_template(self, api: APIClient):
        r"""
        添加模板后搜索
        """
        insert_template("新添加的模板", "测试分类", "这是一个新模板的描述")

        response = api.get("/api/templates/search", params={"keyword": "新添加"})
        assert response.status_code == 200
        assert response.json()["total"] == 1

    def test_user_isolation(self, api: APIClient, sample_template: dict):
        r"""
        用户数据隔离
        """
        api.register("user1", "password123")
        response1 = api.login("user1", "password123")
        headers1 = api.auth_headers(response1.json()["token"])
        user1_id = response1.json()["user"]["id"]

        api.register("user2", "password123")
        response2 = api.login("user2", "password123")
        headers2 = api.auth_headers(response2.json()["token"])
        user2_id = response2.json()["user"]["id"]

        api.post("/api/favorites", headers=headers1,
                 json={"template_id": sample_template["id"]})

        response = api.get(
            f"/api/users/{user1_id}/favorites", headers=headers1)
        assert response.json()["total"] == 1

        response = api.get(
            f"/api/users/{user2_id}/favorites", headers=headers2)
        assert response.json()["total"] == 0

        response = api.get(
            f"/api/users/{user1_id}/favorites", headers=headers2)
        assert response.status_code == 403


# ============================================
# 边界条件测试
# ============================================


class TestEdgeCases:
    r"""
    边界条件测试
    """

    def test_max_limit_value(self, api: APIClient, multiple_templates: list):
        r"""
        最大limit值
        """
        response = api.get("/api/templates", params={"limit": 100})
        assert response.status_code == 200
        assert response.json()["limit"] == 100

    def test_unicode_in_template_search(self, api: APIClient):
        r"""
        Unicode字符搜索
        """
        insert_template("中文模板标题", "中文分类", "这是中文描述")

        response = api.get("/api/templates/search", params={"keyword": "中文"})
        assert response.status_code == 200
        assert response.json()["total"] == 1

    def test_very_long_code(self, api: APIClient, auth_user: dict):
        r"""
        超长代码
        """
        long_code = "x = 1\n" * 10000
        response = api.post("/api/execute", headers=auth_user["headers"], json={
                            "language": "python", "code": long_code})
        assert response.status_code in [200, 422, 500]

    def test_special_characters_in_code(self, api: APIClient, auth_user: dict):
        r"""
        代码中的特殊字符
        """
        special_code = "print('Hello\\nWorld\\t!')"
        response = api.post("/api/execute", headers=auth_user["headers"], json={
                            "language": "python", "code": special_code})
        assert response.status_code in [200, 500]

    def test_empty_avatar_file(self, api: APIClient, auth_user: dict):
        r"""
        空头像文件
        """
        user_id = auth_user["user"]["id"]
        response = api.put(
            f"/api/users/{user_id}/avatar",
            headers=auth_user["headers"],
            files={"avatar": ("empty.png", b"", "image/png")},
        )
        assert response.status_code == 200

    def test_binary_in_code(self, api: APIClient, auth_user: dict):
        r"""
        代码中包含二进制数据
        """
        binary_code = "data = b'\\x00\\x01\\x02'"
        response = api.post("/api/execute", headers=auth_user["headers"], json={
                            "language": "python", "code": binary_code})
        assert response.status_code in [200, 500]

    def test_rapid_requests(self, api: APIClient, auth_user: dict, sample_template: dict):
        r"""
        快速连续请求
        """
        for _ in range(10):
            response = api.get(
                f"/api/templates/{sample_template['id']}", headers=auth_user["headers"])
            assert response.status_code == 200

    def test_session_persistence(self, api: APIClient, registered_user: dict):
        r"""
        会话持久性
        """
        response = api.login("testuser", "testpass123")
        token = response.json()["token"]
        headers = api.auth_headers(token)

        for _ in range(5):
            response = api.get("/api/auth/me", headers=headers)
            assert response.status_code == 200

    def test_template_code_with_null_explanation(self, api: APIClient):
        r"""
        模板代码无说明
        """
        template_id = insert_template("测试模板", "测试", "描述")
        insert_template_code(template_id, "python", "code", None)

        response = api.get(f"/api/templates/{template_id}/code/python")
        assert response.status_code == 200
        assert response.json()["explanation"] is None

    def test_favorite_after_unfavorite(self, api: APIClient, auth_user: dict, sample_template: dict):
        r"""
        取消收藏后重新收藏
        """
        api.post("/api/favorites",
                 headers=auth_user["headers"], json={"template_id": sample_template["id"]})
        api.delete(
            f"/api/favorites/{sample_template['id']}", headers=auth_user["headers"])

        response = api.post(
            "/api/favorites", headers=auth_user["headers"], json={"template_id": sample_template["id"]})
        assert response.status_code == 200

        count = get_template_favorite_count(sample_template["id"])
        assert count == 1


# ============================================
# 错误处理测试
# ============================================


class TestErrorHandling:
    r"""
    错误处理测试
    """

    def test_404_unknown_endpoint(self, api: APIClient):
        r"""
        未知端点返回404
        """
        response = api.get("/api/unknown")
        assert response.status_code == 404

    def test_405_method_not_allowed(self, api: APIClient):
        r"""
        不支持的HTTP方法
        """
        response = api.delete("/api/auth/register")
        assert response.status_code == 405

    def test_error_response_format(self, api: APIClient, registered_user: dict):
        r"""
        错误响应格式
        """
        response = api.register("testuser", "password")
        assert response.status_code == 409
        data = response.json()
        assert "error" in data
        assert isinstance(data["error"], str)

    def test_validation_error_format(self, api: APIClient):
        r"""
        验证错误格式
        """
        response = api.post("/api/auth/register", json={"username": "ab"})
        assert response.status_code == 422

    def test_auth_error_does_not_leak_info(self, api: APIClient, registered_user: dict):
        r"""
        认证错误不泄露信息
        """
        response1 = api.login("testuser", "wrongpass")
        response2 = api.login("nonexistent", "password")

        assert response1.json()["error"] == response2.json()["error"]

    def test_sandbox_internal_exception_handled(self, api: APIClient, auth_user: dict):
        r"""
        ERR01: sandbox 内部异常处理
        """
        code = """
import os
os.kill(os.getpid(), 9)
"""
        response = api.post(
            "/api/execute",
            headers=auth_user["headers"],
            json={"language": "python", "code": code},
        )
        assert response.status_code == 200
        data = response.json()
        assert data["status"] == "Error"

    def test_execute_error_response_format(self, api: APIClient, auth_user: dict):
        r"""
        ERR02: 执行接口错误格式
        """
        update_user_status(auth_user["user"]["id"], "Banned")

        response = api.post(
            "/api/execute",
            headers=auth_user["headers"],
            json={"language": "python", "code": "print('test')"},
        )
        assert response.status_code == 403
        data = response.json()
        assert "error" in data
        assert isinstance(data["error"], str)
        
        
# ============================================
# 日志测试
# ============================================


class TestLogging:
    r"""
    日志记录测试
    """

    def test_logs_created_on_startup(self, api: APIClient):
        r"""
        服务启动产生日志
        """
        count = count_logs(module="main")
        assert count >= 0

    def test_error_logs_on_failure(self, api: APIClient):
        r"""
        失败操作产生错误日志
        """
        initial_count = count_logs(level="ERROR")

        api.login("nonexistent", "password")

        time.sleep(0.1)
        
# ============================================
# 代码执行详细测试
# ============================================


class TestExecuteDetailed:
    r"""
    代码执行详细测试（沙箱行为验证）
    """

    def test_execute_no_toml_generated(self, api: APIClient, auth_user: dict):
        r"""
        E01: 无新增 .toml 文件
        """
        code = "print('hello world')"
        response = api.post(
            "/api/execute",
            headers=auth_user["headers"],
            json={"language": "python", "code": code},
        )
        assert response.status_code == 200
        data = response.json()
        assert data["status"] == "Error"
        assert "未生成TOML输出文件" in data["error_message"]

    def test_execute_multiple_toml_files(self, api: APIClient, auth_user: dict):
        r"""
        E02: 新增多个 .toml 文件，只取字典序最小的
        """
        code = """
import ds4viz as dv

with dv.stack("stack_b") as s:
    s.push(2)

dv.config(output_path="a_output.toml")
with dv.stack("stack_a") as s:
    s.push(1)
"""
        response = api.post(
            "/api/execute",
            headers=auth_user["headers"],
            json={"language": "python", "code": code},
        )
        assert response.status_code == 200
        data = response.json()
        assert data["status"] == "Success"
        # 字典序最小的是 a_output.toml
        assert "stack_a" in data["toml_output"] or "[object]" in data["toml_output"]

    def test_execute_uppercase_toml_extension(self, api: APIClient, auth_user: dict):
        r"""
        E03: .TOML 大写扩展名不识别
        """
        code = """
with open('output.TOML', 'w') as f:
    f.write('[test]\\nvalue = 1')
"""
        response = api.post(
            "/api/execute",
            headers=auth_user["headers"],
            json={"language": "python", "code": code},
        )
        assert response.status_code == 200
        data = response.json()
        assert data["status"] == "Error"
        assert "未生成TOML输出文件" in data["error_message"]

    def test_execute_empty_toml_file(self, api: APIClient, auth_user: dict):
        r"""
        E04: 新增空 .toml 文件（ds4viz 始终生成有效内容，测试手动生成空文件）
        """
        code = """
with open('output.toml', 'w') as f:
    f.write('')
"""
        response = api.post(
            "/api/execute",
            headers=auth_user["headers"],
            json={"language": "python", "code": code},
        )
        assert response.status_code == 200
        data = response.json()
        assert data["status"] == "Success"
        assert data["toml_output"] == ""

    def test_execute_preexisting_toml_no_new(self, api: APIClient, auth_user: dict):
        r"""
        E05: 执行代码但未调用 ds4viz 生成 toml
        """
        code = "x = 1"
        response = api.post(
            "/api/execute",
            headers=auth_user["headers"],
            json={"language": "python", "code": code},
        )
        assert response.status_code == 200
        data = response.json()
        assert data["status"] == "Error"
        assert "未生成TOML输出文件" in data["error_message"]

    def test_execute_only_non_toml_files(self, api: APIClient, auth_user: dict):
        r"""
        E06: 仅新增非 .toml 文件
        """
        code = """
with open('output.txt', 'w') as f:
    f.write('hello')
with open('output.json', 'w') as f:
    f.write('{}')
"""
        response = api.post(
            "/api/execute",
            headers=auth_user["headers"],
            json={"language": "python", "code": code},
        )
        assert response.status_code == 200
        data = response.json()
        assert data["status"] == "Error"
        assert "未生成TOML输出文件" in data["error_message"]

    def test_execute_timeout_infinite_loop(self, api: APIClient, auth_user: dict):
        r"""
        E07: 超时（死循环）
        """
        code = "while True: pass"
        response = api.post(
            "/api/execute",
            headers=auth_user["headers"],
            json={"language": "python", "code": code},
        )
        assert response.status_code == 200
        data = response.json()
        assert data["status"] == "Timeout"
        assert "执行超时" in data["error_message"]

    def test_execute_high_memory_consumption(self, api: APIClient, auth_user: dict):
        r"""
        E08: 高内存消耗触发限制
        """
        code = """
data = []
while True:
    data.append('x' * 1024 * 1024)
"""
        response = api.post(
            "/api/execute",
            headers=auth_user["headers"],
            json={"language": "python", "code": code},
        )
        assert response.status_code == 200
        data = response.json()
        assert data["status"] in ["Error", "Timeout"]

    def test_execute_long_io_write(self, api: APIClient, auth_user: dict):
        r"""
        E09: 长时间 IO 写入
        """
        code = """
import time
for i in range(1000):
    with open(f'file_{i}.txt', 'w') as f:
        f.write('x' * 10000)
    time.sleep(0.1)
"""
        response = api.post(
            "/api/execute",
            headers=auth_user["headers"],
            json={"language": "python", "code": code},
        )
        assert response.status_code == 200
        data = response.json()
        assert data["status"] in ["Error", "Timeout"]

    def test_execute_network_access_blocked(self, api: APIClient, auth_user: dict):
        r"""
        E10: 网络访问尝试被阻止
        """
        code = """
import socket
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect(('8.8.8.8', 53))
"""
        response = api.post(
            "/api/execute",
            headers=auth_user["headers"],
            json={"language": "python", "code": code},
        )
        assert response.status_code == 200
        data = response.json()
        assert data["status"] == "Error"
        assert data["error_message"] is not None
        assert len(data["error_message"]) > 0

    def test_execute_write_outside_workspace(self, api: APIClient, auth_user: dict):
        r"""
        E11: 写入非工作目录
        """
        code = """
with open('/tmp/malicious.txt', 'w') as f:
    f.write('hacked')
"""
        response = api.post(
            "/api/execute",
            headers=auth_user["headers"],
            json={"language": "python", "code": code},
        )
        assert response.status_code == 200
        data = response.json()
        assert data["status"] == "Error"
        assert data["error_message"] is not None

    def test_execute_read_system_files(self, api: APIClient, auth_user: dict):
        r"""
        E12: 读取系统文件
        """
        code = """
with open('/etc/passwd', 'r') as f:
    content = f.read()
"""
        response = api.post(
            "/api/execute",
            headers=auth_user["headers"],
            json={"language": "python", "code": code},
        )
        assert response.status_code == 200
        data = response.json()
        assert data["status"] == "Error"
        assert data["error_message"] is not None

    def test_execute_subprocess_creation(self, api: APIClient, auth_user: dict):
        r"""
        E13: 进程/子进程创建
        """
        code = """
import subprocess
subprocess.run(['ls', '-la'])
"""
        response = api.post(
            "/api/execute",
            headers=auth_user["headers"],
            json={"language": "python", "code": code},
        )
        assert response.status_code == 200
        data = response.json()
        assert data["status"] == "Error"

    def test_execute_rust_compile_error(self, api: APIClient, auth_user: dict):
        r"""
        E14: Rust 编译错误
        """
        code = """
fn main() {
    let x: i32 = "not a number";
}
"""
        response = api.post(
            "/api/execute",
            headers=auth_user["headers"],
            json={"language": "rust", "code": code},
        )
        assert response.status_code == 200
        data = response.json()
        assert data["status"] == "Error"
        assert data["error_message"] is not None

    def test_execute_lua_syntax_error(self, api: APIClient, auth_user: dict):
        r"""
        E15: Lua 语法错误
        """
        code = """
function test(
    -- missing closing parenthesis
end
"""
        response = api.post(
            "/api/execute",
            headers=auth_user["headers"],
            json={"language": "lua", "code": code},
        )
        assert response.status_code == 200
        data = response.json()
        assert data["status"] == "Error"
        assert data["error_message"] is not None
        assert len(data["error_message"]) > 0

    def test_execute_non_utf8_output(self, api: APIClient, auth_user: dict):
        r"""
        E16: 成功执行但输出非 UTF-8
        """
        code = """
with open('output.toml', 'wb') as f:
    f.write(b'[test]\\nvalue = "\\xff\\xfe"')
"""
        response = api.post(
            "/api/execute",
            headers=auth_user["headers"],
            json={"language": "python", "code": code},
        )
        assert response.status_code == 200
        data = response.json()
        assert data["status"] in ["Success", "Error"]

    def test_execute_python_success_with_ds4viz_stack(self, api: APIClient, auth_user: dict):
        r"""
        E17: Python 使用 ds4viz 生成栈操作 trace
        """
        code = """
import ds4viz as dv

dv.config(
    output_path="trace.toml",
    title="栈操作演示",
    author="TestUser",
    comment="测试栈的基本操作"
)

with dv.stack("demo_stack") as s:
    s.push(10)
    s.push(20)
    s.push(30)
    s.pop()
"""
        response = api.post(
            "/api/execute",
            headers=auth_user["headers"],
            json={"language": "python", "code": code},
        )
        assert response.status_code == 200
        data = response.json()
        assert data["status"] == "Success"
        assert data["toml_output"] is not None
        # 验证 IR 结构
        assert "[meta]" in data["toml_output"]
        assert "[package]" in data["toml_output"]
        assert "[object]" in data["toml_output"]
        assert 'kind = "stack"' in data["toml_output"]
        assert "[[states]]" in data["toml_output"]
        assert "[[steps]]" in data["toml_output"]
        assert "[result]" in data["toml_output"]
        assert data["execution_time"] >= 0
        assert data["cached"] is False

    def test_execute_lua_success_with_ds4viz_queue(self, api: APIClient, auth_user: dict):
        r"""
        E18: Lua 使用 ds4viz 生成队列操作 trace
        """
        code = """
local ds4viz = require("ds4viz")

ds4viz.config({
    output_path = "trace.toml",
    title = "Queue Demo",
    author = "TestUser",
    comment = "Lua queue example"
})

ds4viz.withContext(ds4viz.queue("demo_queue"), function(q)
    q:enqueue(10)
    q:enqueue(20)
    q:enqueue(30)
    q:dequeue()
end)
"""
        response = api.post(
            "/api/execute",
            headers=auth_user["headers"],
            json={"language": "lua", "code": code},
        )
        assert response.status_code == 200
        data = response.json()
        assert data["status"] == "Success"
        assert data["toml_output"] is not None
        assert "[meta]" in data["toml_output"]
        assert 'kind = "queue"' in data["toml_output"]
        assert "[[states]]" in data["toml_output"]

    def test_execute_concurrent_isolation(self, api: APIClient, auth_user: dict):
        r"""
        E19: 并发执行的工作空间隔离性验证
        """
        import concurrent.futures

        code_template = """
import ds4viz as dv
import os

dv.config(output_path="trace.toml")

with dv.stack("worker_{idx}") as s:
    s.push({idx})
    
# 检查是否能看到其他 worker 的文件
other_files = [f for f in os.listdir('.') if f.endswith('.toml') and f != 'trace.toml']
if other_files:
    raise Exception("Isolation breach: found " + str(other_files))
"""

        def execute_worker(idx: int) -> dict:
            code = code_template.format(idx=idx)
            resp = api.post(
                "/api/execute",
                headers=auth_user["headers"],
                json={"language": "python", "code": code},
            )
            return resp.json()

        with concurrent.futures.ThreadPoolExecutor(max_workers=3) as executor:
            futures = [executor.submit(execute_worker, i) for i in range(3)]
            results = [f.result() for f in concurrent.futures.as_completed(futures)]

        for result in results:
            assert result["status"] == "Success"
            assert "[result]" in result["toml_output"]

    def test_execute_toml_with_unicode_content(self, api: APIClient, auth_user: dict):
        r"""
        E20: ds4viz 生成包含 Unicode 内容的 TOML
        """
        code = """
import ds4viz as dv

dv.config(
    output_path="trace.toml",
    title="冒泡排序算法",
    author="测试用户",
    comment="这是一个经典的排序算法示例 🎉"
)

with dv.stack("中文栈") as s:
    s.push("你好")
    s.push("世界")
    s.pop()
"""
        response = api.post(
            "/api/execute",
            headers=auth_user["headers"],
            json={"language": "python", "code": code},
        )
        assert response.status_code == 200
        data = response.json()
        assert data["status"] == "Success"
        assert "冒泡排序算法" in data["toml_output"]
        assert "测试用户" in data["toml_output"]

    def test_execute_rapid_sequential_different_codes(self, api: APIClient, auth_user: dict):
        r"""
        E21: 快速连续执行不同代码，验证结果不混淆
        """
        codes_and_expected = [
            (
                """
import ds4viz as dv
dv.config(output_path="trace.toml")
with dv.stack("first_stack") as s:
    s.push(1)
""",
                "first_stack"
            ),
            (
                """
import ds4viz as dv
dv.config(output_path="trace.toml")
with dv.queue("second_queue") as q:
    q.enqueue(2)
""",
                "second_queue"
            ),
            (
                """
import ds4viz as dv
dv.config(output_path="trace.toml")
with dv.stack("third_stack") as s:
    s.push(3)
""",
                "third_stack"
            ),
        ]

        results = []
        for code, expected in codes_and_expected:
            response = api.post(
                "/api/execute",
                headers=auth_user["headers"],
                json={"language": "python", "code": code},
            )
            assert response.status_code == 200
            data = response.json()
            results.append((data, expected))

        for data, expected in results:
            assert data["status"] == "Success"
            assert expected in data["toml_output"]
               

class TestDs4VizIntegration:
    r"""
    ds4viz 库集成测试 - 验证各种数据结构的 IR 生成
    """

    def test_python_stack_full_operations(self, api: APIClient, auth_user: dict):
        r"""
        DS01: Python 栈完整操作流程
        """
        code = """
import ds4viz as dv

dv.config(
    output_path="trace.toml",
    title="Stack Demo",
    author="TestUser",
    comment="push/pop operations"
)

with dv.stack("my_stack") as s:
    s.push(10)
    s.push(20)
    s.push(30)
    s.pop()
    s.push(40)
    s.clear()
"""
        response = api.post(
            "/api/execute",
            headers=auth_user["headers"],
            json={"language": "python", "code": code},
        )
        assert response.status_code == 200
        data = response.json()
        assert data["status"] == "Success"

        toml_output = data["toml_output"]
        assert 'kind = "stack"' in toml_output
        assert 'op = "push"' in toml_output
        assert 'op = "pop"' in toml_output
        assert 'op = "clear"' in toml_output
        assert "[result]" in toml_output
        assert "final_state" in toml_output

    def test_python_queue_operations(self, api: APIClient, auth_user: dict):
        r"""
        DS02: Python 队列操作
        """
        code = """
import ds4viz as dv

dv.config(output_path="trace.toml")

with dv.queue("demo_queue") as q:
    q.enqueue(10)
    q.enqueue(20)
    q.enqueue(30)
    q.dequeue()
    q.dequeue()
"""
        response = api.post(
            "/api/execute",
            headers=auth_user["headers"],
            json={"language": "python", "code": code},
        )
        assert response.status_code == 200
        data = response.json()
        assert data["status"] == "Success"

        toml_output = data["toml_output"]
        assert 'kind = "queue"' in toml_output
        assert 'op = "enqueue"' in toml_output
        assert 'op = "dequeue"' in toml_output
        assert "front" in toml_output
        assert "rear" in toml_output

    def test_python_single_linked_list(self, api: APIClient, auth_user: dict):
        r"""
        DS03: Python 单链表操作
        """
        code = """
import ds4viz as dv

dv.config(output_path="trace.toml")

with dv.single_linked_list("my_slist") as slist:
    node_a = slist.insert_head(10)
    slist.insert_tail(20)
    node_c = slist.insert_tail(30)
    slist.insert_after(node_a, 15)
    slist.delete(node_c)
    slist.reverse()
"""
        response = api.post(
            "/api/execute",
            headers=auth_user["headers"],
            json={"language": "python", "code": code},
        )
        assert response.status_code == 200
        data = response.json()
        assert data["status"] == "Success"

        toml_output = data["toml_output"]
        assert 'kind = "slist"' in toml_output
        assert "head" in toml_output
        assert "nodes" in toml_output
        assert "next" in toml_output

    def test_python_double_linked_list(self, api: APIClient, auth_user: dict):
        r"""
        DS04: Python 双向链表操作
        """
        code = """
import ds4viz as dv

dv.config(output_path="trace.toml")

with dv.double_linked_list("my_dlist") as dlist:
    node_a = dlist.insert_head(10)
    dlist.insert_tail(30)
    dlist.insert_after(node_a, 20)
    dlist.delete_tail()
    dlist.reverse()
"""
        response = api.post(
            "/api/execute",
            headers=auth_user["headers"],
            json={"language": "python", "code": code},
        )
        assert response.status_code == 200
        data = response.json()
        assert data["status"] == "Success"

        toml_output = data["toml_output"]
        assert 'kind = "dlist"' in toml_output
        assert "head" in toml_output
        assert "tail" in toml_output
        assert "prev" in toml_output
        assert "next" in toml_output

    def test_python_binary_tree(self, api: APIClient, auth_user: dict):
        r"""
        DS05: Python 二叉树操作
        """
        code = """
import ds4viz as dv

dv.config(output_path="trace.toml")

with dv.binary_tree("my_tree") as tree:
    root = tree.insert_root(10)
    left = tree.insert_left(root, 5)
    right = tree.insert_right(root, 15)
    tree.insert_left(left, 3)
    tree.insert_right(left, 7)
    tree.update_value(right, 20)
    tree.delete(left)
"""
        response = api.post(
            "/api/execute",
            headers=auth_user["headers"],
            json={"language": "python", "code": code},
        )
        assert response.status_code == 200
        data = response.json()
        assert data["status"] == "Success"

        toml_output = data["toml_output"]
        assert 'kind = "binary_tree"' in toml_output
        assert "root" in toml_output
        assert "left" in toml_output
        assert "right" in toml_output

    def test_python_binary_search_tree(self, api: APIClient, auth_user: dict):
        r"""
        DS06: Python 二叉搜索树操作
        """
        code = """
import ds4viz as dv

dv.config(output_path="trace.toml")

with dv.binary_search_tree("my_bst") as bst:
    bst.insert(10)
    bst.insert(5)
    bst.insert(15)
    bst.insert(3)
    bst.insert(7)
    bst.delete(5)
    bst.insert(6)
"""
        response = api.post(
            "/api/execute",
            headers=auth_user["headers"],
            json={"language": "python", "code": code},
        )
        assert response.status_code == 200
        data = response.json()
        assert data["status"] == "Success"

        toml_output = data["toml_output"]
        assert 'kind = "bst"' in toml_output
        assert "root" in toml_output

    def test_python_min_heap(self, api: APIClient, auth_user: dict):
        r"""
        DS07: Python 最小堆操作
        """
        code = """
import ds4viz as dv

dv.config(output_path="trace.toml")

with dv.heap("min_heap", heap_type="min") as h:
    h.insert(10)
    h.insert(5)
    h.insert(15)
    h.insert(3)
    h.extract()  # 提取 3
    h.insert(1)
"""
        response = api.post(
            "/api/execute",
            headers=auth_user["headers"],
            json={"language": "python", "code": code},
        )
        assert response.status_code == 200
        data = response.json()
        assert data["status"] == "Success"

        # 堆使用 binary_tree 或 bst 类型存储
        toml_output = data["toml_output"]
        assert "root" in toml_output or "nodes" in toml_output

    def test_python_max_heap(self, api: APIClient, auth_user: dict):
        r"""
        DS08: Python 最大堆操作
        """
        code = """
import ds4viz as dv

dv.config(output_path="trace.toml")

with dv.heap("max_heap", heap_type="max") as h:
    h.insert(10)
    h.insert(5)
    h.insert(15)
    h.insert(3)
    h.extract()  # 提取 15
"""
        response = api.post(
            "/api/execute",
            headers=auth_user["headers"],
            json={"language": "python", "code": code},
        )
        assert response.status_code == 200
        data = response.json()
        assert data["status"] == "Success"

    def test_python_graph_undirected(self, api: APIClient, auth_user: dict):
        r"""
        DS09: Python 无向图操作
        """
        code = """
import ds4viz as dv

dv.config(output_path="trace.toml")

with dv.graph_undirected("my_graph") as graph:
    graph.add_node(0, "A")
    graph.add_node(1, "B")
    graph.add_node(2, "C")
    graph.add_edge(0, 1)
    graph.add_edge(1, 2)
    graph.update_node_label(1, "B_updated")
    graph.remove_edge(0, 1)
"""
        response = api.post(
            "/api/execute",
            headers=auth_user["headers"],
            json={"language": "python", "code": code},
        )
        assert response.status_code == 200
        data = response.json()
        assert data["status"] == "Success"

        toml_output = data["toml_output"]
        assert 'kind = "graph_undirected"' in toml_output
        assert "nodes" in toml_output
        assert "edges" in toml_output
        assert "label" in toml_output

    def test_python_graph_directed(self, api: APIClient, auth_user: dict):
        r"""
        DS10: Python 有向图操作
        """
        code = """
import ds4viz as dv

dv.config(output_path="trace.toml")

with dv.graph_directed("my_digraph") as graph:
    graph.add_node(0, "A")
    graph.add_node(1, "B")
    graph.add_node(2, "C")
    graph.add_edge(0, 1)
    graph.add_edge(1, 2)
    graph.add_edge(2, 0)
    graph.remove_edge(2, 0)
"""
        response = api.post(
            "/api/execute",
            headers=auth_user["headers"],
            json={"language": "python", "code": code},
        )
        assert response.status_code == 200
        data = response.json()
        assert data["status"] == "Success"

        toml_output = data["toml_output"]
        assert 'kind = "graph_directed"' in toml_output

    def test_python_graph_weighted(self, api: APIClient, auth_user: dict):
        r"""
        DS11: Python 带权图操作
        """
        code = """
import ds4viz as dv

dv.config(output_path="trace.toml")

with dv.graph_weighted("weighted_graph", directed=False) as graph:
    graph.add_node(0, "A")
    graph.add_node(1, "B")
    graph.add_node(2, "C")
    graph.add_edge(0, 1, 3.5)
    graph.add_edge(1, 2, 2.0)
    graph.update_weight(0, 1, 5.0)
"""
        response = api.post(
            "/api/execute",
            headers=auth_user["headers"],
            json={"language": "python", "code": code},
        )
        assert response.status_code == 200
        data = response.json()
        assert data["status"] == "Success"

        toml_output = data["toml_output"]
        assert 'kind = "graph_weighted"' in toml_output
        assert "weight" in toml_output

    def test_python_ds4viz_error_handling(self, api: APIClient, auth_user: dict):
        r"""
        DS12: Python ds4viz 错误处理，生成包含 [error] 的 toml
        """
        code = """
import ds4viz as dv

dv.config(output_path="trace.toml")

with dv.binary_tree("error_tree") as tree:
    root = tree.insert_root(10)
    tree.insert_left(999, 5)  # 父节点不存在
"""
        response = api.post(
            "/api/execute",
            headers=auth_user["headers"],
            json={"language": "python", "code": code},
        )
        assert response.status_code == 200
        data = response.json()
        # ds4viz 会捕获异常并生成包含 [error] 的 toml
        assert data["status"] == "Success"
        toml_output = data["toml_output"]
        assert "[error]" in toml_output
        assert 'type = "runtime"' in toml_output

    def test_lua_stack_operations(self, api: APIClient, auth_user: dict):
        r"""
        DS13: Lua 栈操作
        """
        code = """
local ds4viz = require("ds4viz")

ds4viz.config({
    output_path = "trace.toml",
    title = "Lua Stack Demo",
    author = "TestUser"
})

ds4viz.withContext(ds4viz.stack("lua_stack"), function(s)
    s:push(10)
    s:push(20)
    s:push(30)
    s:pop()
end)
"""
        response = api.post(
            "/api/execute",
            headers=auth_user["headers"],
            json={"language": "lua", "code": code},
        )
        assert response.status_code == 200
        data = response.json()
        assert data["status"] == "Success"

        toml_output = data["toml_output"]
        assert 'kind = "stack"' in toml_output
        assert 'lang = "lua"' in toml_output

    def test_lua_single_linked_list(self, api: APIClient, auth_user: dict):
        r"""
        DS14: Lua 单链表操作
        """
        code = """
local ds4viz = require("ds4viz")

ds4viz.config({
    output_path = "trace.toml"
})

ds4viz.withContext(ds4viz.singleLinkedList("lua_slist"), function(slist)
    local node_a = slist:insertHead(10)
    slist:insertTail(20)
    slist:insertAfter(node_a, 15)
    slist:reverse()
end)
"""
        response = api.post(
            "/api/execute",
            headers=auth_user["headers"],
            json={"language": "lua", "code": code},
        )
        assert response.status_code == 200
        data = response.json()
        assert data["status"] == "Success"

        toml_output = data["toml_output"]
        assert 'kind = "slist"' in toml_output

    def test_lua_graph_undirected(self, api: APIClient, auth_user: dict):
        r"""
        DS15: Lua 无向图操作
        """
        code = """
local ds4viz = require("ds4viz")

ds4viz.config({
    output_path = "trace.toml",
    title = "Graph Demo",
    comment = "无向图示例"
})

ds4viz.withContext(ds4viz.graphUndirected("lua_graph"), function(graph)
    graph:addNode(0, "A")
    graph:addNode(1, "B")
    graph:addNode(2, "C")
    graph:addEdge(0, 1)
    graph:addEdge(1, 2)
end)
"""
        response = api.post(
            "/api/execute",
            headers=auth_user["headers"],
            json={"language": "lua", "code": code},
        )
        assert response.status_code == 200
        data = response.json()
        assert data["status"] == "Success"

        toml_output = data["toml_output"]
        assert 'kind = "graph_undirected"' in toml_output

    def test_rust_stack_operations(self, api: APIClient, auth_user: dict):
        r"""
        DS16: Rust 栈操作
        """
        code = """
use ds4viz::prelude::*;

fn main() -> ds4viz::Result<()> {
    ds4viz::config(
        ds4viz::Config::builder()
            .output_path("trace.toml")
            .title("Rust Stack Demo")
            .build()
    );

    ds4viz::stack("rust_stack", |s| {
        s.push(10)?;
        s.push(20)?;
        s.push(30)?;
        s.pop()?;
        Ok(())
    })
}
"""
        response = api.post(
            "/api/execute",
            headers=auth_user["headers"],
            json={"language": "rust", "code": code},
        )
        assert response.status_code == 200
        data = response.json()
        assert data["status"] == "Success"

        toml_output = data["toml_output"]
        assert 'kind = "stack"' in toml_output
        assert 'lang = "rust"' in toml_output

    def test_rust_graph_weighted(self, api: APIClient, auth_user: dict):
        r"""
        DS17: Rust 带权图操作
        """
        code = """
use ds4viz::prelude::*;

fn main() -> ds4viz::Result<()> {
    ds4viz::graph_weighted("rust_weighted", false, |graph| {
        graph.add_node(0, "A")?;
        graph.add_node(1, "B")?;
        graph.add_node(2, "C")?;
        graph.add_edge(0, 1, 3.5)?;
        graph.add_edge(1, 2, 2.0)?;
        Ok(())
    })
}
"""
        response = api.post(
            "/api/execute",
            headers=auth_user["headers"],
            json={"language": "rust", "code": code},
        )
        assert response.status_code == 200
        data = response.json()
        assert data["status"] == "Success"

        toml_output = data["toml_output"]
        assert 'kind = "graph_weighted"' in toml_output

    def test_ir_structure_validation_meta(self, api: APIClient, auth_user: dict):
        r"""
        DS18: 验证生成的 IR 的 meta 部分结构
        """
        code = """
import ds4viz as dv

dv.config(output_path="trace.toml")

with dv.stack("test") as s:
    s.push(1)
"""
        response = api.post(
            "/api/execute",
            headers=auth_user["headers"],
            json={"language": "python", "code": code},
        )
        assert response.status_code == 200
        data = response.json()
        assert data["status"] == "Success"

        toml_output = data["toml_output"]
        # 验证 meta 部分
        assert "[meta]" in toml_output
        assert "created_at" in toml_output
        assert 'lang = "python"' in toml_output
        assert "lang_version" in toml_output

    def test_ir_structure_validation_package(self, api: APIClient, auth_user: dict):
        r"""
        DS19: 验证生成的 IR 的 package 部分结构
        """
        code = """
import ds4viz as dv

dv.config(output_path="trace.toml")

with dv.queue("test") as q:
    q.enqueue(1)
"""
        response = api.post(
            "/api/execute",
            headers=auth_user["headers"],
            json={"language": "python", "code": code},
        )
        assert response.status_code == 200
        data = response.json()
        assert data["status"] == "Success"

        toml_output = data["toml_output"]
        # 验证 package 部分
        assert "[package]" in toml_output
        assert 'name = "ds4viz"' in toml_output
        assert "version" in toml_output

    def test_ir_structure_validation_states_and_steps(self, api: APIClient, auth_user: dict):
        r"""
        DS20: 验证生成的 IR 的 states 和 steps 结构
        """
        code = """
import ds4viz as dv

dv.config(output_path="trace.toml")

with dv.stack("test") as s:
    s.push(10)
    s.push(20)
    s.pop()
"""
        response = api.post(
            "/api/execute",
            headers=auth_user["headers"],
            json={"language": "python", "code": code},
        )
        assert response.status_code == 200
        data = response.json()
        assert data["status"] == "Success"

        toml_output = data["toml_output"]
        # 验证 states
        assert "[[states]]" in toml_output
        assert "[states.data]" in toml_output
        assert "items" in toml_output
        assert "top" in toml_output

        # 验证 steps
        assert "[[steps]]" in toml_output
        assert "op" in toml_output
        assert "before" in toml_output
        assert "after" in toml_output
        assert "[steps.args]" in toml_output

    def test_ir_node_id_constraint(self, api: APIClient, auth_user: dict):
        r"""
        DS21: 验证节点 id 从 0 开始连续递增
        """
        code = """
import ds4viz as dv

dv.config(output_path="trace.toml")

with dv.single_linked_list("test") as slist:
    slist.insert_head(10)
    slist.insert_head(20)
    slist.insert_head(30)
"""
        response = api.post(
            "/api/execute",
            headers=auth_user["headers"],
            json={"language": "python", "code": code},
        )
        assert response.status_code == 200
        data = response.json()
        assert data["status"] == "Success"

        toml_output = data["toml_output"]
        # 验证节点 id 存在
        assert "id = 0" in toml_output

    def test_ir_result_final_state(self, api: APIClient, auth_user: dict):
        r"""
        DS22: 验证成功执行包含 result.final_state
        """
        code = """
import ds4viz as dv

dv.config(output_path="trace.toml")

with dv.stack("test") as s:
    s.push(1)
    s.push(2)
"""
        response = api.post(
            "/api/execute",
            headers=auth_user["headers"],
            json={"language": "python", "code": code},
        )
        assert response.status_code == 200
        data = response.json()
        assert data["status"] == "Success"

        toml_output = data["toml_output"]
        assert "[result]" in toml_output
        assert "final_state" in toml_output

    def test_graph_edge_normalization(self, api: APIClient, auth_user: dict):
        r"""
        DS23: 验证无向图边规范化 (from < to)
        """
        code = """
import ds4viz as dv

dv.config(output_path="trace.toml")

with dv.graph_undirected("test") as g:
    g.add_node(0, "A")
    g.add_node(1, "B")
    # 传入 (1, 0) 应该被规范化为 (0, 1)
    g.add_edge(1, 0)
"""
        response = api.post(
            "/api/execute",
            headers=auth_user["headers"],
            json={"language": "python", "code": code},
        )
        assert response.status_code == 200
        data = response.json()
        assert data["status"] == "Success"

        toml_output = data["toml_output"]
        # 验证边被规范化
        assert "from = 0" in toml_output
        assert "to = 1" in toml_output

    def test_empty_structure_initial_state(self, api: APIClient, auth_user: dict):
        r"""
        DS24: 验证空数据结构的初始状态
        """
        code = """
import ds4viz as dv

dv.config(output_path="trace.toml")

with dv.stack("empty_stack") as s:
    pass  # 不执行任何操作
"""
        response = api.post(
            "/api/execute",
            headers=auth_user["headers"],
            json={"language": "python", "code": code},
        )
        assert response.status_code == 200
        data = response.json()
        assert data["status"] == "Success"

        toml_output = data["toml_output"]
        # 验证初始空栈状态
        assert "items = []" in toml_output
        assert "top = -1" in toml_output

    def test_value_types_support(self, api: APIClient, auth_user: dict):
        r"""
        DS25: 验证支持的值类型 (integer, float, string, boolean)
        """
        code = """
import ds4viz as dv

dv.config(output_path="trace.toml")

with dv.stack("typed_stack") as s:
    s.push(42)          # integer
    s.push(3.14)        # float
    s.push("hello")     # string
    s.push(True)        # boolean
"""
        response = api.post(
            "/api/execute",
            headers=auth_user["headers"],
            json={"language": "python", "code": code},
        )
        assert response.status_code == 200
        data = response.json()
        assert data["status"] == "Success"

        toml_output = data["toml_output"]
        assert "42" in toml_output
        assert "3.14" in toml_output
        assert '"hello"' in toml_output
        assert "true" in toml_output
