<?php

declare(strict_types=1);

/**
 * 树形数据结构测试
 *
 * @author WaterRun
 * @file test/TreeTest.php
 * @package Ds4viz\Test
 * @since 2025-12-24
 * @version 1.0.0
 */

namespace Ds4viz\Test;

use Ds4viz\Ds4viz;
use Ds4viz\Exception\StructureException;
use Ds4viz\Structures\BinaryTree;
use Ds4viz\Structures\BinarySearchTree;
use Ds4viz\Structures\Heap;
use InvalidArgumentException;
use PHPUnit\Framework\TestCase;

/**
 * 树形数据结构测试类
 *
 * @package Ds4viz\Test
 * @author WaterRun
 * @since 2025-12-24
 */
final class TreeTest extends TestCase
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
        $this->tempFile = sys_get_temp_dir() . '/ds4viz_tree_test_' . uniqid() . '.toml';
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
     * 测试二叉树基本操作
     *
     * @return void
     */
    public function testBinaryTreeBasicOperations(): void
    {
        Ds4viz::binaryTree(label: 'test_btree', output: $this->tempFile)
            ->run(function (BinaryTree $bt) {
                $root = $bt->insertRoot(10);
                $left = $bt->insertLeft($root, 5);
                $bt->insertRight($root, 15);
                $bt->insertLeft($left, 3);
                $bt->insertRight($left, 7);
            });

        $content = file_get_contents($this->tempFile);
        $this->assertStringContainsString('kind = "binary_tree"', $content);
        $this->assertStringContainsString('[result]', $content);
    }

    /**
     * 测试删除子树
     *
     * @return void
     */
    public function testBinaryTreeDeleteSubtree(): void
    {
        Ds4viz::binaryTree(output: $this->tempFile)
            ->run(function (BinaryTree $bt) {
                $root = $bt->insertRoot(10);
                $left = $bt->insertLeft($root, 5);
                $bt->insertLeft($left, 3);
                $bt->insertRight($left, 7);
                $bt->delete($left);
            });

        $content = file_get_contents($this->tempFile);
        $this->assertStringContainsString('[result]', $content);
    }

    /**
     * 测试更新节点值
     *
     * @return void
     */
    public function testBinaryTreeUpdateValue(): void
    {
        Ds4viz::binaryTree(output: $this->tempFile)
            ->run(function (BinaryTree $bt) {
                $root = $bt->insertRoot(10);
                $bt->updateValue($root, 100);
            });

        $content = file_get_contents($this->tempFile);
        $this->assertStringContainsString('[result]', $content);
    }

    /**
     * 测试重复插入根节点错误
     *
     * @return void
     */
    public function testBinaryTreeDuplicateRootError(): void
    {
        $this->expectException(StructureException::class);

        Ds4viz::binaryTree(output: $this->tempFile)
            ->run(function (BinaryTree $bt) {
                $bt->insertRoot(10);
                $bt->insertRoot(20);
            });
    }

    /**
     * 测试重复插入子节点错误
     *
     * @return void
     */
    public function testBinaryTreeDuplicateChildError(): void
    {
        $this->expectException(StructureException::class);

        Ds4viz::binaryTree(output: $this->tempFile)
            ->run(function (BinaryTree $bt) {
                $root = $bt->insertRoot(10);
                $bt->insertLeft($root, 5);
                $bt->insertLeft($root, 6);
            });
    }

    /**
     * 测试不存在的父节点错误
     *
     * @return void
     */
    public function testBinaryTreeNonexistentParentError(): void
    {
        $this->expectException(StructureException::class);

        Ds4viz::binaryTree(output: $this->tempFile)
            ->run(function (BinaryTree $bt) {
                $bt->insertRoot(10);
                $bt->insertLeft(999, 5);
            });
    }

    /**
     * 测试删除不存在的节点错误
     *
     * @return void
     */
    public function testBinaryTreeDeleteNonexistentError(): void
    {
        $this->expectException(StructureException::class);

        Ds4viz::binaryTree(output: $this->tempFile)
            ->run(function (BinaryTree $bt) {
                $bt->insertRoot(10);
                $bt->delete(999);
            });
    }

    /**
     * 测试更新不存在的节点错误
     *
     * @return void
     */
    public function testBinaryTreeUpdateNonexistentError(): void
    {
        $this->expectException(StructureException::class);

        Ds4viz::binaryTree(output: $this->tempFile)
            ->run(function (BinaryTree $bt) {
                $bt->insertRoot(10);
                $bt->updateValue(999, 100);
            });
    }

    /**
     * 测试 BST 基本操作
     *
     * @return void
     */
    public function testBstBasicOperations(): void
    {
        Ds4viz::binarySearchTree(label: 'test_bst', output: $this->tempFile)
            ->run(function (BinarySearchTree $bst) {
                $bst->insert(10);
                $bst->insert(5);
                $bst->insert(15);
                $bst->insert(3);
                $bst->insert(7);
            });

        $content = file_get_contents($this->tempFile);
        $this->assertStringContainsString('kind = "bst"', $content);
        $this->assertStringContainsString('[result]', $content);
    }

    /**
     * 测试删除叶节点
     *
     * @return void
     */
    public function testBstDeleteLeaf(): void
    {
        Ds4viz::binarySearchTree(output: $this->tempFile)
            ->run(function (BinarySearchTree $bst) {
                $bst->insert(10);
                $bst->insert(5);
                $bst->insert(15);
                $bst->delete(5);
            });

        $content = file_get_contents($this->tempFile);
        $this->assertStringContainsString('[result]', $content);
    }

    /**
     * 测试删除只有一个子节点的节点
     *
     * @return void
     */
    public function testBstDeleteWithOneChild(): void
    {
        Ds4viz::binarySearchTree(output: $this->tempFile)
            ->run(function (BinarySearchTree $bst) {
                $bst->insert(10);
                $bst->insert(5);
                $bst->insert(3);
                $bst->delete(5);
            });

        $content = file_get_contents($this->tempFile);
        $this->assertStringContainsString('[result]', $content);
    }

    /**
     * 测试删除有两个子节点的节点
     *
     * @return void
     */
    public function testBstDeleteWithTwoChildren(): void
    {
        Ds4viz::binarySearchTree(output: $this->tempFile)
            ->run(function (BinarySearchTree $bst) {
                $bst->insert(10);
                $bst->insert(5);
                $bst->insert(15);
                $bst->insert(3);
                $bst->insert(7);
                $bst->delete(5);
            });

        $content = file_get_contents($this->tempFile);
        $this->assertStringContainsString('[result]', $content);
    }

    /**
     * 测试删除不存在的节点错误
     *
     * @return void
     */
    public function testBstDeleteNonexistentError(): void
    {
        $this->expectException(StructureException::class);

        Ds4viz::binarySearchTree(output: $this->tempFile)
            ->run(function (BinarySearchTree $bst) {
                $bst->insert(10);
                $bst->delete(999);
            });
    }

    /**
     * 测试删除根节点
     *
     * @return void
     */
    public function testBstDeleteRoot(): void
    {
        Ds4viz::binarySearchTree(output: $this->tempFile)
            ->run(function (BinarySearchTree $bst) {
                $bst->insert(10);
                $bst->insert(5);
                $bst->insert(15);
                $bst->delete(10);
            });

        $content = file_get_contents($this->tempFile);
        $this->assertStringContainsString('[result]', $content);
    }

    /**
     * 测试最小堆基本操作
     *
     * @return void
     */
    public function testMinHeapBasic(): void
    {
        Ds4viz::heap(label: 'test_heap', heapType: 'min', output: $this->tempFile)
            ->run(function (Heap $h) {
                $h->insert(10);
                $h->insert(5);
                $h->insert(15);
                $h->insert(3);
                $h->extract();
            });

        $content = file_get_contents($this->tempFile);
        $this->assertStringContainsString('kind = "binary_tree"', $content);
        $this->assertStringContainsString('[result]', $content);
    }

    /**
     * 测试最大堆基本操作
     *
     * @return void
     */
    public function testMaxHeapBasic(): void
    {
        Ds4viz::heap(heapType: 'max', output: $this->tempFile)
            ->run(function (Heap $h) {
                $h->insert(10);
                $h->insert(20);
                $h->insert(5);
                $h->extract();
            });

        $content = file_get_contents($this->tempFile);
        $this->assertStringContainsString('[result]', $content);
    }

    /**
     * 测试空堆提取错误
     *
     * @return void
     */
    public function testHeapExtractEmptyError(): void
    {
        $this->expectException(StructureException::class);

        Ds4viz::heap(output: $this->tempFile)
            ->run(function (Heap $h) {
                $h->extract();
            });
    }

    /**
     * 测试堆清空
     *
     * @return void
     */
    public function testHeapClear(): void
    {
        Ds4viz::heap(output: $this->tempFile)
            ->run(function (Heap $h) {
                $h->insert(1);
                $h->insert(2);
                $h->insert(3);
                $h->clear();
            });

        $content = file_get_contents($this->tempFile);
        $this->assertStringContainsString('[result]', $content);
    }

    /**
     * 测试无效堆类型错误
     *
     * @return void
     */
    public function testHeapInvalidTypeError(): void
    {
        $this->expectException(InvalidArgumentException::class);

        Ds4viz::heap(heapType: 'invalid');
    }

    /**
     * 测试多次提取
     *
     * @return void
     */
    public function testHeapMultipleExtract(): void
    {
        Ds4viz::heap(heapType: 'min', output: $this->tempFile)
            ->run(function (Heap $h) {
                $h->insert(10);
                $h->insert(5);
                $h->insert(15);
                $h->insert(3);
                $h->extract();
                $h->extract();
            });

        $content = file_get_contents($this->tempFile);
        $this->assertStringContainsString('[result]', $content);
    }

    /**
     * 测试提取后再插入
     *
     * @return void
     */
    public function testHeapInsertAfterExtract(): void
    {
        Ds4viz::heap(heapType: 'min', output: $this->tempFile)
            ->run(function (Heap $h) {
                $h->insert(10);
                $h->insert(5);
                $h->extract();
                $h->insert(3);
            });

        $content = file_get_contents($this->tempFile);
        $this->assertStringContainsString('[result]', $content);
    }
}
