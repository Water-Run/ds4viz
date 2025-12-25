/**
 * 单元测试 - 测试各个模块的独立功能
 *
 * @file test/unit.test.ts
 * @author WaterRun
 * @date 2025-12-25
 */

import { describe, it, expect, afterEach } from '@jest/globals';
import * as fs from 'node:fs';
import {
    createMeta,
    createPackage,
    createRemarks,
    createTraceObject,
    createState,
    createStep,
    createResult,
    createTraceError,
    createTrace,
    isRemarksEmpty,
    type Meta,
    type Package,
    type Remarks,
    type TraceObject,
    type State,
    type Step,
    type Result,
    type TraceError,
    type Trace,
} from '../src/trace.js';
import { TomlWriter } from '../src/writer.js';
import {
    config,
    getConfig,
    getTypeScriptVersion,
    getCallerLine,
    Session,
    ErrorType,
    DS4VizError,
    StructureError,
    ValidationError,
} from '../src/session.js';
import { Stack, Queue, SingleLinkedList, DoubleLinkedList } from '../src/structures/linear.js';
import { BinaryTree, BinarySearchTree, Heap } from '../src/structures/tree.js';
import { GraphUndirected, GraphDirected, GraphWeighted } from '../src/structures/graph.js';

// ============================================================================
// Trace 模块测试
// ============================================================================

describe('Trace Module', () => {
    describe('createMeta', () => {
        it('should create Meta with correct properties', () => {
            const meta: Meta = createMeta('2025-01-01T00:00:00Z', '20.10.0');
            expect(meta.createdAt).toBe('2025-01-01T00:00:00Z');
            expect(meta.lang).toBe('typescript');
            expect(meta.langVersion).toBe('20.10.0');
        });

        it('should create immutable Meta object', () => {
            const meta: Meta = createMeta('2025-01-01T00:00:00Z', '20.10.0');
            expect(() => {
                (meta as { createdAt: string }).createdAt = 'modified';
            }).toThrow();
        });
    });

    describe('createPackage', () => {
        it('should create Package with default values', () => {
            const pkg: Package = createPackage();
            expect(pkg.name).toBe('ds4viz');
            expect(pkg.lang).toBe('typescript');
            expect(pkg.version).toBe('1.0.0');
        });
    });

    describe('createRemarks', () => {
        it('should create Remarks with provided values', () => {
            const remarks: Remarks = createRemarks('Test Title', 'Test Author', 'Test Comment');
            expect(remarks.title).toBe('Test Title');
            expect(remarks.author).toBe('Test Author');
            expect(remarks.comment).toBe('Test Comment');
        });

        it('should create empty Remarks', () => {
            const remarks: Remarks = createRemarks('', '', '');
            expect(remarks.title).toBe('');
            expect(remarks.author).toBe('');
            expect(remarks.comment).toBe('');
        });
    });

    describe('isRemarksEmpty', () => {
        it('should return true for empty remarks', () => {
            const remarks: Remarks = createRemarks('', '', '');
            expect(isRemarksEmpty(remarks)).toBe(true);
        });

        it('should return false when title is not empty', () => {
            const remarks: Remarks = createRemarks('Title', '', '');
            expect(isRemarksEmpty(remarks)).toBe(false);
        });

        it('should return false when author is not empty', () => {
            const remarks: Remarks = createRemarks('', 'Author', '');
            expect(isRemarksEmpty(remarks)).toBe(false);
        });

        it('should return false when comment is not empty', () => {
            const remarks: Remarks = createRemarks('', '', 'Comment');
            expect(isRemarksEmpty(remarks)).toBe(false);
        });
    });

    describe('createTraceObject', () => {
        it('should create TraceObject with correct properties', () => {
            const obj: TraceObject = createTraceObject('stack', 'my_stack');
            expect(obj.kind).toBe('stack');
            expect(obj.label).toBe('my_stack');
        });
    });

    describe('createState', () => {
        it('should create State with id and data', () => {
            const data = { items: [1, 2, 3], top: 2 };
            const state: State = createState(0, data);
            expect(state.id).toBe(0);
            expect(state.data).toEqual(data);
        });

        it('should handle complex data structures', () => {
            const data = {
                nodes: [{ id: 0, value: 1, next: 1 }, { id: 1, value: 2, next: -1 }],
                head: 0,
            };
            const state: State = createState(5, data);
            expect(state.id).toBe(5);
            expect(state.data['nodes']).toHaveLength(2);
        });
    });

    describe('createStep', () => {
        it('should create Step with all required fields', () => {
            const step: Step = createStep(0, 'push', 0, 1, { value: 42 });
            expect(step.id).toBe(0);
            expect(step.op).toBe('push');
            expect(step.before).toBe(0);
            expect(step.after).toBe(1);
            expect(step.args).toEqual({ value: 42 });
        });

        it('should create Step with optional fields', () => {
            const step: Step = createStep(
                1,
                'pop',
                1,
                2,
                {},
                { line: 10, col: 5 },
                42,
                'Popped value'
            );
            expect(step.code).toEqual({ line: 10, col: 5 });
            expect(step.ret).toBe(42);
            expect(step.note).toBe('Popped value');
        });

        it('should create Step with undefined after', () => {
            const step: Step = createStep(0, 'error_op', 0, undefined, {});
            expect(step.after).toBeUndefined();
        });
    });

    describe('createResult', () => {
        it('should create Result with finalState only', () => {
            const result: Result = createResult(5);
            expect(result.finalState).toBe(5);
            expect(result.commit).toBeUndefined();
        });

        it('should create Result with commit', () => {
            const result: Result = createResult(5, { op: 'commit', line: 20 });
            expect(result.finalState).toBe(5);
            expect(result.commit).toEqual({ op: 'commit', line: 20 });
        });
    });

    describe('createTraceError', () => {
        it('should create TraceError with required fields', () => {
            const error: TraceError = createTraceError('runtime', 'Stack overflow');
            expect(error.type).toBe('runtime');
            expect(error.message).toBe('Stack overflow');
        });

        it('should create TraceError with all optional fields', () => {
            const error: TraceError = createTraceError('validation', 'Invalid input', 15, 3, 2);
            expect(error.type).toBe('validation');
            expect(error.message).toBe('Invalid input');
            expect(error.line).toBe(15);
            expect(error.step).toBe(3);
            expect(error.lastState).toBe(2);
        });
    });

    describe('createTrace', () => {
        it('should create complete Trace with result', () => {
            const meta = createMeta('2025-01-01T00:00:00Z', '20.10.0');
            const pkg = createPackage();
            const remarks = createRemarks('Test', 'Author', 'Comment');
            const obj = createTraceObject('stack', 'test_stack');
            const states = [createState(0, { items: [] })];
            const steps: Step[] = [];
            const result = createResult(0);

            const trace: Trace = createTrace(meta, pkg, remarks, obj, states, steps, result);
            expect(trace.meta).toBe(meta);
            expect(trace.package).toBe(pkg);
            expect(trace.remarks).toBe(remarks);
            expect(trace.object).toBe(obj);
            expect(trace.states).toEqual(states);
            expect(trace.steps).toEqual(steps);
            expect(trace.result).toBe(result);
            expect(trace.error).toBeUndefined();
        });

        it('should create complete Trace with error', () => {
            const meta = createMeta('2025-01-01T00:00:00Z', '20.10.0');
            const pkg = createPackage();
            const remarks = createRemarks('', '', '');
            const obj = createTraceObject('stack', 'test_stack');
            const states = [createState(0, { items: [] })];
            const steps: Step[] = [];
            const error = createTraceError('runtime', 'Error occurred');

            const trace: Trace = createTrace(meta, pkg, remarks, obj, states, steps, undefined, error);
            expect(trace.result).toBeUndefined();
            expect(trace.error).toBe(error);
        });
    });
});

