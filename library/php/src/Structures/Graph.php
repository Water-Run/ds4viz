<?php

declare(strict_types=1);

/**
 * 图数据结构实现：无向图、有向图、带权图
 *
 * @author WaterRun
 * @file src/Structures/Graph.php
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
 * 无向图数据结构
 *
 * @package Ds4viz\Structures
 * @author WaterRun
 * @since 2025-12-24
 */
final class GraphUndirected
{
    /**
     * 会话对象
     *
     * @var Session
     */
    private Session $session;

    /**
     * 节点映射表
     *
     * @var array<int, string>
     */
    private array $nodes = [];

    /**
     * 边集合
     *
     * @var array<string, bool>
     */
    private array $edges = [];

    /**
     * 构造函数
     *
     * @param string $label 无向图的标签
     * @param string|null $output 输出文件路径
     */
    public function __construct(
        private readonly string $label = 'graph',
        ?string $output = null,
    ) {
        $outputPath = $output ?? Ds4viz::getGlobalConfig()->outputPath;
        $this->session = new Session(kind: 'graph_undirected', label: $this->label, output: $outputPath);
    }

    /**
     * 执行无向图操作
     *
     * @param callable(GraphUndirected): void $callback 回调函数
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
     * 添加节点
     *
     * @param int $nodeId 节点 id
     * @param string $label 节点标签
     * @return void
     * @throws StructureException 当节点已存在时
     */
    public function addNode(int $nodeId, string $label): void
    {
        $line = getCallerLine(2);

        if (isset($this->nodes[$nodeId])) {
            $this->session->setFailedStepId($this->session->getStepCounter());
            throw new StructureException("Node already exists: {$nodeId}", 'runtime', $line);
        }

        $labelLen = mb_strlen($label);
        if ($labelLen < 1 || $labelLen > 32) {
            $this->session->setFailedStepId($this->session->getStepCounter());
            throw new StructureException("Label length must be 1-32, got {$labelLen}", 'runtime', $line);
        }

        $before = $this->session->getLastStateId();
        $this->nodes[$nodeId] = $label;
        $after = $this->session->addState($this->buildData());
        $this->session->addStep(
            op: 'add_node',
            before: $before,
            after: $after,
            args: ['id' => $nodeId, 'label' => $label],
            line: $line,
        );
    }

    /**
     * 添加无向边
     *
     * @param int $fromId 起始节点 id
     * @param int $toId 终止节点 id
     * @return void
     * @throws StructureException 当节点不存在、边已存在或形成自环时
     */
    public function addEdge(int $fromId, int $toId): void
    {
        $line = getCallerLine(2);

        if (!isset($this->nodes[$fromId])) {
            $this->session->setFailedStepId($this->session->getStepCounter());
            throw new StructureException("Node not found: {$fromId}", 'runtime', $line);
        }
        if (!isset($this->nodes[$toId])) {
            $this->session->setFailedStepId($this->session->getStepCounter());
            throw new StructureException("Node not found: {$toId}", 'runtime', $line);
        }
        if ($fromId === $toId) {
            $this->session->setFailedStepId($this->session->getStepCounter());
            throw new StructureException("Self-loop not allowed: {$fromId}", 'runtime', $line);
        }

        $edgeKey = $this->normalizeEdge($fromId, $toId);
        if (isset($this->edges[$edgeKey])) {
            $this->session->setFailedStepId($this->session->getStepCounter());
            throw new StructureException("Edge already exists: ({$fromId}, {$toId})", 'runtime', $line);
        }

        $before = $this->session->getLastStateId();
        $this->edges[$edgeKey] = true;
        $after = $this->session->addState($this->buildData());
        $this->session->addStep(
            op: 'add_edge',
            before: $before,
            after: $after,
            args: ['from' => $fromId, 'to' => $toId],
            line: $line,
        );
    }

