/**
 * ds4viz Demo - Main JavaScript
 * Version: 2.0.0
 * 
 * 数据结构可视化教学平台 Demo 前端
 * 支持远程执行(Python, Rust, Lua)和本地执行(JavaScript)
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
        toastDuration: 4000,
        autocompleteMinChars: 2,
        autocompleteDebounce: 100
    };

    const LANGUAGE_COLORS = {
        python: '#3776ab',
        javascript: '#f7df1e',
        lua: '#000080',
        rust: '#dea584'
    };

    // 支持的语言列表
    const SUPPORTED_LANGUAGES = ['python', 'javascript', 'lua', 'rust'];
    const REMOTE_ONLY_LANGUAGES = ['python', 'lua', 'rust'];

    // ============================================
    // 语法规则定义
    // ============================================

    const SYNTAX_RULES = {
        python: {
            keywords: [
                'def', 'class', 'if', 'elif', 'else', 'for', 'while', 'try', 'except',
                'finally', 'with', 'as', 'import', 'from', 'return', 'yield', 'raise',
                'pass', 'break', 'continue', 'and', 'or', 'not', 'in', 'is', 'lambda',
                'global', 'nonlocal', 'async', 'await', 'assert', 'del'
            ],
            constants: ['None', 'True', 'False'],
            builtins: ['print', 'len', 'range', 'str', 'int', 'float', 'list', 'dict', 'set', 'tuple', 'type', 'isinstance', 'hasattr', 'getattr', 'setattr', 'open', 'input', 'map', 'filter', 'zip', 'enumerate', 'sorted', 'reversed', 'min', 'max', 'sum', 'abs', 'all', 'any'],
            ds4vizImports: ['ds4viz', 'dv'],
            ds4vizFunctions: ['stack', 'queue', 'single_linked_list', 'double_linked_list', 'binary_tree', 'binary_search_tree', 'heap', 'graph_undirected', 'graph_directed', 'graph_weighted', 'config'],
            ds4vizMethods: ['push', 'pop', 'clear', 'enqueue', 'dequeue', 'insert_head', 'insert_tail', 'insert_after', 'insert_before', 'delete', 'delete_head', 'delete_tail', 'reverse', 'insert_root', 'insert_left', 'insert_right', 'update_value', 'insert', 'extract', 'add_node', 'add_edge', 'remove_node', 'remove_edge', 'update_node_label', 'update_weight'],
            stringDelimiters: [{ start: '"""', end: '"""' }, { start: "'''", end: "'''" }, { start: '"', end: '"' }, { start: "'", end: "'" }],
            lineComment: '#',
            decorator: '@'
        },
        javascript: {
            keywords: [
                'function', 'const', 'let', 'var', 'if', 'else', 'for', 'while', 'do',
                'switch', 'case', 'break', 'continue', 'return', 'try', 'catch', 'finally',
                'throw', 'new', 'class', 'extends', 'import', 'export', 'from', 'default',
                'async', 'await', 'yield', 'typeof', 'instanceof', 'delete', 'void', 'of', 'in'
            ],
            constants: ['true', 'false', 'null', 'undefined', 'NaN', 'Infinity', 'this', 'super'],
            builtins: ['console', 'Math', 'JSON', 'Object', 'Array', 'String', 'Number', 'Boolean', 'Function', 'Promise', 'Map', 'Set', 'WeakMap', 'WeakSet', 'Symbol', 'Date', 'RegExp', 'Error', 'parseInt', 'parseFloat', 'isNaN', 'isFinite', 'encodeURI', 'decodeURI', 'setTimeout', 'setInterval', 'clearTimeout', 'clearInterval', 'fetch'],
            ds4vizImports: ['ds4viz'],
            ds4vizFunctions: ['stack', 'queue', 'singleLinkedList', 'doubleLinkedList', 'binaryTree', 'binarySearchTree', 'heap', 'graphUndirected', 'graphDirected', 'graphWeighted', 'config', 'withContext'],
            ds4vizMethods: ['push', 'pop', 'clear', 'enqueue', 'dequeue', 'insertHead', 'insertTail', 'insertAfter', 'insertBefore', 'delete', 'deleteHead', 'deleteTail', 'reverse', 'insertRoot', 'insertLeft', 'insertRight', 'updateValue', 'insert', 'extract', 'addNode', 'addEdge', 'removeNode', 'removeEdge', 'updateNodeLabel', 'updateWeight'],
            stringDelimiters: [{ start: '`', end: '`', template: true }, { start: '"', end: '"' }, { start: "'", end: "'" }],
            lineComment: '//',
            blockComment: { start: '/*', end: '*/' }
        },
        lua: {
            keywords: [
                'and', 'break', 'do', 'else', 'elseif', 'end', 'for', 'function', 'goto',
                'if', 'in', 'local', 'not', 'or', 'repeat', 'return', 'then', 'until', 'while'
            ],
            constants: ['nil', 'true', 'false'],
            builtins: ['print', 'type', 'pairs', 'ipairs', 'next', 'select', 'tonumber', 'tostring', 'error', 'assert', 'pcall', 'xpcall', 'require', 'load', 'loadfile', 'dofile', 'rawget', 'rawset', 'rawequal', 'rawlen', 'setmetatable', 'getmetatable', 'collectgarbage', 'table', 'string', 'math', 'io', 'os', 'coroutine', 'debug', 'package', 'utf8'],
            ds4vizImports: ['ds4viz'],
            ds4vizFunctions: ['stack', 'queue', 'singleLinkedList', 'doubleLinkedList', 'binaryTree', 'binarySearchTree', 'heap', 'graphUndirected', 'graphDirected', 'graphWeighted', 'config', 'withContext'],
            ds4vizMethods: ['push', 'pop', 'clear', 'enqueue', 'dequeue', 'insertHead', 'insertTail', 'insertAfter', 'insertBefore', 'delete', 'deleteHead', 'deleteTail', 'reverse', 'insertRoot', 'insertLeft', 'insertRight', 'updateValue', 'insert', 'extract', 'addNode', 'addEdge', 'removeNode', 'removeEdge', 'updateNodeLabel', 'updateWeight'],
            stringDelimiters: [{ start: '[[', end: ']]' }, { start: '"', end: '"' }, { start: "'", end: "'" }],
            lineComment: '--',
            blockComment: { start: '--[[', end: ']]' }
        },
        rust: {
            keywords: [
                'as', 'async', 'await', 'break', 'const', 'continue', 'crate', 'dyn', 'else',
                'enum', 'extern', 'fn', 'for', 'if', 'impl', 'in', 'let', 'loop', 'match',
                'mod', 'move', 'mut', 'pub', 'ref', 'return', 'self', 'Self', 'static',
                'struct', 'super', 'trait', 'type', 'unsafe', 'use', 'where', 'while'
            ],
            constants: ['true', 'false', 'Some', 'None', 'Ok', 'Err'],
            types: ['i8', 'i16', 'i32', 'i64', 'i128', 'isize', 'u8', 'u16', 'u32', 'u64', 'u128', 'usize', 'f32', 'f64', 'bool', 'char', 'str', 'String', 'Vec', 'Option', 'Result', 'Box', 'Rc', 'Arc', 'Cell', 'RefCell', 'HashMap', 'HashSet', 'BTreeMap', 'BTreeSet'],
            builtins: ['println', 'print', 'eprintln', 'eprint', 'format', 'panic', 'assert', 'assert_eq', 'assert_ne', 'debug_assert', 'todo', 'unimplemented', 'unreachable', 'cfg', 'include', 'include_str', 'include_bytes', 'env', 'option_env', 'concat', 'stringify', 'line', 'column', 'file', 'module_path', 'vec'],
            ds4vizImports: ['ds4viz'],
            ds4vizFunctions: ['stack', 'queue', 'single_linked_list', 'double_linked_list', 'binary_tree', 'binary_search_tree', 'heap', 'graph_undirected', 'graph_directed', 'graph_weighted', 'config', 'stack_with_output', 'queue_with_output'],
            ds4vizMethods: ['push', 'pop', 'clear', 'enqueue', 'dequeue', 'insert_head', 'insert_tail', 'insert_after', 'insert_before', 'delete', 'delete_head', 'delete_tail', 'reverse', 'insert_root', 'insert_left', 'insert_right', 'update_value', 'insert', 'extract', 'add_node', 'add_edge', 'remove_node', 'remove_edge', 'update_node_label', 'update_weight'],
            ds4vizTypes: ['Stack', 'Queue', 'SingleLinkedList', 'DoubleLinkedList', 'BinaryTree', 'BinarySearchTree', 'Heap', 'HeapType', 'GraphUndirected', 'GraphDirected', 'GraphWeighted', 'Value', 'NumericValue', 'Config'],
            stringDelimiters: [{ start: 'r#"', end: '"#' }, { start: 'r"', end: '"' }, { start: '"', end: '"' }],
            charDelimiter: "'",
            lineComment: '//',
            blockComment: { start: '/*', end: '*/' },
            lifetime: "'"
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
        lua: `local ds4viz = require("ds4viz")

ds4viz.withContext(ds4viz.stack("demo_stack"), function(s)
    s:push(10)
    s:push(20)
    s:push(30)
    s:pop()
    s:push(40)
end)
`,
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
        editor: {
            cursorLine: 0,
            cursorCol: 0
        },
        autocomplete: {
            visible: false,
            items: [],
            selectedIndex: 0,
            prefix: '',
            startPos: 0
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
            editorContainer: document.getElementById('editorContainer'),
            editorArea: document.getElementById('editorArea'),
            codeEditor: document.getElementById('codeEditor'),
            codeHighlight: document.getElementById('codeHighlight'),
            lineNumbers: document.getElementById('lineNumbers'),
            autocompletePopup: document.getElementById('autocompletePopup'),
            autocompleteList: document.getElementById('autocompleteList'),
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
        const map = {
            '&': '&amp;',
            '<': '&lt;',
            '>': '&gt;',
            '"': '&quot;',
            "'": '&#39;'
        };
        return String(text).replace(/[&<>"']/g, c => map[c]);
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

        toast.querySelector('.toast-close').addEventListener('click', () => toast.remove());
        elements.toastContainer.appendChild(toast);

        setTimeout(() => {
            if (toast.parentNode) toast.remove();
        }, CONFIG.toastDuration);
    }

    // ============================================
    // 语法高亮器
    // ============================================

    class SyntaxHighlighter {
        constructor(language) {
            this.rules = SYNTAX_RULES[language] || SYNTAX_RULES.javascript;
            this.language = language;
        }

        highlight(code) {
            const tokens = this.tokenize(code);
            return tokens.map(t => this.renderToken(t)).join('');
        }

        tokenize(code) {
            const tokens = [];
            let pos = 0;
            const len = code.length;

            while (pos < len) {
                let matched = false;

                // 尝试匹配各种 token 类型
                matched = matched || this.tryMatchBlockComment(code, pos, tokens);
                matched = matched || this.tryMatchLineComment(code, pos, tokens);
                matched = matched || this.tryMatchString(code, pos, tokens);
                matched = matched || this.tryMatchNumber(code, pos, tokens);
                matched = matched || this.tryMatchDecorator(code, pos, tokens);
                matched = matched || this.tryMatchLifetime(code, pos, tokens);
                matched = matched || this.tryMatchWord(code, pos, tokens);
                matched = matched || this.tryMatchOperator(code, pos, tokens);

                if (!matched) {
                    // 普通字符
                    const char = code[pos];
                    if (tokens.length > 0 && tokens[tokens.length - 1].type === 'plain') {
                        tokens[tokens.length - 1].value += char;
                    } else {
                        tokens.push({ type: 'plain', value: char });
                    }
                    pos++;
                } else {
                    pos = tokens.reduce((sum, t) => sum + t.value.length, 0);
                }
            }

            return tokens;
        }

        tryMatchBlockComment(code, pos, tokens) {
            const rules = this.rules;
            if (!rules.blockComment) return false;

            const { start, end } = rules.blockComment;
            if (!code.startsWith(start, pos)) return false;

            let endPos = code.indexOf(end, pos + start.length);
            if (endPos === -1) endPos = code.length - end.length;

            tokens.push({
                type: 'comment',
                value: code.slice(pos, endPos + end.length)
            });
            return true;
        }

        tryMatchLineComment(code, pos, tokens) {
            const comment = this.rules.lineComment;
            if (!comment || !code.startsWith(comment, pos)) return false;

            let endPos = code.indexOf('\n', pos);
            if (endPos === -1) endPos = code.length;

            tokens.push({
                type: 'comment',
                value: code.slice(pos, endPos)
            });
            return true;
        }

        tryMatchString(code, pos, tokens) {
            const delimiters = this.rules.stringDelimiters || [];

            for (const delim of delimiters) {
                if (!code.startsWith(delim.start, pos)) continue;

                let endPos = pos + delim.start.length;
                let escaped = false;

                while (endPos < code.length) {
                    const char = code[endPos];

                    if (escaped) {
                        escaped = false;
                        endPos++;
                        continue;
                    }

                    if (char === '\\' && !delim.template) {
                        escaped = true;
                        endPos++;
                        continue;
                    }

                    if (code.startsWith(delim.end, endPos)) {
                        endPos += delim.end.length;
                        break;
                    }

                    // 单行字符串不能跨行（除了模板字符串和多行字符串）
                    if (char === '\n' && delim.start.length === 1 && !delim.template) {
                        break;
                    }

                    endPos++;
                }

                tokens.push({
                    type: 'string',
                    value: code.slice(pos, endPos)
                });
                return true;
            }

            // Rust char literal
            if (this.language === 'rust' && code[pos] === "'" && pos + 2 < code.length) {
                // 检查是否是 char literal 而不是 lifetime
                const next = code[pos + 1];
                if (next === '\\' || (code[pos + 2] === "'" && /[^a-zA-Z_]/.test(next) === false)) {
                    let endPos = pos + 2;
                    if (next === '\\') endPos++;
                    if (code[endPos] === "'") endPos++;
                    tokens.push({ type: 'string', value: code.slice(pos, endPos) });
                    return true;
                }
            }

            return false;
        }

        tryMatchNumber(code, pos, tokens) {
            // 匹配各种数字格式
            const patterns = [
                /^0x[0-9a-fA-F_]+/,  // 十六进制
                /^0o[0-7_]+/,        // 八进制
                /^0b[01_]+/,         // 二进制
                /^\d[\d_]*\.[\d_]*(?:[eE][+-]?\d+)?/, // 浮点数
                /^\d[\d_]*[eE][+-]?\d+/, // 科学计数法
                /^\d[\d_]*/          // 整数
            ];

            for (const pattern of patterns) {
                const match = code.slice(pos).match(pattern);
                if (match) {
                    // 后面可能有类型后缀 (Rust)
                    let value = match[0];
                    const suffix = code.slice(pos + value.length).match(/^(?:i8|i16|i32|i64|i128|isize|u8|u16|u32|u64|u128|usize|f32|f64)/);
                    if (suffix) value += suffix[0];

                    tokens.push({ type: 'number', value });
                    return true;
                }
            }

            return false;
        }

        tryMatchDecorator(code, pos, tokens) {
            if (this.language !== 'python' || code[pos] !== '@') return false;

            const match = code.slice(pos).match(/^@[a-zA-Z_][a-zA-Z0-9_]*/);
            if (match) {
                tokens.push({ type: 'decorator', value: match[0] });
                return true;
            }
            return false;
        }

        tryMatchLifetime(code, pos, tokens) {
            if (this.language !== 'rust') return false;

            // Rust lifetime: 'a, 'static, 'lifetime
            const match = code.slice(pos).match(/^'(?:static|[a-zA-Z_][a-zA-Z0-9_]*)/);
            if (match && (pos === 0 || /[\s<(,:]/.test(code[pos - 1]))) {
                tokens.push({ type: 'lifetime', value: match[0] });
                return true;
            }
            return false;
        }

        tryMatchWord(code, pos, tokens) {
            const match = code.slice(pos).match(/^[a-zA-Z_][a-zA-Z0-9_]*/);
            if (!match) return false;

            const word = match[0];
            let type = 'plain';

            const rules = this.rules;

            // 检查 ds4viz 相关
            if (rules.ds4vizFunctions?.includes(word) || rules.ds4vizMethods?.includes(word)) {
                type = 'ds4viz';
            } else if (rules.ds4vizTypes?.includes(word)) {
                type = 'ds4viz';
            } else if (rules.ds4vizImports?.includes(word)) {
                type = 'ds4viz';
            }
            // 检查关键字
            else if (rules.keywords?.includes(word)) {
                type = 'keyword';
            }
            // 检查常量
            else if (rules.constants?.includes(word)) {
                type = 'constant';
            }
            // 检查类型 (Rust)
            else if (rules.types?.includes(word)) {
                type = 'type';
            }
            // 检查内置函数
            else if (rules.builtins?.includes(word)) {
                type = 'function';
            }
            // 检查是否是函数调用
            else {
                const nextNonSpace = code.slice(pos + word.length).match(/^\s*(.)/);
                if (nextNonSpace && (nextNonSpace[1] === '(' || nextNonSpace[1] === '!')) {
                    type = 'function';
                }
            }

            tokens.push({ type, value: word });
            return true;
        }

        tryMatchOperator(code, pos, tokens) {
            const operators = [
                '>>>=', '===', '!==', '>>>', '<<=', '>>=', '**=',
                '&&=', '||=', '??=', '...', '::',
                '==', '!=', '<=', '>=', '&&', '||', '??', '++', '--',
                '+=', '-=', '*=', '/=', '%=', '&=', '|=', '^=',
                '<<', '>>', '**', '=>', '->',
                '+', '-', '*', '/', '%', '&', '|', '^', '~', '!',
                '<', '>', '=', '?', ':', ';', ',', '.',
                '(', ')', '[', ']', '{', '}', '@', '#', '$'
            ];

            for (const op of operators) {
                if (code.startsWith(op, pos)) {
                    const type = /^[()[\]{}]$/.test(op) ? 'punctuation' : 'operator';
                    tokens.push({ type, value: op });
                    return true;
                }
            }

            return false;
        }

        renderToken(token) {
            const escaped = escapeHtml(token.value);

            switch (token.type) {
                case 'keyword': return `<span class="tok-keyword">${escaped}</span>`;
                case 'string': return `<span class="tok-string">${escaped}</span>`;
                case 'number': return `<span class="tok-number">${escaped}</span>`;
                case 'comment': return `<span class="tok-comment">${escaped}</span>`;
                case 'function': return `<span class="tok-function">${escaped}</span>`;
                case 'type': return `<span class="tok-type">${escaped}</span>`;
                case 'constant': return `<span class="tok-constant">${escaped}</span>`;
                case 'operator': return `<span class="tok-operator">${escaped}</span>`;
                case 'punctuation': return `<span class="tok-punctuation">${escaped}</span>`;
                case 'ds4viz': return `<span class="tok-ds4viz">${escaped}</span>`;
                case 'decorator': return `<span class="tok-decorator">${escaped}</span>`;
                case 'lifetime': return `<span class="tok-lifetime">${escaped}</span>`;
                default: return escaped;
            }
        }
    }

    // ============================================
    // 编辑器功能
    // ============================================

    function updateHighlight() {
        const code = elements.codeEditor.value;
        const highlighter = new SyntaxHighlighter(state.language);
        elements.codeHighlight.innerHTML = highlighter.highlight(code) + '\n';
        updateLineNumbers();
    }

    function updateLineNumbers() {
        const code = elements.codeEditor.value;
        const lines = code.split('\n');
        const cursorPos = elements.codeEditor.selectionStart;

        // 计算当前行
        let currentLine = 0;
        let charCount = 0;
        for (let i = 0; i < lines.length; i++) {
            charCount += lines[i].length + 1;
            if (charCount > cursorPos) {
                currentLine = i;
                break;
            }
        }

        state.editor.cursorLine = currentLine;

        let html = '';
        for (let i = 0; i < lines.length; i++) {
            const isActive = i === currentLine;
            html += `<div class="line-number${isActive ? ' active' : ''}">${i + 1}</div>`;
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

    function getCompletionItems(prefix, language) {
        const rules = SYNTAX_RULES[language] || SYNTAX_RULES.javascript;
        const items = [];
        const lowerPrefix = prefix.toLowerCase();

        // ds4viz 函数
        if (rules.ds4vizFunctions) {
            for (const fn of rules.ds4vizFunctions) {
                if (fn.toLowerCase().startsWith(lowerPrefix)) {
                    items.push({ text: fn, type: 'ds4viz', hint: 'ds4viz' });
                }
            }
        }

        // ds4viz 方法
        if (rules.ds4vizMethods) {
            for (const method of rules.ds4vizMethods) {
                if (method.toLowerCase().startsWith(lowerPrefix)) {
                    items.push({ text: method, type: 'method', hint: 'ds4viz' });
                }
            }
        }

        // 关键字
        if (rules.keywords) {
            for (const kw of rules.keywords) {
                if (kw.toLowerCase().startsWith(lowerPrefix)) {
                    items.push({ text: kw, type: 'keyword', hint: '' });
                }
            }
        }

        // 内置函数
        if (rules.builtins) {
            for (const fn of rules.builtins) {
                if (fn.toLowerCase().startsWith(lowerPrefix)) {
                    items.push({ text: fn, type: 'function', hint: 'builtin' });
                }
            }
        }

        // 常量
        if (rules.constants) {
            for (const c of rules.constants) {
                if (c.toLowerCase().startsWith(lowerPrefix)) {
                    items.push({ text: c, type: 'type', hint: 'constant' });
                }
            }
        }

        // 去重并限制数量
        const seen = new Set();
        return items.filter(item => {
            if (seen.has(item.text)) return false;
            seen.add(item.text);
            return true;
        }).slice(0, 12);
    }

    function showAutocomplete(items) {
        if (items.length === 0) {
            hideAutocomplete();
            return;
        }

        state.autocomplete.items = items;
        state.autocomplete.selectedIndex = 0;
        state.autocomplete.visible = true;

        renderAutocompleteList();
        positionAutocomplete();
        elements.autocompletePopup.classList.add('visible');
    }

    function renderAutocompleteList() {
        const { items, selectedIndex } = state.autocomplete;

        let html = '';
        items.forEach((item, index) => {
            const iconLetter = item.type === 'ds4viz' ? 'D' :
                item.type === 'method' ? 'M' :
                    item.type === 'function' ? 'F' :
                        item.type === 'keyword' ? 'K' : 'T';

            html += `
                <div class="autocomplete-item${index === selectedIndex ? ' selected' : ''}" data-index="${index}">
                    <span class="autocomplete-icon ${item.type}">${iconLetter}</span>
                    <span class="autocomplete-text">${escapeHtml(item.text)}</span>
                    ${item.hint ? `<span class="autocomplete-hint">${escapeHtml(item.hint)}</span>` : ''}
                </div>
            `;
        });

        elements.autocompleteList.innerHTML = html;

        // 绑定点击事件
        elements.autocompleteList.querySelectorAll('.autocomplete-item').forEach(el => {
            el.addEventListener('mousedown', (e) => {
                e.preventDefault();
                applyCompletion(parseInt(el.dataset.index));
            });
        });
    }

    function positionAutocomplete() {
        const editor = elements.codeEditor;
        const container = elements.editorContainer;

        // 获取光标位置
        const text = editor.value.substring(0, editor.selectionStart);
        const lines = text.split('\n');
        const lineIndex = lines.length - 1;
        const colIndex = lines[lineIndex].length;

        // 计算位置
        const lineHeight = parseFloat(getComputedStyle(editor).lineHeight) || 21;
        const charWidth = 8.4; // 等宽字体近似值
        const gutterWidth = 50;
        const padding = 12;

        const top = (lineIndex + 1) * lineHeight + padding;
        const left = colIndex * charWidth + gutterWidth + padding;

        const popup = elements.autocompletePopup;
        popup.style.top = `${top}px`;
        popup.style.left = `${left}px`;

        // 确保不超出容器
        const containerRect = container.getBoundingClientRect();
        const popupRect = popup.getBoundingClientRect();

        if (popupRect.right > containerRect.right - 10) {
            popup.style.left = `${containerRect.width - popupRect.width - 10}px`;
        }
        if (popupRect.bottom > containerRect.bottom - 10) {
            popup.style.top = `${top - popupRect.height - lineHeight}px`;
        }
    }

    function hideAutocomplete() {
        state.autocomplete.visible = false;
        state.autocomplete.items = [];
        elements.autocompletePopup.classList.remove('visible');
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
        const before = text.substring(0, wordStart);
        const after = text.substring(cursorPos);
        editor.value = before + item.text + after;

        // 设置光标位置
        const newPos = wordStart + item.text.length;
        editor.selectionStart = editor.selectionEnd = newPos;

        hideAutocomplete();
        updateHighlight();
        editor.focus();
    }

    function updateAutocompleteSelection(delta) {
        const { items } = state.autocomplete;
        if (items.length === 0) return;

        state.autocomplete.selectedIndex =
            (state.autocomplete.selectedIndex + delta + items.length) % items.length;

        renderAutocompleteList();

        // 滚动到可见区域
        const selected = elements.autocompleteList.querySelector('.selected');
        if (selected) {
            selected.scrollIntoView({ block: 'nearest' });
        }
    }

    function triggerAutocomplete() {
        const editor = elements.codeEditor;
        const cursorPos = editor.selectionStart;
        const text = editor.value;

        // 获取当前单词
        let wordStart = cursorPos;
        while (wordStart > 0 && /[a-zA-Z0-9_]/.test(text[wordStart - 1])) {
            wordStart--;
        }

        const currentWord = text.substring(wordStart, cursorPos);
        state.autocomplete.prefix = currentWord;
        state.autocomplete.startPos = wordStart;

        if (currentWord.length >= CONFIG.autocompleteMinChars) {
            const items = getCompletionItems(currentWord, state.language);
            showAutocomplete(items);
        } else {
            hideAutocomplete();
        }
    }

    const debouncedTriggerAutocomplete = debounce(triggerAutocomplete, CONFIG.autocompleteDebounce);

    // ============================================
    // 编辑器事件处理
    // ============================================

    function handleEditorInput() {
        updateHighlight();
        debouncedTriggerAutocomplete();
    }

    function handleEditorKeydown(e) {
        const editor = elements.codeEditor;

        // 自动补全导航
        if (state.autocomplete.visible) {
            switch (e.key) {
                case 'ArrowDown':
                    e.preventDefault();
                    updateAutocompleteSelection(1);
                    return;
                case 'ArrowUp':
                    e.preventDefault();
                    updateAutocompleteSelection(-1);
                    return;
                case 'Tab':
                case 'Enter':
                    if (state.autocomplete.items.length > 0) {
                        e.preventDefault();
                        applyCompletion(state.autocomplete.selectedIndex);
                        return;
                    }
                    break;
                case 'Escape':
                    e.preventDefault();
                    hideAutocomplete();
                    return;
            }
        }

        // Tab 缩进
        if (e.key === 'Tab') {
            e.preventDefault();
            const start = editor.selectionStart;
            const end = editor.selectionEnd;
            const value = editor.value;

            if (e.shiftKey) {
                // 减少缩进
                const lineStart = value.lastIndexOf('\n', start - 1) + 1;
                const lineText = value.substring(lineStart, start);
                if (lineText.startsWith('    ')) {
                    editor.value = value.substring(0, lineStart) + value.substring(lineStart + 4);
                    editor.selectionStart = editor.selectionEnd = Math.max(start - 4, lineStart);
                } else if (lineText.startsWith('\t')) {
                    editor.value = value.substring(0, lineStart) + value.substring(lineStart + 1);
                    editor.selectionStart = editor.selectionEnd = Math.max(start - 1, lineStart);
                }
            } else {
                // 增加缩进
                editor.value = value.substring(0, start) + '    ' + value.substring(end);
                editor.selectionStart = editor.selectionEnd = start + 4;
            }
            updateHighlight();
            return;
        }

        // Enter 自动缩进
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
            const needsIndent = [
                ':', '{', '(', '[', 'then', 'do', '->', '=>', 'function'
            ].some(s => trimmed.endsWith(s));

            if (needsIndent) {
                extraIndent = '    ';
            }

            editor.value = value.substring(0, start) + '\n' + indent + extraIndent + value.substring(start);
            editor.selectionStart = editor.selectionEnd = start + 1 + indent.length + extraIndent.length;
            updateHighlight();
            return;
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

        // 跳转到行首 (Home)
        if (e.key === 'Home' && !e.ctrlKey && !e.metaKey) {
            e.preventDefault();
            const value = editor.value;
            const start = editor.selectionStart;
            const lineStart = value.lastIndexOf('\n', start - 1) + 1;
            const lineText = value.substring(lineStart, start);
            const indentMatch = lineText.match(/^(\s*)/);
            const indentEnd = lineStart + (indentMatch ? indentMatch[1].length : 0);

            // 如果光标在缩进后面，跳到缩进后；否则跳到行首
            const newPos = (start === indentEnd) ? lineStart : indentEnd;
            if (e.shiftKey) {
                editor.selectionEnd = start;
                editor.selectionStart = newPos;
            } else {
                editor.selectionStart = editor.selectionEnd = newPos;
            }
            updateLineNumbers();
        }

        // 跳转到行尾 (End)
        if (e.key === 'End' && !e.ctrlKey && !e.metaKey) {
            e.preventDefault();
            const value = editor.value;
            const start = editor.selectionStart;
            let lineEnd = value.indexOf('\n', start);
            if (lineEnd === -1) lineEnd = value.length;

            if (e.shiftKey) {
                editor.selectionEnd = lineEnd;
            } else {
                editor.selectionStart = editor.selectionEnd = lineEnd;
            }
            updateLineNumbers();
        }

        // Ctrl+Home 跳转到文件开头
        if (e.key === 'Home' && (e.ctrlKey || e.metaKey)) {
            e.preventDefault();
            if (e.shiftKey) {
                editor.selectionEnd = editor.selectionStart;
                editor.selectionStart = 0;
            } else {
                editor.selectionStart = editor.selectionEnd = 0;
            }
            updateLineNumbers();
        }

        // Ctrl+End 跳转到文件末尾
        if (e.key === 'End' && (e.ctrlKey || e.metaKey)) {
            e.preventDefault();
            const len = editor.value.length;
            if (e.shiftKey) {
                editor.selectionEnd = len;
            } else {
                editor.selectionStart = editor.selectionEnd = len;
            }
            updateLineNumbers();
        }
    }

    function handleEditorClick() {
        hideAutocomplete();
        updateLineNumbers();
    }

    function handleEditorBlur() {
        setTimeout(hideAutocomplete, 200);
    }

    // ============================================
    // TOML 解析器 (完整重写)
    // ============================================

    class TomlParser {
        constructor(input) {
            this.input = input;
            this.pos = 0;
            this.line = 1;
            this.col = 1;
        }

        parse() {
            const result = {
                meta: {},
                package: {},
                remarks: null,
                object: {},
                states: [],
                steps: [],
                result: null,
                error: null
            };

            let currentSection = null;
            let currentArraySection = null;
            let currentArrayItem = null;
            let currentSubSection = null;

            while (this.pos < this.input.length) {
                this.skipWhitespaceAndComments();
                if (this.pos >= this.input.length) break;

                const char = this.input[this.pos];

                // 数组表头 [[xxx]]
                if (char === '[' && this.input[this.pos + 1] === '[') {
                    // 保存之前的数组项
                    if (currentArraySection && currentArrayItem) {
                        result[currentArraySection].push(currentArrayItem);
                    }

                    const name = this.parseArrayTableHeader();
                    currentArraySection = name;
                    currentArrayItem = {};
                    currentSection = null;
                    currentSubSection = null;
                    continue;
                }

                // 表头 [xxx] 或 [xxx.yyy]
                if (char === '[') {
                    // 保存之前的数组项
                    if (currentArraySection && currentArrayItem && currentSubSection === null) {
                        // 不保存，可能是子节
                    }

                    const name = this.parseTableHeader();

                    if (name.includes('.')) {
                        // 子表头
                        const parts = name.split('.');
                        if (parts[0] === currentArraySection && currentArrayItem) {
                            currentSubSection = parts.slice(1).join('.');
                            if (!currentArrayItem[currentSubSection]) {
                                currentArrayItem[currentSubSection] = {};
                            }
                        } else if (parts[0] === 'result') {
                            if (!result.result) result.result = {};
                            currentSection = 'result';
                            currentSubSection = parts[1];
                            if (!result.result[currentSubSection]) {
                                result.result[currentSubSection] = {};
                            }
                        } else {
                            currentSection = name;
                            currentSubSection = null;
                        }
                    } else {
                        // 普通表头
                        if (currentArraySection && currentArrayItem && Object.keys(currentArrayItem).length > 0 && name !== currentArraySection) {
                            result[currentArraySection].push(currentArrayItem);
                            currentArrayItem = null;
                            currentArraySection = null;
                        }
                        currentSection = name;
                        currentSubSection = null;
                    }
                    continue;
                }

                // 键值对
                if (/[a-zA-Z_]/.test(char)) {
                    const { key, value } = this.parseKeyValue();

                    if (currentArraySection && currentArrayItem) {
                        if (currentSubSection) {
                            currentArrayItem[currentSubSection][key] = value;
                        } else {
                            currentArrayItem[key] = value;
                        }
                    } else if (currentSection) {
                        if (currentSection === 'result' && currentSubSection) {
                            result.result[currentSubSection][key] = value;
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
                    continue;
                }

                this.pos++;
            }

            // 保存最后一个数组项
            if (currentArraySection && currentArrayItem && Object.keys(currentArrayItem).length > 0) {
                result[currentArraySection].push(currentArrayItem);
            }

            return result;
        }

        skipWhitespaceAndComments() {
            while (this.pos < this.input.length) {
                const char = this.input[this.pos];
                if (char === ' ' || char === '\t' || char === '\r') {
                    this.pos++;
                    this.col++;
                } else if (char === '\n') {
                    this.pos++;
                    this.line++;
                    this.col = 1;
                } else if (char === '#') {
                    // 跳过注释
                    while (this.pos < this.input.length && this.input[this.pos] !== '\n') {
                        this.pos++;
                    }
                } else {
                    break;
                }
            }
        }

        parseArrayTableHeader() {
            this.pos += 2; // 跳过 [[
            this.skipWhitespace();

            let name = '';
            while (this.pos < this.input.length && this.input[this.pos] !== ']') {
                name += this.input[this.pos++];
            }
            name = name.trim();

            this.pos += 2; // 跳过 ]]
            this.skipToEndOfLine();

            return name;
        }

        parseTableHeader() {
            this.pos++; // 跳过 [
            this.skipWhitespace();

            let name = '';
            while (this.pos < this.input.length && this.input[this.pos] !== ']') {
                name += this.input[this.pos++];
            }
            name = name.trim();

            this.pos++; // 跳过 ]
            this.skipToEndOfLine();

            return name;
        }

        parseKeyValue() {
            // 解析键
            let key = '';
            while (this.pos < this.input.length && /[a-zA-Z0-9_\-]/.test(this.input[this.pos])) {
                key += this.input[this.pos++];
            }

            this.skipWhitespace();

            // 跳过 =
            if (this.input[this.pos] === '=') {
                this.pos++;
            }

            this.skipWhitespace();

            // 解析值
            const value = this.parseValue();

            this.skipToEndOfLine();

            return { key, value };
        }

        parseValue() {
            this.skipWhitespace();
            const char = this.input[this.pos];

            if (char === '"') {
                return this.parseString();
            } else if (char === "'") {
                return this.parseLiteralString();
            } else if (char === '[') {
                return this.parseArray();
            } else if (char === '{') {
                return this.parseInlineTable();
            } else if (char === 't' || char === 'f') {
                return this.parseBoolean();
            } else if (char === '-' || /\d/.test(char)) {
                return this.parseNumber();
            }

            return null;
        }

        parseString() {
            // 检查是否是多行字符串
            if (this.input.startsWith('"""', this.pos)) {
                return this.parseMultilineString();
            }

            this.pos++; // 跳过开始的 "
            let value = '';
            let escaped = false;

            while (this.pos < this.input.length) {
                const char = this.input[this.pos];

                if (escaped) {
                    switch (char) {
                        case 'n': value += '\n'; break;
                        case 't': value += '\t'; break;
                        case 'r': value += '\r'; break;
                        case '\\': value += '\\'; break;
                        case '"': value += '"'; break;
                        default: value += char;
                    }
                    escaped = false;
                    this.pos++;
                } else if (char === '\\') {
                    escaped = true;
                    this.pos++;
                } else if (char === '"') {
                    this.pos++;
                    break;
                } else {
                    value += char;
                    this.pos++;
                }
            }

            return value;
        }

        parseMultilineString() {
            this.pos += 3; // 跳过 """
            let value = '';

            // 跳过紧跟的换行符
            if (this.input[this.pos] === '\n') {
                this.pos++;
            }

            while (this.pos < this.input.length) {
                if (this.input.startsWith('"""', this.pos)) {
                    this.pos += 3;
                    break;
                }
                value += this.input[this.pos++];
            }

            return value;
        }

        parseLiteralString() {
            this.pos++; // 跳过开始的 '
            let value = '';

            while (this.pos < this.input.length && this.input[this.pos] !== "'") {
                value += this.input[this.pos++];
            }
            this.pos++; // 跳过结束的 '

            return value;
        }

        parseArray() {
            this.pos++; // 跳过 [
            const items = [];

            while (this.pos < this.input.length) {
                this.skipWhitespaceAndComments();

                if (this.input[this.pos] === ']') {
                    this.pos++;
                    break;
                }

                const value = this.parseValue();
                if (value !== null && value !== undefined) {
                    items.push(value);
                }

                this.skipWhitespaceAndComments();

                if (this.input[this.pos] === ',') {
                    this.pos++;
                }
            }

            return items;
        }

        parseInlineTable() {
            this.pos++; // 跳过 {
            const obj = {};

            while (this.pos < this.input.length) {
                this.skipWhitespace();

                if (this.input[this.pos] === '}') {
                    this.pos++;
                    break;
                }

                // 解析键
                let key = '';
                while (this.pos < this.input.length && /[a-zA-Z0-9_\-]/.test(this.input[this.pos])) {
                    key += this.input[this.pos++];
                }

                this.skipWhitespace();

                // 跳过 =
                if (this.input[this.pos] === '=') {
                    this.pos++;
                }

                this.skipWhitespace();

                // 解析值
                const value = this.parseValue();
                obj[key] = value;

                this.skipWhitespace();

                if (this.input[this.pos] === ',') {
                    this.pos++;
                }
            }

            return obj;
        }

        parseBoolean() {
            if (this.input.startsWith('true', this.pos)) {
                this.pos += 4;
                return true;
            } else if (this.input.startsWith('false', this.pos)) {
                this.pos += 5;
                return false;
            }
            return null;
        }

        parseNumber() {
            let numStr = '';

            // 负号
            if (this.input[this.pos] === '-') {
                numStr += this.input[this.pos++];
            }

            // 整数部分
            while (this.pos < this.input.length && /[\d_]/.test(this.input[this.pos])) {
                if (this.input[this.pos] !== '_') {
                    numStr += this.input[this.pos];
                }
                this.pos++;
            }

            // 小数部分
            if (this.input[this.pos] === '.') {
                numStr += this.input[this.pos++];
                while (this.pos < this.input.length && /[\d_]/.test(this.input[this.pos])) {
                    if (this.input[this.pos] !== '_') {
                        numStr += this.input[this.pos];
                    }
                    this.pos++;
                }
            }

            // 指数部分
            if (this.input[this.pos] === 'e' || this.input[this.pos] === 'E') {
                numStr += this.input[this.pos++];
                if (this.input[this.pos] === '+' || this.input[this.pos] === '-') {
                    numStr += this.input[this.pos++];
                }
                while (this.pos < this.input.length && /\d/.test(this.input[this.pos])) {
                    numStr += this.input[this.pos++];
                }
            }

            return numStr.includes('.') || numStr.includes('e') || numStr.includes('E')
                ? parseFloat(numStr)
                : parseInt(numStr, 10);
        }

        skipWhitespace() {
            while (this.pos < this.input.length && /[ \t]/.test(this.input[this.pos])) {
                this.pos++;
            }
        }

        skipToEndOfLine() {
            while (this.pos < this.input.length && this.input[this.pos] !== '\n') {
                this.pos++;
            }
            if (this.input[this.pos] === '\n') {
                this.pos++;
                this.line++;
                this.col = 1;
            }
        }
    }

    function parseToml(tomlString) {
        const parser = new TomlParser(tomlString);
        return parser.parse();
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

        updateStepInfo();
    }

    function updateStepInfo() {
        const steps = state.parsedToml?.steps || [];

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
                <div class="viz-empty-text">不支持的数据结构类型: ${escapeHtml(kind)}</div>
            </div>
        `;
    }

    function renderEmpty(label, structureType) {
        elements.vizContainer.innerHTML = `
            <div class="viz-wrapper">
                <div class="viz-label">${escapeHtml(label)} (${escapeHtml(structureType)})</div>
                <div class="viz-empty">
                    <div class="viz-empty-text">空${escapeHtml(structureType)}</div>
                </div>
            </div>
        `;
    }

    function formatValue(value) {
        if (typeof value === 'string') {
            return `"${value}"`;
        }
        return String(value);
    }

    function renderStack(data, label) {
        const items = data.items || [];
        const top = data.top ?? (items.length > 0 ? items.length - 1 : -1);

        if (items.length === 0) {
            renderEmpty(label, 'Stack');
            return;
        }

        let html = `<div class="viz-wrapper">
            <div class="viz-label">${escapeHtml(label)} (Stack)</div>
            <div class="viz-stack">`;

        for (let i = 0; i < items.length; i++) {
            const isTop = i === top;
            html += `<div class="viz-stack-item${isTop ? ' top' : ''}">${escapeHtml(formatValue(items[i]))}</div>`;
        }

        html += `</div>
            <div class="viz-stack-pointer">top = ${top}</div>
        </div>`;

        elements.vizContainer.innerHTML = html;
    }

    function renderQueue(data, label) {
        const items = data.items || [];
        const front = data.front ?? 0;
        const rear = data.rear ?? (items.length > 0 ? items.length - 1 : -1);

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

            html += `<div class="${classes}">${escapeHtml(formatValue(items[i]))}</div>`;
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
                        <div class="viz-list-node-value">${escapeHtml(formatValue(node.value))}</div>
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
                <span class="viz-list-arrow">↔</span>`;

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
                        <div class="viz-list-node-value">${escapeHtml(formatValue(node.value))}</div>
                        <div class="viz-list-node-pointer">${node.next === -1 ? 'null' : node.next}</div>
                    </div>
                </div>`;

            if (node.next !== -1 && nodeMap[node.next]) {
                html += '<span class="viz-list-arrow">↔</span>';
            }

            currentId = node.next;
            count++;
        }

        html += `<span class="viz-list-arrow">↔</span>
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
            <svg class="viz-tree-svg" viewBox="0 0 ${svgWidth} ${svgHeight}">`;

        // 绘制边
        for (const [id, pos] of Object.entries(positions)) {
            const node = nodeMap[parseInt(id)];
            if (!node) continue;

            if (node.left !== -1 && positions[node.left]) {
                const childPos = positions[node.left];
                svg += `<line x1="${pos.x}" y1="${pos.y}" x2="${childPos.x}" y2="${childPos.y}"/>`;
            }
            if (node.right !== -1 && positions[node.right]) {
                const childPos = positions[node.right];
                svg += `<line x1="${pos.x}" y1="${pos.y}" x2="${childPos.x}" y2="${childPos.y}"/>`;
            }
        }

        // 绘制节点
        for (const [id, pos] of Object.entries(positions)) {
            const node = nodeMap[parseInt(id)];
            if (!node) continue;

            const isRoot = parseInt(id) === root;
            svg += `
                <circle cx="${pos.x}" cy="${pos.y}" r="${nodeRadius}" class="${isRoot ? 'root' : ''}"/>
                <text x="${pos.x}" y="${pos.y}" class="${isRoot ? 'root' : ''}">${escapeHtml(formatValue(node.value))}</text>`;
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
                kind === 'graph_weighted' ? 'Weighted Graph' : 'Undirected Graph';
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
    // 本地执行 (JavaScript only)
    // ============================================

    async function loadDs4vizModule() {
        if (state.ds4vizModule) {
            return state.ds4vizModule;
        }

        try {
            showLoading(true, '加载 ds4viz 库...');
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
            const ds4viz = await loadDs4vizModule();

            // 移除 import 语句
            const cleanedCode = code
                .replace(/import\s+\{[^}]*\}\s+from\s+['"][^'"]*['"]\s*;?/g, '')
                .replace(/import\s+\*\s+as\s+\w+\s+from\s+['"][^'"]*['"]\s*;?/g, '')
                .replace(/const\s+\{[^}]*\}\s*=\s*require\s*\(['"][^'"]*['"]\)\s*;?/g, '')
                .trim();

            // 创建上下文
            const contextVars = { ...ds4viz };

            let generatedToml = null;

            // 包装 withContext
            const originalWithContext = ds4viz.withContext;
            contextVars.withContext = async (structure, callback) => {
                await originalWithContext(structure, callback);
                if (ds4viz.getTomlString) {
                    generatedToml = ds4viz.getTomlString(structure);
                } else if (structure._getToml) {
                    generatedToml = structure._getToml();
                } else if (structure.toml) {
                    generatedToml = structure.toml;
                }
            };

            const AsyncFunction = Object.getPrototypeOf(async function () { }).constructor;
            const argNames = Object.keys(contextVars);
            const argValues = Object.values(contextVars);

            const wrappedCode = `return (async () => { ${cleanedCode} })();`;
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
            state.ds4vizModule = null;
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
            state.ds4vizModule = null;
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
        elements.codeEditor.addEventListener('input', handleEditorInput);
        elements.codeEditor.addEventListener('scroll', syncScroll);
        elements.codeEditor.addEventListener('keydown', handleEditorKeydown);
        elements.codeEditor.addEventListener('click', handleEditorClick);
        elements.codeEditor.addEventListener('blur', handleEditorBlur);
        elements.codeEditor.addEventListener('keyup', (e) => {
            // 更新行号（光标移动时）
            if (['ArrowUp', 'ArrowDown', 'ArrowLeft', 'ArrowRight', 'Home', 'End', 'PageUp', 'PageDown'].includes(e.key)) {
                updateLineNumbers();
            }
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

        // 默认使用 JavaScript
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

        console.log('ds4viz Demo v2.0 initialized');
    }

    if (document.readyState === 'loading') {
        document.addEventListener('DOMContentLoaded', init);
    } else {
        init();
    }
})();
