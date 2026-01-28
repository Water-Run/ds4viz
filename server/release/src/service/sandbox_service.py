r"""
沙箱服务，提供安全的代码执行环境

:file: src/service/sandbox_service.py
:author: WaterRun
:time: 2026-01-28
"""

import os
import shutil
import subprocess
import tempfile
import time
from abc import ABC, abstractmethod
from dataclasses import dataclass
from pathlib import Path

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
    cache_hit: bool
    timed_out: bool


@dataclass
class SandboxConfig:
    r"""
    沙箱配置
    """
    timeout_seconds: int = 10
    memory_limit_mb: int = 128
    workspace_base: str = "/tmp/ds4viz_sandbox"


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
        return ["python3", str(workspace / filename)]


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
    Rust语言执行器
    """

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
        准备Rust执行文件并编译

        :param workspace: 工作目录
        :param code: Rust源代码
        :return str: 主文件名
        """
        filename: str = f"main{self.file_extension}"
        filepath: Path = workspace / filename
        filepath.write_text(code, encoding="utf-8")
        return filename

    def get_run_command(self, workspace: Path, filename: str) -> list[str]:
        r"""
        获取Rust编译并执行的命令（使用shell串联编译和运行）

        :param workspace: 工作目录
        :param filename: 主文件名
        :return list[str]: 命令及参数列表
        """
        source_path: str = str(workspace / filename)
        binary_path: str = str(workspace / "main")
        return ["sh", "-c", f"rustc {source_path} -o {binary_path} && {binary_path}"]


_EXECUTORS: dict[SupportedLanguage, LanguageExecutor] = {
    SupportedLanguage.PYTHON: PythonExecutor(),
    SupportedLanguage.LUA: LuaExecutor(),
    SupportedLanguage.RUST: RustExecutor(),
}


def _get_executor(language: SupportedLanguage) -> LanguageExecutor:
    r"""
    获取语言执行器

    :param language: 语言类型
    :return LanguageExecutor: 执行器实例
    :raise ValueError: 不支持的语言
    """
    executor: LanguageExecutor | None = _EXECUTORS.get(language)
    if executor is None:
        raise ValueError(f"不支持的语言: {language}")
    return executor


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
    :return None: 无返回值
    """
    if workspace.exists():
        shutil.rmtree(workspace, ignore_errors=True)


def _run_in_sandbox(
    executor: LanguageExecutor,
    workspace: Path,
    filename: str,
    config: SandboxConfig,
) -> tuple[int, str, str, bool]:
    r"""
    在systemd-run沙箱中执行代码

    :param executor: 语言执行器
    :param workspace: 工作目录
    :param filename: 主文件名
    :param config: 沙箱配置
    :return tuple[int, str, str, bool]: (返回码, 标准输出, 标准错误, 是否超时)
    """
    run_command: list[str] = executor.get_run_command(workspace, filename)

    systemd_command: list[str] = [
        "systemd-run",
        "--user",
        "--scope",
        "--quiet",
        f"--property=MemoryMax={config.memory_limit_mb}M",
        f"--property=CPUQuota=50%",
        "--property=PrivateTmp=yes",
        "--",
        *run_command,
    ]

    timed_out: bool = False

    try:
        result: subprocess.CompletedProcess = subprocess.run(
            systemd_command,
            capture_output=True,
            text=True,
            timeout=config.timeout_seconds,
            cwd=str(workspace),
            env={
                **os.environ,
                "HOME": str(workspace),
                "TMPDIR": str(workspace),
            },
        )
        return result.returncode, result.stdout, result.stderr, False

    except subprocess.TimeoutExpired:
        return -1, "", "执行超时", True


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

        return_code, stdout, stderr, timed_out = _run_in_sandbox(
            executor, workspace, filename, config
        )

        execution_time_ms: int = int((time.perf_counter() - start_time) * 1000)

        if timed_out:
            return SandboxResult(
                success=False,
                toml_output=None,
                error_message="执行超时",
                execution_time_ms=execution_time_ms,
                cache_hit=False,
                timed_out=True,
            )

        if return_code != 0:
            error_msg: str = stderr.strip(
            ) if stderr else f"进程退出码: {return_code}"
            return SandboxResult(
                success=False,
                toml_output=None,
                error_message=error_msg,
                execution_time_ms=execution_time_ms,
                cache_hit=False,
                timed_out=False,
            )

        toml_output: str | None = _scan_toml_output(workspace, before_files)

        if toml_output is None:
            return SandboxResult(
                success=False,
                toml_output=None,
                error_message="未生成TOML输出文件",
                execution_time_ms=execution_time_ms,
                cache_hit=False,
                timed_out=False,
            )

        return SandboxResult(
            success=True,
            toml_output=toml_output,
            error_message=None,
            execution_time_ms=execution_time_ms,
            cache_hit=False,
            timed_out=False,
        )

    except Exception as e:
        execution_time_ms = int((time.perf_counter() - start_time) * 1000)
        return SandboxResult(
            success=False,
            toml_output=None,
            error_message=f"执行异常: {str(e)}",
            execution_time_ms=execution_time_ms,
            cache_hit=False,
            timed_out=False,
        )

    finally:
        if workspace is not None:
            _cleanup_workspace(workspace)
