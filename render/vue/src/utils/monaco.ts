/**
 * Monaco 编辑器运行时配置与 ds4viz 语法补全
 *
 * @file src/utils/monaco.ts
 * @author WaterRun
 * @date 2026-03-25
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
  /** 种类 */
  kind: 'function' | 'method' | 'snippet' | 'keyword'
}

/**
 * 注册 ds4viz 双语言补全
 *
 * @param monaco - Monaco 编辑器模块
 */
export function registerDs4vizCompletions(monaco: typeof import('monaco-editor')): void {
  if (completionsRegistered) return
  completionsRegistered = true

  registerLanguageCompletions(monaco, 'python', buildPythonCompletions(), ['.'])
  registerLanguageCompletions(monaco, 'c', buildCCompletions(), ['.', '>', '#'])
}

/**
 * 为指定语言注册补全提供器
 *
 * @param monaco - Monaco 编辑器模块
 * @param languageId - 语言标识
 * @param defs - 补全定义列表
 * @param triggerCharacters - 触发补全的字符列表
 */
function registerLanguageCompletions(
  monaco: typeof import('monaco-editor'),
  languageId: string,
  defs: CompletionDef[],
  triggerCharacters?: string[],
): void {
  monaco.languages.registerCompletionItemProvider(languageId, {
    triggerCharacters,
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
    ...buildPythonKeywords(),
    ...buildPythonBuiltins(),
    ...buildPythonDs4vizCore(),
    ...buildPythonStructures(),
    ...buildPythonCommonMethods(),
    ...buildPythonStructureMethods(),
    ...buildPythonHighlightHelpers(),
  ]
}

function buildPythonKeywords(): CompletionDef[] {
  return [
    { label: 'def', insert: 'def ${1:name}(${2:}):\n    $0', detail: '函数定义', kind: 'keyword' },
    { label: 'class', insert: 'class ${1:Name}:\n    $0', detail: '类定义', kind: 'keyword' },
    { label: 'if', insert: 'if ${1:condition}:\n    $0', detail: '条件判断', kind: 'keyword' },
    { label: 'elif', insert: 'elif ${1:condition}:\n    $0', detail: '条件分支', kind: 'keyword' },
    { label: 'else', insert: 'else:\n    $0', detail: '否则', kind: 'keyword' },
    { label: 'for', insert: 'for ${1:item} in ${2:iterable}:\n    $0', detail: 'for 循环', kind: 'keyword' },
    { label: 'while', insert: 'while ${1:condition}:\n    $0', detail: 'while 循环', kind: 'keyword' },
    { label: 'with', insert: 'with ${1:expr} as ${2:name}:\n    $0', detail: '上下文管理', kind: 'keyword' },
    { label: 'try', insert: 'try:\n    $0\nexcept ${1:Exception} as ${2:e}:\n    pass', detail: '异常处理', kind: 'keyword' },
    { label: 'return', insert: 'return ${0}', detail: '返回', kind: 'keyword' },
    { label: 'import', insert: 'import ${0}', detail: '导入模块', kind: 'keyword' },
    { label: 'from', insert: 'from ${1:module} import ${0}', detail: '导入', kind: 'keyword' },
    { label: 'lambda', insert: 'lambda ${1:x}: ${0}', detail: '匿名函数', kind: 'keyword' },
    { label: 'pass', insert: 'pass', detail: '空语句', kind: 'keyword' },
    { label: 'break', insert: 'break', detail: '跳出循环', kind: 'keyword' },
    { label: 'continue', insert: 'continue', detail: '继续循环', kind: 'keyword' },
    { label: 'raise', insert: 'raise ${1:Exception}(${0})', detail: '抛出异常', kind: 'keyword' },
    { label: 'assert', insert: 'assert ${0}', detail: '断言', kind: 'keyword' },
    { label: 'yield', insert: 'yield ${0}', detail: '生成器', kind: 'keyword' },
    { label: 'if __name__', insert: 'if __name__ == "__main__":\n    $0', detail: '主入口', kind: 'snippet' },
  ]
}

function buildPythonBuiltins(): CompletionDef[] {
  return [
    { label: 'print', insert: 'print(${0})', detail: '打印输出', kind: 'function' },
    { label: 'len', insert: 'len(${0})', detail: '长度', kind: 'function' },
    { label: 'range', insert: 'range(${0})', detail: '范围序列', kind: 'function' },
    { label: 'str', insert: 'str(${0})', detail: '转字符串', kind: 'function' },
    { label: 'int', insert: 'int(${0})', detail: '转整数', kind: 'function' },
    { label: 'float', insert: 'float(${0})', detail: '转浮点', kind: 'function' },
    { label: 'bool', insert: 'bool(${0})', detail: '转布尔', kind: 'function' },
    { label: 'list', insert: 'list(${0})', detail: '转列表', kind: 'function' },
    { label: 'dict', insert: 'dict(${0})', detail: '转字典', kind: 'function' },
    { label: 'sorted', insert: 'sorted(${0})', detail: '排序', kind: 'function' },
    { label: 'enumerate', insert: 'enumerate(${0})', detail: '枚举', kind: 'function' },
    { label: 'zip', insert: 'zip(${0})', detail: '打包', kind: 'function' },
    { label: 'map', insert: 'map(${1:func}, ${0})', detail: '映射', kind: 'function' },
    { label: 'filter', insert: 'filter(${1:func}, ${0})', detail: '过滤', kind: 'function' },
    { label: 'isinstance', insert: 'isinstance(${1:obj}, ${0})', detail: '类型检查', kind: 'function' },
    { label: 'input', insert: 'input(${0})', detail: '读取输入', kind: 'function' },
    { label: 'abs', insert: 'abs(${0})', detail: '绝对值', kind: 'function' },
    { label: 'min', insert: 'min(${0})', detail: '最小值', kind: 'function' },
    { label: 'max', insert: 'max(${0})', detail: '最大值', kind: 'function' },
    { label: 'sum', insert: 'sum(${0})', detail: '求和', kind: 'function' },
  ]
}

function buildPythonDs4vizCore(): CompletionDef[] {
  return [
    { label: 'import ds4viz', insert: 'import ds4viz as dv', detail: '导入 ds4viz', kind: 'snippet' },
    { label: 'dv.config', insert: 'dv.config(\n    output_path="${1:trace.toml}",\n    title="${2}",\n    author="${3}",\n    comment="${4}"\n)', detail: '全局配置', kind: 'function' },
  ]
}

function buildPythonStructures(): CompletionDef[] {
  return [
    { label: 'dv.stack', insert: 'with dv.stack(label="${1:stack}") as ${2:s}:\n    $0', detail: '创建栈', kind: 'function' },
    { label: 'dv.queue', insert: 'with dv.queue(label="${1:queue}") as ${2:q}:\n    $0', detail: '创建队列', kind: 'function' },
    { label: 'dv.single_linked_list', insert: 'with dv.single_linked_list(label="${1:slist}") as ${2:slist}:\n    $0', detail: '创建单链表', kind: 'function' },
    { label: 'dv.double_linked_list', insert: 'with dv.double_linked_list(label="${1:dlist}") as ${2:dlist}:\n    $0', detail: '创建双链表', kind: 'function' },
    { label: 'dv.binary_tree', insert: 'with dv.binary_tree(label="${1:tree}") as ${2:tree}:\n    $0', detail: '创建二叉树', kind: 'function' },
    { label: 'dv.binary_search_tree', insert: 'with dv.binary_search_tree(label="${1:bst}") as ${2:bst}:\n    $0', detail: '创建二叉搜索树', kind: 'function' },
    { label: 'dv.graph_undirected', insert: 'with dv.graph_undirected(label="${1:graph}") as ${2:g}:\n    $0', detail: '创建无向图', kind: 'function' },
    { label: 'dv.graph_directed', insert: 'with dv.graph_directed(label="${1:graph}") as ${2:g}:\n    $0', detail: '创建有向图', kind: 'function' },
    { label: 'dv.graph_weighted', insert: 'with dv.graph_weighted(label="${1:graph}") as ${2:g}:\n    $0', detail: '创建带权图', kind: 'function' },
  ]
}

function buildPythonHighlightHelpers(): CompletionDef[] {
  return [
    { label: 'dv.node', insert: 'dv.node(${1:node_id}, "${2:focus}", ${3:1})', detail: '节点高亮标记', kind: 'function' },
    { label: 'dv.item', insert: 'dv.item(${1:index}, "${2:focus}", ${3:1})', detail: '元素高亮标记 (stack/queue)', kind: 'function' },
    { label: 'dv.edge', insert: 'dv.edge(${1:from_id}, ${2:to_id}, "${3:focus}", ${4:1})', detail: '边高亮标记 (graph)', kind: 'function' },
  ]
}

function buildPythonCommonMethods(): CompletionDef[] {
  return [
    { label: 'step', insert: 'step(note="${1}", highlights=[${0}])', detail: '记录观察步骤（不修改状态）', kind: 'method' },
    { label: 'amend', insert: 'amend(note="${1}", highlights=[${0}])', detail: '修改上一步展示属性', kind: 'method' },
    { label: 'phase', insert: 'with ${1:obj}.phase("${2:name}"):\n    $0', detail: '阶段上下文管理器', kind: 'method' },
    { label: 'alias', insert: 'alias(${1:node_id}, "${2:name}")', detail: '设置节点别名', kind: 'method' },
  ]
}

function buildPythonStructureMethods(): CompletionDef[] {
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
    { label: 'insert', insert: 'insert(${1:value})', detail: 'BST: 插入', kind: 'method' },
    { label: 'add_node', insert: 'add_node(${1:node_id}, "${2:label}")', detail: 'Graph: 添加节点', kind: 'method' },
    { label: 'add_edge', insert: 'add_edge(${1:from_id}, ${2:to_id})', detail: 'Graph: 添加边', kind: 'method' },
    { label: 'remove_node', insert: 'remove_node(${1:node_id})', detail: 'Graph: 删除节点', kind: 'method' },
    { label: 'remove_edge', insert: 'remove_edge(${1:from_id}, ${2:to_id})', detail: 'Graph: 删除边', kind: 'method' },
    { label: 'update_node_label', insert: 'update_node_label(${1:node_id}, "${2:label}")', detail: 'Graph: 更新标签', kind: 'method' },
    { label: 'update_weight', insert: 'update_weight(${1:from_id}, ${2:to_id}, ${3:weight})', detail: 'WeightedGraph: 更新权重', kind: 'method' },
  ]
}

