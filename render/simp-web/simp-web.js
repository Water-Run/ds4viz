/**
 * ds4viz Demo - Main JavaScript
 *
 * 数据结构可视化教学平台 Demo 前端
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
        defaultLanguage: 'python',
        defaultTimeout: 5000,
        autoImport: true,
        pingInterval: 30000,
        autoPlayDelay: 1000
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
        autoImport: CONFIG.autoImport,
        connected: false,
        tomlData: null,
        parsedToml: null,
        currentStep: 0,
        totalSteps: 0,
        isPlaying: false,
        playInterval: null
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
            autoImportSwitch: document.getElementById('autoImportSwitch'),
            modalAutoImportSwitch: document.getElementById('modalAutoImportSwitch'),
            runBtn: document.getElementById('runBtn'),
            codeEditor: document.getElementById('codeEditor'),
            lineNumbers: document.getElementById('lineNumbers'),
            editorWrapper: document.getElementById('editorWrapper'),
            executionStatus: document.getElementById('executionStatus'),
            vizContainer: document.getElementById('vizContainer'),
            stepBackBtn: document.getElementById('stepBackBtn'),
            stepForwardBtn: document.getElementById('stepForwardBtn'),
            stepIndicator: document.getElementById('stepIndicator'),
            resetBtn: document.getElementById('resetBtn'),
            autoPlayBtn: document.getElementById('autoPlayBtn'),
            stepInfo: document.getElementById('stepInfo'),
            stepOp: document.getElementById('stepOp'),
            stepLine: document.getElementById('stepLine'),
            stepArgs: document.getElementById('stepArgs'),
            loadingOverlay: document.getElementById('loadingOverlay')
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

    // ============================================
    // 编辑器功能
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

        // 同步滚动
        syncLineNumbersScroll();
    }

    function syncLineNumbersScroll() {
        elements.lineNumbers.scrollTop = elements.codeEditor.scrollTop;
    }

    function handleEditorKeydown(e) {
        // Tab 键处理
        if (e.key === 'Tab') {
            e.preventDefault();
            const start = elements.codeEditor.selectionStart;
            const end = elements.codeEditor.selectionEnd;
            const value = elements.codeEditor.value;

            if (e.shiftKey) {
                // Shift+Tab: 减少缩进
                const lineStart = value.lastIndexOf('\n', start - 1) + 1;
                const lineText = value.substring(lineStart, start);
                if (lineText.startsWith('    ')) {
                    elements.codeEditor.value = value.substring(0, lineStart) + value.substring(lineStart + 4);
                    elements.codeEditor.selectionStart = elements.codeEditor.selectionEnd = start - 4;
                }
            } else {
                // Tab: 增加缩进
                elements.codeEditor.value = value.substring(0, start) + '    ' + value.substring(end);
                elements.codeEditor.selectionStart = elements.codeEditor.selectionEnd = start + 4;
            }
            updateLineNumbers();
        }

        // Enter 键自动缩进
        if (e.key === 'Enter') {
            e.preventDefault();
            const start = elements.codeEditor.selectionStart;
            const value = elements.codeEditor.value;
            const lineStart = value.lastIndexOf('\n', start - 1) + 1;
            const lineText = value.substring(lineStart, start);
            const indent = lineText.match(/^(\s*)/)[1];

            // 检查是否需要额外缩进（如 Python 的冒号结尾）
            let extraIndent = '';
            if (lineText.trimEnd().endsWith(':') || 
                lineText.trimEnd().endsWith('{') || 
                lineText.trimEnd().endsWith('(') ||
                lineText.trimEnd().endsWith('then') ||
                lineText.trimEnd().endsWith('do')) {
                extraIndent = '    ';
            }

            elements.codeEditor.value = value.substring(0, start) + '\n' + indent + extraIndent + value.substring(start);
            elements.codeEditor.selectionStart = elements.codeEditor.selectionEnd = start + 1 + indent.length + extraIndent.length;
            updateLineNumbers();
        }

        // Ctrl+/ 注释切换
        if ((e.ctrlKey || e.metaKey) && e.key === '/') {
            e.preventDefault();
            toggleComment();
        }
    }

    function toggleComment() {
        const start = elements.codeEditor.selectionStart;
        const end = elements.codeEditor.selectionEnd;
        const value = elements.codeEditor.value;

        const commentPrefixes = {
            python: '#',
            javascript: '//',
            typescript: '//',
            lua: '--',
            c: '//',
            rust: '//',
            php: '//'
        };

        const prefix = commentPrefixes[state.language] || '//';
        const lineStart = value.lastIndexOf('\n', start - 1) + 1;
        const lineEnd = value.indexOf('\n', end);
        const lineContent = value.substring(lineStart, lineEnd === -1 ? value.length : lineEnd);

        let newContent;
        let cursorOffset = 0;

        if (lineContent.trimStart().startsWith(prefix)) {
            // 取消注释
            const prefixIndex = lineContent.indexOf(prefix);
            newContent = lineContent.substring(0, prefixIndex) + lineContent.substring(prefixIndex + prefix.length + (lineContent[prefixIndex + prefix.length] === ' ' ? 1 : 0));
            cursorOffset = -(prefix.length + (lineContent[prefixIndex + prefix.length] === ' ' ? 1 : 0));
        } else {
            // 添加注释
            const indent = lineContent.match(/^(\s*)/)[1];
            newContent = indent + prefix + ' ' + lineContent.trimStart();
            cursorOffset = prefix.length + 1;
        }

        elements.codeEditor.value = value.substring(0, lineStart) + newContent + value.substring(lineEnd === -1 ? value.length : lineEnd);
        elements.codeEditor.selectionStart = elements.codeEditor.selectionEnd = start + cursorOffset;
        updateLineNumbers();
    }

    // ============================================
    // 代码处理
    // ============================================

    function processCode(code) {
        if (!state.autoImport) return code;

        let processedCode = code;

        // JavaScript/TypeScript 路径矫正
        if (state.language === 'javascript' || state.language === 'typescript') {
            processedCode = processedCode.replace(
                /(import\s+.*from\s+['"])([^'"]*ds4viz[^'"]*)(['"])/g,
                `$1${state.libraryPath}$3`
            );
            processedCode = processedCode.replace(
                /(require\s*\(\s*['"])([^'"]*ds4viz[^'"]*)(['"])/g,
                `$1${state.libraryPath}$3`
            );
        }

        // C 头文件路径矫正
        if (state.language === 'c') {
            const headerPath = state.libraryPath.replace(/javascript\/ds4viz\.js$/, 'c/ds4viz.h');
            processedCode = processedCode.replace(
                /#include\s+["']([^"']*ds4viz\.[hc])["']/g,
                `#include "${headerPath}"`
            );
        }

        return processedCode;
    }

    function getTemplate() {
        const template = TEMPLATES[state.language];
        if (typeof template === 'function') {
            return template(state.libraryPath);
        }
        return template || '';
    }

    // ============================================
    // 服务器通信
    // ============================================

    async function pingServer() {
        elements.connectionStatus.className = 'connection-status checking';

        try {
            const response = await fetch(`${state.serverUrl}/ping`, {
                method: 'GET',
                headers: { 'Content-Type': 'application/json' }
            });

            if (response.ok) {
                state.connected = true;
                elements.connectionStatus.className = 'connection-status connected';
            } else {
                throw new Error('Server returned error');
            }
        } catch (error) {
            state.connected = false;
            elements.connectionStatus.className = 'connection-status disconnected';
        }
    }

    async function executeCode() {
        if (state.mode === 'local') {
            executeLocalCode();
            return;
        }

        if (!state.connected) {
            await pingServer();
            if (!state.connected) {
                showStatus('error', '无法连接到服务器');
                return;
            }
        }

        const code = processCode(elements.codeEditor.value);

        showLoading(true);
        elements.runBtn.disabled = true;

        try {
            const response = await fetch(`${state.serverUrl}/execute`, {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
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
            } else {
                showStatus('error', data.message || '执行失败');
            }
        } catch (error) {
            showStatus('error', '执行请求失败: ' + error.message);
        } finally {
            showLoading(false);
            elements.runBtn.disabled = false;
        }
    }

    async function executeLocalCode() {
        showStatus('error', '本地模式：请直接上传 TOML 文件，或使用远程模式执行代码');
    }

    // ============================================
    // TOML 解析
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
        let currentArrayName = null;
        let currentObject = {};
        let inDataSection = false;

        for (let i = 0; i < lines.length; i++) {
            const line = lines[i];
            const trimmed = line.trim();

            if (!trimmed || trimmed.startsWith('#')) continue;

            // 数组表头 [[xxx]]
            const arrayMatch = trimmed.match(/^\[\[(\w+)\]\]$/);
            if (arrayMatch) {
                if (currentArrayName && Object.keys(currentObject).length > 0) {
                    result[currentArrayName].push({ ...currentObject });
                }
                currentArrayName = arrayMatch[1];
                currentSection = null;
                currentObject = {};
                inDataSection = false;
                continue;
            }

            // 表头 [xxx] 或 [xxx.data]
            const sectionMatch = trimmed.match(/^\[([^\]]+)\]$/);
            if (sectionMatch) {
                if (currentArrayName && Object.keys(currentObject).length > 0 && !sectionMatch[1].includes('.')) {
                    result[currentArrayName].push({ ...currentObject });
                    currentObject = {};
                }

                const sectionPath = sectionMatch[1];
                if (sectionPath.endsWith('.data')) {
                    inDataSection = true;
                    if (!currentObject.data) currentObject.data = {};
                } else if (sectionPath.endsWith('.args')) {
                    if (!currentObject.args) currentObject.args = {};
                } else if (sectionPath.endsWith('.code')) {
                    if (!currentObject.code) currentObject.code = {};
                } else if (sectionPath.endsWith('.commit')) {
                    if (!result.result) result.result = {};
                    if (!result.result.commit) result.result.commit = {};
                } else {
                    currentSection = sectionPath;
                    currentArrayName = null;
                    inDataSection = false;
                }
                continue;
            }

            // 键值对
            const kvMatch = trimmed.match(/^(\w+)\s*=\s*(.+)$/);
            if (kvMatch) {
                const key = kvMatch[1];
                const value = parseTomlValue(kvMatch[2].trim());

                if (currentArrayName) {
                    if (inDataSection) {
                        currentObject.data = currentObject.data || {};
                        currentObject.data[key] = value;
                    } else if (trimmed.includes('.args.') || (currentObject.args !== undefined)) {
                        currentObject.args = currentObject.args || {};
                        currentObject.args[key] = value;
                    } else if (trimmed.includes('.code.') || (currentObject.code !== undefined)) {
                        currentObject.code = currentObject.code || {};
                        currentObject.code[key] = value;
                    } else {
                        currentObject[key] = value;
                    }
                } else if (currentSection) {
                    const sectionParts = currentSection.split('.');
                    if (sectionParts.length === 1) {
                        if (!result[currentSection]) result[currentSection] = {};
                        result[currentSection][key] = value;
                    } else if (sectionParts[1] === 'commit') {
                        result.result = result.result || {};
                        result.result.commit = result.result.commit || {};
                        result.result.commit[key] = value;
                    }
                }
            }
        }

        if (currentArrayName && Object.keys(currentObject).length > 0) {
            result[currentArrayName].push(currentObject);
        }

        return result;
    }

    function parseTomlValue(value) {
        // 字符串
        if ((value.startsWith('"') && value.endsWith('"')) ||
            (value.startsWith("'") && value.endsWith("'"))) {
            return value.slice(1, -1).replace(/\\"/g, '"').replace(/\\'/g, "'");
        }

        // 布尔值
        if (value === 'true') return true;
        if (value === 'false') return false;

        // 数组
        if (value.startsWith('[')) {
            return parseTomlArray(value);
        }

        // 数字
        const num = parseFloat(value);
        if (!isNaN(num)) return num;

        return value;
    }

    function parseTomlArray(value) {
        const content = value.slice(1, -1).trim();
        if (!content) return [];

        const items = [];
        let current = '';
        let depth = 0;
        let inString = false;
        let stringChar = '';

        for (let i = 0; i < content.length; i++) {
            const char = content[i];

            if (!inString && (char === '"' || char === "'")) {
                inString = true;
                stringChar = char;
                current += char;
            } else if (inString && char === stringChar && content[i - 1] !== '\\') {
                inString = false;
                current += char;
            } else if (!inString && (char === '{' || char === '[')) {
                depth++;
                current += char;
            } else if (!inString && (char === '}' || char === ']')) {
                depth--;
                current += char;
            } else if (!inString && char === ',' && depth === 0) {
                if (current.trim()) {
                    items.push(parseTomlValueOrInlineTable(current.trim()));
                }
                current = '';
            } else {
                current += char;
            }
        }

        if (current.trim()) {
            items.push(parseTomlValueOrInlineTable(current.trim()));
        }

        return items;
    }

    function parseTomlValueOrInlineTable(value) {
        if (value.startsWith('{') && value.endsWith('}')) {
            const obj = {};
            const content = value.slice(1, -1);
            let current = '';
            let depth = 0;
            let inString = false;
            let stringChar = '';

            for (let i = 0; i <= content.length; i++) {
                const char = content[i];

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
                    } else if (inString && char === stringChar && content[i - 1] !== '\\') {
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
        return parseTomlValue(value);
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
                showStatus('error', parsed.error.message || '执行出错');
            }

            updateVizControls();
            renderCurrentState();
        } catch (error) {
            console.error('TOML 解析错误:', error);
            showStatus('error', 'TOML 解析失败: ' + error.message);
        }
    }

    function updateVizControls() {
        const hasData = state.totalSteps > 0;

        elements.stepBackBtn.disabled = !hasData || state.currentStep <= 0;
        elements.stepForwardBtn.disabled = !hasData || state.currentStep >= state.totalSteps - 1;
        elements.resetBtn.disabled = !hasData;
        elements.autoPlayBtn.disabled = !hasData;

        elements.stepIndicator.textContent = hasData
            ? `${state.currentStep + 1} / ${state.totalSteps}`
            : '0 / 0';

        // 更新步骤信息
        const steps = state.parsedToml?.steps || [];
        const currentStepData = steps.find(s => s.after === state.currentStep || s.before === state.currentStep);

        if (hasData && currentStepData) {
            elements.stepInfo.style.display = 'block';
            elements.stepOp.textContent = currentStepData.op || '';
            elements.stepLine.textContent = currentStepData.code?.line ? `Line ${currentStepData.code.line}` : '';

            const args = currentStepData.args || {};
            elements.stepArgs.textContent = Object.entries(args)
                .map(([k, v]) => `${k}: ${JSON.stringify(v)}`)
                .join(', ');
        } else {
            elements.stepInfo.style.display = 'none';
        }
    }

    function renderCurrentState() {
        const parsed = state.parsedToml;
        if (!parsed || !parsed.states || parsed.states.length === 0) {
            elements.vizContainer.innerHTML = `
                <div class="viz-placeholder">
                    <img src="images/flowsheet_100dp_1F1F1F_FILL0_wght400_GRAD0_opsz48.png" alt="可视化" class="placeholder-icon">
                    <p class="placeholder-text">运行代码或上传 TOML 查看可视化</p>
                </div>
            `;
            return;
        }

        const currentState = parsed.states[state.currentStep];
        if (!currentState) return;

        const data = currentState.data || currentState;
        const objectKind = parsed.object?.kind || 'stack';

        switch (objectKind) {
            case 'stack':
                renderStack(data);
                break;
            case 'queue':
                renderQueue(data);
                break;
            case 'slist':
                renderSingleLinkedList(data);
                break;
            case 'dlist':
                renderDoubleLinkedList(data);
                break;
            case 'binary_tree':
            case 'bst':
                renderBinaryTree(data);
                break;
            case 'graph_undirected':
            case 'graph_directed':
            case 'graph_weighted':
                renderGraph(data, objectKind);
                break;
            default:
                elements.vizContainer.innerHTML = `
                    <div class="viz-placeholder">
                        <p class="placeholder-text">不支持的数据结构类型: ${objectKind}</p>
                    </div>
                `;
        }
    }

    function renderStack(data) {
        const items = data.items || [];
        const top = data.top ?? items.length - 1;

        if (items.length === 0) {
            elements.vizContainer.innerHTML = `
                <div class="viz-wrapper">
                    <div class="viz-label">Stack</div>
                    <div class="viz-empty">
                        <div class="viz-empty-icon">📦</div>
                        <div class="viz-empty-text">空栈</div>
                    </div>
                </div>
            `;
            return;
        }

        let html = '<div class="viz-wrapper"><div class="viz-label">Stack</div><div class="viz-stack">';

        for (let i = 0; i < items.length; i++) {
            const isTop = i === top;
            html += `<div class="viz-stack-item ${isTop ? 'top' : ''}">${escapeHtml(String(items[i]))}</div>`;
        }

        html += '</div>';
        if (top >= 0) {
            html += `<div class="viz-stack-pointer">top = ${top}</div>`;
        }
        html += '</div>';

        elements.vizContainer.innerHTML = html;
    }

    function renderQueue(data) {
        const items = data.items || [];
        const front = data.front ?? 0;
        const rear = data.rear ?? items.length - 1;

        if (items.length === 0) {
            elements.vizContainer.innerHTML = `
                <div class="viz-wrapper">
                    <div class="viz-label">Queue</div>
                    <div class="viz-empty">
                        <div class="viz-empty-icon">📦</div>
                        <div class="viz-empty-text">空队列</div>
                    </div>
                </div>
            `;
            return;
        }

        let html = '<div class="viz-wrapper"><div class="viz-label">Queue (front → rear)</div><div class="viz-queue">';

        for (let i = 0; i < items.length; i++) {
            const isFront = i === front;
            const isRear = i === rear;
            let classes = 'viz-queue-item';
            if (isFront) classes += ' front';
            if (isRear) classes += ' rear';

            html += `<div class="${classes}">${escapeHtml(String(items[i]))}</div>`;
            if (i < items.length - 1) {
                html += '<span class="viz-queue-arrow">→</span>';
            }
        }

        html += '</div></div>';
        elements.vizContainer.innerHTML = html;
    }

    function renderSingleLinkedList(data) {
        const nodes = data.nodes || [];
        const head = data.head;

        if (nodes.length === 0 || head === -1) {
            elements.vizContainer.innerHTML = `
                <div class="viz-wrapper">
                    <div class="viz-label">Singly Linked List</div>
                    <div class="viz-empty">
                        <div class="viz-empty-icon">📦</div>
                        <div class="viz-empty-text">空链表</div>
                    </div>
                </div>
            `;
            return;
        }

        const nodeMap = {};
        nodes.forEach(n => nodeMap[n.id] = n);

        let html = '<div class="viz-wrapper"><div class="viz-label">Singly Linked List (head → tail)</div><div class="viz-list">';
        let currentId = head;
        let visited = new Set();

        while (currentId !== -1 && !visited.has(currentId)) {
            visited.add(currentId);
            const node = nodeMap[currentId];
            if (!node) break;

            html += `
                <div class="viz-list-node">
                    <div class="viz-list-node-box">
                        <div class="viz-list-node-value">${escapeHtml(String(node.value))}</div>
                        <div class="viz-list-node-pointer">${node.next === -1 ? '∅' : '→'}</div>
                    </div>
                </div>
            `;

            if (node.next !== -1) {
                html += '<span class="viz-list-arrow">→</span>';
            }

            currentId = node.next;
        }

        if (currentId === -1) {
            html += '<div class="viz-list-null">NULL</div>';
        }

        html += '</div></div>';
        elements.vizContainer.innerHTML = html;
    }

    function renderDoubleLinkedList(data) {
        const nodes = data.nodes || [];
        const head = data.head;

        if (nodes.length === 0 || head === -1) {
            elements.vizContainer.innerHTML = `
                <div class="viz-wrapper">
                    <div class="viz-label">Doubly Linked List</div>
                    <div class="viz-empty">
                        <div class="viz-empty-icon">📦</div>
                        <div class="viz-empty-text">空链表</div>
                    </div>
                </div>
            `;
            return;
        }

        const nodeMap = {};
        nodes.forEach(n => nodeMap[n.id] = n);

        let html = '<div class="viz-wrapper"><div class="viz-label">Doubly Linked List</div><div class="viz-list">';
        html += '<div class="viz-list-null">NULL</div><span class="viz-list-arrow">⟷</span>';

        let currentId = head;
        let visited = new Set();

        while (currentId !== -1 && !visited.has(currentId)) {
            visited.add(currentId);
            const node = nodeMap[currentId];
            if (!node) break;

            html += `
                <div class="viz-list-node">
                    <div class="viz-list-node-box">
                        <div class="viz-list-node-pointer">${node.prev === -1 ? '∅' : '←'}</div>
                        <div class="viz-list-node-value">${escapeHtml(String(node.value))}</div>
                        <div class="viz-list-node-pointer">${node.next === -1 ? '∅' : '→'}</div>
                    </div>
                </div>
            `;

            if (node.next !== -1) {
                html += '<span class="viz-list-arrow">⟷</span>';
            }

            currentId = node.next;
        }

        html += '<span class="viz-list-arrow">⟷</span><div class="viz-list-null">NULL</div>';
        html += '</div></div>';
        elements.vizContainer.innerHTML = html;
    }

    function renderBinaryTree(data) {
        const nodes = data.nodes || [];
        const root = data.root;

        if (nodes.length === 0 || root === -1) {
            elements.vizContainer.innerHTML = `
                <div class="viz-wrapper">
                    <div class="viz-label">Binary Tree</div>
                    <div class="viz-empty">
                        <div class="viz-empty-icon">🌳</div>
                        <div class="viz-empty-text">空树</div>
                    </div>
                </div>
            `;
            return;
        }

        const nodeMap = {};
        nodes.forEach(n => nodeMap[n.id] = n);

        // BFS 按层遍历
        const levels = [];
        const queue = [{ id: root, level: 0 }];

        while (queue.length > 0) {
            const { id, level } = queue.shift();
            if (id === -1 || !nodeMap[id]) continue;

            if (!levels[level]) levels[level] = [];
            levels[level].push(nodeMap[id]);

            const node = nodeMap[id];
            if (node.left !== undefined) queue.push({ id: node.left, level: level + 1 });
            if (node.right !== undefined) queue.push({ id: node.right, level: level + 1 });
        }

        let html = '<div class="viz-wrapper"><div class="viz-label">Binary Tree</div><div class="viz-tree">';

        levels.forEach((level, levelIndex) => {
            html += '<div class="viz-tree-level">';
            level.forEach(node => {
                const isRoot = levelIndex === 0;
                html += `
                    <div class="viz-tree-node">
                        <div class="viz-tree-node-value ${isRoot ? 'root' : ''}">${escapeHtml(String(node.value))}</div>
                    </div>
                `;
            });
            html += '</div>';
        });

        html += '</div></div>';
        elements.vizContainer.innerHTML = html;
    }

    function renderGraph(data, kind) {
        const nodes = data.nodes || [];
        const edges = data.edges || [];

        if (nodes.length === 0) {
            elements.vizContainer.innerHTML = `
                <div class="viz-wrapper">
                    <div class="viz-label">Graph</div>
                    <div class="viz-empty">
                        <div class="viz-empty-icon">🔗</div>
                        <div class="viz-empty-text">空图</div>
                    </div>
                </div>
            `;
            return;
        }

        const isDirected = kind === 'graph_directed';
        const isWeighted = kind === 'graph_weighted';

        const width = 500;
        const height = 350;
        const centerX = width / 2;
        const centerY = height / 2;
        const radius = Math.min(120, 25 * nodes.length);
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

        let svg = `
            <div class="viz-wrapper">
                <div class="viz-label">${isDirected ? 'Directed' : 'Undirected'} ${isWeighted ? 'Weighted ' : ''}Graph</div>
                <div class="viz-graph">
                    <svg class="viz-graph-svg" viewBox="0 0 ${width} ${height}">
                        <defs>
                            <marker id="arrowhead" markerWidth="10" markerHeight="7" refX="9" refY="3.5" orient="auto">
                                <polygon points="0 0, 10 3.5, 0 7" fill="var(--border-focus)" />
                            </marker>
                        </defs>
        `;

        // 绘制边
        edges.forEach(edge => {
            const from = nodePositions[edge.from];
            const to = nodePositions[edge.to];
            if (!from || !to) return;

            // 计算边的端点（考虑节点半径）
            const dx = to.x - from.x;
            const dy = to.y - from.y;
            const dist = Math.sqrt(dx * dx + dy * dy);
            const offsetX = (dx / dist) * nodeRadius;
            const offsetY = (dy / dist) * nodeRadius;

            const x1 = from.x + offsetX;
            const y1 = from.y + offsetY;
            const x2 = to.x - offsetX;
            const y2 = to.y - offsetY;

            svg += `<line class="viz-graph-edge ${isDirected ? 'directed' : ''}" 
                          x1="${x1}" y1="${y1}" x2="${x2}" y2="${y2}" />`;

            if (isWeighted && edge.weight !== undefined) {
                const midX = (from.x + to.x) / 2;
                const midY = (from.y + to.y) / 2 - 10;
                svg += `<text class="viz-graph-edge-weight" x="${midX}" y="${midY}">${edge.weight}</text>`;
            }
        });

        // 绘制节点
        nodes.forEach(node => {
            const pos = nodePositions[node.id];
            svg += `
                <g class="viz-graph-node">
                    <circle class="viz-graph-node-circle" cx="${pos.x}" cy="${pos.y}" r="${nodeRadius}" />
                    <text class="viz-graph-node-label" x="${pos.x}" y="${pos.y}">${escapeHtml(node.label || String(node.id))}</text>
                </g>
            `;
        });

        svg += '</svg></div></div>';
        elements.vizContainer.innerHTML = svg;
    }

    // ============================================
    // UI 状态更新
    // ============================================

    function showLoading(show) {
        elements.loadingOverlay.style.display = show ? 'flex' : 'none';
    }

    function showStatus(type, message, duration) {
        elements.executionStatus.style.display = 'flex';
        elements.executionStatus.className = `execution-status ${type}`;
        elements.executionStatus.querySelector('.status-text').textContent = message;
        elements.executionStatus.querySelector('.status-duration').textContent =
            duration ? `${duration}ms` : '';
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
    }

    function loadTemplate() {
        elements.codeEditor.value = getTemplate();
        updateLineNumbers();
    }

    // ============================================
    // 事件处理
    // ============================================

    function setupEventListeners() {
        // 模式切换
        elements.remoteModeBtn.addEventListener('click', () => {
            state.mode = 'remote';
            elements.remoteModeBtn.classList.add('active');
            elements.localModeBtn.classList.remove('active');
            elements.remoteInputWrapper.classList.remove('hidden');
            elements.localInputWrapper.classList.add('hidden');
            elements.languageSelect.disabled = false;
            pingServer();
        });

        elements.localModeBtn.addEventListener('click', () => {
            state.mode = 'local';
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
            state.serverUrl = elements.serverUrl.value;
            pingServer();
        });

        elements.libraryPath.addEventListener('change', () => {
            state.libraryPath = elements.libraryPath.value;
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
                showStatus('success', 'TOML 已加载');
            };
            reader.onerror = () => {
                showStatus('error', '文件读取失败');
            };
            reader.readAsText(file);
            e.target.value = '';
        });

        // TOML 下载
        elements.downloadTomlBtn.addEventListener('click', () => {
            if (!state.tomlData) {
                showStatus('error', '没有可下载的 TOML 数据');
                return;
            }

            const blob = new Blob([state.tomlData], { type: 'text/plain' });
            const url = URL.createObjectURL(blob);
            const a = document.createElement('a');
            a.href = url;
            a.download = 'trace.toml';
            a.click();
            URL.revokeObjectURL(url);
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

        // 自动导入开关
        elements.autoImportSwitch.addEventListener('change', () => {
            state.autoImport = elements.autoImportSwitch.checked;
            elements.modalAutoImportSwitch.checked = state.autoImport;
        });

        elements.modalAutoImportSwitch.addEventListener('change', () => {
            state.autoImport = elements.modalAutoImportSwitch.checked;
            elements.autoImportSwitch.checked = state.autoImport;
        });

        // 运行按钮
        elements.runBtn.addEventListener('click', executeCode);

        // 代码编辑器
        elements.codeEditor.addEventListener('input', updateLineNumbers);
        elements.codeEditor.addEventListener('scroll', syncLineNumbersScroll);
        elements.codeEditor.addEventListener('keydown', handleEditorKeydown);

        // 可视化控制
        elements.stepBackBtn.addEventListener('click', () => {
            if (state.currentStep > 0) {
                state.currentStep--;
                updateVizControls();
                renderCurrentState();
            }
        });

        elements.stepForwardBtn.addEventListener('click', () => {
            if (state.currentStep < state.totalSteps - 1) {
                state.currentStep++;
                updateVizControls();
                renderCurrentState();
            }
        });

        elements.resetBtn.addEventListener('click', () => {
            state.currentStep = 0;
            stopAutoPlay();
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
            }

            // 左右箭头控制步骤（非编辑器焦点时）
            if (document.activeElement !== elements.codeEditor) {
                if (e.key === 'ArrowLeft' && !elements.stepBackBtn.disabled) {
                    elements.stepBackBtn.click();
                } else if (e.key === 'ArrowRight' && !elements.stepForwardBtn.disabled) {
                    elements.stepForwardBtn.click();
                } else if (e.key === ' ' && !elements.autoPlayBtn.disabled) {
                    e.preventDefault();
                    elements.autoPlayBtn.click();
                }
            }

            // Escape 关闭弹窗
            if (e.key === 'Escape') {
                elements.aboutModal.classList.remove('active');
            }
        });
    }

    function startAutoPlay() {
        state.isPlaying = true;
        elements.autoPlayBtn.innerHTML = `
            <svg width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
                <rect x="6" y="4" width="4" height="16"></rect>
                <rect x="14" y="4" width="4" height="16"></rect>
            </svg>
        `;

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
        elements.autoImportSwitch.checked = state.autoImport;
        elements.modalAutoImportSwitch.checked = state.autoImport;

        updateLanguageUI();
        loadTemplate();

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
    }

    if (document.readyState === 'loading') {
        document.addEventListener('DOMContentLoaded', init);
    } else {
        init();
    }
})();