    /**
     * 删除节点
     *
     * @param int $nodeId 节点 id
     * @return void
     * @throws StructureException 当节点不存在时
     */
    public function removeNode(int $nodeId): void
    {
        $line = getCallerLine(2);

        if (!isset($this->nodes[$nodeId])) {
            $this->session->setFailedStepId($this->session->getStepCounter());
            throw new StructureException("Node not found: {$nodeId}", 'runtime', $line);
        }

        $before = $this->session->getLastStateId();

        $edgesToRemove = [];
        foreach ($this->edges as $edgeKey => $unused) {
            [$from, $to] = explode(':', $edgeKey);
            if ((int) $from === $nodeId || (int) $to === $nodeId) {
                $edgesToRemove[] = $edgeKey;
            }
        }
        foreach ($edgesToRemove as $edgeKey) {
            unset($this->edges[$edgeKey]);
        }
        unset($this->nodes[$nodeId]);

        $after = $this->session->addState($this->buildData());
        $this->session->addStep(
            op: 'remove_node',
            before: $before,
            after: $after,
            args: ['node_id' => $nodeId],
            line: $line,
        );
    }

    /**
     * 删除边
     *
     * @param int $fromId 起始节点 id
     * @param int $toId 终止节点 id
     * @return void
     * @throws StructureException 当边不存在时
     */
    public function removeEdge(int $fromId, int $toId): void
    {
        $line = getCallerLine(2);
        $edgeKey = $this->normalizeEdge($fromId, $toId);

        if (!isset($this->edges[$edgeKey])) {
            $this->session->setFailedStepId($this->session->getStepCounter());
            throw new StructureException("Edge not found: ({$fromId}, {$toId})", 'runtime', $line);
        }

        $before = $this->session->getLastStateId();
        unset($this->edges[$edgeKey]);
        $after = $this->session->addState($this->buildData());
        $this->session->addStep(
            op: 'remove_edge',
            before: $before,
            after: $after,
            args: ['from' => $fromId, 'to' => $toId],
            line: $line,
        );
    }

    /**
     * 更新节点标签
     *
     * @param int $nodeId 节点 id
     * @param string $label 新标签
     * @return void
     * @throws StructureException 当节点不存在时
     */
    public function updateNodeLabel(int $nodeId, string $label): void
    {
        $line = getCallerLine(2);

        if (!isset($this->nodes[$nodeId])) {
            $this->session->setFailedStepId($this->session->getStepCounter());
            throw new StructureException("Node not found: {$nodeId}", 'runtime', $line);
        }

        $labelLen = mb_strlen($label);
        if ($labelLen < 1 || $labelLen > 32) {
            $this->session->setFailedStepId($this->session->getStepCounter());
            throw new StructureException("Label length must be 1-32, got {$labelLen}", 'runtime', $line);
        }

        $before = $this->session->getLastStateId();
        $oldLabel = $this->nodes[$nodeId];
        $this->nodes[$nodeId] = $label;
        $after = $this->session->addState($this->buildData());
        $this->session->addStep(
            op: 'update_node_label',
            before: $before,
            after: $after,
            args: ['node_id' => $nodeId, 'label' => $label],
            ret: $oldLabel,
            line: $line,
        );
    }

    /**
     * 规范化无向边
     *
     * @param int $fromId 起始节点 id
     * @param int $toId 终止节点 id
     * @return string 规范化后的边键
     */
    private function normalizeEdge(int $fromId, int $toId): string
    {
        $min = min($fromId, $toId);
        $max = max($fromId, $toId);
        return "{$min}:{$max}";
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
        foreach ($this->nodes as $nid => $label) {
            $nodesList[] = ['id' => $nid, 'label' => $label];
        }

        $edgesList = [];
        $edgeKeys = array_keys($this->edges);
        sort($edgeKeys);
        foreach ($edgeKeys as $edgeKey) {
            [$from, $to] = explode(':', $edgeKey);
            $edgesList[] = ['from' => (int) $from, 'to' => (int) $to];
        }

        return ['nodes' => $nodesList, 'edges' => $edgesList];
    }
}

/**
 * 有向图数据结构
 *
 * @package Ds4viz\Structures
 * @author WaterRun
 * @since 2025-12-24
 */
