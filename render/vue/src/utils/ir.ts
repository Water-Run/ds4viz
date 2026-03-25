/**
 * IR TOML 解析与映射
 *
 * 严格遵循 IR 定义文档进行字段校验：
 * - 每个 table 仅允许规定字段，多余字段报错
 * - 高亮标记交叉校验 kind ↔ object.kind、引用存在性
 * - alias 格式与唯一性校验
 *
 * 解析后使用 markRaw 标记，防止 Vue 深响应式代理。
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
  IrHighlight,
  IrObjectKind,
  IrStateData,
  StackStateData,
  QueueStateData,
  SListStateData,
  DListStateData,
  BinaryTreeStateData,
  GraphStateData,
  GraphWeightedStateData,
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

/* ================================================================
 *  常量
 * ================================================================ */

const ALLOWED_LANGS = new Set([
  'python', 'c', 'zig', 'rust', 'java', 'csharp', 'typescript', 'lua',
])

const ALLOWED_KINDS = new Set<string>([
  'stack', 'queue', 'slist', 'dlist', 'binary_tree', 'bst',
  'graph_undirected', 'graph_directed', 'graph_weighted',
])

const ALLOWED_ERROR_TYPES = new Set([
  'runtime', 'timeout', 'validation', 'sandbox', 'unknown',
])

const HIGHLIGHT_KINDS_BY_OBJECT: Record<string, Set<string>> = {
  stack: new Set(['item']),
  queue: new Set(['item']),
  slist: new Set(['node']),
  dlist: new Set(['node']),
  binary_tree: new Set(['node']),
  bst: new Set(['node']),
  graph_undirected: new Set(['node', 'edge']),
  graph_directed: new Set(['node', 'edge']),
  graph_weighted: new Set(['node', 'edge']),
}

const ASCII_ID_RE = /^[0-9a-z_-]+$/
const ALIAS_RE = /^[a-zA-Z0-9_-]+$/
const LANG_VERSION_RE = /^[0-9a-z.+-]+$/
const PACKAGE_NAME_RE = /^[0-9a-z_-]+$/
const SEMVER_RE = /^\d+\.\d+\.\d+$/
const STYLE_RE = /^[a-z0-9_-]+$/

/* ================================================================
 *  工具函数
 * ================================================================ */

type RawTable = Record<string, unknown>

function deepMarkRaw<T>(value: T): T {
  if (value === null || value === undefined || typeof value !== 'object') {
    return value
  }
  markRaw(value as RawTable)
  if (Array.isArray(value)) {
    for (const item of value) {
      if (item !== null && typeof item === 'object') {
        deepMarkRaw(item)
      }
    }
  } else {
    for (const nested of Object.values(value as RawTable)) {
      if (nested !== null && typeof nested === 'object') {
        deepMarkRaw(nested)
      }
    }
  }
  return value
}

/**
 * 检查 table 仅包含指定的必需和可选字段
 *
 * @param obj - 待检查对象
 * @param required - 必需字段列表
 * @param optional - 可选字段列表
 * @param ctx - 上下文描述（用于错误消息）
 * @returns 错误消息，通过则为空字符串
 */
function checkKeys(
  obj: RawTable,
  required: string[],
  optional: string[],
  ctx: string,
): string {
  const allowed = new Set([...required, ...optional])
  for (const key of Object.keys(obj)) {
    if (!allowed.has(key)) {
      return `${ctx} 包含不允许的字段: ${key}`
    }
  }
  for (const key of required) {
    if (!(key in obj)) {
      return `${ctx} 缺少必需字段: ${key}`
    }
  }
  return ''
}

function isScalar(v: unknown): boolean {
  const t = typeof v
  return t === 'number' || t === 'string' || t === 'boolean'
}

/* ================================================================
 *  原始结构校验
 * ================================================================ */

