<?php

declare(strict_types=1);

/**
 * 线性数据结构测试
 *
 * @author WaterRun
 * @file test/LinearTest.php
 * @package Ds4viz\Test
 * @since 2025-12-24
 * @version 1.0.0
 */

namespace Ds4viz\Test;

use Ds4viz\Ds4viz;
use Ds4viz\Exception\StructureException;
use Ds4viz\Structures\Stack;
use Ds4viz\Structures\Queue;
use Ds4viz\Structures\SingleLinkedList;
use Ds4viz\Structures\DoubleLinkedList;
use PHPUnit\Framework\TestCase;

/**
 * 线性数据结构测试类
 *
 * @package Ds4viz\Test
 * @author WaterRun
 * @since 2025-12-24
 */
final class LinearTest extends TestCase
{
    /**
     * 临时文件路径
     *
     * @var string
     */
    private string $tempFile;

    /**
     * 设置测试环境
     *
     * @return void
     */
    protected function setUp(): void
    {
        $this->tempFile = sys_get_temp_dir() . '/ds4viz_linear_test_' . uniqid() . '.toml';
        Ds4viz::config();
    }

    /**
     * 清理测试环境
     *
     * @return void
     */
    protected function tearDown(): void
    {
        if (file_exists($this->tempFile)) {
            unlink($this->tempFile);
        }
        Ds4viz::config();
    }

    /**
     * 测试栈基本操作
     *
     * @return void
     */
    public function testStackBasicOperations(): void
    {
        Ds4viz::stack(label: 'test_stack', output: $this->tempFile)
            ->run(function (Stack $s) {
                $s->push(10);
                $s->push(20);
                $s->push(30);
                $s->pop();
            });

        $this->assertFileExists($this->tempFile);
        $content = file_get_contents($this->tempFile);
        $this->assertStringContainsString('kind = "stack"', $content);
        $this->assertStringContainsString('[result]', $content);
    }

    /**
     * 测试空栈弹出错误
     *
     * @return void
     */
    public function testStackEmptyPopError(): void
    {
        $this->expectException(StructureException::class);

        Ds4viz::stack(output: $this->tempFile)
            ->run(function (Stack $s) {
                $s->pop();
            });
    }

    /**
     * 测试栈清空
     *
     * @return void
     */
    public function testStackClear(): void
    {
        Ds4viz::stack(output: $this->tempFile)
            ->run(function (Stack $s) {
                $s->push(1);
                $s->push(2);
                $s->push(3);
                $s->clear();
            });

        $content = file_get_contents($this->tempFile);
        $this->assertStringContainsString('items = []', $content);
    }

    /**
     * 测试栈支持各种值类型
     *
     * @return void
     */
    public function testStackVariousValueTypes(): void
    {
        Ds4viz::stack(output: $this->tempFile)
            ->run(function (Stack $s) {
                $s->push(42);
                $s->push(3.14);
                $s->push('hello');
                $s->push(true);
                $s->push(false);
            });

        $content = file_get_contents($this->tempFile);
        $this->assertStringContainsString('[result]', $content);
    }

    /**
     * 测试栈处理负数
     *
     * @return void
     */
    public function testStackNegativeNumbers(): void
    {
        Ds4viz::stack(output: $this->tempFile)
            ->run(function (Stack $s) {
                $s->push(-10);
                $s->push(-20);
                $s->pop();
            });

        $content = file_get_contents($this->tempFile);
        $this->assertStringContainsString('[result]', $content);
    }

    /**
     * 测试队列基本操作
     *
     * @return void
     */
    public function testQueueBasicOperations(): void
    {
        Ds4viz::queue(label: 'test_queue', output: $this->tempFile)
            ->run(function (Queue $q) {
                $q->enqueue(10);
                $q->enqueue(20);
                $q->dequeue();
                $q->enqueue(30);
            });

        $this->assertFileExists($this->tempFile);
        $content = file_get_contents($this->tempFile);
        $this->assertStringContainsString('kind = "queue"', $content);
        $this->assertStringContainsString('[result]', $content);
    }

    /**
     * 测试空队列出队错误
     *
     * @return void
     */
    public function testQueueEmptyDequeueError(): void
    {
        $this->expectException(StructureException::class);

        Ds4viz::queue(output: $this->tempFile)
            ->run(function (Queue $q) {
                $q->dequeue();
            });
    }

    /**
     * 测试队列清空
     *
     * @return void
     */
    public function testQueueClear(): void
    {
        Ds4viz::queue(output: $this->tempFile)
            ->run(function (Queue $q) {
                $q->enqueue(1);
                $q->enqueue(2);
                $q->enqueue(3);
                $q->clear();
            });

        $content = file_get_contents($this->tempFile);
        $this->assertStringContainsString('items = []', $content);
    }

    /**
     * 测试队列交替操作
     *
     * @return void
     */
    public function testQueueAlternatingOperations(): void
    {
        Ds4viz::queue(output: $this->tempFile)
            ->run(function (Queue $q) {
                $q->enqueue(1);
                $q->enqueue(2);
                $q->dequeue();
                $q->enqueue(3);
                $q->dequeue();
                $q->enqueue(4);
            });

        $content = file_get_contents($this->tempFile);
        $this->assertStringContainsString('[result]', $content);
    }

    /**
     * 测试单链表基本操作
     *
     * @return void
     */
    public function testSingleLinkedListBasicOperations(): void
    {
        Ds4viz::singleLinkedList(label: 'test_slist', output: $this->tempFile)
            ->run(function (SingleLinkedList $slist) {
                $node1 = $slist->insertHead(10);
                $slist->insertTail(20);
                $slist->insertAfter($node1, 15);
                $slist->deleteHead();
            });

        $content = file_get_contents($this->tempFile);
        $this->assertStringContainsString('kind = "slist"', $content);
        $this->assertStringContainsString('[result]', $content);
    }