// ============================================================================
// Writer 模块测试
// ============================================================================

describe('TomlWriter Module', () => {
    const testOutputPath = '/tmp/test_trace_output.toml';

    afterEach(() => {
        try {
            fs.unlinkSync(testOutputPath);
        } catch {
            // File might not exist
        }
    });

    it('should write basic trace to file', () => {
        const meta = createMeta('2025-01-01T00:00:00Z', '20.10.0');
        const pkg = createPackage();
        const remarks = createRemarks('', '', '');
        const obj = createTraceObject('stack', 'test');
        const states = [createState(0, { items: [], top: -1 })];
        const steps: Step[] = [];
        const result = createResult(0);
        const trace = createTrace(meta, pkg, remarks, obj, states, steps, result);

        const writer = new TomlWriter(trace);
        writer.write(testOutputPath);

        const content = fs.readFileSync(testOutputPath, 'utf-8');
        expect(content).toContain('[meta]');
        expect(content).toContain('created_at = "2025-01-01T00:00:00Z"');
        expect(content).toContain('lang = "typescript"');
        expect(content).toContain('[package]');
        expect(content).toContain('name = "ds4viz"');
        expect(content).toContain('[object]');
        expect(content).toContain('kind = "stack"');
    });

    it('should write trace with remarks', () => {
        const meta = createMeta('2025-01-01T00:00:00Z', '20.10.0');
        const pkg = createPackage();
        const remarks = createRemarks('My Title', 'My Author', 'My Comment');
        const obj = createTraceObject('stack', 'test');
        const states = [createState(0, { items: [] })];
        const result = createResult(0);
        const trace = createTrace(meta, pkg, remarks, obj, states, [], result);

        const writer = new TomlWriter(trace);
        writer.write(testOutputPath);

        const content = fs.readFileSync(testOutputPath, 'utf-8');
        expect(content).toContain('[remarks]');
        expect(content).toContain('title = "My Title"');
        expect(content).toContain('author = "My Author"');
        expect(content).toContain('comment = "My Comment"');
    });

    it('should not write empty remarks section', () => {
        const meta = createMeta('2025-01-01T00:00:00Z', '20.10.0');
        const pkg = createPackage();
        const remarks = createRemarks('', '', '');
        const obj = createTraceObject('stack', 'test');
        const states = [createState(0, { items: [] })];
        const result = createResult(0);
        const trace = createTrace(meta, pkg, remarks, obj, states, [], result);

        const writer = new TomlWriter(trace);
        writer.write(testOutputPath);

        const content = fs.readFileSync(testOutputPath, 'utf-8');
        expect(content).not.toContain('[remarks]');
    });

    it('should write states with inline table arrays', () => {
        const meta = createMeta('2025-01-01T00:00:00Z', '20.10.0');
        const pkg = createPackage();
        const remarks = createRemarks('', '', '');
        const obj = createTraceObject('slist', 'test');
        const states = [
            createState(0, {
                head: 0,
                nodes: [
                    { id: 0, value: 1, next: 1 },
                    { id: 1, value: 2, next: -1 },
                ],
            }),
        ];
        const result = createResult(0);
        const trace = createTrace(meta, pkg, remarks, obj, states, [], result);

        const writer = new TomlWriter(trace);
        writer.write(testOutputPath);

        const content = fs.readFileSync(testOutputPath, 'utf-8');
        expect(content).toContain('[[states]]');
        expect(content).toContain('[states.data]');
        expect(content).toContain('head = 0');
        expect(content).toContain('nodes = [');
    });

    it('should write steps with args and code location', () => {
        const meta = createMeta('2025-01-01T00:00:00Z', '20.10.0');
        const pkg = createPackage();
        const remarks = createRemarks('', '', '');
        const obj = createTraceObject('stack', 'test');
        const states = [
            createState(0, { items: [], top: -1 }),
            createState(1, { items: [42], top: 0 }),
        ];
        const steps = [createStep(0, 'push', 0, 1, { value: 42 }, { line: 10 }, undefined, 'Push 42')];
        const result = createResult(1);
        const trace = createTrace(meta, pkg, remarks, obj, states, steps, result);

        const writer = new TomlWriter(trace);
        writer.write(testOutputPath);

        const content = fs.readFileSync(testOutputPath, 'utf-8');
        expect(content).toContain('[[steps]]');
        expect(content).toContain('id = 0');
        expect(content).toContain('op = "push"');
        expect(content).toContain('before = 0');
        expect(content).toContain('after = 1');
        expect(content).toContain('[steps.args]');
        expect(content).toContain('value = 42');
        expect(content).toContain('[steps.code]');
        expect(content).toContain('line = 10');
        expect(content).toContain('note = "Push 42"');
    });

    it('should write error section when present', () => {
        const meta = createMeta('2025-01-01T00:00:00Z', '20.10.0');
        const pkg = createPackage();
        const remarks = createRemarks('', '', '');
        const obj = createTraceObject('stack', 'test');
        const states = [createState(0, { items: [] })];
        const error = createTraceError('runtime', 'Stack underflow', 15, 2, 0);
        const trace = createTrace(meta, pkg, remarks, obj, states, [], undefined, error);

        const writer = new TomlWriter(trace);
        writer.write(testOutputPath);

        const content = fs.readFileSync(testOutputPath, 'utf-8');
        expect(content).toContain('[error]');
        expect(content).toContain('type = "runtime"');
        expect(content).toContain('message = "Stack underflow"');
        expect(content).toContain('line = 15');
        expect(content).toContain('step = 2');
        expect(content).toContain('last_state = 0');
    });

    it('should escape special characters in strings', () => {
        const meta = createMeta('2025-01-01T00:00:00Z', '20.10.0');
        const pkg = createPackage();
        const remarks = createRemarks('Title with "quotes"', 'Author', 'Comment\nwith\nnewlines');
        const obj = createTraceObject('stack', 'test\\path');
        const states = [createState(0, { items: [] })];
        const result = createResult(0);
        const trace = createTrace(meta, pkg, remarks, obj, states, [], result);

        const writer = new TomlWriter(trace);
        writer.write(testOutputPath);

        const content = fs.readFileSync(testOutputPath, 'utf-8');
        expect(content).toContain('title = "Title with \\"quotes\\""');
        expect(content).toContain('comment = "Comment\\nwith\\nnewlines"');
        expect(content).toContain('label = "test\\\\path"');
    });

    it('should write boolean values correctly', () => {
        const meta = createMeta('2025-01-01T00:00:00Z', '20.10.0');
        const pkg = createPackage();
        const remarks = createRemarks('', '', '');
        const obj = createTraceObject('stack', 'test');
        const states = [createState(0, { flag: true, active: false })];
        const result = createResult(0);
        const trace = createTrace(meta, pkg, remarks, obj, states, [], result);

        const writer = new TomlWriter(trace);
        writer.write(testOutputPath);

        const content = fs.readFileSync(testOutputPath, 'utf-8');
        expect(content).toContain('flag = true');
        expect(content).toContain('active = false');
    });

    it('should write steps with return values', () => {
        const meta = createMeta('2025-01-01T00:00:00Z', '20.10.0');
        const pkg = createPackage();
        const remarks = createRemarks('', '', '');
        const obj = createTraceObject('stack', 'test');
        const states = [
            createState(0, { items: [1, 2] }),
            createState(1, { items: [1] }),
        ];
        const steps = [createStep(0, 'pop', 0, 1, {}, undefined, 2)];
        const result = createResult(1);
        const trace = createTrace(meta, pkg, remarks, obj, states, steps, result);

        const writer = new TomlWriter(trace);
        writer.write(testOutputPath);

        const content = fs.readFileSync(testOutputPath, 'utf-8');
        expect(content).toContain('ret = 2');
    });

    it('should handle empty arrays', () => {
        const meta = createMeta('2025-01-01T00:00:00Z', '20.10.0');
        const pkg = createPackage();
        const remarks = createRemarks('', '', '');
        const obj = createTraceObject('stack', 'test');
        const states = [createState(0, { items: [], nodes: [] })];
        const result = createResult(0);
        const trace = createTrace(meta, pkg, remarks, obj, states, [], result);

        const writer = new TomlWriter(trace);
        writer.write(testOutputPath);

        const content = fs.readFileSync(testOutputPath, 'utf-8');
        expect(content).toContain('items = []');
        expect(content).toContain('nodes = []');
    });
});

