<script setup lang="ts">
/**
 * 模板详情展开面板
 *
 * 共享组件，用于模板库页面和个人页面的模板详情展示。
 * 包含只读代码编辑器、描述、收藏按钮、复制/打开操作。
 *
 * @file src/components/common/TemplateDetailPanel.vue
 * @author WaterRun
 * @date 2026-03-15
 * @component TemplateDetailPanel
 */

import { computed, onMounted, ref, watch } from 'vue'
import { useRouter } from 'vue-router'

import { fetchTemplateDetailApi } from '@/api/templates'
import { extractErrorMessage } from '@/utils/error'
import { formatRelativeTime } from '@/utils/time'
import { LANGUAGE_LABELS } from '@/types/api'
import type { Language } from '@/types/api'
import type { Template, TemplateCode } from '@/types/template'

import CodeEditor from '@/components/editor/CodeEditor.vue'
import Loading from '@/components/common/Loading.vue'
import MaterialIcon from '@/components/common/MaterialIcon.vue'

/**
 * 组件属性定义
 */
interface Props {
  /** 模板 ID */
  templateId: number
  /** 默认语言（跟随编辑器） */
  defaultLanguage?: Language
}

/**
 * 组件事件定义
 */
interface Emits {
  /** 关闭面板 */
  (event: 'close'): void
  /** 收藏/取消收藏 */
  (event: 'toggle-favorite', templateId: number): void
}

const props = withDefaults(defineProps<Props>(), {
  defaultLanguage: 'python',
})
const emit = defineEmits<Emits>()

const router = useRouter()

/** 模板详情 */
const detail = ref<Template | null>(null)

/** 加载中 */
const loading = ref<boolean>(true)

/** 错误消息 */
const errorMessage = ref<string>('')

/** 当前选中语言 */
const selectedLanguage = ref<Language>(props.defaultLanguage)

/** 已复制状态 */
const copied = ref<boolean>(false)

/** 本地收藏覆盖（乐观更新） */
const localFavorited = ref<boolean | null>(null)

/**
 * 有效收藏状态
 */
const effectiveFavorited = computed<boolean>(
  () => localFavorited.value ?? detail.value?.isFavorited ?? false,
)

/**
 * 有效收藏数
 */
const effectiveFavoriteCount = computed<number>(() => {
  if (!detail.value) return 0
  if (localFavorited.value === null) return detail.value.favoriteCount
  const delta = localFavorited.value !== detail.value.isFavorited
    ? (localFavorited.value ? 1 : -1)
    : 0
  return detail.value.favoriteCount + delta
})

/**
 * 可用语言列表
 */
const availableLanguages = computed<Language[]>(() => {
  return (detail.value?.codes.map((c) => c.language) ?? []) as Language[]
})

/**
 * 语言标签（汇总）
 */
const languageLabel = computed<string>(() => {
  const codes = detail.value?.codes ?? []
  if (codes.length === 0) return '--'
  return codes
    .map((c) => LANGUAGE_LABELS[c.language as Language] ?? c.language)
    .join(' / ')
})

/**
 * 当前展示的代码块
 */
const currentCode = computed<TemplateCode | null>(() => {
  if (!detail.value) return null
  return (
    detail.value.codes.find((c) => c.language === selectedLanguage.value) ??
    detail.value.codes[0] ??
    null
  )
})

/**
 * 当前语言色点颜色
 */
const languageDotColor = computed<string>(() => {
  if (selectedLanguage.value === 'python') return 'var(--color-lang-python)'
  if (selectedLanguage.value === 'lua') return 'var(--color-lang-lua)'
  if (selectedLanguage.value === 'rust') return 'var(--color-lang-rust)'
  if (selectedLanguage.value === 'c') return 'var(--color-lang-c)'
  return 'var(--color-text-tertiary)'
})

/**
 * 编辑器高度（基于代码行数）
 */
const editorHeight = computed<string>(() => {
  if (!currentCode.value) return '120px'
  const lines = currentCode.value.code.split('\n').length
  return `${Math.min(360, Math.max(100, lines * 20 + 24))}px`
})

/**
 * 加载模板详情
 */
const loadDetail = async (): Promise<void> => {
  loading.value = true
  errorMessage.value = ''
  localFavorited.value = null
  try {
    detail.value = await fetchTemplateDetailApi(props.templateId)
    if (detail.value.codes.some((c) => c.language === props.defaultLanguage)) {
      selectedLanguage.value = props.defaultLanguage
    } else if (detail.value.codes.length > 0) {
      selectedLanguage.value = detail.value.codes[0].language as Language
    }
  } catch (error: unknown) {
    errorMessage.value = extractErrorMessage(error)
  } finally {
    loading.value = false
  }
}

