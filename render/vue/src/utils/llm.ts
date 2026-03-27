/**
 * LLM 集成配置管理与 OpenAI 兼容 API 调用
 *
 * 配置持久化到 localStorage，调用遵循 OpenAI Chat Completions 范式。
 *
 * @file src/utils/llm.ts
 * @author WaterRun
 * @date 2026-03-27
 */

import { reactive, watch } from 'vue'

/**
 * LLM 配置
 *
 * @interface
 */
export interface LlmConfig {
    /** API 地址 */
    apiUrl: string
    /** 模型名称 */
    model: string
    /** API Key */
    apiKey: string
    /** 请求超时（秒） */
    timeout: number
    /** 最大 Token 数 */
    maxTokens: number
}

/** localStorage 键名 */
const STORAGE_KEY = 'ds4viz_llm_config'

/**
 * 默认配置
 */
const DEFAULT_CONFIG: LlmConfig = {
    apiUrl: 'https://api.poe.com/v1',
    model: 'claude-opus-4.6',
    apiKey: '',
    timeout: 150,
    maxTokens: 20480,
}

/**
 * 从 localStorage 恢复配置
 *
 * @returns 存储的部分配置
 */
function loadFromStorage(): Partial<LlmConfig> {
    try {
        const raw = localStorage.getItem(STORAGE_KEY)
        if (raw !== null) {
            return JSON.parse(raw) as Partial<LlmConfig>
        }
    } catch {
        /* localStorage 不可用或数据损坏 */
    }
    return {}
}

/**
 * 持久化配置到 localStorage
 *
 * @param config - 完整配置对象
 */
function saveToStorage(config: LlmConfig): void {
    try {
        localStorage.setItem(STORAGE_KEY, JSON.stringify(config))
    } catch {
        /* localStorage 不可用 */
    }
}

const stored = loadFromStorage()

/** 响应式配置对象 */
export const llmConfig = reactive<LlmConfig>({ ...DEFAULT_CONFIG, ...stored })

/** 配置变更时自动持久化 */
watch(llmConfig, (v) => saveToStorage({ ...v }), { deep: true })

/**
 * 重置配置到默认值
 */
export function resetLlmConfig(): void {
    Object.assign(llmConfig, DEFAULT_CONFIG)
}

/**
 * LLM 调用结果
 *
 * @interface
 */
export interface LlmCallResult {
    /** 是否成功 */
    ok: boolean
    /** 生成内容 */
    content?: string
    /** 错误消息 */
    errorMessage?: string
}

/**
 * 从 LLM 响应中提取纯代码
 *
 * 若响应被 Markdown 代码块包裹则自动去除。
 *
 * @param raw - 原始响应文本
 * @returns 纯代码文本
 */
function extractCode(raw: string): string {
    const trimmed = raw.trim()
    const fenceMatch = trimmed.match(/^```(?:\w*)\n([\s\S]*?)```\s*$/)
    if (fenceMatch?.[1] !== undefined) {
        return fenceMatch[1].trim()
    }
    return trimmed
}

/**
 * 调用 LLM 生成代码
 *
 * @param systemPrompt - 系统提示
 * @param userMessage - 用户消息
 * @returns 调用结果
 */
export async function callLlm(
    systemPrompt: string,
    userMessage: string,
): Promise<LlmCallResult> {
    if (llmConfig.apiKey.length === 0) {
        return { ok: false, errorMessage: '未配置 API Key' }
    }

    if (userMessage.trim().length === 0) {
        return { ok: false, errorMessage: '提示内容不能为空' }
    }

    const controller = new AbortController()
    const timer = window.setTimeout(
        () => controller.abort(),
        llmConfig.timeout * 1000,
    )

    try {
        const url = llmConfig.apiUrl.replace(/\/+$/, '') + '/chat/completions'

        const response = await fetch(url, {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json',
                Authorization: `Bearer ${llmConfig.apiKey}`,
            },
            body: JSON.stringify({
                model: llmConfig.model,
                max_tokens: llmConfig.maxTokens,
                messages: [
                    { role: 'system', content: systemPrompt },
                    { role: 'user', content: userMessage },
                ],
            }),
            signal: controller.signal,
        })

        window.clearTimeout(timer)

        if (!response.ok) {
            let msg = `HTTP ${response.status}`
            try {
                const body: unknown = await response.json()
                if (typeof body === 'object' && body !== null && 'error' in body) {
                    const err = (body as { error: unknown }).error
                    if (typeof err === 'object' && err !== null && 'message' in err) {
                        msg = String((err as { message: unknown }).message)
                    } else if (typeof err === 'string') {
                        msg = err
                    }
                }
            } catch {
                /* 使用状态码回退 */
            }
            return { ok: false, errorMessage: msg }
        }

        const data = (await response.json()) as {
            choices?: Array<{ message?: { content?: string } }>
        }

        const content = data.choices?.[0]?.message?.content
        if (typeof content !== 'string' || content.length === 0) {
            return { ok: false, errorMessage: '响应内容为空' }
        }

        return { ok: true, content: extractCode(content) }
    } catch (error: unknown) {
        window.clearTimeout(timer)
        if (error instanceof DOMException && error.name === 'AbortError') {
            return { ok: false, errorMessage: `请求超时 (${llmConfig.timeout}s)` }
        }
        return {
            ok: false,
            errorMessage: error instanceof Error ? error.message : '请求失败',
        }
    }
}