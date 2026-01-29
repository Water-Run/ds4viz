r"""
应用入口，配置加载与启动

:file: src/main.py
:author: WaterRun
:time: 2026-01-29
"""

import asyncio
import sys
from contextlib import asynccontextmanager
from pathlib import Path
from typing import AsyncGenerator

import uvicorn
from config import AppConfig, init_config, get_config
from database import close_pool, init_pool
from exceptions import Ds4VizException
from fastapi import FastAPI, Request
from fastapi.responses import JSONResponse
from log import LogLevel, get_logger, init_logger
from service.auth_service import cleanup_expired_sessions

_cleanup_task: asyncio.Task | None = None


async def _session_cleanup_loop(interval_seconds: int = 3600) -> None:
    r"""
    定期清理过期会话的后台任务

    :param interval_seconds: 清理间隔（秒），默认1小时
    """
    logger = get_logger("session_cleanup")
    while True:
        try:
            await asyncio.sleep(interval_seconds)
            deleted: int = cleanup_expired_sessions()
            if deleted > 0:
                logger.info(f"清理过期会话: {deleted} 条")
        except asyncio.CancelledError:
            logger.info("会话清理任务已停止")
            break
        except Exception as err:
            logger.error("清理过期会话失败", exc=err)


def _parse_args() -> str:
    r"""
    解析命令行参数

    :return str: 配置环境名
    """
    config_args: list[str] = [
        arg for arg in sys.argv[1:] if arg.startswith("--")]

    if len(config_args) == 0:
        print("错误: 必须指定一个配置参数，例如: --test, --prod", file=sys.stderr)
        sys.exit(1)

    if len(config_args) > 1:
        print(
            f"错误: 只能指定一个配置参数，但提供了: {', '.join(config_args)}",
            file=sys.stderr,
        )
        sys.exit(1)

    return config_args[0][2:]


@asynccontextmanager
async def _lifespan(app: FastAPI) -> AsyncGenerator[None, None]:
    r"""
    应用生命周期管理

    :param app: FastAPI应用实例
    """
    global _cleanup_task
    logger = get_logger("main")

    logger.info("启动会话清理后台任务")
    _cleanup_task = asyncio.create_task(_session_cleanup_loop())

    yield

    if _cleanup_task is not None:
        logger.info("停止会话清理后台任务")
        _cleanup_task.cancel()
        try:
            await _cleanup_task
        except asyncio.CancelledError:
            ...


def _create_app() -> FastAPI:
    r"""
    创建FastAPI应用实例

    :return FastAPI: 应用实例
    """
    app = FastAPI(
        title="ds4viz",
        version="0.1.0",
        docs_url="/docs",
        redoc_url=None,
        lifespan=_lifespan,
    )

    @app.exception_handler(Ds4VizException)
    async def handle_ds4viz_exception(
        _: Request,
        exc: Ds4VizException,
    ) -> JSONResponse:
        r"""
        处理自定义异常

        :param _: 请求对象
        :param exc: 异常实例
        :return JSONResponse: JSON错误响应
        """
        return JSONResponse(
            status_code=exc.status_code,
            content={"error": exc.message},
        )

    @app.get("/health")
    async def health_check() -> dict[str, str]:
        r"""
        健康检查接口

        :return dict: 状态信息
        """
        return {"status": "ok"}

    return app


def main() -> None:
    r"""
    主入口函数
    """
    env: str = _parse_args()
    config_path: Path = Path(__file__).parent / "config" / f"{env}.yml"

    try:
        init_config(config_path, env)
    except FileNotFoundError as err:
        print(f"错误: {err}", file=sys.stderr)
        sys.exit(1)
    except RuntimeError as err:
        print(f"错误: {err}", file=sys.stderr)
        sys.exit(1)

    config: AppConfig = get_config()
    db_conf = config.database

    try:
        init_pool(
            host=db_conf.host,
            port=db_conf.port,
            database=db_conf.database,
            username=db_conf.username,
            password=db_conf.password,
        )
    except Exception as err:
        print(f"数据库连接失败: {err!r}", file=sys.stderr)
        sys.exit(1)

    init_logger(level=LogLevel.INFO)
    logger = get_logger("main")
    logger.info(f"配置加载完成: {env}")

    app: FastAPI = _create_app()
    server_conf = config.server

    try:
        logger.info(f"服务启动: {server_conf.host}:{server_conf.port}")
        uvicorn.run(app, host=server_conf.host, port=server_conf.port)
    except Exception as err:
        logger.error("服务启动失败", exc=err)
        raise
    finally:
        logger.info("服务已停止")
        close_pool()


if __name__ == "__main__":
    main()