/**
 * 切换语言
 */
const handleLanguageChange = (event: Event): void => {
  const target = event.target as HTMLSelectElement
  selectedLanguage.value = target.value as Language
}

/**
 * 复制代码到剪贴板
 */
const handleCopy = async (): Promise<void> => {
  if (!currentCode.value) return
  try {
    await navigator.clipboard.writeText(currentCode.value.code)
    copied.value = true
    window.setTimeout(() => {
      copied.value = false
    }, 2000)
  } catch {
    /* clipboard 不可用 */
  }
}

/**
 * 在编辑器中打开
 */
const handleOpenInEditor = (): void => {
  if (!currentCode.value) return
  try {
    sessionStorage.setItem('ds4viz_edit_code', currentCode.value.code)
    sessionStorage.setItem('ds4viz_edit_language', selectedLanguage.value)
  } catch {
    /* sessionStorage 不可用 */
  }
  emit('close')
  router.push({ name: 'playground' })
}

/**
 * 切换收藏
 */
const handleToggleFavorite = (): void => {
  if (!detail.value) return
  localFavorited.value = !effectiveFavorited.value
  emit('toggle-favorite', detail.value.id)
}

onMounted(async () => {
  await loadDetail()
})

watch(
  () => props.templateId,
  async () => {
    await loadDetail()
  },
)
</script>

<template>
  <div class="tpl-panel">
    <header class="tpl-panel__header">
      <div class="tpl-panel__title-row">
        <h3 class="tpl-panel__title">{{ detail?.title ?? '加载中' }}</h3>
        <span v-if="detail" class="tpl-panel__category">{{ detail.category }}</span>
      </div>
      <button class="tpl-panel__close" aria-label="关闭" @click="emit('close')">
        <MaterialIcon name="close" :size="18" />
      </button>
    </header>

    <Loading v-if="loading" />

    <div v-else-if="errorMessage.length > 0" class="tpl-panel__error">
      <MaterialIcon name="error" :size="16" />
      <span>{{ errorMessage }}</span>
    </div>

    <div v-else-if="detail" class="tpl-panel__body">
      <!-- 左：编辑器 -->
      <div class="tpl-panel__editor-section">
        <div class="tpl-panel__toolbar">
          <div class="tpl-panel__actions">
            <button
              class="code-action-btn"
              :class="{ 'code-action-btn--copied': copied }"
              @click="handleCopy"
            >
              <svg v-if="!copied" viewBox="0 0 24 24" fill="currentColor"><path d="M16 1H4c-1.1 0-2 .9-2 2v14h2V3h12V1zm3 4H8c-1.1 0-2 .9-2 2v14c0 1.1.9 2 2 2h11c1.1 0 2-.9 2-2V7c0-1.1-.9-2-2-2zm0 16H8V7h11v14z" /></svg>
              <svg v-else viewBox="0 0 24 24" fill="currentColor"><path d="M9 16.17L4.83 12l-1.42 1.41L9 19 21 7l-1.41-1.41z" /></svg>
              <span>{{ copied ? '已复制' : '复制' }}</span>
            </button>
            <button class="code-action-btn code-action-btn--primary" @click="handleOpenInEditor">
              <svg viewBox="0 0 24 24" fill="currentColor"><path d="M9.4 16.6L4.8 12l4.6-4.6L8 6l-6 6 6 6 1.4-1.4zm5.2 0l4.6-4.6-4.6-4.6L16 6l6 6-6 6-1.4-1.4z" /></svg>
              <span>在编辑器中打开</span>
            </button>
          </div>
          <div class="tpl-panel__lang-select">
            <span class="tpl-panel__lang-dot" :style="{ backgroundColor: languageDotColor }" />
            <select class="tpl-panel__lang-control" :value="selectedLanguage" @change="handleLanguageChange">
              <option v-for="lang in availableLanguages" :key="lang" :value="lang">
                {{ LANGUAGE_LABELS[lang] ?? lang }}
              </option>
            </select>
          </div>
        </div>
        <div class="tpl-panel__editor-wrap" :style="{ height: editorHeight }">
          <CodeEditor
            v-if="currentCode"
            :model-value="currentCode.code"
            :language="selectedLanguage"
            :readonly="true"
          />
        </div>
      </div>

      <!-- 右：信息 -->
      <div class="tpl-panel__info-section">
        <p class="tpl-panel__desc">{{ detail.description }}</p>
        <div v-if="currentCode?.explanation" class="tpl-panel__explanation">
          {{ currentCode.explanation }}
        </div>
        <div class="tpl-panel__meta-list">
          <div class="tpl-panel__meta-item">
            <span class="tpl-panel__meta-label">分类</span>
            <span>{{ detail.category }}</span>
          </div>
          <div class="tpl-panel__meta-item">
            <span class="tpl-panel__meta-label">语言</span>
            <span>{{ languageLabel }}</span>
          </div>
          <div class="tpl-panel__meta-item">
            <span class="tpl-panel__meta-label">创建</span>
            <span>{{ formatRelativeTime(detail.createdAt) }}</span>
          </div>
        </div>
        <button
          class="tpl-panel__fav-btn"
          :class="{ 'tpl-panel__fav-btn--active': effectiveFavorited }"
          @click="handleToggleFavorite"
        >
          <svg v-if="effectiveFavorited" viewBox="0 0 24 24" fill="currentColor"><path d="M12 21.35l-1.45-1.32C5.4 15.36 2 12.28 2 8.5 2 5.42 4.42 3 7.5 3c1.74 0 3.41.81 4.5 2.09C13.09 3.81 14.76 3 16.5 3 19.58 3 22 5.42 22 8.5c0 3.78-3.4 6.86-8.55 11.54L12 21.35z" /></svg>
          <svg v-else viewBox="0 0 24 24" fill="currentColor"><path d="M16.5 3c-1.74 0-3.41.81-4.5 2.09C10.91 3.81 9.24 3 7.5 3 4.42 3 2 5.42 2 8.5c0 3.78 3.4 6.86 8.55 11.54L12 21.35l1.45-1.32C18.6 15.36 22 12.28 22 8.5 22 5.42 19.58 3 16.5 3zm-4.4 15.55l-.1.1-.1-.1C7.14 14.24 4 11.39 4 8.5 4 6.5 5.5 5 7.5 5c1.54 0 3.04.99 3.57 2.36h1.87C13.46 5.99 14.96 5 16.5 5c2 0 3.5 1.5 3.5 3.5 0 2.89-3.14 5.74-7.9 10.05z" /></svg>
          <span>{{ effectiveFavorited ? '已收藏' : '收藏' }} {{ effectiveFavoriteCount }}</span>
        </button>
      </div>
    </div>
  </div>
