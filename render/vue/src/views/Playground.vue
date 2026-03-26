<!-- src/views/Playground.vue -->
<script setup lang="ts">
/**
 * 编辑器页面
 *
 * 集成可视化 flag：skipReadyPage / autoPlayOnGenerate / playbackInterval。
 * 关键优化：
 * - 步骤摘要改为按 state 索引的 O(1) 查找，避免播放中反复线性扫描
 *
 * @file src/views/Playground.vue
 * @author WaterRun
 * @date 2026-03-26
 * @component Playground
 */

import { computed, onBeforeUnmount, onMounted, ref, shallowRef, watch } from 'vue'
import { useRoute, useRouter } from 'vue-router'

import { executeCodeApi } from '@/api/executions'
import { fetchTemplateCodeApi } from '@/api/templates'
import { extractErrorMessage } from '@/utils/error'
import { formatDuration } from '@/utils/time'
import { parseIrToml } from '@/utils/ir'
import { getStateByIndex, computePhaseSegments } from '@/utils/viz'
import { getDefaultTemplate } from '@/utils/editor-templates'
import { vizFlags, logDebug } from '@/utils/viz-flags'
import { LANGUAGES } from '@/types/api'
import type { Language } from '@/types/api'
import type { IrDocument, IrStep } from '@/types/ir'
import type { StepSummary, PhaseSegment } from '@/types/viz'

import CodeEditor from '@/components/editor/CodeEditor.vue'
import LanguageSelect from '@/components/editor/LanguageSelect.vue'
import VizPanel from '@/components/viz/VizPanel.vue'
import TomlViewer from '@/components/viz/TomlViewer.vue'
import ErrorBanner from '@/components/common/ErrorBanner.vue'
import MaterialIcon from '@/components/common/MaterialIcon.vue'

/* ---- 编辑器状态 ---- */

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
/**
* 面板模式
*
* split: 默认左右等分
* viz-full: 可视化占全宽，编辑器折叠为窄条
* editor-full: 编辑器占全宽，可视化折叠为窄条
*/
const panelMode = ref<'split' | 'viz-full' | 'editor-full'>('split')

/**
 * 面板模式对应的 body CSS 类
 */
const bodyModeClass = computed<string>(() => {
  if (panelMode.value === 'viz-full') return 'playground__body--viz-full'
  if (panelMode.value === 'editor-full') return 'playground__body--editor-full'
  return ''
})

/**
 * 展开可视化面板
 */
const expandViz = (): void => {
  panelMode.value = 'viz-full'
}

/**
 * 展开编辑器面板
 */
const expandEditor = (): void => {
  panelMode.value = 'editor-full'
}

/**
 * 恢复分屏模式
 */
const restoreSplit = (): void => {
  panelMode.value = 'split'
}

/* ---- 可视化状态 ---- */

const irDoc = shallowRef<IrDocument | null>(null)
const currentStateIndex = ref<number>(0)
const isPlaying = ref<boolean>(false)
const playTimer = ref<number | null>(null)
const vizStarted = ref<boolean>(false)

const stepSummaryByStateIndex = shallowRef<Map<number, StepSummary>>(new Map())

/* ---- 派生计算 ---- */

const isDefaultCode = computed<boolean>(() => {
  const trimmed = code.value.trim()
  return trimmed.length === 0 || trimmed === getDefaultTemplate(language.value).trim()
})

const totalStates = computed<number>(() => irDoc.value?.states.length ?? 0)

const currentState = computed(() => {
  if (!irDoc.value) return null
  return getStateByIndex(irDoc.value, currentStateIndex.value)
})

const currentStepInfo = computed<StepSummary | null>(() => {
  return stepSummaryByStateIndex.value.get(currentStateIndex.value) ?? null
})

const highlightLine = computed<number | null>(() => {
  if (!vizFlags.enableCodeLineHighlight) return null
  return currentStepInfo.value?.line ?? null
})

