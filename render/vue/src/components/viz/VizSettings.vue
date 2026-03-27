<script setup lang="ts">
/**
 * 可视化设置浮动面板
 *
 * 固定于可视化面板左下角的齿轮按钮，悬浮展开设置面板。
 *
 * @file src/components/viz/VizSettings.vue
 * @author WaterRun
 * @date 2026-03-27
 * @component VizSettings
 */

import { ref, onUnmounted } from 'vue'
import { vizFlags, setFlag, resetFlags } from '@/utils/viz-flags'
import type { VizFlagsState } from '@/utils/viz-flags'

/**
 * 面板开关状态
 */
const isOpen = ref<boolean>(false)

const debugOpen = ref<boolean>(false)

const closeDebug = (): void => {
    debugOpen.value = false
}

const toggleDebug = (): void => {
    debugOpen.value = !debugOpen.value
}

/**
 * 延迟关闭定时器
 */
const closeTimer = ref<number | null>(null)

/**
 * 布尔开关定义
 *
 * @interface
 */
interface BoolItem {
    /** 配置键名 */
    key: keyof VizFlagsState
    /** 显示标签 */
    label: string
}

/**
 * 布尔开关列表
 */
const booleanItems: BoolItem[] = [
    { key: 'showMetadata', label: '显示元数据' },
    { key: 'enableAutoFit', label: '自适应画布' },
    { key: 'autoPlayOnGenerate', label: '生成后自动播放' },
    { key: 'showPhases', label: '显示阶段导航' },
    { key: 'highlightAutoFocus', label: '高亮聚焦' },
    { key: 'showVizTitle', label: '可视化方案名' },
]

const debugItems: BoolItem[] = [
    { key: 'enableSmoothTransitions', label: '平滑动画衔接' },
    { key: 'enableCodeLineHighlight', label: '联动代码高亮' },
    { key: 'enableDiffHighlight', label: '变更强化' },
    { key: 'skipReadyPage', label: '跳过初始页' },
    { key: 'enableConsoleDebug', label: '控制台调试' },
]

/**
 * 取消延迟关闭
 */
const cancelClose = (): void => {
    if (closeTimer.value !== null) {
        window.clearTimeout(closeTimer.value)
        closeTimer.value = null
    }
}

/**
 * 鼠标进入区域时展开
 */
const handlePointerEnter = (): void => {
    cancelClose()
    isOpen.value = true
}

/**
 * 鼠标离开区域时延迟关闭
 */
const handlePointerLeave = (): void => {
    cancelClose()
    closeTimer.value = window.setTimeout(() => {
        isOpen.value = false
        closeDebug()
        closeTimer.value = null
    }, 300)
}

/**
 * 点击切换（备选交互）
 */
const handleToggleClick = (): void => {
    isOpen.value = !isOpen.value
}

/**
 * 切换布尔开关
 *
 * @param key - 开关键名
 */
const handleToggle = (key: keyof VizFlagsState): void => {
    setFlag(key, !vizFlags[key] as never)
}

/**
 * 播放间隔滑块输入
 */
const handleIntervalInput = (e: Event): void => {
    const val = Number((e.target as HTMLInputElement).value)
    setFlag('playbackInterval', Math.max(0, Math.min(10000, val)))
}

onUnmounted(() => cancelClose())
</script>

