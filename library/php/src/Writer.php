<?php

declare(strict_types=1);

/**
 * TOML 文件写入器，将 Trace 对象序列化为 TOML 格式
 *
 * @author WaterRun
 * @file src/Writer.php
 * @package Ds4viz
 * @since 2025-12-24
 * @version 1.0.0
 */

namespace Ds4viz;

use Ds4viz\Trace\State;
use Ds4viz\Trace\Step;
use Ds4viz\Trace\Trace;

/**
 * TOML 格式写入器
 *
 * @package Ds4viz
 * @author WaterRun
 * @since 2025-12-24
 */
final class TomlWriter
{
    /**
     * 构造函数
     *
     * @param Trace $trace 要序列化的 Trace 对象
     */
    public function __construct(
        private readonly Trace $trace,
    ) {}

    /**
     * 将 Trace 写入指定路径的 TOML 文件
     *
     * @param string $path 输出文件路径
     * @return void
     */
    public function write(string $path): void
    {
        $content = $this->serialize();
        file_put_contents($path, $content);
    }

    /**
     * 将 Trace 序列化为 TOML 字符串
     *
     * @return string TOML 格式的字符串
     */
    private function serialize(): string
    {
        $lines = [];
        $lines = array_merge($lines, $this->serializeMeta());
        $lines[] = '';
        $lines = array_merge($lines, $this->serializePackage());

        if (!$this->trace->remarks->isEmpty()) {
            $lines[] = '';
            $lines = array_merge($lines, $this->serializeRemarks());
        }

        $lines[] = '';
        $lines = array_merge($lines, $this->serializeObject());

        foreach ($this->trace->states as $state) {
            $lines[] = '';
            $lines = array_merge($lines, $this->serializeState($state));
        }

        foreach ($this->trace->steps as $step) {
            $lines[] = '';
            $lines = array_merge($lines, $this->serializeStep($step));
        }

        if ($this->trace->result !== null) {
            $lines[] = '';
            $lines = array_merge($lines, $this->serializeResult());
        }

        if ($this->trace->error !== null) {
            $lines[] = '';
            $lines = array_merge($lines, $this->serializeError());
        }

        return implode("\n", $lines);
    }

    /**
     * 序列化 meta 部分
     *
     * @return array<int, string> TOML 行列表
     */
    private function serializeMeta(): array
    {
        return [
            '[meta]',
            sprintf('created_at = "%s"', $this->trace->meta->createdAt),
            sprintf('lang = "%s"', $this->trace->meta->lang),
            sprintf('lang_version = "%s"', $this->trace->meta->langVersion),
        ];
    }

    /**
     * 序列化 package 部分
     *
     * @return array<int, string> TOML 行列表
     */
    private function serializePackage(): array
    {
        return [
            '[package]',
            sprintf('name = "%s"', $this->trace->package->name),
            sprintf('lang = "%s"', $this->trace->package->lang),
            sprintf('version = "%s"', $this->trace->package->version),
        ];
    }

    /**
     * 序列化 remarks 部分
     *
     * @return array<int, string> TOML 行列表
     */
    private function serializeRemarks(): array
    {
        $lines = ['[remarks]'];

        if ($this->trace->remarks->title !== '') {
            $lines[] = sprintf('title = "%s"', $this->escapeString($this->trace->remarks->title));
        }
        if ($this->trace->remarks->author !== '') {
            $lines[] = sprintf('author = "%s"', $this->escapeString($this->trace->remarks->author));
        }
        if ($this->trace->remarks->comment !== '') {
            $lines[] = sprintf('comment = "%s"', $this->escapeString($this->trace->remarks->comment));
        }

        return $lines;
    }

    /**
     * 序列化 object 部分
     *
     * @return array<int, string> TOML 行列表
     */
    private function serializeObject(): array
    {
        $lines = [
            '[object]',
            sprintf('kind = "%s"', $this->trace->object->kind),
        ];

        if ($this->trace->object->label !== '') {
            $lines[] = sprintf('label = "%s"', $this->escapeString($this->trace->object->label));
        }

        return $lines;
    }

    /**
     * 序列化单个 state
     *
     * @param State $state 状态对象
     * @return array<int, string> TOML 行列表
     */
    private function serializeState(State $state): array
    {
        $lines = [
            '[[states]]',
            sprintf('id = %d', $state->id),
            '',
            '[states.data]',
        ];

        $lines = array_merge($lines, $this->serializeStateData($state->data));

        return $lines;
    }

    /**
     * 序列化 state.data 部分
     *
     * @param array<string, mixed> $data 状态数据字典
     * @return array<int, string> TOML 行列表
     */
    private function serializeStateData(array $data): array
    {
        $lines = [];

        foreach ($data as $key => $value) {
            if ($key === 'nodes' || $key === 'edges') {
                $lines[] = sprintf('%s = %s', $key, $this->serializeInlineTableArray($value));
            } elseif ($key === 'items') {
                $lines[] = sprintf('%s = %s', $key, $this->serializeSimpleArray($value));
            } else {
                $lines[] = sprintf('%s = %s', $key, $this->serializeValue($value));
            }
        }

        return $lines;
    }

