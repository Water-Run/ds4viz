<script setup lang="ts">
/**
 * 可视化面板（完整重写）
 *
 * 支持 Stack / Queue / SList / DList / BinaryTree / BST / Heap / Graph 渲染，
 * 提供缩放/平移、Tooltip、Diff 高亮、平滑过渡、元数据显示。
 *
 * @file src/components/viz/VizPanel.vue
 * @author WaterRun
 * @date 2026-03-02
 * @component VizPanel
 */

import { computed, onBeforeUnmount, ref, shallowRef, watch } from 'vue'

import type {
  IrStateData, IrValue, IrRemarks,
  StackStateData, QueueStateData,
  SListStateData, SListNode,
  DListStateData, DListNode,
  BinaryTreeStateData, BinaryTreeNode,
  GraphStateData, GraphWeightedStateData, GraphNode, GraphEdge,
  IrObjectKind,
} from '@/types/ir'
import type { StepSummary } from '@/types/viz'
import { vizFlags, logDebug } from '@/utils/viz-flags'

import VizPlaceholder from './VizPlaceholder.vue'
import VizSettings from './VizSettings.vue'
import MaterialIcon from '@/components/common/MaterialIcon.vue'

/* ---- Props ---- */

/**
 * 组件属性定义
 */
interface Props {
  kind?: IrObjectKind
  data?: IrStateData
  step?: StepSummary | null
  label?: string
  remarks?: IrRemarks
}

const props = defineProps<Props>()

/* ================================================================
 *  常量
 * ================================================================ */

const CYL_RX = 64
const CYL_RY = 12
const CYL_ITEM_H = 40

const BOX_W = 72
const BOX_H = 44
const BOX_GAP = 36

const NODE_W = 96
const NODE_H = 44
const NODE_GAP = 44

const TREE_R = 20
const TREE_LEVEL_H = 72

const GRAPH_R = 22

const PAD = 40

const NODE_PALETTE = [
  '#0078d4', '#0e7c6b', '#7c3aed', '#c2410c',
  '#0369a1', '#6d28d9', '#b45309', '#059669',
  '#dc2626', '#4f46e5', '#0891b2', '#65a30d',
]

type DiffStatus = 'added' | 'removed' | 'unchanged'

/* ================================================================
 *  基础状态
 * ================================================================ */

const canvasRef = ref<HTMLDivElement | null>(null)
const isEmpty = computed<boolean>(() => props.data === undefined)

const kindStr = computed<string>(() => (props.kind ?? '') as string)

const isHeap = computed<boolean>(() => {
  const k = kindStr.value
  if (k === 'heap') return true
  if ((k === 'binary_tree' || k === 'bst') && (props.label?.toLowerCase().includes('heap') ?? false)) return true
  return false
})

const kindLabel = computed<string>(() => {
  if (isHeap.value) return 'Heap'
  const m: Record<string, string> = {
    stack: 'Stack', queue: 'Queue', slist: 'SList', dlist: 'DList',
    binary_tree: 'BinaryTree', bst: 'BST', heap: 'Heap',
    graph_undirected: 'Graph', graph_directed: 'Digraph', graph_weighted: 'WeightedGraph',
  }
  return m[kindStr.value] ?? kindStr.value
})

const isDirected = computed<boolean>(() => kindStr.value === 'graph_directed')
const isWeighted = computed<boolean>(() => kindStr.value === 'graph_weighted')

/* ================================================================
 *  数据窄化
 * ================================================================ */

const stackData = computed<StackStateData | null>(() => kindStr.value === 'stack' ? props.data as StackStateData : null)
const queueData = computed<QueueStateData | null>(() => kindStr.value === 'queue' ? props.data as QueueStateData : null)
const slistData = computed<SListStateData | null>(() => kindStr.value === 'slist' ? props.data as SListStateData : null)
const dlistData = computed<DListStateData | null>(() => kindStr.value === 'dlist' ? props.data as DListStateData : null)
const treeData = computed<BinaryTreeStateData | null>(() => {
  const k = kindStr.value
  if (k === 'binary_tree' || k === 'bst' || k === 'heap') return props.data as BinaryTreeStateData
  return null
})
const graphData = computed<GraphStateData | GraphWeightedStateData | null>(() => {
  const k = kindStr.value
  if (k === 'graph_undirected' || k === 'graph_directed' || k === 'graph_weighted') return props.data as GraphStateData
  return null
})

/* ================================================================
 *  ViewBox / Zoom / Pan
 * ================================================================ */

interface VB { x: number; y: number; w: number; h: number }

const vb = ref<VB>({ x: 0, y: 0, w: 500, h: 400 })
const viewBoxStr = computed<string>(() => `${vb.value.x} ${vb.value.y} ${vb.value.w} ${vb.value.h}`)

const isPanning = ref<boolean>(false)
const panOrigin = ref<{ cx: number; cy: number; vx: number; vy: number }>({ cx: 0, cy: 0, vx: 0, vy: 0 })

const onWheel = (e: WheelEvent): void => {
  const svg = e.currentTarget as SVGSVGElement
  const rect = svg.getBoundingClientRect()
  const mx = (e.clientX - rect.left) / rect.width
  const my = (e.clientY - rect.top) / rect.height
  const factor = e.deltaY > 0 ? 1.1 : 1 / 1.1
  const nw = vb.value.w * factor
  const nh = vb.value.h * factor
  vb.value = { x: vb.value.x + (vb.value.w - nw) * mx, y: vb.value.y + (vb.value.h - nh) * my, w: nw, h: nh }
}

const onPanStart = (e: PointerEvent): void => {
  isPanning.value = true
  panOrigin.value = { cx: e.clientX, cy: e.clientY, vx: vb.value.x, vy: vb.value.y }
  ;(e.currentTarget as SVGSVGElement).setPointerCapture(e.pointerId)
}

