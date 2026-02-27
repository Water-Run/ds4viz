<script setup lang="ts">
/**
 * 可视化面板（重写）
 *
 * 修复响应式深层代理导致的渲染卡死问题：
 * - 输入数据已通过 markRaw 处理，不会被 Vue 代理化
 * - 布局计算使用 watch + shallowRef，避免级联求值
 * - SVG 坐标全部预计算，模板内无 .find() 调用
 * - TransitionGroup 受 enableSmoothTransitions flag 控制
 *
 * @component VizPanel
 */

import { computed, shallowRef, watch } from 'vue'

import type {
  IrStateData,
  IrObjectKind,
  IrValue,
  StackStateData,
  QueueStateData,
  SListStateData,
  SListNode,
  DListStateData,
  DListNode,
  BinaryTreeStateData,
  BinaryTreeNode,
  GraphStateData,
  GraphWeightedStateData,
} from '@/types/ir'
import type { StepSummary } from '@/types/viz'
import { vizFlags, logDebug } from '@/utils/viz-flags'

import VizPlaceholder from './VizPlaceholder.vue'
import MaterialIcon from '@/components/common/MaterialIcon.vue'

/**
 * 组件属性定义
 */
interface Props {
  /** 结构类型 */
  kind?: IrObjectKind
  /** 状态数据（markRaw 标记，非响应式） */
  data?: IrStateData
  /** 步骤信息 */
  step?: StepSummary | null
}

const props = defineProps<Props>()

/* ---- 辅助类型 ---- */

interface TreeLayoutNode {
  id: number
  value: IrValue
  x: number
  y: number
  isRoot: boolean
}

interface TreeLayoutEdge {
  x1: number
  y1: number
  x2: number
  y2: number
}

interface TreeLayout {
  svgWidth: number
  svgHeight: number
  nodeRadius: number
  nodes: TreeLayoutNode[]
  edges: TreeLayoutEdge[]
}

interface GraphLayoutNode {
  id: number
  label: string
  x: number
  y: number
}

interface GraphLayoutEdge {
  from: number
  to: number
  x1: number
  y1: number
  x2: number
  y2: number
  weight?: number
}

interface GraphLayout {
  svgWidth: number
  svgHeight: number
  nodeRadius: number
  nodes: GraphLayoutNode[]
  edges: GraphLayoutEdge[]
}

/* ---- 格式化 ---- */

/**
 * 格式化显示值
 */
function formatValue(value: IrValue): string {
  if (typeof value === 'string') return `"${value}"`
  return String(value)
}

/* ---- 基础派生 ---- */

const isEmpty = computed<boolean>(() => props.data === undefined)

const kindLabel = computed<string>(() => {
  const mapping: Record<IrObjectKind, string> = {
    stack: 'Stack',
    queue: 'Queue',
    slist: 'SList',
    dlist: 'DList',
    binary_tree: 'BinaryTree',
    bst: 'BST',
    graph_undirected: 'Graph',
    graph_directed: 'Digraph',
    graph_weighted: 'WeightedGraph',
  }
  return props.kind ? mapping[props.kind] ?? props.kind : ''
})

const isWeighted = computed<boolean>(() => props.kind === 'graph_weighted')
const isDirected = computed<boolean>(() => props.kind === 'graph_directed')

/* ---- 类型窄化 ---- */

const stackData = computed<StackStateData | null>(() =>
  props.kind === 'stack' ? (props.data as StackStateData) : null,
)
const queueData = computed<QueueStateData | null>(() =>
  props.kind === 'queue' ? (props.data as QueueStateData) : null,
)
const slistData = computed<SListStateData | null>(() =>
  props.kind === 'slist' ? (props.data as SListStateData) : null,
)
const dlistData = computed<DListStateData | null>(() =>
  props.kind === 'dlist' ? (props.data as DListStateData) : null,
)
const treeData = computed<BinaryTreeStateData | null>(() =>
  props.kind === 'binary_tree' || props.kind === 'bst'
    ? (props.data as BinaryTreeStateData)
    : null,
)
const graphData = computed<GraphStateData | GraphWeightedStateData | null>(() =>
  props.kind === 'graph_undirected' ||
  props.kind === 'graph_directed' ||
  props.kind === 'graph_weighted'
    ? (props.data as GraphStateData)
    : null,
)

