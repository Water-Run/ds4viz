/**
 * 集成测试 - 测试完整工作流程和组件协作
 *
 * @file test/integration.test.ts
 * @author WaterRun
 * @date 2025-12-25
 */

import { describe, it, expect, afterEach } from '@jest/globals';
import * as fs from 'node:fs';
import {
    config,
    withContext,
    stack,
    queue,
    singleLinkedList,
    doubleLinkedList,
    binaryTree,
    binarySearchTree,
    heap,
    graphUndirected,
    graphDirected,
    graphWeighted,
    Stack,
    Queue,
    SingleLinkedList,
    DoubleLinkedList,
    BinaryTree,
    BinarySearchTree,
    Heap,
    GraphUndirected,
    GraphDirected,
    GraphWeighted,
    StructureError,
    VERSION,
    AUTHOR,
} from '../src/index.js';

// ============================================================================
// 工厂函数测试
// ============================================================================

describe('Factory Functions', () => {
    const testOutputPath = '/tmp/test_factory.toml';

    afterEach(() => {
        try {
            fs.unlinkSync(testOutputPath);
        } catch {
            // File might not exist
        }
    });

    describe('stack factory', () => {
        it('should create and initialize stack', () => {
            const s = stack({ output: testOutputPath });
            s.push(1);
            s.dispose();

            const content = fs.readFileSync(testOutputPath, 'utf-8');
            expect(content).toContain('kind = "stack"');
        });
    });

    describe('queue factory', () => {
        it('should create and initialize queue', () => {
            const q = queue({ output: testOutputPath });
            q.enqueue(1);
            q.dispose();

            const content = fs.readFileSync(testOutputPath, 'utf-8');
            expect(content).toContain('kind = "queue"');
        });
    });

    describe('singleLinkedList factory', () => {
        it('should create and initialize single linked list', () => {
            const list = singleLinkedList({ output: testOutputPath });
            list.insertHead(1);
            list.dispose();

            const content = fs.readFileSync(testOutputPath, 'utf-8');
            expect(content).toContain('kind = "slist"');
        });
    });

    describe('doubleLinkedList factory', () => {
        it('should create and initialize double linked list', () => {
            const list = doubleLinkedList({ output: testOutputPath });
            list.insertHead(1);
            list.dispose();

            const content = fs.readFileSync(testOutputPath, 'utf-8');
            expect(content).toContain('kind = "dlist"');
        });
    });

    describe('binaryTree factory', () => {
        it('should create and initialize binary tree', () => {
            const tree = binaryTree({ output: testOutputPath });
            tree.insertRoot(1);
            tree.dispose();

            const content = fs.readFileSync(testOutputPath, 'utf-8');
            expect(content).toContain('kind = "binary_tree"');
        });
    });

    describe('binarySearchTree factory', () => {
        it('should create and initialize BST', () => {
            const bst = binarySearchTree({ output: testOutputPath });
            bst.insert(10);
            bst.dispose();

            const content = fs.readFileSync(testOutputPath, 'utf-8');
            expect(content).toContain('kind = "bst"');
        });
    });

    describe('heap factory', () => {
        it('should create and initialize heap', () => {
            const h = heap({ output: testOutputPath });
            h.insert(1);
            h.dispose();

            const content = fs.readFileSync(testOutputPath, 'utf-8');
            expect(content).toContain('kind = "binary_tree"');
        });
    });

    describe('graphUndirected factory', () => {
        it('should create and initialize undirected graph', () => {
            const g = graphUndirected({ output: testOutputPath });
            g.addNode(0, 'A');
            g.dispose();

            const content = fs.readFileSync(testOutputPath, 'utf-8');
            expect(content).toContain('kind = "graph_undirected"');
        });
    });

    describe('graphDirected factory', () => {
        it('should create and initialize directed graph', () => {
            const g = graphDirected({ output: testOutputPath });
            g.addNode(0, 'A');
            g.dispose();

            const content = fs.readFileSync(testOutputPath, 'utf-8');
            expect(content).toContain('kind = "graph_directed"');
        });
    });

    describe('graphWeighted factory', () => {
        it('should create and initialize weighted graph', () => {
            const g = graphWeighted({ output: testOutputPath });
            g.addNode(0, 'A');
            g.dispose();

            const content = fs.readFileSync(testOutputPath, 'utf-8');
            expect(content).toContain('kind = "graph_weighted"');
        });
    });
});

