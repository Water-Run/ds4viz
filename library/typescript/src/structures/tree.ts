/**
 * 树形数据结构实现：二叉树、二叉搜索树、堆
 *
 * @file src/structures/tree.ts
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
 * 二叉树节点数据
 *
 * @interface
 */
interface TreeNodeData {
    value: ValueType;
    left: number;
    right: number;
}

/**
 * 二叉树数据结构
 */
export class BinaryTree extends ContextManager implements Disposable {
    readonly #nodes: Map<number, TreeNodeData>;
    #root: number;
    #nextId: number;

    /**
     * 构造函数
     *
     * @param options - 结构选项
     */
    constructor(options?: StructureOptions) {
        super('binary_tree', options?.label ?? 'binary_tree', options?.output);
        this.#nodes = new Map();
        this.#root = -1;
        this.#nextId = 0;
    }

    /**
     * 构建二叉树的状态数据
     *
     * @returns 状态数据字典
     */
    protected buildData(): StateData {
        const nodesList: Array<{ id: number; value: ValueType; left: number; right: number }> = [];
        const sortedIds = [...this.#nodes.keys()].sort((a, b) => a - b);
        for (const nid of sortedIds) {
            const data = this.#nodes.get(nid)!;
            nodesList.push({ id: nid, value: data.value, left: data.left, right: data.right });
        }
        return { root: this.#root, nodes: nodesList };
    }

    /**
     * 初始化二叉树
     */
    protected initialize(): void {
        this.session.addState(this.buildData());
    }

    /**
     * 查找指定节点的父节点 ID
     *
     * @param nodeId - 目标节点 ID
     * @returns 父节点 ID，若为根节点或不存在则返回 -1
     */
    #findParent(nodeId: number): number {
        for (const [nid, data] of this.#nodes) {
            if (data.left === nodeId || data.right === nodeId) {
                return nid;
            }
        }
        return -1;
    }

    /**
     * 收集以指定节点为根的子树中所有节点 ID
     *
     * @param nodeId - 根节点 ID
     * @returns 子树中所有节点 ID 列表
     */
    #collectSubtree(nodeId: number): number[] {
        if (nodeId === -1) {
            return [];
        }
        const node = this.#nodes.get(nodeId);
        if (node === undefined) {
            return [];
        }
        const result: number[] = [nodeId];
        result.push(...this.#collectSubtree(node.left));
        result.push(...this.#collectSubtree(node.right));
        return result;
    }

    /**
     * 插入根节点
     *
     * @param value - 根节点的值
     * @returns 根节点的 id
     * @throws {StructureError} 当根节点已存在时抛出异常
     */
    public insertRoot(value: ValueType): number {
        if (this.#root !== -1) {
            this.raiseError('Root node already exists');
        }
        const before: number = this.session.getLastStateId();
        const line: number = getCallerLine(2);
        const newId: number = this.#nextId;
        this.#nextId += 1;
        this.#nodes.set(newId, { value, left: -1, right: -1 });
        this.#root = newId;
        const after: number = this.session.addState(this.buildData());
        this.session.addStep('insert_root', before, after, { value }, newId, undefined, line);
        return newId;
    }

    /**
     * 在指定父节点的左侧插入子节点
     *
     * @param parentId - 父节点的 id
     * @param value - 要插入的子节点值
     * @returns 新插入节点的 id
     * @throws {StructureError} 当父节点不存在或左子节点已存在时抛出异常
     */
    public insertLeft(parentId: number, value: ValueType): number {
        if (!this.#nodes.has(parentId)) {
            this.raiseError(`Parent node not found: ${parentId}`);
        }
        const parentNode = this.#nodes.get(parentId)!;
        if (parentNode.left !== -1) {
            this.raiseError(`Left child already exists for node: ${parentId}`);
        }
        const before: number = this.session.getLastStateId();
        const line: number = getCallerLine(2);
        const newId: number = this.#nextId;
        this.#nextId += 1;
        this.#nodes.set(newId, { value, left: -1, right: -1 });
        parentNode.left = newId;
        const after: number = this.session.addState(this.buildData());
        this.session.addStep('insert_left', before, after, { parent: parentId, value }, newId, undefined, line);
        return newId;
    }

    /**
     * 在指定父节点的右侧插入子节点
     *
     * @param parentId - 父节点的 id
     * @param value - 要插入的子节点值
     * @returns 新插入节点的 id
     * @throws {StructureError} 当父节点不存在或右子节点已存在时抛出异常
     */
    public insertRight(parentId: number, value: ValueType): number {
        if (!this.#nodes.has(parentId)) {
            this.raiseError(`Parent node not found: ${parentId}`);
        }
        const parentNode = this.#nodes.get(parentId)!;
        if (parentNode.right !== -1) {
            this.raiseError(`Right child already exists for node: ${parentId}`);
        }
        const before: number = this.session.getLastStateId();
        const line: number = getCallerLine(2);
        const newId: number = this.#nextId;
        this.#nextId += 1;
        this.#nodes.set(newId, { value, left: -1, right: -1 });
        parentNode.right = newId;
        const after: number = this.session.addState(this.buildData());
        this.session.addStep('insert_right', before, after, { parent: parentId, value }, newId, undefined, line);
        return newId;
    }

    /**
     * 删除指定节点及其所有子树
     *
     * @param nodeId - 要删除的节点 id
     * @throws {StructureError} 当节点不存在时抛出异常
     */
    public delete(nodeId: number): void {
        if (!this.#nodes.has(nodeId)) {
            this.raiseError(`Node not found: ${nodeId}`);
        }
        const before: number = this.session.getLastStateId();
        const line: number = getCallerLine(2);
        const subtreeIds: number[] = this.#collectSubtree(nodeId);
        const parentId: number = this.#findParent(nodeId);
        if (parentId !== -1) {
            const parentNode = this.#nodes.get(parentId);
            if (parentNode !== undefined) {
                if (parentNode.left === nodeId) {
                    parentNode.left = -1;
                } else {
                    parentNode.right = -1;
                }
            }
        } else {
            this.#root = -1;
        }
        for (const nid of subtreeIds) {
            this.#nodes.delete(nid);
        }
        const after: number = this.session.addState(this.buildData());
        this.session.addStep('delete', before, after, { node_id: nodeId }, undefined, undefined, line);
    }

    /**
     * 更新节点的值
     *
     * @param nodeId - 节点 id
     * @param value - 新的值
     * @throws {StructureError} 当节点不存在时抛出异常
     */
    public updateValue(nodeId: number, value: ValueType): void {
        if (!this.#nodes.has(nodeId)) {
            this.raiseError(`Node not found: ${nodeId}`);
        }
        const before: number = this.session.getLastStateId();
        const line: number = getCallerLine(2);
        const node = this.#nodes.get(nodeId)!;
        const oldValue: ValueType = node.value;
        node.value = value;
        const after: number = this.session.addState(this.buildData());
        this.session.addStep('update_value', before, after, { node_id: nodeId, value }, oldValue, undefined, line);
    }
}

/**
 * 二叉搜索树数据结构
 */
export class BinarySearchTree extends ContextManager implements Disposable {
    readonly #nodes: Map<number, TreeNodeData>;
    #root: number;
    #nextId: number;

    /**
     * 构造函数
     *
     * @param options - 结构选项
     */
    constructor(options?: StructureOptions) {
        super('bst', options?.label ?? 'bst', options?.output);
        this.#nodes = new Map();
        this.#root = -1;
        this.#nextId = 0;
    }

    /**
     * 构建二叉搜索树的状态数据
     *
     * @returns 状态数据字典
     */
    protected buildData(): StateData {
        const nodesList: Array<{ id: number; value: number; left: number; right: number }> = [];
        const sortedIds = [...this.#nodes.keys()].sort((a, b) => a - b);
        for (const nid of sortedIds) {
            const data = this.#nodes.get(nid)!;
            nodesList.push({
                id: nid,
                value: data.value as number,
                left: data.left,
                right: data.right,
            });
        }
        return { root: this.#root, nodes: nodesList };
    }

    /**
     * 初始化二叉搜索树
     */
    protected initialize(): void {
        this.session.addState(this.buildData());
    }

    /**
     * 根据值查找节点 ID
     *
     * @param value - 要查找的值
     * @returns 节点 ID，若不存在则返回 -1
     */
    #findNodeByValue(value: number): number {
        for (const [nid, data] of this.#nodes) {
            if (data.value === value) {
                return nid;
            }
        }
        return -1;
    }

    /**
     * 查找指定节点的父节点 ID
     *
     * @param nodeId - 目标节点 ID
     * @returns 父节点 ID，若为根节点或不存在则返回 -1
     */
    #findParent(nodeId: number): number {
        for (const [nid, data] of this.#nodes) {
            if (data.left === nodeId || data.right === nodeId) {
                return nid;
            }
        }
        return -1;
    }

    /**
     * 查找以指定节点为根的子树中的最小值节点
     *
     * @param nodeId - 子树根节点 ID
     * @returns 最小值节点 ID
     */
    #findMinNode(nodeId: number): number {
        let current: number = nodeId;
        let node = this.#nodes.get(current);
        while (node !== undefined && node.left !== -1) {
            current = node.left;
            node = this.#nodes.get(current);
        }
        return current;
    }

    /**
     * 在父节点中替换子节点引用
     *
     * @param parentId - 父节点 ID
     * @param oldChild - 旧子节点 ID
     * @param newChild - 新子节点 ID
     */
    #replaceInParent(parentId: number, oldChild: number, newChild: number): void {
        if (parentId === -1) {
            this.#root = newChild;
        } else {
            const parent = this.#nodes.get(parentId);
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
     * @param nodeId - 要删除的节点 ID
     */
    #deleteNode(nodeId: number): void {
        const node = this.#nodes.get(nodeId);
        if (node === undefined) return;

        const parentId: number = this.#findParent(nodeId);
        const leftChild: number = node.left;
        const rightChild: number = node.right;

        if (leftChild === -1 && rightChild === -1) {
            this.#replaceInParent(parentId, nodeId, -1);
            this.#nodes.delete(nodeId);
        } else if (leftChild === -1) {
            this.#replaceInParent(parentId, nodeId, rightChild);
            this.#nodes.delete(nodeId);
        } else if (rightChild === -1) {
            this.#replaceInParent(parentId, nodeId, leftChild);
            this.#nodes.delete(nodeId);
        } else {
            const successorId: number = this.#findMinNode(rightChild);
            const successor = this.#nodes.get(successorId);
            if (successor !== undefined) {
                const successorValue = successor.value;
                this.#deleteNode(successorId);
                const currentNode = this.#nodes.get(nodeId);
                if (currentNode !== undefined) {
                    currentNode.value = successorValue;
                }
            }
        }
    }

    /**
     * 插入节点，自动维护二叉搜索树性质
     *
     * @param value - 要插入的值
     * @returns 新插入节点的 id
     */
    public insert(value: number): number {
        const before: number = this.session.getLastStateId();
        const line: number = getCallerLine(2);
        const newId: number = this.#nextId;
        this.#nextId += 1;
        this.#nodes.set(newId, { value, left: -1, right: -1 });

        if (this.#root === -1) {
            this.#root = newId;
        } else {
            let current: number = this.#root;
            while (true) {
                const currentNode = this.#nodes.get(current);
                if (currentNode === undefined) break;
                const currentValue = currentNode.value as number;
                if (value < currentValue) {
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

        const after: number = this.session.addState(this.buildData());
        this.session.addStep('insert', before, after, { value }, newId, undefined, line);
        return newId;
    }

    /**
     * 删除节点，自动维护二叉搜索树性质
     *
     * @param value - 要删除的节点值
     * @throws {StructureError} 当节点不存在时抛出异常
     */
    public delete(value: number): void {
        const nodeId: number = this.#findNodeByValue(value);
        if (nodeId === -1) {
            this.raiseError(`Node with value ${value} not found`);
        }
        const before: number = this.session.getLastStateId();
        const line: number = getCallerLine(2);
        this.#deleteNode(nodeId);
        const after: number = this.session.addState(this.buildData());
        this.session.addStep('delete', before, after, { value }, undefined, undefined, line);
    }
}

/**
 * 堆选项接口
 *
 * @interface
 */
export interface HeapOptions extends StructureOptions {
    /** 堆类型 */
    heapType?: 'min' | 'max';
}

/**
 * 堆数据结构（使用完全二叉树表示）
 */
export class Heap extends ContextManager implements Disposable {
    readonly #heapType: 'min' | 'max';
    readonly #items: number[];

    /**
     * 构造函数
     *
     * @param options - 堆选项
     * @throws {Error} 当 heapType 不是 "min" 或 "max" 时抛出异常
     */
    constructor(options?: HeapOptions) {
        const heapType = options?.heapType ?? 'min';
        if (heapType !== 'min' && heapType !== 'max') {
            throw new Error(`heapType must be 'min' or 'max', got '${heapType}'`);
        }
        super('binary_tree', options?.label ?? 'heap', options?.output);
        this.#heapType = heapType;
        this.#items = [];
    }

    /**
     * 构建堆的状态数据（以完全二叉树形式表示）
     *
     * @returns 状态数据字典
     */
    protected buildData(): StateData {
        if (this.#items.length === 0) {
            return { root: -1, nodes: [] };
        }
        const nodesList: Array<{ id: number; value: number; left: number; right: number }> = [];
        for (let i = 0; i < this.#items.length; i++) {
            const left: number = 2 * i + 1 < this.#items.length ? 2 * i + 1 : -1;
            const right: number = 2 * i + 2 < this.#items.length ? 2 * i + 2 : -1;
            nodesList.push({ id: i, value: this.#items[i]!, left, right });
        }
        return { root: 0, nodes: nodesList };
    }

    /**
     * 初始化堆
     */
    protected initialize(): void {
        this.session.addState(this.buildData());
    }

    /**
     * 比较两个值，根据堆类型决定比较方式
     *
     * @param a - 第一个值
     * @param b - 第二个值
     * @returns 如果 a 应该在 b 之前（更接近堆顶）则返回 true
     */
    #compare(a: number, b: number): boolean {
        return this.#heapType === 'min' ? a < b : a > b;
    }

    /**
     * 向上调整堆
     *
     * @param index - 开始调整的索引
     */
    #siftUp(index: number): void {
        let current = index;
        while (current > 0) {
            const parent: number = Math.floor((current - 1) / 2);
            const currentVal = this.#items[current];
            const parentVal = this.#items[parent];
            if (currentVal !== undefined && parentVal !== undefined && this.#compare(currentVal, parentVal)) {
                this.#items[current] = parentVal;
                this.#items[parent] = currentVal;
                current = parent;
            } else {
                break;
            }
        }
    }

    /**
     * 向下调整堆
     *
     * @param index - 开始调整的索引
     */
    #siftDown(index: number): void {
        const size: number = this.#items.length;
        let current = index;
        while (true) {
            let target: number = current;
            const left: number = 2 * current + 1;
            const right: number = 2 * current + 2;
            const targetVal = this.#items[target];
            const leftVal = this.#items[left];
            const rightVal = this.#items[right];

            if (left < size && leftVal !== undefined && targetVal !== undefined && this.#compare(leftVal, targetVal)) {
                target = left;
            }
            const newTargetVal = this.#items[target];
            if (right < size && rightVal !== undefined && newTargetVal !== undefined && this.#compare(rightVal, newTargetVal)) {
                target = right;
            }
            if (target === current) {
                break;
            }
            const currentValSwap = this.#items[current];
            const targetValSwap = this.#items[target];
            if (currentValSwap !== undefined && targetValSwap !== undefined) {
                this.#items[current] = targetValSwap;
                this.#items[target] = currentValSwap;
            }
            current = target;
        }
    }

    /**
     * 插入元素，自动维护堆性质
     *
     * @param value - 要插入的值
     */
    public insert(value: number): void {
        const before: number = this.session.getLastStateId();
        const line: number = getCallerLine(2);
        this.#items.push(value);
        this.#siftUp(this.#items.length - 1);
        const after: number = this.session.addState(this.buildData());
        this.session.addStep('insert', before, after, { value }, undefined, undefined, line);
    }

    /**
     * 提取堆顶元素，自动维护堆性质
     *
     * @throws {StructureError} 当堆为空时抛出异常
     */
    public extract(): void {
        if (this.#items.length === 0) {
            this.raiseError('Cannot extract from empty heap');
        }
        const before: number = this.session.getLastStateId();
        const line: number = getCallerLine(2);
        const extracted: number = this.#items[0]!;
        if (this.#items.length === 1) {
            this.#items.pop();
        } else {
            this.#items[0] = this.#items.pop()!;
            this.#siftDown(0);
        }
        const after: number = this.session.addState(this.buildData());
        this.session.addStep('extract', before, after, {}, extracted, undefined, line);
    }

    /**
     * 清空堆
     */
    public clear(): void {
        const before: number = this.session.getLastStateId();
        const line: number = getCallerLine(2);
        this.#items.length = 0;
        const after: number = this.session.addState(this.buildData());
        this.session.addStep('clear', before, after, {}, undefined, undefined, line);
    }
}
