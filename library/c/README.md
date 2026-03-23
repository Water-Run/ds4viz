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

以下示例演示如何构建一棵二叉树并记录中序遍历过程:

```c
#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"

int main(void) {
    dvConfig((dvConfigOptions){
        .title  = "二叉树中序遍历",
        .author = "WaterRun"
    });

    dvBinaryTree(tree) {
        int root, left, right;

        dvPhase(tree, "构建") {
            root  = dvBtInsertRoot(tree, 10);
            left  = dvBtInsertLeft(tree, root, 5);
            right = dvBtInsertRight(tree, root, 15);
        }

        dvPhase(tree, "中序遍历") {
            dvStep(tree, "访问节点 5",
                dvNode(left, DV_FOCUS));

            dvStep(tree, "访问节点 10",
                dvNode(left, DV_VISITED),
                dvNode(root, DV_FOCUS));

            dvStep(tree, "访问节点 15",
                dvNode(left, DV_VISITED),
                dvNode(root, DV_VISITED),
                dvNode(right, DV_FOCUS));
        }
    }

    return 0;
}
```

使用结构作用域确保无论成功或失败都会生成 `.toml` 文件: 正常退出时包含 `[result]`, 发生错误时包含 `[error]`.

### 运行语义

- 进入结构作用域时: 创建对象并写入初始状态
- 每次 API 调用: 写入一个 `step` 与对应新 `state`
- 作用域正常结束: 输出 `[result]`
- 作用域内发生错误: 后续操作静默跳过, 退出时输出 `[error]`

> **注意:** 不要在结构作用域内使用 `return`、`goto` 跳出块, 否则会跳过 `.toml` 写入及资源清理.

---

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
        .title       = "栈操作演示",
        .author      = "WaterRun",
        .comment     = "演示基本的栈操作"
    });

    // 后续所有结构均输出到 my_trace.toml
    return 0;
}
```

---

### 值类型

#### `dvValue`

```c
/**
 * 带标签联合体, 用于 dvStackPop / dvQueueDequeue / dvHeapExtract 的返回值.
 * tag 字段标识当前存储的类型, 通过 dvInt / dvFloat / dvStr / dvBool 提取值.
 */
typedef struct dvValue {
    enum { DV_INT, DV_FLOAT, DV_STR, DV_BOOL } tag;
    union {
        int64_t     i;
        double      f;
        const char* s;
        bool        b;
    };
} dvValue;
```

#### `dvInt` / `dvFloat` / `dvStr` / `dvBool`

```c
/**
 * 从 dvValue 中提取 int64_t 值.
 * debug 模式下若 tag 不匹配则触发 assert.
 *
 * @param v dvValue 值
 * @return int64_t 存储的整数值
 */
static inline int64_t dvInt(dvValue v);

/**
 * 从 dvValue 中提取 double 值.
 *
 * @param v dvValue 值
 * @return double 存储的浮点值
 */
static inline double dvFloat(dvValue v);

/**
 * 从 dvValue 中提取 const char* 值.
 * 返回的字符串指针生命周期与所属结构作用域相同, 作用域退出后失效.
 *
 * @param v dvValue 值
 * @return const char* 存储的字符串指针
 */
static inline const char* dvStr(dvValue v);

/**
 * 从 dvValue 中提取 bool 值.
 *
 * @param v dvValue 值
 * @return bool 存储的布尔值
 */
static inline bool dvBool(dvValue v);
```

**示例:**

```c
dvStack(s) {
    dvStackPush(s, 42);
    dvStackPush(s, "hello");

    dvValue v1 = dvStackPop(s);    // v1.tag == DV_STR
    const char* str = dvStr(v1);   // "hello"

    dvValue v2 = dvStackPop(s);    // v2.tag == DV_INT
    int64_t num = dvInt(v2);       // 42
}
```

---

### 高亮标记辅助函数

用于构造高亮标记, 供 `dvStep()` 和 `dvAmend()` / `dvAmendHL()` 使用.

#### `dvNode()`

```c
/**
 * 创建节点高亮标记.
 * 适用于含 nodes 的结构: slist, dlist, binary_tree, bst,
 * graph_undirected, graph_directed, graph_weighted.
 *
 * @param target 节点 id, 必须引用关联 state 中存在的节点
 * @param style 高亮样式, 仅允许小写字母/数字/下划线/连字符, 长度 1..32; 默认 "focus"
 * @param level 视觉强度等级, 1..9, 默认 1, 数值越大越醒目
 */
dvNode(target)                   // style = "focus", level = 1
dvNode(target, style)            // level = 1
dvNode(target, style, level)
```

#### `dvItem()`

```c
/**
 * 创建元素高亮标记.
 * 仅适用于 stack 和 queue.
 *
 * @param target 元素在 items 数组中的有效索引
 * @param style 高亮样式, 仅允许小写字母/数字/下划线/连字符, 长度 1..32; 默认 "focus"
 * @param level 视觉强度等级, 1..9, 默认 1, 数值越大越醒目
 */
dvItem(target)                   // style = "focus", level = 1
dvItem(target, style)            // level = 1
dvItem(target, style, level)
```

#### `dvEdge()`

```c
/**
 * 创建边高亮标记.
 * 仅适用于图结构: graph_undirected, graph_directed, graph_weighted.
 * (from_id, to_id) 必须匹配关联 state 中存在的一条边.
 * 无向图时必须满足 from_id < to_id.
 *
 * @param from_id 起始节点 id
 * @param to_id 终止节点 id
 * @param style 高亮样式, 仅允许小写字母/数字/下划线/连字符, 长度 1..32; 默认 "focus"
 * @param level 视觉强度等级, 1..9, 默认 1, 数值越大越醒目
 */
