<script setup lang="ts">
/**
 * 执行记录页面
 *
 * @component 执行记录
 */

import { computed, ref } from 'vue'

import { useAuthStore } from '@/stores/auth'
import { fetchExecutionHistoryApi } from '@/api/users'
import { extractErrorMessage } from '@/utils/error'
import { formatDateTime, formatDuration } from '@/utils/time'
import Pagination from '@/components/common/Pagination.vue'
import ErrorBanner from '@/components/common/ErrorBanner.vue'
import Loading from '@/components/common/Loading.vue'

import type { ExecutionHistoryItem } from '@/api/users'

/**
 * 认证状态
 */
const authStore = useAuthStore()

/**
 * 执行记录列表
 */
const executions = ref<ExecutionHistoryItem[]>([])

/**
 * 总数
 */
const total = ref<number>(0)

/**
 * 当前页
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
 * 加载执行记录
 */
const loadExecutions = async (): Promise<void> => {
  if (!currentUser.value) return
  loading.value = true
  errorMessage.value = ''
  try {
    const result = await fetchExecutionHistoryApi(currentUser.value.id, {
      page: page.value,
      limit: 20,
    })
    executions.value = result.items
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
  await loadExecutions()
}

loadExecutions()
</script>

<template>
  <div class="executions-page">
    <header class="executions-page__header">
      <div class="executions-page__title">
        <span class="material-symbols-outlined">history</span>
        <span>执行记录</span>
      </div>
    </header>

    <ErrorBanner :message="errorMessage" @dismiss="errorMessage = ''" />

    <Loading v-if="loading" message="加载中" />
    <div v-else class="executions-page__content">
      <div v-if="executions.length === 0" class="empty-state">
        <span class="material-symbols-outlined">schedule</span>
        <p>暂无执行记录</p>
      </div>
      <div v-else class="executions-list">
        <article v-for="item in executions" :key="item.id" class="executions-item">
          <div class="executions-item__main">
            <div class="executions-item__title">
              {{ item.language }}
              <span class="executions-item__status">{{ item.status }}</span>
            </div>
            <div class="executions-item__desc">{{ item.code }}</div>
            <div class="executions-item__meta">
              创建 {{ formatDateTime(item.createdAt) }} · 耗时
              {{ item.executionTime ? formatDuration(item.executionTime) : '--' }}
            </div>
          </div>
        </article>
        <Pagination :page="page" :total-pages="totalPages" @change="handlePageChange" />
      </div>
    </div>
  </div>
</template>

<style scoped>
.executions-page {
  display: flex;
  flex-direction: column;
  gap: var(--space-2);
  padding: var(--space-3);
  height: 100%;
  overflow: auto;
}

.executions-page__header {
  display: flex;
  align-items: center;
  justify-content: space-between;
}

.executions-page__title {
  display: flex;
  align-items: center;
  gap: 6px;
  font-size: var(--text-base);
  font-weight: var(--weight-semibold);
  color: var(--color-text-primary);
}

.executions-page__content {
  display: flex;
  flex-direction: column;
  gap: var(--space-2);
}

.executions-list {
  display: flex;
  flex-direction: column;
  gap: var(--space-1);
}

.executions-item {
  padding: var(--space-2);
  border: 1px solid var(--color-border);
  border-radius: var(--radius-md);
  background-color: var(--color-bg-surface);
  transition:
    border-color var(--duration-fast) var(--ease),
    box-shadow var(--duration-fast) var(--ease),
    transform var(--duration-fast) var(--ease);
}

.executions-item:hover {
  border-color: var(--color-border-strong);
  box-shadow: 0 10px 26px rgba(0, 0, 0, 0.1);
  transform: translateY(-1px);
}

.executions-item__title {
  display: flex;
  align-items: center;
  gap: 8px;
  font-size: var(--text-sm);
  font-weight: var(--weight-semibold);
}

.executions-item__status {
  font-size: var(--text-xs);
  color: var(--color-text-tertiary);
  padding: 2px 6px;
  border-radius: var(--radius-sm);
  border: 1px solid var(--color-border);
  background-color: var(--color-bg-surface-alt);
}

.executions-item__desc {
  font-size: var(--text-xs);
  color: var(--color-text-tertiary);
  margin-top: 4px;
}

.executions-item__meta {
  font-size: var(--text-xs);
  color: var(--color-text-tertiary);
  margin-top: 4px;
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
