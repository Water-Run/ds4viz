<script setup lang="ts">
/**
 * 节点变更卡片（SVG 内渲染）
 *
 * 横向数组式布局，每帧一个单元格。
 * 支持简洁/详细两种模式，当前帧/全部帧切换。
 *
 * @file src/components/viz/ChangeCard.vue
 * @author WaterRun
 * @date 2026-03-28
 * @component ChangeCard
 */

import { computed, ref } from 'vue'
import type { NodeChangeTimeline, NodeChangeEntry, NodeChangeType } from '@/types/node-change'
import { CHANGE_TYPE_SYMBOL } from '@/types/node-change'
import type { IrValue } from '@/types/ir'

/**
 * 组件属性定义
 */
interface Props {
    /** 变更时间线 */
    timeline: NodeChangeTimeline
    /** 当前帧索引 */
    currentFrameIndex: number
    /** 卡片 SVG x 坐标 */
    cardX: number
    /** 卡片 SVG y 坐标 */
    cardY: number
    /** 节点中心 x */
    nodeX: number
    /** 节点中心 y */
    nodeY: number
    /** 是否展示全部帧 */
    showAllFrames: boolean
    /** 是否详细模式 */
    detailMode: boolean
    /** 是否消逝态 */
    departed: boolean
}

/**
 * 组件事件定义
 */
interface Emits {
    /** 跳转到帧 */
    (event: 'jump-to-frame', frameIndex: number): void
    /** 切换全部帧/当前帧 */
    (event: 'toggle-all-frames'): void
    /** 切换详细模式 */
    (event: 'toggle-detail'): void
    /** 关闭卡片（消逝态专用） */
    (event: 'close'): void
}

const props = defineProps<Props>()
const emit = defineEmits<Emits>()

/* ---- 尺寸常量 ---- */

const PAD = 6
const HEADER_W = 48
const CELL_W_SIMPLE = 32
const CELL_W_DETAIL = 46
const CELL_H_SIMPLE = 42
const CELL_H_DETAIL = 62
const MAX_VISIBLE = 7
const NAV_W = 14
const BORDER_R = 10

const CELL_W = computed<number>(() => props.detailMode ? CELL_W_DETAIL : CELL_W_SIMPLE)
const CELL_H = computed<number>(() => props.detailMode ? CELL_H_DETAIL : CELL_H_SIMPLE)

/* ---- 滚动偏移 ---- */

const scrollOffset = ref<number>(0)

/* ---- 条目过滤 ---- */

/**
 * 按当前帧截取或全量条目
 */
const filteredEntries = computed<NodeChangeEntry[]>(() => {
    if (props.showAllFrames) return props.timeline.entries
    return props.timeline.entries.filter((e) => e.frameIndex <= props.currentFrameIndex)
})

/**
 * 可见条目（分页窗口）
 */
const visibleEntries = computed<NodeChangeEntry[]>(() => {
    const start = scrollOffset.value
    return filteredEntries.value.slice(start, start + MAX_VISIBLE)
})

/**
 * 是否有左侧溢出
 */
const hasLeft = computed<boolean>(() => scrollOffset.value > 0)

/**
 * 是否有右侧溢出
 */
const hasRight = computed<boolean>(
    () => scrollOffset.value + MAX_VISIBLE < filteredEntries.value.length,
)

/**
 * 可见单元格数
 */
const visibleCount = computed<number>(() => visibleEntries.value.length)

/**
 * 卡片总宽
 */
const totalW = computed<number>(() => {
    const navW = (hasLeft.value || hasRight.value) ? NAV_W * 2 : 0
    return PAD * 2 + HEADER_W + visibleCount.value * CELL_W.value + navW
})

/**
 * 卡片总高
 */
const totalH = computed<number>(() => PAD * 2 + CELL_H.value)

/**
 * 左侧导航箭头 x 偏移
 */
const navLeftX = computed<number>(() => PAD + HEADER_W)

/**
 * 单元格起始 x 偏移
 */
const cellsStartX = computed<number>(() => {
    return PAD + HEADER_W + (hasLeft.value ? NAV_W : 0)
})

/**
 * 右侧导航箭头 x 偏移
 */
const navRightX = computed<number>(() => {
    return cellsStartX.value + visibleCount.value * CELL_W.value
})

/* ---- Leader line ---- */

/**
 * Leader line 目标点（节点中心，相对于卡片原点）
 */