dvEdge(from_id, to_id)                  // style = "focus", level = 1
dvEdge(from_id, to_id, style)           // level = 1
dvEdge(from_id, to_id, style, level)
```

#### 预定义 `style` 值

渲染器必须支持以下预定义样式. 自定义 `style` 值会回退为 `"focus"` 的表现.

```c
#define DV_FOCUS     "focus"      // 当前焦点
#define DV_VISITED   "visited"    // 已访问
#define DV_ACTIVE    "active"     // 参与当前计算
#define DV_COMPARING "comparing"  // 正在比较
#define DV_FOUND     "found"      // 查找命中
#define DV_ERROR_STY "error"      // 异常状态
```

直接传字符串字面量 `"focus"` 同样合法, 常量仅用于防止拼写错误.

---

### 通用实例方法

所有数据结构实例均支持以下方法.

#### `dvStep()`

```c
/**
 * 记录一次观察步骤, 不改变数据结构的状态.
 * 在 IR 中生成 op = "observe"、before == after 的 step.
 * 适用于遍历、查找等需要逐步展示但不修改结构的场景.
 *
 * @param obj 数据结构对象
 * @param note 步骤说明, 长度 1..256; 传入 NULL 表示不设置
 * @param ... 高亮标记, 由 dvNode() / dvItem() / dvEdge() 构造
 */
dvStep(obj);                              // 无 note, 无 highlights
dvStep(obj, note);                        // 有 note, 无 highlights
dvStep(obj, note, ...);                   // 有 note + highlights (note 可为 NULL)
```

**示例:**

```c
dvBinaryTree(tree) {
    int root = dvBtInsertRoot(tree, 10);
    int left = dvBtInsertLeft(tree, root, 5);

    dvStep(tree);
    dvStep(tree, "访问根节点", dvNode(root, DV_FOCUS, 3));
    dvStep(tree, "访问左子节点",
        dvNode(root, DV_VISITED),
        dvNode(left, DV_FOCUS, 3));
}
```

#### `dvAmend()` / `dvAmendHL()` / `dvAmendClearHL()`

```c
/**
 * 修改上一步的 note, 可同时设置 highlights.
 * 传入 highlights 时替换上一步的全部高亮; 不传入时保持不变.
 *
 * @param obj 数据结构对象
 * @param note 步骤说明, 长度 1..256
 * @param ... 高亮标记 (可选), 由 dvNode() / dvItem() / dvEdge() 构造
 * @error 当不存在上一步时记录错误
 */
dvAmend(obj, note);                       // 设置 note, highlights 不变
dvAmend(obj, note, ...);                  // 设置 note + highlights

/**
 * 仅修改上一步的 highlights, note 保持不变.
 *
 * @param obj 数据结构对象
 * @param ... 高亮标记, 由 dvNode() / dvItem() / dvEdge() 构造
 * @error 当不存在上一步时记录错误
 */
dvAmendHL(obj, ...);

/**
 * 清除上一步的所有高亮标记, note 保持不变.
 *
 * @param obj 数据结构对象
 * @error 当不存在上一步时记录错误
 */
void dvAmendClearHL(obj);
```

**示例:**

```c
dvStack(s) {
    dvStackPush(s, 10);
    dvAmend(s, "压入 10", dvItem(0, DV_FOCUS, 3));

    dvStackPush(s, 20);
    dvAmendHL(s,
        dvItem(0, DV_VISITED),
        dvItem(1, DV_FOCUS, 3));
    dvAmend(s, "压入 20");

    dvStackPop(s);
    dvAmendClearHL(s);
}
```

#### `dvPhase()`

```c
/**
 * 阶段作用域块.
 * 块内产生的所有步骤 (包括变更操作自动产生的步骤和手动调用 dvStep() 产生的步骤)
 * 都会标记为指定阶段 (对应 IR steps.phase).
 * 支持嵌套, 内层 phase 优先.
 *
 * phase 不是必须的. 不使用 phase 时, 步骤不会携带阶段标记.
 * 当操作流程存在明确的逻辑划分时, 推荐使用 phase 以便渲染器分段展示.
 *
 * @param obj 数据结构对象
 * @param name 阶段名称, 长度 1..64
 */
dvPhase(obj, name) { ... }
```

**示例:**

```c
dvBinarySearchTree(bst) {
    dvPhase(bst, "插入") {
        dvBstInsert(bst, 10);
        dvBstInsert(bst, 5);
        dvBstInsert(bst, 15);
    }

    dvPhase(bst, "删除") {
        dvBstDelete(bst, 5);
    }
}
```

#### `dvAlias()` (仅含 `nodes` 的结构)

适用于: `SingleLinkedList`, `DoubleLinkedList`, `BinaryTree`, `BinarySearchTree`, `Heap`, `GraphUndirected`, `GraphDirected`, `GraphWeighted`.

```c
/**
 * 设置或清除节点别名.
 * 别名会出现在后续的状态快照中, 供渲染器展示.
 * 不产生新的步骤.
 *
 * @param obj 数据结构对象
 * @param node_id 节点 id
 * @param name 别名, 传入 NULL 表示清除;
 *             长度 1..64, 仅允许: a-z, A-Z, 0-9, _, -; 同一 state 内唯一
 * @error 当节点不存在时记录错误
 */
