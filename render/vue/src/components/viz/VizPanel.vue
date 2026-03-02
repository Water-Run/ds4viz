<script setup lang="ts">
/**
 * 可视化面板
 *
 * 支持 Stack / Queue / SList / DList / BinaryTree / BST / Heap / Graph 渲染，
 * 提供缩放/平移/双击回中、结构化 Tooltip、Diff 高亮（对称渐进渐退）、
 * 自适应节点大小、文本换行、标签碰撞规避、图形化空状态、元数据修复。
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

interface Props {
  kind?: IrObjectKind
  data?: IrStateData
  step?: StepSummary | null
  label?: string
  remarks?: IrRemarks
  autoPlaying?: boolean
}

const props = defineProps<Props>()

/* ================================================================
 *  常量（基准尺寸）
 * ================================================================ */

const BASE_CYL_RX = 64
const BASE_CYL_ITEM_H = 40
const CYL_RY = 12

const BASE_BOX_W = 72
const BASE_BOX_H = 44
const BOX_GAP = 36

const BASE_NODE_W = 96
const BASE_NODE_H = 44
const NODE_GAP = 44

const BASE_TREE_R = 20
const TREE_LEVEL_H = 72

const BASE_GRAPH_R = 22

const PAD = 40
const LINE_H = 14

const BASE_CHARS_CYL = 12
const BASE_CHARS_BOX = 7
const BASE_CHARS_NODE = 9
const BASE_CHARS_CIRCLE = 4

const NODE_PALETTE = [
  '#0078d4', '#0e7c6b', '#7c3aed', '#c2410c',
  '#0369a1', '#6d28d9', '#b45309', '#059669',
  '#dc2626', '#4f46e5', '#0891b2', '#65a30d',
]

type DiffStatus = 'added' | 'removed' | 'unchanged'

const ACCENT_COLOR = 'var(--color-accent)'
const WARNING_COLOR = 'var(--color-warning)'

/* ================================================================
 *  Ghost / Diff 缓存类型
 * ================================================================ */

/**
 * 缓存的位置与形态信息
 */
interface CachedPosition {
  /** 形状类型 */
  shape: 'rect' | 'circle' | 'ellipse'
  /** x 坐标（circle/ellipse 为中心，rect 为左上角） */
  x: number
  /** y 坐标（circle/ellipse 为中心，rect 为左上角） */
  y: number
  /** 圆半径 */
  r?: number
  /** 椭圆水平半径 */
  rx?: number
  /** 椭圆垂直半径 */
  ry?: number
  /** 矩形宽度 */
  w?: number
  /** 矩形高度 */
  h?: number
}

/**
 * Ghost 元素（带标识键）
 */
type GhostItem = CachedPosition & { key: string }

/* ================================================================
 *  文本测量与自适应
 * ================================================================ */

function getEffectiveLength(text: string): number {
  let len = 0
  for (let i = 0; i < text.length; i += 1) {
    len += text.charCodeAt(i) > 0x7F ? 2 : 1
  }
  return len
}

function computeNodeScale(values: IrValue[], baseChars: number): number {
  if (values.length === 0) return 1
  const maxLen = Math.max(...values.map((v) => getEffectiveLength(String(v))))
  if (maxLen <= baseChars) return 1
  return Math.min(2, maxLen / baseChars)
}

function splitDisplayLines(value: IrValue, maxCharsPerLine: number, maxLines: number): string[] {
  const text = String(value)
  const effLen = getEffectiveLength(text)
  if (effLen <= maxCharsPerLine) return [text]

  const lines: string[] = []
  let pos = 0
  for (let line = 0; line < maxLines && pos < text.length; line += 1) {
    let lineChars = 0
    let lineEnd = pos
    while (lineEnd < text.length) {
      const cw = text.charCodeAt(lineEnd) > 0x7F ? 2 : 1
      if (lineChars + cw > maxCharsPerLine) break
      lineChars += cw
      lineEnd += 1
    }
    if (line === maxLines - 1 && lineEnd < text.length) {
      const slice = text.slice(pos, Math.max(pos + 1, lineEnd - 1))
      lines.push(slice + '…')
    } else {
      lines.push(text.slice(pos, lineEnd))
    }
    pos = lineEnd
  }
  return lines.length > 0 ? lines : ['…']
}

/* ================================================================
 *  标签碰撞
 * ================================================================ */

interface LayoutLabel {
  text: string
  position: 'top' | 'bottom'
  color: string
}

function buildLabels(roles: Array<{ text: string; color: string }>): LayoutLabel[] {
  return roles.map((r, i) => ({ text: r.text, position: i === 0 ? 'top' as const : 'bottom' as const, color: r.color }))
}

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

/* ---- 结构空状态 ---- */

const isStructureEmpty = computed<boolean>(() => {
  if (!props.data) return false
  const k = kindStr.value
  if (k === 'stack') return (props.data as StackStateData).items.length === 0
  if (k === 'queue') return (props.data as QueueStateData).items.length === 0
  if (k === 'slist') return (props.data as SListStateData).nodes.length === 0
  if (k === 'dlist') return (props.data as DListStateData).nodes.length === 0
  if (k === 'binary_tree' || k === 'bst' || k === 'heap') {
    const d = props.data as BinaryTreeStateData
    return d.root === -1 || d.nodes.length === 0
  }
  if (k === 'graph_undirected' || k === 'graph_directed' || k === 'graph_weighted') {
    return (props.data as GraphStateData).nodes.length === 0
  }
  return false
})

const emptyLabel = computed<string>(() => {
  const m: Record<string, string> = {
    stack: '空栈', queue: '空队列', slist: '空链表', dlist: '空链表',
    binary_tree: '空树', bst: '空树', heap: '空堆',
    graph_undirected: '空图', graph_directed: '空图', graph_weighted: '空图',
  }
  return m[kindStr.value] ?? '空'
})

const emptyType = computed<string>(() => {
  const k = kindStr.value
  if (k === 'stack') return 'stack'
  if (k === 'queue') return 'queue'
  if (k === 'slist' || k === 'dlist') return 'list'
  if (k === 'binary_tree' || k === 'bst' || k === 'heap') return 'tree'
  return 'graph'
})

/* ---- 元数据显示 ---- */

const hasMetadata = computed<boolean>(() => {
  if (!vizFlags.showMetadata) return false
  if (props.label && props.label.length > 0) return true
  if (props.remarks?.title) return true
  if (props.remarks?.author) return true
  if (props.remarks?.comment) return true
  return false
})

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
 *  ViewBox / Zoom / Pan / 双击回中 / 缩放提示
 * ================================================================ */

interface VB { x: number; y: number; w: number; h: number }

const vb = ref<VB>({ x: 0, y: 0, w: 500, h: 400 })
const idealVb = ref<VB>({ x: 0, y: 0, w: 500, h: 400 })
let animFrame: number | null = null

/** 用户是否手动缩放/平移过 */
const userManualZoom = ref<boolean>(false)

const effectiveViewBox = computed<string>(() => {
  if (isStructureEmpty.value) return '0 0 300 200'
  return `${vb.value.x} ${vb.value.y} ${vb.value.w} ${vb.value.h}`
})

const showZoomHint = computed<boolean>(() => {
  return userManualZoom.value && !isEmpty.value && !isStructureEmpty.value
})

