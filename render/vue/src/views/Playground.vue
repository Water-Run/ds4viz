<script setup lang="ts">
/**
 * 编辑器页面
 *
 * @file src/views/Playground.vue
 * @author WaterRun
 * @date 2026-02-27
 * @component Playground
 */

import { computed, onBeforeUnmount, onMounted, ref, watch } from 'vue'
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
import MaterialIcon from '@/components/common/MaterialIcon.vue'

const language = ref<Language>('python')
const code = ref<string>(getDefaultTemplate('python'))
const languageSelectKey = ref<number>(0)
const templateLoading = ref<boolean>(false)
const templateError = ref<string>('')
const running = ref<boolean>(false)
const executeError = ref<string>('')
const executionInfo = ref<string>('')
const tomlContent = ref<string>('')
const tomlExpanded = ref<boolean>(false)
const irDoc = ref<IrDocument | null>(null)
const vizModel = ref<VizModel | null>(null)
const isPlaying = ref<boolean>(false)
const playTimer = ref<number | null>(null)

const currentState = computed<IrState | null>(() => {
  if (!vizModel.value) return null
  return getStateById(vizModel.value, vizModel.value.currentStateId)
})

const stepSummary = computed(() => {
  if (!vizModel.value) return null
  return getStepSummary(vizModel.value)
})

const currentStepIndex = computed<number>(() => {
  if (!vizModel.value) return -1
  return getCurrentStepIndex(vizModel.value)
})

const totalSteps = computed<number>(() => vizModel.value?.steps.length ?? 0)

const canStepBackward = computed<boolean>(() => currentStepIndex.value > 0)

const canStepForward = computed<boolean>(() => {
  if (!vizModel.value) return false
  return currentStepIndex.value < totalSteps.value - 1
})

const handleLanguageChange = (value: Language): void => {
  if (code.value.trim().length > 0) {
    const confirmed = window.confirm('此操作会覆盖现有代码，继续吗')
    if (!confirmed) {
      languageSelectKey.value += 1
      return
    }
  }
  language.value = value
  code.value = getDefaultTemplate(value)
  localStorage.setItem('ds4viz_language', value)
}

const applyToml = (content: string): void => {
  const parsed = parseIrToml(content)
  if (!parsed.ok || !parsed.document) {
    executeError.value = parsed.errorMessage ?? 'TOML 解析失败'
    irDoc.value = null
    vizModel.value = null
    return
  }
  irDoc.value = parsed.document
  vizModel.value = buildVizModel(parsed.document)

  if (vizModel.value.steps.length > 0) {
    vizModel.value.currentStepId = vizModel.value.steps[0].id
    vizModel.value.currentStateId = getStateIdByStepIndex(vizModel.value, 0)
  }

  executeError.value = parsed.document.error?.message ?? ''
}