function validateRaw(raw: RawTable): string {
  const topErr = checkKeys(
    raw,
    ['meta', 'package', 'object', 'states'],
    ['remarks', 'steps', 'result', 'error'],
    '文档顶层',
  )
  if (topErr) return topErr

  let err = ''

  err = validateRawMeta(raw.meta as RawTable | undefined)
  if (err) return err

  err = validateRawPackage(raw.package as RawTable | undefined, raw.meta as RawTable)
  if (err) return err

  err = validateRawRemarks(raw.remarks as RawTable | undefined)
  if (err) return err

  err = validateRawObject(raw.object as RawTable | undefined)
  if (err) return err

  const kind = String((raw.object as RawTable).kind)

  err = validateRawStates(raw.states, kind)
  if (err) return err

  err = validateRawSteps(raw.steps, kind)
  if (err) return err

  err = validateRawResult(raw.result as RawTable | undefined)
  if (err) return err

  err = validateRawError(raw.error as RawTable | undefined)
  if (err) return err

  return ''
}

function validateRawMeta(meta: RawTable | undefined): string {
  if (!meta || typeof meta !== 'object') return 'meta 缺失或格式错误'
  const err = checkKeys(meta, ['created_at', 'lang', 'lang_version'], [], '[meta]')
  if (err) return err
  if (typeof meta.created_at !== 'string' || meta.created_at.length === 0) return 'meta.created_at 必须为非空字符串'
  if (typeof meta.lang !== 'string' || !ALLOWED_LANGS.has(meta.lang)) return `不支持的语言: ${String(meta.lang)}`
  if (typeof meta.lang_version !== 'string' || !LANG_VERSION_RE.test(meta.lang_version)) return 'meta.lang_version 格式不合法'
  return ''
}

function validateRawPackage(pkg: RawTable | undefined, meta: RawTable): string {
  if (!pkg || typeof pkg !== 'object') return 'package 缺失或格式错误'
  const err = checkKeys(pkg, ['name', 'lang', 'version'], [], '[package]')
  if (err) return err
  if (typeof pkg.name !== 'string' || !PACKAGE_NAME_RE.test(pkg.name) || pkg.name.length === 0 || pkg.name.length > 64) return 'package.name 格式不合法'
  if (typeof pkg.lang !== 'string' || !ALLOWED_LANGS.has(pkg.lang)) return `不支持的 package.lang: ${String(pkg.lang)}`
  if (typeof pkg.version !== 'string' || !SEMVER_RE.test(pkg.version)) return 'package.version 格式不合法'
  if (pkg.lang !== meta.lang) return 'meta.lang 与 package.lang 不一致'
  return ''
}

function validateRawRemarks(remarks: RawTable | undefined): string {
  if (remarks === undefined || remarks === null) return ''
  if (typeof remarks !== 'object') return '[remarks] 格式错误'
  const err = checkKeys(remarks, [], ['title', 'author', 'comment'], '[remarks]')
  if (err) return err
  if (remarks.title === undefined && remarks.author === undefined && remarks.comment === undefined) {
    return '[remarks] 必须至少包含 title/author/comment 之一'
  }
  if (remarks.title !== undefined && (typeof remarks.title !== 'string' || remarks.title.length === 0 || remarks.title.length > 128 || String(remarks.title).includes('\n'))) return 'remarks.title 格式不合法'
  if (remarks.author !== undefined && (typeof remarks.author !== 'string' || remarks.author.length === 0 || remarks.author.length > 64 || String(remarks.author).includes('\n'))) return 'remarks.author 格式不合法'
  if (remarks.comment !== undefined && (typeof remarks.comment !== 'string' || remarks.comment.length === 0 || remarks.comment.length > 256 || String(remarks.comment).includes('\n'))) return 'remarks.comment 格式不合法'
  return ''
}