const isPanning = ref<boolean>(false)
const panOrigin = ref<{ cx: number; cy: number; vx: number; vy: number }>({ cx: 0, cy: 0, vx: 0, vy: 0 })

const animateToVb = (target: VB, duration: number = 200): void => {
  if (animFrame !== null) window.cancelAnimationFrame(animFrame)
  const from = { ...vb.value }
  const start = performance.now()
  const animate = (now: number): void => {
    const t = Math.min(1, (now - start) / duration)
    const ease = t < 0.5 ? 2 * t * t : 1 - Math.pow(-2 * t + 2, 2) / 2
    vb.value = {
      x: from.x + (target.x - from.x) * ease,
      y: from.y + (target.y - from.y) * ease,
      w: from.w + (target.w - from.w) * ease,
      h: from.h + (target.h - from.h) * ease,
    }
    if (t < 1) { animFrame = window.requestAnimationFrame(animate) } else { animFrame = null }
  }
  animFrame = window.requestAnimationFrame(animate)
}

const onWheel = (e: WheelEvent): void => {
  const svg = e.currentTarget as SVGSVGElement
  const rect = svg.getBoundingClientRect()
  const mx = (e.clientX - rect.left) / rect.width
  const my = (e.clientY - rect.top) / rect.height
  const factor = e.deltaY > 0 ? 1.1 : 1 / 1.1
  const nw = vb.value.w * factor
  const nh = vb.value.h * factor
  vb.value = { x: vb.value.x + (vb.value.w - nw) * mx, y: vb.value.y + (vb.value.h - nh) * my, w: nw, h: nh }
  userManualZoom.value = true
}

const onPanStart = (e: PointerEvent): void => {
  isPanning.value = true
  panOrigin.value = { cx: e.clientX, cy: e.clientY, vx: vb.value.x, vy: vb.value.y }
    ; (e.currentTarget as SVGSVGElement).setPointerCapture(e.pointerId)
}

const onPanMove = (e: PointerEvent): void => {
  if (!isPanning.value) return
  const svg = e.currentTarget as SVGSVGElement
  const rect = svg.getBoundingClientRect()
  const dx = (e.clientX - panOrigin.value.cx) / rect.width * vb.value.w
  const dy = (e.clientY - panOrigin.value.cy) / rect.height * vb.value.h
  vb.value = { ...vb.value, x: panOrigin.value.vx - dx, y: panOrigin.value.vy - dy }
}

const onPanEnd = (): void => {
  if (isPanning.value) {
    const dx = Math.abs(vb.value.x - panOrigin.value.vx)
    const dy = Math.abs(vb.value.y - panOrigin.value.vy)
    if (dx > 1 || dy > 1) {
      userManualZoom.value = true
    }
  }
  isPanning.value = false
}

const fitViewBox = (minX: number, minY: number, maxX: number, maxY: number): void => {
  const w = Math.max(200, maxX - minX + PAD * 2)
  const h = Math.max(150, maxY - minY + PAD * 2)
  const target: VB = { x: minX - PAD, y: minY - PAD, w, h }
  idealVb.value = { ...target }
  if (vizFlags.enableAutoFit) {
    animateToVb(target, 180)
    userManualZoom.value = false
  }
}

const handleDblClick = (): void => {
  animateToVb({ ...idealVb.value }, 200)
  userManualZoom.value = false
}

/* ================================================================
 *  Tooltip（结构化白底）
 * ================================================================ */

interface TipLine { label: string; value: string }
const tip = ref<{ show: boolean; x: number; y: number; lines: TipLine[] }>({ show: false, x: 0, y: 0, lines: [] })
const tipStyle = computed(() => ({ left: `${tip.value.x}px`, top: `${tip.value.y}px` }))

const showTip = (lines: TipLine[], e: PointerEvent): void => {
  if (!canvasRef.value) return
  const r = canvasRef.value.getBoundingClientRect()
  tip.value = { show: true, lines, x: e.clientX - r.left + 12, y: e.clientY - r.top - 8 }
}
const hideTip = (): void => { tip.value = { ...tip.value, show: false } }

function fullVal(v: IrValue): string { return typeof v === 'string' ? `"${v}"` : String(v) }

/* ================================================================
 *  Diff 高亮（对称渐进渐退）
 * ================================================================ */

function getGhostDuration(): number {
  if (props.autoPlaying) {
    return Math.max(150, Math.min(2000, vizFlags.playbackInterval * 0.5))
  }
  return 500
}

const ghostDurationMs = computed<string>(() => getGhostDuration() + 'ms')

let prevIds: Set<string> = new Set()
const addedIds = shallowRef<Set<string>>(new Set())
const removedKeys = shallowRef<GhostItem[]>([])
let ghostTimer: number | null = null
const prevPositionCache = new Map<string, CachedPosition>()

function computeDiff(currentKeys: Map<string, CachedPosition>): void {
  if (!vizFlags.enableDiffHighlight) {
    addedIds.value = new Set()
    removedKeys.value = []
    prevIds = new Set(currentKeys.keys())
    prevPositionCache.clear()
    for (const [k, v] of currentKeys) prevPositionCache.set(k, v)
    return
  }
  const curSet = new Set(currentKeys.keys())
  const added = new Set<string>()
  const removed: GhostItem[] = []
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
    ghostTimer = window.setTimeout(() => { removedKeys.value = [] }, getGhostDuration())
  }
}

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

interface StackLayoutItem { value: IrValue; y: number; isTop: boolean; diff: DiffStatus; displayLines: string[]; labels: LayoutLabel[] }
interface StackLayout { svgW: number; svgH: number; cx: number; cylRx: number; cylItemH: number; items: StackLayoutItem[] }
const stackLayout = shallowRef<StackLayout | null>(null)

watch(() => stackData.value, (d) => {
  if (!d) { stackLayout.value = null; return }
  if (d.items.length === 0) { stackLayout.value = null; computeDiff(new Map()); return }
  const scale = computeNodeScale(d.items, BASE_CHARS_CYL)
  const cylRx = BASE_CYL_RX * scale
  const maxChars = Math.floor(BASE_CHARS_CYL * scale)
  const anyTwoLine = d.items.some((v) => getEffectiveLength(String(v)) > maxChars)
  const cylItemH = BASE_CYL_ITEM_H * (anyTwoLine ? 1.35 : 1)
  const n = d.items.length
  const svgW = cylRx * 2 + PAD * 2
  const svgH = n * cylItemH + CYL_RY * 2 + PAD * 2 + 20
  const cx = svgW / 2
  const baseY = svgH - PAD - CYL_RY
  const positions = new Map<string, CachedPosition>()
  const items: StackLayoutItem[] = d.items.map((v, i) => {
    const y = baseY - (i + 0.5) * cylItemH
    positions.set(`s-${i}`, { shape: 'ellipse', x: cx, y, rx: cylRx, ry: CYL_RY })
    const roles: Array<{ text: string; color: string }> = []
    if (i === d.top) roles.push({ text: 'TOP', color: ACCENT_COLOR })
    return { value: v, y, isTop: i === d.top, diff: 'unchanged' as DiffStatus, displayLines: splitDisplayLines(v, maxChars, 2), labels: buildLabels(roles) }
  })
  computeDiff(positions)
  items.forEach((it, i) => { if (addedIds.value.has(`s-${i}`)) it.diff = 'added' })
  stackLayout.value = { svgW, svgH, cx, cylRx, cylItemH, items }
  fitViewBox(0, 0, svgW, svgH)
}, { immediate: true })

