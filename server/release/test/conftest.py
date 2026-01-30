r"""
pytest 配置和共享 fixtures

:file: test/conftest.py
"""

import os
import signal
import socket
import subprocess
import time
from typing import Generator

import psycopg
import pytest


TEST_HOST = "127.0.0.1"
TEST_PORT = 20000

DB_CONFIG = {
    "host": "localhost",
    "port": 5432,
    "dbname": "ds4viz_test",
    "user": "ds4viz_test",
    "password": "test_pwd_123",
}

# 全局变量记录服务器进程，便于清理
_server_process: subprocess.Popen | None = None


def get_db_connection() -> psycopg.Connection:
    return psycopg.connect(**DB_CONFIG)


def reset_database() -> None:
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


def is_port_open(host: str, port: int, timeout: float = 1.0) -> bool:
    try:
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
            sock.settimeout(timeout)
            result = sock.connect_ex((host, port))
            return result == 0
    except socket.error:
        return False


def wait_for_server(host: str, port: int, timeout: float = 30.0) -> bool:
    start_time = time.time()
    while time.time() - start_time < timeout:
        if is_port_open(host, port):
            time.sleep(0.5)
            return True
        time.sleep(0.2)
    return False


def wait_for_server_shutdown(host: str, port: int, timeout: float = 10.0) -> bool:
    start_time = time.time()
    while time.time() - start_time < timeout:
        if not is_port_open(host, port):
            return True
        time.sleep(0.2)
    return False


def kill_process_on_port(port: int) -> None:
    """
    终止占用指定端口的进程（排除当前进程树）
    """
    import subprocess as sp

    current_pid = os.getpid()
    current_pgid = os.getpgid(current_pid)

    try:
        result = sp.run(
            ["lsof", "-ti", f":{port}"],
            capture_output=True,
            text=True,
            timeout=5,
        )
        if result.stdout.strip():
            pids = result.stdout.strip().split('\n')
            for pid_str in pids:
                if pid_str:
                    try:
                        pid = int(pid_str)
                        # 跳过当前进程和同进程组的进程
                        if pid == current_pid:
                            continue
                        try:
                            if os.getpgid(pid) == current_pgid:
                                continue
                        except OSError:
                            pass
                        os.kill(pid, signal.SIGKILL)
                    except (ProcessLookupError, ValueError, OSError):
                        pass
            time.sleep(1)
    except (sp.TimeoutExpired, FileNotFoundError):
        pass


def ensure_clean_state() -> None:
    """
    确保测试环境处于干净状态
    """
    if is_port_open(TEST_HOST, TEST_PORT):
        kill_process_on_port(TEST_PORT)
        wait_for_server_shutdown(TEST_HOST, TEST_PORT, timeout=5.0)

    reset_database()


def terminate_server(process: subprocess.Popen) -> None:
    """
    安全终止服务器进程
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
            pass


@pytest.fixture(scope="session")
def server_process() -> Generator[subprocess.Popen, None, None]:
    """
    启动测试服务器（会话级别，所有测试共享）
    """
    global _server_process

    ensure_clean_state()

    project_root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    src_path = os.path.join(project_root, "src")

    env = os.environ.copy()
    env["PYTHONPATH"] = src_path
    env["PYTHONUNBUFFERED"] = "1"

    process = subprocess.Popen(
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


def pytest_sessionfinish(session, exitstatus):
    """
    pytest 会话结束钩子，确保清理
    """
    global _server_process
    if _server_process is not None:
        terminate_server(_server_process)
        _server_process = None

    # 最终清理
    if is_port_open(TEST_HOST, TEST_PORT):
        kill_process_on_port(TEST_PORT)

    try:
        reset_database()
    except Exception:
        pass
