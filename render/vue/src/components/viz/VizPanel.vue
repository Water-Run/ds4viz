<!-- src/components/viz/VizPanel.vue -->
<script setup lang="ts">
/**
 * 可视化面板
 *
 * @file src/components/viz/VizPanel.vue
 * @author WaterRun
 * @date 2026-04-06
 * @component VizPanel
 */

import { computed, onBeforeUnmount, ref, shallowRef, watch } from 'vue'

import type {
  IrMeta,
  IrPackage,
  IrStateData,
  IrValue,
  IrRemarks,
  IrDocument,
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
  GraphNode,
  GraphEdge,
  IrObjectKind,
} from '@/types/ir'
import type { StepSummary, PhaseSegment, Frame } from '@/types/viz'
import type { NodeLifespan, NodeChangeTimeline } from '@/types/node-change'
import { vizFlags, logDebug } from '@/utils/viz-flags'
import {
  isNodeBasedKind,
  buildLifespans,
  buildTimeline,
  getDepartedKeys,
} from '@/utils/node-timeline'

import VizPlaceholder from './VizPlaceholder.vue'
import VizSettings from './VizSettings.vue'
import ChangeCard from './ChangeCard.vue'
import DepartedNodes from './DepartedNodes.vue'
import MaterialIcon from '@/components/common/MaterialIcon.vue'

/**
 * 组件属性定义
 */
interface Props {
  kind?: IrObjectKind
  data?: IrStateData
  step?: StepSummary | null
  label?: string
  remarks?: IrRemarks
  meta?: IrMeta
  irPackage?: IrPackage
  autoPlaying?: boolean
  /** 阶段段落列表 */
  phases?: PhaseSegment[]
  /** 当前所处阶段索引，null 表示不在任何阶段内 */
  currentPhaseIndex?: number | null
  /** 帧列表（变更卡片需要） */
  frameList?: Frame[]
  /** 当前帧索引（变更卡片需要） */
  currentFrameIndex?: number
  /** 完整 IR 文档引用（变更卡片需要） */
  irDocument?: IrDocument
}

/**
 * 组件事件定义
 */
interface Emits {
  /** 跳转至指定帧 */
  (event: 'jump-to-frame', frameIndex: number): void
}

const props = defineProps<Props>()
const emit = defineEmits<Emits>()

/* ================================================================
 *  常量
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
const TITLE_H = 28
const LINE_H = 14

const BASE_CHARS_CYL = 12
const BASE_CHARS_BOX = 7
const BASE_CHARS_NODE = 9
const BASE_CHARS_CIRCLE = 4

const HL_COLORS: Record<string, { fill: string; stroke: string }> = {
  focus: { fill: 'rgba(0,120,212,0.12)', stroke: '#0078d4' },
  visited: { fill: 'rgba(0,0,0,0.04)', stroke: '#a0a0a0' },
  active: { fill: 'rgba(8,145,178,0.10)', stroke: '#0891b2' },
  comparing: { fill: 'rgba(245,158,11,0.10)', stroke: '#d97706' },
  found: { fill: 'rgba(34,197,94,0.10)', stroke: '#16a34a' },
  error: { fill: 'rgba(239,68,68,0.10)', stroke: '#dc2626' },
}

const NODE_PALETTE = [
  '#0078d4',
  '#0e7c6b',
  '#7c3aed',
  '#c2410c',
  '#0369a1',
  '#6d28d9',
  '#b45309',
  '#059669',
  '#dc2626',
  '#4f46e5',
  '#0891b2',
  '#65a30d',
]

type DiffStatus = 'added' | 'removed' | 'unchanged'

const ACCENT_COLOR = 'var(--color-accent)'
const WARNING_COLOR = 'var(--color-warning)'

/** 卡片布局估算尺寸 */
const CARD_W_EST = 300
const CARD_H_SIMPLE = 56
const CARD_H_DETAIL = 76
const CARD_GAP = 16

/* ================================================================
 *  Ghost / Diff 缓存
 * ================================================================ */

interface CachedPosition {
  shape: 'rect' | 'circle' | 'ellipse'
  x: number
  y: number
  r?: number
  rx?: number
  ry?: number
  w?: number
  h?: number
}

type GhostItem = CachedPosition & { key: string }

/* ================================================================
 *  文本测量与自适应
 * ================================================================ */

function getEffectiveLength(text: string): number {
  let len = 0
  for (let index = 0; index < text.length; index += 1) {
    len += text.charCodeAt(index) > 0x7f ? 2 : 1
  }
  return len
}

function computeNodeScale(values: IrValue[], baseChars: number): number {
  if (values.length === 0) return 1
  const maxLen = Math.max(...values.map((value) => getEffectiveLength(String(value))))
  if (maxLen <= baseChars) return 1
  return Math.min(2, maxLen / baseChars)
}