const onPanMove = (e: PointerEvent): void => {
  if (!isPanning.value) return
  const svg = (e.currentTarget as SVGSVGElement)
  const rect = svg.getBoundingClientRect()
  const dx = (e.clientX - panOrigin.value.cx) / rect.width * vb.value.w
  const dy = (e.clientY - panOrigin.value.cy) / rect.height * vb.value.h
  vb.value = { ...vb.value, x: panOrigin.value.vx - dx, y: panOrigin.value.vy - dy }
}

const onPanEnd = (): void => { isPanning.value = false }

const fitViewBox = (minX: number, minY: number, maxX: number, maxY: number): void => {
  if (!vizFlags.enableAutoFit) return
  const w = Math.max(200, maxX - minX + PAD * 2)
  const h = Math.max(150, maxY - minY + PAD * 2)
  vb.value = { x: minX - PAD, y: minY - PAD, w, h }
}

/* ================================================================
 *  Tooltip
 * ================================================================ */

const tip = ref<{ show: boolean; text: string; x: number; y: number }>({ show: false, text: '', x: 0, y: 0 })
const tipStyle = computed(() => ({ left: `${tip.value.x}px`, top: `${tip.value.y}px` }))

const showTip = (text: string, e: PointerEvent): void => {
  if (!canvasRef.value) return
  const r = canvasRef.value.getBoundingClientRect()
  tip.value = { show: true, text, x: e.clientX - r.left + 12, y: e.clientY - r.top - 8 }
}
const hideTip = (): void => { tip.value = { ...tip.value, show: false } }

/* ================================================================
 *  Diff 高亮
 * ================================================================ */

let prevIds: Set<string> = new Set()
const addedIds = shallowRef<Set<string>>(new Set())
const removedKeys = shallowRef<Array<{ key: string; x: number; y: number; r?: number; w?: number; h?: number }>>([])
let ghostTimer: number | null = null

function computeDiff(currentKeys: Map<string, { x: number; y: number; r?: number; w?: number; h?: number }>): void {
  if (!vizFlags.enableDiffHighlight) {
    addedIds.value = new Set()
    removedKeys.value = []
    prevIds = new Set(currentKeys.keys())
    return
  }
  const curSet = new Set(currentKeys.keys())
  const added = new Set<string>()
  const removed: typeof removedKeys.value = []
  for (const k of curSet) { if (!prevIds.has(k)) added.add(k) }
  for (const k of prevIds) {
    if (!curSet.has(k)) {
      const cached = prevPositionCache.get(k)
      if (cached) removed.push({ key: k, ...cached })
    }
  }
  addedIds.value = added
  removedKeys.value = removed
  prevIds = curSet
  prevPositionCache.clear()
  for (const [k, v] of currentKeys) prevPositionCache.set(k, v)
  if (removed.length > 0) {
    if (ghostTimer !== null) window.clearTimeout(ghostTimer)
    ghostTimer = window.setTimeout(() => { removedKeys.value = [] }, 350)
  }
}

const prevPositionCache = new Map<string, { x: number; y: number; r?: number; w?: number; h?: number }>()

/* ================================================================
 *  格式化工具
 * ================================================================ */

function fmtVal(v: IrValue): string {
  if (typeof v === 'string') return v.length > 12 ? `${v.slice(0, 10)}…` : v
  return String(v)
}
function fullVal(v: IrValue): string { return typeof v === 'string' ? `"${v}"` : String(v) }

/* ================================================================
 *  链表排序
 * ================================================================ */

const slistOrdered = computed<SListNode[]>(() => {
  if (!slistData.value) return []
  const map = new Map<number, SListNode>()
  slistData.value.nodes.forEach((n) => map.set(n.id, n))
  const result: SListNode[] = []
  let cur = slistData.value.head
  const visited = new Set<number>()
  while (cur !== -1 && !visited.has(cur)) { visited.add(cur); const n = map.get(cur); if (!n) break; result.push(n); cur = n.next }
  return result
})

const dlistOrdered = computed<DListNode[]>(() => {
  if (!dlistData.value) return []
  const map = new Map<number, DListNode>()
  dlistData.value.nodes.forEach((n) => map.set(n.id, n))
  const result: DListNode[] = []
  let cur = dlistData.value.head
  const visited = new Set<number>()
  while (cur !== -1 && !visited.has(cur)) { visited.add(cur); const n = map.get(cur); if (!n) break; result.push(n); cur = n.next }
  return result
})

/* ================================================================
 *  Stack 布局
 * ================================================================ */

interface StackLayout { svgW: number; svgH: number; cx: number; items: Array<{ value: IrValue; y: number; isTop: boolean; diff: DiffStatus }> }
const stackLayout = shallowRef<StackLayout | null>(null)

watch(() => stackData.value, (d) => {
  if (!d) { stackLayout.value = null; return }
  const n = Math.max(d.items.length, 1)
  const svgW = CYL_RX * 2 + PAD * 2
  const svgH = n * CYL_ITEM_H + CYL_RY * 2 + PAD * 2 + 20
  const cx = svgW / 2
  const baseY = svgH - PAD - CYL_RY
  const positions = new Map<string, { x: number; y: number }>()
  const items = d.items.map((v, i) => {
    const y = baseY - (i + 0.5) * CYL_ITEM_H
    positions.set(`s-${i}`, { x: cx, y })
    return { value: v, y, isTop: i === d.top, diff: 'unchanged' as DiffStatus }
  })
  computeDiff(positions)
  items.forEach((it, i) => { if (addedIds.value.has(`s-${i}`)) it.diff = 'added' })
  stackLayout.value = { svgW, svgH, cx, items }
  fitViewBox(0, 0, svgW, svgH)
}, { immediate: true })

