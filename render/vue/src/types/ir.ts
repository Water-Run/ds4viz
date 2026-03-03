// src/types/ir.ts
/**
 * IR 数据结构定义（TOML 解析后）
 *
 * @file src/types/ir.ts
 * @author WaterRun
 * @date 2026-03-03
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
  /** 未知但可解析的顶层字段 */
  extra?: Record<string, unknown>
}

/**
 * 元信息
 *
 * @interface
 */
export interface IrMeta {
  createdAt: string
  lang: Language | string
  langVersion: string
  /** 未知但可解析的字段 */
  extra?: Record<string, unknown>
}

/**
 * 包信息
 *
 * @interface
 */
export interface IrPackage {
  name: string
  lang: Language | string
  version: string
  /** 未知但可解析的字段 */
  extra?: Record<string, unknown>
}

/**
 * 备注
 *
 * @interface
 */
export interface IrRemarks {
  title?: string
  author?: string
  comment?: string
  /** 未知但可解析的字段 */
  extra?: Record<string, unknown>
}

/**
 * 数据结构对象
 *
 * @interface
 */
export interface IrObject {
  kind: IrObjectKind
  label?: string
  /** 未知但可解析的字段 */
  extra?: Record<string, unknown>
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
  id: number
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
}

export interface BinaryTreeStateData {
  root: number
  nodes: BinaryTreeNode[]
}

export interface GraphNode {
  id: number
  label: string
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

export interface IrStepArgs {
  [key: string]: IrValue | IrValue[] | Record<string, IrValue>
}

export interface IrCodeLocation {
  line: number
  col?: number
}

/**
 * 操作步骤
 *
 * @interface
 */
export interface IrStep {
  id: number
  op: string
  before: number
  after?: number
  ret?: IrValue | IrValue[] | Record<string, IrValue>
  note?: string
  args: IrStepArgs
  code?: IrCodeLocation
  /** 未知但可解析的字段 */
  extra?: Record<string, unknown>
}

export interface IrResult {
  finalState: number
  commit?: {
    op: string
    line: number
  }
}

export interface IrError {
  type: 'runtime' | 'timeout' | 'validation' | 'sandbox' | 'unknown'
  message: string
  line?: number
  step?: number
  lastState?: number
}

export type IrValue = number | string | boolean
