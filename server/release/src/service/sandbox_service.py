r"""
沙箱服务，提供安全的代码执行环境

:file: src/service/sandbox_service.py
:author: WaterRun
:time: 2026-02-05
"""

import os
import shutil
import subprocess
import time
from abc import ABC, abstractmethod
from dataclasses import dataclass
from pathlib import Path
from typing import NamedTuple

from model import SupportedLanguage
from utils import generate_uuid


@dataclass
class SandboxResult:
    r"""
    沙箱执行结果
    """

    success: bool
    toml_output: str | None
    error_message: str | None
    execution_time_ms: int
    timed_out: bool


@dataclass
class SandboxConfig:
    r"""
    沙箱配置
    """

    timeout_seconds: int = 10
    memory_limit_mb: int = 128
    workspace_base: str = "/tmp/ds4viz_sandbox"


class SubprocessResult(NamedTuple):
    r"""
    子进程执行结果
    """

    return_code: int
    stdout: str
    stderr: str
    timed_out: bool


class LanguageExecutor(ABC):
    r"""
    语言执行器抽象基类
    """

    @property
    @abstractmethod
    def language(self) -> SupportedLanguage:
        r"""
        获取支持的语言类型

        :return SupportedLanguage: 语言类型
        """
        ...

    @property
    @abstractmethod
    def file_extension(self) -> str:
        r"""
        获取文件扩展名

        :return str: 扩展名（含点号）
        """
        ...

    @abstractmethod
    def prepare_files(self, workspace: Path, code: str) -> str:
        r"""
        准备执行所需文件

        :param workspace: 工作目录
        :param code: 源代码
        :return str: 主文件名
        """
        ...

    @abstractmethod
    def get_run_command(self, workspace: Path, filename: str) -> list[str]:
        r"""
        获取执行命令

        :param workspace: 工作目录
        :param filename: 主文件名
        :return list[str]: 命令及参数列表
        """
        ...


class PythonExecutor(LanguageExecutor):
    r"""
    Python语言执行器
    """

    @property
    def language(self) -> SupportedLanguage:
        r"""
        获取支持的语言类型

        :return SupportedLanguage: Python
        """
        return SupportedLanguage.PYTHON

    @property
    def file_extension(self) -> str:
        r"""
        获取文件扩展名

        :return str: .py
        """
        return ".py"

    def prepare_files(self, workspace: Path, code: str) -> str:
        r"""
        准备Python执行文件

        :param workspace: 工作目录
        :param code: Python源代码
        :return str: 主文件名
        """
        filename: str = f"main{self.file_extension}"
        filepath: Path = workspace / filename
        filepath.write_text(code, encoding="utf-8")
        return filename

    def get_run_command(self, workspace: Path, filename: str) -> list[str]:
        r"""
        获取Python执行命令

        :param workspace: 工作目录
        :param filename: 主文件名
        :return list[str]: 命令及参数列表
        """
        return ["python3", "-u", str(workspace / filename)]


class LuaExecutor(LanguageExecutor):
    r"""
    Lua语言执行器
    """

    @property
    def language(self) -> SupportedLanguage:
        r"""
        获取支持的语言类型

        :return SupportedLanguage: Lua
        """
        return SupportedLanguage.LUA

    @property
    def file_extension(self) -> str:
        r"""
        获取文件扩展名

        :return str: .lua
        """
        return ".lua"

    def prepare_files(self, workspace: Path, code: str) -> str:
        r"""
        准备Lua执行文件

        :param workspace: 工作目录
        :param code: Lua源代码
        :return str: 主文件名
        """
        filename: str = f"main{self.file_extension}"
        filepath: Path = workspace / filename
        filepath.write_text(code, encoding="utf-8")
        return filename

    def get_run_command(self, workspace: Path, filename: str) -> list[str]:
        r"""
        获取Lua执行命令

        :param workspace: 工作目录
        :param filename: 主文件名
        :return list[str]: 命令及参数列表
        """
        return ["lua", str(workspace / filename)]


