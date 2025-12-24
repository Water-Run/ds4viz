<?php

declare(strict_types=1);

/**
 * 树形数据结构实现：二叉树、二叉搜索树、堆
 *
 * @author WaterRun
 * @file src/Structures/Tree.php
 * @package Ds4viz\Structures
 * @since 2025-12-24
 * @version 1.0.0
 */

namespace Ds4viz\Structures;

use Ds4viz\Ds4viz;
use Ds4viz\Exception\ErrorType;
use Ds4viz\Exception\StructureException;
use Ds4viz\Session;
use InvalidArgumentException;
use Throwable;

use function Ds4viz\getCallerLine;

/**
 * 二叉树数据结构
 *
 * @package Ds4viz\Structures
 * @author WaterRun
 * @since 2025-12-24
 */
final class BinaryTree
{
    /**
     * 会话对象
     *
     * @var Session
     */
    private Session $session;

    /**
     * 节点存储
     *
     * @var array<int, array{value: int|float|string|bool, left: int, right: int}>
     */
    private array $nodes = [];

    /**
     * 根节点 ID
     *
     * @var int
     */
    private int $root = -1;

    /**
     * 下一个节点 ID
     *
     * @var int
     */
    private int $nextId = 0;

    /**
     * 构造函数
     *
     * @param string $label 二叉树的标签
     * @param string|null $output 输出文件路径
     */
    public function __construct(
        private readonly string $label = 'binary_tree',
        ?string $output = null,
    ) {
        $outputPath = $output ?? Ds4viz::getGlobalConfig()->outputPath;
        $this->session = new Session(kind: 'binary_tree', label: $this->label, output: $outputPath);
    }

    /**
     * 执行二叉树操作
     *
     * @param callable(BinaryTree): void $callback 回调函数
     * @return void
     */
    public function run(callable $callback): void
    {
        $this->session->setEntryLine(getCallerLine(2));
        $this->session->addState($this->buildData());

        try {
            $callback($this);
            $this->session->setExitLine(getCallerLine(2));
        } catch (StructureException $e) {
            $this->session->setExitLine(getCallerLine(2));
            $this->session->setError(
                errorType: ErrorType::from($e->getErrorType()),
                message: $e->getMessage(),
                line: $e->getErrorLine(),
                stepId: $this->session->getFailedStepId(),
            );
            $this->session->write();
            throw $e;
        } catch (Throwable $e) {
            $this->session->setExitLine(getCallerLine(2));
            $this->session->setError(
                errorType: ErrorType::UNKNOWN,
                message: $e->getMessage(),
                line: $e->getLine(),
            );
            $this->session->write();
            throw $e;
        }

        $this->session->write();
    }

    /**
     * 插入根节点
     *
     * @param int|float|string|bool $value 根节点的值
     * @return int 根节点的 id
     * @throws StructureException 当根节点已存在时
     */
    public function insertRoot(int|float|string|bool $value): int
    {
        $line = getCallerLine(2);

        if ($this->root !== -1) {
            $this->session->setFailedStepId($this->session->getStepCounter());
            throw new StructureException('Root node already exists', 'runtime', $line);
        }

        $before = $this->session->getLastStateId();
        $newId = $this->nextId++;
        $this->nodes[$newId] = ['value' => $value, 'left' => -1, 'right' => -1];
        $this->root = $newId;
        $after = $this->session->addState($this->buildData());
        $this->session->addStep(
            op: 'insert_root',
            before: $before,
            after: $after,
            args: ['value' => $value],
            ret: $newId,
            line: $line,
        );
        return $newId;
    }

    /**
     * 在指定父节点的左侧插入子节点
     *
     * @param int $parentId 父节点的 id
     * @param int|float|string|bool $value 要插入的子节点值
     * @return int 新插入节点的 id
     * @throws StructureException 当父节点不存在或左子节点已存在时
     */
    public function insertLeft(int $parentId, int|float|string|bool $value): int
    {
        $line = getCallerLine(2);

        if (!isset($this->nodes[$parentId])) {
            $this->session->setFailedStepId($this->session->getStepCounter());
            throw new StructureException("Parent node not found: {$parentId}", 'runtime', $line);
        }

        if ($this->nodes[$parentId]['left'] !== -1) {
            $this->session->setFailedStepId($this->session->getStepCounter());
            throw new StructureException("Left child already exists for node: {$parentId}", 'runtime', $line);
        }

        $before = $this->session->getLastStateId();
        $newId = $this->nextId++;
        $this->nodes[$newId] = ['value' => $value, 'left' => -1, 'right' => -1];
        $this->nodes[$parentId]['left'] = $newId;
        $after = $this->session->addState($this->buildData());
        $this->session->addStep(
            op: 'insert_left',
            before: $before,
            after: $after,
            args: ['parent' => $parentId, 'value' => $value],
            ret: $newId,
            line: $line,
        );
        return $newId;
    }