/* ----------------------------------------------------------------
 *  C 补全
 * ---------------------------------------------------------------- */

function buildCCompletions(): CompletionDef[] {
  return [
    ...buildCKeywords(),
    ...buildCBuiltins(),
    ...buildDs4vizCCore(),
    ...buildDs4vizCStructures(),
    ...buildDs4vizCCommon(),
    ...buildDs4vizCHighlightHelpers(),
    ...buildDs4vizCOperations(),
  ]
}

function buildCKeywords(): CompletionDef[] {
  return [
    { label: '#include', insert: '#include <${0}>', detail: '包含头文件', kind: 'keyword' },
    { label: '#define', insert: '#define ${1:NAME} ${0}', detail: '宏定义', kind: 'keyword' },
    { label: 'int main', insert: 'int main(void) {\n  $0\n  return 0;\n}', detail: '主函数模板', kind: 'snippet' },
    { label: 'if', insert: 'if (${1:condition}) {\n  $0\n}', detail: '条件判断', kind: 'keyword' },
    { label: 'for', insert: 'for (${1:int i = 0}; ${2:i < n}; ${3:i++}) {\n  $0\n}', detail: 'for 循环', kind: 'keyword' },
    { label: 'while', insert: 'while (${1:condition}) {\n  $0\n}', detail: 'while 循环', kind: 'keyword' },
    { label: 'switch', insert: 'switch (${1:expr}) {\n  case ${2:value}:\n    $0\n    break;\n  default:\n    break;\n}', detail: 'switch 分支', kind: 'keyword' },
    { label: 'typedef struct', insert: 'typedef struct {\n  $0\n} ${1:Name};', detail: '结构体定义', kind: 'snippet' },
  ]
}

