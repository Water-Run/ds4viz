/**
 * Monaco 编辑器运行时配置与 ds4viz 语法补全
 *
 * @file src/utils/monaco.ts
 * @author WaterRun
 * @date 2026-02-27
 */

/** 补全是否已注册 */
let completionsRegistered = false

/**
 * 初始化 Monaco web worker (Vite)
 *
 * 为 Monaco 编辑器注册 getWorker 工厂函数。
 */
export function setupMonacoWorkers(): void {
  const globalScope = self as unknown as {
    MonacoEnvironment?: {
      getWorker: (_moduleId: string, _label: string) => Worker
    }
  }
  globalScope.MonacoEnvironment = {
    getWorker: (_moduleId: string, _label: string): Worker =>
      new Worker(
        new URL('monaco-editor/esm/vs/editor/editor.worker?worker', import.meta.url),
        { type: 'module' },
      ),
  }
}

/**
 * 补全项简明描述
 *
 * @interface
 */
interface CompletionDef {
  /** 显示标签 */
  label: string
  /** 插入文本（支持 snippet 语法） */
  insert: string
  /** 简要说明 */
  detail: string
  /** 种类：function / method / snippet / keyword */
  kind: 'function' | 'method' | 'snippet' | 'keyword'
}

/**
 * 注册 ds4viz 三语言补全
 *
 * @param monaco - Monaco 编辑器模块
 */
export function registerDs4vizCompletions(monaco: typeof import('monaco-editor')): void {
  if (completionsRegistered) return
  completionsRegistered = true

  registerLanguageCompletions(monaco, 'python', buildPythonCompletions())
  registerLanguageCompletions(monaco, 'lua', buildLuaCompletions())
  registerLanguageCompletions(monaco, 'rust', buildRustCompletions())
}

/**
 * 为指定语言注册补全提供器
 */
function registerLanguageCompletions(
  monaco: typeof import('monaco-editor'),
  languageId: string,
  defs: CompletionDef[],
): void {
  monaco.languages.registerCompletionItemProvider(languageId, {
    provideCompletionItems(model, position) {
      const word = model.getWordUntilPosition(position)
      const range = {
        startLineNumber: position.lineNumber,
        endLineNumber: position.lineNumber,
        startColumn: word.startColumn,
        endColumn: word.endColumn,
      }

      const kindMap: Record<CompletionDef['kind'], number> = {
        function: monaco.languages.CompletionItemKind.Function,
        method: monaco.languages.CompletionItemKind.Method,
        snippet: monaco.languages.CompletionItemKind.Snippet,
        keyword: monaco.languages.CompletionItemKind.Keyword,
      }

      const suggestions = defs.map((def) => ({
        label: def.label,
        kind: kindMap[def.kind],
        insertText: def.insert,
        insertTextRules: monaco.languages.CompletionItemInsertTextRule.InsertAsSnippet,
        detail: def.detail,
        range,
      }))

      return { suggestions }
    },
  })
}

/* ----------------------------------------------------------------
 *  Python 补全
 * ---------------------------------------------------------------- */

