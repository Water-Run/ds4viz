/**
 * 线性数据结构实现：栈、队列、单链表、双向链表
 *
 * @file src/structures/linear.ts
 * @author WaterRun
 * @date 2025-12-25
 */

import {
    ContextManager,
    type StructureOptions,
    type Disposable,
    getCallerLine,
} from '../session.js';
import type { StateData } from '../trace.js';

/**
 * 值类型定义
 *
 * @typedef
 */
export type ValueType = number | string | boolean;

/**
 * 栈数据结构
 */
export class Stack extends ContextManager implements Disposable {
    readonly #items: ValueType[];

    /**
     * 构造函数
     *
     * @param options - 结构选项
     */
    constructor(options?: StructureOptions) {
        super('stack', options?.label ?? 'stack', options?.output);
        this.#items = [];
    }

    /**
     * 构建栈的状态数据
     *
     * @returns 状态数据字典
     */
    protected buildData(): StateData {
        return {
            items: [...this.#items],
            top: this.#items.length > 0 ? this.#items.length - 1 : -1,
        };
    }

    /**
     * 初始化栈
     */
    protected initialize(): void {
        this.session.addState(this.buildData());
    }

    /**
     * 压栈操作
     *
     * @param value - 要压入的值
     */
    public push(value: ValueType): void {
        const before: number = this.session.getLastStateId();
        const line: number = getCallerLine(2);
        this.#items.push(value);
        const after: number = this.session.addState(this.buildData());
        this.session.addStep('push', before, after, { value }, undefined, undefined, line);
    }

    /**
     * 弹栈操作
     *
     * @throws {StructureError} 当栈为空时抛出异常
     */
    public pop(): void {
        if (this.#items.length === 0) {
            this.raiseError('Cannot pop from empty stack');
        }
        const before: number = this.session.getLastStateId();
        const line: number = getCallerLine(2);
        const popped: ValueType = this.#items.pop()!;
        const after: number = this.session.addState(this.buildData());
        this.session.addStep('pop', before, after, {}, popped, undefined, line);
    }

    /**
     * 清空栈
     */
    public clear(): void {
        const before: number = this.session.getLastStateId();
        const line: number = getCallerLine(2);
        this.#items.length = 0;
        const after: number = this.session.addState(this.buildData());
        this.session.addStep('clear', before, after, {}, undefined, undefined, line);
    }
}

/**
 * 队列数据结构
 */
export class Queue extends ContextManager implements Disposable {
    readonly #items: ValueType[];

    /**
     * 构造函数
     *
     * @param options - 结构选项
     */
    constructor(options?: StructureOptions) {
        super('queue', options?.label ?? 'queue', options?.output);
        this.#items = [];
    }

    /**
     * 构建队列的状态数据
     *
     * @returns 状态数据字典
     */
    protected buildData(): StateData {
        if (this.#items.length === 0) {
            return { items: [], front: -1, rear: -1 };
        }
        return {
            items: [...this.#items],
            front: 0,
            rear: this.#items.length - 1,
        };
    }

    /**
     * 初始化队列
     */
    protected initialize(): void {
        this.session.addState(this.buildData());
    }

    /**
     * 入队操作
     *
     * @param value - 要入队的值
     */
    public enqueue(value: ValueType): void {
        const before: number = this.session.getLastStateId();
        const line: number = getCallerLine(2);
        this.#items.push(value);
        const after: number = this.session.addState(this.buildData());
        this.session.addStep('enqueue', before, after, { value }, undefined, undefined, line);
    }

    /**
     * 出队操作
     *
     * @throws {StructureError} 当队列为空时抛出异常
     */
    public dequeue(): void {
        if (this.#items.length === 0) {
            this.raiseError('Cannot dequeue from empty queue');
        }
        const before: number = this.session.getLastStateId();
        const line: number = getCallerLine(2);
        const dequeued: ValueType = this.#items.shift()!;
        const after: number = this.session.addState(this.buildData());
        this.session.addStep('dequeue', before, after, {}, dequeued, undefined, line);
    }

    /**
     * 清空队列
     */
    public clear(): void {
        const before: number = this.session.getLastStateId();
        const line: number = getCallerLine(2);
        this.#items.length = 0;
        const after: number = this.session.addState(this.buildData());
        this.session.addStep('clear', before, after, {}, undefined, undefined, line);
    }
}

/**
 * 单链表节点数据
 *
 * @interface
 */
interface SListNodeData {
    value: ValueType;
    next: number;
}

/**
 * 单链表数据结构
 */
export class SingleLinkedList extends ContextManager implements Disposable {
    readonly #nodes: Map<number, SListNodeData>;
    #head: number;
    #nextId: number;