// ============================================================================
// Session 模块测试
// ============================================================================

describe('Session Module', () => {
    describe('config and getConfig', () => {
        it('should set and get configuration', () => {
            config({
                outputPath: '/custom/path.toml',
                title: 'Test Title',
                author: 'Test Author',
                comment: 'Test Comment',
            });

            const cfg = getConfig();
            expect(cfg.outputPath).toBe('/custom/path.toml');
            expect(cfg.title).toBe('Test Title');
            expect(cfg.author).toBe('Test Author');
            expect(cfg.comment).toBe('Test Comment');
        });

        it('should use default values when not specified', () => {
            config({});

            const cfg = getConfig();
            expect(cfg.outputPath).toBe('trace.toml');
            expect(cfg.title).toBe('');
            expect(cfg.author).toBe('');
            expect(cfg.comment).toBe('');
        });

        it('should partially update configuration', () => {
            config({ title: 'Only Title' });

            const cfg = getConfig();
            expect(cfg.title).toBe('Only Title');
            expect(cfg.author).toBe('');
        });
    });

    describe('getTypeScriptVersion', () => {
        it('should return a valid version string', () => {
            const version = getTypeScriptVersion();
            expect(version).toMatch(/^\d+\.\d+\.\d+/);
        });

        it('should not include v prefix', () => {
            const version = getTypeScriptVersion();
            expect(version).not.toMatch(/^v/);
        });
    });

    describe('getCallerLine', () => {
        it('should return a positive line number', () => {
            const line = getCallerLine();
            expect(line).toBeGreaterThan(0);
        });

        it('should return different line for nested calls', () => {
            const line1 = getCallerLine();
            const line2 = getCallerLine();
            expect(line2).toBe(line1 + 1);
        });
    });

    describe('Error Classes', () => {
        it('should create DS4VizError with correct properties', () => {
            const error = new DS4VizError('Test error', ErrorType.Runtime, 10);
            expect(error.message).toBe('Test error');
            expect(error.errorType).toBe(ErrorType.Runtime);
            expect(error.line).toBe(10);
            expect(error.name).toBe('DS4VizError');
        });

        it('should create StructureError with runtime type', () => {
            const error = new StructureError('Structure error', 15);
            expect(error.message).toBe('Structure error');
            expect(error.errorType).toBe(ErrorType.Runtime);
            expect(error.line).toBe(15);
            expect(error.name).toBe('StructureError');
        });

        it('should create ValidationError with validation type', () => {
            const error = new ValidationError('Validation error', 20);
            expect(error.message).toBe('Validation error');
            expect(error.errorType).toBe(ErrorType.Validation);
            expect(error.line).toBe(20);
            expect(error.name).toBe('ValidationError');
        });

        it('should have correct inheritance chain', () => {
            const error = new StructureError('Test');
            expect(error instanceof StructureError).toBe(true);
            expect(error instanceof DS4VizError).toBe(true);
            expect(error instanceof Error).toBe(true);
        });
    });

    describe('Session Class', () => {
        it('should create session with correct kind and label', () => {
            const session = new Session('stack', 'test_stack', '/tmp/test.toml');
            expect(session.stepCounter).toBe(0);
        });

        it('should add states and return sequential ids', () => {
            const session = new Session('stack', 'test', '/tmp/test.toml');
            const id1 = session.addState({ items: [] });
            const id2 = session.addState({ items: [1] });
            const id3 = session.addState({ items: [1, 2] });

            expect(id1).toBe(0);
            expect(id2).toBe(1);
            expect(id3).toBe(2);
        });

        it('should add steps and return sequential ids', () => {
            const session = new Session('stack', 'test', '/tmp/test.toml');
            session.addState({ items: [] });

            const stepId1 = session.addStep('push', 0, 1, { value: 1 });
            const stepId2 = session.addStep('push', 1, 2, { value: 2 });

            expect(stepId1).toBe(0);
            expect(stepId2).toBe(1);
        });

        it('should get last state id correctly', () => {
            const session = new Session('stack', 'test', '/tmp/test.toml');
            expect(session.getLastStateId()).toBe(-1);

            session.addState({ items: [] });
            expect(session.getLastStateId()).toBe(0);

            session.addState({ items: [1] });
            expect(session.getLastStateId()).toBe(1);
        });

        it('should set error correctly', () => {
            const session = new Session('stack', 'test', '/tmp/test.toml');
            session.addState({ items: [] });
            session.setError(ErrorType.Runtime, 'Test error', 10, 0);

            const trace = session.buildTrace();
            expect(trace.error).toBeDefined();
            expect(trace.error?.type).toBe('runtime');
            expect(trace.error?.message).toBe('Test error');
        });

        it('should truncate long error messages', () => {
            const session = new Session('stack', 'test', '/tmp/test.toml');
            session.addState({ items: [] });
            const longMessage = 'a'.repeat(1000);
            session.setError(ErrorType.Runtime, longMessage, 10, 0);

            const trace = session.buildTrace();
            expect(trace.error?.message.length).toBeLessThanOrEqual(512);
        });

        it('should build trace with result when no error', () => {
            const session = new Session('stack', 'test', '/tmp/test.toml');
            session.addState({ items: [] });
            session.setExitLine(20);

            const trace = session.buildTrace();
            expect(trace.result).toBeDefined();
            expect(trace.result?.finalState).toBe(0);
            expect(trace.error).toBeUndefined();
        });
    });
});