void dvAlias(obj, node_id, name);
```

**示例:**

```c
dvBinaryTree(tree) {
    int root = dvBtInsertRoot(tree, 10);
    dvAlias(tree, root, "root");

    int left = dvBtInsertLeft(tree, root, 5);
    dvAlias(tree, left, "L");

    dvAlias(tree, left, NULL);   // 清除别名
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
 * @return dvValue 弹出的栈顶元素值
 * @error 当栈为空时记录错误
 */
dvValue dvStackPop(s);

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
    dvAmend(s, "压入 10", dvItem(0, DV_FOCUS, 3));

    dvStackPush(s, 20);
    dvAmend(s, "压入 20",
        dvItem(0, DV_VISITED),
        dvItem(1, DV_FOCUS, 3));

    dvStackPush(s, 30);

    dvValue val = dvStackPop(s);   // val.tag == DV_INT, dvInt(val) == 30
    dvAmend(s, "弹出 30");

    val = dvStackPop(s);           // dvInt(val) == 20
    dvAmend(s, "弹出 20", dvItem(0, DV_FOCUS, 3));
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
 * @return dvValue 出队的队首元素值
 * @error 当队列为空时记录错误
 */
dvValue dvQueueDequeue(q);

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
    dvPhase(q, "入队") {
        dvQueueEnqueue(q, "甲");
        dvQueueEnqueue(q, "乙");
        dvQueueEnqueue(q, "丙");
    }

    dvPhase(q, "处理") {
        dvValue val = dvQueueDequeue(q);   // dvStr(val) == "甲"
        dvAmend(q, "处理 甲", dvItem(0, DV_FOCUS, 3));

        val = dvQueueDequeue(q);           // dvStr(val) == "乙"
        dvAmend(q, "处理 乙", dvItem(0, DV_FOCUS, 3));

        dvQueueEnqueue(q, "丁");
        dvAmend(q, "新任务入队",
            dvItem(0, DV_VISITED),
            dvItem(1, DV_FOCUS, 3));
    }
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
    int a = dvSlInsertHead(slist, 10);
    dvAlias(slist, a, "head");
    int b = dvSlInsertTail(slist, 20);
    int c = dvSlInsertTail(slist, 30);
    dvAmendHL(slist, dvNode(c, DV_FOCUS, 3));

    int d = dvSlInsertAfter(slist, a, 15);
    dvAmend(slist, "在 10 后插入 15",
        dvNode(a, DV_ACTIVE),
        dvNode(d, DV_FOCUS, 3));

    dvSlDelete(slist, c);
    dvAmend(slist, "删除节点 30");

    dvSlReverse(slist);
    dvAmend(slist, "反转链表");
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
    dvPhase(dlist, "构建") {
        int a = dvDlInsertHead(dlist, 10);
        dvAlias(dlist, a, "head");
        int b = dvDlInsertTail(dlist, 30);
        int c = dvDlInsertBefore(dlist, b, 20);
        dvAmend(dlist, "在 30 前插入 20",
            dvNode(c, DV_FOCUS, 3),
            dvNode(b, DV_ACTIVE));
    }

    dvPhase(dlist, "修改") {
        dvDlInsertAfter(dlist, a, 15);
        dvDlDeleteTail(dlist);
        dvDlReverse(dlist);
    }
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
    dvAlias(tree, root, "root");
    int left  = dvBtInsertLeft(tree, root, 5);
    int right = dvBtInsertRight(tree, root, 15);
    dvBtInsertLeft(tree, left, 3);
    dvBtInsertRight(tree, left, 7);

    dvBtUpdateValue(tree, right, 20);
    dvAmend(tree, "将 15 更新为 20",
        dvNode(right, DV_FOCUS, 3));

    dvBtDelete(tree, left);
    dvAmend(tree, "删除节点 5 及其子树",
        dvNode(root, DV_ACTIVE));
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
    dvPhase(bst, "插入") {
        int n10 = dvBstInsert(bst, 10);
        dvAlias(bst, n10, "root");
        int n5  = dvBstInsert(bst, 5);
        dvBstInsert(bst, 15);
        dvBstInsert(bst, 3);
        dvBstInsert(bst, 7);
    }

    dvPhase(bst, "删除") {
        dvBstDelete(bst, 5);
        dvAmend(bst, "删除 5, 用中序后继替代");

        int n6 = dvBstInsert(bst, 6);
        dvAmend(bst, "插入 6",
            dvNode(n6, DV_FOCUS, 3));
    }
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
 * @return dvValue 提取的堆顶元素值
 * @error 当堆为空时记录错误
 */
dvValue dvHeapExtract(h);

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

    dvValue val = dvHeapExtract(min_heap);  // dvInt(val) == 3
    dvAmend(min_heap, "提取最小值 3");
}