/* ================================================================
 *  Queue 布局
 * ================================================================ */

interface QueueLayoutItem { value: IrValue; x: number; diff: DiffStatus; displayLines: string[]; labels: LayoutLabel[] }
interface QueueLayout { svgW: number; svgH: number; boxW: number; boxH: number; items: QueueLayoutItem[] }
const queueLayout = shallowRef<QueueLayout | null>(null)

watch(() => queueData.value, (d) => {
  if (!d) { queueLayout.value = null; return }
  if (d.items.length === 0) { queueLayout.value = null; computeDiff(new Map()); return }
  const scale = computeNodeScale(d.items, BASE_CHARS_BOX)
  const boxW = BASE_BOX_W * scale
  const boxH = BASE_BOX_H
  const maxChars = Math.floor(BASE_CHARS_BOX * scale)
  const n = d.items.length
  const hasBottomLabel = d.items.some((_, i) => {
    const roles: string[] = []
    if (i === d.front) roles.push('FRONT')
    if (i === d.rear) roles.push('REAR')
    return roles.length > 1
  })
  const svgW = n * boxW + (n - 1) * BOX_GAP + PAD * 2
  const svgH = boxH + PAD * 2 + 24 + (hasBottomLabel ? 36 : 0)
  const positions = new Map<string, CachedPosition>()
  const items: QueueLayoutItem[] = d.items.map((v, i) => {
    const x = PAD + i * (boxW + BOX_GAP)
    positions.set(`q-${i}`, { shape: 'rect', x, y: PAD + 20, w: boxW, h: boxH })
    const roles: Array<{ text: string; color: string }> = []
    if (i === d.front) roles.push({ text: 'FRONT', color: ACCENT_COLOR })
    if (i === d.rear) roles.push({ text: 'REAR', color: WARNING_COLOR })
    return { value: v, x, diff: 'unchanged' as DiffStatus, displayLines: splitDisplayLines(v, maxChars, 2), labels: buildLabels(roles) }
  })
  computeDiff(positions)
  items.forEach((it, i) => { if (addedIds.value.has(`q-${i}`)) it.diff = 'added' })
  queueLayout.value = { svgW, svgH, boxW, boxH, items }
  fitViewBox(0, 0, svgW, svgH)
}, { immediate: true })

/* ================================================================
 *  SList 布局
 * ================================================================ */

interface ListLayoutNode { id: number; value: IrValue; x: number; diff: DiffStatus; displayLines: string[]; labels: LayoutLabel[]; isTail: boolean }
interface ListLayout { svgW: number; svgH: number; nodeW: number; nodeH: number; nodes: ListLayoutNode[] }
const slistLayout = shallowRef<ListLayout | null>(null)

watch(() => slistOrdered.value, (ordered) => {
  if (!slistData.value) { slistLayout.value = null; return }
  if (ordered.length === 0) { slistLayout.value = null; computeDiff(new Map()); return }
  const values = ordered.map((n) => n.value)
  const scale = computeNodeScale(values, BASE_CHARS_NODE)
  const nodeW = BASE_NODE_W * scale
  const nodeH = BASE_NODE_H
  const maxChars = Math.floor(BASE_CHARS_NODE * scale)
  const n = ordered.length
  const svgW = n * nodeW + (n - 1) * NODE_GAP + PAD * 2 + 40
  const svgH = nodeH + PAD * 2 + 24 + 36
  const positions = new Map<string, CachedPosition>()
  const nodes: ListLayoutNode[] = ordered.map((nd, i) => {
    const x = PAD + i * (nodeW + NODE_GAP)
    positions.set(`n-${nd.id}`, { shape: 'rect', x, y: PAD + 20, w: nodeW, h: nodeH })
    const roles: Array<{ text: string; color: string }> = []
    if (nd.id === slistData.value!.head) roles.push({ text: 'HEAD', color: ACCENT_COLOR })
    return { id: nd.id, value: nd.value, x, diff: 'unchanged' as DiffStatus, displayLines: splitDisplayLines(nd.value, maxChars, 2), labels: buildLabels(roles), isTail: nd.next === -1 }
  })
  computeDiff(positions)
  nodes.forEach((nd) => { if (addedIds.value.has(`n-${nd.id}`)) nd.diff = 'added' })
  slistLayout.value = { svgW, svgH, nodeW, nodeH, nodes }
  fitViewBox(0, 0, svgW, svgH)
}, { immediate: true })

/* ================================================================
 *  DList 布局
 * ================================================================ */

const dlistLayout = shallowRef<ListLayout | null>(null)

watch(() => dlistOrdered.value, (ordered) => {
  if (!dlistData.value) { dlistLayout.value = null; return }
  if (ordered.length === 0) { dlistLayout.value = null; computeDiff(new Map()); return }
  const values = ordered.map((n) => n.value)
  const scale = computeNodeScale(values, BASE_CHARS_NODE)
  const nodeW = BASE_NODE_W * scale
  const nodeH = BASE_NODE_H
  const maxChars = Math.floor(BASE_CHARS_NODE * scale)
  const n = ordered.length
  const svgW = n * nodeW + (n - 1) * NODE_GAP + PAD * 2 + 40
  const svgH = nodeH + PAD * 2 + 24 + 36
  const positions = new Map<string, CachedPosition>()
  const nodes: ListLayoutNode[] = ordered.map((nd, i) => {
    const x = PAD + i * (nodeW + NODE_GAP)
    positions.set(`n-${nd.id}`, { shape: 'rect', x, y: PAD + 20, w: nodeW, h: nodeH })
    const roles: Array<{ text: string; color: string }> = []
    if (nd.id === dlistData.value!.head) roles.push({ text: 'HEAD', color: ACCENT_COLOR })
    if (nd.id === dlistData.value!.tail) roles.push({ text: 'TAIL', color: WARNING_COLOR })
    return { id: nd.id, value: nd.value, x, diff: 'unchanged' as DiffStatus, displayLines: splitDisplayLines(nd.value, maxChars, 2), labels: buildLabels(roles), isTail: nd.id === dlistData.value!.tail }
  })
  computeDiff(positions)
  nodes.forEach((nd) => { if (addedIds.value.has(`n-${nd.id}`)) nd.diff = 'added' })
  dlistLayout.value = { svgW, svgH, nodeW, nodeH, nodes }
  fitViewBox(0, 0, svgW, svgH)
}, { immediate: true })

/* ================================================================
 *  Tree 布局
 * ================================================================ */

interface TreeLayoutNode { id: number; value: IrValue; x: number; y: number; isRoot: boolean; diff: DiffStatus; displayLines: string[]; left: number; right: number }
interface TreeLayoutEdge { x1: number; y1: number; x2: number; y2: number }
interface HeapArrayItem { id: number; value: IrValue; index: number; x: number; displayLines: string[] }
interface TreeLayout { svgW: number; svgH: number; treeR: number; nodes: TreeLayoutNode[]; edges: TreeLayoutEdge[]; heapArray: HeapArrayItem[]; heapArrayY: number }
const treeLayout = shallowRef<TreeLayout | null>(null)