function buildPythonCompletions(): CompletionDef[] {
  return [
    { label: 'import ds4viz', insert: 'import ds4viz as dv', detail: '导入 ds4viz', kind: 'snippet' },
    { label: 'dv.config', insert: 'dv.config(\n    output_path="${1:trace.toml}",\n    title="${2}",\n    author="${3}",\n    comment="${4}"\n)', detail: '全局配置', kind: 'function' },
    { label: 'dv.stack', insert: 'with dv.stack(label="${1:stack}") as ${2:s}:\n    $0', detail: '创建栈', kind: 'function' },
    { label: 'dv.queue', insert: 'with dv.queue(label="${1:queue}") as ${2:q}:\n    $0', detail: '创建队列', kind: 'function' },
    { label: 'dv.single_linked_list', insert: 'with dv.single_linked_list(label="${1:slist}") as ${2:slist}:\n    $0', detail: '创建单链表', kind: 'function' },
    { label: 'dv.double_linked_list', insert: 'with dv.double_linked_list(label="${1:dlist}") as ${2:dlist}:\n    $0', detail: '创建双链表', kind: 'function' },
    { label: 'dv.binary_tree', insert: 'with dv.binary_tree(label="${1:tree}") as ${2:tree}:\n    $0', detail: '创建二叉树', kind: 'function' },
    { label: 'dv.binary_search_tree', insert: 'with dv.binary_search_tree(label="${1:bst}") as ${2:bst}:\n    $0', detail: '创建二叉搜索树', kind: 'function' },
    { label: 'dv.heap', insert: 'with dv.heap(label="${1:heap}", heap_type="${2|min,max|}") as ${3:h}:\n    $0', detail: '创建堆', kind: 'function' },
    { label: 'dv.graph_undirected', insert: 'with dv.graph_undirected(label="${1:graph}") as ${2:g}:\n    $0', detail: '创建无向图', kind: 'function' },
    { label: 'dv.graph_directed', insert: 'with dv.graph_directed(label="${1:graph}") as ${2:g}:\n    $0', detail: '创建有向图', kind: 'function' },
    { label: 'dv.graph_weighted', insert: 'with dv.graph_weighted(label="${1:graph}", directed=${2|False,True|}) as ${3:g}:\n    $0', detail: '创建带权图', kind: 'function' },
    ...buildPythonMethods(),
  ]
}

function buildPythonMethods(): CompletionDef[] {
  return [
    { label: 'push', insert: 'push(${1:value})', detail: 'Stack: 压栈', kind: 'method' },
    { label: 'pop', insert: 'pop()', detail: 'Stack: 弹栈', kind: 'method' },
    { label: 'enqueue', insert: 'enqueue(${1:value})', detail: 'Queue: 入队', kind: 'method' },
    { label: 'dequeue', insert: 'dequeue()', detail: 'Queue: 出队', kind: 'method' },
    { label: 'clear', insert: 'clear()', detail: '清空结构', kind: 'method' },
    { label: 'insert_head', insert: 'insert_head(${1:value})', detail: 'List: 头插', kind: 'method' },
    { label: 'insert_tail', insert: 'insert_tail(${1:value})', detail: 'List: 尾插', kind: 'method' },
    { label: 'insert_after', insert: 'insert_after(${1:node_id}, ${2:value})', detail: 'List: 指定节点后插入', kind: 'method' },
    { label: 'insert_before', insert: 'insert_before(${1:node_id}, ${2:value})', detail: 'DList: 指定节点前插入', kind: 'method' },
    { label: 'delete', insert: 'delete(${1:id})', detail: '删除节点', kind: 'method' },
    { label: 'delete_head', insert: 'delete_head()', detail: 'List: 删除头节点', kind: 'method' },
    { label: 'delete_tail', insert: 'delete_tail()', detail: 'DList: 删除尾节点', kind: 'method' },
    { label: 'reverse', insert: 'reverse()', detail: 'List: 反转', kind: 'method' },
    { label: 'insert_root', insert: 'insert_root(${1:value})', detail: 'Tree: 插入根节点', kind: 'method' },
    { label: 'insert_left', insert: 'insert_left(${1:parent_id}, ${2:value})', detail: 'Tree: 插入左子', kind: 'method' },
    { label: 'insert_right', insert: 'insert_right(${1:parent_id}, ${2:value})', detail: 'Tree: 插入右子', kind: 'method' },
    { label: 'update_value', insert: 'update_value(${1:node_id}, ${2:value})', detail: 'Tree: 更新节点值', kind: 'method' },
    { label: 'insert', insert: 'insert(${1:value})', detail: 'BST/Heap: 插入', kind: 'method' },
    { label: 'extract', insert: 'extract()', detail: 'Heap: 提取堆顶', kind: 'method' },
    { label: 'add_node', insert: 'add_node(${1:node_id}, "${2:label}")', detail: 'Graph: 添加节点', kind: 'method' },
    { label: 'add_edge', insert: 'add_edge(${1:from_id}, ${2:to_id})', detail: 'Graph: 添加边', kind: 'method' },
    { label: 'remove_node', insert: 'remove_node(${1:node_id})', detail: 'Graph: 删除节点', kind: 'method' },
    { label: 'remove_edge', insert: 'remove_edge(${1:from_id}, ${2:to_id})', detail: 'Graph: 删除边', kind: 'method' },
    { label: 'update_node_label', insert: 'update_node_label(${1:node_id}, "${2:label}")', detail: 'Graph: 更新标签', kind: 'method' },
    { label: 'update_weight', insert: 'update_weight(${1:from_id}, ${2:to_id}, ${3:weight})', detail: 'WeightedGraph: 更新权重', kind: 'method' },
  ]
}