/* ---- 链表有序遍历 ---- */

const slistOrdered = computed<SListNode[]>(() => {
  if (!slistData.value) return []
  const map = new Map<number, SListNode>()
  slistData.value.nodes.forEach((n) => map.set(n.id, n))
  const result: SListNode[] = []
  let cur = slistData.value.head
  const visited = new Set<number>()
  while (cur !== -1 && !visited.has(cur)) {
    visited.add(cur)
    const node = map.get(cur)
    if (!node) break
    result.push(node)
    cur = node.next
  }
  return result
})

const dlistOrdered = computed<DListNode[]>(() => {
  if (!dlistData.value) return []
  const map = new Map<number, DListNode>()
  dlistData.value.nodes.forEach((n) => map.set(n.id, n))
  const result: DListNode[] = []
  let cur = dlistData.value.head
  const visited = new Set<number>()
  while (cur !== -1 && !visited.has(cur)) {
    visited.add(cur)
    const node = map.get(cur)
    if (!node) break
    result.push(node)
    cur = node.next
  }
  return result
})

/* ---- 树布局（watch + shallowRef 避免 computed 级联） ---- */

const treeLayout = shallowRef<TreeLayout | null>(null)

watch(
  () => treeData.value,
  (data) => {
    if (!data || data.root === -1 || data.nodes.length === 0) {
      treeLayout.value = null
      return
    }
    logDebug('[viz] computing tree layout, nodes =', data.nodes.length)
    treeLayout.value = computeTreeLayout(data)
  },
  { immediate: true },
)

function computeTreeLayout(data: BinaryTreeStateData): TreeLayout {
  const nodeRadius = 16
  const levelHeight = 70
  const padding = 40
  const rootId = data.root

  const nodeMap = new Map<number, BinaryTreeNode>()
  data.nodes.forEach((n) => nodeMap.set(n.id, n))

  const levels: number[][] = []
  const queue: Array<{ id: number; level: number }> = [{ id: rootId, level: 0 }]
  const visited = new Set<number>()

  while (queue.length > 0) {
    const current = queue.shift()!
    if (visited.has(current.id)) continue
    visited.add(current.id)
    if (!levels[current.level]) levels[current.level] = []
    levels[current.level].push(current.id)
    const node = nodeMap.get(current.id)
    if (!node) continue
    if (node.left !== -1) queue.push({ id: node.left, level: current.level + 1 })
    if (node.right !== -1) queue.push({ id: node.right, level: current.level + 1 })
  }

  const maxInLevel = Math.max(...levels.map((l) => l.length))
  const baseWidth = Math.max(300, maxInLevel * nodeRadius * 4 + padding * 2)

  const positions = new Map<number, { x: number; y: number }>()
  levels.forEach((levelNodes, level) => {
    const gap = baseWidth / (levelNodes.length + 1)
    levelNodes.forEach((id, index) => {
      positions.set(id, { x: gap * (index + 1), y: padding + level * levelHeight })
    })
  })

  let maxX = 0
  let maxY = 0
  positions.forEach((pos) => {
    if (pos.x > maxX) maxX = pos.x
    if (pos.y > maxY) maxY = pos.y
  })

  const svgWidth = maxX + padding
  const svgHeight = maxY + padding + nodeRadius

  const nodes: TreeLayoutNode[] = []
  data.nodes.forEach((node) => {
    const pos = positions.get(node.id)
    if (!pos) return
    nodes.push({ id: node.id, value: node.value, x: pos.x, y: pos.y, isRoot: node.id === rootId })
  })

  const edges: TreeLayoutEdge[] = []
  data.nodes.forEach((node) => {
    const from = positions.get(node.id)
    if (!from) return
    if (node.left !== -1) {
      const to = positions.get(node.left)
      if (to) edges.push({ x1: from.x, y1: from.y, x2: to.x, y2: to.y })
    }
    if (node.right !== -1) {
      const to = positions.get(node.right)
      if (to) edges.push({ x1: from.x, y1: from.y, x2: to.x, y2: to.y })
    }
  })

  return { svgWidth, svgHeight, nodeRadius, nodes, edges }
}