</template>

<style scoped>
.tpl-panel {
  border: 1px solid var(--color-border);
  border-radius: var(--radius-lg);
  background-color: var(--color-bg-surface);
  overflow: hidden;
}

.tpl-panel__header {
  display: flex;
  align-items: center;
  justify-content: space-between;
  padding: var(--space-2) var(--space-3);
  border-bottom: 1px solid var(--color-border);
  background-color: var(--color-bg-surface-alt);
}

.tpl-panel__title-row {
  display: flex;
  align-items: center;
  gap: var(--space-1);
  min-width: 0;
}

.tpl-panel__title {
  margin: 0;
  font-size: var(--text-base);
  font-weight: var(--weight-semibold);
  color: var(--color-text-primary);
  overflow: hidden;
  text-overflow: ellipsis;
  white-space: nowrap;
}

.tpl-panel__category {
  flex-shrink: 0;
  padding: 2px 8px;
  border: 1px solid var(--color-border);
  border-radius: var(--radius-full);
  font-size: var(--text-xs);
  color: var(--color-text-tertiary);
}

.tpl-panel__close {
  display: flex;
  align-items: center;
  justify-content: center;
  width: var(--control-height-sm);
  height: var(--control-height-sm);
  flex-shrink: 0;
  border: none;
  background: none;
  color: var(--color-text-tertiary);
  border-radius: var(--radius-control);
  cursor: pointer;
  transition: background-color var(--duration-fast) var(--ease), color var(--duration-fast) var(--ease);
}

.tpl-panel__close:hover {
  background-color: var(--color-bg-hover);
  color: var(--color-text-primary);
}

.tpl-panel__error {
  display: flex;
  align-items: center;
  gap: var(--space-1);
  padding: var(--space-2) var(--space-3);
  color: var(--color-error);
  font-size: var(--text-sm);
}

.tpl-panel__error :deep(.material-icon) {
  width: 16px;
  height: 16px;
}

.tpl-panel__body {
  display: grid;
  grid-template-columns: 1fr 280px;
  gap: var(--space-3);
  padding: var(--space-3);
  min-height: 0;
}

