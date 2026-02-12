/**
 * 可视化渲染状态工具
 *
 * @file src/utils/viz.ts
 * @author WaterRun
 * @date 2026-02-12
 * @updated 2026-02-12
 */

import type { IrDocument, IrState } from '@/types/ir'
import type { StepSummary, VizModel } from '@/types/viz'

/**
 * 构建可视化模型
 *
 * @param document - IR 文档
 * @returns 渲染模型
 */
export function buildVizModel(document: IrDocument): VizModel {
  const states = document.states
  const steps = document.steps ?? []
  const initialStateId = states.length > 0 ? states[0].id : 0
  return {
    document,
    states,
    steps,
    currentStateId: initialStateId,
    currentStepId: null,
  }
}

/**
 * 根据 stateId 获取状态
 *
 * @param model - 渲染模型
 * @param stateId - 状态 ID
 * @returns 状态或 null
 */
export function getStateById(model: VizModel, stateId: number): IrState | null {
  return model.states.find((state) => state.id === stateId) ?? null
}

/**
 * 获取当前步骤摘要
 *
 * @param model - 渲染模型
 * @returns 步骤摘要或 null
 */
export function getStepSummary(model: VizModel): StepSummary | null {
  if (model.currentStepId === null) {
    return null
  }
  const step = model.steps.find((item) => item.id === model.currentStepId)
  if (!step) {
    return null
  }
  return {
    op: step.op,
    line: step.code?.line,
    note: step.note,
    args: step.args as Record<string, unknown>,
    ret: step.ret,
  }
}

/**
 * 计算下一步 stateId
 *
 * @param model - 渲染模型
 * @param stepIndex - 步骤索引
 * @returns 状态 ID
 */
export function getStateIdByStepIndex(model: VizModel, stepIndex: number): number {
  if (stepIndex < 0 || stepIndex >= model.steps.length) {
    return model.currentStateId
  }
  const step = model.steps[stepIndex]
  return step.after ?? step.before
}

/**
 * 获取当前步骤索引
 *
 * @param model - 渲染模型
 * @returns 索引，未选中时返回 -1
 */
export function getCurrentStepIndex(model: VizModel): number {
  if (model.currentStepId === null) {
    return -1
  }
  return model.steps.findIndex((step) => step.id === model.currentStepId)
}