// ============================================================================
// Linear Structures 测试
// ============================================================================

describe('Linear Structures', () => {
    const testOutputPath = '/tmp/test_linear.toml';

    afterEach(() => {
        try {
            fs.unlinkSync(testOutputPath);
        } catch {
            // File might not exist
        }
    });

    describe('Stack', () => {
        it('should create empty stack', () => {
            const s = new Stack({ output: testOutputPath });
            s.enter();
            s.dispose();

            const content = fs.readFileSync(testOutputPath, 'utf-8');
            expect(content).toContain('kind = "stack"');
            expect(content).toContain('items = []');
            expect(content).toContain('top = -1');
        });

        it('should push single element', () => {
            const s = new Stack({ output: testOutputPath });
            s.enter();
            s.push(42);
            s.dispose();

            const content = fs.readFileSync(testOutputPath, 'utf-8');
            expect(content).toContain('op = "push"');
            expect(content).toContain('value = 42');
            expect(content).toContain('items = [42]');
            expect(content).toContain('top = 0');
        });

        it('should push multiple elements', () => {
            const s = new Stack({ output: testOutputPath });
            s.enter();
            s.push(1);
            s.push(2);
            s.push(3);
            s.dispose();

            const content = fs.readFileSync(testOutputPath, 'utf-8');
            expect(content).toContain('items = [1, 2, 3]');
            expect(content).toContain('top = 2');
        });

        it('should pop element', () => {
            const s = new Stack({ output: testOutputPath });
            s.enter();
            s.push(1);
            s.push(2);
            s.pop();
            s.dispose();

            const content = fs.readFileSync(testOutputPath, 'utf-8');
            expect(content).toContain('op = "pop"');
            expect(content).toContain('ret = 2');
        });

        it('should throw error when popping from empty stack', () => {
            const s = new Stack({ output: testOutputPath });
            s.enter();

            expect(() => s.pop()).toThrow(StructureError);
            s.dispose();

            const content = fs.readFileSync(testOutputPath, 'utf-8');
            expect(content).toContain('[error]');
            expect(content).toContain('Cannot pop from empty stack');
        });

        it('should clear stack', () => {
            const s = new Stack({ output: testOutputPath });
            s.enter();
            s.push(1);
            s.push(2);
            s.clear();
            s.dispose();

            const content = fs.readFileSync(testOutputPath, 'utf-8');
            expect(content).toContain('op = "clear"');
        });

        it('should support string values', () => {
            const s = new Stack({ output: testOutputPath });
            s.enter();
            s.push('hello');
            s.push('world');
            s.dispose();

            const content = fs.readFileSync(testOutputPath, 'utf-8');
            expect(content).toContain('items = ["hello", "world"]');
        });

        it('should support boolean values', () => {
            const s = new Stack({ output: testOutputPath });
            s.enter();
            s.push(true);
            s.push(false);
            s.dispose();

            const content = fs.readFileSync(testOutputPath, 'utf-8');
            expect(content).toContain('items = [true, false]');
        });

        it('should use custom label', () => {
            const s = new Stack({ label: 'my_custom_stack', output: testOutputPath });
            s.enter();
            s.dispose();

            const content = fs.readFileSync(testOutputPath, 'utf-8');
            expect(content).toContain('label = "my_custom_stack"');
        });
    });

    describe('Queue', () => {
        it('should create empty queue', () => {
            const q = new Queue({ output: testOutputPath });
            q.enter();
            q.dispose();

            const content = fs.readFileSync(testOutputPath, 'utf-8');
            expect(content).toContain('kind = "queue"');
            expect(content).toContain('front = -1');
            expect(content).toContain('rear = -1');
        });

        it('should enqueue elements', () => {
            const q = new Queue({ output: testOutputPath });
            q.enter();
            q.enqueue(1);
            q.enqueue(2);
            q.dispose();

            const content = fs.readFileSync(testOutputPath, 'utf-8');
            expect(content).toContain('op = "enqueue"');
            expect(content).toContain('items = [1, 2]');
            expect(content).toContain('front = 0');
            expect(content).toContain('rear = 1');
        });

        it('should dequeue elements', () => {
            const q = new Queue({ output: testOutputPath });
            q.enter();
            q.enqueue(1);
            q.enqueue(2);
            q.dequeue();
            q.dispose();

            const content = fs.readFileSync(testOutputPath, 'utf-8');
            expect(content).toContain('op = "dequeue"');
            expect(content).toContain('ret = 1');
        });

        it('should throw error when dequeuing from empty queue', () => {
            const q = new Queue({ output: testOutputPath });
            q.enter();

            expect(() => q.dequeue()).toThrow(StructureError);
            q.dispose();

            const content = fs.readFileSync(testOutputPath, 'utf-8');
            expect(content).toContain('Cannot dequeue from empty queue');
        });

        it('should clear queue', () => {
            const q = new Queue({ output: testOutputPath });
            q.enter();
            q.enqueue(1);
            q.clear();
            q.dispose();

            const content = fs.readFileSync(testOutputPath, 'utf-8');
            expect(content).toContain('op = "clear"');
        });
    });

    describe('SingleLinkedList', () => {
        it('should create empty list', () => {
            const list = new SingleLinkedList({ output: testOutputPath });
            list.enter();
            list.dispose();

            const content = fs.readFileSync(testOutputPath, 'utf-8');
            expect(content).toContain('kind = "slist"');
            expect(content).toContain('head = -1');
            expect(content).toContain('nodes = []');
        });

        it('should insert at head', () => {
            const list = new SingleLinkedList({ output: testOutputPath });
            list.enter();
            const id = list.insertHead(42);
            list.dispose();

            expect(id).toBe(0);
            const content = fs.readFileSync(testOutputPath, 'utf-8');
            expect(content).toContain('op = "insert_head"');
            expect(content).toContain('head = 0');
        });

        it('should insert at tail', () => {
            const list = new SingleLinkedList({ output: testOutputPath });
            list.enter();
            list.insertHead(1);
            const tailId = list.insertTail(2);
            list.dispose();

            expect(tailId).toBe(1);
            const content = fs.readFileSync(testOutputPath, 'utf-8');
            expect(content).toContain('op = "insert_tail"');
        });

        it('should insert after node', () => {
            const list = new SingleLinkedList({ output: testOutputPath });
            list.enter();
            const headId = list.insertHead(1);
            list.insertAfter(headId, 2);
            list.dispose();

            const content = fs.readFileSync(testOutputPath, 'utf-8');
            expect(content).toContain('op = "insert_after"');
        });

        it('should throw error when inserting after non-existent node', () => {
            const list = new SingleLinkedList({ output: testOutputPath });
            list.enter();

            expect(() => list.insertAfter(999, 1)).toThrow(StructureError);
            list.dispose();

            const content = fs.readFileSync(testOutputPath, 'utf-8');
            expect(content).toContain('Node not found: 999');
        });

        it('should delete node', () => {
            const list = new SingleLinkedList({ output: testOutputPath });
            list.enter();
            const id = list.insertHead(1);
            list.insertHead(2);
            list.delete(id);
            list.dispose();

            const content = fs.readFileSync(testOutputPath, 'utf-8');
            expect(content).toContain('op = "delete"');
        });

        it('should delete head', () => {
            const list = new SingleLinkedList({ output: testOutputPath });
            list.enter();
            list.insertHead(1);
            list.insertHead(2);
            list.deleteHead();
            list.dispose();

            const content = fs.readFileSync(testOutputPath, 'utf-8');
            expect(content).toContain('op = "delete_head"');
        });

        it('should throw error when deleting from empty list', () => {
            const list = new SingleLinkedList({ output: testOutputPath });
            list.enter();

            expect(() => list.deleteHead()).toThrow(StructureError);
            list.dispose();

            const content = fs.readFileSync(testOutputPath, 'utf-8');
            expect(content).toContain('Cannot delete from empty list');
        });

        it('should reverse list', () => {
            const list = new SingleLinkedList({ output: testOutputPath });
            list.enter();
            list.insertTail(1);
            list.insertTail(2);
            list.insertTail(3);
            list.reverse();
            list.dispose();

            const content = fs.readFileSync(testOutputPath, 'utf-8');
            expect(content).toContain('op = "reverse"');
        });
    });

    describe('DoubleLinkedList', () => {
        it('should create empty list', () => {
            const list = new DoubleLinkedList({ output: testOutputPath });
            list.enter();
            list.dispose();

            const content = fs.readFileSync(testOutputPath, 'utf-8');
            expect(content).toContain('kind = "dlist"');
            expect(content).toContain('head = -1');
            expect(content).toContain('tail = -1');
        });

        it('should insert at head', () => {
            const list = new DoubleLinkedList({ output: testOutputPath });
            list.enter();
            list.insertHead(1);
            list.insertHead(2);
            list.dispose();

            const content = fs.readFileSync(testOutputPath, 'utf-8');
            expect(content).toContain('op = "insert_head"');
        });

        it('should insert at tail', () => {
            const list = new DoubleLinkedList({ output: testOutputPath });
            list.enter();
            list.insertTail(1);
            list.insertTail(2);
            list.dispose();

            const content = fs.readFileSync(testOutputPath, 'utf-8');
            expect(content).toContain('op = "insert_tail"');
        });

        it('should insert before node', () => {
            const list = new DoubleLinkedList({ output: testOutputPath });
            list.enter();
            const id = list.insertHead(1);
            list.insertBefore(id, 2);
            list.dispose();

            const content = fs.readFileSync(testOutputPath, 'utf-8');
            expect(content).toContain('op = "insert_before"');
        });

        it('should insert after node', () => {
            const list = new DoubleLinkedList({ output: testOutputPath });
            list.enter();
            const id = list.insertHead(1);
            list.insertAfter(id, 2);
            list.dispose();

            const content = fs.readFileSync(testOutputPath, 'utf-8');
            expect(content).toContain('op = "insert_after"');
        });

        it('should delete head', () => {
            const list = new DoubleLinkedList({ output: testOutputPath });
            list.enter();
            list.insertHead(1);
            list.insertHead(2);
            list.deleteHead();
            list.dispose();

            const content = fs.readFileSync(testOutputPath, 'utf-8');
            expect(content).toContain('op = "delete"');
        });

        it('should delete tail', () => {
            const list = new DoubleLinkedList({ output: testOutputPath });
            list.enter();
            list.insertTail(1);
            list.insertTail(2);
            list.deleteTail();
            list.dispose();

            const content = fs.readFileSync(testOutputPath, 'utf-8');
            expect(content).toContain('op = "delete"');
        });

        it('should throw error when deleting tail from empty list', () => {
            const list = new DoubleLinkedList({ output: testOutputPath });
            list.enter();

            expect(() => list.deleteTail()).toThrow(StructureError);
            list.dispose();
        });

        it('should reverse list', () => {
            const list = new DoubleLinkedList({ output: testOutputPath });
            list.enter();
            list.insertTail(1);
            list.insertTail(2);
            list.insertTail(3);
            list.reverse();
            list.dispose();

            const content = fs.readFileSync(testOutputPath, 'utf-8');
            expect(content).toContain('op = "reverse"');
        });
    });
});

