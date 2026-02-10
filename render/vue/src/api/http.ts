/**
 * HTTP 请求统一封装
 *
 * 基于 Fetch API，提供自动认证注入、统一错误拦截与 JSON 解析。
 * 所有业务 API 模块均通过本模块发起请求。
 *
 * @file src/api/http.ts
 * @author WaterRun
 * @date 2026-02-10
 */

import { getToken, removeToken } from '@/utils/storage'
import { getHttpErrorMessage } from '@/utils/error'

/**
 * API 基础路径
 *
 * 优先读取环境变量 VITE_API_BASE_URL，缺省为 /api。
 * 开发环境可通过 Vite server.proxy 将 /api 代理至后端。
 */
const BASE_URL: string =
    (import.meta.env.VITE_API_BASE_URL as string | undefined) ?? '/api'

/**
 * API 请求错误
 *
 * 携带 HTTP 状态码与后端返回的错误消息。
 */
export class ApiError extends Error {
    /** HTTP 状态码 */
    readonly status: number

    /**
     * 构造 API 错误
     *
     * @param status - HTTP 状态码
     * @param message - 可读的错误消息
     */
    constructor(status: number, message: string) {
        super(message)
        this.name = 'ApiError'
        this.status = status
    }
}

/**
 * 拼接完整请求 URL 并附加查询参数
 *
 * @param path - 相对于 BASE_URL 的路径（以 / 开头）
 * @param params - 可选查询参数，值为 undefined 的键自动忽略
 * @returns 完整 URL 字符串
 */
function buildUrl(
    path: string,
    params?: Record<string, string | number | undefined>,
): string {
    const url = new URL(`${BASE_URL}${path}`, window.location.origin)

    if (params !== undefined) {
        for (const [key, value] of Object.entries(params)) {
            if (value !== undefined) {
                url.searchParams.set(key, String(value))
            }
        }
    }

    return url.toString()
}

/**
 * 构建请求头，自动注入 Authorization 与 Content-Type
 *
 * @param contentType - 请求体类型，无请求体时传 undefined
 * @returns Headers 实例
 */
function buildHeaders(contentType?: string): Headers {
    const headers = new Headers()

    if (contentType !== undefined) {
        headers.set('Content-Type', contentType)
    }

    const token = getToken()
    if (token !== null) {
        headers.set('Authorization', `Bearer ${token}`)
    }

    return headers
}

/**
 * 从错误响应中提取可读消息
 *
 * 优先解析后端统一错误格式 { error: string }，
 * 解析失败时回退到 HTTP 状态码对应的默认文案。
 *
 * @param response - fetch Response 对象
 * @returns 可展示给用户的错误消息
 */
async function parseErrorMessage(response: Response): Promise<string> {
    try {
        const body: unknown = await response.json()
        if (
            typeof body === 'object' &&
            body !== null &&
            'error' in body
        ) {
            const errorField = (body as { error: unknown }).error
            if (typeof errorField === 'string') {
                return errorField
            }
        }
    } catch {
        /* JSON 解析失败，使用状态码回退 */
    }
    return getHttpErrorMessage(response.status)
}

/**
 * 统一处理响应：错误拦截、401 清除 token、JSON 解析
 *
 * @template T - 预期的响应数据类型
 * @param response - fetch Response 对象
 * @returns 解析后的响应数据
 * @throws {ApiError} HTTP 状态码非 2xx 时抛出
 */
async function handleResponse<T>(response: Response): Promise<T> {
    if (!response.ok) {
        if (response.status === 401) {
            removeToken()
        }
        const message = await parseErrorMessage(response)
        throw new ApiError(response.status, message)
    }

    if (response.status === 204) {
        return undefined as T
    }

    return response.json() as Promise<T>
}

/**
 * 发送 GET 请求
 *
 * @template T - 响应数据类型
 * @param path - 请求路径
 * @param params - 查询参数
 * @returns 响应数据
 * @throws {ApiError} 请求失败时抛出
 */
export async function get<T>(
    path: string,
    params?: Record<string, string | number | undefined>,
): Promise<T> {
    const response = await fetch(buildUrl(path, params), {
        method: 'GET',
        headers: buildHeaders(),
    })
    return handleResponse<T>(response)
}

/**
 * 发送 POST 请求
 *
 * @template T - 响应数据类型
 * @param path - 请求路径
 * @param body - 请求体（自动序列化为 JSON）
 * @returns 响应数据
 * @throws {ApiError} 请求失败时抛出
 */
export async function post<T>(
    path: string,
    body?: unknown,
): Promise<T> {
    const hasBody = body !== undefined
    const response = await fetch(buildUrl(path), {
        method: 'POST',
        headers: buildHeaders(hasBody ? 'application/json' : undefined),
        body: hasBody ? JSON.stringify(body) : undefined,
    })
    return handleResponse<T>(response)
}

/**
 * 发送 PUT 请求
 *
 * @template T - 响应数据类型
 * @param path - 请求路径
 * @param body - 请求体（自动序列化为 JSON）
 * @returns 响应数据
 * @throws {ApiError} 请求失败时抛出
 */
export async function put<T>(
    path: string,
    body?: unknown,
): Promise<T> {
    const hasBody = body !== undefined
    const response = await fetch(buildUrl(path), {
        method: 'PUT',
        headers: buildHeaders(hasBody ? 'application/json' : undefined),
        body: hasBody ? JSON.stringify(body) : undefined,
    })
    return handleResponse<T>(response)
}

/**
 * 发送 DELETE 请求
 *
 * @template T - 响应数据类型
 * @param path - 请求路径
 * @returns 响应数据
 * @throws {ApiError} 请求失败时抛出
 */
export async function del<T>(path: string): Promise<T> {
    const response = await fetch(buildUrl(path), {
        method: 'DELETE',
        headers: buildHeaders(),
    })
    return handleResponse<T>(response)
}