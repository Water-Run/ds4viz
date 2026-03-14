# `ds4viz`

`ds4viz` 是一个 `C` 单头文件库, 提供了 `C` 语言的 `ds4viz` 支持.  
`ds4viz` 是一个可视化数据结构工具. 参见: [GitHub](https://github.com/Water-Run/ds4viz)

## 安装

`ds4viz` 以单头文件 `ds4viz.h` 提供.  
前往 [Release](https://github.com/Water-Run/ds4viz/releases/tag/lib-0.1.0) 下载 `ds4viz.h`, 将其加入项目后即可使用:

```c
#define DS4VIZ_IMPLEMENTATION
#include "ds4viz.h"
```

> `DS4VIZ_IMPLEMENTATION` 必须且只能在一个 `.c` 文件中的 `#include "ds4viz.h"` 之前定义一次, 用于生成库实现代码.

## 宏开关

### `DS4VIZ_IMPLEMENTATION`

```c
/**
 * 在当前编译单元生成库实现代码.
 * 必须且只能在一个 .c 文件中定义一次.
 * 若在多个编译单元中定义, 将导致重复符号链接错误.
 */
#define DS4VIZ_IMPLEMENTATION
```

### `DS4VIZ_SHORT_NAMES`

```c
/**
 * 启用短名称 API (dv 前缀), 提高调用可读性.
 * 例如 ds4vizStackPush -> dvStackPush
 * 若不定义该宏, 仍可使用 ds4viz 长名前缀 API.
 */
#define DS4VIZ_SHORT_NAMES
```

## 编译与测试

**编译器要求:** GCC (需要 C23 支持)

**测试命令:**

```bash
gcc -std=c2x test.c -o test && ./test
```

## 快速上手

以下示例演示如何使用 `ds4viz` 生成 `trace.toml`, 并在发生错误时仍然产出包含 `[error]` 的 `.toml` IR 文件:

```c
#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"

int main(void) {
    dvConfig((dvConfigOptions){
        .output_path = "trace.toml",
        .title = "无向图演示",
        .author = "WaterRun",
        .comment = "C quick start"
    });

    dvGraphUndirected(graph, "graph") {
        // 添加节点
        dvGuAddNode(graph, 0, "A");
        dvGuAddNode(graph, 1, "B");
        dvGuAddNode(graph, 2, "C");

        // 添加无向边 (内部会规范化为 from < to)
        dvGuAddEdge(graph, 0, 1);
        dvGuAddEdge(graph, 1, 2);

        // 也可以在这里继续调用更多 API, 每次操作都会记录到 trace 中
    }

    return 0;
}
```

### 运行语义

- 进入结构作用域时: 创建对象并写入初始状态
- 每次 API 调用: 写入一个 `step` 与对应新 `state`
- 作用域正常结束: 输出 `[result]`
- 作用域内发生错误: 后续操作静默跳过, 退出时输出 `[error]`

> **注意:** 不要在结构作用域内使用 `return`、`goto` 跳出块, 否则会跳过 `.toml` 写入及资源清理.

## API参考

### 全局配置

#### `config`

```c
typedef struct {
    const char* output_path; // 输出文件路径, NULL 则默认为 "trace.toml"
    const char* title;       // 可视化标题, NULL 则不输出
    const char* author;      // 作者信息, NULL 则不输出
    const char* comment;     // 注释说明, NULL 则不输出
} dvConfigOptions;

/**
 * 配置全局参数.
 * 未指定的字段自动零初始化 (NULL), 使用默认值.
 * 调用 dvConfig 是可选的; 若不调用, 输出路径默认为 "trace.toml".
 *
 * @param options 配置选项
 */
void dvConfig(dvConfigOptions options);
```

**示例:**

```c
#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"

int main(void) {
    dvConfig((dvConfigOptions){
        .output_path = "my_trace.toml",
        .title      = "栈操作演示",
        .author     = "WaterRun",
        .comment    = "演示基本的栈操作"
    });

    // 后续所有结构均输出到 my_trace.toml
    return 0;
}
```

---

### 线性结构

#### `stack`

```c
/**
 * 创建栈实例
 *
 * @param s 栈对象的变量名
 * @param label 栈的标签, 默认为 "stack"
 */
dvStack(s) { ... }              // label = "stack"
dvStack(s, "my_stack") { ... }  // 自定义 label
```

**栈操作:**

```c
/**
 * 压栈操作
 *
 * @param s 栈对象
 * @param value 要压入的值
 */
void dvStackPush(s, value);

/**
 * 弹栈操作
 *
 * @param s 栈对象
 * @error 当栈为空时记录错误
 */
void dvStackPop(s);

/**
 * 清空栈
 *
 * @param s 栈对象
 */
void dvStackClear(s);
```

**示例:**

```c
dvStack(s, "demo_stack") {
    dvStackPush(s, 10);
    dvStackPush(s, 20);
    dvStackPush(s, 30);

    dvStackPop(s);
    dvStackPop(s);
}
```

#### `queue`

```c
/**
 * 创建队列实例
 *
 * @param q 队列对象的变量名
 * @param label 队列的标签, 默认为 "queue"
 */
dvQueue(q) { ... }               // label = "queue"
dvQueue(q, "my_queue") { ... }   // 自定义 label
```

**队列操作:**

```c
/**
 * 入队操作
 *
 * @param q 队列对象
 * @param value 要入队的值
 */
void dvQueueEnqueue(q, value);

/**
 * 出队操作
 *
 * @param q 队列对象
 * @error 当队列为空时记录错误
 */
void dvQueueDequeue(q);

/**
 * 清空队列
 *
 * @param q 队列对象
 */
void dvQueueClear(q);
```

**示例:**

```c
dvQueue(q, "demo_queue") {
    dvQueueEnqueue(q, 10);
    dvQueueEnqueue(q, 20);
    dvQueueEnqueue(q, 30);

    dvQueueDequeue(q);
    dvQueueDequeue(q);
}
```

#### `single_linked_list`

```c
/**
 * 创建单链表实例
 *
 * @param l 单链表对象的变量名
 * @param label 单链表的标签, 默认为 "slist"
 */
dvSingleLinkedList(l) { ... }               // label = "slist"
dvSingleLinkedList(l, "my_slist") { ... }   // 自定义 label
```

**单链表操作:**

```c
/**
 * 在链表头部插入节点
 *
 * @param l 链表对象
 * @param value 要插入的值
 * @return int 新插入节点的 id
 */
int dvSlInsertHead(l, value);

/**
 * 在链表尾部插入节点
 *
 * @param l 链表对象
 * @param value 要插入的值
 * @return int 新插入节点的 id
 */
int dvSlInsertTail(l, value);

/**
 * 在指定节点后插入新节点
 *
 * @param l 链表对象
 * @param node_id 目标节点的 id
 * @param value 要插入的值
 * @return int 新插入节点的 id
 * @error 当指定节点不存在时记录错误
 */
int dvSlInsertAfter(l, node_id, value);

/**
 * 删除指定节点
 *
 * @param l 链表对象
 * @param node_id 要删除的节点 id
 * @error 当指定节点不存在时记录错误
 */
void dvSlDelete(l, node_id);

/**
 * 删除头节点
 *
 * @param l 链表对象
 * @error 当链表为空时记录错误
 */
void dvSlDeleteHead(l);

/**
 * 反转链表
 *
 * @param l 链表对象
 */
void dvSlReverse(l);
```

**示例:**

```c
dvSingleLinkedList(slist, "demo_slist") {
    int node_a = dvSlInsertHead(slist, 10);
    dvSlInsertTail(slist, 20);
    int node_c = dvSlInsertTail(slist, 30);

    dvSlInsertAfter(slist, node_a, 15);

    dvSlDelete(slist, node_c);
    dvSlReverse(slist);
}
```

#### `double_linked_list`

```c
/**
 * 创建双向链表实例
 *
 * @param l 双向链表对象的变量名
 * @param label 双向链表的标签, 默认为 "dlist"
 */
dvDoubleLinkedList(l) { ... }               // label = "dlist"
dvDoubleLinkedList(l, "my_dlist") { ... }   // 自定义 label
```

**双向链表操作:**

```c
/**
 * 在链表头部插入节点
 *
 * @param l 链表对象
 * @param value 要插入的值
 * @return int 新插入节点的 id
 */
int dvDlInsertHead(l, value);

/**
 * 在链表尾部插入节点
 *
 * @param l 链表对象
 * @param value 要插入的值
 * @return int 新插入节点的 id
 */
int dvDlInsertTail(l, value);

/**
 * 在指定节点前插入新节点
 *
 * @param l 链表对象
 * @param node_id 目标节点的 id
 * @param value 要插入的值
 * @return int 新插入节点的 id
 * @error 当指定节点不存在时记录错误
 */
int dvDlInsertBefore(l, node_id, value);

/**
 * 在指定节点后插入新节点
 *
 * @param l 链表对象
 * @param node_id 目标节点的 id
 * @param value 要插入的值
 * @return int 新插入节点的 id
 * @error 当指定节点不存在时记录错误
 */
int dvDlInsertAfter(l, node_id, value);

/**
 * 删除指定节点
 *
 * @param l 链表对象
 * @param node_id 要删除的节点 id
 * @error 当指定节点不存在时记录错误
 */
void dvDlDelete(l, node_id);

/**
 * 删除头节点
 *
 * @param l 链表对象
 * @error 当链表为空时记录错误
 */
void dvDlDeleteHead(l);

/**
 * 删除尾节点
 *
 * @param l 链表对象
 * @error 当链表为空时记录错误
 */
void dvDlDeleteTail(l);

/**
 * 反转链表
 *
 * @param l 链表对象
 */
void dvDlReverse(l);
```

**示例:**

```c
dvDoubleLinkedList(dlist, "demo_dlist") {
    int node_a = dvDlInsertHead(dlist, 10);
    dvDlInsertTail(dlist, 30);

    dvDlInsertAfter(dlist, node_a, 20);

    dvDlDeleteTail(dlist);
    dvDlReverse(dlist);
}
```

---

### 树结构

#### `binary_tree`

```c
/**
 * 创建二叉树实例
 *
 * @param t 二叉树对象的变量名
 * @param label 二叉树的标签, 默认为 "binary_tree"
 */
dvBinaryTree(t) { ... }                // label = "binary_tree"
dvBinaryTree(t, "my_tree") { ... }     // 自定义 label
```

**二叉树操作:**

```c
/**
 * 插入根节点
 *
 * @param t 树对象
 * @param value 根节点的值
 * @return int 根节点的 id
 * @error 当根节点已存在时记录错误
 */
int dvBtInsertRoot(t, value);

/**
 * 在指定父节点的左侧插入子节点
 *
 * @param t 树对象
 * @param parent_id 父节点的 id
 * @param value 要插入的子节点值
 * @return int 新插入节点的 id
 * @error 当父节点不存在或左子节点已存在时记录错误
 */
int dvBtInsertLeft(t, parent_id, value);

/**
 * 在指定父节点的右侧插入子节点
 *
 * @param t 树对象
 * @param parent_id 父节点的 id
 * @param value 要插入的子节点值
 * @return int 新插入节点的 id
 * @error 当父节点不存在或右子节点已存在时记录错误
 */
int dvBtInsertRight(t, parent_id, value);

/**
 * 删除指定节点及其所有子树
 *
 * @param t 树对象
 * @param node_id 要删除的节点 id
 * @error 当节点不存在时记录错误
 */
void dvBtDelete(t, node_id);

/**
 * 更新节点的值
 *
 * @param t 树对象
 * @param node_id 节点 id
 * @param value 新的值
 * @error 当节点不存在时记录错误
 */
void dvBtUpdateValue(t, node_id, value);
```

**示例:**

```c
dvBinaryTree(tree, "demo_tree") {
    int root  = dvBtInsertRoot(tree, 10);
    int left  = dvBtInsertLeft(tree, root, 5);
    int right = dvBtInsertRight(tree, root, 15);

    dvBtInsertLeft(tree, left, 3);
    dvBtInsertRight(tree, left, 7);

    dvBtUpdateValue(tree, right, 20);
    dvBtDelete(tree, left);
}
```

#### `binary_search_tree`

```c
/**
 * 创建二叉搜索树实例
 *
 * @param b 二叉搜索树对象的变量名
 * @param label 二叉搜索树的标签, 默认为 "bst"
 */
dvBinarySearchTree(b) { ... }            // label = "bst"
dvBinarySearchTree(b, "my_bst") { ... }  // 自定义 label
```

**二叉搜索树操作:**

```c
/**
 * 插入节点, 自动维护二叉搜索树性质
 *
 * @param b BST 对象
 * @param value 要插入的值 (仅数值: int | double)
 * @return int 新插入节点的 id
 */
int dvBstInsert(b, value);

/**
 * 删除节点, 自动维护二叉搜索树性质
 *
 * @param b BST 对象
 * @param value 要删除的节点值 (仅数值: int | double)
 * @error 当节点不存在时记录错误
 */
void dvBstDelete(b, value);
```

**示例:**

```c
dvBinarySearchTree(bst, "demo_bst") {
    dvBstInsert(bst, 10);
    dvBstInsert(bst, 5);
    dvBstInsert(bst, 15);
    dvBstInsert(bst, 3);
    dvBstInsert(bst, 7);

    dvBstDelete(bst, 5);
    dvBstInsert(bst, 6);
}
```

#### `heap`

```c
typedef enum {
    dvHeapOrderMin = 0,  // 最小堆
    dvHeapOrderMax = 1   // 最大堆
} dvHeapOrder;

/**
 * 创建堆实例
 *
 * @param h 堆对象的变量名
 * @param label 堆的标签, 默认为 "heap"
 * @param order 堆类型, 默认为 dvHeapOrderMin
 */
dvHeap(h) { ... }                                 // label = "heap", min heap
dvHeap(h, "my_heap") { ... }                      // min heap
dvHeap(h, "my_min_heap", dvHeapOrderMin) { ... }  // 显式最小堆
dvHeap(h, "my_max_heap", dvHeapOrderMax) { ... }  // 最大堆
```

**堆操作:**

```c
/**
 * 插入元素, 自动维护堆性质
 *
 * @param h 堆对象
 * @param value 要插入的值 (仅数值: int | double)
 */
void dvHeapInsert(h, value);

/**
 * 提取堆顶元素, 自动维护堆性质
 *
 * @param h 堆对象
 * @error 当堆为空时记录错误
 */
void dvHeapExtract(h);

/**
 * 清空堆
 *
 * @param h 堆对象
 */
void dvHeapClear(h);
```

**示例:**

```c
// 最小堆
dvHeap(min_heap, "demo_min_heap", dvHeapOrderMin) {
    dvHeapInsert(min_heap, 10);
    dvHeapInsert(min_heap, 5);
    dvHeapInsert(min_heap, 15);
    dvHeapInsert(min_heap, 3);

    dvHeapExtract(min_heap);  // 提取最小值 3
}

// 最大堆
dvHeap(max_heap, "demo_max_heap", dvHeapOrderMax) {
    dvHeapInsert(max_heap, 10);
    dvHeapInsert(max_heap, 5);
    dvHeapInsert(max_heap, 15);
    dvHeapInsert(max_heap, 3);

    dvHeapExtract(max_heap);  // 提取最大值 15
}
```

---

### 图结构

#### `graph_undirected`

```c
/**
 * 创建无向图实例
 *
 * @param g 无向图对象的变量名
 * @param label 无向图的标签, 默认为 "graph"
 */
dvGraphUndirected(g) { ... }               // label = "graph"
dvGraphUndirected(g, "my_graph") { ... }   // 自定义 label
```

**无向图操作:**

```c
/**
 * 添加节点
 *
 * @param g 图对象
 * @param node_id 节点 id
 * @param label 节点标签, 长度限制为 1-32 字符
 * @error 当节点已存在时记录错误
 */
void dvGuAddNode(g, node_id, label);

/**
 * 添加无向边, 内部自动规范化为 from_id < to_id
 *
 * @param g 图对象
 * @param from_id 起始节点 id
 * @param to_id 终止节点 id
 * @error 当节点不存在, 边已存在或形成自环时记录错误
 */
void dvGuAddEdge(g, from_id, to_id);

/**
 * 删除节点及其相关的所有边
 *
 * @param g 图对象
 * @param node_id 要删除的节点 id
 * @error 当节点不存在时记录错误
 */
void dvGuRemoveNode(g, node_id);

/**
 * 删除边
 *
 * @param g 图对象
 * @param from_id 起始节点 id
 * @param to_id 终止节点 id
 * @error 当边不存在时记录错误
 */
void dvGuRemoveEdge(g, from_id, to_id);

/**
 * 更新节点标签
 *
 * @param g 图对象
 * @param node_id 节点 id
 * @param label 新的节点标签, 长度限制为 1-32 字符
 * @error 当节点不存在时记录错误
 */
void dvGuUpdateNodeLabel(g, node_id, label);
```

**示例:**

```c
dvGraphUndirected(graph, "demo_graph") {
    dvGuAddNode(graph, 0, "A");
    dvGuAddNode(graph, 1, "B");
    dvGuAddNode(graph, 2, "C");

    dvGuAddEdge(graph, 0, 1);
    dvGuAddEdge(graph, 1, 2);

    dvGuUpdateNodeLabel(graph, 1, "B_updated");
    dvGuRemoveEdge(graph, 0, 1);
}
```

#### `graph_directed`

```c
/**
 * 创建有向图实例
 *
 * @param g 有向图对象的变量名
 * @param label 有向图的标签, 默认为 "graph"
 */
dvGraphDirected(g) { ... }               // label = "graph"
dvGraphDirected(g, "my_graph") { ... }   // 自定义 label
```

**有向图操作:**

```c
/**
 * 添加节点
 *
 * @param g 图对象
 * @param node_id 节点 id
 * @param label 节点标签, 长度限制为 1-32 字符
 * @error 当节点已存在时记录错误
 */
void dvGdAddNode(g, node_id, label);

/**
 * 添加有向边
 *
 * @param g 图对象
 * @param from_id 起始节点 id
 * @param to_id 终止节点 id
 * @error 当节点不存在, 边已存在或形成自环时记录错误
 */
void dvGdAddEdge(g, from_id, to_id);

/**
 * 删除节点及其相关的所有边
 *
 * @param g 图对象
 * @param node_id 要删除的节点 id
 * @error 当节点不存在时记录错误
 */
void dvGdRemoveNode(g, node_id);

/**
 * 删除边
 *
 * @param g 图对象
 * @param from_id 起始节点 id
 * @param to_id 终止节点 id
 * @error 当边不存在时记录错误
 */
void dvGdRemoveEdge(g, from_id, to_id);

/**
 * 更新节点标签
 *
 * @param g 图对象
 * @param node_id 节点 id
 * @param label 新的节点标签, 长度限制为 1-32 字符
 * @error 当节点不存在时记录错误
 */
void dvGdUpdateNodeLabel(g, node_id, label);
```

**示例:**

```c
dvGraphDirected(graph, "demo_digraph") {
    dvGdAddNode(graph, 0, "A");
    dvGdAddNode(graph, 1, "B");
    dvGdAddNode(graph, 2, "C");

    dvGdAddEdge(graph, 0, 1);
    dvGdAddEdge(graph, 1, 2);
    dvGdAddEdge(graph, 2, 0);

    dvGdUpdateNodeLabel(graph, 1, "B_updated");
    dvGdRemoveEdge(graph, 2, 0);
}
```

#### `graph_weighted`

```c
/**
 * 创建带权图实例
 *
 * @param g 带权图对象的变量名
 * @param label 带权图的标签, 默认为 "graph"
 * @param directed 是否为有向图, 默认为 false (无向图)
 */
dvGraphWeighted(g) { ... }                      // label = "graph", directed = false
dvGraphWeighted(g, "my_graph") { ... }          // directed = false
dvGraphWeighted(g, "my_graph", false) { ... }   // 显式无向
dvGraphWeighted(g, "my_graph", true) { ... }    // 有向
```

**带权图操作:**

```c
/**
 * 添加节点
 *
 * @param g 图对象
 * @param node_id 节点 id
 * @param label 节点标签, 长度限制为 1-32 字符
 * @error 当节点已存在时记录错误
 */
void dvGwAddNode(g, node_id, label);

/**
 * 添加带权边
 *
 * @param g 图对象
 * @param from_id 起始节点 id
 * @param to_id 终止节点 id
 * @param weight 边的权重 (double)
 * @error 当节点不存在, 边已存在或形成自环时记录错误
 */
void dvGwAddEdge(g, from_id, to_id, weight);

/**
 * 删除节点及其相关的所有边
 *
 * @param g 图对象
 * @param node_id 要删除的节点 id
 * @error 当节点不存在时记录错误
 */
void dvGwRemoveNode(g, node_id);

/**
 * 删除边
 *
 * @param g 图对象
 * @param from_id 起始节点 id
 * @param to_id 终止节点 id
 * @error 当边不存在时记录错误
 */
void dvGwRemoveEdge(g, from_id, to_id);

/**
 * 更新边的权重
 *
 * @param g 图对象
 * @param from_id 起始节点 id
 * @param to_id 终止节点 id
 * @param weight 新的权重值 (double)
 * @error 当边不存在时记录错误
 */
void dvGwUpdateWeight(g, from_id, to_id, weight);

/**
 * 更新节点标签
 *
 * @param g 图对象
 * @param node_id 节点 id
 * @param label 新的节点标签, 长度限制为 1-32 字符
 * @error 当节点不存在时记录错误
 */
void dvGwUpdateNodeLabel(g, node_id, label);
```

**示例:**

```c
// 无向带权图
dvGraphWeighted(graph, "demo_weighted_graph") {
    dvGwAddNode(graph, 0, "A");
    dvGwAddNode(graph, 1, "B");
    dvGwAddNode(graph, 2, "C");

    dvGwAddEdge(graph, 0, 1, 3.5);
    dvGwAddEdge(graph, 1, 2, 2.0);
    dvGwAddEdge(graph, 2, 0, 4.2);

    dvGwUpdateWeight(graph, 0, 1, 5.0);
    dvGwRemoveEdge(graph, 2, 0);
}

// 有向带权图
dvGraphWeighted(graph2, "demo_directed_weighted", true) {
    dvGwAddNode(graph2, 0, "Start");
    dvGwAddNode(graph2, 1, "Middle");
    dvGwAddNode(graph2, 2, "End");

    dvGwAddEdge(graph2, 0, 1, 1.5);
    dvGwAddEdge(graph2, 1, 2, 2.5);
}
```

---

### 错误处理

所有数据结构在遇到非法操作时会记录错误信息. 错误发生后, 同一作用域内后续的所有操作将被静默跳过, 返回 ID 的操作将返回 -1. 作用域结束时, `.toml` 文件将包含 `[error]` 部分而非 `[result]` 部分.

**示例:**

```c
dvBinaryTree(tree) {
    int root = dvBtInsertRoot(tree, 10);
    dvBtInsertLeft(tree, 999, 5);  // 父节点不存在, 记录错误
    // 后续操作静默跳过
    dvBtInsertRight(tree, root, 15);  // 不执行 (返回 -1)
}
// 即使发生错误, trace.toml 仍会生成, 包含 [error] 部分
```

生成的 `trace.toml` 将包含:

```toml
[error]
type = "runtime"
message = "Parent node not found: 999"
line = 4
step = 1
last_state = 1
```

---

### 类型约束

根据 IR 定义, `value` 字段仅支持以下 C 类型:

- `int` (及其他整型, 如 `long`, `short`): 序列化为 TOML 整数
- `float` / `double`: 序列化为 TOML 浮点数
- `const char*`: 序列化为 TOML 字符串
- `bool` (`true` / `false`): 序列化为 TOML 布尔值

值类型通过 `_Generic` 自动推断, 无需手动指定.

> **约束:** `binary_search_tree` 和 `heap` 的 `value` 仅接受数值类型 (`int` / `double`), 不接受字符串和布尔值.

---

## 长名与短名映射

启用 `DS4VIZ_SHORT_NAMES` 时使用 `dv...` 前缀; 否则使用 `ds4viz...` 前缀. 以下为完整映射表:

**类型与枚举:**

| 长名                  | 短名              |
|-----------------------|-------------------|
| `ds4vizConfigOptions` | `dvConfigOptions` |
| `ds4vizHeapOrder`     | `dvHeapOrder`     |
| `ds4vizHeapOrderMin`  | `dvHeapOrderMin`  |
| `ds4vizHeapOrderMax`  | `dvHeapOrderMax`  |

**全局配置:**

| 长名           | 短名       |
|----------------|------------|
| `ds4vizConfig` | `dvConfig` |

**栈:**

| 长名               | 短名           |
|--------------------|----------------|
| `ds4vizStack`      | `dvStack`      |
| `ds4vizStackPush`  | `dvStackPush`  |
| `ds4vizStackPop`   | `dvStackPop`   |
| `ds4vizStackClear` | `dvStackClear` |

**队列:**

| 长名                 | 短名             |
|----------------------|------------------|
| `ds4vizQueue`        | `dvQueue`        |
| `ds4vizQueueEnqueue` | `dvQueueEnqueue` |
| `ds4vizQueueDequeue` | `dvQueueDequeue` |
| `ds4vizQueueClear`   | `dvQueueClear`   |

**单链表:**

| 长名                     | 短名                 |
|--------------------------|----------------------|
| `ds4vizSingleLinkedList` | `dvSingleLinkedList` |
| `ds4vizSlInsertHead`     | `dvSlInsertHead`     |
| `ds4vizSlInsertTail`     | `dvSlInsertTail`     |
| `ds4vizSlInsertAfter`    | `dvSlInsertAfter`    |
| `ds4vizSlDelete`         | `dvSlDelete`         |
| `ds4vizSlDeleteHead`     | `dvSlDeleteHead`     |
| `ds4vizSlReverse`        | `dvSlReverse`        |

**双向链表:**

| 长名                     | 短名                 |
|--------------------------|----------------------|
| `ds4vizDoubleLinkedList` | `dvDoubleLinkedList` |
| `ds4vizDlInsertHead`     | `dvDlInsertHead`     |
| `ds4vizDlInsertTail`     | `dvDlInsertTail`     |
| `ds4vizDlInsertBefore`   | `dvDlInsertBefore`   |
| `ds4vizDlInsertAfter`    | `dvDlInsertAfter`    |
| `ds4vizDlDelete`         | `dvDlDelete`         |
| `ds4vizDlDeleteHead`     | `dvDlDeleteHead`     |
| `ds4vizDlDeleteTail`     | `dvDlDeleteTail`     |
| `ds4vizDlReverse`        | `dvDlReverse`        |

**二叉树:**

| 长名                  | 短名              |
|-----------------------|-------------------|
| `ds4vizBinaryTree`    | `dvBinaryTree`    |
| `ds4vizBtInsertRoot`  | `dvBtInsertRoot`  |
| `ds4vizBtInsertLeft`  | `dvBtInsertLeft`  |
| `ds4vizBtInsertRight` | `dvBtInsertRight` |
| `ds4vizBtDelete`      | `dvBtDelete`      |
| `ds4vizBtUpdateValue` | `dvBtUpdateValue` |

**二叉搜索树:**

| 长名                     | 短名                 |
|--------------------------|----------------------|
| `ds4vizBinarySearchTree` | `dvBinarySearchTree` |
| `ds4vizBstInsert`        | `dvBstInsert`        |
| `ds4vizBstDelete`        | `dvBstDelete`        |

**堆:**

| 长名                | 短名            |
|---------------------|-----------------|
| `ds4vizHeap`        | `dvHeap`        |
| `ds4vizHeapInsert`  | `dvHeapInsert`  |
| `ds4vizHeapExtract` | `dvHeapExtract` |
| `ds4vizHeapClear`   | `dvHeapClear`   |

**无向图:**

| 长名                      | 短名                  |
|---------------------------|-----------------------|
| `ds4vizGraphUndirected`   | `dvGraphUndirected`   |
| `ds4vizGuAddNode`         | `dvGuAddNode`         |
| `ds4vizGuAddEdge`         | `dvGuAddEdge`         |
| `ds4vizGuRemoveNode`      | `dvGuRemoveNode`      |
| `ds4vizGuRemoveEdge`      | `dvGuRemoveEdge`      |
| `ds4vizGuUpdateNodeLabel` | `dvGuUpdateNodeLabel` |

**有向图:**

| 长名                      | 短名                  |
|---------------------------|-----------------------|
| `ds4vizGraphDirected`     | `dvGraphDirected`     |
| `ds4vizGdAddNode`         | `dvGdAddNode`         |
| `ds4vizGdAddEdge`         | `dvGdAddEdge`         |
| `ds4vizGdRemoveNode`      | `dvGdRemoveNode`      |
| `ds4vizGdRemoveEdge`      | `dvGdRemoveEdge`      |
| `ds4vizGdUpdateNodeLabel` | `dvGdUpdateNodeLabel` |

**带权图:**

| 长名                      | 短名                  |
|---------------------------|-----------------------|
| `ds4vizGraphWeighted`     | `dvGraphWeighted`     |
| `ds4vizGwAddNode`         | `dvGwAddNode`         |
| `ds4vizGwAddEdge`         | `dvGwAddEdge`         |
| `ds4vizGwRemoveNode`      | `dvGwRemoveNode`      |
| `ds4vizGwRemoveEdge`      | `dvGwRemoveEdge`      |
| `ds4vizGwUpdateWeight`    | `dvGwUpdateWeight`    |
| `ds4vizGwUpdateNodeLabel` | `dvGwUpdateNodeLabel` |