function splitDisplayLines(
  value: IrValue,
  maxCharsPerLine: number,
  maxLines: number,
): string[] {
  const text = String(value)
  const effectiveLength = getEffectiveLength(text)
  if (effectiveLength <= maxCharsPerLine) return [text]

  const lines: string[] = []
  let position = 0

  for (let lineIndex = 0; lineIndex < maxLines && position < text.length; lineIndex += 1) {
    let lineChars = 0
    let lineEnd = position

    while (lineEnd < text.length) {
      const charWidth = text.charCodeAt(lineEnd) > 0x7f ? 2 : 1
      if (lineChars + charWidth > maxCharsPerLine) break
      lineChars += charWidth
      lineEnd += 1
    }

    if (lineIndex === maxLines - 1 && lineEnd < text.length) {
      const slice = text.slice(position, Math.max(position + 1, lineEnd - 1))
      lines.push(`${slice}…`)
    } else {
      lines.push(text.slice(position, lineEnd))
    }

    position = lineEnd
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
  return roles.map((role, index) => ({
    text: role.text,
    position: index === 0 ? 'top' : 'bottom',
    color: role.color,
  }))
}

/* ================================================================
 *  基础状态
 * ================================================================ */

const canvasRef = ref<HTMLDivElement | null>(null)

/* ================================================================
 *  变更卡片状态
 * ================================================================ */

/** 已展开的变更卡片节点键集合 */
const expandedCards = ref<Set<string>>(new Set())

/** 消逝节点中已展开的卡片键集合 */
const departedCards = ref<Set<string>>(new Set())

/** 各卡片的"展示全部帧"状态 */
const showAllFramesMap = ref<Map<string, boolean>>(new Map())

/** 全局变更卡片详细模式 */
const changeCardDetailMode = ref<boolean>(false)

/**
 * 节点最后已知位置缓存（响应式，保证消逝节点位置可追踪）
 *
 * key = 节点标识键（n-3 / t-0 / g-5）
 */
const lastKnownPositions = ref<Map<string, { x: number; y: number; r: number }>>(new Map())

/** 变更时间线缓存（按 IrDocument 实例） */
let timelineCacheDoc: object | null = null
const timelineCache = new Map<string, NodeChangeTimeline>()

/* ---- 布局 ref 前置声明（避免 computed TDZ） ---- */
const slistLayout = shallowRef<ListLayout | null>(null)
const dlistLayout = shallowRef<ListLayout | null>(null)
const treeLayout = shallowRef<TreeLayout | null>(null)
const graphLayout = shallowRef<GraphLayout | null>(null)

/* ================================================================
 *  分阶段渲染状态
 * ================================================================ */

/** 分阶段渲染：实际提交给布局的状态 */
const committedData = shallowRef<IrStateData | undefined>(props.data)

/** 等待提交的新状态 */
const pendingData = shallowRef<IrStateData | undefined>(undefined)

/** 新状态中的新增 key（仅用于 commit 后渐入） */
const pendingAddedKeys = shallowRef<Set<string>>(new Set())

/** 当前一轮应渐入 key */
const forceAddedKeys = shallowRef<Set<string>>(new Set())

/** 首次从空到非空时禁用平滑过渡，避免左上角飞入 */
const suppressSmoothOnce = ref<boolean>(false)

/** 动画时序计时器 */
let commitTimer: number | null = null
let clearAddedTimer: number | null = null

const isEmpty = computed<boolean>(() => committedData.value === undefined)

const kindStr = computed<string>(() => (props.kind ?? '') as string)

const kindLabel = computed<string>(() => {
  const mapping: Record<string, string> = {
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
  return mapping[kindStr.value] ?? kindStr.value
})

/* ---- 高亮映射 ---- */

const highlightMap = computed<Map<string, { style: string; level: number }>>(() => {
  const map = new Map<string, { style: string; level: number }>()
  const highlights = props.step?.highlights
  if (!highlights || highlights.length === 0) return map

  const kind = kindStr.value
  for (const h of highlights) {
    const level = h.level ?? 1
    if (h.kind === 'item') {
      const prefix = kind === 'stack' ? 's' : 'q'
      map.set(`${prefix}-${h.target}`, { style: h.style, level })
    } else if (h.kind === 'node') {
      let prefix = 'n'
      if (kind === 'binary_tree' || kind === 'bst') prefix = 't'
      else if (kind.startsWith('graph')) prefix = 'g'
      map.set(`${prefix}-${h.target}`, { style: h.style, level })
    } else if (h.kind === 'edge') {
      map.set(`e-${h.from}-${h.to}`, { style: h.style, level })
    }
  }
  return map
})

/**
 * 获取元素高亮内联样式（fill/stroke/strokeWidth）
 *
 * @param key - 元素标识键
 * @returns 内联样式对象，无高亮时返回空对象
 */
function getHlStyle(key: string): Record<string, string> {
  const hl = highlightMap.value.get(key)
  if (!hl) return {}
  const c = HL_COLORS[hl.style] ?? HL_COLORS.focus
  const sw = 1 + hl.level * 0.22
  return { fill: c.fill, stroke: c.stroke, strokeWidth: String(sw) }
}

/**
 * 获取边高亮内联样式
 *
 * @param from - 起始节点 ID
 * @param to - 目标节点 ID
 * @returns 内联样式对象
 */
function getEdgeHlStyle(from: number, to: number): Record<string, string> {
  const hl = highlightMap.value.get(`e-${from}-${to}`)
  if (!hl) return {}
  const c = HL_COLORS[hl.style] ?? HL_COLORS.focus
  const sw = 1.2 + hl.level * 0.2
  return { stroke: c.stroke, strokeWidth: String(sw) }
}

/**
 * 获取图节点内联样式（含高亮覆盖）
 *
 * @param nodeId - 节点 ID
 * @param defaultColor - 默认颜色
 * @returns 内联样式对象
 */
function getGraphNodeHlStyle(nodeId: number, defaultColor: string): Record<string, string> {
  const hl = highlightMap.value.get(`g-${nodeId}`)
  if (hl) {
    const c = HL_COLORS[hl.style] ?? HL_COLORS.focus
    return { fill: c.fill, stroke: c.stroke, strokeWidth: String(1.5 + hl.level * 0.2) }
  }
  return { fill: defaultColor + '18', stroke: defaultColor }
}

const isDirected = computed<boolean>(() => kindStr.value === 'graph_directed')
const isWeighted = computed<boolean>(() => kindStr.value === 'graph_weighted')

const smoothEnabled = computed<boolean>(() => {
  return vizFlags.enableSmoothTransitions && !suppressSmoothOnce.value
})

/* ---- 变更卡片派生 ---- */

/**
 * 有效帧索引（兜底 0）
 */
const effectiveFrameIndex = computed<number>(() => props.currentFrameIndex ?? 0)

/**
 * 当前结构是否支持节点变更卡片
 *
 * Stack / Queue 不启用变更卡片。
 */
const isNodeStructure = computed<boolean>(() => {
  if (!vizFlags.showChangeCards) return false
  if (!props.irDocument || !props.frameList) return false
  return isNodeBasedKind(kindStr.value)
})

/**
 * 所有节点的生命周期概要
 */
const nodeLifespans = computed<Map<string, NodeLifespan>>(() => {
  if (!props.irDocument || !props.frameList) return new Map()
  return buildLifespans(props.irDocument, props.frameList)
})

/**
 * 当前帧已消逝节点键列表
 */
const departedKeys = computed<string[]>(() => {
  if (!isNodeStructure.value) return []
  return getDepartedKeys(nodeLifespans.value, effectiveFrameIndex.value)
})

/**
 * 消逝节点生命周期列表（传给 DepartedNodes 组件）
 */
const departedLifespans = computed<NodeLifespan[]>(() => {
  return departedKeys.value
    .map((k) => nodeLifespans.value.get(k))
    .filter((s): s is NodeLifespan => s !== undefined)
})

/**
 * 是否显示消逝节点按钮
 */
const showDepartedButton = computed<boolean>(() => {
  return isNodeStructure.value && departedKeys.value.length > 0
})

/**
 * 当前可见节点的位置映射（从各布局 ref 中提取）
 */
const currentNodePositions = computed<Map<string, { x: number; y: number; r: number }>>(() => {
  const map = new Map<string, { x: number; y: number; r: number }>()

  if (treeLayout.value) {
    for (const n of treeLayout.value.nodes) {
      map.set(`t-${n.id}`, { x: n.x, y: n.y, r: treeLayout.value.treeR })
    }
  }

  if (graphLayout.value) {
    for (const n of graphLayout.value.nodes) {
      map.set(`g-${n.id}`, { x: n.x, y: n.y, r: graphLayout.value.graphR })
    }
  }

  if (slistLayout.value) {
    for (const n of slistLayout.value.nodes) {
      const cx = n.x + slistLayout.value.nodeW / 2
      const cy = PAD + 20 + slistLayout.value.nodeH / 2
      map.set(`n-${n.id}`, { x: cx, y: cy, r: slistLayout.value.nodeH / 2 })
    }
  }

  if (dlistLayout.value) {
    for (const n of dlistLayout.value.nodes) {
      const cx = n.x + dlistLayout.value.nodeW / 2
      const cy = PAD + 20 + dlistLayout.value.nodeH / 2
      map.set(`n-${n.id}`, { x: cx, y: cy, r: dlistLayout.value.nodeH / 2 })
    }
  }

  return map
})

/**
 * 同步当前节点位置到 lastKnownPositions
 */
watch(
  currentNodePositions,
  (positions) => {
    const next = new Map(lastKnownPositions.value)
    for (const [key, pos] of positions) {
      next.set(key, pos)
    }
    lastKnownPositions.value = next
  },
)

/**
 * 消逝节点的 SVG 渲染位置（取最后已知位置）
 */
const departedNodePositions = computed<Map<string, { x: number; y: number; r: number }>>(() => {
  const map = new Map<string, { x: number; y: number; r: number }>()
  for (const key of departedKeys.value) {
    const pos = lastKnownPositions.value.get(key)
    if (pos) map.set(key, pos)
  }
  return map
})

/**
 * 所有活跃卡片的键（含展开节点与消逝卡片）
 */
const allActiveCardKeys = computed<string[]>(() => {
  const keys: string[] = []
  for (const k of expandedCards.value) {
    if (currentNodePositions.value.has(k)) keys.push(k)
  }
  for (const k of departedCards.value) {
    if (lastKnownPositions.value.has(k)) keys.push(k)
  }
  return keys
})

/**
 * 活跃卡片的时间线映射
 */
const activeTimelines = computed<Map<string, NodeChangeTimeline>>(() => {
  const map = new Map<string, NodeChangeTimeline>()
  for (const key of allActiveCardKeys.value) {
    const tl = getOrBuildTimeline(key)
    if (tl) map.set(key, filterUnchangedEntries(tl))
  }
  return map
})


/**
 * 计算点 (px,py) 到线段 (ax,ay)-(bx,by) 的最短距离
 */
function pointToSegmentDist(
  px: number, py: number,
  ax: number, ay: number,
  bx: number, by: number,
): number {
  const dx = bx - ax
  const dy = by - ay
  const lenSq = dx * dx + dy * dy
  if (lenSq === 0) return Math.sqrt((px - ax) ** 2 + (py - ay) ** 2)
  const t = Math.max(0, Math.min(1, ((px - ax) * dx + (py - ay) * dy) / lenSq))
  const projX = ax + t * dx
  const projY = ay + t * dy
  return Math.sqrt((px - projX) ** 2 + (py - projY) ** 2)
}


/**
* 活跃卡片的布局位置
*
* 评分式算法：
*   1) 对每个节点生成 24 角度 × 5 距离级别共 120 个候选位置
*   2) 排除碰撞后，对每个候选评分：
*      - 距离惩罚：偏好靠近源节点
*      - 连线穿越惩罚：惩罚引线穿过其他节点
*      - 拥挤惩罚：惩罚与已放置矩形过近
*   3) 选取最低分候选；全部碰撞时兜底右侧偏移
*
* 碰撞池包含：全部可见节点、消逝节点、标题区域、已放置卡片。
*/
const cardPositions = computed<Map<string, { x: number; y: number }>>(() => {
  const result = new Map<string, { x: number; y: number }>()
  const placed: Array<{ x: number; y: number; w: number; h: number }> = []
  const cardH = changeCardDetailMode.value ? CARD_H_DETAIL : CARD_H_SIMPLE
  const gap = CARD_GAP + 8

  /* 收集节点圆心，用于连线穿越检测 */
  const nodeCenters: Array<{ x: number; y: number; r: number }> = []

  for (const [, pos] of currentNodePositions.value) {
    const rect = {
      x: pos.x - pos.r - 4,
      y: pos.y - pos.r - 4,
      w: pos.r * 2 + 8,
      h: pos.r * 2 + 8,
    }
    placed.push(rect)
    nodeCenters.push(pos)
  }
  for (const [, pos] of departedNodePositions.value) {
    const rect = {
      x: pos.x - pos.r - 4,
      y: pos.y - pos.r - 4,
      w: pos.r * 2 + 8,
      h: pos.r * 2 + 8,
    }
    placed.push(rect)
    nodeCenters.push(pos)
  }

  /* 标题 / 标注区域纳入碰撞池 */
  if (vizFlags.showVizTitle && headerTotalH.value > 0 && !isStructureEmpty.value) {
    const bounds = contentBounds.value
    placed.push({
      x: bounds.minX,
      y: bounds.minY - headerTotalH.value - 6,
      w: bounds.maxX - bounds.minX,
      h: headerTotalH.value + 12,
    })
  }

  const collides = (
    rect: { x: number; y: number; w: number; h: number },
  ): boolean =>
    placed.some(
      (p) =>
        rect.x < p.x + p.w
        && rect.x + rect.w > p.x
        && rect.y < p.y + p.h
        && rect.y + rect.h > p.y,
    )

  /**
   * 统计从节点到卡片中心的引线穿越了多少个其他节点（跳过自身）
   */
  const countCrossings = (
    cardCx: number,
    cardCy: number,
    nodeX: number,
    nodeY: number,
  ): number => {
    let n = 0
    for (const nc of nodeCenters) {
      if (Math.abs(nc.x - nodeX) < 1 && Math.abs(nc.y - nodeY) < 1) continue
      if (pointToSegmentDist(nc.x, nc.y, nodeX, nodeY, cardCx, cardCy) < nc.r + 8) {
        n += 1
      }
    }
    return n
  }

  /** 24 等分角度，5 级距离 → 最多 120 候选 */
  const ANGLE_STEPS = 24
  const DIST_MULTS = [1.0, 1.5, 2.2, 3.0, 4.0]

  for (const key of allActiveCardKeys.value) {
    const nodePos =
      currentNodePositions.value.get(key) ?? lastKnownPositions.value.get(key)
    if (!nodePos) continue

    const baseDistance = nodePos.r + gap
    let bestPos: { x: number; y: number } | null = null
    let bestScore = Infinity

    for (const dm of DIST_MULTS) {
      const distance = baseDistance * dm
      for (let a = 0; a < ANGLE_STEPS; a += 1) {
        const angle = (2 * Math.PI * a) / ANGLE_STEPS
        const cx = nodePos.x + Math.cos(angle) * distance - CARD_W_EST / 2
        const cy = nodePos.y + Math.sin(angle) * distance - cardH / 2

        if (collides({ x: cx, y: cy, w: CARD_W_EST, h: cardH })) continue

        let score = 0

        /* 距离惩罚：偏好近处 */
        score += distance * 0.8

        /* 连线穿越惩罚 */
        const cardCx = cx + CARD_W_EST / 2
        const cardCy = cy + cardH / 2
        score += countCrossings(cardCx, cardCy, nodePos.x, nodePos.y) * 300

        /* 拥挤惩罚：与已放置矩形中心过近 */
        for (const p of placed) {
          const pcx = p.x + p.w / 2
          const pcy = p.y + p.h / 2
          const d = Math.sqrt((cardCx - pcx) ** 2 + (cardCy - pcy) ** 2)
          if (d < 120) score += (120 - d) * 0.4
        }

        if (score < bestScore) {
          bestScore = score
          bestPos = { x: cx, y: cy }
        }
      }
    }

    /* 兜底 */
    if (!bestPos) {
      bestPos = {
        x: nodePos.x + baseDistance,
        y: nodePos.y - cardH / 2,
      }
    }

    result.set(key, bestPos)
    placed.push({ x: bestPos.x, y: bestPos.y, w: CARD_W_EST, h: cardH })
  }

  return result
})

/* ---- 元数据显示 ---- */

function formatUnknownValue(value: unknown): string {
  if (typeof value === 'string') return value
  if (typeof value === 'number' || typeof value === 'boolean') return String(value)
  try {
    return JSON.stringify(value)
  } catch {
    return '[unserializable]'
  }
}

const stepArgEntries = computed<Array<{ key: string; value: string }>>(() => {
  if (!props.step?.args) return []
  return Object.entries(props.step.args).map(([key, value]) => ({
    key,
    value: formatUnknownValue(value),
  }))
})

const hasMetadata = computed<boolean>(() => {
  if (!vizFlags.showMetadata) return false
  if (isStructureEmpty.value) return false
  if (props.label && props.label.length > 0) return true
  if (props.remarks?.title) return true
  if (props.remarks?.author) return true
  if (props.remarks?.comment) return true
  if (props.meta?.createdAt) return true
  if (props.step) return true
  if (stepArgEntries.value.length > 0) return true
  return false
})

/* ================================================================
 *  数据窄化（基于 committedData）
 * ================================================================ */

const stackData = computed<StackStateData | null>(() => {
  return kindStr.value === 'stack' ? (committedData.value as StackStateData) : null
})

const queueData = computed<QueueStateData | null>(() => {
  return kindStr.value === 'queue' ? (committedData.value as QueueStateData) : null
})

const slistData = computed<SListStateData | null>(() => {
  return kindStr.value === 'slist' ? (committedData.value as SListStateData) : null
})

const dlistData = computed<DListStateData | null>(() => {
  return kindStr.value === 'dlist' ? (committedData.value as DListStateData) : null
})

const treeData = computed<BinaryTreeStateData | null>(() => {
  const kind = kindStr.value
  if (kind === 'binary_tree' || kind === 'bst') {
    return committedData.value as BinaryTreeStateData
  }
  return null
})

const graphData = computed<GraphStateData | GraphWeightedStateData | null>(() => {
  const kind = kindStr.value
  if (kind === 'graph_undirected' || kind === 'graph_directed' || kind === 'graph_weighted') {
    return committedData.value as GraphStateData
  }
  return null
})

/* ---- 结构空状态 ---- */

const isStructureEmpty = computed<boolean>(() => {
  const data = committedData.value
  if (!data) return false

  const kind = kindStr.value
  if (kind === 'stack') return (data as StackStateData).items.length === 0
  if (kind === 'queue') return (data as QueueStateData).items.length === 0
  if (kind === 'slist') return (data as SListStateData).nodes.length === 0
  if (kind === 'dlist') return (data as DListStateData).nodes.length === 0

  if (kind === 'binary_tree' || kind === 'bst') {
    const tree = data as BinaryTreeStateData
    return tree.root === -1 || tree.nodes.length === 0
  }

  if (kind === 'graph_undirected' || kind === 'graph_directed' || kind === 'graph_weighted') {
    return (data as GraphStateData).nodes.length === 0
  }

  return false
})

const emptyLabel = computed<string>(() => {
  const mapping: Record<string, string> = {
    stack: '空栈',
    queue: '空队列',
    slist: '空链表',
    dlist: '空链表',
    binary_tree: '空树',
    bst: '空树',
    graph_undirected: '空图',
    graph_directed: '空图',
    graph_weighted: '空图',
  }
  return mapping[kindStr.value] ?? '空'
})

const emptyType = computed<string>(() => {
  const kind = kindStr.value
  if (kind === 'stack') return 'stack'
  if (kind === 'queue') return 'queue'
  if (kind === 'slist' || kind === 'dlist') return 'list'
  if (kind === 'binary_tree' || kind === 'bst') return 'tree'
  return 'graph'
})

/* ================================================================
 *  ViewBox / Zoom / Pan
 * ================================================================ */

interface ViewBoxState {
  x: number
  y: number
  w: number
  h: number
}

const vb = ref<ViewBoxState>({ x: 0, y: 0, w: 500, h: 400 })
const idealVb = ref<ViewBoxState>({ x: 0, y: 0, w: 500, h: 400 })
let viewBoxAnimFrame: number | null = null

const userManualZoom = ref<boolean>(false)

const effectiveViewBox = computed<string>(() => {
  if (isStructureEmpty.value) return '0 0 300 200'
  return `${vb.value.x} ${vb.value.y} ${vb.value.w} ${vb.value.h}`
})

const showZoomHint = computed<boolean>(() => {
  return userManualZoom.value && !isEmpty.value && !isStructureEmpty.value
})

/* ---- 标题渲染 ---- */

const contentBounds = ref<{ minX: number; minY: number; maxX: number; maxY: number }>({
  minX: 0,
  minY: 0,
  maxX: 500,
  maxY: 400,
})

const titleText = computed<string>(() => props.remarks?.title ?? '')
const commentText = computed<string>(() => props.remarks?.comment ?? '')
const authorText = computed<string>(() => props.remarks?.author ?? '')

const labelDisplayText = computed<string>(() => {
  const l = props.label ?? ''
  if (l === titleText.value) return ''
  return l
})

const headerTotalH = computed<number>(() => {
  if (!vizFlags.showVizTitle) return 0
  let h = 0
  if (labelDisplayText.value.length > 0) h += 14
  if (titleText.value.length > 0) h += 18
  if (commentText.value.length > 0) h += 8
  if (authorText.value.length > 0) h += 8
  if (h > 0) h += 6
  return h
})

const effectiveTitleX = computed<number>(() => {
  if (isStructureEmpty.value) return 150
  return (contentBounds.value.minX + contentBounds.value.maxX) / 2
})

const effectiveLabelY = computed<number>(() => {
  if (isStructureEmpty.value) return 12
  return contentBounds.value.minY - headerTotalH.value + 12
})

const effectiveTitleY = computed<number>(() => {
  let base: number
  if (isStructureEmpty.value) {
    base = labelDisplayText.value.length > 0 ? 26 : 14
  } else {
    base = contentBounds.value.minY - headerTotalH.value
    if (labelDisplayText.value.length > 0) base += 14
    base += 14
  }
  return base
})

const effectiveCommentY = computed<number>(() => {
  return effectiveTitleY.value + 12
})

const effectiveAuthorY = computed<number>(() => {
  if (commentText.value.length > 0) return effectiveCommentY.value + 7
  return effectiveTitleY.value + 12
})

const isPanning = ref<boolean>(false)
const didDrag = ref<boolean>(false)
const DRAG_THRESHOLD = 4
const panOrigin = ref<{ cx: number; cy: number; vx: number; vy: number }>({
  cx: 0,
  cy: 0,
  vx: 0,
  vy: 0,
})

const animateToVb = (target: ViewBoxState, duration = 200): void => {
  if (viewBoxAnimFrame !== null) {
    window.cancelAnimationFrame(viewBoxAnimFrame)
  }

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

    if (t < 1) {
      viewBoxAnimFrame = window.requestAnimationFrame(animate)
    } else {
      viewBoxAnimFrame = null
    }
  }

  viewBoxAnimFrame = window.requestAnimationFrame(animate)
}

const onWheel = (event: WheelEvent): void => {
  const svg = event.currentTarget as SVGSVGElement
  const rect = svg.getBoundingClientRect()
  const mx = (event.clientX - rect.left) / rect.width
  const my = (event.clientY - rect.top) / rect.height
  const factor = event.deltaY > 0 ? 1.1 : 1 / 1.1
  const nextW = vb.value.w * factor
  const nextH = vb.value.h * factor

  vb.value = {
    x: vb.value.x + (vb.value.w - nextW) * mx,
    y: vb.value.y + (vb.value.h - nextH) * my,
    w: nextW,
    h: nextH,
  }

  userManualZoom.value = true
}

const onPanStart = (event: PointerEvent): void => {
    isPanning.value = true
    didDrag.value = false
    panOrigin.value = {
      cx: event.clientX,
      cy: event.clientY,
      vx: vb.value.x,
      vy: vb.value.y,
    }
  }

const onPanMove = (event: PointerEvent): void => {
    if (!isPanning.value) return

    const rawDx = event.clientX - panOrigin.value.cx
    const rawDy = event.clientY - panOrigin.value.cy

    if (!didDrag.value && (Math.abs(rawDx) > DRAG_THRESHOLD || Math.abs(rawDy) > DRAG_THRESHOLD)) {
      didDrag.value = true
    }

    const svg = event.currentTarget as SVGSVGElement
    const rect = svg.getBoundingClientRect()
    const dx = (rawDx / rect.width) * vb.value.w
    const dy = (rawDy / rect.height) * vb.value.h

    vb.value = {
      ...vb.value,
      x: panOrigin.value.vx - dx,
      y: panOrigin.value.vy - dy,
    }
  }

const onPanEnd = (): void => {
    if (isPanning.value && didDrag.value) {
      userManualZoom.value = true
    }
    isPanning.value = false
  }

const fitViewBox = (minX: number, minY: number, maxX: number, maxY: number): void => {
  contentBounds.value = { minX, minY, maxX, maxY }

  const effectiveMinY = minY - headerTotalH.value

  const width = Math.max(200, maxX - minX + PAD * 2)
  const height = Math.max(150, maxY - effectiveMinY + PAD * 2)
  const target: ViewBoxState = {
    x: minX - PAD,
    y: effectiveMinY - PAD,
    w: width,
    h: height,
  }

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
 *  变更卡片交互
 * ================================================================ */

/**
 * 获取指定节点的 SVG 位置（优先当前帧，降级最后已知）
 *
 * @param key - 节点标识键
 * @returns 位置信息
 */
function getNodePos(key: string): { x: number; y: number; r: number } {
  return (
    currentNodePositions.value.get(key)
    ?? lastKnownPositions.value.get(key)
    ?? { x: 0, y: 0, r: 0 }
  )
}

/**
 * 过滤时间线中连续未变化的条目，仅保留与前一条相比发生了变化的帧。
 *
 * 假设 NodeChangeTimeline 拥有 entries 数组，每条含 frameIndex 及其他状态字段。
 * 比较时排除 frameIndex，仅以状态字段序列化结果判定是否变化。
 *
 * @param timeline - 原始时间线
 * @returns 过滤后的时间线（entries 长度 ≤ 原始长度）
 */
function filterUnchangedEntries(timeline: NodeChangeTimeline): NodeChangeTimeline {
  const { entries } = timeline as { entries: Array<Record<string, unknown>> }
  if (!entries || entries.length <= 1) return timeline

  const stateKey = (entry: Record<string, unknown>): string => {
    const parts: string[] = []
    for (const [k, v] of Object.entries(entry)) {
      if (k !== 'frameIndex') parts.push(`${k}:${JSON.stringify(v)}`)
    }
    return parts.sort().join('|')
  }

  const filtered = [entries[0]]
  let prevKey = stateKey(entries[0])

  for (let i = 1; i < entries.length; i += 1) {
    const key = stateKey(entries[i])
    if (key !== prevKey) {
      filtered.push(entries[i])
      prevKey = key
    }
  }

  return { ...timeline, entries: filtered } as unknown as NodeChangeTimeline
}


/**
 * 获取或构建指定节点的变更时间线（懒加载 + 缓存）
 *
 * @param key - 节点标识键
 * @returns 变更时间线，数据不足时返回 null
 */
function getOrBuildTimeline(key: string): NodeChangeTimeline | null {
  if (!props.irDocument || !props.frameList) return null

  if (timelineCacheDoc !== props.irDocument) {
    timelineCache.clear()
    timelineCacheDoc = props.irDocument
  }

  const cached = timelineCache.get(key)
  if (cached) return cached

  const timeline = buildTimeline(props.irDocument, props.frameList, key)
  timelineCache.set(key, timeline)
  return timeline
}

/**
 * 点击节点切换变更卡片
 *
 * @param key - 节点标识键
 * @param event - 点击事件
 */
function handleNodeCardClick(key: string, event: Event): void {
  event.stopPropagation()
  if (didDrag.value) return
  if (!isNodeStructure.value) return

  const next = new Set(expandedCards.value)
  if (next.has(key)) {
    next.delete(key)
  } else {
    next.add(key)
  }
  expandedCards.value = next
}

/**
 * 获取消逝节点的显示标签（截断）
 *
 * @param key - 节点标识键
 * @returns 截断后的显示文本
 */
function formatDepartedLabel(key: string): string {
  const span = nodeLifespans.value.get(key)
  if (!span) return ''
  const v = span.lastValue !== undefined ? String(span.lastValue) : (span.lastLabel ?? '')
  return v.length > 4 ? `${v.slice(0, 3)}…` : v
}

/**
 * 消逝节点卡片选中
 *
 * @param key - 节点标识键
 */
function handleDepartedSelect(key: string): void {
  const next = new Set(departedCards.value)
  if (next.has(key)) {
    next.delete(key)
  } else {
    next.add(key)
  }
  departedCards.value = next
}

/**
 * 关闭消逝节点卡片
 *
 * @param key - 节点标识键
 */
function handleDepartedCardClose(key: string): void {
  const next = new Set(departedCards.value)
  next.delete(key)
  departedCards.value = next
}

/**
 * 展开当前帧全部节点的变更卡片
 */
function handleExpandAllCards(): void {
  const next = new Set(expandedCards.value)
  for (const key of currentNodePositions.value.keys()) {
    next.add(key)
  }
  expandedCards.value = next
}

/**
 * 折叠全部变更卡片
 */
function handleCollapseAllCards(): void {
  expandedCards.value = new Set()
  departedCards.value = new Set()
}

/**
 * 是否有任何变更卡片处于展开状态
 */
const hasAnyCardsExpanded = computed<boolean>(() => {
  return expandedCards.value.size > 0 || departedCards.value.size > 0
})


/**
 * 卡片内跳帧
 *
 * @param frameIndex - 目标帧索引
 */
function handleCardJumpToFrame(frameIndex: number): void {
  emit('jump-to-frame', frameIndex)
}

/**
 * 切换指定卡片的"展示全部帧"
 *
 * @param key - 节点标识键
 */
function toggleShowAllFrames(key: string): void {
  const map = new Map(showAllFramesMap.value)
  map.set(key, !(map.get(key) ?? false))
  showAllFramesMap.value = map
}

/* ================================================================
 *  Tooltip
 * ================================================================ */

interface TipLine {
  label: string
  value: string
}

const tip = ref<{
  show: boolean
  x: number
  y: number
  lines: TipLine[]
}>({
  show: false,
  x: 0,
  y: 0,
  lines: [],
})

const tipStyle = computed(() => ({
  left: `${tip.value.x}px`,
  top: `${tip.value.y}px`,
}))

const showTip = (lines: TipLine[], event: PointerEvent): void => {
  if (!canvasRef.value) return
  const rect = canvasRef.value.getBoundingClientRect()
  tip.value = {
    show: true,
    lines,
    x: event.clientX - rect.left + 12,
    y: event.clientY - rect.top - 8,
  }
}

const hideTip = (): void => {
  tip.value = { ...tip.value, show: false }
}

function fullVal(value: IrValue): string {
  return typeof value === 'string' ? `"${value}"` : String(value)
}

/* ================================================================
 *  时序控制：Ghost -> Commit -> Added
 * ================================================================ */

const removedKeys = shallowRef<GhostItem[]>([])
const prevPositionCache = new Map<string, CachedPosition>()
const prevIds = shallowRef<Set<string>>(new Set())

function getGhostDuration(): number {
  if (props.autoPlaying) {
    return Math.max(150, Math.min(2000, vizFlags.playbackInterval * 0.5))
  }
  return 500
}

const ghostDurationMs = computed<string>(() => `${getGhostDuration()}ms`)

function clearCommitTimer(): void {
  if (commitTimer !== null) {
    window.clearTimeout(commitTimer)
    commitTimer = null
  }
}

function clearAddedKeysTimer(): void {
  if (clearAddedTimer !== null) {
    window.clearTimeout(clearAddedTimer)
    clearAddedTimer = null
  }
}

function scheduleClearAddedKeys(): void {
  clearAddedKeysTimer()
  clearAddedTimer = window.setTimeout(() => {
    forceAddedKeys.value = new Set()
    clearAddedTimer = null
  }, getGhostDuration())
}

function isAddedKey(key: string): boolean {
  return vizFlags.enableDiffHighlight && forceAddedKeys.value.has(key)
}

function getDataKeys(data: IrStateData | undefined, kind: string): Set<string> {
  const keys = new Set<string>()
  if (!data) return keys

  if (kind === 'stack') {
    const d = data as StackStateData
    d.items.forEach((_, index) => keys.add(`s-${index}`))
    return keys
  }

  if (kind === 'queue') {
    const d = data as QueueStateData
    d.items.forEach((_, index) => keys.add(`q-${index}`))
    return keys
  }

  if (kind === 'slist') {
    const d = data as SListStateData
    d.nodes.forEach((node) => keys.add(`n-${node.id}`))
    return keys
  }

  if (kind === 'dlist') {
    const d = data as DListStateData
    d.nodes.forEach((node) => keys.add(`n-${node.id}`))
    return keys
  }

  if (kind === 'binary_tree' || kind === 'bst') {
    const d = data as BinaryTreeStateData
    d.nodes.forEach((node) => keys.add(`t-${node.id}`))
    return keys
  }

  if (kind === 'graph_undirected' || kind === 'graph_directed' || kind === 'graph_weighted') {
    const d = data as GraphStateData
    d.nodes.forEach((node) => keys.add(`g-${node.id}`))
    return keys
  }

  return keys
}

function applyTransition(newData: IrStateData | undefined): void {
  const kind = kindStr.value
  const current = committedData.value

  if (!current && newData) {
    clearCommitTimer()
    clearAddedKeysTimer()
    removedKeys.value = []
    forceAddedKeys.value = new Set()
    suppressSmoothOnce.value = true
    committedData.value = newData

    window.setTimeout(() => {
      suppressSmoothOnce.value = false
    }, 0)

    logDebug('[viz] first non-empty frame committed without transition')
    return
  }

  if (!newData || !current || !vizFlags.enableDiffHighlight) {
    clearCommitTimer()
    clearAddedKeysTimer()
    removedKeys.value = []
    const added = getDataKeys(newData, kind)
    const old = getDataKeys(current, kind)
    const addedOnly = new Set<string>()
    for (const key of added) {
      if (!old.has(key)) addedOnly.add(key)
    }
    forceAddedKeys.value = addedOnly
    committedData.value = newData
    scheduleClearAddedKeys()
    return
  }

  const oldKeys = getDataKeys(current, kind)
  const nextKeys = getDataKeys(newData, kind)

  const removedSet = new Set<string>()
  const addedSet = new Set<string>()

  for (const key of oldKeys) {
    if (!nextKeys.has(key)) removedSet.add(key)
  }

  for (const key of nextKeys) {
    if (!oldKeys.has(key)) addedSet.add(key)
  }

  if (removedSet.size === 0) {
    clearCommitTimer()
    clearAddedKeysTimer()
    removedKeys.value = []
    forceAddedKeys.value = addedSet
    committedData.value = newData
    scheduleClearAddedKeys()
    return
  }

  const ghosts: GhostItem[] = []
  for (const key of removedSet) {
    const cached = prevPositionCache.get(key)
    if (cached) ghosts.push({ key, ...cached })
  }

  removedKeys.value = ghosts
  pendingData.value = newData
  pendingAddedKeys.value = addedSet

  clearCommitTimer()
  clearAddedKeysTimer()

  commitTimer = window.setTimeout(() => {
    removedKeys.value = []
    committedData.value = pendingData.value
    forceAddedKeys.value = new Set(pendingAddedKeys.value)
    pendingData.value = undefined
    pendingAddedKeys.value = new Set()
    commitTimer = null
    scheduleClearAddedKeys()
    logDebug('[viz] commit after ghost fade-out')
  }, getGhostDuration())
}

watch(
  () => props.data,
  (nextData) => {
    applyTransition(nextData)
  },
)

/* ================================================================
 *  链表排序
 * ================================================================ */

const slistOrdered = computed<SListNode[]>(() => {
  if (!slistData.value) return []

  const map = new Map<number, SListNode>()
  slistData.value.nodes.forEach((node) => map.set(node.id, node))

  const result: SListNode[] = []
  let current = slistData.value.head
  const visited = new Set<number>()

  while (current !== -1 && !visited.has(current)) {
    visited.add(current)
    const node = map.get(current)
    if (!node) break
    result.push(node)
    current = node.next
  }

  return result
})

const dlistOrdered = computed<DListNode[]>(() => {
  if (!dlistData.value) return []

  const map = new Map<number, DListNode>()
  dlistData.value.nodes.forEach((node) => map.set(node.id, node))

  const result: DListNode[] = []
  let current = dlistData.value.head
  const visited = new Set<number>()

  while (current !== -1 && !visited.has(current)) {
    visited.add(current)
    const node = map.get(current)
    if (!node) break
    result.push(node)
    current = node.next
  }

  return result
})

/* ================================================================
 *  布局：Stack
 * ================================================================ */

interface StackLayoutItem {
  value: IrValue
  y: number
  isTop: boolean
  diff: DiffStatus
  displayLines: string[]
  labels: LayoutLabel[]
}

interface StackLayout {
  svgW: number
  svgH: number
  cx: number
  cylRx: number
  cylItemH: number
  items: StackLayoutItem[]
}

const stackLayout = shallowRef<StackLayout | null>(null)

watch(
  () => stackData.value,
  (data) => {
    if (!data) {
      stackLayout.value = null
      return
    }

    if (data.items.length === 0) {
      stackLayout.value = null
      prevIds.value = new Set()
      prevPositionCache.clear()
      return
    }

    const scale = computeNodeScale(data.items, BASE_CHARS_CYL)
    const cylRx = BASE_CYL_RX * scale
    const maxChars = Math.floor(BASE_CHARS_CYL * scale)
    const anyTwoLine = data.items.some((item) => getEffectiveLength(String(item)) > maxChars)
    const cylItemH = BASE_CYL_ITEM_H * (anyTwoLine ? 1.35 : 1)

    const n = data.items.length
    const svgW = cylRx * 2 + PAD * 2
    const svgH = n * cylItemH + CYL_RY * 2 + PAD * 2 + 20
    const cx = svgW / 2
    const baseY = svgH - PAD - CYL_RY

    const positions = new Map<string, CachedPosition>()

    const items: StackLayoutItem[] = data.items.map((value, index) => {
      const y = baseY - (index + 0.5) * cylItemH
      const key = `s-${index}`
      positions.set(key, {
        shape: 'ellipse',
        x: cx,
        y,
        rx: cylRx,
        ry: CYL_RY,
      })

      const roles: Array<{ text: string; color: string }> = []
      if (index === data.top) roles.push({ text: 'TOP', color: ACCENT_COLOR })

      return {
        value,
        y,
        isTop: index === data.top,
        diff: isAddedKey(key) ? 'added' : 'unchanged',
        displayLines: splitDisplayLines(value, maxChars, 2),
        labels: buildLabels(roles),
      }
    })

    stackLayout.value = { svgW, svgH, cx, cylRx, cylItemH, items }

    prevIds.value = new Set(positions.keys())
    prevPositionCache.clear()
    for (const [key, value] of positions) {
      prevPositionCache.set(key, value)
    }

    fitViewBox(0, 0, svgW, svgH)
  },
  { immediate: true },
)

/* ================================================================
 *  布局：Queue
 * ================================================================ */

interface QueueLayoutItem {
  value: IrValue
  x: number
  diff: DiffStatus
  displayLines: string[]
  labels: LayoutLabel[]
}

interface QueueLayout {
  svgW: number
  svgH: number
  boxW: number
  boxH: number
  items: QueueLayoutItem[]
}

const queueLayout = shallowRef<QueueLayout | null>(null)

watch(
  () => queueData.value,
  (data) => {
    if (!data) {
      queueLayout.value = null
      return
    }

    if (data.items.length === 0) {
      queueLayout.value = null
      prevIds.value = new Set()
      prevPositionCache.clear()
      return
    }

    const scale = computeNodeScale(data.items, BASE_CHARS_BOX)
    const boxW = BASE_BOX_W * scale
    const boxH = BASE_BOX_H
    const maxChars = Math.floor(BASE_CHARS_BOX * scale)

    const n = data.items.length
    const hasBottomLabel = data.items.some((_, index) => {
      const roles: string[] = []
      if (index === data.front) roles.push('FRONT')
      if (index === data.rear) roles.push('REAR')
      return roles.length > 1
    })

    const svgW = n * boxW + (n - 1) * BOX_GAP + PAD * 2
    const svgH = boxH + PAD * 2 + 24 + (hasBottomLabel ? 36 : 0)

    const positions = new Map<string, CachedPosition>()

    const items: QueueLayoutItem[] = data.items.map((value, index) => {
      const x = PAD + index * (boxW + BOX_GAP)
      const key = `q-${index}`

      positions.set(key, {
        shape: 'rect',
        x,
        y: PAD + 20,
        w: boxW,
        h: boxH,
      })

      const roles: Array<{ text: string; color: string }> = []
      if (index === data.front) roles.push({ text: 'FRONT', color: ACCENT_COLOR })
      if (index === data.rear) roles.push({ text: 'REAR', color: WARNING_COLOR })

      return {
        value,
        x,
        diff: isAddedKey(key) ? 'added' : 'unchanged',
        displayLines: splitDisplayLines(value, maxChars, 2),
        labels: buildLabels(roles),
      }
    })

    queueLayout.value = { svgW, svgH, boxW, boxH, items }

    prevIds.value = new Set(positions.keys())
    prevPositionCache.clear()
    for (const [key, value] of positions) {
      prevPositionCache.set(key, value)
    }

    fitViewBox(0, 0, svgW, svgH)
  },
  { immediate: true },
)

/* ================================================================
 *  布局：List
 * ================================================================ */

interface ListLayoutNode {
  id: number
  value: IrValue
  x: number
  diff: DiffStatus
  displayLines: string[]
  labels: LayoutLabel[]
  isTail: boolean
}

interface ListLayout {
  svgW: number
  svgH: number
  nodeW: number
  nodeH: number
  nodes: ListLayoutNode[]
}

watch(
  () => slistOrdered.value,
  (ordered) => {
    if (!slistData.value) {
      slistLayout.value = null
      return
    }

    if (ordered.length === 0) {
      slistLayout.value = null
      prevIds.value = new Set()
      prevPositionCache.clear()
      return
    }

    const values = ordered.map((node) => node.value)
    const scale = computeNodeScale(values, BASE_CHARS_NODE)
    const nodeW = BASE_NODE_W * scale
    const nodeH = BASE_NODE_H
    const maxChars = Math.floor(BASE_CHARS_NODE * scale)

    const n = ordered.length
    const svgW = n * nodeW + (n - 1) * NODE_GAP + PAD * 2 + 40
    const svgH = nodeH + PAD * 2 + 24 + 36

    const positions = new Map<string, CachedPosition>()

    const nodes: ListLayoutNode[] = ordered.map((node, index) => {
      const x = PAD + index * (nodeW + NODE_GAP)
      const key = `n-${node.id}`

      positions.set(key, {
        shape: 'rect',
        x,
        y: PAD + 20,
        w: nodeW,
        h: nodeH,
      })

      const roles: Array<{ text: string; color: string }> = []
      if (node.id === slistData.value!.head) {
        roles.push({ text: 'HEAD', color: ACCENT_COLOR })
      }

      return {
        id: node.id,
        value: node.value,
        x,
        diff: isAddedKey(key) ? 'added' : 'unchanged',
        displayLines: splitDisplayLines(node.value, maxChars, 2),
        labels: buildLabels(roles),
        isTail: node.next === -1,
      }
    })

    slistLayout.value = { svgW, svgH, nodeW, nodeH, nodes }

    prevIds.value = new Set(positions.keys())
    prevPositionCache.clear()
    for (const [key, value] of positions) {
      prevPositionCache.set(key, value)
    }

    fitViewBox(0, 0, svgW, svgH)
  },
  { immediate: true },
)

watch(
  () => dlistOrdered.value,
  (ordered) => {
    if (!dlistData.value) {
      dlistLayout.value = null
      return
    }

    if (ordered.length === 0) {
      dlistLayout.value = null
      prevIds.value = new Set()
      prevPositionCache.clear()
      return
    }

    const values = ordered.map((node) => node.value)
    const scale = computeNodeScale(values, BASE_CHARS_NODE)
    const nodeW = BASE_NODE_W * scale
    const nodeH = BASE_NODE_H
    const maxChars = Math.floor(BASE_CHARS_NODE * scale)

    const n = ordered.length
    const svgW = n * nodeW + (n - 1) * NODE_GAP + PAD * 2 + 40
    const svgH = nodeH + PAD * 2 + 24 + 36

    const positions = new Map<string, CachedPosition>()

    const nodes: ListLayoutNode[] = ordered.map((node, index) => {
      const x = PAD + index * (nodeW + NODE_GAP)
      const key = `n-${node.id}`

      positions.set(key, {
        shape: 'rect',
        x,
        y: PAD + 20,
        w: nodeW,
        h: nodeH,
      })

      const roles: Array<{ text: string; color: string }> = []
      if (node.id === dlistData.value!.head) roles.push({ text: 'HEAD', color: ACCENT_COLOR })
      if (node.id === dlistData.value!.tail) roles.push({ text: 'TAIL', color: WARNING_COLOR })

      return {
        id: node.id,
        value: node.value,
        x,
        diff: isAddedKey(key) ? 'added' : 'unchanged',
        displayLines: splitDisplayLines(node.value, maxChars, 2),
        labels: buildLabels(roles),
        isTail: node.id === dlistData.value!.tail,
      }
    })

    dlistLayout.value = { svgW, svgH, nodeW, nodeH, nodes }

    prevIds.value = new Set(positions.keys())
    prevPositionCache.clear()
    for (const [key, value] of positions) {
      prevPositionCache.set(key, value)
    }

    fitViewBox(0, 0, svgW, svgH)
  },
  { immediate: true },
)

/* ================================================================
 *  布局：Tree / BST
 * ================================================================ */

interface TreeLayoutNode {
  id: number
  value: IrValue
  x: number
  y: number
  isRoot: boolean
  diff: DiffStatus
  displayLines: string[]
  left: number
  right: number
}

interface TreeLayoutEdge {
  x1: number
  y1: number
  x2: number
  y2: number
}

interface TreeLayout {
  svgW: number
  svgH: number
  treeR: number
  nodes: TreeLayoutNode[]
  edges: TreeLayoutEdge[]
}

watch(
  () => treeData.value,
  (data) => {
    if (!data || data.root === -1 || data.nodes.length === 0) {
      treeLayout.value = null
      prevIds.value = new Set()
      prevPositionCache.clear()
      return
    }

    logDebug('[viz] tree layout, nodes =', data.nodes.length)

    const values = data.nodes.map((node) => node.value)
    const scale = computeNodeScale(values, BASE_CHARS_CIRCLE)
    const treeR = Math.round(BASE_TREE_R * scale)
    const maxChars = Math.floor(BASE_CHARS_CIRCLE * scale)

    const nodeMap = new Map<number, BinaryTreeNode>()
    data.nodes.forEach((node) => nodeMap.set(node.id, node))

    const spacing = treeR * 2.5 + 12
    let nextPos = 0
    const posMap = new Map<number, { x: number; y: number }>()
    const visitedSet = new Set<number>()

    const inOrder = (nodeId: number, depth: number): void => {
      if (nodeId === -1 || visitedSet.has(nodeId)) return
      const node = nodeMap.get(nodeId)
      if (!node) return
      visitedSet.add(nodeId)
      inOrder(node.left, depth + 1)
      posMap.set(nodeId, { x: nextPos * spacing, y: PAD + depth * TREE_LEVEL_H })
      nextPos += 1
      inOrder(node.right, depth + 1)
    }

    inOrder(data.root, 0)

    let minX = Infinity
    let maxX = -Infinity
    let maxY = 0
    posMap.forEach((p) => {
      if (p.x < minX) minX = p.x
      if (p.x > maxX) maxX = p.x
      if (p.y > maxY) maxY = p.y
    })

    const offsetX = PAD + treeR - minX
    posMap.forEach((p) => {
      p.x += offsetX
    })

    const finalMaxX = maxX + offsetX
    const svgW = finalMaxX + PAD + treeR
    const svgH = maxY + PAD + treeR

    const positions = new Map<string, CachedPosition>()

    const nodes: TreeLayoutNode[] = data.nodes
      .filter((node) => posMap.has(node.id))
      .map((node) => {
        const point = posMap.get(node.id)!
        const key = `t-${node.id}`

        positions.set(key, {
          shape: 'circle',
          x: point.x,
          y: point.y,
          r: treeR,
        })

        return {
          id: node.id,
          value: node.value,
          x: point.x,
          y: point.y,
          isRoot: node.id === data.root,
          diff: isAddedKey(key) ? 'added' : 'unchanged',
          displayLines: splitDisplayLines(node.value, maxChars, 1),
          left: node.left,
          right: node.right,
        }
      })

    const edges: TreeLayoutEdge[] = []
    data.nodes.forEach((node) => {
      const from = posMap.get(node.id)
      if (!from) return
      for (const childId of [node.left, node.right]) {
        if (childId === -1) continue
        const to = posMap.get(childId)
        if (to) {
          edges.push({ x1: from.x, y1: from.y, x2: to.x, y2: to.y })
        }
      }
    })

    treeLayout.value = { svgW, svgH, treeR, nodes, edges }

    prevIds.value = new Set(positions.keys())
    prevPositionCache.clear()
    for (const [key, value] of positions) {
      prevPositionCache.set(key, value)
    }

    fitViewBox(0, 0, svgW, svgH)
  },
  { immediate: true },
)

/* ================================================================
 *  布局：Graph
 * ================================================================ */

interface GraphLayoutNode {
  id: number
  label: string
  x: number
  y: number
  color: string
  diff: DiffStatus
  displayLines: string[]
}

interface GraphLayoutEdge {
  from: number
  to: number
  x1: number
  y1: number
  x2: number
  y2: number
  weight?: number
  mx: number
  my: number
}

interface GraphLayout {
  svgW: number
  svgH: number
  graphR: number
  nodes: GraphLayoutNode[]
  edges: GraphLayoutEdge[]
}

watch(
  () => graphData.value,
  (data) => {
    if (!data || data.nodes.length === 0) {
      graphLayout.value = null
      prevIds.value = new Set()
      prevPositionCache.clear()
      return
    }

    logDebug('[viz] graph layout, nodes =', data.nodes.length, 'edges =', data.edges.length)

    const labels = data.nodes.map((node) => node.label)
    const scale = computeNodeScale(labels, BASE_CHARS_CIRCLE)
    const graphR = Math.round(BASE_GRAPH_R * scale)
    const maxChars = Math.floor(BASE_CHARS_CIRCLE * scale)

    const svgW = 500
    const svgH = 380
    const cx = svgW / 2
    const cy = svgH / 2
    const circleR = Math.min(140, 34 * Math.max(3, data.nodes.length))

    const positionMap = new Map<number, { x: number; y: number }>()
    const positions = new Map<string, CachedPosition>()

    const nodes: GraphLayoutNode[] = data.nodes.map((node: GraphNode, index: number) => {
      const angle = (2 * Math.PI * index) / Math.max(1, data.nodes.length) - Math.PI / 2
      const x = cx + circleR * Math.cos(angle)
      const y = cy + circleR * Math.sin(angle)

      positionMap.set(node.id, { x, y })

      const key = `g-${node.id}`
      positions.set(key, {
        shape: 'circle',
        x,
        y,
        r: graphR,
      })

      return {
        id: node.id,
        label: node.label,
        x,
        y,
        color: NODE_PALETTE[Math.abs(node.id) % NODE_PALETTE.length],
        diff: isAddedKey(key) ? 'added' : 'unchanged',
        displayLines: splitDisplayLines(node.label, maxChars, 1),
      }
    })

    const edges: GraphLayoutEdge[] = []
    data.edges.forEach((edge: GraphEdge & { weight?: number }) => {
      const from = positionMap.get(edge.from)
      const to = positionMap.get(edge.to)
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
        x1: from.x + ux * graphR,
        y1: from.y + uy * graphR,
        x2: to.x - ux * graphR,
        y2: to.y - uy * graphR,
        weight: edge.weight,
        mx: (from.x + to.x) / 2,
        my: (from.y + to.y) / 2,
      })
    })

    graphLayout.value = { svgW, svgH, graphR, nodes, edges }

    prevIds.value = new Set(positions.keys())
    prevPositionCache.clear()
    for (const [key, value] of positions) {
      prevPositionCache.set(key, value)
    }

    fitViewBox(0, 0, svgW, svgH)
  },
  { immediate: true },
)