function validateRawObject(obj: RawTable | undefined): string {
  if (!obj || typeof obj !== 'object') return '[object] 缺失或格式错误'
  const err = checkKeys(obj, ['kind'], ['label'], '[object]')
  if (err) return err
  if (typeof obj.kind !== 'string' || !ALLOWED_KINDS.has(obj.kind)) return `不支持的数据结构类型: ${String(obj.kind)}`
  if (obj.label !== undefined && (typeof obj.label !== 'string' || obj.label.length === 0 || obj.label.length > 64 || String(obj.label).includes('\n'))) return 'object.label 格式不合法'
  return ''
}

function validateRawStates(states: unknown, kind: string): string {
  if (!Array.isArray(states) || states.length === 0) return 'states 不能为空'
  for (let i = 0; i < states.length; i += 1) {
    const s = states[i] as RawTable
    const err = checkKeys(s, ['id', 'data'], [], `states[${i}]`)
    if (err) return err
    if (s.id !== i) return `states[${i}].id 必须为 ${i}，实际为 ${String(s.id)}`
    const dataErr = validateRawStateData(s.data as RawTable, kind, i)
    if (dataErr) return dataErr
  }
  return ''
}

function validateRawStateData(data: RawTable | undefined, kind: string, idx: number): string {
  if (!data || typeof data !== 'object') return `states[${idx}].data 缺失或格式错误`
  const ctx = `states[${idx}].data`

  if (kind === 'stack') return validateStackData(data, ctx)
  if (kind === 'queue') return validateQueueData(data, ctx)
  if (kind === 'slist') return validateSListData(data, ctx)
  if (kind === 'dlist') return validateDListData(data, ctx)
  if (kind === 'binary_tree' || kind === 'bst') return validateTreeData(data, ctx)
  if (kind === 'graph_undirected') return validateGraphData(data, ctx, 'undirected')
  if (kind === 'graph_directed') return validateGraphData(data, ctx, 'directed')
  if (kind === 'graph_weighted') return validateGraphData(data, ctx, 'weighted')
  return ''
}

function validateStackData(data: RawTable, ctx: string): string {
  const err = checkKeys(data, ['items', 'top'], [], ctx)
  if (err) return err
  if (!Array.isArray(data.items)) return `${ctx}.items 必须为数组`
  for (const v of data.items as unknown[]) {
    if (!isScalar(v)) return `${ctx}.items 元素必须为标量`
  }
  const items = data.items as unknown[]
  const top = data.top as number
  if (items.length === 0 && top !== -1) return `${ctx} 空栈时 top 必须为 -1`
  if (items.length > 0 && top !== items.length - 1) return `${ctx} top 必须为 len(items) - 1`
  return ''
}

function validateQueueData(data: RawTable, ctx: string): string {
  const err = checkKeys(data, ['items', 'front', 'rear'], [], ctx)
  if (err) return err
  if (!Array.isArray(data.items)) return `${ctx}.items 必须为数组`
  for (const v of data.items as unknown[]) {
    if (!isScalar(v)) return `${ctx}.items 元素必须为标量`
  }
  const items = data.items as unknown[]
  const front = data.front as number
  const rear = data.rear as number
  if (items.length === 0) {
    if (front !== -1 || rear !== -1) return `${ctx} 空队列时 front/rear 必须为 -1`
  } else {
    if (front < 0 || rear < 0 || front > rear || rear >= items.length) return `${ctx} front/rear 范围不合法`
  }
  return ''
}

function validateNodeAlias(nodes: RawTable[], ctx: string): string {
  const aliases = new Set<string>()
  for (let j = 0; j < nodes.length; j += 1) {
    const alias = nodes[j].alias
    if (alias !== undefined) {
      if (typeof alias !== 'string' || alias.length === 0 || alias.length > 64 || !ALIAS_RE.test(alias)) {
        return `${ctx}.nodes[${j}].alias 格式不合法`
      }
      if (aliases.has(alias)) return `${ctx} alias 重复: ${alias}`
      aliases.add(alias)
    }
  }
  return ''
}

