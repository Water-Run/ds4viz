<script setup lang="ts">
/**
 * 模板库页面
 *
 * 卡片列表 + 分类筛选 + 实时搜索 + 收藏切换 + 无限滚动。
 * 点击卡片展开详情面板查看代码与信息。
 *
 * @file src/views/Templates.vue
 * @author WaterRun
 * @date 2026-02-27
 * @component Templates
 */

import { computed, onBeforeUnmount, onMounted, ref, watchEffect } from 'vue'
import { storeToRefs } from 'pinia'
import { useTemplatesStore } from '@/stores/templates'
import { extractErrorMessage } from '@/utils/error'
import { formatRelativeTime } from '@/utils/time'
import { LANGUAGES } from '@/types/api'
import type { Language } from '@/types/api'

import ErrorBanner from '@/components/common/ErrorBanner.vue'
import Loading from '@/components/common/Loading.vue'
import MaterialIcon from '@/components/common/MaterialIcon.vue'
import TemplateDetailPanel from '@/components/common/TemplateDetailPanel.vue'

const store = useTemplatesStore()
const {
  items,
  total,
  categories,
  selectedCategory,
  keyword,
  loading,
  errorMessage,
  isSearchMode,
  hasMore,
} = storeToRefs(store)

/** 搜索输入框绑定值 */
const searchInput = ref<string>('')

/** 中文输入法组合状态 */
const isComposing = ref<boolean>(false)

/** 搜索防抖定时器 */
let searchTimer: number | null = null

/** IntersectionObserver 实例 */
let observer: IntersectionObserver | null = null

/** 无限滚动哨兵元素 */
const sentinel = ref<HTMLElement | null>(null)

/** 内容区容器引用 */
const contentRef = ref<HTMLElement | null>(null)

/** 收藏操作中的模板 ID 集合 */
const togglingIds = ref<Set<number>>(new Set())

/** 收藏操作错误提示 */
const favoriteError = ref<string>('')

/** 当前展开的模板 ID */
const selectedTemplateId = ref<number | null>(null)

/**
 * 编辑器当前语言
 */
const currentPlaygroundLanguage = computed<Language>(() => {
  const stored = localStorage.getItem('ds4viz_language') as Language | null
  if (stored && LANGUAGES.includes(stored)) return stored
  return 'python'
})

/** 我收藏的模板 */
const favoritedItems = computed(() => items.value.filter((item) => item.isFavorited))

/** 其他模板 */
const otherItems = computed(() => items.value.filter((item) => !item.isFavorited))

/** 是否存在已收藏项 */
const hasFavorited = computed<boolean>(() => favoritedItems.value.length > 0)

/** 内容切换动画 key */
const contentKey = computed<string>(() => `${selectedCategory.value}/${keyword.value}`)

/** 空状态文案 */
const emptyMessage = computed<string>(() => {
  if (selectedCategory.value.length > 0 || isSearchMode.value) {
    return '指定条件下无符合的模板内容'
  }
  return '暂无模板'
})

/** 滚动内容区到顶部 */
const scrollToTop = (): void => {
  contentRef.value?.scrollTo({ top: 0 })
}

/** 防抖搜索 */
const debouncedSearch = (): void => {
  if (searchTimer !== null) window.clearTimeout(searchTimer)
  searchTimer = window.setTimeout(async () => {
    const query = searchInput.value.trim()
    if (query.length > 0) {
      await store.search(query)
    } else {
      await store.clearSearch()
    }
    scrollToTop()
  }, 300)
}

const handleInput = (): void => {
  if (isComposing.value) return
  debouncedSearch()
}

const handleCompositionStart = (): void => { isComposing.value = true }
const handleCompositionEnd = (): void => { isComposing.value = false; debouncedSearch() }

const handleClearSearch = async (): Promise<void> => {
  searchInput.value = ''
  if (searchTimer !== null) { window.clearTimeout(searchTimer); searchTimer = null }
  await store.clearSearch()
  scrollToTop()
}

