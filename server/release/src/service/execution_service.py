r"""
执行服务，处理代码执行请求与缓存管理

:file: src/service/execution_service.py
:author: WaterRun
:time: 2026-01-29
"""

from config import get_config, SandboxConfig as ConfigSandboxConfig
from database import get_connection
from exceptions import ExecutionNotFoundError, ExecutionPermissionError
from model import (
    ExecuteRequest,
    ExecuteResponse,
    ExecutionStatus,
    ExecutionDetail,
    ExecutionListResponse,
    ExecutionHistoryItem,
    SupportedLanguage,
)
from service.sandbox_service import SandboxResult, SandboxConfig, run_code
from service.user_service import require_active_status
from utils import get_utc_now, compute_hash


def _check_cache(code_hash: str, language: str) -> tuple[bool, str | None]:
    r"""
    检查执行缓存

    :param code_hash: 代码哈希值
    :param language: 语言类型
    :return tuple[bool, str | None]: (是否命中, TOML输出内容)
    """
    with get_connection() as conn:
        with conn.cursor() as cur:
            cur.execute(
                """
                SELECT toml_output FROM execution_cache
                WHERE code_hash = %s AND language = %s
                """,
                (code_hash, language),
            )
            row: tuple | None = cur.fetchone()

            if row is None:
                return False, None

            cur.execute(
                """
                UPDATE execution_cache
                SET hit_count = hit_count + 1, last_hit_at = %s
                WHERE code_hash = %s AND language = %s
                """,
                (get_utc_now(), code_hash, language),
            )
        conn.commit()

    return True, row[0]


def _save_to_cache(code_hash: str, language: str, toml_output: str) -> None:
    r"""
    保存执行结果到缓存

    :param code_hash: 代码哈希值
    :param language: 语言类型
    :param toml_output: TOML输出内容
    """
    with get_connection() as conn:
        with conn.cursor() as cur:
            cur.execute(
                """
                INSERT INTO execution_cache (code_hash, language, toml_output)
                VALUES (%s, %s, %s)
                ON CONFLICT (code_hash, language) DO UPDATE
                SET toml_output = EXCLUDED.toml_output,
                    hit_count = execution_cache.hit_count + 1,
                    last_hit_at = %s
                """,
                (code_hash, language, toml_output, get_utc_now()),
            )
        conn.commit()


def _save_execution(
    user_id: int,
    code_hash: str,
    language: str,
    code: str,
    toml_output: str | None,
    status: ExecutionStatus,
    error_message: str | None,
    execution_time: int,
    ip_address: str,
) -> int:
    r"""
    保存执行记录

    :param user_id: 用户ID
    :param code_hash: 代码哈希值
    :param language: 语言类型
    :param code: 源代码
    :param toml_output: TOML输出（可选）
    :param status: 执行状态
    :param error_message: 错误信息（可选）
    :param execution_time: 执行耗时（毫秒）
    :param ip_address: IP地址
    :return int: 执行记录ID
    """
    with get_connection() as conn:
        with conn.cursor() as cur:
            cur.execute(
                """
                INSERT INTO executions (
                    user_id, code_hash, language, code, toml_output,
                    status, error_message, execution_time, ip_address
                )
                VALUES (%s, %s, %s, %s, %s, %s, %s, %s, %s)
                RETURNING id
                """,
                (
                    user_id,
                    code_hash,
                    language,
                    code,
                    toml_output,
                    status.value,
                    error_message,
                    execution_time,
                    ip_address,
                ),
            )
            execution_id: int = cur.fetchone()[0]
        conn.commit()

    return execution_id


def create_sandbox_config_from_app_config() -> SandboxConfig:
    r"""
    从应用配置创建沙箱配置

    :return SandboxConfig: 沙箱配置对象
    """
    config = get_config()
    sandbox_conf: ConfigSandboxConfig = config.sandbox
    return SandboxConfig(
        timeout_seconds=sandbox_conf.timeout_seconds,
        memory_limit_mb=sandbox_conf.max_memory_mb,
        workspace_base=sandbox_conf.temp_dir,
    )


