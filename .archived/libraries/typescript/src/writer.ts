/**
 * TOML 文件写入器，将 Trace 对象序列化为 TOML 格式
 *
 * @file src/writer.ts
 * @author WaterRun
 * @date 2025-12-25
 */

import { writeFileSync } from 'node:fs';
import type { Trace, State, Step } from './trace.js';
import { isRemarksEmpty } from './trace.js';

/**
 * TOML 写入器类
 */
export class TomlWriter {
    readonly #trace: Trace;

    /**
     * 构造函数
     *
     * @param trace - 要序列化的 Trace 对象
     */
    constructor(trace: Trace) {
        this.#trace = trace;
    }

    /**
     * 将 Trace 写入指定路径的 TOML 文件
     *
     * @param path - 输出文件路径
     */
    public write(path: string): void {
        const content: string = this.#serialize();
        writeFileSync(path, content, 'utf-8');
    }

    /**
     * 将 Trace 序列化为 TOML 字符串
     *
     * @returns TOML 格式的字符串
     */
    #serialize(): string {
        const lines: string[] = [];

        lines.push(...this.#serializeMeta());
        lines.push('');
        lines.push(...this.#serializePackage());

        if (!isRemarksEmpty(this.#trace.remarks)) {
            lines.push('');
            lines.push(...this.#serializeRemarks());
        }

        lines.push('');
        lines.push(...this.#serializeObject());

        for (const state of this.#trace.states) {
            lines.push('');
            lines.push(...this.#serializeState(state));
        }

        for (const step of this.#trace.steps) {
            lines.push('');
            lines.push(...this.#serializeStep(step));
        }

        if (this.#trace.result !== undefined) {
            lines.push('');
            lines.push(...this.#serializeResult());
        }

        if (this.#trace.error !== undefined) {
            lines.push('');
            lines.push(...this.#serializeError());
        }

        return lines.join('\n');
    }

    /**
     * 序列化 meta 部分
     *
     * @returns TOML 行列表
     */
    #serializeMeta(): string[] {
        return [
            '[meta]',
            `created_at = "${this.#trace.meta.createdAt}"`,
            `lang = "${this.#trace.meta.lang}"`,
            `lang_version = "${this.#trace.meta.langVersion}"`,
        ];
    }

    /**
     * 序列化 package 部分
     *
     * @returns TOML 行列表
     */
    #serializePackage(): string[] {
        return [
            '[package]',
            `name = "${this.#trace.package.name}"`,
            `lang = "${this.#trace.package.lang}"`,
            `version = "${this.#trace.package.version}"`,
        ];
    }

    /**
     * 序列化 remarks 部分
     *
     * @returns TOML 行列表
     */
    #serializeRemarks(): string[] {
        const lines: string[] = ['[remarks]'];
        if (this.#trace.remarks.title !== '') {
            lines.push(`title = "${this.#escapeString(this.#trace.remarks.title)}"`);
        }
        if (this.#trace.remarks.author !== '') {
            lines.push(`author = "${this.#escapeString(this.#trace.remarks.author)}"`);
        }
        if (this.#trace.remarks.comment !== '') {
            lines.push(`comment = "${this.#escapeString(this.#trace.remarks.comment)}"`);
        }
        return lines;
    }

    /**
     * 序列化 object 部分
     *
     * @returns TOML 行列表
     */
    #serializeObject(): string[] {
        const lines: string[] = [
            '[object]',
            `kind = "${this.#trace.object.kind}"`,
        ];
        if (this.#trace.object.label !== '') {
            lines.push(`label = "${this.#escapeString(this.#trace.object.label)}"`);
        }
        return lines;
    }

    /**
     * 序列化单个 state
     *
     * @param state - 状态对象
     * @returns TOML 行列表
     */
    #serializeState(state: State): string[] {
        const lines: string[] = [
            '[[states]]',
            `id = ${state.id}`,
            '',
            '[states.data]',
        ];
        lines.push(...this.#serializeStateData(state.data));
        return lines;
    }

    /**
     * 序列化 state.data 部分
     *
     * @param data - 状态数据字典
     * @returns TOML 行列表
     */
    #serializeStateData(data: Record<string, unknown>): string[] {
        const lines: string[] = [];
        for (const [key, value] of Object.entries(data)) {
            if (key === 'nodes' || key === 'edges') {
                lines.push(`${key} = ${this.#serializeInlineTableArray(value as Record<string, unknown>[])}`);
            } else if (key === 'items') {
                lines.push(`${key} = ${this.#serializeSimpleArray(value as unknown[])}`);
            } else {
                lines.push(`${key} = ${this.#serializeValue(value)}`);
            }
        }
        return lines;
    }

    /**
     * 序列化 inline table 数组
     *
     * @param arr - 字典数组
     * @returns TOML 格式的 inline table 数组字符串
     */
    #serializeInlineTableArray(arr: Record<string, unknown>[]): string {
        if (arr.length === 0) {
            return '[]';
        }
        const items: string[] = arr.map((item) => {
            const pairs: string[] = Object.entries(item).map(
                ([k, v]) => `${k} = ${this.#serializeValue(v)}`
            );
            return '{ ' + pairs.join(', ') + ' }';
        });
        return '[\n  ' + items.join(',\n  ') + '\n]';
    }

    /**
     * 序列化简单数组
     *
     * @param arr - 值数组
     * @returns TOML 格式的数组字符串
     */
    #serializeSimpleArray(arr: unknown[]): string {
        if (arr.length === 0) {
            return '[]';
        }
        const items: string[] = arr.map((v) => this.#serializeValue(v));
        return '[' + items.join(', ') + ']';
    }

    /**
     * 序列化单个 step
     *
     * @param step - 步骤对象
     * @returns TOML 行列表
     */
    #serializeStep(step: Step): string[] {
        const lines: string[] = [
            '[[steps]]',
            `id = ${step.id}`,
            `op = "${step.op}"`,
            `before = ${step.before}`,
        ];

        if (step.after !== undefined) {
            lines.push(`after = ${step.after}`);
        }

        if (step.ret !== undefined) {
            lines.push(`ret = ${this.#serializeValue(step.ret)}`);
        }

        if (step.note !== undefined) {
            lines.push(`note = "${this.#escapeString(step.note)}"`);
        }

        lines.push('');
        lines.push('[steps.args]');

        for (const [key, value] of Object.entries(step.args)) {
            lines.push(`${key} = ${this.#serializeValue(value)}`);
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
     * @returns TOML 行列表
     */
    #serializeResult(): string[] {
        const result = this.#trace.result;
        if (result === undefined) {
            return [];
        }
        const lines: string[] = [
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
     * @returns TOML 行列表
     */
    #serializeError(): string[] {
        const error = this.#trace.error;
        if (error === undefined) {
            return [];
        }
        const lines: string[] = [
            '[error]',
            `type = "${error.type}"`,
            `message = "${this.#escapeString(error.message)}"`,
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
     * @param value - 要序列化的值
     * @returns TOML 格式的值字符串
     */
    #serializeValue(value: unknown): string {
        if (typeof value === 'boolean') {
            return value ? 'true' : 'false';
        }
        if (typeof value === 'number') {
            return String(value);
        }
        if (typeof value === 'string') {
            return `"${this.#escapeString(value)}"`;
        }
        if (Array.isArray(value)) {
            return this.#serializeSimpleArray(value);
        }
        if (typeof value === 'object' && value !== null) {
            const pairs: string[] = Object.entries(value as Record<string, unknown>).map(
                ([k, v]) => `${k} = ${this.#serializeValue(v)}`
            );
            return '{ ' + pairs.join(', ') + ' }';
        }
        return `"${this.#escapeString(String(value))}"`;
    }

    /**
     * 转义字符串中的特殊字符
     *
     * @param s - 原始字符串
     * @returns 转义后的字符串
     */
    #escapeString(s: string): string {
        return s
            .replace(/\\/g, '\\\\')
            .replace(/"/g, '\\"')
            .replace(/\n/g, '\\n')
            .replace(/\r/g, '\\r');
    }
}
