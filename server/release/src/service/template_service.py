r"""
模板服务，提供模板查询功能

:file: src/service/template_service.py
:author: WaterRun
:time: 2026-01-28
"""

from database import Database
from model import (
    TemplateDetail,
    TemplateListResponse,
    TemplateListItem,
    TemplateCode,
    SupportedLanguage,
)


class TemplateServiceError(Exception):
    r"""
    模板服务异常基类
    """
    ...


class TemplateNotFoundError(TemplateServiceError):
    r"""
    模板不存在异常
    """
    ...


def get_template_by_id(template_id: int, user_id: int | None = None) -> TemplateDetail:
    r"""
    获取模板详情

    :param template_id: 模板ID
    :param user_id: 当前用户ID（用于判断是否已收藏，可选）
    :return TemplateDetail: 模板详情
    :raise TemplateNotFoundError: 模板不存在
    """
    with Database.get_connection() as conn:
        with conn.cursor() as cur:
            cur.execute(
                """
                SELECT id, title, category, description, favorite_count, created_at, updated_at
                FROM templates WHERE id = %s
                """,
                (template_id,),
            )
            row: tuple | None = cur.fetchone()

            if row is None:
                raise TemplateNotFoundError(f"模板不存在: {template_id}")

            t_id, title, category, description, favorite_count, created_at, updated_at = row

            cur.execute(
                """
                SELECT language, code, explanation
                FROM template_codes WHERE template_id = %s
                """,
                (template_id,),
            )
            code_rows: list[tuple] = cur.fetchall()

            codes: list[TemplateCode] = [
                TemplateCode(
                    language=SupportedLanguage(code_row[0]),
                    code=code_row[1],
                    explanation=code_row[2],
                )
                for code_row in code_rows
            ]

            is_favorited: bool = False
            if user_id is not None:
                cur.execute(
                    """
                    SELECT id FROM user_favorites
                    WHERE user_id = %s AND template_id = %s
                    """,
                    (user_id, template_id),
                )
                is_favorited = cur.fetchone() is not None

    return TemplateDetail(
        id=t_id,
        title=title,
        category=category,
        description=description,
        codes=codes,
        favorite_count=favorite_count,
        is_favorited=is_favorited,
        created_at=created_at,
        updated_at=updated_at,
    )


def list_templates(
    page: int = 1,
    limit: int = 20,
    category: str | None = None,
    user_id: int | None = None,
) -> TemplateListResponse:
    r"""
    获取模板列表

    :param page: 页码（从1开始）
    :param limit: 每页数量
    :param category: 分类筛选（可选）
    :param user_id: 当前用户ID（用于判断是否已收藏，可选）
    :return TemplateListResponse: 模板列表响应
    """
    offset: int = (page - 1) * limit

    with Database.get_connection() as conn:
        with conn.cursor() as cur:
            if category is not None:
                cur.execute(
                    """
                    SELECT COUNT(*) FROM templates WHERE category = %s
                    """,
                    (category,),
                )
            else:
                cur.execute("SELECT COUNT(*) FROM templates")

            total: int = cur.fetchone()[0]

            if category is not None:
                cur.execute(
                    """
                    SELECT id, title, category, description, favorite_count, created_at
                    FROM templates
                    WHERE category = %s
                    ORDER BY created_at DESC
                    LIMIT %s OFFSET %s
                    """,
                    (category, limit, offset),
                )
            else:
                cur.execute(
                    """
                    SELECT id, title, category, description, favorite_count, created_at
                    FROM templates
                    ORDER BY created_at DESC
                    LIMIT %s OFFSET %s
                    """,
                    (limit, offset),
                )

            rows: list[tuple] = cur.fetchall()
            template_ids: list[int] = [row[0] for row in rows]

            favorited_ids: set[int] = set()
            if user_id is not None and template_ids:
                placeholders: str = ",".join(["%s"] * len(template_ids))
                cur.execute(
                    f"""
                    SELECT template_id FROM user_favorites
                    WHERE user_id = %s AND template_id IN ({placeholders})
                    """,
                    (user_id, *template_ids),
                )
                favorited_ids = {r[0] for r in cur.fetchall()}

            items: list[TemplateListItem] = [
                TemplateListItem(
                    id=row[0],
                    title=row[1],
                    category=row[2],
                    description=row[3],
                    favorite_count=row[4],
                    is_favorited=row[0] in favorited_ids,
                    created_at=row[5],
                )
                for row in rows
            ]

    return TemplateListResponse(
        items=items,
        total=total,
        page=page,
        limit=limit,
    )


def get_categories() -> list[str]:
    r"""
    获取所有模板分类

    :return list[str]: 分类列表
    """
    with Database.get_connection() as conn:
        with conn.cursor() as cur:
            cur.execute(
                """
                SELECT DISTINCT category FROM templates ORDER BY category
                """
            )
            rows: list[tuple] = cur.fetchall()

    return [row[0] for row in rows]


def search_templates(
    keyword: str,
    page: int = 1,
    limit: int = 20,
    user_id: int | None = None,
) -> TemplateListResponse:
    r"""
    搜索模板

    :param keyword: 搜索关键词
    :param page: 页码（从1开始）
    :param limit: 每页数量
    :param user_id: 当前用户ID（用于判断是否已收藏，可选）
    :return TemplateListResponse: 搜索结果
    """
    offset: int = (page - 1) * limit
    search_pattern: str = f"%{keyword}%"

    with Database.get_connection() as conn:
        with conn.cursor() as cur:
            cur.execute(
                """
                SELECT COUNT(*) FROM templates
                WHERE title ILIKE %s OR description ILIKE %s
                """,
                (search_pattern, search_pattern),
            )
            total: int = cur.fetchone()[0]

            cur.execute(
                """
                SELECT id, title, category, description, favorite_count, created_at
                FROM templates
                WHERE title ILIKE %s OR description ILIKE %s
                ORDER BY favorite_count DESC, created_at DESC
                LIMIT %s OFFSET %s
                """,
                (search_pattern, search_pattern, limit, offset),
            )
            rows: list[tuple] = cur.fetchall()
            template_ids: list[int] = [row[0] for row in rows]

            favorited_ids: set[int] = set()
            if user_id is not None and template_ids:
                placeholders: str = ",".join(["%s"] * len(template_ids))
                cur.execute(
                    f"""
                    SELECT template_id FROM user_favorites
                    WHERE user_id = %s AND template_id IN ({placeholders})
                    """,
                    (user_id, *template_ids),
                )
                favorited_ids = {r[0] for r in cur.fetchall()}

            items: list[TemplateListItem] = [
                TemplateListItem(
                    id=row[0],
                    title=row[1],
                    category=row[2],
                    description=row[3],
                    favorite_count=row[4],
                    is_favorited=row[0] in favorited_ids,
                    created_at=row[5],
                )
                for row in rows
            ]

    return TemplateListResponse(
        items=items,
        total=total,
        page=page,
        limit=limit,
    )
