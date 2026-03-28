<script setup lang="ts">
/**
 * 模板详情页面
 *
 * @component TemplateDetail
 */

import { computed, onMounted, ref } from 'vue'
import { useRoute, useRouter } from 'vue-router'

import { useTemplatesStore } from '@/stores/templates'
import { formatRelativeTime } from '@/utils/time'
import { extractErrorMessage } from '@/utils/error'
import { LANGUAGE_LABELS } from '@/types/api'
import type { Language } from '@/types/api'
import type { TemplateCode } from '@/types/template'
import ErrorBanner from '@/components/common/ErrorBanner.vue'
import Loading from '@/components/common/Loading.vue'
import MaterialIcon from '@/components/common/MaterialIcon.vue'

/**
 * 路由参数
 */
const route = useRoute()
const router = useRouter()
const store = useTemplatesStore()

/**
 * 模板 ID
 */
const templateId = computed<number>(() => Number(route.params.id))

/**
 * 错误提示
 */
const errorMessage = ref<string>('')

/**
 * 加载状态
 */
const loading = ref<boolean>(false)

/**
 * 模板详情
 */
const detail = computed(() => store.currentDetail)

/**
 * 当前选中的代码语言
 */
const selectedCodeLang = ref<string>('')

/**
 * 可用语言列表
 */
const availableLanguages = computed<string[]>(() => {
  return detail.value?.codes.map((c) => c.language) ?? []
})

/**
 * 当前展示的代码块
 */
const currentCode = computed<TemplateCode | null>(() => {
  if (!detail.value) return null
  const lang = selectedCodeLang.value
  return detail.value.codes.find((c) => c.language === lang) ?? detail.value.codes[0] ?? null
})

/**
 * 语言显示名称
 */
const languageLabel = computed<string>(() => {
  const codes = detail.value?.codes ?? []
  if (codes.length === 0) return '--'
  const labels = codes.map((code) => {
    return LANGUAGE_LABELS[code.language as keyof typeof LANGUAGE_LABELS] ?? code.language
  })
  return labels.join(' / ')
})

/**
 * 切换代码语言
 *
 * @param lang - 语言标识
 */
const handleCodeLangChange = (lang: string): void => {
  selectedCodeLang.value = lang
}

/**
 * 加载模板详情
 */
const handleLoad = async (): Promise<void> => {
  if (Number.isNaN(templateId.value)) {
    router.replace({ name: 'templates' })
    return
  }
  loading.value = true
  errorMessage.value = ''
  try {
    await store.loadDetail(templateId.value)
    if (detail.value && detail.value.codes.length > 0) {
      selectedCodeLang.value = detail.value.codes[0].language
    }
  } catch (error: unknown) {
    errorMessage.value = extractErrorMessage(error)
  } finally {
    loading.value = false
  }
}

/**
 * 打开到编辑器
 */
const handleOpenInEditor = (): void => {
  if (!detail.value) return
  router.push({ name: 'playground', query: { templateId: detail.value.id } })
}

onMounted(async () => {
  await handleLoad()
})
</script>

<template>
  <div class="template-detail">
    <header class="template-detail__header">
      <div class="template-detail__title">
        <MaterialIcon name="description" :size="18" />
        <span>模板详情</span>
      </div>
      <button class="primary-btn" :disabled="loading || !detail" @click="handleOpenInEditor">
        打开编辑器
      </button>
    </header>

    <ErrorBanner :message="errorMessage" @dismiss="errorMessage = ''" />
    <Loading v-if="loading" message="加载中" />

    <div v-if="detail" class="template-detail__content">
      <section class="detail-card">
        <div class="detail-card__title">{{ detail.title }}</div>
        <div class="detail-card__meta">
          类别 {{ detail.category }} · 创建 {{ formatRelativeTime(detail.createdAt) }}
        </div>
        <p class="detail-card__desc">{{ detail.description }}</p>
        <div class="detail-card__stats">
          <div class="stat">
            <MaterialIcon name="favorite" :size="16" />
            <span>{{ detail.favoriteCount }}</span>
          </div>
          <div class="stat">
            <MaterialIcon name="code" :size="16" />
            <span>{{ languageLabel }}</span>
          </div>
        </div>
      </section>

      <section v-if="detail.codes.length > 0" class="detail-card">
        <div class="code-section__header">
          <h3 class="card-title">代码</h3>
          <div class="code-section__langs">
            <button v-for="lang in availableLanguages" :key="lang" class="code-lang-btn"
              :class="{ 'code-lang-btn--active': selectedCodeLang === lang }" @click="handleCodeLangChange(lang)">
              {{ LANGUAGE_LABELS[lang as Language] ?? lang }}
            </button>
          </div>
        </div>
        <pre v-if="currentCode" class="code-block"><code>{{ currentCode.code }}</code></pre>
        <p v-if="currentCode?.explanation" class="code-explanation">
          {{ currentCode.explanation }}
        </p>
      </section>
    </div>
  </div>
