r"""
配置单例模块，提供全局配置访问

:file: src/config.py
:author: WaterRun
:time: 2026-02-04
"""

import os
import re
from dataclasses import dataclass, field
from pathlib import Path
from typing import Any

import yaml


@dataclass(frozen=True)
class ServerConfig:
    r"""
    服务器配置
    """

    host: str = "0.0.0.0"
    port: int = 10000


@dataclass(frozen=True)
class DatabaseConfig:
    r"""
    数据库配置
    """

    host: str = "localhost"
    port: int = 5432
    database: str = "ds4viz_prod"
    username: str = "ds4viz"
    password: str = ""


@dataclass(frozen=True)
class SecurityConfig:
    r"""
    安全配置
    """

    access_token_expire_hours: int = 24
    bcrypt_rounds: int = 12
    jwt_secret: str = ""
    jwt_algorithm: str = "HS256"


@dataclass(frozen=True)
class SandboxConfig:
    r"""
    沙箱配置
    """

    timeout_seconds: int = 30
    max_memory_mb: int = 512
    temp_dir: str = "/tmp/ds4viz/exec"


@dataclass(frozen=True)
class LibraryConfig:
    r"""
    ds4viz库路径配置
    """

    python_path: str = ""
    lua_path: str = "/usr/share/lua/5.4"
    lua_cpath: str = "/usr/lib64/lua/5.4"
    rust_ds4viz_path: str = ""


@dataclass(frozen=True)
class AppConfig:
    r"""
    应用总配置
    """

    server: ServerConfig = field(default_factory=ServerConfig)
    database: DatabaseConfig = field(default_factory=DatabaseConfig)
    security: SecurityConfig = field(default_factory=SecurityConfig)
    sandbox: SandboxConfig = field(default_factory=SandboxConfig)
    library: LibraryConfig = field(default_factory=LibraryConfig)
    env: str = "prod"


_config: AppConfig | None = None


def _replace_env_vars(content: str) -> str:
    r"""
    替换配置内容中的环境变量占位符

    :param content: 原始配置内容
    :return str: 替换后的内容
    """
    pattern = re.compile(r"\$\{(\w+)}")

    def replacer(match: re.Match) -> str:
        r"""
        替换匹配的环境变量

        :param match: 正则匹配对象
        :return str: 环境变量值或原字符串
        """
        var_name: str = match.group(1)
        return os.environ.get(var_name, match.group(0))

    return pattern.sub(replacer, content)


def _parse_config(raw: dict[str, Any], env: str) -> AppConfig:
    r"""
    解析原始配置字典为配置对象

    :param raw: 原始配置字典
    :param env: 环境名称
    :return AppConfig: 配置对象
    """
    server_raw: dict[str, Any] = raw.get("server", {})
    db_raw: dict[str, Any] = raw.get("database", {})
    sec_raw: dict[str, Any] = raw.get("security", {})
    sandbox_raw: dict[str, Any] = raw.get("sandbox", {})
    library_raw: dict[str, Any] = raw.get("library", {})

    return AppConfig(
        server=ServerConfig(
            host=server_raw.get("host", "0.0.0.0"),
            port=server_raw.get("port", 10000),
        ),
        database=DatabaseConfig(
            host=db_raw.get("host", "localhost"),
            port=db_raw.get("port", 5432),
            database=db_raw.get("database", "ds4viz_prod"),
            username=db_raw.get("username", "ds4viz"),
            password=db_raw.get("password", ""),
        ),
        security=SecurityConfig(
            access_token_expire_hours=sec_raw.get(
                "access_token_expire_hours", 24),
            bcrypt_rounds=sec_raw.get("bcrypt_rounds", 12),
            jwt_secret=sec_raw.get("jwt_secret", os.environ.get(
                "DS4VIZ_JWT_SECRET", "dev-secret-change-in-production")),
            jwt_algorithm=sec_raw.get("jwt_algorithm", "HS256"),
        ),
        sandbox=SandboxConfig(
            timeout_seconds=sandbox_raw.get("timeout_seconds", 30),
            max_memory_mb=sandbox_raw.get("max_memory_mb", 512),
            temp_dir=sandbox_raw.get("temp_dir", "/tmp/ds4viz/exec"),
        ),
        library=LibraryConfig(
            python_path=library_raw.get("python_path", ""),
            lua_path=library_raw.get("lua_path", "/usr/share/lua/5.4"),
            lua_cpath=library_raw.get("lua_cpath", "/usr/lib64/lua/5.4"),
            rust_ds4viz_path=library_raw.get("rust_ds4viz_path", ""),
        ),
        env=env,
    )


def init_config(config_path: Path, env: str) -> None:
    r"""
    初始化全局配置

    :param config_path: 配置文件路径
    :param env: 环境名称
    :raise FileNotFoundError: 配置文件不存在
    :raise RuntimeError: 配置已初始化
    """
    global _config

    if _config is not None:
        raise RuntimeError("配置已初始化")

    if not config_path.exists():
        raise FileNotFoundError(f"配置文件不存在: {config_path}")

    content: str = config_path.read_text(encoding="utf-8")
    content = _replace_env_vars(content)
    raw: dict[str, Any] = yaml.safe_load(content)

    _config = _parse_config(raw, env)


def get_config() -> AppConfig:
    r"""
    获取全局配置

    :return AppConfig: 配置对象
    :raise RuntimeError: 配置未初始化
    """
    if _config is None:
        raise RuntimeError("配置未初始化，请先调用 init_config()")
    return _config


def reset_config() -> None:
    r"""
    重置配置（仅用于测试）
    """
    global _config
    _config = None
