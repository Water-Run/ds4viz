r"""
模型包，定义API请求/响应的数据结构

:file: src/model/__init__.py
:author: WaterRun
:time: 2026-01-28
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
    TemplateCodeCreate,
    TemplateCodeResponse,
    TemplateCreate,
    TemplateUpdate,
    TemplateListItem,
    TemplateDetail,
    TemplateListResponse,
)
from model.execution import (
    ExecutionStatus,
    SupportedLanguage,
    ExecuteRequest,
    ExecuteResponse,
    ExecutionHistoryItem,
    ExecutionDetail,
    ExecutionListResponse,
)

__all__ = [
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
    "TemplateCodeCreate",
    "TemplateCodeResponse",
    "TemplateCreate",
    "TemplateUpdate",
    "TemplateListItem",
    "TemplateDetail",
    "TemplateListResponse",
    # execution
    "ExecutionStatus",
    "SupportedLanguage",
    "ExecuteRequest",
    "ExecuteResponse",
    "ExecutionHistoryItem",
    "ExecutionDetail",
    "ExecutionListResponse",
]
