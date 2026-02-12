/**
 * IR TOML 解析与映射
 *
 * @file src/utils/ir.ts
 * @author WaterRun
 * @date 2026-02-12
 * @updated 2026-02-12
 */

import * as toml from '@iarna/toml'
import type {
  IrDocument,
  IrMeta,
  IrPackage,
  IrRemarks,
  IrObject,
  IrState,
  IrStep,
  IrResult,
  IrError,
} from '@/types/ir'

/**
 * TOML 解析结果
 *
 * @interface
 */
export interface ParseIrResult {
  /** 是否解析成功 */
  ok: boolean
  /** IR 文档 */
  document?: IrDocument
  /** 错误消息 */
  errorMessage?: string
}

/**
 * 解析 TOML 并映射为 IR 文档
 *
 * @param content - TOML 原始文本
 * @returns 解析结果
 */
export function parseIrToml(content: string): ParseIrResult {
  try {
    const raw = toml.parse(content) as Record<string, unknown>
    const document = mapDocument(raw)
    return { ok: true, document }
  } catch (error: unknown) {
    const message = error instanceof Error ? error.message : 'TOML 解析失败'
    return { ok: false, errorMessage: message }
  }
}

/**
 * 映射 IR 文档
 *
 * @param raw - TOML 解析结果
 * @returns IR 文档
 */
function mapDocument(raw: Record<string, unknown>): IrDocument {
  return {
    meta: mapMeta(raw.meta),
    package: mapPackage(raw.package),
    remarks: mapRemarks(raw.remarks),
    object: mapObject(raw.object),
    states: mapStates(raw.states),
    steps: mapSteps(raw.steps),
    result: mapResult(raw.result),
    error: mapError(raw.error),
  }
}

/**
 * 映射 meta
 */
function mapMeta(value: unknown): IrMeta {
  const meta = value as Record<string, unknown>
  return {
    createdAt: String(meta?.created_at ?? ''),
    lang: String(meta?.lang ?? ''),
    langVersion: String(meta?.lang_version ?? ''),
  }
}

/**
 * 映射 package
 */
function mapPackage(value: unknown): IrPackage {
  const pkg = value as Record<string, unknown>
  return {
    name: String(pkg?.name ?? ''),
    lang: String(pkg?.lang ?? ''),
    version: String(pkg?.version ?? ''),
  }
}

/**
 * 映射 remarks
 */
function mapRemarks(value: unknown): IrRemarks | undefined {
  if (value === undefined || value === null) {
    return undefined
  }
  const remarks = value as Record<string, unknown>
  return {
    title: remarks.title !== undefined ? String(remarks.title) : undefined,
    author: remarks.author !== undefined ? String(remarks.author) : undefined,
    comment: remarks.comment !== undefined ? String(remarks.comment) : undefined,
  }
}

/**
 * 映射 object
 */
function mapObject(value: unknown): IrObject {
  const object = value as Record<string, unknown>
  return {
    kind: String(object?.kind ?? '') as IrObject['kind'],
    label: object?.label !== undefined ? String(object.label) : undefined,
  }
}

/**
 * 映射 states
 */
function mapStates(value: unknown): IrState[] {
  if (!Array.isArray(value)) {
    return []
  }
  return value.map((item) => {
    const state = item as Record<string, unknown>
    return {
      id: Number(state.id ?? 0),
      data: (state.data ?? {}) as IrState['data'],
    }
  })
}

/**
 * 映射 steps
 */
function mapSteps(value: unknown): IrStep[] | undefined {
  if (!Array.isArray(value)) {
    return undefined
  }
  return value.map((item) => {
    const step = item as Record<string, unknown>
    const code = step.code as Record<string, unknown> | undefined
    return {
      id: Number(step.id ?? 0),
      op: String(step.op ?? ''),
      before: Number(step.before ?? 0),
      after: step.after !== undefined ? Number(step.after) : undefined,
      ret: step.ret as IrStep['ret'],
      note: step.note !== undefined ? String(step.note) : undefined,
      args: (step.args ?? {}) as IrStep['args'],
      code:
        code !== undefined
          ? {
              line: Number(code.line ?? 0),
              col: code.col !== undefined ? Number(code.col) : undefined,
            }
          : undefined,
    }
  })
}

/**
 * 映射 result
 */
function mapResult(value: unknown): IrResult | undefined {
  if (value === undefined || value === null) {
    return undefined
  }
  const result = value as Record<string, unknown>
  const commit = result.commit as Record<string, unknown> | undefined
  return {
    finalState: Number(result.final_state ?? 0),
    commit:
      commit !== undefined
        ? {
            op: String(commit.op ?? ''),
            line: Number(commit.line ?? 0),
          }
        : undefined,
  }
}

/**
 * 映射 error
 */
function mapError(value: unknown): IrError | undefined {
  if (value === undefined || value === null) {
    return undefined
  }
  const error = value as Record<string, unknown>
  return {
    type: String(error.type ?? 'unknown') as IrError['type'],
    message: String(error.message ?? ''),
    line: error.line !== undefined ? Number(error.line) : undefined,
    step: error.step !== undefined ? Number(error.step) : undefined,
    lastState: error.last_state !== undefined ? Number(error.last_state) : undefined,
  }
}