/**
 * 切换收藏状态
 */
const handleToggleFavorite = async (templateId: number): Promise<void> => {
  if (togglingIds.value.has(templateId)) return
  togglingIds.value.add(templateId)
  favoriteError.value = ''
  try {
    await store.toggleFavorite(templateId)
  } catch (error: unknown) {
    favoriteError.value = extractErrorMessage(error)
  } finally {
    togglingIds.value.delete(templateId)
  }
}

/**
 * 展开模板详情
 */
const handleSelectTemplate = (templateId: number): void => {
  selectedTemplateId.value = templateId
  scrollToTop()
}

/**
 * 关闭详情面板
 */
const handleClosePanel = (): void => {
  selectedTemplateId.value = null
}

/**
 * 详情面板中切换收藏
 */
const handlePanelToggleFavorite = async (templateId: number): Promise<void> => {
  await handleToggleFavorite(templateId)
}

const handleSelectCategory = async (category: string): Promise<void> => {
  searchInput.value = ''
  if (searchTimer !== null) { window.clearTimeout(searchTimer); searchTimer = null }
  selectedTemplateId.value = null
  await store.selectCategory(category)
  scrollToTop()
}

onMounted(async () => {
  observer = new IntersectionObserver(
    (entries) => {
      const entry = entries[0]
      if (entry?.isIntersecting && !loading.value && hasMore.value) {
        store.appendTemplates()
      }
    },
    { rootMargin: '200px' },
  )
  await Promise.all([store.loadCategories(), store.loadTemplates()])
})

watchEffect(() => {
  const el = sentinel.value
  if (!observer) return
  observer.disconnect()
  if (el) observer.observe(el)
})

onBeforeUnmount(() => {
  observer?.disconnect()
  if (searchTimer !== null) window.clearTimeout(searchTimer)
})
</script>