/* ================================================================
 *  Tooltip 构建
 * ================================================================ */

function stackTipLines(index: number, value: IrValue, isTop: boolean): TipLine[] {
  const lines: TipLine[] = [
    { label: '索引', value: `[${index}]` },
    { label: '值', value: fullVal(value) },
  ]
  if (isTop) lines.push({ label: '角色', value: 'TOP' })
  return lines
}

function queueTipLines(index: number, value: IrValue, labels: LayoutLabel[]): TipLine[] {
  const lines: TipLine[] = [
    { label: '索引', value: `[${index}]` },
    { label: '值', value: fullVal(value) },
  ]
  if (labels.length > 0) {
    lines.push({ label: '角色', value: labels.map((item) => item.text).join(', ') })
  }
  return lines
}

function listTipLines(
  id: number,
  value: IrValue,
  labels: LayoutLabel[],
  nextId?: number,
  prevId?: number,
): TipLine[] {
  const lines: TipLine[] = [
    { label: 'ID', value: `#${id}` },
    { label: '值', value: fullVal(value) },
  ]

  if (labels.length > 0) {
    lines.push({ label: '角色', value: labels.map((item) => item.text).join(', ') })
  }

  if (nextId !== undefined && nextId !== -1) {
    lines.push({ label: '后继', value: `#${nextId}` })
  }

  if (prevId !== undefined && prevId !== -1) {
    lines.push({ label: '前驱', value: `#${prevId}` })
  }

  return lines
}