function validateSListData(data: RawTable, ctx: string): string {
  let err = checkKeys(data, ['head', 'nodes'], [], ctx)
  if (err) return err
  if (!Array.isArray(data.nodes)) return `${ctx}.nodes 必须为数组`
  const nodes = data.nodes as RawTable[]
  const ids = new Set<number>()
  for (let j = 0; j < nodes.length; j += 1) {
    err = checkKeys(nodes[j], ['id', 'value', 'next'], ['alias'], `${ctx}.nodes[${j}]`)
    if (err) return err
    if (!isScalar(nodes[j].value)) return `${ctx}.nodes[${j}].value 必须为标量`
    ids.add(nodes[j].id as number)
  }
  const head = data.head as number
  if (head !== -1 && !ids.has(head)) return `${ctx}.head 引用不存在的节点 ${head}`
  for (let j = 0; j < nodes.length; j += 1) {
    const next = nodes[j].next as number
    if (next !== -1 && !ids.has(next)) return `${ctx}.nodes[${j}].next 引用不存在的节点 ${next}`
  }
  return validateNodeAlias(nodes, ctx)
}

function validateDListData(data: RawTable, ctx: string): string {
  let err = checkKeys(data, ['head', 'tail', 'nodes'], [], ctx)
  if (err) return err
  if (!Array.isArray(data.nodes)) return `${ctx}.nodes 必须为数组`
  const nodes = data.nodes as RawTable[]
  const ids = new Set<number>()
  for (let j = 0; j < nodes.length; j += 1) {
    err = checkKeys(nodes[j], ['id', 'value', 'prev', 'next'], ['alias'], `${ctx}.nodes[${j}]`)
    if (err) return err
    if (!isScalar(nodes[j].value)) return `${ctx}.nodes[${j}].value 必须为标量`
    ids.add(nodes[j].id as number)
  }
  const head = data.head as number
  const tail = data.tail as number
  if ((head === -1) !== (tail === -1)) return `${ctx} head/tail 必须同时为 -1 或同时不为 -1`
  if (head !== -1 && !ids.has(head)) return `${ctx}.head 引用不存在的节点`
  if (tail !== -1 && !ids.has(tail)) return `${ctx}.tail 引用不存在的节点`
  for (let j = 0; j < nodes.length; j += 1) {
    const prev = nodes[j].prev as number
    const next = nodes[j].next as number
    if (prev !== -1 && !ids.has(prev)) return `${ctx}.nodes[${j}].prev 引用不存在的节点`
    if (next !== -1 && !ids.has(next)) return `${ctx}.nodes[${j}].next 引用不存在的节点`
  }
  return validateNodeAlias(nodes, ctx)
}

function validateTreeData(data: RawTable, ctx: string): string {
  let err = checkKeys(data, ['root', 'nodes'], [], ctx)
  if (err) return err
  if (!Array.isArray(data.nodes)) return `${ctx}.nodes 必须为数组`
  const nodes = data.nodes as RawTable[]
  const ids = new Set<number>()
  for (let j = 0; j < nodes.length; j += 1) {
    err = checkKeys(nodes[j], ['id', 'value', 'left', 'right'], ['alias'], `${ctx}.nodes[${j}]`)
    if (err) return err
    if (!isScalar(nodes[j].value)) return `${ctx}.nodes[${j}].value 必须为标量`
    ids.add(nodes[j].id as number)
  }
  const root = data.root as number
  if (root !== -1 && !ids.has(root)) return `${ctx}.root 引用不存在的节点`
  for (let j = 0; j < nodes.length; j += 1) {
    const left = nodes[j].left as number
    const right = nodes[j].right as number
    if (left !== -1 && !ids.has(left)) return `${ctx}.nodes[${j}].left 引用不存在的节点`
    if (right !== -1 && !ids.has(right)) return `${ctx}.nodes[${j}].right 引用不存在的节点`
  }
  return validateNodeAlias(nodes, ctx)
}

