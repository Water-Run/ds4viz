/**
 * IR 数据结构定义（TOML 解析后）
 *
 * @file src/types/ir.ts
 * @author WaterRun
 * @date 2026-02-12
 * @updated 2026-02-12
 */

import type { Language } from './api'

/**
 * IR 顶层结构
 *
 * @interface
 */
export interface IrDocument {
  /** 元信息 */
  meta: IrMeta
  /** 包信息 */
  package: IrPackage
  /** 展示备注（可选） */
  remarks?: IrRemarks
  /** 数据结构对象 */
  object: IrObject
  /** 状态快照 */
  states: IrState[]
  /** 步骤列表 */
  steps?: IrStep[]
  /** 成功结果 */
  result?: IrResult
  /** 失败结果 */
  error?: IrError
}

/**
 * 元信息
 *
 * @interface
 */
export interface IrMeta {
  /** 创建时间（RFC3339） */
  createdAt: string
  /** 语言 */
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
  /** 版本 */
  version: string
}

/**
 * 备注
 *
 * @interface
 */
export interface IrRemarks {
  /** 标题 */
  title?: string
  /** 作者 */
  author?: string
  /** 说明 */
  comment?: string
}

/**
 * 数据结构对象
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
  /** 状态 ID */
  id: number
  /** 状态数据 */
  data: IrStateData
}

/**
 * 状态数据联合类型
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

/**
 * 栈状态数据
 *
 * @interface
 */
export interface StackStateData {
  items: IrValue[]
  top: number
}

/**
 * 队列状态数据
 *
 * @interface
 */
export interface QueueStateData {
  items: IrValue[]
  front: number
  rear: number
}

/**
 * 单链表节点
 *
 * @interface
 */
export interface SListNode {
  id: number
  value: IrValue
  next: number
}

/**
 * 单链表状态数据
 *
 * @interface
 */
export interface SListStateData {
  head: number
  nodes: SListNode[]
}

/**
 * 双链表节点
 *
 * @interface
 */
export interface DListNode {
  id: number
  value: IrValue
  prev: number
  next: number
}

/**
 * 双链表状态数据
 *
 * @interface
 */
export interface DListStateData {
  head: number
  tail: number
  nodes: DListNode[]
}

/**
 * 二叉树节点
 *
 * @interface
 */
export interface BinaryTreeNode {
  id: number
  value: IrValue
  left: number
  right: number
}

/**
 * 二叉树状态数据
 *
 * @interface
 */
export interface BinaryTreeStateData {
  root: number
  nodes: BinaryTreeNode[]
}

/**
 * 图节点
 *
 * @interface
 */
export interface GraphNode {
  id: number
  label: string
}

/**
 * 图边
 *
 * @interface
 */
export interface GraphEdge {
  from: number
  to: number
}

/**
 * 带权图边
 *
 * @interface
 */
export interface GraphWeightedEdge extends GraphEdge {
  weight: number
}

/**
 * 图状态数据
 *
 * @interface
 */
export interface GraphStateData {
  nodes: GraphNode[]
  edges: GraphEdge[]
}

/**
 * 带权图状态数据
 *
 * @interface
 */
export interface GraphWeightedStateData {
  nodes: GraphNode[]
  edges: GraphWeightedEdge[]
}

/**
 * 步骤参数
 *
 * @interface
 */
export interface IrStepArgs {
  [key: string]: IrValue | IrValue[] | Record<string, IrValue>
}

/**
 * 代码定位
 *
 * @interface
 */
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
}

/**
 * 成功结果
 *
 * @interface
 */
export interface IrResult {
  finalState: number
  commit?: {
    op: string
    line: number
  }
}

/**
 * 失败结果
 *
 * @interface
 */
export interface IrError {
  type: 'runtime' | 'timeout' | 'validation' | 'sandbox' | 'unknown'
  message: string
  line?: number
  step?: number
  lastState?: number
}

/**
 * IR 值
 *
 * @typedef
 */
export type IrValue = number | string | boolean