/* ---- 图布局（watch + shallowRef） ---- */

const graphLayout = shallowRef<GraphLayout | null>(null)

watch(
  () => graphData.value,
  (data) => {
    if (!data || data.nodes.length === 0) {
      graphLayout.value = null
      return
    }
    logDebug('[viz] computing graph layout, nodes =', data.nodes.length, 'edges =', data.edges.length)
    graphLayout.value = computeGraphLayout(data)
  },
  { immediate: true },
)

function computeGraphLayout(data: GraphStateData | GraphWeightedStateData): GraphLayout {
  const svgWidth = 500
  const svgHeight = 350
  const nodeRadius = 16
  const circleRadius = Math.min(120, 30 * Math.max(3, data.nodes.length))
  const cx = svgWidth / 2
  const cy = svgHeight / 2

  const nodePositionMap = new Map<number, { x: number; y: number }>()
  const nodes: GraphLayoutNode[] = data.nodes.map((node, i) => {
    const angle = (2 * Math.PI * i) / Math.max(1, data.nodes.length) - Math.PI / 2
    const x = cx + circleRadius * Math.cos(angle)
    const y = cy + circleRadius * Math.sin(angle)
    nodePositionMap.set(node.id, { x, y })
    return { id: node.id, label: node.label, x, y }
  })

  const edges: GraphLayoutEdge[] = []
  data.edges.forEach((edge) => {
    const from = nodePositionMap.get(edge.from)
    const to = nodePositionMap.get(edge.to)
    if (!from || !to) return
    const dx = to.x - from.x
    const dy = to.y - from.y
    const dist = Math.sqrt(dx * dx + dy * dy)
    if (dist === 0) return
    const ux = dx / dist
    const uy = dy / dist
    edges.push({
      from: edge.from,
      to: edge.to,
      x1: from.x + ux * nodeRadius,
      y1: from.y + uy * nodeRadius,
      x2: to.x - ux * nodeRadius,
      y2: to.y - uy * nodeRadius,
      weight: (edge as { weight?: number }).weight,
    })
  })

  return { svgWidth, svgHeight, nodeRadius, nodes, edges }
}
</script>

