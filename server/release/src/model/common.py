r"""
通用数据模型，定义共享的枚举和类型

:file: src/model/common.py
:author: WaterRun
:time: 2026-01-28
"""

from enum import Enum


class SupportedLanguage(Enum):
    r"""
    支持的编程语言
    """
    PYTHON = "python"
    LUA = "lua"
    RUST = "rust"
