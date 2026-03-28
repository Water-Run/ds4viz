/**
 * 节点变更卡片类型定义
 *
 * @file src/types/node-change.ts
 * @author WaterRun
 * @date 2026-03-28
 */

import type { IrValue } from './ir'

/**
 * 节点变更类型
 *
 * @typedef
 */
export type NodeChangeType =
    | 'created'
    | 'deleted'
    | 'value_changed'
    | 'structural'
    | 'highlighted'
    | 'alias_changed'
    | 'weight_changed'

/**
 * 变更类型显示优先级（数值越小优先级越高）
 */
export const CHANGE_TYPE_PRIORITY: Record<NodeChangeType, number> = {
    created: 0,
    deleted: 1,
    value_changed: 2,
    structural: 3,
    alias_changed: 4,
    weight_changed: 5,
    highlighted: 6,
}

/**
 * 变更类型符号
 */
export const CHANGE_TYPE_SYMBOL: Record<NodeChangeType, string> = {
    created: '+',
    deleted: '✕',
    value_changed: '→',
    structural: '↔',
    highlighted: '·',
    alias_changed: '≡',
    weight_changed: '⚖',
}

/**
 * 结构关系变化
 *
 * @interface
 */
export interface StructuralChange {
    /** 字段名（next / prev / left / right / edge_added / edge_removed / weight） */
    field: string
    /** 变化前值 */
    before: string
    /** 变化后值 */
    after: string
}

/**
 * 变更详情
 *
 * @interface
 */
export interface ChangeDetail {
    /** 值变化前 */
    valueBefore?: IrValue
    /** 值变化后 */
    valueAfter?: IrValue
    /** 结构变化列表 */
    structuralChanges?: StructuralChange[]
    /** alias 变化前 */
    aliasBefore?: string | null
    /** alias 变化后 */
    aliasAfter?: string | null
}

/**
 * 节点变更条目（一帧内同一节点的所有变更合并为一条）
 *
 * @interface
 */
export interface NodeChangeEntry {
    /** 帧索引 */
    frameIndex: number
    /** 步骤 ID（初始帧为 null） */
    stepId: number | null
    /** 操作名称 */
    op: string
    /** 变更类型列表（按优先级排序） */
    types: NodeChangeType[]
    /** 主类型（types[0]） */
    primaryType: NodeChangeType
    /** 详细信息 */
    detail: ChangeDetail
    /** 步骤备注 */
    note?: string
    /** 高亮信息 */
    highlight?: { style: string; level: number }
}

/**
 * 节点生命周期概要（轻量，用于消逝节点列表）
 *
 * @interface
 */
export interface NodeLifespan {
    /** 节点标识键（n-3 / t-0 / g-5） */
    key: string
    /** 节点 ID */
    nodeId: number
    /** 显示名称 */
    displayName: string
    /** 最后已知值 */
    lastValue?: IrValue
    /** 最后已知标签（graph） */
    lastLabel?: string
    /** 最后已知 alias */
    lastAlias?: string | null
    /** 首次出现帧 */
    createdAtFrame: number
    /** 消失帧（null = 当前仍存在） */
    deletedAtFrame: number | null
}

/**
 * 节点变更完整时间线
 *
 * @interface
 */
export interface NodeChangeTimeline extends NodeLifespan {
    /** 全部变更条目 */
    entries: NodeChangeEntry[]
}
