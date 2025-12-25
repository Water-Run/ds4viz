# `ds4viz` 说明文档

`ds4viz` 是一个 JavaScript 库，提供了 JavaScript 语言的 `ds4viz` 支持。`ds4viz` 是一个可视化数据结构工具。参见：[GitHub](https://github.com/Water-Run/ds4viz)

## 特性

与其他语言的 `ds4viz` 库不同，`js-ds4viz` 具有以下独特特性：

**双环境支持**：可在浏览器（客户端）和 Node.js（服务端）环境下运行。浏览器环境下生成 TOML 字符串供下载或进一步处理；Node.js 环境下可直接写入文件。

**单文件分发**：以单个 `ds4viz.js` 文件形式提供，无需构建工具或包管理器，可直接通过 `<script>` 标签引入或下载使用。

## 安装

### 浏览器环境

直接下载 `ds4viz.js` 文件并通过 `<script>` 标签引入：

```html
<script src="ds4viz.js"></script>
<script>
    const { stack } = ds4viz;
    // 使用库...
</script>
```

或使用 ES 模块：

```html
<script type="module">
    import { stack } from './ds4viz.js';
    // 使用库...
</script>
```

### Node.js 环境

将 `ds4viz.js` 文件放入项目目录后直接引入：

```javascript
import { stack } from './ds4viz.js';
// 或使用 CommonJS
const { stack } = require('./ds4viz.js');
```

## 测试

使用浏览器打开：

```bash
./test/ds4viz_web_test.html
```

点击运行按钮运行. 查看输出.  
可访问控制台查看详细的测试信息.  

## 快速上手

以下脚本提供了一个快速使用示例，演示如何使用 `js-ds4viz` 生成 `trace.toml`，并在发生异常时仍然产出包含 `[error]` 的 TOML IR：

### 浏览器环境

```html
<script type="module">
import { graphUndirected, withContext } from './ds4viz.js';

async function main() {
    // 使用上下文：确保无论成功/失败，都会生成 trace（成功 => [result]，异常 => [error]）
    await withContext(graphUndirected(), async (graph) => {
        // 添加节点
        graph.addNode(0, "A");
        graph.addNode(1, "B");
        graph.addNode(2, "C");

        // 添加无向边（内部会规范化为 from < to）
        graph.addEdge(0, 1);
        graph.addEdge(1, 2);

        // 也可以在这里继续调用更多 API，每次操作都会记录到 trace 中
    });

    // 浏览器环境下，可通过 getTomlString() 获取 TOML 字符串
    console.log("已生成 trace");
}

main();
</script>
```

### Node.js 环境

```javascript
import { graphUndirected, withContext } from './ds4viz.js';

async function main() {
    // 使用上下文：确保无论成功/失败，都会生成 trace.toml（成功 => [result]，异常 => [error]）
    await withContext(graphUndirected(), async (graph) => {
        // 添加节点
        graph.addNode(0, "A");
        graph.addNode(1, "B");
        graph.addNode(2, "C");

        // 添加无向边（内部会规范化为 from < to）
        graph.addEdge(0, 1);
        graph.addEdge(1, 2);
    });

    console.log("已生成 trace.toml");
}

main();
```

## API 参考

### 全局配置

#### `config`

```javascript
/**
 * 配置全局参数
 *
 * @param {Object} options - 配置选项
 * @param {string} [options.outputPath="trace.toml"] - 输出文件路径（仅 Node.js 环境有效）
 * @param {string} [options.title=""] - 可视化标题
 * @param {string} [options.author=""] - 作者信息
 * @param {string} [options.comment=""] - 注释说明
 * @returns {void}
 */
function config(options) {}
```

**示例：**

```javascript
import { config } from './ds4viz.js';

config({
    outputPath: "my_trace.toml",
    title: "栈操作演示",
    author: "WaterRun",
    comment: "演示基本的栈操作"
});
```

### 上下文管理

#### `withContext`

```javascript
/**
 * 使用上下文管理数据结构的生命周期
 *
 * @param {Object} structure - 数据结构实例
 * @param {Function} callback - 回调函数，接收数据结构实例作为参数
 * @returns {Promise<void>}
 */
async function withContext(structure, callback) {}
```

**示例：**

```javascript
import { stack, withContext } from './ds4viz.js';

await withContext(stack(), async (s) => {
    s.push(1);
    s.push(2);
    s.pop();
});
```

### 线性结构

#### `stack`

```javascript
/**
 * 创建栈实例
 *
 * @param {Object} [options] - 结构选项
 * @param {string} [options.label="stack"] - 栈的标签
 * @param {string} [options.output] - 输出文件路径，若未指定则使用全局配置
 * @returns {Stack} 栈实例
 */
function stack(options) {}
```

**Stack 类方法：**

```javascript
class Stack {
    /**
     * 压栈操作
     *
     * @param {number|string|boolean} value - 要压入的值
     * @returns {void}
     */
    push(value) {}

    /**
     * 弹栈操作
     *
     * @returns {void}
     * @throws {StructureError} 当栈为空时抛出异常
     */
    pop() {}

    /**
     * 清空栈
     *
     * @returns {void}
     */
    clear() {}
}
```

**示例：**

```javascript
import { stack, withContext } from './ds4viz.js';

await withContext(stack({ label: "demo_stack" }), async (s) => {
    s.push(10);
    s.push(20);
    s.push(30);

    s.pop();
    s.pop();
});
```

#### `queue`

```javascript
/**
 * 创建队列实例
 *
 * @param {Object} [options] - 结构选项
 * @param {string} [options.label="queue"] - 队列的标签
 * @param {string} [options.output] - 输出文件路径，若未指定则使用全局配置
 * @returns {Queue} 队列实例
 */
function queue(options) {}
```

**Queue 类方法：**

```javascript
class Queue {
    /**
     * 入队操作
     *
     * @param {number|string|boolean} value - 要入队的值
     * @returns {void}
     */
    enqueue(value) {}

    /**
     * 出队操作
     *
     * @returns {void}
     * @throws {StructureError} 当队列为空时抛出异常
     */
    dequeue() {}

    /**
     * 清空队列
     *
     * @returns {void}
     */
    clear() {}
}
```

**示例：**

```javascript
import { queue, withContext } from './ds4viz.js';

await withContext(queue({ label: "demo_queue" }), async (q) => {
    q.enqueue(10);
    q.enqueue(20);
    q.enqueue(30);

    q.dequeue();
    q.dequeue();
});
```

#### `singleLinkedList`

```javascript
/**
 * 创建单链表实例
 *
 * @param {Object} [options] - 结构选项
 * @param {string} [options.label="slist"] - 单链表的标签
 * @param {string} [options.output] - 输出文件路径，若未指定则使用全局配置
 * @returns {SingleLinkedList} 单链表实例
 */
function singleLinkedList(options) {}
```

**SingleLinkedList 类方法：**

```javascript
class SingleLinkedList {
    /**
     * 在链表头部插入节点
     *
     * @param {number|string|boolean} value - 要插入的值
     * @returns {number} 新插入节点的 id
     */
    insertHead(value) {}

    /**
     * 在链表尾部插入节点
     *
     * @param {number|string|boolean} value - 要插入的值
     * @returns {number} 新插入节点的 id
     */
    insertTail(value) {}

    /**
     * 在指定节点后插入新节点
     *
     * @param {number} nodeId - 目标节点的 id
     * @param {number|string|boolean} value - 要插入的值
     * @returns {number} 新插入节点的 id
     * @throws {StructureError} 当指定节点不存在时抛出异常
     */
    insertAfter(nodeId, value) {}

    /**
     * 删除指定节点
     *
     * @param {number} nodeId - 要删除的节点 id
     * @returns {void}
     * @throws {StructureError} 当指定节点不存在时抛出异常
     */
    delete(nodeId) {}

    /**
     * 删除头节点
     *
     * @returns {void}
     * @throws {StructureError} 当链表为空时抛出异常
     */
    deleteHead() {}

    /**
     * 反转链表
     *
     * @returns {void}
     */
    reverse() {}
}
```

**示例：**

```javascript
import { singleLinkedList, withContext } from './ds4viz.js';

await withContext(singleLinkedList({ label: "demo_slist" }), async (slist) => {
    const nodeA = slist.insertHead(10);
    slist.insertTail(20);
    const nodeC = slist.insertTail(30);

    slist.insertAfter(nodeA, 15);

    slist.delete(nodeC);
    slist.reverse();
});
```

#### `doubleLinkedList`

```javascript
/**
 * 创建双向链表实例
 *
 * @param {Object} [options] - 结构选项
 * @param {string} [options.label="dlist"] - 双向链表的标签
 * @param {string} [options.output] - 输出文件路径，若未指定则使用全局配置
 * @returns {DoubleLinkedList} 双向链表实例
 */
function doubleLinkedList(options) {}
```

**DoubleLinkedList 类方法：**

```javascript
class DoubleLinkedList {
    /**
     * 在链表头部插入节点
     *
     * @param {number|string|boolean} value - 要插入的值
     * @returns {number} 新插入节点的 id
     */
    insertHead(value) {}

    /**
     * 在链表尾部插入节点
     *
     * @param {number|string|boolean} value - 要插入的值
     * @returns {number} 新插入节点的 id
     */
    insertTail(value) {}

    /**
     * 在指定节点前插入新节点
     *
     * @param {number} nodeId - 目标节点的 id
     * @param {number|string|boolean} value - 要插入的值
     * @returns {number} 新插入节点的 id
     * @throws {StructureError} 当指定节点不存在时抛出异常
     */
    insertBefore(nodeId, value) {}

    /**
     * 在指定节点后插入新节点
     *
     * @param {number} nodeId - 目标节点的 id
     * @param {number|string|boolean} value - 要插入的值
     * @returns {number} 新插入节点的 id
     * @throws {StructureError} 当指定节点不存在时抛出异常
     */
    insertAfter(nodeId, value) {}

    /**
     * 删除指定节点
     *
     * @param {number} nodeId - 要删除的节点 id
     * @returns {void}
     * @throws {StructureError} 当指定节点不存在时抛出异常
     */
    delete(nodeId) {}

    /**
     * 删除头节点
     *
     * @returns {void}
     * @throws {StructureError} 当链表为空时抛出异常
     */
    deleteHead() {}

    /**
     * 删除尾节点
     *
     * @returns {void}
     * @throws {StructureError} 当链表为空时抛出异常
     */
    deleteTail() {}

    /**
     * 反转链表
     *
     * @returns {void}
     */
    reverse() {}
}
```

**示例：**

```javascript
import { doubleLinkedList, withContext } from './ds4viz.js';

await withContext(doubleLinkedList({ label: "demo_dlist" }), async (dlist) => {
    const nodeA = dlist.insertHead(10);
    dlist.insertTail(30);

    dlist.insertAfter(nodeA, 20);

    dlist.deleteTail();
    dlist.reverse();
});
```

### 树结构

#### `binaryTree`

```javascript
/**
 * 创建二叉树实例
 *
 * @param {Object} [options] - 结构选项
 * @param {string} [options.label="binary_tree"] - 二叉树的标签
 * @param {string} [options.output] - 输出文件路径，若未指定则使用全局配置
 * @returns {BinaryTree} 二叉树实例
 */
function binaryTree(options) {}
```

**BinaryTree 类方法：**

```javascript
class BinaryTree {
    /**
     * 插入根节点
     *
     * @param {number|string|boolean} value - 根节点的值
     * @returns {number} 根节点的 id
     * @throws {StructureError} 当根节点已存在时抛出异常
     */
    insertRoot(value) {}

    /**
     * 在指定父节点的左侧插入子节点
     *
     * @param {number} parentId - 父节点的 id
     * @param {number|string|boolean} value - 要插入的子节点值
     * @returns {number} 新插入节点的 id
     * @throws {StructureError} 当父节点不存在或左子节点已存在时抛出异常
     */
    insertLeft(parentId, value) {}

    /**
     * 在指定父节点的右侧插入子节点
     *
     * @param {number} parentId - 父节点的 id
     * @param {number|string|boolean} value - 要插入的子节点值
     * @returns {number} 新插入节点的 id
     * @throws {StructureError} 当父节点不存在或右子节点已存在时抛出异常
     */
    insertRight(parentId, value) {}

    /**
     * 删除指定节点及其所有子树
     *
     * @param {number} nodeId - 要删除的节点 id
     * @returns {void}
     * @throws {StructureError} 当节点不存在时抛出异常
     */
    delete(nodeId) {}

    /**
     * 更新节点的值
     *
     * @param {number} nodeId - 节点 id
     * @param {number|string|boolean} value - 新的值
     * @returns {void}
     * @throws {StructureError} 当节点不存在时抛出异常
     */
    updateValue(nodeId, value) {}
}
```

**示例：**

```javascript
import { binaryTree, withContext } from './ds4viz.js';

await withContext(binaryTree({ label: "demo_tree" }), async (tree) => {
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

```javascript
/**
 * 创建二叉搜索树实例
 *
 * @param {Object} [options] - 结构选项
 * @param {string} [options.label="bst"] - 二叉搜索树的标签
 * @param {string} [options.output] - 输出文件路径，若未指定则使用全局配置
 * @returns {BinarySearchTree} 二叉搜索树实例
 */
function binarySearchTree(options) {}
```

**BinarySearchTree 类方法：**

```javascript
class BinarySearchTree {
    /**
     * 插入节点，自动维护二叉搜索树性质
     *
     * @param {number} value - 要插入的值
     * @returns {number} 新插入节点的 id
     */
    insert(value) {}

    /**
     * 删除节点，自动维护二叉搜索树性质
     *
     * @param {number} value - 要删除的节点值
     * @returns {void}
     * @throws {StructureError} 当节点不存在时抛出异常
     */
    delete(value) {}
}
```

**示例：**

```javascript
import { binarySearchTree, withContext } from './ds4viz.js';

await withContext(binarySearchTree({ label: "demo_bst" }), async (bst) => {
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

```javascript
/**
 * 创建堆实例
 *
 * @param {Object} [options] - 堆选项
 * @param {string} [options.label="heap"] - 堆的标签
 * @param {string} [options.heapType="min"] - 堆类型，"min" 表示最小堆，"max" 表示最大堆
 * @param {string} [options.output] - 输出文件路径，若未指定则使用全局配置
 * @returns {Heap} 堆实例
 * @throws {Error} 当 heapType 不是 "min" 或 "max" 时抛出异常
 */
function heap(options) {}
```

**Heap 类方法：**

```javascript
class Heap {
    /**
     * 插入元素，自动维护堆性质
     *
     * @param {number} value - 要插入的值
     * @returns {void}
     */
    insert(value) {}

    /**
     * 提取堆顶元素，自动维护堆性质
     *
     * @returns {void}
     * @throws {StructureError} 当堆为空时抛出异常
     */
    extract() {}

    /**
     * 清空堆
     *
     * @returns {void}
     */
    clear() {}
}
```

**示例：**

```javascript
import { heap, withContext } from './ds4viz.js';

// 最小堆
await withContext(heap({ label: "demo_min_heap", heapType: "min" }), async (minHeap) => {
    minHeap.insert(10);
    minHeap.insert(5);
    minHeap.insert(15);
    minHeap.insert(3);

    minHeap.extract();  // 提取最小值 3
});

// 最大堆
await withContext(heap({ label: "demo_max_heap", heapType: "max" }), async (maxHeap) => {
    maxHeap.insert(10);
    maxHeap.insert(5);
    maxHeap.insert(15);
    maxHeap.insert(3);

    maxHeap.extract();  // 提取最大值 15
});
```

### 图结构

#### `graphUndirected`

```javascript
/**
 * 创建无向图实例
 *
 * @param {Object} [options] - 结构选项
 * @param {string} [options.label="graph"] - 无向图的标签
 * @param {string} [options.output] - 输出文件路径，若未指定则使用全局配置
 * @returns {GraphUndirected} 无向图实例
 */
function graphUndirected(options) {}
```

**GraphUndirected 类方法：**

```javascript
class GraphUndirected {
    /**
     * 添加节点
     *
     * @param {number} nodeId - 节点 id
     * @param {string} label - 节点标签，长度限制为 1-32 字符
     * @returns {void}
     * @throws {StructureError} 当节点已存在时抛出异常
     */
    addNode(nodeId, label) {}

    /**
     * 添加无向边，内部自动规范化为 fromId < toId
     *
     * @param {number} fromId - 起始节点 id
     * @param {number} toId - 终止节点 id
     * @returns {void}
     * @throws {StructureError} 当节点不存在、边已存在或形成自环时抛出异常
     */
    addEdge(fromId, toId) {}

    /**
     * 删除节点及其相关的所有边
     *
     * @param {number} nodeId - 要删除的节点 id
     * @returns {void}
     * @throws {StructureError} 当节点不存在时抛出异常
     */
    removeNode(nodeId) {}

    /**
     * 删除边
     *
     * @param {number} fromId - 起始节点 id
     * @param {number} toId - 终止节点 id
     * @returns {void}
     * @throws {StructureError} 当边不存在时抛出异常
     */
    removeEdge(fromId, toId) {}

    /**
     * 更新节点标签
     *
     * @param {number} nodeId - 节点 id
     * @param {string} label - 新的节点标签，长度限制为 1-32 字符
     * @returns {void}
     * @throws {StructureError} 当节点不存在时抛出异常
     */
    updateNodeLabel(nodeId, label) {}
}
```

**示例：**

```javascript
import { graphUndirected, withContext } from './ds4viz.js';

await withContext(graphUndirected({ label: "demo_graph" }), async (graph) => {
    graph.addNode(0, "A");
    graph.addNode(1, "B");
    graph.addNode(2, "C");

    graph.addEdge(0, 1);
    graph.addEdge(1, 2);

    graph.updateNodeLabel(1, "B_updated");
    graph.removeEdge(0, 1);
});
```

#### `graphDirected`

```javascript
/**
 * 创建有向图实例
 *
 * @param {Object} [options] - 结构选项
 * @param {string} [options.label="graph"] - 有向图的标签
 * @param {string} [options.output] - 输出文件路径，若未指定则使用全局配置
 * @returns {GraphDirected} 有向图实例
 */
function graphDirected(options) {}
```

**GraphDirected 类方法：**

```javascript
class GraphDirected {
    /**
     * 添加节点
     *
     * @param {number} nodeId - 节点 id
     * @param {string} label - 节点标签，长度限制为 1-32 字符
     * @returns {void}
     * @throws {StructureError} 当节点已存在时抛出异常
     */
    addNode(nodeId, label) {}

    /**
     * 添加有向边
     *
     * @param {number} fromId - 起始节点 id
     * @param {number} toId - 终止节点 id
     * @returns {void}
     * @throws {StructureError} 当节点不存在、边已存在或形成自环时抛出异常
     */
    addEdge(fromId, toId) {}

    /**
     * 删除节点及其相关的所有边
     *
     * @param {number} nodeId - 要删除的节点 id
     * @returns {void}
     * @throws {StructureError} 当节点不存在时抛出异常
     */
    removeNode(nodeId) {}

    /**
     * 删除边
     *
     * @param {number} fromId - 起始节点 id
     * @param {number} toId - 终止节点 id
     * @returns {void}
     * @throws {StructureError} 当边不存在时抛出异常
     */
    removeEdge(fromId, toId) {}

    /**
     * 更新节点标签
     *
     * @param {number} nodeId - 节点 id
     * @param {string} label - 新的节点标签，长度限制为 1-32 字符
     * @returns {void}
     * @throws {StructureError} 当节点不存在时抛出异常
     */
    updateNodeLabel(nodeId, label) {}
}
```

**示例：**

```javascript
import { graphDirected, withContext } from './ds4viz.js';

await withContext(graphDirected({ label: "demo_digraph" }), async (graph) => {
    graph.addNode(0, "A");
    graph.addNode(1, "B");
    graph.addNode(2, "C");

    graph.addEdge(0, 1);
    graph.addEdge(1, 2);
    graph.addEdge(2, 0);

    graph.updateNodeLabel(1, "B_updated");
    graph.removeEdge(2, 0);
});
```

#### `graphWeighted`

```javascript
/**
 * 创建带权图实例
 *
 * @param {Object} [options] - 带权图选项
 * @param {string} [options.label="graph"] - 带权图的标签
 * @param {boolean} [options.directed=false] - 是否为有向图，默认为 false（无向图）
 * @param {string} [options.output] - 输出文件路径，若未指定则使用全局配置
 * @returns {GraphWeighted} 带权图实例
 */
function graphWeighted(options) {}
```

**GraphWeighted 类方法：**

```javascript
class GraphWeighted {
    /**
     * 添加节点
     *
     * @param {number} nodeId - 节点 id
     * @param {string} label - 节点标签，长度限制为 1-32 字符
     * @returns {void}
     * @throws {StructureError} 当节点已存在时抛出异常
     */
    addNode(nodeId, label) {}

    /**
     * 添加带权边
     *
     * @param {number} fromId - 起始节点 id
     * @param {number} toId - 终止节点 id
     * @param {number} weight - 边的权重
     * @returns {void}
     * @throws {StructureError} 当节点不存在、边已存在或形成自环时抛出异常
     */
    addEdge(fromId, toId, weight) {}

    /**
     * 删除节点及其相关的所有边
     *
     * @param {number} nodeId - 要删除的节点 id
     * @returns {void}
     * @throws {StructureError} 当节点不存在时抛出异常
     */
    removeNode(nodeId) {}

    /**
     * 删除边
     *
     * @param {number} fromId - 起始节点 id
     * @param {number} toId - 终止节点 id
     * @returns {void}
     * @throws {StructureError} 当边不存在时抛出异常
     */
    removeEdge(fromId, toId) {}

    /**
     * 更新边的权重
     *
     * @param {number} fromId - 起始节点 id
     * @param {number} toId - 终止节点 id
     * @param {number} weight - 新的权重值
     * @returns {void}
     * @throws {StructureError} 当边不存在时抛出异常
     */
    updateWeight(fromId, toId, weight) {}

    /**
     * 更新节点标签
     *
     * @param {number} nodeId - 节点 id
     * @param {string} label - 新的节点标签，长度限制为 1-32 字符
     * @returns {void}
     * @throws {StructureError} 当节点不存在时抛出异常
     */
    updateNodeLabel(nodeId, label) {}
}
```

**示例：**

```javascript
import { graphWeighted, withContext } from './ds4viz.js';

// 无向带权图
await withContext(graphWeighted({ label: "demo_weighted_graph", directed: false }), async (graph) => {
    graph.addNode(0, "A");
    graph.addNode(1, "B");
    graph.addNode(2, "C");

    graph.addEdge(0, 1, 3.5);
    graph.addEdge(1, 2, 2.0);
    graph.addEdge(2, 0, 4.2);

    graph.updateWeight(0, 1, 5.0);
    graph.removeEdge(2, 0);
});

// 有向带权图
await withContext(graphWeighted({ label: "demo_directed_weighted", directed: true }), async (graph) => {
    graph.addNode(0, "Start");
    graph.addNode(1, "Middle");
    graph.addNode(2, "End");

    graph.addEdge(0, 1, 1.5);
    graph.addEdge(1, 2, 2.5);
});
```

### 浏览器环境特有 API

#### `getTomlString`

在浏览器环境下，由于无法直接写入文件，可通过此方法获取生成的 TOML 字符串：

```javascript
/**
 * 获取生成的 TOML 字符串（仅浏览器环境）
 *
 * @param {Object} structure - 数据结构实例
 * @returns {string} TOML 格式的字符串
 */
function getTomlString(structure) {}
```

**示例：**

```javascript
import { stack, withContext, getTomlString } from './ds4viz.js';

let tomlContent = '';

await withContext(stack(), async (s) => {
    s.push(1);
    s.push(2);
});

// 获取 TOML 字符串
tomlContent = getTomlString(s);

// 可以创建下载链接或进行其他处理
const blob = new Blob([tomlContent], { type: 'text/plain' });
const url = URL.createObjectURL(blob);
// ...
```

### 异常处理

所有数据结构在遇到错误时都会抛出 `StructureError`，并在退出上下文时自动生成包含 `[error]` 部分的 TOML。

**示例：**

```javascript
import { binaryTree, withContext } from './ds4viz.js';

try {
    await withContext(binaryTree(), async (tree) => {
        const root = tree.insertRoot(10);
        tree.insertLeft(999, 5);  // 父节点不存在，抛出异常
    });
} catch (error) {
    console.log(`发生错误: ${error.message}`);
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

### 错误类型

```javascript
/**
 * 错误类型枚举
 *
 * @enum {string}
 */
const ErrorType = {
    Runtime: 'runtime',
    Timeout: 'timeout',
    Validation: 'validation',
    Sandbox: 'sandbox',
    Unknown: 'unknown',
};
```

### 自定义错误类

```javascript
/**
 * ds4viz 库的基础异常类
 */
class DS4VizError extends Error {
    /** @type {string} 错误类型 */
    errorType;
    /** @type {number|undefined} 发生错误的代码行号 */
    line;
}

/**
 * 数据结构操作错误
 */
class StructureError extends DS4VizError {}

/**
 * 数据验证错误
 */
class ValidationError extends DS4VizError {}
```

## 类型约束

根据 IR 定义，`value` 字段仅支持以下类型：

- `number`：数值（整数或浮点数）
- `string`：字符串
- `boolean`：布尔值

所有 API 在 JSDoc 类型注解中都严格遵循此约束。

## 环境检测

库会自动检测运行环境：

- **浏览器环境**：生成 TOML 字符串，通过 `getTomlString()` 获取或触发下载
- **Node.js 环境**：直接写入文件系统

```javascript
import { isNodeEnvironment, isBrowserEnvironment } from './ds4viz.js';

if (isNodeEnvironment()) {
    console.log('运行在 Node.js 环境');
} else if (isBrowserEnvironment()) {
    console.log('运行在浏览器环境');
}
```