final class GraphDirected
{
    /**
     * 会话对象
     *
     * @var Session
     */
    private Session $session;

    /**
     * 节点映射表
     *
     * @var array<int, string>
     */
    private array $nodes = [];

    /**
     * 边集合
     *
     * @var array<string, bool>
     */
    private array $edges = [];

    /**
     * 构造函数
     *
     * @param string $label 有向图的标签
     * @param string|null $output 输出文件路径
     */
    public function __construct(
        private readonly string $label = 'graph',
        ?string $output = null,
    ) {
        $outputPath = $output ?? Ds4viz::getGlobalConfig()->outputPath;
        $this->session = new Session(kind: 'graph_directed', label: $this->label, output: $outputPath);
    }

    /**
     * 执行有向图操作
     *
     * @param callable(GraphDirected): void $callback 回调函数
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
     * 添加节点
     *
     * @param int $nodeId 节点 id
     * @param string $label 节点标签
     * @return void
     * @throws StructureException 当节点已存在时
     */
    public function addNode(int $nodeId, string $label): void
    {
        $line = getCallerLine(2);

        if (isset($this->nodes[$nodeId])) {
            $this->session->setFailedStepId($this->session->getStepCounter());
            throw new StructureException("Node already exists: {$nodeId}", 'runtime', $line);
        }

        $labelLen = mb_strlen($label);
        if ($labelLen < 1 || $labelLen > 32) {
            $this->session->setFailedStepId($this->session->getStepCounter());
            throw new StructureException("Label length must be 1-32, got {$labelLen}", 'runtime', $line);
        }

        $before = $this->session->getLastStateId();
        $this->nodes[$nodeId] = $label;
        $after = $this->session->addState($this->buildData());
        $this->session->addStep(
            op: 'add_node',
            before: $before,
            after: $after,
            args: ['id' => $nodeId, 'label' => $label],
            line: $line,
        );
    }

    /**
     * 添加有向边
     *
     * @param int $fromId 起始节点 id
     * @param int $toId 终止节点 id
     * @return void
     * @throws StructureException 当节点不存在、边已存在或形成自环时
     */
    public function addEdge(int $fromId, int $toId): void
    {
        $line = getCallerLine(2);

        if (!isset($this->nodes[$fromId])) {
            $this->session->setFailedStepId($this->session->getStepCounter());
            throw new StructureException("Node not found: {$fromId}", 'runtime', $line);
        }
        if (!isset($this->nodes[$toId])) {
            $this->session->setFailedStepId($this->session->getStepCounter());
            throw new StructureException("Node not found: {$toId}", 'runtime', $line);
        }
        if ($fromId === $toId) {
            $this->session->setFailedStepId($this->session->getStepCounter());
            throw new StructureException("Self-loop not allowed: {$fromId}", 'runtime', $line);
        }

        $edgeKey = "{$fromId}:{$toId}";
        if (isset($this->edges[$edgeKey])) {
            $this->session->setFailedStepId($this->session->getStepCounter());
            throw new StructureException("Edge already exists: ({$fromId}, {$toId})", 'runtime', $line);
        }

        $before = $this->session->getLastStateId();
        $this->edges[$edgeKey] = true;
        $after = $this->session->addState($this->buildData());
        $this->session->addStep(
            op: 'add_edge',
            before: $before,
            after: $after,
            args: ['from' => $fromId, 'to' => $toId],
            line: $line,
        );
    }

    /**
     * 删除节点
     *
     * @param int $nodeId 节点 id
     * @return void
     * @throws StructureException 当节点不存在时
     */
    public function removeNode(int $nodeId): void
    {
        $line = getCallerLine(2);

        if (!isset($this->nodes[$nodeId])) {
            $this->session->setFailedStepId($this->session->getStepCounter());
            throw new StructureException("Node not found: {$nodeId}", 'runtime', $line);
        }

        $before = $this->session->getLastStateId();

        $edgesToRemove = [];
        foreach ($this->edges as $edgeKey => $unused) {
            [$from, $to] = explode(':', $edgeKey);
            if ((int) $from === $nodeId || (int) $to === $nodeId) {
                $edgesToRemove[] = $edgeKey;
            }
        }
        foreach ($edgesToRemove as $edgeKey) {
            unset($this->edges[$edgeKey]);
        }
        unset($this->nodes[$nodeId]);

        $after = $this->session->addState($this->buildData());
        $this->session->addStep(
            op: 'remove_node',
            before: $before,
            after: $after,
            args: ['node_id' => $nodeId],
            line: $line,
        );
    }

