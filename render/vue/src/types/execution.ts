/**
 * 代码执行相关类型定义
 *
 * @file src/types/execution.ts
 * @author WaterRun
 * @date 2026-02-10
 */

import type { Language } from './api'

/**
 * 执行状态
 *
 * @typedef
 */
export type ExecutionStatus =
    | 'pending'
    | 'running'
    | 'success'
    | 'error'
    | 'timeout'

/** 状态对应的显示文案 */
export const EXECUTION_STATUS_LABELS: Record<ExecutionStatus, string> = {
    pending: '等待中',
    running: '运行中',
    success: '成功',
    error: '错误',
    timeout: '超时',
}

/**
 * 执行记录
 *
 * @interface
 */
export interface Execution {
    /** 记录 ID */
    id: number
    /** 关联的模板 ID（无模板时为 null） */
    templateId: number | null
    /** 执行语言 */
    language: Language
    /** 源代码 */
    code: string
    /** 执行状态 */
    status: ExecutionStatus
    /** 标准输出 */
    output: string | null
    /** 错误信息 */
    error: string | null
    /** 执行耗时（毫秒） */
    durationMs: number | null
    /** 创建时间（ISO 8601） */
    createdAt: string
}

/**
 * 提交执行请求
 *
 * @interface
 */
export interface ExecuteRequest {
    /** 源代码 */
    code: string
    /** 语言 */
    language: Language
}

/**
 * 提交执行响应
 *
 * @interface
 */
export interface ExecuteResponse {
    /** 记录 ID */
    id: number
    /** 执行状态 */
    status: ExecutionStatus
    /** 标准输出 */
    output: string | null
    /** 错误信息 */
    error: string | null
    /** 执行耗时（毫秒） */
    durationMs: number | null
}