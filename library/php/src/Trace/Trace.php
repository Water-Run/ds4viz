<?php

declare(strict_types=1);

/**
 * Trace 数据结构定义，用于记录操作轨迹
 *
 * @author WaterRun
 * @file src/Trace.php
 * @package Ds4viz\Trace
 * @since 2025-12-24
 * @version 1.0.0
 */

namespace Ds4viz\Trace;

/**
 * 元数据信息
 *
 * @package Ds4viz\Trace
 * @author WaterRun
 * @since 2025-12-24
 */
final readonly class Meta
{
    /**
     * 构造函数
     *
     * @param string $createdAt 创建时间
     * @param string $lang 语言标识
     * @param string $langVersion 语言版本
     */
    public function __construct(
        public string $createdAt,
        public string $lang = 'php',
        public string $langVersion = '',
    ) {}
}

/**
 * 包信息
 *
 * @package Ds4viz\Trace
 * @author WaterRun
 * @since 2025-12-24
 */
final readonly class Package
{
    /**
     * 构造函数
     *
     * @param string $name 包名
     * @param string $lang 语言标识
     * @param string $version 包版本
     */
    public function __construct(
        public string $name = 'ds4viz',
        public string $lang = 'php',
        public string $version = '1.0.0',
    ) {}
}

/**
 * 备注信息
 *
 * @package Ds4viz\Trace
 * @author WaterRun
 * @since 2025-12-24
 */
final readonly class Remarks
{
    /**
     * 构造函数
     *
     * @param string $title 标题
     * @param string $author 作者
     * @param string $comment 注释
     */
    public function __construct(
        public string $title = '',
        public string $author = '',
        public string $comment = '',
    ) {}

    /**
     * 检查备注是否为空
     *
     * @return bool 如果所有字段都为空则返回 true
     */
    public function isEmpty(): bool
    {
        return $this->title === '' && $this->author === '' && $this->comment === '';
    }
}

/**
 * 数据结构对象描述
 *
 * @package Ds4viz\Trace
 * @author WaterRun
 * @since 2025-12-24
 */
final readonly class ObjectInfo
{
    /**
     * 构造函数
     *
     * @param string $kind 数据结构类型
     * @param string $label 标签
     */
    public function __construct(
        public string $kind,
        public string $label = '',
    ) {}
}

/**
 * 状态快照
 *
 * @package Ds4viz\Trace
 * @author WaterRun
 * @since 2025-12-24
 */
final readonly class State
{
    /**
     * 构造函数
     *
     * @param int $id 状态 ID
     * @param array<string, mixed> $data 状态数据
     */
    public function __construct(
        public int $id,
        public array $data = [],
    ) {}
}

/**
 * 操作步骤
 *
 * @package Ds4viz\Trace
 * @author WaterRun
 * @since 2025-12-24
 */
final readonly class Step
{
    /**
     * 构造函数
     *
     * @param int $id 步骤 ID
     * @param string $op 操作名称
     * @param int $before 操作前状态 ID
     * @param int|null $after 操作后状态 ID
     * @param array<string, mixed> $args 操作参数
     * @param int|null $line 代码行号
     * @param mixed $ret 返回值
     * @param string|null $note 备注
     */
    public function __construct(
        public int $id,
        public string $op,
        public int $before,
        public ?int $after = null,
        public array $args = [],
        public ?int $line = null,
        public mixed $ret = null,
        public ?string $note = null,
    ) {}
}

/**
 * 提交信息
 *
 * @package Ds4viz\Trace
 * @author WaterRun
 * @since 2025-12-24
 */
final readonly class Commit
{
    /**
     * 构造函数
     *
     * @param string $op 操作名称
     * @param int $line 代码行号
     */
    public function __construct(
        public string $op,
        public int $line,
    ) {}
}

/**
 * 成功结果
 *
 * @package Ds4viz\Trace
 * @author WaterRun
 * @since 2025-12-24
 */
final readonly class Result
{
    /**
     * 构造函数
     *
     * @param int $finalState 最终状态 ID
     * @param Commit|null $commit 提交信息
     */
    public function __construct(
        public int $finalState,
        public ?Commit $commit = null,
    ) {}
}

/**
 * 错误信息
 *
 * @package Ds4viz\Trace
 * @author WaterRun
 * @since 2025-12-24
 */
final readonly class Error
{
    /**
     * 构造函数
     *
     * @param string $type 错误类型
     * @param string $message 错误消息
     * @param int|null $line 错误行号
     * @param int|null $step 错误步骤 ID
     * @param int|null $lastState 最后状态 ID
     */
    public function __construct(
        public string $type,
        public string $message,
        public ?int $line = null,
        public ?int $step = null,
        public ?int $lastState = null,
    ) {}
}

/**
 * 完整的操作轨迹记录
 *
 * @package Ds4viz\Trace
 * @author WaterRun
 * @since 2025-12-24
 */
final readonly class Trace
{
    /**
     * 构造函数
     *
     * @param Meta $meta 元数据
     * @param Package $package 包信息
     * @param Remarks $remarks 备注信息
     * @param ObjectInfo $object 对象信息
     * @param array<int, State> $states 状态列表
     * @param array<int, Step> $steps 步骤列表
     * @param Result|null $result 成功结果
     * @param Error|null $error 错误信息
     */
    public function __construct(
        public Meta $meta,
        public Package $package,
        public Remarks $remarks,
        public ObjectInfo $object,
        public array $states = [],
        public array $steps = [],
        public ?Result $result = null,
        public ?Error $error = null,
    ) {}
}
