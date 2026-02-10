/**
 * 错误信息提取与用户友好消息映射
 *
 * @file src/utils/error.ts
 * @author WaterRun
 * @date 2026-02-10
 */

import type { ApiErrorBody } from '@/types/api'

/** 兜底默认错误文案 */
const DEFAULT_MESSAGE = '发生未知错误，请稍后重试'

/** HTTP 状态码对应的默认错误消息 */
const HTTP_STATUS_MESSAGES: Record<number, string> = {
    400: '请求参数有误',
    401: '未登录或登录已过期',
    403: '没有权限执行此操作',
    404: '请求的资源不存在',
    409: '数据冲突，请检查后重试',
    422: '提交的数据无法处理',
    429: '操作过于频繁，请稍后重试',
    500: '服务器内部错误',
    502: '网关错误，请稍后重试',
    503: '服务暂时不可用',
}

/**
 * 从任意错误对象中提取用户可读的消息文本
 *
 * 优先级：字符串 → ApiErrorBody → Error.message → 兜底文案
 *
 * @param error - 任意类型的错误对象
 * @returns 可展示给用户的错误消息
 *
 * @example
 * ```typescript
 * const message = extractErrorMessage(new Error('network timeout'))
 * // → 'network timeout'
 * ```
 */
export function extractErrorMessage(error: unknown): string {
    if (typeof error === 'string') {
        return error
    }

    if (isApiErrorBody(error)) {
        return error.error
    }

    if (error instanceof Error) {
        return error.message
    }

    return DEFAULT_MESSAGE
}

/**
 * 根据 HTTP 状态码返回对应的默认错误消息
 *
 * @param status - HTTP 状态码
 * @returns 可展示给用户的错误消息
 *
 * @example
 * ```typescript
 * const message = getHttpErrorMessage(404)
 * // → '请求的资源不存在'
 * ```
 */
export function getHttpErrorMessage(status: number): string {
    return HTTP_STATUS_MESSAGES[status] ?? DEFAULT_MESSAGE
}

/**
 * 获取兜底错误文案
 *
 * @returns 默认错误消息
 */
export function getDefaultErrorMessage(): string {
    return DEFAULT_MESSAGE
}

/**
 * 判断值是否符合 ApiErrorBody 结构
 *
 * @param value - 待检测值
 * @returns 是否为 ApiErrorBody
 */
function isApiErrorBody(value: unknown): value is ApiErrorBody {
    return (
        typeof value === 'object'
        && value !== null
        && 'error' in value
        && typeof (value as ApiErrorBody).error === 'string'
    )
}