function buildCBuiltins(): CompletionDef[] {
  return [
    { label: 'printf', insert: 'printf("${1:%d\\\\n}", ${0});', detail: '标准输出', kind: 'function' },
    { label: 'scanf', insert: 'scanf("${1:%d}", &${0});', detail: '标准输入', kind: 'function' },
    { label: 'malloc', insert: 'malloc(${0})', detail: '动态分配内存', kind: 'function' },
    { label: 'free', insert: 'free(${0});', detail: '释放内存', kind: 'function' },
    { label: 'memset', insert: 'memset(${1:ptr}, ${2:0}, ${0});', detail: '内存填充', kind: 'function' },
    { label: 'memcpy', insert: 'memcpy(${1:dst}, ${2:src}, ${0});', detail: '内存拷贝', kind: 'function' },
    { label: 'snprintf', insert: 'snprintf(${1:buf}, sizeof ${1:buf}, "${2:%s}", ${0});', detail: '格式化字符串', kind: 'function' },
  ]
}

function buildDs4vizCCore(): CompletionDef[] {
  return [
    {
      label: 'ds4viz include',
      insert: '#define DS4VIZ_IMPLEMENTATION\n#define DS4VIZ_SHORT_NAMES\n#include "ds4viz.h"',
      detail: '引入 ds4viz C 单头文件',
      kind: 'snippet',
    },
    {
      label: 'dvConfig',
      insert: 'dvConfig((dvConfigOptions){\n  .output_path = "${1:trace.toml}",\n  .title = "${2}",\n  .author = "${3}",\n  .comment = "${4}"\n});',
      detail: '配置 trace 输出',
      kind: 'function',
    },
  ]
}

