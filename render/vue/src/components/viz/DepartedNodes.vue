<script setup lang="ts">
/**
 * 消逝节点浮层
 *
 * 列出当前帧已消逝的节点，支持点击查看变更历史。
 *
 * @file src/components/viz/DepartedNodes.vue
 * @author WaterRun
 * @date 2026-03-28
 * @component DepartedNodes
 */

import { ref, onUnmounted } from 'vue'
import type { NodeLifespan } from '@/types/node-change'
import MaterialIcon from '@/components/common/MaterialIcon.vue'

/**
 * 组件属性定义
 */
interface Props {
    /** 消逝节点列表 */
    lifespans: NodeLifespan[]
}

/**
 * 组件事件定义
 */
interface Emits {
    /** 选中某个消逝节点 */
    (event: 'select', key: string): void
}

defineProps<Props>()
const emit = defineEmits<Emits>()

/** 浮层开关 */
const isOpen = ref<boolean>(false)

/** 延迟关闭计时器 */
const closeTimer = ref<number | null>(null)

/**
 * 取消延迟关闭
 */
const cancelClose = (): void => {
    if (closeTimer.value !== null) {
        window.clearTimeout(closeTimer.value)
        closeTimer.value = null
    }
}

/**
 * 鼠标进入时展开
 */
const handleEnter = (): void => {
    cancelClose()
    isOpen.value = true
}

/**
 * 鼠标离开时延迟关闭
 */
const handleLeave = (): void => {
    cancelClose()
    closeTimer.value = window.setTimeout(() => {
        isOpen.value = false
        closeTimer.value = null
    }, 300)
}

/**
 * 点击选中消逝节点
 *
 * @param key - 节点标识键
 */
const handleSelect = (key: string): void => {
    emit('select', key)
    isOpen.value = false
}

onUnmounted(() => cancelClose())
</script>

<template>
    <div class="departed-nodes" @pointerenter="handleEnter" @pointerleave="handleLeave">
        <button class="departed-nodes__trigger">
            <MaterialIcon name="skull" :size="14" />
            <span class="departed-nodes__count">{{ lifespans.length }}</span>
        </button>

        <Transition name="departed-slide">
            <div v-if="isOpen" class="departed-nodes__popover">
                <div class="departed-nodes__header">消逝节点</div>
                <div class="departed-nodes__list">
                    <button v-for="span in lifespans" :key="span.key" class="departed-nodes__item"
                        @click="handleSelect(span.key)">
                        <span class="departed-nodes__id">{{ span.displayName }}</span>
                        <span class="departed-nodes__range">
                            F{{ span.createdAtFrame }}–F{{ span.deletedAtFrame }}
                        </span>
                    </button>
                    <div v-if="lifespans.length === 0" class="departed-nodes__empty">
                        无消逝节点
                    </div>
                </div>
            </div>
        </Transition>
    </div>
</template>

<style scoped>
.departed-nodes {
    position: absolute;
    top: 8px;
    left: 8px;
    z-index: 6;
}

.departed-nodes__trigger {
    display: inline-flex;
    align-items: center;
    gap: 4px;
    height: 26px;
    padding: 0 8px;
    border: 1px solid var(--color-border-strong);
    border-radius: var(--radius-full);
    background-color: var(--color-bg-surface);
    color: var(--color-error);
    font-size: 11px;
    font-weight: 600;
    font-family: inherit;
    cursor: pointer;
    box-shadow: var(--shadow-static);
    transition:
        background-color var(--duration-fast) var(--ease),
        box-shadow var(--duration-fast) var(--ease);
}

.departed-nodes__trigger:hover {
    background-color: var(--color-bg-hover);
    box-shadow: var(--shadow-hover);
}

.departed-nodes__trigger :deep(.material-icon) {
    width: 14px;
    height: 14px;
}

.departed-nodes__count {
    font-variant-numeric: tabular-nums;
}

.departed-nodes__popover {
    position: absolute;
    top: calc(100% + 6px);
    left: 0;
    min-width: 200px;
    max-width: 280px;
    max-height: 240px;
    background-color: var(--color-bg-surface);
    border: 1px solid var(--color-border);
    border-radius: var(--radius-lg);
    box-shadow: var(--shadow-hover);
    overflow: hidden;
    z-index: 7;
}

.departed-nodes__header {
    padding: 8px 12px 6px;
    border-bottom: 1px solid var(--color-border);
    font-size: 11px;
    font-weight: 600;
    color: var(--color-text-primary);
    letter-spacing: 0.06em;
    text-transform: uppercase;
}

.departed-nodes__list {
    max-height: 200px;
    overflow-y: auto;
    padding: 4px 0;
}

.departed-nodes__item {
    display: flex;
    align-items: center;
    justify-content: space-between;
    gap: 8px;
    width: 100%;
    padding: 6px 12px;
    border: none;
    background: none;
    font-family: inherit;
    cursor: pointer;
    transition: background-color var(--duration-fast) var(--ease);
}

.departed-nodes__item:hover {
    background-color: var(--color-bg-hover);
}

.departed-nodes__id {
    font-size: 12px;
    font-weight: 500;
    color: var(--color-text-primary);
    white-space: nowrap;
    overflow: hidden;
    text-overflow: ellipsis;
}

.departed-nodes__range {
    font-size: 10px;
    color: var(--color-text-tertiary);
    white-space: nowrap;
    flex-shrink: 0;
    font-family: var(--font-mono);
}

.departed-nodes__empty {
    padding: 12px;
    font-size: 11px;
    color: var(--color-text-tertiary);
    text-align: center;
}

.departed-slide-enter-active,
.departed-slide-leave-active {
    transition:
        opacity var(--duration-fast) var(--ease),
        transform var(--duration-fast) var(--ease);
}

.departed-slide-enter-from,
.departed-slide-leave-to {
    opacity: 0;
    transform: translateY(-4px);
}
</style>