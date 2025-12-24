<?php

declare(strict_types=1);

/**
 * 线性数据结构实现：栈、队列、单链表、双向链表
 *
 * @author WaterRun
 * @file src/Structures/Linear.php
 * @package Ds4viz\Structures
 * @since 2025-12-24
 * @version 1.0.0
 */

namespace Ds4viz\Structures;

use Ds4viz\Ds4viz;
use Ds4viz\Exception\ErrorType;
use Ds4viz\Exception\StructureException;
use Ds4viz\Session;
use Throwable;

use function Ds4viz\getCallerLine;

/**
 * 栈数据结构
 *
 * @package Ds4viz\Structures
 * @author WaterRun
 * @since 2025-12-24
 */
final class Stack
{
    /**
     * 会话对象
     *
     * @var Session
     */
    private Session $session;

    /**
     * 栈元素
     *
     * @var array<int, int|float|string|bool>
     */
    private array $items = [];

    /**
     * 构造函数
     *
     * @param string $label 栈的标签
     * @param string|null $output 输出文件路径
     */
    public function __construct(
        private readonly string $label = 'stack',
        ?string $output = null,
    ) {
        $outputPath = $output ?? Ds4viz::getGlobalConfig()->outputPath;
        $this->session = new Session(kind: 'stack', label: $this->label, output: $outputPath);
    }

    /**
     * 执行栈操作
     *
     * @param callable(Stack): void $callback 回调函数
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
     * 压栈操作
     *
     * @param int|float|string|bool $value 要压入的值
     * @return void
     */
    public function push(int|float|string|bool $value): void
    {
        $before = $this->session->getLastStateId();
        $line = getCallerLine(2);
        $this->items[] = $value;
        $after = $this->session->addState($this->buildData());
        $this->session->addStep(
            op: 'push',
            before: $before,
            after: $after,
            args: ['value' => $value],
            line: $line,
        );
    }

    /**
     * 弹栈操作
     *
     * @return void
     * @throws StructureException 当栈为空时
     */
    public function pop(): void
    {
        $line = getCallerLine(2);

        if (count($this->items) === 0) {
            $this->session->setFailedStepId($this->session->getStepCounter());
            throw new StructureException('Cannot pop from empty stack', 'runtime', $line);
        }

        $before = $this->session->getLastStateId();
        $popped = array_pop($this->items);
        $after = $this->session->addState($this->buildData());
        $this->session->addStep(
            op: 'pop',
            before: $before,
            after: $after,
            args: [],
            ret: $popped,
            line: $line,
        );
    }

    /**
     * 清空栈
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
     * 构建状态数据
     *
     * @return array<string, mixed> 状态数据
     */
    private function buildData(): array
    {
        return [
            'items' => array_values($this->items),
            'top' => count($this->items) > 0 ? count($this->items) - 1 : -1,
        ];
    }
}

/**
 * 队列数据结构
 *
 * @package Ds4viz\Structures
 * @author WaterRun
 * @since 2025-12-24
 */
final class Queue
{
    /**
     * 会话对象
     *
     * @var Session
     */
    private Session $session;

    /**
     * 队列元素
     *
     * @var array<int, int|float|string|bool>
     */
    private array $items = [];

    /**
     * 构造函数
     *
     * @param string $label 队列的标签
     * @param string|null $output 输出文件路径
     */
    public function __construct(
        private readonly string $label = 'queue',
        ?string $output = null,
    ) {
        $outputPath = $output ?? Ds4viz::getGlobalConfig()->outputPath;
        $this->session = new Session(kind: 'queue', label: $this->label, output: $outputPath);
    }

    /**
     * 执行队列操作
     *
     * @param callable(Queue): void $callback 回调函数
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
     * 入队操作
     *
     * @param int|float|string|bool $value 要入队的值
     * @return void
     */
    public function enqueue(int|float|string|bool $value): void
    {
        $before = $this->session->getLastStateId();
        $line = getCallerLine(2);
        $this->items[] = $value;
        $after = $this->session->addState($this->buildData());
        $this->session->addStep(
            op: 'enqueue',
            before: $before,
            after: $after,
            args: ['value' => $value],
            line: $line,
        );
    }