    /**
     * 构造函数
     *
     * @param options - 结构选项
     */
    constructor(options?: StructureOptions) {
        super('slist', options?.label ?? 'slist', options?.output);
        this.#nodes = new Map();
        this.#head = -1;
        this.#nextId = 0;
    }

    /**
     * 构建单链表的状态数据
     *
     * @returns 状态数据字典
     */
    protected buildData(): StateData {
        const nodesList: Array<{ id: number; value: ValueType; next: number }> = [];
        const sortedIds = [...this.#nodes.keys()].sort((a, b) => a - b);
        for (const nid of sortedIds) {
            const data = this.#nodes.get(nid)!;
            nodesList.push({ id: nid, value: data.value, next: data.next });
        }
        return { head: this.#head, nodes: nodesList };
    }

    /**
     * 初始化单链表
     */
    protected initialize(): void {
        this.session.addState(this.buildData());
    }

    /**
     * 查找尾节点 ID
     *
     * @returns 尾节点 ID，若链表为空则返回 -1
     */
    #findTail(): number {
        if (this.#head === -1) {
            return -1;
        }
        let current: number = this.#head;
        let node = this.#nodes.get(current);
        while (node !== undefined && node.next !== -1) {
            current = node.next;
            node = this.#nodes.get(current);
        }
        return current;
    }

    /**
     * 查找指定节点的前驱节点 ID
     *
     * @param nodeId - 目标节点 ID
     * @returns 前驱节点 ID，若为头节点或不存在则返回 -1
     */
    #findPrev(nodeId: number): number {
        if (this.#head === -1 || this.#head === nodeId) {
            return -1;
        }
        let current: number = this.#head;
        let node = this.#nodes.get(current);
        while (node !== undefined) {
            if (node.next === nodeId) {
                return current;
            }
            current = node.next;
            node = this.#nodes.get(current);
        }
        return -1;
    }

    /**
     * 在链表头部插入节点
     *
     * @param value - 要插入的值
     * @returns 新插入节点的 id
     */
    public insertHead(value: ValueType): number {
        const before: number = this.session.getLastStateId();
        const line: number = getCallerLine(2);
        const newId: number = this.#nextId;
        this.#nextId += 1;
        this.#nodes.set(newId, { value, next: this.#head });
        this.#head = newId;
        const after: number = this.session.addState(this.buildData());
        this.session.addStep('insert_head', before, after, { value }, newId, undefined, line);
        return newId;
    }

    /**
     * 在链表尾部插入节点
     *
     * @param value - 要插入的值
     * @returns 新插入节点的 id
     */
    public insertTail(value: ValueType): number {
        const before: number = this.session.getLastStateId();
        const line: number = getCallerLine(2);
        const newId: number = this.#nextId;
        this.#nextId += 1;
        this.#nodes.set(newId, { value, next: -1 });
        if (this.#head === -1) {
            this.#head = newId;
        } else {
            const tail: number = this.#findTail();
            const tailNode = this.#nodes.get(tail);
            if (tailNode !== undefined) {
                tailNode.next = newId;
            }
        }
        const after: number = this.session.addState(this.buildData());
        this.session.addStep('insert_tail', before, after, { value }, newId, undefined, line);
        return newId;
    }

    /**
     * 在指定节点后插入新节点
     *
     * @param nodeId - 目标节点的 id
     * @param value - 要插入的值
     * @returns 新插入节点的 id
     * @throws {StructureError} 当指定节点不存在时抛出异常
     */
    public insertAfter(nodeId: number, value: ValueType): number {
        if (!this.#nodes.has(nodeId)) {
            this.raiseError(`Node not found: ${nodeId}`);
        }
        const before: number = this.session.getLastStateId();
        const line: number = getCallerLine(2);
        const newId: number = this.#nextId;
        this.#nextId += 1;
        const targetNode = this.#nodes.get(nodeId)!;
        const oldNext: number = targetNode.next;
        this.#nodes.set(newId, { value, next: oldNext });
        targetNode.next = newId;
        const after: number = this.session.addState(this.buildData());
        this.session.addStep('insert_after', before, after, { node_id: nodeId, value }, newId, undefined, line);
        return newId;
    }

    /**
     * 删除指定节点
     *
     * @param nodeId - 要删除的节点 id
     * @throws {StructureError} 当指定节点不存在时抛出异常
     */
    public delete(nodeId: number): void {
        if (!this.#nodes.has(nodeId)) {
            this.raiseError(`Node not found: ${nodeId}`);
        }
        const before: number = this.session.getLastStateId();
        const line: number = getCallerLine(2);
        const deletedNode = this.#nodes.get(nodeId)!;
        const deletedValue: ValueType = deletedNode.value;
        if (this.#head === nodeId) {
            this.#head = deletedNode.next;
        } else {
            const prev: number = this.#findPrev(nodeId);
            if (prev !== -1) {
                const prevNode = this.#nodes.get(prev);
                if (prevNode !== undefined) {
                    prevNode.next = deletedNode.next;
                }
            }
        }
        this.#nodes.delete(nodeId);
        const after: number = this.session.addState(this.buildData());
        this.session.addStep('delete', before, after, { node_id: nodeId }, deletedValue, undefined, line);
    }

