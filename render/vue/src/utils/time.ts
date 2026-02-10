/**
 * 时间格式化工具函数
 *
 * @file src/utils/time.ts
 * @author WaterRun
 * @date 2026-02-10
 */

/**
 * ISO 8601 时间字符串转本地日期时间
 *
 * @param iso - ISO 8601 格式时间字符串
 * @returns 格式化后的本地时间，解析失败时原样返回
 *
 * @example
 * ```typescript
 * formatDateTime('2025-08-01T14:30:00Z')
 * // → '2025/08/01 14:30'
 * ```
 */
export function formatDateTime(iso: string): string {
    const date = new Date(iso)
    if (Number.isNaN(date.getTime())) {
        return iso
    }
    return date.toLocaleString('zh-CN', {
        year: 'numeric',
        month: '2-digit',
        day: '2-digit',
        hour: '2-digit',
        minute: '2-digit',
        hour12: false,
    })
}

/**
 * ISO 8601 时间字符串转相对时间描述
 *
 * 30 天内使用相对表述（如 "3 分钟前"），超出后回退到完整日期。
 *
 * @param iso - ISO 8601 格式时间字符串
 * @returns 相对时间描述或完整日期
 *
 * @example
 * ```typescript
 * formatRelativeTime('2026-02-10T12:00:00Z')
 * // → '3 分钟前'（假设当前时间为 12:03）
 * ```
 */
export function formatRelativeTime(iso: string): string {
    const target = new Date(iso).getTime()
    if (Number.isNaN(target)) {
        return iso
    }

    const diff = Date.now() - target
    if (diff < 0) {
        return formatDateTime(iso)
    }

    const seconds = Math.floor(diff / 1_000)
    if (seconds < 60) return '刚刚'

    const minutes = Math.floor(seconds / 60)
    if (minutes < 60) return `${minutes} 分钟前`

    const hours = Math.floor(minutes / 60)
    if (hours < 24) return `${hours} 小时前`

    const days = Math.floor(hours / 24)
    if (days < 30) return `${days} 天前`

    return formatDateTime(iso)
}

/**
 * 毫秒耗时转可读字符串
 *
 * @param ms - 毫秒数
 * @returns 格式化的耗时字符串
 *
 * @example
 * ```typescript
 * formatDuration(128)   // → '128ms'
 * formatDuration(2340)  // → '2.34s'
 * ```
 */
export function formatDuration(ms: number): string {
    if (ms < 1_000) {
        return `${Math.round(ms)}ms`
    }
    return `${(ms / 1_000).toFixed(2)}s`
}