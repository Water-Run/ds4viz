<?php

declare(strict_types=1);

/**
 * 自定义异常类型定义
 *
 * @author WaterRun
 * @file src/Exception/StructureException.php
 * @package Ds4viz\Exception
 * @since 2025-12-24
 * @version 1.0.0
 */

namespace Ds4viz\Exception;

use Exception;

/**
 * 错误类型枚举，对应 IR 定义中的 error.type
 *
 * @package Ds4viz\Exception
 * @author WaterRun
 * @since 2025-12-24
 */
enum ErrorType: string
{
    /**
     * 运行时错误
     */
    case RUNTIME = 'runtime';

    /**
     * 超时错误
     */
    case TIMEOUT = 'timeout';

    /**
     * 验证错误
     */
    case VALIDATION = 'validation';

    /**
     * 沙箱错误
     */
    case SANDBOX = 'sandbox';

    /**
     * 未知错误
     */
    case UNKNOWN = 'unknown';
}

/**
 * 数据结构操作错误
 *
 * @package Ds4viz\Exception
 * @author WaterRun
 * @since 2025-12-24
 */
class StructureException extends Exception
{
    /**
     * 构造函数
     *
     * @param string $message 错误消息
     * @param string $errorType 错误类型
     * @param int|null $errorLine 错误行号
     */
    public function __construct(
        string $message,
        private readonly string $errorType = 'runtime',
        private readonly ?int $errorLine = null,
    ) {
        parent::__construct($message);
    }

    /**
     * 获取错误类型
     *
     * @return string 错误类型
     */
    public function getErrorType(): string
    {
        return $this->errorType;
    }

    /**
     * 获取错误行号
     *
     * @return int|null 错误行号
     */
    public function getErrorLine(): ?int
    {
        return $this->errorLine;
    }
}
