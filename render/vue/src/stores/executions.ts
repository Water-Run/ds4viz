/**
 * 执行记录状态管理
 *
 * @file src/stores/executions.ts
 * @author WaterRun
 * @date 2026-02-12
 * @updated 2026-02-12
 */

import { computed, ref } from 'vue'
import { defineStore } from 'pinia'

import type { ExecutionHistoryItem } from '@/api/users'
import type { PaginatedResult } from '@/types/api'
import { fetchExecutionHistoryApi } from '@/api/users'

/**
 * 执行记录状态管理
 *
 * @example
 * ```typescript
 * const executions = useExecutionsStore()
 * await executions.loadExecutions(1)
 * ```
 */
export const useExecutionsStore = defineStore('executions', () => {
  /* ---- State ---- */

  /** 执行记录列表 */
  const items = ref<ExecutionHistoryItem[]>([])

  /** 总条数 */
  const total = ref<number>(0)

  /** 当前页码 */
  const page = ref<number>(1)

  /** 每页条数 */
  const limit = ref<number>(20)

  /** 列表加载中 */
  const loading = ref<boolean>(false)

  /** 错误提示 */
  const errorMessage = ref<string>('')

  /* ---- Getters ---- */

  /**
   * 总页数
   */
  const totalPages = computed<number>(() => Math.max(1, Math.ceil(total.value / limit.value)))

  /* ---- Actions ---- */

  /**
   * 加载执行记录
   *
   * @param userId - 用户 ID
   */
  const loadExecutions = async (userId: number): Promise<void> => {
    loading.value = true
    errorMessage.value = ''
    try {
      const result: PaginatedResult<ExecutionHistoryItem> = await fetchExecutionHistoryApi(
        userId,
        {
          page: page.value,
          limit: limit.value,
        },
      )
      items.value = result.items
      total.value = result.total
    } catch (error: unknown) {
      const { extractErrorMessage } = await import('@/utils/error')
      errorMessage.value = extractErrorMessage(error)
    } finally {
      loading.value = false
    }
  }

  /**
   * 跳转到指定页并重新加载
   *
   * @param targetPage - 目标页码
   * @param userId - 用户 ID
   */
  const goToPage = async (targetPage: number, userId: number): Promise<void> => {
    page.value = targetPage
    await loadExecutions(userId)
  }

  return {
    items,
    total,
    page,
    limit,
    loading,
    errorMessage,
    totalPages,
    loadExecutions,
    goToPage,
  }
})