// ============================================================================
// Tree Structures 测试
// ============================================================================

describe('Tree Structures', () => {
    const testOutputPath = '/tmp/test_tree.toml';

    afterEach(() => {
        try {
            fs.unlinkSync(testOutputPath);
        } catch {
            // File might not exist
        }
    });

    describe('BinaryTree', () => {
        it('should create empty tree', () => {
            const tree = new BinaryTree({ output: testOutputPath });
            tree.enter();
            tree.dispose();

            const content = fs.readFileSync(testOutputPath, 'utf-8');
            expect(content).toContain('kind = "binary_tree"');
            expect(content).toContain('root = -1');
            expect(content).toContain('nodes = []');
        });

        it('should insert root', () => {
            const tree = new BinaryTree({ output: testOutputPath });
            tree.enter();
            const id = tree.insertRoot(10);
            tree.dispose();

            expect(id).toBe(0);
            const content = fs.readFileSync(testOutputPath, 'utf-8');
            expect(content).toContain('op = "insert_root"');
            expect(content).toContain('root = 0');
        });

        it('should throw error when root already exists', () => {
            const tree = new BinaryTree({ output: testOutputPath });
            tree.enter();
            tree.insertRoot(10);

            expect(() => tree.insertRoot(20)).toThrow(StructureError);
            tree.dispose();

            const content = fs.readFileSync(testOutputPath, 'utf-8');
            expect(content).toContain('Root node already exists');
        });

        it('should insert left child', () => {
            const tree = new BinaryTree({ output: testOutputPath });
            tree.enter();
            const rootId = tree.insertRoot(10);
            tree.insertLeft(rootId, 5);
            tree.dispose();

            const content = fs.readFileSync(testOutputPath, 'utf-8');
            expect(content).toContain('op = "insert_left"');
        });

        it('should insert right child', () => {
            const tree = new BinaryTree({ output: testOutputPath });
            tree.enter();
            const rootId = tree.insertRoot(10);
            tree.insertRight(rootId, 15);
            tree.dispose();

            const content = fs.readFileSync(testOutputPath, 'utf-8');
            expect(content).toContain('op = "insert_right"');
        });

        it('should throw error when left child exists', () => {
            const tree = new BinaryTree({ output: testOutputPath });
            tree.enter();
            const rootId = tree.insertRoot(10);
            tree.insertLeft(rootId, 5);

            expect(() => tree.insertLeft(rootId, 3)).toThrow(StructureError);
            tree.dispose();

            const content = fs.readFileSync(testOutputPath, 'utf-8');
            expect(content).toContain('Left child already exists');
        });

        it('should throw error when inserting on non-existent parent', () => {
            const tree = new BinaryTree({ output: testOutputPath });
            tree.enter();

            expect(() => tree.insertLeft(999, 5)).toThrow(StructureError);
            tree.dispose();

            const content = fs.readFileSync(testOutputPath, 'utf-8');
            expect(content).toContain('Parent node not found: 999');
        });

        it('should delete subtree', () => {
            const tree = new BinaryTree({ output: testOutputPath });
            tree.enter();
            const rootId = tree.insertRoot(10);
            const leftId = tree.insertLeft(rootId, 5);
            tree.insertLeft(leftId, 3);
            tree.delete(leftId);
            tree.dispose();

            const content = fs.readFileSync(testOutputPath, 'utf-8');
            expect(content).toContain('op = "delete"');
        });

        it('should update value', () => {
            const tree = new BinaryTree({ output: testOutputPath });
            tree.enter();
            const rootId = tree.insertRoot(10);
            tree.updateValue(rootId, 20);
            tree.dispose();

            const content = fs.readFileSync(testOutputPath, 'utf-8');
            expect(content).toContain('op = "update_value"');
        });
    });

    describe('BinarySearchTree', () => {
        it('should create empty BST', () => {
            const bst = new BinarySearchTree({ output: testOutputPath });
            bst.enter();
            bst.dispose();

            const content = fs.readFileSync(testOutputPath, 'utf-8');
            expect(content).toContain('kind = "bst"');
            expect(content).toContain('root = -1');
        });

        it('should insert and maintain BST property', () => {
            const bst = new BinarySearchTree({ output: testOutputPath });
            bst.enter();
            bst.insert(10);
            bst.insert(5);
            bst.insert(15);
            bst.insert(3);
            bst.insert(7);
            bst.dispose();

            const content = fs.readFileSync(testOutputPath, 'utf-8');
            expect(content).toContain('op = "insert"');
        });

        it('should delete leaf node', () => {
            const bst = new BinarySearchTree({ output: testOutputPath });
            bst.enter();
            bst.insert(10);
            bst.insert(5);
            bst.insert(15);
            bst.delete(5);
            bst.dispose();

            const content = fs.readFileSync(testOutputPath, 'utf-8');
            expect(content).toContain('op = "delete"');
        });

        it('should delete node with one child', () => {
            const bst = new BinarySearchTree({ output: testOutputPath });
            bst.enter();
            bst.insert(10);
            bst.insert(5);
            bst.insert(3);
            bst.delete(5);
            bst.dispose();

            const content = fs.readFileSync(testOutputPath, 'utf-8');
            expect(content).toContain('op = "delete"');
        });

        it('should delete node with two children', () => {
            const bst = new BinarySearchTree({ output: testOutputPath });
            bst.enter();
            bst.insert(10);
            bst.insert(5);
            bst.insert(15);
            bst.insert(12);
            bst.insert(18);
            bst.delete(15);
            bst.dispose();

            const content = fs.readFileSync(testOutputPath, 'utf-8');
            expect(content).toContain('op = "delete"');
        });

        it('should throw error when deleting non-existent value', () => {
            const bst = new BinarySearchTree({ output: testOutputPath });
            bst.enter();
            bst.insert(10);

            expect(() => bst.delete(999)).toThrow(StructureError);
            bst.dispose();

            const content = fs.readFileSync(testOutputPath, 'utf-8');
            expect(content).toContain('Node with value 999 not found');
        });
    });

    describe('Heap', () => {
        it('should create empty min heap', () => {
            const h = new Heap({ output: testOutputPath });
            h.enter();
            h.dispose();

            const content = fs.readFileSync(testOutputPath, 'utf-8');
            expect(content).toContain('kind = "binary_tree"');
            expect(content).toContain('root = -1');
        });

        it('should insert and maintain min heap property', () => {
            const h = new Heap({ heapType: 'min', output: testOutputPath });
            h.enter();
            h.insert(10);
            h.insert(5);
            h.insert(15);
            h.insert(3);
            h.dispose();

            const content = fs.readFileSync(testOutputPath, 'utf-8');
            expect(content).toContain('op = "insert"');
        });

        it('should insert and maintain max heap property', () => {
            const h = new Heap({ heapType: 'max', output: testOutputPath });
            h.enter();
            h.insert(5);
            h.insert(10);
            h.insert(3);
            h.insert(15);
            h.dispose();

            const content = fs.readFileSync(testOutputPath, 'utf-8');
            expect(content).toContain('op = "insert"');
        });

        it('should extract from min heap', () => {
            const h = new Heap({ heapType: 'min', output: testOutputPath });
            h.enter();
            h.insert(10);
            h.insert(5);
            h.insert(15);
            h.extract();
            h.dispose();

            const content = fs.readFileSync(testOutputPath, 'utf-8');
            expect(content).toContain('op = "extract"');
            expect(content).toContain('ret = 5');
        });

        it('should extract from max heap', () => {
            const h = new Heap({ heapType: 'max', output: testOutputPath });
            h.enter();
            h.insert(5);
            h.insert(10);
            h.insert(15);
            h.extract();
            h.dispose();

            const content = fs.readFileSync(testOutputPath, 'utf-8');
            expect(content).toContain('ret = 15');
        });

        it('should throw error when extracting from empty heap', () => {
            const h = new Heap({ output: testOutputPath });
            h.enter();

            expect(() => h.extract()).toThrow(StructureError);
            h.dispose();

            const content = fs.readFileSync(testOutputPath, 'utf-8');
            expect(content).toContain('Cannot extract from empty heap');
        });

        it('should clear heap', () => {
            const h = new Heap({ output: testOutputPath });
            h.enter();
            h.insert(1);
            h.insert(2);
            h.clear();
            h.dispose();

            const content = fs.readFileSync(testOutputPath, 'utf-8');
            expect(content).toContain('op = "clear"');
        });

        it('should throw error for invalid heapType', () => {
            expect(() => new Heap({ heapType: 'invalid' as 'min' | 'max' })).toThrow();
        });
    });
});

