r"""
pytest 配置和共享 fixtures

:file: test/conftest.py
:author: WaterRun
:time: 2026-02-05
"""

import os
import signal
import socket
import subprocess
import time
from typing import Generator

import psycopg
import pytest


TEST_HOST: str = "127.0.0.1"
TEST_PORT: int = 20000

DB_CONFIG: dict[str, str | int] = {
    "host": "localhost",
    "port": 5432,
    "dbname": "ds4viz_test",
    "user": "ds4viz_test",
    "password": "test_pwd_123",
}

_server_process: subprocess.Popen | None = None


def get_db_connection() -> psycopg.Connection:
    r"""
    获取数据库连接

    :return psycopg.Connection: 数据库连接对象
    """
    return psycopg.connect(**DB_CONFIG)


def reset_database() -> None:
    r"""
    重置数据库到初始状态，清空所有表并重置序列
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


def is_port_open(host: str, port: int, timeout: float = 1.0) -> bool:
    r"""
    检查指定端口是否开放

    :param host: 主机地址
    :param port: 端口号
    :param timeout: 超时时间（秒）
    :return bool: 端口是否开放
    """
    try:
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
            sock.settimeout(timeout)
            return sock.connect_ex((host, port)) == 0
    except socket.error:
        return False


def wait_for_server(host: str, port: int, timeout: float = 30.0) -> bool:
    r"""
    等待服务器启动

    :param host: 主机地址
    :param port: 端口号
    :param timeout: 最大等待时间（秒）
    :return bool: 服务器是否成功启动
    """
    start_time: float = time.time()
    while time.time() - start_time < timeout:
        if is_port_open(host, port):
            time.sleep(0.5)
            return True
        time.sleep(0.2)
    return False


def wait_for_server_shutdown(host: str, port: int, timeout: float = 10.0) -> bool:
    r"""
    等待服务器关闭

    :param host: 主机地址
    :param port: 端口号
    :param timeout: 最大等待时间（秒）
    :return bool: 服务器是否成功关闭
    """
    start_time: float = time.time()
    while time.time() - start_time < timeout:
        if not is_port_open(host, port):
            return True
        time.sleep(0.2)
    return False


def kill_process_on_port(port: int) -> None:
    r"""
    终止占用指定端口的进程（排除当前进程树）

    :param port: 端口号
    """
    current_pid: int = os.getpid()
    current_pgid: int = os.getpgid(current_pid)

    try:
        result: subprocess.CompletedProcess = subprocess.run(
            ["lsof", "-ti", f":{port}"],
            capture_output=True,
            text=True,
            timeout=5,
        )
        if result.stdout.strip():
            for pid_str in result.stdout.strip().split('\n'):
                if not pid_str:
                    continue
                try:
                    pid: int = int(pid_str)
                    if pid == current_pid:
                        continue
                    try:
                        if os.getpgid(pid) == current_pgid:
                            continue
                    except OSError:
                        ...
                    os.kill(pid, signal.SIGKILL)
                except (ProcessLookupError, ValueError, OSError):
                    ...
            time.sleep(1)
    except (subprocess.TimeoutExpired, FileNotFoundError):
        ...


def ensure_clean_state() -> None:
    r"""
    确保测试环境处于干净状态
    """
    if is_port_open(TEST_HOST, TEST_PORT):
        kill_process_on_port(TEST_PORT)
        wait_for_server_shutdown(TEST_HOST, TEST_PORT, timeout=5.0)
    reset_database()


def terminate_server(process: subprocess.Popen) -> None:
    r"""
    安全终止服务器进程

    :param process: 服务器进程对象
    """
    if process.poll() is not None:
        return

    try:
        os.killpg(os.getpgid(process.pid), signal.SIGTERM)
        process.wait(timeout=10)
    except (subprocess.TimeoutExpired, ProcessLookupError, OSError):
        try:
            os.killpg(os.getpgid(process.pid), signal.SIGKILL)
            process.wait(timeout=5)
        except (ProcessLookupError, OSError):
            ...


@pytest.fixture(scope="session")
def server_process() -> Generator[subprocess.Popen, None, None]:
    r"""
    启动测试服务器（会话级别，所有测试共享）

    :return Generator[subprocess.Popen, None, None]: 服务器进程生成器
    """
    global _server_process

    ensure_clean_state()

    project_root: str = os.path.dirname(
        os.path.dirname(os.path.abspath(__file__)))
    src_path: str = os.path.join(project_root, "src")

    env: dict[str, str] = os.environ.copy()
    env["PYTHONPATH"] = src_path
    env["PYTHONUNBUFFERED"] = "1"
    env.pop("DBUS_SESSION_BUS_ADDRESS", None)
    env.pop("XDG_RUNTIME_DIR", None)

    process: subprocess.Popen = subprocess.Popen(
        ["python", "main.py", "--test"],
        cwd=src_path,
        env=env,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        preexec_fn=os.setsid,
    )
    _server_process = process

    try:
        if not wait_for_server(TEST_HOST, TEST_PORT, timeout=30.0):
            process.terminate()
            stdout, stderr = process.communicate(timeout=5)
            pytest.exit(
                f"服务器启动超时\nstdout: {stdout.decode()}\nstderr: {stderr.decode()}"
            )
        yield process
    finally:
        _server_process = None
        terminate_server(process)
        wait_for_server_shutdown(TEST_HOST, TEST_PORT, timeout=5.0)
        reset_database()


def pytest_sessionfinish(session: pytest.Session, exitstatus: int) -> None:
    r"""
    pytest 会话结束钩子，确保清理

    :param session: pytest会话对象
    :param exitstatus: 退出状态码
    """
    global _server_process
    if _server_process is not None:
        terminate_server(_server_process)
        _server_process = None

    if is_port_open(TEST_HOST, TEST_PORT):
        kill_process_on_port(TEST_PORT)

    try:
        reset_database()
    except Exception:
        ...