function treeTipLines(node: TreeLayoutNode): TipLine[] {
  const lines: TipLine[] = [
    { label: 'ID', value: `#${node.id}` },
    { label: '值', value: fullVal(node.value) },
  ]

  if (node.isRoot) lines.push({ label: '角色', value: 'ROOT' })
  if (node.left !== -1) lines.push({ label: '左子', value: `#${node.left}` })
  if (node.right !== -1) lines.push({ label: '右子', value: `#${node.right}` })

  return lines
}

function graphTipLines(node: GraphLayoutNode): TipLine[] {
  return [
    { label: 'ID', value: `#${node.id}` },
    { label: '标签', value: node.label },
  ]
}

/* ================================================================
 *  配置实时化
 * ================================================================ */

watch(
  () => vizFlags.enableAutoFit,
  (enabled) => {
    if (enabled) {
      animateToVb({ ...idealVb.value }, 200)
      userManualZoom.value = false
    }
  },
)

watch(
  () => vizFlags.enableDiffHighlight,
  (enabled) => {
    if (!enabled) {
      forceAddedKeys.value = new Set()
      removedKeys.value = []
      clearAddedKeysTimer()
      clearCommitTimer()
    }
  },
)

/**
 * showChangeCards 关闭时折叠全部卡片
 */
