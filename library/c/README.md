# `ds4viz`

`ds4viz` 是一个 **C 单头文件库**，用于记录数据结构操作过程并生成 `.toml` IR 文件。  
该 IR 文件可直接被渲染端消费，用于可视化回放。

- 项目地址：[GitHub](https://github.com/Water-Run/ds4viz)

## 安装与集成

`ds4viz` 以单头文件提供：`ds4viz.h`。  
将其加入项目后即可通过 `#include "ds4viz.h"` 使用。
`ds4viz`通过宏, 增强C有限的表达力, 实现和其它库类似的上下文闭包能力(确保异常存储), 并简化了使用的认知负担.  

## 宏开关说明

- `DS4VIZ_IMPLEMENTATION`

```c
/**
 * @brief 在当前编译单元生成库实现代码。
 * @constraint 必须且只能在一个 .c 文件中定义一次。
 * @error 若在多个 .c 文件中定义，将导致重复符号链接错误。
 */
#define DS4VIZ_IMPLEMENTATION
```

- `DS4VIZ_SHORT_NAMES`

```c
/**
 * @brief 启用短名称 API（dv 前缀），提高调用可读性。
 * @example ds4vizGraphUndirectedAddNode -> dvGuAddNode
 * @note 若不定义该宏，仍可使用长名前缀 API。
 */
#define DS4VIZ_SHORT_NAMES
```

## 编译与测试

### 编译器要求

当前仅支持 GCC。

### 测试命令

项目测试程序为根目录 `test.c`：

```bash
gcc test.c -o test && ./test
```

## 快速开始

```c
#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"

int main(void) {
    dvConfig((dvConfigOptions){
        .outputPath = "trace.toml",
        .title = "Graph demo",
        .author = "WaterRun",
        .comment = "C quick start"
    });

    dvGraphUndirected(g, "graph") {
        dvGuAddNode(g, 0, "A");
        dvGuAddNode(g, 1, "B");
        dvGuAddNode(g, 2, "C");

        dvGuAddEdge(g, 0, 1);
        dvGuAddEdge(g, 1, 2);
    }

    return 0;
}
```

### 运行语义

- 进入结构作用域时：创建对象并写入初始状态。
- 每次 API 调用：写入一个 `step` 与对应新 `state`。
- 作用域正常结束：输出 `[result]`。
- 作用域内发生失败：收尾输出 `[error]`。

## API 参考

## 全局配置

```c
typedef struct {
    /**
     * @brief 输出文件路径。
     * @default "trace.toml"
     */
    const char* outputPath;

    /**
     * @brief 标题。
     * @constraint 建议长度 1..128，且不包含换行。
     */
    const char* title;

    /**
     * @brief 作者。
     * @constraint 建议长度 1..64，且不包含换行。
     */
    const char* author;

    /**
     * @brief 备注。
     * @constraint 建议长度 1..256，且不包含换行。
     */
    const char* comment;
} dvConfigOptions;

/**
 * @brief 设置全局输出配置。
 * @param options 配置对象。
 */
void dvConfig(dvConfigOptions options);
```

## 线性结构

### `dvStack`

创建：

```c
dvStack(s) { ... }               
dvStack(s, "demoStack") { ... }  // 自定义 label
```

```c
/**
 * @brief 栈入栈。
 * @param s 栈对象。
 * @param value 入栈值。
 * @constraint value 必须为 IR 支持标量：int/float/double/const char*/bool。
 */
void dvStackPush(s, value);

/**
 * @brief 栈出栈。
 * @param s 栈对象。
 * @constraint 栈不能为空。
 */
void dvStackPop(s);

/**
 * @brief 清空栈。
 * @param s 栈对象。
 */
void dvStackClear(s);
```

### `dvQueue`

创建：

```c
dvQueue(q) { ... }               // label 默认 "queue"
dvQueue(q, "demoQueue") { ... }  // 自定义 label
```

```c
/**
 * @brief 入队。
 * @param q 队列对象。
 * @param value 入队值（同 stack.push 类型约束）。
 */
void dvQueueEnqueue(q, value);

/**
 * @brief 出队。
 * @param q 队列对象。
 * @constraint 队列不能为空。
 */
void dvQueueDequeue(q);

/**
 * @brief 清空队列。
 * @param q 队列对象。
 */
void dvQueueClear(q);
```

### `dvSingleLinkedList`

创建：

```c
dvSingleLinkedList(l) { ... }               // label 默认 "slist"
dvSingleLinkedList(l, "demoSlist") { ... }  // 自定义 label
```

```c
/**
 * @brief 头插。
 * @return 新节点 id（非负整数）。
 */
int dvSlInsertHead(l, value);

/**
 * @brief 尾插。
 * @return 新节点 id（非负整数）。
 */
int dvSlInsertTail(l, value);

/**
 * @brief 在指定节点后插入。
 * @param nodeId 目标节点 id。
 * @constraint nodeId 必须存在。
 * @return 新节点 id。
 */
int dvSlInsertAfter(l, nodeId, value);

/**
 * @brief 删除指定节点。
 * @constraint nodeId 必须存在。
 */
void dvSlDelete(l, nodeId);

/**
 * @brief 删除头节点。
 * @constraint 链表不能为空。
 */
void dvSlDeleteHead(l);

/**
 * @brief 反转链表。
 */
void dvSlReverse(l);
```

### `dvDoubleLinkedList`

创建：

```c
dvDoubleLinkedList(l) { ... }               // label 默认 "dlist"
dvDoubleLinkedList(l, "demoDlist") { ... }  // 自定义 label
```

```c
int dvDlInsertHead(l, value);
int dvDlInsertTail(l, value);

/**
 * @brief 在指定节点前插入。
 * @constraint nodeId 必须存在。
 */
int dvDlInsertBefore(l, nodeId, value);

/**
 * @brief 在指定节点后插入。
 * @constraint nodeId 必须存在。
 */
int dvDlInsertAfter(l, nodeId, value);

/**
 * @brief 删除指定节点。
 * @constraint nodeId 必须存在。
 */
void dvDlDelete(l, nodeId);

/**
 * @brief 删除头节点。
 * @constraint 链表不能为空。
 */
void dvDlDeleteHead(l);

/**
 * @brief 删除尾节点。
 * @constraint 链表不能为空。
 */
void dvDlDeleteTail(l);

/**
 * @brief 反转链表。
 */
void dvDlReverse(l);
```

## 树结构

### `dvBinaryTree`

创建：

```c
dvBinaryTree(t) { ... }             // label 默认 "binaryTree"
dvBinaryTree(t, "demoTree") { ... } // 自定义 label
```

```c
/**
 * @brief 插入根节点。
 * @constraint 根节点必须不存在。
 * @return 根节点 id。
 */
int dvBtInsertRoot(t, value);

/**
 * @brief 插入左子节点。
 * @constraint parentId 必须存在，且其左孩子必须为空。
 * @return 新节点 id。
 */
int dvBtInsertLeft(t, parentId, value);

/**
 * @brief 插入右子节点。
 * @constraint parentId 必须存在，且其右孩子必须为空。
 * @return 新节点 id。
 */
int dvBtInsertRight(t, parentId, value);

/**
 * @brief 删除节点及其子树。
 * @constraint nodeId 必须存在。
 */
void dvBtDelete(t, nodeId);

/**
 * @brief 更新节点值。
 * @constraint nodeId 必须存在。
 */
void dvBtUpdateValue(t, nodeId, value);
```

### `dvBinarySearchTree`

创建：

```c
dvBinarySearchTree(b) { ... }            // label 默认 "bst"
dvBinarySearchTree(b, "demoBst") { ... } // 自定义 label
```

```c
/**
 * @brief 插入 BST 节点。
 * @constraint value 类型必须为数值（int/float/double）。
 * @return 新节点 id。
 */
int dvBstInsert(b, value);

/**
 * @brief 删除指定值节点。
 * @constraint value 必须存在于树中。
 */
void dvBstDelete(b, value);
```

### `dvHeap`

创建：

```c
dvHeap(h) { ... }                                  // label="heap", order=min
dvHeap(h, "demoHeap") { ... }                      // order=min
dvHeap(h, "demoMinHeap", dvHeapOrderMin) { ... }
dvHeap(h, "demoMaxHeap", dvHeapOrderMax) { ... }
```

```c
typedef enum {
    dvHeapOrderMin = 0,
    dvHeapOrderMax = 1
} dvHeapOrder;

/**
 * @brief 插入堆元素。
 * @constraint value 类型必须为数值（int/float/double）。
 */
void dvHeapInsert(h, value);

/**
 * @brief 提取堆顶元素。
 * @constraint 堆不能为空。
 */
void dvHeapExtract(h);

/**
 * @brief 清空堆。
 */
void dvHeapClear(h);
```

## 图结构

### `dvGraphUndirected`

创建：

```c
dvGraphUndirected(g) { ... }              // label 默认 "graph"
dvGraphUndirected(g, "demoGraph") { ... } // 自定义 label
```

```c
/**
 * @brief 添加节点。
 * @param nodeId 非负整数。
 * @param label 节点标签。
 * @constraint nodeId 不可重复；label 非空且不含换行，建议长度 1..32。
 */
void dvGuAddNode(g, nodeId, label);

/**
 * @brief 添加无向边。
 * @constraint 两端节点必须存在；禁止自环；禁止重复边。
 * @note 内部规范化存储为 from < to。
 */
void dvGuAddEdge(g, fromId, toId);

/**
 * @brief 删除节点及其关联边。
 * @constraint nodeId 必须存在。
 */
void dvGuRemoveNode(g, nodeId);

/**
 * @brief 删除无向边。
 * @constraint 边必须存在。
 * @note 按规范化端点匹配。
 */
void dvGuRemoveEdge(g, fromId, toId);

/**
 * @brief 更新节点标签。
 * @constraint 节点必须存在；label 满足长度与字符限制。
 */
void dvGuUpdateNodeLabel(g, nodeId, label);
```

### `dvGraphDirected`

创建：

```c
dvGraphDirected(g) { ... }
dvGraphDirected(g, "demoDigraph") { ... }
```

```c
void dvGdAddNode(g, nodeId, label);

/**
 * @brief 添加有向边 (fromId -> toId)。
 * @constraint 两端节点必须存在；禁止自环；禁止重复边。
 */
void dvGdAddEdge(g, fromId, toId);

void dvGdRemoveNode(g, nodeId);
void dvGdRemoveEdge(g, fromId, toId);
void dvGdUpdateNodeLabel(g, nodeId, label);
```

### `dvGraphWeighted`

创建：

```c
dvGraphWeighted(g) { ... }
dvGraphWeighted(g, "demoWeightedGraph") { ... }
```

```c
void dvGwAddNode(g, nodeId, label);

/**
 * @brief 添加带权边。
 * @constraint 两端节点必须存在；禁止自环；禁止重复边；weight 必须为数值。
 */
void dvGwAddEdge(g, fromId, toId, weight);

void dvGwRemoveNode(g, nodeId);
void dvGwRemoveEdge(g, fromId, toId);

/**
 * @brief 更新边权。
 * @constraint 边必须存在；weight 必须为数值。
 */
void dvGwUpdateWeight(g, fromId, toId, weight);

void dvGwUpdateNodeLabel(g, nodeId, label);
```

## 类型约束

`value` 仅允许 IR 标量类型：

- `int`（及其他整型）
- `float` / `double`
- `const char*`
- `bool`

不支持将结构体、数组、对象指针直接写入 `value` 字段。

## 8. 长名与短名映射

启用 `DS4VIZ_SHORT_NAMES` 时使用 `dv...`；否则使用 `ds4viz...`。

| 长名                                 | 短名               |
|--------------------------------------|--------------------|
| `ds4vizConfig`                       | `dvConfig`         |
| `ds4vizStack`                        | `dvStack`          |
| `ds4vizStackPush`                    | `dvStackPush`      |
| `ds4vizQueueEnqueue`                 | `dvQueueEnqueue`   |
| `ds4vizSingleLinkedListInsertHead`   | `dvSlInsertHead`   |
| `ds4vizDoubleLinkedListInsertBefore` | `dvDlInsertBefore` |
| `ds4vizBinaryTreeInsertRoot`         | `dvBtInsertRoot`   |
| `ds4vizBinarySearchTreeInsert`       | `dvBstInsert`      |
| `ds4vizHeapInsert`                   | `dvHeapInsert`     |
| `ds4vizGraphUndirectedAddNode`       | `dvGuAddNode`      |
| `ds4vizGraphDirectedAddEdge`         | `dvGdAddEdge`      |
| `ds4vizGraphWeightedUpdateWeight`    | `dvGwUpdateWeight` |
