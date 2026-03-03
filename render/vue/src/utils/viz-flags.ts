/**
 * 可视化功能开关管理
 *
 * 所有可视化功能的运行时开关，支持动态切换与 localStorage 持久化。
 *
 * @file src/utils/viz-flags.ts
 * @author WaterRun
 * @date 2026-03-02
 */

import { reactive, readonly, watch } from 'vue'

/**
 * 可视化功能开关状态
 *
 * @interface
 */
export interface VizFlagsState {
    /** 显示 TOML remarks 元数据（标题/作者/备注） */
    showMetadata: boolean
    /** 帧间 PPT 平滑过渡动画 */
    enableSmoothTransitions: boolean
    /** 联动编辑器代码行高亮 */
    enableCodeLineHighlight: boolean
    /** 自适应画布缩放/平移 */
    enableAutoFit: boolean
    /** 变更强化（虚线移除 + 高亮新增） */
    enableDiffHighlight: boolean
    /** 控制台调试日志 */
    enableConsoleDebug: boolean
    /** 跳过就绪页 */
    skipReadyPage: boolean
    /** 生成后自动播放 */
    autoPlayOnGenerate: boolean
    /** 自动播放间隔（毫秒，0–10000，步进 100） */
    playbackInterval: number
}

/** 向后兼容别名 */
export type VizFlags = VizFlagsState

/** localStorage 键名 */
const STORAGE_KEY = 'ds4viz_viz_flags'

/**
 * 默认开关配置
 */
const DEFAULT_FLAGS: VizFlagsState = {
    showMetadata: false,
    enableSmoothTransitions: true,
    enableCodeLineHighlight: true,
    enableAutoFit: true,
    enableDiffHighlight: true,
    enableConsoleDebug: false,
    skipReadyPage: false,
    autoPlayOnGenerate: false,
    playbackInterval: 500,
}

/**
 * 从 localStorage 恢复配置
 *
 * @returns 存储的部分配置，解析失败时返回空对象
 */
function loadFromStorage(): Partial<VizFlagsState> {
    try {
        const raw = localStorage.getItem(STORAGE_KEY)
        if (raw !== null) {
            return JSON.parse(raw) as Partial<VizFlagsState>
        }
    } catch {
        /* localStorage 不可用或数据损坏 */
    }
    return {}
}

/**
 * 持久化配置到 localStorage
 *
 * @param flags - 完整配置对象
 */
function saveToStorage(flags: VizFlagsState): void {
    try {
        localStorage.setItem(STORAGE_KEY, JSON.stringify(flags))
    } catch {
        /* localStorage 不可用 */
    }
}

/** 合并持久化值与默认值 */
const stored = loadFromStorage()
const _flags = reactive<VizFlagsState>({ ...DEFAULT_FLAGS, ...stored })

/** 只读响应式代理，供组件模板绑定 */
export const vizFlags = readonly(_flags) as Readonly<VizFlagsState>

/** 配置变更时自动持久化 */
watch(_flags, (v) => saveToStorage({ ...v }), { deep: true })

/**
 * 获取指定开关当前值
 *
 * @param key - 开关名称
 * @returns 当前值
 */
export function getFlag<K extends keyof VizFlagsState>(key: K): VizFlagsState[K] {
    return _flags[key]
}

/**
 * 设置指定开关
 *
 * @param key - 开关名称
 * @param value - 新值
 */
export function setFlag<K extends keyof VizFlagsState>(key: K, value: VizFlagsState[K]): void {
    ; (_flags as VizFlagsState)[key] = value
    logDebug(`[viz-flags] ${key} = ${String(value)}`)
}

/**
 * 批量设置开关
 *
 * @param partial - 需要修改的开关子集
 */
export function setFlags(partial: Partial<VizFlagsState>): void {
    for (const [key, value] of Object.entries(partial)) {
        if (key in _flags) {
            ; (_flags as Record<string, unknown>)[key] = value
        }
    }
    logDebug('[viz-flags] batch update', partial)
}

/**
 * 重置所有开关到默认值
 */
export function resetFlags(): void {
    Object.assign(_flags, DEFAULT_FLAGS)
    logDebug('[viz-flags] reset to defaults')
}

/**
 * 条件调试日志
 *
 * @param args - 与 console.log 参数一致
 */
export function logDebug(...args: unknown[]): void {
    if (_flags.enableConsoleDebug) {
        console.log('%c[ds4viz]', 'color:#0078d4;font-weight:600', ...args)
    }
}

/* ---- 开发时控制台快捷入口 ---- */

if (typeof window !== 'undefined') {
    const win = window as unknown as Record<string, unknown>
    win.__ds4vizFlags = { get: getFlag, set: setFlag, setAll: setFlags, reset: resetFlags }
}