function buildDs4vizCStructures(): CompletionDef[] {
  return [
    { label: 'dvStack', insert: 'dvStack(${1:s}, "${2:stack}") {\n  $0\n}', detail: '创建栈作用域', kind: 'snippet' },
    { label: 'dvQueue', insert: 'dvQueue(${1:q}, "${2:queue}") {\n  $0\n}', detail: '创建队列作用域', kind: 'snippet' },
    { label: 'dvSingleLinkedList', insert: 'dvSingleLinkedList(${1:l}, "${2:slist}") {\n  $0\n}', detail: '创建单链表作用域', kind: 'snippet' },
    { label: 'dvDoubleLinkedList', insert: 'dvDoubleLinkedList(${1:l}, "${2:dlist}") {\n  $0\n}', detail: '创建双链表作用域', kind: 'snippet' },
    { label: 'dvBinaryTree', insert: 'dvBinaryTree(${1:t}, "${2:tree}") {\n  $0\n}', detail: '创建二叉树作用域', kind: 'snippet' },
    { label: 'dvBinarySearchTree', insert: 'dvBinarySearchTree(${1:b}, "${2:bst}") {\n  $0\n}', detail: '创建 BST 作用域', kind: 'snippet' },
    { label: 'dvGraphUndirected', insert: 'dvGraphUndirected(${1:g}, "${2:graph}") {\n  $0\n}', detail: '创建无向图作用域', kind: 'snippet' },
    { label: 'dvGraphDirected', insert: 'dvGraphDirected(${1:g}, "${2:graph}") {\n  $0\n}', detail: '创建有向图作用域', kind: 'snippet' },
    { label: 'dvGraphWeighted', insert: 'dvGraphWeighted(${1:g}, "${2:graph}") {\n  $0\n}', detail: '创建带权图作用域', kind: 'snippet' },
  ]
}

function buildDs4vizCCommon(): CompletionDef[] {
  return [
    { label: 'dvStep', insert: 'dvStep(${1:obj}, "${2:note}",\n  ${0});', detail: '记录观察步骤', kind: 'function' },
    { label: 'dvAmend', insert: 'dvAmend(${1:obj}, "${2:note}",\n  ${0});', detail: '修改上一步 note + highlights', kind: 'function' },
    { label: 'dvAmendHL', insert: 'dvAmendHL(${1:obj},\n  ${0});', detail: '仅修改上一步 highlights', kind: 'function' },
    { label: 'dvAmendClearHL', insert: 'dvAmendClearHL(${1:obj});', detail: '清除上一步 highlights', kind: 'function' },
    { label: 'dvPhase', insert: 'dvPhase(${1:obj}, "${2:name}") {\n  $0\n}', detail: '阶段作用域块', kind: 'snippet' },
    { label: 'dvAlias', insert: 'dvAlias(${1:obj}, ${2:node_id}, "${3:name}");', detail: '设置节点别名', kind: 'function' },
    { label: 'dvValue', insert: 'dvValue ${1:val} = ${0};', detail: '带标签联合体', kind: 'snippet' },
    { label: 'dvInt', insert: 'dvInt(${1:val})', detail: '提取 int64_t', kind: 'function' },
    { label: 'dvFloat', insert: 'dvFloat(${1:val})', detail: '提取 double', kind: 'function' },
    { label: 'dvStr', insert: 'dvStr(${1:val})', detail: '提取 const char*', kind: 'function' },
    { label: 'dvBool', insert: 'dvBool(${1:val})', detail: '提取 bool', kind: 'function' },
  ]
}

