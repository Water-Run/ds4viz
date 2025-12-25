/**
 * Trace 数据结构定义，用于记录操作轨迹
 *
 * @file src/trace.ts
 * @author WaterRun
 * @date 2025-12-25
 */

/**
 * 元数据信息
 *
 * @interface
 */
export interface Meta {
    readonly createdAt: string;
    readonly lang: string;
    readonly langVersion: string;
}

/**
 * 包信息
 *
 * @interface
 */
export interface Package {
    readonly name: string;
    readonly lang: string;
    readonly version: string;
}

/**
 * 备注信息
 *
 * @interface
 */
export interface Remarks {
    readonly title: string;
    readonly author: string;
    readonly comment: string;
}

/**
 * 数据结构对象描述
 *
 * @interface
 */
export interface TraceObject {
    readonly kind: string;
    readonly label: string;
}

/**
 * 状态数据类型
 *
 * @typedef
 */
export type StateData = Record<string, unknown>;

/**
 * 状态快照
 *
 * @interface
 */
export interface State {
    readonly id: number;
    readonly data: StateData;
}

/**
 * 代码位置信息
 *
 * @interface
 */
export interface CodeLocation {
    readonly line: number;
    readonly col?: number;
}

/**
 * 步骤参数类型
 *
 * @typedef
 */
export type StepArgs = Record<string, unknown>;

/**
 * 操作步骤
 *
 * @interface
 */
export interface Step {
    readonly id: number;
    readonly op: string;
    readonly before: number;
    readonly after?: number;
    readonly args: StepArgs;
    readonly code?: CodeLocation;
    readonly ret?: unknown;
    readonly note?: string;
}

/**
 * 提交信息
 *
 * @interface
 */
export interface Commit {
    readonly op: string;
    readonly line: number;
}

/**
 * 成功结果
 *
 * @interface
 */
export interface Result {
    readonly finalState: number;
    readonly commit?: Commit;
}

/**
 * 错误信息
 *
 * @interface
 */
export interface TraceError {
    readonly type: string;
    readonly message: string;
    readonly line?: number;
    readonly step?: number;
    readonly lastState?: number;
}

/**
 * 完整的操作轨迹记录
 *
 * @interface
 */
export interface Trace {
    readonly meta: Meta;
    readonly package: Package;
    readonly remarks: Remarks;
    readonly object: TraceObject;
    readonly states: readonly State[];
    readonly steps: readonly Step[];
    readonly result?: Result;
    readonly error?: TraceError;
}

/**
 * 创建默认的 Meta 对象
 *
 * @param createdAt - 创建时间
 * @param langVersion - 语言版本
 * @returns Meta 对象
 */
export function createMeta(createdAt: string, langVersion: string): Meta {
    return {
        createdAt,
        lang: 'typescript',
        langVersion,
    };
}

/**
 * 创建默认的 Package 对象
 *
 * @returns Package 对象
 */
export function createPackage(): Package {
    return {
        name: 'ds4viz',
        lang: 'typescript',
        version: '1.0.0',
    };
}

/**
 * 创建 Remarks 对象
 *
 * @param title - 标题
 * @param author - 作者
 * @param comment - 注释
 * @returns Remarks 对象
 */
export function createRemarks(title: string, author: string, comment: string): Remarks {
    return { title, author, comment };
}

/**
 * 检查备注是否为空
 *
 * @param remarks - 备注对象
 * @returns 如果所有字段都为空则返回 true
 */
export function isRemarksEmpty(remarks: Remarks): boolean {
    return remarks.title === '' && remarks.author === '' && remarks.comment === '';
}

/**
 * 创建 TraceObject 对象
 *
 * @param kind - 数据结构类型
 * @param label - 标签
 * @returns TraceObject 对象
 */
export function createTraceObject(kind: string, label: string): TraceObject {
    return { kind, label };
}

/**
 * 创建 State 对象
 *
 * @param id - 状态 ID
 * @param data - 状态数据
 * @returns State 对象
 */
export function createState(id: number, data: StateData): State {
    return { id, data };
}

/**
 * 创建 Step 对象
 *
 * @param id - 步骤 ID
 * @param op - 操作名称
 * @param before - 操作前状态 ID
 * @param after - 操作后状态 ID
 * @param args - 操作参数
 * @param code - 代码位置
 * @param ret - 返回值
 * @param note - 备注
 * @returns Step 对象
 */
export function createStep(
    id: number,
    op: string,
    before: number,
    after: number | undefined,
    args: StepArgs,
    code?: CodeLocation,
    ret?: unknown,
    note?: string
): Step {
    const step: Step = {
        id,
        op,
        before,
        after,
        args,
        code,
        ret,
        note,
    };
    return step;
}

/**
 * 创建 Result 对象
 *
 * @param finalState - 最终状态 ID
 * @param commit - 提交信息
 * @returns Result 对象
 */
export function createResult(finalState: number, commit?: Commit): Result {
    return { finalState, commit };
}

/**
 * 创建 TraceError 对象
 *
 * @param type - 错误类型
 * @param message - 错误消息
 * @param line - 错误行号
 * @param step - 错误步骤 ID
 * @param lastState - 最后状态 ID
 * @returns TraceError 对象
 */
export function createTraceError(
    type: string,
    message: string,
    line?: number,
    step?: number,
    lastState?: number
): TraceError {
    return { type, message, line, step, lastState };
}

/**
 * 创建完整的 Trace 对象
 *
 * @param meta - 元数据
 * @param pkg - 包信息
 * @param remarks - 备注
 * @param object - 对象描述
 * @param states - 状态列表
 * @param steps - 步骤列表
 * @param result - 成功结果
 * @param error - 错误信息
 * @returns Trace 对象
 */
export function createTrace(
    meta: Meta,
    pkg: Package,
    remarks: Remarks,
    object: TraceObject,
    states: readonly State[],
    steps: readonly Step[],
    result?: Result,
    error?: TraceError
): Trace {
    return {
        meta,
        package: pkg,
        remarks,
        object,
        states,
        steps,
        result,
        error,
    };
}
