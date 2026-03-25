/**
 * API 通用类型定义
 *
 * @file src/types/api.ts
 * @author WaterRun
 * @date 2026-03-25
 */

/**
 * 支持的编程语言
 *
 * @typedef
 */
export type Language = 'python' | 'c'

/** 语言显示名称映射 */
export const LANGUAGE_LABELS: Record<Language, string> = {
  python: 'Python',
  c: 'C',
}

/** 全部可用语言列表 */
export const LANGUAGES: Language[] = ['python', 'c']

/**
 * 通用分页请求参数
 *
 * @interface
 */
export interface PaginationParams {
  /** 页码（从 1 起） */
  page?: number
  /** 每页条数 */
  limit?: number
}

/**
 * 通用分页响应结构
 *
 * @interface
 * @template T - 数据项类型
 */
export interface PaginatedResult<T> {
  /** 当前页数据 */
  items: T[]
  /** 总条数 */
  total: number
  /** 当前页码 */
  page: number
  /** 每页条数 */
  limit: number
}

/**
 * 后端统一错误响应体
 *
 * @interface
 */
export interface ApiErrorBody {
  /** 错误消息 */
  error: string
}