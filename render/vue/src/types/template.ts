/**
 * 模板相关类型定义
 *
 * @file src/types/template.ts
 * @author WaterRun
 * @date 2026-02-10
 */

import type { Language, PaginationParams } from './api'

/**
 * 模板关联的单语言代码
 *
 * @interface
 */
export interface TemplateCode {
    /** 语言 */
    language: Language
    /** 代码内容 */
    code: string
}

/**
 * 模板完整信息
 *
 * @interface
 */
export interface Template {
    /** 模板 ID */
    id: number
    /** 标题 */
    title: string
    /** 描述 */
    description: string
    /** 分类 */
    category: string
    /** 标签 */
    tags: string[]
    /** 收藏数 */
    favoriteCount: number
    /** 当前用户是否已收藏 */
    isFavorited: boolean
    /** 各语言代码 */
    codes: TemplateCode[]
    /** 创建时间（ISO 8601） */
    createdAt: string
    /** 更新时间（ISO 8601） */
    updatedAt: string
}

/**
 * 模板摘要信息（列表页）
 *
 * @interface
 */
export interface TemplateListItem {
    /** 模板 ID */
    id: number
    /** 标题 */
    title: string
    /** 描述 */
    description: string
    /** 分类 */
    category: string
    /** 标签 */
    tags: string[]
    /** 收藏数 */
    favoriteCount: number
    /** 当前用户是否已收藏 */
    isFavorited: boolean
}

/**
 * 模板分类
 *
 * @interface
 */
export interface Category {
    /** 分类名称 */
    name: string
    /** 该分类下模板数量 */
    count: number
}

/**
 * 模板搜索 / 筛选参数
 *
 * @interface
 */
export interface TemplateSearchParams extends PaginationParams {
    /** 关键词 */
    keyword?: string
    /** 分类 */
    category?: string
}