// ============================================================================
// withContext 测试
// ============================================================================

describe('withContext', () => {
    const testOutputPath = '/tmp/test_with_context.toml';

    afterEach(() => {
        try {
            fs.unlinkSync(testOutputPath);
        } catch {
            // File might not exist
        }
    });

    it('should execute synchronous callback', async () => {
        await withContext(new Stack({ output: testOutputPath }), (s) => {
            s.push(1);
            s.push(2);
            s.pop();
        });

        const content = fs.readFileSync(testOutputPath, 'utf-8');
        expect(content).toContain('op = "push"');
        expect(content).toContain('op = "pop"');
        expect(content).toContain('[result]');
    });

    it('should execute async callback', async () => {
        await withContext(new Stack({ output: testOutputPath }), async (s) => {
            s.push(1);
            await new Promise((resolve) => setTimeout(resolve, 10));
            s.push(2);
        });

        const content = fs.readFileSync(testOutputPath, 'utf-8');
        expect(content).toContain('[result]');
    });

    it('should catch and record error in callback', async () => {
        await expect(
            withContext(new Stack({ output: testOutputPath }), (s) => {
                s.pop();
            })
        ).rejects.toThrow(StructureError);

        const content = fs.readFileSync(testOutputPath, 'utf-8');
        expect(content).toContain('[error]');
        expect(content).toContain('Cannot pop from empty stack');
    });

    it('should re-throw error after recording', async () => {
        await expect(
            withContext(new Stack({ output: testOutputPath }), () => {
                throw new Error('Custom error');
            })
        ).rejects.toThrow('Custom error');
    });
});

// ============================================================================
// 全局配置测试
// ============================================================================

describe('Global Configuration', () => {
    const testOutputPath = '/tmp/test_config.toml';

    afterEach(() => {
        try {
            fs.unlinkSync(testOutputPath);
        } catch {
            // File might not exist
        }
        config({});
    });

    it('should apply global title', () => {
        config({ title: 'Global Title' });

        const s = new Stack({ output: testOutputPath });
        s.enter();
        s.dispose();

        const content = fs.readFileSync(testOutputPath, 'utf-8');
        expect(content).toContain('title = "Global Title"');
    });

    it('should apply global author', () => {
        config({ author: 'Global Author' });

        const s = new Stack({ output: testOutputPath });
        s.enter();
        s.dispose();

        const content = fs.readFileSync(testOutputPath, 'utf-8');
        expect(content).toContain('author = "Global Author"');
    });

    it('should apply global comment', () => {
        config({ comment: 'Global Comment' });

        const s = new Stack({ output: testOutputPath });
        s.enter();
        s.dispose();

        const content = fs.readFileSync(testOutputPath, 'utf-8');
        expect(content).toContain('comment = "Global Comment"');
    });

    it('should use global output path', () => {
        config({ outputPath: testOutputPath });

        const s = new Stack();
        s.enter();
        s.dispose();

        expect(fs.existsSync(testOutputPath)).toBe(true);
    });

    it('should allow local output to override global', () => {
        const localPath = '/tmp/test_local_override.toml';
        config({ outputPath: testOutputPath });

        const s = new Stack({ output: localPath });
        s.enter();
        s.dispose();

        expect(fs.existsSync(localPath)).toBe(true);

        try {
            fs.unlinkSync(localPath);
        } catch {
            // ignore
        }
    });
});

// ============================================================================
// 复杂操作序列测试
// ============================================================================

