<!-- src/components/editor/CodeEditor.vue -->
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

interface Props {
  modelValue: string
  language: Language
  readonly?: boolean
  highlightLine?: number | null
}

interface Emits {
  (event: 'update:modelValue', value: string): void
}

const props = withDefaults(defineProps<Props>(), {
  readonly: false,
  highlightLine: null,
})
const emit = defineEmits<Emits>()

const editorRef = shallowRef<editor.IStandaloneCodeEditor | null>(null)
const monacoRef = shallowRef<typeof import('monaco-editor') | null>(null)
const decorationsRef = shallowRef<editor.IEditorDecorationsCollection | null>(null)
const lastAppliedLine = shallowRef<number | null>(null)
const pendingLine = shallowRef<number | null>(null)
const rafId = shallowRef<number | null>(null)

const editorLanguage = computed<string>(() => {
  if (props.language === 'python') return 'python'
  if (props.language === 'lua') return 'lua'
  if (props.language === 'rust') return 'rust'
  return 'plaintext'
})

const EDITOR_MINIMAP = { enabled: false } as const
const EDITOR_SCROLLBAR = {
  verticalScrollbarSize: 6,
  horizontalScrollbarSize: 6,
} as const

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

const handleChange = (value: string | undefined): void => {
  emit('update:modelValue', value ?? '')
}

const cancelScheduledHighlight = (): void => {
  if (rafId.value !== null) {
    window.cancelAnimationFrame(rafId.value)
    rafId.value = null
  }
}

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

const scheduleHighlight = (line: number | null): void => {
  pendingLine.value = line
  if (rafId.value !== null) return

  rafId.value = window.requestAnimationFrame(() => {
    rafId.value = null
    applyHighlightNow(pendingLine.value)
  })
}

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

watch(
  () => props.highlightLine,
  (line) => {
    scheduleHighlight(line ?? null)
  },
)

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

/* 代码行高亮改为橙色 */
:deep(.monaco-editor .ds4viz-line-highlight) {
  background: rgba(234, 88, 12, 0.14);
}
</style>
