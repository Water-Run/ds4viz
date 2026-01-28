r"""
数据模型模块，定义所有请求响应数据结构

:file: src/model/__init__.py
:author: WaterRun
:time: 2026-01-28
"""

from model.common import SupportedLanguage
from model.user import (
    UserStatus,
    UserCreate,
    UserLogin,
    UserResponse,
    PasswordChange,
    FavoriteItem,
    FavoriteListResponse,
)
from model.template import (
    TemplateCode,
    TemplateDetail,
    TemplateListItem,
    TemplateListResponse,
)
from model.execution import (
    ExecutionStatus,
    ExecuteRequest,
    ExecuteResponse,
    ExecutionDetail,
    ExecutionListItem,
    ExecutionListResponse,
)

__all__: list[str] = [
    # common
    "SupportedLanguage",
    # user
    "UserStatus",
    "UserCreate",
    "UserLogin",
    "UserResponse",
    "PasswordChange",
    "FavoriteItem",
    "FavoriteListResponse",
    # template
    "TemplateCode",
    "TemplateDetail",
    "TemplateListItem",
    "TemplateListResponse",
    # execution
    "ExecutionStatus",
    "ExecuteRequest",
    "ExecuteResponse",
    "ExecutionDetail",
    "ExecutionListItem",
    "ExecutionListResponse",
]