/* ================================================================
 *  Queue 布局
 * ================================================================ */

interface QueueLayout { svgW: number; svgH: number; items: Array<{ value: IrValue; x: number; isFront: boolean; isRear: boolean; diff: DiffStatus }> }
const queueLayout = shallowRef<QueueLayout | null>(null)

watch(() => queueData.value, (d) => {
  if (!d) { queueLayout.value = null; return }
  const n = Math.max(d.items.length, 1)
  const svgW = n * BOX_W + (n - 1) * BOX_GAP + PAD * 2
  const svgH = BOX_H + PAD * 2 + 24
  const positions = new Map<string, { x: number; y: number; w: number; h: number }>()
  const items = d.items.map((v, i) => {
    const x = PAD + i * (BOX_W + BOX_GAP)
    positions.set(`q-${i}`, { x, y: PAD + 20, w: BOX_W, h: BOX_H })
    return { value: v, x, isFront: i === d.front, isRear: i === d.rear, diff: 'unchanged' as DiffStatus }
  })
  computeDiff(positions)
  items.forEach((it, i) => { if (addedIds.value.has(`q-${i}`)) it.diff = 'added' })
  queueLayout.value = { svgW, svgH, items }
  fitViewBox(0, 0, svgW, svgH)
}, { immediate: true })

/* ================================================================
 *  SList 布局
 * ================================================================ */

interface ListLayout { svgW: number; svgH: number; nodes: Array<{ id: number; value: IrValue; x: number; isHead: boolean; isTail: boolean; diff: DiffStatus }> }
const slistLayout = shallowRef<ListLayout | null>(null)

watch(() => slistOrdered.value, (ordered) => {
  if (!slistData.value) { slistLayout.value = null; return }
  const n = Math.max(ordered.length, 1)
  const svgW = n * NODE_W + (n - 1) * NODE_GAP + PAD * 2 + 40
  const svgH = NODE_H + PAD * 2 + 24
  const positions = new Map<string, { x: number; y: number; w: number; h: number }>()
  const nodes = ordered.map((nd, i) => {
    const x = PAD + i * (NODE_W + NODE_GAP)
    positions.set(`n-${nd.id}`, { x, y: PAD + 20, w: NODE_W, h: NODE_H })
    return { id: nd.id, value: nd.value, x, isHead: nd.id === slistData.value!.head, isTail: nd.next === -1, diff: 'unchanged' as DiffStatus }
  })
  computeDiff(positions)
  nodes.forEach((nd) => { if (addedIds.value.has(`n-${nd.id}`)) nd.diff = 'added' })
  slistLayout.value = { svgW, svgH, nodes }
  fitViewBox(0, 0, svgW, svgH)
}, { immediate: true })

/* ================================================================
 *  DList 布局
 * ================================================================ */

const dlistLayout = shallowRef<ListLayout | null>(null)

watch(() => dlistOrdered.value, (ordered) => {
  if (!dlistData.value) { dlistLayout.value = null; return }
  const n = Math.max(ordered.length, 1)
  const svgW = n * NODE_W + (n - 1) * NODE_GAP + PAD * 2 + 40
  const svgH = NODE_H + PAD * 2 + 24
  const positions = new Map<string, { x: number; y: number; w: number; h: number }>()
  const nodes = ordered.map((nd, i) => {
    const x = PAD + i * (NODE_W + NODE_GAP)
    positions.set(`n-${nd.id}`, { x, y: PAD + 20, w: NODE_W, h: NODE_H })
    return { id: nd.id, value: nd.value, x, isHead: nd.id === dlistData.value!.head, isTail: nd.id === dlistData.value!.tail, diff: 'unchanged' as DiffStatus }
  })
  computeDiff(positions)
  nodes.forEach((nd) => { if (addedIds.value.has(`n-${nd.id}`)) nd.diff = 'added' })
  dlistLayout.value = { svgW, svgH, nodes }
  fitViewBox(0, 0, svgW, svgH)
}, { immediate: true })

/* ================================================================
 *  Tree 布局（BinaryTree / BST / Heap）
 * ================================================================ */

interface TreeLayoutNode { id: number; value: IrValue; x: number; y: number; isRoot: boolean; diff: DiffStatus }
interface TreeLayoutEdge { x1: number; y1: number; x2: number; y2: number }
interface HeapArrayItem { id: number; value: IrValue; index: number; x: number }
interface TreeLayout { svgW: number; svgH: number; nodes: TreeLayoutNode[]; edges: TreeLayoutEdge[]; heapArray: HeapArrayItem[]; heapArrayY: number }

const treeLayout = shallowRef<TreeLayout | null>(null)

