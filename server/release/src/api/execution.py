r"""
执行与收藏相关API

:file: src/api/execution.py
:author: WaterRun
:time: 2026-01-29
"""

from fastapi import APIRouter, Depends, Request
from pydantic import BaseModel, Field

from api import get_current_user
from model import ExecuteRequest, ExecuteResponse, ExecutionDetail
from service import execute_code, get_execution_by_id, add_favorite, remove_favorite
from utils import get_client_ip

execute_router = APIRouter(tags=["执行"])
favorites_router = APIRouter(prefix="/api/favorites", tags=["收藏"])
executions_router = APIRouter(prefix="/api/executions", tags=["执行记录"])


class FavoriteRequest(BaseModel):
    r"""
    收藏请求体
    """

    template_id: int = Field(description="模板ID")


@execute_router.post("/api/execute", response_model=ExecuteResponse)
def execute(
    data: ExecuteRequest,
    request: Request,
    user_id: int = Depends(get_current_user),
) -> ExecuteResponse:
    r"""
    执行代码

    :param data: 执行请求
    :param request: FastAPI请求对象
    :param user_id: 当前用户ID
    :return ExecuteResponse: 执行响应
    """
    ip_address: str = get_client_ip(request)
    return execute_code(user_id, data, ip_address)


@executions_router.get("/{execution_id}", response_model=ExecutionDetail)
def get_execution(
    execution_id: int,
    user_id: int = Depends(get_current_user),
) -> ExecutionDetail:
    r"""
    获取执行详情

    :param execution_id: 执行记录ID
    :param user_id: 当前用户ID
    :return ExecutionDetail: 执行详情
    """
    return get_execution_by_id(execution_id, user_id)


@favorites_router.post("")
def create_favorite(
    data: FavoriteRequest,
    user_id: int = Depends(get_current_user),
) -> dict[str, str]:
    r"""
    收藏模板

    :param data: 收藏请求
    :param user_id: 当前用户ID
    :return dict: 收藏成功消息
    """
    add_favorite(user_id, data.template_id)
    return {"message": "Template favorited successfully"}


@favorites_router.delete("/{template_id}")
def delete_favorite(
    template_id: int,
    user_id: int = Depends(get_current_user),
) -> dict[str, str]:
    r"""
    取消收藏

    :param template_id: 模板ID
    :param user_id: 当前用户ID
    :return dict: 取消成功消息
    """
    remove_favorite(user_id, template_id)
    return {"message": "Favorite removed successfully"}
