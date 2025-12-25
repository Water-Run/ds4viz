/**
 * 会话管理模块，提供上下文管理和全局配置
 *
 * @file src/session.ts
 * @author WaterRun
 * @date 2025-12-25
 */

import {
    type Trace,
    type State,
    type Step,
    type StateData,
    type StepArgs,
    type CodeLocation,
    type Result,
    type TraceError,
    createMeta,
    createPackage,
    createRemarks,
    createTraceObject,
    createState,
    createStep,
    createResult,
    createTraceError,
    createTrace,
} from './trace.js';
import { TomlWriter } from './writer.js';

/**
 * 全局配置选项
 *
 * @interface
 */
export interface ConfigOptions {
    /** 输出文件路径 */
    outputPath?: string;
    /** 可视化标题 */
    title?: string;
    /** 作者信息 */
    author?: string;
    /** 注释说明 */
    comment?: string;
}

/**
 * 全局配置数据
 *
 * @interface
 */
interface GlobalConfigData {
    outputPath: string;
    title: string;
    author: string;
    comment: string;
}

/**
 * 错误类型枚举
 *
 * @enum
 */
export enum ErrorType {
    Runtime = 'runtime',
    Timeout = 'timeout',
    Validation = 'validation',
    Sandbox = 'sandbox',
    Unknown = 'unknown',
}

/**
 * ds4viz 库的基础异常类
 */
export class DS4VizError extends Error {
    /** 错误类型 */
    readonly errorType: ErrorType;
    /** 发生错误的代码行号 */
    readonly line: number | undefined;

    /**
     * 构造函数
     *
     * @param message - 错误消息
     * @param errorType - 错误类型
     * @param line - 发生错误的代码行号
     */
    constructor(message: string, errorType: ErrorType = ErrorType.Runtime, line?: number) {
        super(message);
        this.name = 'DS4VizError';
        this.errorType = errorType;
        this.line = line;
    }
}

/**
 * 数据结构操作错误
 */
export class StructureError extends DS4VizError {
    /**
     * 构造函数
     *
     * @param message - 错误消息
     * @param line - 发生错误的代码行号
     */
    constructor(message: string, line?: number) {
        super(message, ErrorType.Runtime, line);
        this.name = 'StructureError';
    }
}

/**
 * 数据验证错误
 */
export class ValidationError extends DS4VizError {
    /**
     * 构造函数
     *
     * @param message - 错误消息
     * @param line - 发生错误的代码行号
     */
    constructor(message: string, line?: number) {
        super(message, ErrorType.Validation, line);
        this.name = 'ValidationError';
    }
}

/** 全局配置实例 */
let globalConfig: GlobalConfigData = {
    outputPath: 'trace.toml',
    title: '',
    author: '',
    comment: '',
};

/**
 * 配置全局参数
 *
 * @param options - 配置选项
 */
export function config(options: ConfigOptions): void {
    globalConfig = {
        outputPath: options.outputPath ?? 'trace.toml',
        title: options.title ?? '',
        author: options.author ?? '',
        comment: options.comment ?? '',
    };
}

/**
 * 获取全局配置
 *
 * @returns 当前全局配置
 */
export function getConfig(): Readonly<GlobalConfigData> {
    return globalConfig;
}

/**
 * 获取当前 TypeScript/Node.js 版本字符串
 *
 * @returns 版本字符串
 */
export function getTypeScriptVersion(): string {
    return process.version.replace('v', '');
}

/**
 * 获取调用者的代码行号
 *
 * @param depth - 调用栈深度
 * @returns 调用者所在行号
 */
export function getCallerLine(depth: number = 2): number {
    const error = new Error();
    const stack = error.stack;
    if (stack === undefined) {
        return 1;
    }
    const lines = stack.split('\n');
    const targetLine = lines[depth + 1];
    if (targetLine === undefined) {
        return 1;
    }
    const match = /:(\d+):\d+\)?$/.exec(targetLine);
    if (match?.[1] !== undefined) {
        return parseInt(match[1], 10);
    }
    return 1;
}

/**
 * 会话管理器，用于管理数据结构的操作轨迹记录
 */