/* ----------------------------------------------------------------
 *  Lua 补全
 * ---------------------------------------------------------------- */

function buildLuaCompletions(): CompletionDef[] {
  return [
    { label: 'require ds4viz', insert: 'local ds4viz = require("ds4viz")', detail: '导入 ds4viz', kind: 'snippet' },
    { label: 'ds4viz.config', insert: 'ds4viz.config({\n    output_path = "${1:trace.toml}",\n    title = "${2}",\n    author = "${3}",\n    comment = "${4}"\n})', detail: '全局配置', kind: 'function' },
    { label: 'ds4viz.withContext', insert: 'ds4viz.withContext(${1:structure}, function(${2:s})\n    $0\nend)', detail: '上下文管理', kind: 'function' },
    { label: 'ds4viz.stack', insert: 'ds4viz.stack("${1:stack}")', detail: '创建栈', kind: 'function' },
    { label: 'ds4viz.queue', insert: 'ds4viz.queue("${1:queue}")', detail: '创建队列', kind: 'function' },
    { label: 'ds4viz.singleLinkedList', insert: 'ds4viz.singleLinkedList("${1:slist}")', detail: '创建单链表', kind: 'function' },
    { label: 'ds4viz.doubleLinkedList', insert: 'ds4viz.doubleLinkedList("${1:dlist}")', detail: '创建双链表', kind: 'function' },
    { label: 'ds4viz.binaryTree', insert: 'ds4viz.binaryTree("${1:tree}")', detail: '创建二叉树', kind: 'function' },
    { label: 'ds4viz.binarySearchTree', insert: 'ds4viz.binarySearchTree("${1:bst}")', detail: '创建二叉搜索树', kind: 'function' },
    { label: 'ds4viz.heap', insert: 'ds4viz.heap("${1:heap}", "${2|min,max|}")', detail: '创建堆', kind: 'function' },
    { label: 'ds4viz.graphUndirected', insert: 'ds4viz.graphUndirected("${1:graph}")', detail: '创建无向图', kind: 'function' },
    { label: 'ds4viz.graphDirected', insert: 'ds4viz.graphDirected("${1:graph}")', detail: '创建有向图', kind: 'function' },
    { label: 'ds4viz.graphWeighted', insert: 'ds4viz.graphWeighted("${1:graph}", ${2|false,true|})', detail: '创建带权图', kind: 'function' },
    ...buildLuaMethods(),
  ]
}