watch(() => treeData.value, (d) => {
  if (!d || d.root === -1 || d.nodes.length === 0) { treeLayout.value = null; computeDiff(new Map()); return }
  logDebug('[viz] tree layout, nodes =', d.nodes.length)
  const values = d.nodes.map((n) => n.value)
  const scale = computeNodeScale(values, BASE_CHARS_CIRCLE)
  const treeR = Math.round(BASE_TREE_R * scale)
  const maxChars = Math.floor(BASE_CHARS_CIRCLE * scale)
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
  const baseW = Math.max(300, maxInLvl * treeR * 5 + PAD * 2)
  const posMap = new Map<number, { x: number; y: number }>()
  levels.forEach((ids, lvl) => {
    const gap = baseW / (ids.length + 1)
    ids.forEach((id, i) => posMap.set(id, { x: gap * (i + 1), y: PAD + lvl * TREE_LEVEL_H }))
  })
  let maxX = 0; let maxY = 0
  posMap.forEach((p) => { if (p.x > maxX) maxX = p.x; if (p.y > maxY) maxY = p.y })
  const positions = new Map<string, CachedPosition>()
  const nodes: TreeLayoutNode[] = d.nodes.filter((n) => posMap.has(n.id)).map((n) => {
    const p = posMap.get(n.id)!
    if (isHeap.value) {
      positions.set(`t-${n.id}`, { shape: 'rect', x: p.x - treeR, y: p.y - treeR, w: treeR * 2, h: treeR * 2 })
    } else {
      positions.set(`t-${n.id}`, { shape: 'circle', x: p.x, y: p.y, r: treeR })
    }
    return { id: n.id, value: n.value, x: p.x, y: p.y, isRoot: n.id === d.root, diff: 'unchanged' as DiffStatus, displayLines: splitDisplayLines(n.value, maxChars, 1), left: n.left, right: n.right }
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
    heapArrayY = maxY + treeR + 40
    const bfsOrder: Array<{ id: number; value: IrValue }> = []
    const q2 = [d.root]; const v2 = new Set<number>()
    while (q2.length > 0) { const id = q2.shift()!; if (v2.has(id)) continue; v2.add(id); const n = nodeMap.get(id); if (!n) continue; bfsOrder.push({ id: n.id, value: n.value }); if (n.left !== -1) q2.push(n.left); if (n.right !== -1) q2.push(n.right) }
    const arrW = 36; const totalArrW = bfsOrder.length * arrW; const arrStartX = Math.max(PAD, (baseW - totalArrW) / 2)
    bfsOrder.forEach((item, i) => { heapArray.push({ id: item.id, value: item.value, index: i, x: arrStartX + i * arrW, displayLines: splitDisplayLines(item.value, 3, 1) }) })
    if (heapArrayY + 40 > maxY + treeR + PAD) maxY = heapArrayY + 40 - treeR
  }
  computeDiff(positions)
  nodes.forEach((nd) => { if (addedIds.value.has(`t-${nd.id}`)) nd.diff = 'added' })
  const svgW = maxX + PAD; const svgH = maxY + PAD + treeR + (isHeap.value ? 60 : 0)
  treeLayout.value = { svgW, svgH, treeR, nodes, edges, heapArray, heapArrayY }
  fitViewBox(0, 0, svgW, svgH)
}, { immediate: true })

/* ================================================================
 *  Graph 布局
 * ================================================================ */

interface GLayoutNode { id: number; label: string; x: number; y: number; color: string; diff: DiffStatus; displayLines: string[] }
interface GLayoutEdge { from: number; to: number; x1: number; y1: number; x2: number; y2: number; weight?: number; mx: number; my: number }
interface GLayout { svgW: number; svgH: number; graphR: number; nodes: GLayoutNode[]; edges: GLayoutEdge[] }
const graphLayout = shallowRef<GLayout | null>(null)

watch(() => graphData.value, (d) => {
  if (!d || d.nodes.length === 0) { graphLayout.value = null; computeDiff(new Map()); return }
  logDebug('[viz] graph layout, nodes =', d.nodes.length, 'edges =', d.edges.length)
  const labels = d.nodes.map((n: GraphNode) => n.label)
  const scale = computeNodeScale(labels, BASE_CHARS_CIRCLE)
  const graphR = Math.round(BASE_GRAPH_R * scale)
  const maxChars = Math.floor(BASE_CHARS_CIRCLE * scale)
  const svgW = 500; const svgH = 380; const cx = svgW / 2; const cy = svgH / 2
  const cR = Math.min(140, 34 * Math.max(3, d.nodes.length))
  const pMap = new Map<number, { x: number; y: number }>()
  const positions = new Map<string, CachedPosition>()
  const nodes: GLayoutNode[] = d.nodes.map((n: GraphNode, i: number) => {
    const angle = (2 * Math.PI * i) / Math.max(1, d.nodes.length) - Math.PI / 2
    const x = cx + cR * Math.cos(angle); const y = cy + cR * Math.sin(angle)
    pMap.set(n.id, { x, y }); positions.set(`g-${n.id}`, { shape: 'circle', x, y, r: graphR })
    return { id: n.id, label: n.label, x, y, color: NODE_PALETTE[Math.abs(n.id) % NODE_PALETTE.length], diff: 'unchanged' as DiffStatus, displayLines: splitDisplayLines(n.label, maxChars, 1) }
  })
  const edges: GLayoutEdge[] = []
  d.edges.forEach((e: GraphEdge & { weight?: number }) => {
    const from = pMap.get(e.from); const to = pMap.get(e.to)
    if (!from || !to) return
    const dx = to.x - from.x; const dy = to.y - from.y; const dist = Math.sqrt(dx * dx + dy * dy)
    if (dist === 0) return
    const ux = dx / dist; const uy = dy / dist
    edges.push({ from: e.from, to: e.to, x1: from.x + ux * graphR, y1: from.y + uy * graphR, x2: to.x - ux * graphR, y2: to.y - uy * graphR, weight: e.weight, mx: (from.x + to.x) / 2, my: (from.y + to.y) / 2 })
  })
  computeDiff(positions)
  nodes.forEach((n) => { if (addedIds.value.has(`g-${n.id}`)) n.diff = 'added' })
  graphLayout.value = { svgW, svgH, graphR, nodes, edges }
  fitViewBox(0, 0, svgW, svgH)
}, { immediate: true })

/* ================================================================
 *  Tooltip 构建器
 * ================================================================ */

function stackTipLines(index: number, value: IrValue, isTop: boolean): TipLine[] {
  const lines: TipLine[] = [{ label: '索引', value: `[${index}]` }, { label: '值', value: fullVal(value) }]
  if (isTop) lines.push({ label: '角色', value: 'TOP' })
  return lines
}

function queueTipLines(index: number, value: IrValue, labels: LayoutLabel[]): TipLine[] {
  const lines: TipLine[] = [{ label: '索引', value: `[${index}]` }, { label: '值', value: fullVal(value) }]
  if (labels.length > 0) lines.push({ label: '角色', value: labels.map((l) => l.text).join(', ') })
  return lines
}

function listTipLines(id: number, value: IrValue, labels: LayoutLabel[], nextId?: number, prevId?: number): TipLine[] {
  const lines: TipLine[] = [{ label: 'ID', value: `#${id}` }, { label: '值', value: fullVal(value) }]
  if (labels.length > 0) lines.push({ label: '角色', value: labels.map((l) => l.text).join(', ') })
  if (nextId !== undefined && nextId !== -1) lines.push({ label: '后继', value: `#${nextId}` })
  if (prevId !== undefined && prevId !== -1) lines.push({ label: '前驱', value: `#${prevId}` })
  return lines
}

function treeTipLines(nd: TreeLayoutNode): TipLine[] {
  const lines: TipLine[] = [{ label: 'ID', value: `#${nd.id}` }, { label: '值', value: fullVal(nd.value) }]
  if (nd.isRoot) lines.push({ label: '角色', value: 'ROOT' })
  if (nd.left !== -1) lines.push({ label: '左子', value: `#${nd.left}` })
  if (nd.right !== -1) lines.push({ label: '右子', value: `#${nd.right}` })
  return lines
}

function graphTipLines(nd: GLayoutNode): TipLine[] {
  return [{ label: 'ID', value: `#${nd.id}` }, { label: '标签', value: nd.label }]
}

/* ================================================================
 *  配置实时化
 * ================================================================ */

watch(() => vizFlags.enableAutoFit, (enabled) => {
  if (enabled) {
    animateToVb({ ...idealVb.value }, 200)
    userManualZoom.value = false
  }
})

watch(() => vizFlags.enableDiffHighlight, (enabled) => {
  if (!enabled) {
    addedIds.value = new Set()
    removedKeys.value = []
  }
})

/* ================================================================
 *  生命周期
 * ================================================================ */

onBeforeUnmount(() => {
  if (ghostTimer !== null) window.clearTimeout(ghostTimer)
  if (animFrame !== null) window.cancelAnimationFrame(animFrame)
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
    <div v-if="hasMetadata" class="viz-panel__meta">
      <span v-if="label" class="viz-panel__meta-item viz-panel__meta-label">{{ label }}</span>
      <span v-if="remarks?.title" class="viz-panel__meta-item">{{ remarks.title }}</span>
      <span v-if="remarks?.author" class="viz-panel__meta-item">{{ remarks.author }}</span>
      <span v-if="remarks?.comment" class="viz-panel__meta-item viz-panel__meta-comment">{{ remarks.comment }}</span>
    </div>

    <div ref="canvasRef" class="viz-panel__body">
      <Transition name="fade" mode="out-in">
        <VizPlaceholder v-if="isEmpty" key="empty" />

        <svg v-else key="canvas" class="viz-panel__svg" :viewBox="effectiveViewBox" preserveAspectRatio="xMidYMid meet"
          :style="{ '--ghost-ms': ghostDurationMs }" @wheel.prevent="onWheel" @pointerdown="onPanStart"
          @pointermove="onPanMove" @pointerup="onPanEnd" @pointerleave="onPanEnd" @dblclick="handleDblClick">
          <defs>
            <marker id="viz-arrow" markerWidth="8" markerHeight="8" refX="7" refY="4" orient="auto">
              <path d="M 0 0 L 8 4 L 0 8 Z" fill="var(--color-border-strong)" />
            </marker>
            <marker id="viz-arrow-accent" markerWidth="8" markerHeight="8" refX="7" refY="4" orient="auto">
              <path d="M 0 0 L 8 4 L 0 8 Z" fill="var(--color-accent)" />
            </marker>
          </defs>

          <!-- ═══════ 图形化空状态 ═══════ -->
          <g v-if="isStructureEmpty">
            <g v-if="emptyType === 'stack'" transform="translate(90,20)">
              <ellipse cx="60" cy="10" rx="50" ry="10" class="viz-empty-shape" />
              <line x1="10" y1="10" x2="10" y2="100" class="viz-empty-shape" />
              <line x1="110" y1="10" x2="110" y2="100" class="viz-empty-shape" />
              <ellipse cx="60" cy="100" rx="50" ry="10" class="viz-empty-shape" />
            </g>
            <g v-else-if="emptyType === 'queue'" transform="translate(50,60)">
              <rect x="0" y="0" width="50" height="36" rx="4" class="viz-empty-shape" />
              <rect x="70" y="0" width="50" height="36" rx="4" class="viz-empty-shape" />
              <rect x="140" y="0" width="50" height="36" rx="4" class="viz-empty-shape" />
              <line x1="54" y1="18" x2="66" y2="18" class="viz-empty-shape" />
              <line x1="124" y1="18" x2="136" y2="18" class="viz-empty-shape" />
            </g>
            <g v-else-if="emptyType === 'list'" transform="translate(55,60)">
              <rect x="0" y="0" width="70" height="40" rx="6" class="viz-empty-shape" />
              <line x1="74" y1="20" x2="116" y2="20" class="viz-empty-shape" marker-end="url(#viz-arrow)" />
              <rect x="120" y="0" width="70" height="40" rx="6" class="viz-empty-shape" />
            </g>
            <g v-else-if="emptyType === 'tree'" transform="translate(80,30)">
              <circle cx="70" cy="16" r="16" class="viz-empty-shape" />
              <line x1="58" y1="28" x2="36" y2="62" class="viz-empty-shape" />
              <line x1="82" y1="28" x2="104" y2="62" class="viz-empty-shape" />
              <circle cx="30" cy="78" r="16" class="viz-empty-shape" />
              <circle cx="110" cy="78" r="16" class="viz-empty-shape" />
            </g>
            <g v-else transform="translate(80,24)">
              <circle cx="70" cy="16" r="16" class="viz-empty-shape" />
              <circle cx="24" cy="120" r="16" class="viz-empty-shape" />
              <circle cx="116" cy="120" r="16" class="viz-empty-shape" />
              <line x1="58" y1="28" x2="36" y2="108" class="viz-empty-shape" />
              <line x1="82" y1="28" x2="104" y2="108" class="viz-empty-shape" />
              <line x1="40" y1="120" x2="100" y2="120" class="viz-empty-shape" />
            </g>
            <text x="150" y="170" class="viz-empty-text">{{ emptyLabel }}</text>
          </g>

          <!-- ═══════ Stack ═══════ -->
          <g v-if="stackLayout && stackLayout.items.length > 0">
            <template v-for="(it, i) in stackLayout.items" :key="`si-${i}`">
              <ellipse :cx="stackLayout.cx" :cy="it.y + stackLayout.cylItemH / 2" :rx="stackLayout.cylRx" :ry="CYL_RY"
                class="viz-cyl-sep" :class="{ 'viz-node--added': it.diff === 'added' }" />
              <rect :x="stackLayout.cx - stackLayout.cylRx" :y="it.y - stackLayout.cylItemH / 2"
                :width="stackLayout.cylRx * 2" :height="stackLayout.cylItemH" class="viz-cyl-body"
                :class="{ 'viz-cyl-body--top': it.isTop, 'viz-node--added': it.diff === 'added' }" />
              <line :x1="stackLayout.cx - stackLayout.cylRx" :y1="it.y - stackLayout.cylItemH / 2"
                :x2="stackLayout.cx - stackLayout.cylRx" :y2="it.y + stackLayout.cylItemH / 2" class="viz-cyl-side" />
              <line :x1="stackLayout.cx + stackLayout.cylRx" :y1="it.y - stackLayout.cylItemH / 2"
                :x2="stackLayout.cx + stackLayout.cylRx" :y2="it.y + stackLayout.cylItemH / 2" class="viz-cyl-side" />
              <ellipse :cx="stackLayout.cx" :cy="it.y - stackLayout.cylItemH / 2" :rx="stackLayout.cylRx" :ry="CYL_RY"
                class="viz-cyl-top" :class="{ 'viz-cyl-top--accent': it.isTop }" />
              <text :x="stackLayout.cx" :y="it.displayLines.length === 1 ? it.y + 4 : it.y - LINE_H / 2 + 4"
                class="viz-val" @pointerenter="showTip(stackTipLines(i, it.value, it.isTop), $event)"
                @pointerleave="hideTip">
                <template v-if="it.displayLines.length === 1">{{ it.displayLines[0] }}</template>
                <template v-else>
                  <tspan :x="stackLayout.cx" dy="0">{{ it.displayLines[0] }}</tspan>
                  <tspan :x="stackLayout.cx" :dy="LINE_H">{{ it.displayLines[1] }}</tspan>
                </template>
              </text>
              <template v-for="(lbl, li) in it.labels" :key="`sl-${i}-${li}`">
                <text v-if="lbl.position === 'top'" :x="stackLayout.cx + stackLayout.cylRx + 8" :y="it.y + 4"
                  class="viz-badge" :style="{ fill: lbl.color }" text-anchor="start">{{ lbl.text }}</text>
              </template>
            </template>
          </g>

          <!-- ═══════ Queue ═══════ -->
          <g v-if="queueLayout && queueLayout.items.length > 0">
            <template v-for="(it, i) in queueLayout.items" :key="`qi-${i}`">
              <rect :x="it.x" :y="PAD + 20" :width="queueLayout.boxW" :height="queueLayout.boxH" rx="6" class="viz-box"
                :class="{ 'viz-box--accent': it.labels.length > 0, 'viz-node--added': it.diff === 'added' }" />
              <text :x="it.x + queueLayout.boxW / 2"
                :y="it.displayLines.length === 1 ? PAD + 20 + queueLayout.boxH / 2 + 4 : PAD + 20 + queueLayout.boxH / 2 - LINE_H / 2 + 4"
                class="viz-val" @pointerenter="showTip(queueTipLines(i, it.value, it.labels), $event)"
                @pointerleave="hideTip">
                <template v-if="it.displayLines.length === 1">{{ it.displayLines[0] }}</template>
                <template v-else>
                  <tspan :x="it.x + queueLayout.boxW / 2" dy="0">{{ it.displayLines[0] }}</tspan>
                  <tspan :x="it.x + queueLayout.boxW / 2" :dy="LINE_H">{{ it.displayLines[1] }}</tspan>
                </template>
              </text>
              <line v-if="i < queueLayout.items.length - 1" :x1="it.x + queueLayout.boxW + 4"
                :y1="PAD + 20 + queueLayout.boxH / 2" :x2="queueLayout.items[i + 1].x - 4"
                :y2="PAD + 20 + queueLayout.boxH / 2" class="viz-arrow" marker-end="url(#viz-arrow)" />
              <template v-for="(lbl, li) in it.labels" :key="`qlbl-${i}-${li}`">
                <text v-if="lbl.position === 'top'" :x="it.x + queueLayout.boxW / 2" :y="PAD + 12" class="viz-badge"
                  :style="{ fill: lbl.color }">{{ lbl.text }}</text>
                <g v-else>
                  <line :x1="it.x + queueLayout.boxW / 2" :y1="PAD + 20 + queueLayout.boxH + 2"
                    :x2="it.x + queueLayout.boxW / 2" :y2="PAD + 20 + queueLayout.boxH + 18" class="viz-leader"
                    :style="{ stroke: lbl.color }" />
                  <text :x="it.x + queueLayout.boxW / 2" :y="PAD + 20 + queueLayout.boxH + 28" class="viz-badge"
                    :style="{ fill: lbl.color }">{{ lbl.text }}</text>
                </g>
              </template>
            </template>
          </g>

          <!-- ═══════ SList ═══════ -->
          <g v-if="slistLayout && slistLayout.nodes.length > 0">
            <template v-for="(nd, i) in slistLayout.nodes" :key="`sn-${nd.id}`">
              <rect :x="nd.x" :y="PAD + 20" :width="slistLayout.nodeW" :height="slistLayout.nodeH" rx="8"
                class="viz-node-rect"
                :class="{ 'viz-node-rect--accent': nd.labels.length > 0, 'viz-node--added': nd.diff === 'added' }" />
              <text :x="nd.x + slistLayout.nodeW / 2"
                :y="nd.displayLines.length === 1 ? PAD + 20 + slistLayout.nodeH / 2 - 4 : PAD + 20 + slistLayout.nodeH / 2 - LINE_H / 2 - 2"
                class="viz-val"
                @pointerenter="showTip(listTipLines(nd.id, nd.value, nd.labels, slistOrdered[i]?.next), $event)"
                @pointerleave="hideTip">
                <template v-if="nd.displayLines.length === 1">{{ nd.displayLines[0] }}</template>
                <template v-else>
                  <tspan :x="nd.x + slistLayout.nodeW / 2" dy="0">{{ nd.displayLines[0] }}</tspan>
                  <tspan :x="nd.x + slistLayout.nodeW / 2" :dy="LINE_H">{{ nd.displayLines[1] }}</tspan>
                </template>
              </text>
              <text :x="nd.x + slistLayout.nodeW / 2" :y="PAD + 20 + slistLayout.nodeH / 2 + 14" class="viz-ptr">#{{
                nd.id }}</text>
              <template v-for="(lbl, li) in nd.labels" :key="`snlbl-${nd.id}-${li}`">
                <text v-if="lbl.position === 'top'" :x="nd.x + slistLayout.nodeW / 2" :y="PAD + 12" class="viz-badge"
                  :style="{ fill: lbl.color }">{{ lbl.text }}</text>
                <g v-else>
                  <line :x1="nd.x + slistLayout.nodeW / 2" :y1="PAD + 20 + slistLayout.nodeH + 2"
                    :x2="nd.x + slistLayout.nodeW / 2" :y2="PAD + 20 + slistLayout.nodeH + 18" class="viz-leader"
                    :style="{ stroke: lbl.color }" />
                  <text :x="nd.x + slistLayout.nodeW / 2" :y="PAD + 20 + slistLayout.nodeH + 28" class="viz-badge"
                    :style="{ fill: lbl.color }">{{ lbl.text }}</text>
                </g>
              </template>
              <line v-if="i < slistLayout.nodes.length - 1" :x1="nd.x + slistLayout.nodeW + 2"
                :y1="PAD + 20 + slistLayout.nodeH / 2" :x2="slistLayout.nodes[i + 1].x - 2"
                :y2="PAD + 20 + slistLayout.nodeH / 2" class="viz-arrow" marker-end="url(#viz-arrow)" />
              <g v-if="nd.isTail">
                <line :x1="nd.x + slistLayout.nodeW + 2" :y1="PAD + 20 + slistLayout.nodeH / 2"
                  :x2="nd.x + slistLayout.nodeW + 28" :y2="PAD + 20 + slistLayout.nodeH / 2" class="viz-arrow" />
                <text :x="nd.x + slistLayout.nodeW + 32" :y="PAD + 20 + slistLayout.nodeH / 2 + 4"
                  class="viz-null">⊘</text>
              </g>
            </template>
          </g>

          <!-- ═══════ DList ═══════ -->
          <g v-if="dlistLayout && dlistLayout.nodes.length > 0">
            <template v-for="(nd, i) in dlistLayout.nodes" :key="`dn-${nd.id}`">
              <rect :x="nd.x" :y="PAD + 20" :width="dlistLayout.nodeW" :height="dlistLayout.nodeH" rx="8"
                class="viz-node-rect"
                :class="{ 'viz-node-rect--accent': nd.labels.length > 0, 'viz-node--added': nd.diff === 'added' }" />
              <text :x="nd.x + dlistLayout.nodeW / 2"
                :y="nd.displayLines.length === 1 ? PAD + 20 + dlistLayout.nodeH / 2 - 4 : PAD + 20 + dlistLayout.nodeH / 2 - LINE_H / 2 - 2"
                class="viz-val"
                @pointerenter="showTip(listTipLines(nd.id, nd.value, nd.labels, dlistOrdered[i]?.next, dlistOrdered[i]?.prev), $event)"
                @pointerleave="hideTip">
                <template v-if="nd.displayLines.length === 1">{{ nd.displayLines[0] }}</template>
                <template v-else>
                  <tspan :x="nd.x + dlistLayout.nodeW / 2" dy="0">{{ nd.displayLines[0] }}</tspan>
                  <tspan :x="nd.x + dlistLayout.nodeW / 2" :dy="LINE_H">{{ nd.displayLines[1] }}</tspan>
                </template>
              </text>
              <text :x="nd.x + dlistLayout.nodeW / 2" :y="PAD + 20 + dlistLayout.nodeH / 2 + 14" class="viz-ptr">#{{
                nd.id }}</text>
              <template v-for="(lbl, li) in nd.labels" :key="`dnlbl-${nd.id}-${li}`">
                <text v-if="lbl.position === 'top'" :x="nd.x + dlistLayout.nodeW / 2" :y="PAD + 12" class="viz-badge"
                  :style="{ fill: lbl.color }">{{ lbl.text }}</text>
                <g v-else>
                  <line :x1="nd.x + dlistLayout.nodeW / 2" :y1="PAD + 20 + dlistLayout.nodeH + 2"
                    :x2="nd.x + dlistLayout.nodeW / 2" :y2="PAD + 20 + dlistLayout.nodeH + 18" class="viz-leader"
                    :style="{ stroke: lbl.color }" />
                  <text :x="nd.x + dlistLayout.nodeW / 2" :y="PAD + 20 + dlistLayout.nodeH + 28" class="viz-badge"
                    :style="{ fill: lbl.color }">{{ lbl.text }}</text>
                </g>
              </template>
              <g v-if="i < dlistLayout.nodes.length - 1">
                <line :x1="nd.x + dlistLayout.nodeW + 2" :y1="PAD + 20 + dlistLayout.nodeH / 2 - 4"
                  :x2="dlistLayout.nodes[i + 1].x - 2" :y2="PAD + 20 + dlistLayout.nodeH / 2 - 4" class="viz-arrow"
                  marker-end="url(#viz-arrow)" />
                <line :x1="dlistLayout.nodes[i + 1].x - 2" :y1="PAD + 20 + dlistLayout.nodeH / 2 + 4"
                  :x2="nd.x + dlistLayout.nodeW + 2" :y2="PAD + 20 + dlistLayout.nodeH / 2 + 4" class="viz-arrow"
                  marker-end="url(#viz-arrow)" />
              </g>
            </template>
          </g>

          <!-- ═══════ Tree / BST / Heap ═══════ -->
          <g v-if="treeLayout">
            <line v-for="(e, i) in treeLayout.edges" :key="`te-${i}`" :x1="e.x1" :y1="e.y1" :x2="e.x2" :y2="e.y2"
              :class="isHeap ? 'viz-edge--dashed' : 'viz-edge'" />
            <g v-for="nd in treeLayout.nodes" :key="`tn-${nd.id}`">
              <circle v-if="!isHeap" :cx="nd.x" :cy="nd.y" :r="treeLayout.treeR" class="viz-tree-node"
                :class="{ 'viz-tree-node--root': nd.isRoot, 'viz-node--added': nd.diff === 'added' }" />
              <rect v-else :x="nd.x - treeLayout.treeR" :y="nd.y - treeLayout.treeR" :width="treeLayout.treeR * 2"
                :height="treeLayout.treeR * 2" rx="6" class="viz-tree-node"
                :class="{ 'viz-tree-node--root': nd.isRoot, 'viz-node--added': nd.diff === 'added' }" />
              <text :x="nd.x" :y="nd.y + 5" class="viz-val" @pointerenter="showTip(treeTipLines(nd), $event)"
                @pointerleave="hideTip">{{ nd.displayLines[0] }}</text>
            </g>
            <g v-if="isHeap && treeLayout.heapArray.length > 0">
              <line :x1="treeLayout.heapArray[0].x - 4" :y1="treeLayout.heapArrayY - 4"
                :x2="treeLayout.heapArray[treeLayout.heapArray.length - 1].x + 36 + 4" :y2="treeLayout.heapArrayY - 4"
                class="viz-edge" />
              <g v-for="h in treeLayout.heapArray" :key="`ha-${h.index}`">
                <rect :x="h.x" :y="treeLayout.heapArrayY" :width="34" :height="28" rx="4" class="viz-box" />
                <text :x="h.x + 17" :y="treeLayout.heapArrayY + 18" class="viz-val viz-val--sm">{{ h.displayLines[0]
                }}</text>
                <text :x="h.x + 17" :y="treeLayout.heapArrayY + 42" class="viz-ptr">{{ h.index }}</text>
              </g>
            </g>
          </g>

          <!-- ═══════ Graph ═══════ -->
          <g v-if="graphLayout">
            <line v-for="(e, i) in graphLayout.edges" :key="`ge-${i}`" :x1="e.x1" :y1="e.y1" :x2="e.x2" :y2="e.y2"
              class="viz-edge" :marker-end="isDirected ? 'url(#viz-arrow-accent)' : undefined" />
            <text v-for="(e, i) in graphLayout.edges" :key="`gw-${i}`" v-show="isWeighted && e.weight !== undefined"
              :x="e.mx" :y="e.my - 6" class="viz-weight">{{ e.weight }}</text>
            <g v-for="nd in graphLayout.nodes" :key="`gn-${nd.id}`">
              <circle :cx="nd.x" :cy="nd.y" :r="graphLayout.graphR" class="viz-graph-node"
                :class="{ 'viz-node--added': nd.diff === 'added' }"
                :style="{ fill: nd.color + '18', stroke: nd.color }" />
              <text :x="nd.x" :y="nd.y + 5" class="viz-val" @pointerenter="showTip(graphTipLines(nd), $event)"
                @pointerleave="hideTip">{{ nd.displayLines[0] }}</text>
            </g>
          </g>

          <!-- ═══════ Ghost 元素（移除渐退） ═══════ -->
          <template v-for="g in removedKeys" :key="`ghost-${g.key}`">
            <circle v-if="g.shape === 'circle'" :cx="g.x" :cy="g.y" :r="g.r" class="viz-ghost viz-ghost--circle" />
            <ellipse v-else-if="g.shape === 'ellipse'" :cx="g.x" :cy="g.y" :rx="g.rx" :ry="g.ry"
              class="viz-ghost viz-ghost--circle" />
            <rect v-else :x="g.x" :y="g.y" :width="g.w ?? 72" :height="g.h ?? 44" rx="6" class="viz-ghost" />
          </template>
        </svg>
      </Transition>

      <!-- Tooltip -->
      <Transition name="fade">
        <div v-if="tip.show" class="viz-tooltip" :style="tipStyle">
          <div v-for="(line, li) in tip.lines" :key="li" class="viz-tooltip__row">
            <span class="viz-tooltip__label">{{ line.label }}</span>
            <span class="viz-tooltip__value">{{ line.value }}</span>
          </div>
        </div>
      </Transition>

      <!-- 缩放还原提示 -->
      <Transition name="fade">
        <div v-if="showZoomHint" class="viz-panel__zoom-hint">
          双击画布还原缩放定位
        </div>
      </Transition>

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

