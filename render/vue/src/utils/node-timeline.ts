/**
 * 节点变更时间线构建
 *
 * 扫描 IrDocument 的全部 states 与 frames，为每个节点构建变更历史。
 * 支持 slist / dlist / binary_tree / bst / graph_* 六类含 nodes 的结构。
 *
 * @file src/utils/node-timeline.ts
 * @author WaterRun
 * @date 2026-03-28
 */

import type {
    IrDocument,
    IrStateData,
    IrObjectKind,
    IrValue,
    IrHighlight,
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
    GraphWeightedEdge,
} from '@/types/ir'
import type { Frame } from '@/types/viz'
import type {
    NodeChangeType,
    NodeChangeEntry,
    NodeLifespan,
    NodeChangeTimeline,
    ChangeDetail,
    StructuralChange,
} from '@/types/node-change'
import { CHANGE_TYPE_PRIORITY } from '@/types/node-change'

/** 支持变更卡片的结构类型集合 */
const NODE_KINDS = new Set<string>([
    'slist', 'dlist', 'binary_tree', 'bst',
    'graph_undirected', 'graph_directed', 'graph_weighted',
])

/**
 * 判断结构类型是否支持节点变更卡片
 *
 * @param kind - 结构类型
 * @returns 是否为含 nodes 的结构
 */
export function isNodeBasedKind(kind: string): boolean {
    return NODE_KINDS.has(kind)
}

/**
 * 获取节点标识键前缀
 *
 * @param kind - 结构类型
 * @returns 前缀字符
 */
function getKeyPrefix(kind: string): string {
    if (kind === 'slist' || kind === 'dlist') return 'n'
    if (kind === 'binary_tree' || kind === 'bst') return 't'
    return 'g'
}

/**
 * 通用节点快照（跨结构类型归一化）
 *
 * @interface
 */
interface NormalizedNode {
    /** 节点 ID */
    id: number
    /** 值（graph 用 label） */
    value: IrValue | null
    /** 别名 */
    alias: string | null
    /** 结构指针字段 */
    pointers: Record<string, number>
    /** 关联边（仅 graph） */
    edges: Array<{ from: number; to: number; weight?: number }>
}

/**
 * 从状态数据中提取并归一化节点列表
 *
 * @param data - 状态数据
 * @param kind - 结构类型
 * @returns 归一化节点映射（nodeId → NormalizedNode）
 */
function extractNodes(data: IrStateData, kind: string): Map<number, NormalizedNode> {
    const map = new Map<number, NormalizedNode>()

    if (kind === 'slist') {
        const d = data as SListStateData
        for (const n of d.nodes) {
            map.set(n.id, {
                id: n.id,
                value: n.value,
                alias: n.alias ?? null,
                pointers: { next: n.next },
                edges: [],
            })
        }
    } else if (kind === 'dlist') {
        const d = data as DListStateData
        for (const n of d.nodes) {
            map.set(n.id, {
                id: n.id,
                value: n.value,
                alias: n.alias ?? null,
                pointers: { prev: n.prev, next: n.next },
                edges: [],
            })
        }
    } else if (kind === 'binary_tree' || kind === 'bst') {
        const d = data as BinaryTreeStateData
        for (const n of d.nodes) {
            map.set(n.id, {
                id: n.id,
                value: n.value,
                alias: n.alias ?? null,
                pointers: { left: n.left, right: n.right },
                edges: [],
            })
        }
    } else if (kind.startsWith('graph')) {
        const d = data as GraphStateData | GraphWeightedStateData
        for (const n of d.nodes) {
            const nodeEdges = d.edges.filter(
                (e) => e.from === n.id || e.to === n.id,
            )
            map.set(n.id, {
                id: n.id,
                value: (n as GraphNode).label as IrValue,
                alias: n.alias ?? null,
                pointers: {},
                edges: nodeEdges.map((e) => ({
                    from: e.from,
                    to: e.to,
                    weight: (e as GraphWeightedEdge).weight,
                })),
            })
        }
    }

    return map
}

/**
 * 比较两个归一化节点，生成变更类型与详情
 *
 * @param prev - 前一帧节点
 * @param curr - 当前帧节点
 * @param kind - 结构类型
 * @returns 变更类型列表与详情
 */