watch(() => treeData.value, (d) => {
  if (!d || d.root === -1 || d.nodes.length === 0) { treeLayout.value = null; return }
  logDebug('[viz] tree layout, nodes =', d.nodes.length)

  const nodeMap = new Map<number, BinaryTreeNode>()
  d.nodes.forEach((n) => nodeMap.set(n.id, n))

  const levels: number[][] = []
  const bfsQ: Array<{ id: number; lvl: number }> = [{ id: d.root, lvl: 0 }]
  const vis = new Set<number>()
  while (bfsQ.length > 0) {
    const c = bfsQ.shift()!
    if (vis.has(c.id)) continue
    vis.add(c.id)
    if (!levels[c.lvl]) levels[c.lvl] = []
    levels[c.lvl].push(c.id)
    const n = nodeMap.get(c.id)
    if (!n) continue
    if (n.left !== -1) bfsQ.push({ id: n.left, lvl: c.lvl + 1 })
    if (n.right !== -1) bfsQ.push({ id: n.right, lvl: c.lvl + 1 })
  }

  const maxInLvl = Math.max(...levels.map((l) => l.length))
  const baseW = Math.max(300, maxInLvl * TREE_R * 5 + PAD * 2)
  const posMap = new Map<number, { x: number; y: number }>()
  levels.forEach((ids, lvl) => {
    const gap = baseW / (ids.length + 1)
    ids.forEach((id, i) => posMap.set(id, { x: gap * (i + 1), y: PAD + lvl * TREE_LEVEL_H }))
  })

  let maxX = 0, maxY = 0
  posMap.forEach((p) => { if (p.x > maxX) maxX = p.x; if (p.y > maxY) maxY = p.y })

  const positions = new Map<string, { x: number; y: number; r: number }>()
  const nodes: TreeLayoutNode[] = d.nodes.filter((n) => posMap.has(n.id)).map((n) => {
    const p = posMap.get(n.id)!
    positions.set(`t-${n.id}`, { x: p.x, y: p.y, r: TREE_R })
    return { id: n.id, value: n.value, x: p.x, y: p.y, isRoot: n.id === d.root, diff: 'unchanged' as DiffStatus }
  })

  const edges: TreeLayoutEdge[] = []
  d.nodes.forEach((n) => {
    const from = posMap.get(n.id)
    if (!from) return
    for (const childId of [n.left, n.right]) {
      if (childId === -1) continue
      const to = posMap.get(childId)
      if (to) edges.push({ x1: from.x, y1: from.y, x2: to.x, y2: to.y })
    }
  })

  const heapArray: HeapArrayItem[] = []
  let heapArrayY = 0
  if (isHeap.value) {
    heapArrayY = maxY + TREE_R + 40
    const bfsOrder: Array<{ id: number; value: IrValue }> = []
    const q2 = [d.root]
    const v2 = new Set<number>()
    while (q2.length > 0) {
      const id = q2.shift()!
      if (v2.has(id)) continue
      v2.add(id)
      const n = nodeMap.get(id)
      if (!n) continue
      bfsOrder.push({ id: n.id, value: n.value })
      if (n.left !== -1) q2.push(n.left)
      if (n.right !== -1) q2.push(n.right)
    }
    const arrW = 36
    const totalArrW = bfsOrder.length * arrW
    const arrStartX = Math.max(PAD, (baseW - totalArrW) / 2)
    bfsOrder.forEach((item, i) => {
      heapArray.push({ id: item.id, value: item.value, index: i, x: arrStartX + i * arrW })
    })
    if (heapArrayY + 40 > maxY + TREE_R + PAD) maxY = heapArrayY + 40 - TREE_R
  }

  computeDiff(positions)
  nodes.forEach((nd) => { if (addedIds.value.has(`t-${nd.id}`)) nd.diff = 'added' })

  const svgW = maxX + PAD
  const svgH = maxY + PAD + TREE_R + (isHeap.value ? 60 : 0)
  treeLayout.value = { svgW, svgH, nodes, edges, heapArray, heapArrayY }
  fitViewBox(0, 0, svgW, svgH)
}, { immediate: true })

/* ================================================================
 *  Graph 布局
 * ================================================================ */

interface GLayoutNode { id: number; label: string; x: number; y: number; color: string; diff: DiffStatus }
interface GLayoutEdge { from: number; to: number; x1: number; y1: number; x2: number; y2: number; weight?: number; mx: number; my: number }
interface GLayout { svgW: number; svgH: number; nodes: GLayoutNode[]; edges: GLayoutEdge[] }

const graphLayout = shallowRef<GLayout | null>(null)

watch(() => graphData.value, (d) => {
  if (!d || d.nodes.length === 0) { graphLayout.value = null; return }
  logDebug('[viz] graph layout, nodes =', d.nodes.length, 'edges =', d.edges.length)

  const svgW = 500
  const svgH = 380
  const cx = svgW / 2
  const cy = svgH / 2
  const cR = Math.min(140, 34 * Math.max(3, d.nodes.length))

  const pMap = new Map<number, { x: number; y: number }>()
  const positions = new Map<string, { x: number; y: number; r: number }>()

  const nodes: GLayoutNode[] = d.nodes.map((n: GraphNode, i: number) => {
    const angle = (2 * Math.PI * i) / Math.max(1, d.nodes.length) - Math.PI / 2
    const x = cx + cR * Math.cos(angle)
    const y = cy + cR * Math.sin(angle)
    pMap.set(n.id, { x, y })
    positions.set(`g-${n.id}`, { x, y, r: GRAPH_R })
    return { id: n.id, label: n.label, x, y, color: NODE_PALETTE[Math.abs(n.id) % NODE_PALETTE.length], diff: 'unchanged' as DiffStatus }
  })

  const edges: GLayoutEdge[] = []
  d.edges.forEach((e: GraphEdge & { weight?: number }) => {
    const from = pMap.get(e.from)
    const to = pMap.get(e.to)
    if (!from || !to) return
    const dx = to.x - from.x
    const dy = to.y - from.y
    const dist = Math.sqrt(dx * dx + dy * dy)
    if (dist === 0) return
    const ux = dx / dist, uy = dy / dist
    edges.push({
      from: e.from, to: e.to,
      x1: from.x + ux * GRAPH_R, y1: from.y + uy * GRAPH_R,
      x2: to.x - ux * GRAPH_R, y2: to.y - uy * GRAPH_R,
      weight: e.weight, mx: (from.x + to.x) / 2, my: (from.y + to.y) / 2,
    })
  })

  computeDiff(positions)
  nodes.forEach((n) => { if (addedIds.value.has(`g-${n.id}`)) n.diff = 'added' })

  graphLayout.value = { svgW, svgH, nodes, edges }
  fitViewBox(0, 0, svgW, svgH)
}, { immediate: true })

/* ================================================================
 *  生命周期
 * ================================================================ */