watch(
  () => vizFlags.showChangeCards,
  (enabled) => {
    if (!enabled) {
      handleCollapseAllCards()
    }
  },
)

/**
 * 卡片展开/收起时自动扩展 viewBox，使变更卡片不溢出画布
 */
watch(
  cardPositions,
  (positions) => {
    if (!vizFlags.enableAutoFit || userManualZoom.value) return

    if (positions.size === 0) {
      animateToVb({ ...idealVb.value }, 180)
      return
    }

    const cardH = changeCardDetailMode.value ? CARD_H_DETAIL : CARD_H_SIMPLE
    const base = contentBounds.value
    let minX = base.minX
    let minY = base.minY - headerTotalH.value
    let maxX = base.maxX
    let maxY = base.maxY

    for (const [, pos] of positions) {
      minX = Math.min(minX, pos.x)
      minY = Math.min(minY, pos.y)
      maxX = Math.max(maxX, pos.x + CARD_W_EST)
      maxY = Math.max(maxY, pos.y + cardH)
    }

    const width = Math.max(200, maxX - minX + PAD * 2)
    const height = Math.max(150, maxY - minY + PAD * 2)

    animateToVb(
      { x: minX - PAD, y: minY - PAD, w: width, h: height },
      180,
    )
  },
)

/* ================================================================
 *  阶段导航
 * ================================================================ */

const phasePopoverOpen = ref<boolean>(false)

const activePhaseIndex = ref<number | null>(props.currentPhaseIndex ?? null)

const effectivePhases = computed<PhaseSegment[]>(() => props.phases ?? [])