function buildDs4vizCHighlightHelpers(): CompletionDef[] {
  return [
    { label: 'dvNode', insert: 'dvNode(${1:target}, ${2:DV_FOCUS}, ${3:1})', detail: '节点高亮标记', kind: 'function' },
    { label: 'dvItem', insert: 'dvItem(${1:index}, ${2:DV_FOCUS}, ${3:1})', detail: '元素高亮标记 (stack/queue)', kind: 'function' },
    { label: 'dvEdge', insert: 'dvEdge(${1:from}, ${2:to}, ${3:DV_FOCUS}, ${4:1})', detail: '边高亮标记 (graph)', kind: 'function' },
    { label: 'DV_FOCUS', insert: 'DV_FOCUS', detail: '样式: 当前焦点', kind: 'keyword' },
    { label: 'DV_VISITED', insert: 'DV_VISITED', detail: '样式: 已访问', kind: 'keyword' },
    { label: 'DV_ACTIVE', insert: 'DV_ACTIVE', detail: '样式: 参与当前计算', kind: 'keyword' },
    { label: 'DV_COMPARING', insert: 'DV_COMPARING', detail: '样式: 正在比较', kind: 'keyword' },
    { label: 'DV_FOUND', insert: 'DV_FOUND', detail: '样式: 查找命中', kind: 'keyword' },
    { label: 'DV_ERROR_STY', insert: 'DV_ERROR_STY', detail: '样式: 异常状态', kind: 'keyword' },
  ]
}