class RustExecutor(LanguageExecutor):
    r"""
    Rust语言执行器，使用rust-script执行
    """

    def __init__(self, ds4viz_path: str = "") -> None:
        r"""
        初始化Rust执行器

        :param ds4viz_path: ds4viz库路径
        """
        self._ds4viz_path: str = ds4viz_path

    @property
    def language(self) -> SupportedLanguage:
        r"""
        获取支持的语言类型

        :return SupportedLanguage: Rust
        """
        return SupportedLanguage.RUST

    @property
    def file_extension(self) -> str:
        r"""
        获取文件扩展名

        :return str: .rs
        """
        return ".rs"

    def prepare_files(self, workspace: Path, code: str) -> str:
        r"""
        准备Rust执行文件，添加cargo依赖声明

        :param workspace: 工作目录
        :param code: Rust源代码
        :return str: 主文件名
        """
        filename: str = "_prepared.rs"
        filepath: Path = workspace / filename

        cargo_header: str = f"""//! ```cargo
//! [dependencies]
//! ds4viz = {{ path = "{self._ds4viz_path}" }}
//! ```

"""
        prepared_code: str = cargo_header + code
        filepath.write_text(prepared_code, encoding="utf-8")
        return filename

    def get_run_command(self, workspace: Path, filename: str) -> list[str]:
        r"""
        获取Rust执行命令

        :param workspace: 工作目录
        :param filename: 主文件名
        :return list[str]: 命令及参数列表
        """
        return ["rust-script", str(workspace / filename)]


def _get_executor(language: SupportedLanguage) -> LanguageExecutor:
    r"""
    获取语言执行器

    :param language: 语言类型
    :return LanguageExecutor: 执行器实例
    :raise ValueError: 不支持的语言
    """
    from config import get_config

    match language:
        case SupportedLanguage.PYTHON:
            return PythonExecutor()
        case SupportedLanguage.LUA:
            return LuaExecutor()
        case SupportedLanguage.RUST:
            config = get_config()
            ds4viz_path: str = getattr(
                getattr(config, "library", None), "rust_ds4viz_path", ""
            ) if hasattr(config, "library") else ""
            return RustExecutor(ds4viz_path=ds4viz_path)
        case _:
            raise ValueError(f"不支持的语言: {language}")


def _prepare_workspace(config: SandboxConfig) -> Path:
    r"""
    创建工作目录

    :param config: 沙箱配置
    :return Path: 工作目录路径
    """
    base_path: Path = Path(config.workspace_base)
    base_path.mkdir(parents=True, exist_ok=True)

    workspace_id: str = generate_uuid()
    workspace: Path = base_path / workspace_id
    workspace.mkdir(parents=True, exist_ok=True)

    return workspace


def _scan_toml_output(workspace: Path, before_files: set[str]) -> str | None:
    r"""
    扫描工作目录中新增的TOML文件

    :param workspace: 工作目录
    :param before_files: 执行前的文件集合
    :return str | None: TOML文件内容，无新增则返回None
    """
    after_files: set[str] = {
        f.name for f in workspace.iterdir() if f.is_file()}
    new_files: set[str] = after_files - before_files

    toml_files: list[str] = [f for f in new_files if f.endswith(".toml")]

    if not toml_files:
        return None

    toml_file: str = sorted(toml_files)[0]
    toml_path: Path = workspace / toml_file

    return toml_path.read_text(encoding="utf-8")


def _cleanup_workspace(workspace: Path) -> None:
    r"""
    清理工作目录

    :param workspace: 工作目录
    """
    if workspace.exists():
        shutil.rmtree(workspace, ignore_errors=True)


