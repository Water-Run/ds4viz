<script setup lang="ts">
/**
 * LLM 代码生成浮动面板
 *
 * 固定于编辑器面板右下角的圆形按钮，悬浮展开生成面板。
 * 包含提示输入、生成操作和 LLM 配置设置。
 *
 * @file src/components/editor/LlmPanel.vue
 * @author WaterRun
 * @date 2026-03-28
 * @component LlmPanel
 */

import { ref, onUnmounted } from 'vue'
import { llmConfig, resetLlmConfig, callLlm } from '@/utils/llm'
import { buildSystemPrompt } from '@/utils/llm-prompts'
import type { Language } from '@/types/api'

/**
 * 组件属性定义
 */
interface Props {
    /** 当前编辑器语言 */
    language: Language
    /** 编辑器是否为默认代码 */
    isDefaultCode: boolean
    /** 当前编辑器代码 */
    currentCode?: string
}

/**
 * 组件事件定义
 */
interface Emits {
    /** 代码生成完成 */
    (event: 'generated', code: string): void
}

const props = withDefaults(defineProps<Props>(), {
    currentCode: '',
})
const emit = defineEmits<Emits>()

/** 面板开关状态 */
const isOpen = ref<boolean>(false)

/** 设置区展开状态 */
const settingsOpen = ref<boolean>(false)

/** 提示输入内容 */
const promptText = ref<string>('')

/** 加载中 */
const generating = ref<boolean>(false)

/** 错误消息 */
const errorMessage = ref<string>('')

/** 覆盖确认状态 */
const confirmingOverwrite = ref<boolean>(false)

/** 是否修改当前代码（勾选后将当前代码注入 Prompt） */
const modifyCurrentCode = ref<boolean>(false)

/** 延迟关闭定时器 */
const closeTimer = ref<number | null>(null)

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
    if (generating.value) return
    closeTimer.value = window.setTimeout(() => {
        isOpen.value = false
        settingsOpen.value = false
        confirmingOverwrite.value = false
        closeTimer.value = null
    }, 300)
}

/**
 * 点击切换
 */
const handleToggleClick = (): void => {
    isOpen.value = !isOpen.value
    if (!isOpen.value) {
        settingsOpen.value = false
        confirmingOverwrite.value = false
    }
}

/**
 * 切换设置区展开
 */
const toggleSettings = (): void => {
    settingsOpen.value = !settingsOpen.value
}

/**
 * 处理按键事件（Enter 提交，Shift+Enter 换行）
 *
 * @param event - 键盘事件
 */
const handleKeydown = (event: KeyboardEvent): void => {
    if (event.key === 'Enter' && !event.shiftKey) {
        event.preventDefault()
        requestGenerate()
    }
}

/**
 * 请求生成代码
 */
const requestGenerate = (): void => {
    if (generating.value) return
    if (promptText.value.trim().length === 0) return

    errorMessage.value = ''

    if (
        !modifyCurrentCode.value
        && !props.isDefaultCode
        && !confirmingOverwrite.value
    ) {
        confirmingOverwrite.value = true
        return
    }

    confirmingOverwrite.value = false
    executeGenerate()
}

/**
 * 确认覆盖后生成
 */
const confirmOverwrite = (): void => {
    confirmingOverwrite.value = false
    executeGenerate()
}

/**
 * 取消覆盖
 */
const cancelOverwrite = (): void => {
    confirmingOverwrite.value = false
}

/**
 * 执行 LLM 生成
 */
const executeGenerate = async (): Promise<void> => {
    generating.value = true
    errorMessage.value = ''

    const systemPrompt = buildSystemPrompt(props.language, {
        modifyCurrentCode: modifyCurrentCode.value,
        currentCode: props.currentCode,
    })

    try {
        const result = await callLlm(systemPrompt, promptText.value.trim())

        if (result.ok && result.content !== undefined) {
            emit('generated', result.content)
            promptText.value = ''
        } else {
            errorMessage.value = result.errorMessage ?? '生成失败'
        }
    } catch (error: unknown) {
        errorMessage.value = error instanceof Error ? error.message : '生成失败'
    } finally {
        generating.value = false
    }
}