</template>

<style scoped>
.template-detail {
  display: flex;
  flex-direction: column;
  gap: var(--space-2);
  padding: var(--space-3);
  height: 100%;
  overflow: auto;
}

.template-detail__header {
  display: flex;
  align-items: center;
  justify-content: space-between;
}

.template-detail__title {
  display: flex;
  align-items: center;
  gap: 6px;
  font-size: var(--text-base);
  font-weight: var(--weight-semibold);
  color: var(--color-text-primary);
}

.template-detail__title :deep(.material-icon) {
  width: 18px;
  height: 18px;
}

.template-detail__content {
  display: flex;
  flex-direction: column;
  gap: var(--space-2);
}

.detail-card {
  border: 1px solid var(--color-border);
  border-radius: var(--radius-lg);
  padding: var(--space-3);
  background-color: var(--color-bg-surface);
  display: flex;
  flex-direction: column;
  gap: var(--space-2);
}

.detail-card__title {
  font-size: var(--text-base);
  font-weight: var(--weight-semibold);
}

.detail-card__meta {
  font-size: var(--text-xs);
  color: var(--color-text-tertiary);
}

.detail-card__desc {
  font-size: var(--text-sm);
  color: var(--color-text-body);
  line-height: var(--leading-relaxed);
}

.detail-card__stats {
  display: flex;
  gap: var(--space-2);
}

.stat {
  display: inline-flex;
  align-items: center;
  gap: 6px;
  padding: 4px 8px;
  border-radius: var(--radius-md);
  border: 1px solid var(--color-border);
  font-size: var(--text-xs);
  color: var(--color-text-body);
}

.stat :deep(.material-icon) {
  width: 16px;
  height: 16px;
}

.card-title {
  margin: 0;
  font-size: var(--text-sm);
  font-weight: var(--weight-semibold);
}

.code-section__header {
  display: flex;
  align-items: center;
  justify-content: space-between;
}

.code-section__langs {
  display: inline-flex;
  gap: var(--space-1);
}

.code-lang-btn {
  height: var(--control-height-sm);
  padding: 0 12px;
  border-radius: var(--radius-control);
  border: 1px solid var(--color-border-strong);
  background-color: var(--color-bg-surface);
  color: var(--color-text-body);
  font-size: var(--text-xs);
  font-family: inherit;
  cursor: pointer;
  transition:
    border-color var(--duration-fast) var(--ease),
    background-color var(--duration-fast) var(--ease),
    color var(--duration-fast) var(--ease);
}

.code-lang-btn:hover {
  border-color: var(--color-accent);
}

.code-lang-btn--active {
  border-color: var(--color-accent);
  color: var(--color-accent);
  background-color: var(--color-accent-wash);
}

.code-block {
  margin: 0;
  padding: var(--space-2);
  border-radius: var(--radius-md);
  background-color: var(--color-bg-surface-alt);
  border: 1px solid var(--color-border);
  font-family: var(--font-mono);
  font-size: var(--text-xs);
  line-height: var(--leading-code);
  color: var(--color-text-body);
  overflow: auto;
  max-height: 400px;
}

.code-explanation {
  margin: 0;
  font-size: var(--text-sm);
  color: var(--color-text-tertiary);
  line-height: var(--leading-relaxed);
}

.primary-btn {
  height: var(--control-height-md);
  padding: 0 16px;
  border-radius: var(--radius-control);
  border: none;
  background-color: var(--color-accent);
  color: var(--color-accent-contrast);
  font-weight: var(--weight-medium);
  transition:
    background-color var(--duration-fast) var(--ease),
    transform var(--duration-fast) var(--ease);
}

.primary-btn:hover {
  background-color: var(--color-accent-hover);
  transform: translateY(-1px);
}

.primary-btn:disabled {
  background-color: var(--color-border-strong);
  color: var(--color-text-tertiary);
  cursor: not-allowed;
  transform: none;
}
</style>