    /**
     * 出队操作
     *
     * @return void
     * @throws StructureException 当队列为空时
     */
    public function dequeue(): void
    {
        $line = getCallerLine(2);

        if (count($this->items) === 0) {
            $this->session->setFailedStepId($this->session->getStepCounter());
            throw new StructureException('Cannot dequeue from empty queue', 'runtime', $line);
        }

        $before = $this->session->getLastStateId();
        $dequeued = array_shift($this->items);
        $this->items = array_values($this->items);
        $after = $this->session->addState($this->buildData());
        $this->session->addStep(
            op: 'dequeue',
            before: $before,
            after: $after,
            args: [],
            ret: $dequeued,
            line: $line,
        );
    }

    /**
     * 清空队列
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
     * 构建状态数据
     *
     * @return array<string, mixed> 状态数据
     */
    private function buildData(): array
    {
        $count = count($this->items);
        return [
            'items' => array_values($this->items),
            'front' => $count > 0 ? 0 : -1,
            'rear' => $count > 0 ? $count - 1 : -1,
        ];
    }
}

/**
 * 单链表数据结构
 *
 * @package Ds4viz\Structures
 * @author WaterRun
 * @since 2025-12-24
 */
final class SingleLinkedList
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
     * @var array<int, array{value: int|float|string|bool, next: int}>
     */
    private array $nodes = [];

    /**
     * 头节点 ID
     *
     * @var int
     */
    private int $head = -1;

    /**
     * 下一个节点 ID
     *
     * @var int
     */
    private int $nextId = 0;

    /**
     * 构造函数
     *
     * @param string $label 单链表的标签
     * @param string|null $output 输出文件路径
     */
    public function __construct(
        private readonly string $label = 'slist',
        ?string $output = null,
    ) {
        $outputPath = $output ?? Ds4viz::getGlobalConfig()->outputPath;
        $this->session = new Session(kind: 'slist', label: $this->label, output: $outputPath);
    }

    /**
     * 执行链表操作
     *
     * @param callable(SingleLinkedList): void $callback 回调函数
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
     * 在链表头部插入节点
     *
     * @param int|float|string|bool $value 要插入的值
     * @return int 新插入节点的 id
     */
    public function insertHead(int|float|string|bool $value): int
    {
        $before = $this->session->getLastStateId();
        $line = getCallerLine(2);
        $newId = $this->nextId++;
        $this->nodes[$newId] = ['value' => $value, 'next' => $this->head];
        $this->head = $newId;
        $after = $this->session->addState($this->buildData());
        $this->session->addStep(
            op: 'insert_head',
            before: $before,
            after: $after,
            args: ['value' => $value],
            ret: $newId,
            line: $line,
        );
        return $newId;
    }

    /**
     * 在链表尾部插入节点
     *
     * @param int|float|string|bool $value 要插入的值
     * @return int 新插入节点的 id
     */
    public function insertTail(int|float|string|bool $value): int
    {
        $before = $this->session->getLastStateId();
        $line = getCallerLine(2);
        $newId = $this->nextId++;
        $this->nodes[$newId] = ['value' => $value, 'next' => -1];

        if ($this->head === -1) {
            $this->head = $newId;
        } else {
            $tail = $this->findTail();
            $this->nodes[$tail]['next'] = $newId;
        }

        $after = $this->session->addState($this->buildData());
        $this->session->addStep(
            op: 'insert_tail',
            before: $before,
            after: $after,
            args: ['value' => $value],
            ret: $newId,
            line: $line,
        );
        return $newId;
    }

    /**
     * 在指定节点后插入新节点
     *
     * @param int $nodeId 目标节点的 id
     * @param int|float|string|bool $value 要插入的值
     * @return int 新插入节点的 id
     * @throws StructureException 当指定节点不存在时
     */
    public function insertAfter(int $nodeId, int|float|string|bool $value): int
    {
        $line = getCallerLine(2);

        if (!isset($this->nodes[$nodeId])) {
            $this->session->setFailedStepId($this->session->getStepCounter());
            throw new StructureException("Node not found: {$nodeId}", 'runtime', $line);
        }

        $before = $this->session->getLastStateId();
        $newId = $this->nextId++;
        $oldNext = $this->nodes[$nodeId]['next'];
        $this->nodes[$newId] = ['value' => $value, 'next' => $oldNext];
        $this->nodes[$nodeId]['next'] = $newId;
        $after = $this->session->addState($this->buildData());
        $this->session->addStep(
            op: 'insert_after',
            before: $before,
            after: $after,
            args: ['node_id' => $nodeId, 'value' => $value],
            ret: $newId,
            line: $line,
        );
        return $newId;
    }

    /**
     * 删除指定节点
     *
     * @param int $nodeId 要删除的节点 id
     * @return void
     * @throws StructureException 当指定节点不存在时
     */
    public function delete(int $nodeId): void
    {
        $line = getCallerLine(2);

        if (!isset($this->nodes[$nodeId])) {
            $this->session->setFailedStepId($this->session->getStepCounter());
            throw new StructureException("Node not found: {$nodeId}", 'runtime', $line);
        }

        $before = $this->session->getLastStateId();
        $deletedValue = $this->nodes[$nodeId]['value'];

        if ($this->head === $nodeId) {
            $this->head = $this->nodes[$nodeId]['next'];
        } else {
            $prev = $this->findPrev($nodeId);
            if ($prev !== -1) {
                $this->nodes[$prev]['next'] = $this->nodes[$nodeId]['next'];
            }
        }
        unset($this->nodes[$nodeId]);

        $after = $this->session->addState($this->buildData());
        $this->session->addStep(
            op: 'delete',
            before: $before,
            after: $after,
            args: ['node_id' => $nodeId],
            ret: $deletedValue,
            line: $line,
        );
    }

    /**
     * 删除头节点
     *
     * @return void
     * @throws StructureException 当链表为空时
     */
    public function deleteHead(): void
    {
        $line = getCallerLine(2);

        if ($this->head === -1) {
            $this->session->setFailedStepId($this->session->getStepCounter());
            throw new StructureException('Cannot delete from empty list', 'runtime', $line);
        }

        $before = $this->session->getLastStateId();
        $oldHead = $this->head;
        $deletedValue = $this->nodes[$oldHead]['value'];
        $this->head = $this->nodes[$oldHead]['next'];
        unset($this->nodes[$oldHead]);

        $after = $this->session->addState($this->buildData());
        $this->session->addStep(
            op: 'delete_head',
            before: $before,
            after: $after,
            args: [],
            ret: $deletedValue,
            line: $line,
        );
    }

    /**
     * 反转链表
     *
     * @return void
     */
    public function reverse(): void
    {
        $before = $this->session->getLastStateId();
        $line = getCallerLine(2);

        $prev = -1;
        $current = $this->head;
        while ($current !== -1) {
            $nextNode = $this->nodes[$current]['next'];
            $this->nodes[$current]['next'] = $prev;
            $prev = $current;
            $current = $nextNode;
        }
        $this->head = $prev;

        $after = $this->session->addState($this->buildData());
        $this->session->addStep(
            op: 'reverse',
            before: $before,
            after: $after,
            args: [],
            line: $line,
        );
    }

    /**
     * 查找尾节点 ID
     *
     * @return int 尾节点 ID
     */
    private function findTail(): int
    {
        if ($this->head === -1) {
            return -1;
        }
        $current = $this->head;
        while ($this->nodes[$current]['next'] !== -1) {
            $current = $this->nodes[$current]['next'];
        }
        return $current;
    }

    /**
     * 查找指定节点的前驱节点 ID
     *
     * @param int $nodeId 目标节点 ID
     * @return int 前驱节点 ID
     */
    private function findPrev(int $nodeId): int
    {
        if ($this->head === -1 || $this->head === $nodeId) {
            return -1;
        }
        $current = $this->head;
        while ($current !== -1) {
            if ($this->nodes[$current]['next'] === $nodeId) {
                return $current;
            }
            $current = $this->nodes[$current]['next'];
        }
        return -1;
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
            $nodesList[] = ['id' => $nid, 'value' => $data['value'], 'next' => $data['next']];
        }
        return ['head' => $this->head, 'nodes' => $nodesList];
    }
}