/**
 * 处理超时设置输入
 *
 * @param event - 输入事件
 */
const handleTimeoutInput = (event: Event): void => {
    const val = Number((event.target as HTMLInputElement).value)
    llmConfig.timeout = Math.max(1, Math.min(600, val))
}

/**
 * 处理 Token 设置输入
 *
 * @param event - 输入事件
 */
const handleTokenInput = (event: Event): void => {
    const val = Number((event.target as HTMLInputElement).value)
    llmConfig.maxTokens = Math.max(1, Math.min(131072, val))
}

onUnmounted(() => cancelClose())
</script>

<template>
    <div class="llm-panel" @pointerenter="handlePointerEnter" @pointerleave="handlePointerLeave">
        <button class="llm-panel__toggle"
            :class="{ 'llm-panel__toggle--open': isOpen, 'llm-panel__toggle--busy': generating }"
            :title="isOpen ? '关闭生成面板' : 'LLM 代码生成'" @click="handleToggleClick">
            <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.6" stroke-linecap="round"
                stroke-linejoin="round">
                <path
                    d="M12 2l1.09 3.26L16.36 6.36l-1.01 1.01L16.36 10.64 12 12l-3.26 1.09L6.36 16.36l-1.01-1.01L2 12l3.26-1.09L6.36 6.36l1.01 1.01L12 2z" />
                <circle cx="18" cy="6" r="2" />
                <circle cx="6" cy="18" r="2" />
            </svg>
        </button>

        <Transition name="llm-slide">
            <div v-if="isOpen" class="llm-panel__popover">
                <div class="llm-panel__header">
                    <span class="llm-panel__title">LLM 代码生成</span>
                </div>

                <div class="llm-panel__body">
                    <!-- 输入区 -->
                    <div class="llm-panel__input-wrap">
                        <textarea v-model="promptText" class="llm-panel__textarea" placeholder="描述需要生成的数据结构可视化代码…"
                            rows="3" :disabled="generating" @keydown="handleKeydown" />
                    </div>

                    <!-- 修改当前代码开关 -->
                    <label class="llm-panel__modify-toggle">
                        <input v-model="modifyCurrentCode" type="checkbox" :disabled="generating" />
                        <span>修改当前代码</span>
                    </label>

                    <!-- 覆盖确认 -->
                    <Transition name="llm-fade">
                        <div v-if="confirmingOverwrite" class="llm-panel__confirm">
                            <span class="llm-panel__confirm-text">将覆盖当前代码</span>
                            <div class="llm-panel__confirm-actions">
                                <button class="llm-panel__confirm-btn" @click="cancelOverwrite">取消</button>
                                <button class="llm-panel__confirm-btn llm-panel__confirm-btn--primary"
                                    @click="confirmOverwrite">确认</button>
                            </div>
                        </div>
                    </Transition>

                    <!-- 错误 -->
                    <Transition name="llm-fade">
                        <div v-if="errorMessage.length > 0" class="llm-panel__error">
                            {{ errorMessage }}
                        </div>
                    </Transition>

                    <!-- 操作栏 -->
                    <div class="llm-panel__actions">
                        <button class="llm-panel__generate-btn" :disabled="generating || promptText.trim().length === 0"
                            @click="requestGenerate">
                            <svg v-if="!generating" viewBox="0 0 24 24" fill="currentColor" class="llm-panel__btn-icon">
                                <path
                                    d="M12 2l1.09 3.26L16.36 6.36l-1.01 1.01L16.36 10.64 12 12l-3.26 1.09L6.36 16.36l-1.01-1.01L2 12l3.26-1.09L6.36 6.36l1.01 1.01L12 2z" />
                            </svg>
                            <span v-if="generating" class="llm-panel__spinner" />
                            <span>{{ generating ? '生成中…' : '生成' }}</span>
                        </button>
                        <button class="llm-panel__settings-trigger" @click="toggleSettings">
                            <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"
                                stroke-linecap="round" stroke-linejoin="round" class="llm-panel__btn-icon">
                                <circle cx="12" cy="12" r="3" />
                                <path
                                    d="M19.4 15a1.65 1.65 0 0 0 .33 1.82l.06.06a2 2 0 1 1-2.83 2.83l-.06-.06a1.65 1.65 0 0 0-1.82-.33 1.65 1.65 0 0 0-1 1.51V21a2 2 0 0 1-4 0v-.09A1.65 1.65 0 0 0 9 19.4a1.65 1.65 0 0 0-1.82.33l-.06.06a2 2 0 1 1-2.83-2.83l.06-.06A1.65 1.65 0 0 0 4.68 15a1.65 1.65 0 0 0-1.51-1H3a2 2 0 0 1 0-4h.09A1.65 1.65 0 0 0 4.6 9a1.65 1.65 0 0 0-.33-1.82l-.06-.06a2 2 0 1 1 2.83-2.83l.06.06A1.65 1.65 0 0 0 9 4.68a1.65 1.65 0 0 0 1-1.51V3a2 2 0 0 1 4 0v.09a1.65 1.65 0 0 0 1 1.51 1.65 1.65 0 0 0 1.82-.33l.06-.06a2 2 0 1 1 2.83 2.83l-.06.06A1.65 1.65 0 0 0 19.4 9a1.65 1.65 0 0 0 1.51 1H21a2 2 0 0 1 0 4h-.09a1.65 1.65 0 0 0-1.51 1z" />
                            </svg>
                        </button>
                    </div>

                    <!-- 设置区 -->
                    <Transition name="llm-slide">
                        <div v-if="settingsOpen" class="llm-panel__settings">
                            <div class="llm-panel__setting-row">
                                <span class="llm-panel__setting-label">API 地址</span>
                                <input v-model="llmConfig.apiUrl" type="text" class="llm-panel__setting-input"
                                    placeholder="https://api.poe.com/v1" />
                            </div>
                            <div class="llm-panel__setting-row">
                                <span class="llm-panel__setting-label">模型</span>
                                <input v-model="llmConfig.model" type="text" class="llm-panel__setting-input"
                                    placeholder="claude-opus-4.6" />
                            </div>
                            <div class="llm-panel__setting-row">
                                <span class="llm-panel__setting-label">API Key</span>
                                <input v-model="llmConfig.apiKey" type="password" class="llm-panel__setting-input"
                                    placeholder="sk-..." />
                            </div>
                            <div class="llm-panel__setting-row">
                                <span class="llm-panel__setting-label">超时 (秒)</span>
                                <input type="number" class="llm-panel__setting-input llm-panel__setting-input--narrow"
                                    :value="llmConfig.timeout" min="1" max="600" @input="handleTimeoutInput" />
                            </div>
                            <div class="llm-panel__setting-row">
                                <span class="llm-panel__setting-label">最大 Token</span>
                                <input type="number" class="llm-panel__setting-input llm-panel__setting-input--narrow"
                                    :value="llmConfig.maxTokens" min="1" max="131072" @input="handleTokenInput" />
                            </div>
                            <button class="llm-panel__reset" @click="resetLlmConfig">恢复默认</button>
                        </div>
                    </Transition>
                </div>

                <div class="llm-panel__footer">
                    <span class="llm-panel__hint">LLM 配置存储在本机</span>
                </div>
            </div>
        </Transition>
    </div>