.tpl-panel__editor-section {
  display: flex;
  flex-direction: column;
  gap: var(--space-1);
  min-width: 0;
}

.tpl-panel__toolbar {
  display: flex;
  align-items: center;
  justify-content: space-between;
  gap: var(--space-1);
  flex-shrink: 0;
}

.tpl-panel__actions {
  display: flex;
  align-items: center;
  gap: var(--space-1);
}

.tpl-panel__lang-select {
  display: inline-flex;
  align-items: center;
  gap: 6px;
  padding: 0 10px;
  height: var(--control-height-sm);
  border: 1px solid var(--color-border-strong);
  border-radius: var(--radius-control);
  background-color: var(--color-bg-surface);
}

.tpl-panel__lang-dot {
  width: 8px;
  height: 8px;
  border-radius: 999px;
  flex-shrink: 0;
}

.tpl-panel__lang-control {
  font-size: var(--text-sm);
  color: var(--color-text-primary);
  background: none;
  border: none;
  outline: none;
  cursor: pointer;
  font-family: inherit;
}

.tpl-panel__editor-wrap {
  display: flex;
  border-radius: var(--radius-md);
  overflow: hidden;
}

.tpl-panel__info-section {
  display: flex;
  flex-direction: column;
  gap: var(--space-2);
  overflow-y: auto;
}

.tpl-panel__desc {
  margin: 0;
  font-size: var(--text-sm);
  color: var(--color-text-body);
  line-height: var(--leading-relaxed);
}

.tpl-panel__explanation {
  font-size: var(--text-xs);
  color: var(--color-text-tertiary);
  line-height: var(--leading-relaxed);
  padding: var(--space-1);
  border-radius: var(--radius-sm);
  background-color: var(--color-bg-surface-alt);
  border: 1px solid var(--color-border);
}

.tpl-panel__meta-list {
  display: flex;
  flex-direction: column;
  gap: 6px;
}

.tpl-panel__meta-item {
  display: flex;
  align-items: center;
  gap: var(--space-1);
  font-size: var(--text-xs);
  color: var(--color-text-body);
}

.tpl-panel__meta-label {
  color: var(--color-text-tertiary);
  min-width: 36px;
}

.tpl-panel__fav-btn {
  display: inline-flex;
  align-items: center;
  gap: 6px;
  height: var(--control-height-md);
  padding: 0 14px;
  border: 1px solid var(--color-border-strong);
  border-radius: var(--radius-control);
  background-color: var(--color-bg-surface);
  color: var(--color-text-body);
  font-size: var(--text-sm);
  font-family: inherit;
  cursor: pointer;
  transition: background-color var(--duration-fast) var(--ease), border-color var(--duration-fast) var(--ease), color var(--duration-fast) var(--ease);
}

.tpl-panel__fav-btn svg {
  width: 18px;
  height: 18px;
  flex-shrink: 0;
}

.tpl-panel__fav-btn:hover {
  background-color: var(--color-bg-hover);
  border-color: var(--color-border-strong);
}

.tpl-panel__fav-btn--active {
  color: var(--color-error);
  border-color: var(--color-error);
}

.tpl-panel__fav-btn--active:hover {
  background-color: var(--color-error-muted);
}

/* ---- 统一操作按钮 ---- */

.code-action-btn {
  display: inline-flex;
  align-items: center;
  gap: 4px;
  height: var(--control-height-sm);
  padding: 0 10px;
  border: 1px solid var(--color-border-strong);
  border-radius: var(--radius-control);
  background-color: var(--color-bg-surface);
  color: var(--color-text-body);
  font-size: var(--text-xs);
  font-family: inherit;
  cursor: pointer;
  transition: background-color var(--duration-fast) var(--ease), border-color var(--duration-fast) var(--ease), color var(--duration-fast) var(--ease);
}

.code-action-btn:hover {
  background-color: var(--color-bg-hover);
  color: var(--color-text-primary);
}

.code-action-btn svg {
  width: 16px;
  height: 16px;
  flex-shrink: 0;
}

.code-action-btn--copied {
  border-color: var(--color-success);
  color: var(--color-success);
}

.code-action-btn--primary {
  border-color: var(--color-accent);
  background-color: var(--color-accent);
  color: var(--color-accent-contrast);
}

.code-action-btn--primary:hover {
  background-color: var(--color-accent-hover);
  border-color: var(--color-accent-hover);
}

@media (max-width: 860px) {
  .tpl-panel__body {
    grid-template-columns: 1fr;
  }
}
</style>