function validateGraphData(data: RawTable, ctx: string, variant: 'undirected' | 'directed' | 'weighted'): string {
  let err = checkKeys(data, ['nodes', 'edges'], [], ctx)
  if (err) return err
  if (!Array.isArray(data.nodes)) return `${ctx}.nodes 必须为数组`
  if (!Array.isArray(data.edges)) return `${ctx}.edges 必须为数组`

  const nodes = data.nodes as RawTable[]
  const ids = new Set<number>()
  for (let j = 0; j < nodes.length; j += 1) {
    err = checkKeys(nodes[j], ['id', 'label'], ['alias'], `${ctx}.nodes[${j}]`)
    if (err) return err
    if (typeof nodes[j].label !== 'string' || (nodes[j].label as string).length === 0 || (nodes[j].label as string).length > 32 || (nodes[j].label as string).includes('\n')) {
      return `${ctx}.nodes[${j}].label 格式不合法`
    }
    ids.add(nodes[j].id as number)
  }

  const edgeRequiredFields = variant === 'weighted' ? ['from', 'to', 'weight'] : ['from', 'to']
  const edges = data.edges as RawTable[]
  const edgeSet = new Set<string>()
  for (let j = 0; j < edges.length; j += 1) {
    err = checkKeys(edges[j], edgeRequiredFields, [], `${ctx}.edges[${j}]`)
    if (err) return err
    const from = edges[j].from as number
    const to = edges[j].to as number
    if (!ids.has(from)) return `${ctx}.edges[${j}].from 引用不存在的节点 ${from}`
    if (!ids.has(to)) return `${ctx}.edges[${j}].to 引用不存在的节点 ${to}`
    if (from === to) return `${ctx}.edges[${j}] 不允许自环`
    if (variant === 'undirected' && from >= to) return `${ctx}.edges[${j}] 无向边必须满足 from < to`
    if (variant === 'weighted' && typeof edges[j].weight !== 'number') return `${ctx}.edges[${j}].weight 必须为数值`
    const edgeKey = `${from}-${to}`
    if (edgeSet.has(edgeKey)) return `${ctx} 存在重复边 (${from}, ${to})`
    edgeSet.add(edgeKey)
  }

  return validateNodeAlias(nodes, ctx)
}

function validateRawSteps(steps: unknown, kind: string): string {
  if (steps === undefined || steps === null) return ''
  if (!Array.isArray(steps)) return 'steps 格式错误'
  for (let i = 0; i < steps.length; i += 1) {
    const s = steps[i] as RawTable
    const err = checkKeys(s, ['id', 'op', 'before', 'args'], ['phase', 'after', 'ret', 'note', 'code', 'highlights'], `steps[${i}]`)
    if (err) return err
    if (s.id !== i) return `steps[${i}].id 必须为 ${i}`
    if (typeof s.op !== 'string' || !ASCII_ID_RE.test(s.op) || s.op.length === 0 || s.op.length > 64) return `steps[${i}].op 格式不合法`
    if (s.phase !== undefined && (typeof s.phase !== 'string' || s.phase.length === 0 || s.phase.length > 64 || String(s.phase).includes('\n'))) return `steps[${i}].phase 格式不合法`
    if (s.note !== undefined && (typeof s.note !== 'string' || s.note.length === 0 || s.note.length > 256 || String(s.note).includes('\n'))) return `steps[${i}].note 格式不合法`

    if (typeof s.args !== 'object' || s.args === null || Array.isArray(s.args)) return `steps[${i}].args 必须为 table`
    for (const key of Object.keys(s.args as RawTable)) {
      if (!ASCII_ID_RE.test(key)) return `steps[${i}].args 的键 "${key}" 格式不合法`
    }

    if (s.code !== undefined) {
      if (typeof s.code !== 'object' || s.code === null) return `steps[${i}].code 格式错误`
      const codeErr = checkKeys(s.code as RawTable, ['line'], ['col'], `steps[${i}].code`)
      if (codeErr) return codeErr
      if (typeof (s.code as RawTable).line !== 'number' || (s.code as RawTable).line as number < 1) return `steps[${i}].code.line 必须为正整数`
      if ((s.code as RawTable).col !== undefined && (typeof (s.code as RawTable).col !== 'number' || (s.code as RawTable).col as number < 1)) return `steps[${i}].code.col 必须为正整数`
    }

    if (s.highlights !== undefined) {
      if (!Array.isArray(s.highlights)) return `steps[${i}].highlights 必须为数组`
      const hlErr = validateRawHighlights(s.highlights as RawTable[], i, kind)
      if (hlErr) return hlErr
    }
  }
  return ''
}