    /**
     * 序列化 inline table 数组
     *
     * @param array<int, array<string, mixed>> $arr 字典数组
     * @return string TOML 格式的 inline table 数组字符串
     */
    private function serializeInlineTableArray(array $arr): string
    {
        if (count($arr) === 0) {
            return '[]';
        }

        $items = [];
        foreach ($arr as $item) {
            $pairs = [];
            foreach ($item as $k => $v) {
                $pairs[] = sprintf('%s = %s', $k, $this->serializeValue($v));
            }
            $items[] = '{ ' . implode(', ', $pairs) . ' }';
        }

        return "[\n  " . implode(",\n  ", $items) . "\n]";
    }

    /**
     * 序列化简单数组
     *
     * @param array<int, mixed> $arr 值数组
     * @return string TOML 格式的数组字符串
     */
    private function serializeSimpleArray(array $arr): string
    {
        if (count($arr) === 0) {
            return '[]';
        }

        $items = array_map(fn($v) => $this->serializeValue($v), $arr);
        return '[' . implode(', ', $items) . ']';
    }

    /**
     * 序列化单个 step
     *
     * @param Step $step 步骤对象
     * @return array<int, string> TOML 行列表
     */
    private function serializeStep(Step $step): array
    {
        $lines = [
            '[[steps]]',
            sprintf('id = %d', $step->id),
            sprintf('op = "%s"', $step->op),
            sprintf('before = %d', $step->before),
        ];

        if ($step->after !== null) {
            $lines[] = sprintf('after = %d', $step->after);
        }

        if ($step->ret !== null) {
            $lines[] = sprintf('ret = %s', $this->serializeValue($step->ret));
        }

        if ($step->note !== null) {
            $lines[] = sprintf('note = "%s"', $this->escapeString($step->note));
        }

        $lines[] = '';
        $lines[] = '[steps.args]';

        foreach ($step->args as $key => $value) {
            $lines[] = sprintf('%s = %s', $key, $this->serializeValue($value));
        }

        if ($step->line !== null) {
            $lines[] = '';
            $lines[] = '[steps.code]';
            $lines[] = sprintf('line = %d', $step->line);
        }

        return $lines;
    }

    /**
     * 序列化 result 部分
     *
     * @return array<int, string> TOML 行列表
     */
    private function serializeResult(): array
    {
        $result = $this->trace->result;
        if ($result === null) {
            return [];
        }

        $lines = [
            '[result]',
            sprintf('final_state = %d', $result->finalState),
        ];

        if ($result->commit !== null) {
            $lines[] = '';
            $lines[] = '[result.commit]';
            $lines[] = sprintf('op = "%s"', $result->commit->op);
            $lines[] = sprintf('line = %d', $result->commit->line);
        }

        return $lines;
    }

    /**
     * 序列化 error 部分
     *
     * @return array<int, string> TOML 行列表
     */
    private function serializeError(): array
    {
        $error = $this->trace->error;
        if ($error === null) {
            return [];
        }

        $lines = [
            '[error]',
            sprintf('type = "%s"', $error->type),
            sprintf('message = "%s"', $this->escapeString($error->message)),
        ];

        if ($error->line !== null) {
            $lines[] = sprintf('line = %d', $error->line);
        }
        if ($error->step !== null) {
            $lines[] = sprintf('step = %d', $error->step);
        }
        if ($error->lastState !== null) {
            $lines[] = sprintf('last_state = %d', $error->lastState);
        }

        return $lines;
    }

    /**
     * 序列化单个值
     *
     * @param mixed $value 要序列化的值
     * @return string TOML 格式的值字符串
     */
    private function serializeValue(mixed $value): string
    {
        return match (true) {
            is_bool($value) => $value ? 'true' : 'false',
            is_int($value) => (string) $value,
            is_float($value) => $this->formatFloat($value),
            is_string($value) => sprintf('"%s"', $this->escapeString($value)),
            is_array($value) && array_is_list($value) => $this->serializeSimpleArray($value),
            is_array($value) => $this->serializeInlineTable($value),
            default => sprintf('"%s"', $this->escapeString((string) $value)),
        };
    }

    /**
     * 序列化 inline table
     *
     * @param array<string, mixed> $arr 关联数组
     * @return string TOML 格式的 inline table 字符串
     */
    private function serializeInlineTable(array $arr): string
    {
        $pairs = [];
        foreach ($arr as $k => $v) {
            $pairs[] = sprintf('%s = %s', $k, $this->serializeValue($v));
        }
        return '{ ' . implode(', ', $pairs) . ' }';
    }

    /**
     * 格式化浮点数
     *
     * @param float $value 浮点数值
     * @return string 格式化后的字符串
     */
    private function formatFloat(float $value): string
    {
        $str = (string) $value;
        if (!str_contains($str, '.') && !str_contains($str, 'e') && !str_contains($str, 'E')) {
            $str .= '.0';
        }
        return $str;
    }

    /**
     * 转义字符串中的特殊字符
     *
     * @param string $s 原始字符串
     * @return string 转义后的字符串
     */
    private function escapeString(string $s): string
    {
        return str_replace(
            ['\\', '"', "\n", "\r"],
            ['\\\\', '\\"', '\\n', '\\r'],
            $s,
        );
    }
}
