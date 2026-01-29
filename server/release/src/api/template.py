r"""
模板相关API

:file: src/api/template.py
:author: WaterRun
:time: 2026-01-29
"""

from fastapi import APIRouter, Depends, Query, Request

from api import get_optional_user
from model import TemplateDetail, TemplateListResponse, TemplateCodeResponse
from service import (
    get_template_by_id,
    get_template_code,
    list_templates,
    get_categories,
    search_templates,
)

router = APIRouter(prefix="/api/templates", tags=["模板"])


@router.get("", response_model=TemplateListResponse)
def get_templates(
    category: str | None = Query(default=None),
    page: int = Query(default=1, ge=1),
    limit: int = Query(default=20, ge=1, le=100),
    user_id: int | None = Depends(get_optional_user),
) -> TemplateListResponse:
    r"""
    获取模板列表

    :param category: 分类筛选
    :param page: 页码
    :param limit: 每页数量
    :param user_id: 当前用户ID（可选）
    :return TemplateListResponse: 模板列表
    """
    return list_templates(page=page, limit=limit, category=category, user_id=user_id)


@router.get("/categories")
def get_template_categories() -> dict[str, list[str]]:
    r"""
    获取模板分类列表

    :return dict: 分类列表
    """
    categories: list[str] = get_categories()
    return {"items": categories}


@router.get("/search", response_model=TemplateListResponse)
def search(
    keyword: str = Query(...),
    page: int = Query(default=1, ge=1),
    limit: int = Query(default=20, ge=1, le=100),
    user_id: int | None = Depends(get_optional_user),
) -> TemplateListResponse:
    r"""
    搜索模板

    :param keyword: 搜索关键词
    :param page: 页码
    :param limit: 每页数量
    :param user_id: 当前用户ID（可选）
    :return TemplateListResponse: 搜索结果
    """
    return search_templates(keyword=keyword, page=page, limit=limit, user_id=user_id)


@router.get("/{template_id}", response_model=TemplateDetail)
def get_template(
    template_id: int,
    user_id: int | None = Depends(get_optional_user),
) -> TemplateDetail:
    r"""
    获取模板详情

    :param template_id: 模板ID
    :param user_id: 当前用户ID（可选）
    :return TemplateDetail: 模板详情
    """
    return get_template_by_id(template_id, user_id)


@router.get("/{template_id}/code/{language}", response_model=TemplateCodeResponse)
def get_template_code_by_language(
    template_id: int,
    language: str,
) -> TemplateCodeResponse:
    r"""
    获取模板指定语言的代码

    :param template_id: 模板ID
    :param language: 编程语言
    :return TemplateCodeResponse: 模板代码
    """
    return get_template_code(template_id, language)