/**
 * 双向链表数据结构
 *
 * @package Ds4viz\Structures
 * @author WaterRun
 * @since 2025-12-24
 */
final class DoubleLinkedList
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
     * @var array<int, array{value: int|float|string|bool, prev: int, next: int}>
     */
    private array $nodes = [];

    /**
     * 头节点 ID
     *
     * @var int
     */
    private int $head = -1;

    /**
     * 尾节点 ID
     *
     * @var int
     */
    private int $tail = -1;

    /**
     * 下一个节点 ID
     *
     * @var int
     */
    private int $nextId = 0;

    /**
     * 构造函数
     *
     * @param string $label 双向链表的标签
     * @param string|null $output 输出文件路径
     */
    public function __construct(
        private readonly string $label = 'dlist',
        ?string $output = null,
    ) {
        $outputPath = $output ?? Ds4viz::getGlobalConfig()->outputPath;
        $this->session = new Session(kind: 'dlist', label: $this->label, output: $outputPath);
    }

    /**
     * 执行链表操作
     *
     * @param callable(DoubleLinkedList): void $callback 回调函数
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
     * 在链表头部插入节点
     *
     * @param int|float|string|bool $value 要插入的值
     * @return int 新插入节点的 id
     */
    public function insertHead(int|float|string|bool $value): int
    {
        $before = $this->session->getLastStateId();
        $line = getCallerLine(2);
        $newId = $this->nextId++;
        $this->nodes[$newId] = ['value' => $value, 'prev' => -1, 'next' => $this->head];

        if ($this->head !== -1) {
            $this->nodes[$this->head]['prev'] = $newId;
        }
        $this->head = $newId;
        if ($this->tail === -1) {
            $this->tail = $newId;
        }

        $after = $this->session->addState($this->buildData());
        $this->session->addStep(
            op: 'insert_head',
            before: $before,
            after: $after,
            args: ['value' => $value],
            ret: $newId,
            line: $line,
        );
        return $newId;
    }

    /**
     * 在链表尾部插入节点
     *
     * @param int|float|string|bool $value 要插入的值
     * @return int 新插入节点的 id
     */
    public function insertTail(int|float|string|bool $value): int
    {
        $before = $this->session->getLastStateId();
        $line = getCallerLine(2);
        $newId = $this->nextId++;
        $this->nodes[$newId] = ['value' => $value, 'prev' => $this->tail, 'next' => -1];

        if ($this->tail !== -1) {
            $this->nodes[$this->tail]['next'] = $newId;
        }
        $this->tail = $newId;
        if ($this->head === -1) {
            $this->head = $newId;
        }

        $after = $this->session->addState($this->buildData());
        $this->session->addStep(
            op: 'insert_tail',
            before: $before,
            after: $after,
            args: ['value' => $value],
            ret: $newId,
            line: $line,
        );
        return $newId;
    }

    /**
     * 在指定节点前插入新节点
     *
     * @param int $nodeId 目标节点的 id
     * @param int|float|string|bool $value 要插入的值
     * @return int 新插入节点的 id
     * @throws StructureException 当指定节点不存在时
     */
    public function insertBefore(int $nodeId, int|float|string|bool $value): int
    {
        $line = getCallerLine(2);

        if (!isset($this->nodes[$nodeId])) {
            $this->session->setFailedStepId($this->session->getStepCounter());
            throw new StructureException("Node not found: {$nodeId}", 'runtime', $line);
        }

        $before = $this->session->getLastStateId();
        $newId = $this->nextId++;
        $oldPrev = $this->nodes[$nodeId]['prev'];
        $this->nodes[$newId] = ['value' => $value, 'prev' => $oldPrev, 'next' => $nodeId];
        $this->nodes[$nodeId]['prev'] = $newId;

        if ($oldPrev !== -1) {
            $this->nodes[$oldPrev]['next'] = $newId;
        } else {
            $this->head = $newId;
        }

        $after = $this->session->addState($this->buildData());
        $this->session->addStep(
            op: 'insert_before',
            before: $before,
            after: $after,
            args: ['node_id' => $nodeId, 'value' => $value],
            ret: $newId,
            line: $line,
        );
        return $newId;
    }

    /**
     * 在指定节点后插入新节点
     *
     * @param int $nodeId 目标节点的 id
     * @param int|float|string|bool $value 要插入的值
     * @return int 新插入节点的 id
     * @throws StructureException 当指定节点不存在时
     */
    public function insertAfter(int $nodeId, int|float|string|bool $value): int
    {
        $line = getCallerLine(2);

        if (!isset($this->nodes[$nodeId])) {
            $this->session->setFailedStepId($this->session->getStepCounter());
            throw new StructureException("Node not found: {$nodeId}", 'runtime', $line);
        }

        $before = $this->session->getLastStateId();
        $newId = $this->nextId++;
        $oldNext = $this->nodes[$nodeId]['next'];
        $this->nodes[$newId] = ['value' => $value, 'prev' => $nodeId, 'next' => $oldNext];
        $this->nodes[$nodeId]['next'] = $newId;

        if ($oldNext !== -1) {
            $this->nodes[$oldNext]['prev'] = $newId;
        } else {
            $this->tail = $newId;
        }

        $after = $this->session->addState($this->buildData());
        $this->session->addStep(
            op: 'insert_after',
            before: $before,
            after: $after,
            args: ['node_id' => $nodeId, 'value' => $value],
            ret: $newId,
            line: $line,
        );
        return $newId;
    }

    /**
     * 删除指定节点
     *
     * @param int $nodeId 要删除的节点 id
     * @return void
     * @throws StructureException 当指定节点不存在时
     */
    public function delete(int $nodeId): void
    {
        $line = getCallerLine(2);

        if (!isset($this->nodes[$nodeId])) {
            $this->session->setFailedStepId($this->session->getStepCounter());
            throw new StructureException("Node not found: {$nodeId}", 'runtime', $line);
        }

        $before = $this->session->getLastStateId();
        $deletedValue = $this->nodes[$nodeId]['value'];
        $prevId = $this->nodes[$nodeId]['prev'];
        $nextId = $this->nodes[$nodeId]['next'];

        if ($prevId !== -1) {
            $this->nodes[$prevId]['next'] = $nextId;
        } else {
            $this->head = $nextId;
        }

        if ($nextId !== -1) {
            $this->nodes[$nextId]['prev'] = $prevId;
        } else {
            $this->tail = $prevId;
        }

        unset($this->nodes[$nodeId]);

        $after = $this->session->addState($this->buildData());
        $this->session->addStep(
            op: 'delete',
            before: $before,
            after: $after,
            args: ['node_id' => $nodeId],
            ret: $deletedValue,
            line: $line,
        );
    }

    /**
     * 删除头节点
     *
     * @return void
     * @throws StructureException 当链表为空时
     */
    public function deleteHead(): void
    {
        $line = getCallerLine(2);

        if ($this->head === -1) {
            $this->session->setFailedStepId($this->session->getStepCounter());
            throw new StructureException('Cannot delete from empty list', 'runtime', $line);
        }

        $this->delete($this->head);
    }

    /**
     * 删除尾节点
     *
     * @return void
     * @throws StructureException 当链表为空时
     */
    public function deleteTail(): void
    {
        $line = getCallerLine(2);

        if ($this->tail === -1) {
            $this->session->setFailedStepId($this->session->getStepCounter());
            throw new StructureException('Cannot delete from empty list', 'runtime', $line);
        }

        $this->delete($this->tail);
    }

    /**
     * 反转链表
     *
     * @return void
     */
    public function reverse(): void
    {
        $before = $this->session->getLastStateId();
        $line = getCallerLine(2);

        $current = $this->head;
        while ($current !== -1) {
            $node = &$this->nodes[$current];
            $temp = $node['prev'];
            $node['prev'] = $node['next'];
            $node['next'] = $temp;
            $current = $node['prev'];
        }
        $temp = $this->head;
        $this->head = $this->tail;
        $this->tail = $temp;

        $after = $this->session->addState($this->buildData());
        $this->session->addStep(
            op: 'reverse',
            before: $before,
            after: $after,
            args: [],
            line: $line,
        );
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
                'prev' => $data['prev'],
                'next' => $data['next'],
            ];
        }
        return ['head' => $this->head, 'tail' => $this->tail, 'nodes' => $nodesList];
    }
}
