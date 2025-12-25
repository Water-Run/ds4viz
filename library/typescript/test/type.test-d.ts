/**
 * 类型测试 - 验证 TypeScript 类型定义的正确性
 *
 * @file test/type.test-d.ts
 * @author WaterRun
 * @date 2025-12-25
 */

import { expectType, expectError, expectAssignable, expectNotAssignable } from 'tsd';
import {
    config,
    withContext,
    type ConfigOptions,
    type StructureOptions,
    type Disposable,
    ErrorType,
    DS4VizError,
    StructureError,
    ValidationError,
    Stack,
    Queue,
    SingleLinkedList,
    DoubleLinkedList,
    BinaryTree,
    BinarySearchTree,
    Heap,
    type HeapOptions,
    GraphUndirected,
    GraphDirected,
    GraphWeighted,
    type GraphWeightedOptions,
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
    type Trace,
    type Meta,
    type Package,
    type Remarks,
    type TraceObject,
    type State,
    type StateData,
    type Step,
    type StepArgs,
    type CodeLocation,
    type Result,
    type Commit,
    type TraceError,
    type ValueType,
    VERSION,
    AUTHOR,
} from '../src/index.js';

// ============================================================================
// ConfigOptions 类型测试
// ============================================================================

// ConfigOptions 应接受有效选项
expectAssignable<ConfigOptions>({});
expectAssignable<ConfigOptions>({ outputPath: 'test.toml' });
expectAssignable<ConfigOptions>({ title: 'Test' });
expectAssignable<ConfigOptions>({ author: 'Author' });
expectAssignable<ConfigOptions>({ comment: 'Comment' });
expectAssignable<ConfigOptions>({
    outputPath: 'test.toml',
    title: 'Test',
    author: 'Author',
    comment: 'Comment',
});

// ConfigOptions 所有字段应为可选
const partialConfig: ConfigOptions = { title: 'Only Title' };
expectType<ConfigOptions>(partialConfig);

// ============================================================================
// StructureOptions 类型测试
// ============================================================================

expectAssignable<StructureOptions>({});
expectAssignable<StructureOptions>({ label: 'my_label' });
expectAssignable<StructureOptions>({ output: 'output.toml' });
expectAssignable<StructureOptions>({ label: 'my_label', output: 'output.toml' });

// ============================================================================
// HeapOptions 类型测试
// ============================================================================

expectAssignable<HeapOptions>({});
expectAssignable<HeapOptions>({ heapType: 'min' });
expectAssignable<HeapOptions>({ heapType: 'max' });
expectAssignable<HeapOptions>({ heapType: 'min', label: 'my_heap' });

// HeapOptions 应继承 StructureOptions
expectAssignable<HeapOptions>({ label: 'test', output: 'out.toml', heapType: 'min' });

// ============================================================================
// GraphWeightedOptions 类型测试
// ============================================================================

expectAssignable<GraphWeightedOptions>({});
expectAssignable<GraphWeightedOptions>({ directed: true });
expectAssignable<GraphWeightedOptions>({ directed: false });
expectAssignable<GraphWeightedOptions>({ directed: true, label: 'my_graph' });

// ============================================================================
// ValueType 类型测试
// ============================================================================

expectAssignable<ValueType>(42);
expectAssignable<ValueType>('string');
expectAssignable<ValueType>(true);
expectAssignable<ValueType>(false);
expectAssignable<ValueType>(3.14);
expectAssignable<ValueType>(-100);

// ============================================================================
// ErrorType 枚举测试
// ============================================================================

expectType<ErrorType>(ErrorType.Runtime);
expectType<ErrorType>(ErrorType.Timeout);
expectType<ErrorType>(ErrorType.Validation);
expectType<ErrorType>(ErrorType.Sandbox);
expectType<ErrorType>(ErrorType.Unknown);

// ============================================================================
// 错误类型测试
// ============================================================================

// DS4VizError 应有正确的属性类型
const ds4vizError = new DS4VizError('test', ErrorType.Runtime, 10);
expectType<string>(ds4vizError.message);
expectType<ErrorType>(ds4vizError.errorType);
expectType<number | undefined>(ds4vizError.line);

// StructureError 应继承 DS4VizError
const structError = new StructureError('test', 10);
expectAssignable<DS4VizError>(structError);
expectAssignable<Error>(structError);

// ValidationError 应继承 DS4VizError
const validError = new ValidationError('test', 10);
expectAssignable<DS4VizError>(validError);
expectAssignable<Error>(validError);

// ============================================================================
// Trace 相关类型测试
// ============================================================================

// Meta 类型
declare const meta: Meta;
expectType<string>(meta.createdAt);
expectType<string>(meta.lang);
expectType<string>(meta.langVersion);

// Package 类型
declare const pkg: Package;
expectType<string>(pkg.name);
expectType<string>(pkg.lang);
expectType<string>(pkg.version);

// Remarks 类型
declare const remarks: Remarks;
expectType<string>(remarks.title);
expectType<string>(remarks.author);
expectType<string>(remarks.comment);

