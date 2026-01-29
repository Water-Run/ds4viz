r"""
模板服务，提供模板查询功能

:file: src/service/template_service.py
:author: WaterRun
:time: 2026-01-29
"""

from psycopg import sql

from database import get_connection
from exceptions import TemplateNotFoundError, TemplateCodeNotFoundError
from model import (
    TemplateDetail,
    TemplateListResponse,
    TemplateListItem,
    TemplateCodeResponse,
)


def get_template_by_id(template_id: int, user_id: int | None = None) -> TemplateDetail:
    r"""
    获取模板详情

    :param template_id: 模板ID
    :param user_id: 当前用户ID（用于判断是否已收藏，可选）
    :return TemplateDetail: 模板详情
    :raise TemplateNotFoundError: 模板不存在
    """
    with get_connection() as conn:
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

            codes: list[TemplateCodeResponse] = [
                TemplateCodeResponse(
                    language=code_row[0],
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


def get_template_code(template_id: int, language: str) -> TemplateCodeResponse:
    r"""
    获取指定模板的指定语言代码

    :param template_id: 模板ID
    :param language: 编程语言
    :return TemplateCodeResponse: 模板代码响应
    :raise TemplateNotFoundError: 模板不存在
    :raise TemplateCodeNotFoundError: 模板语言实现不存在
    """
    with get_connection() as conn:
        with conn.cursor() as cur:
            cur.execute(
                """
                SELECT id FROM templates WHERE id = %s
                """,
                (template_id,),
            )
            if cur.fetchone() is None:
                raise TemplateNotFoundError(f"模板不存在: {template_id}")

            cur.execute(
                """
                SELECT language, code, explanation
                FROM template_codes
                WHERE template_id = %s AND language = %s
                """,
                (template_id, language),
            )
            row: tuple | None = cur.fetchone()

            if row is None:
                raise TemplateCodeNotFoundError(
                    f"模板语言实现不存在: template_id={template_id}, language={language}"
                )

            return TemplateCodeResponse(
                language=row[0],
                code=row[1],
                explanation=row[2],
            )


def _build_in_clause(column: str, values: list[int]) -> tuple[sql.Composed, list[int]]:
    r"""
    构建安全的IN子句

    :param column: 列名
    :param values: 值列表
    :return tuple[sql.Composed, list[int]]: (SQL片段, 参数列表)
    """
    placeholders = sql.SQL(", ").join(sql.Placeholder() for _ in values)
    clause = sql.SQL("{} IN ({})").format(sql.Identifier(column), placeholders)
    return clause, values


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

    with get_connection() as conn:
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
                in_clause, in_params = _build_in_clause(
                    "template_id", template_ids)
                query = sql.SQL(
                    "SELECT template_id FROM user_favorites WHERE user_id = %s AND {}"
                ).format(in_clause)
                cur.execute(query, [user_id, *in_params])
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
    with get_connection() as conn:
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

    with get_connection() as conn:
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
                in_clause, in_params = _build_in_clause(
                    "template_id", template_ids)
                query = sql.SQL(
                    "SELECT template_id FROM user_favorites WHERE user_id = %s AND {}"
                ).format(in_clause)
                cur.execute(query, [user_id, *in_params])
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