</template>

<style scoped>
.llm-panel {
    position: absolute;
    bottom: 12px;
    right: 12px;
    z-index: 1000;
    display: flex;
    flex-direction: column;
    align-items: flex-end;
}

.llm-panel__toggle {
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
        background-color var(--duration-fast) var(--ease),
        color var(--duration-fast) var(--ease),
        box-shadow var(--duration-fast) var(--ease),
        transform var(--duration-normal) var(--ease);
    box-shadow: var(--shadow-static);
}

.llm-panel__toggle svg {
    width: 18px;
    height: 18px;
}

.llm-panel__toggle:hover {
    background: var(--color-bg-hover);
    color: var(--color-accent);
    box-shadow: var(--shadow-hover);
}

.llm-panel__toggle--open {
    transform: rotate(45deg);
}

.llm-panel__toggle--busy {
    color: var(--color-accent);
    animation: llm-pulse 1.5s ease-in-out infinite;
}

@keyframes llm-pulse {

    0%,
    100% {
        opacity: 1;
    }

    50% {
        opacity: 0.5;
    }
}

.llm-panel__popover {
    position: absolute;
    bottom: 44px;
    right: 0;
    width: 320px;
    max-height: 520px;
    overflow: visible;
    background: var(--color-bg-surface);
    border: 1px solid var(--color-border);
    border-radius: var(--radius-lg);
    box-shadow: var(--shadow-hover);
    z-index: 1001;
    display: flex;
    flex-direction: column;
}

