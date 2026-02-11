/**
 * 模板状态管理
 *
 * 管理模板列表、分类筛选、搜索与收藏切换。
 *
 * @file src/stores/templates.ts
 * @author WaterRun
 * @date 2026-02-11
 */

import { ref, computed } from 'vue'
import { defineStore } from 'pinia'

import type { TemplateListItem, Template } from '@/types/template'
import {
    fetchTemplatesApi,
    searchTemplatesApi,
    fetchTemplateDetailApi,
    fetchCategoriesApi,
    favoriteTemplateApi,
    unfavoriteTemplateApi,
} from '@/api/templates'

/**
 * 模板 Store
 *
 * @example
 * ```typescript
 * const templates = useTemplatesStore()
 * await templates.loadTemplates()
 * ```
 */
export const useTemplatesStore = defineStore('templates', () => {
    /* ---- State ---- */

    /** 模板列表 */
    const items = ref<TemplateListItem[]>([])

    /** 总条数 */
    const total = ref<number>(0)

    /** 当前页码 */
    const page = ref<number>(1)

    /** 每页条数 */
    const limit = ref<number>(20)

    /** 分类列表 */
    const categories = ref<string[]>([])

    /** 当前选中分类（空字符串表示全部） */
    const selectedCategory = ref<string>('')

    /** 搜索关键词（空字符串表示非搜索模式） */
    const keyword = ref<string>('')

    /** 列表加载中 */
    const loading = ref<boolean>(false)

    /** 列表加载错误消息 */
    const errorMessage = ref<string>('')

    /** 当前查看的模板详情 */
    const currentDetail = ref<Template | null>(null)

    /** 详情加载中 */
    const detailLoading = ref<boolean>(false)

    /** 详情错误消息 */
    const detailError = ref<string>('')

    /* ---- Getters ---- */

    /**
     * 总页数
     */
    const totalPages = computed<number>(() =>
        Math.max(1, Math.ceil(total.value / limit.value)),
    )

    /**
     * 是否处于搜索模式
     */
    const isSearchMode = computed<boolean>(() => keyword.value.length > 0)

    /* ---- Actions ---- */

    /**
     * 加载模板列表（根据当前筛选条件）
     *
     * 搜索关键词非空时走搜索接口，否则走列表接口。
     */
    const loadTemplates = async (): Promise<void> => {
        loading.value = true
        errorMessage.value = ''
        try {
            const params = {
                page: page.value,
                limit: limit.value,
                category: selectedCategory.value.length > 0
                    ? selectedCategory.value
                    : undefined,
                keyword: keyword.value.length > 0
                    ? keyword.value
                    : undefined,
            }

            const result = keyword.value.length > 0
                ? await searchTemplatesApi(params)
                : await fetchTemplatesApi(params)

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
     * 加载分类列表
     */
    const loadCategories = async (): Promise<void> => {
        try {
            categories.value = await fetchCategoriesApi()
        } catch {
            categories.value = []
        }
    }

    /**
     * 切换分类筛选，重置页码并重新加载
     *
     * @param category - 分类名称，空字符串表示全部
     */
    const selectCategory = async (category: string): Promise<void> => {
        selectedCategory.value = category
        page.value = 1
        await loadTemplates()
    }

    /**
     * 执行搜索，重置页码并重新加载
     *
     * @param query - 搜索关键词
     */
    const search = async (query: string): Promise<void> => {
        keyword.value = query
        page.value = 1
        await loadTemplates()
    }

    /**
     * 清除搜索状态并重新加载
     */
    const clearSearch = async (): Promise<void> => {
        keyword.value = ''
        page.value = 1
        await loadTemplates()
    }

    /**
     * 跳转到指定页码并重新加载
     *
     * @param targetPage - 目标页码
     */
    const goToPage = async (targetPage: number): Promise<void> => {
        page.value = targetPage
        await loadTemplates()
    }

    /**
     * 加载模板详情
     *
     * @param id - 模板 ID
     */
    const loadDetail = async (id: number): Promise<void> => {
        detailLoading.value = true
        detailError.value = ''
        currentDetail.value = null
        try {
            currentDetail.value = await fetchTemplateDetailApi(id)
        } catch (error: unknown) {
            const { extractErrorMessage } = await import('@/utils/error')
            detailError.value = extractErrorMessage(error)
        } finally {
            detailLoading.value = false
        }
    }

    /**
     * 切换收藏状态
     *
     * @param templateId - 模板 ID
     * @throws {ApiError} 收藏/取消收藏失败
     */
    const toggleFavorite = async (templateId: number): Promise<void> => {
        const item = items.value.find((t) => t.id === templateId)
        const detail = currentDetail.value?.id === templateId
            ? currentDetail.value
            : null

        const wasFavorited = item?.isFavorited ?? detail?.isFavorited ?? false

        if (wasFavorited) {
            await unfavoriteTemplateApi(templateId)
        } else {
            await favoriteTemplateApi(templateId)
        }

        const delta = wasFavorited ? -1 : 1

        if (item !== undefined) {
            item.isFavorited = !wasFavorited
            item.favoriteCount += delta
        }

        if (detail !== null) {
            detail.isFavorited = !wasFavorited
            detail.favoriteCount += delta
        }
    }

    return {
        items,
        total,
        page,
        limit,
        categories,
        selectedCategory,
        keyword,
        loading,
        errorMessage,
        currentDetail,
        detailLoading,
        detailError,
        totalPages,
        isSearchMode,
        loadTemplates,
        loadCategories,
        selectCategory,
        search,
        clearSearch,
        goToPage,
        loadDetail,
        toggleFavorite,
    }
})