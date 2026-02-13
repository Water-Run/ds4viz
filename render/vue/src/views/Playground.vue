<script setup lang="ts">
/**
 * 编辑器页面
 *
 * @component 编辑器
 */

import { computed, onMounted, ref, watch } from 'vue'
import { useRoute, useRouter } from 'vue-router'

import { executeCodeApi } from '@/api/executions'
import { fetchTemplateCodeApi } from '@/api/templates'
import { extractErrorMessage } from '@/utils/error'
import { formatDuration } from '@/utils/time'
import { parseIrToml } from '@/utils/ir'
import {
  buildVizModel,
  getCurrentStepIndex,
  getStateById,
  getStepSummary,
  getStateIdByStepIndex,
} from '@/utils/viz'
import { getDefaultTemplate } from '@/utils/editor-templates'
import { LANGUAGES } from '@/types/api'
import type { Language } from '@/types/api'
import type { IrDocument, IrState } from '@/types/ir'
import type { VizModel } from '@/types/viz'

import CodeEditor from '@/components/editor/CodeEditor.vue'
import LanguageSelect from '@/components/editor/LanguageSelect.vue'
import VizPanel from '@/components/viz/VizPanel.vue'
import TomlViewer from '@/components/viz/TomlViewer.vue'
import ErrorBanner from '@/components/common/ErrorBanner.vue'

/**
 * 语言状态
 */
const language = ref<Language>('python')

/**
 * 代码内容
 */
const code = ref<string>(getDefaultTemplate('python'))

/**
 * 模板加载中
 */
const templateLoading = ref<boolean>(false)

/**
 * 模板错误提示
 */
const templateError = ref<string>('')

/**
 * 执行状态
 */
const running = ref<boolean>(false)

/**
 * 执行错误信息
 */
const executeError = ref<string>('')

/**
 * 执行结果信息
 */
const executionInfo = ref<string>('')

/**
 * TOML 内容
 */
const tomlContent = ref<string>('')

/**
 * IR 文档
 */
const document = ref<IrDocument | null>(null)

/**
 * 可视化模型
 */
const vizModel = ref<VizModel | null>(null)

/**
 * 自动播放状态
 */
const isPlaying = ref<boolean>(false)

/**
 * 自动播放定时器
 */
const playTimer = ref<number | null>(null)

/**
 * 当前状态
 */
const currentState = computed<IrState | null>(() => {
  if (!vizModel.value) return null
  return getStateById(vizModel.value, vizModel.value.currentStateId)
})

/**
 * 当前步骤摘要
 */
const stepSummary = computed(() => {
  if (!vizModel.value) return null
  return getStepSummary(vizModel.value)
})

/**
 * 当前步骤索引
 */
const currentStepIndex = computed<number>(() => {
  if (!vizModel.value) return -1
  return getCurrentStepIndex(vizModel.value)
})

/**
 * 步骤总数
 */
const totalSteps = computed<number>(() => vizModel.value?.steps.length ?? 0)

/**
 * 是否可后退
 */
const canStepBackward = computed<boolean>(() => currentStepIndex.value > 0)

/**
 * 是否可前进
 */
const canStepForward = computed<boolean>(() => {
  if (!vizModel.value) return false
  return currentStepIndex.value < totalSteps.value - 1
})

/**
 * 处理语言切换
 */
const handleLanguageChange = (value: Language): void => {
  if (code.value.trim().length > 0) {
    const confirmed = window.confirm('此操作会覆盖现有代码，继续吗')
    if (!confirmed) {
      return
    }
  }
  language.value = value
  code.value = getDefaultTemplate(value)
  localStorage.setItem('ds4viz_language', value)
}

/**
 * 解析 TOML 内容
 */
const applyToml = (content: string): void => {
  const parsed = parseIrToml(content)
  if (!parsed.ok || !parsed.document) {
    executeError.value = parsed.errorMessage ?? 'TOML 解析失败'
    document.value = null
    vizModel.value = null
    return
  }
  document.value = parsed.document
  vizModel.value = buildVizModel(parsed.document)
}