    /**
     * 在指定父节点的右侧插入子节点
     *
     * @param int $parentId 父节点的 id
     * @param int|float|string|bool $value 要插入的子节点值
     * @return int 新插入节点的 id
     * @throws StructureException 当父节点不存在或右子节点已存在时
     */
    public function insertRight(int $parentId, int|float|string|bool $value): int
    {
        $line = getCallerLine(2);

        if (!isset($this->nodes[$parentId])) {
            $this->session->setFailedStepId($this->session->getStepCounter());
            throw new StructureException("Parent node not found: {$parentId}", 'runtime', $line);
        }

        if ($this->nodes[$parentId]['right'] !== -1) {
            $this->session->setFailedStepId($this->session->getStepCounter());
            throw new StructureException("Right child already exists for node: {$parentId}", 'runtime', $line);
        }

        $before = $this->session->getLastStateId();
        $newId = $this->nextId++;
        $this->nodes[$newId] = ['value' => $value, 'left' => -1, 'right' => -1];
        $this->nodes[$parentId]['right'] = $newId;
        $after = $this->session->addState($this->buildData());
        $this->session->addStep(
            op: 'insert_right',
            before: $before,
            after: $after,
            args: ['parent' => $parentId, 'value' => $value],
            ret: $newId,
            line: $line,
        );
        return $newId;
    }

    /**
     * 删除指定节点及其所有子树
     *
     * @param int $nodeId 要删除的节点 id
     * @return void
     * @throws StructureException 当节点不存在时
     */
    public function delete(int $nodeId): void
    {
        $line = getCallerLine(2);

        if (!isset($this->nodes[$nodeId])) {
            $this->session->setFailedStepId($this->session->getStepCounter());
            throw new StructureException("Node not found: {$nodeId}", 'runtime', $line);
        }

        $before = $this->session->getLastStateId();
        $subtreeIds = $this->collectSubtree($nodeId);
        $parentId = $this->findParent($nodeId);

        if ($parentId !== -1) {
            if ($this->nodes[$parentId]['left'] === $nodeId) {
                $this->nodes[$parentId]['left'] = -1;
            } else {
                $this->nodes[$parentId]['right'] = -1;
            }
        } else {
            $this->root = -1;
        }

        foreach ($subtreeIds as $nid) {
            unset($this->nodes[$nid]);
        }

        $after = $this->session->addState($this->buildData());
        $this->session->addStep(
            op: 'delete',
            before: $before,
            after: $after,
            args: ['node_id' => $nodeId],
            line: $line,
        );
    }

    /**
     * 更新节点的值
     *
     * @param int $nodeId 节点 id
     * @param int|float|string|bool $value 新的值
     * @return void
     * @throws StructureException 当节点不存在时
     */
    public function updateValue(int $nodeId, int|float|string|bool $value): void
    {
        $line = getCallerLine(2);

        if (!isset($this->nodes[$nodeId])) {
            $this->session->setFailedStepId($this->session->getStepCounter());
            throw new StructureException("Node not found: {$nodeId}", 'runtime', $line);
        }

        $before = $this->session->getLastStateId();
        $oldValue = $this->nodes[$nodeId]['value'];
        $this->nodes[$nodeId]['value'] = $value;
        $after = $this->session->addState($this->buildData());
        $this->session->addStep(
            op: 'update_value',
            before: $before,
            after: $after,
            args: ['node_id' => $nodeId, 'value' => $value],
            ret: $oldValue,
            line: $line,
        );
    }

    /**
     * 查找指定节点的父节点 ID
     *
     * @param int $nodeId 目标节点 ID
     * @return int 父节点 ID
     */
    private function findParent(int $nodeId): int
    {
        foreach ($this->nodes as $nid => $data) {
            if ($data['left'] === $nodeId || $data['right'] === $nodeId) {
                return $nid;
            }
        }
        return -1;
    }

    /**
     * 收集以指定节点为根的子树中所有节点 ID
     *
     * @param int $nodeId 根节点 ID
     * @return array<int, int> 子树中所有节点 ID 列表
     */
    private function collectSubtree(int $nodeId): array
    {
        if ($nodeId === -1) {
            return [];
        }
        $result = [$nodeId];
        $result = array_merge($result, $this->collectSubtree($this->nodes[$nodeId]['left']));
        $result = array_merge($result, $this->collectSubtree($this->nodes[$nodeId]['right']));
        return $result;
    }

