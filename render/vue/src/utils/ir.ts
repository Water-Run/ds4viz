// src/utils/ir.ts
/**
 * IR TOML 解析与映射
 *
 * 解析后的 IrDocument 使用 markRaw 标记，
 * 防止 Vue 对嵌套数据创建响应式代理（消除渲染卡死根因）。
 *
 * 同时保留未知但可解析字段（extra），避免信息丢失。
 *
 * @file src/utils/ir.ts
 * @author WaterRun
 * @date 2026-03-25
 */

import * as toml from '@iarna/toml'
import { markRaw } from 'vue'

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
import { logDebug } from '@/utils/viz-flags'

/**
 * TOML 解析结果
 *
 * @interface
 */
export interface ParseIrResult {
  ok: boolean
  document?: IrDocument
  errorMessage?: string
}

/* ----------------------------------------------------------------
 *  markRaw 工具
 * ---------------------------------------------------------------- */

function deepMarkRaw<T>(value: T): T {
  if (value === null || value === undefined || typeof value !== 'object') {
    return value
  }
  markRaw(value as Record<string, unknown>)
  if (Array.isArray(value)) {
    for (const item of value) {
      if (item !== null && typeof item === 'object') {
        deepMarkRaw(item)
      }
    }
  } else {
    for (const nested of Object.values(value as Record<string, unknown>)) {
      if (nested !== null && typeof nested === 'object') {
        deepMarkRaw(nested)
      }
    }
  }
  return value
}

/* ----------------------------------------------------------------
 *  Unknown 字段提取
 * ---------------------------------------------------------------- */

function pickExtra(
  source: Record<string, unknown>,
  knownKeys: string[],
): Record<string, unknown> | undefined {
  const known = new Set<string>(knownKeys)
  const extraEntries = Object.entries(source).filter(([key]) => !known.has(key))
  if (extraEntries.length === 0) return undefined
  return Object.fromEntries(extraEntries)
}

/* ----------------------------------------------------------------
 *  公开 API
 * ---------------------------------------------------------------- */

export function parseIrToml(content: string): ParseIrResult {
  try {
    logDebug('[ir] parsing TOML, length =', content.length)
    const raw = toml.parse(content) as Record<string, unknown>
    const document = mapDocument(raw)
    const validationError = validateDocument(document)

    if (validationError) {
      logDebug('[ir] validation failed:', validationError)
      return { ok: false, errorMessage: validationError }
    }

    deepMarkRaw(document)

    logDebug('[ir] parse ok', {
      kind: document.object.kind,
      states: document.states.length,
      steps: document.steps?.length ?? 0,
      hasResult: document.result !== undefined,
      hasError: document.error !== undefined,
    })

    return { ok: true, document }
  } catch (error: unknown) {
    const message = error instanceof Error ? error.message : 'TOML 解析失败'
    logDebug('[ir] parse exception:', message)
    return { ok: false, errorMessage: message }
  }
}

/* ----------------------------------------------------------------
 *  校验
 * ---------------------------------------------------------------- */

function validateDocument(document: IrDocument): string {
  const allowedLangs = new Set([
    'python',
    'c',
    'zig',
    'rust',
    'java',
    'csharp',
    'typescript',
    'lua',
  ])
  const allowedKinds = new Set([
    'stack',
    'queue',
    'slist',
    'dlist',
    'binary_tree',
    'bst',
    'graph_undirected',
    'graph_directed',
    'graph_weighted',
  ])

  if (!document.meta.createdAt || !document.meta.lang || !document.meta.langVersion) {
    return 'IR 缺少 meta 信息'
  }

  if (!allowedLangs.has(String(document.meta.lang))) {
    return `不支持的语言: ${document.meta.lang}`
  }

  if (!document.package.name || !document.package.lang || !document.package.version) {
    return 'IR 缺少 package 信息'
  }

  if (document.meta.lang !== document.package.lang) {
    return 'meta.lang 与 package.lang 不一致'
  }

  if (!allowedKinds.has(document.object.kind)) {
    return `不支持的数据结构类型: ${document.object.kind}`
  }

  if (document.meta.langVersion && !/^[0-9a-z.+-]+$/.test(document.meta.langVersion)) {
    return 'meta.lang_version 格式不合法'
  }

  if (document.states.length === 0) {
    return 'IR states 不能为空'
  }

  for (let index = 0; index < document.states.length; index += 1) {
    const state = document.states[index]
    if (state.id !== index) {
      return 'states.id 必须从 0 连续递增'
    }
  }

  if (document.result && document.error) {
    return 'IR 同时存在 result 与 error'
  }

  if (!document.result && !document.error) {
    return 'IR 缺少 result 或 error'
  }

  if (document.result && !Number.isFinite(document.result.finalState)) {
    return 'result.final_state 无效'
  }

  if (document.result && document.result.finalState >= document.states.length) {
    return 'result.final_state 超出 states 范围'
  }

  if (document.error && document.error.step !== undefined) {
    const stepId = document.error.step
    if (!document.steps || !document.steps.some((step) => step.id === stepId)) {
      return 'error.step 未引用有效 steps.id'
    }
  }

  if (document.error && document.error.line !== undefined && document.error.line < 1) {
    return 'error.line 必须为正整数'
  }

  if (document.error && document.error.lastState !== undefined) {
    const lastState = document.error.lastState
    if (lastState < 0 || lastState >= document.states.length) {
      return 'error.last_state 未引用有效 states.id'
    }
  }

  if (document.error) {
    const allowedErrorTypes = new Set(['runtime', 'timeout', 'validation', 'sandbox', 'unknown'])
    if (!allowedErrorTypes.has(document.error.type)) {
      return 'error.type 不在允许范围内'
    }
    if (!document.error.message || document.error.message.length > 512) {
      return 'error.message 长度不合法'
    }
    if (document.error.message.includes('\n')) {
      return 'error.message 不允许包含换行符'
    }
  }

  const steps = document.steps ?? []
  for (let index = 0; index < steps.length; index += 1) {
    const step = steps[index]

    if (step.id !== index) {
      return 'steps.id 必须从 0 连续递增'
    }

    if (step.before < 0 || step.before >= document.states.length) {
      return 'steps.before 未引用有效 states.id'
    }

    if (step.after !== undefined) {
      if (step.after < 0 || step.after >= document.states.length) {
        return 'steps.after 未引用有效 states.id'
      }
    } else if (!document.error || document.error.step !== step.id) {
      return 'steps.after 缺失时必须存在 error.step'
    }

    if (step.code?.line !== undefined && step.code.line < 1) {
      return 'steps.code.line 必须为正整数'
    }

    if (step.code?.col !== undefined && step.code.col < 1) {
      return 'steps.code.col 必须为正整数'
    }
  }

  return ''
}

