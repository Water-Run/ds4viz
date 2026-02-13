<script setup lang="ts">
/**
 * 模板库页面
 *
 * 卡片列表 + 分类筛选 + 关键词搜索 + 收藏切换。
 * 点击卡片跳转到编辑器页加载对应代码。
 *
 * @component Templates
 */

import { onMounted, ref } from 'vue'
import { useRouter } from 'vue-router'
import { storeToRefs } from 'pinia'
import { useTemplatesStore } from '@/stores/templates'
import { extractErrorMessage } from '@/utils/error'
import { formatRelativeTime } from '@/utils/time'

import ErrorBanner from '@/components/common/ErrorBanner.vue'
import Pagination from '@/components/common/Pagination.vue'
import Loading from '@/components/common/Loading.vue'
import MaterialIcon from '@/components/common/MaterialIcon.vue'

const store = useTemplatesStore()
const router = useRouter()
const {
  items,
  total,
  page,
  totalPages,
  categories,
  selectedCategory,
  keyword,
  loading,
  errorMessage,
  isSearchMode,
} = storeToRefs(store)

/**
 * 搜索输入框绑定值
 */
const searchInput = ref<string>('')

/**
 * 收藏操作中的模板 ID 集合
 */
const togglingIds = ref<Set<number>>(new Set())

/**
 * 收藏操作错误提示
 */
const favoriteError = ref<string>('')

/**
 * 悬浮提示状态
 */
const hoveredId = ref<number | null>(null)

/**
 * 执行搜索
 */
async function handleSearch(): Promise<void> {
  const query = searchInput.value.trim()
  if (query.length > 0) {
    await store.search(query)
  } else {
    await store.clearSearch()
  }
}

/**
 * 清除搜索
 */
async function handleClearSearch(): Promise<void> {
  searchInput.value = ''
  await store.clearSearch()
}

/**
 * 切换收藏状态
 *
 * @param templateId - 模板 ID
 */
async function handleToggleFavorite(templateId: number): Promise<void> {
  if (togglingIds.value.has(templateId)) {
    return
  }
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
 * 分页跳转
 *
 * @param targetPage - 目标页码
 */
async function handlePageChange(targetPage: number): Promise<void> {
  await store.goToPage(targetPage)
}

/**
 * 进入编辑器
 *
 * @param templateId - 模板 ID
 */
function handleOpenTemplate(templateId: number): void {
  router.push({ name: 'playground', query: { templateId } })
}

/**
 * 初始化：加载分类列表与模板列表
 */
onMounted(async () => {
  await Promise.all([
    store.loadCategories(),
    store.loadTemplates(),
  ])
})
</script>

<template>
  <div class="templates-page">
    <!-- 顶栏 -->
    <header class="templates-page__header">
      <h1 class="templates-page__title">模板库</h1>
      <div class="templates-page__search">
        <MaterialIcon name="search" class="templates-page__search-icon" :size="18" />
        <input
          v-model="searchInput"
          type="text"
          class="templates-page__search-input"
          placeholder="搜索模板…"
          @keydown.enter="handleSearch"
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

    <!-- 错误 -->
    <ErrorBanner
      :message="errorMessage"
      @dismiss="errorMessage = ''"
    />
    <ErrorBanner
      :message="favoriteError"
      @dismiss="favoriteError = ''"
    />

    <!-- 主体 -->
    <div class="templates-page__body">
      <!-- 分类侧栏 -->
      <aside class="templates-page__sidebar">
        <h2 class="templates-page__sidebar-title">分类</h2>
        <ul class="category-list">
          <li>
            <button
              class="category-list__item"
              :class="{
                'category-list__item--active': selectedCategory.length === 0 && !isSearchMode,
              }"
              @click="store.selectCategory('')"
            >
              全部
            </button>
          </li>
          <li v-for="cat in categories" :key="cat">
            <button
              class="category-list__item"
              :class="{
                'category-list__item--active': selectedCategory === cat && !isSearchMode,
              }"
              @click="store.selectCategory(cat)"
            >
              {{ cat }}
            </button>
          </li>
        </ul>
      </aside>

      <!-- 内容区 -->
      <section class="templates-page__content">
        <!-- 搜索状态提示 -->
        <div v-if="isSearchMode" class="templates-page__search-status">
          <span class="templates-page__search-hint">
            搜索 "{{ keyword }}" 共 {{ total }} 条结果
          </span>
          <button
            class="templates-page__search-reset"
            @click="handleClearSearch"
          >
            清除搜索
          </button>
        </div>

        <!-- 加载中 -->
        <div v-if="loading" class="templates-page__loading">
          <Loading />
        </div>

        <!-- 空状态 -->
        <div
          v-else-if="items.length === 0"
          class="templates-page__empty"
        >
          <MaterialIcon name="folder_open" class="templates-page__empty-icon" :size="48" />
          <p class="templates-page__empty-text">暂无模板</p>
        </div>

        <!-- 卡片网格 -->
        <div v-else class="template-grid">
          <article
            v-for="item in items"
            :key="item.id"
            class="template-card"
            @mouseenter="hoveredId = item.id"
            @mouseleave="hoveredId = null"
            @click="handleOpenTemplate(item.id)"
          >
            <div class="template-card__header">
              <h3 class="template-card__title">
                {{ item.title }}
              </h3>
              <span class="template-card__category">
                {{ item.category }}
              </span>
            </div>
            <p
              class="template-card__desc"
              :title="hoveredId === item.id ? item.description : undefined"
            >
              {{ item.description }}
            </p>
            <div class="template-card__footer">
              <span class="template-card__time">
                {{ formatRelativeTime(item.createdAt) }}
              </span>
              <div class="template-card__actions">
                <button
                  class="template-card__fav"
                  :class="{
                    'template-card__fav--active': item.isFavorited,
                  }"
                  :disabled="togglingIds.has(item.id)"
                  :aria-label="item.isFavorited ? '取消收藏' : '收藏'"
                  @click.stop="handleToggleFavorite(item.id)"
                >
                  <MaterialIcon
                    :name="item.isFavorited ? 'favorite' : 'favorite_border'"
                    :size="18"
                  />
                  <span class="template-card__fav-count">
                    {{ item.favoriteCount }}
                  </span>
                </button>
              </div>
            </div>
          </article>
        </div>

        <!-- 分页 -->
        <div v-if="!loading && items.length > 0" class="templates-page__pagination">
          <Pagination
            :page="page"
            :total-pages="totalPages"
            @change="handlePageChange"
          />
        </div>
      </section>
    </div>
  </div>