describe('Complex Operation Sequences', () => {
    const testOutputPath = '/tmp/test_complex.toml';

    afterEach(() => {
        try {
            fs.unlinkSync(testOutputPath);
        } catch {
            // File might not exist
        }
    });

    it('should handle stack push-pop sequence', () => {
        const s = new Stack({ output: testOutputPath });
        s.enter();

        for (let i = 0; i < 10; i++) {
            s.push(i);
        }
        for (let i = 0; i < 5; i++) {
            s.pop();
        }

        s.dispose();

        const content = fs.readFileSync(testOutputPath, 'utf-8');
        const pushCount = (content.match(/op = "push"/g) || []).length;
        const popCount = (content.match(/op = "pop"/g) || []).length;
        expect(pushCount).toBe(10);
        expect(popCount).toBe(5);
    });

    it('should handle queue enqueue-dequeue sequence', () => {
        const q = new Queue({ output: testOutputPath });
        q.enter();

        for (let i = 0; i < 5; i++) {
            q.enqueue(i);
        }
        for (let i = 0; i < 3; i++) {
            q.dequeue();
        }
        for (let i = 5; i < 8; i++) {
            q.enqueue(i);
        }

        q.dispose();

        const content = fs.readFileSync(testOutputPath, 'utf-8');
        const enqueueCount = (content.match(/op = "enqueue"/g) || []).length;
        const dequeueCount = (content.match(/op = "dequeue"/g) || []).length;
        expect(enqueueCount).toBe(8);
        expect(dequeueCount).toBe(3);
    });

    it('should handle linked list complex operations', () => {
        const list = new SingleLinkedList({ output: testOutputPath });
        list.enter();

        const id1 = list.insertHead(1);
        const id2 = list.insertTail(3);
        list.insertAfter(id1, 2);
        list.delete(id2);
        list.reverse();

        list.dispose();

        const content = fs.readFileSync(testOutputPath, 'utf-8');
        expect(content).toContain('op = "insert_head"');
        expect(content).toContain('op = "insert_tail"');
        expect(content).toContain('op = "insert_after"');
        expect(content).toContain('op = "delete"');
        expect(content).toContain('op = "reverse"');
    });

    it('should handle BST complex operations', () => {
        const bst = new BinarySearchTree({ output: testOutputPath });
        bst.enter();

        bst.insert(50);
        bst.insert(30);
        bst.insert(70);
        bst.insert(20);
        bst.insert(40);
        bst.insert(60);
        bst.insert(80);
        bst.delete(30);
        bst.delete(70);

        bst.dispose();

        const content = fs.readFileSync(testOutputPath, 'utf-8');
        const insertCount = (content.match(/op = "insert"/g) || []).length;
        const deleteCount = (content.match(/op = "delete"/g) || []).length;
        expect(insertCount).toBe(7);
        expect(deleteCount).toBe(2);
    });

    it('should handle heap complex operations', () => {
        const h = new Heap({ heapType: 'min', output: testOutputPath });
        h.enter();

        const values = [3, 1, 4, 1, 5, 9, 2, 6, 5, 3, 5];
        for (const v of values) {
            h.insert(v);
        }
        for (let i = 0; i < 5; i++) {
            h.extract();
        }

        h.dispose();

        const content = fs.readFileSync(testOutputPath, 'utf-8');
        const insertCount = (content.match(/op = "insert"/g) || []).length;
        const extractCount = (content.match(/op = "extract"/g) || []).length;
        expect(insertCount).toBe(11);
        expect(extractCount).toBe(5);
    });

    it('should handle graph complex operations', () => {
        const g = new GraphWeighted({ directed: true, output: testOutputPath });
        g.enter();

        g.addNode(0, 'A');
        g.addNode(1, 'B');
        g.addNode(2, 'C');
        g.addNode(3, 'D');
        g.addEdge(0, 1, 10);
        g.addEdge(0, 2, 5);
        g.addEdge(1, 2, 2);
        g.addEdge(1, 3, 1);
        g.addEdge(2, 3, 9);
        g.updateWeight(0, 1, 8);
        g.removeEdge(1, 2);
        g.removeNode(3);

        g.dispose();

        const content = fs.readFileSync(testOutputPath, 'utf-8');
        expect(content).toContain('op = "add_node"');
        expect(content).toContain('op = "add_edge"');
        expect(content).toContain('op = "update_weight"');
        expect(content).toContain('op = "remove_edge"');
        expect(content).toContain('op = "remove_node"');
    });
});

// ============================================================================
// 边缘情况测试
// ============================================================================

