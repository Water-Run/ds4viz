<script setup lang="ts">
/**
 * 可视化面板
 *
 * @component VizPanel
 */

import { computed } from 'vue'
import type {
  IrStateData,
  StackStateData,
  QueueStateData,
  SListStateData,
  DListStateData,
  BinaryTreeStateData,
  GraphStateData,
  GraphWeightedStateData,
  IrObjectKind,
  BinaryTreeNode,
} from '@/types/ir'
import type { StepSummary } from '@/types/viz'

import VizPlaceholder from './VizPlaceholder.vue'
import MaterialIcon from '@/components/common/MaterialIcon.vue'

/**
 * 组件属性定义
 */
interface Props {
  /** 结构类型 */
  kind?: IrObjectKind
  /** 状态数据 */
  data?: IrStateData
  /** 步骤信息 */
  step?: StepSummary | null
}

const props = defineProps<Props>()

/**
 * 是否为空状态
 */
const isEmpty = computed<boolean>(() => props.data === undefined)

/**
 * 类型标题
 */
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
  return props.kind ? mapping[props.kind] : ''
})

/**
 * 栈数据
 */
const stackData = computed<StackStateData | null>(() =>
  props.kind === 'stack' ? (props.data as StackStateData) : null,
)

/**
 * 队列数据
 */
const queueData = computed<QueueStateData | null>(() =>
  props.kind === 'queue' ? (props.data as QueueStateData) : null,
)

/**
 * 单链表数据
 */
const slistData = computed<SListStateData | null>(() =>
  props.kind === 'slist' ? (props.data as SListStateData) : null,
)

/**
 * 双链表数据
 */
const dlistData = computed<DListStateData | null>(() =>
  props.kind === 'dlist' ? (props.data as DListStateData) : null,
)

/**
 * 树数据
 */
const treeData = computed<BinaryTreeStateData | null>(() => {
  if (props.kind === 'binary_tree' || props.kind === 'bst') {
    return props.data as BinaryTreeStateData
  }
  return null
})

/**
 * 图数据
 */
const graphData = computed<GraphStateData | GraphWeightedStateData | null>(() => {
  if (props.kind === 'graph_undirected' || props.kind === 'graph_directed') {
    return props.data as GraphStateData
  }
  if (props.kind === 'graph_weighted') {
    return props.data as GraphWeightedStateData
  }
  return null
})

/**
 * 是否带权图
 */
const isWeighted = computed<boolean>(() => props.kind === 'graph_weighted')

/**
 * 是否有向图
 */
const isDirected = computed<boolean>(() => props.kind === 'graph_directed')

/**
 * 提取高亮节点 ID
 */
const highlightIds = computed<Set<number>>(() => {
  const ids = new Set<number>()
  if (!props.step) return ids
  collectNumbers(props.step.args, ids)
  collectNumbers(props.step.ret, ids)
  collectSpecialIndices(props.step.args, ids)
  return ids
})

/**
 * 提取高亮边
 */
const highlightEdges = computed<Array<{ from: number; to: number }>>(() => {
  const edges: Array<{ from: number; to: number }> = []
  if (!props.step) return edges
  collectEdges(props.step.args, edges)
  collectEdges(props.step.ret, edges)
  return edges
})

/**
 * 树布局
 */
