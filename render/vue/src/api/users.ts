/**
 * 用户相关 API
 *
 * 对接后端 /api/users/* 端点，负责 snake_case ↔ camelCase 映射。
 *
 * @file src/api/users.ts
 * @author WaterRun
 * @date 2026-02-11
 */

import { get, put } from './http'
import type { PaginatedResult, PaginationParams } from '@/types/api'
import type { ChangePasswordRequest } from '@/types/auth'

/* ----------------------------------------------------------------
 *  后端 JSON 原始结构（snake_case）
 * ---------------------------------------------------------------- */

/**
 * 后端收藏列表项 JSON
 *
 * @interface
 */
interface RawFavoriteItem {
  /** 模板 ID */
  template_id: number
  /** 标题 */
  title: string
  /** 分类 */
  category: string
  /** 描述 */
  description: string
  /** 收藏数 */
  favorite_count: number
  /** 收藏时间 */
  favorited_at: string
}

/**
 * 后端执行历史项 JSON
 *
 * @interface
 */
interface RawExecutionItem {
  /** 记录 ID */
  id: number
  /** 语言 */
  language: string
  /** 代码 */
  code: string
  /** 状态 */
  status: string
  /** 执行耗时（毫秒，可为 null） */
  execution_time: number | null
  /** 创建时间 */
  created_at: string
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

/* ----------------------------------------------------------------
 *  前端类型
 * ---------------------------------------------------------------- */

/**
 * 收藏列表项
 *
 * @interface
 */
export interface FavoriteItem {
  /** 模板 ID */
  templateId: number
  /** 标题 */
  title: string
  /** 分类 */
  category: string
  /** 描述 */
  description: string
  /** 收藏数 */
  favoriteCount: number
  /** 收藏时间（ISO 8601） */
  favoritedAt: string
}

/**
 * 执行历史项
 *
 * @interface
 */
export interface ExecutionHistoryItem {
  /** 记录 ID */
  id: number
  /** 语言 */
  language: string
  /** 代码 */
  code: string
  /** 状态 */
  status: string
  /** 执行耗时（毫秒，可为 null） */
  executionTime: number | null
  /** 创建时间（ISO 8601） */
  createdAt: string
}

/* ----------------------------------------------------------------
 *  映射函数
 * ---------------------------------------------------------------- */

/**
 * 将后端收藏列表项映射为前端 FavoriteItem
 *
 * @param raw - 后端原始 JSON
 * @returns 前端 FavoriteItem 对象
 */
function mapFavoriteItem(raw: RawFavoriteItem): FavoriteItem {
  return {
    templateId: raw.template_id,
    title: raw.title,
    category: raw.category,
    description: raw.description,
    favoriteCount: raw.favorite_count,
    favoritedAt: raw.favorited_at,
  }
}

/**
 * 将后端执行历史项映射为前端 ExecutionHistoryItem
 *
 * @param raw - 后端原始 JSON
 * @returns 前端 ExecutionHistoryItem 对象
 */
function mapExecutionHistoryItem(raw: RawExecutionItem): ExecutionHistoryItem {
  return {
    id: raw.id,
    language: raw.language,
    code: raw.code,
    status: raw.status,
    executionTime: raw.execution_time,
    createdAt: raw.created_at,
  }
}

/* ----------------------------------------------------------------
 *  API 函数
 * ---------------------------------------------------------------- */

/**
 * 获取用户头像 URL
 *
 * 不发请求，仅拼接路径。头像通过 img src 直接加载。
 *
 * @param userId - 用户 ID
 * @returns 头像 URL 字符串
 */
export function getUserAvatarUrl(userId: number): string {
  const base =
    (import.meta.env.VITE_API_BASE_URL as string | undefined) ?? '/api'
  return `${base}/users/${userId}/avatar`
}

/**
 * 上传/更新用户头像
 *
 * @param userId - 用户 ID
 * @param file - 头像文件
 * @returns 上传结果（含新 avatar_url）
 * @throws {ApiError} 401 未认证 / 403 权限不足 / 404 用户不存在
 */
export async function uploadAvatarApi(
  userId: number,
  file: File,
): Promise<{ message: string; avatarUrl: string }> {
  const formData = new FormData()
  formData.append('avatar', file)

  const base =
    (import.meta.env.VITE_API_BASE_URL as string | undefined) ?? '/api'

  const { getToken } = await import('@/utils/storage')
  const headers = new Headers()
  const token = getToken()
  if (token !== null) {
    headers.set('Authorization', `Bearer ${token}`)
  }

  const response = await fetch(`${base}/users/${userId}/avatar`, {
    method: 'PUT',
    headers,
    body: formData,
  })

  if (!response.ok) {
    const { ApiError } = await import('./http')
    const { getHttpErrorMessage } = await import('@/utils/error')
    let message: string
    try {
      const body: unknown = await response.json()
      if (
        typeof body === 'object'
        && body !== null
        && 'error' in body
        && typeof (body as { error: unknown }).error === 'string'
      ) {
        message = (body as { error: string }).error
      } else {
        message = getHttpErrorMessage(response.status)
      }
    } catch {
      message = getHttpErrorMessage(response.status)
    }
    throw new ApiError(response.status, message)
  }

  const raw = (await response.json()) as { message: string; avatar_url: string }
  return {
    message: raw.message,
    avatarUrl: raw.avatar_url,
  }
}

/**
 * 修改密码
 *
 * @param userId - 用户 ID
 * @param request - 旧密码与新密码
 * @throws {ApiError} 400 原密码错误 / 401 未认证 / 403 权限不足 / 404 用户不存在
 */
export async function changePasswordApi(
  userId: number,
  request: ChangePasswordRequest,
): Promise<void> {
  await put<{ message: string }>(`/users/${userId}/password`, {
    old_password: request.oldPassword,
    new_password: request.newPassword,
  })
}

/**
 * 获取用户收藏列表
 *
 * @param userId - 用户 ID
 * @param params - 分页参数
 * @returns 分页收藏列表
 * @throws {ApiError} 401 未认证 / 403 权限不足 / 404 用户不存在
 */
export async function fetchFavoritesApi(
  userId: number,
  params?: PaginationParams,
): Promise<PaginatedResult<FavoriteItem>> {
  const raw = await get<RawPaginated<RawFavoriteItem>>(
    `/users/${userId}/favorites`,
    {
      page: params?.page,
      limit: params?.limit,
    },
  )
  return {
    items: raw.items.map(mapFavoriteItem),
    total: raw.total,
    page: raw.page,
    limit: raw.limit,
  }
}

/**
 * 获取用户执行历史
 *
 * @param userId - 用户 ID
 * @param params - 分页参数
 * @returns 分页执行历史列表
 * @throws {ApiError} 401 未认证 / 403 权限不足
 */
export async function fetchExecutionHistoryApi(
  userId: number,
  params?: PaginationParams,
): Promise<PaginatedResult<ExecutionHistoryItem>> {
  const raw = await get<RawPaginated<RawExecutionItem>>(
    `/users/${userId}/executions`,
    {
      page: params?.page,
      limit: params?.limit,
    },
  )
  return {
    items: raw.items.map(mapExecutionHistoryItem),
    total: raw.total,
    page: raw.page,
    limit: raw.limit,
  }
}
