<?php

declare(strict_types=1);

/**
 * 会话管理模块，提供全局配置、上下文管理和异常处理
 *
 * @author WaterRun
 * @file src/Session.php
 * @package Ds4viz
 * @since 2025-12-24
 * @version 1.0.0
 */

namespace Ds4viz;

use Ds4viz\Exception\ErrorType;
use Ds4viz\Exception\StructureException;
use Ds4viz\Trace\Commit;
use Ds4viz\Trace\Error;
use Ds4viz\Trace\Meta;
use Ds4viz\Trace\ObjectInfo;
use Ds4viz\Trace\Package;
use Ds4viz\Trace\Remarks;
use Ds4viz\Trace\Result;
use Ds4viz\Trace\State;
use Ds4viz\Trace\Step;
use Ds4viz\Trace\Trace;
use Throwable;

/**
 * 全局配置类
 *
 * @package Ds4viz
 * @author WaterRun
 * @since 2025-12-24
 */
final class GlobalConfig
{
    /**
     * 默认输出路径
     *
     * @var string
     */
    private const DEFAULT_OUTPUT_PATH = 'trace.toml';

    /**
     * 输出文件路径
     *
     * @var string
     */
    public string $outputPath = self::DEFAULT_OUTPUT_PATH;

    /**
     * 可视化标题
     *
     * @var string
     */
    public string $title = '';

    /**
     * 作者信息
     *
     * @var string
     */
    public string $author = '';

    /**
     * 注释说明
     *
     * @var string
     */
    public string $comment = '';
}

/**
 * 会话管理器，用于管理数据结构的操作轨迹记录
 *
 * @package Ds4viz
 * @author WaterRun
 * @since 2025-12-24
 */
final class Session
{
    /**
     * 状态列表
     *
     * @var array<int, State>
     */
    private array $states = [];

    /**
     * 步骤列表
     *
     * @var array<int, Step>
     */
    private array $steps = [];

    /**
     * 状态计数器
     *
     * @var int
     */
    private int $stateCounter = 0;

    /**
     * 步骤计数器
     *
     * @var int
     */
    private int $stepCounter = 0;

    /**
     * 错误信息
     *
     * @var Error|null
     */
    private ?Error $error = null;

    /**
     * 上下文入口行号
     *
     * @var int
     */
    private int $entryLine = 1;

    /**
     * 上下文退出行号
     *
     * @var int
     */
    private int $exitLine = 1;

    /**
     * 失败步骤 ID
     *
     * @var int|null
     */
    private ?int $failedStepId = null;

    /**
     * 构造函数
     *
     * @param string $kind 数据结构类型
     * @param string $label 数据结构标签
     * @param string $output 输出文件路径
     */
    public function __construct(
        private readonly string $kind,
        private readonly string $label,
        private readonly string $output,
    ) {}

    /**
     * 获取失败步骤 ID
     *
     * @return int|null 失败步骤 ID
     */
    public function getFailedStepId(): ?int
    {
        return $this->failedStepId;
    }

    /**
     * 获取当前步骤计数器
     *
     * @return int 当前步骤计数
     */
    public function getStepCounter(): int
    {
        return $this->stepCounter;
    }

    /**
     * 添加状态快照
     *
     * @param array<string, mixed> $data 状态数据
     * @return int 新状态的 ID
     */
    public function addState(array $data): int
    {
        $stateId = $this->stateCounter;
        $this->states[] = new State(id: $stateId, data: $data);
        $this->stateCounter++;
        return $stateId;
    }

    /**
     * 添加操作步骤
     *
     * @param string $op 操作名称
     * @param int $before 操作前状态 ID
     * @param int|null $after 操作后状态 ID
     * @param array<string, mixed> $args 操作参数
     * @param mixed $ret 返回值
     * @param string|null $note 备注
     * @param int|null $line 代码行号
     * @return int 新步骤的 ID
     */
    public function addStep(
        string $op,
        int $before,
        ?int $after,
        array $args,
        mixed $ret = null,
        ?string $note = null,
        ?int $line = null,
    ): int {
        $stepId = $this->stepCounter;
        $this->steps[] = new Step(
            id: $stepId,
            op: $op,
            before: $before,
            after: $after,
            args: $args,
            line: $line,
            ret: $ret,
            note: $note,
        );
        $this->stepCounter++;
        return $stepId;
    }