.viz-panel__title :deep(.material-icon) {
  width: 18px;
  height: 18px;
}

.viz-panel__header-right {
  display: flex;
  align-items: center;
  gap: 8px;
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
  font-weight: var(--weight-medium);
}

.viz-panel__step-line {
  padding: 2px 6px;
  border-radius: var(--radius-sm);
  background-color: var(--color-bg-hover);
}

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

.viz-panel__meta-item {
  white-space: nowrap;
}

.viz-panel__meta-label {
  font-weight: var(--weight-semibold);
  color: var(--color-accent);
}

.viz-panel__meta-comment {
  font-style: italic;
}

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

.viz-panel__svg:active {
  cursor: grabbing;
}

/* ---- 缩放还原提示 ---- */
.viz-panel__zoom-hint {
  position: absolute;
  bottom: 14px;
  left: 50%;
  transform: translateX(-50%);
  padding: 5px 16px;
  border-radius: var(--radius-full);
  background-color: rgba(0, 0, 0, 0.48);
  color: rgba(255, 255, 255, 0.88);
  font-size: 11px;
  font-weight: var(--weight-medium);
  white-space: nowrap;
  pointer-events: none;
  z-index: 5;
  user-select: none;
  letter-spacing: 0.02em;
}

/* ---- Tooltip ---- */
.viz-tooltip {
  position: absolute;
  padding: 8px 12px;
  border-radius: var(--radius-control);
  background-color: var(--color-bg-surface);
  border: 1px solid var(--color-border-strong);
  box-shadow: var(--shadow-hover);
  pointer-events: none;
  z-index: 10;
  min-width: 100px;
}