const treeLayout = computed(() => {
  if (!treeData.value) {
    return null
  }
  const width = 520
  const height = 300
  const levelHeight = 80
  const nodeRadius = 16

  const nodes = treeData.value.nodes
  const nodeMap = new Map<number, BinaryTreeNode>()
  nodes.forEach((node) => nodeMap.set(node.id, node))

  const levels: number[][] = []
  const rootId = treeData.value.root
  if (rootId === -1) {
    return { width, height, nodeRadius, nodes: [], edges: [] }
  }

  const queue: Array<{ id: number; level: number }> = [{ id: rootId, level: 0 }]
  const visited = new Set<number>()
  while (queue.length > 0) {
    const current = queue.shift()
    if (!current) break
    if (visited.has(current.id)) continue
    visited.add(current.id)
    if (!levels[current.level]) {
      levels[current.level] = []
    }
    levels[current.level].push(current.id)
    const node = nodeMap.get(current.id)
    if (!node) continue
    if (node.left !== -1) queue.push({ id: node.left, level: current.level + 1 })
    if (node.right !== -1) queue.push({ id: node.right, level: current.level + 1 })
  }

  const positions = new Map<number, { x: number; y: number }>()
  levels.forEach((levelNodes, level) => {
    const gap = width / (levelNodes.length + 1)
    levelNodes.forEach((id, index) => {
      positions.set(id, { x: gap * (index + 1), y: 40 + level * levelHeight })
    })
  })

  const layoutNodes = nodes
    .filter((node) => positions.has(node.id))
    .map((node) => ({
      id: node.id,
      value: node.value,
      x: positions.get(node.id)!.x,
      y: positions.get(node.id)!.y,
      isRoot: node.id === rootId,
    }))

  const edges: Array<{ from: number; to: number }> = []
  nodes.forEach((node) => {
    if (node.left !== -1) edges.push({ from: node.id, to: node.left })
    if (node.right !== -1) edges.push({ from: node.id, to: node.right })
  })

  return { width, height, nodeRadius, nodes: layoutNodes, edges }
})

/**
 * 图布局
 */
const graphLayout = computed(() => {
  if (!graphData.value) {
    return null
  }
  const width = 520
  const height = 300
  const radius = 110
  const nodes = graphData.value.nodes
  const edges = graphData.value.edges

  const layoutNodes = nodes.map((node, index) => {
    const angle = (2 * Math.PI * index) / Math.max(1, nodes.length)
    const x = width / 2 + radius * Math.cos(angle)
    const y = height / 2 + radius * Math.sin(angle)
    return { id: node.id, label: node.label, x, y }
  })

  const nodeMap = new Map<number, { x: number; y: number }>()
  layoutNodes.forEach((node) => nodeMap.set(node.id, { x: node.x, y: node.y }))

  const layoutEdges = edges
    .map((edge) => {
      const from = nodeMap.get(edge.from)
      const to = nodeMap.get(edge.to)
      if (!from || !to) {
        return null
      }
      return {
        from: edge.from,
        to: edge.to,
        x1: from.x,
        y1: from.y,
        x2: to.x,
        y2: to.y,
        weight: (edge as { weight?: number }).weight,
      }
    })
    .filter((edge): edge is NonNullable<typeof edge> => edge !== null)

  return { width, height, nodes: layoutNodes, edges: layoutEdges }
})

/**
 * 判断节点是否高亮
 */
const isNodeHighlighted = (id: number): boolean => highlightIds.value.has(id)

/**
 * 判断边是否高亮
 */
const isEdgeHighlighted = (from: number, to: number): boolean => {
  if (highlightEdges.value.length === 0) return false
  return highlightEdges.value.some((edge) => {
    if (edge.from === from && edge.to === to) return true
    if (!isDirected.value && edge.from === to && edge.to === from) return true
    return false
  })
}

/**
 * 递归提取数字
 */
function collectNumbers(value: unknown, ids: Set<number>): void {
  if (typeof value === 'number' && Number.isFinite(value)) {
    ids.add(value)
    return
  }
  if (Array.isArray(value)) {
    value.forEach((item) => collectNumbers(item, ids))
    return
  }
  if (value && typeof value === 'object') {
    Object.values(value as Record<string, unknown>).forEach((item) => collectNumbers(item, ids))
  }
}

/**
 * 提取特殊索引
 */
function collectSpecialIndices(value: unknown, ids: Set<number>): void {
  if (!value || typeof value !== 'object') return
  if (Array.isArray(value)) {
    value.forEach((item) => collectSpecialIndices(item, ids))
    return
  }
  const record = value as Record<string, unknown>
  const keys = ['top', 'front', 'rear', 'head', 'tail', 'root', 'node', 'index', 'id']
  keys.forEach((key) => {
    const item = record[key]
    if (typeof item === 'number' && Number.isFinite(item)) {
      ids.add(item)
    }
  })
  Object.values(record).forEach((item) => collectSpecialIndices(item, ids))
}

/**
 * 递归提取边
 */
function collectEdges(value: unknown, edges: Array<{ from: number; to: number }>): void {
  if (Array.isArray(value)) {
    value.forEach((item) => collectEdges(item, edges))
    return
  }
  if (value && typeof value === 'object') {
    const record = value as Record<string, unknown>
    const from = record.from
    const to = record.to
    if (typeof from === 'number' && typeof to === 'number') {
      edges.push({ from, to })
    }
    Object.values(record).forEach((item) => collectEdges(item, edges))
  }
}
</script>