    /**
     * 构建状态数据
     *
     * @return array<string, mixed> 状态数据
     */
    private function buildData(): array
    {
        $nodesList = [];
        ksort($this->nodes);
        foreach ($this->nodes as $nid => $data) {
            $nodesList[] = [
                'id' => $nid,
                'value' => $data['value'],
                'left' => $data['left'],
                'right' => $data['right'],
            ];
        }
        return ['root' => $this->root, 'nodes' => $nodesList];
    }
}

/**
 * 二叉搜索树数据结构
 *
 * @package Ds4viz\Structures
 * @author WaterRun
 * @since 2025-12-24
 */
final class BinarySearchTree
{
    /**
     * 会话对象
     *
     * @var Session
     */
    private Session $session;

    /**
     * 节点存储
     *
     * @var array<int, array{value: int|float, left: int, right: int}>
     */
    private array $nodes = [];

    /**
     * 根节点 ID
     *
     * @var int
     */
    private int $root = -1;

    /**
     * 下一个节点 ID
     *
     * @var int
     */
    private int $nextId = 0;

    /**
     * 构造函数
     *
     * @param string $label 二叉搜索树的标签
     * @param string|null $output 输出文件路径
     */
    public function __construct(
        private readonly string $label = 'bst',
        ?string $output = null,
    ) {
        $outputPath = $output ?? Ds4viz::getGlobalConfig()->outputPath;
        $this->session = new Session(kind: 'bst', label: $this->label, output: $outputPath);
    }

    /**
     * 执行 BST 操作
     *
     * @param callable(BinarySearchTree): void $callback 回调函数
     * @return void
     */
    public function run(callable $callback): void
    {
        $this->session->setEntryLine(getCallerLine(2));
        $this->session->addState($this->buildData());

        try {
            $callback($this);
            $this->session->setExitLine(getCallerLine(2));
        } catch (StructureException $e) {
            $this->session->setExitLine(getCallerLine(2));
            $this->session->setError(
                errorType: ErrorType::from($e->getErrorType()),
                message: $e->getMessage(),
                line: $e->getErrorLine(),
                stepId: $this->session->getFailedStepId(),
            );
            $this->session->write();
            throw $e;
        } catch (Throwable $e) {
            $this->session->setExitLine(getCallerLine(2));
            $this->session->setError(
                errorType: ErrorType::UNKNOWN,
                message: $e->getMessage(),
                line: $e->getLine(),
            );
            $this->session->write();
            throw $e;
        }

        $this->session->write();
    }

    /**
     * 插入节点
     *
     * @param int|float $value 要插入的值
     * @return int 新插入节点的 id
     */
    public function insert(int|float $value): int
    {
        $before = $this->session->getLastStateId();
        $line = getCallerLine(2);
        $newId = $this->nextId++;
        $this->nodes[$newId] = ['value' => $value, 'left' => -1, 'right' => -1];

        if ($this->root === -1) {
            $this->root = $newId;
        } else {
            $current = $this->root;
            while (true) {
                $currentValue = $this->nodes[$current]['value'];
                if ($value < $currentValue) {
                    if ($this->nodes[$current]['left'] === -1) {
                        $this->nodes[$current]['left'] = $newId;
                        break;
                    }
                    $current = $this->nodes[$current]['left'];
                } else {
                    if ($this->nodes[$current]['right'] === -1) {
                        $this->nodes[$current]['right'] = $newId;
                        break;
                    }
                    $current = $this->nodes[$current]['right'];
                }
            }
        }

        $after = $this->session->addState($this->buildData());
        $this->session->addStep(
            op: 'insert',
            before: $before,
            after: $after,
            args: ['value' => $value],
            ret: $newId,
            line: $line,
        );
        return $newId;
    }

    /**
     * 删除节点
     *
     * @param int|float $value 要删除的节点值
     * @return void
     * @throws StructureException 当节点不存在时
     */
    public function delete(int|float $value): void
    {
        $line = getCallerLine(2);
        $nodeId = $this->findNodeByValue($value);

        if ($nodeId === -1) {
            $this->session->setFailedStepId($this->session->getStepCounter());
            throw new StructureException("Node with value {$value} not found", 'runtime', $line);
        }

        $before = $this->session->getLastStateId();
        $this->deleteNode($nodeId);
        $after = $this->session->addState($this->buildData());
        $this->session->addStep(
            op: 'delete',
            before: $before,
            after: $after,
            args: ['value' => $value],
            line: $line,
        );
    }