function validateRawHighlights(highlights: RawTable[], stepIdx: number, kind: string): string {
  const allowed = HIGHLIGHT_KINDS_BY_OBJECT[kind]
  if (!allowed) return ''

  for (let j = 0; j < highlights.length; j += 1) {
    const h = highlights[j]
    const ctx = `steps[${stepIdx}].highlights[${j}]`
    if (typeof h.kind !== 'string') return `${ctx}.kind 缺失`

    if (!allowed.has(h.kind as string)) {
      return `${ctx}.kind "${String(h.kind)}" 不适用于 object.kind "${kind}"`
    }

    if (h.kind === 'node' || h.kind === 'item') {
      const err = checkKeys(h, ['kind', 'target', 'style'], ['level'], ctx)
      if (err) return err
      if (typeof h.target !== 'number') return `${ctx}.target 必须为整数`
    } else if (h.kind === 'edge') {
      const err = checkKeys(h, ['kind', 'from', 'to', 'style'], ['level'], ctx)
      if (err) return err
      if (typeof h.from !== 'number' || typeof h.to !== 'number') return `${ctx}.from/to 必须为整数`
      if (kind === 'graph_undirected' && (h.from as number) >= (h.to as number)) return `${ctx} 无向图边高亮必须满足 from < to`
      if (kind !== 'graph_undirected' && (h.from as number) === (h.to as number)) return `${ctx} 边高亮 from 不能等于 to`
    } else {
      return `${ctx}.kind 取值不合法: ${String(h.kind)}`
    }

    if (typeof h.style !== 'string' || !STYLE_RE.test(h.style as string) || (h.style as string).length === 0 || (h.style as string).length > 32) {
      return `${ctx}.style 格式不合法`
    }
    if (h.level !== undefined && (typeof h.level !== 'number' || h.level < 1 || h.level > 9 || !Number.isInteger(h.level))) {
      return `${ctx}.level 必须为 1..9 的整数`
    }
  }
  return ''
}

function validateRawResult(result: RawTable | undefined): string {
  if (result === undefined || result === null) return ''
  if (typeof result !== 'object') return '[result] 格式错误'
  const err = checkKeys(result, ['final_state'], ['commit'], '[result]')
  if (err) return err
  if (typeof result.final_state !== 'number' || !Number.isFinite(result.final_state)) return 'result.final_state 无效'
  if (result.commit !== undefined) {
    if (typeof result.commit !== 'object' || result.commit === null) return 'result.commit 格式错误'
    const cErr = checkKeys(result.commit as RawTable, ['op', 'line'], [], 'result.commit')
    if (cErr) return cErr
    const commit = result.commit as RawTable
    if (typeof commit.op !== 'string' || !ASCII_ID_RE.test(commit.op as string) || (commit.op as string).length === 0 || (commit.op as string).length > 64) return 'result.commit.op 格式不合法'
    if (typeof commit.line !== 'number' || commit.line < 1) return 'result.commit.line 必须为正整数'
  }
  return ''
}