.llm-panel__header {
    padding: 12px 16px 8px;
    border-bottom: 1px solid var(--color-border);
}

.llm-panel__title {
    font-size: var(--text-xs);
    font-weight: var(--weight-semibold);
    color: var(--color-text-primary);
    letter-spacing: var(--tracking-panel-head);
    text-transform: uppercase;
}

.llm-panel__body {
    padding: 8px 12px;
    display: flex;
    flex-direction: column;
    gap: 8px;
}

.llm-panel__input-wrap {
    display: flex;
}

.llm-panel__textarea {
    width: 100%;
    min-height: 64px;
    max-height: 160px;
    padding: 8px 10px;
    border: 1px solid var(--color-border-strong);
    border-radius: var(--radius-control);
    background: var(--color-bg-surface);
    color: var(--color-text-primary);
    font-size: var(--text-sm);
    font-family: inherit;
    line-height: var(--leading-normal);
    resize: vertical;
    transition: border-color var(--duration-fast) var(--ease);
}

.llm-panel__textarea:focus {
    border-color: var(--color-accent);
    outline: none;
}

.llm-panel__textarea::placeholder {
    color: var(--color-text-tertiary);
}

.llm-panel__textarea:disabled {
    opacity: 0.5;
    cursor: not-allowed;
}

.llm-panel__modify-toggle {
    display: inline-flex;
    align-items: center;
    gap: 6px;
    font-size: var(--text-xs);
    color: var(--color-text-tertiary);
    user-select: none;
}

.llm-panel__modify-toggle input[type='checkbox'] {
    width: 14px;
    height: 14px;
    accent-color: var(--color-accent);
    cursor: pointer;
}

.llm-panel__confirm {
    display: flex;
    align-items: center;
    justify-content: space-between;
    gap: 8px;
    padding: 6px 10px;
    border-radius: var(--radius-control);
    background-color: var(--color-warning-muted);
    border: 1px solid var(--color-warning);
}

.llm-panel__confirm-text {
    font-size: var(--text-xs);
    color: var(--color-warning);
    font-weight: var(--weight-medium);
}

.llm-panel__confirm-actions {
    display: flex;
    gap: 4px;
}

.llm-panel__confirm-btn {
    height: 24px;
    padding: 0 10px;
    border: 1px solid var(--color-border-strong);
    border-radius: var(--radius-control);
    background: var(--color-bg-surface);
    color: var(--color-text-body);
    font-size: 11px;
    font-family: inherit;
    cursor: pointer;
    transition: background-color var(--duration-fast) var(--ease);
}

.llm-panel__confirm-btn:hover {
    background: var(--color-bg-hover);
}

.llm-panel__confirm-btn--primary {
    background: var(--color-accent);
    border-color: var(--color-accent);
    color: var(--color-accent-contrast);
}

.llm-panel__confirm-btn--primary:hover {
    background: var(--color-accent-hover);
}

.llm-panel__error {
    padding: 6px 10px;
    border-radius: var(--radius-control);
    background-color: var(--color-error-muted);
    border: 1px solid var(--color-error);
    color: var(--color-error);
    font-size: 11px;
    line-height: var(--leading-normal);
}

.llm-panel__actions {
    display: flex;
    align-items: center;
    gap: 6px;
}