<template>
  <div class="templates-page">
    <header class="templates-page__header">
      <div class="templates-page__title">
        <MaterialIcon name="dashboard" :size="18" />
        <span>模板库</span>
      </div>
      <div class="templates-page__search">
        <MaterialIcon name="search" class="templates-page__search-icon" :size="18" />
        <input
          v-model="searchInput"
          type="text"
          class="templates-page__search-input"
          placeholder="搜索模板…"
          @input="handleInput"
          @compositionstart="handleCompositionStart"
          @compositionend="handleCompositionEnd"
        />
        <button
          v-if="searchInput.length > 0"
          class="templates-page__search-clear"
          aria-label="清除搜索"
          @click="handleClearSearch"
        >
          <MaterialIcon name="close" :size="16" />
        </button>
      </div>
    </header>

    <ErrorBanner :message="errorMessage" @dismiss="errorMessage = ''" />
    <ErrorBanner :message="favoriteError" @dismiss="favoriteError = ''" />

    <div class="templates-page__body">
      <aside class="templates-page__sidebar">
        <h2 class="templates-page__sidebar-title">分类</h2>
        <ul class="category-list">
          <li>
            <button
              class="category-list__item"
              :class="{ 'category-list__item--active': selectedCategory.length === 0 && !isSearchMode }"
              @click="handleSelectCategory('')"
            >全部</button>
          </li>
          <li v-for="cat in categories" :key="cat">
            <button
              class="category-list__item"
              :class="{ 'category-list__item--active': selectedCategory === cat && !isSearchMode }"
              @click="handleSelectCategory(cat)"
            >{{ cat }}</button>
          </li>
        </ul>
      </aside>

      <section ref="contentRef" class="templates-page__content">
        <!-- 详情面板 -->
        <Transition name="slide-fade">
          <div v-if="selectedTemplateId !== null" class="templates-page__detail">
            <TemplateDetailPanel
              :template-id="selectedTemplateId"
              :default-language="currentPlaygroundLanguage"
              @close="handleClosePanel"
              @toggle-favorite="handlePanelToggleFavorite"
            />
          </div>
        </Transition>

        <!-- 搜索状态 -->
        <div v-if="isSearchMode" class="templates-page__search-status">
          <span class="templates-page__search-hint">搜索 "{{ keyword }}" 共 {{ total }} 条结果</span>
          <button class="templates-page__search-reset" @click="handleClearSearch">清除搜索</button>
        </div>

        <div v-if="loading && items.length === 0" class="templates-page__loading"><Loading /></div>

        <div v-else-if="!loading && items.length === 0" class="templates-page__empty">
          <MaterialIcon name="folder_open" class="templates-page__empty-icon" :size="48" />
          <p class="templates-page__empty-text">{{ emptyMessage }}</p>
        </div>

        <template v-if="items.length > 0">
          <Transition name="tpl-fade" mode="out-in">
            <div :key="contentKey" class="templates-page__grids">
              <div v-if="hasFavorited" class="template-section">
                <h3 class="template-section__title">我收藏的</h3>
                <div class="template-grid">
                  <article
                    v-for="item in favoritedItems"
                    :key="item.id"
                    class="template-card"
                    :class="{ 'template-card--selected': selectedTemplateId === item.id }"
                    @click="handleSelectTemplate(item.id)"
                  >
                    <div class="template-card__header">
                      <h3 class="template-card__title">{{ item.title }}</h3>
                      <span class="template-card__category">{{ item.category }}</span>
                    </div>
                    <p class="template-card__desc">{{ item.description }}</p>
                    <div class="template-card__footer">
                      <span class="template-card__time">{{ formatRelativeTime(item.createdAt) }}</span>
                      <button
                        class="template-card__fav template-card__fav--active"
                        :disabled="togglingIds.has(item.id)"
                        aria-label="取消收藏"
                        @click.stop="handleToggleFavorite(item.id)"
                      >
                        <MaterialIcon name="favorite" :size="18" />
                        <span class="template-card__fav-count">{{ item.favoriteCount }}</span>
                      </button>
                    </div>
                  </article>
                </div>
                <div class="template-section__divider" />
              </div>

              <div class="template-grid">
                <article
                  v-for="item in otherItems"
                  :key="item.id"
                  class="template-card"
                  :class="{ 'template-card--selected': selectedTemplateId === item.id }"
                  @click="handleSelectTemplate(item.id)"
                >
                  <div class="template-card__header">
                    <h3 class="template-card__title">{{ item.title }}</h3>
                    <span class="template-card__category">{{ item.category }}</span>
                  </div>
                  <p class="template-card__desc">{{ item.description }}</p>
                  <div class="template-card__footer">
                    <span class="template-card__time">{{ formatRelativeTime(item.createdAt) }}</span>
                    <button
                      class="template-card__fav"
                      :class="{ 'template-card__fav--active': item.isFavorited }"
                      :disabled="togglingIds.has(item.id)"
                      :aria-label="item.isFavorited ? '取消收藏' : '收藏'"
                      @click.stop="handleToggleFavorite(item.id)"
                    >
                      <MaterialIcon :name="item.isFavorited ? 'favorite' : 'favorite_border'" :size="18" />
                      <span class="template-card__fav-count">{{ item.favoriteCount }}</span>
                    </button>
                  </div>
                </article>
              </div>
            </div>
          </Transition>

          <div v-if="hasMore && !loading" ref="sentinel" class="sentinel" />
          <div v-if="loading" class="templates-page__loading-more"><Loading /></div>
        </template>
      </section>
    </div>
  </div>
</template>

<style scoped>
.templates-page { display: flex; flex-direction: column; height: 100%; padding: var(--space-3); gap: var(--space-2); overflow: hidden; }