/**
 * 执行代码
 */
const handleRun = async (): Promise<void> => {
  executeError.value = ''
  executionInfo.value = ''
  running.value = true
  try {
    const result = await executeCodeApi(language.value, code.value)
    if (result.tomlOutput) {
      tomlContent.value = result.tomlOutput
      applyToml(result.tomlOutput)
    }
    const statusLabel = result.status
    const timeLabel = result.executionTime !== null ? formatDuration(result.executionTime) : '--'
    executionInfo.value = `${statusLabel} / ${timeLabel}`
    if (result.errorMessage) {
      executeError.value = result.errorMessage
    }
  } catch (error: unknown) {
    executeError.value = extractErrorMessage(error)
  } finally {
    running.value = false
  }
}

/**
 * 加载模板代码
 *
 * @param templateId - 模板 ID
 */
const loadTemplate = async (templateId: number): Promise<void> => {
  if (templateLoading.value) return
  templateError.value = ''
  if (code.value.trim().length > 0) {
    const confirmed = window.confirm('此操作会覆盖现有代码，继续吗')
    if (!confirmed) {
      return
    }
  }
  templateLoading.value = true
  try {
    const result = await fetchTemplateCodeApi(templateId, language.value)
    code.value = result.code
    executionInfo.value = `已加载模板 #${templateId}`
  } catch (error: unknown) {
    templateError.value = extractErrorMessage(error)
  } finally {
    templateLoading.value = false
  }
}

/**
 * 上传 TOML
 */
const handleUploadToml = async (event: Event): Promise<void> => {
  const input = event.target as HTMLInputElement
  const file = input.files?.[0]
  if (!file) return
  const content = await file.text()
  tomlContent.value = content
  applyToml(content)
  input.value = ''
}

/**
 * 下载 TOML
 */
const handleDownloadToml = (): void => {
  if (!tomlContent.value) return
  const blob = new Blob([tomlContent.value], { type: 'text/plain;charset=utf-8' })
  const url = URL.createObjectURL(blob)
  const link = document.createElement('a')
  link.href = url
  link.download = 'ds4viz.toml'
  document.body.appendChild(link)
  link.click()
  document.body.removeChild(link)
  URL.revokeObjectURL(url)
}

/**
 * 跳转到第一步
 */
const goFirst = (): void => {
  if (!vizModel.value || vizModel.value.steps.length === 0) return
  vizModel.value.currentStepId = vizModel.value.steps[0].id
  vizModel.value.currentStateId = getStateIdByStepIndex(vizModel.value, 0)
}

/**
 * 上一步
 */
const goPrev = (): void => {
  if (!vizModel.value) return
  const index = currentStepIndex.value
  if (index <= 0) return
  const nextIndex = index - 1
  vizModel.value.currentStepId = vizModel.value.steps[nextIndex].id
  vizModel.value.currentStateId = getStateIdByStepIndex(vizModel.value, nextIndex)
}

/**
 * 下一步
 */
const goNext = (): void => {
  if (!vizModel.value) return
  const index = currentStepIndex.value
  const nextIndex = index + 1
  if (nextIndex >= vizModel.value.steps.length) return
  vizModel.value.currentStepId = vizModel.value.steps[nextIndex].id
  vizModel.value.currentStateId = getStateIdByStepIndex(vizModel.value, nextIndex)
}

/**
 * 跳转到最后一步
 */
const goLast = (): void => {
  if (!vizModel.value || vizModel.value.steps.length === 0) return
  const lastIndex = vizModel.value.steps.length - 1
  vizModel.value.currentStepId = vizModel.value.steps[lastIndex].id
  vizModel.value.currentStateId = getStateIdByStepIndex(vizModel.value, lastIndex)
}

/**
 * 切换自动播放
 */