<template>
  <section class="viz-panel" :class="{ 'viz-panel--animated': vizFlags.enableSmoothTransitions }">
    <header class="viz-panel__header">
      <div class="viz-panel__title">
        <MaterialIcon name="graph_3" :size="18" />
        <span>{{ kindLabel || '可视化' }}</span>
      </div>
      <div v-if="step" class="viz-panel__step">
        <span class="viz-panel__step-op">{{ step.op }}</span>
        <span v-if="step.line" class="viz-panel__step-line">L{{ step.line }}</span>
      </div>
    </header>

    <div class="viz-panel__body">
      <VizPlaceholder v-if="isEmpty" />

      <div v-else class="viz-panel__content">
        <!-- ── 栈 ── -->
        <div v-if="stackData" class="stack-view">
          <div class="stack-view__items">
            <div
              v-for="(item, index) in stackData.items"
              :key="index"
              class="stack-view__item"
              :class="{ 'stack-view__item--top': vizFlags.enableNodeColors && index === stackData.top }"
            >
              <span class="stack-view__value">{{ formatValue(item) }}</span>
              <span
                v-if="vizFlags.enableStepBadges && index === stackData.top"
                class="stack-view__badge"
              >
                TOP
              </span>
            </div>
          </div>
        </div>

        <!-- ── 队列 ── -->
        <div v-else-if="queueData" class="queue-view">
          <div class="queue-view__items">
            <div
              v-for="(item, index) in queueData.items"
              :key="index"
              class="queue-view__item"
              :class="{
                'queue-view__item--front': vizFlags.enableNodeColors && index === queueData.front,
                'queue-view__item--rear': vizFlags.enableNodeColors && index === queueData.rear,
              }"
            >
              <span class="queue-view__value">{{ formatValue(item) }}</span>
              <span
                v-if="vizFlags.enableStepBadges && index === queueData.front"
                class="queue-view__badge"
              >
                FRONT
              </span>
              <span
                v-if="vizFlags.enableStepBadges && index === queueData.rear"
                class="queue-view__badge"
              >
                REAR
              </span>
            </div>
          </div>
        </div>

        <!-- ── 单链表 ── -->
        <div v-else-if="slistData" class="list-view">
          <div class="list-view__nodes">
            <div
              v-for="node in slistOrdered"
              :key="node.id"
              class="list-view__node"
              :class="{ 'list-view__node--head': vizFlags.enableNodeColors && node.id === slistData.head }"
            >
              <div class="list-view__value">{{ formatValue(node.value) }}</div>
              <div class="list-view__meta">#{{ node.id }} → {{ node.next === -1 ? 'null' : node.next }}</div>
              <span
                v-if="vizFlags.enableStepBadges && node.id === slistData.head"
                class="list-view__badge"
              >
                HEAD
              </span>
            </div>
          </div>
        </div>

        <!-- ── 双链表 ── -->
        <div v-else-if="dlistData" class="list-view">
          <div class="list-view__nodes">
            <div
              v-for="node in dlistOrdered"
              :key="node.id"
              class="list-view__node"
              :class="{
                'list-view__node--head': vizFlags.enableNodeColors && node.id === dlistData.head,
                'list-view__node--tail': vizFlags.enableNodeColors && node.id === dlistData.tail,
              }"
            >
              <div class="list-view__value">{{ formatValue(node.value) }}</div>
              <div class="list-view__meta">{{ node.prev === -1 ? 'null' : node.prev }} ← #{{ node.id }} → {{ node.next === -1 ? 'null' : node.next }}</div>
              <span v-if="vizFlags.enableStepBadges && node.id === dlistData.head" class="list-view__badge">HEAD</span>
              <span v-if="vizFlags.enableStepBadges && node.id === dlistData.tail" class="list-view__badge list-view__badge--tail">TAIL</span>
            </div>
          </div>
        </div>

        <!-- ── 树 ── -->
        <div v-else-if="treeLayout" class="tree-view">
          <svg
            class="tree-view__svg"
            :viewBox="`0 0 ${treeLayout.svgWidth} ${treeLayout.svgHeight}`"
            preserveAspectRatio="xMidYMid meet"
          >
            <line
              v-for="(edge, i) in treeLayout.edges"
              :key="`te-${i}`"
              :x1="edge.x1"
              :y1="edge.y1"
              :x2="edge.x2"
              :y2="edge.y2"
              class="tree-view__edge"
            />
            <g v-for="node in treeLayout.nodes" :key="`tn-${node.id}`">
              <circle
                :cx="node.x"
                :cy="node.y"
                :r="treeLayout.nodeRadius"
                class="tree-view__node"
                :class="{ 'tree-view__node--root': vizFlags.enableNodeColors && node.isRoot }"
              />
              <text :x="node.x" :y="node.y + 4" class="tree-view__label">
                {{ formatValue(node.value) }}
              </text>
            </g>
          </svg>
        </div>

        <!-- ── 图 ── -->
        <div v-else-if="graphLayout" class="graph-view">
          <svg
            class="graph-view__svg"
            :viewBox="`0 0 ${graphLayout.svgWidth} ${graphLayout.svgHeight}`"
            preserveAspectRatio="xMidYMid meet"
          >
            <defs>
              <marker
                v-if="vizFlags.enableGraphArrows && isDirected"
                id="arrow"
                markerWidth="10"
                markerHeight="10"
                refX="9"
                refY="5"
                orient="auto"
              >
                <path d="M 0 0 L 10 5 L 0 10 z" class="graph-view__arrow" />
              </marker>
            </defs>
            <line
              v-for="(edge, i) in graphLayout.edges"
              :key="`ge-${i}`"
              :x1="edge.x1"
              :y1="edge.y1"
              :x2="edge.x2"
              :y2="edge.y2"
              class="graph-view__edge"
              :marker-end="vizFlags.enableGraphArrows && isDirected ? 'url(#arrow)' : undefined"
            />
            <g v-if="vizFlags.enableEdgeWeightLabels && isWeighted">
              <text
                v-for="(edge, i) in graphLayout.edges"
                :key="`gw-${i}`"
                :x="(edge.x1 + edge.x2) / 2"
                :y="(edge.y1 + edge.y2) / 2 - 6"
                class="graph-view__weight"
              >
                {{ edge.weight }}
              </text>
            </g>
            <g v-for="node in graphLayout.nodes" :key="`gn-${node.id}`">
              <circle
                :cx="node.x"
                :cy="node.y"
                :r="graphLayout.nodeRadius"
                class="graph-view__node"
              />
              <text :x="node.x" :y="node.y + 4" class="graph-view__label">
                {{ node.label }}
              </text>
            </g>
          </svg>
        </div>
      </div>
    </div>
  </section>
