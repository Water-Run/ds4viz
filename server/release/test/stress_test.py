r"""
压力测试模块，通过高并发HTTP请求验证服务器稳定性

:file: test/stress_test.py
:author: WaterRun
:time: 2026-01-30
"""

import os
import signal
import socket
import subprocess
import time
import uuid
from dataclasses import dataclass
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


from conftest import TEST_HOST, TEST_PORT

BASE_URL = f"http://{TEST_HOST}:{TEST_PORT}"

DB_CONFIG: dict[str, str | int] = {
    "host": "localhost",
    "port": 5432,
    "dbname": "ds4viz_test",
    "user": "ds4viz_test",
    "password": "test_pwd_123",
}

STRESS_USERS: int = 20
STRESS_TEMPLATES: int = 50
STRESS_WORKERS: int = 40
STRESS_OPS_PER_WORKER: int = 100
STRESS_TIMEOUT_SECONDS: float = 10.0

def insert_template(title: str, category: str, description: str) -> int:
    r"""
    直接插入模板记录

    :param title: 模板标题
    :param category: 模板分类
    :param description: 模板描述
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
        conn.commit()
    return template_id


def insert_template_code(
    template_id: int,
    language: str,
    code: str,
    explanation: str | None = None,
) -> int:
    r"""
    直接插入模板代码记录

    :param template_id: 模板ID
    :param language: 编程语言
    :param code: 代码内容
    :param explanation: 代码说明
    :return int: 模板代码ID
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
            code_id: int = cur.fetchone()[0]
        conn.commit()
    return code_id


def seed_templates(count: int) -> list[int]:
    r"""
    批量创建模板与默认代码

    :param count: 模板数量
    :return list[int]: 模板ID列表
    """
    template_ids: list[int] = []
    for i in range(count):
        title: str = f"压力测试模板-{i}"
        category: str = "压力测试分类"
        description: str = f"压力测试模板描述 {i}"
        template_id: int = insert_template(title, category, description)
        insert_template_code(
            template_id,
            "python",
            "def demo():\n    print('ok')",
            "压力测试Python实现",
        )
        template_ids.append(template_id)
    return template_ids

class APIClient:
    r"""
    API客户端封装
    """

    def __init__(self, base_url: str = BASE_URL):
        r"""
        初始化API客户端

        :param base_url: 服务基础URL
        """
        self.base_url = base_url
        self.session = requests.Session()

    def get(self, path: str, headers: dict | None = None, **kwargs) -> requests.Response:
        r"""
        发送GET请求

        :param path: 请求路径
        :param headers: 请求头
        :return requests.Response: 响应
        """
        return self.session.get(f"{self.base_url}{path}", headers=headers, **kwargs)

    def post(self, path: str, headers: dict | None = None, **kwargs) -> requests.Response:
        r"""
        发送POST请求

        :param path: 请求路径
        :param headers: 请求头
        :return requests.Response: 响应
        """
        return self.session.post(f"{self.base_url}{path}", headers=headers, **kwargs)

    def delete(self, path: str, headers: dict | None = None, **kwargs) -> requests.Response:
        r"""
        发送DELETE请求

        :param path: 请求路径
        :param headers: 请求头
        :return requests.Response: 响应
        """
        return self.session.delete(f"{self.base_url}{path}", headers=headers, **kwargs)

    def register(self, username: str, password: str) -> requests.Response:
        r"""
        注册用户

        :param username: 用户名
        :param password: 密码
        :return requests.Response: 响应
        """
        return self.post("/api/auth/register", json={"username": username, "password": password})

    def login(self, username: str, password: str) -> requests.Response:
        r"""
        登录用户

        :param username: 用户名
        :param password: 密码
        :return requests.Response: 响应
        """
        return self.post("/api/auth/login", json={"username": username, "password": password})

    @staticmethod
    def auth_headers(token: str) -> dict[str, str]:
        r"""
        构建认证头

        :param token: JWT令牌
        :return dict: 认证头
        """
        return {"Authorization": f"Bearer {token}"}


@dataclass(frozen=True)
class UserContext:
    r"""
    用户上下文
    """

    user_id: int
    token: str
    headers: dict[str, str]


@dataclass(frozen=True)
class RequestResult:
    r"""
    单次请求结果
    """

    ok: bool
    status_code: int
    elapsed_ms: int
    error: str | None
    operation: str


def create_users(api: APIClient, count: int) -> list[UserContext]:
    r"""
    创建并登录多个用户

    :param api: API客户端
    :param count: 用户数量
    :return list[UserContext]: 用户上下文列表
    """
    users: list[UserContext] = []
    for i in range(count):
        username: str = f"stress_user_{i}_{uuid.uuid4().hex[:8]}"
        password: str = "stress_pass_123"

        reg_resp: requests.Response = api.register(username, password)
        if reg_resp.status_code != 200:
            raise RuntimeError(f"注册失败: {reg_resp.status_code} {reg_resp.text}")

        login_resp: requests.Response = api.login(username, password)
        if login_resp.status_code != 200:
            raise RuntimeError(
                f"登录失败: {login_resp.status_code} {login_resp.text}")

        data: dict = login_resp.json()
        token: str = data["token"]
        user_id: int = data["user"]["id"]
        users.append(UserContext(user_id=user_id, token=token,
                     headers=api.auth_headers(token)))

    return users