function buildLuaMethods(): CompletionDef[] {
  return [
    { label: 'push', insert: 'push(${1:value})', detail: 'Stack: 压栈', kind: 'method' },
    { label: 'pop', insert: 'pop()', detail: 'Stack: 弹栈', kind: 'method' },
    { label: 'enqueue', insert: 'enqueue(${1:value})', detail: 'Queue: 入队', kind: 'method' },
    { label: 'dequeue', insert: 'dequeue()', detail: 'Queue: 出队', kind: 'method' },
    { label: 'clear', insert: 'clear()', detail: '清空结构', kind: 'method' },
    { label: 'insertHead', insert: 'insertHead(${1:value})', detail: 'List: 头插', kind: 'method' },
    { label: 'insertTail', insert: 'insertTail(${1:value})', detail: 'List: 尾插', kind: 'method' },
    { label: 'insertAfter', insert: 'insertAfter(${1:node_id}, ${2:value})', detail: 'List: 指定节点后插入', kind: 'method' },
    { label: 'insertBefore', insert: 'insertBefore(${1:node_id}, ${2:value})', detail: 'DList: 指定节点前插入', kind: 'method' },
    { label: 'deleteHead', insert: 'deleteHead()', detail: 'List: 删除头节点', kind: 'method' },
    { label: 'deleteTail', insert: 'deleteTail()', detail: 'DList: 删除尾节点', kind: 'method' },
    { label: 'reverse', insert: 'reverse()', detail: 'List: 反转', kind: 'method' },
    { label: 'insertRoot', insert: 'insertRoot(${1:value})', detail: 'Tree: 插入根节点', kind: 'method' },
    { label: 'insertLeft', insert: 'insertLeft(${1:parent_id}, ${2:value})', detail: 'Tree: 插入左子', kind: 'method' },
    { label: 'insertRight', insert: 'insertRight(${1:parent_id}, ${2:value})', detail: 'Tree: 插入右子', kind: 'method' },
    { label: 'updateValue', insert: 'updateValue(${1:node_id}, ${2:value})', detail: 'Tree: 更新节点值', kind: 'method' },
    { label: 'insert', insert: 'insert(${1:value})', detail: 'BST/Heap: 插入', kind: 'method' },
    { label: 'extract', insert: 'extract()', detail: 'Heap: 提取堆顶', kind: 'method' },
    { label: 'addNode', insert: 'addNode(${1:node_id}, "${2:label}")', detail: 'Graph: 添加节点', kind: 'method' },
    { label: 'addEdge', insert: 'addEdge(${1:from_id}, ${2:to_id})', detail: 'Graph: 添加边', kind: 'method' },
    { label: 'removeNode', insert: 'removeNode(${1:node_id})', detail: 'Graph: 删除节点', kind: 'method' },
    { label: 'removeEdge', insert: 'removeEdge(${1:from_id}, ${2:to_id})', detail: 'Graph: 删除边', kind: 'method' },
    { label: 'updateNodeLabel', insert: 'updateNodeLabel(${1:node_id}, "${2:label}")', detail: 'Graph: 更新标签', kind: 'method' },
    { label: 'updateWeight', insert: 'updateWeight(${1:from_id}, ${2:to_id}, ${3:weight})', detail: 'WeightedGraph: 更新权重', kind: 'method' },
  ]
}

/* ----------------------------------------------------------------
 *  Rust 补全
 * ---------------------------------------------------------------- */

function buildRustCompletions(): CompletionDef[] {
  return [
    { label: 'use ds4viz', insert: 'use ds4viz::prelude::*;', detail: '导入 ds4viz', kind: 'snippet' },
    { label: 'ds4viz::config', insert: 'ds4viz::config(\n    Config::builder()\n        .output_path("${1:trace.toml}")\n        .title("${2}")\n        .author("${3}")\n        .comment("${4}")\n        .build()\n);', detail: '全局配置', kind: 'function' },
    { label: 'ds4viz::stack', insert: 'ds4viz::stack("${1:stack}", |${2:s}| {\n    $0\n    Ok(())\n})', detail: '创建栈', kind: 'function' },
    { label: 'ds4viz::queue', insert: 'ds4viz::queue("${1:queue}", |${2:q}| {\n    $0\n    Ok(())\n})', detail: '创建队列', kind: 'function' },
    { label: 'ds4viz::single_linked_list', insert: 'ds4viz::single_linked_list("${1:slist}", |${2:slist}| {\n    $0\n    Ok(())\n})', detail: '创建单链表', kind: 'function' },
    { label: 'ds4viz::double_linked_list', insert: 'ds4viz::double_linked_list("${1:dlist}", |${2:dlist}| {\n    $0\n    Ok(())\n})', detail: '创建双链表', kind: 'function' },
    { label: 'ds4viz::binary_tree', insert: 'ds4viz::binary_tree("${1:tree}", |${2:tree}| {\n    $0\n    Ok(())\n})', detail: '创建二叉树', kind: 'function' },
    { label: 'ds4viz::binary_search_tree', insert: 'ds4viz::binary_search_tree("${1:bst}", |${2:bst}| {\n    $0\n    Ok(())\n})', detail: '创建二叉搜索树', kind: 'function' },
    { label: 'ds4viz::heap', insert: 'ds4viz::heap("${1:heap}", HeapType::${2|Min,Max|}, |${3:h}| {\n    $0\n    Ok(())\n})', detail: '创建堆', kind: 'function' },
    { label: 'ds4viz::graph_undirected', insert: 'ds4viz::graph_undirected("${1:graph}", |${2:g}| {\n    $0\n    Ok(())\n})', detail: '创建无向图', kind: 'function' },
    { label: 'ds4viz::graph_directed', insert: 'ds4viz::graph_directed("${1:graph}", |${2:g}| {\n    $0\n    Ok(())\n})', detail: '创建有向图', kind: 'function' },
    { label: 'ds4viz::graph_weighted', insert: 'ds4viz::graph_weighted("${1:graph}", ${2|false,true|}, |${3:g}| {\n    $0\n    Ok(())\n})', detail: '创建带权图', kind: 'function' },
    ...buildRustMethods(),
  ]
}