    /**
     * 删除边
     *
     * @param int $fromId 起始节点 id
     * @param int $toId 终止节点 id
     * @return void
     * @throws StructureException 当边不存在时
     */
    public function removeEdge(int $fromId, int $toId): void
    {
        $line = getCallerLine(2);
        $edgeKey = "{$fromId}:{$toId}";

        if (!isset($this->edges[$edgeKey])) {
            $this->session->setFailedStepId($this->session->getStepCounter());
            throw new StructureException("Edge not found: ({$fromId}, {$toId})", 'runtime', $line);
        }

        $before = $this->session->getLastStateId();
        unset($this->edges[$edgeKey]);
        $after = $this->session->addState($this->buildData());
        $this->session->addStep(
            op: 'remove_edge',
            before: $before,
            after: $after,
            args: ['from' => $fromId, 'to' => $toId],
            line: $line,
        );
    }

    /**
     * 更新节点标签
     *
     * @param int $nodeId 节点 id
     * @param string $label 新标签
     * @return void
     * @throws StructureException 当节点不存在时
     */
    public function updateNodeLabel(int $nodeId, string $label): void
    {
        $line = getCallerLine(2);

        if (!isset($this->nodes[$nodeId])) {
            $this->session->setFailedStepId($this->session->getStepCounter());
            throw new StructureException("Node not found: {$nodeId}", 'runtime', $line);
        }

        $labelLen = mb_strlen($label);
        if ($labelLen < 1 || $labelLen > 32) {
            $this->session->setFailedStepId($this->session->getStepCounter());
            throw new StructureException("Label length must be 1-32, got {$labelLen}", 'runtime', $line);
        }

        $before = $this->session->getLastStateId();
        $oldLabel = $this->nodes[$nodeId];
        $this->nodes[$nodeId] = $label;
        $after = $this->session->addState($this->buildData());
        $this->session->addStep(
            op: 'update_node_label',
            before: $before,
            after: $after,
            args: ['node_id' => $nodeId, 'label' => $label],
            ret: $oldLabel,
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
        foreach ($this->nodes as $nid => $label) {
            $nodesList[] = ['id' => $nid, 'label' => $label];
        }

        $edgesList = [];
        $edgeKeys = array_keys($this->edges);
        sort($edgeKeys);
        foreach ($edgeKeys as $edgeKey) {
            [$from, $to] = explode(':', $edgeKey);
            $edgesList[] = ['from' => (int) $from, 'to' => (int) $to];
        }

        return ['nodes' => $nodesList, 'edges' => $edgesList];
    }
}

/**
 * 带权图数据结构
 *
 * @package Ds4viz\Structures
 * @author WaterRun
 * @since 2025-12-24
 */
final class GraphWeighted
{
    /**
     * 会话对象
     *
     * @var Session
     */
    private Session $session;

    /**
     * 节点映射表
     *
     * @var array<int, string>
     */
    private array $nodes = [];

    /**
     * 边及权重
     *
     * @var array<string, float>
     */
    private array $edges = [];

    /**
     * 构造函数
     *
     * @param string $label 带权图的标签
     * @param bool $directed 是否为有向图
     * @param string|null $output 输出文件路径
     */
    public function __construct(
        private readonly string $label = 'graph',
        private readonly bool $directed = false,
        ?string $output = null,
    ) {
        $outputPath = $output ?? Ds4viz::getGlobalConfig()->outputPath;
        $this->session = new Session(kind: 'graph_weighted', label: $this->label, output: $outputPath);
    }

