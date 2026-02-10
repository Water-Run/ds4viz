/**
 * JWT token 存储管理
 *
 * 内存缓存优先，sessionStorage 作为持久化降级方案。
 *
 * @file src/utils/storage.ts
 * @author WaterRun
 * @date 2026-02-10
 */

/** sessionStorage 中 token 对应的键名 */
const TOKEN_KEY = 'ds4viz_token'

/** 内存缓存，避免频繁读 sessionStorage */
let memoryToken: string | null = null

/**
 * 读取 JWT token
 *
 * 优先返回内存缓存，缓存未命中时从 sessionStorage 恢复。
 *
 * @returns token 字符串，未持有时为 null
 *
 * @example
 * ```typescript
 * const token = getToken()
 * ```
 */
export function getToken(): string | null {
    if (memoryToken !== null) {
        return memoryToken
    }
    try {
        const stored = sessionStorage.getItem(TOKEN_KEY)
        if (stored !== null) {
            memoryToken = stored
        }
        return stored
    } catch {
        return null
    }
}

/**
 * 存储 JWT token
 *
 * 同时写入内存缓存与 sessionStorage。
 *
 * @param token - 待存储的 token 字符串
 *
 * @example
 * ```typescript
 * setToken('eyJhbGciOiJIUzI1NiIs...')
 * ```
 */
export function setToken(token: string): void {
    memoryToken = token
    try {
        sessionStorage.setItem(TOKEN_KEY, token)
    } catch {
        /* sessionStorage 不可用时仅保留内存存储 */
    }
}

/**
 * 清除 JWT token
 *
 * 同时清除内存缓存与 sessionStorage 中的记录。
 *
 * @example
 * ```typescript
 * removeToken()
 * ```
 */
export function removeToken(): void {
    memoryToken = null
    try {
        sessionStorage.removeItem(TOKEN_KEY)
    } catch {
        /* sessionStorage 不可用 */
    }
}

/**
 * 检查是否已持有 token
 *
 * @returns 是否存在有效 token
 *
 * @example
 * ```typescript
 * if (hasToken()) {
 *   // 已登录
 * }
 * ```
 */
export function hasToken(): boolean {
    return getToken() !== null
}