.templates-page__header { display: flex; align-items: center; justify-content: space-between; flex-shrink: 0; gap: var(--space-2); }
.templates-page__title { display: flex; align-items: center; gap: 6px; font-size: var(--text-base); font-weight: var(--weight-semibold); color: var(--color-text-primary); margin: 0; white-space: nowrap; }
.templates-page__title :deep(.material-icon) { width: 18px; height: 18px; }

.templates-page__search { position: relative; display: flex; align-items: center; width: 280px; max-width: 100%; flex-shrink: 0; }
.templates-page__search-icon { position: absolute; left: 10px; width: 18px; height: 18px; color: var(--color-text-tertiary); pointer-events: none; }
.templates-page__search-input { width: 100%; height: var(--control-height-md); padding: 0 var(--space-4) 0 36px; border: 1px solid var(--color-border-strong); border-radius: var(--radius-control); background-color: var(--color-bg-surface); font-size: var(--text-sm); color: var(--color-text-primary); transition: border-color var(--duration-fast) var(--ease); }
.templates-page__search-input:focus { border-color: var(--color-accent); }
.templates-page__search-input::placeholder { color: var(--color-text-tertiary); }
.templates-page__search-clear { position: absolute; right: 6px; display: flex; align-items: center; justify-content: center; width: 24px; height: 24px; border: none; background: none; color: var(--color-text-tertiary); border-radius: var(--radius-sm); cursor: pointer; transition: color var(--duration-fast) var(--ease); }
.templates-page__search-clear:hover { color: var(--color-text-primary); }
.templates-page__search-clear :deep(.material-icon) { width: 16px; height: 16px; }

.templates-page__body { display: flex; flex: 1; min-height: 0; gap: var(--space-3); }

.templates-page__sidebar { width: 160px; flex-shrink: 0; display: flex; flex-direction: column; gap: var(--space-1); overflow-y: auto; }
.templates-page__sidebar-title { font-size: var(--text-xs); font-weight: var(--weight-semibold); color: var(--color-text-tertiary); text-transform: uppercase; letter-spacing: var(--tracking-panel-head); margin: 0; padding: 0 var(--space-1); }
.category-list { display: flex; flex-direction: column; gap: 2px; }
.category-list__item { display: block; width: 100%; padding: 6px var(--space-1); border: none; background: none; border-radius: var(--radius-control); text-align: left; font-size: var(--text-sm); font-family: inherit; color: var(--color-text-body); cursor: pointer; transition: background-color var(--duration-fast) var(--ease), color var(--duration-fast) var(--ease); }
.category-list__item:hover { background-color: var(--color-bg-hover); color: var(--color-text-primary); }
.category-list__item--active { background-color: var(--color-accent-wash); color: var(--color-accent); }
.category-list__item--active:hover { background-color: var(--color-accent-wash); }

.templates-page__content { flex: 1; min-width: 0; display: flex; flex-direction: column; gap: var(--space-2); overflow-y: auto; position: relative; isolation: isolate; }
.templates-page__detail { position: sticky; top: 0; z-index: var(--z-sticky); background-color: var(--color-bg-base); padding-bottom: var(--space-2); }
.templates-page__grids { display: flex; flex-direction: column; gap: var(--space-2); }
.templates-page__search-status { display: flex; align-items: center; gap: var(--space-1); flex-shrink: 0; }
.templates-page__search-hint { font-size: var(--text-sm); color: var(--color-text-body); }
.templates-page__search-reset { border: none; background: none; color: var(--color-accent); font-size: var(--text-sm); font-family: inherit; font-weight: var(--weight-medium); cursor: pointer; transition: color var(--duration-fast) var(--ease); }
.templates-page__search-reset:hover { color: var(--color-accent-hover); }
.templates-page__loading { flex: 1; display: flex; align-items: center; justify-content: center; }
.templates-page__loading-more { display: flex; align-items: center; justify-content: center; padding: var(--space-2) 0; flex-shrink: 0; }
.templates-page__empty { flex: 1; display: flex; flex-direction: column; align-items: center; justify-content: center; gap: var(--space-1); }
.templates-page__empty-icon { width: 48px; height: 48px; color: var(--color-text-tertiary); }
.templates-page__empty-text { margin: 0; font-size: var(--text-sm); color: var(--color-text-tertiary); }

