r"""
模板相关模型定义

:file: src/model/template.py
:author: WaterRun
:time: 2026-01-29
"""

from datetime import datetime

from pydantic import BaseModel, Field


class TemplateCodeCreate(BaseModel):
    r"""
    模板代码创建请求体，用于管理员创建模板
    """

    language: str = Field(description="编程语言")
    code: str = Field(description="代码内容")
    explanation: str | None = Field(
        default=None, description="代码说明，Markdown格式")


class TemplateCodeResponse(BaseModel):
    r"""
    模板代码响应
    """

    language: str = Field(description="编程语言")
    code: str = Field(description="代码内容")
    explanation: str | None = Field(
        default=None, description="代码说明，Markdown格式")


class TemplateCreate(BaseModel):
    r"""
    模板创建请求体，用于管理员创建模板
    """

    title: str = Field(max_length=256, description="模板标题")
    category: str = Field(max_length=64, description="模板分类")
    description: str = Field(description="模板描述，Markdown格式")
    codes: list[TemplateCodeCreate] = Field(description="多语言代码实现列表")


class TemplateUpdate(BaseModel):
    r"""
    模板更新请求体，用于管理员更新模板
    """

    title: str | None = Field(default=None, max_length=256, description="模板标题")
    category: str | None = Field(
        default=None, max_length=64, description="模板分类")
    description: str | None = Field(
        default=None, description="模板描述，Markdown格式")


class TemplateListItem(BaseModel):
    r"""
    模板列表项，不包含完整代码
    """

    id: int = Field(description="模板ID")
    title: str = Field(description="模板标题")
    category: str = Field(description="模板分类")
    description: str = Field(description="模板描述，Markdown格式")
    favorite_count: int = Field(description="收藏数")
    is_favorited: bool = Field(default=False, description="当前用户是否已收藏")
    created_at: datetime = Field(description="创建时间")


class TemplateDetail(BaseModel):
    r"""
    模板详情，包含所有语言的代码实现
    """

    id: int = Field(description="模板ID")
    title: str = Field(description="模板标题")
    category: str = Field(description="模板分类")
    description: str = Field(description="模板描述，Markdown格式")
    favorite_count: int = Field(description="收藏数")
    codes: list[TemplateCodeResponse] = Field(description="多语言代码实现列表")
    is_favorited: bool = Field(default=False, description="当前用户是否已收藏")
    created_at: datetime = Field(description="创建时间")
    updated_at: datetime = Field(description="更新时间")


class TemplateListResponse(BaseModel):
    r"""
    模板列表分页响应
    """

    items: list[TemplateListItem] = Field(description="模板列表")
    total: int = Field(description="总数")
    page: int = Field(description="当前页码")
    limit: int = Field(description="每页数量")