// 最大堆
dvHeap(max_heap, "demo_max_heap", dvHeapOrderMax) {
    dvHeapInsert(max_heap, 10);
    dvHeapInsert(max_heap, 5);
    dvHeapInsert(max_heap, 15);
    dvHeapInsert(max_heap, 3);

    dvValue val = dvHeapExtract(max_heap);  // dvInt(val) == 15
    dvAmend(max_heap, "提取最大值 15");
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
    dvGuAddNode(graph, 0, "甲");
    dvGuAddNode(graph, 1, "乙");
    dvGuAddNode(graph, 2, "丙");
    dvGuAddNode(graph, 3, "丁");

    dvGuAddEdge(graph, 0, 1);
    dvGuAddEdge(graph, 0, 2);
    dvGuAddEdge(graph, 1, 3);
    dvGuAddEdge(graph, 2, 3);

    dvGuUpdateNodeLabel(graph, 1, "乙*");
    dvGuRemoveEdge(graph, 0, 1);
    dvGuRemoveNode(graph, 3);
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
    dvPhase(graph, "构建") {
        dvGdAddNode(graph, 0, "甲");
        dvGdAddNode(graph, 1, "乙");
        dvGdAddNode(graph, 2, "丙");

        dvGdAddEdge(graph, 0, 1);
        dvGdAddEdge(graph, 1, 2);
        dvGdAddEdge(graph, 2, 0);
    }

    dvPhase(graph, "修改") {
        dvGdRemoveEdge(graph, 2, 0);
        dvAmend(graph, "断开 丙→甲 的环",
            dvNode(0, DV_ACTIVE),
            dvNode(2, DV_ACTIVE));
    }
}
```

#### `graph_weighted`

```c
/**
 * 创建带权有向图实例
 *
 * @param g 带权图对象的变量名
 * @param label 带权图的标签, 默认为 "graph"
 */
dvGraphWeighted(g) { ... }               // label = "graph"
dvGraphWeighted(g, "my_graph") { ... }   // 自定义 label
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
 * 添加带权有向边
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
dvGraphWeighted(graph, "demo_weighted") {
    dvGwAddNode(graph, 0, "甲");
    dvAlias(graph, 0, "source");
    dvGwAddNode(graph, 1, "乙");
    dvGwAddNode(graph, 2, "丙");

    dvGwAddEdge(graph, 0, 1, 3.5);
    dvAmendHL(graph,
        dvNode(0, DV_ACTIVE),
        dvNode(1, DV_ACTIVE),
        dvEdge(0, 1, DV_FOCUS, 3));

    dvGwAddEdge(graph, 1, 2, 2.0);
    dvGwAddEdge(graph, 0, 2, 10.0);

    dvGwUpdateWeight(graph, 0, 2, 5.5);
    dvAmend(graph, "松弛 甲→丙 的权重: 10.0 → 5.5",
        dvEdge(0, 2, DV_FOCUS, 3));
}
```

---

### 错误处理

所有数据结构在遇到非法操作时会记录错误信息. 错误发生后, 同一作用域内后续的所有操作将被静默跳过, 返回 ID 的操作将返回 -1, 返回 `dvValue` 的操作将返回零值. 作用域结束时, `.toml` 文件将包含 `[error]` 部分而非 `[result]` 部分.

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
| `ds4vizValue`         | `dvValue`         |
| `DS4VIZ_INT`          | `DV_INT`          |
| `DS4VIZ_FLOAT`        | `DV_FLOAT`        |
| `DS4VIZ_STR`          | `DV_STR`          |
| `DS4VIZ_BOOL`         | `DV_BOOL`         |
| `ds4vizHeapOrder`     | `dvHeapOrder`     |
| `ds4vizHeapOrderMin`  | `dvHeapOrderMin`  |
| `ds4vizHeapOrderMax`  | `dvHeapOrderMax`  |

**值提取:**

| 长名          | 短名      |
|---------------|-----------|
| `ds4vizInt`   | `dvInt`   |
| `ds4vizFloat` | `dvFloat` |
| `ds4vizStr`   | `dvStr`   |
| `ds4vizBool`  | `dvBool`  |

**预定义样式:**

| 长名               | 短名           |
|--------------------|----------------|
| `DS4VIZ_FOCUS`     | `DV_FOCUS`     |
| `DS4VIZ_VISITED`   | `DV_VISITED`   |
| `DS4VIZ_ACTIVE`    | `DV_ACTIVE`    |
| `DS4VIZ_COMPARING` | `DV_COMPARING` |
| `DS4VIZ_FOUND`     | `DV_FOUND`     |
| `DS4VIZ_ERROR_STY` | `DV_ERROR_STY` |

**高亮标记:**

| 长名         | 短名     |
|--------------|----------|
| `ds4vizNode` | `dvNode` |
| `ds4vizItem` | `dvItem` |
| `ds4vizEdge` | `dvEdge` |

**全局配置:**

| 长名           | 短名       |
|----------------|------------|
| `ds4vizConfig` | `dvConfig` |

**通用方法:**

| 长名                 | 短名             |
|----------------------|------------------|
| `ds4vizStep`         | `dvStep`         |
| `ds4vizAmend`        | `dvAmend`        |
| `ds4vizAmendHL`      | `dvAmendHL`      |
| `ds4vizAmendClearHL` | `dvAmendClearHL` |
| `ds4vizPhase`        | `dvPhase`        |
| `ds4vizAlias`        | `dvAlias`        |

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

---

## 使用示例

### 二叉树: 前序遍历

构建二叉树后, 使用 `dvStep()` 逐步记录前序遍历 (根→左→右) 的访问过程.

```c
#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"

