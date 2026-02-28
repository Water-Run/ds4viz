/**
 * 时间格式化工具函数
 *
 * 后端存储 UTC 时间，前端显示时转换为用户当前时区并附加偏移标识。
 *
 * @file src/utils/time.ts
 * @author WaterRun
 * @date 2026-02-28
 */

/**
 * 确保 ISO 字符串包含时区标记
 *
 * 后端返回的 UTC 时间可能不含 Z 后缀，
 * 需要补充以确保浏览器按 UTC 解析。
 *
 * @param iso - 原始 ISO 字符串
 * @returns 带时区标记的 ISO 字符串
 */
function ensureUtcSuffix(iso: string): string {
    if (iso.endsWith('Z') || /[+-]\d{2}(:\d{2})?$/.test(iso)) {
        return iso
    }
    return `${iso}Z`
}

/**
 * 获取当前时区的 UTC 偏移标识
 *
 * @returns 形如 "UTC+8" 或 "UTC-5:30" 的字符串
 */
function getUtcOffsetLabel(): string {
    const offsetMinutes = -new Date().getTimezoneOffset()
    const sign = offsetMinutes >= 0 ? '+' : '-'
    const hours = Math.floor(Math.abs(offsetMinutes) / 60)
    const minutes = Math.abs(offsetMinutes) % 60
    if (minutes === 0) {
        return `UTC${sign}${hours}`
    }
    return `UTC${sign}${hours}:${String(minutes).padStart(2, '0')}`
}

/**
 * ISO 8601 时间字符串转本地日期时间（附带时区标识）
 *
 * @param iso - ISO 8601 格式时间字符串（UTC）
 * @returns 格式化后的本地时间，解析失败时原样返回
 *
 * @example
 * ```typescript
 * formatDateTime('2025-08-01T06:30:00')
 * // → '2025/08/01 14:30 (UTC+8)'
 * ```
 */
export function formatDateTime(iso: string): string {
    const date = new Date(ensureUtcSuffix(iso))
    if (Number.isNaN(date.getTime())) {
        return iso
    }
    const formatted = date.toLocaleString('zh-CN', {
        year: 'numeric',
        month: '2-digit',
        day: '2-digit',
        hour: '2-digit',
        minute: '2-digit',
        hour12: false,
    })
    return `${formatted} (${getUtcOffsetLabel()})`
}

/**
 * ISO 8601 时间字符串转相对时间描述
 *
 * 30 天内使用相对表述（如 "3 分钟前"），超出后回退到完整日期（附时区标识）。
 *
 * @param iso - ISO 8601 格式时间字符串（UTC）
 * @returns 相对时间描述或完整日期
 *
 * @example
 * ```typescript
 * formatRelativeTime('2026-02-28T12:00:00')
 * // → '3 分钟前'
 * ```
 */
export function formatRelativeTime(iso: string): string {
    const target = new Date(ensureUtcSuffix(iso)).getTime()
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