    /**
     * 根据值查找节点 ID
     *
     * @param int|float $value 要查找的值
     * @return int 节点 ID
     */
    private function findNodeByValue(int|float $value): int
    {
        foreach ($this->nodes as $nid => $data) {
            if ($data['value'] === $value) {
                return $nid;
            }
        }
        return -1;
    }

    /**
     * 查找指定节点的父节点 ID
     *
     * @param int $nodeId 目标节点 ID
     * @return int 父节点 ID
     */
    private function findParent(int $nodeId): int
    {
        foreach ($this->nodes as $nid => $data) {
            if ($data['left'] === $nodeId || $data['right'] === $nodeId) {
                return $nid;
            }
        }
        return -1;
    }

    /**
     * 查找以指定节点为根的子树中的最小值节点
     *
     * @param int $nodeId 子树根节点 ID
     * @return int 最小值节点 ID
     */
    private function findMinNode(int $nodeId): int
    {
        $current = $nodeId;
        while ($this->nodes[$current]['left'] !== -1) {
            $current = $this->nodes[$current]['left'];
        }
        return $current;
    }

    /**
     * 删除指定节点并维护 BST 性质
     *
     * @param int $nodeId 要删除的节点 ID
     * @return void
     */
    private function deleteNode(int $nodeId): void
    {
        $node = $this->nodes[$nodeId];
        $parentId = $this->findParent($nodeId);
        $leftChild = $node['left'];
        $rightChild = $node['right'];

        if ($leftChild === -1 && $rightChild === -1) {
            $this->replaceInParent($parentId, $nodeId, -1);
            unset($this->nodes[$nodeId]);
        } elseif ($leftChild === -1) {
            $this->replaceInParent($parentId, $nodeId, $rightChild);
            unset($this->nodes[$nodeId]);
        } elseif ($rightChild === -1) {
            $this->replaceInParent($parentId, $nodeId, $leftChild);
            unset($this->nodes[$nodeId]);
        } else {
            $successorId = $this->findMinNode($rightChild);
            $successorValue = $this->nodes[$successorId]['value'];
            $this->deleteNode($successorId);
            $this->nodes[$nodeId]['value'] = $successorValue;
        }
    }

    /**
     * 在父节点中替换子节点引用
     *
     * @param int $parentId 父节点 ID
     * @param int $oldChild 旧子节点 ID
     * @param int $newChild 新子节点 ID
     * @return void
     */
    private function replaceInParent(int $parentId, int $oldChild, int $newChild): void
    {
        if ($parentId === -1) {
            $this->root = $newChild;
        } elseif ($this->nodes[$parentId]['left'] === $oldChild) {
            $this->nodes[$parentId]['left'] = $newChild;
        } else {
            $this->nodes[$parentId]['right'] = $newChild;
        }
    }

    /**
     * 构建状态数据
     *
     * @return array<string, mixed> 状态数据
     */
    private function buildData(): array
    {
        $nodesList = [];
        ksort($this->nodes);
        foreach ($this->nodes as $nid => $data) {
            $nodesList[] = [
                'id' => $nid,
                'value' => $data['value'],
                'left' => $data['left'],
                'right' => $data['right'],
            ];
        }
        return ['root' => $this->root, 'nodes' => $nodesList];
    }
}

/**
 * 堆数据结构（使用完全二叉树表示）
 *
 * @package Ds4viz\Structures
 * @author WaterRun
 * @since 2025-12-24
 */
final class Heap
{
    /**
     * 会话对象
     *
     * @var Session
     */
    private Session $session;

    /**
     * 堆元素
     *
     * @var array<int, int|float>
     */
    private array $items = [];

    /**
     * 构造函数
     *
     * @param string $label 堆的标签
     * @param string $heapType 堆类型
     * @param string|null $output 输出文件路径
     * @throws InvalidArgumentException 当 heapType 不是 "min" 或 "max" 时
     */
    public function __construct(
        private readonly string $label = 'heap',
        private readonly string $heapType = 'min',
        ?string $output = null,
    ) {
        if ($heapType !== 'min' && $heapType !== 'max') {
            throw new InvalidArgumentException("heap_type must be 'min' or 'max', got '{$heapType}'");
        }
        $outputPath = $output ?? Ds4viz::getGlobalConfig()->outputPath;
        $this->session = new Session(kind: 'binary_tree', label: $this->label, output: $outputPath);
    }