export class Session {
    readonly #kind: string;
    readonly #label: string;
    readonly #output: string;
    readonly #states: State[];
    readonly #steps: Step[];
    #stateCounter: number;
    #stepCounter: number;
    #error: TraceError | undefined;
    #exitLine: number;
    #failedStepId: number | undefined;

    /**
     * 构造函数
     *
     * @param kind - 数据结构类型
     * @param label - 数据结构标签
     * @param output - 输出文件路径
     */
    constructor(kind: string, label: string, output?: string) {
        this.#kind = kind;
        this.#label = label;
        this.#output = output ?? globalConfig.outputPath;
        this.#states = [];
        this.#steps = [];
        this.#stateCounter = 0;
        this.#stepCounter = 0;
        this.#error = undefined;
        this.#exitLine = 1;
        this.#failedStepId = undefined;
    }

    /**
     * 获取失败步骤 ID
     */
    get failedStepId(): number | undefined {
        return this.#failedStepId;
    }

    /**
     * 设置失败步骤 ID
     */
    set failedStepId(value: number | undefined) {
        this.#failedStepId = value;
    }

    /**
     * 获取当前步骤计数器
     */
    get stepCounter(): number {
        return this.#stepCounter;
    }

    /**
     * 添加状态快照
     *
     * @param data - 状态数据
     * @returns 新状态的 ID
     */
    public addState(data: StateData): number {
        const stateId: number = this.#stateCounter;
        this.#states.push(createState(stateId, data));
        this.#stateCounter += 1;
        return stateId;
    }

    /**
     * 添加操作步骤
     *
     * @param op - 操作名称
     * @param before - 操作前状态 ID
     * @param after - 操作后状态 ID
     * @param args - 操作参数
     * @param ret - 返回值
     * @param note - 备注
     * @param line - 代码行号
     * @returns 新步骤的 ID
     */
    public addStep(
        op: string,
        before: number,
        after: number | undefined,
        args: StepArgs,
        ret?: unknown,
        note?: string,
        line?: number
    ): number {
        const stepId: number = this.#stepCounter;
        const codeLoc: CodeLocation | undefined = line !== undefined ? { line } : undefined;
        this.#steps.push(createStep(stepId, op, before, after, args, codeLoc, ret, note));
        this.#stepCounter += 1;
        return stepId;
    }

