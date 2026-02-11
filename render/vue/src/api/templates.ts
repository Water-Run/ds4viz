/**
 * 模板相关 API
 *
 * 对接后端 /api/templates/* 及 /api/favorites/* 端点，
 * 负责 snake_case ↔ camelCase 映射。
 *
 * @file src/api/templates.ts
 * @author WaterRun
 * @date 2026-02-11
 */

import { get, post, del } from './http'
import type { PaginatedResult } from '@/types/api'
import type {
    TemplateListItem,
    Template,
    TemplateCode,
    TemplateSearchParams,
} from '@/types/template'

/* ----------------------------------------------------------------
 *  后端 JSON 原始结构（snake_case）
 * ---------------------------------------------------------------- */

/**
 * 后端模板列表项 JSON
 *
 * @interface
 */
interface RawTemplateListItem {
    /** 模板 ID */
    id: number
    /** 标题 */
    title: string
    /** 分类 */
    category: string
    /** 描述 */
    description: string
    /** 收藏数 */
    favorite_count: number
    /** 当前用户是否已收藏 */
    is_favorited: boolean
    /** 创建时间 */
    created_at: string
}

/**
 * 后端模板代码 JSON
 *
 * @interface
 */
interface RawTemplateCode {
    /** 语言 */
    language: string
    /** 代码内容 */
    code: string
    /** 说明（可为 null） */
    explanation: string | null
}

/**
 * 后端模板详情 JSON
 *
 * @interface
 */
interface RawTemplate {
    /** 模板 ID */
    id: number
    /** 标题 */
    title: string
    /** 分类 */
    category: string
    /** 描述 */
    description: string
    /** 收藏数 */
    favorite_count: number
    /** 各语言代码 */
    codes: RawTemplateCode[]
    /** 当前用户是否已收藏 */
    is_favorited: boolean
    /** 创建时间 */
    created_at: string
    /** 更新时间 */
    updated_at: string
}

/**
 * 后端分页响应 JSON
 *
 * @interface
 */
interface RawPaginated<T> {
    /** 数据列表 */
    items: T[]
    /** 总条数 */
    total: number
    /** 当前页码 */
    page: number
    /** 每页条数 */
    limit: number
}

/**
 * 后端分类列表响应 JSON
 *
 * @interface
 */
interface RawCategoriesResponse {
    /** 分类名称列表 */
    items: string[]
}

/* ----------------------------------------------------------------
 *  映射函数
 * ---------------------------------------------------------------- */

/**
 * 将后端模板列表项映射为前端 TemplateListItem
 *
 * @param raw - 后端原始 JSON
 * @returns 前端 TemplateListItem 对象
 */
function mapTemplateListItem(raw: RawTemplateListItem): TemplateListItem {
    return {
        id: raw.id,
        title: raw.title,
        description: raw.description,
        category: raw.category,
        favoriteCount: raw.favorite_count,
        isFavorited: raw.is_favorited,
        createdAt: raw.created_at,
    }
}

/**
 * 将后端模板代码映射为前端 TemplateCode
 *
 * @param raw - 后端原始 JSON
 * @returns 前端 TemplateCode 对象
 */
function mapTemplateCode(raw: RawTemplateCode): TemplateCode {
    return {
        language: raw.language,
        code: raw.code,
        explanation: raw.explanation,
    }
}

/**
 * 将后端模板详情映射为前端 Template
 *
 * @param raw - 后端原始 JSON
 * @returns 前端 Template 对象
 */
function mapTemplate(raw: RawTemplate): Template {
    return {
        id: raw.id,
        title: raw.title,
        description: raw.description,
        category: raw.category,
        favoriteCount: raw.favorite_count,
        isFavorited: raw.is_favorited,
        codes: raw.codes.map(mapTemplateCode),
        createdAt: raw.created_at,
        updatedAt: raw.updated_at,
    }
}

/* ----------------------------------------------------------------
 *  API 函数
 * ---------------------------------------------------------------- */

/**
 * 获取模板列表（支持分类筛选与分页）
 *
 * @param params - 分类、分页参数
 * @returns 分页模板列表
 */
export async function fetchTemplatesApi(
    params?: TemplateSearchParams,
): Promise<PaginatedResult<TemplateListItem>> {
    const raw = await get<RawPaginated<RawTemplateListItem>>(
        '/templates',
        {
            category: params?.category,
            page: params?.page,
            limit: params?.limit,
        },
    )
    return {
        items: raw.items.map(mapTemplateListItem),
        total: raw.total,
        page: raw.page,
        limit: raw.limit,
    }
}

/**
 * 搜索模板
 *
 * @param params - 必须包含 keyword，可选分页
 * @returns 搜索结果分页列表
 * @throws {ApiError} 422 缺少 keyword
 */
export async function searchTemplatesApi(
    params: TemplateSearchParams,
): Promise<PaginatedResult<TemplateListItem>> {
    const raw = await get<RawPaginated<RawTemplateListItem>>(
        '/templates/search',
        {
            keyword: params.keyword,
            page: params.page,
            limit: params.limit,
        },
    )
    return {
        items: raw.items.map(mapTemplateListItem),
        total: raw.total,
        page: raw.page,
        limit: raw.limit,
    }
}

/**
 * 获取模板详情
 *
 * @param id - 模板 ID
 * @returns 模板完整信息
 * @throws {ApiError} 404 模板不存在
 */
export async function fetchTemplateDetailApi(
    id: number,
): Promise<Template> {
    const raw = await get<RawTemplate>(`/templates/${id}`)
    return mapTemplate(raw)
}

/**
 * 获取模板代码（指定语言）
 *
 * @param id - 模板 ID
 * @param language - 语言标识
 * @returns 对应语言的代码
 * @throws {ApiError} 404 模板或语言实现不存在
 */
export async function fetchTemplateCodeApi(
    id: number,
    language: string,
): Promise<TemplateCode> {
    const raw = await get<RawTemplateCode>(`/templates/${id}/code/${language}`)
    return mapTemplateCode(raw)
}

/**
 * 获取模板分类列表
 *
 * @returns 去重且按字母排序的分类名称数组
 */
export async function fetchCategoriesApi(): Promise<string[]> {
    const raw = await get<RawCategoriesResponse>('/templates/categories')
    return raw.items
}

/**
 * 收藏模板
 *
 * @param templateId - 模板 ID
 * @throws {ApiError} 403 用户被暂停或封禁 / 404 模板不存在 / 409 已收藏
 */
export async function favoriteTemplateApi(templateId: number): Promise<void> {
    await post<{ message: string }>('/favorites', { template_id: templateId })
}

/**
 * 取消收藏模板
 *
 * @param templateId - 模板 ID
 * @throws {ApiError} 404 未收藏该模板
 */
export async function unfavoriteTemplateApi(templateId: number): Promise<void> {
    await del<{ message: string }>(`/favorites/${templateId}`)
}