.template-section { display: flex; flex-direction: column; gap: var(--space-2); }
.template-section__title { margin: 0; font-size: var(--text-xs); font-weight: var(--weight-semibold); color: var(--color-text-tertiary); text-transform: uppercase; letter-spacing: var(--tracking-panel-head); }
.template-section__divider { height: 1px; background-color: var(--color-border); }

.template-grid { display: grid; grid-template-columns: repeat(auto-fill, minmax(260px, 1fr)); gap: var(--space-2); }

.template-card { display: flex; flex-direction: column; padding: var(--space-2); border: 1px solid var(--color-border); border-radius: var(--radius-lg); background-color: var(--color-bg-surface); box-shadow: var(--shadow-static); cursor: pointer; transition: box-shadow var(--duration-normal) var(--ease), border-color var(--duration-normal) var(--ease), transform var(--duration-fast) var(--ease); }
.template-card:hover { box-shadow: var(--shadow-hover); border-color: var(--color-border-strong); transform: translateY(-1px); }
.template-card--selected { border-color: var(--color-accent); box-shadow: 0 0 0 2px var(--color-accent-wash); }

.template-card__header { display: flex; align-items: baseline; justify-content: space-between; gap: var(--space-1); margin-bottom: var(--space-1); }
.template-card__title { margin: 0; font-size: var(--text-base); font-weight: var(--weight-semibold); color: var(--color-text-primary); line-height: var(--leading-tight); overflow: hidden; text-overflow: ellipsis; white-space: nowrap; min-width: 0; }
.template-card__category { flex-shrink: 0; padding: 2px 8px; border: 1px solid var(--color-border); border-radius: var(--radius-full); font-size: var(--text-xs); color: var(--color-text-tertiary); white-space: nowrap; }

.template-card__desc { margin: 0; font-size: var(--text-sm); color: var(--color-text-body); line-height: var(--leading-normal); display: -webkit-box; -webkit-line-clamp: 2; -webkit-box-orient: vertical; overflow: hidden; flex: 1; }

.template-card__footer { display: flex; align-items: center; justify-content: space-between; margin-top: var(--space-1); padding-top: var(--space-1); border-top: 1px solid var(--color-border); }
.template-card__time { font-size: var(--text-xs); color: var(--color-text-tertiary); }
.template-card__fav { display: flex; align-items: center; gap: 4px; border: 1px solid transparent; background: none; color: var(--color-text-tertiary); font-size: var(--text-xs); font-family: inherit; cursor: pointer; padding: 4px 6px; border-radius: var(--radius-sm); transition: color var(--duration-fast) var(--ease), background-color var(--duration-fast) var(--ease), border-color var(--duration-fast) var(--ease); }
.template-card__fav:hover:not(:disabled) { background-color: var(--color-bg-hover); color: var(--color-text-primary); border-color: var(--color-border-strong); }
.template-card__fav--active { color: var(--color-error); }
.template-card__fav--active:hover:not(:disabled) { color: var(--color-error); }
.template-card__fav :deep(.material-icon) { width: 18px; height: 18px; }
.template-card__fav-count { font-variant-numeric: tabular-nums; }

.tpl-fade-enter-active, .tpl-fade-leave-active { transition: opacity var(--duration-normal) var(--ease), transform var(--duration-normal) var(--ease); }
.tpl-fade-enter-from { opacity: 0; transform: translateY(6px); }
.tpl-fade-leave-to { opacity: 0; transform: translateY(-4px); }

.sentinel { height: 1px; width: 100%; flex-shrink: 0; }
</style>