const showPhaseIndicator = computed<boolean>(() => {
  if (!vizFlags.showPhases) return false
  if (effectivePhases.value.length === 0) return false
  if (isStructureEmpty.value) return false
  return activePhaseIndex.value !== null
})

const currentPhaseName = computed<string>(() => {
  if (activePhaseIndex.value === null) return ''
  return effectivePhases.value[activePhaseIndex.value]?.name ?? ''
})

const canPrevPhase = computed<boolean>(() => {
  if (props.autoPlaying) return false
  if (activePhaseIndex.value === null) return false
  return activePhaseIndex.value > 0
})

const canNextPhase = computed<boolean>(() => {
  if (props.autoPlaying) return false
  if (activePhaseIndex.value === null) return false
  return activePhaseIndex.value < effectivePhases.value.length - 1
})

const handlePrevPhase = (): void => {
  if (!canPrevPhase.value || activePhaseIndex.value === null) return
  const prevIndex = activePhaseIndex.value - 1
  const target = effectivePhases.value[prevIndex]
  if (target) {
    activePhaseIndex.value = prevIndex
    emit('jump-to-frame', target.targetFrameIndex)
  }
}

const handleNextPhase = (): void => {
  if (!canNextPhase.value || activePhaseIndex.value === null) return
  const nextIndex = activePhaseIndex.value + 1
  const target = effectivePhases.value[nextIndex]
  if (target) {
    activePhaseIndex.value = nextIndex
    emit('jump-to-frame', target.targetFrameIndex)
  }
}

const handleJumpToPhase = (index: number): void => {
  if (props.autoPlaying) return
  const target = effectivePhases.value[index]
  if (target) {
    activePhaseIndex.value = index
    emit('jump-to-frame', target.targetFrameIndex)
    phasePopoverOpen.value = false
  }
}

const togglePhasePopover = (): void => {
  if (props.autoPlaying) return
  phasePopoverOpen.value = !phasePopoverOpen.value
}

watch(
  () => props.currentPhaseIndex,
  (newVal) => {
    activePhaseIndex.value = newVal ?? null
    phasePopoverOpen.value = false
  },
)

watch(
  () => props.autoPlaying,
  (playing) => {
    if (playing) phasePopoverOpen.value = false
  },
)

/* ================================================================
 *  生命周期
 * ================================================================ */

onBeforeUnmount(() => {
  clearCommitTimer()
  clearAddedKeysTimer()
  if (viewBoxAnimFrame !== null) {
    window.cancelAnimationFrame(viewBoxAnimFrame)
  }
})
</script>