def execute_code(
    user_id: int,
    data: ExecuteRequest,
    ip_address: str,
    sandbox_config: SandboxConfig | None = None,
) -> ExecuteResponse:
    r"""
    执行代码

    :param user_id: 用户ID
    :param data: 执行请求数据
    :param ip_address: 客户端IP地址
    :param sandbox_config: 沙箱配置（可选，默认从应用配置读取）
    :return ExecuteResponse: 执行响应
    :raise UserBannedError: 用户已被封禁
    :raise UserSuspendedError: 用户已被暂停
    """
    require_active_status(user_id)

    if sandbox_config is None:
        sandbox_config = create_sandbox_config_from_app_config()

    code_hash: str = compute_hash(data.code)
    language_str: str = data.language.value

    cache_hit, cached_output = _check_cache(code_hash, language_str)

    if cache_hit and cached_output is not None:
        execution_id: int = _save_execution(
            user_id=user_id,
            code_hash=code_hash,
            language=language_str,
            code=data.code,
            toml_output=cached_output,
            status=ExecutionStatus.SUCCESS,
            error_message=None,
            execution_time=0,
            ip_address=ip_address,
        )

        return ExecuteResponse(
            execution_id=execution_id,
            status=ExecutionStatus.SUCCESS,
            toml_output=cached_output,
            error_message=None,
            execution_time=0,
            cached=True,
        )

    result: SandboxResult = run_code(
        language=data.language,
        code=data.code,
        config=sandbox_config,
    )

    if result.timed_out:
        status = ExecutionStatus.TIMEOUT
    elif result.success:
        status = ExecutionStatus.SUCCESS
    else:
        status = ExecutionStatus.ERROR

    if result.success and result.toml_output is not None:
        _save_to_cache(code_hash, language_str, result.toml_output)

    execution_id = _save_execution(
        user_id=user_id,
        code_hash=code_hash,
        language=language_str,
        code=data.code,
        toml_output=result.toml_output,
        status=status,
        error_message=result.error_message,
        execution_time=result.execution_time_ms,
        ip_address=ip_address,
    )

    return ExecuteResponse(
        execution_id=execution_id,
        status=status,
        toml_output=result.toml_output,
        error_message=result.error_message,
        execution_time=result.execution_time_ms,
        cached=False,
    )


def get_execution_by_id(execution_id: int, user_id: int) -> ExecutionDetail:
    r"""
    获取执行详情（限本人访问）

    :param execution_id: 执行记录ID
    :param user_id: 当前用户ID
    :return ExecutionDetail: 执行详情
    :raise ExecutionNotFoundError: 执行记录不存在
    :raise ExecutionPermissionError: 无权访问该记录
    """
    with get_connection() as conn:
        with conn.cursor() as cur:
            cur.execute(
                """
                SELECT id, user_id, code_hash, language, code, toml_output,
                       status, error_message, execution_time, created_at
                FROM executions WHERE id = %s
                """,
                (execution_id,),
            )
            row: tuple | None = cur.fetchone()

            if row is None:
                raise ExecutionNotFoundError(f"执行记录不存在: {execution_id}")

            record_user_id: int = row[1]
            if record_user_id != user_id:
                raise ExecutionPermissionError(f"无权访问该执行记录: {execution_id}")

            return ExecutionDetail(
                id=row[0],
                language=SupportedLanguage(row[3]),
                code=row[4],
                toml_output=row[5],
                status=ExecutionStatus(row[6]),
                error_message=row[7],
                execution_time=row[8],
                created_at=row[9],
            )


def list_executions(
    user_id: int,
    page: int = 1,
    limit: int = 20,
) -> ExecutionListResponse:
    r"""
    获取用户执行历史

    :param user_id: 用户ID
    :param page: 页码（从1开始）
    :param limit: 每页数量
    :return ExecutionListResponse: 执行历史列表
    """
    offset: int = (page - 1) * limit

    with get_connection() as conn:
        with conn.cursor() as cur:
            cur.execute(
                """
                SELECT COUNT(*) FROM executions WHERE user_id = %s
                """,
                (user_id,),
            )
            total: int = cur.fetchone()[0]

            cur.execute(
                """
                SELECT id, language, code, status, execution_time, created_at
                FROM executions
                WHERE user_id = %s
                ORDER BY created_at DESC
                LIMIT %s OFFSET %s
                """,
                (user_id, limit, offset),
            )
            rows: list[tuple] = cur.fetchall()

            items: list[ExecutionHistoryItem] = [
                ExecutionHistoryItem(
                    id=row[0],
                    language=SupportedLanguage(row[1]),
                    code=row[2],
                    status=ExecutionStatus(row[3]),
                    execution_time=row[4],
                    created_at=row[5],
                )
                for row in rows
            ]

    return ExecutionListResponse(
        items=items,
        total=total,
        page=page,
        limit=limit,
    )
