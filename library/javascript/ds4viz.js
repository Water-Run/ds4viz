/**
 * ds4viz - 可扩展的数据结构可视化教学平台 JavaScript 库
 *
 * 提供数据结构的可视化追踪功能，生成标准化的 TOML 中间表示文件。
 * 支持浏览器和 Node.js 双环境运行。
 *
 * @file ds4viz.js
 * @author WaterRun
 * @date 2025-12-25
 */

'use strict';

// ============================================================================
// 环境检测
// ============================================================================

/**
 * 检测是否为 Node.js 环境
 *
 * @returns {boolean} 是否为 Node.js 环境
 */
function isNodeEnvironment() {
    return typeof process !== 'undefined' &&
        process.versions != null &&
        process.versions.node != null;
}

/**
 * 检测是否为浏览器环境
 *
 * @returns {boolean} 是否为浏览器环境
 */
function isBrowserEnvironment() {
    return typeof window !== 'undefined' && typeof document !== 'undefined';
}

// ============================================================================
// 错误类型定义
// ============================================================================

/**
 * 错误类型枚举
 *
 * @enum {string}
 * @readonly
 */
const ErrorType = Object.freeze({
    Runtime: 'runtime',
    Timeout: 'timeout',
    Validation: 'validation',
    Sandbox: 'sandbox',
    Unknown: 'unknown',
});

/**
 * ds4viz 库的基础异常类
 */
class DS4VizError extends Error {
    /**
     * 构造函数
     *
     * @param {string} message - 错误消息
     * @param {string} [errorType="runtime"] - 错误类型
     * @param {number} [line] - 发生错误的代码行号
     */
    constructor(message, errorType = ErrorType.Runtime, line = undefined) {
        super(message);
        this.name = 'DS4VizError';
        /** @type {string} */
        this.errorType = errorType;
        /** @type {number|undefined} */
        this.line = line;
    }
}

/**
 * 数据结构操作错误
 */
class StructureError extends DS4VizError {
    /**
     * 构造函数
     *
     * @param {string} message - 错误消息
     * @param {number} [line] - 发生错误的代码行号
     */
    constructor(message, line = undefined) {
        super(message, ErrorType.Runtime, line);
        this.name = 'StructureError';
    }
}

/**
 * 数据验证错误
 */
class ValidationError extends DS4VizError {
    /**
     * 构造函数
     *
     * @param {string} message - 错误消息
     * @param {number} [line] - 发生错误的代码行号
     */
    constructor(message, line = undefined) {
        super(message, ErrorType.Validation, line);
        this.name = 'ValidationError';
    }
}

// ============================================================================
// 全局配置
// ============================================================================

/**
 * 全局配置数据
 *
 * @typedef {Object} GlobalConfigData
 * @property {string} outputPath - 输出文件路径
 * @property {string} title - 可视化标题
 * @property {string} author - 作者信息
 * @property {string} comment - 注释说明
 */

/** @type {GlobalConfigData} */
let globalConfig = {
    outputPath: 'trace.toml',
    title: '',
    author: '',
    comment: '',
};

/**
 * 配置全局参数
 *
 * @param {Object} options - 配置选项
 * @param {string} [options.outputPath="trace.toml"] - 输出文件路径
 * @param {string} [options.title=""] - 可视化标题
 * @param {string} [options.author=""] - 作者信息
 * @param {string} [options.comment=""] - 注释说明
 * @returns {void}
 */