def run_worker(
    worker_id: int,
    base_url: str,
    ops_count: int,
    users: list[UserContext],
    template_ids: list[int],
) -> list[RequestResult]:
    r"""
    运行单个压力测试工作线程

    :param worker_id: 工作线程ID
    :param base_url: 服务基础URL
    :param ops_count: 操作次数
    :param users: 用户上下文列表
    :param template_ids: 模板ID列表
    :return list[RequestResult]: 请求结果列表
    """
    import random

    session = requests.Session()
    rng = random.Random(worker_id + 20260130)

    operations: list[str] = [
        "health",
        "list_templates",
        "search_templates",
        "template_detail",
        "auth_me",
        "favorite_add",
        "favorite_remove",
        "favorites_list",
    ]

    results: list[RequestResult] = []

    for _ in range(ops_count):
        op: str = rng.choice(operations)
        user: UserContext = rng.choice(users)
        template_id: int = rng.choice(template_ids)

        start = time.perf_counter()
        try:
            if op == "health":
                resp = session.get(f"{base_url}/health",
                                   timeout=STRESS_TIMEOUT_SECONDS)
                ok = resp.status_code == 200

            elif op == "list_templates":
                resp = session.get(
                    f"{base_url}/api/templates",
                    params={"page": 1, "limit": 20},
                    timeout=STRESS_TIMEOUT_SECONDS,
                )
                ok = resp.status_code == 200

            elif op == "search_templates":
                resp = session.get(
                    f"{base_url}/api/templates/search",
                    params={"keyword": "压力测试"},
                    timeout=STRESS_TIMEOUT_SECONDS,
                )
                ok = resp.status_code == 200

            elif op == "template_detail":
                resp = session.get(
                    f"{base_url}/api/templates/{template_id}",
                    timeout=STRESS_TIMEOUT_SECONDS,
                )
                ok = resp.status_code == 200

            elif op == "auth_me":
                resp = session.get(
                    f"{base_url}/api/auth/me",
                    headers=user.headers,
                    timeout=STRESS_TIMEOUT_SECONDS,
                )
                ok = resp.status_code == 200

            elif op == "favorite_add":
                resp = session.post(
                    f"{base_url}/api/favorites",
                    headers=user.headers,
                    json={"template_id": template_id},
                    timeout=STRESS_TIMEOUT_SECONDS,
                )
                ok = resp.status_code in {200, 409}

            elif op == "favorite_remove":
                resp = session.delete(
                    f"{base_url}/api/favorites/{template_id}",
                    headers=user.headers,
                    timeout=STRESS_TIMEOUT_SECONDS,
                )
                ok = resp.status_code in {200, 404}

            elif op == "favorites_list":
                resp = session.get(
                    f"{base_url}/api/users/{user.user_id}/favorites",
                    headers=user.headers,
                    params={"page": 1, "limit": 20},
                    timeout=STRESS_TIMEOUT_SECONDS,
                )
                ok = resp.status_code == 200

            else:
                resp = session.get(f"{base_url}/health",
                                   timeout=STRESS_TIMEOUT_SECONDS)
                ok = resp.status_code == 200

            elapsed_ms: int = int((time.perf_counter() - start) * 1000)
            results.append(
                RequestResult(
                    ok=ok,
                    status_code=resp.status_code,
                    elapsed_ms=elapsed_ms,
                    error=None,
                    operation=op,
                )
            )
        except Exception as err:
            elapsed_ms = int((time.perf_counter() - start) * 1000)
            results.append(
                RequestResult(
                    ok=False,
                    status_code=0,
                    elapsed_ms=elapsed_ms,
                    error=f"{err!r}",
                    operation=op,
                )
            )

    return results


class TestStress:
    r"""
    压力测试
    """

    def test_server_under_load(self, server_process) -> None:
        r"""
        服务器高负载能力测试
        """
        api = APIClient()

        template_ids: list[int] = seed_templates(STRESS_TEMPLATES)
        users: list[UserContext] = create_users(api, STRESS_USERS)

        total_requests: int = STRESS_WORKERS * STRESS_OPS_PER_WORKER

        from concurrent.futures import ThreadPoolExecutor, as_completed

        all_results: list[RequestResult] = []

        with ThreadPoolExecutor(max_workers=STRESS_WORKERS) as executor:
            futures = [
                executor.submit(
                    run_worker,
                    worker_id=i,
                    base_url=BASE_URL,
                    ops_count=STRESS_OPS_PER_WORKER,
                    users=users,
                    template_ids=template_ids,
                )
                for i in range(STRESS_WORKERS)
            ]

            for future in as_completed(futures):
                all_results.extend(future.result())

        assert len(all_results) == total_requests

        success_count: int = sum(1 for r in all_results if r.ok)
        error_count: int = sum(1 for r in all_results if not r.ok)
        status_5xx: int = sum(
            1 for r in all_results if 500 <= r.status_code < 600)

        success_rate: float = success_count / \
            total_requests if total_requests > 0 else 0.0

        avg_latency_ms: float = (
            sum(r.elapsed_ms for r in all_results) /
            total_requests if total_requests > 0 else 0.0
        )

        assert status_5xx == 0
        assert success_rate >= 0.98
        assert error_count <= total_requests * 0.02
        assert avg_latency_ms < 2000