    /**
     * 设置错误信息
     *
     * @param errorType - 错误类型
     * @param message - 错误消息
     * @param line - 错误发生的代码行号
     * @param stepId - 发生错误的步骤 ID
     */
    public setError(
        errorType: ErrorType,
        message: string,
        line?: number,
        stepId?: number
    ): void {
        const lastState: number | undefined =
            this.#states.length > 0 ? this.#states[this.#states.length - 1]?.id : undefined;
        this.#error = createTraceError(
            errorType,
            message.slice(0, 512),
            line,
            stepId,
            lastState
        );
        this.#failedStepId = stepId;
    }

    /**
     * 设置上下文退出行号
     *
     * @param line - 退出行号
     */
    public setExitLine(line: number): void {
        this.#exitLine = line;
    }

    /**
     * 获取最后一个状态的 ID
     *
     * @returns 最后一个状态的 ID，若无状态则返回 -1
     */
    public getLastStateId(): number {
        if (this.#states.length === 0) {
            return -1;
        }
        const lastState = this.#states[this.#states.length - 1];
        return lastState?.id ?? -1;
    }

    /**
     * 构建完整的 Trace 对象
     *
     * @returns 完整的 Trace 对象
     */
    public buildTrace(): Trace {
        const now: string = new Date().toISOString().replace(/\.\d{3}Z$/, 'Z');
        const meta = createMeta(now, getTypeScriptVersion());
        const pkg = createPackage();
        const remarks = createRemarks(
            globalConfig.title,
            globalConfig.author,
            globalConfig.comment
        );
        const obj = createTraceObject(this.#kind, this.#label);

        let result: Result | undefined;
        let error: TraceError | undefined;

        if (this.#error !== undefined) {
            error = this.#error;
        } else {
            const finalStateId: number =
                this.#states.length > 0
                    ? (this.#states[this.#states.length - 1]?.id ?? 0)
                    : 0;
            result = createResult(finalStateId, { op: 'commit', line: this.#exitLine });
        }

        return createTrace(meta, pkg, remarks, obj, this.#states, this.#steps, result, error);
    }

    /**
     * 将 Trace 写入文件
     */
    public write(): void {
        const trace: Trace = this.buildTrace();
        const writer = new TomlWriter(trace);
        writer.write(this.#output);
    }
}

/**
 * 数据结构选项接口
 *
 * @interface
 */
export interface StructureOptions {
    /** 数据结构的标签 */
    label?: string;
    /** 输出文件路径 */
    output?: string;
}

/**
 * 可释放资源接口
 *
 * @interface
 */
export interface Disposable {
    [Symbol.dispose](): void;
}

/**
 * 上下文管理器基类，用于管理数据结构的生命周期
 */
export abstract class ContextManager implements Disposable {
    protected readonly session: Session;
    #caughtError: Error | undefined;

    /**
     * 构造函数
     *
     * @param kind - 数据结构类型
     * @param label - 数据结构标签
     * @param output - 输出文件路径
     */
    constructor(kind: string, label: string, output?: string) {
        this.session = new Session(kind, label, output);
        this.#caughtError = undefined;
    }

    /**
     * 进入上下文
     */
    public enter(): void {
        this.initialize();
    }

    /**
     * 设置捕获的错误
     *
     * @param error - 捕获的错误
     */
    public setCaughtError(error: Error): void {
        this.#caughtError = error;
    }

    /**
     * 释放资源（实现 Symbol.dispose）
     */
    [Symbol.dispose](): void {
        this.dispose();
    }

    /**
     * 释放资源
     */
    public dispose(): void {
        this.session.setExitLine(getCallerLine(3));

        if (this.#caughtError !== undefined) {
            let errorLine: number | undefined;
            let errorType: ErrorType = ErrorType.Unknown;
            const message: string = this.#caughtError.message;

            if (this.#caughtError instanceof DS4VizError) {
                errorType = this.#caughtError.errorType;
                errorLine = this.#caughtError.line;
            }

            this.session.setError(
                errorType,
                message,
                errorLine,
                this.session.failedStepId
            );
        }

        this.session.write();
    }

    /**
     * 初始化数据结构，子类需要重写此方法添加初始状态
     */
    protected abstract initialize(): void;

    /**
     * 构建当前状态的数据字典
     *
     * @returns 状态数据字典
     */
    protected abstract buildData(): StateData;

    /**
     * 抛出数据结构错误
     *
     * @param message - 错误消息
     * @throws {StructureError} 始终抛出
     */
    protected raiseError(message: string): never {
        const line: number = getCallerLine(3);
        const stepId: number = this.session.stepCounter;
        this.session.failedStepId = stepId;
        const error = new StructureError(message, line);
        this.#caughtError = error;
        throw error;
    }

    /**
     * 记录操作步骤并添加新状态
     *
     * @param op - 操作名称
     * @param args - 操作参数
     * @param ret - 返回值
     * @param note - 备注
     */
    protected recordStep(
        op: string,
        args: StepArgs,
        ret?: unknown,
        note?: string
    ): void {
        const before: number = this.session.getLastStateId();
        const line: number = getCallerLine(3);
        const newData: StateData = this.buildData();
        const after: number = this.session.addState(newData);
        this.session.addStep(op, before, after, args, ret, note, line);
    }
}

/**
 * 使用上下文管理数据结构的生命周期
 *
 * @template T - 数据结构类型
 * @param structure - 数据结构实例
 * @param callback - 回调函数
 *
 * @example
 * ```typescript
 * await withContext(stack(), async (s) => {
 *     s.push(1);
 *     s.push(2);
 * });
 * ```
 */
export async function withContext<T extends ContextManager>(
    structure: T,
    callback: (structure: T) => void | Promise<void>
): Promise<void> {
    structure.enter();
    try {
        await callback(structure);
    } catch (error) {
        if (error instanceof Error) {
            structure.setCaughtError(error);
        }
        throw error;
    } finally {
        structure.dispose();
    }
}