/* ----------------------------------------------------------------
 *  映射
 * ---------------------------------------------------------------- */

function mapDocument(raw: Record<string, unknown>): IrDocument {
  const document: IrDocument = {
    meta: mapMeta(raw.meta),
    package: mapPackage(raw.package),
    remarks: mapRemarks(raw.remarks),
    object: mapObject(raw.object),
    states: mapStates(raw.states),
    steps: mapSteps(raw.steps),
    result: mapResult(raw.result),
    error: mapError(raw.error),
    extra: pickExtra(raw, ['meta', 'package', 'remarks', 'object', 'states', 'steps', 'result', 'error']),
  }
  return document
}

function mapMeta(value: unknown): IrMeta {
  const meta = (value ?? {}) as Record<string, unknown>
  return {
    createdAt: String(meta.created_at ?? ''),
    lang: String(meta.lang ?? ''),
    langVersion: String(meta.lang_version ?? ''),
    extra: pickExtra(meta, ['created_at', 'lang', 'lang_version']),
  }
}

function mapPackage(value: unknown): IrPackage {
  const pkg = (value ?? {}) as Record<string, unknown>
  return {
    name: String(pkg.name ?? ''),
    lang: String(pkg.lang ?? ''),
    version: String(pkg.version ?? ''),
    extra: pickExtra(pkg, ['name', 'lang', 'version']),
  }
}

function mapRemarks(value: unknown): IrRemarks | undefined {
  if (value === undefined || value === null) return undefined
  const remarks = value as Record<string, unknown>
  return {
    title: remarks.title !== undefined ? String(remarks.title) : undefined,
    author: remarks.author !== undefined ? String(remarks.author) : undefined,
    comment: remarks.comment !== undefined ? String(remarks.comment) : undefined,
    extra: pickExtra(remarks, ['title', 'author', 'comment']),
  }
}

function mapObject(value: unknown): IrObject {
  const object = (value ?? {}) as Record<string, unknown>
  return {
    kind: String(object.kind ?? '') as IrObject['kind'],
    label: object.label !== undefined ? String(object.label) : undefined,
    extra: pickExtra(object, ['kind', 'label']),
  }
}

function mapStates(value: unknown): IrState[] {
  if (!Array.isArray(value)) return []
  return value.map((item) => {
    const state = item as Record<string, unknown>
    return {
      id: Number(state.id ?? 0),
      data: (state.data ?? {}) as IrState['data'],
    }
  })
}

function mapSteps(value: unknown): IrStep[] | undefined {
  if (!Array.isArray(value)) return undefined
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
      code: code
        ? {
          line: Number(code.line ?? 0),
          col: code.col !== undefined ? Number(code.col) : undefined,
        }
        : undefined,
      extra: pickExtra(step, ['id', 'op', 'before', 'after', 'ret', 'note', 'args', 'code']),
    }
  })
}

function mapResult(value: unknown): IrResult | undefined {
  if (value === undefined || value === null) return undefined
  const result = value as Record<string, unknown>
  const commit = result.commit as Record<string, unknown> | undefined

  return {
    finalState: Number(result.final_state ?? 0),
    commit: commit
      ? {
        op: String(commit.op ?? ''),
        line: Number(commit.line ?? 0),
      }
      : undefined,
  }
}

function mapError(value: unknown): IrError | undefined {
  if (value === undefined || value === null) return undefined
  const error = value as Record<string, unknown>
  return {
    type: String(error.type ?? 'unknown') as IrError['type'],
    message: String(error.message ?? ''),
    line: error.line !== undefined ? Number(error.line) : undefined,
    step: error.step !== undefined ? Number(error.step) : undefined,
    lastState: error.last_state !== undefined ? Number(error.last_state) : undefined,
  }
}