const togglePlay = (): void => {
  if (!vizModel.value) return
  if (isPlaying.value) {
    isPlaying.value = false
    if (playTimer.value !== null) {
      window.clearInterval(playTimer.value)
      playTimer.value = null
    }
    return
  }
  isPlaying.value = true
  playTimer.value = window.setInterval(() => {
    if (!canStepForward.value) {
      togglePlay()
      return
    }
    goNext()
  }, 240)
}

/**
 * 路由状态
 */
const route = useRoute()
const router = useRouter()

onMounted(() => {
  const stored = localStorage.getItem('ds4viz_language') as Language | null
  if (stored && LANGUAGES.includes(stored)) {
    language.value = stored
    code.value = getDefaultTemplate(stored)
  }
})

watch(
  () => route.query.templateId,
  async (value) => {
    if (!value) return
    const id = Number(value)
    if (Number.isNaN(id)) return
    await loadTemplate(id)
    const query = { ...route.query }
    delete query.templateId
    router.replace({ query })
  },
)
</script>

<template>
  <div class="playground">
    <header class="playground__header">
      <div class="playground__title">
        <span class="material-symbols-outlined">code</span>
        <span>编辑器</span>
      </div>
      <div class="playground__actions">
        <label class="action-btn">
          <span class="material-symbols-outlined">upload</span>
          <input class="sr-only" type="file" accept=".toml" @change="handleUploadToml" />
        </label>
        <button class="action-btn" :disabled="!tomlContent" @click="handleDownloadToml">
          <span class="material-symbols-outlined">download</span>
        </button>
      </div>
    </header>

    <ErrorBanner :message="executeError" @dismiss="executeError = ''" />
    <ErrorBanner :message="templateError" @dismiss="templateError = ''" />

    <div class="playground__body">
      <section class="playground__panel playground__panel--viz">
        <div class="panel-toolbar">
          <div class="panel-toolbar__left">
            <span class="panel-toolbar__label">可视化</span>
          </div>
          <div class="panel-toolbar__controls">
            <button class="icon-btn" :disabled="!canStepBackward" @click="goFirst">
              <span class="material-symbols-outlined">first_page</span>
            </button>
            <button class="icon-btn" :disabled="!canStepBackward" @click="goPrev">
              <span class="material-symbols-outlined">chevron_left</span>
            </button>
            <span class="step-indicator">{{ currentStepIndex + 1 }} / {{ totalSteps }}</span>
            <button class="icon-btn" :disabled="!canStepForward" @click="goNext">
              <span class="material-symbols-outlined">chevron_right</span>
            </button>
            <button class="icon-btn" :disabled="!canStepForward" @click="goLast">
              <span class="material-symbols-outlined">last_page</span>
            </button>
            <button class="icon-btn" :disabled="totalSteps === 0" @click="togglePlay">
              <span class="material-symbols-outlined">
                {{ isPlaying ? 'pause' : 'play_arrow' }}
              </span>
            </button>
          </div>
        </div>
        <VizPanel
          :kind="document?.object.kind"
          :data="currentState?.data"
          :step="stepSummary"
        />
        <div v-if="tomlContent" class="toml-view">
          <TomlViewer :content="tomlContent" />
        </div>
      </section>

      <section class="playground__panel playground__panel--editor">
        <div class="panel-toolbar">
          <div class="panel-toolbar__left">
            <span class="panel-toolbar__label">代码</span>
          </div>
          <div class="panel-toolbar__controls">
            <LanguageSelect v-model="language" @update:model-value="handleLanguageChange" />
            <button class="run-btn" :disabled="running || templateLoading" @click="handleRun">
              <span class="material-symbols-outlined">play_arrow</span>
              <span>{{ running ? '运行中' : '运行' }}</span>
            </button>
          </div>
        </div>
        <CodeEditor v-model="code" :language="language" :highlight-line="stepSummary?.line" />
        <div v-if="executionInfo" class="execution-info">
          <span class="material-symbols-outlined">bolt</span>
          <span>{{ executionInfo }}</span>
        </div>
      </section>
    </div>
  </div>
</template>

