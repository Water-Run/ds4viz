/**
 * 可视化辅助工具
 *
 * 提供纯函数，基于 IrDocument + 状态索引派生渲染所需数据。
 * 不持有可变状态，不依赖 Vue 响应式。
 *
 * @file src/utils/viz.ts
 * @author WaterRun
 * @date 2026-03-25
 */

import type { IrDocument, IrState, IrStep } from '@/types/ir'
import type { StepSummary } from '@/types/viz'

/**
 * 根据状态索引获取状态快照
 *
 * @param doc - IR 文档
 * @param stateIndex - 状态索引（0-based）
 * @returns 状态对象，越界时返回 null
 */
export function getStateByIndex(doc: IrDocument, stateIndex: number): IrState | null {
  if (stateIndex < 0 || stateIndex >= doc.states.length) {
    return null
  }
  return doc.states[stateIndex]
}

/**
 * 查找产生指定状态的操作步骤
 *
 * 即 step.after === stateIndex 的步骤。
 * 对于初始状态（stateIndex = 0），通常无对应步骤。
 *
 * @param doc - IR 文档
 * @param stateIndex - 状态索引
 * @returns 对应的 IrStep，未找到时返回 null
 */
export function getStepForStateIndex(doc: IrDocument, stateIndex: number): IrStep | null {
  const steps = doc.steps ?? []
  for (let i = 0; i < steps.length; i += 1) {
    if (steps[i].after === stateIndex) {
      return steps[i]
    }
  }
  return null
}

/**
 * 获取指定状态对应的步骤摘要
 *
 * @param doc - IR 文档
 * @param stateIndex - 状态索引
 * @returns 步骤摘要，无对应步骤时返回 null
 */
export function getStepSummaryForState(doc: IrDocument, stateIndex: number): StepSummary | null {
  const step = getStepForStateIndex(doc, stateIndex)
  if (!step) {
    return null
  }
  return {
    op: step.op,
    line: step.code?.line,
    note: step.note,
    args: step.args as Record<string, unknown>,
    ret: step.ret,
    phase: step.phase,
    highlights: step.highlights,
  }
}