/**
 * 阶段段落列表
 */
const phaseSegments = computed<PhaseSegment[]>(() => {
  if (!irDoc.value) return []
  return computePhaseSegments(irDoc.value)
})

/**
 * 当前所处阶段索引
 */
const currentPhaseIndex = computed<number | null>(() => {
  if (phaseSegments.value.length === 0) return null
  const stateIdx = currentStateIndex.value
  for (let i = 0; i < phaseSegments.value.length; i += 1) {
    const seg = phaseSegments.value[i]
    if (stateIdx >= seg.targetStateIndex && stateIdx <= seg.endStateIndex) {
      return i
    }
  }
  return null
})

const canStepBackward = computed<boolean>(() => currentStateIndex.value > 0)
const canStepForward = computed<boolean>(() => currentStateIndex.value < totalStates.value - 1)

const showVizReady = computed<boolean>(() => {
  return (
    irDoc.value !== null
    && !vizStarted.value
    && !vizFlags.skipReadyPage
    && totalStates.value > 1
    && currentStateIndex.value === 0
  )
})

/* ---- 步骤索引 ---- */

function mapStepToSummary(step: IrStep): StepSummary {
  return {
    op: step.op,
    line: step.code?.line,
    note: step.note,
    args: step.args as Record<string, unknown>,
    ret: step.ret,
  }
}

function rebuildStepSummaryIndex(doc: IrDocument | null): void {
  const map = new Map<number, StepSummary>()
  if (doc?.steps) {
    for (const step of doc.steps) {
      if (step.after !== undefined) {
        map.set(step.after, mapStepToSummary(step))
      }
    }
  }
  stepSummaryByStateIndex.value = map
}

/* ---- TOML 解析 ---- */

const applyToml = (content: string): void => {
  const parsed = parseIrToml(content)

  if (!parsed.ok || !parsed.document) {
    executeError.value = parsed.errorMessage ?? 'TOML 解析失败'
    irDoc.value = null
    stepSummaryByStateIndex.value = new Map()
    currentStateIndex.value = 0
    vizStarted.value = false
    return
  }

  irDoc.value = parsed.document
  rebuildStepSummaryIndex(parsed.document)
  currentStateIndex.value = 0
  vizStarted.value = false
  executeError.value = parsed.document.error?.message ?? ''

  logDebug('[playground] TOML applied, states =', parsed.document.states.length)

  if (vizFlags.skipReadyPage) {
    vizStarted.value = true
  }

  if (vizFlags.autoPlayOnGenerate && totalStates.value > 1) {
    vizStarted.value = true
    startPlay()
  }
}

/* ---- 语言与模板 ---- */

