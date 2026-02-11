/**
 * 模板相关类型定义
 *
 * 与后端 /api/templates/* 接口一一对应，字段名使用 camelCase，
 * API 层负责 snake_case ↔ camelCase 的双向映射。
 *
 * @file src/types/template.ts
 * @author WaterRun
 * @date 2026-02-11
 */

import type { PaginationParams } from './api'

/**
 * 模板关联的单语言代码
 *
 * @interface
 */
export interface TemplateCode {
    /** 语言标识 */
    language: string
    /** 代码内容 */
    code: string
    /** 说明（可为 null） */
    explanation: string | null
}

/**
 * 模板完整信息（详情页）
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
    /** 收藏数 */
    favoriteCount: number
    /** 当前用户是否已收藏 */
    isFavorited: boolean
    /** 创建时间（ISO 8601） */
    createdAt: string
}

/**
 * 模板搜索 / 筛选参数
 *
 * @interface
 */
export interface TemplateSearchParams extends PaginationParams {
    /** 搜索关键词 */
    keyword?: string
    /** 分类筛选 */
    category?: string
}