function diffNode(
    prev: NormalizedNode,
    curr: NormalizedNode,
    kind: string,
): { types: NodeChangeType[]; detail: ChangeDetail } {
    const types: NodeChangeType[] = []
    const detail: ChangeDetail = {}

    if (String(prev.value) !== String(curr.value)) {
        types.push(kind.startsWith('graph') ? 'value_changed' : 'value_changed')
        detail.valueBefore = prev.value ?? undefined
        detail.valueAfter = curr.value ?? undefined
    }

    if ((prev.alias ?? '') !== (curr.alias ?? '')) {
        types.push('alias_changed')
        detail.aliasBefore = prev.alias
        detail.aliasAfter = curr.alias
    }

    const structChanges: StructuralChange[] = []

    for (const field of Object.keys(curr.pointers)) {
        const oldVal = prev.pointers[field] ?? -1
        const newVal = curr.pointers[field] ?? -1
        if (oldVal !== newVal) {
            structChanges.push({
                field,
                before: oldVal === -1 ? 'nil' : `#${oldVal}`,
                after: newVal === -1 ? 'nil' : `#${newVal}`,
            })
        }
    }

    const prevEdgeKeys = new Set(prev.edges.map((e) => `${e.from}-${e.to}`))
    const currEdgeKeys = new Set(curr.edges.map((e) => `${e.from}-${e.to}`))

    for (const e of curr.edges) {
        const k = `${e.from}-${e.to}`
        if (!prevEdgeKeys.has(k)) {
            structChanges.push({ field: 'edge+', before: '', after: `(${e.from},${e.to})` })
        }
    }

    for (const e of prev.edges) {
        const k = `${e.from}-${e.to}`
        if (!currEdgeKeys.has(k)) {
            structChanges.push({ field: 'edge-', before: `(${e.from},${e.to})`, after: '' })
        }
    }

    if (kind === 'graph_weighted') {
        const prevWeights = new Map(prev.edges.map((e) => [`${e.from}-${e.to}`, e.weight]))
        for (const e of curr.edges) {
            const k = `${e.from}-${e.to}`
            if (prevEdgeKeys.has(k) && prevWeights.get(k) !== e.weight) {
                types.push('weight_changed')
                structChanges.push({
                    field: 'weight',
                    before: `(${e.from},${e.to})=${prevWeights.get(k) ?? '?'}`,
                    after: `(${e.from},${e.to})=${e.weight ?? '?'}`,
                })
            }
        }
    }

    if (structChanges.length > 0) {
        if (!types.includes('structural')) {
            types.push('structural')
        }
        detail.structuralChanges = structChanges
    }

    types.sort((a, b) => CHANGE_TYPE_PRIORITY[a] - CHANGE_TYPE_PRIORITY[b])

    return { types, detail }
}

/**
 * 从步骤摘要中提取指定节点的高亮信息
 *
 * @param highlights - 高亮列表
 * @param nodeId - 节点 ID
 * @param kind - 结构类型
 * @returns 高亮信息，无则返回 undefined
 */
function findHighlight(
    highlights: IrHighlight[] | undefined,
    nodeId: number,
    kind: string,
): { style: string; level: number } | undefined {
    if (!highlights || highlights.length === 0) return undefined

    for (const h of highlights) {
        if (h.kind === 'node' && h.target === nodeId) {
            return { style: h.style, level: h.level ?? 1 }
        }
    }
    return undefined
}

/**
 * 构建显示名称
 *
 * @param nodeId - 节点 ID
 * @param alias - 别名
 * @param value - 值
 * @param kind - 结构类型
 * @returns 显示名称
 */
function buildDisplayName(
    nodeId: number,
    alias: string | null,
    value: IrValue | null,
    kind: string,
): string {
    const idStr = `#${nodeId}`
    if (kind.startsWith('graph')) {
        const label = value !== null ? String(value) : ''
        const parts = [label, `(${idStr})`]
        if (alias) parts.push(alias)
        return parts.join(' ')
    }
    const parts = [idStr]
    if (alias) parts.push(alias)
    return parts.join(' ')
}

/**
 * 构建所有节点的生命周期概要（轻量计算）
 *
 * @param doc - IR 文档
 * @param frames - 帧列表
 * @returns 节点生命周期映射
 */
export function buildLifespans(
    doc: IrDocument,
    frames: Frame[],
): Map<string, NodeLifespan> {
    const kind = doc.object.kind
    if (!isNodeBasedKind(kind)) return new Map()

    const prefix = getKeyPrefix(kind)
    const result = new Map<string, NodeLifespan>()

    const allIds = new Set<number>()
    for (const state of doc.states) {
        const nodes = extractNodes(state.data, kind)
        for (const id of nodes.keys()) {
            allIds.add(id)
        }
    }

    for (const nodeId of allIds) {
        const key = `${prefix}-${nodeId}`
        let createdAtFrame = -1
        let deletedAtFrame: number | null = null
        let lastValue: IrValue | undefined
        let lastAlias: string | null = null
        let wasAlive = false

        for (const frame of frames) {
            const state = doc.states[frame.stateIndex]
            if (!state) continue

            const nodes = extractNodes(state.data, kind)
            const node = nodes.get(nodeId)

            if (node) {
                if (!wasAlive) {
                    createdAtFrame = frame.index
                    wasAlive = true
                }
                lastValue = node.value ?? undefined
                lastAlias = node.alias
                deletedAtFrame = null
            } else if (wasAlive && deletedAtFrame === null) {
                deletedAtFrame = frame.index
            }
        }

        if (createdAtFrame >= 0) {
            result.set(key, {
                key,
                nodeId,
                displayName: buildDisplayName(nodeId, lastAlias, lastValue ?? null, kind),
                lastValue,
                lastLabel: kind.startsWith('graph') ? String(lastValue ?? '') : undefined,
                lastAlias,
                createdAtFrame,
                deletedAtFrame,
            })
        }
    }

    return result
}

