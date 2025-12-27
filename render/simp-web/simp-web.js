/**
 * ds4viz Demo - Main JavaScript
 * Version: 2.0.0
 * 
 * 数据结构可视化教学平台 Demo 前端
 * 支持远程执行和本地 JavaScript 执行
 * 
 * @file render/simp-web/simp-web.js
 * @author WaterRun
 * @date 2025-12-27
 */

(function () {
    'use strict';

    // ============================================
    // 配置与常量
    // ============================================

    const CONFIG = {
        defaultServer: 'http://localhost:9090',
        defaultLibraryPath: '../../library/javascript/ds4viz.js',
        defaultLanguage: 'javascript',
        defaultTimeout: 10000,
        pingInterval: 30000,
        autoPlayDelay: 800,
        toastDuration: 4000
    };

    const LANGUAGE_COLORS = {
        python: '#3776ab',
        javascript: '#f7df1e',
        typescript: '#3178c6',
        lua: '#000080',
        c: '#a8b9cc',
        rust: '#dea584',
        php: '#777bb4'
    };

    // ============================================
    // 语法高亮规则
    // ============================================

    const SYNTAX_RULES = {
        python: {
            keywords: ['def', 'class', 'if', 'elif', 'else', 'for', 'while', 'try', 'except', 'finally', 'with', 'as', 'import', 'from', 'return', 'yield', 'raise', 'pass', 'break', 'continue', 'and', 'or', 'not', 'in', 'is', 'None', 'True', 'False', 'lambda', 'global', 'nonlocal', 'async', 'await'],
            types: ['int', 'float', 'str', 'bool', 'list', 'dict', 'set', 'tuple', 'None'],
            ds4vizImports: ['ds4viz', 'dv'],
            ds4vizFunctions: ['stack', 'queue', 'single_linked_list', 'double_linked_list', 'binary_tree', 'binary_search_tree', 'heap', 'graph_undirected', 'graph_directed', 'graph_weighted', 'config'],
            ds4vizMethods: ['push', 'pop', 'clear', 'enqueue', 'dequeue', 'insert_head', 'insert_tail', 'insert_after', 'insert_before', 'delete', 'delete_head', 'delete_tail', 'reverse', 'insert_root', 'insert_left', 'insert_right', 'update_value', 'insert', 'extract', 'add_node', 'add_edge', 'remove_node', 'remove_edge', 'update_node_label', 'update_weight'],
            commentSingle: '#',
            stringDelimiters: ['"', "'", '"""', "'''"]
        },
        javascript: {
            keywords: ['function', 'const', 'let', 'var', 'if', 'else', 'for', 'while', 'do', 'switch', 'case', 'break', 'continue', 'return', 'try', 'catch', 'finally', 'throw', 'new', 'class', 'extends', 'import', 'export', 'from', 'default', 'async', 'await', 'yield', 'typeof', 'instanceof', 'delete', 'void', 'this', 'super', 'true', 'false', 'null', 'undefined'],
            types: ['Array', 'Object', 'String', 'Number', 'Boolean', 'Function', 'Promise', 'Map', 'Set'],
            ds4vizImports: ['ds4viz'],
            ds4vizFunctions: ['stack', 'queue', 'singleLinkedList', 'doubleLinkedList', 'binaryTree', 'binarySearchTree', 'heap', 'graphUndirected', 'graphDirected', 'graphWeighted', 'config', 'withContext'],
            ds4vizMethods: ['push', 'pop', 'clear', 'enqueue', 'dequeue', 'insertHead', 'insertTail', 'insertAfter', 'insertBefore', 'delete', 'deleteHead', 'deleteTail', 'reverse', 'insertRoot', 'insertLeft', 'insertRight', 'updateValue', 'insert', 'extract', 'addNode', 'addEdge', 'removeNode', 'removeEdge', 'updateNodeLabel', 'updateWeight'],
            commentSingle: '//',
            commentMultiStart: '/*',
            commentMultiEnd: '*/',
            stringDelimiters: ['"', "'", '`']
        },
        typescript: {
            keywords: ['function', 'const', 'let', 'var', 'if', 'else', 'for', 'while', 'do', 'switch', 'case', 'break', 'continue', 'return', 'try', 'catch', 'finally', 'throw', 'new', 'class', 'extends', 'implements', 'import', 'export', 'from', 'default', 'async', 'await', 'yield', 'typeof', 'instanceof', 'delete', 'void', 'this', 'super', 'true', 'false', 'null', 'undefined', 'interface', 'type', 'enum', 'namespace', 'module', 'declare', 'abstract', 'public', 'private', 'protected', 'readonly', 'static', 'as', 'is', 'keyof', 'infer'],
            types: ['string', 'number', 'boolean', 'void', 'never', 'any', 'unknown', 'object', 'Array', 'Promise', 'Map', 'Set', 'Record', 'Partial', 'Required', 'Readonly'],
            ds4vizImports: ['ds4viz'],
            ds4vizFunctions: ['stack', 'queue', 'singleLinkedList', 'doubleLinkedList', 'binaryTree', 'binarySearchTree', 'heap', 'graphUndirected', 'graphDirected', 'graphWeighted', 'config', 'withContext'],
            ds4vizMethods: ['push', 'pop', 'clear', 'enqueue', 'dequeue', 'insertHead', 'insertTail', 'insertAfter', 'insertBefore', 'delete', 'deleteHead', 'deleteTail', 'reverse', 'insertRoot', 'insertLeft', 'insertRight', 'updateValue', 'insert', 'extract', 'addNode', 'addEdge', 'removeNode', 'removeEdge', 'updateNodeLabel', 'updateWeight'],
            commentSingle: '//',
            commentMultiStart: '/*',
            commentMultiEnd: '*/',
            stringDelimiters: ['"', "'", '`']
        },
        lua: {
            keywords: ['and', 'break', 'do', 'else', 'elseif', 'end', 'false', 'for', 'function', 'if', 'in', 'local', 'nil', 'not', 'or', 'repeat', 'return', 'then', 'true', 'until', 'while', 'require'],
            types: ['table', 'string', 'number', 'boolean', 'function', 'thread', 'userdata'],
            ds4vizImports: ['ds4viz'],
            ds4vizFunctions: ['stack', 'queue', 'singleLinkedList', 'doubleLinkedList', 'binaryTree', 'binarySearchTree', 'heap', 'graphUndirected', 'graphDirected', 'graphWeighted', 'config', 'withContext'],
            ds4vizMethods: ['push', 'pop', 'clear', 'enqueue', 'dequeue', 'insertHead', 'insertTail', 'insertAfter', 'insertBefore', 'delete', 'deleteHead', 'deleteTail', 'reverse', 'insertRoot', 'insertLeft', 'insertRight', 'updateValue', 'insert', 'extract', 'addNode', 'addEdge', 'removeNode', 'removeEdge', 'updateNodeLabel', 'updateWeight'],
            commentSingle: '--',
            commentMultiStart: '--[[',
            commentMultiEnd: ']]',
            stringDelimiters: ['"', "'", '[[']
        },
        c: {
            keywords: ['auto', 'break', 'case', 'char', 'const', 'continue', 'default', 'do', 'double', 'else', 'enum', 'extern', 'float', 'for', 'goto', 'if', 'inline', 'int', 'long', 'register', 'restrict', 'return', 'short', 'signed', 'sizeof', 'static', 'struct', 'switch', 'typedef', 'union', 'unsigned', 'void', 'volatile', 'while', '_Bool', '_Complex', '_Imaginary', 'NULL', 'true', 'false'],
            types: ['int', 'char', 'float', 'double', 'void', 'long', 'short', 'unsigned', 'signed', 'size_t', 'int8_t', 'int16_t', 'int32_t', 'int64_t', 'uint8_t', 'uint16_t', 'uint32_t', 'uint64_t', 'bool'],
            ds4vizImports: ['ds4viz'],
            ds4vizFunctions: ['dv_stack_create', 'dv_stack_destroy', 'dv_stack_commit', 'dv_stack_push_int', 'dv_stack_push_float', 'dv_stack_push_string', 'dv_stack_push_bool', 'dv_stack_pop', 'dv_stack_clear', 'dv_queue_create', 'dv_queue_destroy', 'dv_queue_commit', 'dv_queue_enqueue_int', 'dv_queue_dequeue', 'dv_slist_create', 'dv_slist_destroy', 'dv_slist_commit', 'dv_slist_insert_head_int', 'dv_slist_insert_tail_int', 'dv_dlist_create', 'dv_binary_tree_create', 'dv_bst_create', 'dv_heap_create', 'dv_graph_undirected_create', 'dv_graph_directed_create', 'dv_graph_weighted_create', 'dv_config', 'DV_CHECK_GOTO', 'DV_CHECK_RETURN'],
            ds4vizMethods: [],
            ds4vizTypes: ['dv_stack_t', 'dv_queue_t', 'dv_slist_t', 'dv_dlist_t', 'dv_binary_tree_t', 'dv_bst_t', 'dv_heap_t', 'dv_graph_undirected_t', 'dv_graph_directed_t', 'dv_graph_weighted_t', 'dv_error_t', 'dv_heap_type_t', 'DV_OK', 'DV_HEAP_MIN', 'DV_HEAP_MAX'],
            commentSingle: '//',
            commentMultiStart: '/*',
            commentMultiEnd: '*/',
            stringDelimiters: ['"', "'"],
            preprocessor: '#'
        },
        rust: {
            keywords: ['as', 'async', 'await', 'break', 'const', 'continue', 'crate', 'dyn', 'else', 'enum', 'extern', 'false', 'fn', 'for', 'if', 'impl', 'in', 'let', 'loop', 'match', 'mod', 'move', 'mut', 'pub', 'ref', 'return', 'self', 'Self', 'static', 'struct', 'super', 'trait', 'true', 'type', 'unsafe', 'use', 'where', 'while', 'Ok', 'Err', 'Some', 'None'],
            types: ['i8', 'i16', 'i32', 'i64', 'i128', 'isize', 'u8', 'u16', 'u32', 'u64', 'u128', 'usize', 'f32', 'f64', 'bool', 'char', 'str', 'String', 'Vec', 'Option', 'Result', 'Box', 'Rc', 'Arc', 'Cell', 'RefCell', 'HashMap', 'HashSet'],
            ds4vizImports: ['ds4viz'],
            ds4vizFunctions: ['stack', 'queue', 'single_linked_list', 'double_linked_list', 'binary_tree', 'binary_search_tree', 'heap', 'graph_undirected', 'graph_directed', 'graph_weighted', 'config', 'stack_with_output', 'queue_with_output'],
            ds4vizMethods: ['push', 'pop', 'clear', 'enqueue', 'dequeue', 'insert_head', 'insert_tail', 'insert_after', 'insert_before', 'delete', 'delete_head', 'delete_tail', 'reverse', 'insert_root', 'insert_left', 'insert_right', 'update_value', 'insert', 'extract', 'add_node', 'add_edge', 'remove_node', 'remove_edge', 'update_node_label', 'update_weight'],
            ds4vizTypes: ['Stack', 'Queue', 'SingleLinkedList', 'DoubleLinkedList', 'BinaryTree', 'BinarySearchTree', 'Heap', 'HeapType', 'GraphUndirected', 'GraphDirected', 'GraphWeighted', 'Value', 'NumericValue', 'Config'],
            commentSingle: '//',
            commentMultiStart: '/*',
            commentMultiEnd: '*/',
            stringDelimiters: ['"'],
            lifetimes: true
        },
        php: {
            keywords: ['abstract', 'and', 'array', 'as', 'break', 'callable', 'case', 'catch', 'class', 'clone', 'const', 'continue', 'declare', 'default', 'die', 'do', 'echo', 'else', 'elseif', 'empty', 'enddeclare', 'endfor', 'endforeach', 'endif', 'endswitch', 'endwhile', 'eval', 'exit', 'extends', 'final', 'finally', 'fn', 'for', 'foreach', 'function', 'global', 'goto', 'if', 'implements', 'include', 'include_once', 'instanceof', 'insteadof', 'interface', 'isset', 'list', 'match', 'namespace', 'new', 'or', 'print', 'private', 'protected', 'public', 'readonly', 'require', 'require_once', 'return', 'static', 'switch', 'throw', 'trait', 'try', 'unset', 'use', 'var', 'while', 'xor', 'yield', 'true', 'false', 'null'],
            types: ['int', 'float', 'string', 'bool', 'array', 'object', 'callable', 'iterable', 'void', 'mixed', 'never'],
            ds4vizImports: ['Ds4viz'],
            ds4vizFunctions: ['stack', 'queue', 'singleLinkedList', 'doubleLinkedList', 'binaryTree', 'binarySearchTree', 'heap', 'graphUndirected', 'graphDirected', 'graphWeighted', 'config'],
            ds4vizMethods: ['run', 'push', 'pop', 'clear', 'enqueue', 'dequeue', 'insertHead', 'insertTail', 'insertAfter', 'insertBefore', 'delete', 'deleteHead', 'deleteTail', 'reverse', 'insertRoot', 'insertLeft', 'insertRight', 'updateValue', 'insert', 'extract', 'addNode', 'addEdge', 'removeNode', 'removeEdge', 'updateNodeLabel', 'updateWeight'],
            commentSingle: '//',
            commentMultiStart: '/*',
            commentMultiEnd: '*/',
            stringDelimiters: ['"', "'"],
            phpTag: '<?php'
        }
    };

    // ============================================
    // 代码模板
    // ============================================

    const TEMPLATES = {
        python: `import ds4viz as dv

def main():
    with dv.stack(label="demo_stack") as s:
        s.push(10)
        s.push(20)
        s.push(30)
        s.pop()
        s.push(40)

if __name__ == "__main__":
    main()
`,
        javascript: (libPath) => `import { stack, withContext } from '${libPath}';

async function main() {
    await withContext(stack({ label: 'demo_stack' }), async (s) => {
        s.push(10);
        s.push(20);
        s.push(30);
        s.pop();
        s.push(40);
    });
}

main();
`,
        typescript: `import { stack, withContext } from 'ds4viz';

async function main(): Promise<void> {
    await withContext(stack({ label: 'demo_stack' }), async (s) => {
        s.push(10);
        s.push(20);
        s.push(30);
        s.pop();
        s.push(40);
    });
}

main();
`,
        lua: `local ds4viz = require("ds4viz")

ds4viz.withContext(ds4viz.stack("demo_stack"), function(s)
    s:push(10)
    s:push(20)
    s:push(30)
    s:pop()
    s:push(40)
end)
`,
        c: (libPath) => {
            const headerPath = libPath.replace(/javascript\/ds4viz\.js$/, 'c/ds4viz.h');
            return `#define DS4VIZ_IMPLEMENTATION
#include "${headerPath}"

int main(void)
{
    dv_stack_t *s = dv_stack_create("demo_stack", NULL);
    if (s == NULL) return 1;

    dv_stack_push_int(s, 10);
    dv_stack_push_int(s, 20);
    dv_stack_push_int(s, 30);
    dv_stack_pop(s);
    dv_stack_push_int(s, 40);

    dv_stack_commit(s);
    dv_stack_destroy(s);

    return 0;
}
`;
        },
        rust: `use ds4viz::prelude::*;

fn main() -> ds4viz::Result<()> {
    ds4viz::stack("demo_stack", |s| {
        s.push(10)?;
        s.push(20)?;
        s.push(30)?;
        s.pop()?;
        s.push(40)?;
        Ok(())
    })
}
`,
        php: `<?php

require_once __DIR__ . '/vendor/autoload.php';

use Ds4viz\\Ds4viz;

Ds4viz::stack(label: 'demo_stack')
    ->run(function ($s) {
        $s->push(10);
        $s->push(20);
        $s->push(30);
        $s->pop();
        $s->push(40);
    });
`
    };

    // ============================================
    // 状态管理
    // ============================================

    const state = {
        mode: 'remote',
        serverUrl: CONFIG.defaultServer,
        libraryPath: CONFIG.defaultLibraryPath,
        language: CONFIG.defaultLanguage,
        connected: false,
        tomlData: null,
        parsedToml: null,
        currentStep: 0,
        totalSteps: 0,
        isPlaying: false,
        playInterval: null,
        ds4vizModule: null,
        autocomplete: {
            visible: false,
            items: [],
            selectedIndex: 0,
            triggerPosition: null
        }
    };

    // ============================================
    // DOM 元素引用
    // ============================================

    let elements = {};

    function initElements() {
        elements = {
            remoteModeBtn: document.querySelector('[data-mode="remote"]'),
            localModeBtn: document.querySelector('[data-mode="local"]'),
            remoteInputWrapper: document.getElementById('remoteInputWrapper'),
            localInputWrapper: document.getElementById('localInputWrapper'),
            serverUrl: document.getElementById('serverUrl'),
            libraryPath: document.getElementById('libraryPath'),
            pingBtn: document.getElementById('pingBtn'),
            connectionStatus: document.getElementById('connectionStatus'),
            uploadTomlBtn: document.getElementById('uploadTomlBtn'),
            downloadTomlBtn: document.getElementById('downloadTomlBtn'),
            tomlFileInput: document.getElementById('tomlFileInput'),
            aboutBtn: document.getElementById('aboutBtn'),
            aboutModal: document.getElementById('aboutModal'),
            closeAboutBtn: document.getElementById('closeAboutBtn'),
            languageSelect: document.getElementById('languageSelect'),
            langColorDot: document.getElementById('langColorDot'),
            runBtn: document.getElementById('runBtn'),
            codeEditor: document.getElementById('codeEditor'),
            codeHighlight: document.getElementById('codeHighlight'),
            lineNumbers: document.getElementById('lineNumbers'),
            autocompleteContainer: document.getElementById('autocompleteContainer'),
            errorIndicator: document.getElementById('errorIndicator'),
            executionStatus: document.getElementById('executionStatus'),
            vizContainer: document.getElementById('vizContainer'),
            stepFirstBtn: document.getElementById('stepFirstBtn'),
            stepBackBtn: document.getElementById('stepBackBtn'),
            stepForwardBtn: document.getElementById('stepForwardBtn'),
            stepLastBtn: document.getElementById('stepLastBtn'),
            stepIndicator: document.getElementById('stepIndicator'),
            autoPlayBtn: document.getElementById('autoPlayBtn'),
            stepInfo: document.getElementById('stepInfo'),
            stepOp: document.getElementById('stepOp'),
            stepLine: document.getElementById('stepLine'),
            stepArgs: document.getElementById('stepArgs'),
            stepNote: document.getElementById('stepNote'),
            loadingOverlay: document.getElementById('loadingOverlay'),
            loadingText: document.getElementById('loadingText'),
            toastContainer: document.getElementById('toastContainer')
        };
    }

    // ============================================
    // 工具函数
    // ============================================

    function escapeHtml(text) {
        const div = document.createElement('div');
        div.textContent = text;
        return div.innerHTML;
    }

    function debounce(func, wait) {
        let timeout;
        return function (...args) {
            clearTimeout(timeout);
            timeout = setTimeout(() => func.apply(this, args), wait);
        };
    }

    function showToast(message, type = 'info') {
        const toast = document.createElement('div');
        toast.className = `toast ${type}`;
        toast.innerHTML = `
            <svg class="toast-icon" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
                ${type === 'error'
                ? '<circle cx="12" cy="12" r="10"></circle><line x1="15" y1="9" x2="9" y2="15"></line><line x1="9" y1="9" x2="15" y2="15"></line>'
                : '<path d="M22 11.08V12a10 10 0 1 1-5.93-9.14"></path><polyline points="22 4 12 14.01 9 11.01"></polyline>'}
            </svg>
            <span class="toast-message">${escapeHtml(message)}</span>
            <button class="toast-close">
                <svg width="12" height="12" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
                    <line x1="18" y1="6" x2="6" y2="18"></line>
                    <line x1="6" y1="6" x2="18" y2="18"></line>
                </svg>
            </button>
        `;

        toast.querySelector('.toast-close').addEventListener('click', () => {
            toast.remove();
        });

        elements.toastContainer.appendChild(toast);

        setTimeout(() => {
            if (toast.parentNode) {
                toast.remove();
            }
        }, CONFIG.toastDuration);
    }

    // ============================================
    // 语法高亮
    // ============================================

    function highlightCode(code, language) {
        const rules = SYNTAX_RULES[language] || SYNTAX_RULES.javascript;
        let result = escapeHtml(code);

        // 处理注释
        if (rules.commentMultiStart && rules.commentMultiEnd) {
            const multiStart = escapeHtml(rules.commentMultiStart);
            const multiEnd = escapeHtml(rules.commentMultiEnd);
            const multiRegex = new RegExp(
                multiStart.replace(/[.*+?^${}()|[\]\\]/g, '\\$&') +
                '[\\s\\S]*?' +
                multiEnd.replace(/[.*+?^${}()|[\]\\]/g, '\\$&'),
                'g'
            );
            result = result.replace(multiRegex, match => `<span class="token-comment">${match}</span>`);
        }

        // 处理单行注释
        if (rules.commentSingle) {
            const singleComment = escapeHtml(rules.commentSingle);
            const singleRegex = new RegExp(`(${singleComment.replace(/[.*+?^${}()|[\]\\]/g, '\\$&')}[^\\n]*)`, 'g');
            result = result.replace(singleRegex, '<span class="token-comment">$1</span>');
        }

        // 处理字符串
        result = result.replace(/(&quot;(?:[^&]|&(?!quot;))*?&quot;|&#39;(?:[^&]|&(?!#39;))*?&#39;|`[^`]*`)/g,
            '<span class="token-string">$1</span>');

        // 处理数字
        result = result.replace(/\b(\d+\.?\d*(?:e[+-]?\d+)?)\b/gi, '<span class="token-number">$1</span>');

        // 处理 ds4viz 函数
        if (rules.ds4vizFunctions) {
            rules.ds4vizFunctions.forEach(fn => {
                const regex = new RegExp(`\\b(${fn})\\b`, 'g');
                result = result.replace(regex, '<span class="token-ds4viz">$1</span>');
            });
        }

        // 处理 ds4viz 方法
        if (rules.ds4vizMethods) {
            rules.ds4vizMethods.forEach(method => {
                const regex = new RegExp(`\\.(${method})\\b`, 'g');
                result = result.replace(regex, '.<span class="token-ds4viz">$1</span>');
            });
        }

        // 处理 ds4viz 类型 (C/Rust)
        if (rules.ds4vizTypes) {
            rules.ds4vizTypes.forEach(type => {
                const regex = new RegExp(`\\b(${type})\\b`, 'g');
                result = result.replace(regex, '<span class="token-ds4viz">$1</span>');
            });
        }

        // 处理类型
        if (rules.types) {
            rules.types.forEach(type => {
                const regex = new RegExp(`\\b(${type})\\b`, 'g');
                result = result.replace(regex, '<span class="token-type">$1</span>');
            });
        }

        // 处理关键字
        if (rules.keywords) {
            rules.keywords.forEach(keyword => {
                const regex = new RegExp(`\\b(${keyword})\\b`, 'g');
                result = result.replace(regex, '<span class="token-keyword">$1</span>');
            });
        }

        // 处理函数名
        result = result.replace(/\b([a-zA-Z_][a-zA-Z0-9_]*)\s*(?=\()/g, '<span class="token-function">$1</span>');

        // 处理预处理器指令 (C)
        if (rules.preprocessor) {
            result = result.replace(/^(#\w+)/gm, '<span class="token-keyword">$1</span>');
        }

        return result;
    }

    function updateHighlight() {
        const code = elements.codeEditor.value;
        elements.codeHighlight.innerHTML = highlightCode(code, state.language);
        updateLineNumbers();
    }

    // ============================================
    // 行号
    // ============================================

    function updateLineNumbers() {
        const code = elements.codeEditor.value;
        const lines = code.split('\n');
        const lineCount = lines.length;

        let html = '';
        for (let i = 1; i <= lineCount; i++) {
            html += `<div class="line-number">${i}</div>`;
        }
        elements.lineNumbers.innerHTML = html;
    }

    function syncScroll() {
        elements.codeHighlight.scrollTop = elements.codeEditor.scrollTop;
        elements.codeHighlight.scrollLeft = elements.codeEditor.scrollLeft;
        elements.lineNumbers.scrollTop = elements.codeEditor.scrollTop;
    }

    // ============================================
    // 自动补全
    // ============================================

    function getCompletions(prefix, language) {
        const rules = SYNTAX_RULES[language] || SYNTAX_RULES.javascript;
        const completions = [];

        // ds4viz 函数
        if (rules.ds4vizFunctions) {
            rules.ds4vizFunctions.forEach(fn => {
                if (fn.toLowerCase().startsWith(prefix.toLowerCase())) {
                    completions.push({ text: fn, type: 'function', hint: 'ds4viz' });
                }
            });
        }

        // ds4viz 方法
        if (rules.ds4vizMethods) {
            rules.ds4vizMethods.forEach(method => {
                if (method.toLowerCase().startsWith(prefix.toLowerCase())) {
                    completions.push({ text: method, type: 'method', hint: 'ds4viz' });
                }
            });
        }

        // 关键字
        if (rules.keywords) {
            rules.keywords.forEach(keyword => {
                if (keyword.toLowerCase().startsWith(prefix.toLowerCase())) {
                    completions.push({ text: keyword, type: 'keyword', hint: '' });
                }
            });
        }

        // 类型
        if (rules.types) {
            rules.types.forEach(type => {
                if (type.toLowerCase().startsWith(prefix.toLowerCase())) {
                    completions.push({ text: type, type: 'type', hint: '' });
                }
            });
        }

        return completions.slice(0, 10);
    }

    function showAutocomplete(items, position) {
        if (items.length === 0) {
            hideAutocomplete();
            return;
        }

        state.autocomplete.items = items;
        state.autocomplete.selectedIndex = 0;
        state.autocomplete.visible = true;

        let html = '';
        items.forEach((item, index) => {
            html += `
                <div class="autocomplete-item ${index === 0 ? 'selected' : ''}" data-index="${index}">
                    <span class="autocomplete-item-icon ${item.type}">${item.type[0].toUpperCase()}</span>
                    <span class="autocomplete-item-text">${escapeHtml(item.text)}</span>
                    ${item.hint ? `<span class="autocomplete-item-hint">${escapeHtml(item.hint)}</span>` : ''}
                </div>
            `;
        });

        elements.autocompleteContainer.innerHTML = html;
        elements.autocompleteContainer.classList.add('visible');

        // 定位
        const editorRect = elements.codeEditor.getBoundingClientRect();
        const lineHeight = parseFloat(getComputedStyle(elements.codeEditor).lineHeight);
        const charWidth = 8; // 估算的字符宽度

        elements.autocompleteContainer.style.left = `${position.col * charWidth + 48 + 16}px`;
        elements.autocompleteContainer.style.top = `${position.line * lineHeight + 16}px`;

        // 绑定点击事件
        elements.autocompleteContainer.querySelectorAll('.autocomplete-item').forEach(item => {
            item.addEventListener('click', () => {
                applyCompletion(parseInt(item.dataset.index));
            });
        });
    }

    function hideAutocomplete() {
        state.autocomplete.visible = false;
        elements.autocompleteContainer.classList.remove('visible');
    }

    function applyCompletion(index) {
        const item = state.autocomplete.items[index];
        if (!item) return;

        const editor = elements.codeEditor;
        const cursorPos = editor.selectionStart;
        const text = editor.value;

        // 找到当前单词的开始位置
        let wordStart = cursorPos;
        while (wordStart > 0 && /[a-zA-Z0-9_]/.test(text[wordStart - 1])) {
            wordStart--;
        }

        // 替换当前单词
        editor.value = text.substring(0, wordStart) + item.text + text.substring(cursorPos);
        editor.selectionStart = editor.selectionEnd = wordStart + item.text.length;

        hideAutocomplete();
        updateHighlight();
        editor.focus();
    }

    function updateAutocompleteSelection(delta) {
        const items = elements.autocompleteContainer.querySelectorAll('.autocomplete-item');
        if (items.length === 0) return;

        items[state.autocomplete.selectedIndex].classList.remove('selected');
        state.autocomplete.selectedIndex = (state.autocomplete.selectedIndex + delta + items.length) % items.length;
        items[state.autocomplete.selectedIndex].classList.add('selected');
        items[state.autocomplete.selectedIndex].scrollIntoView({ block: 'nearest' });
    }

    // ============================================
    // 编辑器事件处理
    // ============================================

    function handleEditorInput() {
        updateHighlight();

        // 触发自动补全
        const editor = elements.codeEditor;
        const cursorPos = editor.selectionStart;
        const text = editor.value;

        // 获取当前单词
        let wordStart = cursorPos;
        while (wordStart > 0 && /[a-zA-Z0-9_]/.test(text[wordStart - 1])) {
            wordStart--;
        }
        const currentWord = text.substring(wordStart, cursorPos);

        if (currentWord.length >= 2) {
            const completions = getCompletions(currentWord, state.language);
            const lines = text.substring(0, cursorPos).split('\n');
            const position = {
                line: lines.length - 1,
                col: lines[lines.length - 1].length
            };
            showAutocomplete(completions, position);
        } else {
            hideAutocomplete();
        }
    }

    function handleEditorKeydown(e) {
        // 自动补全导航
        if (state.autocomplete.visible) {
            if (e.key === 'ArrowDown') {
                e.preventDefault();
                updateAutocompleteSelection(1);
                return;
            }
            if (e.key === 'ArrowUp') {
                e.preventDefault();
                updateAutocompleteSelection(-1);
                return;
            }
            if (e.key === 'Tab' || e.key === 'Enter') {
                if (state.autocomplete.items.length > 0) {
                    e.preventDefault();
                    applyCompletion(state.autocomplete.selectedIndex);
                    return;
                }
            }
            if (e.key === 'Escape') {
                hideAutocomplete();
                return;
            }
        }

        const editor = elements.codeEditor;

        // Tab 键处理
        if (e.key === 'Tab' && !state.autocomplete.visible) {
            e.preventDefault();
            const start = editor.selectionStart;
            const end = editor.selectionEnd;
            const value = editor.value;

            if (e.shiftKey) {
                // Shift+Tab: 减少缩进
                const lineStart = value.lastIndexOf('\n', start - 1) + 1;
                const lineText = value.substring(lineStart, start);
                if (lineText.startsWith('    ')) {
                    editor.value = value.substring(0, lineStart) + value.substring(lineStart + 4);
                    editor.selectionStart = editor.selectionEnd = Math.max(start - 4, lineStart);
                }
            } else {
                // Tab: 增加缩进
                editor.value = value.substring(0, start) + '    ' + value.substring(end);
                editor.selectionStart = editor.selectionEnd = start + 4;
            }
            updateHighlight();
        }

        // Enter 键自动缩进
        if (e.key === 'Enter' && !state.autocomplete.visible) {
            e.preventDefault();
            const start = editor.selectionStart;
            const value = editor.value;
            const lineStart = value.lastIndexOf('\n', start - 1) + 1;
            const lineText = value.substring(lineStart, start);
            const indent = lineText.match(/^(\s*)/)[1];

            // 检查是否需要额外缩进
            let extraIndent = '';
            const trimmed = lineText.trimEnd();
            if (trimmed.endsWith(':') || trimmed.endsWith('{') || trimmed.endsWith('(') ||
                trimmed.endsWith('then') || trimmed.endsWith('do') || trimmed.endsWith('->')) {
                extraIndent = '    ';
            }

            editor.value = value.substring(0, start) + '\n' + indent + extraIndent + value.substring(start);
            editor.selectionStart = editor.selectionEnd = start + 1 + indent.length + extraIndent.length;
            updateHighlight();
        }

        // 自动闭合括号和引号
        const pairs = { '(': ')', '[': ']', '{': '}', '"': '"', "'": "'", '`': '`' };
        if (pairs[e.key]) {
            const start = editor.selectionStart;
            const end = editor.selectionEnd;
            const value = editor.value;

            if (start !== end) {
                // 包围选中文本
                e.preventDefault();
                const selected = value.substring(start, end);
                editor.value = value.substring(0, start) + e.key + selected + pairs[e.key] + value.substring(end);
                editor.selectionStart = start + 1;
                editor.selectionEnd = end + 1;
                updateHighlight();
            }
        }
    }

    // ============================================
    // TOML 解析器
    // ============================================

    function parseToml(tomlString) {
        const result = {
            meta: {},
            package: {},
            remarks: {},
            object: {},
            states: [],
            steps: [],
            result: null,
            error: null
        };

        const lines = tomlString.split('\n');
        let currentSection = null;
        let currentArrayItem = null;
        let inArraySection = null;

        for (let i = 0; i < lines.length; i++) {
            const line = lines[i];
            const trimmed = line.trim();

            if (!trimmed || trimmed.startsWith('#')) continue;

            // 数组表头 [[xxx]]
            const arrayMatch = trimmed.match(/^\[\[(\w+)\]\]$/);
            if (arrayMatch) {
                // 保存之前的数组项
                if (inArraySection && currentArrayItem && Object.keys(currentArrayItem).length > 0) {
                    result[inArraySection].push(currentArrayItem);
                }
                inArraySection = arrayMatch[1];
                currentArrayItem = {};
                currentSection = null;
                continue;
            }

            // 子表头 [xxx.yyy]
            const subSectionMatch = trimmed.match(/^\[(\w+)\.(\w+)\]$/);
            if (subSectionMatch) {
                const [, parent, child] = subSectionMatch;
                if (inArraySection === parent) {
                    currentSection = child;
                    if (!currentArrayItem[child]) {
                        currentArrayItem[child] = {};
                    }
                } else if (parent === 'result') {
                    if (!result.result) result.result = {};
                    currentSection = `result.${child}`;
                    if (!result.result[child]) result.result[child] = {};
                }
                continue;
            }

            // 普通表头 [xxx]
            const sectionMatch = trimmed.match(/^\[(\w+)\]$/);
            if (sectionMatch) {
                // 保存之前的数组项
                if (inArraySection && currentArrayItem && Object.keys(currentArrayItem).length > 0) {
                    result[inArraySection].push(currentArrayItem);
                    currentArrayItem = null;
                }
                inArraySection = null;
                currentSection = sectionMatch[1];
                continue;
            }

            // 键值对
            const kvMatch = trimmed.match(/^(\w+)\s*=\s*(.+)$/);
            if (kvMatch) {
                const [, key, valueStr] = kvMatch;
                const value = parseTomlValue(valueStr.trim());

                if (inArraySection && currentArrayItem) {
                    if (currentSection && currentArrayItem[currentSection]) {
                        currentArrayItem[currentSection][key] = value;
                    } else {
                        currentArrayItem[key] = value;
                    }
                } else if (currentSection) {
                    if (currentSection.startsWith('result.')) {
                        const subKey = currentSection.split('.')[1];
                        result.result[subKey][key] = value;
                    } else if (currentSection === 'result') {
                        if (!result.result) result.result = {};
                        result.result[key] = value;
                    } else if (currentSection === 'error') {
                        if (!result.error) result.error = {};
                        result.error[key] = value;
                    } else {
                        if (!result[currentSection]) result[currentSection] = {};
                        result[currentSection][key] = value;
                    }
                }
            }
        }

        // 保存最后一个数组项
        if (inArraySection && currentArrayItem && Object.keys(currentArrayItem).length > 0) {
            result[inArraySection].push(currentArrayItem);
        }

        return result;
    }

    function parseTomlValue(valueStr) {
        // 去除首尾空白
        valueStr = valueStr.trim();

        // 字符串
        if ((valueStr.startsWith('"') && valueStr.endsWith('"')) ||
            (valueStr.startsWith("'") && valueStr.endsWith("'"))) {
            return valueStr.slice(1, -1)
                .replace(/\\n/g, '\n')
                .replace(/\\t/g, '\t')
                .replace(/\\"/g, '"')
                .replace(/\\'/g, "'")
                .replace(/\\\\/g, '\\');
        }

        // 布尔值
        if (valueStr === 'true') return true;
        if (valueStr === 'false') return false;

        // 数组
        if (valueStr.startsWith('[')) {
            return parseTomlArray(valueStr);
        }

        // 数字
        if (/^-?\d+$/.test(valueStr)) {
            return parseInt(valueStr, 10);
        }
        if (/^-?\d+\.\d+$/.test(valueStr)) {
            return parseFloat(valueStr);
        }

        return valueStr;
    }

    function parseTomlArray(valueStr) {
        const content = valueStr.slice(1, -1).trim();
        if (!content) return [];

        const items = [];
        let current = '';
        let depth = 0;
        let inString = false;
        let stringChar = '';

        for (let i = 0; i < content.length; i++) {
            const char = content[i];
            const prevChar = i > 0 ? content[i - 1] : '';

            if (!inString && (char === '"' || char === "'")) {
                inString = true;
                stringChar = char;
                current += char;
            } else if (inString && char === stringChar && prevChar !== '\\') {
                inString = false;
                current += char;
            } else if (!inString && (char === '{' || char === '[')) {
                depth++;
                current += char;
            } else if (!inString && (char === '}' || char === ']')) {
                depth--;
                current += char;
            } else if (!inString && char === ',' && depth === 0) {
                const trimmed = current.trim();
                if (trimmed) {
                    items.push(parseTomlValueOrInlineTable(trimmed));
                }
                current = '';
            } else {
                current += char;
            }
        }

        const trimmed = current.trim();
        if (trimmed) {
            items.push(parseTomlValueOrInlineTable(trimmed));
        }

        return items;
    }

    function parseTomlValueOrInlineTable(valueStr) {
        valueStr = valueStr.trim();

        if (valueStr.startsWith('{') && valueStr.endsWith('}')) {
            const obj = {};
            const content = valueStr.slice(1, -1).trim();
            if (!content) return obj;

            let current = '';
            let depth = 0;
            let inString = false;
            let stringChar = '';

            for (let i = 0; i <= content.length; i++) {
                const char = content[i];
                const prevChar = i > 0 ? content[i - 1] : '';

                if (i === content.length || (!inString && char === ',' && depth === 0)) {
                    const pair = current.trim();
                    const eqIndex = pair.indexOf('=');
                    if (eqIndex > 0) {
                        const key = pair.substring(0, eqIndex).trim();
                        const val = pair.substring(eqIndex + 1).trim();
                        obj[key] = parseTomlValue(val);
                    }
                    current = '';
                } else {
                    if (!inString && (char === '"' || char === "'")) {
                        inString = true;
                        stringChar = char;
                    } else if (inString && char === stringChar && prevChar !== '\\') {
                        inString = false;
                    } else if (!inString && (char === '{' || char === '[')) {
                        depth++;
                    } else if (!inString && (char === '}' || char === ']')) {
                        depth--;
                    }
                    current += char;
                }
            }

            return obj;
        }

        return parseTomlValue(valueStr);
    }

    // ============================================
    // 可视化渲染
    // ============================================

    function parseAndRenderToml(tomlString) {
        try {
            const parsed = parseToml(tomlString);
            state.parsedToml = parsed;

            state.totalSteps = (parsed.states || []).length;
            state.currentStep = 0;

            if (parsed.error) {
                showToast(parsed.error.message || '执行出错', 'error');
            }

            updateVizControls();
            renderCurrentState();
        } catch (error) {
            console.error('TOML 解析错误:', error);
            showToast('TOML 解析失败: ' + error.message, 'error');
        }
    }

    function updateVizControls() {
        const hasData = state.totalSteps > 0;

        elements.stepFirstBtn.disabled = !hasData || state.currentStep <= 0;
        elements.stepBackBtn.disabled = !hasData || state.currentStep <= 0;
        elements.stepForwardBtn.disabled = !hasData || state.currentStep >= state.totalSteps - 1;
        elements.stepLastBtn.disabled = !hasData || state.currentStep >= state.totalSteps - 1;
        elements.autoPlayBtn.disabled = !hasData;

        if (state.isPlaying) {
            elements.autoPlayBtn.classList.add('playing');
        } else {
            elements.autoPlayBtn.classList.remove('playing');
        }

        elements.stepIndicator.textContent = hasData
            ? `${state.currentStep + 1} / ${state.totalSteps}`
            : '0 / 0';

        // 更新步骤信息
        updateStepInfo();
    }

    function updateStepInfo() {
        const steps = state.parsedToml?.steps || [];

        // 找到当前状态对应的步骤
        let currentStepData = null;
        for (const step of steps) {
            if (step.after === state.currentStep) {
                currentStepData = step;
                break;
            }
        }

        if (currentStepData) {
            elements.stepInfo.style.display = 'block';
            elements.stepOp.textContent = currentStepData.op || '';
            elements.stepLine.textContent = currentStepData.code?.line ? `Line ${currentStepData.code.line}` : '';

            const args = currentStepData.args || {};
            const argsStr = Object.entries(args)
                .map(([k, v]) => `${k}: ${JSON.stringify(v)}`)
                .join(', ');
            elements.stepArgs.textContent = argsStr || '';
            elements.stepNote.textContent = currentStepData.note || '';
            elements.stepNote.style.display = currentStepData.note ? 'block' : 'none';
        } else {
            elements.stepInfo.style.display = 'none';
        }
    }

    function renderCurrentState() {
        const parsed = state.parsedToml;
        if (!parsed || !parsed.states || parsed.states.length === 0) {
            renderPlaceholder();
            return;
        }

        const currentState = parsed.states[state.currentStep];
        if (!currentState) {
            renderPlaceholder();
            return;
        }

        const data = currentState.data || currentState;
        const objectKind = parsed.object?.kind || 'stack';
        const label = parsed.object?.label || objectKind;

        switch (objectKind) {
            case 'stack':
                renderStack(data, label);
                break;
            case 'queue':
                renderQueue(data, label);
                break;
            case 'slist':
                renderSingleLinkedList(data, label);
                break;
            case 'dlist':
                renderDoubleLinkedList(data, label);
                break;
            case 'binary_tree':
            case 'bst':
                renderBinaryTree(data, label, objectKind);
                break;
            case 'graph_undirected':
            case 'graph_directed':
            case 'graph_weighted':
                renderGraph(data, label, objectKind);
                break;
            default:
                renderUnsupported(objectKind);
        }
    }

    function renderPlaceholder() {
        elements.vizContainer.innerHTML = `
            <div class="viz-placeholder">
                <img src="images/flowsheet_100dp_1F1F1F_FILL0_wght400_GRAD0_opsz48.png" alt="可视化" class="placeholder-icon">
                <p class="placeholder-text">运行代码或上传 TOML 查看可视化</p>
            </div>
        `;
    }

    function renderUnsupported(kind) {
        elements.vizContainer.innerHTML = `
            <div class="viz-empty">
                <img src="images/code_off_100dp_1F1F1F_FILL0_wght400_GRAD0_opsz48.png" alt="不支持" class="viz-empty-icon">
                <div class="viz-empty-text">不支持的数据结构类型: ${escapeHtml(kind)}</div>
            </div>
        `;
    }

    function renderEmpty(label, structureType) {
        elements.vizContainer.innerHTML = `
            <div class="viz-wrapper">
                <div class="viz-label">${escapeHtml(label)} (${escapeHtml(structureType)})</div>
                <div class="viz-empty">
                    <img src="images/data_array_100dp_1F1F1F_FILL0_wght400_GRAD0_opsz48.png" alt="空" class="viz-empty-icon">
                    <div class="viz-empty-text">空${escapeHtml(structureType)}</div>
                </div>
            </div>
        `;
    }

    function renderStack(data, label) {
        const items = data.items || [];
        const top = data.top ?? items.length - 1;

        if (items.length === 0) {
            renderEmpty(label, 'Stack');
            return;
        }

        let html = `<div class="viz-wrapper">
            <div class="viz-label">${escapeHtml(label)} (Stack)</div>
            <div class="viz-stack">`;

        for (let i = 0; i < items.length; i++) {
            const isTop = i === top;
            html += `<div class="viz-stack-item ${isTop ? 'top' : ''}">${escapeHtml(String(items[i]))}</div>`;
        }

        html += `</div>
            <div class="viz-stack-pointer">top = ${top}</div>
        </div>`;

        elements.vizContainer.innerHTML = html;
    }

    function renderQueue(data, label) {
        const items = data.items || [];
        const front = data.front ?? 0;
        const rear = data.rear ?? items.length - 1;

        if (items.length === 0) {
            renderEmpty(label, 'Queue');
            return;
        }

        let html = `<div class="viz-wrapper">
            <div class="viz-label">${escapeHtml(label)} (Queue: front → rear)</div>
            <div class="viz-queue">`;

        for (let i = 0; i < items.length; i++) {
            const isFront = i === front;
            const isRear = i === rear;
            let classes = 'viz-queue-item';
            if (isFront && isRear) classes += ' front rear';
            else if (isFront) classes += ' front';
            else if (isRear) classes += ' rear';

            html += `<div class="${classes}">${escapeHtml(String(items[i]))}</div>`;
            if (i < items.length - 1) {
                html += '<span class="viz-queue-arrow">→</span>';
            }
        }

        html += `</div>
            <div class="viz-meta">front = ${front}, rear = ${rear}</div>
        </div>`;

        elements.vizContainer.innerHTML = html;
    }

    function renderSingleLinkedList(data, label) {
        const nodes = data.nodes || [];
        const head = data.head;

        if (nodes.length === 0 || head === -1) {
            renderEmpty(label, 'Single Linked List');
            return;
        }

        const nodeMap = {};
        nodes.forEach(n => nodeMap[n.id] = n);

        let html = `<div class="viz-wrapper">
            <div class="viz-label">${escapeHtml(label)} (Single Linked List)</div>
            <div class="viz-list">`;

        let currentId = head;
        let visited = new Set();
        let count = 0;
        const maxNodes = 50;

        while (currentId !== -1 && !visited.has(currentId) && count < maxNodes) {
            visited.add(currentId);
            const node = nodeMap[currentId];
            if (!node) break;

            html += `
                <div class="viz-list-node">
                    <div class="viz-list-node-box">
                        <div class="viz-list-node-value">${escapeHtml(String(node.value))}</div>
                        <div class="viz-list-node-pointer">${node.next === -1 ? 'null' : node.next}</div>
                    </div>
                </div>`;

            if (node.next !== -1 && nodeMap[node.next]) {
                html += '<span class="viz-list-arrow">→</span>';
            }

            currentId = node.next;
            count++;
        }

        html += '<div class="viz-list-null">NULL</div>';
        html += `</div></div>`;

        elements.vizContainer.innerHTML = html;
    }

    function renderDoubleLinkedList(data, label) {
        const nodes = data.nodes || [];
        const head = data.head;

        if (nodes.length === 0 || head === -1) {
            renderEmpty(label, 'Double Linked List');
            return;
        }

        const nodeMap = {};
        nodes.forEach(n => nodeMap[n.id] = n);

        let html = `<div class="viz-wrapper">
            <div class="viz-label">${escapeHtml(label)} (Double Linked List)</div>
            <div class="viz-list">
                <div class="viz-list-null">NULL</div>
                <span class="viz-list-arrow">⟷</span>`;

        let currentId = head;
        let visited = new Set();
        let count = 0;
        const maxNodes = 50;

        while (currentId !== -1 && !visited.has(currentId) && count < maxNodes) {
            visited.add(currentId);
            const node = nodeMap[currentId];
            if (!node) break;

            html += `
                <div class="viz-list-node">
                    <div class="viz-list-node-box">
                        <div class="viz-list-node-pointer">${node.prev === -1 ? 'null' : node.prev}</div>
                        <div class="viz-list-node-value">${escapeHtml(String(node.value))}</div>
                        <div class="viz-list-node-pointer">${node.next === -1 ? 'null' : node.next}</div>
                    </div>
                </div>`;

            if (node.next !== -1 && nodeMap[node.next]) {
                html += '<span class="viz-list-arrow">⟷</span>';
            }

            currentId = node.next;
            count++;
        }

        html += `<span class="viz-list-arrow">⟷</span>
            <div class="viz-list-null">NULL</div>
        </div></div>`;

        elements.vizContainer.innerHTML = html;
    }

    function renderBinaryTree(data, label, kind) {
        const nodes = data.nodes || [];
        const root = data.root;

        if (nodes.length === 0 || root === -1) {
            renderEmpty(label, kind === 'bst' ? 'BST' : 'Binary Tree');
            return;
        }

        const nodeMap = {};
        nodes.forEach(n => nodeMap[n.id] = n);

        // 计算树的布局
        const layout = calculateTreeLayout(root, nodeMap);
        const { width, height, positions } = layout;

        const svgWidth = Math.max(400, width * 80 + 100);
        const svgHeight = Math.max(200, height * 80 + 60);
        const nodeRadius = 24;

        let svg = `<div class="viz-wrapper">
            <div class="viz-label">${escapeHtml(label)} (${kind === 'bst' ? 'BST' : 'Binary Tree'})</div>
            <svg class="viz-tree-svg" viewBox="0 0 ${svgWidth} ${svgHeight}" style="max-width: 100%;">`;

        // 绘制边
        for (const [id, pos] of Object.entries(positions)) {
            const node = nodeMap[parseInt(id)];
            if (!node) continue;

            if (node.left !== -1 && positions[node.left]) {
                const childPos = positions[node.left];
                svg += `<line x1="${pos.x}" y1="${pos.y}" x2="${childPos.x}" y2="${childPos.y}" 
                         stroke="var(--border-default)" stroke-width="2"/>`;
            }
            if (node.right !== -1 && positions[node.right]) {
                const childPos = positions[node.right];
                svg += `<line x1="${pos.x}" y1="${pos.y}" x2="${childPos.x}" y2="${childPos.y}" 
                         stroke="var(--border-default)" stroke-width="2"/>`;
            }
        }

        // 绘制节点
        for (const [id, pos] of Object.entries(positions)) {
            const node = nodeMap[parseInt(id)];
            if (!node) continue;

            const isRoot = parseInt(id) === root;
            svg += `
                <circle cx="${pos.x}" cy="${pos.y}" r="${nodeRadius}" 
                        class="${isRoot ? 'root' : ''}"
                        fill="${isRoot ? 'var(--accent-brand)' : 'var(--bg-surface)'}"
                        stroke="${isRoot ? 'var(--accent-brand)' : 'var(--border-default)'}" 
                        stroke-width="2"/>
                <text x="${pos.x}" y="${pos.y}" 
                      class="${isRoot ? 'root' : ''}"
                      fill="${isRoot ? 'white' : 'var(--text-primary)'}"
                      font-family="var(--font-mono)" font-size="14" font-weight="500"
                      text-anchor="middle" dominant-baseline="central">
                    ${escapeHtml(String(node.value))}
                </text>`;
        }

        svg += `</svg></div>`;
        elements.vizContainer.innerHTML = svg;
    }

    function calculateTreeLayout(rootId, nodeMap) {
        const positions = {};
        let maxWidth = 0;
        let maxDepth = 0;

        function getHeight(id) {
            if (id === -1 || !nodeMap[id]) return 0;
            const node = nodeMap[id];
            return 1 + Math.max(getHeight(node.left), getHeight(node.right));
        }

        function layout(id, depth, left, right) {
            if (id === -1 || !nodeMap[id]) return;

            const node = nodeMap[id];
            const x = (left + right) / 2;
            const y = depth * 70 + 40;

            positions[id] = { x, y };
            maxWidth = Math.max(maxWidth, right);
            maxDepth = Math.max(maxDepth, depth);

            const mid = (left + right) / 2;
            layout(node.left, depth + 1, left, mid);
            layout(node.right, depth + 1, mid, right);
        }

        const height = getHeight(rootId);
        const width = Math.pow(2, height) * 40;
        layout(rootId, 0, 0, width);

        return { width: maxWidth / 40, height: maxDepth + 1, positions };
    }

    function renderGraph(data, label, kind) {
        const nodes = data.nodes || [];
        const edges = data.edges || [];

        if (nodes.length === 0) {
            const typeName = kind === 'graph_directed' ? 'Directed Graph' :
                kind === 'graph_weighted' ? 'Weighted Graph' : 'Graph';
            renderEmpty(label, typeName);
            return;
        }

        const isDirected = kind === 'graph_directed';
        const isWeighted = kind === 'graph_weighted';
        const typeName = isDirected ? 'Directed Graph' :
            isWeighted ? 'Weighted Graph' : 'Undirected Graph';

        const width = 500;
        const height = 350;
        const centerX = width / 2;
        const centerY = height / 2;
        const radius = Math.min(120, 30 * Math.max(3, nodes.length));
        const nodeRadius = 24;

        // 计算节点位置（圆形布局）
        const nodePositions = {};
        nodes.forEach((node, i) => {
            const angle = (2 * Math.PI * i) / nodes.length - Math.PI / 2;
            nodePositions[node.id] = {
                x: centerX + radius * Math.cos(angle),
                y: centerY + radius * Math.sin(angle)
            };
        });

        let svg = `<div class="viz-wrapper">
            <div class="viz-label">${escapeHtml(label)} (${typeName})</div>
            <div class="viz-graph">
                <svg class="viz-graph-svg" viewBox="0 0 ${width} ${height}">
                    <defs>
                        <marker id="arrowhead" markerWidth="10" markerHeight="7" 
                                refX="9" refY="3.5" orient="auto">
                            <polygon points="0 0, 10 3.5, 0 7" fill="var(--text-secondary)" />
                        </marker>
                    </defs>`;

        // 绘制边
        edges.forEach(edge => {
            const from = nodePositions[edge.from];
            const to = nodePositions[edge.to];
            if (!from || !to) return;

            // 计算边的端点（考虑节点半径）
            const dx = to.x - from.x;
            const dy = to.y - from.y;
            const dist = Math.sqrt(dx * dx + dy * dy);
            if (dist === 0) return;

            const offsetX = (dx / dist) * nodeRadius;
            const offsetY = (dy / dist) * nodeRadius;

            const x1 = from.x + offsetX;
            const y1 = from.y + offsetY;
            const x2 = to.x - offsetX;
            const y2 = to.y - offsetY;

            svg += `<line class="viz-graph-edge" 
                          x1="${x1}" y1="${y1}" x2="${x2}" y2="${y2}"
                          ${isDirected ? 'marker-end="url(#arrowhead)"' : ''} />`;

            if (isWeighted && edge.weight !== undefined) {
                const midX = (from.x + to.x) / 2;
                const midY = (from.y + to.y) / 2 - 8;
                svg += `<text class="viz-graph-edge-weight" x="${midX}" y="${midY}">${edge.weight}</text>`;
            }
        });

        // 绘制节点
        nodes.forEach(node => {
            const pos = nodePositions[node.id];
            svg += `
                <circle class="viz-graph-node-circle" cx="${pos.x}" cy="${pos.y}" r="${nodeRadius}" />
                <text class="viz-graph-node-label" x="${pos.x}" y="${pos.y}">
                    ${escapeHtml(node.label || String(node.id))}
                </text>`;
        });

        svg += `</svg></div></div>`;
        elements.vizContainer.innerHTML = svg;
    }

    // ============================================
    // 服务器通信
    // ============================================

    async function pingServer() {
        elements.connectionStatus.className = 'connection-status checking';

        try {
            const response = await fetch(`${state.serverUrl}/ping`, {
                method: 'GET',
                headers: { 'Accept': 'application/json' }
            });

            if (response.ok) {
                state.connected = true;
                elements.connectionStatus.className = 'connection-status connected';
                return true;
            } else {
                throw new Error('Server returned error');
            }
        } catch (error) {
            state.connected = false;
            elements.connectionStatus.className = 'connection-status disconnected';
            return false;
        }
    }

    async function executeRemoteCode() {
        const code = elements.codeEditor.value;

        if (!code.trim()) {
            showToast('请输入代码', 'error');
            return;
        }

        if (!state.connected) {
            const connected = await pingServer();
            if (!connected) {
                showToast('无法连接到服务器', 'error');
                return;
            }
        }

        showLoading(true, '执行中...');
        elements.runBtn.disabled = true;
        showStatus('running', '执行中...');

        try {
            const response = await fetch(`${state.serverUrl}/execute`, {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json',
                    'Accept': 'application/json'
                },
                body: JSON.stringify({
                    lang: state.language,
                    code: code,
                    timeout_ms: CONFIG.defaultTimeout
                })
            });

            const data = await response.json();

            if (response.ok && data.toml) {
                state.tomlData = data.toml;
                parseAndRenderToml(data.toml);
                showStatus('success', '执行成功', data.duration_ms);
            } else if (data.error) {
                showStatus('error', data.message || '执行失败');
                showToast(data.message || '执行失败', 'error');
            } else {
                showStatus('error', '未知错误');
            }
        } catch (error) {
            console.error('执行请求失败:', error);
            showStatus('error', '请求失败: ' + error.message);
            showToast('请求失败: ' + error.message, 'error');
        } finally {
            showLoading(false);
            elements.runBtn.disabled = false;
        }
    }

    // ============================================
    // 本地执行
    // ============================================

    async function loadDs4vizModule() {
        if (state.ds4vizModule) {
            return state.ds4vizModule;
        }

        try {
            showLoading(true, '加载 ds4viz 库...');

            // 动态导入模块
            const module = await import(state.libraryPath);
            state.ds4vizModule = module;

            showLoading(false);
            return module;
        } catch (error) {
            showLoading(false);
            console.error('加载 ds4viz 失败:', error);
            throw new Error(`无法加载 ds4viz 库: ${error.message}`);
        }
    }

    async function executeLocalCode() {
        const code = elements.codeEditor.value;

        if (!code.trim()) {
            showToast('请输入代码', 'error');
            return;
        }

        showLoading(true, '执行中...');
        elements.runBtn.disabled = true;
        showStatus('running', '执行中...');

        const startTime = performance.now();

        try {
            // 加载 ds4viz 模块
            const ds4viz = await loadDs4vizModule();

            // 创建执行环境
            // 移除 import 语句并提取实际代码
            const cleanedCode = code
                .replace(/import\s+\{[^}]*\}\s+from\s+['"][^'"]*['"]\s*;?/g, '')
                .replace(/import\s+\*\s+as\s+\w+\s+from\s+['"][^'"]*['"]\s*;?/g, '')
                .replace(/const\s+\{[^}]*\}\s*=\s*require\s*\(['"][^'"]*['"]\)\s*;?/g, '')
                .trim();

            // 创建包含 ds4viz 所有导出的上下文
            const contextVars = {};
            for (const [key, value] of Object.entries(ds4viz)) {
                contextVars[key] = value;
            }

            // 捕获生成的 TOML
            let generatedToml = null;
            let capturedStructure = null;

            // 包装 withContext 以捕获结果
            const originalWithContext = ds4viz.withContext;
            contextVars.withContext = async (structure, callback) => {
                capturedStructure = structure;
                await originalWithContext(structure, callback);
                // 尝试获取 TOML 字符串
                if (ds4viz.getTomlString) {
                    generatedToml = ds4viz.getTomlString(structure);
                } else if (structure._getToml) {
                    generatedToml = structure._getToml();
                } else if (structure.toml) {
                    generatedToml = structure.toml;
                }
            };

            // 创建异步函数来执行代码
            const AsyncFunction = Object.getPrototypeOf(async function () { }).constructor;
            const argNames = Object.keys(contextVars);
            const argValues = Object.values(contextVars);

            // 包装代码以确保返回 Promise
            const wrappedCode = `
                return (async () => {
                    ${cleanedCode}
                })();
            `;

            const fn = new AsyncFunction(...argNames, wrappedCode);
            await fn(...argValues);

            const duration = Math.round(performance.now() - startTime);

            if (generatedToml) {
                state.tomlData = generatedToml;
                parseAndRenderToml(generatedToml);
                showStatus('success', '执行成功', duration);
            } else {
                showStatus('error', '未能获取 TOML 输出');
                showToast('代码执行完成但未生成 TOML。请确保使用 withContext 包装数据结构操作。', 'error');
            }

        } catch (error) {
            console.error('本地执行失败:', error);
            const duration = Math.round(performance.now() - startTime);
            showStatus('error', '执行失败: ' + error.message);
            showToast('执行失败: ' + error.message, 'error');
        } finally {
            showLoading(false);
            elements.runBtn.disabled = false;
        }
    }

    async function executeCode() {
        if (state.mode === 'local') {
            await executeLocalCode();
        } else {
            await executeRemoteCode();
        }
    }

    // ============================================
    // UI 状态更新
    // ============================================

    function showLoading(show, text = '执行中...') {
        elements.loadingOverlay.style.display = show ? 'flex' : 'none';
        elements.loadingText.textContent = text;
    }

    function showStatus(type, message, duration) {
        elements.executionStatus.style.display = 'flex';
        elements.executionStatus.className = `execution-status ${type}`;
        elements.executionStatus.querySelector('.status-text').textContent = message;
        elements.executionStatus.querySelector('.status-duration').textContent =
            duration !== undefined ? `${duration}ms` : '';
    }

    function updateLanguageUI() {
        const color = LANGUAGE_COLORS[state.language] || '#666';
        elements.langColorDot.style.backgroundColor = color;
        elements.languageSelect.value = state.language;

        // 本地模式仅支持 JavaScript
        if (state.mode === 'local' && state.language !== 'javascript') {
            state.language = 'javascript';
            elements.languageSelect.value = 'javascript';
            elements.langColorDot.style.backgroundColor = LANGUAGE_COLORS.javascript;
        }

        // 更新语法高亮
        updateHighlight();
    }

    function loadTemplate() {
        const template = TEMPLATES[state.language];
        if (typeof template === 'function') {
            elements.codeEditor.value = template(state.libraryPath);
        } else {
            elements.codeEditor.value = template || '';
        }
        updateHighlight();
    }

    // ============================================
    // 自动播放
    // ============================================

    function startAutoPlay() {
        if (state.isPlaying) return;

        state.isPlaying = true;
        elements.autoPlayBtn.innerHTML = `
            <svg width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
                <rect x="6" y="4" width="4" height="16"></rect>
                <rect x="14" y="4" width="4" height="16"></rect>
            </svg>
        `;
        updateVizControls();

        state.playInterval = setInterval(() => {
            if (state.currentStep < state.totalSteps - 1) {
                state.currentStep++;
                updateVizControls();
                renderCurrentState();
            } else {
                stopAutoPlay();
            }
        }, CONFIG.autoPlayDelay);
    }

    function stopAutoPlay() {
        state.isPlaying = false;
        if (state.playInterval) {
            clearInterval(state.playInterval);
            state.playInterval = null;
        }
        elements.autoPlayBtn.innerHTML = `
            <svg width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
                <polygon points="5 3 19 12 5 21 5 3"></polygon>
            </svg>
        `;
        updateVizControls();
    }

    // ============================================
    // 事件处理
    // ============================================

    function setupEventListeners() {
        // 模式切换
        elements.remoteModeBtn.addEventListener('click', () => {
            if (state.mode === 'remote') return;
            state.mode = 'remote';
            elements.remoteModeBtn.classList.add('active');
            elements.localModeBtn.classList.remove('active');
            elements.remoteInputWrapper.classList.remove('hidden');
            elements.localInputWrapper.classList.add('hidden');
            elements.languageSelect.disabled = false;
            pingServer();
        });

        elements.localModeBtn.addEventListener('click', () => {
            if (state.mode === 'local') return;
            state.mode = 'local';
            state.ds4vizModule = null; // 重置模块缓存
            elements.localModeBtn.classList.add('active');
            elements.remoteModeBtn.classList.remove('active');
            elements.remoteInputWrapper.classList.add('hidden');
            elements.localInputWrapper.classList.remove('hidden');
            elements.connectionStatus.className = 'connection-status';

            // 本地模式强制使用 JavaScript
            if (state.language !== 'javascript') {
                state.language = 'javascript';
                updateLanguageUI();
                loadTemplate();
            }
            elements.languageSelect.disabled = true;
        });

        // 服务器/库路径
        elements.serverUrl.addEventListener('change', () => {
            state.serverUrl = elements.serverUrl.value.trim();
            pingServer();
        });

        elements.libraryPath.addEventListener('change', () => {
            state.libraryPath = elements.libraryPath.value.trim();
            state.ds4vizModule = null; // 重置模块缓存
        });

        elements.pingBtn.addEventListener('click', pingServer);

        // TOML 上传
        elements.uploadTomlBtn.addEventListener('click', () => {
            elements.tomlFileInput.click();
        });

        elements.tomlFileInput.addEventListener('change', (e) => {
            const file = e.target.files[0];
            if (!file) return;

            const reader = new FileReader();
            reader.onload = (event) => {
                state.tomlData = event.target.result;
                parseAndRenderToml(state.tomlData);
                showToast('TOML 已加载', 'success');
            };
            reader.onerror = () => {
                showToast('文件读取失败', 'error');
            };
            reader.readAsText(file);
            e.target.value = '';
        });

        // TOML 下载
        elements.downloadTomlBtn.addEventListener('click', () => {
            if (!state.tomlData) {
                showToast('没有可下载的 TOML 数据', 'error');
                return;
            }

            const blob = new Blob([state.tomlData], { type: 'text/plain' });
            const url = URL.createObjectURL(blob);
            const a = document.createElement('a');
            a.href = url;
            a.download = 'trace.toml';
            a.click();
            URL.revokeObjectURL(url);
            showToast('TOML 已下载', 'success');
        });

        // 关于弹窗
        elements.aboutBtn.addEventListener('click', () => {
            elements.aboutModal.classList.add('active');
        });

        elements.closeAboutBtn.addEventListener('click', () => {
            elements.aboutModal.classList.remove('active');
        });

        elements.aboutModal.addEventListener('click', (e) => {
            if (e.target === elements.aboutModal) {
                elements.aboutModal.classList.remove('active');
            }
        });

        // 语言选择
        elements.languageSelect.addEventListener('change', () => {
            state.language = elements.languageSelect.value;
            updateLanguageUI();
            loadTemplate();
        });

        // 运行按钮
        elements.runBtn.addEventListener('click', executeCode);

        // 代码编辑器
        elements.codeEditor.addEventListener('input', debounce(handleEditorInput, 50));
        elements.codeEditor.addEventListener('scroll', syncScroll);
        elements.codeEditor.addEventListener('keydown', handleEditorKeydown);
        elements.codeEditor.addEventListener('click', hideAutocomplete);
        elements.codeEditor.addEventListener('blur', () => {
            setTimeout(hideAutocomplete, 150);
        });

        // 可视化控制
        elements.stepFirstBtn.addEventListener('click', () => {
            stopAutoPlay();
            state.currentStep = 0;
            updateVizControls();
            renderCurrentState();
        });

        elements.stepBackBtn.addEventListener('click', () => {
            stopAutoPlay();
            if (state.currentStep > 0) {
                state.currentStep--;
                updateVizControls();
                renderCurrentState();
            }
        });

        elements.stepForwardBtn.addEventListener('click', () => {
            stopAutoPlay();
            if (state.currentStep < state.totalSteps - 1) {
                state.currentStep++;
                updateVizControls();
                renderCurrentState();
            }
        });

        elements.stepLastBtn.addEventListener('click', () => {
            stopAutoPlay();
            state.currentStep = state.totalSteps - 1;
            updateVizControls();
            renderCurrentState();
        });

        elements.autoPlayBtn.addEventListener('click', () => {
            if (state.isPlaying) {
                stopAutoPlay();
            } else {
                startAutoPlay();
            }
        });

        // 键盘快捷键
        document.addEventListener('keydown', (e) => {
            // Ctrl+Enter 运行
            if ((e.ctrlKey || e.metaKey) && e.key === 'Enter') {
                e.preventDefault();
                executeCode();
                return;
            }

            // 编辑器非焦点时的快捷键
            if (document.activeElement !== elements.codeEditor) {
                if (e.key === 'ArrowLeft' && !elements.stepBackBtn.disabled) {
                    e.preventDefault();
                    elements.stepBackBtn.click();
                } else if (e.key === 'ArrowRight' && !elements.stepForwardBtn.disabled) {
                    e.preventDefault();
                    elements.stepForwardBtn.click();
                } else if (e.key === ' ' && !elements.autoPlayBtn.disabled) {
                    e.preventDefault();
                    elements.autoPlayBtn.click();
                } else if (e.key === 'Home' && !elements.stepFirstBtn.disabled) {
                    e.preventDefault();
                    elements.stepFirstBtn.click();
                } else if (e.key === 'End' && !elements.stepLastBtn.disabled) {
                    e.preventDefault();
                    elements.stepLastBtn.click();
                }
            }

            // Escape 关闭弹窗
            if (e.key === 'Escape') {
                elements.aboutModal.classList.remove('active');
                hideAutocomplete();
            }
        });

        // 窗口大小变化时重新渲染
        window.addEventListener('resize', debounce(() => {
            if (state.parsedToml) {
                renderCurrentState();
            }
        }, 250));
    }

    // ============================================
    // 初始化
    // ============================================

    function init() {
        initElements();
        setupEventListeners();

        // 设置初始状态
        elements.serverUrl.value = state.serverUrl;
        elements.libraryPath.value = state.libraryPath;

        // 默认使用 JavaScript（本地模式友好）
        state.language = CONFIG.defaultLanguage;
        updateLanguageUI();
        loadTemplate();

        // 初始化高亮
        updateHighlight();

        // 初始连接测试
        if (state.mode === 'remote') {
            pingServer();
        }

        // 定期检查连接
        setInterval(() => {
            if (state.mode === 'remote') {
                pingServer();
            }
        }, CONFIG.pingInterval);

        console.log('ds4viz Demo initialized');
    }

    if (document.readyState === 'loading') {
        document.addEventListener('DOMContentLoaded', init);
    } else {
        init();
    }
})();