int main(void) {
    dvConfig((dvConfigOptions){
        .title  = "二叉树前序遍历",
        .author = "WaterRun"
    });

    dvBinaryTree(tree, "前序遍历") {
        int root, n5, n15, n3, n7, n12;

        dvPhase(tree, "构建") {
            root = dvBtInsertRoot(tree, 10);
            dvAlias(tree, root, "root");
            n5   = dvBtInsertLeft(tree, root, 5);
            n15  = dvBtInsertRight(tree, root, 15);
            n3   = dvBtInsertLeft(tree, n5, 3);
            n7   = dvBtInsertRight(tree, n5, 7);
            n12  = dvBtInsertLeft(tree, n15, 12);
        }

        dvPhase(tree, "前序遍历") {
            dvStep(tree, "访问节点 10",
                dvNode(root, DV_FOCUS, 3));

            dvStep(tree, "访问节点 5",
                dvNode(root, DV_VISITED),
                dvNode(n5, DV_FOCUS, 3));

            dvStep(tree, "访问节点 3",
                dvNode(root, DV_VISITED),
                dvNode(n5, DV_VISITED),
                dvNode(n3, DV_FOCUS, 3));

            dvStep(tree, "访问节点 7",
                dvNode(root, DV_VISITED),
                dvNode(n5, DV_VISITED),
                dvNode(n3, DV_VISITED),
                dvNode(n7, DV_FOCUS, 3));

            dvStep(tree, "访问节点 15",
                dvNode(root, DV_VISITED),
                dvNode(n5, DV_VISITED),
                dvNode(n3, DV_VISITED),
                dvNode(n7, DV_VISITED),
                dvNode(n15, DV_FOCUS, 3));

            dvStep(tree, "访问节点 12",
                dvNode(root, DV_VISITED),
                dvNode(n5, DV_VISITED),
                dvNode(n3, DV_VISITED),
                dvNode(n7, DV_VISITED),
                dvNode(n15, DV_VISITED),
                dvNode(n12, DV_FOCUS, 3));
        }
    }

    return 0;
}
```

### 二叉树: 中序遍历

中序遍历 (左→根→右) 对 BST 可得到有序序列.

```c
#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"

int main(void) {
    dvConfig((dvConfigOptions){ .title = "BST 中序遍历" });

    dvBinarySearchTree(bst, "中序遍历") {
        int n8  = dvBstInsert(bst, 8);
        dvAlias(bst, n8, "root");
        int n3  = dvBstInsert(bst, 3);
        int n10 = dvBstInsert(bst, 10);
        int n1  = dvBstInsert(bst, 1);
        int n6  = dvBstInsert(bst, 6);
        int n14 = dvBstInsert(bst, 14);
        int n4  = dvBstInsert(bst, 4);
        int n7  = dvBstInsert(bst, 7);

        // 中序: 1, 3, 4, 6, 7, 8, 10, 14
        dvStep(bst, "访问 1",
            dvNode(n1, DV_FOCUS, 3));

        dvStep(bst, "访问 3",
            dvNode(n1, DV_VISITED),
            dvNode(n3, DV_FOCUS, 3));

        dvStep(bst, "访问 4",
            dvNode(n1, DV_VISITED),
            dvNode(n3, DV_VISITED),
            dvNode(n4, DV_FOCUS, 3));

        dvStep(bst, "访问 6",
            dvNode(n1, DV_VISITED),
            dvNode(n3, DV_VISITED),
            dvNode(n4, DV_VISITED),
            dvNode(n6, DV_FOCUS, 3));

        dvStep(bst, "访问 7",
            dvNode(n1, DV_VISITED),
            dvNode(n3, DV_VISITED),
            dvNode(n4, DV_VISITED),
            dvNode(n6, DV_VISITED),
            dvNode(n7, DV_FOCUS, 3));

        dvStep(bst, "访问 8",
            dvNode(n1, DV_VISITED),
            dvNode(n3, DV_VISITED),
            dvNode(n4, DV_VISITED),
            dvNode(n6, DV_VISITED),
            dvNode(n7, DV_VISITED),
            dvNode(n8, DV_FOCUS, 3));

        dvStep(bst, "访问 10",
            dvNode(n1, DV_VISITED),
            dvNode(n3, DV_VISITED),
            dvNode(n4, DV_VISITED),
            dvNode(n6, DV_VISITED),
            dvNode(n7, DV_VISITED),
            dvNode(n8, DV_VISITED),
            dvNode(n10, DV_FOCUS, 3));

        dvStep(bst, "访问 14",
            dvNode(n1, DV_VISITED),
            dvNode(n3, DV_VISITED),
            dvNode(n4, DV_VISITED),
            dvNode(n6, DV_VISITED),
            dvNode(n7, DV_VISITED),
            dvNode(n8, DV_VISITED),
            dvNode(n10, DV_VISITED),
            dvNode(n14, DV_FOCUS, 3));
    }

    return 0;
}
```

### 二叉树: 后序遍历

后序遍历 (左→右→根) 常用于释放树节点或计算子树属性.

```c
#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"