<template>
  <section class="viz-panel" :class="{
    'viz-panel--smooth': smoothEnabled,
    'viz-panel--node-clickable': isNodeStructure,
  }">
    <header class="viz-panel__header">
      <div class="viz-panel__title">
        <MaterialIcon name="graph_3" :size="18" />
        <span>{{ kindLabel || '可视化' }}</span>
      </div>
      <div class="viz-panel__header-right">
        <div v-if="step && !isStructureEmpty" class="viz-panel__step">
          <span class="viz-panel__step-op">{{ step.op }}</span>
          <span v-if="step.line" class="viz-panel__step-line">L{{ step.line }}</span>
        </div>
      </div>
    </header>

    <!-- 元数据 -->
    <div v-if="hasMetadata" class="viz-panel__meta">
      <div class="viz-panel__meta-row">
        <span class="viz-panel__meta-tag">Object</span>
        <span v-if="label" class="viz-panel__meta-item viz-panel__meta-label">label={{ label }}</span>
        <span class="viz-panel__meta-item">kind={{ kindStr }}</span>
      </div>

      <div v-if="remarks?.title || remarks?.author || remarks?.comment" class="viz-panel__meta-row">
        <span class="viz-panel__meta-tag">Remarks</span>
        <span v-if="remarks?.title" class="viz-panel__meta-item">title={{ remarks.title }}</span>
        <span v-if="remarks?.author" class="viz-panel__meta-item">author={{ remarks.author }}</span>
        <span v-if="remarks?.comment" class="viz-panel__meta-item viz-panel__meta-comment">
          comment={{ remarks.comment }}
        </span>
      </div>

      <div v-if="meta?.createdAt" class="viz-panel__meta-row">
        <span class="viz-panel__meta-tag">IR</span>
        <span class="viz-panel__meta-item">created_at={{ meta.createdAt }}</span>
      </div>

      <div v-if="step" class="viz-panel__meta-row">
        <span class="viz-panel__meta-tag">Step</span>
        <span class="viz-panel__meta-item">op={{ step.op }}</span>
        <span v-if="step.line" class="viz-panel__meta-item">line={{ step.line }}</span>
        <span v-if="step.note" class="viz-panel__meta-item">note={{ step.note }}</span>
        <span v-for="entry in stepArgEntries" :key="`step-arg-${entry.key}`" class="viz-panel__meta-item">
          {{ entry.key }}={{ entry.value }}
        </span>
      </div>
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

          <!-- 标签 / 标题 / 副标题 -->
          <template v-if="vizFlags.showVizTitle">
            <text v-if="labelDisplayText.length > 0 && !isStructureEmpty" :x="effectiveTitleX" :y="effectiveLabelY"
              class="viz-label">
              {{ labelDisplayText }}
            </text>
            <text v-if="titleText.length > 0 && !isStructureEmpty" :x="effectiveTitleX" :y="effectiveTitleY"
              class="viz-title">
              {{ titleText }}
            </text>
            <text v-if="commentText.length > 0 && !isStructureEmpty" :x="effectiveTitleX" :y="effectiveCommentY"
              class="viz-comment">
              {{ commentText }}
            </text>
            <text v-if="authorText.length > 0 && !isStructureEmpty" :x="effectiveTitleX" :y="effectiveAuthorY"
              class="viz-author">
              {{ authorText }}
            </text>
          </template>

          <!-- 空结构图形 -->
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

          <!-- Stack -->
          <g v-if="stackLayout && stackLayout.items.length > 0">
            <template v-for="(item, index) in stackLayout.items" :key="`si-${index}`">
              <ellipse :cx="stackLayout.cx" :cy="item.y + stackLayout.cylItemH / 2" :rx="stackLayout.cylRx" :ry="CYL_RY"
                class="viz-cyl-sep" :class="{ 'viz-node--added': item.diff === 'added' }" />
              <rect :x="stackLayout.cx - stackLayout.cylRx" :y="item.y - stackLayout.cylItemH / 2"
                :width="stackLayout.cylRx * 2" :height="stackLayout.cylItemH" class="viz-cyl-body"
                :class="{ 'viz-cyl-body--top': item.isTop, 'viz-node--added': item.diff === 'added' }" />
              <line :x1="stackLayout.cx - stackLayout.cylRx" :y1="item.y - stackLayout.cylItemH / 2"
                :x2="stackLayout.cx - stackLayout.cylRx" :y2="item.y + stackLayout.cylItemH / 2" class="viz-cyl-side" />
              <line :x1="stackLayout.cx + stackLayout.cylRx" :y1="item.y - stackLayout.cylItemH / 2"
                :x2="stackLayout.cx + stackLayout.cylRx" :y2="item.y + stackLayout.cylItemH / 2" class="viz-cyl-side" />
              <ellipse :cx="stackLayout.cx" :cy="item.y - stackLayout.cylItemH / 2" :rx="stackLayout.cylRx" :ry="CYL_RY"
                class="viz-cyl-top" :class="{ 'viz-cyl-top--accent': item.isTop }" :style="getHlStyle(`s-${index}`)" />
              <text :x="stackLayout.cx" :y="item.displayLines.length === 1 ? item.y + 4 : item.y - LINE_H / 2 + 4"
                class="viz-val" @pointerenter="showTip(stackTipLines(index, item.value, item.isTop), $event)"
                @pointerleave="hideTip">
                <template v-if="item.displayLines.length === 1">
                  {{ item.displayLines[0] }}
                </template>
                <template v-else>
                  <tspan :x="stackLayout.cx" dy="0">{{ item.displayLines[0] }}</tspan>
                  <tspan :x="stackLayout.cx" :dy="LINE_H">{{ item.displayLines[1] }}</tspan>
                </template>
              </text>
              <template v-for="(labelItem, labelIndex) in item.labels" :key="`sl-${index}-${labelIndex}`">
                <text v-if="labelItem.position === 'top'" :x="stackLayout.cx + stackLayout.cylRx + 8" :y="item.y + 4"
                  class="viz-badge" :style="{ fill: labelItem.color }" text-anchor="start">
                  {{ labelItem.text }}
                </text>
              </template>
            </template>
          </g>

          <!-- Queue -->
          <g v-if="queueLayout && queueLayout.items.length > 0">
            <template v-for="(item, index) in queueLayout.items" :key="`qi-${index}`">
              <rect :x="item.x" :y="PAD + 20" :width="queueLayout.boxW" :height="queueLayout.boxH" rx="6"
                class="viz-box"
                :class="{ 'viz-box--accent': item.labels.length > 0, 'viz-node--added': item.diff === 'added' }"
                :style="getHlStyle(`q-${index}`)" />
              <text :x="item.x + queueLayout.boxW / 2" :y="item.displayLines.length === 1
                  ? PAD + 20 + queueLayout.boxH / 2 + 4
                  : PAD + 20 + queueLayout.boxH / 2 - LINE_H / 2 + 4
                " class="viz-val" @pointerenter="showTip(queueTipLines(index, item.value, item.labels), $event)"
                @pointerleave="hideTip">
                <template v-if="item.displayLines.length === 1">
                  {{ item.displayLines[0] }}
                </template>
                <template v-else>
                  <tspan :x="item.x + queueLayout.boxW / 2" dy="0">{{ item.displayLines[0] }}</tspan>
                  <tspan :x="item.x + queueLayout.boxW / 2" :dy="LINE_H">{{ item.displayLines[1] }}</tspan>
                </template>
              </text>
              <line v-if="index < queueLayout.items.length - 1" :x1="item.x + queueLayout.boxW + 4"
                :y1="PAD + 20 + queueLayout.boxH / 2" :x2="queueLayout.items[index + 1].x - 4"
                :y2="PAD + 20 + queueLayout.boxH / 2" class="viz-arrow" marker-end="url(#viz-arrow)" />
              <template v-for="(labelItem, labelIndex) in item.labels" :key="`ql-${index}-${labelIndex}`">
                <text v-if="labelItem.position === 'top'" :x="item.x + queueLayout.boxW / 2" :y="PAD + 12"
                  class="viz-badge" :style="{ fill: labelItem.color }">
                  {{ labelItem.text }}
                </text>
                <g v-else>
                  <line :x1="item.x + queueLayout.boxW / 2" :y1="PAD + 20 + queueLayout.boxH + 2"
                    :x2="item.x + queueLayout.boxW / 2" :y2="PAD + 20 + queueLayout.boxH + 18" class="viz-leader"
                    :style="{ stroke: labelItem.color }" />
                  <text :x="item.x + queueLayout.boxW / 2" :y="PAD + 20 + queueLayout.boxH + 28" class="viz-badge"
                    :style="{ fill: labelItem.color }">
                    {{ labelItem.text }}
                  </text>
                </g>
              </template>
            </template>
          </g>

          <!-- SList -->
          <g v-if="slistLayout && slistLayout.nodes.length > 0">
            <template v-for="(node, index) in slistLayout.nodes" :key="`sn-${node.id}`">
              <rect :x="node.x" :y="PAD + 20" :width="slistLayout.nodeW" :height="slistLayout.nodeH" rx="8"
                class="viz-node-rect" :class="{
                  'viz-node-rect--accent': node.labels.length > 0,
                  'viz-node--added': node.diff === 'added',
                }" :style="getHlStyle(`n-${node.id}`)" @click.stop="handleNodeCardClick(`n-${node.id}`, $event)" />
              <text :x="node.x + slistLayout.nodeW / 2" :y="node.displayLines.length === 1
                  ? PAD + 20 + slistLayout.nodeH / 2 - 4
                  : PAD + 20 + slistLayout.nodeH / 2 - LINE_H / 2 - 2
                " class="viz-val"
                @pointerenter="showTip(listTipLines(node.id, node.value, node.labels, slistOrdered[index]?.next), $event)"
                @pointerleave="hideTip">
                <template v-if="node.displayLines.length === 1">
                  {{ node.displayLines[0] }}
                </template>
                <template v-else>
                  <tspan :x="node.x + slistLayout.nodeW / 2" dy="0">{{ node.displayLines[0] }}</tspan>
                  <tspan :x="node.x + slistLayout.nodeW / 2" :dy="LINE_H">{{ node.displayLines[1] }}</tspan>
                </template>
              </text>
              <text :x="node.x + slistLayout.nodeW / 2" :y="PAD + 20 + slistLayout.nodeH / 2 + 14" class="viz-ptr">
                #{{ node.id }}
              </text>
              <template v-for="(labelItem, labelIndex) in node.labels" :key="`snl-${node.id}-${labelIndex}`">
                <text v-if="labelItem.position === 'top'" :x="node.x + slistLayout.nodeW / 2" :y="PAD + 12"
                  class="viz-badge" :style="{ fill: labelItem.color }">
                  {{ labelItem.text }}
                </text>
                <g v-else>
                  <line :x1="node.x + slistLayout.nodeW / 2" :y1="PAD + 20 + slistLayout.nodeH + 2"
                    :x2="node.x + slistLayout.nodeW / 2" :y2="PAD + 20 + slistLayout.nodeH + 18" class="viz-leader"
                    :style="{ stroke: labelItem.color }" />
                  <text :x="node.x + slistLayout.nodeW / 2" :y="PAD + 20 + slistLayout.nodeH + 28" class="viz-badge"
                    :style="{ fill: labelItem.color }">
                    {{ labelItem.text }}
                  </text>
                </g>
              </template>
              <line v-if="index < slistLayout.nodes.length - 1" :x1="node.x + slistLayout.nodeW + 2"
                :y1="PAD + 20 + slistLayout.nodeH / 2" :x2="slistLayout.nodes[index + 1].x - 2"
                :y2="PAD + 20 + slistLayout.nodeH / 2" class="viz-arrow" marker-end="url(#viz-arrow)" />
              <g v-if="node.isTail">
                <line :x1="node.x + slistLayout.nodeW + 2" :y1="PAD + 20 + slistLayout.nodeH / 2"
                  :x2="node.x + slistLayout.nodeW + 28" :y2="PAD + 20 + slistLayout.nodeH / 2" class="viz-arrow" />
                <text :x="node.x + slistLayout.nodeW + 32" :y="PAD + 20 + slistLayout.nodeH / 2 + 4" class="viz-null">
                  ⊘
                </text>
              </g>
            </template>
          </g>

          <!-- DList -->
          <g v-if="dlistLayout && dlistLayout.nodes.length > 0">
            <template v-for="(node, index) in dlistLayout.nodes" :key="`dn-${node.id}`">
              <rect :x="node.x" :y="PAD + 20" :width="dlistLayout.nodeW" :height="dlistLayout.nodeH" rx="8"
                class="viz-node-rect" :class="{
                  'viz-node-rect--accent': node.labels.length > 0,
                  'viz-node--added': node.diff === 'added',
                }" :style="getHlStyle(`n-${node.id}`)" @click.stop="handleNodeCardClick(`n-${node.id}`, $event)" />
              <text :x="node.x + dlistLayout.nodeW / 2" :y="node.displayLines.length === 1
                  ? PAD + 20 + dlistLayout.nodeH / 2 - 4
                  : PAD + 20 + dlistLayout.nodeH / 2 - LINE_H / 2 - 2
                " class="viz-val" @pointerenter="
                  showTip(
                    listTipLines(
                      node.id,
                      node.value,
                      node.labels,
                      dlistOrdered[index]?.next,
                      dlistOrdered[index]?.prev,
                    ),
                    $event,
                  )
                  " @pointerleave="hideTip">
                <template v-if="node.displayLines.length === 1">
                  {{ node.displayLines[0] }}
                </template>
                <template v-else>
                  <tspan :x="node.x + dlistLayout.nodeW / 2" dy="0">{{ node.displayLines[0] }}</tspan>
                  <tspan :x="node.x + dlistLayout.nodeW / 2" :dy="LINE_H">{{ node.displayLines[1] }}</tspan>
                </template>
              </text>
              <text :x="node.x + dlistLayout.nodeW / 2" :y="PAD + 20 + dlistLayout.nodeH / 2 + 14" class="viz-ptr">
                #{{ node.id }}
              </text>
              <template v-for="(labelItem, labelIndex) in node.labels" :key="`dnl-${node.id}-${labelIndex}`">
                <text v-if="labelItem.position === 'top'" :x="node.x + dlistLayout.nodeW / 2" :y="PAD + 12"
                  class="viz-badge" :style="{ fill: labelItem.color }">
                  {{ labelItem.text }}
                </text>
                <g v-else>
                  <line :x1="node.x + dlistLayout.nodeW / 2" :y1="PAD + 20 + dlistLayout.nodeH + 2"
                    :x2="node.x + dlistLayout.nodeW / 2" :y2="PAD + 20 + dlistLayout.nodeH + 18" class="viz-leader"
                    :style="{ stroke: labelItem.color }" />
                  <text :x="node.x + dlistLayout.nodeW / 2" :y="PAD + 20 + dlistLayout.nodeH + 28" class="viz-badge"
                    :style="{ fill: labelItem.color }">
                    {{ labelItem.text }}
                  </text>
                </g>
              </template>
              <g v-if="index < dlistLayout.nodes.length - 1">
                <line :x1="node.x + dlistLayout.nodeW + 2" :y1="PAD + 20 + dlistLayout.nodeH / 2 - 4"
                  :x2="dlistLayout.nodes[index + 1].x - 2" :y2="PAD + 20 + dlistLayout.nodeH / 2 - 4" class="viz-arrow"
                  marker-end="url(#viz-arrow)" />
                <line :x1="dlistLayout.nodes[index + 1].x - 2" :y1="PAD + 20 + dlistLayout.nodeH / 2 + 4"
                  :x2="node.x + dlistLayout.nodeW + 2" :y2="PAD + 20 + dlistLayout.nodeH / 2 + 4" class="viz-arrow"
                  marker-end="url(#viz-arrow)" />
              </g>
            </template>
          </g>

          <!-- Tree / BST -->
          <g v-if="treeLayout">
            <line v-for="(edge, index) in treeLayout.edges" :key="`te-${index}`" :x1="edge.x1" :y1="edge.y1"
              :x2="edge.x2" :y2="edge.y2" class="viz-edge" />
            <g v-for="node in treeLayout.nodes" :key="`tn-${node.id}`"
              @click.stop="handleNodeCardClick(`t-${node.id}`, $event)">
              <circle :cx="node.x" :cy="node.y" :r="treeLayout.treeR" class="viz-tree-node" :class="{
                'viz-tree-node--root': node.isRoot,
                'viz-node--added': node.diff === 'added',
              }" :style="getHlStyle(`t-${node.id}`)" />
              <text :x="node.x" :y="node.y + 5" class="viz-val" @pointerenter="showTip(treeTipLines(node), $event)"
                @pointerleave="hideTip">
                {{ node.displayLines[0] }}
              </text>
            </g>
          </g>

          <!-- Graph -->
          <g v-if="graphLayout">
            <line v-for="(edge, index) in graphLayout.edges" :key="`ge-${index}`" :x1="edge.x1" :y1="edge.y1"
              :x2="edge.x2" :y2="edge.y2" class="viz-edge"
              :marker-end="isDirected ? 'url(#viz-arrow-accent)' : undefined"
              :style="getEdgeHlStyle(edge.from, edge.to)" />
            <text v-for="(edge, index) in graphLayout.edges" :key="`gw-${index}`"
              v-show="isWeighted && edge.weight !== undefined" :x="edge.mx" :y="edge.my - 6" class="viz-weight">
              {{ edge.weight }}
            </text>
            <g v-for="node in graphLayout.nodes" :key="`gn-${node.id}`"
              @click.stop="handleNodeCardClick(`g-${node.id}`, $event)">
              <circle :cx="node.x" :cy="node.y" :r="graphLayout.graphR" class="viz-graph-node"
                :class="{ 'viz-node--added': node.diff === 'added' }"
                :style="getGraphNodeHlStyle(node.id, node.color)" />
              <text :x="node.x" :y="node.y + 5" class="viz-val" @pointerenter="showTip(graphTipLines(node), $event)"
                @pointerleave="hideTip">
                {{ node.displayLines[0] }}
              </text>
            </g>
          </g>

          <!-- 消逝节点虚线圆 -->
          <template v-if="isNodeStructure">
            <g v-for="[dKey, dPos] in departedNodePositions" :key="`departed-${dKey}`"
              @click.stop="handleDepartedSelect(dKey)">
              <!-- 展开态红色高亮背景 -->
              <circle v-if="departedCards.has(dKey)" :cx="dPos.x" :cy="dPos.y" :r="dPos.r + 3"
                fill="rgba(239,68,68,0.10)" stroke="none" />
              <!-- 虚线圆 -->
              <circle :cx="dPos.x" :cy="dPos.y" :r="dPos.r"
                :class="departedCards.has(dKey) ? 'viz-departed-ghost--active' : 'viz-departed-ghost'" />
              <!-- 节点最后已知值 -->
              <text :x="dPos.x" :y="dPos.y + 4" class="viz-departed-ghost__label">
                {{ formatDepartedLabel(dKey) }}
              </text>
            </g>
          </template>

          <!-- 变更卡片 -->
          <template v-if="isNodeStructure">
            <ChangeCard v-for="[cKey, tl] in activeTimelines" :key="`cc-${cKey}`" :timeline="tl"
              :current-frame-index="effectiveFrameIndex" :card-x="cardPositions.get(cKey)?.x ?? 0"
              :card-y="cardPositions.get(cKey)?.y ?? 0" :node-x="getNodePos(cKey).x" :node-y="getNodePos(cKey).y"
              :node-radius="getNodePos(cKey).r" :show-all-frames="showAllFramesMap.get(cKey) ?? false"
              :detail-mode="changeCardDetailMode" :departed="departedCards.has(cKey)"
              @jump-to-frame="handleCardJumpToFrame" @toggle-all-frames="toggleShowAllFrames(cKey)"
              @toggle-detail="changeCardDetailMode = !changeCardDetailMode" @close="handleDepartedCardClose(cKey)" />
          </template>

          <!-- Ghost（移除渐出） -->
          <template v-for="ghost in removedKeys" :key="`ghost-${ghost.key}`">
            <circle v-if="ghost.shape === 'circle'" :cx="ghost.x" :cy="ghost.y" :r="ghost.r"
              class="viz-ghost viz-ghost--circle" />
            <ellipse v-else-if="ghost.shape === 'ellipse'" :cx="ghost.x" :cy="ghost.y" :rx="ghost.rx" :ry="ghost.ry"
              class="viz-ghost viz-ghost--circle" />
            <rect v-else :x="ghost.x" :y="ghost.y" :width="ghost.w ?? 72" :height="ghost.h ?? 44" rx="6"
              class="viz-ghost" />
          </template>
        </svg>
      </Transition>

      <Transition name="fade">
        <div v-if="tip.show" class="viz-tooltip" :style="tipStyle">
          <div v-for="(line, index) in tip.lines" :key="index" class="viz-tooltip__row">
            <span class="viz-tooltip__label">{{ line.label }}</span>
            <span class="viz-tooltip__value">{{ line.value }}</span>
          </div>
        </div>
      </Transition>

      <Transition name="fade">
        <div v-if="showZoomHint" class="viz-panel__zoom-hint">
          双击画布还原缩放定位
        </div>
      </Transition>

      <VizSettings />

      <!-- 消逝节点浮层 -->
      <DepartedNodes v-if="showDepartedButton" :lifespans="departedLifespans" @select="handleDepartedSelect" />

      <!-- 变更卡片控制按钮 -->
      <div v-if="isNodeStructure && !isEmpty && !isStructureEmpty" class="change-card-controls">
        <button class="change-card-controls__toggle"
          @click="hasAnyCardsExpanded ? handleCollapseAllCards() : handleExpandAllCards()">
          <svg width="14" height="14" viewBox="0 0 16 16" fill="none" stroke="currentColor" stroke-width="1.3"
            stroke-linecap="round" stroke-linejoin="round" aria-hidden="true">
            <template v-if="hasAnyCardsExpanded">
              <rect x="2" y="4" width="12" height="3.5" rx="1.5" />
              <rect x="2" y="8.5" width="12" height="3.5" rx="1.5" />
              <polyline points="6.5,2.5 8,4 9.5,2.5" />
              <polyline points="6.5,13.5 8,12 9.5,13.5" />
            </template>
            <template v-else>
              <rect x="2" y="1" width="12" height="5" rx="1.5" />
              <rect x="2" y="10" width="12" height="5" rx="1.5" />
              <polyline points="6.5,8.5 8,7 9.5,8.5" />
            </template>
          </svg>
          <span>{{ hasAnyCardsExpanded ? '折叠' : '展开' }}</span>
        </button>
      </div>
      <!-- Phase indicator -->
      <Transition name="fade">
        <div v-if="showPhaseIndicator" class="viz-phase-bar">
          <button class="viz-phase-bar__nav" :disabled="!canPrevPhase" aria-label="上一阶段" @click="handlePrevPhase">
            <MaterialIcon name="chevron_left" :size="16" />
          </button>
          <div class="viz-phase-bar__center">
            <button class="viz-phase-bar__badge" :class="{ 'viz-phase-bar__badge--readonly': autoPlaying }"
              @click="togglePhasePopover">
              <span class="viz-phase-bar__name" :title="currentPhaseName">{{ currentPhaseName }}</span>
              <MaterialIcon v-if="!autoPlaying" name="expand_more" :size="14" class="viz-phase-bar__chevron"
                :class="{ 'viz-phase-bar__chevron--open': phasePopoverOpen }" />
              <MaterialIcon v-else name="play_arrow" :size="14" class="viz-phase-bar__playing-icon" />
            </button>
            <Transition name="fade">
              <div v-if="phasePopoverOpen" class="viz-phase-popover">
                <div v-for="(phase, idx) in effectivePhases" :key="idx" class="viz-phase-popover__item"
                  :class="{ 'viz-phase-popover__item--active': idx === activePhaseIndex }"
                  @click="handleJumpToPhase(idx)">
                  <span class="viz-phase-popover__name" :title="phase.name">{{ phase.name }}</span>
                  <span class="viz-phase-popover__range">
                    步骤 {{ phase.targetFrameIndex }}–{{ phase.endFrameIndex }}
                  </span>
                </div>
              </div>
            </Transition>
          </div>
          <button class="viz-phase-bar__nav" :disabled="!canNextPhase" aria-label="下一阶段" @click="handleNextPhase">
            <MaterialIcon name="chevron_right" :size="16" />
          </button>
        </div>
      </Transition>
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