.llm-panel__generate-btn {
    flex: 1;
    display: inline-flex;
    align-items: center;
    justify-content: center;
    gap: 6px;
    height: 32px;
    border: none;
    border-radius: var(--radius-control);
    background: var(--color-accent);
    color: var(--color-accent-contrast);
    font-size: var(--text-xs);
    font-weight: var(--weight-semibold);
    font-family: inherit;
    cursor: pointer;
    transition: background-color var(--duration-fast) var(--ease);
}

.llm-panel__generate-btn:hover:not(:disabled) {
    background: var(--color-accent-hover);
}

.llm-panel__generate-btn:disabled {
    opacity: 0.4;
    cursor: not-allowed;
}

.llm-panel__btn-icon {
    width: 14px;
    height: 14px;
    flex-shrink: 0;
}

.llm-panel__spinner {
    width: 14px;
    height: 14px;
    border: 2px solid rgba(255, 255, 255, 0.35);
    border-top-color: #fff;
    border-radius: 50%;
    animation: llm-spin 0.7s linear infinite;
}

@keyframes llm-spin {
    to {
        transform: rotate(360deg);
    }
}

.llm-panel__settings-trigger {
    width: 32px;
    height: 32px;
    flex-shrink: 0;
    display: flex;
    align-items: center;
    justify-content: center;
    border: 1px solid var(--color-border-strong);
    border-radius: var(--radius-control);
    background: var(--color-bg-surface);
    color: var(--color-text-tertiary);
    cursor: pointer;
    transition:
        background-color var(--duration-fast) var(--ease),
        color var(--duration-fast) var(--ease);
}

.llm-panel__settings-trigger:hover {
    background: var(--color-bg-hover);
    color: var(--color-text-primary);
}

.llm-panel__settings {
    display: flex;
    flex-direction: column;
    gap: 6px;
    padding-top: 8px;
    border-top: 1px solid var(--color-border);
}

.llm-panel__setting-row {
    display: flex;
    align-items: center;
    justify-content: space-between;
    gap: 8px;
}

.llm-panel__setting-label {
    font-size: 11px;
    color: var(--color-text-tertiary);
    white-space: nowrap;
    min-width: 56px;
}

.llm-panel__setting-input {
    flex: 1;
    min-width: 0;
    height: 26px;
    padding: 0 8px;
    border: 1px solid var(--color-border-strong);
    border-radius: var(--radius-sm);
    background: var(--color-bg-surface);
    color: var(--color-text-primary);
    font-size: 11px;
    font-family: var(--font-mono);
    transition: border-color var(--duration-fast) var(--ease);
}

.llm-panel__setting-input:focus {
    border-color: var(--color-accent);
    outline: none;
}

.llm-panel__setting-input--narrow {
    max-width: 100px;
}

.llm-panel__reset {
    width: 100%;
    padding: 4px 0;
    border-radius: var(--radius-control);
    border: 1px solid var(--color-border);
    background: transparent;
    color: var(--color-text-tertiary);
    font-size: 11px;
    font-family: inherit;
    cursor: pointer;
    transition:
        color var(--duration-fast) var(--ease),
        border-color var(--duration-fast) var(--ease);
}

.llm-panel__reset:hover {
    color: var(--color-text-body);
    border-color: var(--color-border-strong);
}

.llm-panel__footer {
    padding: 6px 16px 10px;
    border-top: 1px solid var(--color-border);
}

.llm-panel__hint {
    font-size: 10px;
    color: var(--color-text-tertiary);
    opacity: 0.7;
}

/* ---- Transitions ---- */

.llm-slide-enter-active,
.llm-slide-leave-active {
    transition:
        opacity var(--duration-fast) var(--ease),
        transform var(--duration-fast) var(--ease);
}

.llm-slide-enter-from,
.llm-slide-leave-to {
    opacity: 0;
    transform: translateY(6px) scale(0.96);
}

.llm-fade-enter-active,
.llm-fade-leave-active {
    transition: opacity var(--duration-fast) var(--ease);
}

.llm-fade-enter-from,
.llm-fade-leave-to {
    opacity: 0;
}

@media (prefers-reduced-motion: reduce) {
    .llm-panel__toggle--busy {
        animation: none;
    }

    .llm-panel__spinner {
        animation: none;
        opacity: 0.5;
    }
}
</style>