int main(void) {
    dvConfig((dvConfigOptions){ .title = "二叉树后序遍历" });

    dvBinaryTree(tree, "后序遍历") {
        int root = dvBtInsertRoot(tree, "A");
        dvAlias(tree, root, "root");
        int b = dvBtInsertLeft(tree, root, "B");
        int c = dvBtInsertRight(tree, root, "C");
        int d = dvBtInsertLeft(tree, b, "D");
        int e = dvBtInsertRight(tree, b, "E");

        // 后序: D, E, B, C, A
        dvStep(tree, "访问节点 D",
            dvNode(d, DV_FOCUS, 3));

        dvStep(tree, "访问节点 E",
            dvNode(d, DV_VISITED),
            dvNode(e, DV_FOCUS, 3));

        dvStep(tree, "访问节点 B",
            dvNode(d, DV_VISITED),
            dvNode(e, DV_VISITED),
            dvNode(b, DV_FOCUS, 3));

        dvStep(tree, "访问节点 C",
            dvNode(d, DV_VISITED),
            dvNode(e, DV_VISITED),
            dvNode(b, DV_VISITED),
            dvNode(c, DV_FOCUS, 3));

        dvStep(tree, "访问节点 A",
            dvNode(d, DV_VISITED),
            dvNode(e, DV_VISITED),
            dvNode(b, DV_VISITED),
            dvNode(c, DV_VISITED),
            dvNode(root, DV_FOCUS, 3));
    }

    return 0;
}
```

### 二叉树: 层序遍历

逐层从左到右访问所有节点.

```c
#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"

int main(void) {
    dvConfig((dvConfigOptions){ .title = "二叉树层序遍历" });

    dvBinaryTree(tree, "层序遍历") {
        int root, n2, n3, n4, n5, n6;

        dvPhase(tree, "构建") {
            root = dvBtInsertRoot(tree, 1);
            n2   = dvBtInsertLeft(tree, root, 2);
            n3   = dvBtInsertRight(tree, root, 3);
            n4   = dvBtInsertLeft(tree, n2, 4);
            n5   = dvBtInsertRight(tree, n2, 5);
            n6   = dvBtInsertLeft(tree, n3, 6);
        }

        dvPhase(tree, "层序遍历") {
            dvStep(tree, "第 1 层: [1]",
                dvNode(root, DV_FOCUS, 3));

            dvStep(tree, "第 2 层: [2, 3]",
                dvNode(root, DV_VISITED),
                dvNode(n2, DV_FOCUS, 3),
                dvNode(n3, DV_FOCUS, 3));

            dvStep(tree, "第 3 层: [4, 5, 6]",
                dvNode(root, DV_VISITED),
                dvNode(n2, DV_VISITED),
                dvNode(n3, DV_VISITED),
                dvNode(n4, DV_FOCUS, 3),
                dvNode(n5, DV_FOCUS, 3),
                dvNode(n6, DV_FOCUS, 3));
        }
    }

    return 0;
}
```

### 二叉搜索树: 查找过程

用 `dvStep()` 记录在 BST 中查找目标值的比较路径.

```c
#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"

int main(void) {
    dvConfig((dvConfigOptions){ .title = "BST 查找 7" });

    dvBinarySearchTree(bst, "BST查找") {
        int n10, n5, n15, n3, n7, n12, n20;

        dvPhase(bst, "构建") {
            n10 = dvBstInsert(bst, 10);
            dvAlias(bst, n10, "root");
            n5  = dvBstInsert(bst, 5);
            n15 = dvBstInsert(bst, 15);
            n3  = dvBstInsert(bst, 3);
            n7  = dvBstInsert(bst, 7);
            n12 = dvBstInsert(bst, 12);
            n20 = dvBstInsert(bst, 20);
        }

        dvPhase(bst, "查找") {
            dvStep(bst, "7 < 10, 向左",
                dvNode(n10, DV_COMPARING, 3));

            dvStep(bst, "7 > 5, 向右",
                dvNode(n10, DV_VISITED),
                dvNode(n5, DV_COMPARING, 3));

            dvStep(bst, "7 == 7, 找到!",
                dvNode(n10, DV_VISITED),
                dvNode(n5, DV_VISITED),
                dvNode(n7, DV_FOUND, 3));
        }
    }

    return 0;
}
```

### 栈: 括号匹配检查

演示使用栈检查括号字符串 `"(())"` 是否合法.

```c
#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"

int main(void) {
    dvConfig((dvConfigOptions){ .title = "括号匹配: (())" });

    dvStack(s, "括号匹配") {
        dvPhase(s, "扫描") {
            dvStackPush(s, "(");
            dvAmend(s, "读入 '(', 压栈",
                dvItem(0, DV_FOCUS, 3));

            dvStackPush(s, "(");
            dvAmend(s, "读入 '(', 压栈",
                dvItem(0, DV_VISITED),
                dvItem(1, DV_FOCUS, 3));

            dvValue v1 = dvStackPop(s);
            char buf1[256];
            snprintf(buf1, sizeof buf1,
                     "读入 ')', 弹出 '%s' 匹配成功", dvStr(v1));
            dvAmend(s, buf1, dvItem(0, DV_FOCUS, 3));

            dvValue v2 = dvStackPop(s);
            char buf2[256];
            snprintf(buf2, sizeof buf2,
                     "读入 ')', 弹出 '%s' 匹配成功", dvStr(v2));
            dvAmend(s, buf2);
        }

        dvStep(s, "栈为空, 括号合法");
    }

    return 0;
}
```

### 队列: 任务调度

演示简单的先进先出任务调度.

```c
#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"