<template>
    <div class="viz-settings" @pointerenter="handlePointerEnter" @pointerleave="handlePointerLeave">
        <button class="viz-settings__toggle" :class="{ 'viz-settings__toggle--open': isOpen }"
            :title="isOpen ? '关闭设置' : '渲染器策略'" @click="handleToggleClick">
            <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round"
                stroke-linejoin="round">
                <circle cx="12" cy="12" r="3" />
                <path
                    d="M19.4 15a1.65 1.65 0 0 0 .33 1.82l.06.06a2 2 0 1 1-2.83 2.83l-.06-.06a1.65 1.65 0 0 0-1.82-.33 1.65 1.65 0 0 0-1 1.51V21a2 2 0 0 1-4 0v-.09A1.65 1.65 0 0 0 9 19.4a1.65 1.65 0 0 0-1.82.33l-.06.06a2 2 0 1 1-2.83-2.83l.06-.06A1.65 1.65 0 0 0 4.68 15a1.65 1.65 0 0 0-1.51-1H3a2 2 0 0 1 0-4h.09A1.65 1.65 0 0 0 4.6 9a1.65 1.65 0 0 0-.33-1.82l-.06-.06a2 2 0 1 1 2.83-2.83l.06.06A1.65 1.65 0 0 0 9 4.68a1.65 1.65 0 0 0 1-1.51V3a2 2 0 0 1 4 0v.09a1.65 1.65 0 0 0 1 1.51 1.65 1.65 0 0 0 1.82-.33l.06-.06a2 2 0 1 1 2.83 2.83l-.06.06A1.65 1.65 0 0 0 19.4 9a1.65 1.65 0 0 0 1.51 1H21a2 2 0 0 1 0 4h-.09a1.65 1.65 0 0 0-1.51 1z" />
            </svg>
        </button>

        <Transition name="settings-slide">
            <div v-if="isOpen" class="viz-settings__panel">
                <div class="viz-settings__header">
                    <span class="viz-settings__title">渲染器策略</span>
                </div>

                <div class="viz-settings__body">
                    <div v-for="item in booleanItems" :key="item.key" class="viz-settings__row"
                        @click="handleToggle(item.key)">
                        <span class="viz-settings__label">{{ item.label }}</span>
                        <span class="viz-settings__switch" :class="{ 'viz-settings__switch--on': vizFlags[item.key] }">
                            <span class="viz-settings__thumb" />
                        </span>
                    </div>

                    <div class="viz-settings__row viz-settings__row--debug" @pointerenter="debugOpen = true"
                        @pointerleave="debugOpen = false">
                        <button class="viz-settings__debug-trigger" @click.stop="toggleDebug">
                            <span class="viz-settings__debug-left">
                                <svg viewBox="0 0 24 24" fill="currentColor" class="viz-settings__bug-icon"
                                    aria-hidden="true">
                                    <path
                                        d="M20 8h-2.81a5.985 5.985 0 0 0-1.82-2.19L17 4.19 15.59 2.78l-2.01 2.01A5.94 5.94 0 0 0 12 4c-.55 0-1.08.07-1.58.2L8.41 2.19 7 3.6l1.63 1.62A5.985 5.985 0 0 0 6.81 8H4v2h2v2H4v2h2v2H4v2h2.81A6.001 6.001 0 0 0 18 18h2v-2h-2v-2h2v-2h-2v-2h2V8zm-8 10a4 4 0 0 1-4-4v-4a4 4 0 0 1 8 0v4a4 4 0 0 1-4 4z" />
                                </svg>
                                <span>调试</span>
                            </span>
                            <span class="viz-settings__debug-right">
                                <svg viewBox="0 0 24 24" fill="currentColor" class="viz-settings__chevron-icon">
                                    <path d="M10 6l6 6-6 6z" />
                                </svg>
                            </span>
                        </button>

                        <Transition name="settings-slide">
                            <div v-if="debugOpen" class="viz-settings__sub-panel">
                                <div v-for="item in debugItems" :key="item.key" class="viz-settings__row"
                                    @click="handleToggle(item.key)">
                                    <span class="viz-settings__label">{{ item.label }}</span>
                                    <span class="viz-settings__switch"
                                        :class="{ 'viz-settings__switch--on': vizFlags[item.key] }">
                                        <span class="viz-settings__thumb" />
                                    </span>
                                </div>
                            </div>
                        </Transition>
                    </div>

                    <div class="viz-settings__row viz-settings__row--interval">
                        <span class="viz-settings__label">播放间隔</span>
                        <div class="viz-settings__interval">
                            <input type="range" min="0" max="10000" step="100" :value="vizFlags.playbackInterval"
                                @input="handleIntervalInput" />
                            <span class="viz-settings__interval-value">{{ vizFlags.playbackInterval }}ms</span>
                        </div>
                    </div>
                </div>

                <div class="viz-settings__footer">
                    <button class="viz-settings__reset" @click="resetFlags">恢复默认</button>
                </div>
            </div>
        </Transition>
    </div>
</template>

<style scoped>
.viz-settings {
    position: absolute;
    bottom: 12px;
    left: 12px;
    z-index: 1000;
    /* 明确给大值，避免变量不够高 */
    display: flex;
    flex-direction: column;
    align-items: flex-start;
}

.viz-settings__toggle {
    width: 36px;
    height: 36px;
    border-radius: 50%;
    border: 1px solid var(--color-border-strong);
    background: var(--color-bg-surface);
    color: var(--color-text-tertiary);
    display: flex;
    align-items: center;
    justify-content: center;
    cursor: pointer;
    transition:
        transform var(--duration-normal) var(--ease),
        background-color var(--duration-fast) var(--ease),
        color var(--duration-fast) var(--ease),
        box-shadow var(--duration-fast) var(--ease);
    box-shadow: var(--shadow-static);
}

.viz-settings__toggle svg {
    width: 18px;
    height: 18px;
}

.viz-settings__toggle:hover {
    background: var(--color-bg-hover);
    color: var(--color-accent);
    box-shadow: var(--shadow-hover);
}

.viz-settings__toggle--open {
    transform: rotate(90deg);
}

