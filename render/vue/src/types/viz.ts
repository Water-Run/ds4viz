/**
 * 交互渲染状态类型
 *
 * @file src/types/viz.ts
 * @author WaterRun
 * @date 2026-02-12
 * @updated 2026-02-12
 */

import type { IrDocument, IrState, IrStep } from './ir'

/**
 * 渲染模型
 *
 * @interface
 */
export interface VizModel {
  /** IR 文档 */
  document: IrDocument
  /** 状态列表 */
  states: IrState[]
  /** 步骤列表 */
  steps: IrStep[]
  /** 当前状态 ID */
  currentStateId: number
  /** 当前步骤 ID */
  currentStepId: number | null
}

/**
 * 步骤摘要信息
 *
 * @interface
 */
export interface StepSummary {
  /** 操作名称 */
  op: string
  /** 行号 */
  line?: number
  /** 备注 */
  note?: string
  /** 参数 */
  args?: Record<string, unknown>
  /** 返回值 */
  ret?: unknown
}