describe('Edge Cases', () => {
    const testOutputPath = '/tmp/test_edge.toml';

    afterEach(() => {
        try {
            fs.unlinkSync(testOutputPath);
        } catch {
            // File might not exist
        }
    });

    it('should handle empty structure disposal', () => {
        const s = new Stack({ output: testOutputPath });
        s.enter();
        s.dispose();

        const content = fs.readFileSync(testOutputPath, 'utf-8');
        expect(content).toContain('[result]');
        expect(content).toContain('final_state = 0');
    });

    it('should handle single element operations', () => {
        const s = new Stack({ output: testOutputPath });
        s.enter();
        s.push(42);
        s.pop();
        s.dispose();

        const content = fs.readFileSync(testOutputPath, 'utf-8');
        expect(content).toContain('items = []');
    });

    it('should handle mixed value types in stack', () => {
        const s = new Stack({ output: testOutputPath });
        s.enter();
        s.push(42);
        s.push('hello');
        s.push(true);
        s.push(false);
        s.push(3.14);
        s.dispose();

        const content = fs.readFileSync(testOutputPath, 'utf-8');
        expect(content).toContain('42');
        expect(content).toContain('"hello"');
        expect(content).toContain('true');
        expect(content).toContain('false');
        expect(content).toContain('3.14');
    });

    it('should handle special characters in string values', () => {
        const s = new Stack({ output: testOutputPath });
        s.enter();
        s.push('hello\nworld');
        s.push('quote"test');
        s.push('backslash\\path');
        s.dispose();

        const content = fs.readFileSync(testOutputPath, 'utf-8');
        expect(content).toContain('\\n');
        expect(content).toContain('\\"');
        expect(content).toContain('\\\\');
    });

    it('should handle large number of states', () => {
        const s = new Stack({ output: testOutputPath });
        s.enter();

        for (let i = 0; i < 100; i++) {
            s.push(i);
        }

        s.dispose();

        const content = fs.readFileSync(testOutputPath, 'utf-8');
        const stateCount = (content.match(/\[\[states\]\]/g) || []).length;
        expect(stateCount).toBe(101); // Initial + 100 pushes
    });

    it('should handle negative numbers', () => {
        const s = new Stack({ output: testOutputPath });
        s.enter();
        s.push(-42);
        s.push(-3.14);
        s.dispose();

        const content = fs.readFileSync(testOutputPath, 'utf-8');
        expect(content).toContain('-42');
        expect(content).toContain('-3.14');
    });

    it('should handle zero values', () => {
        const s = new Stack({ output: testOutputPath });
        s.enter();
        s.push(0);
        s.push(0.0);
        s.dispose();

        const content = fs.readFileSync(testOutputPath, 'utf-8');
        expect(content).toContain('0');
    });

    it('should handle empty string values', () => {
        const s = new Stack({ output: testOutputPath });
        s.enter();
        s.push('');
        s.dispose();

        const content = fs.readFileSync(testOutputPath, 'utf-8');
        expect(content).toContain('""');
    });

    it('should handle max safe integer', () => {
        const s = new Stack({ output: testOutputPath });
        s.enter();
        s.push(Number.MAX_SAFE_INTEGER);
        s.dispose();

        const content = fs.readFileSync(testOutputPath, 'utf-8');
        expect(content).toContain(String(Number.MAX_SAFE_INTEGER));
    });

    it('should handle binary tree with only left children', () => {
        const tree = new BinaryTree({ output: testOutputPath });
        tree.enter();

        const root = tree.insertRoot(1);
        const left1 = tree.insertLeft(root, 2);
        const left2 = tree.insertLeft(left1, 3);
        tree.insertLeft(left2, 4);

        tree.dispose();

        const content = fs.readFileSync(testOutputPath, 'utf-8');
        expect((content.match(/op = "insert_left"/g) || []).length).toBe(3);
    });

    it('should handle binary tree with only right children', () => {
        const tree = new BinaryTree({ output: testOutputPath });
        tree.enter();

        const root = tree.insertRoot(1);
        const right1 = tree.insertRight(root, 2);
        const right2 = tree.insertRight(right1, 3);
        tree.insertRight(right2, 4);

        tree.dispose();

        const content = fs.readFileSync(testOutputPath, 'utf-8');
        expect((content.match(/op = "insert_right"/g) || []).length).toBe(3);
    });

    it('should handle graph with isolated nodes', () => {
        const g = new GraphUndirected({ output: testOutputPath });
        g.enter();

        g.addNode(0, 'A');
        g.addNode(1, 'B');
        g.addNode(2, 'C');
        // No edges added

        g.dispose();

        const content = fs.readFileSync(testOutputPath, 'utf-8');
        expect(content).toContain('edges = []');
    });

    it('should handle complete graph (all nodes connected)', () => {
        const g = new GraphUndirected({ output: testOutputPath });
        g.enter();

        g.addNode(0, 'A');
        g.addNode(1, 'B');
        g.addNode(2, 'C');
        g.addNode(3, 'D');
        g.addEdge(0, 1);
        g.addEdge(0, 2);
        g.addEdge(0, 3);
        g.addEdge(1, 2);
        g.addEdge(1, 3);
        g.addEdge(2, 3);

        g.dispose();

        const content = fs.readFileSync(testOutputPath, 'utf-8');
        expect((content.match(/op = "add_edge"/g) || []).length).toBe(6);
    });

    it('should handle double linked list with single element operations', () => {
        const list = new DoubleLinkedList({ output: testOutputPath });
        list.enter();

        list.insertHead(1);
        list.deleteHead();
        list.insertTail(2);
        list.deleteTail();

        list.dispose();

        const content = fs.readFileSync(testOutputPath, 'utf-8');
        expect(content).toContain('op = "insert_head"');
        expect(content).toContain('op = "insert_tail"');
    });

    it('should handle heap with duplicate values', () => {
        const h = new Heap({ heapType: 'min', output: testOutputPath });
        h.enter();

        h.insert(5);
        h.insert(5);
        h.insert(5);
        h.extract();
        h.extract();

        h.dispose();

        const content = fs.readFileSync(testOutputPath, 'utf-8');
        expect((content.match(/op = "insert"/g) || []).length).toBe(3);
        expect((content.match(/op = "extract"/g) || []).length).toBe(2);
    });
});

