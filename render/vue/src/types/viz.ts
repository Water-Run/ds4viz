/**
 * 可视化渲染辅助类型
 *
 * @file src/types/viz.ts
 * @author WaterRun
 * @date 2026-03-26
 */

import type { IrHighlight } from './ir'

/**
 * 步骤摘要信息（传递给 VizPanel 显示）
 *
 * @interface
 */
export interface StepSummary {
  /** 操作名称 */
  op: string
  /** 源码行号 */
  line?: number
  /** 备注 */
  note?: string
  /** 参数 */
  args?: Record<string, unknown>
  /** 返回值 */
  ret?: unknown
  /** 阶段标记 */
  phase?: string
  /** 高亮标记列表 */
  highlights?: IrHighlight[]
}

/**
 * 可视化帧
 *
 * 导航的最小单位。初始帧对应 state 0 且无 step，
 * 后续每帧对应一个 step 的结果（即使多个 step 指向同一 state）。
 *
 * @interface
 */
export interface Frame {
  /** 帧索引（0-based） */
  index: number
  /** 对应的 states 数组索引 */
  stateIndex: number
  /** 步骤摘要（初始帧为 null） */
  summary: StepSummary | null
}

/**
 * 阶段段落信息
 *
 * 同一 phase 值的连续 steps 合并为一个段落。
 *
 * @interface
 */
export interface PhaseSegment {
  /** 阶段名称 */
  name: string
  /** 阶段内第一帧的 frame 索引（跳转目标） */
  targetFrameIndex: number
  /** 阶段内最后一帧的 frame 索引 */
  endFrameIndex: number
  /** 阶段包含的步骤数 */
  stepCount: number
  /** 步骤 ID 范围起始 */
  firstStepId: number
  /** 步骤 ID 范围结束 */
  lastStepId: number
}