// TraceObject 类型
declare const traceObject: TraceObject;
expectType<string>(traceObject.kind);
expectType<string>(traceObject.label);

// State 类型
declare const state: State;
expectType<number>(state.id);
expectType<StateData>(state.data);

// StateData 类型
declare const stateData: StateData;
expectAssignable<Record<string, unknown>>(stateData);

// Step 类型
declare const step: Step;
expectType<number>(step.id);
expectType<string>(step.op);
expectType<number>(step.before);
expectType<number | undefined>(step.after);
expectType<StepArgs>(step.args);
expectType<CodeLocation | undefined>(step.code);
expectType<unknown>(step.ret);
expectType<string | undefined>(step.note);

// CodeLocation 类型
declare const codeLoc: CodeLocation;
expectType<number>(codeLoc.line);
expectType<number | undefined>(codeLoc.col);

// Result 类型
declare const result: Result;
expectType<number>(result.finalState);
expectType<Commit | undefined>(result.commit);

// Commit 类型
declare const commit: Commit;
expectType<string>(commit.op);
expectType<number>(commit.line);

// TraceError 类型
declare const traceError: TraceError;
expectType<string>(traceError.type);
expectType<string>(traceError.message);
expectType<number | undefined>(traceError.line);
expectType<number | undefined>(traceError.step);
expectType<number | undefined>(traceError.lastState);

// Trace 类型
declare const trace: Trace;
expectType<Meta>(trace.meta);
expectType<Package>(trace.package);
expectType<Remarks>(trace.remarks);
expectType<TraceObject>(trace.object);
expectType<readonly State[]>(trace.states);
expectType<readonly Step[]>(trace.steps);
expectType<Result | undefined>(trace.result);
expectType<TraceError | undefined>(trace.error);

// ============================================================================
// Stack 类型测试
// ============================================================================

const stackInstance = new Stack();
expectType<Stack>(stackInstance);

// Stack 方法类型
expectType<void>(stackInstance.push(42));
expectType<void>(stackInstance.push('string'));
expectType<void>(stackInstance.push(true));
expectType<void>(stackInstance.pop());
expectType<void>(stackInstance.clear());
expectType<void>(stackInstance.enter());
expectType<void>(stackInstance.dispose());

// Stack 应实现 Disposable
expectAssignable<Disposable>(stackInstance);

// ============================================================================
// Queue 类型测试
// ============================================================================

const queueInstance = new Queue();
expectType<Queue>(queueInstance);

expectType<void>(queueInstance.enqueue(42));
expectType<void>(queueInstance.enqueue('string'));
expectType<void>(queueInstance.enqueue(true));
expectType<void>(queueInstance.dequeue());
expectType<void>(queueInstance.clear());

// ============================================================================
// SingleLinkedList 类型测试
// ============================================================================

const slistInstance = new SingleLinkedList();
expectType<SingleLinkedList>(slistInstance);

expectType<number>(slistInstance.insertHead(42));
expectType<number>(slistInstance.insertTail(42));
expectType<number>(slistInstance.insertAfter(0, 42));
expectType<void>(slistInstance.delete(0));
expectType<void>(slistInstance.deleteHead());
expectType<void>(slistInstance.reverse());

// ============================================================================
// DoubleLinkedList 类型测试
// ============================================================================

const dlistInstance = new DoubleLinkedList();
expectType<DoubleLinkedList>(dlistInstance);

expectType<number>(dlistInstance.insertHead(42));
expectType<number>(dlistInstance.insertTail(42));
expectType<number>(dlistInstance.insertBefore(0, 42));
expectType<number>(dlistInstance.insertAfter(0, 42));
expectType<void>(dlistInstance.delete(0));
expectType<void>(dlistInstance.deleteHead());
expectType<void>(dlistInstance.deleteTail());
expectType<void>(dlistInstance.reverse());

// ============================================================================
// BinaryTree 类型测试
// ============================================================================

const btreeInstance = new BinaryTree();
expectType<BinaryTree>(btreeInstance);

expectType<number>(btreeInstance.insertRoot(42));
expectType<number>(btreeInstance.insertLeft(0, 42));
expectType<number>(btreeInstance.insertRight(0, 42));
expectType<void>(btreeInstance.delete(0));
expectType<void>(btreeInstance.updateValue(0, 42));

// ============================================================================
// BinarySearchTree 类型测试
// ============================================================================

const bstInstance = new BinarySearchTree();
expectType<BinarySearchTree>(bstInstance);

expectType<number>(bstInstance.insert(42));
expectType<void>(bstInstance.delete(42));

// ============================================================================
// Heap 类型测试
// ============================================================================

const heapInstance = new Heap();
expectType<Heap>(heapInstance);

const minHeap = new Heap({ heapType: 'min' });
expectType<Heap>(minHeap);

const maxHeap = new Heap({ heapType: 'max' });
expectType<Heap>(maxHeap);

expectType<void>(heapInstance.insert(42));
expectType<void>(heapInstance.extract());
expectType<void>(heapInstance.clear());

// ============================================================================
// GraphUndirected 类型测试
// ============================================================================