int main(void) {
    dvConfig((dvConfigOptions){ .title = "任务调度" });

    dvQueue(q, "任务队列") {
        dvQueueEnqueue(q, "下载文件");
        dvQueueEnqueue(q, "解压缩");
        dvQueueEnqueue(q, "编译");

        dvValue val = dvQueueDequeue(q);
        char buf[256];
        snprintf(buf, sizeof buf, "执行: %s", dvStr(val));
        dvAmend(q, buf, dvItem(0, DV_FOCUS, 3));

        dvQueueEnqueue(q, "测试");
        dvAmend(q, "新增任务: 测试");

        val = dvQueueDequeue(q);
        snprintf(buf, sizeof buf, "执行: %s", dvStr(val));
        dvAmend(q, buf, dvItem(0, DV_FOCUS, 3));

        val = dvQueueDequeue(q);
        snprintf(buf, sizeof buf, "执行: %s", dvStr(val));
        dvAmend(q, buf, dvItem(0, DV_FOCUS, 3));

        val = dvQueueDequeue(q);
        snprintf(buf, sizeof buf, "执行: %s", dvStr(val));
        dvAmend(q, buf);

        dvStep(q, "所有任务已完成");
    }

    return 0;
}
```

### 单链表: 构建与遍历

```c
#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"

int main(void) {
    dvConfig((dvConfigOptions){ .title = "单链表遍历" });

    dvSingleLinkedList(slist, "遍历演示") {
        int a, b, c, d;

        dvPhase(slist, "构建") {
            a = dvSlInsertHead(slist, "甲");
            b = dvSlInsertTail(slist, "乙");
            c = dvSlInsertTail(slist, "丙");
            d = dvSlInsertTail(slist, "丁");
        }

        dvPhase(slist, "遍历") {
            dvStep(slist, "访问 甲",
                dvNode(a, DV_FOCUS, 3));

            dvStep(slist, "访问 乙",
                dvNode(a, DV_VISITED),
                dvNode(b, DV_FOCUS, 3));

            dvStep(slist, "访问 丙",
                dvNode(a, DV_VISITED),
                dvNode(b, DV_VISITED),
                dvNode(c, DV_FOCUS, 3));

            dvStep(slist, "访问 丁",
                dvNode(a, DV_VISITED),
                dvNode(b, DV_VISITED),
                dvNode(c, DV_VISITED),
                dvNode(d, DV_FOCUS, 3));
        }
    }

    return 0;
}
```

### 无向图: BFS 遍历

从节点 甲 出发, 广度优先遍历无向图.

```c
#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"

int main(void) {
    dvConfig((dvConfigOptions){ .title = "无向图 BFS" });

    dvGraphUndirected(g, "BFS演示") {
        dvGuAddNode(g, 0, "甲");
        dvGuAddNode(g, 1, "乙");
        dvGuAddNode(g, 2, "丙");
        dvGuAddNode(g, 3, "丁");
        dvGuAddNode(g, 4, "戊");

        dvGuAddEdge(g, 0, 1);    // 甲-乙
        dvGuAddEdge(g, 0, 2);    // 甲-丙
        dvGuAddEdge(g, 1, 3);    // 乙-丁
        dvGuAddEdge(g, 2, 3);    // 丙-丁
        dvGuAddEdge(g, 3, 4);    // 丁-戊

        // 起点 甲
        dvStep(g, "起点 甲 入队",
            dvNode(0, DV_FOCUS, 3));

        // 出队 甲, 发现邻居 乙, 丙
        dvStep(g, "出队 甲, 发现邻居 乙, 丙",
            dvNode(0, DV_VISITED),
            dvNode(1, DV_FOCUS, 2),
            dvNode(2, DV_FOCUS, 2),
            dvEdge(0, 1, DV_ACTIVE),
            dvEdge(0, 2, DV_ACTIVE));

        // 出队 乙, 发现邻居 丁
        dvStep(g, "出队 乙, 发现邻居 丁",
            dvNode(0, DV_VISITED),
            dvNode(1, DV_VISITED),
            dvNode(2, DV_ACTIVE),
            dvNode(3, DV_FOCUS, 2),
            dvEdge(1, 3, DV_ACTIVE));

        // 出队 丙, 邻居 丁 已访问
        dvStep(g, "出队 丙, 邻居 丁 已访问",
            dvNode(0, DV_VISITED),
            dvNode(1, DV_VISITED),
            dvNode(2, DV_VISITED),
            dvNode(3, DV_ACTIVE));

        // 出队 丁, 发现邻居 戊
        dvStep(g, "出队 丁, 发现邻居 戊",
            dvNode(0, DV_VISITED),
            dvNode(1, DV_VISITED),
            dvNode(2, DV_VISITED),
            dvNode(3, DV_VISITED),
            dvNode(4, DV_FOCUS, 2),
            dvEdge(3, 4, DV_ACTIVE));

        // 出队 戊, BFS 结束
        dvStep(g, "出队 戊, BFS 完成",
            dvNode(0, DV_VISITED),
            dvNode(1, DV_VISITED),
            dvNode(2, DV_VISITED),
            dvNode(3, DV_VISITED),
            dvNode(4, DV_VISITED));
    }

    return 0;
}
```

### 有向图: DFS 遍历

从节点 甲 出发, 深度优先遍历有向图.

```c
#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"

