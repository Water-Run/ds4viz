r"""
应用入口，配置加载与启动

:file: src/main.py
:author: WaterRun
:time: 2026-01-27
"""

import os
import re
import sys
from pathlib import Path
from typing import Any

import uvicorn
import yaml
from database import close_pool, init_pool
from exceptions import Ds4VizException
from fastapi import FastAPI, Request
from fastapi.responses import JSONResponse
from log import LogLevel, get_logger, init_logger

CONFIG: dict[str, Any] = {}


def _load_yaml(path: Path) -> dict[str, Any]:
    r"""
    加载YAML配置文件并替换环境变量占位符
    :param path: 配置文件路径
    :return dict: 配置字典
    :raise FileNotFoundError: 配置文件不存在
    """
    if not path.exists():
        raise FileNotFoundError(f"配置文件不存在: {path}")
    content = path.read_text(encoding="utf-8")
    # 替换 ${VAR_NAME} 格式的环境变量
    pattern = re.compile(r"\$\{(\w+)}")

    def replacer(match: re.Match) -> str:
        r"""
        替换匹配的环境变量
        :param match: 正则匹配对象
        :return str: 环境变量值或原字符串
        """
        var_name = match.group(1)
        return os.environ.get(var_name) or match.group(0)

    content = pattern.sub(replacer, content)
    return yaml.safe_load(content)


def _parse_args() -> str:
    r"""
    解析命令行参数
    :return str: 配置环境名
    """
    config_args = [arg for arg in sys.argv[1:] if arg.startswith("--")]

    if len(config_args) == 0:
        print("错误: 必须指定一个配置参数，例如: --test, --default", file=sys.stderr)
        sys.exit(1)

    if len(config_args) > 1:
        print(
            f"错误: 只能指定一个配置参数，但提供了: {', '.join(config_args)}",
            file=sys.stderr,
        )
        sys.exit(1)

    config_name = config_args[0][2:]
    return config_name


def _create_app() -> FastAPI:
    r"""
    创建FastAPI应用实例
    :return FastAPI: 应用实例
    """
    app = FastAPI(title="ds4viz", version="0.1.0",
                  docs_url="/docs", redoc_url=None)

    @app.exception_handler(Ds4VizException)
    async def handle_ds4viz_exception(_: Request, exc: Ds4VizException) -> JSONResponse:
        r"""
        处理自定义异常
        :param _: 请求对象
        :param exc: 异常实例
        :return JSONResponse: JSON错误响应
        """
        return JSONResponse(status_code=exc.status_code, content={"error": exc.message})

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
    global CONFIG
    env = _parse_args()
    config_path = Path(__file__).parent / "config" / f"{env}.yml"
    # 加载配置
    try:
        CONFIG = _load_yaml(config_path)
    except FileNotFoundError as e:
        print(f"错误: {e}", file=sys.stderr)
        sys.exit(1)
    # 初始化数据库
    db_conf = CONFIG["database"]
    try:
        init_pool(
            host=db_conf["host"],
            port=db_conf["port"],
            database=db_conf["database"],
            username=db_conf["username"],
            password=db_conf["password"],
        )
    except Exception as e:
        print(f"数据库连接失败: {e}", file=sys.stderr)
        sys.exit(1)
    # 初始化日志
    init_logger(level=LogLevel.INFO)
    logger = get_logger("main")
    logger.info(f"配置加载完成: {env}")
    # 创建应用
    app = _create_app()
    # 启动服务
    server_conf = CONFIG["server"]
    try:
        logger.info(f"服务启动: {server_conf['host']}:{server_conf['port']}")
        uvicorn.run(app, host=server_conf["host"], port=server_conf["port"])
    finally:
        logger.info("服务已停止")
        close_pool()


if __name__ == "__main__":
    main()