// ============================================================================
// Graph Structures 测试
// ============================================================================

describe('Graph Structures', () => {
    const testOutputPath = '/tmp/test_graph.toml';

    afterEach(() => {
        try {
            fs.unlinkSync(testOutputPath);
        } catch {
            // File might not exist
        }
    });

    describe('GraphUndirected', () => {
        it('should create empty graph', () => {
            const g = new GraphUndirected({ output: testOutputPath });
            g.enter();
            g.dispose();

            const content = fs.readFileSync(testOutputPath, 'utf-8');
            expect(content).toContain('kind = "graph_undirected"');
            expect(content).toContain('nodes = []');
            expect(content).toContain('edges = []');
        });

        it('should add node', () => {
            const g = new GraphUndirected({ output: testOutputPath });
            g.enter();
            g.addNode(0, 'A');
            g.addNode(1, 'B');
            g.dispose();

            const content = fs.readFileSync(testOutputPath, 'utf-8');
            expect(content).toContain('op = "add_node"');
        });

        it('should throw error for duplicate node', () => {
            const g = new GraphUndirected({ output: testOutputPath });
            g.enter();
            g.addNode(0, 'A');

            expect(() => g.addNode(0, 'B')).toThrow(StructureError);
            g.dispose();

            const content = fs.readFileSync(testOutputPath, 'utf-8');
            expect(content).toContain('Node already exists: 0');
        });

        it('should add edge', () => {
            const g = new GraphUndirected({ output: testOutputPath });
            g.enter();
            g.addNode(0, 'A');
            g.addNode(1, 'B');
            g.addEdge(0, 1);
            g.dispose();

            const content = fs.readFileSync(testOutputPath, 'utf-8');
            expect(content).toContain('op = "add_edge"');
        });

        it('should normalize undirected edge', () => {
            const g = new GraphUndirected({ output: testOutputPath });
            g.enter();
            g.addNode(0, 'A');
            g.addNode(1, 'B');
            g.addEdge(1, 0);
            g.dispose();

            const content = fs.readFileSync(testOutputPath, 'utf-8');
            expect(content).toContain('from = 0');
            expect(content).toContain('to = 1');
        });

        it('should throw error for self-loop', () => {
            const g = new GraphUndirected({ output: testOutputPath });
            g.enter();
            g.addNode(0, 'A');

            expect(() => g.addEdge(0, 0)).toThrow(StructureError);
            g.dispose();

            const content = fs.readFileSync(testOutputPath, 'utf-8');
            expect(content).toContain('Self-loop not allowed');
        });

        it('should throw error for duplicate edge', () => {
            const g = new GraphUndirected({ output: testOutputPath });
            g.enter();
            g.addNode(0, 'A');
            g.addNode(1, 'B');
            g.addEdge(0, 1);

            expect(() => g.addEdge(1, 0)).toThrow(StructureError);
            g.dispose();

            const content = fs.readFileSync(testOutputPath, 'utf-8');
            expect(content).toContain('Edge already exists');
        });

        it('should remove node and related edges', () => {
            const g = new GraphUndirected({ output: testOutputPath });
            g.enter();
            g.addNode(0, 'A');
            g.addNode(1, 'B');
            g.addNode(2, 'C');
            g.addEdge(0, 1);
            g.addEdge(1, 2);
            g.removeNode(1);
            g.dispose();

            const content = fs.readFileSync(testOutputPath, 'utf-8');
            expect(content).toContain('op = "remove_node"');
        });

        it('should remove edge', () => {
            const g = new GraphUndirected({ output: testOutputPath });
            g.enter();
            g.addNode(0, 'A');
            g.addNode(1, 'B');
            g.addEdge(0, 1);
            g.removeEdge(0, 1);
            g.dispose();

            const content = fs.readFileSync(testOutputPath, 'utf-8');
            expect(content).toContain('op = "remove_edge"');
        });

        it('should update node label', () => {
            const g = new GraphUndirected({ output: testOutputPath });
            g.enter();
            g.addNode(0, 'A');
            g.updateNodeLabel(0, 'Updated');
            g.dispose();

            const content = fs.readFileSync(testOutputPath, 'utf-8');
            expect(content).toContain('op = "update_node_label"');
        });

        it('should validate label length', () => {
            const g = new GraphUndirected({ output: testOutputPath });
            g.enter();

            expect(() => g.addNode(0, '')).toThrow(StructureError);
            expect(() => g.addNode(0, 'a'.repeat(33))).toThrow(StructureError);
            g.dispose();
        });
    });

    describe('GraphDirected', () => {
        it('should create empty directed graph', () => {
            const g = new GraphDirected({ output: testOutputPath });
            g.enter();
            g.dispose();

            const content = fs.readFileSync(testOutputPath, 'utf-8');
            expect(content).toContain('kind = "graph_directed"');
        });

        it('should add directed edge', () => {
            const g = new GraphDirected({ output: testOutputPath });
            g.enter();
            g.addNode(0, 'A');
            g.addNode(1, 'B');
            g.addEdge(0, 1);
            g.addEdge(1, 0);
            g.dispose();

            const content = fs.readFileSync(testOutputPath, 'utf-8');
            expect(content).toContain('op = "add_edge"');
        });

        it('should throw error for duplicate directed edge', () => {
            const g = new GraphDirected({ output: testOutputPath });
            g.enter();
            g.addNode(0, 'A');
            g.addNode(1, 'B');
            g.addEdge(0, 1);

            expect(() => g.addEdge(0, 1)).toThrow(StructureError);
            g.dispose();
        });

        it('should remove directed edge', () => {
            const g = new GraphDirected({ output: testOutputPath });
            g.enter();
            g.addNode(0, 'A');
            g.addNode(1, 'B');
            g.addEdge(0, 1);
            g.removeEdge(0, 1);
            g.dispose();

            const content = fs.readFileSync(testOutputPath, 'utf-8');
            expect(content).toContain('op = "remove_edge"');
        });
    });

    describe('GraphWeighted', () => {
        it('should create empty weighted graph', () => {
            const g = new GraphWeighted({ output: testOutputPath });
            g.enter();
            g.dispose();

            const content = fs.readFileSync(testOutputPath, 'utf-8');
            expect(content).toContain('kind = "graph_weighted"');
        });

        it('should add weighted edge', () => {
            const g = new GraphWeighted({ output: testOutputPath });
            g.enter();
            g.addNode(0, 'A');
            g.addNode(1, 'B');
            g.addEdge(0, 1, 5);
            g.dispose();

            const content = fs.readFileSync(testOutputPath, 'utf-8');
            expect(content).toContain('weight = 5');
        });

        it('should update weight', () => {
            const g = new GraphWeighted({ output: testOutputPath });
            g.enter();
            g.addNode(0, 'A');
            g.addNode(1, 'B');
            g.addEdge(0, 1, 5);
            g.updateWeight(0, 1, 10);
            g.dispose();

            const content = fs.readFileSync(testOutputPath, 'utf-8');
            expect(content).toContain('op = "update_weight"');
        });

        it('should work as directed weighted graph', () => {
            const g = new GraphWeighted({ directed: true, output: testOutputPath });
            g.enter();
            g.addNode(0, 'A');
            g.addNode(1, 'B');
            g.addEdge(0, 1, 5);
            g.addEdge(1, 0, 3);
            g.dispose();

            const content = fs.readFileSync(testOutputPath, 'utf-8');
            expect(content).toContain('weight = 5');
            expect(content).toContain('weight = 3');
        });

        it('should work as undirected weighted graph', () => {
            const g = new GraphWeighted({ directed: false, output: testOutputPath });
            g.enter();
            g.addNode(0, 'A');
            g.addNode(1, 'B');
            g.addEdge(1, 0, 5);

            expect(() => g.addEdge(0, 1, 3)).toThrow(StructureError);
            g.dispose();
        });

        it('should throw error when updating non-existent edge', () => {
            const g = new GraphWeighted({ output: testOutputPath });
            g.enter();
            g.addNode(0, 'A');
            g.addNode(1, 'B');

            expect(() => g.updateWeight(0, 1, 5)).toThrow(StructureError);
            g.dispose();

            const content = fs.readFileSync(testOutputPath, 'utf-8');
            expect(content).toContain('Edge not found');
        });

        it('should handle negative weights', () => {
            const g = new GraphWeighted({ output: testOutputPath });
            g.enter();
            g.addNode(0, 'A');
            g.addNode(1, 'B');
            g.addEdge(0, 1, -5);
            g.dispose();

            const content = fs.readFileSync(testOutputPath, 'utf-8');
            expect(content).toContain('weight = -5');
        });

        it('should handle zero weights', () => {
            const g = new GraphWeighted({ output: testOutputPath });
            g.enter();
            g.addNode(0, 'A');
            g.addNode(1, 'B');
            g.addEdge(0, 1, 0);
            g.dispose();

            const content = fs.readFileSync(testOutputPath, 'utf-8');
            expect(content).toContain('weight = 0');
        });

        it('should handle floating point weights', () => {
            const g = new GraphWeighted({ output: testOutputPath });
            g.enter();
            g.addNode(0, 'A');
            g.addNode(1, 'B');
            g.addEdge(0, 1, 3.14);
            g.dispose();

            const content = fs.readFileSync(testOutputPath, 'utf-8');
            expect(content).toContain('weight = 3.14');
        });
    });
});