r"""
数据模型模块，定义所有请求响应数据结构

:file: src/model/__init__.py
:author: WaterRun
:time: 2026-01-29
"""

from model.user import (
    UserStatus,
    UserCreate,
    UserLogin,
    UserResponse,
    UserWithToken,
    PasswordChange,
    FavoriteItem,
    FavoriteListResponse,
)
from model.template import (
    TemplateCodeResponse,
    TemplateDetail,
    TemplateListItem,
    TemplateListResponse,
)
from model.execution import (
    SupportedLanguage,
    ExecutionStatus,
    ExecuteRequest,
    ExecuteResponse,
    ExecutionDetail,
    ExecutionHistoryItem,
    ExecutionListResponse,
)

__all__: list[str] = [
    # user
    "UserStatus",
    "UserCreate",
    "UserLogin",
    "UserResponse",
    "UserWithToken",
    "PasswordChange",
    "FavoriteItem",
    "FavoriteListResponse",
    # template
    "TemplateCodeResponse",
    "TemplateDetail",
    "TemplateListItem",
    "TemplateListResponse",
    # execution
    "SupportedLanguage",
    "ExecutionStatus",
    "ExecuteRequest",
    "ExecuteResponse",
    "ExecutionDetail",
    "ExecutionHistoryItem",
    "ExecutionListResponse",
]