<template>
  <section class="viz-panel">
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
        <div v-if="stackData" class="stack-view">
          <div class="stack-view__items">
            <div
              v-for="(item, index) in stackData.items"
              :key="index"
              class="stack-view__item"
              :class="{ 'stack-view__item--top': index === stackData.top }"
            >
              <span class="stack-view__value">{{ item }}</span>
              <span v-if="index === stackData.top" class="stack-view__badge">TOP</span>
            </div>
          </div>
        </div>

        <div v-else-if="queueData" class="queue-view">
          <div class="queue-view__items">
            <div
              v-for="(item, index) in queueData.items"
              :key="index"
              class="queue-view__item"
              :class="{
                'queue-view__item--front': index === queueData.front,
                'queue-view__item--rear': index === queueData.rear,
              }"
            >
              <span class="queue-view__value">{{ item }}</span>
              <span v-if="index === queueData.front" class="queue-view__badge">FRONT</span>
              <span v-if="index === queueData.rear" class="queue-view__badge">REAR</span>
            </div>
          </div>
        </div>

        <div v-else-if="slistData" class="list-view">
          <div class="list-view__nodes">
            <div
              v-for="node in slistData.nodes"
              :key="node.id"
              class="list-view__node"
              :class="{ 'list-view__node--head': node.id === slistData.head }"
            >
              <div
                class="list-view__value"
                :class="{ 'list-view__value--highlight': isNodeHighlighted(node.id) }"
              >
                {{ node.value }}
              </div>
              <div class="list-view__meta">#{{ node.id }} -> {{ node.next }}</div>
              <span v-if="node.id === slistData.head" class="list-view__badge">HEAD</span>
            </div>
          </div>
        </div>

        <div v-else-if="dlistData" class="list-view">
          <div class="list-view__nodes">
            <div
              v-for="node in dlistData.nodes"
              :key="node.id"
              class="list-view__node"
              :class="{
                'list-view__node--head': node.id === dlistData.head,
                'list-view__node--tail': node.id === dlistData.tail,
              }"
            >
              <div
                class="list-view__value"
                :class="{ 'list-view__value--highlight': isNodeHighlighted(node.id) }"
              >
                {{ node.value }}
              </div>
              <div class="list-view__meta">#{{ node.id }} <- {{ node.prev }} -> {{ node.next }}</div>
              <span v-if="node.id === dlistData.head" class="list-view__badge">HEAD</span>
              <span v-if="node.id === dlistData.tail" class="list-view__badge">TAIL</span>
            </div>
          </div>
        </div>

        <div v-else-if="treeLayout" class="tree-view">
          <svg
            class="tree-view__svg"
            :viewBox="`0 0 ${treeLayout.width} ${treeLayout.height}`"
            preserveAspectRatio="xMidYMid meet"
          >
            <line
              v-for="(edge, index) in treeLayout.edges"
              :key="`edge-${index}`"
              :x1="treeLayout.nodes.find((node) => node.id === edge.from)?.x"
              :y1="treeLayout.nodes.find((node) => node.id === edge.from)?.y"
              :x2="treeLayout.nodes.find((node) => node.id === edge.to)?.x"
              :y2="treeLayout.nodes.find((node) => node.id === edge.to)?.y"
              class="tree-view__edge"
            />
            <g v-for="node in treeLayout.nodes" :key="node.id">
              <circle
                :cx="node.x"
                :cy="node.y"
                :r="treeLayout.nodeRadius"
                class="tree-view__node"
                :class="{
                  'tree-view__node--root': node.isRoot,
                  'tree-view__node--highlight': isNodeHighlighted(node.id),
                }"
              />
              <text :x="node.x" :y="node.y + 4" class="tree-view__label">{{ node.value }}</text>
            </g>
          </svg>
        </div>

        <div v-else-if="graphLayout" class="graph-view">
          <svg
            class="graph-view__svg"
            :viewBox="`0 0 ${graphLayout.width} ${graphLayout.height}`"
            preserveAspectRatio="xMidYMid meet"
          >
            <defs>
              <marker
                v-if="isDirected"
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
              v-for="(edge, index) in graphLayout.edges"
              :key="`edge-${index}`"
              :x1="edge.x1"
              :y1="edge.y1"
              :x2="edge.x2"
              :y2="edge.y2"
              class="graph-view__edge"
              :class="{ 'graph-view__edge--highlight': isEdgeHighlighted(edge.from, edge.to) }"
              :marker-end="isDirected ? 'url(#arrow)' : undefined"
            />
            <g v-if="isWeighted">
              <text
                v-for="(edge, index) in graphLayout.edges"
                :key="`weight-${index}`"
                :x="(edge.x1 + edge.x2) / 2"
                :y="(edge.y1 + edge.y2) / 2 - 6"
                class="graph-view__weight"
              >
                {{ edge.weight }}
              </text>
            </g>
            <g v-for="node in graphLayout.nodes" :key="node.id">
              <circle
                :cx="node.x"
                :cy="node.y"
                r="16"
                class="graph-view__node"
                :class="{ 'graph-view__node--highlight': isNodeHighlighted(node.id) }"
              />
              <text :x="node.x" :y="node.y + 4" class="graph-view__label">{{ node.label }}</text>
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
  height: 100%;
  border: 1px solid var(--color-border);
  border-radius: var(--radius-lg);
  background-color: var(--color-bg-surface);
  overflow: hidden;
}