onBeforeUnmount(() => {
  if (ghostTimer !== null) window.clearTimeout(ghostTimer)
})
</script>

<template>
  <section class="viz-panel" :class="{ 'viz-panel--smooth': vizFlags.enableSmoothTransitions }">
    <header class="viz-panel__header">
      <div class="viz-panel__title">
        <MaterialIcon name="graph_3" :size="18" />
        <span>{{ kindLabel || '可视化' }}</span>
      </div>
      <div class="viz-panel__header-right">
        <div v-if="step" class="viz-panel__step">
          <span class="viz-panel__step-op">{{ step.op }}</span>
          <span v-if="step.line" class="viz-panel__step-line">L{{ step.line }}</span>
        </div>
      </div>
    </header>

    <!-- 元数据条 -->
    <div v-if="vizFlags.showMetadata && remarks" class="viz-panel__meta">
      <span v-if="remarks.title" class="viz-panel__meta-item">{{ remarks.title }}</span>
      <span v-if="remarks.author" class="viz-panel__meta-item">{{ remarks.author }}</span>
      <span v-if="remarks.comment" class="viz-panel__meta-item viz-panel__meta-comment">{{ remarks.comment }}</span>
    </div>

    <div ref="canvasRef" class="viz-panel__body">
      <VizPlaceholder v-if="isEmpty" />

      <template v-else>
        <svg
          class="viz-panel__svg"
          :viewBox="viewBoxStr"
          preserveAspectRatio="xMidYMid meet"
          @wheel.prevent="onWheel"
          @pointerdown="onPanStart"
          @pointermove="onPanMove"
          @pointerup="onPanEnd"
          @pointerleave="onPanEnd"
        >
          <defs>
            <marker id="viz-arrow" markerWidth="8" markerHeight="8" refX="7" refY="4" orient="auto">
              <path d="M 0 0 L 8 4 L 0 8 Z" fill="var(--color-border-strong)" />
            </marker>
            <marker id="viz-arrow-accent" markerWidth="8" markerHeight="8" refX="7" refY="4" orient="auto">
              <path d="M 0 0 L 8 4 L 0 8 Z" fill="var(--color-accent)" />
            </marker>
          </defs>

          <!-- ═══════ Stack (Cylinder) ═══════ -->
          <g v-if="stackLayout">
            <!-- 圆柱体 -->
            <g v-if="stackLayout.items.length > 0">
              <template v-for="(it, i) in stackLayout.items" :key="`si-${i}`">
                <ellipse
                  :cx="stackLayout.cx" :cy="it.y + CYL_ITEM_H / 2" :rx="CYL_RX" :ry="CYL_RY"
                  class="viz-cyl-sep" :class="{ 'viz-node--added': it.diff === 'added' }"
                />
                <rect
                  :x="stackLayout.cx - CYL_RX" :y="it.y - CYL_ITEM_H / 2" :width="CYL_RX * 2" :height="CYL_ITEM_H"
                  class="viz-cyl-body" :class="{ 'viz-cyl-body--top': it.isTop, 'viz-node--added': it.diff === 'added' }"
                />
                <line :x1="stackLayout.cx - CYL_RX" :y1="it.y - CYL_ITEM_H / 2" :x2="stackLayout.cx - CYL_RX" :y2="it.y + CYL_ITEM_H / 2" class="viz-cyl-side" />
                <line :x1="stackLayout.cx + CYL_RX" :y1="it.y - CYL_ITEM_H / 2" :x2="stackLayout.cx + CYL_RX" :y2="it.y + CYL_ITEM_H / 2" class="viz-cyl-side" />
                <ellipse
                  :cx="stackLayout.cx" :cy="it.y - CYL_ITEM_H / 2" :rx="CYL_RX" :ry="CYL_RY"
                  class="viz-cyl-top" :class="{ 'viz-cyl-top--accent': it.isTop }"
                />
                <text :x="stackLayout.cx" :y="it.y + 4" class="viz-val"
                  @pointerenter="showTip(`[${i}] = ${fullVal(it.value)}${it.isTop ? ' (TOP)' : ''}`, $event)"
                  @pointerleave="hideTip">{{ fmtVal(it.value) }}</text>
              </template>
              <!-- TOP 标记 -->
              <text v-if="stackLayout.items.length > 0" :x="stackLayout.cx + CYL_RX + 8" :y="stackLayout.items[stackLayout.items.length - 1].y + 4" class="viz-badge">TOP</text>
            </g>
            <text v-else :x="stackLayout.cx" :y="stackLayout.svgH / 2" class="viz-empty-label">空栈</text>

            <!-- Diff ghosts -->
            <rect v-for="g in removedKeys" :key="`gs-${g.key}`"
              :x="(g.x ?? stackLayout.cx) - CYL_RX" :y="(g.y ?? 0) - CYL_ITEM_H / 2" :width="CYL_RX * 2" :height="CYL_ITEM_H"
              class="viz-ghost" />
          </g>

          <!-- ═══════ Queue ═══════ -->
          <g v-if="queueLayout">
            <template v-for="(it, i) in queueLayout.items" :key="`qi-${i}`">
              <rect
                :x="it.x" :y="PAD + 20" :width="BOX_W" :height="BOX_H" rx="6"
                class="viz-box" :class="{ 'viz-box--accent': it.isFront || it.isRear, 'viz-node--added': it.diff === 'added' }"
              />
              <text :x="it.x + BOX_W / 2" :y="PAD + 20 + BOX_H / 2 + 4" class="viz-val"
                @pointerenter="showTip(`[${i}] = ${fullVal(it.value)}${it.isFront ? ' (FRONT)' : ''}${it.isRear ? ' (REAR)' : ''}`, $event)"
                @pointerleave="hideTip">{{ fmtVal(it.value) }}</text>
              <!-- 箭头 -->
              <line v-if="i < queueLayout.items.length - 1"
                :x1="it.x + BOX_W + 4" :y1="PAD + 20 + BOX_H / 2"
                :x2="queueLayout.items[i + 1].x - 4" :y2="PAD + 20 + BOX_H / 2"
                class="viz-arrow" marker-end="url(#viz-arrow)" />
              <!-- 标签 -->
              <text v-if="it.isFront" :x="it.x + BOX_W / 2" :y="PAD + 12" class="viz-badge">FRONT</text>
              <text v-if="it.isRear" :x="it.x + BOX_W / 2" :y="PAD + 12" class="viz-badge">REAR</text>
            </template>
            <text v-if="queueLayout.items.length === 0" :x="PAD + 60" :y="PAD + 40" class="viz-empty-label">空队列</text>
            <rect v-for="g in removedKeys" :key="`gq-${g.key}`" :x="g.x" :y="g.y" :width="g.w ?? BOX_W" :height="g.h ?? BOX_H" rx="6" class="viz-ghost" />
          </g>

          <!-- ═══════ SList ═══════ -->
          <g v-if="slistLayout">
            <template v-for="(nd, i) in slistLayout.nodes" :key="`sn-${nd.id}`">
              <rect
                :x="nd.x" :y="PAD + 20" :width="NODE_W" :height="NODE_H" rx="8"
                class="viz-node-rect" :class="{ 'viz-node-rect--accent': nd.isHead, 'viz-node--added': nd.diff === 'added' }"
              />
              <text :x="nd.x + NODE_W / 2" :y="PAD + 20 + NODE_H / 2 - 4" class="viz-val"
                @pointerenter="showTip(`#${nd.id} = ${fullVal(nd.value)}${nd.isHead ? ' (HEAD)' : ''}`, $event)"
                @pointerleave="hideTip">{{ fmtVal(nd.value) }}</text>
              <text :x="nd.x + NODE_W / 2" :y="PAD + 20 + NODE_H / 2 + 12" class="viz-ptr">#{{ nd.id }}</text>
              <text v-if="nd.isHead" :x="nd.x + NODE_W / 2" :y="PAD + 12" class="viz-badge">HEAD</text>
              <!-- 箭头 -->
              <line v-if="i < slistLayout.nodes.length - 1"
                :x1="nd.x + NODE_W + 2" :y1="PAD + 20 + NODE_H / 2"
                :x2="slistLayout.nodes[i + 1].x - 2" :y2="PAD + 20 + NODE_H / 2"
                class="viz-arrow" marker-end="url(#viz-arrow)" />
              <!-- null -->
              <g v-if="nd.isTail">
                <line :x1="nd.x + NODE_W + 2" :y1="PAD + 20 + NODE_H / 2" :x2="nd.x + NODE_W + 28" :y2="PAD + 20 + NODE_H / 2" class="viz-arrow" />
                <text :x="nd.x + NODE_W + 32" :y="PAD + 20 + NODE_H / 2 + 4" class="viz-null">⊘</text>
              </g>
            </template>
            <rect v-for="g in removedKeys" :key="`gsl-${g.key}`" :x="g.x" :y="g.y" :width="g.w ?? NODE_W" :height="g.h ?? NODE_H" rx="8" class="viz-ghost" />
          </g>

          <!-- ═══════ DList ═══════ -->
          <g v-if="dlistLayout">
            <template v-for="(nd, i) in dlistLayout.nodes" :key="`dn-${nd.id}`">
              <rect
                :x="nd.x" :y="PAD + 20" :width="NODE_W" :height="NODE_H" rx="8"
                class="viz-node-rect" :class="{ 'viz-node-rect--accent': nd.isHead || nd.isTail, 'viz-node--added': nd.diff === 'added' }"
              />
              <text :x="nd.x + NODE_W / 2" :y="PAD + 20 + NODE_H / 2 - 4" class="viz-val"
                @pointerenter="showTip(`#${nd.id} = ${fullVal(nd.value)}${nd.isHead ? ' (HEAD)' : ''}${nd.isTail ? ' (TAIL)' : ''}`, $event)"
                @pointerleave="hideTip">{{ fmtVal(nd.value) }}</text>
              <text :x="nd.x + NODE_W / 2" :y="PAD + 20 + NODE_H / 2 + 12" class="viz-ptr">#{{ nd.id }}</text>
              <text v-if="nd.isHead" :x="nd.x + NODE_W / 2" :y="PAD + 12" class="viz-badge">HEAD</text>
              <text v-if="nd.isTail" :x="nd.x + NODE_W / 2" :y="PAD + 12" class="viz-badge viz-badge--tail">TAIL</text>
              <!-- 双向箭头 -->
              <g v-if="i < dlistLayout.nodes.length - 1">
                <line :x1="nd.x + NODE_W + 2" :y1="PAD + 20 + NODE_H / 2 - 4"
                  :x2="dlistLayout.nodes[i + 1].x - 2" :y2="PAD + 20 + NODE_H / 2 - 4"
                  class="viz-arrow" marker-end="url(#viz-arrow)" />
                <line :x1="dlistLayout.nodes[i + 1].x - 2" :y1="PAD + 20 + NODE_H / 2 + 4"
                  :x2="nd.x + NODE_W + 2" :y2="PAD + 20 + NODE_H / 2 + 4"
                  class="viz-arrow" marker-end="url(#viz-arrow)" />
              </g>
            </template>
            <rect v-for="g in removedKeys" :key="`gdl-${g.key}`" :x="g.x" :y="g.y" :width="g.w ?? NODE_W" :height="g.h ?? NODE_H" rx="8" class="viz-ghost" />
          </g>

          <!-- ═══════ Tree / BST / Heap ═══════ -->
          <g v-if="treeLayout">
            <line v-for="(e, i) in treeLayout.edges" :key="`te-${i}`"
              :x1="e.x1" :y1="e.y1" :x2="e.x2" :y2="e.y2"
              :class="isHeap ? 'viz-edge--dashed' : 'viz-edge'" />
            <g v-for="nd in treeLayout.nodes" :key="`tn-${nd.id}`">
              <circle v-if="!isHeap"
                :cx="nd.x" :cy="nd.y" :r="TREE_R"
                class="viz-tree-node" :class="{ 'viz-tree-node--root': nd.isRoot, 'viz-node--added': nd.diff === 'added' }" />
              <rect v-else
                :x="nd.x - TREE_R" :y="nd.y - TREE_R" :width="TREE_R * 2" :height="TREE_R * 2" rx="6"
                class="viz-tree-node" :class="{ 'viz-tree-node--root': nd.isRoot, 'viz-node--added': nd.diff === 'added' }" />
              <text :x="nd.x" :y="nd.y + 5" class="viz-val"
                @pointerenter="showTip(`#${nd.id} = ${fullVal(nd.value)}${nd.isRoot ? ' (ROOT)' : ''}`, $event)"
                @pointerleave="hideTip">{{ fmtVal(nd.value) }}</text>
            </g>
            <!-- Heap 数组条 -->
            <g v-if="isHeap && treeLayout.heapArray.length > 0">
              <line :x1="treeLayout.heapArray[0].x - 4" :y1="treeLayout.heapArrayY - 4"
                :x2="treeLayout.heapArray[treeLayout.heapArray.length - 1].x + 36 + 4" :y2="treeLayout.heapArrayY - 4"
                class="viz-edge" />
              <g v-for="h in treeLayout.heapArray" :key="`ha-${h.index}`">
                <rect :x="h.x" :y="treeLayout.heapArrayY" :width="34" :height="28" rx="4" class="viz-box" />
                <text :x="h.x + 17" :y="treeLayout.heapArrayY + 18" class="viz-val viz-val--sm">{{ fmtVal(h.value) }}</text>
                <text :x="h.x + 17" :y="treeLayout.heapArrayY + 42" class="viz-ptr">{{ h.index }}</text>
              </g>
            </g>
            <circle v-for="g in removedKeys" :key="`gt-${g.key}`" :cx="g.x" :cy="g.y" :r="g.r ?? TREE_R" class="viz-ghost viz-ghost--circle" />
          </g>

          <!-- ═══════ Graph ═══════ -->
          <g v-if="graphLayout">
            <line v-for="(e, i) in graphLayout.edges" :key="`ge-${i}`"
              :x1="e.x1" :y1="e.y1" :x2="e.x2" :y2="e.y2"
              class="viz-edge" :marker-end="isDirected ? 'url(#viz-arrow-accent)' : undefined" />
            <text v-for="(e, i) in graphLayout.edges" :key="`gw-${i}`"
              v-show="isWeighted && e.weight !== undefined"
              :x="e.mx" :y="e.my - 6" class="viz-weight">{{ e.weight }}</text>
            <g v-for="nd in graphLayout.nodes" :key="`gn-${nd.id}`">
              <circle :cx="nd.x" :cy="nd.y" :r="GRAPH_R" class="viz-graph-node"
                :class="{ 'viz-node--added': nd.diff === 'added' }"
                :style="{ fill: nd.color + '18', stroke: nd.color }" />
              <text :x="nd.x" :y="nd.y + 5" class="viz-val"
                @pointerenter="showTip('#' + nd.id + ' ' + nd.label, $event)" @pointerleave="hideTip">{{ nd.label.length
                  > 4 ? nd.label.slice(0, 3) + '…' : nd.label }}</text>
            </g>
            <circle v-for="g in removedKeys" :key="`gg-${g.key}`" :cx="g.x" :cy="g.y" :r="g.r ?? GRAPH_R" class="viz-ghost viz-ghost--circle" />
          </g>
        </svg>

        <!-- Tooltip -->
        <Transition name="fade">
          <div v-if="tip.show" class="viz-tooltip" :style="tipStyle">{{ tip.text }}</div>
        </Transition>
      </template>

      <!-- 设置齿轮 -->
      <VizSettings />
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
  padding: var(--space-1) var(--space-2);
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