int main(void) {
    dvConfig((dvConfigOptions){ .title = "有向图 DFS" });

    dvGraphDirected(g, "DFS演示") {
        dvGdAddNode(g, 0, "甲");
        dvGdAddNode(g, 1, "乙");
        dvGdAddNode(g, 2, "丙");
        dvGdAddNode(g, 3, "丁");

        dvGdAddEdge(g, 0, 1);    // 甲→乙
        dvGdAddEdge(g, 0, 2);    // 甲→丙
        dvGdAddEdge(g, 1, 3);    // 乙→丁
        dvGdAddEdge(g, 2, 3);    // 丙→丁

        // DFS: 甲 → 乙 → 丁 → (回溯) → 丙 (丁已访问)
        dvStep(g, "进入 甲",
            dvNode(0, DV_FOCUS, 3));

        dvStep(g, "进入 乙",
            dvNode(0, DV_VISITED),
            dvNode(1, DV_FOCUS, 3));

        dvStep(g, "进入 丁",
            dvNode(0, DV_VISITED),
            dvNode(1, DV_VISITED),
            dvNode(3, DV_FOCUS, 3));

        dvStep(g, "丁 无出边, 回溯到 甲",
            dvNode(0, DV_VISITED),
            dvNode(1, DV_VISITED),
            dvNode(3, DV_VISITED),
            dvNode(0, DV_ACTIVE, 2));

        dvStep(g, "进入 丙",
            dvNode(0, DV_VISITED),
            dvNode(1, DV_VISITED),
            dvNode(3, DV_VISITED),
            dvNode(2, DV_FOCUS, 3));

        dvStep(g, "丙→丁 已访问, DFS 完成",
            dvNode(0, DV_VISITED),
            dvNode(1, DV_VISITED),
            dvNode(2, DV_VISITED),
            dvNode(3, DV_VISITED));
    }

    return 0;
}
```

### 带权图: 最短路径松弛

演示 Dijkstra 风格的边松弛过程.

```c
#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"

int main(void) {
    dvConfig((dvConfigOptions){ .title = "Dijkstra 松弛过程" });

    dvGraphWeighted(g, "最短路径") {
        dvPhase(g, "构建") {
            dvGwAddNode(g, 0, "起点");
            dvAlias(g, 0, "source");
            dvGwAddNode(g, 1, "甲");
            dvGwAddNode(g, 2, "乙");
            dvGwAddNode(g, 3, "终点");

            dvGwAddEdge(g, 0, 1, 1);      // 起点→甲: 1
            dvGwAddEdge(g, 0, 2, 4);      // 起点→乙: 4
            dvGwAddEdge(g, 1, 2, 2);      // 甲→乙: 2
            dvGwAddEdge(g, 1, 3, 6);      // 甲→终点: 6
            dvGwAddEdge(g, 2, 3, 3);      // 乙→终点: 3
        }

        dvPhase(g, "松弛") {
            // 从 起点 出发
            dvStep(g, "从 起点 松弛: dist[甲]=1, dist[乙]=4",
                dvNode(0, DV_VISITED),
                dvNode(1, DV_FOCUS, 2),
                dvNode(2, DV_FOCUS, 2),
                dvEdge(0, 1, DV_ACTIVE, 3),
                dvEdge(0, 2, DV_ACTIVE));

            // 取 甲
            dvStep(g, "从 甲 松弛: dist[乙]=4→3, dist[终点]=7",
                dvNode(0, DV_VISITED),
                dvNode(1, DV_VISITED),
                dvNode(2, DV_FOCUS, 2),
                dvNode(3, DV_FOCUS, 2),
                dvEdge(1, 2, DV_ACTIVE, 3),
                dvEdge(1, 3, DV_ACTIVE));

            // 取 乙
            dvStep(g, "从 乙 松弛: dist[终点]=7→6",
                dvNode(0, DV_VISITED),
                dvNode(1, DV_VISITED),
                dvNode(2, DV_VISITED),
                dvNode(3, DV_FOCUS, 3),
                dvEdge(2, 3, DV_ACTIVE, 3));

            // 完成
            dvStep(g, "完成, 最短路径 起点→甲→乙→终点, 距离=6",
                dvNode(0, DV_FOUND),
                dvNode(1, DV_FOUND),
                dvNode(2, DV_FOUND),
                dvNode(3, DV_FOUND),
                dvEdge(0, 1, DV_FOUND, 3),
                dvEdge(1, 2, DV_FOUND, 3),
                dvEdge(2, 3, DV_FOUND, 3));
        }
    }

    return 0;
}
```

### 错误处理示例

异常发生后, 作用域自动将已记录的状态和步骤写入 `.toml`, 并附带 `[error]`.

```c
#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"

int main(void) {
    dvConfig((dvConfigOptions){ .title = "错误处理演示" });

    dvStack(s, "错误演示") {
        dvStackPush(s, 10);
        dvStackPush(s, 20);
        dvStackPop(s);
        dvStackPop(s);
        dvStackPop(s);         // 栈已空, 记录错误
        dvStackPush(s, 30);    // 不执行
    }
    // trace.toml 已生成, 包含前 4 步的正常记录 + [error] 信息

    return 0;
}
```

```c
#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"

int main(void) {
    dvConfig((dvConfigOptions){ .title = "图错误演示" });

    dvGraphDirected(g, "错误图") {
        dvGdAddNode(g, 0, "甲");
        dvGdAddNode(g, 1, "乙");
        dvGdAddEdge(g, 0, 1);
        dvGdAddEdge(g, 0, 1);    // 重复边, 记录错误
    }

    return 0;
}
```
