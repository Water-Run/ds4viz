# `ds4viz`

`ds4viz` 是一个 TypeScript 库，为 TypeScript/JavaScript 环境提供 `ds4viz` 数据结构可视化支持。

`ds4viz` 是一个可扩展的数据结构可视化教学平台。参见：[GitHub](https://github.com/Water-Run/ds4viz)

## 安装

`ds4viz` 发布在 [npm](https://www.npmjs.com/package/ds4viz) 上：

```bash
npm install ds4viz
```

或使用其他包管理器：

```bash
# pnpm
pnpm add ds4viz

# yarn
yarn add ds4viz

# bun
bun add ds4viz
```

## 测试

在目录下运行：

```bash
npm test
```

## 快速上手

以下脚本提供了一个快速使用示例，演示如何使用 `ds4viz` 生成 `trace.toml`，并在发生异常时仍然产出包含 `[error]` 的 `.toml` IR 文件：

```typescript
import { graphUndirected, config } from 'ds4viz';

async function main(): Promise<void> {
    // 可选：配置全局参数
    config({
        outputPath: 'trace.toml',
        title: '无向图演示',
        author: 'WaterRun',
        comment: 'TypeScript 无向图示例'
    });

    // 使用 using 语法（需要 TypeScript 5.2+ 和支持 Symbol.dispose 的运行时）
    // 或使用 try-finally 模式确保资源释放
    await using graph = graphUndirected();
    
    // 添加节点
    graph.addNode(0, 'A');
    graph.addNode(1, 'B');
    graph.addNode(2, 'C');

    // 添加无向边（内部会规范化为 from < to）
    graph.addEdge(0, 1);
    graph.addEdge(1, 2);

    // 也可以在这里继续调用更多 API，每次操作都会记录到 trace 中

    console.log('已生成 trace.toml');
}

main().catch(console.error);
```

### 不使用 `using` 语法的替代方案

如果你的运行时不支持 `using` 语法，可以使用 `withContext` 辅助函数：

```typescript
import { graphUndirected, withContext } from 'ds4viz';

async function main(): Promise<void> {
    await withContext(graphUndirected(), async (graph) => {
        graph.addNode(0, 'A');
        graph.addNode(1, 'B');
        graph.addEdge(0, 1);
    });

    console.log('已生成 trace.toml');
}

main().catch(console.error);
```

## API 参考

### 全局配置

#### `config`

```typescript
function config(options: ConfigOptions): void;

interface ConfigOptions {
    /** 输出文件路径，默认为 "trace.toml" */
    outputPath?: string;
    /** 可视化标题 */
    title?: string;
    /** 作者信息 */
    author?: string;
    /** 注释说明 */
    comment?: string;
}
```

**示例：**

```typescript
import { config } from 'ds4viz';

config({
    outputPath: 'my_trace.toml',
    title: '栈操作演示',
    author: 'WaterRun',
    comment: '演示基本的栈操作'
});
```

---

### 线性结构

#### `stack`

```typescript
function stack(options?: StructureOptions): Stack;

interface StructureOptions {
    /** 数据结构的标签 */
    label?: string;
    /** 输出文件路径，若为 undefined 则使用全局配置 */
    output?: string;
}
```

**Stack 类方法：**

```typescript
interface Stack extends Disposable {
    /**
     * 压栈操作
     * @param value - 要压入的值
     */
    push(value: number | string | boolean): void;

    /**
     * 弹栈操作
     * @throws {StructureError} 当栈为空时抛出异常
     */
    pop(): void;

    /**
     * 清空栈
     */
    clear(): void;
}
```

**示例：**

```typescript
import { stack, withContext } from 'ds4viz';

await withContext(stack({ label: 'demo_stack' }), async (s) => {
    s.push(10);
    s.push(20);
    s.push(30);
    
    s.pop();
    s.pop();
});
```

#### `queue`

```typescript
function queue(options?: StructureOptions): Queue;
```

**Queue 类方法：**

```typescript
interface Queue extends Disposable {
    /**
     * 入队操作
     * @param value - 要入队的值
     */
    enqueue(value: number | string | boolean): void;

    /**
     * 出队操作
     * @throws {StructureError} 当队列为空时抛出异常
     */
    dequeue(): void;

    /**
     * 清空队列
     */
    clear(): void;
}
```

**示例：**

```typescript
import { queue, withContext } from 'ds4viz';

await withContext(queue({ label: 'demo_queue' }), async (q) => {
    q.enqueue(10);
    q.enqueue(20);
    q.enqueue(30);
    
    q.dequeue();
    q.dequeue();
});
```

#### `singleLinkedList`

```typescript
function singleLinkedList(options?: StructureOptions): SingleLinkedList;
```

**SingleLinkedList 类方法：**

```typescript
interface SingleLinkedList extends Disposable {
    /**
     * 在链表头部插入节点
     * @param value - 要插入的值
     * @returns 新插入节点的 id
     */
    insertHead(value: number | string | boolean): number;

    /**
     * 在链表尾部插入节点
     * @param value - 要插入的值
     * @returns 新插入节点的 id
     */
    insertTail(value: number | string | boolean): number;

    /**
     * 在指定节点后插入新节点
     * @param nodeId - 目标节点的 id
     * @param value - 要插入的值
     * @returns 新插入节点的 id
     * @throws {StructureError} 当指定节点不存在时抛出异常
     */
    insertAfter(nodeId: number, value: number | string | boolean): number;

    /**
     * 删除指定节点
     * @param nodeId - 要删除的节点 id
     * @throws {StructureError} 当指定节点不存在时抛出异常
     */
    delete(nodeId: number): void;

    /**
     * 删除头节点
     * @throws {StructureError} 当链表为空时抛出异常
     */
    deleteHead(): void;

    /**
     * 反转链表
     */
    reverse(): void;
}
```

**示例：**

```typescript
import { singleLinkedList, withContext } from 'ds4viz';

await withContext(singleLinkedList({ label: 'demo_slist' }), async (slist) => {
    const nodeA = slist.insertHead(10);
    slist.insertTail(20);
    const nodeC = slist.insertTail(30);
    
    slist.insertAfter(nodeA, 15);
    
    slist.delete(nodeC);
    slist.reverse();
});
```

#### `doubleLinkedList`

```typescript
function doubleLinkedList(options?: StructureOptions): DoubleLinkedList;
```

**DoubleLinkedList 类方法：**

```typescript
interface DoubleLinkedList extends Disposable {
    /**
     * 在链表头部插入节点
     * @param value - 要插入的值
     * @returns 新插入节点的 id
     */
    insertHead(value: number | string | boolean): number;

    /**
     * 在链表尾部插入节点
     * @param value - 要插入的值
     * @returns 新插入节点的 id
     */
    insertTail(value: number | string | boolean): number;

    /**
     * 在指定节点前插入新节点
     * @param nodeId - 目标节点的 id
     * @param value - 要插入的值
     * @returns 新插入节点的 id
     * @throws {StructureError} 当指定节点不存在时抛出异常
     */
    insertBefore(nodeId: number, value: number | string | boolean): number;

    /**
     * 在指定节点后插入新节点
     * @param nodeId - 目标节点的 id
     * @param value - 要插入的值
     * @returns 新插入节点的 id
     * @throws {StructureError} 当指定节点不存在时抛出异常
     */
    insertAfter(nodeId: number, value: number | string | boolean): number;

    /**
     * 删除指定节点
     * @param nodeId - 要删除的节点 id
     * @throws {StructureError} 当指定节点不存在时抛出异常
     */
    delete(nodeId: number): void;

    /**
     * 删除头节点
     * @throws {StructureError} 当链表为空时抛出异常
     */
    deleteHead(): void;

    /**
     * 删除尾节点
     * @throws {StructureError} 当链表为空时抛出异常
     */
    deleteTail(): void;

    /**
     * 反转链表
     */
    reverse(): void;
}
```

**示例：**

```typescript
import { doubleLinkedList, withContext } from 'ds4viz';

await withContext(doubleLinkedList({ label: 'demo_dlist' }), async (dlist) => {
    const nodeA = dlist.insertHead(10);
    dlist.insertTail(30);
    
    dlist.insertAfter(nodeA, 20);
    
    dlist.deleteTail();
    dlist.reverse();
});
```

---

### 树结构

#### `binaryTree`

```typescript
function binaryTree(options?: StructureOptions): BinaryTree;
```

**BinaryTree 类方法：**

```typescript
interface BinaryTree extends Disposable {
    /**
     * 插入根节点
     * @param value - 根节点的值
     * @returns 根节点的 id
     * @throws {StructureError} 当根节点已存在时抛出异常
     */
    insertRoot(value: number | string | boolean): number;

    /**
     * 在指定父节点的左侧插入子节点
     * @param parentId - 父节点的 id
     * @param value - 要插入的子节点值
     * @returns 新插入节点的 id
     * @throws {StructureError} 当父节点不存在或左子节点已存在时抛出异常
     */
    insertLeft(parentId: number, value: number | string | boolean): number;

    /**
     * 在指定父节点的右侧插入子节点
     * @param parentId - 父节点的 id
     * @param value - 要插入的子节点值
     * @returns 新插入节点的 id
     * @throws {StructureError} 当父节点不存在或右子节点已存在时抛出异常
     */
    insertRight(parentId: number, value: number | string | boolean): number;

    /**
     * 删除指定节点及其所有子树
     * @param nodeId - 要删除的节点 id
     * @throws {StructureError} 当节点不存在时抛出异常
     */
    delete(nodeId: number): void;

    /**
     * 更新节点的值
     * @param nodeId - 节点 id
     * @param value - 新的值
     * @throws {StructureError} 当节点不存在时抛出异常
     */
    updateValue(nodeId: number, value: number | string | boolean): void;
}
```

**示例：**

```typescript
import { binaryTree, withContext } from 'ds4viz';

await withContext(binaryTree({ label: 'demo_tree' }), async (tree) => {
    const root = tree.insertRoot(10);
    const left = tree.insertLeft(root, 5);
    const right = tree.insertRight(root, 15);
    
    tree.insertLeft(left, 3);
    tree.insertRight(left, 7);
    
    tree.updateValue(right, 20);
    tree.delete(left);
});
```

#### `binarySearchTree`

```typescript
function binarySearchTree(options?: StructureOptions): BinarySearchTree;
```

**BinarySearchTree 类方法：**

```typescript
interface BinarySearchTree extends Disposable {
    /**
     * 插入节点，自动维护二叉搜索树性质
     * @param value - 要插入的值
     * @returns 新插入节点的 id
     */
    insert(value: number): number;

    /**
     * 删除节点，自动维护二叉搜索树性质
     * @param value - 要删除的节点值
     * @throws {StructureError} 当节点不存在时抛出异常
     */
    delete(value: number): void;
}
```

**示例：**

```typescript
import { binarySearchTree, withContext } from 'ds4viz';

await withContext(binarySearchTree({ label: 'demo_bst' }), async (bst) => {
    bst.insert(10);
    bst.insert(5);
    bst.insert(15);
    bst.insert(3);
    bst.insert(7);
    
    bst.delete(5);
    bst.insert(6);
});
```

#### `heap`

```typescript
function heap(options?: HeapOptions): Heap;

interface HeapOptions extends StructureOptions {
    /** 堆类型，"min" 表示最小堆，"max" 表示最大堆，默认为 "min" */
    heapType?: 'min' | 'max';
}
```

**Heap 类方法：**

```typescript
interface Heap extends Disposable {
    /**
     * 插入元素，自动维护堆性质
     * @param value - 要插入的值
     */
    insert(value: number): void;

    /**
     * 提取堆顶元素，自动维护堆性质
     * @throws {StructureError} 当堆为空时抛出异常
     */
    extract(): void;

    /**
     * 清空堆
     */
    clear(): void;
}
```

**示例：**

```typescript
import { heap, withContext } from 'ds4viz';

// 最小堆
await withContext(heap({ label: 'demo_min_heap', heapType: 'min' }), async (minHeap) => {
    minHeap.insert(10);
    minHeap.insert(5);
    minHeap.insert(15);
    minHeap.insert(3);
    
    minHeap.extract();  // 提取最小值 3
});

// 最大堆
await withContext(heap({ label: 'demo_max_heap', heapType: 'max' }), async (maxHeap) => {
    maxHeap.insert(10);
    maxHeap.insert(5);
    maxHeap.insert(15);
    maxHeap.insert(3);
    
    maxHeap.extract();  // 提取最大值 15
});
```

---

### 图结构

#### `graphUndirected`

```typescript
function graphUndirected(options?: StructureOptions): GraphUndirected;
```

**GraphUndirected 类方法：**

```typescript
interface GraphUndirected extends Disposable {
    /**
     * 添加节点
     * @param nodeId - 节点 id
     * @param label - 节点标签，长度限制为 1-32 字符
     * @throws {StructureError} 当节点已存在时抛出异常
     */
    addNode(nodeId: number, label: string): void;

    /**
     * 添加无向边，内部自动规范化为 fromId < toId
     * @param fromId - 起始节点 id
     * @param toId - 终止节点 id
     * @throws {StructureError} 当节点不存在、边已存在或形成自环时抛出异常
     */
    addEdge(fromId: number, toId: number): void;

    /**
     * 删除节点及其相关的所有边
     * @param nodeId - 要删除的节点 id
     * @throws {StructureError} 当节点不存在时抛出异常
     */
    removeNode(nodeId: number): void;

    /**
     * 删除边
     * @param fromId - 起始节点 id
     * @param toId - 终止节点 id
     * @throws {StructureError} 当边不存在时抛出异常
     */
    removeEdge(fromId: number, toId: number): void;

    /**
     * 更新节点标签
     * @param nodeId - 节点 id
     * @param label - 新的节点标签，长度限制为 1-32 字符
     * @throws {StructureError} 当节点不存在时抛出异常
     */
    updateNodeLabel(nodeId: number, label: string): void;
}
```

**示例：**

```typescript
import { graphUndirected, withContext } from 'ds4viz';

await withContext(graphUndirected({ label: 'demo_graph' }), async (graph) => {
    graph.addNode(0, 'A');
    graph.addNode(1, 'B');
    graph.addNode(2, 'C');
    
    graph.addEdge(0, 1);
    graph.addEdge(1, 2);
    
    graph.updateNodeLabel(1, 'B_updated');
    graph.removeEdge(0, 1);
});
```

#### `graphDirected`

```typescript
function graphDirected(options?: StructureOptions): GraphDirected;
```

**GraphDirected 类方法：**

```typescript
interface GraphDirected extends Disposable {
    /**
     * 添加节点
     * @param nodeId - 节点 id
     * @param label - 节点标签，长度限制为 1-32 字符
     * @throws {StructureError} 当节点已存在时抛出异常
     */
    addNode(nodeId: number, label: string): void;

    /**
     * 添加有向边
     * @param fromId - 起始节点 id
     * @param toId - 终止节点 id
     * @throws {StructureError} 当节点不存在、边已存在或形成自环时抛出异常
     */
    addEdge(fromId: number, toId: number): void;

    /**
     * 删除节点及其相关的所有边
     * @param nodeId - 要删除的节点 id
     * @throws {StructureError} 当节点不存在时抛出异常
     */
    removeNode(nodeId: number): void;

    /**
     * 删除边
     * @param fromId - 起始节点 id
     * @param toId - 终止节点 id
     * @throws {StructureError} 当边不存在时抛出异常
     */
    removeEdge(fromId: number, toId: number): void;

    /**
     * 更新节点标签
     * @param nodeId - 节点 id
     * @param label - 新的节点标签，长度限制为 1-32 字符
     * @throws {StructureError} 当节点不存在时抛出异常
     */
    updateNodeLabel(nodeId: number, label: string): void;
}
```

**示例：**

```typescript
import { graphDirected, withContext } from 'ds4viz';

await withContext(graphDirected({ label: 'demo_digraph' }), async (graph) => {
    graph.addNode(0, 'A');
    graph.addNode(1, 'B');
    graph.addNode(2, 'C');
    
    graph.addEdge(0, 1);
    graph.addEdge(1, 2);
    graph.addEdge(2, 0);
    
    graph.updateNodeLabel(1, 'B_updated');
    graph.removeEdge(2, 0);
});
```

#### `graphWeighted`

```typescript
function graphWeighted(options?: GraphWeightedOptions): GraphWeighted;

interface GraphWeightedOptions extends StructureOptions {
    /** 是否为有向图，默认为 false（无向图） */
    directed?: boolean;
}
```

**GraphWeighted 类方法：**

```typescript
interface GraphWeighted extends Disposable {
    /**
     * 添加节点
     * @param nodeId - 节点 id
     * @param label - 节点标签，长度限制为 1-32 字符
     * @throws {StructureError} 当节点已存在时抛出异常
     */
    addNode(nodeId: number, label: string): void;

    /**
     * 添加带权边
     * @param fromId - 起始节点 id
     * @param toId - 终止节点 id
     * @param weight - 边的权重
     * @throws {StructureError} 当节点不存在、边已存在或形成自环时抛出异常
     */
    addEdge(fromId: number, toId: number, weight: number): void;

    /**
     * 删除节点及其相关的所有边
     * @param nodeId - 要删除的节点 id
     * @throws {StructureError} 当节点不存在时抛出异常
     */
    removeNode(nodeId: number): void;

    /**
     * 删除边
     * @param fromId - 起始节点 id
     * @param toId - 终止节点 id
     * @throws {StructureError} 当边不存在时抛出异常
     */
    removeEdge(fromId: number, toId: number): void;

    /**
     * 更新边的权重
     * @param fromId - 起始节点 id
     * @param toId - 终止节点 id
     * @param weight - 新的权重值
     * @throws {StructureError} 当边不存在时抛出异常
     */
    updateWeight(fromId: number, toId: number, weight: number): void;

    /**
     * 更新节点标签
     * @param nodeId - 节点 id
     * @param label - 新的节点标签，长度限制为 1-32 字符
     * @throws {StructureError} 当节点不存在时抛出异常
     */
    updateNodeLabel(nodeId: number, label: string): void;
}
```

**示例：**

```typescript
import { graphWeighted, withContext } from 'ds4viz';

// 无向带权图
await withContext(graphWeighted({ label: 'demo_weighted_graph', directed: false }), async (graph) => {
    graph.addNode(0, 'A');
    graph.addNode(1, 'B');
    graph.addNode(2, 'C');
    
    graph.addEdge(0, 1, 3.5);
    graph.addEdge(1, 2, 2.0);
    graph.addEdge(2, 0, 4.2);
    
    graph.updateWeight(0, 1, 5.0);
    graph.removeEdge(2, 0);
});

// 有向带权图
await withContext(graphWeighted({ label: 'demo_directed_weighted', directed: true }), async (graph) => {
    graph.addNode(0, 'Start');
    graph.addNode(1, 'Middle');
    graph.addNode(2, 'End');
    
    graph.addEdge(0, 1, 1.5);
    graph.addEdge(1, 2, 2.5);
});
```

---

### 辅助函数

#### `withContext`

用于在不支持 `using` 语法的环境中管理数据结构的生命周期：

```typescript
async function withContext<T extends Disposable>(
    structure: T,
    callback: (structure: T) => void | Promise<void>
): Promise<void>;
```

**示例：**

```typescript
import { stack, withContext } from 'ds4viz';

await withContext(stack(), async (s) => {
    s.push(1);
    s.push(2);
    s.pop();
});
// 退出回调后自动生成 trace.toml
```

---

### 异常处理

所有数据结构在遇到错误时都会抛出 `StructureError`，并在资源释放时自动生成包含 `[error]` 部分的 `.toml` 文件。

**异常类型：**

```typescript
class DS4VizError extends Error {
    readonly errorType: ErrorType;
    readonly line?: number;
}

class StructureError extends DS4VizError {
    // 数据结构操作错误
}

class ValidationError extends DS4VizError {
    // 数据验证错误
}

enum ErrorType {
    Runtime = 'runtime',
    Timeout = 'timeout',
    Validation = 'validation',
    Sandbox = 'sandbox',
    Unknown = 'unknown'
}
```

**示例：**

```typescript
import { binaryTree, withContext, StructureError } from 'ds4viz';

try {
    await withContext(binaryTree(), async (tree) => {
        const root = tree.insertRoot(10);
        tree.insertLeft(999, 5);  // 父节点不存在，抛出异常
    });
} catch (e) {
    if (e instanceof StructureError) {
        console.error(`发生错误: ${e.message}`);
    }
}

// 即使发生异常，trace.toml 仍会生成，包含 [error] 部分
```

生成的 `trace.toml` 将包含：

```toml
[error]
type = "runtime"
message = "Parent node not found: 999"
line = 18
step = 1
last_state = 1
```

---

## 类型约束

根据 IR 定义，`value` 字段仅支持以下类型：

- `number`：整数或浮点数
- `string`：字符串
- `boolean`：布尔值

所有 API 在类型签名中都严格遵循此约束。

```typescript
type ValueType = number | string | boolean;
```

---

## 运行时要求

- **Node.js**: 18.0.0+
- **TypeScript**: 5.2+（如需使用 `using` 语法）
- **浏览器**: 支持 ES2022 的现代浏览器

### 关于 `using` 语法

TypeScript 5.2 引入了对 ECMAScript Explicit Resource Management 提案的支持。如果你的运行时支持 `Symbol.dispose`，可以直接使用 `using` 语法：

```typescript
await using stack = stack();
stack.push(1);
// 离开作用域时自动调用 dispose
```

对于不支持的环境，请使用 `withContext` 辅助函数。

---

## ESM 与 CommonJS

本库同时支持 ESM 和 CommonJS：

```typescript
// ESM
import { stack, queue, binaryTree } from 'ds4viz';

// CommonJS
const { stack, queue, binaryTree } = require('ds4viz');
```