    /**
     * 删除头节点
     *
     * @throws {StructureError} 当链表为空时抛出异常
     */
    public deleteHead(): void {
        if (this.#head === -1) {
            this.raiseError('Cannot delete from empty list');
        }
        const before: number = this.session.getLastStateId();
        const line: number = getCallerLine(2);
        const oldHead: number = this.#head;
        const headNode = this.#nodes.get(oldHead)!;
        const deletedValue: ValueType = headNode.value;
        this.#head = headNode.next;
        this.#nodes.delete(oldHead);
        const after: number = this.session.addState(this.buildData());
        this.session.addStep('delete_head', before, after, {}, deletedValue, undefined, line);
    }

    /**
     * 反转链表
     */
    public reverse(): void {
        const before: number = this.session.getLastStateId();
        const line: number = getCallerLine(2);
        let prev: number = -1;
        let current: number = this.#head;
        while (current !== -1) {
            const node = this.#nodes.get(current);
            if (node === undefined) break;
            const nextNode: number = node.next;
            node.next = prev;
            prev = current;
            current = nextNode;
        }
        this.#head = prev;
        const after: number = this.session.addState(this.buildData());
        this.session.addStep('reverse', before, after, {}, undefined, undefined, line);
    }
}

/**
 * 双向链表节点数据
 *
 * @interface
 */
interface DListNodeData {
    value: ValueType;
    prev: number;
    next: number;
}

/**
 * 双向链表数据结构
 */
export class DoubleLinkedList extends ContextManager implements Disposable {
    readonly #nodes: Map<number, DListNodeData>;
    #head: number;
    #tail: number;
    #nextId: number;

    /**
     * 构造函数
     *
     * @param options - 结构选项
     */
    constructor(options?: StructureOptions) {
        super('dlist', options?.label ?? 'dlist', options?.output);
        this.#nodes = new Map();
        this.#head = -1;
        this.#tail = -1;
        this.#nextId = 0;
    }