function validateRawError(error: RawTable | undefined): string {
  if (error === undefined || error === null) return ''
  if (typeof error !== 'object') return '[error] 格式错误'
  const err = checkKeys(error, ['type', 'message'], ['line', 'step', 'last_state'], '[error]')
  if (err) return err
  if (typeof error.type !== 'string' || !ALLOWED_ERROR_TYPES.has(error.type as string)) return 'error.type 取值不合法'
  if (typeof error.message !== 'string' || error.message.length === 0 || error.message.length > 512 || (error.message as string).includes('\n')) return 'error.message 格式不合法'
  if (error.line !== undefined && (typeof error.line !== 'number' || error.line < 1)) return 'error.line 必须为正整数'
  return ''
}

/* ================================================================
 *  语义校验（基于映射后的文档）
 * ================================================================ */

function validateSemantics(doc: IrDocument): string {
  if (doc.result !== undefined && doc.error !== undefined) return 'IR 同时存在 result 与 error'
  if (doc.result === undefined && doc.error === undefined) return 'IR 缺少 result 或 error'

  if (doc.result !== undefined) {
    if (doc.result.finalState < 0 || doc.result.finalState >= doc.states.length) return 'result.final_state 超出 states 范围'
  }

  if (doc.error !== undefined) {
    if (doc.error.step !== undefined) {
      const steps = doc.steps ?? []
      if (!steps.some((s) => s.id === doc.error!.step)) return 'error.step 未引用有效 steps.id'
    }
    if (doc.error.lastState !== undefined) {
      if (doc.error.lastState < 0 || doc.error.lastState >= doc.states.length) return 'error.last_state 未引用有效 states.id'
    }
  }

  const steps = doc.steps ?? []
  for (let i = 0; i < steps.length; i += 1) {
    const step = steps[i]
    if (step.before < 0 || step.before >= doc.states.length) return `steps[${i}].before 未引用有效 states.id`
    if (step.after !== undefined) {
      if (step.after < 0 || step.after >= doc.states.length) return `steps[${i}].after 未引用有效 states.id`
    } else if (!doc.error || doc.error.step !== step.id) {
      return `steps[${i}].after 缺失时必须存在 error.step`
    }

    if (step.highlights !== undefined && step.highlights.length > 0) {
      const assocIdx = step.after ?? step.before
      const state = doc.states[assocIdx]
      if (state) {
        const hlErr = validateHighlightRefs(step.highlights, state.data, doc.object.kind, i)
        if (hlErr) return hlErr
      }
    }
  }

  return ''
}

function validateHighlightRefs(
  highlights: IrHighlight[],
  data: IrStateData,
  kind: IrObjectKind,
  stepIdx: number,
): string {
  for (let j = 0; j < highlights.length; j += 1) {
    const h = highlights[j]
    const ctx = `steps[${stepIdx}].highlights[${j}]`

    if (h.kind === 'item') {
      const items = (data as StackStateData | QueueStateData).items
      if (!items || h.target < 0 || h.target >= items.length) {
        return `${ctx}.target 索引 ${h.target} 超出 items 范围`
      }
    } else if (h.kind === 'node') {
      const nodes = (data as { nodes?: Array<{ id: number }> }).nodes
      if (!nodes || !nodes.some((n) => n.id === h.target)) {
        return `${ctx}.target 引用不存在的节点 ${h.target}`
      }
    } else if (h.kind === 'edge') {
      const edges = (data as { edges?: Array<{ from: number; to: number }> }).edges
      if (!edges || !edges.some((e) => e.from === h.from && e.to === h.to)) {
        return `${ctx} 引用不存在的边 (${h.from}, ${h.to})`
      }
    }
  }
  return ''
}

/* ================================================================
 *  映射函数
 * ================================================================ */

function mapDocument(raw: RawTable): IrDocument {
  return {
    meta: mapMeta(raw.meta as RawTable),
    package: mapPackage(raw.package as RawTable),
    remarks: mapRemarks(raw.remarks as RawTable | undefined),
    object: mapObject(raw.object as RawTable),
    states: mapStates(raw.states as RawTable[]),
    steps: mapSteps(raw.steps as RawTable[] | undefined),
    result: mapResult(raw.result as RawTable | undefined),
    error: mapError(raw.error as RawTable | undefined),
  }
}