<style scoped>
.playground {
  display: flex;
  flex-direction: column;
  gap: var(--space-2);
  padding: var(--space-3);
  height: 100%;
  overflow: hidden;
}

.playground__header {
  display: flex;
  align-items: center;
  justify-content: space-between;
}

.playground__title {
  display: flex;
  align-items: center;
  gap: 6px;
  font-size: var(--text-base);
  font-weight: var(--weight-semibold);
  color: var(--color-text-primary);
}

.playground__actions {
  display: flex;
  align-items: center;
  gap: var(--space-1);
}

.action-btn {
  display: inline-flex;
  align-items: center;
  justify-content: center;
  width: var(--control-height-md);
  height: var(--control-height-md);
  border: 1px solid var(--color-border-strong);
  border-radius: var(--radius-control);
  background-color: var(--color-bg-surface);
  color: var(--color-text-body);
  cursor: pointer;
  transition:
    background-color var(--duration-fast) var(--ease),
    border-color var(--duration-fast) var(--ease);
}

.action-btn:hover {
  background-color: var(--color-bg-hover);
  border-color: var(--color-border-strong);
}

.playground__body {
  display: grid;
  grid-template-columns: 1fr 1fr;
  gap: var(--space-3);
  flex: 1;
  min-height: 0;
}

.playground__panel {
  display: flex;
  flex-direction: column;
  gap: var(--space-2);
  min-height: 0;
}

.panel-toolbar {
  display: flex;
  align-items: center;
  justify-content: space-between;
}

.panel-toolbar__label {
  font-size: var(--text-xs);
  letter-spacing: var(--tracking-panel-head);
  text-transform: uppercase;
  color: var(--color-text-tertiary);
}

.panel-toolbar__controls {
  display: inline-flex;
  align-items: center;
  gap: var(--space-1);
}

.icon-btn {
  width: var(--control-height-sm);
  height: var(--control-height-sm);
  display: inline-flex;
  align-items: center;
  justify-content: center;
  border: 1px solid var(--color-border-strong);
  border-radius: var(--radius-control);
  background-color: var(--color-bg-surface);
  cursor: pointer;
  transition:
    background-color var(--duration-fast) var(--ease),
    border-color var(--duration-fast) var(--ease);
}

.icon-btn:hover {
  background-color: var(--color-bg-hover);
  border-color: var(--color-border-strong);
}

.icon-btn:disabled {
  opacity: 0.4;
  cursor: not-allowed;
}

.step-indicator {
  min-width: 72px;
  text-align: center;
  font-size: var(--text-xs);
  color: var(--color-text-tertiary);
}

.run-btn {
  display: inline-flex;
  align-items: center;
  gap: 6px;
  height: var(--control-height-sm);
  padding: 0 12px;
  border-radius: var(--radius-control);
  border: 1px solid var(--color-accent);
  background-color: var(--color-accent);
  color: var(--color-accent-contrast);
  font-size: var(--text-sm);
  font-weight: var(--weight-medium);
  transition:
    background-color var(--duration-fast) var(--ease),
    transform var(--duration-fast) var(--ease);
}

.run-btn:hover {
  background-color: var(--color-accent-hover);
  transform: translateY(-1px);
}

.run-btn:disabled {
  background-color: var(--color-border-strong);
  border-color: var(--color-border-strong);
  color: var(--color-text-tertiary);
  cursor: not-allowed;
  transform: none;
}

.toml-view {
  margin-top: var(--space-1);
}

.execution-info {
  display: inline-flex;
  align-items: center;
  gap: 6px;
  font-size: var(--text-xs);
  color: var(--color-text-tertiary);
}

.sr-only {
  position: absolute;
  width: 1px;
  height: 1px;
  padding: 0;
  margin: -1px;
  overflow: hidden;
  clip: rect(0, 0, 0, 0);
  white-space: nowrap;
  border: 0;
}

@media (max-width: 1100px) {
  .playground__body {
    grid-template-columns: 1fr;
  }
}
</style>