const leaderTarget = computed<{ x: number; y: number }>(() => ({
    x: props.nodeX - props.cardX,
    y: props.nodeY - props.cardY,
}))

/**
 * Leader line 起始点（卡片边缘最近点）
 */
const leaderStart = computed<{ x: number; y: number }>(() => {
    const tx = leaderTarget.value.x
    const ty = leaderTarget.value.y
    const cx = totalW.value / 2
    const cy = totalH.value / 2
    const dx = tx - cx
    const dy = ty - cy

    if (Math.abs(dx) * totalH.value > Math.abs(dy) * totalW.value) {
        return dx > 0
            ? { x: totalW.value, y: cy }
            : { x: 0, y: cy }
    }
    return dy > 0
        ? { x: cx, y: totalH.value }
        : { x: cx, y: 0 }
})

/* ---- 单元格渲染 ---- */

/**
 * 变更类型颜色
 *
 * @param t - 变更类型
 * @returns 填充颜色
 */
function typeColor(t: NodeChangeType): string {
    const colors: Record<NodeChangeType, string> = {
        created: '#22c55e',
        deleted: '#ef4444',
        value_changed: '#0078d4',
        structural: '#0891b2',
        highlighted: '#d97706',
        alias_changed: '#7c3aed',
        weight_changed: '#0369a1',
    }
    return colors[t]
}

/**
 * 单元格值文本
 *
 * @param entry - 变更条目
 * @returns 显示值
 */
function cellValue(entry: NodeChangeEntry): string {
    if (entry.primaryType === 'created') {
        return formatVal(entry.detail.valueAfter)
    }
    if (entry.primaryType === 'deleted') {
        return '✕'
    }
    if (entry.primaryType === 'value_changed') {
        return formatVal(entry.detail.valueAfter)
    }
    return CHANGE_TYPE_SYMBOL[entry.primaryType]
}

/**
 * 格式化值显示
 *
 * @param v - 值
 * @returns 截断字符串
 */
function formatVal(v: IrValue | undefined): string {
    if (v === undefined) return '--'
    const s = String(v)
    return s.length > 4 ? `${s.slice(0, 3)}…` : s
}

/**
 * 判断条目是否对应当前帧
 *
 * @param entry - 变更条目
 * @returns 是否当前帧
 */
function isCurrentFrame(entry: NodeChangeEntry): boolean {
    return entry.frameIndex === props.currentFrameIndex
}

/**
 * 判断条目是否为未来帧（全部帧模式下）
 *
 * @param entry - 变更条目
 * @returns 是否未来帧
 */
function isFutureFrame(entry: NodeChangeEntry): boolean {
    return props.showAllFrames && entry.frameIndex > props.currentFrameIndex
}

/**
 * 详细模式下的附加行文本
 *
 * @param entry - 变更条目
 * @returns 附加文本
 */
function detailText(entry: NodeChangeEntry): string {
    if (entry.primaryType === 'value_changed' && entry.detail.valueBefore !== undefined) {
        return `${formatVal(entry.detail.valueBefore)}→${formatVal(entry.detail.valueAfter)}`
    }
    if (entry.primaryType === 'structural' && entry.detail.structuralChanges) {
        const c = entry.detail.structuralChanges[0]
        if (c) return `${c.field}: ${c.before}→${c.after}`
    }
    if (entry.primaryType === 'alias_changed') {
        return `${entry.detail.aliasBefore ?? '∅'}→${entry.detail.aliasAfter ?? '∅'}`
    }
    if (entry.note) {
        return entry.note.length > 8 ? `${entry.note.slice(0, 7)}…` : entry.note
    }
    return ''
}

/**
 * 当前显示值（标题栏）
 */
const headerValue = computed<string>(() => {
    const v = props.timeline.lastValue
    if (v === undefined) return ''
    return `=${formatVal(v)}`
})

/**
 * 滚动至当前帧
 */
function scrollToCurrent(): void {
    const idx = filteredEntries.value.findIndex(
        (e) => e.frameIndex === props.currentFrameIndex,
    )
    if (idx >= 0) {
        scrollOffset.value = Math.max(0, Math.min(idx - 2, filteredEntries.value.length - MAX_VISIBLE))
    }
}

/**
 * 处理单元格点击
 *
 * @param entry - 变更条目
 * @param event - 点击事件
 */
