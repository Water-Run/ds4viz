/**
 * Monaco 编辑器运行时配置与 ds4viz 语法补全
 *
 * @file src/utils/monaco.ts
 * @author WaterRun
 * @date 2026-02-28
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

  registerLanguageCompletions(monaco, 'python', buildPythonCompletions(), ['.'])
  registerLanguageCompletions(monaco, 'lua', buildLuaCompletions(), ['.', ':'])
  registerLanguageCompletions(monaco, 'rust', buildRustCompletions(), ['.', ':'])
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
    { label: 'assert', insert: 'assert ${0}', detail: '断言', kind: 'keyword' },
    { label: 'raise', insert: 'raise ${1:Exception}(${0})', detail: '抛出异常', kind: 'keyword' },
    { label: 'pass', insert: 'pass', detail: '空语句', kind: 'keyword' },
    { label: 'break', insert: 'break', detail: '跳出循环', kind: 'keyword' },
    { label: 'continue', insert: 'continue', detail: '继续循环', kind: 'keyword' },
    { label: 'yield', insert: 'yield ${0}', detail: '生成器', kind: 'keyword' },
    { label: 'async def', insert: 'async def ${1:name}(${2:}):\n    $0', detail: '异步函数', kind: 'keyword' },
    { label: 'await', insert: 'await ${0}', detail: '等待异步', kind: 'keyword' },
  ]
}

function buildPythonBuiltins(): CompletionDef[] {
  return [
    { label: 'print', insert: 'print(${0})', detail: '打印输出', kind: 'function' },
    { label: 'len', insert: 'len(${0})', detail: '长度', kind: 'function' },
    { label: 'range', insert: 'range(${0})', detail: '范围序列', kind: 'function' },
    { label: 'type', insert: 'type(${0})', detail: '类型', kind: 'function' },
    { label: 'str', insert: 'str(${0})', detail: '转字符串', kind: 'function' },
    { label: 'int', insert: 'int(${0})', detail: '转整数', kind: 'function' },
    { label: 'float', insert: 'float(${0})', detail: '转浮点', kind: 'function' },
    { label: 'bool', insert: 'bool(${0})', detail: '转布尔', kind: 'function' },
    { label: 'list', insert: 'list(${0})', detail: '转列表', kind: 'function' },
    { label: 'dict', insert: 'dict(${0})', detail: '转字典', kind: 'function' },
    { label: 'tuple', insert: 'tuple(${0})', detail: '转元组', kind: 'function' },
    { label: 'set', insert: 'set(${0})', detail: '转集合', kind: 'function' },
    { label: 'sorted', insert: 'sorted(${0})', detail: '排序', kind: 'function' },
    { label: 'reversed', insert: 'reversed(${0})', detail: '反转迭代器', kind: 'function' },
    { label: 'enumerate', insert: 'enumerate(${0})', detail: '枚举', kind: 'function' },
    { label: 'zip', insert: 'zip(${0})', detail: '打包', kind: 'function' },
    { label: 'map', insert: 'map(${1:func}, ${0})', detail: '映射', kind: 'function' },
    { label: 'filter', insert: 'filter(${1:func}, ${0})', detail: '过滤', kind: 'function' },
    { label: 'isinstance', insert: 'isinstance(${1:obj}, ${0})', detail: '类型检查', kind: 'function' },
    { label: 'hasattr', insert: 'hasattr(${1:obj}, "${0}")', detail: '属性检查', kind: 'function' },
    { label: 'getattr', insert: 'getattr(${1:obj}, "${0}")', detail: '获取属性', kind: 'function' },
    { label: 'input', insert: 'input(${0})', detail: '读取输入', kind: 'function' },
    { label: 'abs', insert: 'abs(${0})', detail: '绝对值', kind: 'function' },
    { label: 'min', insert: 'min(${0})', detail: '最小值', kind: 'function' },
    { label: 'max', insert: 'max(${0})', detail: '最大值', kind: 'function' },
    { label: 'sum', insert: 'sum(${0})', detail: '求和', kind: 'function' },
    { label: 'round', insert: 'round(${0})', detail: '四舍五入', kind: 'function' },
    { label: 'open', insert: 'open("${0}")', detail: '打开文件', kind: 'function' },
    { label: 'super', insert: 'super()', detail: '父类引用', kind: 'function' },
    { label: '__init__', insert: 'def __init__(self, ${0}):\n    ', detail: '构造函数', kind: 'method' },
    { label: '__str__', insert: 'def __str__(self):\n    return ${0}', detail: '字符串表示', kind: 'method' },
    { label: '__repr__', insert: 'def __repr__(self):\n    return ${0}', detail: '对象表示', kind: 'method' },
    { label: 'if __name__', insert: 'if __name__ == "__main__":\n    $0', detail: '主入口', kind: 'snippet' },
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
    ...buildLuaKeywords(),
    ...buildLuaBuiltins(),
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

function buildLuaKeywords(): CompletionDef[] {
  return [
    { label: 'function', insert: 'function ${1:name}(${2:})\n    $0\nend', detail: '函数定义', kind: 'keyword' },
    { label: 'local function', insert: 'local function ${1:name}(${2:})\n    $0\nend', detail: '局部函数', kind: 'keyword' },
    { label: 'local', insert: 'local ${1:name} = ${0}', detail: '局部变量', kind: 'keyword' },
    { label: 'if', insert: 'if ${1:condition} then\n    $0\nend', detail: '条件判断', kind: 'keyword' },
    { label: 'elseif', insert: 'elseif ${1:condition} then\n    $0', detail: '条件分支', kind: 'keyword' },
    { label: 'else', insert: 'else\n    $0', detail: '否则', kind: 'keyword' },
    { label: 'for numeric', insert: 'for ${1:i} = ${2:1}, ${3:10} do\n    $0\nend', detail: '数值 for 循环', kind: 'keyword' },
    { label: 'for generic', insert: 'for ${1:k}, ${2:v} in pairs(${3:t}) do\n    $0\nend', detail: '泛型 for 循环', kind: 'keyword' },
    { label: 'while', insert: 'while ${1:condition} do\n    $0\nend', detail: 'while 循环', kind: 'keyword' },
    { label: 'repeat', insert: 'repeat\n    $0\nuntil ${1:condition}', detail: 'repeat-until 循环', kind: 'keyword' },
    { label: 'return', insert: 'return ${0}', detail: '返回', kind: 'keyword' },
    { label: 'do', insert: 'do\n    $0\nend', detail: 'do 块', kind: 'keyword' },
    { label: 'break', insert: 'break', detail: '跳出循环', kind: 'keyword' },
    { label: 'require', insert: 'require("${0}")', detail: '导入模块', kind: 'keyword' },
    { label: 'nil', insert: 'nil', detail: '空值', kind: 'keyword' },
    { label: 'true', insert: 'true', detail: '真', kind: 'keyword' },
    { label: 'false', insert: 'false', detail: '假', kind: 'keyword' },
    { label: 'not', insert: 'not ', detail: '逻辑非', kind: 'keyword' },
    { label: 'and', insert: 'and ', detail: '逻辑与', kind: 'keyword' },
    { label: 'or', insert: 'or ', detail: '逻辑或', kind: 'keyword' },
  ]
}

function buildLuaBuiltins(): CompletionDef[] {
  return [
    { label: 'print', insert: 'print(${0})', detail: '打印输出', kind: 'function' },
    { label: 'tostring', insert: 'tostring(${0})', detail: '转字符串', kind: 'function' },
    { label: 'tonumber', insert: 'tonumber(${0})', detail: '转数字', kind: 'function' },
    { label: 'type', insert: 'type(${0})', detail: '类型', kind: 'function' },
    { label: 'pairs', insert: 'pairs(${0})', detail: '键值对迭代', kind: 'function' },
    { label: 'ipairs', insert: 'ipairs(${0})', detail: '索引迭代', kind: 'function' },
    { label: 'pcall', insert: 'pcall(${0})', detail: '保护调用', kind: 'function' },
    { label: 'error', insert: 'error("${0}")', detail: '抛出错误', kind: 'function' },
    { label: 'assert', insert: 'assert(${0})', detail: '断言', kind: 'function' },
    { label: 'select', insert: 'select(${0})', detail: '选择参数', kind: 'function' },
    { label: 'unpack', insert: 'unpack(${0})', detail: '解包表', kind: 'function' },
    { label: 'rawget', insert: 'rawget(${1:t}, ${0})', detail: '原始获取', kind: 'function' },
    { label: 'rawset', insert: 'rawset(${1:t}, ${2:k}, ${0})', detail: '原始设置', kind: 'function' },
    { label: 'setmetatable', insert: 'setmetatable(${1:t}, ${0})', detail: '设置元表', kind: 'function' },
    { label: 'getmetatable', insert: 'getmetatable(${0})', detail: '获取元表', kind: 'function' },
    { label: 'table.insert', insert: 'table.insert(${1:t}, ${0})', detail: '插入元素', kind: 'function' },
    { label: 'table.remove', insert: 'table.remove(${1:t}, ${0})', detail: '移除元素', kind: 'function' },
    { label: 'table.sort', insert: 'table.sort(${0})', detail: '排序', kind: 'function' },
    { label: 'table.concat', insert: 'table.concat(${1:t}, "${0}")', detail: '连接', kind: 'function' },
    { label: 'string.format', insert: 'string.format("${0}")', detail: '格式化', kind: 'function' },
    { label: 'string.find', insert: 'string.find(${1:s}, "${0}")', detail: '查找', kind: 'function' },
    { label: 'string.sub', insert: 'string.sub(${1:s}, ${2:i}, ${0})', detail: '子串', kind: 'function' },
    { label: 'string.len', insert: 'string.len(${0})', detail: '长度', kind: 'function' },
    { label: 'math.floor', insert: 'math.floor(${0})', detail: '向下取整', kind: 'function' },
    { label: 'math.ceil', insert: 'math.ceil(${0})', detail: '向上取整', kind: 'function' },
    { label: 'math.abs', insert: 'math.abs(${0})', detail: '绝对值', kind: 'function' },
    { label: 'math.max', insert: 'math.max(${0})', detail: '最大值', kind: 'function' },
    { label: 'math.min', insert: 'math.min(${0})', detail: '最小值', kind: 'function' },
    { label: 'math.random', insert: 'math.random(${0})', detail: '随机数', kind: 'function' },
    { label: 'math.sqrt', insert: 'math.sqrt(${0})', detail: '平方根', kind: 'function' },
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
    ...buildRustKeywords(),
    ...buildRustBuiltins(),
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

function buildRustKeywords(): CompletionDef[] {
  return [
    { label: 'fn', insert: 'fn ${1:name}(${2:}) -> ${3:()} {\n    $0\n}', detail: '函数定义', kind: 'keyword' },
    { label: 'pub fn', insert: 'pub fn ${1:name}(${2:}) -> ${3:()} {\n    $0\n}', detail: '公开函数', kind: 'keyword' },
    { label: 'let', insert: 'let ${1:name} = ${0};', detail: '变量绑定', kind: 'keyword' },
    { label: 'let mut', insert: 'let mut ${1:name} = ${0};', detail: '可变变量', kind: 'keyword' },
    { label: 'const', insert: 'const ${1:NAME}: ${2:type} = ${0};', detail: '常量', kind: 'keyword' },
    { label: 'struct', insert: 'struct ${1:Name} {\n    $0\n}', detail: '结构体', kind: 'keyword' },
    { label: 'enum', insert: 'enum ${1:Name} {\n    $0\n}', detail: '枚举', kind: 'keyword' },
    { label: 'impl', insert: 'impl ${1:Name} {\n    $0\n}', detail: '实现块', kind: 'keyword' },
    { label: 'trait', insert: 'trait ${1:Name} {\n    $0\n}', detail: '特质', kind: 'keyword' },
    { label: 'if', insert: 'if ${1:condition} {\n    $0\n}', detail: '条件判断', kind: 'keyword' },
    { label: 'if let', insert: 'if let ${1:pattern} = ${2:expr} {\n    $0\n}', detail: '模式匹配条件', kind: 'keyword' },
    { label: 'else', insert: 'else {\n    $0\n}', detail: '否则', kind: 'keyword' },
    { label: 'for', insert: 'for ${1:item} in ${2:iter} {\n    $0\n}', detail: 'for 循环', kind: 'keyword' },
    { label: 'while', insert: 'while ${1:condition} {\n    $0\n}', detail: 'while 循环', kind: 'keyword' },
    { label: 'while let', insert: 'while let ${1:pattern} = ${2:expr} {\n    $0\n}', detail: '模式匹配循环', kind: 'keyword' },
    { label: 'loop', insert: 'loop {\n    $0\n}', detail: '无限循环', kind: 'keyword' },
    { label: 'match', insert: 'match ${1:expr} {\n    ${2:_} => $0,\n}', detail: '模式匹配', kind: 'keyword' },
    { label: 'return', insert: 'return ${0};', detail: '返回', kind: 'keyword' },
    { label: 'break', insert: 'break;', detail: '跳出循环', kind: 'keyword' },
    { label: 'continue', insert: 'continue;', detail: '继续循环', kind: 'keyword' },
    { label: 'use', insert: 'use ${0};', detail: '导入', kind: 'keyword' },
    { label: 'mod', insert: 'mod ${1:name} {\n    $0\n}', detail: '模块', kind: 'keyword' },
    { label: 'type', insert: 'type ${1:Name} = ${0};', detail: '类型别名', kind: 'keyword' },
    { label: 'async fn', insert: 'async fn ${1:name}(${2:}) -> ${3:()} {\n    $0\n}', detail: '异步函数', kind: 'keyword' },
    { label: 'pub', insert: 'pub ', detail: '公开', kind: 'keyword' },
    { label: 'mut', insert: 'mut ', detail: '可变', kind: 'keyword' },
    { label: 'self', insert: 'self', detail: '自身', kind: 'keyword' },
    { label: 'Self', insert: 'Self', detail: '自身类型', kind: 'keyword' },
    { label: 'where', insert: 'where\n    ${0}', detail: '泛型约束', kind: 'keyword' },
  ]
}

function buildRustBuiltins(): CompletionDef[] {
  return [
    { label: 'println!', insert: 'println!("${0}");', detail: '打印行', kind: 'function' },
    { label: 'print!', insert: 'print!("${0}");', detail: '打印', kind: 'function' },
    { label: 'eprintln!', insert: 'eprintln!("${0}");', detail: '错误输出', kind: 'function' },
    { label: 'format!', insert: 'format!("${0}")', detail: '格式化字符串', kind: 'function' },
    { label: 'vec!', insert: 'vec![${0}]', detail: '创建 Vec', kind: 'function' },
    { label: 'todo!', insert: 'todo!("${0}")', detail: '待实现', kind: 'function' },
    { label: 'unimplemented!', insert: 'unimplemented!()', detail: '未实现', kind: 'function' },
    { label: 'panic!', insert: 'panic!("${0}")', detail: '恐慌', kind: 'function' },
    { label: 'assert!', insert: 'assert!(${0});', detail: '断言', kind: 'function' },
    { label: 'assert_eq!', insert: 'assert_eq!(${1:left}, ${0});', detail: '相等断言', kind: 'function' },
    { label: 'dbg!', insert: 'dbg!(${0})', detail: '调试输出', kind: 'function' },
    { label: 'String::new', insert: 'String::new()', detail: '空字符串', kind: 'function' },
    { label: 'String::from', insert: 'String::from("${0}")', detail: '创建 String', kind: 'function' },
    { label: 'Vec::new', insert: 'Vec::new()', detail: '空 Vec', kind: 'function' },
    { label: 'HashMap::new', insert: 'HashMap::new()', detail: '空 HashMap', kind: 'function' },
    { label: 'Option::Some', insert: 'Some(${0})', detail: '有值', kind: 'function' },
    { label: 'Option::None', insert: 'None', detail: '无值', kind: 'function' },
    { label: 'Result::Ok', insert: 'Ok(${0})', detail: '成功', kind: 'function' },
    { label: 'Result::Err', insert: 'Err(${0})', detail: '错误', kind: 'function' },
    { label: '.unwrap()', insert: 'unwrap()', detail: '解包', kind: 'method' },
    { label: '.expect()', insert: 'expect("${0}")', detail: '解包或报错', kind: 'method' },
    { label: '.map()', insert: 'map(|${1:x}| ${0})', detail: '映射', kind: 'method' },
    { label: '.filter()', insert: 'filter(|${1:x}| ${0})', detail: '过滤', kind: 'method' },
    { label: '.collect()', insert: 'collect::<${0}>()', detail: '收集', kind: 'method' },
    { label: '.iter()', insert: 'iter()', detail: '迭代器', kind: 'method' },
    { label: '.into_iter()', insert: 'into_iter()', detail: '消费迭代器', kind: 'method' },
    { label: '.len()', insert: 'len()', detail: '长度', kind: 'method' },
    { label: '.is_empty()', insert: 'is_empty()', detail: '是否为空', kind: 'method' },
    { label: '.push()', insert: 'push(${0})', detail: '追加', kind: 'method' },
    { label: '.clone()', insert: 'clone()', detail: '克隆', kind: 'method' },
    { label: '.to_string()', insert: 'to_string()', detail: '转字符串', kind: 'method' },
    { label: '.as_str()', insert: 'as_str()', detail: '转 &str', kind: 'method' },
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
