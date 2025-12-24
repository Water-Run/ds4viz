<?php

declare(strict_types=1);

/**
 * 通用功能测试
 *
 * @author WaterRun
 * @file test/GeneralTest.php
 * @package Ds4viz\Test
 * @since 2025-12-24
 * @version 1.0.0
 */

namespace Ds4viz\Test;

use Ds4viz\Ds4viz;
use Ds4viz\GlobalConfig;
use Ds4viz\Session;
use Ds4viz\TomlWriter;
use Ds4viz\Exception\ErrorType;
use Ds4viz\Exception\StructureException;
use Ds4viz\Trace\Meta;
use Ds4viz\Trace\Package;
use Ds4viz\Trace\Remarks;
use Ds4viz\Trace\ObjectInfo;
use Ds4viz\Trace\State;
use Ds4viz\Trace\Step;
use Ds4viz\Trace\Commit;
use Ds4viz\Trace\Result;
use Ds4viz\Trace\Error;
use Ds4viz\Trace\Trace;
use PHPUnit\Framework\TestCase;

/**
 * 通用功能测试类
 *
 * @package Ds4viz\Test
 * @author WaterRun
 * @since 2025-12-24
 */
final class GeneralTest extends TestCase
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
        $this->tempFile = sys_get_temp_dir() . '/ds4viz_test_' . uniqid() . '.toml';
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
     * 测试 ErrorType 枚举值
     *
     * @return void
     */
    public function testErrorTypeEnumValues(): void
    {
        $this->assertSame('runtime', ErrorType::RUNTIME->value);
        $this->assertSame('timeout', ErrorType::TIMEOUT->value);
        $this->assertSame('validation', ErrorType::VALIDATION->value);
        $this->assertSame('sandbox', ErrorType::SANDBOX->value);
        $this->assertSame('unknown', ErrorType::UNKNOWN->value);
    }

    /**
     * 测试 ErrorType 枚举数量
     *
     * @return void
     */
    public function testErrorTypeEnumCount(): void
    {
        $this->assertCount(5, ErrorType::cases());
    }

    /**
     * 测试 StructureException 基本功能
     *
     * @return void
     */
    public function testStructureExceptionBasic(): void
    {
        $exception = new StructureException('test message');
        $this->assertSame('test message', $exception->getMessage());
        $this->assertSame('runtime', $exception->getErrorType());
        $this->assertNull($exception->getErrorLine());
    }

    /**
     * 测试 StructureException 带所有参数
     *
     * @return void
     */
    public function testStructureExceptionWithAllParams(): void
    {
        $exception = new StructureException('error msg', 'validation', 42);
        $this->assertSame('error msg', $exception->getMessage());
        $this->assertSame('validation', $exception->getErrorType());
        $this->assertSame(42, $exception->getErrorLine());
    }

    /**
     * 测试 Meta 创建
     *
     * @return void
     */
    public function testMetaCreation(): void
    {
        $meta = new Meta(createdAt: '2025-12-24T00:00:00Z', lang: 'php', langVersion: '8.2.0');
        $this->assertSame('2025-12-24T00:00:00Z', $meta->createdAt);
        $this->assertSame('php', $meta->lang);
        $this->assertSame('8.2.0', $meta->langVersion);
    }

    /**
     * 测试 Meta 默认值
     *
     * @return void
     */
    public function testMetaDefaultValues(): void
    {
        $meta = new Meta(createdAt: '2025-12-24T00:00:00Z');
        $this->assertSame('php', $meta->lang);
        $this->assertSame('', $meta->langVersion);
    }

    /**
     * 测试 Package 创建
     *
     * @return void
     */
    public function testPackageCreation(): void
    {
        $package = new Package();
        $this->assertSame('ds4viz', $package->name);
        $this->assertSame('php', $package->lang);
        $this->assertSame('1.0.0', $package->version);
    }

    /**
     * 测试 Package 自定义值
     *
     * @return void
     */
    public function testPackageCustomValues(): void
    {
        $package = new Package(name: 'custom', lang: 'rust', version: '2.0.0');
        $this->assertSame('custom', $package->name);
        $this->assertSame('rust', $package->lang);
        $this->assertSame('2.0.0', $package->version);
    }

    /**
     * 测试 Remarks isEmpty 方法
     *
     * @return void
     */
    public function testRemarksIsEmpty(): void
    {
        $emptyRemarks = new Remarks();
        $this->assertTrue($emptyRemarks->isEmpty());

        $remarksWithTitle = new Remarks(title: 'Test');
        $this->assertFalse($remarksWithTitle->isEmpty());

        $remarksWithAuthor = new Remarks(author: 'Author');
        $this->assertFalse($remarksWithAuthor->isEmpty());

        $remarksWithComment = new Remarks(comment: 'Comment');
        $this->assertFalse($remarksWithComment->isEmpty());
    }

    /**
     * 测试 ObjectInfo 创建
     *
     * @return void
     */
    public function testObjectInfoCreation(): void
    {
        $obj = new ObjectInfo(kind: 'stack', label: 'my_stack');
        $this->assertSame('stack', $obj->kind);
        $this->assertSame('my_stack', $obj->label);
    }

    /**
     * 测试 State 创建
     *
     * @return void
     */
    public function testStateCreation(): void
    {
        $state = new State(id: 0, data: ['items' => [1, 2, 3], 'top' => 2]);
        $this->assertSame(0, $state->id);
        $this->assertSame(['items' => [1, 2, 3], 'top' => 2], $state->data);
    }

    /**
     * 测试 Step 创建
     *
     * @return void
     */
    public function testStepCreation(): void
    {
        $step = new Step(
            id: 0,
            op: 'push',
            before: 0,
            after: 1,
            args: ['value' => 10],
            line: 5,
            ret: 10,
            note: 'test note',
        );
        $this->assertSame(0, $step->id);
        $this->assertSame('push', $step->op);
        $this->assertSame(0, $step->before);
        $this->assertSame(1, $step->after);
        $this->assertSame(['value' => 10], $step->args);
        $this->assertSame(5, $step->line);
        $this->assertSame(10, $step->ret);
        $this->assertSame('test note', $step->note);
    }

    /**
     * 测试 Commit 创建
     *
     * @return void
     */
    public function testCommitCreation(): void
    {
        $commit = new Commit(op: 'commit', line: 100);
        $this->assertSame('commit', $commit->op);
        $this->assertSame(100, $commit->line);
    }

    /**
     * 测试 Result 创建
     *
     * @return void
     */
    public function testResultCreation(): void
    {
        $result = new Result(finalState: 5, commit: new Commit(op: 'commit', line: 50));
        $this->assertSame(5, $result->finalState);
        $this->assertNotNull($result->commit);
        $this->assertSame(50, $result->commit->line);
    }

    /**
     * 测试 Error 创建
     *
     * @return void
     */
    public function testErrorCreation(): void
    {
        $error = new Error(
            type: 'runtime',
            message: 'test error',
            line: 30,
            step: 2,
            lastState: 1,
        );
        $this->assertSame('runtime', $error->type);
        $this->assertSame('test error', $error->message);
        $this->assertSame(30, $error->line);
        $this->assertSame(2, $error->step);
        $this->assertSame(1, $error->lastState);
    }

    /**
     * 测试 Trace 创建
     *
     * @return void
     */
    public function testTraceCreation(): void
    {
        $trace = new Trace(
            meta: new Meta(createdAt: '2025-12-24T00:00:00Z'),
            package: new Package(),
            remarks: new Remarks(),
            object: new ObjectInfo(kind: 'stack'),
            states: [new State(id: 0, data: ['items' => [], 'top' => -1])],
            steps: [],
            result: new Result(finalState: 0),
            error: null,
        );
        $this->assertSame('stack', $trace->object->kind);
        $this->assertCount(1, $trace->states);
        $this->assertNotNull($trace->result);
        $this->assertNull($trace->error);
    }

    /**
     * 测试全局配置默认值
     *
     * @return void
     */
    public function testDefaultGlobalConfig(): void
    {
        Ds4viz::config();
        $cfg = Ds4viz::getGlobalConfig();
        $this->assertSame('trace.toml', $cfg->outputPath);
        $this->assertSame('', $cfg->title);
        $this->assertSame('', $cfg->author);
        $this->assertSame('', $cfg->comment);
    }

    /**
     * 测试全局配置设置
     *
     * @return void
     */
    public function testGlobalConfigSetting(): void
    {
        Ds4viz::config(
            outputPath: 'custom.toml',
            title: 'Test Title',
            author: 'Test Author',
            comment: 'Test Comment',
        );
        $cfg = Ds4viz::getGlobalConfig();
        $this->assertSame('custom.toml', $cfg->outputPath);
        $this->assertSame('Test Title', $cfg->title);
        $this->assertSame('Test Author', $cfg->author);
        $this->assertSame('Test Comment', $cfg->comment);
    }

    /**
     * 测试 Session 初始化
     *
     * @return void
     */
    public function testSessionInit(): void
    {
        $session = new Session(kind: 'stack', label: 'test_stack', output: $this->tempFile);
        $this->assertSame(0, $session->getStepCounter());
        $this->assertNull($session->getFailedStepId());
        $this->assertSame(-1, $session->getLastStateId());
    }

    /**
     * 测试 Session 添加状态
     *
     * @return void
     */
    public function testSessionAddState(): void
    {
        $session = new Session(kind: 'stack', label: 'test', output: $this->tempFile);
        $stateId = $session->addState(['items' => [], 'top' => -1]);
        $this->assertSame(0, $stateId);
        $this->assertSame(0, $session->getLastStateId());

        $stateId2 = $session->addState(['items' => [10], 'top' => 0]);
        $this->assertSame(1, $stateId2);
        $this->assertSame(1, $session->getLastStateId());
    }

    /**
     * 测试 Session 添加步骤
     *
     * @return void
     */
    public function testSessionAddStep(): void
    {
        $session = new Session(kind: 'stack', label: 'test', output: $this->tempFile);
        $session->addState(['items' => [], 'top' => -1]);
        $session->addState(['items' => [10], 'top' => 0]);

        $stepId = $session->addStep(
            op: 'push',
            before: 0,
            after: 1,
            args: ['value' => 10],
            ret: null,
            note: 'pushed 10',
            line: 5,
        );
        $this->assertSame(0, $stepId);
        $this->assertSame(1, $session->getStepCounter());
    }

    /**
     * 测试 Session 设置错误
     *
     * @return void
     */
    public function testSessionSetError(): void
    {
        $session = new Session(kind: 'stack', label: 'test', output: $this->tempFile);
        $session->addState(['items' => [], 'top' => -1]);
        $session->setError(
            errorType: ErrorType::RUNTIME,
            message: 'test error',
            line: 10,
            stepId: 0,
        );
        $this->assertSame(0, $session->getFailedStepId());
    }

    /**
     * 测试 Session 构建成功的 Trace
     *
     * @return void
     */
    public function testSessionBuildTraceSuccess(): void
    {
        $session = new Session(kind: 'stack', label: 'test_stack', output: $this->tempFile);
        $session->setEntryLine(1);
        $session->setExitLine(10);
        $session->addState(['items' => [], 'top' => -1]);
        $session->addState(['items' => [10], 'top' => 0]);
        $session->addStep(op: 'push', before: 0, after: 1, args: ['value' => 10], line: 5);

        $trace = $session->buildTrace();
        $this->assertSame('stack', $trace->object->kind);
        $this->assertSame('test_stack', $trace->object->label);
        $this->assertCount(2, $trace->states);
        $this->assertCount(1, $trace->steps);
        $this->assertNotNull($trace->result);
        $this->assertSame(1, $trace->result->finalState);
        $this->assertNull($trace->error);
    }

    /**
     * 测试 Session 构建带错误的 Trace
     *
     * @return void
     */
    public function testSessionBuildTraceError(): void
    {
        $session = new Session(kind: 'stack', label: 'test_stack', output: $this->tempFile);
        $session->addState(['items' => [], 'top' => -1]);
        $session->setError(
            errorType: ErrorType::RUNTIME,
            message: 'Cannot pop from empty stack',
            line: 10,
            stepId: null,
        );

        $trace = $session->buildTrace();
        $this->assertNull($trace->result);
        $this->assertNotNull($trace->error);
        $this->assertSame('runtime', $trace->error->type);
        $this->assertSame('Cannot pop from empty stack', $trace->error->message);
    }

    /**
     * 测试 TomlWriter 字符串转义
     *
     * @return void
     */
    public function testTomlWriterEscapeString(): void
    {
        $trace = new Trace(
            meta: new Meta(createdAt: '2025-12-24T00:00:00Z'),
            package: new Package(),
            remarks: new Remarks(title: 'Test "Quote"'),
            object: new ObjectInfo(kind: 'stack'),
            states: [new State(id: 0, data: ['items' => [], 'top' => -1])],
            steps: [],
            result: new Result(finalState: 0),
        );
        $writer = new TomlWriter($trace);
        $writer->write($this->tempFile);

        $content = file_get_contents($this->tempFile);
        $this->assertStringContainsString('title = "Test \\"Quote\\""', $content);
    }

    /**
     * 测试 TomlWriter 序列化布尔值
     *
     * @return void
     */
    public function testTomlWriterSerializeBool(): void
    {
        $trace = new Trace(
            meta: new Meta(createdAt: '2025-12-24T00:00:00Z'),
            package: new Package(),
            remarks: new Remarks(),
            object: new ObjectInfo(kind: 'stack'),
            states: [new State(id: 0, data: ['items' => [true, false], 'top' => 1])],
            steps: [],
            result: new Result(finalState: 0),
        );
        $writer = new TomlWriter($trace);
        $writer->write($this->tempFile);

        $content = file_get_contents($this->tempFile);
        $this->assertStringContainsString('items = [true, false]', $content);
    }

    /**
     * 测试 TomlWriter 序列化浮点数
     *
     * @return void
     */
    public function testTomlWriterSerializeFloat(): void
    {
        $trace = new Trace(
            meta: new Meta(createdAt: '2025-12-24T00:00:00Z'),
            package: new Package(),
            remarks: new Remarks(),
            object: new ObjectInfo(kind: 'stack'),
            states: [new State(id: 0, data: ['items' => [3.14, 0.0], 'top' => 1])],
            steps: [],
            result: new Result(finalState: 0),
        );
        $writer = new TomlWriter($trace);
        $writer->write($this->tempFile);

        $content = file_get_contents($this->tempFile);
        $this->assertStringContainsString('3.14', $content);
    }

    /**
     * 测试 TomlWriter 写入文件
     *
     * @return void
     */
    public function testTomlWriterWriteToFile(): void
    {
        $trace = new Trace(
            meta: new Meta(createdAt: '2025-12-24T00:00:00Z', lang: 'php', langVersion: PHP_VERSION),
            package: new Package(),
            remarks: new Remarks(),
            object: new ObjectInfo(kind: 'stack'),
            states: [new State(id: 0, data: ['items' => [], 'top' => -1])],
            steps: [],
            result: new Result(finalState: 0),
        );
        $writer = new TomlWriter($trace);
        $writer->write($this->tempFile);

        $this->assertFileExists($this->tempFile);
        $content = file_get_contents($this->tempFile);
        $this->assertStringContainsString('[meta]', $content);
        $this->assertStringContainsString('[package]', $content);
        $this->assertStringContainsString('[object]', $content);
        $this->assertStringContainsString('[[states]]', $content);
        $this->assertStringContainsString('[result]', $content);
    }

    /**
     * 测试 TomlWriter 带 remarks 写入
     *
     * @return void
     */
    public function testTomlWriterWithRemarks(): void
    {
        $trace = new Trace(
            meta: new Meta(createdAt: '2025-12-24T00:00:00Z'),
            package: new Package(),
            remarks: new Remarks(title: 'Test', author: 'Author', comment: 'Comment'),
            object: new ObjectInfo(kind: 'stack'),
            states: [new State(id: 0, data: ['items' => [], 'top' => -1])],
            steps: [],
            result: new Result(finalState: 0),
        );
        $writer = new TomlWriter($trace);
        $writer->write($this->tempFile);

        $content = file_get_contents($this->tempFile);
        $this->assertStringContainsString('[remarks]', $content);
        $this->assertStringContainsString('title = "Test"', $content);
        $this->assertStringContainsString('author = "Author"', $content);
        $this->assertStringContainsString('comment = "Comment"', $content);
    }

    /**
     * 测试 TomlWriter 带 error 写入
     *
     * @return void
     */
    public function testTomlWriterWithError(): void
    {
        $trace = new Trace(
            meta: new Meta(createdAt: '2025-12-24T00:00:00Z'),
            package: new Package(),
            remarks: new Remarks(),
            object: new ObjectInfo(kind: 'stack'),
            states: [new State(id: 0, data: ['items' => [], 'top' => -1])],
            steps: [],
            result: null,
            error: new Error(type: 'runtime', message: 'Error occurred', line: 5),
        );
        $writer = new TomlWriter($trace);
        $writer->write($this->tempFile);

        $content = file_get_contents($this->tempFile);
        $this->assertStringContainsString('[error]', $content);
        $this->assertStringContainsString('type = "runtime"', $content);
        $this->assertStringContainsString('message = "Error occurred"', $content);
        $this->assertStringNotContainsString('[result]', $content);
    }

    /**
     * 测试工厂函数存在性
     *
     * @return void
     */
    public function testFactoryFunctionsExist(): void
    {
        $this->assertTrue(method_exists(Ds4viz::class, 'stack'));
        $this->assertTrue(method_exists(Ds4viz::class, 'queue'));
        $this->assertTrue(method_exists(Ds4viz::class, 'singleLinkedList'));
        $this->assertTrue(method_exists(Ds4viz::class, 'doubleLinkedList'));
        $this->assertTrue(method_exists(Ds4viz::class, 'binaryTree'));
        $this->assertTrue(method_exists(Ds4viz::class, 'binarySearchTree'));
        $this->assertTrue(method_exists(Ds4viz::class, 'heap'));
        $this->assertTrue(method_exists(Ds4viz::class, 'graphUndirected'));
        $this->assertTrue(method_exists(Ds4viz::class, 'graphDirected'));
        $this->assertTrue(method_exists(Ds4viz::class, 'graphWeighted'));
        $this->assertTrue(method_exists(Ds4viz::class, 'config'));
    }

    /**
     * 测试配置只设置标题
     *
     * @return void
     */
    public function testConfigOnlyTitle(): void
    {
        Ds4viz::config(title: 'Only Title');
        $cfg = Ds4viz::getGlobalConfig();
        $this->assertSame('Only Title', $cfg->title);
        $this->assertSame('trace.toml', $cfg->outputPath);
    }

    /**
     * 测试工厂函数返回类型
     *
     * @return void
     */
    public function testFactoryFunctionsReturnTypes(): void
    {
        $this->assertInstanceOf(\Ds4viz\Structures\Stack::class, Ds4viz::stack());
        $this->assertInstanceOf(\Ds4viz\Structures\Queue::class, Ds4viz::queue());
        $this->assertInstanceOf(\Ds4viz\Structures\SingleLinkedList::class, Ds4viz::singleLinkedList());
        $this->assertInstanceOf(\Ds4viz\Structures\DoubleLinkedList::class, Ds4viz::doubleLinkedList());
        $this->assertInstanceOf(\Ds4viz\Structures\BinaryTree::class, Ds4viz::binaryTree());
        $this->assertInstanceOf(\Ds4viz\Structures\BinarySearchTree::class, Ds4viz::binarySearchTree());
        $this->assertInstanceOf(\Ds4viz\Structures\Heap::class, Ds4viz::heap());
        $this->assertInstanceOf(\Ds4viz\Structures\GraphUndirected::class, Ds4viz::graphUndirected());
        $this->assertInstanceOf(\Ds4viz\Structures\GraphDirected::class, Ds4viz::graphDirected());
        $this->assertInstanceOf(\Ds4viz\Structures\GraphWeighted::class, Ds4viz::graphWeighted());
    }
}