const handleLanguageChange = (value: Language): void => {
  if (!isDefaultCode.value) {
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

const loadTemplate = async (templateId: number): Promise<void> => {
  if (templateLoading.value) return

  templateError.value = ''

  if (!isDefaultCode.value) {
    const confirmed = window.confirm('此操作会覆盖现有代码，继续吗')
    if (!confirmed) return
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

/* ---- 执行 ---- */

const handleRun = async (): Promise<void> => {
  executeError.value = ''
  executionInfo.value = ''
  running.value = true
  tomlExpanded.value = false
  stopPlay()

  logDebug('[playground] run', language.value)

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

/* ---- TOML 文件操作 ---- */

const handleUploadToml = async (event: Event): Promise<void> => {
  const input = event.target as HTMLInputElement
  const file = input.files?.[0]
  if (!file) return

  const content = await file.text()
  tomlContent.value = content
  tomlExpanded.value = false
  stopPlay()
  applyToml(content)
  input.value = ''
}

const handleDownloadToml = (): void => {
  if (!tomlContent.value) return

  const blob = new Blob([tomlContent.value], {
    type: 'text/plain;charset=utf-8',
  })
  const url = URL.createObjectURL(blob)

  const link = document.createElement('a')
  link.href = url
  link.download = 'ds4viz.toml'
  document.body.appendChild(link)
  link.click()
  document.body.removeChild(link)

  URL.revokeObjectURL(url)
}

/* ---- 步骤导航 ---- */

const goFirst = (): void => {
  stopPlay()
  currentStateIndex.value = 0
}

const goPrev = (): void => {
  stopPlay()
  if (currentStateIndex.value > 0) {
    currentStateIndex.value -= 1
  }
}

const goNext = (): void => {
  if (currentStateIndex.value < totalStates.value - 1) {
    currentStateIndex.value += 1
  }
}

const goLast = (): void => {
  stopPlay()
  if (totalStates.value > 0) {
    currentStateIndex.value = totalStates.value - 1
  }
}

/**
 * 跳转至指定状态（由 VizPanel 阶段导航触发）
 *
 * @param stateIndex - 目标状态索引
 */
const handleJumpToState = (stateIndex: number): void => {
  stopPlay()
  if (stateIndex >= 0 && stateIndex < totalStates.value) {
    currentStateIndex.value = stateIndex
  }
}

/* ---- 自动播放 ---- */

const stopPlay = (): void => {
  isPlaying.value = false
  if (playTimer.value !== null) {
    window.clearInterval(playTimer.value)
    playTimer.value = null
  }
}

const startPlay = (): void => {
  if (!irDoc.value || totalStates.value <= 1) return

  isPlaying.value = true
  const interval = Math.max(50, vizFlags.playbackInterval)

  playTimer.value = window.setInterval(() => {
    if (!canStepForward.value) {
      stopPlay()
      return
    }
    goNext()
  }, interval)
}

const togglePlay = (): void => {
  if (isPlaying.value) {
    stopPlay()
    return
  }
  startPlay()
}

/* ---- 就绪页 ---- */

const handleStartNext = (): void => {
  vizStarted.value = true
  goNext()
}

const handleStartPlay = (): void => {
  vizStarted.value = true
  startPlay()
}

/* ---- 播放间隔热更新 ---- */

watch(
  () => vizFlags.playbackInterval,
  () => {
    if (isPlaying.value) {
      stopPlay()
      startPlay()
    }
  },
)

/* ---- 生命周期 ---- */

const route = useRoute()
const router = useRouter()

onMounted(() => {
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

watch(currentStateIndex, (next, prev) => {
  if (next !== prev && next > 0) {
    vizStarted.value = true
  }
})
</script>

<template>
  <div class="playground">
    <header class="playground__header">
      <div class="playground__title">
        <MaterialIcon name="code" :size="18" />
        <span>编辑器</span>
      </div>
      <div class="playground__actions">
        <label class="action-btn" title="上传 IR TOML">
          <MaterialIcon name="upload" :size="18" />
          <input class="sr-only" type="file" accept=".toml" @change="handleUploadToml" />
        </label>
        <button class="action-btn" title="下载 IR TOML" :disabled="!tomlContent" @click="handleDownloadToml">
          <MaterialIcon name="download" :size="18" />
        </button>
      </div>
    </header>

    <ErrorBanner :message="executeError" @dismiss="executeError = ''" />
    <ErrorBanner :message="templateError" @dismiss="templateError = ''" />

    <div class="playground__body" :class="bodyModeClass">
      <!-- 左列：可视化 -->
      <div class="playground__col">
        <section class="playground__panel playground__panel--viz"
          :class="{ 'playground__panel--collapsed': panelMode === 'editor-full' }">
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
              <span class="step-indicator">
                {{ totalStates > 0 ? currentStateIndex + 1 : 0 }} / {{ totalStates }}
              </span>
              <button class="icon-btn" :disabled="!canStepForward" @click="goNext">
                <MaterialIcon name="chevron_right" :size="18" />
              </button>
              <button class="icon-btn" :disabled="!canStepForward" @click="goLast">
                <MaterialIcon name="last_page" :size="18" />
              </button>
              <button class="icon-btn" :disabled="totalStates <= 1" @click="togglePlay">
                <MaterialIcon :name="isPlaying ? 'pause' : 'play_arrow'" :size="18" />
              </button>
            </div>
          </div>

          <div class="playground__panel-content">
            <Transition name="fade" mode="out-in">
              <div v-if="showVizReady" key="ready" class="viz-ready">
                <MaterialIcon name="play_arrow" :size="40" class="viz-ready__icon" />
                <p class="viz-ready__title">可视化结果已生成</p>
                <p class="viz-ready__desc">共 {{ totalStates }} 个状态快照</p>
                <div class="viz-ready__actions">
                  <button class="viz-ready__btn" @click="handleStartNext">
                    <MaterialIcon name="chevron_right" :size="18" />
                    <span>下一步</span>
                  </button>
                  <button class="viz-ready__btn viz-ready__btn--primary" @click="handleStartPlay">
                    <MaterialIcon name="play_arrow" :size="18" />
                    <span>自动播放</span>
                  </button>
                </div>
              </div>

              <VizPanel v-else key="viz" :kind="irDoc?.object.kind" :data="currentState?.data" :step="currentStepInfo"
                :label="irDoc?.object.label" :remarks="irDoc?.remarks" :meta="irDoc?.meta" :ir-package="irDoc?.package"
                :auto-playing="isPlaying" :phases="phaseSegments" :current-phase-index="currentPhaseIndex"
                @jump-to-state="handleJumpToState" />
            </Transition>

            <button class="playground__edge-trigger playground__edge-trigger--right"
              :title="panelMode === 'viz-full' ? '恢复分屏' : '展开可视化'"
              @click="panelMode === 'viz-full' ? restoreSplit() : expandViz()">
              <svg width="8" height="14" viewBox="0 0 8 14" fill="none" stroke="currentColor" stroke-width="1.5"
                stroke-linecap="round" stroke-linejoin="round">
                <polyline v-if="panelMode === 'split'" points="2,1 6,7 2,13" />
                <polyline v-else points="6,1 2,7 6,13" />
              </svg>
            </button>
          </div>

          <div v-if="tomlContent" class="toml-section">
            <button class="toml-section__toggle" @click="tomlExpanded = !tomlExpanded">
              <MaterialIcon name="expand_more" :size="16" class="toml-section__chevron"
                :class="{ 'toml-section__chevron--open': tomlExpanded }" />
              <MaterialIcon name="data_object" :size="16" />
              <span>TOML IR</span>
            </button>
            <Transition name="slide-fade">
              <div v-if="tomlExpanded" class="toml-section__content">
                <TomlViewer :content="tomlContent" />
              </div>
            </Transition>
          </div>
        </section>
        <button class="playground__collapsed-strip"
          :class="{ 'playground__collapsed-strip--active': panelMode === 'editor-full' }" aria-label="点击展开可视化"
          @click="restoreSplit">
          <MaterialIcon name="graph_3" :size="18" />
          <span class="playground__collapsed-text">点击展开可视化</span>
        </button>
      </div>

      <!-- 右列：编辑器 -->
      <div class="playground__col">
        <section class="playground__panel playground__panel--editor"
          :class="{ 'playground__panel--collapsed': panelMode === 'viz-full' }">
          <div class="panel-toolbar">
            <div class="panel-toolbar__left">
              <span class="panel-toolbar__label">代码</span>
            </div>
            <div class="panel-toolbar__controls">
              <LanguageSelect :key="languageSelectKey" :model-value="language"
                @update:model-value="handleLanguageChange" />
              <button class="run-btn" :disabled="running || templateLoading" @click="handleRun">
                <MaterialIcon name="play_arrow" :size="18" />
                <span>{{ running ? '运行中' : '运行' }}</span>
              </button>
            </div>
          </div>

          <div class="playground__panel-content">
            <CodeEditor v-model="code" :language="language" :highlight-line="highlightLine" />

            <button class="playground__edge-trigger playground__edge-trigger--left"
              :title="panelMode === 'editor-full' ? '恢复分屏' : '展开编辑器'"
              @click="panelMode === 'editor-full' ? restoreSplit() : expandEditor()">
              <svg width="8" height="14" viewBox="0 0 8 14" fill="none" stroke="currentColor" stroke-width="1.5"
                stroke-linecap="round" stroke-linejoin="round">
                <polyline v-if="panelMode === 'split'" points="6,1 2,7 6,13" />
                <polyline v-else points="2,1 6,7 2,13" />
              </svg>
            </button>
          </div>

          <div v-if="executionInfo" class="execution-info">
            <MaterialIcon name="bolt" :size="16" />
            <span>{{ executionInfo }}</span>
          </div>
        </section>
        <button class="playground__collapsed-strip"
          :class="{ 'playground__collapsed-strip--active': panelMode === 'viz-full' }" aria-label="点击展开编辑器"
          @click="restoreSplit">
          <MaterialIcon name="code" :size="18" />
          <span class="playground__collapsed-text">点击展开编辑器</span>
        </button>
      </div>
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
  display: flex;
  gap: var(--space-3);
  flex: 1;
  min-height: 0;
}

.playground__col {
  position: relative;
  display: flex;
  flex-direction: column;
  flex: 1 1 0%;
  min-height: 0;
  min-width: 0;
  overflow: hidden;
  border-radius: var(--radius-lg);
  transition: flex 0.4s cubic-bezier(0.22, 0.61, 0.36, 1);
}

.playground__panel {
  position: relative;
  display: flex;
  flex-direction: column;
  gap: var(--space-2);
  flex: 1;
  min-height: 0;
  min-width: 0;
  overflow: hidden;
  opacity: 1;
  transition: opacity 0.25s cubic-bezier(0.22, 0.61, 0.36, 1) 0.15s;
}

.playground__panel--collapsed {
  opacity: 0;
  pointer-events: none;
  transition: opacity 0.15s cubic-bezier(0.4, 0, 1, 1);
}

.playground__panel-content {
  position: relative;
  flex: 1;
  min-height: 0;
  display: flex;
  flex-direction: column;
  overflow: hidden;
  border-radius: var(--radius-lg);
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
  font-variant-numeric: tabular-nums;
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

.viz-ready {
  display: flex;
  align-items: center;
  justify-content: center;
  flex-direction: column;
  gap: var(--space-1);
  flex: 1;
  min-height: 0;
  border: 1px dashed var(--color-border);
  border-radius: var(--radius-lg);
  background-color: var(--color-bg-surface-alt);
  color: var(--color-text-tertiary);
}

.viz-ready__icon {
  width: 40px;
  height: 40px;
  color: var(--color-accent);
}

.viz-ready__title {
  margin: 0;
  font-size: var(--text-sm);
  font-weight: var(--weight-medium);
  color: var(--color-text-primary);
}

.viz-ready__desc {
  margin: 0;
  font-size: var(--text-xs);
}

.viz-ready__actions {
  display: flex;
  align-items: center;
  gap: var(--space-1);
  margin-top: var(--space-1);
}

.viz-ready__btn {
  display: inline-flex;
  align-items: center;
  gap: 4px;
  height: var(--control-height-sm);
  padding: 0 12px;
  border: 1px solid var(--color-border-strong);
  border-radius: var(--radius-control);
  background-color: var(--color-bg-surface);
  color: var(--color-text-body);
  font-size: var(--text-xs);
  font-family: inherit;
  cursor: pointer;
  transition:
    background-color var(--duration-fast) var(--ease),
    border-color var(--duration-fast) var(--ease);
}

.viz-ready__btn:hover {
  background-color: var(--color-bg-hover);
}

.viz-ready__btn :deep(.material-icon) {
  width: 18px;
  height: 18px;
}

.viz-ready__btn--primary {
  border-color: var(--color-accent);
  background-color: var(--color-accent);
  color: var(--color-accent-contrast);
}

.viz-ready__btn--primary:hover {
  background-color: var(--color-accent-hover);
  border-color: var(--color-accent-hover);
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

.toml-section__chevron {
  transition: transform var(--duration-fast) var(--ease);
}

.toml-section__chevron--open {
  transform: rotate(180deg);
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

/* ---- 面板展开/折叠 ---- */

.playground__body--viz-full>.playground__col:last-child {
  flex: 0 0 48px;
}

.playground__body--editor-full>.playground__col:first-child {
  flex: 0 0 48px;
}

.playground__collapsed-strip {
  position: absolute;
  inset: 0;
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  gap: var(--space-1);
  padding: var(--space-2) 0;
  border: 1px solid var(--color-border);
  border-radius: var(--radius-lg);
  background-color: var(--color-bg-surface);
  color: var(--color-text-tertiary);
  cursor: pointer;
  font-family: inherit;
  opacity: 0;
  pointer-events: none;
  z-index: 2;
  transition:
    opacity 0.15s cubic-bezier(0.4, 0, 1, 1),
    background-color var(--duration-fast) var(--ease),
    color var(--duration-fast) var(--ease),
    border-color var(--duration-fast) var(--ease);
}

.playground__collapsed-strip--active {
  opacity: 1;
  pointer-events: auto;
  transition:
    opacity 0.25s cubic-bezier(0.22, 0.61, 0.36, 1) 0.15s,
    background-color var(--duration-fast) var(--ease),
    color var(--duration-fast) var(--ease),
    border-color var(--duration-fast) var(--ease);
}

.playground__collapsed-strip:hover {
  background-color: var(--color-bg-hover);
  color: var(--color-text-primary);
  border-color: var(--color-border-strong);
}

.playground__collapsed-strip:focus-visible {
  outline: 2px solid var(--color-border-focus);
  outline-offset: 2px;
}

.playground__collapsed-strip :deep(.material-icon) {
  width: 18px;
  height: 18px;
}

.playground__collapsed-text {
  writing-mode: vertical-rl;
  font-size: var(--text-xs);
  font-weight: var(--weight-medium);
  letter-spacing: 0.06em;
  white-space: nowrap;
  user-select: none;
}

/* ---- 边缘触发条 ---- */

.playground__edge-trigger {
  position: absolute;
  top: 0;
  bottom: 0;
  width: 20px;
  display: flex;
  align-items: center;
  justify-content: center;
  background-color: transparent;
  border: none;
  cursor: pointer;
  color: var(--color-text-tertiary);
  opacity: 0.15;
  padding: 0;
  z-index: 5;
  transition:
    opacity var(--duration-fast) var(--ease),
    background-color var(--duration-fast) var(--ease);
}

.playground__panel-content:hover>.playground__edge-trigger {
  opacity: 0.6;
}

.playground__edge-trigger:hover {
  opacity: 1 !important;
  background-color: var(--color-bg-hover);
}

.playground__edge-trigger:focus-visible {
  opacity: 1;
  outline: 2px solid var(--color-border-focus);
  outline-offset: -2px;
}

.playground__edge-trigger--right {
  right: 0;
  border-left: 1px solid var(--color-border);
}

.playground__edge-trigger--left {
  left: 0;
  border-right: 1px solid var(--color-border);
}

@media (max-width: 1100px) {
  .playground__body {
    flex-direction: column;
  }

  .playground__col {
    transition: none;
  }

  .playground__edge-trigger,
  .playground__collapsed-strip {
    display: none;
  }
}
</style>