    /**
     * 设置错误信息
     *
     * @param ErrorType $errorType 错误类型
     * @param string $message 错误消息
     * @param int|null $line 错误发生的代码行号
     * @param int|null $stepId 发生错误的步骤 ID
     * @return void
     */
    public function setError(
        ErrorType $errorType,
        string $message,
        ?int $line = null,
        ?int $stepId = null,
    ): void {
        $lastState = count($this->states) > 0 ? $this->states[count($this->states) - 1]->id : null;
        $this->error = new Error(
            type: $errorType->value,
            message: mb_substr($message, 0, 512),
            line: $line,
            step: $stepId,
            lastState: $lastState,
        );
        $this->failedStepId = $stepId;
    }

    /**
     * 设置上下文入口行号
     *
     * @param int $line 入口行号
     * @return void
     */
    public function setEntryLine(int $line): void
    {
        $this->entryLine = $line;
    }

    /**
     * 设置上下文退出行号
     *
     * @param int $line 退出行号
     * @return void
     */
    public function setExitLine(int $line): void
    {
        $this->exitLine = $line;
    }

    /**
     * 获取最后一个状态的 ID
     *
     * @return int 最后一个状态的 ID，若无状态则返回 -1
     */
    public function getLastStateId(): int
    {
        return count($this->states) > 0 ? $this->states[count($this->states) - 1]->id : -1;
    }

    /**
     * 设置失败步骤 ID
     *
     * @param int $stepId 步骤 ID
     * @return void
     */
    public function setFailedStepId(int $stepId): void
    {
        $this->failedStepId = $stepId;
    }

    /**
     * 构建完整的 Trace 对象
     *
     * @return Trace 完整的 Trace 对象
     */
    public function buildTrace(): Trace
    {
        $now = gmdate('Y-m-d\TH:i:s\Z');
        $meta = new Meta(
            createdAt: $now,
            lang: 'php',
            langVersion: PHP_VERSION,
        );
        $package = new Package();

        $globalConfig = Ds4viz::getGlobalConfig();
        $remarks = new Remarks(
            title: $globalConfig->title,
            author: $globalConfig->author,
            comment: $globalConfig->comment,
        );

        $object = new ObjectInfo(kind: $this->kind, label: $this->label);

        $result = null;
        $error = null;

        if ($this->error !== null) {
            $error = $this->error;
        } else {
            $finalStateId = count($this->states) > 0 ? $this->states[count($this->states) - 1]->id : 0;
            $result = new Result(
                finalState: $finalStateId,
                commit: new Commit(op: 'commit', line: $this->exitLine),
            );
        }

        return new Trace(
            meta: $meta,
            package: $package,
            remarks: $remarks,
            object: $object,
            states: $this->states,
            steps: $this->steps,
            result: $result,
            error: $error,
        );
    }

    /**
     * 将 Trace 写入文件
     *
     * @return void
     */
    public function write(): void
    {
        $trace = $this->buildTrace();
        $writer = new TomlWriter($trace);
        $writer->write($this->output);
    }
}

/**
 * 数据结构入口类，提供静态工厂方法
 *
 * @package Ds4viz
 * @author WaterRun
 * @since 2025-12-24
 */
final class Ds4viz
{
    /**
     * 全局配置实例
     *
     * @var GlobalConfig
     */
    private static GlobalConfig $globalConfig;

    /**
     * 配置全局参数
     *
     * @param string $outputPath 输出文件路径
     * @param string $title 可视化标题
     * @param string $author 作者信息
     * @param string $comment 注释说明
     * @return void
     */
    public static function config(
        string $outputPath = 'trace.toml',
        string $title = '',
        string $author = '',
        string $comment = '',
    ): void {
        self::$globalConfig = new GlobalConfig();
        self::$globalConfig->outputPath = $outputPath;
        self::$globalConfig->title = $title;
        self::$globalConfig->author = $author;
        self::$globalConfig->comment = $comment;
    }

    /**
     * 获取全局配置
     *
     * @return GlobalConfig 全局配置实例
     */
    public static function getGlobalConfig(): GlobalConfig
    {
        if (!isset(self::$globalConfig)) {
            self::$globalConfig = new GlobalConfig();
        }
        return self::$globalConfig;
    }