function mapMeta(meta: RawTable): IrMeta {
  return {
    createdAt: String(meta.created_at),
    lang: String(meta.lang),
    langVersion: String(meta.lang_version),
  }
}

function mapPackage(pkg: RawTable): IrPackage {
  return {
    name: String(pkg.name),
    lang: String(pkg.lang),
    version: String(pkg.version),
  }
}

function mapRemarks(raw: RawTable | undefined): IrRemarks | undefined {
  if (!raw) return undefined
  return {
    title: raw.title !== undefined ? String(raw.title) : undefined,
    author: raw.author !== undefined ? String(raw.author) : undefined,
    comment: raw.comment !== undefined ? String(raw.comment) : undefined,
  }
}

function mapObject(raw: RawTable): IrObject {
  return {
    kind: String(raw.kind) as IrObjectKind,
    label: raw.label !== undefined ? String(raw.label) : undefined,
  }
}

function mapStates(raw: RawTable[]): IrState[] {
  return raw.map((s) => ({
    id: Number(s.id),
    data: s.data as IrStateData,
  }))
}

function mapSteps(raw: RawTable[] | undefined): IrStep[] | undefined {
  if (!raw) return undefined
  return raw.map((s) => {
    const code = s.code as RawTable | undefined
    return {
      id: Number(s.id),
      op: String(s.op),
      phase: s.phase !== undefined ? String(s.phase) : undefined,
      before: Number(s.before),
      after: s.after !== undefined ? Number(s.after) : undefined,
      ret: s.ret as IrStep['ret'],
      note: s.note !== undefined ? String(s.note) : undefined,
      args: (s.args ?? {}) as IrStep['args'],
      code: code ? { line: Number(code.line), col: code.col !== undefined ? Number(code.col) : undefined } : undefined,
      highlights: mapHighlights(s.highlights as RawTable[] | undefined),
    }
  })
}

function mapHighlights(raw: RawTable[] | undefined): IrHighlight[] | undefined {
  if (!raw || raw.length === 0) return undefined
  return raw.map((h) => {
    const base = { style: String(h.style), level: h.level !== undefined ? Number(h.level) : undefined }
    if (h.kind === 'edge') {
      return { kind: 'edge' as const, from: Number(h.from), to: Number(h.to), ...base }
    }
    return { kind: String(h.kind) as 'node' | 'item', target: Number(h.target), ...base }
  })
}

function mapResult(raw: RawTable | undefined): IrResult | undefined {
  if (!raw) return undefined
  const commit = raw.commit as RawTable | undefined
  return {
    finalState: Number(raw.final_state),
    commit: commit ? { op: String(commit.op), line: Number(commit.line) } : undefined,
  }
}

function mapError(raw: RawTable | undefined): IrError | undefined {
  if (!raw) return undefined
  return {
    type: String(raw.type) as IrError['type'],
    message: String(raw.message),
    line: raw.line !== undefined ? Number(raw.line) : undefined,
    step: raw.step !== undefined ? Number(raw.step) : undefined,
    lastState: raw.last_state !== undefined ? Number(raw.last_state) : undefined,
  }
}

/* ================================================================
 *  公开 API
 * ================================================================ */

/**
 * 解析 TOML IR 内容
 *
 * @param content - TOML 字符串
 * @returns 解析结果
 */
export function parseIrToml(content: string): ParseIrResult {
  try {
    logDebug('[ir] parsing TOML, length =', content.length)
    const raw = toml.parse(content) as RawTable

    const rawError = validateRaw(raw)
    if (rawError) {
      logDebug('[ir] raw validation failed:', rawError)
      return { ok: false, errorMessage: rawError }
    }

    const document = mapDocument(raw)

    const semanticError = validateSemantics(document)
    if (semanticError) {
      logDebug('[ir] semantic validation failed:', semanticError)
      return { ok: false, errorMessage: semanticError }
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
