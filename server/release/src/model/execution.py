r"""
执行相关模型定义

:file: src/model/execution.py
:author: WaterRun
:time: 2026-03-15
"""

from datetime import datetime
from enum import StrEnum

from pydantic import BaseModel, Field


class ExecutionStatus(StrEnum):
    r"""
    执行状态枚举
    """

    SUCCESS = "Success"
    ERROR = "Error"
    TIMEOUT = "Timeout"


class SupportedLanguage(StrEnum):
    r"""
    支持的编程语言枚举
    """

    PYTHON = "python"
    LUA = "lua"
    RUST = "rust"
    C = "c"


class ExecuteRequest(BaseModel):
    r"""
    代码执行请求体
    """

    language: SupportedLanguage = Field(description="编程语言")
    code: str = Field(description="待执行的代码")


class ExecuteResponse(BaseModel):
    r"""
    代码执行响应
    """

    execution_id: int = Field(description="执行记录ID")
    status: ExecutionStatus = Field(description="执行状态")
    toml_output: str | None = Field(default=None, description="生成的TOML内容")
    error_message: str | None = Field(default=None, description="错误信息")
    execution_time: int = Field(description="执行时间，单位毫秒")
    cached: bool = Field(description="是否命中缓存")


class ExecutionHistoryItem(BaseModel):
    r"""
    执行历史列表项
    """

    id: int = Field(description="执行记录ID")
    language: SupportedLanguage = Field(description="编程语言")
    code: str = Field(description="源代码")
    status: ExecutionStatus = Field(description="执行状态")
    execution_time: int | None = Field(default=None, description="执行时间，单位毫秒")
    created_at: datetime = Field(description="执行时间")


class ExecutionDetail(BaseModel):
    r"""
    执行记录详情
    """

    id: int = Field(description="执行记录ID")
    language: SupportedLanguage = Field(description="编程语言")
    code: str = Field(description="源代码")
    toml_output: str | None = Field(default=None, description="生成的TOML内容")
    status: ExecutionStatus = Field(description="执行状态")
    error_message: str | None = Field(default=None, description="错误信息")
    execution_time: int | None = Field(default=None, description="执行时间，单位毫秒")
    created_at: datetime = Field(description="执行时间")


class ExecutionListResponse(BaseModel):
    r"""
    执行历史分页响应
    """

    items: list[ExecutionHistoryItem] = Field(description="执行历史列表")
    total: int = Field(description="总数")
    page: int = Field(description="当前页码")
    limit: int = Field(description="每页数量")