def _build_execution_env(workspace: Path) -> dict[str, str]:
    r"""
    构建执行环境变量，继承系统关键路径以支持已安装的库

    :param workspace: 工作目录
    :return dict[str, str]: 环境变量字典
    """
    system_path: str = os.environ.get("PATH", "/usr/local/bin:/usr/bin:/bin")
    home_dir: str = os.environ.get("HOME", str(workspace))

    env: dict[str, str] = {
        "PATH": system_path,
        "HOME": home_dir,
        "TMPDIR": str(workspace),
        "LANG": "C.UTF-8",
        "LC_ALL": "C.UTF-8",
    }

    pythonpath: str = os.environ.get("PYTHONPATH", "")
    if pythonpath:
        env["PYTHONPATH"] = pythonpath

    lua_path: str = os.environ.get("LUA_PATH", "")
    if lua_path:
        env["LUA_PATH"] = lua_path

    lua_cpath: str = os.environ.get("LUA_CPATH", "")
    if lua_cpath:
        env["LUA_CPATH"] = lua_cpath

    cargo_home: str = os.environ.get("CARGO_HOME", "")
    if cargo_home:
        env["CARGO_HOME"] = cargo_home

    rustup_home: str = os.environ.get("RUSTUP_HOME", "")
    if rustup_home:
        env["RUSTUP_HOME"] = rustup_home

    return env


def _run_with_timeout(
    command: list[str],
    workspace: Path,
    timeout_seconds: int,
    env: dict[str, str],
) -> SubprocessResult:
    r"""
    使用timeout命令执行代码

    :param command: 执行命令列表
    :param workspace: 工作目录
    :param timeout_seconds: 超时秒数
    :param env: 环境变量
    :return SubprocessResult: 子进程执行结果
    """
    timeout_command: list[str] = [
        "timeout",
        "--signal=KILL",
        str(timeout_seconds),
        *command,
    ]

    try:
        result: subprocess.CompletedProcess = subprocess.run(
            timeout_command,
            capture_output=True,
            text=True,
            timeout=timeout_seconds + 5,
            cwd=str(workspace),
            env=env,
        )

        timed_out: bool = result.returncode == 137 or result.returncode == -9

        return SubprocessResult(
            return_code=result.returncode,
            stdout=result.stdout,
            stderr=result.stderr,
            timed_out=timed_out,
        )

    except subprocess.TimeoutExpired:
        return SubprocessResult(
            return_code=-1,
            stdout="",
            stderr="执行超时",
            timed_out=True,
        )


def _run_with_systemd(
    command: list[str],
    workspace: Path,
    timeout_seconds: int,
    memory_limit_mb: int,
    env: dict[str, str],
) -> SubprocessResult:
    r"""
    使用systemd-run执行代码（资源限制）

    :param command: 执行命令列表
    :param workspace: 工作目录
    :param timeout_seconds: 超时秒数
    :param memory_limit_mb: 内存限制MB
    :param env: 环境变量
    :return SubprocessResult: 子进程执行结果
    """
    systemd_command: list[str] = [
        "systemd-run",
        "--user",
        "--scope",
        "--quiet",
        f"--property=MemoryMax={memory_limit_mb}M",
        "--property=CPUQuota=50%",
        "--",
        "timeout",
        "--signal=KILL",
        str(timeout_seconds),
        *command,
    ]

    try:
        result: subprocess.CompletedProcess = subprocess.run(
            systemd_command,
            capture_output=True,
            text=True,
            timeout=timeout_seconds + 10,
            cwd=str(workspace),
            env=env,
        )

        timed_out: bool = result.returncode == 137 or result.returncode == -9

        return SubprocessResult(
            return_code=result.returncode,
            stdout=result.stdout,
            stderr=result.stderr,
            timed_out=timed_out,
        )

    except subprocess.TimeoutExpired:
        return SubprocessResult(
            return_code=-1,
            stdout="",
            stderr="执行超时",
            timed_out=True,
        )