    /**
     * 执行带权图操作
     *
     * @param callable(GraphWeighted): void $callback 回调函数
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
     * 添加节点
     *
     * @param int $nodeId 节点 id
     * @param string $label 节点标签
     * @return void
     * @throws StructureException 当节点已存在时
     */
    public function addNode(int $nodeId, string $label): void
    {
        $line = getCallerLine(2);

        if (isset($this->nodes[$nodeId])) {
            $this->session->setFailedStepId($this->session->getStepCounter());
            throw new StructureException("Node already exists: {$nodeId}", 'runtime', $line);
        }

        $labelLen = mb_strlen($label);
        if ($labelLen < 1 || $labelLen > 32) {
            $this->session->setFailedStepId($this->session->getStepCounter());
            throw new StructureException("Label length must be 1-32, got {$labelLen}", 'runtime', $line);
        }

        $before = $this->session->getLastStateId();
        $this->nodes[$nodeId] = $label;
        $after = $this->session->addState($this->buildData());
        $this->session->addStep(
            op: 'add_node',
            before: $before,
            after: $after,
            args: ['id' => $nodeId, 'label' => $label],
            line: $line,
        );
    }

    /**
     * 添加带权边
     *
     * @param int $fromId 起始节点 id
     * @param int $toId 终止节点 id
     * @param float $weight 边的权重
     * @return void
     * @throws StructureException 当节点不存在、边已存在或形成自环时
     */
    public function addEdge(int $fromId, int $toId, float $weight): void
    {
        $line = getCallerLine(2);

        if (!isset($this->nodes[$fromId])) {
            $this->session->setFailedStepId($this->session->getStepCounter());
            throw new StructureException("Node not found: {$fromId}", 'runtime', $line);
        }
        if (!isset($this->nodes[$toId])) {
            $this->session->setFailedStepId($this->session->getStepCounter());
            throw new StructureException("Node not found: {$toId}", 'runtime', $line);
        }
        if ($fromId === $toId) {
            $this->session->setFailedStepId($this->session->getStepCounter());
            throw new StructureException("Self-loop not allowed: {$fromId}", 'runtime', $line);
        }

        $edgeKey = $this->normalizeEdge($fromId, $toId);
        if (isset($this->edges[$edgeKey])) {
            $this->session->setFailedStepId($this->session->getStepCounter());
            throw new StructureException("Edge already exists: ({$fromId}, {$toId})", 'runtime', $line);
        }

        $before = $this->session->getLastStateId();
        $this->edges[$edgeKey] = $weight;
        $after = $this->session->addState($this->buildData());
        $this->session->addStep(
            op: 'add_edge',
            before: $before,
            after: $after,
            args: ['from' => $fromId, 'to' => $toId, 'weight' => $weight],
            line: $line,
        );
    }

    /**
     * 删除节点
     *
     * @param int $nodeId 节点 id
     * @return void
     * @throws StructureException 当节点不存在时
     */
    public function removeNode(int $nodeId): void
    {
        $line = getCallerLine(2);

        if (!isset($this->nodes[$nodeId])) {
            $this->session->setFailedStepId($this->session->getStepCounter());
            throw new StructureException("Node not found: {$nodeId}", 'runtime', $line);
        }

        $before = $this->session->getLastStateId();

        $edgesToRemove = [];
        foreach ($this->edges as $edgeKey => $unused) {
            [$from, $to] = explode(':', $edgeKey);
            if ((int) $from === $nodeId || (int) $to === $nodeId) {
                $edgesToRemove[] = $edgeKey;
            }
        }
        foreach ($edgesToRemove as $edgeKey) {
            unset($this->edges[$edgeKey]);
        }
        unset($this->nodes[$nodeId]);

        $after = $this->session->addState($this->buildData());
        $this->session->addStep(
            op: 'remove_node',
            before: $before,
            after: $after,
            args: ['node_id' => $nodeId],
            line: $line,
        );
    }

