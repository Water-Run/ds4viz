/**
 * 可视化渲染辅助类型
 *
 * @file src/types/viz.ts
 * @author WaterRun
 * @date 2026-03-25
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
