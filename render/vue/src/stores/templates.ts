/**
 * 模板状态管理
 *
 * 管理模板列表、分类筛选、搜索与收藏切换。
 *
 * @file src/stores/templates.ts
 * @author WaterRun
 * @date 2026-03-28
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

export const useTemplatesStore = defineStore('templates', () => {
    const items = ref<TemplateListItem[]>([])
    const total = ref<number>(0)
    const page = ref<number>(1)
    const limit = ref<number>(20)

    const categories = ref<string[]>([])
    const selectedCategory = ref<string>('')
    const keyword = ref<string>('')

    /** 首屏加载 / 追加加载共用 */
    const loading = ref<boolean>(false)
    /** 分类切换/搜索切换时的“平滑过渡”态 */
    const switching = ref<boolean>(false)

    const errorMessage = ref<string>('')

    const currentDetail = ref<Template | null>(null)
    const detailLoading = ref<boolean>(false)
    const detailError = ref<string>('')

    /** 用于触发内容过渡 key */
    const viewVersion = ref<number>(0)

    const totalPages = computed<number>(() =>
        Math.max(1, Math.ceil(total.value / limit.value)),
    )

    const isSearchMode = computed<boolean>(() => keyword.value.length > 0)
    const hasMore = computed<boolean>(() => items.value.length < total.value)

    const sortIfNeeded = (list: TemplateListItem[]): TemplateListItem[] => {
        if (!isSearchMode.value) {
            list.sort((a, b) => b.favoriteCount - a.favoriteCount)
        }
        return list
    }

    const buildParams = (targetPage: number) => ({
        page: targetPage,
        limit: limit.value,
        category: selectedCategory.value.length > 0 ? selectedCategory.value : undefined,
        keyword: keyword.value.length > 0 ? keyword.value : undefined,
    })

    const fetchPage = async (targetPage: number) => {
        const params = buildParams(targetPage)
        return keyword.value.length > 0
            ? await searchTemplatesApi(params)
            : await fetchTemplatesApi(params)
    }

    /**
     * 解决“全部页收藏区要滚动才出现”：
     * 在“全部 + 非搜索”且首屏未见收藏时，主动预取后续页（有上限），
     * 直到出现收藏或无更多数据。
     */
    const ensureFavoritesVisible = async (): Promise<void> => {
        if (selectedCategory.value.length > 0 || isSearchMode.value) return
        if (items.value.some((t) => t.isFavorited)) return
        if (!hasMore.value) return

        let prefetchCount = 0
        const maxPrefetchPages = 4

        while (
            prefetchCount < maxPrefetchPages &&
            !items.value.some((t) => t.isFavorited) &&
            items.value.length < total.value
        ) {
            const nextPage = page.value + 1
            const result = await fetchPage(nextPage)
            if (result.items.length === 0) {
                total.value = items.value.length
                break
            }
            page.value = nextPage
            items.value = sortIfNeeded([...items.value, ...result.items])
            total.value = result.total
            prefetchCount += 1
        }
    }

    const loadTemplates = async (): Promise<void> => {
        loading.value = true
        errorMessage.value = ''
        try {
            const result = await fetchPage(page.value)
            items.value = sortIfNeeded(result.items)
            total.value = result.total
            await ensureFavoritesVisible()
            viewVersion.value += 1
        } catch (error: unknown) {
            const { extractErrorMessage } = await import('@/utils/error')
            errorMessage.value = extractErrorMessage(error)
        } finally {
            loading.value = false
        }
    }

    const appendTemplates = async (): Promise<void> => {
        if (loading.value || !hasMore.value) return
        loading.value = true
        errorMessage.value = ''
        try {
            const nextPage = page.value + 1
            const result = await fetchPage(nextPage)
            if (result.items.length === 0) {
                total.value = items.value.length
            } else {
                page.value = nextPage
                items.value = sortIfNeeded([...items.value, ...result.items])
                total.value = result.total
            }
        } catch (error: unknown) {
            const { extractErrorMessage } = await import('@/utils/error')
            errorMessage.value = extractErrorMessage(error)
        } finally {
            loading.value = false
        }
    }

    const loadCategories = async (): Promise<void> => {
        try {
            categories.value = await fetchCategoriesApi()
        } catch {
            categories.value = []
        }
    }

    /** 平滑切换（不先清空列表，避免生硬闪断） */
    const switchContextAndReload = async (next: {
        category?: string
        keyword?: string
    }): Promise<void> => {
        switching.value = true
        errorMessage.value = ''

        if (typeof next.category === 'string') selectedCategory.value = next.category
        if (typeof next.keyword === 'string') keyword.value = next.keyword

        page.value = 1
        total.value = 0
        // 注意：这里不先置空 items，等新数据到达再替换

        try {
            const result = await fetchPage(1)
            items.value = sortIfNeeded(result.items)
            total.value = result.total
            await ensureFavoritesVisible()
            viewVersion.value += 1
        } catch (error: unknown) {
            const { extractErrorMessage } = await import('@/utils/error')
            errorMessage.value = extractErrorMessage(error)
        } finally {
            switching.value = false
        }
    }

    const selectCategory = async (category: string): Promise<void> => {
        await switchContextAndReload({ category, keyword: '' })
    }

    const search = async (query: string): Promise<void> => {
        // 关键修复：不清空 selectedCategory，保留当前分类搜索
        await switchContextAndReload({ keyword: query })
    }

    const clearSearch = async (): Promise<void> => {
        await switchContextAndReload({ keyword: '' })
    }

    const goToPage = async (targetPage: number): Promise<void> => {
        page.value = targetPage
        await loadTemplates()
    }

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

    const toggleFavorite = async (templateId: number): Promise<void> => {
        const item = items.value.find((t) => t.id === templateId)
        const detail = currentDetail.value?.id === templateId ? currentDetail.value : null

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
        switching,
        errorMessage,
        currentDetail,
        detailLoading,
        detailError,
        totalPages,
        isSearchMode,
        hasMore,
        viewVersion,
        loadTemplates,
        appendTemplates,
        loadCategories,
        selectCategory,
        search,
        clearSearch,
        goToPage,
        loadDetail,
        toggleFavorite,
    }
})
