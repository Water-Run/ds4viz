/**
 * IR 数据结构定义（TOML 解析后）
 *
 * @file src/types/ir.ts
 * @author WaterRun
 * @date 2026-03-25
 */

import type { Language } from './api'

/**
 * IR 顶层结构
 *
 * @interface
 */
export interface IrDocument {
  meta: IrMeta
  package: IrPackage
  remarks?: IrRemarks
  object: IrObject
  states: IrState[]
  steps?: IrStep[]
  result?: IrResult
  error?: IrError
}

/**
 * 元信息
 *
 * @interface
 */
export interface IrMeta {
  /** RFC 3339 时间戳 */
  createdAt: string
  /** 生成 trace 的语言 */
  lang: Language | string
  /** 语言版本 */
  langVersion: string
}

/**
 * 包信息
 *
 * @interface
 */
export interface IrPackage {
  /** 包名 */
  name: string
  /** 语言 */
  lang: Language | string
  /** 语义化版本 */
  version: string
}

/**
 * 备注（可选）
 *
 * @interface
 */
export interface IrRemarks {
  /** 标题 */
  title?: string
  /** 作者 */
  author?: string
  /** 注释 */
  comment?: string
}

/**
 * 数据结构对象描述
 *
 * @interface
 */
export interface IrObject {
  /** 结构类型 */
  kind: IrObjectKind
  /** 标签 */
  label?: string
}

/**
 * 数据结构类型
 *
 * @typedef
 */
export type IrObjectKind =
  | 'stack'
  | 'queue'
  | 'slist'
  | 'dlist'
  | 'binary_tree'
  | 'bst'
  | 'graph_undirected'
  | 'graph_directed'
  | 'graph_weighted'

/**
 * 状态快照
 *
 * @interface
 */
export interface IrState {
  /** 状态 ID（从 0 连续递增） */
  id: number
  /** 完整状态数据 */
  data: IrStateData
}

/**
 * 状态数据联合
 *
 * @typedef
 */
export type IrStateData =
  | StackStateData
  | QueueStateData
  | SListStateData
  | DListStateData
  | BinaryTreeStateData
  | GraphStateData
  | GraphWeightedStateData

export interface StackStateData {
  items: IrValue[]
  top: number
}

export interface QueueStateData {
  items: IrValue[]
  front: number
  rear: number
}

export interface SListNode {
  id: number
  value: IrValue
  next: number
  alias?: string
}

export interface SListStateData {
  head: number
  nodes: SListNode[]
}

export interface DListNode {
  id: number
  value: IrValue
  prev: number
  next: number
  alias?: string
}

export interface DListStateData {
  head: number
  tail: number
  nodes: DListNode[]
}

export interface BinaryTreeNode {
  id: number
  value: IrValue
  left: number
  right: number
  alias?: string
}

export interface BinaryTreeStateData {
  root: number
  nodes: BinaryTreeNode[]
}

export interface GraphNode {
  id: number
  label: string
  alias?: string
}

export interface GraphEdge {
  from: number
  to: number
}

export interface GraphWeightedEdge extends GraphEdge {
  weight: number
}

export interface GraphStateData {
  nodes: GraphNode[]
  edges: GraphEdge[]
}

export interface GraphWeightedStateData {
  nodes: GraphNode[]
  edges: GraphWeightedEdge[]
}

/* ----------------------------------------------------------------
 *  高亮标记
 * ---------------------------------------------------------------- */

/**
 * 节点高亮标记
 *
 * @interface
 */
export interface IrNodeHighlight {
  /** 标记类型 */
  kind: 'node'
  /** 节点 ID */
  target: number
  /** 高亮样式 */
  style: string
  /** 视觉强度 1..9 */
  level?: number
}

/**
 * 元素高亮标记（stack / queue 的 items 索引）
 *
 * @interface
 */
export interface IrItemHighlight {
  /** 标记类型 */
  kind: 'item'
  /** items 数组索引 */
  target: number
  /** 高亮样式 */
  style: string
  /** 视觉强度 1..9 */
  level?: number
}

/**
 * 边高亮标记
 *
 * @interface
 */
export interface IrEdgeHighlight {
  /** 标记类型 */
  kind: 'edge'
  /** 起始节点 ID */
  from: number
  /** 终止节点 ID */
  to: number
  /** 高亮样式 */
  style: string
  /** 视觉强度 1..9 */
  level?: number
}

/**
 * 高亮标记联合
 *
 * @typedef
 */
export type IrHighlight = IrNodeHighlight | IrItemHighlight | IrEdgeHighlight

/* ----------------------------------------------------------------
 *  步骤
 * ---------------------------------------------------------------- */

/**
 * 步骤参数
 *
 * @interface
 */
export interface IrStepArgs {
  [key: string]: IrValue | IrValue[] | Record<string, IrValue>
}

/**
 * 源码位置
 *
 * @interface
 */
export interface IrCodeLocation {
  /** 行号（从 1 开始） */
  line: number
  /** 列号（从 1 开始，可选） */
  col?: number
}

/**
 * 操作步骤
 *
 * @interface
 */
export interface IrStep {
  /** 步骤 ID（从 0 连续递增） */
  id: number
  /** 操作名称 */
  op: string
  /** 阶段标记 */
  phase?: string
  /** 操作前状态 ID */
  before: number
  /** 操作后状态 ID */
  after?: number
  /** 返回值 */
  ret?: IrValue | IrValue[] | Record<string, IrValue>
  /** 说明 */
  note?: string
  /** 参数 */
  args: IrStepArgs
  /** 源码位置 */
  code?: IrCodeLocation
  /** 高亮标记 */
  highlights?: IrHighlight[]
}

/* ----------------------------------------------------------------
 *  结局
 * ---------------------------------------------------------------- */

/**
 * 成功结果
 *
 * @interface
 */
export interface IrResult {
  /** 最终状态 ID */
  finalState: number
  /** 提交信息 */
  commit?: {
    op: string
    line: number
  }
}

/**
 * 执行错误
 *
 * @interface
 */
export interface IrError {
  /** 错误类型 */
  type: 'runtime' | 'timeout' | 'validation' | 'sandbox' | 'unknown'
  /** 错误消息 */
  message: string
  /** 出错行号 */
  line?: number
  /** 出错步骤 ID */
  step?: number
  /** 最后有效状态 ID */
  lastState?: number
}

/**
 * IR 值标量类型
 *
 * @typedef
 */
export type IrValue = number | string | boolean