    /**
     * 删除边
     *
     * @param int $fromId 起始节点 id
     * @param int $toId 终止节点 id
     * @return void
     * @throws StructureException 当边不存在时
     */
    public function removeEdge(int $fromId, int $toId): void
    {
        $line = getCallerLine(2);
        $edgeKey = $this->normalizeEdge($fromId, $toId);

        if (!isset($this->edges[$edgeKey])) {
            $this->session->setFailedStepId($this->session->getStepCounter());
            throw new StructureException("Edge not found: ({$fromId}, {$toId})", 'runtime', $line);
        }

        $before = $this->session->getLastStateId();
        unset($this->edges[$edgeKey]);
        $after = $this->session->addState($this->buildData());
        $this->session->addStep(
            op: 'remove_edge',
            before: $before,
            after: $after,
            args: ['from' => $fromId, 'to' => $toId],
            line: $line,
        );
    }

    /**
     * 更新边的权重
     *
     * @param int $fromId 起始节点 id
     * @param int $toId 终止节点 id
     * @param float $weight 新的权重值
     * @return void
     * @throws StructureException 当边不存在时
     */
    public function updateWeight(int $fromId, int $toId, float $weight): void
    {
        $line = getCallerLine(2);
        $edgeKey = $this->normalizeEdge($fromId, $toId);

        if (!isset($this->edges[$edgeKey])) {
            $this->session->setFailedStepId($this->session->getStepCounter());
            throw new StructureException("Edge not found: ({$fromId}, {$toId})", 'runtime', $line);
        }

        $before = $this->session->getLastStateId();
        $oldWeight = $this->edges[$edgeKey];
        $this->edges[$edgeKey] = $weight;
        $after = $this->session->addState($this->buildData());
        $this->session->addStep(
            op: 'update_weight',
            before: $before,
            after: $after,
            args: ['from' => $fromId, 'to' => $toId, 'weight' => $weight],
            ret: $oldWeight,
            line: $line,
        );
    }

    /**
     * 更新节点标签
     *
     * @param int $nodeId 节点 id
     * @param string $label 新标签
     * @return void
     * @throws StructureException 当节点不存在时
     */
    public function updateNodeLabel(int $nodeId, string $label): void
    {
        $line = getCallerLine(2);

        if (!isset($this->nodes[$nodeId])) {
            $this->session->setFailedStepId($this->session->getStepCounter());
            throw new StructureException("Node not found: {$nodeId}", 'runtime', $line);
        }

        $labelLen = mb_strlen($label);
        if ($labelLen < 1 || $labelLen > 32) {
            $this->session->setFailedStepId($this->session->getStepCounter());
            throw new StructureException("Label length must be 1-32, got {$labelLen}", 'runtime', $line);
        }

        $before = $this->session->getLastStateId();
        $oldLabel = $this->nodes[$nodeId];
        $this->nodes[$nodeId] = $label;
        $after = $this->session->addState($this->buildData());
        $this->session->addStep(
            op: 'update_node_label',
            before: $before,
            after: $after,
            args: ['node_id' => $nodeId, 'label' => $label],
            ret: $oldLabel,
            line: $line,
        );
    }

    /**
     * 规范化边
     *
     * @param int $fromId 起始节点 id
     * @param int $toId 终止节点 id
     * @return string 规范化后的边键
     */
    private function normalizeEdge(int $fromId, int $toId): string
    {
        if ($this->directed) {
            return "{$fromId}:{$toId}";
        }
        $min = min($fromId, $toId);
        $max = max($fromId, $toId);
        return "{$min}:{$max}";
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
        foreach ($this->nodes as $nid => $label) {
            $nodesList[] = ['id' => $nid, 'label' => $label];
        }

        $edgesList = [];
        $edgeKeys = array_keys($this->edges);
        sort($edgeKeys);
        foreach ($edgeKeys as $edgeKey) {
            [$from, $to] = explode(':', $edgeKey);
            $edgesList[] = ['from' => (int) $from, 'to' => (int) $to, 'weight' => $this->edges[$edgeKey]];
        }

        return ['nodes' => $nodesList, 'edges' => $edgesList];
    }
}
