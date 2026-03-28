/**
 * 可视化辅助工具
 *
 * 提供纯函数，基于 IrDocument + 状态索引派生渲染所需数据。
 * 不持有可变状态，不依赖 Vue 响应式。
 *
 * @file src/utils/viz.ts
 * @author WaterRun
 * @date 2026-03-28
 */

import type { IrDocument, IrState, IrStep } from '@/types/ir'
import type { StepSummary, PhaseSegment, Frame } from '@/types/viz'

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

/**
 * 构建帧序列
 *
 * 帧 0 为初始状态（无 step），后续每帧对应一个 step。
 * 即使多个 step 指向同一 state（如 observe），也各自生成独立帧。
 * 总帧数 = 1 + steps.length。
 *
 * @param doc - IR 文档
 * @returns 帧列表
 */
export function buildFrameList(doc: IrDocument): Frame[] {
  const frames: Frame[] = [{
    index: 0,
    stateIndex: 0,
    summary: null,
  }]

  const steps = doc.steps ?? []
  for (const step of steps) {
    frames.push({
      index: frames.length,
      stateIndex: step.after ?? step.before,
      summary: {
        op: step.op,
        line: step.code?.line,
        note: step.note,
        args: step.args as Record<string, unknown>,
        ret: step.ret,
        phase: step.phase,
        highlights: step.highlights,
      },
    })
  }

  return frames
}

/**
 * 从 IR 文档中提取阶段段落列表
 *
 * 同一 phase 值的连续 steps 合并为一个段落，
 * 无 phase 的步骤跳过。
 * 帧索引 = step.id + 1（帧 0 为初始状态）。
 *
 * @param doc - IR 文档
 * @returns 阶段段落数组（按步骤顺序）
 */
export function computePhaseSegments(doc: IrDocument): PhaseSegment[] {
  const steps = doc.steps ?? []
  const segments: PhaseSegment[] = []

  let i = 0
  while (i < steps.length) {
    const step = steps[i]
    if (step.phase === undefined) {
      i += 1
      continue
    }

    const phaseName = step.phase
    let lastStep = step
    let count = 1
    let j = i + 1

    while (j < steps.length && steps[j].phase === phaseName) {
      lastStep = steps[j]
      count += 1
      j += 1
    }

    segments.push({
      name: phaseName,
      targetFrameIndex: step.id + 1,
      endFrameIndex: lastStep.id + 1,
      stepCount: count,
      firstStepId: step.id,
      lastStepId: lastStep.id,
    })

    i = j
  }

  return segments
}