const undirectedGraph = new GraphUndirected();
expectType<GraphUndirected>(undirectedGraph);

expectType<void>(undirectedGraph.addNode(0, 'A'));
expectType<void>(undirectedGraph.addEdge(0, 1));
expectType<void>(undirectedGraph.removeNode(0));
expectType<void>(undirectedGraph.removeEdge(0, 1));
expectType<void>(undirectedGraph.updateNodeLabel(0, 'B'));

// ============================================================================
// GraphDirected 类型测试
// ============================================================================

const directedGraph = new GraphDirected();
expectType<GraphDirected>(directedGraph);

expectType<void>(directedGraph.addNode(0, 'A'));
expectType<void>(directedGraph.addEdge(0, 1));
expectType<void>(directedGraph.removeNode(0));
expectType<void>(directedGraph.removeEdge(0, 1));
expectType<void>(directedGraph.updateNodeLabel(0, 'B'));

// ============================================================================
// GraphWeighted 类型测试
// ============================================================================

const weightedGraph = new GraphWeighted();
expectType<GraphWeighted>(weightedGraph);

const directedWeightedGraph = new GraphWeighted({ directed: true });
expectType<GraphWeighted>(directedWeightedGraph);

expectType<void>(weightedGraph.addNode(0, 'A'));
expectType<void>(weightedGraph.addEdge(0, 1, 5));
expectType<void>(weightedGraph.removeNode(0));
expectType<void>(weightedGraph.removeEdge(0, 1));
expectType<void>(weightedGraph.updateWeight(0, 1, 10));
expectType<void>(weightedGraph.updateNodeLabel(0, 'B'));

// ============================================================================
// 工厂函数类型测试
// ============================================================================

expectType<Stack>(stack());
expectType<Stack>(stack({ label: 'my_stack' }));
expectType<Stack>(stack({ output: 'out.toml' }));

expectType<Queue>(queue());
expectType<Queue>(queue({ label: 'my_queue' }));

expectType<SingleLinkedList>(singleLinkedList());
expectType<SingleLinkedList>(singleLinkedList({ label: 'my_list' }));

expectType<DoubleLinkedList>(doubleLinkedList());
expectType<DoubleLinkedList>(doubleLinkedList({ label: 'my_list' }));

expectType<BinaryTree>(binaryTree());
expectType<BinaryTree>(binaryTree({ label: 'my_tree' }));

expectType<BinarySearchTree>(binarySearchTree());
expectType<BinarySearchTree>(binarySearchTree({ label: 'my_bst' }));

expectType<Heap>(heap());
expectType<Heap>(heap({ heapType: 'min' }));
expectType<Heap>(heap({ heapType: 'max' }));
expectType<Heap>(heap({ heapType: 'min', label: 'my_heap' }));

expectType<GraphUndirected>(graphUndirected());
expectType<GraphUndirected>(graphUndirected({ label: 'my_graph' }));

expectType<GraphDirected>(graphDirected());
expectType<GraphDirected>(graphDirected({ label: 'my_graph' }));

expectType<GraphWeighted>(graphWeighted());
expectType<GraphWeighted>(graphWeighted({ directed: true }));
expectType<GraphWeighted>(graphWeighted({ directed: false, label: 'my_graph' }));

// ============================================================================
// config 函数类型测试
// ============================================================================

expectType<void>(config({}));
expectType<void>(config({ outputPath: 'test.toml' }));
expectType<void>(config({ title: 'Test', author: 'Author' }));

// ============================================================================
// withContext 函数类型测试
// ============================================================================

// withContext 应返回 Promise<void>
expectType<Promise<void>>(withContext(new Stack(), () => { }));
expectType<Promise<void>>(withContext(new Stack(), async () => { }));
expectType<Promise<void>>(
    withContext(new Stack(), (s) => {
        s.push(1);
    })
);

// ============================================================================
// 常量类型测试
// ============================================================================

expectType<string>(VERSION);
expectType<string>(AUTHOR);

// ============================================================================
// 只读属性测试
// ============================================================================

// Trace 相关类型的属性应为只读
declare const immutableMeta: Meta;
// @ts-expect-error - createdAt 应为只读
immutableMeta.createdAt = 'new';

declare const immutableState: State;
// @ts-expect-error - id 应为只读
immutableState.id = 1;

declare const immutableStep: Step;
// @ts-expect-error - op 应为只读
immutableStep.op = 'new';

declare const immutableTrace: Trace;
// @ts-expect-error - states 应为只读
immutableTrace.states = [];

// ============================================================================
// Disposable 接口测试
// ============================================================================

const disposableStack: Disposable = new Stack();
expectType<void>(disposableStack[Symbol.dispose]());

const disposableQueue: Disposable = new Queue();
expectType<void>(disposableQueue[Symbol.dispose]());

// ============================================================================
// 错误类型继承测试
// ============================================================================

const errorInstance: Error = new StructureError('test');
expectAssignable<Error>(errorInstance);

const ds4vizInstance: DS4VizError = new ValidationError('test');
expectAssignable<DS4VizError>(ds4vizInstance);