/**
 * 构建指定节点的完整变更时间线
 *
 * @param doc - IR 文档
 * @param frames - 帧列表
 * @param targetKey - 节点标识键（如 "t-3"）
 * @returns 完整时间线
 */
export function buildTimeline(
    doc: IrDocument,
    frames: Frame[],
    targetKey: string,
): NodeChangeTimeline {
    const kind = doc.object.kind
    const prefix = getKeyPrefix(kind)
    const nodeId = Number(targetKey.slice(prefix.length + 1))

    const entries: NodeChangeEntry[] = []
    let createdAtFrame = -1
    let deletedAtFrame: number | null = null
    let lastValue: IrValue | undefined
    let lastAlias: string | null = null
    let prevNode: NormalizedNode | null = null

    for (const frame of frames) {
        const state = doc.states[frame.stateIndex]
        if (!state) continue

        const nodes = extractNodes(state.data, kind)
        const currNode = nodes.get(nodeId) ?? null
        const step = frame.summary
        const hl = findHighlight(step?.highlights, nodeId, kind)

        if (currNode && !prevNode) {
            createdAtFrame = frame.index
            const entry: NodeChangeEntry = {
                frameIndex: frame.index,
                stepId: step ? frame.index - 1 : null,
                op: step?.op ?? 'init',
                types: ['created'],
                primaryType: 'created',
                detail: { valueAfter: currNode.value ?? undefined },
                note: step?.note,
                highlight: hl,
            }
            if (hl && !entry.types.includes('highlighted')) {
                entry.types.push('highlighted')
            }
            entries.push(entry)
            lastValue = currNode.value ?? undefined
            lastAlias = currNode.alias
        } else if (!currNode && prevNode) {
            deletedAtFrame = frame.index
            const entry: NodeChangeEntry = {
                frameIndex: frame.index,
                stepId: step ? frame.index - 1 : null,
                op: step?.op ?? 'delete',
                types: ['deleted'],
                primaryType: 'deleted',
                detail: { valueBefore: prevNode.value ?? undefined },
                note: step?.note,
                highlight: hl,
            }
            entries.push(entry)
        } else if (currNode && prevNode) {
            const diff = diffNode(prevNode, currNode, kind)
            const hasHighlight = hl !== undefined

            if (diff.types.length > 0 || hasHighlight) {
                const types = [...diff.types]
                if (hasHighlight && !types.includes('highlighted')) {
                    types.push('highlighted')
                }
                if (types.length === 0) {
                    types.push('highlighted')
                }
                types.sort((a, b) => CHANGE_TYPE_PRIORITY[a] - CHANGE_TYPE_PRIORITY[b])

                entries.push({
                    frameIndex: frame.index,
                    stepId: step ? frame.index - 1 : null,
                    op: step?.op ?? 'observe',
                    types,
                    primaryType: types[0],
                    detail: diff.detail,
                    note: step?.note,
                    highlight: hl,
                })
            }

            lastValue = currNode.value ?? undefined
            lastAlias = currNode.alias
        }

        prevNode = currNode
    }

    return {
        key: targetKey,
        nodeId,
        displayName: buildDisplayName(nodeId, lastAlias, lastValue ?? null, kind),
        lastValue,
        lastLabel: kind.startsWith('graph') ? String(lastValue ?? '') : undefined,
        lastAlias,
        createdAtFrame,
        deletedAtFrame,
        entries,
    }
}

/**
 * 获取当前帧已消逝的节点键列表
 *
 * @param lifespans - 生命周期映射
 * @param currentFrameIndex - 当前帧索引
 * @returns 已消逝节点键数组
 */
export function getDepartedKeys(
    lifespans: Map<string, NodeLifespan>,
    currentFrameIndex: number,
): string[] {
    const result: string[] = []
    for (const [key, span] of lifespans) {
        if (
            span.deletedAtFrame !== null
            && span.deletedAtFrame <= currentFrameIndex
        ) {
            result.push(key)
        }
    }
    return result
}