    /**
     * 构建双向链表的状态数据
     *
     * @returns 状态数据字典
     */
    protected buildData(): StateData {
        const nodesList: Array<{ id: number; value: ValueType; prev: number; next: number }> = [];
        const sortedIds = [...this.#nodes.keys()].sort((a, b) => a - b);
        for (const nid of sortedIds) {
            const data = this.#nodes.get(nid)!;
            nodesList.push({ id: nid, value: data.value, prev: data.prev, next: data.next });
        }
        return { head: this.#head, tail: this.#tail, nodes: nodesList };
    }

    /**
     * 初始化双向链表
     */
    protected initialize(): void {
        this.session.addState(this.buildData());
    }

    /**
     * 在链表头部插入节点
     *
     * @param value - 要插入的值
     * @returns 新插入节点的 id
     */
    public insertHead(value: ValueType): number {
        const before: number = this.session.getLastStateId();
        const line: number = getCallerLine(2);
        const newId: number = this.#nextId;
        this.#nextId += 1;
        this.#nodes.set(newId, { value, prev: -1, next: this.#head });
        if (this.#head !== -1) {
            const headNode = this.#nodes.get(this.#head);
            if (headNode !== undefined) {
                headNode.prev = newId;
            }
        }
        this.#head = newId;
        if (this.#tail === -1) {
            this.#tail = newId;
        }
        const after: number = this.session.addState(this.buildData());
        this.session.addStep('insert_head', before, after, { value }, newId, undefined, line);
        return newId;
    }

    /**
     * 在链表尾部插入节点
     *
     * @param value - 要插入的值
     * @returns 新插入节点的 id
     */
    public insertTail(value: ValueType): number {
        const before: number = this.session.getLastStateId();
        const line: number = getCallerLine(2);
        const newId: number = this.#nextId;
        this.#nextId += 1;
        this.#nodes.set(newId, { value, prev: this.#tail, next: -1 });
        if (this.#tail !== -1) {
            const tailNode = this.#nodes.get(this.#tail);
            if (tailNode !== undefined) {
                tailNode.next = newId;
            }
        }
        this.#tail = newId;
        if (this.#head === -1) {
            this.#head = newId;
        }
        const after: number = this.session.addState(this.buildData());
        this.session.addStep('insert_tail', before, after, { value }, newId, undefined, line);
        return newId;
    }

    /**
     * 在指定节点前插入新节点
     *
     * @param nodeId - 目标节点的 id
     * @param value - 要插入的值
     * @returns 新插入节点的 id
     * @throws {StructureError} 当指定节点不存在时抛出异常
     */
    public insertBefore(nodeId: number, value: ValueType): number {
        if (!this.#nodes.has(nodeId)) {
            this.raiseError(`Node not found: ${nodeId}`);
        }
        const before: number = this.session.getLastStateId();
        const line: number = getCallerLine(2);
        const newId: number = this.#nextId;
        this.#nextId += 1;
        const targetNode = this.#nodes.get(nodeId)!;
        const oldPrev: number = targetNode.prev;
        this.#nodes.set(newId, { value, prev: oldPrev, next: nodeId });
        targetNode.prev = newId;
        if (oldPrev !== -1) {
            const prevNode = this.#nodes.get(oldPrev);
            if (prevNode !== undefined) {
                prevNode.next = newId;
            }
        } else {
            this.#head = newId;
        }
        const after: number = this.session.addState(this.buildData());
        this.session.addStep('insert_before', before, after, { node_id: nodeId, value }, newId, undefined, line);
        return newId;
    }

    /**
     * 在指定节点后插入新节点
     *
     * @param nodeId - 目标节点的 id
     * @param value - 要插入的值
     * @returns 新插入节点的 id
     * @throws {StructureError} 当指定节点不存在时抛出异常
     */
    public insertAfter(nodeId: number, value: ValueType): number {
        if (!this.#nodes.has(nodeId)) {
            this.raiseError(`Node not found: ${nodeId}`);
        }
        const before: number = this.session.getLastStateId();
        const line: number = getCallerLine(2);
        const newId: number = this.#nextId;
        this.#nextId += 1;
        const targetNode = this.#nodes.get(nodeId)!;
        const oldNext: number = targetNode.next;
        this.#nodes.set(newId, { value, prev: nodeId, next: oldNext });
        targetNode.next = newId;
        if (oldNext !== -1) {
            const nextNode = this.#nodes.get(oldNext);
            if (nextNode !== undefined) {
                nextNode.prev = newId;
            }
        } else {
            this.#tail = newId;
        }
        const after: number = this.session.addState(this.buildData());
        this.session.addStep('insert_after', before, after, { node_id: nodeId, value }, newId, undefined, line);
        return newId;
    }

    /**
     * 删除指定节点
     *
     * @param nodeId - 要删除的节点 id
     * @throws {StructureError} 当指定节点不存在时抛出异常
     */
    public delete(nodeId: number): void {
        if (!this.#nodes.has(nodeId)) {
            this.raiseError(`Node not found: ${nodeId}`);
        }
        const before: number = this.session.getLastStateId();
        const line: number = getCallerLine(2);
        const nodeData = this.#nodes.get(nodeId)!;
        const deletedValue: ValueType = nodeData.value;
        const prevId: number = nodeData.prev;
        const nextId: number = nodeData.next;

        if (prevId !== -1) {
            const prevNode = this.#nodes.get(prevId);
            if (prevNode !== undefined) {
                prevNode.next = nextId;
            }
        } else {
            this.#head = nextId;
        }

        if (nextId !== -1) {
            const nextNode = this.#nodes.get(nextId);
            if (nextNode !== undefined) {
                nextNode.prev = prevId;
            }
        } else {
            this.#tail = prevId;
        }

        this.#nodes.delete(nodeId);
        const after: number = this.session.addState(this.buildData());
        this.session.addStep('delete', before, after, { node_id: nodeId }, deletedValue, undefined, line);
    }

    /**
     * 删除头节点
     *
     * @throws {StructureError} 当链表为空时抛出异常
     */
    public deleteHead(): void {
        if (this.#head === -1) {
            this.raiseError('Cannot delete from empty list');
        }
        this.delete(this.#head);
    }

    /**
     * 删除尾节点
     *
     * @throws {StructureError} 当链表为空时抛出异常
     */
    public deleteTail(): void {
        if (this.#tail === -1) {
            this.raiseError('Cannot delete from empty list');
        }
        this.delete(this.#tail);
    }

    /**
     * 反转链表
     */
    public reverse(): void {
        const before: number = this.session.getLastStateId();
        const line: number = getCallerLine(2);
        let current: number = this.#head;
        while (current !== -1) {
            const node = this.#nodes.get(current);
            if (node === undefined) break;
            const temp = node.prev;
            node.prev = node.next;
            node.next = temp;
            current = node.prev;
        }
        const temp = this.#head;
        this.#head = this.#tail;
        this.#tail = temp;
        const after: number = this.session.addState(this.buildData());
        this.session.addStep('reverse', before, after, {}, undefined, undefined, line);
    }
}