function handleCellClick(entry: NodeChangeEntry, event: Event): void {
    event.stopPropagation()
    emit('jump-to-frame', entry.frameIndex)
}

/**
 * 滚动左移
 *
 * @param event - 点击事件
 */
function handleScrollLeft(event: Event): void {
    event.stopPropagation()
    if (scrollOffset.value > 0) scrollOffset.value -= 1
}

/**
 * 滚动右移
 *
 * @param event - 点击事件
 */
function handleScrollRight(event: Event): void {
    event.stopPropagation()
    if (hasRight.value) scrollOffset.value += 1
}
</script>

<template>
    <g :transform="`translate(${cardX},${cardY})`" class="change-card">
        <!-- Leader line -->
        <line :x1="leaderStart.x" :y1="leaderStart.y" :x2="leaderTarget.x" :y2="leaderTarget.y"
            :class="departed ? 'change-card__leader--departed' : 'change-card__leader'" />
        <circle :cx="leaderTarget.x" :cy="leaderTarget.y" r="3"
            :class="departed ? 'change-card__dot--departed' : 'change-card__dot'" />

        <!-- Card background -->
        <rect x="0" y="0" :width="totalW" :height="totalH" :rx="BORDER_R"
            :class="departed ? 'change-card__bg--departed' : 'change-card__bg'" />

        <!-- Header cell -->
        <g :transform="`translate(${PAD}, ${PAD})`">
            <rect x="0" y="0" :width="HEADER_W" :height="CELL_H" rx="6" class="change-card__header-bg" />
            <text :x="HEADER_W / 2" y="14" class="change-card__id">#{{ timeline.nodeId }}</text>
            <text v-if="timeline.lastAlias" :x="HEADER_W / 2" y="25" class="change-card__alias">{{ timeline.lastAlias
                }}</text>
            <text :x="HEADER_W / 2" :y="timeline.lastAlias ? 36 : 28" class="change-card__header-val">{{ headerValue
                }}</text>
        </g>

        <!-- Toggle buttons (top right of header) -->
        <g :transform="`translate(${PAD + 2}, ${PAD + CELL_H - 14})`" class="change-card__toggles">
            <text x="0" y="0" class="change-card__toggle-btn" @click.stop="emit('toggle-all-frames')">{{ showAllFrames ?
                '◂' : '▸' }}</text>
            <text x="12" y="0" class="change-card__toggle-btn" @click.stop="emit('toggle-detail')">{{ detailMode ? '−' :
                '+' }}</text>
            <text v-if="departed" x="24" y="0" class="change-card__toggle-btn change-card__toggle-btn--close"
                @click.stop="emit('close')">✕</text>
        </g>

        <!-- Left nav arrow -->
        <g v-if="hasLeft" :transform="`translate(${navLeftX}, ${PAD})`" class="change-card__nav"
            @click.stop="handleScrollLeft">
            <rect x="0" y="0" :width="NAV_W" :height="CELL_H" rx="4" class="change-card__nav-bg" />
            <text :x="NAV_W / 2" :y="CELL_H / 2 + 4" class="change-card__nav-arrow">‹</text>
        </g>

        <!-- Entry cells -->
        <g v-for="(entry, i) in visibleEntries" :key="`cell-${entry.frameIndex}`"
            :transform="`translate(${cellsStartX + i * CELL_W}, ${PAD})`" class="change-card__cell-group"
            :opacity="isFutureFrame(entry) ? 0.35 : 1" @click="handleCellClick(entry, $event)">
            <rect x="0.5" y="0.5" :width="CELL_W - 1" :height="CELL_H - 1" rx="4" :class="{
                'change-card__cell': true,
                'change-card__cell--current': isCurrentFrame(entry),
                'change-card__cell--deleted': entry.primaryType === 'deleted',
            }" />

            <!-- Type indicator dot -->
            <circle :cx="CELL_W / 2" cy="10" r="3" :fill="typeColor(entry.primaryType)" />

            <!-- Type symbol -->
            <text :x="CELL_W / 2" cy="10" y="22" class="change-card__cell-sym">{{ cellValue(entry) }}</text>

            <!-- Frame number -->
            <text :x="CELL_W / 2" y="33" class="change-card__cell-frame">F{{ entry.frameIndex }}</text>

            <!-- Detail mode extra line -->
            <text v-if="detailMode" :x="CELL_W / 2" y="44" class="change-card__cell-detail">{{ detailText(entry)
                }}</text>

            <text v-if="detailMode && entry.op" :x="CELL_W / 2" y="55" class="change-card__cell-op">{{ entry.op.length >
                6 ? entry.op.slice(0, 5) + '…' : entry.op }}</text>
        </g>

        <!-- Right nav arrow -->
        <g v-if="hasRight" :transform="`translate(${navRightX}, ${PAD})`" class="change-card__nav"
            @click.stop="handleScrollRight">
            <rect x="0" y="0" :width="NAV_W" :height="CELL_H" rx="4" class="change-card__nav-bg" />
            <text :x="NAV_W / 2" :y="CELL_H / 2 + 4" class="change-card__nav-arrow">›</text>
        </g>
    </g>