function buildDs4vizCOperations(): CompletionDef[] {
  return [
    { label: 'dvStackPush', insert: 'dvStackPush(${1:s}, ${0});', detail: '栈压入', kind: 'method' },
    { label: 'dvStackPop', insert: 'dvValue ${1:val} = dvStackPop(${2:s});', detail: '栈弹出', kind: 'method' },
    { label: 'dvStackClear', insert: 'dvStackClear(${1:s});', detail: '清空栈', kind: 'method' },
    { label: 'dvQueueEnqueue', insert: 'dvQueueEnqueue(${1:q}, ${0});', detail: '队列入队', kind: 'method' },
    { label: 'dvQueueDequeue', insert: 'dvValue ${1:val} = dvQueueDequeue(${2:q});', detail: '队列出队', kind: 'method' },
    { label: 'dvQueueClear', insert: 'dvQueueClear(${1:q});', detail: '清空队列', kind: 'method' },
    { label: 'dvSlInsertHead', insert: 'int ${1:id} = dvSlInsertHead(${2:l}, ${0});', detail: '单链表头插', kind: 'method' },
    { label: 'dvSlInsertTail', insert: 'int ${1:id} = dvSlInsertTail(${2:l}, ${0});', detail: '单链表尾插', kind: 'method' },
    { label: 'dvSlInsertAfter', insert: 'int ${1:id} = dvSlInsertAfter(${2:l}, ${3:node_id}, ${0});', detail: '单链表后插', kind: 'method' },
    { label: 'dvSlDelete', insert: 'dvSlDelete(${1:l}, ${2:node_id});', detail: '单链表删除节点', kind: 'method' },
    { label: 'dvSlDeleteHead', insert: 'dvSlDeleteHead(${1:l});', detail: '单链表删除头', kind: 'method' },
    { label: 'dvSlReverse', insert: 'dvSlReverse(${1:l});', detail: '单链表反转', kind: 'method' },
    { label: 'dvDlInsertHead', insert: 'int ${1:id} = dvDlInsertHead(${2:l}, ${0});', detail: '双链表头插', kind: 'method' },
    { label: 'dvDlInsertTail', insert: 'int ${1:id} = dvDlInsertTail(${2:l}, ${0});', detail: '双链表尾插', kind: 'method' },
    { label: 'dvDlInsertBefore', insert: 'int ${1:id} = dvDlInsertBefore(${2:l}, ${3:node_id}, ${0});', detail: '双链表前插', kind: 'method' },
    { label: 'dvDlInsertAfter', insert: 'int ${1:id} = dvDlInsertAfter(${2:l}, ${3:node_id}, ${0});', detail: '双链表后插', kind: 'method' },
    { label: 'dvDlDelete', insert: 'dvDlDelete(${1:l}, ${2:node_id});', detail: '双链表删除节点', kind: 'method' },
    { label: 'dvDlDeleteHead', insert: 'dvDlDeleteHead(${1:l});', detail: '双链表删除头', kind: 'method' },
    { label: 'dvDlDeleteTail', insert: 'dvDlDeleteTail(${1:l});', detail: '双链表删除尾', kind: 'method' },
    { label: 'dvDlReverse', insert: 'dvDlReverse(${1:l});', detail: '双链表反转', kind: 'method' },
    { label: 'dvBtInsertRoot', insert: 'int ${1:root} = dvBtInsertRoot(${2:t}, ${0});', detail: '树插入根节点', kind: 'method' },
    { label: 'dvBtInsertLeft', insert: 'int ${1:left} = dvBtInsertLeft(${2:t}, ${3:parent_id}, ${0});', detail: '树插入左子', kind: 'method' },
    { label: 'dvBtInsertRight', insert: 'int ${1:right} = dvBtInsertRight(${2:t}, ${3:parent_id}, ${0});', detail: '树插入右子', kind: 'method' },
    { label: 'dvBtDelete', insert: 'dvBtDelete(${1:t}, ${2:node_id});', detail: '树删除节点及子树', kind: 'method' },
    { label: 'dvBtUpdateValue', insert: 'dvBtUpdateValue(${1:t}, ${2:node_id}, ${0});', detail: '树更新节点值', kind: 'method' },
    { label: 'dvBstInsert', insert: 'int ${1:id} = dvBstInsert(${2:b}, ${0});', detail: 'BST 插入', kind: 'method' },
    { label: 'dvBstDelete', insert: 'dvBstDelete(${1:b}, ${0});', detail: 'BST 删除', kind: 'method' },
    { label: 'dvGuAddNode', insert: 'dvGuAddNode(${1:g}, ${2:id}, "${0:label}");', detail: '无向图添加节点', kind: 'method' },
    { label: 'dvGuAddEdge', insert: 'dvGuAddEdge(${1:g}, ${2:from}, ${3:to});', detail: '无向图添加边', kind: 'method' },
    { label: 'dvGuRemoveNode', insert: 'dvGuRemoveNode(${1:g}, ${2:id});', detail: '无向图删除节点', kind: 'method' },
    { label: 'dvGuRemoveEdge', insert: 'dvGuRemoveEdge(${1:g}, ${2:from}, ${3:to});', detail: '无向图删除边', kind: 'method' },
    { label: 'dvGuUpdateNodeLabel', insert: 'dvGuUpdateNodeLabel(${1:g}, ${2:id}, "${0:label}");', detail: '无向图更新标签', kind: 'method' },
    { label: 'dvGdAddNode', insert: 'dvGdAddNode(${1:g}, ${2:id}, "${0:label}");', detail: '有向图添加节点', kind: 'method' },
    { label: 'dvGdAddEdge', insert: 'dvGdAddEdge(${1:g}, ${2:from}, ${3:to});', detail: '有向图添加边', kind: 'method' },
    { label: 'dvGdRemoveNode', insert: 'dvGdRemoveNode(${1:g}, ${2:id});', detail: '有向图删除节点', kind: 'method' },
    { label: 'dvGdRemoveEdge', insert: 'dvGdRemoveEdge(${1:g}, ${2:from}, ${3:to});', detail: '有向图删除边', kind: 'method' },
    { label: 'dvGdUpdateNodeLabel', insert: 'dvGdUpdateNodeLabel(${1:g}, ${2:id}, "${0:label}");', detail: '有向图更新标签', kind: 'method' },
    { label: 'dvGwAddNode', insert: 'dvGwAddNode(${1:g}, ${2:id}, "${0:label}");', detail: '带权图添加节点', kind: 'method' },
    { label: 'dvGwAddEdge', insert: 'dvGwAddEdge(${1:g}, ${2:from}, ${3:to}, ${0:weight});', detail: '带权图添加边', kind: 'method' },
    { label: 'dvGwRemoveNode', insert: 'dvGwRemoveNode(${1:g}, ${2:id});', detail: '带权图删除节点', kind: 'method' },
    { label: 'dvGwRemoveEdge', insert: 'dvGwRemoveEdge(${1:g}, ${2:from}, ${3:to});', detail: '带权图删除边', kind: 'method' },
    { label: 'dvGwUpdateWeight', insert: 'dvGwUpdateWeight(${1:g}, ${2:from}, ${3:to}, ${0:weight});', detail: '带权图更新权重', kind: 'method' },
    { label: 'dvGwUpdateNodeLabel', insert: 'dvGwUpdateNodeLabel(${1:g}, ${2:id}, "${0:label}");', detail: '带权图更新标签', kind: 'method' },
  ]
}
