r"""
数据库连接池管理

:file: src/database.py
:author: WaterRun
:time: 2026-01-29
"""

from contextlib import contextmanager
from typing import Generator

import psycopg
from psycopg_pool import ConnectionPool

_pool: ConnectionPool | None = None


def init_pool(
    host: str,
    port: int,
    database: str,
    username: str,
    password: str,
    min_size: int = 2,
    max_size: int = 10,
) -> None:
    r"""
    初始化数据库连接池

    :param host: 数据库主机
    :param port: 数据库端口
    :param database: 数据库名
    :param username: 用户名
    :param password: 密码
    :param min_size: 最小连接数
    :param max_size: 最大连接数
    :raise RuntimeError: 连接池已初始化
    """
    global _pool

    if _pool is not None:
        raise RuntimeError("连接池已初始化")

    conninfo: str = (
        f"host={host} port={port} dbname={database} "
        f"user={username} password={password} "
        f"options='-c timezone=UTC'"
    )
    _pool = ConnectionPool(conninfo, min_size=min_size, max_size=max_size)


def close_pool() -> None:
    r"""
    关闭数据库连接池
    """
    global _pool

    if _pool is not None:
        _pool.close()
        _pool = None


def get_pool() -> ConnectionPool:
    r"""
    获取当前连接池

    :return ConnectionPool: 连接池对象
    :raise RuntimeError: 连接池未初始化
    """
    if _pool is None:
        raise RuntimeError("连接池未初始化")
    return _pool


@contextmanager
def get_connection() -> Generator[psycopg.Connection, None, None]:
    r"""
    获取数据库连接的上下文管理器

    :return Generator: 数据库连接
    :raise RuntimeError: 连接池未初始化
    """
    pool: ConnectionPool = get_pool()
    with pool.connection() as conn:
        yield conn
