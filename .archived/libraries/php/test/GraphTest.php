<?php

declare(strict_types=1);

/**
 * 图数据结构测试
 *
 * @author WaterRun
 * @file test/GraphTest.php
 * @package Ds4viz\Test
 * @since 2025-12-24
 * @version 1.0.0
 */

namespace Ds4viz\Test;

use Ds4viz\Ds4viz;
use Ds4viz\Exception\StructureException;
use Ds4viz\Structures\GraphUndirected;
use Ds4viz\Structures\GraphDirected;
use Ds4viz\Structures\GraphWeighted;
use PHPUnit\Framework\TestCase;

/**
 * 图数据结构测试类
 *
 * @package Ds4viz\Test
 * @author WaterRun
 * @since 2025-12-24
 */
final class GraphTest extends TestCase
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
        $this->tempFile = sys_get_temp_dir() . '/ds4viz_graph_test_' . uniqid() . '.toml';
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
     * 测试无向图基本操作
     *
     * @return void
     */
    public function testGraphUndirectedBasic(): void
    {
        Ds4viz::graphUndirected(label: 'test_graph', output: $this->tempFile)
            ->run(function (GraphUndirected $g) {
                $g->addNode(0, 'A');
                $g->addNode(1, 'B');
                $g->addNode(2, 'C');
                $g->addEdge(0, 1);
                $g->addEdge(1, 2);
            });

        $content = file_get_contents($this->tempFile);
        $this->assertStringContainsString('kind = "graph_undirected"', $content);
        $this->assertStringContainsString('[result]', $content);
    }

    /**
     * 测试无向图边规范化
     *
     * @return void
     */
    public function testGraphUndirectedEdgeNormalization(): void
    {
        Ds4viz::graphUndirected(output: $this->tempFile)
            ->run(function (GraphUndirected $g) {
                $g->addNode(0, 'A');
                $g->addNode(1, 'B');
                $g->addEdge(1, 0);
            });

        $content = file_get_contents($this->tempFile);
        $this->assertStringContainsString('from = 0', $content);
        $this->assertStringContainsString('to = 1', $content);
    }

    /**
     * 测试无向图自环错误
     *
     * @return void
     */
    public function testGraphUndirectedSelfLoopError(): void
    {
        $this->expectException(StructureException::class);

        Ds4viz::graphUndirected(output: $this->tempFile)
            ->run(function (GraphUndirected $g) {
                $g->addNode(0, 'A');
                $g->addEdge(0, 0);
            });
    }

    /**
     * 测试无向图重复边错误
     *
     * @return void
     */
    public function testGraphUndirectedDuplicateEdgeError(): void
    {
        $this->expectException(StructureException::class);

        Ds4viz::graphUndirected(output: $this->tempFile)
            ->run(function (GraphUndirected $g) {
                $g->addNode(0, 'A');
                $g->addNode(1, 'B');
                $g->addEdge(0, 1);
                $g->addEdge(0, 1);
            });
    }

    /**
     * 测试删除节点（同时删除相关边）
     *
     * @return void
     */
    public function testGraphUndirectedRemoveNode(): void
    {
        Ds4viz::graphUndirected(output: $this->tempFile)
            ->run(function (GraphUndirected $g) {
                $g->addNode(0, 'A');
                $g->addNode(1, 'B');
                $g->addNode(2, 'C');
                $g->addEdge(0, 1);
                $g->addEdge(1, 2);
                $g->removeNode(1);
            });

        $content = file_get_contents($this->tempFile);
        $this->assertStringContainsString('[result]', $content);
    }

    /**
     * 测试删除边
     *
     * @return void
     */
    public function testGraphUndirectedRemoveEdge(): void
    {
        Ds4viz::graphUndirected(output: $this->tempFile)
            ->run(function (GraphUndirected $g) {
                $g->addNode(0, 'A');
                $g->addNode(1, 'B');
                $g->addEdge(0, 1);
                $g->removeEdge(0, 1);
            });

        $content = file_get_contents($this->tempFile);
        $this->assertStringContainsString('[result]', $content);
    }

    /**
     * 测试添加重复节点错误
     *
     * @return void
     */
    public function testGraphUndirectedDuplicateNodeError(): void
    {
        $this->expectException(StructureException::class);

        Ds4viz::graphUndirected(output: $this->tempFile)
            ->run(function (GraphUndirected $g) {
                $g->addNode(0, 'A');
                $g->addNode(0, 'B');
            });
    }

    /**
     * 测试删除不存在的节点错误
     *
     * @return void
     */
    public function testGraphUndirectedRemoveNonexistentNodeError(): void
    {
        $this->expectException(StructureException::class);

        Ds4viz::graphUndirected(output: $this->tempFile)
            ->run(function (GraphUndirected $g) {
                $g->addNode(0, 'A');
                $g->removeNode(999);
            });
    }

    /**
     * 测试更新节点标签
     *
     * @return void
     */
    public function testGraphUndirectedUpdateNodeLabel(): void
    {
        Ds4viz::graphUndirected(output: $this->tempFile)
            ->run(function (GraphUndirected $g) {
                $g->addNode(0, 'A');
                $g->updateNodeLabel(0, 'X');
            });

        $content = file_get_contents($this->tempFile);
        $this->assertStringContainsString('[result]', $content);
    }

    /**
     * 测试标签长度过长错误
     *
     * @return void
     */
    public function testGraphUndirectedLabelTooLongError(): void
    {
        $this->expectException(StructureException::class);

        Ds4viz::graphUndirected(output: $this->tempFile)
            ->run(function (GraphUndirected $g) {
                $g->addNode(0, str_repeat('A', 33));
            });
    }

    /**
     * 测试标签为空错误
     *
     * @return void
     */
    public function testGraphUndirectedEmptyLabelError(): void
    {
        $this->expectException(StructureException::class);

        Ds4viz::graphUndirected(output: $this->tempFile)
            ->run(function (GraphUndirected $g) {
                $g->addNode(0, '');
            });
    }

    /**
     * 测试有向图基本操作
     *
     * @return void
     */
    public function testGraphDirectedBasic(): void
    {
        Ds4viz::graphDirected(label: 'test_digraph', output: $this->tempFile)
            ->run(function (GraphDirected $g) {
                $g->addNode(0, 'A');
                $g->addNode(1, 'B');
                $g->addEdge(0, 1);
                $g->addEdge(1, 0);
            });

        $content = file_get_contents($this->tempFile);
        $this->assertStringContainsString('kind = "graph_directed"', $content);
        $this->assertStringContainsString('[result]', $content);
    }

    /**
     * 测试有向图不进行边规范化
     *
     * @return void
     */
    public function testGraphDirectedNoEdgeNormalization(): void
    {
        Ds4viz::graphDirected(output: $this->tempFile)
            ->run(function (GraphDirected $g) {
                $g->addNode(0, 'A');
                $g->addNode(1, 'B');
                $g->addEdge(1, 0);
            });

        $content = file_get_contents($this->tempFile);
        $this->assertStringContainsString('from = 1', $content);
        $this->assertStringContainsString('to = 0', $content);
    }

    /**
     * 测试有向图自环错误
     *
     * @return void
     */
    public function testGraphDirectedSelfLoopError(): void
    {
        $this->expectException(StructureException::class);

        Ds4viz::graphDirected(output: $this->tempFile)
            ->run(function (GraphDirected $g) {
                $g->addNode(0, 'A');
                $g->addEdge(0, 0);
            });
    }

    /**
     * 测试带权图无向操作
     *
     * @return void
     */
    public function testGraphWeightedUndirected(): void
    {
        Ds4viz::graphWeighted(label: 'test_wgraph', directed: false, output: $this->tempFile)
            ->run(function (GraphWeighted $g) {
                $g->addNode(0, 'A');
                $g->addNode(1, 'B');
                $g->addEdge(0, 1, 3.5);
            });

        $content = file_get_contents($this->tempFile);
        $this->assertStringContainsString('kind = "graph_weighted"', $content);
        $this->assertStringContainsString('weight = 3.5', $content);
    }

    /**
     * 测试带权图有向操作
     *
     * @return void
     */
    public function testGraphWeightedDirected(): void
    {
        Ds4viz::graphWeighted(directed: true, output: $this->tempFile)
            ->run(function (GraphWeighted $g) {
                $g->addNode(0, 'A');
                $g->addNode(1, 'B');
                $g->addEdge(0, 1, 2.0);
                $g->addEdge(1, 0, 3.0);
            });

        $content = file_get_contents($this->tempFile);
        $this->assertStringContainsString('[result]', $content);
    }

    /**
     * 测试更新边权重
     *
     * @return void
     */
    public function testGraphWeightedUpdateWeight(): void
    {
        Ds4viz::graphWeighted(output: $this->tempFile)
            ->run(function (GraphWeighted $g) {
                $g->addNode(0, 'A');
                $g->addNode(1, 'B');
                $g->addEdge(0, 1, 1.0);
                $g->updateWeight(0, 1, 5.0);
            });

        $content = file_get_contents($this->tempFile);
        $this->assertStringContainsString('[result]', $content);
    }

    /**
     * 测试更新不存在的边权重错误
     *
     * @return void
     */
    public function testGraphWeightedUpdateNonexistentEdgeError(): void
    {
        $this->expectException(StructureException::class);

        Ds4viz::graphWeighted(output: $this->tempFile)
            ->run(function (GraphWeighted $g) {
                $g->addNode(0, 'A');
                $g->addNode(1, 'B');
                $g->updateWeight(0, 1, 5.0);
            });
    }

    /**
     * 测试负权重
     *
     * @return void
     */
    public function testGraphWeightedNegativeWeight(): void
    {
        Ds4viz::graphWeighted(output: $this->tempFile)
            ->run(function (GraphWeighted $g) {
                $g->addNode(0, 'A');
                $g->addNode(1, 'B');
                $g->addEdge(0, 1, -5.5);
            });

        $content = file_get_contents($this->tempFile);
        $this->assertStringContainsString('[result]', $content);
    }

    /**
     * 测试零权重
     *
     * @return void
     */
    public function testGraphWeightedZeroWeight(): void
    {
        Ds4viz::graphWeighted(output: $this->tempFile)
            ->run(function (GraphWeighted $g) {
                $g->addNode(0, 'A');
                $g->addNode(1, 'B');
                $g->addEdge(0, 1, 0.0);
            });

        $content = file_get_contents($this->tempFile);
        $this->assertStringContainsString('[result]', $content);
    }

    /**
     * 测试删除带权边
     *
     * @return void
     */
    public function testGraphWeightedRemoveEdge(): void
    {
        Ds4viz::graphWeighted(output: $this->tempFile)
            ->run(function (GraphWeighted $g) {
                $g->addNode(0, 'A');
                $g->addNode(1, 'B');
                $g->addEdge(0, 1, 1.0);
                $g->removeEdge(0, 1);
            });

        $content = file_get_contents($this->tempFile);
        $this->assertStringContainsString('[result]', $content);
    }

    /**
     * 测试删除不存在的边错误
     *
     * @return void
     */
    public function testGraphWeightedRemoveNonexistentEdgeError(): void
    {
        $this->expectException(StructureException::class);

        Ds4viz::graphWeighted(output: $this->tempFile)
            ->run(function (GraphWeighted $g) {
                $g->addNode(0, 'A');
                $g->addNode(1, 'B');
                $g->removeEdge(0, 1);
            });
    }
}