</template>

<style scoped>
/* ============================================================
 *  模板库页面
 * ============================================================ */

.templates-page {
  display: flex;
  flex-direction: column;
  height: 100%;
  padding: var(--space-3);
  gap: var(--space-2);
  overflow: hidden;
}

/* ---- 顶栏 ---- */

.templates-page__header {
  display: flex;
  align-items: center;
  justify-content: space-between;
  flex-shrink: 0;
  gap: var(--space-2);
}

.templates-page__title {
  font-size: var(--text-lg);
  font-weight: var(--weight-semibold);
  color: var(--color-text-primary);
  margin: 0;
  white-space: nowrap;
}

.templates-page__search {
  position: relative;
  display: flex;
  align-items: center;
  width: 280px;
  max-width: 100%;
  flex-shrink: 0;
}

.templates-page__search-icon {
  position: absolute;
  left: 10px;
  width: 18px;
  height: 18px;
  color: var(--color-text-tertiary);
  pointer-events: none;
}

.templates-page__search-input {
  width: 100%;
  height: var(--control-height-md);
  padding: 0 var(--space-4) 0 36px;
  border: 1px solid var(--color-border-strong);
  border-radius: var(--radius-control);
  background-color: var(--color-bg-surface);
  font-size: var(--text-sm);
  color: var(--color-text-primary);
  transition: border-color var(--duration-fast) var(--ease);
}

.templates-page__search-input:focus {
  border-color: var(--color-accent);
}

.templates-page__search-input::placeholder {
  color: var(--color-text-tertiary);
}

.templates-page__search-clear {
  position: absolute;
  right: 6px;
  display: flex;
  align-items: center;
  justify-content: center;
  width: 24px;
  height: 24px;
  border: none;
  background: none;
  color: var(--color-text-tertiary);
  border-radius: var(--radius-sm);
  cursor: pointer;
  transition: color var(--duration-fast) var(--ease);
}

.templates-page__search-clear:hover {
  color: var(--color-text-primary);
}

.templates-page__search-clear :deep(.material-icon) {
  width: 16px;
  height: 16px;
}

/* ---- 主体 ---- */

.templates-page__body {
  display: flex;
  flex: 1;
  min-height: 0;
  gap: var(--space-3);
}

/* ---- 分类侧栏 ---- */

.templates-page__sidebar {
  width: 160px;
  flex-shrink: 0;
  display: flex;
  flex-direction: column;
  gap: var(--space-1);
  overflow-y: auto;
}

.templates-page__sidebar-title {
  font-size: var(--text-xs);
  font-weight: var(--weight-semibold);
  color: var(--color-text-tertiary);
  text-transform: uppercase;
  letter-spacing: var(--tracking-panel-head);
  margin: 0;
  padding: 0 var(--space-1);
}

.category-list {
  display: flex;
  flex-direction: column;
  gap: 2px;
}

.category-list__item {
  display: block;
  width: 100%;
  padding: 6px var(--space-1);
  border: none;
  background: none;
  border-radius: var(--radius-control);
  text-align: left;
  font-size: var(--text-sm);
  font-family: inherit;
  color: var(--color-text-body);
  cursor: pointer;
  transition:
    background-color var(--duration-fast) var(--ease),
    color var(--duration-fast) var(--ease);
}