const handleRun = async (): Promise<void> => {
  executeError.value = ''
  executionInfo.value = ''
  running.value = true
  tomlExpanded.value = false
  stopPlay()
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

const handleUploadToml = async (event: Event): Promise<void> => {
  const input = event.target as HTMLInputElement
  const file = input.files?.[0]
  if (!file) return
  const content = await file.text()
  tomlContent.value = content
  tomlExpanded.value = false
  applyToml(content)
  stopPlay()
  input.value = ''
}

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

const stopPlay = (): void => {
  isPlaying.value = false
  if (playTimer.value !== null) {
    window.clearInterval(playTimer.value)
    playTimer.value = null
  }
}

const goFirst = (): void => {
  if (!vizModel.value || vizModel.value.steps.length === 0) return
  stopPlay()
  vizModel.value.currentStepId = vizModel.value.steps[0].id
  vizModel.value.currentStateId = getStateIdByStepIndex(vizModel.value, 0)
}

const goPrev = (): void => {
  if (!vizModel.value) return
  stopPlay()
  const index = currentStepIndex.value
  if (index <= 0) return
  const nextIndex = index - 1
  vizModel.value.currentStepId = vizModel.value.steps[nextIndex].id
  vizModel.value.currentStateId = getStateIdByStepIndex(vizModel.value, nextIndex)
}

const goNext = (): void => {
  if (!vizModel.value) return
  const index = currentStepIndex.value
  const nextIndex = index + 1
  if (nextIndex >= vizModel.value.steps.length) return
  vizModel.value.currentStepId = vizModel.value.steps[nextIndex].id
  vizModel.value.currentStateId = getStateIdByStepIndex(vizModel.value, nextIndex)
}

const goLast = (): void => {
  if (!vizModel.value || vizModel.value.steps.length === 0) return
  stopPlay()
  const lastIndex = vizModel.value.steps.length - 1
  vizModel.value.currentStepId = vizModel.value.steps[lastIndex].id
  vizModel.value.currentStateId = getStateIdByStepIndex(vizModel.value, lastIndex)
}

const togglePlay = (): void => {
  if (!vizModel.value) return
  if (isPlaying.value) {
    stopPlay()
    return
  }
  isPlaying.value = true
  playTimer.value = window.setInterval(() => {
    if (!canStepForward.value) {
      stopPlay()
      return
    }
    goNext()
  }, 220)
}

const route = useRoute()
const router = useRouter()

onMounted(() => {
  /* 从执行记录跳转：读取 sessionStorage 中的代码 */
  try {
    const editCode = sessionStorage.getItem('ds4viz_edit_code')
    const editLang = sessionStorage.getItem('ds4viz_edit_language') as Language | null
    if (editCode !== null) {
      sessionStorage.removeItem('ds4viz_edit_code')
      sessionStorage.removeItem('ds4viz_edit_language')
      if (editLang && LANGUAGES.includes(editLang)) {
        language.value = editLang
        localStorage.setItem('ds4viz_language', editLang)
      }
      code.value = editCode
      return
    }
  } catch {
    /* sessionStorage 不可用 */
  }

  const stored = localStorage.getItem('ds4viz_language') as Language | null
  if (stored && LANGUAGES.includes(stored)) {
    language.value = stored
    code.value = getDefaultTemplate(stored)
  }
})

onBeforeUnmount(() => {
  stopPlay()
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
  { immediate: true },
)
</script>

<template>
  <div class="playground">
    <header class="playground__header">
      <div class="playground__title">
        <MaterialIcon name="code" :size="18" />
        <span>编辑器</span>
      </div>
      <div class="playground__actions">
        <label class="action-btn">
          <MaterialIcon name="upload" :size="18" />
          <input class="sr-only" type="file" accept=".toml" @change="handleUploadToml" />
        </label>
        <button class="action-btn" :disabled="!tomlContent" @click="handleDownloadToml">
          <MaterialIcon name="download" :size="18" />
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
              <MaterialIcon name="first_page" :size="18" />
            </button>
            <button class="icon-btn" :disabled="!canStepBackward" @click="goPrev">
              <MaterialIcon name="chevron_left" :size="18" />
            </button>
            <span class="step-indicator">{{ currentStepIndex + 1 }} / {{ totalSteps }}</span>
            <button class="icon-btn" :disabled="!canStepForward" @click="goNext">
              <MaterialIcon name="chevron_right" :size="18" />
            </button>
            <button class="icon-btn" :disabled="!canStepForward" @click="goLast">
              <MaterialIcon name="last_page" :size="18" />
            </button>
            <button class="icon-btn" :disabled="totalSteps === 0" @click="togglePlay">
              <MaterialIcon :name="isPlaying ? 'pause' : 'play_arrow'" :size="18" />
            </button>
          </div>
        </div>
        <VizPanel
          :kind="irDoc?.object.kind"
          :data="currentState?.data"
          :step="stepSummary"
        />
        <div v-if="tomlContent" class="toml-section">
          <button class="toml-section__toggle" @click="tomlExpanded = !tomlExpanded">
            <MaterialIcon :name="tomlExpanded ? 'expand_less' : 'expand_more'" :size="16" />
            <span>TOML IR</span>
          </button>
          <Transition name="slide-fade">
            <div v-if="tomlExpanded" class="toml-section__content">
              <TomlViewer :content="tomlContent" />
            </div>
          </Transition>
        </div>
      </section>

      <section class="playground__panel playground__panel--editor">
        <div class="panel-toolbar">
          <div class="panel-toolbar__left">
            <span class="panel-toolbar__label">代码</span>
          </div>
          <div class="panel-toolbar__controls">
            <LanguageSelect
              :key="languageSelectKey"
              :model-value="language"
              @update:model-value="handleLanguageChange"
            />
            <button class="run-btn" :disabled="running || templateLoading" @click="handleRun">
              <MaterialIcon name="play_arrow" :size="18" />
              <span>{{ running ? '运行中' : '运行' }}</span>
            </button>
          </div>
        </div>
        <CodeEditor v-model="code" :language="language" :highlight-line="stepSummary?.line" />
        <div v-if="executionInfo" class="execution-info">
          <MaterialIcon name="bolt" :size="16" />
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
  flex-shrink: 0;
}

.playground__title {
  display: flex;
  align-items: center;
  gap: 6px;
  font-size: var(--text-base);
  font-weight: var(--weight-semibold);
  color: var(--color-text-primary);
}

.playground__title :deep(.material-icon) {
  width: 18px;
  height: 18px;
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

.action-btn :deep(.material-icon) {
  width: 18px;
  height: 18px;
}

.playground__body {
  display: grid;
  grid-template-columns: minmax(0, 1fr) minmax(0, 1fr);
  gap: var(--space-3);
  flex: 1;
  min-height: 0;
}

.playground__panel {
  display: flex;
  flex-direction: column;
  gap: var(--space-2);
  min-height: 0;
  min-width: 0;
  overflow: hidden;
}

.panel-toolbar {
  display: flex;
  align-items: center;
  justify-content: space-between;
  flex-shrink: 0;
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

.icon-btn :deep(.material-icon) {
  width: 18px;
  height: 18px;
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

.run-btn :deep(.material-icon) {
  width: 18px;
  height: 18px;
}

.toml-section {
  flex-shrink: 0;
  margin-top: var(--space-1);
}

.toml-section__toggle {
  display: inline-flex;
  align-items: center;
  gap: 4px;
  border: none;
  background: none;
  color: var(--color-text-tertiary);
  font-size: var(--text-xs);
  font-family: inherit;
  cursor: pointer;
  padding: 4px 0;
  transition: color var(--duration-fast) var(--ease);
}

.toml-section__toggle:hover {
  color: var(--color-text-primary);
}

.toml-section__toggle :deep(.material-icon) {
  width: 16px;
  height: 16px;
}

.toml-section__content {
  margin-top: var(--space-1);
}

.execution-info {
  display: inline-flex;
  align-items: center;
  gap: 6px;
  flex-shrink: 0;
  font-size: var(--text-xs);
  color: var(--color-text-tertiary);
}

.execution-info :deep(.material-icon) {
  width: 16px;
  height: 16px;
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
    grid-template-columns: minmax(0, 1fr);
  }
}
</style>