def _check_systemd_available() -> bool:
    r"""
    检查systemd-run --user是否可用

    需要DBUS_SESSION_BUS_ADDRESS和XDG_RUNTIME_DIR环境变量支持用户会话，
    并实际测试执行以确保可用

    :return bool: 是否可用
    """
    has_dbus: bool = bool(os.environ.get("DBUS_SESSION_BUS_ADDRESS"))
    has_xdg: bool = bool(os.environ.get("XDG_RUNTIME_DIR"))

    if not has_dbus or not has_xdg:
        return False

    xdg_runtime_dir: str = os.environ.get("XDG_RUNTIME_DIR", "")
    if xdg_runtime_dir and not os.path.isdir(xdg_runtime_dir):
        return False

    try:
        result: subprocess.CompletedProcess = subprocess.run(
            ["systemd-run", "--user", "--scope", "--quiet", "--", "true"],
            capture_output=True,
            text=True,
            timeout=5,
        )
        if result.returncode != 0:
            return False
        if "DBUS_SESSION_BUS_ADDRESS" in result.stderr:
            return False
        if "XDG_RUNTIME_DIR" in result.stderr:
            return False
        return True
    except (subprocess.TimeoutExpired, FileNotFoundError, OSError):
        return False

def _run_in_sandbox(
    executor: LanguageExecutor,
    workspace: Path,
    filename: str,
    config: SandboxConfig,
) -> SubprocessResult:
    r"""
    在沙箱中执行代码

    优先使用systemd-run进行资源限制，不可用时降级为timeout命令

    :param executor: 语言执行器
    :param workspace: 工作目录
    :param filename: 主文件名
    :param config: 沙箱配置
    :return SubprocessResult: 子进程执行结果
    """
    run_command: list[str] = executor.get_run_command(workspace, filename)
    env: dict[str, str] = _build_execution_env(workspace)

    if _check_systemd_available():
        return _run_with_systemd(
            command=run_command,
            workspace=workspace,
            timeout_seconds=config.timeout_seconds,
            memory_limit_mb=config.memory_limit_mb,
            env=env,
        )

    return _run_with_timeout(
        command=run_command,
        workspace=workspace,
        timeout_seconds=config.timeout_seconds,
        env=env,
    )

def run_code(
    language: SupportedLanguage,
    code: str,
    config: SandboxConfig | None = None,
) -> SandboxResult:
    r"""
    在沙箱中执行代码

    :param language: 语言类型
    :param code: 源代码
    :param config: 沙箱配置（可选，使用默认配置）
    :return SandboxResult: 执行结果
    """
    if config is None:
        config = SandboxConfig()

    executor: LanguageExecutor = _get_executor(language)
    workspace: Path | None = None

    start_time: float = time.perf_counter()

    try:
        workspace = _prepare_workspace(config)
        filename: str = executor.prepare_files(workspace, code)

        before_files: set[str] = {
            f.name for f in workspace.iterdir() if f.is_file()}

        proc_result: SubprocessResult = _run_in_sandbox(
            executor, workspace, filename, config
        )

        execution_time_ms: int = int((time.perf_counter() - start_time) * 1000)

        if proc_result.timed_out:
            return SandboxResult(
                success=False,
                toml_output=None,
                error_message="执行超时",
                execution_time_ms=execution_time_ms,
                timed_out=True,
            )

        if proc_result.return_code != 0:
            error_msg: str = (
                proc_result.stderr.strip()
                if proc_result.stderr
                else f"进程退出码: {proc_result.return_code}"
            )
            return SandboxResult(
                success=False,
                toml_output=None,
                error_message=error_msg,
                execution_time_ms=execution_time_ms,
                timed_out=False,
            )

        toml_output: str | None = _scan_toml_output(workspace, before_files)

        if toml_output is None:
            return SandboxResult(
                success=False,
                toml_output=None,
                error_message="未生成TOML输出文件",
                execution_time_ms=execution_time_ms,
                timed_out=False,
            )

        return SandboxResult(
            success=True,
            toml_output=toml_output,
            error_message=None,
            execution_time_ms=execution_time_ms,
            timed_out=False,
        )

    except Exception as err:
        execution_time_ms = int((time.perf_counter() - start_time) * 1000)
        return SandboxResult(
            success=False,
            toml_output=None,
            error_message=f"执行异常: {err!r}",
            execution_time_ms=execution_time_ms,
            timed_out=False,
        )

    finally:
        if workspace is not None:
            _cleanup_workspace(workspace)
