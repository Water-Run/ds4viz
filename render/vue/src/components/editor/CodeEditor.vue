<script setup lang="ts">
/**
 * 代码编辑器组件
 *
 * 关键实现：
 * 1) Monaco 实例使用 shallowRef + markRaw，避免深响应式代理
 * 2) 使用 DecorationsCollection 管理行高亮，减少 deltaDecorations 抖动
 * 3) 高亮更新使用 rAF 合帧 + 同值短路，降低高频步骤切换开销
 *
 * @file src/components/editor/CodeEditor.vue
 * @author WaterRun
 * @date 2026-03-03
 * @component CodeEditor
 *
 * @example
 * ```vue
 * <CodeEditor v-model="code" language="python" :highlight-line="10" />
 * ```
 */

import { computed, markRaw, onBeforeUnmount, shallowRef, watch } from 'vue'
import { VueMonacoEditor } from '@guolao/vue-monaco-editor'
import type { editor } from 'monaco-editor'

import type { Language } from '@/types/api'
import { registerDs4vizCompletions } from '@/utils/monaco'

/**
 * 组件属性定义
 */
interface Props {
  /** 代码内容 */
  modelValue: string
  /** 当前语言 */
  language: Language
  /** 是否只读 */
  readonly?: boolean
  /** 高亮行号（null 或 <= 0 时清除） */
  highlightLine?: number | null
}

/**
 * 组件事件定义
 */
interface Emits {
  /** 代码变更 */
  (event: 'update:modelValue', value: string): void
}

const props = withDefaults(defineProps<Props>(), {
  readonly: false,
  highlightLine: null,
})
const emit = defineEmits<Emits>()

/**
 * Monaco 编辑器实例（raw）
 */
const editorRef = shallowRef<editor.IStandaloneCodeEditor | null>(null)

/**
 * Monaco 模块实例（raw）
 */
const monacoRef = shallowRef<typeof import('monaco-editor') | null>(null)

/**
 * 行高亮装饰集合（raw）
 */
const decorationsRef = shallowRef<editor.IEditorDecorationsCollection | null>(null)

/**
 * 上次已应用的高亮行号（用于同值短路）
 */
const lastAppliedLine = shallowRef<number | null>(null)

/**
 * 待应用的高亮行号
 */
const pendingLine = shallowRef<number | null>(null)

/**
 * requestAnimationFrame 句柄
 */
const rafId = shallowRef<number | null>(null)

/**
 * Monaco 语言映射
 */
const editorLanguage = computed<string>(() => {
  if (props.language === 'python') return 'python'
  if (props.language === 'lua') return 'lua'
  if (props.language === 'rust') return 'rust'
  return 'plaintext'
})

/**
 * Monaco 编辑器静态配置
 */
const EDITOR_MINIMAP = { enabled: false } as const
const EDITOR_SCROLLBAR = {
  verticalScrollbarSize: 6,
  horizontalScrollbarSize: 6,
} as const

/**
 * Monaco 编辑器配置
 */
const editorOptions = computed(() => ({
  fontFamily: 'JetBrains Mono, ui-monospace, SFMono-Regular, Menlo, monospace',
  fontSize: 13,
  lineHeight: 20,
  minimap: EDITOR_MINIMAP,
  scrollBeyondLastLine: false,
  wordWrap: 'on',
  lineNumbers: 'on',
  renderLineHighlight: 'all',
  readOnly: props.readonly,
  tabSize: 4,
  automaticLayout: true,
  scrollbar: EDITOR_SCROLLBAR,
}))

/**
 * 处理编辑器内容变更
 *
 * @param value - Monaco 返回的新值
 */
const handleChange = (value: string | undefined): void => {
  emit('update:modelValue', value ?? '')
}

/**
 * 取消已调度的高亮更新帧
 */
const cancelScheduledHighlight = (): void => {
  if (rafId.value !== null) {
    window.cancelAnimationFrame(rafId.value)
    rafId.value = null
  }
}

/**
 * 立即应用高亮（无调度）
 *
 * @param requestedLine - 请求高亮行号
 */
const applyHighlightNow = (requestedLine: number | null): void => {
  const editorInstance = editorRef.value
  const monacoInstance = monacoRef.value
  const collection = decorationsRef.value
  if (!editorInstance || !monacoInstance || !collection) return

  if (requestedLine === null || requestedLine <= 0) {
    if (lastAppliedLine.value !== null) {
      collection.clear()
      lastAppliedLine.value = null
    }
    return
  }

  const model = editorInstance.getModel()
  if (!model) return

  const safeLine = Math.min(Math.max(1, requestedLine), model.getLineCount())
  if (lastAppliedLine.value === safeLine) return

  collection.set([
    {
      range: new monacoInstance.Range(safeLine, 1, safeLine, 1),
      options: {
        isWholeLine: true,
        className: 'ds4viz-line-highlight',
      },
    },
  ])

  lastAppliedLine.value = safeLine
}

/**
 * 调度高亮应用（rAF 合帧）
 *
 * @param line - 待高亮行号
 */
const scheduleHighlight = (line: number | null): void => {
  pendingLine.value = line
  if (rafId.value !== null) return

  rafId.value = window.requestAnimationFrame(() => {
    rafId.value = null
    applyHighlightNow(pendingLine.value)
  })
}

/**
 * 处理编辑器挂载
 *
 * @param editorInstance - Monaco 编辑器实例
 * @param monacoInstance - Monaco 模块实例
 */
const handleMount = (
  editorInstance: editor.IStandaloneCodeEditor,
  monacoInstance: typeof import('monaco-editor'),
): void => {
  editorRef.value = markRaw(editorInstance)
  monacoRef.value = markRaw(monacoInstance)

  decorationsRef.value = markRaw(editorInstance.createDecorationsCollection())

  registerDs4vizCompletions(monacoInstance)

  lastAppliedLine.value = null
  scheduleHighlight(props.highlightLine ?? null)
}

/**
 * 监听高亮行变化
 */
watch(
  () => props.highlightLine,
  (line) => {
    scheduleHighlight(line ?? null)
  },
)

/**
 * 生命周期：卸载清理
 */
onBeforeUnmount(() => {
  cancelScheduledHighlight()
  decorationsRef.value?.clear()
  decorationsRef.value = null
  monacoRef.value = null
  editorRef.value = null
  lastAppliedLine.value = null
  pendingLine.value = null
})
</script>

<template>
  <div class="code-editor">
    <VueMonacoEditor :value="modelValue" class="code-editor__instance" :language="editorLanguage" :theme="'vs'"
      :options="editorOptions" @mount="handleMount" @change="handleChange" />
  </div>
</template>

<style scoped>
.code-editor {
  display: flex;
  flex: 1;
  min-height: 0;
  border: 1px solid var(--color-border);
  border-radius: var(--radius-lg);
  background-color: var(--color-bg-surface);
  overflow: hidden;
}

.code-editor__instance {
  flex: 1;
  min-height: 0;
}

:deep(.monaco-editor) {
  background: var(--color-bg-surface);
}

:deep(.monaco-editor .margin) {
  background: var(--color-bg-surface);
}

:deep(.monaco-editor .cursor) {
  border-left-color: var(--color-accent);
}

:deep(.monaco-editor .current-line) {
  border: none;
  background: rgba(0, 0, 0, 0.03);
}

:deep(.monaco-editor .ds4viz-line-highlight) {
  background: rgba(14, 116, 144, 0.12);
}
</style>