function config(options) {
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
 * @returns {Readonly<GlobalConfigData>} 当前全局配置
 */
function getConfig() {
    return globalConfig;
}

// ============================================================================
// 工具函数
// ============================================================================

/**
 * 获取当前 JavaScript 版本字符串
 *
 * @returns {string} 版本字符串
 */
function getJavaScriptVersion() {
    if (isNodeEnvironment()) {
        return process.version.replace('v', '');
    }
    return 'ES2022';
}

/**
 * 获取调用者的代码行号
 *
 * @param {number} [depth=2] - 调用栈深度
 * @returns {number} 调用者所在行号
 */
function getCallerLine(depth = 2) {
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
 * 从参数列表中提取注入的行号
 *
 * @param {IArguments|Array} args - 原始参数数组
 * @returns {{ line: number|null, args: Array }} - 行号和清理后的参数
 */
function extractLineFromArgs(args) {
    const argsArray = Array.from(args);
    if (argsArray.length === 0) {
        return { line: null, args: [] };
    }

    const lastArg = argsArray[argsArray.length - 1];

    if (lastArg && typeof lastArg === 'object' && lastArg !== null && '__line' in lastArg) {
        return {
            line: lastArg.__line,
            args: argsArray.slice(0, -1)
        };
    }

    return { line: null, args: argsArray };
}

// ============================================================================
// Trace 数据结构定义
// ============================================================================

/**
 * 元数据信息
 *
 * @typedef {Object} Meta
 * @property {string} createdAt - 创建时间
 * @property {string} lang - 语言
 * @property {string} langVersion - 语言版本
 */

/**
 * 包信息
 *
 * @typedef {Object} Package
 * @property {string} name - 包名
 * @property {string} lang - 语言
 * @property {string} version - 版本
 */

/**
 * 备注信息
 *
 * @typedef {Object} Remarks
 * @property {string} title - 标题
 * @property {string} author - 作者
 * @property {string} comment - 注释
 */

/**
 * 数据结构对象描述
 *
 * @typedef {Object} TraceObject
 * @property {string} kind - 数据结构类型
 * @property {string} label - 标签
 */

/**
 * 状态数据类型
 *
 * @typedef {Object.<string, *>} StateData
 */

/**
 * 状态快照
 *
 * @typedef {Object} State
 * @property {number} id - 状态 ID
 * @property {StateData} data - 状态数据
 */

/**
 * 代码位置信息
 *
 * @typedef {Object} CodeLocation
 * @property {number} line - 行号
 * @property {number} [col] - 列号
 */

/**
 * 步骤参数类型
 *
 * @typedef {Object.<string, *>} StepArgs
 */

/**
 * 操作步骤
 *
 * @typedef {Object} Step
 * @property {number} id - 步骤 ID
 * @property {string} op - 操作名称
 * @property {number} before - 操作前状态 ID
 * @property {number} [after] - 操作后状态 ID
 * @property {StepArgs} args - 操作参数
 * @property {CodeLocation} [code] - 代码位置
 * @property {*} [ret] - 返回值
 * @property {string} [note] - 备注
 */

/**
 * 提交信息
 *
 * @typedef {Object} Commit
 * @property {string} op - 操作名称
 * @property {number} line - 行号
 */

/**
 * 成功结果
 *
 * @typedef {Object} Result
 * @property {number} finalState - 最终状态 ID
 * @property {Commit} [commit] - 提交信息
 */

/**
 * 错误信息
 *
 * @typedef {Object} TraceError
 * @property {string} type - 错误类型
 * @property {string} message - 错误消息
 * @property {number} [line] - 错误行号
 * @property {number} [step] - 错误步骤 ID
 * @property {number} [lastState] - 最后状态 ID
 */

/**
 * 完整的操作轨迹记录
 *
 * @typedef {Object} Trace
 * @property {Meta} meta - 元数据
 * @property {Package} package - 包信息
 * @property {Remarks} remarks - 备注
 * @property {TraceObject} object - 对象描述
 * @property {State[]} states - 状态列表
 * @property {Step[]} steps - 步骤列表
 * @property {Result} [result] - 成功结果
 * @property {TraceError} [error] - 错误信息
 */

/**
 * 创建默认的 Meta 对象
 *
 * @param {string} createdAt - 创建时间
 * @param {string} langVersion - 语言版本
 * @returns {Meta} Meta 对象
 */
function createMeta(createdAt, langVersion) {
    return {
        createdAt,
        lang: 'javascript',
        langVersion,
    };
}

/**
 * 创建默认的 Package 对象
 *
 * @returns {Package} Package 对象
 */
function createPackage() {
    return {
        name: 'ds4viz',
        lang: 'javascript',
        version: '0.1.0',
    };
}

/**
 * 创建 Remarks 对象
 *
 * @param {string} title - 标题
 * @param {string} author - 作者
 * @param {string} comment - 注释
 * @returns {Remarks} Remarks 对象
 */
function createRemarks(title, author, comment) {
    return { title, author, comment };
}

/**
 * 检查备注是否为空
 *
 * @param {Remarks} remarks - 备注对象
 * @returns {boolean} 如果所有字段都为空则返回 true
 */
function isRemarksEmpty(remarks) {
    return remarks.title === '' && remarks.author === '' && remarks.comment === '';
}

/**
 * 创建 TraceObject 对象
 *
 * @param {string} kind - 数据结构类型
 * @param {string} label - 标签
 * @returns {TraceObject} TraceObject 对象
 */
function createTraceObject(kind, label) {
    return { kind, label };
}

/**
 * 创建 State 对象
 *
 * @param {number} id - 状态 ID
 * @param {StateData} data - 状态数据
 * @returns {State} State 对象
 */
function createState(id, data) {
    return { id, data };
}

/**
 * 创建 Step 对象
 *
 * @param {number} id - 步骤 ID
 * @param {string} op - 操作名称
 * @param {number} before - 操作前状态 ID
 * @param {number|undefined} after - 操作后状态 ID
 * @param {StepArgs} args - 操作参数
 * @param {CodeLocation} [code] - 代码位置
 * @param {*} [ret] - 返回值
 * @param {string} [note] - 备注
 * @returns {Step} Step 对象
 */
function createStep(id, op, before, after, args, code, ret, note) {
    const step = {
        id,
        op,
        before,
        args,
    };
    if (after !== undefined) {
        step.after = after;
    }
    if (code !== undefined) {
        step.code = code;
    }
    if (ret !== undefined) {
        step.ret = ret;
    }
    if (note !== undefined) {
        step.note = note;
    }
    return step;
}

/**
 * 创建 Result 对象
 *
 * @param {number} finalState - 最终状态 ID
 * @param {Commit} [commit] - 提交信息
 * @returns {Result} Result 对象
 */
function createResult(finalState, commit) {
    if (commit !== undefined) {
        return { finalState, commit };
    }
    return { finalState };
}

/**
 * 创建 TraceError 对象
 *
 * @param {string} type - 错误类型
 * @param {string} message - 错误消息
 * @param {number} [line] - 错误行号
 * @param {number} [step] - 错误步骤 ID
 * @param {number} [lastState] - 最后状态 ID
 * @returns {TraceError} TraceError 对象
 */
function createTraceError(type, message, line, step, lastState) {
    const error = { type, message };
    if (line !== undefined) {
        error.line = line;
    }
    if (step !== undefined) {
        error.step = step;
    }
    if (lastState !== undefined) {
        error.lastState = lastState;
    }
    return error;
}

/**
 * 创建完整的 Trace 对象
 *
 * @param {Meta} meta - 元数据
 * @param {Package} pkg - 包信息
 * @param {Remarks} remarks - 备注
 * @param {TraceObject} object - 对象描述
 * @param {State[]} states - 状态列表
 * @param {Step[]} steps - 步骤列表
 * @param {Result} [result] - 成功结果
 * @param {TraceError} [error] - 错误信息
 * @returns {Trace} Trace 对象
 */
function createTrace(meta, pkg, remarks, object, states, steps, result, error) {
    const trace = {
        meta,
        package: pkg,
        remarks,
        object,
        states,
        steps,
    };
    if (result !== undefined) {
        trace.result = result;
    }
    if (error !== undefined) {
        trace.error = error;
    }
    return trace;
}

// ============================================================================
// TOML 写入器
// ============================================================================

/**
 * TOML 写入器类
 */
class TomlWriter {
    /**
     * 构造函数
     *
     * @param {Trace} trace - 要序列化的 Trace 对象
     */
    constructor(trace) {
        /** @type {Trace} */
        this._trace = trace;
    }

    /**
     * 将 Trace 写入指定路径的 TOML 文件（仅 Node.js 环境）
     *
     * @param {string} path - 输出文件路径
     * @returns {void}
     */
    write(path) {
        const content = this.serialize();
        if (isNodeEnvironment()) {
            // 动态导入 fs 模块
            const fs = require('fs');
            fs.writeFileSync(path, content, 'utf-8');
        }
    }

    /**
     * 将 Trace 序列化为 TOML 字符串
     *
     * @returns {string} TOML 格式的字符串
     */
    serialize() {
        const lines = [];

        lines.push(...this._serializeMeta());
        lines.push('');
        lines.push(...this._serializePackage());

        if (!isRemarksEmpty(this._trace.remarks)) {
            lines.push('');
            lines.push(...this._serializeRemarks());
        }

        lines.push('');
        lines.push(...this._serializeObject());

        for (const state of this._trace.states) {
            lines.push('');
            lines.push(...this._serializeState(state));
        }

        for (const step of this._trace.steps) {
            lines.push('');
            lines.push(...this._serializeStep(step));
        }

        if (this._trace.result !== undefined) {
            lines.push('');
            lines.push(...this._serializeResult());
        }

        if (this._trace.error !== undefined) {
            lines.push('');
            lines.push(...this._serializeError());
        }

        return lines.join('\n');
    }

    /**
     * 序列化 meta 部分
     *
     * @returns {string[]} TOML 行列表
     */
    _serializeMeta() {
        return [
            '[meta]',
            `created_at = "${this._trace.meta.createdAt}"`,
            `lang = "${this._trace.meta.lang}"`,
            `lang_version = "${this._trace.meta.langVersion}"`,
        ];
    }

    /**
     * 序列化 package 部分
     *
     * @returns {string[]} TOML 行列表
     */
    _serializePackage() {
        return [
            '[package]',
            `name = "${this._trace.package.name}"`,
            `lang = "${this._trace.package.lang}"`,
            `version = "${this._trace.package.version}"`,
        ];
    }

    /**
     * 序列化 remarks 部分
     *
     * @returns {string[]} TOML 行列表
     */
    _serializeRemarks() {
        const lines = ['[remarks]'];
        if (this._trace.remarks.title !== '') {
            lines.push(`title = "${this._escapeString(this._trace.remarks.title)}"`);
        }
        if (this._trace.remarks.author !== '') {
            lines.push(`author = "${this._escapeString(this._trace.remarks.author)}"`);
        }
        if (this._trace.remarks.comment !== '') {
            lines.push(`comment = "${this._escapeString(this._trace.remarks.comment)}"`);
        }
        return lines;
    }

    /**
     * 序列化 object 部分
     *
     * @returns {string[]} TOML 行列表
     */
    _serializeObject() {
        const lines = [
            '[object]',
            `kind = "${this._trace.object.kind}"`,
        ];
        if (this._trace.object.label !== '') {
            lines.push(`label = "${this._escapeString(this._trace.object.label)}"`);
        }
        return lines;
    }

    /**
     * 序列化单个 state
     *
     * @param {State} state - 状态对象
     * @returns {string[]} TOML 行列表
     */
    _serializeState(state) {
        const lines = [
            '[[states]]',
            `id = ${state.id}`,
            '',
            '[states.data]',
        ];
        lines.push(...this._serializeStateData(state.data));
        return lines;
    }

    /**
     * 序列化 state.data 部分
     *
     * @param {StateData} data - 状态数据字典
     * @returns {string[]} TOML 行列表
     */
    _serializeStateData(data) {
        const lines = [];
        for (const [key, value] of Object.entries(data)) {
            if (key === 'nodes' || key === 'edges') {
                lines.push(`${key} = ${this._serializeInlineTableArray(value)}`);
            } else if (key === 'items') {
                lines.push(`${key} = ${this._serializeSimpleArray(value)}`);
            } else {
                lines.push(`${key} = ${this._serializeValue(value)}`);
            }
        }
        return lines;
    }

    /**
     * 序列化 inline table 数组
     *
     * @param {Object[]} arr - 字典数组
     * @returns {string} TOML 格式的 inline table 数组字符串
     */
    _serializeInlineTableArray(arr) {
        if (arr.length === 0) {
            return '[]';
        }
        const items = arr.map((item) => {
            const pairs = Object.entries(item).map(
                ([k, v]) => `${k} = ${this._serializeValue(v)}`
            );
            return '{ ' + pairs.join(', ') + ' }';
        });
        return '[\n  ' + items.join(',\n  ') + '\n]';
    }

    /**
     * 序列化简单数组
     *
     * @param {*[]} arr - 值数组
     * @returns {string} TOML 格式的数组字符串
     */
    _serializeSimpleArray(arr) {
        if (arr.length === 0) {
            return '[]';
        }
        const items = arr.map((v) => this._serializeValue(v));
        return '[' + items.join(', ') + ']';
    }

    /**
     * 序列化单个 step
     *
     * @param {Step} step - 步骤对象
     * @returns {string[]} TOML 行列表
     */
    _serializeStep(step) {
        const lines = [
            '[[steps]]',
            `id = ${step.id}`,
            `op = "${step.op}"`,
            `before = ${step.before}`,
        ];

        if (step.after !== undefined) {
            lines.push(`after = ${step.after}`);
        }

        if (step.ret !== undefined) {
            lines.push(`ret = ${this._serializeValue(step.ret)}`);
        }

        if (step.note !== undefined) {
            lines.push(`note = "${this._escapeString(step.note)}"`);
        }

        lines.push('');
        lines.push('[steps.args]');

        for (const [key, value] of Object.entries(step.args)) {
            lines.push(`${key} = ${this._serializeValue(value)}`);
        }

        if (step.code !== undefined) {
            lines.push('');
            lines.push('[steps.code]');
            lines.push(`line = ${step.code.line}`);
            if (step.code.col !== undefined) {
                lines.push(`col = ${step.code.col}`);
            }
        }

        return lines;
    }

    /**
     * 序列化 result 部分
     *
     * @returns {string[]} TOML 行列表
     */
    _serializeResult() {
        const result = this._trace.result;
        if (result === undefined) {
            return [];
        }
        const lines = [
            '[result]',
            `final_state = ${result.finalState}`,
        ];
        if (result.commit !== undefined) {
            lines.push('');
            lines.push('[result.commit]');
            lines.push(`op = "${result.commit.op}"`);
            lines.push(`line = ${result.commit.line}`);
        }
        return lines;
    }

    /**
     * 序列化 error 部分
     *
     * @returns {string[]} TOML 行列表
     */
    _serializeError() {
        const error = this._trace.error;
        if (error === undefined) {
            return [];
        }
        const lines = [
            '[error]',
            `type = "${error.type}"`,
            `message = "${this._escapeString(error.message)}"`,
        ];
        if (error.line !== undefined) {
            lines.push(`line = ${error.line}`);
        }
        if (error.step !== undefined) {
            lines.push(`step = ${error.step}`);
        }
        if (error.lastState !== undefined) {
            lines.push(`last_state = ${error.lastState}`);
        }
        return lines;
    }

    /**
     * 序列化单个值
     *
     * @param {*} value - 要序列化的值
     * @returns {string} TOML 格式的值字符串
     */
    _serializeValue(value) {
        if (typeof value === 'boolean') {
            return value ? 'true' : 'false';
        }
        if (typeof value === 'number') {
            return String(value);
        }
        if (typeof value === 'string') {
            return `"${this._escapeString(value)}"`;
        }
        if (Array.isArray(value)) {
            return this._serializeSimpleArray(value);
        }
        if (typeof value === 'object' && value !== null) {
            const pairs = Object.entries(value).map(
                ([k, v]) => `${k} = ${this._serializeValue(v)}`
            );
            return '{ ' + pairs.join(', ') + ' }';
        }
        return `"${this._escapeString(String(value))}"`;
    }

    /**
     * 转义字符串中的特殊字符
     *
     * @param {string} s - 原始字符串
     * @returns {string} 转义后的字符串
     */
    _escapeString(s) {
        return s
            .replace(/\\/g, '\\\\')
            .replace(/"/g, '\\"')
            .replace(/\n/g, '\\n')
            .replace(/\r/g, '\\r');
    }
}

// ============================================================================
// 会话管理器
// ============================================================================

/**
 * 会话管理器，用于管理数据结构的操作轨迹记录
 */
class Session {
    /**
     * 构造函数
     *
     * @param {string} kind - 数据结构类型
     * @param {string} label - 数据结构标签
     * @param {string} [output] - 输出文件路径
     */
    constructor(kind, label, output) {
        /** @type {string} */
        this._kind = kind;
        /** @type {string} */
        this._label = label;
        /** @type {string} */
        this._output = output ?? globalConfig.outputPath;
        /** @type {State[]} */
        this._states = [];
        /** @type {Step[]} */
        this._steps = [];
        /** @type {number} */
        this._stateCounter = 0;
        /** @type {number} */
        this._stepCounter = 0;
        /** @type {TraceError|undefined} */
        this._error = undefined;
        /** @type {number} */
        this._exitLine = 1;
        /** @type {number|undefined} */
        this._failedStepId = undefined;
        /** @type {string|undefined} */
        this._tomlString = undefined;
    }

    /**
     * 获取失败步骤 ID
     *
     * @returns {number|undefined} 失败步骤 ID
     */
    get failedStepId() {
        return this._failedStepId;
    }

    /**
     * 设置失败步骤 ID
     *
     * @param {number|undefined} value - 失败步骤 ID
     */
    set failedStepId(value) {
        this._failedStepId = value;
    }

    /**
     * 获取当前步骤计数器
     *
     * @returns {number} 当前步骤计数
     */
    get stepCounter() {
        return this._stepCounter;
    }

    /**
     * 获取生成的 TOML 字符串
     *
     * @returns {string|undefined} TOML 字符串
     */
    get tomlString() {
        return this._tomlString;
    }

    /**
     * 添加状态快照
     *
     * @param {StateData} data - 状态数据
     * @returns {number} 新状态的 ID
     */
    addState(data) {
        const stateId = this._stateCounter;
        this._states.push(createState(stateId, data));
        this._stateCounter += 1;
        return stateId;
    }

    /**
     * 添加操作步骤
     *
     * @param {string} op - 操作名称
     * @param {number} before - 操作前状态 ID
     * @param {number|undefined} after - 操作后状态 ID
     * @param {StepArgs} args - 操作参数
     * @param {*} [ret] - 返回值
     * @param {string} [note] - 备注
     * @param {number} [line] - 代码行号
     * @returns {number} 新步骤的 ID
     */
    addStep(op, before, after, args, ret, note, line) {
        const stepId = this._stepCounter;
        const codeLoc = line !== undefined ? { line } : undefined;
        this._steps.push(createStep(stepId, op, before, after, args, codeLoc, ret, note));
        this._stepCounter += 1;
        return stepId;
    }

    /**
     * 设置错误信息
     *
     * @param {string} errorType - 错误类型
     * @param {string} message - 错误消息
     * @param {number} [line] - 错误发生的代码行号
     * @param {number} [stepId] - 发生错误的步骤 ID
     * @returns {void}
     */
    setError(errorType, message, line, stepId) {
        const lastState = this._states.length > 0
            ? this._states[this._states.length - 1]?.id
            : undefined;
        this._error = createTraceError(
            errorType,
            message.slice(0, 512),
            line,
            stepId,
            lastState
        );
        this._failedStepId = stepId;
    }

    /**
     * 设置上下文退出行号
     *
     * @param {number} line - 退出行号
     * @returns {void}
     */
    setExitLine(line) {
        this._exitLine = line;
    }

    /**
     * 获取最后一个状态的 ID
     *
     * @returns {number} 最后一个状态的 ID，若无状态则返回 -1
     */
    getLastStateId() {
        if (this._states.length === 0) {
            return -1;
        }
        const lastState = this._states[this._states.length - 1];
        return lastState?.id ?? -1;
    }

    /**
     * 构建完整的 Trace 对象
     *
     * @returns {Trace} 完整的 Trace 对象
     */
    buildTrace() {
        const now = new Date().toISOString().replace(/\.\d{3}Z$/, 'Z');
        const meta = createMeta(now, getJavaScriptVersion());
        const pkg = createPackage();
        const remarks = createRemarks(
            globalConfig.title,
            globalConfig.author,
            globalConfig.comment
        );
        const obj = createTraceObject(this._kind, this._label);

        let result;
        let error;

        if (this._error !== undefined) {
            error = this._error;
        } else {
            const finalStateId = this._states.length > 0
                ? (this._states[this._states.length - 1]?.id ?? 0)
                : 0;
            result = createResult(finalStateId, { op: 'commit', line: this._exitLine });
        }

        return createTrace(meta, pkg, remarks, obj, this._states, this._steps, result, error);
    }

    /**
     * 将 Trace 写入文件或生成 TOML 字符串
     *
     * @returns {void}
     */
    write() {
        const trace = this.buildTrace();
        const writer = new TomlWriter(trace);
        this._tomlString = writer.serialize();

        if (isNodeEnvironment()) {
            writer.write(this._output);
        }
    }
}

// ============================================================================
// 上下文管理器基类
// ============================================================================

/**
 * 结构选项接口
 *
 * @typedef {Object} StructureOptions
 * @property {string} [label] - 数据结构的标签
 * @property {string} [output] - 输出文件路径
 */

/**
 * 上下文管理器基类，用于管理数据结构的生命周期
 */
class ContextManager {
    /**
     * 构造函数
     *
     * @param {string} kind - 数据结构类型
     * @param {string} label - 数据结构标签
     * @param {string} [output] - 输出文件路径
     */
    constructor(kind, label, output) {
        /** @type {Session} */
        this._session = new Session(kind, label, output);
        /** @type {Error|undefined} */
        this._caughtError = undefined;
    }

    /**
     * 获取会话对象
     *
     * @returns {Session} 会话对象
     */
    get session() {
        return this._session;
    }

    /**
     * 进入上下文
     *
     * @returns {void}
     */
    enter() {
        this._initialize();
    }

    /**
     * 设置捕获的错误
     *
     * @param {Error} error - 捕获的错误
     * @returns {void}
     */
    setCaughtError(error) {
        this._caughtError = error;
    }

    /**
     * 释放资源
     *
     * @returns {void}
     */
    dispose() {
        this._session.setExitLine(getCallerLine(3));

        if (this._caughtError !== undefined) {
            let errorLine;
            let errorType = ErrorType.Unknown;
            const message = this._caughtError.message;

            if (this._caughtError instanceof DS4VizError) {
                errorType = this._caughtError.errorType;
                errorLine = this._caughtError.line;
            }

            this._session.setError(
                errorType,
                message,
                errorLine,
                this._session.failedStepId
            );
        }

        this._session.write();
    }

    /**
     * 获取生成的 TOML 字符串
     *
     * @returns {string|undefined} TOML 字符串
     */
    getTomlString() {
        return this._session.tomlString;
    }

    /**
     * 初始化数据结构，子类需要重写此方法添加初始状态
     *
     * @returns {void}
     * @abstract
     */
    _initialize() {
        throw new Error('Method _initialize() must be implemented');
    }

    /**
     * 构建当前状态的数据字典
     *
     * @returns {StateData} 状态数据字典
     * @abstract
     */
    _buildData() {
        throw new Error('Method _buildData() must be implemented');
    }

    /**
     * 抛出数据结构错误（支持注入行号）
     *
     * @param {string} message - 错误消息
     * @param {number} [injectedLine] - 注入的行号
     * @throws {StructureError} 始终抛出
     */
    _raiseError(message, injectedLine) {
        const line = injectedLine ?? getCallerLine(3);
        const stepId = this._session.stepCounter;
        this._session.failedStepId = stepId;
        const error = new StructureError(message, line);
        this._caughtError = error;
        throw error;
    }

    /**
     * 记录操作步骤并添加新状态（支持注入行号）
     *
     * @param {string} op - 操作名称
     * @param {StepArgs} args - 操作参数
     * @param {number} [injectedLine] - 注入的行号
     * @param {*} [ret] - 返回值
     * @param {string} [note] - 备注
     * @returns {void}
     */
    _recordStep(op, args, injectedLine, ret, note) {
        const before = this._session.getLastStateId();
        const line = injectedLine ?? getCallerLine(3);
        const newData = this._buildData();
        const after = this._session.addState(newData);
        this._session.addStep(op, before, after, args, ret, note, line);
    }
}

/**
 * 使用上下文管理数据结构的生命周期
 *
 * @param {ContextManager} structure - 数据结构实例
 * @param {function(ContextManager): void|Promise<void>} callback - 回调函数
 * @returns {Promise<void>}
 */
async function withContext(structure, callback) {
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

// ============================================================================
// 值类型定义
// ============================================================================

/**
 * 值类型定义
 *
 * @typedef {number|string|boolean} ValueType
 */

// ============================================================================
// 线性数据结构
// ============================================================================

/**
 * 栈数据结构
 */
class Stack extends ContextManager {
    /**
     * 构造函数
     *
     * @param {StructureOptions} [options] - 结构选项
     */
    constructor(options) {
        super('stack', options?.label ?? 'stack', options?.output);
        /** @type {ValueType[]} */
        this._items = [];
    }

    /**
     * 构建栈的状态数据
     *
     * @returns {StateData} 状态数据字典
     */
    _buildData() {
        return {
            items: [...this._items],
            top: this._items.length > 0 ? this._items.length - 1 : -1,
        };
    }

    /**
     * 初始化栈
     *
     * @returns {void}
     */
    _initialize() {
        this._session.addState(this._buildData());
    }

    /**
     * 压栈操作
     *
     * @param {ValueType} value - 要压入的值
     * @returns {void}
     */
    push(value) {
        const extracted = extractLineFromArgs(arguments);
        const line = extracted.line ?? getCallerLine(2);
        const actualValue = extracted.args.length > 0 ? extracted.args[0] : value;

        const before = this._session.getLastStateId();
        this._items.push(actualValue);
        const after = this._session.addState(this._buildData());
        this._session.addStep('push', before, after, { value: actualValue }, undefined, undefined, line);
    }

    /**
     * 弹栈操作
     *
     * @returns {void}
     * @throws {StructureError} 当栈为空时抛出异常
     */
    pop() {
        const extracted = extractLineFromArgs(arguments);
        const line = extracted.line ?? getCallerLine(2);

        if (this._items.length === 0) {
            this._raiseError('Cannot pop from empty stack', line);
        }
        const before = this._session.getLastStateId();
        const popped = this._items.pop();
        const after = this._session.addState(this._buildData());
        this._session.addStep('pop', before, after, {}, popped, undefined, line);
    }

    /**
     * 清空栈
     *
     * @returns {void}
     */
    clear() {
        const extracted = extractLineFromArgs(arguments);
        const line = extracted.line ?? getCallerLine(2);

        const before = this._session.getLastStateId();
        this._items.length = 0;
        const after = this._session.addState(this._buildData());
        this._session.addStep('clear', before, after, {}, undefined, undefined, line);
    }
}

/**
 * 队列数据结构
 */
class Queue extends ContextManager {
    /**
     * 构造函数
     *
     * @param {StructureOptions} [options] - 结构选项
     */
    constructor(options) {
        super('queue', options?.label ?? 'queue', options?.output);
        /** @type {ValueType[]} */
        this._items = [];
    }

    /**
     * 构建队列的状态数据
     *
     * @returns {StateData} 状态数据字典
     */
    _buildData() {
        if (this._items.length === 0) {
            return { items: [], front: -1, rear: -1 };
        }
        return {
            items: [...this._items],
            front: 0,
            rear: this._items.length - 1,
        };
    }

    /**
     * 初始化队列
     *
     * @returns {void}
     */
    _initialize() {
        this._session.addState(this._buildData());
    }

    /**
     * 入队操作
     *
     * @param {ValueType} value - 要入队的值
     * @returns {void}
     */
    enqueue(value) {
        const extracted = extractLineFromArgs(arguments);
        const line = extracted.line ?? getCallerLine(2);
        const actualValue = extracted.args.length > 0 ? extracted.args[0] : value;

        const before = this._session.getLastStateId();
        this._items.push(actualValue);
        const after = this._session.addState(this._buildData());
        this._session.addStep('enqueue', before, after, { value: actualValue }, undefined, undefined, line);
    }

    /**
     * 出队操作
     *
     * @returns {void}
     * @throws {StructureError} 当队列为空时抛出异常
     */
    dequeue() {
        const extracted = extractLineFromArgs(arguments);
        const line = extracted.line ?? getCallerLine(2);

        if (this._items.length === 0) {
            this._raiseError('Cannot dequeue from empty queue', line);
        }
        const before = this._session.getLastStateId();
        const dequeued = this._items.shift();
        const after = this._session.addState(this._buildData());
        this._session.addStep('dequeue', before, after, {}, dequeued, undefined, line);
    }

    /**
     * 清空队列
     *
     * @returns {void}
     */
    clear() {
        const extracted = extractLineFromArgs(arguments);
        const line = extracted.line ?? getCallerLine(2);

        const before = this._session.getLastStateId();
        this._items.length = 0;
        const after = this._session.addState(this._buildData());
        this._session.addStep('clear', before, after, {}, undefined, undefined, line);
    }
}

/**
 * 单链表节点数据
 *
 * @typedef {Object} SListNodeData
 * @property {ValueType} value - 节点值
 * @property {number} next - 下一个节点 ID
 */

/**
 * 单链表数据结构
 */
class SingleLinkedList extends ContextManager {
    /**
     * 构造函数
     *
     * @param {StructureOptions} [options] - 结构选项
     */
    constructor(options) {
        super('slist', options?.label ?? 'slist', options?.output);
        /** @type {Map<number, SListNodeData>} */
        this._nodes = new Map();
        /** @type {number} */
        this._head = -1;
        /** @type {number} */
        this._nextId = 0;
    }

    /**
     * 构建单链表的状态数据
     *
     * @returns {StateData} 状态数据字典
     */
    _buildData() {
        const nodesList = [];
        const sortedIds = [...this._nodes.keys()].sort((a, b) => a - b);
        for (const nid of sortedIds) {
            const data = this._nodes.get(nid);
            nodesList.push({ id: nid, value: data.value, next: data.next });
        }
        return { head: this._head, nodes: nodesList };
    }

    /**
     * 初始化单链表
     *
     * @returns {void}
     */
    _initialize() {
        this._session.addState(this._buildData());
    }

    /**
     * 查找尾节点 ID
     *
     * @returns {number} 尾节点 ID，若链表为空则返回 -1
     */
    _findTail() {
        if (this._head === -1) {
            return -1;
        }
        let current = this._head;
        let node = this._nodes.get(current);
        while (node !== undefined && node.next !== -1) {
            current = node.next;
            node = this._nodes.get(current);
        }
        return current;
    }

    /**
     * 查找指定节点的前驱节点 ID
     *
     * @param {number} nodeId - 目标节点 ID
     * @returns {number} 前驱节点 ID，若为头节点或不存在则返回 -1
     */
    _findPrev(nodeId) {
        if (this._head === -1 || this._head === nodeId) {
            return -1;
        }
        let current = this._head;
        let node = this._nodes.get(current);
        while (node !== undefined) {
            if (node.next === nodeId) {
                return current;
            }
            current = node.next;
            node = this._nodes.get(current);
        }
        return -1;
    }

    /**
     * 在链表头部插入节点
     *
     * @param {ValueType} value - 要插入的值
     * @returns {number} 新插入节点的 id
     */
    insertHead(value) {
        const extracted = extractLineFromArgs(arguments);
        const line = extracted.line ?? getCallerLine(2);
        const actualValue = extracted.args.length > 0 ? extracted.args[0] : value;

        const before = this._session.getLastStateId();
        const newId = this._nextId;
        this._nextId += 1;
        this._nodes.set(newId, { value: actualValue, next: this._head });
        this._head = newId;
        const after = this._session.addState(this._buildData());
        this._session.addStep('insert_head', before, after, { value: actualValue }, newId, undefined, line);
        return newId;
    }

    /**
     * 在链表尾部插入节点
     *
     * @param {ValueType} value - 要插入的值
     * @returns {number} 新插入节点的 id
     */
    insertTail(value) {
        const extracted = extractLineFromArgs(arguments);
        const line = extracted.line ?? getCallerLine(2);
        const actualValue = extracted.args.length > 0 ? extracted.args[0] : value;

        const before = this._session.getLastStateId();
        const newId = this._nextId;
        this._nextId += 1;
        this._nodes.set(newId, { value: actualValue, next: -1 });
        if (this._head === -1) {
            this._head = newId;
        } else {
            const tail = this._findTail();
            const tailNode = this._nodes.get(tail);
            if (tailNode !== undefined) {
                tailNode.next = newId;
            }
        }
        const after = this._session.addState(this._buildData());
        this._session.addStep('insert_tail', before, after, { value: actualValue }, newId, undefined, line);
        return newId;
    }

    /**
     * 在指定节点后插入新节点
     *
     * @param {number} nodeId - 目标节点的 id
     * @param {ValueType} value - 要插入的值
     * @returns {number} 新插入节点的 id
     * @throws {StructureError} 当指定节点不存在时抛出异常
     */
    insertAfter(nodeId, value) {
        const extracted = extractLineFromArgs(arguments);
        const line = extracted.line ?? getCallerLine(2);
        const actualNodeId = extracted.args.length >= 1 ? extracted.args[0] : nodeId;
        const actualValue = extracted.args.length >= 2 ? extracted.args[1] : value;

        if (!this._nodes.has(actualNodeId)) {
            this._raiseError(`Node not found: ${actualNodeId}`, line);
        }
        const before = this._session.getLastStateId();
        const newId = this._nextId;
        this._nextId += 1;
        const targetNode = this._nodes.get(actualNodeId);
        const oldNext = targetNode.next;
        this._nodes.set(newId, { value: actualValue, next: oldNext });
        targetNode.next = newId;
        const after = this._session.addState(this._buildData());
        this._session.addStep('insert_after', before, after, { node_id: actualNodeId, value: actualValue }, newId, undefined, line);
        return newId;
    }

    /**
     * 删除指定节点
     *
     * @param {number} nodeId - 要删除的节点 id
     * @returns {void}
     * @throws {StructureError} 当指定节点不存在时抛出异常
     */
    delete(nodeId) {
        const extracted = extractLineFromArgs(arguments);
        const line = extracted.line ?? getCallerLine(2);
        const actualNodeId = extracted.args.length > 0 ? extracted.args[0] : nodeId;

        if (!this._nodes.has(actualNodeId)) {
            this._raiseError(`Node not found: ${actualNodeId}`, line);
        }
        const before = this._session.getLastStateId();
        const deletedNode = this._nodes.get(actualNodeId);
        const deletedValue = deletedNode.value;
        if (this._head === actualNodeId) {
            this._head = deletedNode.next;
        } else {
            const prev = this._findPrev(actualNodeId);
            if (prev !== -1) {
                const prevNode = this._nodes.get(prev);
                if (prevNode !== undefined) {
                    prevNode.next = deletedNode.next;
                }
            }
        }
        this._nodes.delete(actualNodeId);
        const after = this._session.addState(this._buildData());
        this._session.addStep('delete', before, after, { node_id: actualNodeId }, deletedValue, undefined, line);
    }

    /**
     * 删除头节点
     *
     * @returns {void}
     * @throws {StructureError} 当链表为空时抛出异常
     */
    deleteHead() {
        const extracted = extractLineFromArgs(arguments);
        const line = extracted.line ?? getCallerLine(2);

        if (this._head === -1) {
            this._raiseError('Cannot delete from empty list', line);
        }
        const before = this._session.getLastStateId();
        const oldHead = this._head;
        const headNode = this._nodes.get(oldHead);
        const deletedValue = headNode.value;
        this._head = headNode.next;
        this._nodes.delete(oldHead);
        const after = this._session.addState(this._buildData());
        this._session.addStep('delete_head', before, after, {}, deletedValue, undefined, line);
    }

    /**
     * 删除尾节点
     *
     * @returns {void}
     * @throws {StructureError} 当链表为空时抛出异常
     */
    deleteTail() {
        const extracted = extractLineFromArgs(arguments);
        const line = extracted.line ?? getCallerLine(2);

        if (this._head === -1) {
            this._raiseError('Cannot delete from empty list', line);
        }
        const before = this._session.getLastStateId();

        // 如果只有一个节点
        if (this._nodes.get(this._head).next === -1) {
            const deletedValue = this._nodes.get(this._head).value;
            const oldHead = this._head;
            this._nodes.delete(this._head);
            this._head = -1;
            const after = this._session.addState(this._buildData());
            this._session.addStep('delete_tail', before, after, {}, deletedValue, undefined, line);
            return;
        }

        // 找到倒数第二个节点
        let prev = this._head;
        let current = this._nodes.get(this._head);
        while (current.next !== -1) {
            const nextNode = this._nodes.get(current.next);
            if (nextNode.next === -1) {
                break;
            }
            prev = current.next;
            current = nextNode;
        }

        const tailId = current.next;
        const deletedValue = this._nodes.get(tailId).value;
        current.next = -1;
        this._nodes.delete(tailId);
        const after = this._session.addState(this._buildData());
        this._session.addStep('delete_tail', before, after, {}, deletedValue, undefined, line);
    }

    /**
     * 反转链表
     *
     * @returns {void}
     */
    reverse() {
        const extracted = extractLineFromArgs(arguments);
        const line = extracted.line ?? getCallerLine(2);

        const before = this._session.getLastStateId();
        let prev = -1;
        let current = this._head;
        while (current !== -1) {
            const node = this._nodes.get(current);
            if (node === undefined) {
                break;
            }
            const nextNode = node.next;
            node.next = prev;
            prev = current;
            current = nextNode;
        }
        this._head = prev;
        const after = this._session.addState(this._buildData());
        this._session.addStep('reverse', before, after, {}, undefined, undefined, line);
    }

    /**
     * 清空链表
     *
     * @returns {void}
     */
    clear() {
        const extracted = extractLineFromArgs(arguments);
        const line = extracted.line ?? getCallerLine(2);

        const before = this._session.getLastStateId();
        this._nodes.clear();
        this._head = -1;
        this._nextId = 0;
        const after = this._session.addState(this._buildData());
        this._session.addStep('clear', before, after, {}, undefined, undefined, line);
    }
}

/**
 * 双向链表节点数据
 *
 * @typedef {Object} DListNodeData
 * @property {ValueType} value - 节点值
 * @property {number} prev - 前一个节点 ID
 * @property {number} next - 下一个节点 ID
 */

/**
 * 双向链表数据结构
 */
class DoubleLinkedList extends ContextManager {
    /**
     * 构造函数
     *
     * @param {StructureOptions} [options] - 结构选项
     */
    constructor(options) {
        super('dlist', options?.label ?? 'dlist', options?.output);
        /** @type {Map<number, DListNodeData>} */
        this._nodes = new Map();
        /** @type {number} */
        this._head = -1;
        /** @type {number} */
        this._tail = -1;
        /** @type {number} */
        this._nextId = 0;
    }

    /**
     * 构建双向链表的状态数据
     *
     * @returns {StateData} 状态数据字典
     */
    _buildData() {
        const nodesList = [];
        const sortedIds = [...this._nodes.keys()].sort((a, b) => a - b);
        for (const nid of sortedIds) {
            const data = this._nodes.get(nid);
            nodesList.push({ id: nid, value: data.value, prev: data.prev, next: data.next });
        }
        return { head: this._head, tail: this._tail, nodes: nodesList };
    }

    /**
     * 初始化双向链表
     *
     * @returns {void}
     */
    _initialize() {
        this._session.addState(this._buildData());
    }

    /**
     * 在链表头部插入节点
     *
     * @param {ValueType} value - 要插入的值
     * @returns {number} 新插入节点的 id
     */
    insertHead(value) {
        const extracted = extractLineFromArgs(arguments);
        const line = extracted.line ?? getCallerLine(2);
        const actualValue = extracted.args.length > 0 ? extracted.args[0] : value;

        const before = this._session.getLastStateId();
        const newId = this._nextId;
        this._nextId += 1;
        this._nodes.set(newId, { value: actualValue, prev: -1, next: this._head });
        if (this._head !== -1) {
            const headNode = this._nodes.get(this._head);
            if (headNode !== undefined) {
                headNode.prev = newId;
            }
        }
        this._head = newId;
        if (this._tail === -1) {
            this._tail = newId;
        }
        const after = this._session.addState(this._buildData());
        this._session.addStep('insert_head', before, after, { value: actualValue }, newId, undefined, line);
        return newId;
    }

    /**
     * 在链表尾部插入节点
     *
     * @param {ValueType} value - 要插入的值
     * @returns {number} 新插入节点的 id
     */
    insertTail(value) {
        const extracted = extractLineFromArgs(arguments);
        const line = extracted.line ?? getCallerLine(2);
        const actualValue = extracted.args.length > 0 ? extracted.args[0] : value;

        const before = this._session.getLastStateId();
        const newId = this._nextId;
        this._nextId += 1;
        this._nodes.set(newId, { value: actualValue, prev: this._tail, next: -1 });
        if (this._tail !== -1) {
            const tailNode = this._nodes.get(this._tail);
            if (tailNode !== undefined) {
                tailNode.next = newId;
            }
        }
        this._tail = newId;
        if (this._head === -1) {
            this._head = newId;
        }
        const after = this._session.addState(this._buildData());
        this._session.addStep('insert_tail', before, after, { value: actualValue }, newId, undefined, line);
        return newId;
    }

    /**
     * 在指定节点前插入新节点
     *
     * @param {number} nodeId - 目标节点的 id
     * @param {ValueType} value - 要插入的值
     * @returns {number} 新插入节点的 id
     * @throws {StructureError} 当指定节点不存在时抛出异常
     */
    insertBefore(nodeId, value) {
        const extracted = extractLineFromArgs(arguments);
        const line = extracted.line ?? getCallerLine(2);
        const actualNodeId = extracted.args.length >= 1 ? extracted.args[0] : nodeId;
        const actualValue = extracted.args.length >= 2 ? extracted.args[1] : value;

        if (!this._nodes.has(actualNodeId)) {
            this._raiseError(`Node not found: ${actualNodeId}`, line);
        }
        const before = this._session.getLastStateId();
        const newId = this._nextId;
        this._nextId += 1;
        const targetNode = this._nodes.get(actualNodeId);
        const oldPrev = targetNode.prev;
        this._nodes.set(newId, { value: actualValue, prev: oldPrev, next: actualNodeId });
        targetNode.prev = newId;
        if (oldPrev !== -1) {
            const prevNode = this._nodes.get(oldPrev);
            if (prevNode !== undefined) {
                prevNode.next = newId;
            }
        } else {
            this._head = newId;
        }
        const after = this._session.addState(this._buildData());
        this._session.addStep('insert_before', before, after, { node_id: actualNodeId, value: actualValue }, newId, undefined, line);
        return newId;
    }

    /**
     * 在指定节点后插入新节点
     *
     * @param {number} nodeId - 目标节点的 id
     * @param {ValueType} value - 要插入的值
     * @returns {number} 新插入节点的 id
     * @throws {StructureError} 当指定节点不存在时抛出异常
     */
    insertAfter(nodeId, value) {
        const extracted = extractLineFromArgs(arguments);
        const line = extracted.line ?? getCallerLine(2);
        const actualNodeId = extracted.args.length >= 1 ? extracted.args[0] : nodeId;
        const actualValue = extracted.args.length >= 2 ? extracted.args[1] : value;

        if (!this._nodes.has(actualNodeId)) {
            this._raiseError(`Node not found: ${actualNodeId}`, line);
        }
        const before = this._session.getLastStateId();
        const newId = this._nextId;
        this._nextId += 1;
        const targetNode = this._nodes.get(actualNodeId);
        const oldNext = targetNode.next;
        this._nodes.set(newId, { value: actualValue, prev: actualNodeId, next: oldNext });
        targetNode.next = newId;
        if (oldNext !== -1) {
            const nextNode = this._nodes.get(oldNext);
            if (nextNode !== undefined) {
                nextNode.prev = newId;
            }
        } else {
            this._tail = newId;
        }
        const after = this._session.addState(this._buildData());
        this._session.addStep('insert_after', before, after, { node_id: actualNodeId, value: actualValue }, newId, undefined, line);
        return newId;
    }

    /**
     * 删除指定节点
     *
     * @param {number} nodeId - 要删除的节点 id
     * @returns {void}
     * @throws {StructureError} 当指定节点不存在时抛出异常
     */
    delete(nodeId) {
        const extracted = extractLineFromArgs(arguments);
        const line = extracted.line ?? getCallerLine(2);
        const actualNodeId = extracted.args.length > 0 ? extracted.args[0] : nodeId;

        if (!this._nodes.has(actualNodeId)) {
            this._raiseError(`Node not found: ${actualNodeId}`, line);
        }
        const before = this._session.getLastStateId();
        const nodeData = this._nodes.get(actualNodeId);
        const deletedValue = nodeData.value;
        const prevId = nodeData.prev;
        const nextId = nodeData.next;

        if (prevId !== -1) {
            const prevNode = this._nodes.get(prevId);
            if (prevNode !== undefined) {
                prevNode.next = nextId;
            }
        } else {
            this._head = nextId;
        }

        if (nextId !== -1) {
            const nextNode = this._nodes.get(nextId);
            if (nextNode !== undefined) {
                nextNode.prev = prevId;
            }
        } else {
            this._tail = prevId;
        }

        this._nodes.delete(actualNodeId);
        const after = this._session.addState(this._buildData());
        this._session.addStep('delete', before, after, { node_id: actualNodeId }, deletedValue, undefined, line);
    }

    /**
     * 删除头节点
     *
     * @returns {void}
     * @throws {StructureError} 当链表为空时抛出异常
     */
    deleteHead() {
        const extracted = extractLineFromArgs(arguments);
        const line = extracted.line ?? getCallerLine(2);

        if (this._head === -1) {
            this._raiseError('Cannot delete from empty list', line);
        }

        // 直接调用 delete 方法，但需要传递行号
        const headId = this._head;

        const before = this._session.getLastStateId();
        const nodeData = this._nodes.get(headId);
        const deletedValue = nodeData.value;
        const prevId = nodeData.prev;
        const nextId = nodeData.next;

        if (prevId !== -1) {
            const prevNode = this._nodes.get(prevId);
            if (prevNode !== undefined) {
                prevNode.next = nextId;
            }
        } else {
            this._head = nextId;
        }

        if (nextId !== -1) {
            const nextNode = this._nodes.get(nextId);
            if (nextNode !== undefined) {
                nextNode.prev = prevId;
            }
        } else {
            this._tail = prevId;
        }

        this._nodes.delete(headId);
        const after = this._session.addState(this._buildData());
        this._session.addStep('delete', before, after, { node_id: headId }, deletedValue, undefined, line);
    }
    

    /**
     * 删除尾节点
     *
     * @returns {void}
     * @throws {StructureError} 当链表为空时抛出异常
     */
    deleteTail() {
        const extracted = extractLineFromArgs(arguments);
        const line = extracted.line ?? getCallerLine(2);

        if (this._tail === -1) {
            this._raiseError('Cannot delete from empty list', line);
        }

        const tailId = this._tail;

        const before = this._session.getLastStateId();
        const nodeData = this._nodes.get(tailId);
        const deletedValue = nodeData.value;
        const prevId = nodeData.prev;
        const nextId = nodeData.next;

        if (prevId !== -1) {
            const prevNode = this._nodes.get(prevId);
            if (prevNode !== undefined) {
                prevNode.next = nextId;
            }
        } else {
            this._head = nextId;
        }

        if (nextId !== -1) {
            const nextNode = this._nodes.get(nextId);
            if (nextNode !== undefined) {
                nextNode.prev = prevId;
            }
        } else {
            this._tail = prevId;
        }

        this._nodes.delete(tailId);
        const after = this._session.addState(this._buildData());
        this._session.addStep('delete', before, after, { node_id: tailId }, deletedValue, undefined, line);
    }


    /**
     * 反转链表
     *
     * @returns {void}
     */
    reverse() {
        const extracted = extractLineFromArgs(arguments);
        const line = extracted.line ?? getCallerLine(2);

        const before = this._session.getLastStateId();
        let current = this._head;
        while (current !== -1) {
            const node = this._nodes.get(current);
            if (node === undefined) {
                break;
            }
            const temp = node.prev;
            node.prev = node.next;
            node.next = temp;
            current = node.prev;
        }
        const temp = this._head;
        this._head = this._tail;
        this._tail = temp;
        const after = this._session.addState(this._buildData());
        this._session.addStep('reverse', before, after, {}, undefined, undefined, line);
    }

    /**
     * 清空链表
     *
     * @returns {void}
     */
    clear() {
        const extracted = extractLineFromArgs(arguments);
        const line = extracted.line ?? getCallerLine(2);

        const before = this._session.getLastStateId();
        this._nodes.clear();
        this._head = -1;
        this._tail = -1;
        this._nextId = 0;
        const after = this._session.addState(this._buildData());
        this._session.addStep('clear', before, after, {}, undefined, undefined, line);
    }
}

// ============================================================================
// 树形数据结构
// ============================================================================

/**
 * 二叉树节点数据
 *
 * @typedef {Object} TreeNodeData
 * @property {ValueType} value - 节点值
 * @property {number} left - 左子节点 ID
 * @property {number} right - 右子节点 ID
 */

/**
 * 二叉树数据结构
 */
class BinaryTree extends ContextManager {
    /**
     * 构造函数
     *
     * @param {StructureOptions} [options] - 结构选项
     */
    constructor(options) {
        super('binary_tree', options?.label ?? 'binary_tree', options?.output);
        /** @type {Map<number, TreeNodeData>} */
        this._nodes = new Map();
        /** @type {number} */
        this._root = -1;
        /** @type {number} */
        this._nextId = 0;
    }

    /**
     * 构建二叉树的状态数据
     *
     * @returns {StateData} 状态数据字典
     */
    _buildData() {
        const nodesList = [];
        const sortedIds = [...this._nodes.keys()].sort((a, b) => a - b);
        for (const nid of sortedIds) {
            const data = this._nodes.get(nid);
            nodesList.push({ id: nid, value: data.value, left: data.left, right: data.right });
        }
        return { root: this._root, nodes: nodesList };
    }

    /**
     * 初始化二叉树
     *
     * @returns {void}
     */
    _initialize() {
        this._session.addState(this._buildData());
    }

    /**
     * 查找指定节点的父节点 ID
     *
     * @param {number} nodeId - 目标节点 ID
     * @returns {number} 父节点 ID，若为根节点或不存在则返回 -1
     */
    _findParent(nodeId) {
        for (const [nid, data] of this._nodes) {
            if (data.left === nodeId || data.right === nodeId) {
                return nid;
            }
        }
        return -1;
    }

    /**
     * 收集以指定节点为根的子树中所有节点 ID
     *
     * @param {number} nodeId - 根节点 ID
     * @returns {number[]} 子树中所有节点 ID 列表
     */
    _collectSubtree(nodeId) {
        if (nodeId === -1) {
            return [];
        }
        const node = this._nodes.get(nodeId);
        if (node === undefined) {
            return [];
        }
        const result = [nodeId];
        result.push(...this._collectSubtree(node.left));
        result.push(...this._collectSubtree(node.right));
        return result;
    }

    /**
     * 插入根节点
     *
     * @param {ValueType} value - 根节点的值
     * @returns {number} 根节点的 id
     * @throws {StructureError} 当根节点已存在时抛出异常
     */
    insertRoot(value) {
        const extracted = extractLineFromArgs(arguments);
        const line = extracted.line ?? getCallerLine(2);
        const actualValue = extracted.args.length > 0 ? extracted.args[0] : value;

        if (this._root !== -1) {
            this._raiseError('Root node already exists', line);
        }
        const before = this._session.getLastStateId();
        const newId = this._nextId;
        this._nextId += 1;
        this._nodes.set(newId, { value: actualValue, left: -1, right: -1 });
        this._root = newId;
        const after = this._session.addState(this._buildData());
        this._session.addStep('insert_root', before, after, { value: actualValue }, newId, undefined, line);
        return newId;
    }

    /**
     * 在指定父节点的左侧插入子节点
     *
     * @param {number} parentId - 父节点的 id
     * @param {ValueType} value - 要插入的子节点值
     * @returns {number} 新插入节点的 id
     * @throws {StructureError} 当父节点不存在或左子节点已存在时抛出异常
     */
    insertLeft(parentId, value) {
        const extracted = extractLineFromArgs(arguments);
        const line = extracted.line ?? getCallerLine(2);
        const actualParentId = extracted.args.length >= 1 ? extracted.args[0] : parentId;
        const actualValue = extracted.args.length >= 2 ? extracted.args[1] : value;

        if (!this._nodes.has(actualParentId)) {
            this._raiseError(`Parent node not found: ${actualParentId}`, line);
        }
        const parentNode = this._nodes.get(actualParentId);
        if (parentNode.left !== -1) {
            this._raiseError(`Left child already exists for node: ${actualParentId}`, line);
        }
        const before = this._session.getLastStateId();
        const newId = this._nextId;
        this._nextId += 1;
        this._nodes.set(newId, { value: actualValue, left: -1, right: -1 });
        parentNode.left = newId;
        const after = this._session.addState(this._buildData());
        this._session.addStep('insert_left', before, after, { parent: actualParentId, value: actualValue }, newId, undefined, line);
        return newId;
    }

    /**
     * 在指定父节点的右侧插入子节点
     *
     * @param {number} parentId - 父节点的 id
     * @param {ValueType} value - 要插入的子节点值
     * @returns {number} 新插入节点的 id
     * @throws {StructureError} 当父节点不存在或右子节点已存在时抛出异常
     */
    insertRight(parentId, value) {
        const extracted = extractLineFromArgs(arguments);
        const line = extracted.line ?? getCallerLine(2);
        const actualParentId = extracted.args.length >= 1 ? extracted.args[0] : parentId;
        const actualValue = extracted.args.length >= 2 ? extracted.args[1] : value;

        if (!this._nodes.has(actualParentId)) {
            this._raiseError(`Parent node not found: ${actualParentId}`, line);
        }
        const parentNode = this._nodes.get(actualParentId);
        if (parentNode.right !== -1) {
            this._raiseError(`Right child already exists for node: ${actualParentId}`, line);
        }
        const before = this._session.getLastStateId();
        const newId = this._nextId;
        this._nextId += 1;
        this._nodes.set(newId, { value: actualValue, left: -1, right: -1 });
        parentNode.right = newId;
        const after = this._session.addState(this._buildData());
        this._session.addStep('insert_right', before, after, { parent: actualParentId, value: actualValue }, newId, undefined, line);
        return newId;
    }

    /**
     * 删除指定节点及其所有子树
     *
     * @param {number} nodeId - 要删除的节点 id
     * @returns {void}
     * @throws {StructureError} 当节点不存在时抛出异常
     */
    delete(nodeId) {
        const extracted = extractLineFromArgs(arguments);
        const line = extracted.line ?? getCallerLine(2);
        const actualNodeId = extracted.args.length > 0 ? extracted.args[0] : nodeId;

        if (!this._nodes.has(actualNodeId)) {
            this._raiseError(`Node not found: ${actualNodeId}`, line);
        }
        const before = this._session.getLastStateId();
        const subtreeIds = this._collectSubtree(actualNodeId);
        const parentId = this._findParent(actualNodeId);
        if (parentId !== -1) {
            const parentNode = this._nodes.get(parentId);
            if (parentNode !== undefined) {
                if (parentNode.left === actualNodeId) {
                    parentNode.left = -1;
                } else {
                    parentNode.right = -1;
                }
            }
        } else {
            this._root = -1;
        }
        for (const nid of subtreeIds) {
            this._nodes.delete(nid);
        }
        const after = this._session.addState(this._buildData());
        this._session.addStep('delete', before, after, { node_id: actualNodeId }, undefined, undefined, line);
    }

    /**
     * 更新节点的值
     *
     * @param {number} nodeId - 节点 id
     * @param {ValueType} value - 新的值
     * @returns {void}
     * @throws {StructureError} 当节点不存在时抛出异常
     */
    updateValue(nodeId, value) {
        const extracted = extractLineFromArgs(arguments);
        const line = extracted.line ?? getCallerLine(2);
        const actualNodeId = extracted.args.length >= 1 ? extracted.args[0] : nodeId;
        const actualValue = extracted.args.length >= 2 ? extracted.args[1] : value;

        if (!this._nodes.has(actualNodeId)) {
            this._raiseError(`Node not found: ${actualNodeId}`, line);
        }
        const before = this._session.getLastStateId();
        const node = this._nodes.get(actualNodeId);
        const oldValue = node.value;
        node.value = actualValue;
        const after = this._session.addState(this._buildData());
        this._session.addStep('update_value', before, after, { node_id: actualNodeId, value: actualValue }, oldValue, undefined, line);
    }

    /**
     * 清空二叉树
     *
     * @returns {void}
     */
    clear() {
        const extracted = extractLineFromArgs(arguments);
        const line = extracted.line ?? getCallerLine(2);

        const before = this._session.getLastStateId();
        this._nodes.clear();
        this._root = -1;
        this._nextId = 0;
        const after = this._session.addState(this._buildData());
        this._session.addStep('clear', before, after, {}, undefined, undefined, line);
    }
}

/**
 * 二叉搜索树数据结构
 */
class BinarySearchTree extends ContextManager {
    /**
     * 构造函数
     *
     * @param {StructureOptions} [options] - 结构选项
     */
    constructor(options) {
        super('bst', options?.label ?? 'bst', options?.output);
        /** @type {Map<number, TreeNodeData>} */
        this._nodes = new Map();
        /** @type {number} */
        this._root = -1;
        /** @type {number} */
        this._nextId = 0;
    }

    /**
     * 构建二叉搜索树的状态数据
     *
     * @returns {StateData} 状态数据字典
     */
    _buildData() {
        const nodesList = [];
        const sortedIds = [...this._nodes.keys()].sort((a, b) => a - b);
        for (const nid of sortedIds) {
            const data = this._nodes.get(nid);
            nodesList.push({
                id: nid,
                value: data.value,
                left: data.left,
                right: data.right,
            });
        }
        return { root: this._root, nodes: nodesList };
    }

    /**
     * 初始化二叉搜索树
     *
     * @returns {void}
     */
    _initialize() {
        this._session.addState(this._buildData());
    }

    /**
     * 根据值查找节点 ID
     *
     * @param {number} value - 要查找的值
     * @returns {number} 节点 ID，若不存在则返回 -1
     */
    _findNodeByValue(value) {
        for (const [nid, data] of this._nodes) {
            if (data.value === value) {
                return nid;
            }
        }
        return -1;
    }

    /**
     * 查找指定节点的父节点 ID
     *
     * @param {number} nodeId - 目标节点 ID
     * @returns {number} 父节点 ID，若为根节点或不存在则返回 -1
     */
    _findParent(nodeId) {
        for (const [nid, data] of this._nodes) {
            if (data.left === nodeId || data.right === nodeId) {
                return nid;
            }
        }
        return -1;
    }

    /**
     * 查找以指定节点为根的子树中的最小值节点
     *
     * @param {number} nodeId - 子树根节点 ID
     * @returns {number} 最小值节点 ID
     */
    _findMinNode(nodeId) {
        let current = nodeId;
        let node = this._nodes.get(current);
        while (node !== undefined && node.left !== -1) {
            current = node.left;
            node = this._nodes.get(current);
        }
        return current;
    }

    /**
     * 在父节点中替换子节点引用
     *
     * @param {number} parentId - 父节点 ID
     * @param {number} oldChild - 旧子节点 ID
     * @param {number} newChild - 新子节点 ID
     * @returns {void}
     */
    _replaceInParent(parentId, oldChild, newChild) {
        if (parentId === -1) {
            this._root = newChild;
        } else {
            const parent = this._nodes.get(parentId);
            if (parent !== undefined) {
                if (parent.left === oldChild) {
                    parent.left = newChild;
                } else {
                    parent.right = newChild;
                }
            }
        }
    }

    /**
     * 删除指定节点并维护 BST 性质
     *
     * @param {number} nodeId - 要删除的节点 ID
     * @returns {void}
     */
    _deleteNode(nodeId) {
        const node = this._nodes.get(nodeId);
        if (node === undefined) {
            return;
        }

        const parentId = this._findParent(nodeId);
        const leftChild = node.left;
        const rightChild = node.right;

        if (leftChild === -1 && rightChild === -1) {
            this._replaceInParent(parentId, nodeId, -1);
            this._nodes.delete(nodeId);
        } else if (leftChild === -1) {
            this._replaceInParent(parentId, nodeId, rightChild);
            this._nodes.delete(nodeId);
        } else if (rightChild === -1) {
            this._replaceInParent(parentId, nodeId, leftChild);
            this._nodes.delete(nodeId);
        } else {
            const successorId = this._findMinNode(rightChild);
            const successor = this._nodes.get(successorId);
            if (successor !== undefined) {
                const successorValue = successor.value;
                this._deleteNode(successorId);
                const currentNode = this._nodes.get(nodeId);
                if (currentNode !== undefined) {
                    currentNode.value = successorValue;
                }
            }
        }
    }

    /**
     * 插入节点，自动维护二叉搜索树性质
     *
     * @param {number} value - 要插入的值
     * @returns {number} 新插入节点的 id
     */
    insert(value) {
        const extracted = extractLineFromArgs(arguments);
        const line = extracted.line ?? getCallerLine(2);
        const actualValue = extracted.args.length > 0 ? extracted.args[0] : value;

        const before = this._session.getLastStateId();
        const newId = this._nextId;
        this._nextId += 1;
        this._nodes.set(newId, { value: actualValue, left: -1, right: -1 });

        if (this._root === -1) {
            this._root = newId;
        } else {
            let current = this._root;
            while (true) {
                const currentNode = this._nodes.get(current);
                if (currentNode === undefined) {
                    break;
                }
                const currentValue = currentNode.value;
                if (actualValue < currentValue) {
                    if (currentNode.left === -1) {
                        currentNode.left = newId;
                        break;
                    }
                    current = currentNode.left;
                } else {
                    if (currentNode.right === -1) {
                        currentNode.right = newId;
                        break;
                    }
                    current = currentNode.right;
                }
            }
        }

        const after = this._session.addState(this._buildData());
        this._session.addStep('insert', before, after, { value: actualValue }, newId, undefined, line);
        return newId;
    }

    /**
     * 删除节点，自动维护二叉搜索树性质
     *
     * @param {number} value - 要删除的节点值
     * @returns {void}
     * @throws {StructureError} 当节点不存在时抛出异常
     */
    delete(value) {
        const extracted = extractLineFromArgs(arguments);
        const line = extracted.line ?? getCallerLine(2);
        const actualValue = extracted.args.length > 0 ? extracted.args[0] : value;

        const nodeId = this._findNodeByValue(actualValue);
        if (nodeId === -1) {
            this._raiseError(`Node with value ${actualValue} not found`, line);
        }
        const before = this._session.getLastStateId();
        this._deleteNode(nodeId);
        const after = this._session.addState(this._buildData());
        this._session.addStep('delete', before, after, { value: actualValue }, undefined, undefined, line);
    }

    /**
     * 清空二叉搜索树
     *
     * @returns {void}
     */
    clear() {
        const extracted = extractLineFromArgs(arguments);
        const line = extracted.line ?? getCallerLine(2);

        const before = this._session.getLastStateId();
        this._nodes.clear();
        this._root = -1;
        this._nextId = 0;
        const after = this._session.addState(this._buildData());
        this._session.addStep('clear', before, after, {}, undefined, undefined, line);
    }
}

/**
 * 堆选项接口
 *
 * @typedef {Object} HeapOptions
 * @property {string} [label] - 堆的标签
 * @property {string} [heapType="min"] - 堆类型，"min" 或 "max"
 * @property {string} [output] - 输出文件路径
 */

/**
 * 堆数据结构（使用完全二叉树表示）
 */
class Heap extends ContextManager {
    /**
     * 构造函数
     *
     * @param {HeapOptions} [options] - 堆选项
     * @throws {Error} 当 heapType 不是 "min" 或 "max" 时抛出异常
     */
    constructor(options) {
        const heapType = options?.heapType ?? 'min';
        if (heapType !== 'min' && heapType !== 'max') {
            throw new Error(`heapType must be 'min' or 'max', got '${heapType}'`);
        }
        super('binary_tree', options?.label ?? 'heap', options?.output);
        /** @type {string} */
        this._heapType = heapType;
        /** @type {number[]} */
        this._items = [];
    }

    /**
     * 构建堆的状态数据（以完全二叉树形式表示）
     *
     * @returns {StateData} 状态数据字典
     */
    _buildData() {
        if (this._items.length === 0) {
            return { root: -1, nodes: [] };
        }
        const nodesList = [];
        for (let i = 0; i < this._items.length; i++) {
            const left = 2 * i + 1 < this._items.length ? 2 * i + 1 : -1;
            const right = 2 * i + 2 < this._items.length ? 2 * i + 2 : -1;
            nodesList.push({ id: i, value: this._items[i], left, right });
        }
        return { root: 0, nodes: nodesList };
    }

    /**
     * 初始化堆
     *
     * @returns {void}
     */
    _initialize() {
        this._session.addState(this._buildData());
    }

    /**
     * 比较两个值，根据堆类型决定比较方式
     *
     * @param {number} a - 第一个值
     * @param {number} b - 第二个值
     * @returns {boolean} 如果 a 应该在 b 之前（更接近堆顶）则返回 true
     */
    _compare(a, b) {
        return this._heapType === 'min' ? a < b : a > b;
    }

    /**
     * 向上调整堆
     *
     * @param {number} index - 开始调整的索引
     * @returns {void}
     */
    _siftUp(index) {
        let current = index;
        while (current > 0) {
            const parent = Math.floor((current - 1) / 2);
            const currentVal = this._items[current];
            const parentVal = this._items[parent];
            if (currentVal !== undefined && parentVal !== undefined && this._compare(currentVal, parentVal)) {
                this._items[current] = parentVal;
                this._items[parent] = currentVal;
                current = parent;
            } else {
                break;
            }
        }
    }

    /**
     * 向下调整堆
     *
     * @param {number} index - 开始调整的索引
     * @returns {void}
     */
    _siftDown(index) {
        const size = this._items.length;
        let current = index;
        while (true) {
            let target = current;
            const left = 2 * current + 1;
            const right = 2 * current + 2;
            const targetVal = this._items[target];
            const leftVal = this._items[left];
            const rightVal = this._items[right];

            if (left < size && leftVal !== undefined && targetVal !== undefined && this._compare(leftVal, targetVal)) {
                target = left;
            }
            const newTargetVal = this._items[target];
            if (right < size && rightVal !== undefined && newTargetVal !== undefined && this._compare(rightVal, newTargetVal)) {
                target = right;
            }
            if (target === current) {
                break;
            }
            const currentValSwap = this._items[current];
            const targetValSwap = this._items[target];
            if (currentValSwap !== undefined && targetValSwap !== undefined) {
                this._items[current] = targetValSwap;
                this._items[target] = currentValSwap;
            }
            current = target;
        }
    }

    /**
     * 插入元素，自动维护堆性质
     *
     * @param {number} value - 要插入的值
     * @returns {void}
     */
    insert(value) {
        const extracted = extractLineFromArgs(arguments);
        const line = extracted.line ?? getCallerLine(2);
        const actualValue = extracted.args.length > 0 ? extracted.args[0] : value;

        const before = this._session.getLastStateId();
        this._items.push(actualValue);
        this._siftUp(this._items.length - 1);
        const after = this._session.addState(this._buildData());
        this._session.addStep('insert', before, after, { value: actualValue }, undefined, undefined, line);
    }

    /**
     * 提取堆顶元素，自动维护堆性质
     *
     * @returns {void}
     * @throws {StructureError} 当堆为空时抛出异常
     */
    extract() {
        const extracted = extractLineFromArgs(arguments);
        const line = extracted.line ?? getCallerLine(2);

        if (this._items.length === 0) {
            this._raiseError('Cannot extract from empty heap', line);
        }
        const before = this._session.getLastStateId();
        const extractedValue = this._items[0];
        if (this._items.length === 1) {
            this._items.pop();
        } else {
            this._items[0] = this._items.pop();
            this._siftDown(0);
        }
        const after = this._session.addState(this._buildData());
        this._session.addStep('extract', before, after, {}, extractedValue, undefined, line);
    }

    /**
     * 清空堆
     *
     * @returns {void}
     */
    clear() {
        const extracted = extractLineFromArgs(arguments);
        const line = extracted.line ?? getCallerLine(2);

        const before = this._session.getLastStateId();
        this._items.length = 0;
        const after = this._session.addState(this._buildData());
        this._session.addStep('clear', before, after, {}, undefined, undefined, line);
    }
}

// ============================================================================
// 图数据结构
// ============================================================================

/**
 * 无向图数据结构
 */
class GraphUndirected extends ContextManager {
    /**
     * 构造函数
     *
     * @param {StructureOptions} [options] - 结构选项
     */
    constructor(options) {
        super('graph_undirected', options?.label ?? 'graph', options?.output);
        /** @type {Map<number, string>} */
        this._nodes = new Map();
        /** @type {Set<string>} */
        this._edges = new Set();
    }

    /**
     * 构建无向图的状态数据
     *
     * @returns {StateData} 状态数据字典
     */
    _buildData() {
        const nodesList = [];
        const sortedIds = [...this._nodes.keys()].sort((a, b) => a - b);
        for (const nid of sortedIds) {
            nodesList.push({ id: nid, label: this._nodes.get(nid) });
        }
        const edgesList = [];
        const sortedEdges = [...this._edges].sort();
        for (const edgeKey of sortedEdges) {
            const [from, to] = edgeKey.split(',').map(Number);
            edgesList.push({ from, to });
        }
        return { nodes: nodesList, edges: edgesList };
    }

    /**
     * 初始化无向图
     *
     * @returns {void}
     */
    _initialize() {
        this._session.addState(this._buildData());
    }

    /**
     * 规范化无向边，确保 from < to
     *
     * @param {number} fromId - 起始节点 id
     * @param {number} toId - 终止节点 id
     * @returns {string} 规范化后的边键
     */
    static _normalizeEdge(fromId, toId) {
        return fromId < toId ? `${fromId},${toId}` : `${toId},${fromId}`;
    }

    /**
     * 添加节点
     *
     * @param {number} nodeId - 节点 id
     * @param {string} label - 节点标签，长度限制为 1-32 字符
     * @returns {void}
     * @throws {StructureError} 当节点已存在时抛出异常
     */
    addNode(nodeId, label) {
        const extracted = extractLineFromArgs(arguments);
        const line = extracted.line ?? getCallerLine(2);
        const actualNodeId = extracted.args.length >= 1 ? extracted.args[0] : nodeId;
        const actualLabel = extracted.args.length >= 2 ? extracted.args[1] : label;

        if (this._nodes.has(actualNodeId)) {
            this._raiseError(`Node already exists: ${actualNodeId}`, line);
        }
        if (actualLabel.length < 1 || actualLabel.length > 32) {
            this._raiseError(`Label length must be 1-32, got ${actualLabel.length}`, line);
        }
        const before = this._session.getLastStateId();
        this._nodes.set(actualNodeId, actualLabel);
        const after = this._session.addState(this._buildData());
        this._session.addStep('add_node', before, after, { id: actualNodeId, label: actualLabel }, undefined, undefined, line);
    }

    /**
     * 添加无向边，内部自动规范化为 fromId < toId
     *
     * @param {number} fromId - 起始节点 id
     * @param {number} toId - 终止节点 id
     * @returns {void}
     * @throws {StructureError} 当节点不存在、边已存在或形成自环时抛出异常
     */
    addEdge(fromId, toId) {
        const extracted = extractLineFromArgs(arguments);
        const line = extracted.line ?? getCallerLine(2);
        const actualFromId = extracted.args.length >= 1 ? extracted.args[0] : fromId;
        const actualToId = extracted.args.length >= 2 ? extracted.args[1] : toId;

        if (!this._nodes.has(actualFromId)) {
            this._raiseError(`Node not found: ${actualFromId}`, line);
        }
        if (!this._nodes.has(actualToId)) {
            this._raiseError(`Node not found: ${actualToId}`, line);
        }
        if (actualFromId === actualToId) {
            this._raiseError(`Self-loop not allowed: ${actualFromId}`, line);
        }
        const edgeKey = GraphUndirected._normalizeEdge(actualFromId, actualToId);
        if (this._edges.has(edgeKey)) {
            this._raiseError(`Edge already exists: (${actualFromId}, ${actualToId})`, line);
        }
        const before = this._session.getLastStateId();
        this._edges.add(edgeKey);
        const after = this._session.addState(this._buildData());
        this._session.addStep('add_edge', before, after, { from: actualFromId, to: actualToId }, undefined, undefined, line);
    }

    /**
     * 删除节点及其相关的所有边
     *
     * @param {number} nodeId - 要删除的节点 id
     * @returns {void}
     * @throws {StructureError} 当节点不存在时抛出异常
     */
    removeNode(nodeId) {
        const extracted = extractLineFromArgs(arguments);
        const line = extracted.line ?? getCallerLine(2);
        const actualNodeId = extracted.args.length > 0 ? extracted.args[0] : nodeId;

        if (!this._nodes.has(actualNodeId)) {
            this._raiseError(`Node not found: ${actualNodeId}`, line);
        }
        const before = this._session.getLastStateId();
        const edgesToRemove = [];
        for (const edgeKey of this._edges) {
            const [from, to] = edgeKey.split(',').map(Number);
            if (from === actualNodeId || to === actualNodeId) {
                edgesToRemove.push(edgeKey);
            }
        }
        for (const edgeKey of edgesToRemove) {
            this._edges.delete(edgeKey);
        }
        this._nodes.delete(actualNodeId);
        const after = this._session.addState(this._buildData());
        this._session.addStep('remove_node', before, after, { node_id: actualNodeId }, undefined, undefined, line);
    }

    /**
     * 删除边
     *
     * @param {number} fromId - 起始节点 id
     * @param {number} toId - 终止节点 id
     * @returns {void}
     * @throws {StructureError} 当边不存在时抛出异常
     */
    removeEdge(fromId, toId) {
        const extracted = extractLineFromArgs(arguments);
        const line = extracted.line ?? getCallerLine(2);
        const actualFromId = extracted.args.length >= 1 ? extracted.args[0] : fromId;
        const actualToId = extracted.args.length >= 2 ? extracted.args[1] : toId;

        const edgeKey = GraphUndirected._normalizeEdge(actualFromId, actualToId);
        if (!this._edges.has(edgeKey)) {
            this._raiseError(`Edge not found: (${actualFromId}, ${actualToId})`, line);
        }
        const before = this._session.getLastStateId();
        this._edges.delete(edgeKey);
        const after = this._session.addState(this._buildData());
        this._session.addStep('remove_edge', before, after, { from: actualFromId, to: actualToId }, undefined, undefined, line);
    }

    /**
     * 更新节点标签
     *
     * @param {number} nodeId - 节点 id
     * @param {string} label - 新的节点标签，长度限制为 1-32 字符
     * @returns {void}
     * @throws {StructureError} 当节点不存在时抛出异常
     */
    updateNodeLabel(nodeId, label) {
        const extracted = extractLineFromArgs(arguments);
        const line = extracted.line ?? getCallerLine(2);
        const actualNodeId = extracted.args.length >= 1 ? extracted.args[0] : nodeId;
        const actualLabel = extracted.args.length >= 2 ? extracted.args[1] : label;

        if (!this._nodes.has(actualNodeId)) {
            this._raiseError(`Node not found: ${actualNodeId}`, line);
        }
        if (actualLabel.length < 1 || actualLabel.length > 32) {
            this._raiseError(`Label length must be 1-32, got ${actualLabel.length}`, line);
        }
        const before = this._session.getLastStateId();
        const oldLabel = this._nodes.get(actualNodeId);
        this._nodes.set(actualNodeId, actualLabel);
        const after = this._session.addState(this._buildData());
        this._session.addStep('update_node_label', before, after, { node_id: actualNodeId, label: actualLabel }, oldLabel, undefined, line);
    }

    /**
     * 清空图
     *
     * @returns {void}
     */
    clear() {
        const extracted = extractLineFromArgs(arguments);
        const line = extracted.line ?? getCallerLine(2);

        const before = this._session.getLastStateId();
        this._nodes.clear();
        this._edges.clear();
        const after = this._session.addState(this._buildData());
        this._session.addStep('clear', before, after, {}, undefined, undefined, line);
    }
}

/**
 * 有向图数据结构
 */
class GraphDirected extends ContextManager {
    /**
     * 构造函数
     *
     * @param {StructureOptions} [options] - 结构选项
     */
    constructor(options) {
        super('graph_directed', options?.label ?? 'graph', options?.output);
        /** @type {Map<number, string>} */
        this._nodes = new Map();
        /** @type {Set<string>} */
        this._edges = new Set();
    }

    /**
     * 构建有向图的状态数据
     *
     * @returns {StateData} 状态数据字典
     */
    _buildData() {
        const nodesList = [];
        const sortedIds = [...this._nodes.keys()].sort((a, b) => a - b);
        for (const nid of sortedIds) {
            nodesList.push({ id: nid, label: this._nodes.get(nid) });
        }
        const edgesList = [];
        const sortedEdges = [...this._edges].sort();
        for (const edgeKey of sortedEdges) {
            const [from, to] = edgeKey.split(',').map(Number);
            edgesList.push({ from, to });
        }
        return { nodes: nodesList, edges: edgesList };
    }

    /**
     * 初始化有向图
     *
     * @returns {void}
     */
    _initialize() {
        this._session.addState(this._buildData());
    }

    /**
     * 添加节点
     *
     * @param {number} nodeId - 节点 id
     * @param {string} label - 节点标签，长度限制为 1-32 字符
     * @returns {void}
     * @throws {StructureError} 当节点已存在时抛出异常
     */
    addNode(nodeId, label) {
        const extracted = extractLineFromArgs(arguments);
        const line = extracted.line ?? getCallerLine(2);
        const actualNodeId = extracted.args.length >= 1 ? extracted.args[0] : nodeId;
        const actualLabel = extracted.args.length >= 2 ? extracted.args[1] : label;

        if (this._nodes.has(actualNodeId)) {
            this._raiseError(`Node already exists: ${actualNodeId}`, line);
        }
        if (actualLabel.length < 1 || actualLabel.length > 32) {
            this._raiseError(`Label length must be 1-32, got ${actualLabel.length}`, line);
        }
        const before = this._session.getLastStateId();
        this._nodes.set(actualNodeId, actualLabel);
        const after = this._session.addState(this._buildData());
        this._session.addStep('add_node', before, after, { id: actualNodeId, label: actualLabel }, undefined, undefined, line);
    }

    /**
     * 添加有向边
     *
     * @param {number} fromId - 起始节点 id
     * @param {number} toId - 终止节点 id
     * @returns {void}
     * @throws {StructureError} 当节点不存在、边已存在或形成自环时抛出异常
     */
    addEdge(fromId, toId) {
        const extracted = extractLineFromArgs(arguments);
        const line = extracted.line ?? getCallerLine(2);
        const actualFromId = extracted.args.length >= 1 ? extracted.args[0] : fromId;
        const actualToId = extracted.args.length >= 2 ? extracted.args[1] : toId;

        if (!this._nodes.has(actualFromId)) {
            this._raiseError(`Node not found: ${actualFromId}`, line);
        }
        if (!this._nodes.has(actualToId)) {
            this._raiseError(`Node not found: ${actualToId}`, line);
        }
        if (actualFromId === actualToId) {
            this._raiseError(`Self-loop not allowed: ${actualFromId}`, line);
        }
        const edgeKey = `${actualFromId},${actualToId}`;
        if (this._edges.has(edgeKey)) {
            this._raiseError(`Edge already exists: (${actualFromId}, ${actualToId})`, line);
        }
        const before = this._session.getLastStateId();
        this._edges.add(edgeKey);
        const after = this._session.addState(this._buildData());
        this._session.addStep('add_edge', before, after, { from: actualFromId, to: actualToId }, undefined, undefined, line);
    }

    /**
     * 删除节点及其相关的所有边
     *
     * @param {number} nodeId - 要删除的节点 id
     * @returns {void}
     * @throws {StructureError} 当节点不存在时抛出异常
     */
    removeNode(nodeId) {
        const extracted = extractLineFromArgs(arguments);
        const line = extracted.line ?? getCallerLine(2);
        const actualNodeId = extracted.args.length > 0 ? extracted.args[0] : nodeId;

        if (!this._nodes.has(actualNodeId)) {
            this._raiseError(`Node not found: ${actualNodeId}`, line);
        }
        const before = this._session.getLastStateId();
        const edgesToRemove = [];
        for (const edgeKey of this._edges) {
            const [from, to] = edgeKey.split(',').map(Number);
            if (from === actualNodeId || to === actualNodeId) {
                edgesToRemove.push(edgeKey);
            }
        }
        for (const edgeKey of edgesToRemove) {
            this._edges.delete(edgeKey);
        }
        this._nodes.delete(actualNodeId);
        const after = this._session.addState(this._buildData());
        this._session.addStep('remove_node', before, after, { node_id: actualNodeId }, undefined, undefined, line);
    }

    /**
     * 删除边
     *
     * @param {number} fromId - 起始节点 id
     * @param {number} toId - 终止节点 id
     * @returns {void}
     * @throws {StructureError} 当边不存在时抛出异常
     */
    removeEdge(fromId, toId) {
        const extracted = extractLineFromArgs(arguments);
        const line = extracted.line ?? getCallerLine(2);
        const actualFromId = extracted.args.length >= 1 ? extracted.args[0] : fromId;
        const actualToId = extracted.args.length >= 2 ? extracted.args[1] : toId;

        const edgeKey = `${actualFromId},${actualToId}`;
        if (!this._edges.has(edgeKey)) {
            this._raiseError(`Edge not found: (${actualFromId}, ${actualToId})`, line);
        }
        const before = this._session.getLastStateId();
        this._edges.delete(edgeKey);
        const after = this._session.addState(this._buildData());
        this._session.addStep('remove_edge', before, after, { from: actualFromId, to: actualToId }, undefined, undefined, line);
    }

    /**
     * 更新节点标签
     *
     * @param {number} nodeId - 节点 id
     * @param {string} label - 新的节点标签，长度限制为 1-32 字符
     * @returns {void}
     * @throws {StructureError} 当节点不存在时抛出异常
     */
    updateNodeLabel(nodeId, label) {
        const extracted = extractLineFromArgs(arguments);
        const line = extracted.line ?? getCallerLine(2);
        const actualNodeId = extracted.args.length >= 1 ? extracted.args[0] : nodeId;
        const actualLabel = extracted.args.length >= 2 ? extracted.args[1] : label;

        if (!this._nodes.has(actualNodeId)) {
            this._raiseError(`Node not found: ${actualNodeId}`, line);
        }
        if (actualLabel.length < 1 || actualLabel.length > 32) {
            this._raiseError(`Label length must be 1-32, got ${actualLabel.length}`, line);
        }
        const before = this._session.getLastStateId();
        const oldLabel = this._nodes.get(actualNodeId);
        this._nodes.set(actualNodeId, actualLabel);
        const after = this._session.addState(this._buildData());
        this._session.addStep('update_node_label', before, after, { node_id: actualNodeId, label: actualLabel }, oldLabel, undefined, line);
    }

    /**
     * 清空图
     *
     * @returns {void}
     */
    clear() {
        const extracted = extractLineFromArgs(arguments);
        const line = extracted.line ?? getCallerLine(2);

        const before = this._session.getLastStateId();
        this._nodes.clear();
        this._edges.clear();
        const after = this._session.addState(this._buildData());
        this._session.addStep('clear', before, after, {}, undefined, undefined, line);
    }
}

/**
 * 带权图选项接口
 *
 * @typedef {Object} GraphWeightedOptions
 * @property {string} [label] - 带权图的标签
 * @property {boolean} [directed=false] - 是否为有向图
 * @property {string} [output] - 输出文件路径
 */

/**
 * 带权图数据结构
 */
class GraphWeighted extends ContextManager {
    /**
     * 构造函数
     *
     * @param {GraphWeightedOptions} [options] - 带权图选项
     */
    constructor(options) {
        super('graph_weighted', options?.label ?? 'graph', options?.output);
        /** @type {boolean} */
        this._directed = options?.directed ?? false;
        /** @type {Map<number, string>} */
        this._nodes = new Map();
        /** @type {Map<string, number>} */
        this._edges = new Map();
    }

    /**
     * 构建带权图的状态数据
     *
     * @returns {StateData} 状态数据字典
     */
    _buildData() {
        const nodesList = [];
        const sortedIds = [...this._nodes.keys()].sort((a, b) => a - b);
        for (const nid of sortedIds) {
            nodesList.push({ id: nid, label: this._nodes.get(nid) });
        }
        const edgesList = [];
        const sortedEdges = [...this._edges.keys()].sort();
        for (const edgeKey of sortedEdges) {
            const [from, to] = edgeKey.split(',').map(Number);
            edgesList.push({ from, to, weight: this._edges.get(edgeKey) });
        }
        return { nodes: nodesList, edges: edgesList };
    }

    /**
     * 初始化带权图
     *
     * @returns {void}
     */
    _initialize() {
        this._session.addState(this._buildData());
    }

    /**
     * 规范化边（无向图时确保 from < to）
     *
     * @param {number} fromId - 起始节点 id
     * @param {number} toId - 终止节点 id
     * @returns {string} 规范化后的边键
     */
    _normalizeEdge(fromId, toId) {
        if (this._directed) {
            return `${fromId},${toId}`;
        }
        return fromId < toId ? `${fromId},${toId}` : `${toId},${fromId}`;
    }

    /**
     * 添加节点
     *
     * @param {number} nodeId - 节点 id
     * @param {string} label - 节点标签，长度限制为 1-32 字符
     * @returns {void}
     * @throws {StructureError} 当节点已存在时抛出异常
     */
    addNode(nodeId, label) {
        const extracted = extractLineFromArgs(arguments);
        const line = extracted.line ?? getCallerLine(2);
        const actualNodeId = extracted.args.length >= 1 ? extracted.args[0] : nodeId;
        const actualLabel = extracted.args.length >= 2 ? extracted.args[1] : label;

        if (this._nodes.has(actualNodeId)) {
            this._raiseError(`Node already exists: ${actualNodeId}`, line);
        }
        if (actualLabel.length < 1 || actualLabel.length > 32) {
            this._raiseError(`Label length must be 1-32, got ${actualLabel.length}`, line);
        }
        const before = this._session.getLastStateId();
        this._nodes.set(actualNodeId, actualLabel);
        const after = this._session.addState(this._buildData());
        this._session.addStep('add_node', before, after, { id: actualNodeId, label: actualLabel }, undefined, undefined, line);
    }

    /**
     * 添加带权边
     *
     * @param {number} fromId - 起始节点 id
     * @param {number} toId - 终止节点 id
     * @param {number} weight - 边的权重
     * @returns {void}
     * @throws {StructureError} 当节点不存在、边已存在或形成自环时抛出异常
     */
    addEdge(fromId, toId, weight) {
        const extracted = extractLineFromArgs(arguments);
        const line = extracted.line ?? getCallerLine(2);
        const actualFromId = extracted.args.length >= 1 ? extracted.args[0] : fromId;
        const actualToId = extracted.args.length >= 2 ? extracted.args[1] : toId;
        const actualWeight = extracted.args.length >= 3 ? extracted.args[2] : weight;

        if (!this._nodes.has(actualFromId)) {
            this._raiseError(`Node not found: ${actualFromId}`, line);
        }
        if (!this._nodes.has(actualToId)) {
            this._raiseError(`Node not found: ${actualToId}`, line);
        }
        if (actualFromId === actualToId) {
            this._raiseError(`Self-loop not allowed: ${actualFromId}`, line);
        }
        const edgeKey = this._normalizeEdge(actualFromId, actualToId);
        if (this._edges.has(edgeKey)) {
            this._raiseError(`Edge already exists: (${actualFromId}, ${actualToId})`, line);
        }
        const before = this._session.getLastStateId();
        this._edges.set(edgeKey, actualWeight);
        const after = this._session.addState(this._buildData());
        this._session.addStep('add_edge', before, after, { from: actualFromId, to: actualToId, weight: actualWeight }, undefined, undefined, line);
    }

    /**
     * 删除节点及其相关的所有边
     *
     * @param {number} nodeId - 要删除的节点 id
     * @returns {void}
     * @throws {StructureError} 当节点不存在时抛出异常
     */
    removeNode(nodeId) {
        const extracted = extractLineFromArgs(arguments);
        const line = extracted.line ?? getCallerLine(2);
        const actualNodeId = extracted.args.length > 0 ? extracted.args[0] : nodeId;

        if (!this._nodes.has(actualNodeId)) {
            this._raiseError(`Node not found: ${actualNodeId}`, line);
        }
        const before = this._session.getLastStateId();
        const edgesToRemove = [];
        for (const edgeKey of this._edges.keys()) {
            const [from, to] = edgeKey.split(',').map(Number);
            if (from === actualNodeId || to === actualNodeId) {
                edgesToRemove.push(edgeKey);
            }
        }
        for (const edgeKey of edgesToRemove) {
            this._edges.delete(edgeKey);
        }
        this._nodes.delete(actualNodeId);
        const after = this._session.addState(this._buildData());
        this._session.addStep('remove_node', before, after, { node_id: actualNodeId }, undefined, undefined, line);
    }

    /**
     * 删除边
     *
     * @param {number} fromId - 起始节点 id
     * @param {number} toId - 终止节点 id
     * @returns {void}
     * @throws {StructureError} 当边不存在时抛出异常
     */
    removeEdge(fromId, toId) {
        const extracted = extractLineFromArgs(arguments);
        const line = extracted.line ?? getCallerLine(2);
        const actualFromId = extracted.args.length >= 1 ? extracted.args[0] : fromId;
        const actualToId = extracted.args.length >= 2 ? extracted.args[1] : toId;

        const edgeKey = this._normalizeEdge(actualFromId, actualToId);
        if (!this._edges.has(edgeKey)) {
            this._raiseError(`Edge not found: (${actualFromId}, ${actualToId})`, line);
        }
        const before = this._session.getLastStateId();
        this._edges.delete(edgeKey);
        const after = this._session.addState(this._buildData());
        this._session.addStep('remove_edge', before, after, { from: actualFromId, to: actualToId }, undefined, undefined, line);
    }

    /**
     * 更新边的权重
     *
     * @param {number} fromId - 起始节点 id
     * @param {number} toId - 终止节点 id
     * @param {number} weight - 新的权重值
     * @returns {void}
     * @throws {StructureError} 当边不存在时抛出异常
     */
    updateWeight(fromId, toId, weight) {
        const extracted = extractLineFromArgs(arguments);
        const line = extracted.line ?? getCallerLine(2);
        const actualFromId = extracted.args.length >= 1 ? extracted.args[0] : fromId;
        const actualToId = extracted.args.length >= 2 ? extracted.args[1] : toId;
        const actualWeight = extracted.args.length >= 3 ? extracted.args[2] : weight;

        const edgeKey = this._normalizeEdge(actualFromId, actualToId);
        if (!this._edges.has(edgeKey)) {
            this._raiseError(`Edge not found: (${actualFromId}, ${actualToId})`, line);
        }
        const before = this._session.getLastStateId();
        const oldWeight = this._edges.get(edgeKey);
        this._edges.set(edgeKey, actualWeight);
        const after = this._session.addState(this._buildData());
        this._session.addStep('update_weight', before, after, { from: actualFromId, to: actualToId, weight: actualWeight }, oldWeight, undefined, line);
    }

    /**
     * 更新节点标签
     *
     * @param {number} nodeId - 节点 id
     * @param {string} label - 新的节点标签，长度限制为 1-32 字符
     * @returns {void}
     * @throws {StructureError} 当节点不存在时抛出异常
     */
    updateNodeLabel(nodeId, label) {
        const extracted = extractLineFromArgs(arguments);
        const line = extracted.line ?? getCallerLine(2);
        const actualNodeId = extracted.args.length >= 1 ? extracted.args[0] : nodeId;
        const actualLabel = extracted.args.length >= 2 ? extracted.args[1] : label;

        if (!this._nodes.has(actualNodeId)) {
            this._raiseError(`Node not found: ${actualNodeId}`, line);
        }
        if (actualLabel.length < 1 || actualLabel.length > 32) {
            this._raiseError(`Label length must be 1-32, got ${actualLabel.length}`, line);
        }
        const before = this._session.getLastStateId();
        const oldLabel = this._nodes.get(actualNodeId);
        this._nodes.set(actualNodeId, actualLabel);
        const after = this._session.addState(this._buildData());
        this._session.addStep('update_node_label', before, after, { node_id: actualNodeId, label: actualLabel }, oldLabel, undefined, line);
    }

    /**
     * 清空图
     *
     * @returns {void}
     */
    clear() {
        const extracted = extractLineFromArgs(arguments);
        const line = extracted.line ?? getCallerLine(2);

        const before = this._session.getLastStateId();
        this._nodes.clear();
        this._edges.clear();
        const after = this._session.addState(this._buildData());
        this._session.addStep('clear', before, after, {}, undefined, undefined, line);
    }
}

// ============================================================================
// 工厂函数
// ============================================================================

/**
 * 创建栈实例
 *
 * @param {StructureOptions} [options] - 结构选项
 * @returns {Stack} 栈实例
 */
function stack(options) {
    return new Stack(options);
}

/**
 * 创建队列实例
 *
 * @param {StructureOptions} [options] - 结构选项
 * @returns {Queue} 队列实例
 */
function queue(options) {
    return new Queue(options);
}

/**
 * 创建单链表实例
 *
 * @param {StructureOptions} [options] - 结构选项
 * @returns {SingleLinkedList} 单链表实例
 */
function singleLinkedList(options) {
    return new SingleLinkedList(options);
}

/**
 * 创建双向链表实例
 *
 * @param {StructureOptions} [options] - 结构选项
 * @returns {DoubleLinkedList} 双向链表实例
 */
function doubleLinkedList(options) {
    return new DoubleLinkedList(options);
}

/**
 * 创建二叉树实例
 *
 * @param {StructureOptions} [options] - 结构选项
 * @returns {BinaryTree} 二叉树实例
 */
function binaryTree(options) {
    return new BinaryTree(options);
}

/**
 * 创建二叉搜索树实例
 *
 * @param {StructureOptions} [options] - 结构选项
 * @returns {BinarySearchTree} 二叉搜索树实例
 */
function binarySearchTree(options) {
    return new BinarySearchTree(options);
}

/**
 * 创建堆实例
 *
 * @param {HeapOptions} [options] - 堆选项
 * @returns {Heap} 堆实例
 */
function heap(options) {
    return new Heap(options);
}

/**
 * 创建无向图实例
 *
 * @param {StructureOptions} [options] - 结构选项
 * @returns {GraphUndirected} 无向图实例
 */
function graphUndirected(options) {
    return new GraphUndirected(options);
}

/**
 * 创建有向图实例
 *
 * @param {StructureOptions} [options] - 结构选项
 * @returns {GraphDirected} 有向图实例
 */
function graphDirected(options) {
    return new GraphDirected(options);
}

/**
 * 创建带权图实例
 *
 * @param {GraphWeightedOptions} [options] - 带权图选项
 * @returns {GraphWeighted} 带权图实例
 */
function graphWeighted(options) {
    return new GraphWeighted(options);
}

/**
 * 获取数据结构生成的 TOML 字符串
 *
 * @param {ContextManager} structure - 数据结构实例
 * @returns {string|undefined} TOML 字符串
 */
function getTomlString(structure) {
    return structure.getTomlString();
}

// ============================================================================
// 版本信息
// ============================================================================

/** 库版本 */
const VERSION = '0.1.0';

/** 库作者 */
const AUTHOR = 'WaterRun <linzhuangrun49@gmail.com>';

// ============================================================================
// 模块导出
// ============================================================================

// UMD 模块导出
(function (root, factory) {
    if (typeof define === 'function' && define.amd) {
        define([], factory);
    } else if (typeof module === 'object' && module.exports) {
        module.exports = factory();
    } else {
        root.ds4viz = factory();
    }
}(typeof self !== 'undefined' ? self : this, function () {
    return {
        // 环境检测
        isNodeEnvironment,
        isBrowserEnvironment,

        // 错误类型
        ErrorType,
        DS4VizError,
        StructureError,
        ValidationError,

        // 配置
        config,
        getConfig,

        // 上下文管理
        withContext,
        getTomlString,

        // 工厂函数
        stack,
        queue,
        singleLinkedList,
        doubleLinkedList,
        binaryTree,
        binarySearchTree,
        heap,
        graphUndirected,
        graphDirected,
        graphWeighted,

        // 类导出
        Stack,
        Queue,
        SingleLinkedList,
        DoubleLinkedList,
        BinaryTree,
        BinarySearchTree,
        Heap,
        GraphUndirected,
        GraphDirected,
        GraphWeighted,

        createMeta,
        createPackage,
        createRemarks,
        isRemarksEmpty,
        createTraceObject,
        createState,
        createStep,
        createResult,
        createTraceError,
        createTrace,

        TomlWriter,
        Session,

        getJavaScriptVersion,
        getCallerLine,
        extractLineFromArgs,

        VERSION,
        AUTHOR,
    };
}));