// ============================================================================
// 错误处理测试
// ============================================================================

describe('Error Handling', () => {
    const testOutputPath = '/tmp/test_error.toml';

    afterEach(() => {
        try {
            fs.unlinkSync(testOutputPath);
        } catch {
            // File might not exist
        }
    });

    it('should record error type correctly', () => {
        const s = new Stack({ output: testOutputPath });
        s.enter();

        try {
            s.pop();
        } catch (e) {
            // expected
        }

        s.dispose();

        const content = fs.readFileSync(testOutputPath, 'utf-8');
        expect(content).toContain('type = "runtime"');
    });

    it('should record multiple errors but only last one counts', () => {
        const q = new Queue({ output: testOutputPath });
        q.enter();

        try {
            q.dequeue();
        } catch (e) {
            // first error
        }

        q.dispose();

        const content = fs.readFileSync(testOutputPath, 'utf-8');
        expect((content.match(/\[error\]/g) || []).length).toBe(1);
    });

    it('should record error after successful operations', () => {
        const s = new Stack({ output: testOutputPath });
        s.enter();

        s.push(1);
        s.push(2);
        s.pop();
        s.pop();

        try {
            s.pop();
        } catch (e) {
            // expected
        }

        s.dispose();

        const content = fs.readFileSync(testOutputPath, 'utf-8');
        expect(content).toContain('[error]');
        expect(content).toContain('last_state');
    });

    it('should not have result when error occurs', () => {
        const s = new Stack({ output: testOutputPath });
        s.enter();

        try {
            s.pop();
        } catch (e) {
            // expected
        }

        s.dispose();

        const content = fs.readFileSync(testOutputPath, 'utf-8');
        expect(content).toContain('[error]');
        expect(content).not.toContain('[result]');
    });
});

// ============================================================================
// 模块导出测试
// ============================================================================

describe('Module Exports', () => {
    it('should export VERSION constant', () => {
        expect(VERSION).toBe('1.0.0');
    });

    it('should export AUTHOR constant', () => {
        expect(AUTHOR).toBe('WaterRun <linzhuangrun49@gmail.com>');
    });

    it('should export all structure classes', () => {
        expect(Stack).toBeDefined();
        expect(Queue).toBeDefined();
        expect(SingleLinkedList).toBeDefined();
        expect(DoubleLinkedList).toBeDefined();
        expect(BinaryTree).toBeDefined();
        expect(BinarySearchTree).toBeDefined();
        expect(Heap).toBeDefined();
        expect(GraphUndirected).toBeDefined();
        expect(GraphDirected).toBeDefined();
        expect(GraphWeighted).toBeDefined();
    });

    it('should export all factory functions', () => {
        expect(typeof stack).toBe('function');
        expect(typeof queue).toBe('function');
        expect(typeof singleLinkedList).toBe('function');
        expect(typeof doubleLinkedList).toBe('function');
        expect(typeof binaryTree).toBe('function');
        expect(typeof binarySearchTree).toBe('function');
        expect(typeof heap).toBe('function');
        expect(typeof graphUndirected).toBe('function');
        expect(typeof graphDirected).toBe('function');
        expect(typeof graphWeighted).toBe('function');
    });

    it('should export config and withContext', () => {
        expect(typeof config).toBe('function');
        expect(typeof withContext).toBe('function');
    });

    it('should export error classes', () => {
        expect(StructureError).toBeDefined();
    });
});