    /**
     * 创建栈实例
     *
     * @param string $label 栈的标签
     * @param string|null $output 输出文件路径
     * @return Structures\Stack 栈实例
     */
    public static function stack(string $label = 'stack', ?string $output = null): Structures\Stack
    {
        return new Structures\Stack(label: $label, output: $output);
    }

    /**
     * 创建队列实例
     *
     * @param string $label 队列的标签
     * @param string|null $output 输出文件路径
     * @return Structures\Queue 队列实例
     */
    public static function queue(string $label = 'queue', ?string $output = null): Structures\Queue
    {
        return new Structures\Queue(label: $label, output: $output);
    }

    /**
     * 创建单链表实例
     *
     * @param string $label 单链表的标签
     * @param string|null $output 输出文件路径
     * @return Structures\SingleLinkedList 单链表实例
     */
    public static function singleLinkedList(
        string $label = 'slist',
        ?string $output = null,
    ): Structures\SingleLinkedList {
        return new Structures\SingleLinkedList(label: $label, output: $output);
    }

    /**
     * 创建双向链表实例
     *
     * @param string $label 双向链表的标签
     * @param string|null $output 输出文件路径
     * @return Structures\DoubleLinkedList 双向链表实例
     */
    public static function doubleLinkedList(
        string $label = 'dlist',
        ?string $output = null,
    ): Structures\DoubleLinkedList {
        return new Structures\DoubleLinkedList(label: $label, output: $output);
    }

    /**
     * 创建二叉树实例
     *
     * @param string $label 二叉树的标签
     * @param string|null $output 输出文件路径
     * @return Structures\BinaryTree 二叉树实例
     */
    public static function binaryTree(
        string $label = 'binary_tree',
        ?string $output = null,
    ): Structures\BinaryTree {
        return new Structures\BinaryTree(label: $label, output: $output);
    }

    /**
     * 创建二叉搜索树实例
     *
     * @param string $label 二叉搜索树的标签
     * @param string|null $output 输出文件路径
     * @return Structures\BinarySearchTree 二叉搜索树实例
     */
    public static function binarySearchTree(
        string $label = 'bst',
        ?string $output = null,
    ): Structures\BinarySearchTree {
        return new Structures\BinarySearchTree(label: $label, output: $output);
    }

    /**
     * 创建堆实例
     *
     * @param string $label 堆的标签
     * @param string $heapType 堆类型
     * @param string|null $output 输出文件路径
     * @return Structures\Heap 堆实例
     * @throws \InvalidArgumentException 当 heapType 不是 "min" 或 "max" 时
     */
    public static function heap(
        string $label = 'heap',
        string $heapType = 'min',
        ?string $output = null,
    ): Structures\Heap {
        return new Structures\Heap(label: $label, heapType: $heapType, output: $output);
    }

    /**
     * 创建无向图实例
     *
     * @param string $label 无向图的标签
     * @param string|null $output 输出文件路径
     * @return Structures\GraphUndirected 无向图实例
     */
    public static function graphUndirected(
        string $label = 'graph',
        ?string $output = null,
    ): Structures\GraphUndirected {
        return new Structures\GraphUndirected(label: $label, output: $output);
    }

    /**
     * 创建有向图实例
     *
     * @param string $label 有向图的标签
     * @param string|null $output 输出文件路径
     * @return Structures\GraphDirected 有向图实例
     */
    public static function graphDirected(
        string $label = 'graph',
        ?string $output = null,
    ): Structures\GraphDirected {
        return new Structures\GraphDirected(label: $label, output: $output);
    }

    /**
     * 创建带权图实例
     *
     * @param string $label 带权图的标签
     * @param bool $directed 是否为有向图
     * @param string|null $output 输出文件路径
     * @return Structures\GraphWeighted 带权图实例
     */
    public static function graphWeighted(
        string $label = 'graph',
        bool $directed = false,
        ?string $output = null,
    ): Structures\GraphWeighted {
        return new Structures\GraphWeighted(label: $label, directed: $directed, output: $output);
    }
}

/**
 * 获取调用者的代码行号
 *
 * @param int $depth 调用栈深度
 * @return int 调用者所在行号
 */
function getCallerLine(int $depth = 2): int
{
    $trace = debug_backtrace(DEBUG_BACKTRACE_IGNORE_ARGS, $depth + 1);
    return $trace[$depth]['line'] ?? 1;
}