.viz-panel__title :deep(.material-icon) { width: 18px; height: 18px; }

.viz-panel__header-right { display: flex; align-items: center; gap: 8px; }

.viz-panel__step { display: flex; align-items: center; gap: 6px; font-size: var(--text-xs); color: var(--color-text-tertiary); }
.viz-panel__step-op { padding: 2px 6px; border-radius: var(--radius-sm); background-color: var(--color-accent-wash); color: var(--color-accent); font-weight: var(--weight-medium); }
.viz-panel__step-line { padding: 2px 6px; border-radius: var(--radius-sm); background-color: var(--color-bg-hover); }

.viz-panel__meta {
  display: flex;
  flex-wrap: wrap;
  gap: 4px 12px;
  padding: 6px var(--space-2);
  border-bottom: 1px solid var(--color-border);
  background-color: var(--color-bg-surface-alt);
  font-size: 11px;
  color: var(--color-text-tertiary);
}

.viz-panel__meta-item { white-space: nowrap; }
.viz-panel__meta-comment { font-style: italic; }

.viz-panel__body {
  flex: 1;
  min-height: 0;
  position: relative;
  overflow: hidden;
}

.viz-panel__svg {
  width: 100%;
  height: 100%;
  cursor: grab;
  background-color: var(--color-bg-surface-alt);
  user-select: none;
  touch-action: none;
}