.category-list__item:hover {
  background-color: var(--color-bg-hover);
  color: var(--color-text-primary);
}

.category-list__item--active {
  background-color: var(--color-accent-wash);
  color: var(--color-accent);
}

.category-list__item--active:hover {
  background-color: var(--color-accent-wash);
}

/* ---- 内容区 ---- */

.templates-page__content {
  flex: 1;
  min-width: 0;
  display: flex;
  flex-direction: column;
  gap: var(--space-2);
  overflow-y: auto;
}

.templates-page__search-status {
  display: flex;
  align-items: center;
  gap: var(--space-1);
  flex-shrink: 0;
}

.templates-page__search-hint {
  font-size: var(--text-sm);
  color: var(--color-text-body);
}

.templates-page__search-reset {
  border: none;
  background: none;
  color: var(--color-accent);
  font-size: var(--text-sm);
  font-family: inherit;
  font-weight: var(--weight-medium);
  cursor: pointer;
  transition: color var(--duration-fast) var(--ease);
}

.templates-page__search-reset:hover {
  color: var(--color-accent-hover);
}

.templates-page__loading {
  flex: 1;
  display: flex;
  align-items: center;
  justify-content: center;
}

.templates-page__empty {
  flex: 1;
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  gap: var(--space-1);
}

.templates-page__empty-icon {
  width: 48px;
  height: 48px;
  color: var(--color-text-tertiary);
}

.templates-page__empty-text {
  margin: 0;
  font-size: var(--text-sm);
  color: var(--color-text-tertiary);
}

/* ---- 卡片网格 ---- */

.template-grid {
  display: grid;
  grid-template-columns: repeat(auto-fill, minmax(260px, 1fr));
  gap: var(--space-2);
}

/* ---- 卡片 ---- */

.template-card {
  display: flex;
  flex-direction: column;
  padding: var(--space-2);
  border: 1px solid var(--color-border);
  border-radius: var(--radius-lg);
  background-color: var(--color-bg-surface);
  box-shadow: var(--shadow-static);
  cursor: pointer;
  transition:
    box-shadow var(--duration-normal) var(--ease),
    border-color var(--duration-normal) var(--ease),
    transform var(--duration-fast) var(--ease);
}

.template-card:hover {
  box-shadow: var(--shadow-hover);
  border-color: var(--color-border-strong);
  transform: translateY(-1px);
}

.template-card__header {
  display: flex;
  align-items: baseline;
  justify-content: space-between;
  gap: var(--space-1);
  margin-bottom: var(--space-1);
}

.template-card__title {
  margin: 0;
  font-size: var(--text-base);
  font-weight: var(--weight-semibold);
  color: var(--color-text-primary);
  line-height: var(--leading-tight);
  overflow: hidden;
  text-overflow: ellipsis;
  white-space: nowrap;
  min-width: 0;
}

.template-card__category {
  flex-shrink: 0;
  padding: 2px 8px;
  border: 1px solid var(--color-border);
  border-radius: var(--radius-full);
  font-size: var(--text-xs);
  color: var(--color-text-tertiary);
  white-space: nowrap;
}

.template-card__desc {
  margin: 0;
  font-size: var(--text-sm);
  color: var(--color-text-body);
  line-height: var(--leading-normal);
  display: -webkit-box;
  -webkit-line-clamp: 3;
  -webkit-box-orient: vertical;
  overflow: hidden;
  flex: 1;
}

.template-card__desc[title] {
  cursor: help;
}

.template-card__footer {
  display: flex;
  align-items: center;
  justify-content: space-between;
  margin-top: var(--space-1);
  padding-top: var(--space-1);
  border-top: 1px solid var(--color-border);
}

.template-card__time {
  font-size: var(--text-xs);
  color: var(--color-text-tertiary);
}

.template-card__actions {
  display: flex;
  align-items: center;
}

.template-card__fav {
  display: flex;
  align-items: center;
  gap: 4px;
  border: 1px solid transparent;
  background: none;
  color: var(--color-text-tertiary);
  font-size: var(--text-xs);
  font-family: inherit;
  cursor: pointer;
  padding: 4px 6px;
  border-radius: var(--radius-sm);
  transition:
    color var(--duration-fast) var(--ease),
    background-color var(--duration-fast) var(--ease),
    border-color var(--duration-fast) var(--ease);
}

.template-card__fav:hover:not(:disabled) {
  background-color: var(--color-bg-hover);
  color: var(--color-text-primary);
  border-color: var(--color-border-strong);
}

.template-card__fav--active {
  color: var(--color-error);
}

.template-card__fav--active:hover:not(:disabled) {
  color: var(--color-error);
}

.template-card__fav :deep(.material-icon) {
  width: 18px;
  height: 18px;
}


.template-card__fav-count {
  font-variant-numeric: tabular-nums;
}

/* ---- 分页 ---- */

.templates-page__pagination {
  display: flex;
  justify-content: center;
  flex-shrink: 0;
  padding-top: var(--space-1);
}
</style>
