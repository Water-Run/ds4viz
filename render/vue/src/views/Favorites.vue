<script setup lang="ts">
/**
 * 收藏页面
 *
 * @component 收藏
 */

import { computed, ref } from 'vue'

import { useAuthStore } from '@/stores/auth'
import { fetchFavoritesApi } from '@/api/users'
import { extractErrorMessage } from '@/utils/error'
import { formatDateTime } from '@/utils/time'
import Pagination from '@/components/common/Pagination.vue'
import ErrorBanner from '@/components/common/ErrorBanner.vue'
import Loading from '@/components/common/Loading.vue'

import type { FavoriteItem } from '@/api/users'

/**
 * 认证状态
 */
const authStore = useAuthStore()

/**
 * 收藏列表
 */
const favorites = ref<FavoriteItem[]>([])

/**
 * 收藏总数
 */
const total = ref<number>(0)

/**
 * 当前页码
 */
const page = ref<number>(1)

/**
 * 加载状态
 */
const loading = ref<boolean>(false)

/**
 * 错误提示
 */
const errorMessage = ref<string>('')

/**
 * 当前用户
 */
const currentUser = computed(() => authStore.currentUser)

/**
 * 总页数
 */
const totalPages = computed(() => Math.max(1, Math.ceil(total.value / 20)))

/**
 * 加载收藏
 */
const loadFavorites = async (): Promise<void> => {
  if (!currentUser.value) return
  loading.value = true
  errorMessage.value = ''
  try {
    const result = await fetchFavoritesApi(currentUser.value.id, {
      page: page.value,
      limit: 20,
    })
    favorites.value = result.items
    total.value = result.total
  } catch (error: unknown) {
    errorMessage.value = extractErrorMessage(error)
  } finally {
    loading.value = false
  }
}

/**
 * 切换分页
 *
 * @param targetPage - 目标页码
 */
const handlePageChange = async (targetPage: number): Promise<void> => {
  page.value = targetPage
  await loadFavorites()
}

loadFavorites()
</script>

<template>
  <div class="favorites-page">
    <header class="favorites-page__header">
      <div class="favorites-page__title">
        <span class="material-symbols-outlined">favorite</span>
        <span>收藏</span>
      </div>
    </header>

    <ErrorBanner :message="errorMessage" @dismiss="errorMessage = ''" />

    <Loading v-if="loading" message="加载中" />
    <div v-else class="favorites-page__content">
      <div v-if="favorites.length === 0" class="empty-state">
        <span class="material-symbols-outlined">bookmark</span>
        <p>暂无收藏</p>
      </div>
      <div v-else class="favorites-list">
        <article v-for="item in favorites" :key="item.templateId" class="favorites-item">
          <div class="favorites-item__main">
            <div class="favorites-item__title">{{ item.title }}</div>
            <div class="favorites-item__desc">{{ item.description }}</div>
            <div class="favorites-item__meta">
              分类 {{ item.category }} · 收藏数 {{ item.favoriteCount }}
            </div>
          </div>
          <div class="favorites-item__time">{{ formatDateTime(item.favoritedAt) }}</div>
        </article>
        <Pagination :page="page" :total-pages="totalPages" @change="handlePageChange" />
      </div>
    </div>
  </div>
</template>

<style scoped>
.favorites-page {
  display: flex;
  flex-direction: column;
  gap: var(--space-2);
  padding: var(--space-3);
  height: 100%;
  overflow: auto;
}

.favorites-page__header {
  display: flex;
  align-items: center;
  justify-content: space-between;
}

.favorites-page__title {
  display: flex;
  align-items: center;
  gap: 6px;
  font-size: var(--text-base);
  font-weight: var(--weight-semibold);
  color: var(--color-text-primary);
}

.favorites-page__content {
  display: flex;
  flex-direction: column;
  gap: var(--space-2);
}

.favorites-list {
  display: flex;
  flex-direction: column;
  gap: var(--space-1);
}

.favorites-item {
  display: flex;
  justify-content: space-between;
  gap: var(--space-2);
  padding: var(--space-2);
  border: 1px solid var(--color-border);
  border-radius: var(--radius-md);
  background-color: var(--color-bg-surface);
  transition:
    border-color var(--duration-fast) var(--ease),
    box-shadow var(--duration-fast) var(--ease),
    transform var(--duration-fast) var(--ease);
}

.favorites-item:hover {
  border-color: var(--color-border-strong);
  box-shadow: 0 10px 26px rgba(0, 0, 0, 0.1);
  transform: translateY(-1px);
}

.favorites-item__title {
  font-size: var(--text-sm);
  font-weight: var(--weight-semibold);
}

.favorites-item__desc {
  font-size: var(--text-xs);
  color: var(--color-text-tertiary);
}

.favorites-item__meta {
  font-size: var(--text-xs);
  color: var(--color-text-tertiary);
}

.favorites-item__time {
  font-size: var(--text-xs);
  color: var(--color-text-tertiary);
  white-space: nowrap;
}

.empty-state {
  display: flex;
  flex-direction: column;
  align-items: center;
  gap: var(--space-1);
  color: var(--color-text-tertiary);
  padding: var(--space-4) 0;
}

.empty-state span {
  font-size: 32px;
}
</style>