</template>

<style scoped>
.viz-panel {
  display: flex;
  flex-direction: column;
  flex: 1;
  min-height: 0;
  border: 1px solid var(--color-border);
  border-radius: var(--radius-lg);
  background-color: var(--color-bg-surface);
  overflow: hidden;
}

.viz-panel__header {
  display: flex;
  align-items: center;
  justify-content: space-between;
  flex-shrink: 0;
  padding: var(--space-2) var(--space-3);
  border-bottom: 1px solid var(--color-border);
  background-color: var(--color-bg-surface-alt);
}

.viz-panel__title {
  display: flex;
  align-items: center;
  gap: 6px;
  font-size: var(--text-sm);
  font-weight: var(--weight-semibold);
  color: var(--color-text-primary);
}

.viz-panel__title :deep(.material-icon) {
  width: 18px;
  height: 18px;
}

.viz-panel__step {
  display: flex;
  align-items: center;
  gap: 6px;
  font-size: var(--text-xs);
  color: var(--color-text-tertiary);
}

.viz-panel__step-op {
  padding: 2px 6px;
  border-radius: var(--radius-sm);
  background-color: var(--color-accent-wash);
  color: var(--color-accent);
}

.viz-panel__step-line {
  padding: 2px 6px;
  border-radius: var(--radius-sm);
  background-color: var(--color-bg-hover);
}

.viz-panel__body {
  flex: 1;
  min-height: 0;
  padding: var(--space-3);
  overflow: auto;
}

.viz-panel__content {
  display: flex;
  flex-direction: column;
  gap: var(--space-2);
}

/* ---- 栈/队列/链表共用网格 ---- */

.stack-view__items,
.queue-view__items,
.list-view__nodes {
  display: grid;
  grid-template-columns: repeat(auto-fill, minmax(120px, 1fr));
  gap: var(--space-2);
}

.list-view__nodes {
  grid-template-columns: repeat(auto-fill, minmax(140px, 1fr));
}

.stack-view__item,
.queue-view__item,
.list-view__node {
  position: relative;
  padding: var(--space-2);
  border: 1px solid var(--color-border);
  border-radius: var(--radius-md);
  background-color: var(--color-bg-surface);
}