    /**
     * 测试单链表反转
     *
     * @return void
     */
    public function testSingleLinkedListReverse(): void
    {
        Ds4viz::singleLinkedList(output: $this->tempFile)
            ->run(function (SingleLinkedList $slist) {
                $slist->insertTail(1);
                $slist->insertTail(2);
                $slist->insertTail(3);
                $slist->reverse();
            });

        $content = file_get_contents($this->tempFile);
        $this->assertStringContainsString('[result]', $content);
    }

    /**
     * 测试删除不存在的节点错误
     *
     * @return void
     */
    public function testSingleLinkedListDeleteNonexistentNodeError(): void
    {
        $this->expectException(StructureException::class);

        Ds4viz::singleLinkedList(output: $this->tempFile)
            ->run(function (SingleLinkedList $slist) {
                $slist->insertHead(10);
                $slist->delete(999);
            });
    }

    /**
     * 测试空链表删除头节点错误
     *
     * @return void
     */
    public function testSingleLinkedListEmptyDeleteHeadError(): void
    {
        $this->expectException(StructureException::class);

        Ds4viz::singleLinkedList(output: $this->tempFile)
            ->run(function (SingleLinkedList $slist) {
                $slist->deleteHead();
            });
    }

    /**
     * 测试在不存在的节点后插入错误
     *
     * @return void
     */
    public function testSingleLinkedListInsertAfterNonexistentError(): void
    {
        $this->expectException(StructureException::class);

        Ds4viz::singleLinkedList(output: $this->tempFile)
            ->run(function (SingleLinkedList $slist) {
                $slist->insertHead(10);
                $slist->insertAfter(999, 20);
            });
    }

    /**
     * 测试反转空链表
     *
     * @return void
     */
    public function testSingleLinkedListReverseEmpty(): void
    {
        Ds4viz::singleLinkedList(output: $this->tempFile)
            ->run(function (SingleLinkedList $slist) {
                $slist->reverse();
            });

        $content = file_get_contents($this->tempFile);
        $this->assertStringContainsString('[result]', $content);
    }

    /**
     * 测试双向链表基本操作
     *
     * @return void
     */
    public function testDoubleLinkedListBasicOperations(): void
    {
        Ds4viz::doubleLinkedList(label: 'test_dlist', output: $this->tempFile)
            ->run(function (DoubleLinkedList $dlist) {
                $node1 = $dlist->insertHead(10);
                $node2 = $dlist->insertTail(30);
                $dlist->insertBefore($node2, 20);
                $dlist->insertAfter($node1, 15);
                $dlist->deleteTail();
            });

        $content = file_get_contents($this->tempFile);
        $this->assertStringContainsString('kind = "dlist"', $content);
        $this->assertStringContainsString('[result]', $content);
    }

    /**
     * 测试双向链表反转
     *
     * @return void
     */
    public function testDoubleLinkedListReverse(): void
    {
        Ds4viz::doubleLinkedList(output: $this->tempFile)
            ->run(function (DoubleLinkedList $dlist) {
                $dlist->insertTail(1);
                $dlist->insertTail(2);
                $dlist->insertTail(3);
                $dlist->reverse();
            });

        $content = file_get_contents($this->tempFile);
        $this->assertStringContainsString('[result]', $content);
    }

    /**
     * 测试空双向链表删除头节点错误
     *
     * @return void
     */
    public function testDoubleLinkedListEmptyDeleteHeadError(): void
    {
        $this->expectException(StructureException::class);

        Ds4viz::doubleLinkedList(output: $this->tempFile)
            ->run(function (DoubleLinkedList $dlist) {
                $dlist->deleteHead();
            });
    }

    /**
     * 测试空双向链表删除尾节点错误
     *
     * @return void
     */
    public function testDoubleLinkedListEmptyDeleteTailError(): void
    {
        $this->expectException(StructureException::class);

        Ds4viz::doubleLinkedList(output: $this->tempFile)
            ->run(function (DoubleLinkedList $dlist) {
                $dlist->deleteTail();
            });
    }

    /**
     * 测试在不存在的节点前插入错误
     *
     * @return void
     */
    public function testDoubleLinkedListInsertBeforeNonexistentError(): void
    {
        $this->expectException(StructureException::class);

        Ds4viz::doubleLinkedList(output: $this->tempFile)
            ->run(function (DoubleLinkedList $dlist) {
                $dlist->insertHead(10);
                $dlist->insertBefore(999, 20);
            });
    }

    /**
     * 测试删除双向链表中不存在的节点错误
     *
     * @return void
     */
    public function testDoubleLinkedListDeleteNonexistentError(): void
    {
        $this->expectException(StructureException::class);

        Ds4viz::doubleLinkedList(output: $this->tempFile)
            ->run(function (DoubleLinkedList $dlist) {
                $dlist->insertHead(10);
                $dlist->delete(999);
            });
    }

    /**
     * 测试错误后生成错误文件
     *
     * @return void
     */
    public function testErrorFileGeneration(): void
    {
        try {
            Ds4viz::stack(output: $this->tempFile)
                ->run(function (Stack $s) {
                    $s->pop();
                });
        } catch (StructureException $e) {
            // 预期异常
        }

        $this->assertFileExists($this->tempFile);
        $content = file_get_contents($this->tempFile);
        $this->assertStringContainsString('[error]', $content);
        $this->assertStringContainsString('type = "runtime"', $content);
    }
}