// ============================================================================
// 状态一致性测试
// ============================================================================

describe('State Consistency', () => {
    const testOutputPath = '/tmp/test_state.toml';

    afterEach(() => {
        try {
            fs.unlinkSync(testOutputPath);
        } catch {
            // File might not exist
        }
    });

    it('should have consecutive state ids', () => {
        const s = new Stack({ output: testOutputPath });
        s.enter();

        s.push(1);
        s.push(2);
        s.push(3);
        s.pop();

        s.dispose();

        const content = fs.readFileSync(testOutputPath, 'utf-8');
        expect(content).toContain('id = 0');
        expect(content).toContain('id = 1');
        expect(content).toContain('id = 2');
        expect(content).toContain('id = 3');
        expect(content).toContain('id = 4');
    });

    it('should have consecutive step ids', () => {
        const s = new Stack({ output: testOutputPath });
        s.enter();

        s.push(1);
        s.push(2);
        s.pop();

        s.dispose();

        const content = fs.readFileSync(testOutputPath, 'utf-8');
        const stepMatches = content.match(/\[\[steps\]\]\nid = (\d+)/g) || [];
        const stepIds = stepMatches.map((m) => parseInt(m.match(/id = (\d+)/)![1]!, 10));
        expect(stepIds).toEqual([0, 1, 2]);
    });

    it('should have matching before/after references', () => {
        const s = new Stack({ output: testOutputPath });
        s.enter();

        s.push(1);
        s.push(2);

        s.dispose();

        const content = fs.readFileSync(testOutputPath, 'utf-8');
        // First step: before=0, after=1
        expect(content).toContain('before = 0');
        expect(content).toContain('after = 1');
        // Second step: before=1, after=2
        expect(content).toContain('before = 1');
        expect(content).toContain('after = 2');
    });

    it('should have final_state matching last state id', () => {
        const s = new Stack({ output: testOutputPath });
        s.enter();

        s.push(1);
        s.push(2);
        s.push(3);

        s.dispose();

        const content = fs.readFileSync(testOutputPath, 'utf-8');
        expect(content).toContain('final_state = 3');
    });
});

// ============================================================================
// TOML 格式验证测试
// ============================================================================

describe('TOML Format Validation', () => {
    const testOutputPath = '/tmp/test_toml.toml';

    afterEach(() => {
        try {
            fs.unlinkSync(testOutputPath);
        } catch {
            // File might not exist
        }
    });

    it('should produce valid TOML structure', () => {
        const s = new Stack({ output: testOutputPath });
        s.enter();
        s.push(1);
        s.dispose();

        const content = fs.readFileSync(testOutputPath, 'utf-8');

        // Check required sections exist
        expect(content).toContain('[meta]');
        expect(content).toContain('[package]');
        expect(content).toContain('[object]');
        expect(content).toContain('[[states]]');
        expect(content).toContain('[[steps]]');
    });

    it('should have correct meta fields', () => {
        const s = new Stack({ output: testOutputPath });
        s.enter();
        s.dispose();

        const content = fs.readFileSync(testOutputPath, 'utf-8');

        expect(content).toMatch(/created_at = "\d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2}Z"/);
        expect(content).toContain('lang = "typescript"');
        expect(content).toMatch(/lang_version = "\d+\.\d+\.\d+/);
    });

    it('should have correct package fields', () => {
        const s = new Stack({ output: testOutputPath });
        s.enter();
        s.dispose();

        const content = fs.readFileSync(testOutputPath, 'utf-8');

        expect(content).toContain('name = "ds4viz"');
        expect(content).toContain('lang = "typescript"');
        expect(content).toContain('version = "1.0.0"');
    });

    it('should format arrays correctly', () => {
        const s = new Stack({ output: testOutputPath });
        s.enter();
        s.push(1);
        s.push(2);
        s.push(3);
        s.dispose();

        const content = fs.readFileSync(testOutputPath, 'utf-8');
        expect(content).toContain('items = [1, 2, 3]');
    });

    it('should format inline tables correctly', () => {
        const list = new SingleLinkedList({ output: testOutputPath });
        list.enter();
        list.insertHead(1);
        list.insertHead(2);
        list.dispose();

        const content = fs.readFileSync(testOutputPath, 'utf-8');
        expect(content).toMatch(/\{ id = \d+, value = \d+, next = -?\d+ \}/);
    });
});