# `ds4viz`  

`ds4viz` 是一个 PHP 库，提供了 PHP 语言的 `ds4viz` 支持。
`ds4viz` 是一个可视化数据结构工具。参见：[GitHub](https://github.com/Water-Run/ds4viz)

## 安装

`ds4viz` 发布在 [Packagist](https://packagist.org/packages/ds4viz/ds4viz) 上：

```bash
composer require ds4viz/ds4viz
```

## 要求

- PHP 8.2 或更高版本
- Composer

## 测试

在目录下运行：

```bash
composer test
```

或直接使用 PHPUnit：

```bash
./vendor/bin/phpunit
```

## 快速上手

以下脚本提供了一个快速使用示例，演示如何使用 `ds4viz` 生成 `trace.toml`，并在发生异常时仍然产出包含 `[error]` 的 `.toml` IR 文件：

```php
<?php

declare(strict_types=1);

require_once __DIR__ . '/vendor/autoload.php';

use Ds4viz\Ds4viz;

function main(): void
{
    // 使用上下文：确保无论成功/失败，都会生成 trace.toml（成功 => [result]，异常 => [error]）
    Ds4viz::graphUndirected(label: 'demo_graph')
        ->run(function ($graph) {
            // 添加节点
            $graph->addNode(0, 'A');
            $graph->addNode(1, 'B');
            $graph->addNode(2, 'C');

            // 添加无向边（内部会规范化为 from < to）
            $graph->addEdge(0, 1);
            $graph->addEdge(1, 2);

            // 也可以在这里继续调用更多 API，每次操作都会记录到 trace 中
        });

    echo "已生成 trace.toml\n";
}

main();
```

## API 参考

### 全局配置

#### `Ds4viz::config()`

```php
/**
 * 配置全局参数
 *
 * @param string $outputPath 输出文件路径，默认为 "trace.toml"
 * @param string $title 可视化标题
 * @param string $author 作者信息
 * @param string $comment 注释说明
 * @return void
 */
public static function config(
    string $outputPath = 'trace.toml',
    string $title = '',
    string $author = '',
    string $comment = ''
): void;
```

**示例：**

```php
<?php

use Ds4viz\Ds4viz;

Ds4viz::config(
    outputPath: 'my_trace.toml',
    title: '栈操作演示',
    author: 'WaterRun',
    comment: '演示基本的栈操作'
);
```

---

### 线性结构

#### `Ds4viz::stack()`

```php
/**
 * 创建栈实例
 *
 * @param string $label 栈的标签，默认为 "stack"
 * @param string|null $output 输出文件路径，若为 null 则使用全局配置
 * @return Stack 栈实例（支持链式调用）
 */
public static function stack(
    string $label = 'stack',
    ?string $output = null
): Stack;
```

**Stack 类方法：**

```php
/**
 * 栈数据结构
 */
class Stack
{
    /**
     * 执行栈操作
     *
     * @param callable(Stack): void $callback 回调函数
     * @return void
     */
    public function run(callable $callback): void;

    /**
     * 压栈操作
     *
     * @param int|float|string|bool $value 要压入的值
     * @return void
     */
    public function push(int|float|string|bool $value): void;

    /**
     * 弹栈操作
     *
     * @return void
     * @throws StructureException 当栈为空时抛出异常
     */
    public function pop(): void;

    /**
     * 清空栈
     *
     * @return void
     */
    public function clear(): void;
}
```

**示例：**

```php
<?php

use Ds4viz\Ds4viz;

Ds4viz::stack(label: 'demo_stack')
    ->run(function ($s) {
        $s->push(10);
        $s->push(20);
        $s->push(30);

        $s->pop();
        $s->pop();
    });
```

#### `Ds4viz::queue()`

```php
/**
 * 创建队列实例
 *
 * @param string $label 队列的标签，默认为 "queue"
 * @param string|null $output 输出文件路径，若为 null 则使用全局配置
 * @return Queue 队列实例
 */
public static function queue(
    string $label = 'queue',
    ?string $output = null
): Queue;
```

**Queue 类方法：**

```php
/**
 * 队列数据结构
 */
class Queue
{
    /**
     * 执行队列操作
     *
     * @param callable(Queue): void $callback 回调函数
     * @return void
     */
    public function run(callable $callback): void;

    /**
     * 入队操作
     *
     * @param int|float|string|bool $value 要入队的值
     * @return void
     */
    public function enqueue(int|float|string|bool $value): void;

    /**
     * 出队操作
     *
     * @return void
     * @throws StructureException 当队列为空时抛出异常
     */
    public function dequeue(): void;

    /**
     * 清空队列
     *
     * @return void
     */
    public function clear(): void;
}
```

**示例：**

```php
<?php

use Ds4viz\Ds4viz;

Ds4viz::queue(label: 'demo_queue')
    ->run(function ($q) {
        $q->enqueue(10);
        $q->enqueue(20);
        $q->enqueue(30);

        $q->dequeue();
        $q->dequeue();
    });
```

#### `Ds4viz::singleLinkedList()`

```php
/**
 * 创建单链表实例
 *
 * @param string $label 单链表的标签，默认为 "slist"
 * @param string|null $output 输出文件路径，若为 null 则使用全局配置
 * @return SingleLinkedList 单链表实例
 */
public static function singleLinkedList(
    string $label = 'slist',
    ?string $output = null
): SingleLinkedList;
```

**SingleLinkedList 类方法：**

```php
/**
 * 单链表数据结构
 */
class SingleLinkedList
{
    /**
     * 执行链表操作
     *
     * @param callable(SingleLinkedList): void $callback 回调函数
     * @return void
     */
    public function run(callable $callback): void;

    /**
     * 在链表头部插入节点
     *
     * @param int|float|string|bool $value 要插入的值
     * @return int 新插入节点的 id
     */
    public function insertHead(int|float|string|bool $value): int;

    /**
     * 在链表尾部插入节点
     *
     * @param int|float|string|bool $value 要插入的值
     * @return int 新插入节点的 id
     */
    public function insertTail(int|float|string|bool $value): int;

    /**
     * 在指定节点后插入新节点
     *
     * @param int $nodeId 目标节点的 id
     * @param int|float|string|bool $value 要插入的值
     * @return int 新插入节点的 id
     * @throws StructureException 当指定节点不存在时抛出异常
     */
    public function insertAfter(int $nodeId, int|float|string|bool $value): int;

    /**
     * 删除指定节点
     *
     * @param int $nodeId 要删除的节点 id
     * @return void
     * @throws StructureException 当指定节点不存在时抛出异常
     */
    public function delete(int $nodeId): void;

    /**
     * 删除头节点
     *
     * @return void
     * @throws StructureException 当链表为空时抛出异常
     */
    public function deleteHead(): void;

    /**
     * 反转链表
     *
     * @return void
     */
    public function reverse(): void;
}
```

**示例：**

```php
<?php

use Ds4viz\Ds4viz;

Ds4viz::singleLinkedList(label: 'demo_slist')
    ->run(function ($slist) {
        $nodeA = $slist->insertHead(10);
        $slist->insertTail(20);
        $nodeC = $slist->insertTail(30);

        $slist->insertAfter($nodeA, 15);

        $slist->delete($nodeC);
        $slist->reverse();
    });
```

#### `Ds4viz::doubleLinkedList()`

```php
/**
 * 创建双向链表实例
 *
 * @param string $label 双向链表的标签，默认为 "dlist"
 * @param string|null $output 输出文件路径，若为 null 则使用全局配置
 * @return DoubleLinkedList 双向链表实例
 */
public static function doubleLinkedList(
    string $label = 'dlist',
    ?string $output = null
): DoubleLinkedList;
```

**DoubleLinkedList 类方法：**

```php
/**
 * 双向链表数据结构
 */
class DoubleLinkedList
{
    /**
     * 执行链表操作
     *
     * @param callable(DoubleLinkedList): void $callback 回调函数
     * @return void
     */
    public function run(callable $callback): void;

    /**
     * 在链表头部插入节点
     *
     * @param int|float|string|bool $value 要插入的值
     * @return int 新插入节点的 id
     */
    public function insertHead(int|float|string|bool $value): int;

    /**
     * 在链表尾部插入节点
     *
     * @param int|float|string|bool $value 要插入的值
     * @return int 新插入节点的 id
     */
    public function insertTail(int|float|string|bool $value): int;

    /**
     * 在指定节点前插入新节点
     *
     * @param int $nodeId 目标节点的 id
     * @param int|float|string|bool $value 要插入的值
     * @return int 新插入节点的 id
     * @throws StructureException 当指定节点不存在时抛出异常
     */
    public function insertBefore(int $nodeId, int|float|string|bool $value): int;

    /**
     * 在指定节点后插入新节点
     *
     * @param int $nodeId 目标节点的 id
     * @param int|float|string|bool $value 要插入的值
     * @return int 新插入节点的 id
     * @throws StructureException 当指定节点不存在时抛出异常
     */
    public function insertAfter(int $nodeId, int|float|string|bool $value): int;

    /**
     * 删除指定节点
     *
     * @param int $nodeId 要删除的节点 id
     * @return void
     * @throws StructureException 当指定节点不存在时抛出异常
     */
    public function delete(int $nodeId): void;

    /**
     * 删除头节点
     *
     * @return void
     * @throws StructureException 当链表为空时抛出异常
     */
    public function deleteHead(): void;

    /**
     * 删除尾节点
     *
     * @return void
     * @throws StructureException 当链表为空时抛出异常
     */
    public function deleteTail(): void;

    /**
     * 反转链表
     *
     * @return void
     */
    public function reverse(): void;
}
```

**示例：**

```php
<?php

use Ds4viz\Ds4viz;

Ds4viz::doubleLinkedList(label: 'demo_dlist')
    ->run(function ($dlist) {
        $nodeA = $dlist->insertHead(10);
        $dlist->insertTail(30);

        $dlist->insertAfter($nodeA, 20);

        $dlist->deleteTail();
        $dlist->reverse();
    });
```

---

### 树结构

#### `Ds4viz::binaryTree()`

```php
/**
 * 创建二叉树实例
 *
 * @param string $label 二叉树的标签，默认为 "binary_tree"
 * @param string|null $output 输出文件路径，若为 null 则使用全局配置
 * @return BinaryTree 二叉树实例
 */
public static function binaryTree(
    string $label = 'binary_tree',
    ?string $output = null
): BinaryTree;
```

**BinaryTree 类方法：**

```php
/**
 * 二叉树数据结构
 */
class BinaryTree
{
    /**
     * 执行二叉树操作
     *
     * @param callable(BinaryTree): void $callback 回调函数
     * @return void
     */
    public function run(callable $callback): void;

    /**
     * 插入根节点
     *
     * @param int|float|string|bool $value 根节点的值
     * @return int 根节点的 id
     * @throws StructureException 当根节点已存在时抛出异常
     */
    public function insertRoot(int|float|string|bool $value): int;

    /**
     * 在指定父节点的左侧插入子节点
     *
     * @param int $parentId 父节点的 id
     * @param int|float|string|bool $value 要插入的子节点值
     * @return int 新插入节点的 id
     * @throws StructureException 当父节点不存在或左子节点已存在时抛出异常
     */
    public function insertLeft(int $parentId, int|float|string|bool $value): int;

    /**
     * 在指定父节点的右侧插入子节点
     *
     * @param int $parentId 父节点的 id
     * @param int|float|string|bool $value 要插入的子节点值
     * @return int 新插入节点的 id
     * @throws StructureException 当父节点不存在或右子节点已存在时抛出异常
     */
    public function insertRight(int $parentId, int|float|string|bool $value): int;

    /**
     * 删除指定节点及其所有子树
     *
     * @param int $nodeId 要删除的节点 id
     * @return void
     * @throws StructureException 当节点不存在时抛出异常
     */
    public function delete(int $nodeId): void;

    /**
     * 更新节点的值
     *
     * @param int $nodeId 节点 id
     * @param int|float|string|bool $value 新的值
     * @return void
     * @throws StructureException 当节点不存在时抛出异常
     */
    public function updateValue(int $nodeId, int|float|string|bool $value): void;
}
```

**示例：**

```php
<?php

use Ds4viz\Ds4viz;

Ds4viz::binaryTree(label: 'demo_tree')
    ->run(function ($tree) {
        $root = $tree->insertRoot(10);
        $left = $tree->insertLeft($root, 5);
        $right = $tree->insertRight($root, 15);

        $tree->insertLeft($left, 3);
        $tree->insertRight($left, 7);

        $tree->updateValue($right, 20);
        $tree->delete($left);
    });
```

#### `Ds4viz::binarySearchTree()`

```php
/**
 * 创建二叉搜索树实例
 *
 * @param string $label 二叉搜索树的标签，默认为 "bst"
 * @param string|null $output 输出文件路径，若为 null 则使用全局配置
 * @return BinarySearchTree 二叉搜索树实例
 */
public static function binarySearchTree(
    string $label = 'bst',
    ?string $output = null
): BinarySearchTree;
```

**BinarySearchTree 类方法：**

```php
/**
 * 二叉搜索树数据结构
 */
class BinarySearchTree
{
    /**
     * 执行 BST 操作
     *
     * @param callable(BinarySearchTree): void $callback 回调函数
     * @return void
     */
    public function run(callable $callback): void;

    /**
     * 插入节点，自动维护二叉搜索树性质
     *
     * @param int|float $value 要插入的值
     * @return int 新插入节点的 id
     */
    public function insert(int|float $value): int;

    /**
     * 删除节点，自动维护二叉搜索树性质
     *
     * @param int|float $value 要删除的节点值
     * @return void
     * @throws StructureException 当节点不存在时抛出异常
     */
    public function delete(int|float $value): void;
}
```

**示例：**

```php
<?php

use Ds4viz\Ds4viz;

Ds4viz::binarySearchTree(label: 'demo_bst')
    ->run(function ($bst) {
        $bst->insert(10);
        $bst->insert(5);
        $bst->insert(15);
        $bst->insert(3);
        $bst->insert(7);

        $bst->delete(5);
        $bst->insert(6);
    });
```

#### `Ds4viz::heap()`

```php
/**
 * 创建堆实例
 *
 * @param string $label 堆的标签，默认为 "heap"
 * @param string $heapType 堆类型，"min" 表示最小堆，"max" 表示最大堆，默认为 "min"
 * @param string|null $output 输出文件路径，若为 null 则使用全局配置
 * @return Heap 堆实例
 * @throws \InvalidArgumentException 当 heapType 不是 "min" 或 "max" 时抛出异常
 */
public static function heap(
    string $label = 'heap',
    string $heapType = 'min',
    ?string $output = null
): Heap;
```

**Heap 类方法：**

```php
/**
 * 堆数据结构
 */
class Heap
{
    /**
     * 执行堆操作
     *
     * @param callable(Heap): void $callback 回调函数
     * @return void
     */
    public function run(callable $callback): void;

    /**
     * 插入元素，自动维护堆性质
     *
     * @param int|float $value 要插入的值
     * @return void
     */
    public function insert(int|float $value): void;

    /**
     * 提取堆顶元素，自动维护堆性质
     *
     * @return void
     * @throws StructureException 当堆为空时抛出异常
     */
    public function extract(): void;

    /**
     * 清空堆
     *
     * @return void
     */
    public function clear(): void;
}
```

**示例：**

```php
<?php

use Ds4viz\Ds4viz;

// 最小堆
Ds4viz::heap(label: 'demo_min_heap', heapType: 'min')
    ->run(function ($minHeap) {
        $minHeap->insert(10);
        $minHeap->insert(5);
        $minHeap->insert(15);
        $minHeap->insert(3);

        $minHeap->extract();  // 提取最小值 3
    });

// 最大堆
Ds4viz::heap(label: 'demo_max_heap', heapType: 'max')
    ->run(function ($maxHeap) {
        $maxHeap->insert(10);
        $maxHeap->insert(5);
        $maxHeap->insert(15);
        $maxHeap->insert(3);

        $maxHeap->extract();  // 提取最大值 15
    });
```

---

### 图结构

#### `Ds4viz::graphUndirected()`

```php
/**
 * 创建无向图实例
 *
 * @param string $label 无向图的标签，默认为 "graph"
 * @param string|null $output 输出文件路径，若为 null 则使用全局配置
 * @return GraphUndirected 无向图实例
 */
public static function graphUndirected(
    string $label = 'graph',
    ?string $output = null
): GraphUndirected;
```

**GraphUndirected 类方法：**

```php
/**
 * 无向图数据结构
 */
class GraphUndirected
{
    /**
     * 执行无向图操作
     *
     * @param callable(GraphUndirected): void $callback 回调函数
     * @return void
     */
    public function run(callable $callback): void;

    /**
     * 添加节点
     *
     * @param int $nodeId 节点 id
     * @param string $label 节点标签，长度限制为 1-32 字符
     * @return void
     * @throws StructureException 当节点已存在时抛出异常
     */
    public function addNode(int $nodeId, string $label): void;

    /**
     * 添加无向边，内部自动规范化为 fromId < toId
     *
     * @param int $fromId 起始节点 id
     * @param int $toId 终止节点 id
     * @return void
     * @throws StructureException 当节点不存在、边已存在或形成自环时抛出异常
     */
    public function addEdge(int $fromId, int $toId): void;

    /**
     * 删除节点及其相关的所有边
     *
     * @param int $nodeId 要删除的节点 id
     * @return void
     * @throws StructureException 当节点不存在时抛出异常
     */
    public function removeNode(int $nodeId): void;

    /**
     * 删除边
     *
     * @param int $fromId 起始节点 id
     * @param int $toId 终止节点 id
     * @return void
     * @throws StructureException 当边不存在时抛出异常
     */
    public function removeEdge(int $fromId, int $toId): void;

    /**
     * 更新节点标签
     *
     * @param int $nodeId 节点 id
     * @param string $label 新的节点标签，长度限制为 1-32 字符
     * @return void
     * @throws StructureException 当节点不存在时抛出异常
     */
    public function updateNodeLabel(int $nodeId, string $label): void;
}
```

**示例：**

```php
<?php

use Ds4viz\Ds4viz;

Ds4viz::graphUndirected(label: 'demo_graph')
    ->run(function ($graph) {
        $graph->addNode(0, 'A');
        $graph->addNode(1, 'B');
        $graph->addNode(2, 'C');

        $graph->addEdge(0, 1);
        $graph->addEdge(1, 2);

        $graph->updateNodeLabel(1, 'B_updated');
        $graph->removeEdge(0, 1);
    });
```

#### `Ds4viz::graphDirected()`

```php
/**
 * 创建有向图实例
 *
 * @param string $label 有向图的标签，默认为 "graph"
 * @param string|null $output 输出文件路径，若为 null 则使用全局配置
 * @return GraphDirected 有向图实例
 */
public static function graphDirected(
    string $label = 'graph',
    ?string $output = null
): GraphDirected;
```

**GraphDirected 类方法：**

```php
/**
 * 有向图数据结构
 */
class GraphDirected
{
    /**
     * 执行有向图操作
     *
     * @param callable(GraphDirected): void $callback 回调函数
     * @return void
     */
    public function run(callable $callback): void;

    /**
     * 添加节点
     *
     * @param int $nodeId 节点 id
     * @param string $label 节点标签，长度限制为 1-32 字符
     * @return void
     * @throws StructureException 当节点已存在时抛出异常
     */
    public function addNode(int $nodeId, string $label): void;

    /**
     * 添加有向边
     *
     * @param int $fromId 起始节点 id
     * @param int $toId 终止节点 id
     * @return void
     * @throws StructureException 当节点不存在、边已存在或形成自环时抛出异常
     */
    public function addEdge(int $fromId, int $toId): void;

    /**
     * 删除节点及其相关的所有边
     *
     * @param int $nodeId 要删除的节点 id
     * @return void
     * @throws StructureException 当节点不存在时抛出异常
     */
    public function removeNode(int $nodeId): void;

    /**
     * 删除边
     *
     * @param int $fromId 起始节点 id
     * @param int $toId 终止节点 id
     * @return void
     * @throws StructureException 当边不存在时抛出异常
     */
    public function removeEdge(int $fromId, int $toId): void;

    /**
     * 更新节点标签
     *
     * @param int $nodeId 节点 id
     * @param string $label 新的节点标签，长度限制为 1-32 字符
     * @return void
     * @throws StructureException 当节点不存在时抛出异常
     */
    public function updateNodeLabel(int $nodeId, string $label): void;
}
```

**示例：**

```php
<?php

use Ds4viz\Ds4viz;

Ds4viz::graphDirected(label: 'demo_digraph')
    ->run(function ($graph) {
        $graph->addNode(0, 'A');
        $graph->addNode(1, 'B');
        $graph->addNode(2, 'C');

        $graph->addEdge(0, 1);
        $graph->addEdge(1, 2);
        $graph->addEdge(2, 0);

        $graph->updateNodeLabel(1, 'B_updated');
        $graph->removeEdge(2, 0);
    });
```

#### `Ds4viz::graphWeighted()`

```php
/**
 * 创建带权图实例
 *
 * @param string $label 带权图的标签，默认为 "graph"
 * @param bool $directed 是否为有向图，默认为 false（无向图）
 * @param string|null $output 输出文件路径，若为 null 则使用全局配置
 * @return GraphWeighted 带权图实例
 */
public static function graphWeighted(
    string $label = 'graph',
    bool $directed = false,
    ?string $output = null
): GraphWeighted;
```

**GraphWeighted 类方法：**

```php
/**
 * 带权图数据结构
 */
class GraphWeighted
{
    /**
     * 执行带权图操作
     *
     * @param callable(GraphWeighted): void $callback 回调函数
     * @return void
     */
    public function run(callable $callback): void;

    /**
     * 添加节点
     *
     * @param int $nodeId 节点 id
     * @param string $label 节点标签，长度限制为 1-32 字符
     * @return void
     * @throws StructureException 当节点已存在时抛出异常
     */
    public function addNode(int $nodeId, string $label): void;

    /**
     * 添加带权边
     *
     * @param int $fromId 起始节点 id
     * @param int $toId 终止节点 id
     * @param float $weight 边的权重
     * @return void
     * @throws StructureException 当节点不存在、边已存在或形成自环时抛出异常
     */
    public function addEdge(int $fromId, int $toId, float $weight): void;

    /**
     * 删除节点及其相关的所有边
     *
     * @param int $nodeId 要删除的节点 id
     * @return void
     * @throws StructureException 当节点不存在时抛出异常
     */
    public function removeNode(int $nodeId): void;

    /**
     * 删除边
     *
     * @param int $fromId 起始节点 id
     * @param int $toId 终止节点 id
     * @return void
     * @throws StructureException 当边不存在时抛出异常
     */
    public function removeEdge(int $fromId, int $toId): void;

    /**
     * 更新边的权重
     *
     * @param int $fromId 起始节点 id
     * @param int $toId 终止节点 id
     * @param float $weight 新的权重值
     * @return void
     * @throws StructureException 当边不存在时抛出异常
     */
    public function updateWeight(int $fromId, int $toId, float $weight): void;

    /**
     * 更新节点标签
     *
     * @param int $nodeId 节点 id
     * @param string $label 新的节点标签，长度限制为 1-32 字符
     * @return void
     * @throws StructureException 当节点不存在时抛出异常
     */
    public function updateNodeLabel(int $nodeId, string $label): void;
}
```

**示例：**

```php
<?php

use Ds4viz\Ds4viz;

// 无向带权图
Ds4viz::graphWeighted(label: 'demo_weighted_graph', directed: false)
    ->run(function ($graph) {
        $graph->addNode(0, 'A');
        $graph->addNode(1, 'B');
        $graph->addNode(2, 'C');

        $graph->addEdge(0, 1, 3.5);
        $graph->addEdge(1, 2, 2.0);
        $graph->addEdge(2, 0, 4.2);

        $graph->updateWeight(0, 1, 5.0);
        $graph->removeEdge(2, 0);
    });

// 有向带权图
Ds4viz::graphWeighted(label: 'demo_directed_weighted', directed: true)
    ->run(function ($graph) {
        $graph->addNode(0, 'Start');
        $graph->addNode(1, 'Middle');
        $graph->addNode(2, 'End');

        $graph->addEdge(0, 1, 1.5);
        $graph->addEdge(1, 2, 2.5);
    });
```

---

### 异常处理

所有数据结构在遇到错误时都会抛出 `StructureException`，并在退出回调时自动生成包含 `[error]` 部分的 `.toml` 文件。

**示例：**

```php
<?php

use Ds4viz\Ds4viz;
use Ds4viz\Exception\StructureException;

try {
    Ds4viz::binaryTree()
        ->run(function ($tree) {
            $root = $tree->insertRoot(10);
            $tree->insertLeft(999, 5);  // 父节点不存在，抛出异常
        });
} catch (StructureException $e) {
    echo "发生错误: " . $e->getMessage() . "\n";
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

- `int`：整数
- `float`：浮点数
- `string`：字符串
- `bool`：布尔值

所有 API 在类型声明中都严格遵循此约束。