    /**
     * 执行堆操作
     *
     * @param callable(Heap): void $callback 回调函数
     * @return void
     */
    public function run(callable $callback): void
    {
        $this->session->setEntryLine(getCallerLine(2));
        $this->session->addState($this->buildData());

        try {
            $callback($this);
            $this->session->setExitLine(getCallerLine(2));
        } catch (StructureException $e) {
            $this->session->setExitLine(getCallerLine(2));
            $this->session->setError(
                errorType: ErrorType::from($e->getErrorType()),
                message: $e->getMessage(),
                line: $e->getErrorLine(),
                stepId: $this->session->getFailedStepId(),
            );
            $this->session->write();
            throw $e;
        } catch (Throwable $e) {
            $this->session->setExitLine(getCallerLine(2));
            $this->session->setError(
                errorType: ErrorType::UNKNOWN,
                message: $e->getMessage(),
                line: $e->getLine(),
            );
            $this->session->write();
            throw $e;
        }

        $this->session->write();
    }

    /**
     * 插入元素
     *
     * @param int|float $value 要插入的值
     * @return void
     */
    public function insert(int|float $value): void
    {
        $before = $this->session->getLastStateId();
        $line = getCallerLine(2);
        $this->items[] = $value;
        $this->siftUp(count($this->items) - 1);
        $after = $this->session->addState($this->buildData());
        $this->session->addStep(
            op: 'insert',
            before: $before,
            after: $after,
            args: ['value' => $value],
            line: $line,
        );
    }

    /**
     * 提取堆顶元素
     *
     * @return void
     * @throws StructureException 当堆为空时
     */
    public function extract(): void
    {
        $line = getCallerLine(2);

        if (count($this->items) === 0) {
            $this->session->setFailedStepId($this->session->getStepCounter());
            throw new StructureException('Cannot extract from empty heap', 'runtime', $line);
        }

        $before = $this->session->getLastStateId();
        $extracted = $this->items[0];

        if (count($this->items) === 1) {
            array_pop($this->items);
        } else {
            $this->items[0] = array_pop($this->items);
            $this->siftDown(0);
        }

        $after = $this->session->addState($this->buildData());
        $this->session->addStep(
            op: 'extract',
            before: $before,
            after: $after,
            args: [],
            ret: $extracted,
            line: $line,
        );
    }

    /**
     * 清空堆
     *
     * @return void
     */
    public function clear(): void
    {
        $before = $this->session->getLastStateId();
        $line = getCallerLine(2);
        $this->items = [];
        $after = $this->session->addState($this->buildData());
        $this->session->addStep(
            op: 'clear',
            before: $before,
            after: $after,
            args: [],
            line: $line,
        );
    }

    /**
     * 比较两个值
     *
     * @param int|float $a 第一个值
     * @param int|float $b 第二个值
     * @return bool 如果 a 应该在 b 之前则返回 true
     */
    private function compare(int|float $a, int|float $b): bool
    {
        return $this->heapType === 'min' ? $a < $b : $a > $b;
    }

    /**
     * 向上调整堆
     *
     * @param int $index 开始调整的索引
     * @return void
     */
    private function siftUp(int $index): void
    {
        while ($index > 0) {
            $parent = intdiv($index - 1, 2);
            if ($this->compare($this->items[$index], $this->items[$parent])) {
                $temp = $this->items[$index];
                $this->items[$index] = $this->items[$parent];
                $this->items[$parent] = $temp;
                $index = $parent;
            } else {
                break;
            }
        }
    }

    /**
     * 向下调整堆
     *
     * @param int $index 开始调整的索引
     * @return void
     */
    private function siftDown(int $index): void
    {
        $size = count($this->items);
        while (true) {
            $target = $index;
            $left = 2 * $index + 1;
            $right = 2 * $index + 2;

            if ($left < $size && $this->compare($this->items[$left], $this->items[$target])) {
                $target = $left;
            }
            if ($right < $size && $this->compare($this->items[$right], $this->items[$target])) {
                $target = $right;
            }
            if ($target === $index) {
                break;
            }
            $temp = $this->items[$index];
            $this->items[$index] = $this->items[$target];
            $this->items[$target] = $temp;
            $index = $target;
        }
    }

    /**
     * 构建状态数据
     *
     * @return array<string, mixed> 状态数据
     */
    private function buildData(): array
    {
        if (count($this->items) === 0) {
            return ['root' => -1, 'nodes' => []];
        }

        $nodesList = [];
        foreach ($this->items as $i => $value) {
            $left = 2 * $i + 1 < count($this->items) ? 2 * $i + 1 : -1;
            $right = 2 * $i + 2 < count($this->items) ? 2 * $i + 2 : -1;
            $nodesList[] = ['id' => $i, 'value' => $value, 'left' => $left, 'right' => $right];
        }

        return ['root' => 0, 'nodes' => $nodesList];
    }
}
