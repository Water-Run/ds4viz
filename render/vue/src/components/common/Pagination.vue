<script setup lang="ts">
/**
 * 通用分页控件
 *
 * 显示页码与翻页按钮，支持跳转到指定页。
 *
 * @component Pagination
 *
 * @example
 * ```vue
 * <Pagination :page="1" :total-pages="10" @change="goToPage" />
 * ```
 */

import { computed } from 'vue'

/**
 * 组件属性定义
 */
interface Props {
    /** 当前页码（从 1 起） */
    page: number
    /** 总页数 */
    totalPages: number
}

/**
 * 组件事件定义
 */
interface Emits {
    /** 页码变化 */
    (event: 'change', page: number): void
}

const props = defineProps<Props>()
const emit = defineEmits<Emits>()

/**
 * 可见页码列表，最多显示 7 个页码按钮
 *
 * 当总页数 ≤ 7 时全量显示，否则根据当前页位置截取窗口。
 * -1 表示省略号占位。
 */
const visiblePages = computed<number[]>(() => {
    const total = props.totalPages
    const current = props.page

    if (total <= 7) {
        return Array.from({ length: total }, (_, i) => i + 1)
    }

    if (current <= 4) {
        return [1, 2, 3, 4, 5, -1, total]
    }

    if (current >= total - 3) {
        return [1, -1, total - 4, total - 3, total - 2, total - 1, total]
    }

    return [1, -1, current - 1, current, current + 1, -1, total]
})

/**
 * 是否可以前翻
 */
const hasPrev = computed<boolean>(() => props.page > 1)

/**
 * 是否可以后翻
 */
const hasNext = computed<boolean>(() => props.page < props.totalPages)

/**
 * 跳转到指定页
 *
 * @param target - 目标页码
 */
function goTo(target: number): void {
    if (target >= 1 && target <= props.totalPages && target !== props.page) {
        emit('change', target)
    }
}
</script>

<template>
    <nav v-if="totalPages > 1" class="pagination" aria-label="分页导航">
        <button
            class="pagination__btn"
            :disabled="!hasPrev"
            aria-label="上一页"
            @click="goTo(page - 1)"
        >
            <span class="material-symbols-outlined">chevron_left</span>
        </button>

        <template v-for="(p, index) in visiblePages" :key="index">
            <span v-if="p === -1" class="pagination__ellipsis">…</span>
            <button
                v-else
                class="pagination__btn pagination__page"
                :class="{ 'pagination__page--active': p === page }"
                :aria-current="p === page ? 'page' : undefined"
                @click="goTo(p)"
            >
                {{ p }}
            </button>
        </template>

        <button
            class="pagination__btn"
            :disabled="!hasNext"
            aria-label="下一页"
            @click="goTo(page + 1)"
        >
            <span class="material-symbols-outlined">chevron_right</span>
        </button>
    </nav>
</template>

<style scoped>
.pagination {
    display: flex;
    align-items: center;
    justify-content: center;
    gap: 4px;
}

.pagination__btn {
    display: flex;
    align-items: center;
    justify-content: center;
    min-width: var(--control-height-sm);
    height: var(--control-height-sm);
    padding: 0 6px;
    border: 1px solid var(--color-border);
    border-radius: var(--radius-control);
    background-color: var(--color-bg-surface);
    color: var(--color-text-body);
    font-size: var(--text-xs);
    font-family: inherit;
    cursor: pointer;
    transition:
        background-color var(--duration-fast) var(--ease),
        border-color var(--duration-fast) var(--ease),
        color var(--duration-fast) var(--ease);
}

.pagination__btn:hover:not(:disabled) {
    background-color: var(--color-bg-hover);
    border-color: var(--color-border-strong);
}

.pagination__btn:disabled {
    opacity: 0.4;
    cursor: not-allowed;
}

.pagination__btn .material-symbols-outlined {
    font-size: 18px;
}

.pagination__page--active {
    background-color: var(--color-accent);
    border-color: var(--color-accent);
    color: var(--color-accent-contrast);
}

.pagination__page--active:hover:not(:disabled) {
    background-color: var(--color-accent-hover);
    border-color: var(--color-accent-hover);
    color: var(--color-accent-contrast);
}

.pagination__ellipsis {
    display: flex;
    align-items: center;
    justify-content: center;
    min-width: var(--control-height-sm);
    height: var(--control-height-sm);
    color: var(--color-text-tertiary);
    font-size: var(--text-xs);
    user-select: none;
}
</style>