.viz-settings__panel {
    position: absolute;
    bottom: 44px;
    left: 0;
    width: 260px;
    max-height: 420px;
    overflow: visible;
    background: var(--color-bg-surface);
    border: 1px solid var(--color-border);
    border-radius: var(--radius-lg);
    box-shadow: var(--shadow-hover);
    z-index: 1001;
}

.viz-settings__header {
    padding: 12px 16px 8px;
    border-bottom: 1px solid var(--color-border);
}

.viz-settings__title {
    font-size: var(--text-xs);
    font-weight: var(--weight-semibold);
    color: var(--color-text-primary);
    letter-spacing: var(--tracking-panel-head);
    text-transform: uppercase;
}

.viz-settings__body {
    padding: 4px 0;
}

.viz-settings__row {
    display: flex;
    align-items: center;
    justify-content: space-between;
    padding: 7px 16px;
    cursor: pointer;
    transition: background-color var(--duration-fast) var(--ease);
}

.viz-settings__row:hover {
    background: var(--color-bg-hover);
}

.viz-settings__label {
    font-size: var(--text-xs);
    color: var(--color-text-body);
    user-select: none;
}

.viz-settings__switch {
    position: relative;
    width: 34px;
    height: 18px;
    border-radius: 9px;
    background: var(--color-bg-active);
    border: 1px solid var(--color-border-strong);
    transition:
        background-color var(--duration-fast) var(--ease),
        border-color var(--duration-fast) var(--ease);
    flex-shrink: 0;
}

.viz-settings__switch--on {
    background: var(--color-accent);
    border-color: var(--color-accent);
}

.viz-settings__thumb {
    position: absolute;
    top: 2px;
    left: 2px;
    width: 12px;
    height: 12px;
    border-radius: 50%;
    background: var(--color-bg-surface);
    transition: transform var(--duration-fast) var(--ease);
    box-shadow: 0 1px 2px rgba(0, 0, 0, 0.12);
}

.viz-settings__switch--on .viz-settings__thumb {
    transform: translateX(16px);
}

.viz-settings__row--interval {
    flex-direction: column;
    align-items: flex-start;
    gap: 6px;
    cursor: default;
}

.viz-settings__interval {
    display: flex;
    align-items: center;
    gap: 8px;
    width: 100%;
}

.viz-settings__interval input[type='range'] {
    flex: 1;
    height: 4px;
    accent-color: var(--color-accent);
    cursor: pointer;
}

.viz-settings__interval-value {
    font-size: 11px;
    font-weight: var(--weight-semibold);
    color: var(--color-accent);
    min-width: 52px;
    text-align: right;
    font-family: var(--font-mono);
}

.viz-settings__footer {
    padding: 8px 16px 12px;
    border-top: 1px solid var(--color-border);
}

.viz-settings__reset {
    width: 100%;
    padding: 5px 0;
    border-radius: var(--radius-control);
    border: 1px solid var(--color-border);
    background: transparent;
    color: var(--color-text-tertiary);
    font-size: var(--text-xs);
    font-family: inherit;
    cursor: pointer;
    transition:
        color var(--duration-fast) var(--ease),
        border-color var(--duration-fast) var(--ease);
}

.viz-settings__reset:hover {
    color: var(--color-text-body);
    border-color: var(--color-border-strong);
}

.settings-slide-enter-active,
.settings-slide-leave-active {
    transition:
        opacity var(--duration-fast) var(--ease),
        transform var(--duration-fast) var(--ease);
}

.settings-slide-enter-from,
.settings-slide-leave-to {
    opacity: 0;
    transform: translateY(6px) scale(0.96);
}

.viz-settings__row--debug {
    position: relative;
    padding: 0;
}

.viz-settings__debug-trigger {
    width: 100%;
    height: 34px;
    border: none;
    background: none;
    padding: 0 16px;
    display: flex;
    align-items: center;
    justify-content: space-between;
    cursor: pointer;
    color: var(--color-text-body);
    font-family: inherit;
    transition: background-color var(--duration-fast) var(--ease);
}

.viz-settings__debug-trigger:hover {
    background: var(--color-bg-hover);
}

.viz-settings__debug-left {
    display: inline-flex;
    align-items: center;
    gap: 8px;
    font-size: var(--text-xs);
}

.viz-settings__bug-icon {
    width: 14px;
    height: 14px;
}

.viz-settings__chevron-icon {
    width: 14px;
    height: 14px;
    color: var(--color-text-tertiary);
}

.viz-settings__sub-panel {
    position: absolute;
    left: calc(100% + 6px);
    top: 0;
    width: 220px;
    background: var(--color-bg-surface);
    border: 1px solid var(--color-border);
    border-radius: var(--radius-lg);
    box-shadow: var(--shadow-hover);
    padding: 4px 0;
    z-index: 1002;
}
</style>
