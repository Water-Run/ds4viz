/**
 * ds4viz - 可扩展的数据结构可视化教学平台 TypeScript 库
 *
 * 提供数据结构的可视化追踪功能，生成标准化的 TOML 中间表示文件。
 *
 * @file src/index.ts
 * @author WaterRun
 * @date 2025-12-25
 */

// 导出会话管理相关
export {
    config,
    withContext,
    type ConfigOptions,
    type StructureOptions,
    type Disposable,
    ErrorType,
    DS4VizError,
    StructureError,
    ValidationError,
} from './session.js';

// 导出线性结构
export {
    Stack,
    Queue,
    SingleLinkedList,
    DoubleLinkedList,
    type ValueType,
} from './structures/linear.js';

// 导出树结构
export {
    BinaryTree,
    BinarySearchTree,
    Heap,
    type HeapOptions,
} from './structures/tree.js';

// 导出图结构
export {
    GraphUndirected,
    GraphDirected,
    GraphWeighted,
    type GraphWeightedOptions,
} from './structures/graph.js';

// 导出 Trace 相关类型
export type {
    Trace,
    Meta,
    Package,
    Remarks,
    TraceObject,
    State,
    StateData,
    Step,
    StepArgs,
    CodeLocation,
    Result,
    Commit,
    TraceError,
} from './trace.js';

// 工厂函数
import { Stack, Queue, SingleLinkedList, DoubleLinkedList } from './structures/linear.js';
import { BinaryTree, BinarySearchTree, Heap, type HeapOptions } from './structures/tree.js';
import { GraphUndirected, GraphDirected, GraphWeighted, type GraphWeightedOptions } from './structures/graph.js';
import type { StructureOptions } from './session.js';

/**
 * 创建栈实例
 *
 * @param options - 结构选项
 * @returns 栈实例
 */
export function stack(options?: StructureOptions): Stack {
    const s = new Stack(options);
    s.enter();
    return s;
}

/**
 * 创建队列实例
 *
 * @param options - 结构选项
 * @returns 队列实例
 */
export function queue(options?: StructureOptions): Queue {
    const q = new Queue(options);
    q.enter();
    return q;
}

/**
 * 创建单链表实例
 *
 * @param options - 结构选项
 * @returns 单链表实例
 */
export function singleLinkedList(options?: StructureOptions): SingleLinkedList {
    const list = new SingleLinkedList(options);
    list.enter();
    return list;
}

/**
 * 创建双向链表实例
 *
 * @param options - 结构选项
 * @returns 双向链表实例
 */
export function doubleLinkedList(options?: StructureOptions): DoubleLinkedList {
    const list = new DoubleLinkedList(options);
    list.enter();
    return list;
}

/**
 * 创建二叉树实例
 *
 * @param options - 结构选项
 * @returns 二叉树实例
 */
export function binaryTree(options?: StructureOptions): BinaryTree {
    const tree = new BinaryTree(options);
    tree.enter();
    return tree;
}

/**
 * 创建二叉搜索树实例
 *
 * @param options - 结构选项
 * @returns 二叉搜索树实例
 */
export function binarySearchTree(options?: StructureOptions): BinarySearchTree {
    const tree = new BinarySearchTree(options);
    tree.enter();
    return tree;
}

/**
 * 创建堆实例
 *
 * @param options - 堆选项
 * @returns 堆实例
 */
export function heap(options?: HeapOptions): Heap {
    const h = new Heap(options);
    h.enter();
    return h;
}

/**
 * 创建无向图实例
 *
 * @param options - 结构选项
 * @returns 无向图实例
 */
export function graphUndirected(options?: StructureOptions): GraphUndirected {
    const graph = new GraphUndirected(options);
    graph.enter();
    return graph;
}

/**
 * 创建有向图实例
 *
 * @param options - 结构选项
 * @returns 有向图实例
 */
export function graphDirected(options?: StructureOptions): GraphDirected {
    const graph = new GraphDirected(options);
    graph.enter();
    return graph;
}

/**
 * 创建带权图实例
 *
 * @param options - 带权图选项
 * @returns 带权图实例
 */
export function graphWeighted(options?: GraphWeightedOptions): GraphWeighted {
    const graph = new GraphWeighted(options);
    graph.enter();
    return graph;
}

/** 库版本 */
export const VERSION = '1.0.0';

/** 库作者 */
export const AUTHOR = 'WaterRun <linzhuangrun49@gmail.com>';