.viz-panel__header {
  display: flex;
  align-items: center;
  justify-content: space-between;
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
  padding: var(--space-3);
  overflow: auto;
}

.viz-panel__content {
  display: flex;
  flex-direction: column;
  gap: var(--space-2);
}

.viz-panel__content > * {
  animation: vizFadeIn var(--duration-viz) var(--ease);
}

.stack-view__items,
.queue-view__items {
  display: grid;
  grid-template-columns: repeat(auto-fill, minmax(120px, 1fr));
  gap: var(--space-2);
}

.stack-view__item,
.queue-view__item,
.list-view__node {
  position: relative;
  padding: var(--space-2);
  border: 1px solid var(--color-border);
  border-radius: var(--radius-md);
  background-color: var(--color-bg-surface);
  transition: transform var(--duration-viz) var(--ease);
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
}

.list-view__nodes {
  display: grid;
  grid-template-columns: repeat(auto-fill, minmax(140px, 1fr));
  gap: var(--space-2);
}

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
  transition: stroke var(--duration-viz) var(--ease);
}

.tree-view__node {
  fill: #ffffff;
  stroke: var(--color-border-strong);
  stroke-width: 1;
  transition:
    fill var(--duration-viz) var(--ease),
    stroke var(--duration-viz) var(--ease),
    transform var(--duration-viz) var(--ease);
}

.tree-view__node--root {
  fill: var(--color-accent-wash);
  stroke: var(--color-accent);
}

.tree-view__label,
.graph-view__label {
  font-size: 12px;
  fill: var(--color-text-primary);
  text-anchor: middle;
}

.graph-view__edge {
  stroke: rgba(0, 0, 0, 0.12);
  stroke-width: 1.2;
  transition: stroke var(--duration-viz) var(--ease);
}

.graph-view__edge--highlight {
  stroke: var(--color-accent);
}

.graph-view__arrow {
  fill: var(--color-border-strong);
}

.graph-view__node {
  fill: #ffffff;
  stroke: var(--color-border-strong);
  stroke-width: 1;
  transition:
    fill var(--duration-viz) var(--ease),
    stroke var(--duration-viz) var(--ease);
}

.graph-view__node--highlight {
  fill: var(--color-accent-wash);
  stroke: var(--color-accent);
}

.list-view__value--highlight {
  color: var(--color-accent);
}

.graph-view__weight {
  font-size: 11px;
  fill: var(--color-accent);
  text-anchor: middle;
}

@media (max-width: 1100px) {
  .tree-view__svg,
  .graph-view__svg {
    height: 260px;
  }
}

@keyframes vizFadeIn {
  from {
    opacity: 0;
    transform: translateY(6px);
  }
  to {
    opacity: 1;
    transform: translateY(0);
  }
}

.tree-view__node--highlight {
  fill: var(--color-accent-wash);
  stroke: var(--color-accent);
}

</style>