</template>

<style scoped>
.change-card__leader {
    stroke: var(--color-border-strong);
    stroke-width: 1;
    stroke-dasharray: 4 3;
}

.change-card__leader--departed {
    stroke: var(--color-error);
    stroke-width: 1;
    stroke-dasharray: 4 3;
    opacity: 0.6;
}

.change-card__dot {
    fill: var(--color-accent);
}

.change-card__dot--departed {
    fill: var(--color-error);
    opacity: 0.6;
}

.change-card__bg {
    fill: #ffffff;
    stroke: rgba(0, 0, 0, 0.14);
    stroke-width: 1;
    filter: drop-shadow(0 1px 2px rgba(0, 0, 0, 0.04));
}

.change-card__bg--departed {
    fill: #ffffff;
    stroke: var(--color-error);
    stroke-width: 1;
    stroke-dasharray: 6 3;
    filter: drop-shadow(0 1px 2px rgba(0, 0, 0, 0.04));
}

.change-card__header-bg {
    fill: var(--color-bg-surface-alt);
    stroke: var(--color-border);
    stroke-width: 0.5;
}

.change-card__id {
    font-size: 10px;
    font-weight: 600;
    fill: var(--color-text-primary);
    text-anchor: middle;
    font-family: var(--font-mono);
}

.change-card__alias {
    font-size: 9px;
    font-weight: 500;
    fill: var(--color-accent);
    text-anchor: middle;
}

.change-card__header-val {
    font-size: 9px;
    fill: var(--color-text-tertiary);
    text-anchor: middle;
    font-family: var(--font-mono);
}

.change-card__cell {
    fill: var(--color-bg-surface);
    stroke: var(--color-border);
    stroke-width: 0.5;
    cursor: pointer;
    transition: fill 140ms cubic-bezier(0.2, 0, 0, 1);
}

.change-card__cell:hover {
    fill: var(--color-bg-hover);
}

.change-card__cell--current {
    fill: rgba(0, 120, 212, 0.08);
    stroke: var(--color-accent);
    stroke-width: 1.5;
}

.change-card__cell--deleted {
    fill: rgba(239, 68, 68, 0.06);
}

.change-card__cell-sym {
    font-size: 10px;
    font-family: var(--font-mono);
    fill: var(--color-text-primary);
    text-anchor: middle;
}

.change-card__cell-frame {
    font-size: 8px;
    fill: var(--color-text-tertiary);
    text-anchor: middle;
}

.change-card__cell-detail {
    font-size: 7px;
    fill: var(--color-text-tertiary);
    text-anchor: middle;
    font-family: var(--font-mono);
}

.change-card__cell-op {
    font-size: 7px;
    fill: var(--color-text-tertiary);
    text-anchor: middle;
    font-style: italic;
}

.change-card__nav {
    cursor: pointer;
}

.change-card__nav-bg {
    fill: var(--color-bg-surface-alt);
    stroke: var(--color-border);
    stroke-width: 0.5;
    transition: fill 140ms cubic-bezier(0.2, 0, 0, 1);
}

.change-card__nav:hover .change-card__nav-bg {
    fill: var(--color-bg-hover);
}

.change-card__nav-arrow {
    font-size: 12px;
    fill: var(--color-text-tertiary);
    text-anchor: middle;
}

.change-card__toggles text {
    font-size: 9px;
    cursor: pointer;
}

.change-card__toggle-btn {
    fill: var(--color-text-tertiary);
    transition: fill 140ms cubic-bezier(0.2, 0, 0, 1);
}

.change-card__toggle-btn:hover {
    fill: var(--color-accent);
}

.change-card__toggle-btn--close:hover {
    fill: var(--color-error);
}
</style>