/**
 * 可视化功能开关管理
 *
 * 所有可选可视化功能的运行时开关，支持动态切换。
 * 默认配置为最小化渲染 + 控制台调试，便于定位问题。
 *
 * 如需修改默认值：直接修改下方 DEFAULT_FLAGS 对象。
 * 如需运行时切换：在浏览器控制台执行
 *   window.__ds4vizFlags.set('enableNodeColors', true)
 * 或在代码中调用 setFlag / setFlags。
 *
 * @file src/utils/viz-flags.ts
 * @author WaterRun
 * @date 2026-02-27
 */

import { reactive, readonly } from 'vue'

/**
 * 可视化功能开关定义
 *
 * @interface
 */
export interface VizFlags {
    /** 节点色彩（TOP/FRONT/HEAD/ROOT 等特殊节点着色） */
    enableNodeColors: boolean
    /** 步骤切换时的平滑过渡动画 */
    enableSmoothTransitions: boolean
    /** 编辑器中当前步骤对应行高亮 */
    enableCodeLineHighlight: boolean
    /** 向 console 输出解析/渲染/导航调试信息 */
    enableConsoleDebug: boolean
    /** TOP / FRONT / REAR / HEAD / TAIL 等位置标记 */
    enableStepBadges: boolean
    /** 有向图箭头渲染 */
    enableGraphArrows: boolean
    /** 带权图权值标签 */
    enableEdgeWeightLabels: boolean
}

/**
 * 默认开关配置（最小化 + 调试模式）
 *
 * ┌─────────────────────────────┬─────────┬──────────────────────────┐
 * │ Flag                        │ Default │ 说明                     │
 * ├─────────────────────────────┼─────────┼──────────────────────────┤
 * │ enableNodeColors            │ false   │ 关闭以排除 CSS 渲染干扰  │
 * │ enableSmoothTransitions     │ false   │ 关闭以排除动画卡顿       │
 * │ enableCodeLineHighlight     │ false   │ 关闭以排除 Monaco 问题   │
 * │ enableConsoleDebug          │ true    │ 开启以观察运行时流程      │
 * │ enableStepBadges            │ true    │ 纯文本标记，无性能影响    │
 * │ enableGraphArrows           │ true    │ 基础 SVG marker          │
 * │ enableEdgeWeightLabels      │ true    │ 基础 SVG text            │
 * └─────────────────────────────┴─────────┴──────────────────────────┘
 */
const DEFAULT_FLAGS: VizFlags = {
    enableNodeColors: false,
    enableSmoothTransitions: false,
    enableCodeLineHighlight: false,
    enableConsoleDebug: true,
    enableStepBadges: true,
    enableGraphArrows: true,
    enableEdgeWeightLabels: true,
}

/** 内部可变状态 */
const _flags = reactive<VizFlags>({ ...DEFAULT_FLAGS })

/** 只读响应式代理，供组件模板绑定 */
export const vizFlags = readonly(_flags) as Readonly<VizFlags>

/**
 * 获取指定开关当前值
 *
 * @param key - 开关名称
 * @returns 当前值
 */
export function getFlag(key: keyof VizFlags): boolean {
    return _flags[key]
}

/**
 * 设置指定开关
 *
 * @param key - 开关名称
 * @param value - 新值
 */
export function setFlag(key: keyof VizFlags, value: boolean): void {
    _flags[key] = value
    logDebug(`[viz-flags] ${key} = ${String(value)}`)
}

/**
 * 批量设置开关
 *
 * @param partial - 需要修改的开关子集
 */
export function setFlags(partial: Partial<VizFlags>): void {
    for (const [key, value] of Object.entries(partial)) {
        if (key in _flags) {
            _flags[key as keyof VizFlags] = value as boolean
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
 * 仅在 enableConsoleDebug 为 true 时输出到 console.log。
 *
 * @param args - 与 console.log 参数一致
 */
export function logDebug(...args: unknown[]): void {
    if (_flags.enableConsoleDebug) {
        console.log('%c[ds4viz]', 'color:#0078d4;font-weight:600', ...args)
    }
}

/* ----------------------------------------------------------------
 *  开发时控制台快捷入口
 * ---------------------------------------------------------------- */

if (typeof window !== 'undefined') {
    const win = window as unknown as Record<string, unknown>
    win.__ds4vizFlags = { get: getFlag, set: setFlag, setAll: setFlags, reset: resetFlags }
}