function buildRustMethods(): CompletionDef[] {
  return [
    { label: 'push', insert: 'push(${1:value})?;', detail: 'Stack: 压栈', kind: 'method' },
    { label: 'pop', insert: 'pop()?;', detail: 'Stack: 弹栈', kind: 'method' },
    { label: 'enqueue', insert: 'enqueue(${1:value})?;', detail: 'Queue: 入队', kind: 'method' },
    { label: 'dequeue', insert: 'dequeue()?;', detail: 'Queue: 出队', kind: 'method' },
    { label: 'clear', insert: 'clear()?;', detail: '清空结构', kind: 'method' },
    { label: 'insert_head', insert: 'insert_head(${1:value})?;', detail: 'List: 头插', kind: 'method' },
    { label: 'insert_tail', insert: 'insert_tail(${1:value})?;', detail: 'List: 尾插', kind: 'method' },
    { label: 'insert_after', insert: 'insert_after(${1:node_id}, ${2:value})?;', detail: 'List: 指定节点后插入', kind: 'method' },
    { label: 'insert_before', insert: 'insert_before(${1:node_id}, ${2:value})?;', detail: 'DList: 指定节点前插入', kind: 'method' },
    { label: 'delete', insert: 'delete(${1:id})?;', detail: '删除节点', kind: 'method' },
    { label: 'delete_head', insert: 'delete_head()?;', detail: 'List: 删除头节点', kind: 'method' },
    { label: 'delete_tail', insert: 'delete_tail()?;', detail: 'DList: 删除尾节点', kind: 'method' },
    { label: 'reverse', insert: 'reverse()?;', detail: 'List: 反转', kind: 'method' },
    { label: 'insert_root', insert: 'insert_root(${1:value})?;', detail: 'Tree: 插入根节点', kind: 'method' },
    { label: 'insert_left', insert: 'insert_left(${1:parent_id}, ${2:value})?;', detail: 'Tree: 插入左子', kind: 'method' },
    { label: 'insert_right', insert: 'insert_right(${1:parent_id}, ${2:value})?;', detail: 'Tree: 插入右子', kind: 'method' },
    { label: 'update_value', insert: 'update_value(${1:node_id}, ${2:value})?;', detail: 'Tree: 更新节点值', kind: 'method' },
    { label: 'insert', insert: 'insert(${1:value})?;', detail: 'BST/Heap: 插入', kind: 'method' },
    { label: 'extract', insert: 'extract()?;', detail: 'Heap: 提取堆顶', kind: 'method' },
    { label: 'add_node', insert: 'add_node(${1:node_id}, "${2:label}")?;', detail: 'Graph: 添加节点', kind: 'method' },
    { label: 'add_edge', insert: 'add_edge(${1:from_id}, ${2:to_id})?;', detail: 'Graph: 添加边', kind: 'method' },
    { label: 'remove_node', insert: 'remove_node(${1:node_id})?;', detail: 'Graph: 删除节点', kind: 'method' },
    { label: 'remove_edge', insert: 'remove_edge(${1:from_id}, ${2:to_id})?;', detail: 'Graph: 删除边', kind: 'method' },
    { label: 'update_node_label', insert: 'update_node_label(${1:node_id}, "${2:label}")?;', detail: 'Graph: 更新标签', kind: 'method' },
    { label: 'update_weight', insert: 'update_weight(${1:from_id}, ${2:to_id}, ${3:weight})?;', detail: 'WeightedGraph: 更新权重', kind: 'method' },
  ]
}
