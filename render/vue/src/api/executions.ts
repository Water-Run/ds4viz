/**
 * 代码执行相关 API
 *
 * 对接后端 /api/execute 及 /api/executions/* 端点，
 * 负责 snake_case ↔ camelCase 映射。
 *
 * @file src/api/executions.ts
 * @author WaterRun
 * @date 2026-02-11
 */

import { get, post } from './http'
import type { Language } from '@/types/api'

/* ----------------------------------------------------------------
 *  后端 JSON 原始结构（snake_case）
 * ---------------------------------------------------------------- */

/**
 * 后端执行响应 JSON
 *
 * @interface
 */
interface RawExecuteResponse {
  /** 执行记录 ID */
  execution_id: number
  /** 状态 */
  status: string
  /** TOML 输出（成功时存在） */
  toml_output: string | null
  /** 错误消息（失败时存在） */
  error_message: string | null
  /** 执行耗时（毫秒） */
  execution_time: number | null
  /** 是否命中缓存 */
  cached: boolean
}

/**
 * 后端执行详情 JSON
 *
 * @interface
 */
interface RawExecutionDetail {
  /** 记录 ID */
  id: number
  /** 语言 */
  language: string
  /** 代码 */
  code: string
  /** TOML 输出 */
  toml_output: string | null
  /** 状态 */
  status: string
  /** 错误消息 */
  error_message: string | null
  /** 执行耗时（毫秒） */
  execution_time: number | null
  /** 创建时间 */
  created_at: string
}

/* ----------------------------------------------------------------
 *  前端类型
 * ---------------------------------------------------------------- */

/**
 * 执行响应
 *
 * @interface
 */
export interface ExecuteResult {
  /** 执行记录 ID */
  executionId: number
  /** 状态 */
  status: string
  /** TOML 输出（成功时存在） */
  tomlOutput: string | null
  /** 错误消息（失败时存在） */
  errorMessage: string | null
  /** 执行耗时（毫秒） */
  executionTime: number | null
  /** 是否命中缓存 */
  cached: boolean
}

/**
 * 执行详情
 *
 * @interface
 */
export interface ExecutionDetail {
  /** 记录 ID */
  id: number
  /** 语言 */
  language: string
  /** 代码 */
  code: string
  /** TOML 输出 */
  tomlOutput: string | null
  /** 状态 */
  status: string
  /** 错误消息 */
  errorMessage: string | null
  /** 执行耗时（毫秒） */
  executionTime: number | null
  /** 创建时间（ISO 8601） */
  createdAt: string
}

/* ----------------------------------------------------------------
 *  映射函数
 * ---------------------------------------------------------------- */

/**
 * 将后端执行响应映射为前端 ExecuteResult
 *
 * @param raw - 后端原始 JSON
 * @returns 前端 ExecuteResult 对象
 */
function mapExecuteResult(raw: RawExecuteResponse): ExecuteResult {
  return {
    executionId: raw.execution_id,
    status: raw.status,
    tomlOutput: raw.toml_output,
    errorMessage: raw.error_message,
    executionTime: raw.execution_time,
    cached: raw.cached,
  }
}

/**
 * 将后端执行详情映射为前端 ExecutionDetail
 *
 * @param raw - 后端原始 JSON
 * @returns 前端 ExecutionDetail 对象
 */
function mapExecutionDetail(raw: RawExecutionDetail): ExecutionDetail {
  return {
    id: raw.id,
    language: raw.language,
    code: raw.code,
    tomlOutput: raw.toml_output,
    status: raw.status,
    errorMessage: raw.error_message,
    executionTime: raw.execution_time,
    createdAt: raw.created_at,
  }
}

/* ----------------------------------------------------------------
 *  API 函数
 * ---------------------------------------------------------------- */

/**
 * 执行代码
 *
 * @param language - 编程语言
 * @param code - 源代码
 * @returns 执行结果
 * @throws {ApiError} 401 未认证 / 403 用户被暂停或封禁 / 422 不支持的语言
 */
export async function executeCodeApi(
  language: Language,
  code: string,
): Promise<ExecuteResult> {
  const raw = await post<RawExecuteResponse>('/execute', { language, code })
  return mapExecuteResult(raw)
}

/**
 * 获取执行详情
 *
 * @param id - 执行记录 ID
 * @returns 执行详情
 * @throws {ApiError} 401 未认证 / 403 无权访问 / 404 记录不存在
 */
export async function fetchExecutionDetailApi(
  id: number,
): Promise<ExecutionDetail> {
  const raw = await get<RawExecutionDetail>(`/executions/${id}`)
  return mapExecutionDetail(raw)
}
