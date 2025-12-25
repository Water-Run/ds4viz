/**
 * 图数据结构实现：无向图、有向图、带权图
 *
 * @file src/structures/graph.ts
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
 * 无向图数据结构
 */
export class GraphUndirected extends ContextManager implements Disposable {
    readonly #nodes: Map<number, string>;
    readonly #edges: Set<string>;

    /**
     * 构造函数
     *
     * @param options - 结构选项
     */
    constructor(options?: StructureOptions) {
        super('graph_undirected', options?.label ?? 'graph', options?.output);
        this.#nodes = new Map();
        this.#edges = new Set();
    }

    /**
     * 构建无向图的状态数据
     *
     * @returns 状态数据字典
     */
    protected buildData(): StateData {
        const nodesList: Array<{ id: number; label: string }> = [];
        const sortedIds = [...this.#nodes.keys()].sort((a, b) => a - b);
        for (const nid of sortedIds) {
            nodesList.push({ id: nid, label: this.#nodes.get(nid)! });
        }
        const edgesList: Array<{ from: number; to: number }> = [];
        const sortedEdges = [...this.#edges].sort();
        for (const edgeKey of sortedEdges) {
            const [from, to] = edgeKey.split(',').map(Number) as [number, number];
            edgesList.push({ from, to });
        }
        return { nodes: nodesList, edges: edgesList };
    }

    /**
     * 初始化无向图
     */
    protected initialize(): void {
        this.session.addState(this.buildData());
    }

    /**
     * 规范化无向边，确保 from < to
     *
     * @param fromId - 起始节点 id
     * @param toId - 终止节点 id
     * @returns 规范化后的边键
     */
    static #normalizeEdge(fromId: number, toId: number): string {
        return fromId < toId ? `${fromId},${toId}` : `${toId},${fromId}`;
    }

    /**
     * 添加节点
     *
     * @param nodeId - 节点 id
     * @param label - 节点标签，长度限制为 1-32 字符
     * @throws {StructureError} 当节点已存在时抛出异常
     */
    public addNode(nodeId: number, label: string): void {
        if (this.#nodes.has(nodeId)) {
            this.raiseError(`Node already exists: ${nodeId}`);
        }
        if (label.length < 1 || label.length > 32) {
            this.raiseError(`Label length must be 1-32, got ${label.length}`);
        }
        const before: number = this.session.getLastStateId();
        const line: number = getCallerLine(2);
        this.#nodes.set(nodeId, label);
        const after: number = this.session.addState(this.buildData());
        this.session.addStep('add_node', before, after, { id: nodeId, label }, undefined, undefined, line);
    }

    /**
     * 添加无向边，内部自动规范化为 fromId < toId
     *
     * @param fromId - 起始节点 id
     * @param toId - 终止节点 id
     * @throws {StructureError} 当节点不存在、边已存在或形成自环时抛出异常
     */
    public addEdge(fromId: number, toId: number): void {
        if (!this.#nodes.has(fromId)) {
            this.raiseError(`Node not found: ${fromId}`);
        }
        if (!this.#nodes.has(toId)) {
            this.raiseError(`Node not found: ${toId}`);
        }
        if (fromId === toId) {
            this.raiseError(`Self-loop not allowed: ${fromId}`);
        }
        const edgeKey: string = GraphUndirected.#normalizeEdge(fromId, toId);
        if (this.#edges.has(edgeKey)) {
            this.raiseError(`Edge already exists: (${fromId}, ${toId})`);
        }
        const before: number = this.session.getLastStateId();
        const line: number = getCallerLine(2);
        this.#edges.add(edgeKey);
        const after: number = this.session.addState(this.buildData());
        this.session.addStep('add_edge', before, after, { from: fromId, to: toId }, undefined, undefined, line);
    }

    /**
     * 删除节点及其相关的所有边
     *
     * @param nodeId - 要删除的节点 id
     * @throws {StructureError} 当节点不存在时抛出异常
     */
    public removeNode(nodeId: number): void {
        if (!this.#nodes.has(nodeId)) {
            this.raiseError(`Node not found: ${nodeId}`);
        }
        const before: number = this.session.getLastStateId();
        const line: number = getCallerLine(2);
        const edgesToRemove: string[] = [];
        for (const edgeKey of this.#edges) {
            const [from, to] = edgeKey.split(',').map(Number) as [number, number];
            if (from === nodeId || to === nodeId) {
                edgesToRemove.push(edgeKey);
            }
        }
        for (const edgeKey of edgesToRemove) {
            this.#edges.delete(edgeKey);
        }
        this.#nodes.delete(nodeId);
        const after: number = this.session.addState(this.buildData());
        this.session.addStep('remove_node', before, after, { node_id: nodeId }, undefined, undefined, line);
    }

    /**
     * 删除边
     *
     * @param fromId - 起始节点 id
     * @param toId - 终止节点 id
     * @throws {StructureError} 当边不存在时抛出异常
     */
    public removeEdge(fromId: number, toId: number): void {
        const edgeKey: string = GraphUndirected.#normalizeEdge(fromId, toId);
        if (!this.#edges.has(edgeKey)) {
            this.raiseError(`Edge not found: (${fromId}, ${toId})`);
        }
        const before: number = this.session.getLastStateId();
        const line: number = getCallerLine(2);
        this.#edges.delete(edgeKey);
        const after: number = this.session.addState(this.buildData());
        this.session.addStep('remove_edge', before, after, { from: fromId, to: toId }, undefined, undefined, line);
    }

    /**
     * 更新节点标签
     *
     * @param nodeId - 节点 id
     * @param label - 新的节点标签，长度限制为 1-32 字符
     * @throws {StructureError} 当节点不存在时抛出异常
     */
    public updateNodeLabel(nodeId: number, label: string): void {
        if (!this.#nodes.has(nodeId)) {
            this.raiseError(`Node not found: ${nodeId}`);
        }
        if (label.length < 1 || label.length > 32) {
            this.raiseError(`Label length must be 1-32, got ${label.length}`);
        }
        const before: number = this.session.getLastStateId();
        const line: number = getCallerLine(2);
        const oldLabel: string = this.#nodes.get(nodeId)!;
        this.#nodes.set(nodeId, label);
        const after: number = this.session.addState(this.buildData());
        this.session.addStep('update_node_label', before, after, { node_id: nodeId, label }, oldLabel, undefined, line);
    }
}

/**
 * 有向图数据结构
 */
export class GraphDirected extends ContextManager implements Disposable {
    readonly #nodes: Map<number, string>;
    readonly #edges: Set<string>;

    /**
     * 构造函数
     *
     * @param options - 结构选项
     */
    constructor(options?: StructureOptions) {
        super('graph_directed', options?.label ?? 'graph', options?.output);
        this.#nodes = new Map();
        this.#edges = new Set();
    }

    /**
     * 构建有向图的状态数据
     *
     * @returns 状态数据字典
     */
    protected buildData(): StateData {
        const nodesList: Array<{ id: number; label: string }> = [];
        const sortedIds = [...this.#nodes.keys()].sort((a, b) => a - b);
        for (const nid of sortedIds) {
            nodesList.push({ id: nid, label: this.#nodes.get(nid)! });
        }
        const edgesList: Array<{ from: number; to: number }> = [];
        const sortedEdges = [...this.#edges].sort();
        for (const edgeKey of sortedEdges) {
            const [from, to] = edgeKey.split(',').map(Number) as [number, number];
            edgesList.push({ from, to });
        }
        return { nodes: nodesList, edges: edgesList };
    }

    /**
     * 初始化有向图
     */
    protected initialize(): void {
        this.session.addState(this.buildData());
    }

    /**
     * 添加节点
     *
     * @param nodeId - 节点 id
     * @param label - 节点标签，长度限制为 1-32 字符
     * @throws {StructureError} 当节点已存在时抛出异常
     */
    public addNode(nodeId: number, label: string): void {
        if (this.#nodes.has(nodeId)) {
            this.raiseError(`Node already exists: ${nodeId}`);
        }
        if (label.length < 1 || label.length > 32) {
            this.raiseError(`Label length must be 1-32, got ${label.length}`);
        }
        const before: number = this.session.getLastStateId();
        const line: number = getCallerLine(2);
        this.#nodes.set(nodeId, label);
        const after: number = this.session.addState(this.buildData());
        this.session.addStep('add_node', before, after, { id: nodeId, label }, undefined, undefined, line);
    }

    /**
     * 添加有向边
     *
     * @param fromId - 起始节点 id
     * @param toId - 终止节点 id
     * @throws {StructureError} 当节点不存在、边已存在或形成自环时抛出异常
     */
    public addEdge(fromId: number, toId: number): void {
        if (!this.#nodes.has(fromId)) {
            this.raiseError(`Node not found: ${fromId}`);
        }
        if (!this.#nodes.has(toId)) {
            this.raiseError(`Node not found: ${toId}`);
        }
        if (fromId === toId) {
            this.raiseError(`Self-loop not allowed: ${fromId}`);
        }
        const edgeKey: string = `${fromId},${toId}`;
        if (this.#edges.has(edgeKey)) {
            this.raiseError(`Edge already exists: (${fromId}, ${toId})`);
        }
        const before: number = this.session.getLastStateId();
        const line: number = getCallerLine(2);
        this.#edges.add(edgeKey);
        const after: number = this.session.addState(this.buildData());
        this.session.addStep('add_edge', before, after, { from: fromId, to: toId }, undefined, undefined, line);
    }

    /**
     * 删除节点及其相关的所有边
     *
     * @param nodeId - 要删除的节点 id
     * @throws {StructureError} 当节点不存在时抛出异常
     */
    public removeNode(nodeId: number): void {
        if (!this.#nodes.has(nodeId)) {
            this.raiseError(`Node not found: ${nodeId}`);
        }
        const before: number = this.session.getLastStateId();
        const line: number = getCallerLine(2);
        const edgesToRemove: string[] = [];
        for (const edgeKey of this.#edges) {
            const [from, to] = edgeKey.split(',').map(Number) as [number, number];
            if (from === nodeId || to === nodeId) {
                edgesToRemove.push(edgeKey);
            }
        }
        for (const edgeKey of edgesToRemove) {
            this.#edges.delete(edgeKey);
        }
        this.#nodes.delete(nodeId);
        const after: number = this.session.addState(this.buildData());
        this.session.addStep('remove_node', before, after, { node_id: nodeId }, undefined, undefined, line);
    }

    /**
     * 删除边
     *
     * @param fromId - 起始节点 id
     * @param toId - 终止节点 id
     * @throws {StructureError} 当边不存在时抛出异常
     */
    public removeEdge(fromId: number, toId: number): void {
        const edgeKey: string = `${fromId},${toId}`;
        if (!this.#edges.has(edgeKey)) {
            this.raiseError(`Edge not found: (${fromId}, ${toId})`);
        }
        const before: number = this.session.getLastStateId();
        const line: number = getCallerLine(2);
        this.#edges.delete(edgeKey);
        const after: number = this.session.addState(this.buildData());
        this.session.addStep('remove_edge', before, after, { from: fromId, to: toId }, undefined, undefined, line);
    }

    /**
     * 更新节点标签
     *
     * @param nodeId - 节点 id
     * @param label - 新的节点标签，长度限制为 1-32 字符
     * @throws {StructureError} 当节点不存在时抛出异常
     */
    public updateNodeLabel(nodeId: number, label: string): void {
        if (!this.#nodes.has(nodeId)) {
            this.raiseError(`Node not found: ${nodeId}`);
        }
        if (label.length < 1 || label.length > 32) {
            this.raiseError(`Label length must be 1-32, got ${label.length}`);
        }
        const before: number = this.session.getLastStateId();
        const line: number = getCallerLine(2);
        const oldLabel: string = this.#nodes.get(nodeId)!;
        this.#nodes.set(nodeId, label);
        const after: number = this.session.addState(this.buildData());
        this.session.addStep('update_node_label', before, after, { node_id: nodeId, label }, oldLabel, undefined, line);
    }
}

/**
 * 带权图选项接口
 *
 * @interface
 */
export interface GraphWeightedOptions extends StructureOptions {
    /** 是否为有向图 */
    directed?: boolean;
}

/**
 * 带权图数据结构
 */
export class GraphWeighted extends ContextManager implements Disposable {
    readonly #directed: boolean;
    readonly #nodes: Map<number, string>;
    readonly #edges: Map<string, number>;

    /**
     * 构造函数
     *
     * @param options - 带权图选项
     */
    constructor(options?: GraphWeightedOptions) {
        super('graph_weighted', options?.label ?? 'graph', options?.output);
        this.#directed = options?.directed ?? false;
        this.#nodes = new Map();
        this.#edges = new Map();
    }

    /**
     * 构建带权图的状态数据
     *
     * @returns 状态数据字典
     */
    protected buildData(): StateData {
        const nodesList: Array<{ id: number; label: string }> = [];
        const sortedIds = [...this.#nodes.keys()].sort((a, b) => a - b);
        for (const nid of sortedIds) {
            nodesList.push({ id: nid, label: this.#nodes.get(nid)! });
        }
        const edgesList: Array<{ from: number; to: number; weight: number }> = [];
        const sortedEdges = [...this.#edges.keys()].sort();
        for (const edgeKey of sortedEdges) {
            const [from, to] = edgeKey.split(',').map(Number) as [number, number];
            edgesList.push({ from, to, weight: this.#edges.get(edgeKey)! });
        }
        return { nodes: nodesList, edges: edgesList };
    }

    /**
     * 初始化带权图
     */
    protected initialize(): void {
        this.session.addState(this.buildData());
    }

    /**
     * 规范化边（无向图时确保 from < to）
     *
     * @param fromId - 起始节点 id
     * @param toId - 终止节点 id
     * @returns 规范化后的边键
     */
    #normalizeEdge(fromId: number, toId: number): string {
        if (this.#directed) {
            return `${fromId},${toId}`;
        }
        return fromId < toId ? `${fromId},${toId}` : `${toId},${fromId}`;
    }

    /**
     * 添加节点
     *
     * @param nodeId - 节点 id
     * @param label - 节点标签，长度限制为 1-32 字符
     * @throws {StructureError} 当节点已存在时抛出异常
     */
    public addNode(nodeId: number, label: string): void {
        if (this.#nodes.has(nodeId)) {
            this.raiseError(`Node already exists: ${nodeId}`);
        }
        if (label.length < 1 || label.length > 32) {
            this.raiseError(`Label length must be 1-32, got ${label.length}`);
        }
        const before: number = this.session.getLastStateId();
        const line: number = getCallerLine(2);
        this.#nodes.set(nodeId, label);
        const after: number = this.session.addState(this.buildData());
        this.session.addStep('add_node', before, after, { id: nodeId, label }, undefined, undefined, line);
    }

    /**
     * 添加带权边
     *
     * @param fromId - 起始节点 id
     * @param toId - 终止节点 id
     * @param weight - 边的权重
     * @throws {StructureError} 当节点不存在、边已存在或形成自环时抛出异常
     */
    public addEdge(fromId: number, toId: number, weight: number): void {
        if (!this.#nodes.has(fromId)) {
            this.raiseError(`Node not found: ${fromId}`);
        }
        if (!this.#nodes.has(toId)) {
            this.raiseError(`Node not found: ${toId}`);
        }
        if (fromId === toId) {
            this.raiseError(`Self-loop not allowed: ${fromId}`);
        }
        const edgeKey: string = this.#normalizeEdge(fromId, toId);
        if (this.#edges.has(edgeKey)) {
            this.raiseError(`Edge already exists: (${fromId}, ${toId})`);
        }
        const before: number = this.session.getLastStateId();
        const line: number = getCallerLine(2);
        this.#edges.set(edgeKey, weight);
        const after: number = this.session.addState(this.buildData());
        this.session.addStep('add_edge', before, after, { from: fromId, to: toId, weight }, undefined, undefined, line);
    }

    /**
     * 删除节点及其相关的所有边
     *
     * @param nodeId - 要删除的节点 id
     * @throws {StructureError} 当节点不存在时抛出异常
     */
    public removeNode(nodeId: number): void {
        if (!this.#nodes.has(nodeId)) {
            this.raiseError(`Node not found: ${nodeId}`);
        }
        const before: number = this.session.getLastStateId();
        const line: number = getCallerLine(2);
        const edgesToRemove: string[] = [];
        for (const edgeKey of this.#edges.keys()) {
            const [from, to] = edgeKey.split(',').map(Number) as [number, number];
            if (from === nodeId || to === nodeId) {
                edgesToRemove.push(edgeKey);
            }
        }
        for (const edgeKey of edgesToRemove) {
            this.#edges.delete(edgeKey);
        }
        this.#nodes.delete(nodeId);
        const after: number = this.session.addState(this.buildData());
        this.session.addStep('remove_node', before, after, { node_id: nodeId }, undefined, undefined, line);
    }

    /**
     * 删除边
     *
     * @param fromId - 起始节点 id
     * @param toId - 终止节点 id
     * @throws {StructureError} 当边不存在时抛出异常
     */
    public removeEdge(fromId: number, toId: number): void {
        const edgeKey: string = this.#normalizeEdge(fromId, toId);
        if (!this.#edges.has(edgeKey)) {
            this.raiseError(`Edge not found: (${fromId}, ${toId})`);
        }
        const before: number = this.session.getLastStateId();
        const line: number = getCallerLine(2);
        this.#edges.delete(edgeKey);
        const after: number = this.session.addState(this.buildData());
        this.session.addStep('remove_edge', before, after, { from: fromId, to: toId }, undefined, undefined, line);
    }

    /**
     * 更新边的权重
     *
     * @param fromId - 起始节点 id
     * @param toId - 终止节点 id
     * @param weight - 新的权重值
     * @throws {StructureError} 当边不存在时抛出异常
     */
    public updateWeight(fromId: number, toId: number, weight: number): void {
        const edgeKey: string = this.#normalizeEdge(fromId, toId);
        if (!this.#edges.has(edgeKey)) {
            this.raiseError(`Edge not found: (${fromId}, ${toId})`);
        }
        const before: number = this.session.getLastStateId();
        const line: number = getCallerLine(2);
        const oldWeight: number = this.#edges.get(edgeKey)!;
        this.#edges.set(edgeKey, weight);
        const after: number = this.session.addState(this.buildData());
        this.session.addStep('update_weight', before, after, { from: fromId, to: toId, weight }, oldWeight, undefined, line);
    }

    /**
     * 更新节点标签
     *
     * @param nodeId - 节点 id
     * @param label - 新的节点标签，长度限制为 1-32 字符
     * @throws {StructureError} 当节点不存在时抛出异常
     */
    public updateNodeLabel(nodeId: number, label: string): void {
        if (!this.#nodes.has(nodeId)) {
            this.raiseError(`Node not found: ${nodeId}`);
        }
        if (label.length < 1 || label.length > 32) {
            this.raiseError(`Label length must be 1-32, got ${label.length}`);
        }
        const before: number = this.session.getLastStateId();
        const line: number = getCallerLine(2);
        const oldLabel: string = this.#nodes.get(nodeId)!;
        this.#nodes.set(nodeId, label);
        const after: number = this.session.addState(this.buildData());
        this.session.addStep('update_node_label', before, after, { node_id: nodeId, label }, oldLabel, undefined, line);
    }
}
