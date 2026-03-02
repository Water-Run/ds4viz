<script setup lang="ts">
/**
 * 代码编辑器组件
 *
 * 行高亮由父组件通过 highlightLine prop 控制，
 * null 或 ≤0 时清除高亮。
 *
 * @component CodeEditor
 * @example
 * ```vue
 * <CodeEditor v-model="code" language="python" :highlight-line="10" />
 * ```
 */

import { computed, onBeforeUnmount, ref, watch } from 'vue'
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
  /** 高亮行号（null 时不高亮） */
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
 * 编辑器实例
 */
const editorRef = ref<editor.IStandaloneCodeEditor | null>(null)

/**
 * Monaco 实例
 */
const monacoRef = ref<typeof import('monaco-editor') | null>(null)

/**
 * 当前高亮装饰
 */
const decorationIds = ref<string[]>([])

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
const EDITOR_SCROLLBAR = { verticalScrollbarSize: 6, horizontalScrollbarSize: 6 } as const

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
 */
const handleChange = (value: string | undefined): void => {
  emit('update:modelValue', value ?? '')
}

/**
 * 应用行高亮
 *
 * @param line - 行号，null/undefined/≤0 时清除
 */
const applyHighlight = (line: number | null | undefined): void => {
  if (!editorRef.value || !monacoRef.value) return

  if (!line || line < 1) {
    if (decorationIds.value.length > 0) {
      decorationIds.value = editorRef.value.deltaDecorations(decorationIds.value, [])
    }
    return
  }

  const range = new monacoRef.value.Range(line, 1, line, 1)
  decorationIds.value = editorRef.value.deltaDecorations(decorationIds.value, [
    {
      range,
      options: {
        isWholeLine: true,
        className: 'ds4viz-line-highlight',
      },
    },
  ])
}

/**
 * 处理编辑器挂载
 */
const handleMount = (
  editorInstance: editor.IStandaloneCodeEditor,
  monacoInstance: typeof import('monaco-editor'),
): void => {
  editorRef.value = editorInstance
  monacoRef.value = monacoInstance
  registerDs4vizCompletions(monacoInstance)
  applyHighlight(props.highlightLine)
}

/**
 * 监听 highlightLine 变化
 */
watch(() => props.highlightLine, (line) => applyHighlight(line))

onBeforeUnmount(() => {
  editorRef.value = null
  monacoRef.value = null
  decorationIds.value = []
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