.viz-panel__svg:active { cursor: grabbing; }

/* ---- Tooltip ---- */

.viz-tooltip {
  position: absolute;
  padding: 5px 10px;
  border-radius: var(--radius-control);
  background-color: var(--color-text-primary);
  color: var(--color-bg-surface);
  font-size: 11px;
  font-family: var(--font-mono);
  white-space: nowrap;
  pointer-events: none;
  z-index: 10;
  box-shadow: 0 4px 12px rgba(0, 0, 0, 0.18);
}

/* ---- 公共 SVG 样式 ---- */

.viz-val {
  font-size: 13px;
  font-family: var(--font-mono);
  fill: var(--color-text-primary);
  text-anchor: middle;
  pointer-events: all;
  cursor: default;
}

.viz-val--sm { font-size: 11px; }

.viz-ptr {
  font-size: 10px;
  font-family: var(--font-mono);
  fill: var(--color-text-tertiary);
  text-anchor: middle;
}

.viz-badge {
  font-size: 10px;
  font-weight: 600;
  fill: var(--color-accent);
  text-anchor: middle;
  letter-spacing: 0.04em;
}

.viz-badge--tail { fill: var(--color-warning); }

.viz-null {
  font-size: 16px;
  fill: var(--color-text-tertiary);
  dominant-baseline: central;
}