/* metadata */
.viz-panel__meta {
  display: flex;
  flex-direction: column;
  gap: 4px;
  padding: 6px var(--space-2);
  border-bottom: 1px solid var(--color-border);
  background-color: var(--color-bg-surface-alt);
  font-size: 11px;
  color: var(--color-text-tertiary);
  position: relative;
  z-index: 1;
}

.viz-panel__meta-row {
  display: flex;
  flex-wrap: wrap;
  gap: 4px 10px;
  align-items: center;
}

.viz-panel__meta-tag {
  padding: 1px 6px;
  border: 1px solid var(--color-border);
  border-radius: var(--radius-full);
  font-size: 10px;
  color: var(--color-text-tertiary);
  letter-spacing: 0.04em;
  text-transform: uppercase;
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
  overflow: visible;
  z-index: 2;
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

.viz-title {
  font-size: 12.6px;
  font-weight: 600;
  fill: var(--color-text-primary);
  text-anchor: middle;
  user-select: none;
  pointer-events: none;
}

.viz-label {
  font-size: 10px;
  fill: var(--color-accent);
  text-anchor: middle;
  font-weight: 500;
  user-select: none;
  pointer-events: none;
  opacity: 0.85;
}

.viz-comment {
  font-size: 5.5px;
  fill: var(--color-text-tertiary);
  text-anchor: middle;
  user-select: none;
  pointer-events: none;
  opacity: 0.72;
}

.viz-author {
  font-size: 5.5px;
  font-style: italic;
  fill: var(--color-text-tertiary);
  text-anchor: middle;
  user-select: none;
  pointer-events: none;
  opacity: 0.72;
}

/* svg common */
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

/* ---- 节点可点击态（仅变更卡片可用的结构类型） ---- */

.viz-panel--node-clickable .viz-tree-node,
.viz-panel--node-clickable .viz-graph-node,
.viz-panel--node-clickable .viz-node-rect {
  cursor: pointer;
}

/* 新增渐入 */
.viz-node--added {
  transform-box: fill-box;
  transform-origin: center;
  animation: viz-pop var(--ghost-ms, 220ms) var(--ease);
}

@keyframes viz-pop {
  from {
    opacity: 0;
    transform: scale(0.88);
  }

  to {
    opacity: 1;
    transform: scale(1);
  }
}

/* 移除 ghost（红虚线） */
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
    opacity: 0.45;
    transform: scale(1);
  }

  to {
    opacity: 0;
    transform: scale(0.86);
  }
}

/* ---- 消逝节点虚线圆 ---- */

.viz-departed-ghost {
  fill: none;
  stroke: var(--color-error);
  stroke-width: 1;
  stroke-dasharray: 5 3;
  opacity: 0.4;
  cursor: pointer;
  transition: opacity var(--duration-fast) var(--ease);
}

.viz-departed-ghost:hover {
  opacity: 0.7;
}

.viz-departed-ghost--active {
  fill: rgba(239, 68, 68, 0.06);
  stroke: var(--color-error);
  stroke-width: 1.5;
  stroke-dasharray: 5 3;
  opacity: 0.7;
  cursor: pointer;
  transition: opacity var(--duration-fast) var(--ease);
}

.viz-departed-ghost--active:hover {
  opacity: 0.9;
}

.viz-departed-ghost__label {
  font-size: 10px;
  font-family: var(--font-mono);
  fill: var(--color-error);
  text-anchor: middle;
  pointer-events: none;
  opacity: 0.6;
}

.change-card-controls {
  position: absolute;
  top: 8px;
  right: 8px;
  z-index: 6;
  display: flex;
}

.change-card-controls__toggle {
  display: inline-flex;
  align-items: center;
  gap: 4px;
  height: 28px;
  padding: 0 12px;
  border: 1px solid var(--color-border-strong);
  border-radius: var(--radius-full);
  background-color: var(--color-bg-surface);
  color: var(--color-text-body);
  font-size: var(--text-xs);
  font-weight: var(--weight-medium);
  font-family: inherit;
  cursor: pointer;
  box-shadow: var(--shadow-static);
  white-space: nowrap;
  transition:
    background-color var(--duration-fast) var(--ease),
    color var(--duration-fast) var(--ease),
    box-shadow var(--duration-fast) var(--ease);
}

.change-card-controls__toggle:hover {
  background-color: var(--color-bg-hover);
  color: var(--color-text-primary);
  box-shadow: var(--shadow-hover);
}

.change-card-controls__toggle svg {
  width: 14px;
  height: 14px;
  flex-shrink: 0;
}

/* 平滑位移 */
.viz-panel--smooth .viz-tree-node,
.viz-panel--smooth .viz-graph-node {
  transition:
    cx var(--duration-viz) var(--ease),
    cy var(--duration-viz) var(--ease),
    x var(--duration-viz) var(--ease),
    y var(--duration-viz) var(--ease),
    r var(--duration-viz) var(--ease),
    fill var(--duration-fast) var(--ease),
    stroke var(--duration-fast) var(--ease),
    stroke-width var(--duration-fast) var(--ease);
}

.viz-panel--smooth .viz-edge,
.viz-panel--smooth .viz-arrow {
  transition:
    x1 var(--duration-viz) var(--ease),
    y1 var(--duration-viz) var(--ease),
    x2 var(--duration-viz) var(--ease),
    y2 var(--duration-viz) var(--ease),
    stroke var(--duration-fast) var(--ease),
    stroke-width var(--duration-fast) var(--ease);
}

.viz-panel--smooth .viz-val,
.viz-panel--smooth .viz-ptr,
.viz-panel--smooth .viz-badge {
  transition:
    x var(--duration-viz) var(--ease),
    y var(--duration-viz) var(--ease);
}

/* 高亮过渡（始终生效） */
.viz-cyl-top,
.viz-cyl-body,
.viz-box,
.viz-node-rect,
.viz-tree-node,
.viz-graph-node,
.viz-edge {
  transition:
    fill var(--duration-fast) var(--ease),
    stroke var(--duration-fast) var(--ease),
    stroke-width var(--duration-fast) var(--ease);
}

.viz-panel--smooth .viz-cyl-body,
.viz-panel--smooth .viz-cyl-top,
.viz-panel--smooth .viz-box,
.viz-panel--smooth .viz-node-rect {
  transition:
    fill var(--duration-fast) var(--ease),
    stroke var(--duration-fast) var(--ease),
    stroke-width var(--duration-fast) var(--ease);
}

.viz-panel--smooth .viz-node--added {
  transition: none;
}

/* ---- Phase indicator ---- */

.viz-phase-bar {
  position: absolute;
  bottom: 48px;
  left: 50%;
  transform: translateX(-50%);
  display: inline-flex;
  align-items: center;
  gap: 4px;
  z-index: 6;
  user-select: none;
}

.viz-phase-bar__nav {
  display: flex;
  align-items: center;
  justify-content: center;
  width: 26px;
  height: 26px;
  border: 1px solid var(--color-border-strong);
  border-radius: var(--radius-control);
  background-color: var(--color-bg-surface);
  color: var(--color-text-body);
  cursor: pointer;
  box-shadow: var(--shadow-static);
  transition:
    background-color var(--duration-fast) var(--ease),
    color var(--duration-fast) var(--ease);
}

.viz-phase-bar__nav:hover:not(:disabled) {
  background-color: var(--color-bg-hover);
  color: var(--color-text-primary);
}

.viz-phase-bar__nav:disabled {
  opacity: 0.35;
  cursor: not-allowed;
}

.viz-phase-bar__nav :deep(.material-icon) {
  width: 16px;
  height: 16px;
}

.viz-phase-bar__center {
  position: relative;
}

.viz-phase-bar__badge {
  display: inline-flex;
  align-items: center;
  gap: 4px;
  height: 28px;
  padding: 0 12px;
  border: 1px solid var(--color-border-strong);
  border-radius: var(--radius-full);
  background-color: var(--color-bg-surface);
  color: var(--color-text-primary);
  font-size: var(--text-xs);
  font-weight: var(--weight-semibold);
  font-family: inherit;
  cursor: pointer;
  box-shadow: var(--shadow-static);
  white-space: nowrap;
  transition:
    background-color var(--duration-fast) var(--ease),
    border-color var(--duration-fast) var(--ease),
    box-shadow var(--duration-fast) var(--ease);
}

.viz-phase-bar__badge:hover {
  background-color: var(--color-bg-hover);
  box-shadow: var(--shadow-hover);
}

.viz-phase-bar__badge--readonly {
  cursor: default;
  opacity: 0.6;
}

.viz-phase-bar__badge--readonly:hover {
  background-color: var(--color-bg-surface);
  box-shadow: var(--shadow-static);
}

.viz-phase-bar__name {
  max-width: 160px;
  overflow: hidden;
  text-overflow: ellipsis;
}

.viz-phase-bar__chevron {
  width: 14px;
  height: 14px;
  transition: transform var(--duration-fast) var(--ease);
}

.viz-phase-bar__chevron--open {
  transform: rotate(180deg);
}

.viz-phase-bar__playing-icon {
  width: 14px;
  height: 14px;
  opacity: 0.5;
}

.viz-phase-popover {
  position: absolute;
  bottom: calc(100% + 6px);
  left: 50%;
  transform: translateX(-50%);
  min-width: 180px;
  max-width: 280px;
  max-height: 240px;
  overflow-y: auto;
  background-color: var(--color-bg-surface);
  border: 1px solid var(--color-border);
  border-radius: var(--radius-lg);
  box-shadow: var(--shadow-hover);
  padding: 4px 0;
  z-index: 7;
}

.viz-phase-popover__item {
  display: flex;
  align-items: center;
  justify-content: space-between;
  gap: var(--space-1);
  padding: 7px 14px;
  cursor: pointer;
  transition: background-color var(--duration-fast) var(--ease);
}

.viz-phase-popover__item:hover {
  background-color: var(--color-bg-hover);
}

.viz-phase-popover__item--active {
  background-color: var(--color-accent-wash);
}

.viz-phase-popover__name {
  font-size: var(--text-xs);
  font-weight: var(--weight-medium);
  color: var(--color-text-primary);
  white-space: nowrap;
  overflow: hidden;
  text-overflow: ellipsis;
  min-width: 0;
  flex: 1;
}

.viz-phase-popover__item--active .viz-phase-popover__name {
  color: var(--color-accent);
}

.viz-phase-popover__range {
  font-size: 11px;
  color: var(--color-text-tertiary);
  white-space: nowrap;
  flex-shrink: 0;
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