.viz-tooltip__row {
  display: flex;
  align-items: baseline;
  gap: 8px;
  line-height: 1.6;
}

.viz-tooltip__label {
  font-size: 11px;
  color: var(--color-text-tertiary);
  white-space: nowrap;
  min-width: 28px;
}

.viz-tooltip__value {
  font-size: 11px;
  font-family: var(--font-mono);
  color: var(--color-text-primary);
  white-space: nowrap;
}

/* ---- 空状态图形 ---- */
.viz-empty-shape {
  fill: none;
  stroke: var(--color-text-tertiary);
  stroke-width: 1.5;
  stroke-dasharray: 6 4;
  opacity: 0.4;
}

.viz-empty-text {
  font-size: 13px;
  fill: var(--color-text-tertiary);
  text-anchor: middle;
  opacity: 0.6;
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

.viz-val--sm {
  font-size: 11px;
}

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

.viz-null {
  font-size: 16px;
  fill: var(--color-text-tertiary);
  dominant-baseline: central;
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

.viz-leader {
  stroke-width: 1;
  stroke-dasharray: 3 2;
}

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

.viz-box {
  fill: var(--color-bg-surface);
  stroke: var(--color-border-strong);
  stroke-width: 1;
}

.viz-box--accent {
  fill: var(--color-accent-wash);
  stroke: var(--color-accent);
}

.viz-node-rect {
  fill: var(--color-bg-surface);
  stroke: var(--color-border-strong);
  stroke-width: 1;
}

.viz-node-rect--accent {
  fill: var(--color-accent-wash);
  stroke: var(--color-accent);
}

.viz-tree-node {
  fill: var(--color-bg-surface);
  stroke: var(--color-border-strong);
  stroke-width: 1;
}

.viz-tree-node--root {
  fill: var(--color-accent-wash);
  stroke: var(--color-accent);
}

.viz-graph-node {
  stroke-width: 1.5;
}

.viz-node--added {
  transform-box: fill-box;
  transform-origin: center;
  animation: viz-pop var(--ghost-ms, 220ms) var(--ease);
}

@keyframes viz-pop {
  from {
    opacity: 0;
    transform: scale(0.85);
  }

  to {
    opacity: 1;
    transform: scale(1);
  }
}

.viz-ghost {
  fill: none;
  stroke: var(--color-error);
  stroke-width: 1;
  stroke-dasharray: 4 2;
  transform-box: fill-box;
  transform-origin: center;
  animation: viz-fade-out var(--ghost-ms, 350ms) var(--ease) forwards;
}

.viz-ghost--circle {
  fill: none;
}

@keyframes viz-fade-out {
  from {
    opacity: 0.4;
    transform: scale(1);
  }

  to {
    opacity: 0;
    transform: scale(0.85);
  }
}

/* ---- 平滑过渡 ---- */

.viz-panel--smooth .viz-tree-node,
.viz-panel--smooth .viz-graph-node {
  transition: cx var(--duration-viz) var(--ease), cy var(--duration-viz) var(--ease), x var(--duration-viz) var(--ease), y var(--duration-viz) var(--ease), r var(--duration-viz) var(--ease), fill var(--duration-viz) var(--ease), stroke var(--duration-viz) var(--ease);
}

.viz-panel--smooth .viz-edge,
.viz-panel--smooth .viz-edge--dashed,
.viz-panel--smooth .viz-arrow {
  transition: x1 var(--duration-viz) var(--ease), y1 var(--duration-viz) var(--ease), x2 var(--duration-viz) var(--ease), y2 var(--duration-viz) var(--ease);
}

.viz-panel--smooth .viz-val,
.viz-panel--smooth .viz-ptr,
.viz-panel--smooth .viz-badge {
  transition: x var(--duration-viz) var(--ease), y var(--duration-viz) var(--ease);
}

.viz-panel--smooth .viz-cyl-body,
.viz-panel--smooth .viz-cyl-top,
.viz-panel--smooth .viz-box,
.viz-panel--smooth .viz-node-rect {
  transition: fill var(--duration-viz) var(--ease), stroke var(--duration-viz) var(--ease);
}

/* 新增元素禁用位移过渡，避免从默认坐标飞入 */
.viz-panel--smooth .viz-node--added {
  transition: none;
}

@media (max-width: 1100px) {
  .viz-panel__svg {
    min-height: 240px;
  }
}

@media (prefers-reduced-motion: reduce) {
  .viz-node--added {
    animation: none;
  }

  .viz-ghost {
    animation: none;
    opacity: 0.2;
  }
}
</style>