.stack-view__item--top,
.queue-view__item--front,
.queue-view__item--rear,
.list-view__node--head,
.list-view__node--tail {
  border-color: var(--color-accent);
  background-color: var(--color-accent-wash);
}

.stack-view__badge,
.queue-view__badge,
.list-view__badge {
  position: absolute;
  top: -8px;
  right: 8px;
  padding: 2px 6px;
  border-radius: var(--radius-sm);
  background-color: var(--color-accent);
  color: var(--color-accent-contrast);
  font-size: var(--text-xs);
  font-weight: var(--weight-medium);
}

.list-view__badge--tail {
  right: auto;
  left: 8px;
}

.stack-view__value,
.queue-view__value,
.list-view__value {
  font-size: var(--text-base);
  font-weight: var(--weight-semibold);
  color: var(--color-text-primary);
}

.list-view__meta {
  margin-top: 4px;
  font-size: var(--text-xs);
  color: var(--color-text-tertiary);
  font-family: var(--font-mono);
}

/* ---- 树 SVG ---- */

.tree-view__svg,
.graph-view__svg {
  width: 100%;
  height: 320px;
  border: 1px solid var(--color-border);
  border-radius: var(--radius-lg);
  background-color: var(--color-bg-surface-alt);
}

.tree-view__edge {
  stroke: rgba(0, 0, 0, 0.12);
  stroke-width: 1;
}

.tree-view__node {
  fill: #ffffff;
  stroke: var(--color-border-strong);
  stroke-width: 1;
}

.tree-view__node--root {
  fill: var(--color-accent-wash);
  stroke: var(--color-accent);
}

.tree-view__label {
  font-size: 12px;
  fill: var(--color-text-primary);
  text-anchor: middle;
}

/* ---- 图 SVG ---- */

.graph-view__edge {
  stroke: rgba(0, 0, 0, 0.12);
  stroke-width: 1.2;
}

.graph-view__arrow {
  fill: var(--color-border-strong);
}

.graph-view__node {
  fill: #ffffff;
  stroke: var(--color-border-strong);
  stroke-width: 1;
}

.graph-view__label {
  font-size: 12px;
  fill: var(--color-text-primary);
  text-anchor: middle;
}

.graph-view__weight {
  font-size: 11px;
  fill: var(--color-accent);
  text-anchor: middle;
}

/* ---- 动画（仅在 flag 开启时生效） ---- */

.viz-panel--animated .tree-view__edge {
  transition:
    x1 var(--duration-viz) var(--ease),
    y1 var(--duration-viz) var(--ease),
    x2 var(--duration-viz) var(--ease),
    y2 var(--duration-viz) var(--ease);
}

.viz-panel--animated .tree-view__node {
  transition:
    cx var(--duration-viz) var(--ease),
    cy var(--duration-viz) var(--ease),
    fill var(--duration-viz) var(--ease),
    stroke var(--duration-viz) var(--ease);
}

.viz-panel--animated .tree-view__label {
  transition:
    x var(--duration-viz) var(--ease),
    y var(--duration-viz) var(--ease);
}

.viz-panel--animated .graph-view__edge {
  transition:
    x1 var(--duration-viz) var(--ease),
    y1 var(--duration-viz) var(--ease),
    x2 var(--duration-viz) var(--ease),
    y2 var(--duration-viz) var(--ease);
}

.viz-panel--animated .graph-view__node {
  transition:
    cx var(--duration-viz) var(--ease),
    cy var(--duration-viz) var(--ease),
    fill var(--duration-viz) var(--ease),
    stroke var(--duration-viz) var(--ease);
}

.viz-panel--animated .stack-view__item,
.viz-panel--animated .queue-view__item,
.viz-panel--animated .list-view__node {
  transition:
    border-color var(--duration-viz) var(--ease),
    background-color var(--duration-viz) var(--ease);
}

@media (max-width: 1100px) {
  .tree-view__svg,
  .graph-view__svg {
    height: 260px;
  }
}
</style>