.viz-empty-label {
  font-size: 13px;
  fill: var(--color-text-tertiary);
  text-anchor: middle;
}

.viz-arrow {
  stroke: var(--color-border-strong);
  stroke-width: 1.2;
}

.viz-edge {
  stroke: rgba(0, 0, 0, 0.12);
  stroke-width: 1.2;
}

.viz-edge--dashed {
  stroke: rgba(0, 0, 0, 0.12);
  stroke-width: 1.2;
  stroke-dasharray: 5 3;
}

.viz-weight {
  font-size: 11px;
  font-weight: 600;
  fill: var(--color-accent);
  text-anchor: middle;
}

/* ---- Stack Cylinder ---- */

.viz-cyl-body {
  fill: var(--color-bg-surface);
  stroke: none;
}

.viz-cyl-body--top {
  fill: var(--color-accent-wash);
}

.viz-cyl-side {
  stroke: var(--color-border-strong);
  stroke-width: 1;
}

.viz-cyl-sep {
  fill: var(--color-bg-surface-alt);
  stroke: var(--color-border);
  stroke-width: 0.5;
}

.viz-cyl-top {
  fill: var(--color-bg-surface);
  stroke: var(--color-border-strong);
  stroke-width: 1;
}

.viz-cyl-top--accent {
  fill: var(--color-accent-wash);
  stroke: var(--color-accent);
}

/* ---- Queue / Box ---- */

.viz-box {
  fill: var(--color-bg-surface);
  stroke: var(--color-border-strong);
  stroke-width: 1;
}

.viz-box--accent {
  fill: var(--color-accent-wash);
  stroke: var(--color-accent);
}

/* ---- List Node Rect ---- */

.viz-node-rect {
  fill: var(--color-bg-surface);
  stroke: var(--color-border-strong);
  stroke-width: 1;
}

.viz-node-rect--accent {
  fill: var(--color-accent-wash);
  stroke: var(--color-accent);
}

/* ---- Tree Node ---- */

.viz-tree-node {
  fill: var(--color-bg-surface);
  stroke: var(--color-border-strong);
  stroke-width: 1;
}

.viz-tree-node--root {
  fill: var(--color-accent-wash);
  stroke: var(--color-accent);
}

/* ---- Graph Node ---- */

.viz-graph-node {
  stroke-width: 1.5;
}

/* ---- Diff: Added ---- */

.viz-node--added {
  animation: viz-pop 220ms var(--ease);
}

@keyframes viz-pop {
  from { opacity: 0; }
  to { opacity: 1; }
}

/* ---- Diff: Ghost (Removed) ---- */

.viz-ghost {
  fill: none;
  stroke: var(--color-error);
  stroke-width: 1;
  stroke-dasharray: 4 2;
  opacity: 0.35;
  animation: viz-fade-out 350ms var(--ease) forwards;
}

.viz-ghost--circle {
  fill: none;
}

@keyframes viz-fade-out {
  to { opacity: 0; }
}

/* ---- Smooth Transitions ---- */

.viz-panel--smooth .viz-tree-node,
.viz-panel--smooth .viz-graph-node {
  transition:
    cx var(--duration-viz) var(--ease),
    cy var(--duration-viz) var(--ease),
    x var(--duration-viz) var(--ease),
    y var(--duration-viz) var(--ease),
    fill var(--duration-viz) var(--ease),
    stroke var(--duration-viz) var(--ease);
}

.viz-panel--smooth .viz-edge,
.viz-panel--smooth .viz-edge--dashed,
.viz-panel--smooth .viz-arrow {
  transition:
    x1 var(--duration-viz) var(--ease),
    y1 var(--duration-viz) var(--ease),
    x2 var(--duration-viz) var(--ease),
    y2 var(--duration-viz) var(--ease);
}

.viz-panel--smooth .viz-val,
.viz-panel--smooth .viz-ptr,
.viz-panel--smooth .viz-badge {
  transition:
    x var(--duration-viz) var(--ease),
    y var(--duration-viz) var(--ease);
}

.viz-panel--smooth .viz-cyl-body,
.viz-panel--smooth .viz-cyl-top,
.viz-panel--smooth .viz-box,
.viz-panel--smooth .viz-node-rect {
  transition:
    fill var(--duration-viz) var(--ease),
    stroke var(--duration-viz) var(--ease);
}

@media (max-width: 1100px) {
  .viz-panel__svg { min-height: 240px; }
}

@media (prefers-reduced-motion: reduce) {
  .viz-node--added { animation: none; }
  .viz-ghost { animation: none; opacity: 0.2; }
}
</style>
