<script setup lang="ts">
/**
 * 代码编辑器组件
 *
 * @component CodeEditor
 * @example
 * ```vue
 * <CodeEditor v-model="code" language="python" />
 * ```
 */

import { computed, onBeforeUnmount, ref, watch } from 'vue'
import { VueMonacoEditor } from '@guolao/vue-monaco-editor'
import type { editor } from 'monaco-editor'

import type { Language } from '@/types/api'

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
  /** 高亮行号 */
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
 *
 * 提升为模块级常量，避免每次 computed 求值产生新的嵌套对象引用。
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
 */
const applyHighlight = (line: number | null | undefined): void => {
  if (!editorRef.value || !monacoRef.value) return
  if (!line || line < 1) {
    decorationIds.value = editorRef.value.deltaDecorations(decorationIds.value, [])
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
  applyHighlight(props.highlightLine)
}

watch(
  () => props.highlightLine,
  (line) => applyHighlight(line),
)

onBeforeUnmount(() => {
  /*
   * 不手动调用 editorRef.value.dispose()。
   * VueMonacoEditor 包装组件在自身 onBeforeUnmount 中管理 Monaco 实例的销毁，
   * 若此处提前 dispose 会导致子组件清理时访问已销毁实例，引发页面冻结。
   */
  editorRef.value = null
  monacoRef.value = null
  decorationIds.value = []
})
</script>

<template>
  <div class="code-editor">
    <VueMonacoEditor
      :value="modelValue"
      class="code-editor__instance"
      :language="editorLanguage"
      :theme="'vs'"
      :options="editorOptions"
      @mount="handleMount"
      @change="handleChange"
    />
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