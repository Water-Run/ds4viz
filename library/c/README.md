# `ds4viz`  

`ds4viz`是一个`C`库，提供了`C`语言的`ds4viz`支持。  
`ds4viz`是一个可视化数据结构工具。参见：[GitHub](https://github.com/Water-Run/ds4viz)  

## 安装  

`ds4viz`是一个单头文件库（header-only），直接将`ds4viz.h`复制到项目中即可使用。

```c
#include "ds4viz.h"
```

在**一个**源文件中定义`DS4VIZ_IMPLEMENTATION`宏以包含实现：

```c
#define DS4VIZ_IMPLEMENTATION
#include "ds4viz.h"
```

## 编译要求

- C17标准或更高
- 支持GCC、Clang、MSVC等主流编译器

```bash
# GCC示例
gcc -std=c17 -Wall -Wextra -Wpedantic -o my_program my_program.c

# Clang示例
clang -std=c17 -Wall -Wextra -Wpedantic -o my_program my_program.c
```

## 测试  

在目录下运行：

```bash
make test
```

## 快速上手  

以下代码提供了一个快速使用示例，演示如何使用`ds4viz`生成`trace.toml`，并在发生错误时仍然产出包含`[error]`的`.toml`IR文件：

```c
#define DS4VIZ_IMPLEMENTATION
#include "ds4viz.h"

int main(void)
{
    dv_graph_undirected_t *graph = NULL;
    dv_error_t err;
    
    /* 创建无向图实例 */
    graph = dv_graph_undirected_create("demo_graph", NULL);
    if (graph == NULL)
    {
        return 1;
    }
    
    /* 添加节点 */
    err = dv_graph_undirected_add_node(graph, 0, "A");
    if (err != DV_OK) goto cleanup;
    
    err = dv_graph_undirected_add_node(graph, 1, "B");
    if (err != DV_OK) goto cleanup;
    
    err = dv_graph_undirected_add_node(graph, 2, "C");
    if (err != DV_OK) goto cleanup;
    
    /* 添加无向边（内部会规范化为 from < to）*/
    err = dv_graph_undirected_add_edge(graph, 0, 1);
    if (err != DV_OK) goto cleanup;
    
    err = dv_graph_undirected_add_edge(graph, 1, 2);
    if (err != DV_OK) goto cleanup;

cleanup:
    /* 提交并生成trace.toml，无论成功还是失败都会写入文件 */
    dv_graph_undirected_commit(graph);
    dv_graph_undirected_destroy(graph);
    
    return (err == DV_OK) ? 0 : 1;
}
```

## 设计理念

由于C语言没有RAII（资源获取即初始化）和上下文管理器，`ds4viz` C库采用显式的创建/提交/销毁模式：

1. **创建阶段**：调用`dv_xxx_create()`创建数据结构实例
2. **操作阶段**：调用各种操作函数，每次操作都会记录到trace中
3. **提交阶段**：调用`dv_xxx_commit()`将trace写入TOML文件
4. **销毁阶段**：调用`dv_xxx_destroy()`释放所有资源

错误处理采用返回值模式，所有操作函数返回`dv_error_t`枚举值表示成功或失败原因。

## API参考  

### 全局配置

#### `dv_config`

```c
/**
 * @brief 配置全局参数
 * 
 * @param output_path 输出文件路径，默认为 "trace.toml"，传入NULL使用默认值
 * @param title 可视化标题，可为NULL
 * @param author 作者信息，可为NULL
 * @param comment 注释说明，可为NULL
 */
void dv_config(
    const char *output_path,
    const char *title,
    const char *author,
    const char *comment
);
```

**示例：**

```c
#include "ds4viz.h"

int main(void)
{
    dv_config(
        "my_trace.toml",
        "栈操作演示",
        "WaterRun",
        "演示基本的栈操作"
    );
    
    /* 后续操作... */
    return 0;
}
```

### 错误类型

```c
/**
 * @brief 错误码枚举
 */
typedef enum dv_error_e
{
    DV_OK = 0,                  /**< 成功 */
    DV_ERR_NULL_PTR,            /**< 空指针错误 */
    DV_ERR_ALLOC,               /**< 内存分配失败 */
    DV_ERR_EMPTY,               /**< 数据结构为空 */
    DV_ERR_NOT_FOUND,           /**< 元素未找到 */
    DV_ERR_ALREADY_EXISTS,      /**< 元素已存在 */
    DV_ERR_SELF_LOOP,           /**< 自环不允许 */
    DV_ERR_INVALID_PARAM,       /**< 无效参数 */
    DV_ERR_ROOT_EXISTS,         /**< 根节点已存在 */
    DV_ERR_CHILD_EXISTS,        /**< 子节点已存在 */
    DV_ERR_IO                   /**< IO错误 */
} dv_error_t;
```

---

### 线性结构

#### `dv_stack`

```c
/**
 * @brief 创建栈实例
 * 
 * @param label 栈的标签，传入NULL使用默认值 "stack"
 * @param output 输出文件路径，传入NULL使用全局配置
 * @return dv_stack_t* 栈实例指针，失败返回NULL
 */
dv_stack_t *dv_stack_create(const char *label, const char *output);

/**
 * @brief 销毁栈实例并释放所有资源
 * 
 * @param stack 栈实例指针
 */
void dv_stack_destroy(dv_stack_t *stack);

/**
 * @brief 提交并写入trace文件
 * 
 * @param stack 栈实例指针
 * @return dv_error_t 错误码
 */
dv_error_t dv_stack_commit(dv_stack_t *stack);
```

**栈操作函数：**

```c
/**
 * @brief 压栈操作
 * 
 * @param stack 栈实例指针
 * @param value 要压入的值（整数）
 * @return dv_error_t 错误码
 */
dv_error_t dv_stack_push_int(dv_stack_t *stack, int64_t value);

/**
 * @brief 压栈操作（浮点数）
 * 
 * @param stack 栈实例指针
 * @param value 要压入的值
 * @return dv_error_t 错误码
 */
dv_error_t dv_stack_push_float(dv_stack_t *stack, double value);

/**
 * @brief 压栈操作（字符串）
 * 
 * @param stack 栈实例指针
 * @param value 要压入的字符串
 * @return dv_error_t 错误码
 */
dv_error_t dv_stack_push_string(dv_stack_t *stack, const char *value);

/**
 * @brief 压栈操作（布尔值）
 * 
 * @param stack 栈实例指针
 * @param value 要压入的布尔值
 * @return dv_error_t 错误码
 */
dv_error_t dv_stack_push_bool(dv_stack_t *stack, bool value);

/**
 * @brief 弹栈操作
 * 
 * @param stack 栈实例指针
 * @return dv_error_t 错误码，栈为空时返回DV_ERR_EMPTY
 */
dv_error_t dv_stack_pop(dv_stack_t *stack);

/**
 * @brief 清空栈
 * 
 * @param stack 栈实例指针
 * @return dv_error_t 错误码
 */
dv_error_t dv_stack_clear(dv_stack_t *stack);
```

**示例：**

```c
#include "ds4viz.h"

int main(void)
{
    dv_stack_t *s = dv_stack_create("demo_stack", NULL);
    if (s == NULL) return 1;
    
    dv_stack_push_int(s, 10);
    dv_stack_push_int(s, 20);
    dv_stack_push_int(s, 30);
    
    dv_stack_pop(s);
    dv_stack_pop(s);
    
    dv_stack_commit(s);
    dv_stack_destroy(s);
    
    return 0;
}
```

#### `dv_queue`

```c
/**
 * @brief 创建队列实例
 * 
 * @param label 队列的标签，传入NULL使用默认值 "queue"
 * @param output 输出文件路径，传入NULL使用全局配置
 * @return dv_queue_t* 队列实例指针，失败返回NULL
 */
dv_queue_t *dv_queue_create(const char *label, const char *output);

/**
 * @brief 销毁队列实例
 * 
 * @param queue 队列实例指针
 */
void dv_queue_destroy(dv_queue_t *queue);

/**
 * @brief 提交并写入trace文件
 * 
 * @param queue 队列实例指针
 * @return dv_error_t 错误码
 */
dv_error_t dv_queue_commit(dv_queue_t *queue);
```

**队列操作函数：**

```c
/**
 * @brief 入队操作
 * 
 * @param queue 队列实例指针
 * @param value 要入队的值（整数）
 * @return dv_error_t 错误码
 */
dv_error_t dv_queue_enqueue_int(dv_queue_t *queue, int64_t value);

/**
 * @brief 入队操作（浮点数）
 */
dv_error_t dv_queue_enqueue_float(dv_queue_t *queue, double value);

/**
 * @brief 入队操作（字符串）
 */
dv_error_t dv_queue_enqueue_string(dv_queue_t *queue, const char *value);

/**
 * @brief 入队操作（布尔值）
 */
dv_error_t dv_queue_enqueue_bool(dv_queue_t *queue, bool value);

/**
 * @brief 出队操作
 * 
 * @param queue 队列实例指针
 * @return dv_error_t 错误码，队列为空时返回DV_ERR_EMPTY
 */
dv_error_t dv_queue_dequeue(dv_queue_t *queue);

/**
 * @brief 清空队列
 * 
 * @param queue 队列实例指针
 * @return dv_error_t 错误码
 */
dv_error_t dv_queue_clear(dv_queue_t *queue);
```

**示例：**

```c
#include "ds4viz.h"

int main(void)
{
    dv_queue_t *q = dv_queue_create("demo_queue", NULL);
    if (q == NULL) return 1;
    
    dv_queue_enqueue_int(q, 10);
    dv_queue_enqueue_int(q, 20);
    dv_queue_enqueue_int(q, 30);
    
    dv_queue_dequeue(q);
    dv_queue_dequeue(q);
    
    dv_queue_commit(q);
    dv_queue_destroy(q);
    
    return 0;
}
```

#### `dv_slist`（单链表）

```c
/**
 * @brief 创建单链表实例
 * 
 * @param label 单链表的标签，传入NULL使用默认值 "slist"
 * @param output 输出文件路径，传入NULL使用全局配置
 * @return dv_slist_t* 单链表实例指针，失败返回NULL
 */
dv_slist_t *dv_slist_create(const char *label, const char *output);

/**
 * @brief 销毁单链表实例
 */
void dv_slist_destroy(dv_slist_t *slist);

/**
 * @brief 提交并写入trace文件
 */
dv_error_t dv_slist_commit(dv_slist_t *slist);
```

**单链表操作函数：**

```c
/**
 * @brief 在链表头部插入节点
 * 
 * @param slist 单链表实例指针
 * @param value 要插入的值（整数）
 * @param out_node_id 输出参数，新插入节点的id，可为NULL
 * @return dv_error_t 错误码
 */
dv_error_t dv_slist_insert_head_int(dv_slist_t *slist, int64_t value, int32_t *out_node_id);

/**
 * @brief 在链表尾部插入节点
 */
dv_error_t dv_slist_insert_tail_int(dv_slist_t *slist, int64_t value, int32_t *out_node_id);

/**
 * @brief 在指定节点后插入新节点
 * 
 * @param slist 单链表实例指针
 * @param node_id 目标节点的id
 * @param value 要插入的值
 * @param out_node_id 输出参数，新插入节点的id，可为NULL
 * @return dv_error_t 错误码，节点不存在时返回DV_ERR_NOT_FOUND
 */
dv_error_t dv_slist_insert_after_int(dv_slist_t *slist, int32_t node_id, int64_t value, int32_t *out_node_id);

/* 同样提供 _float, _string, _bool 变体 */

/**
 * @brief 删除指定节点
 * 
 * @param slist 单链表实例指针
 * @param node_id 要删除的节点id
 * @return dv_error_t 错误码，节点不存在时返回DV_ERR_NOT_FOUND
 */
dv_error_t dv_slist_delete(dv_slist_t *slist, int32_t node_id);

/**
 * @brief 删除头节点
 * 
 * @param slist 单链表实例指针
 * @return dv_error_t 错误码，链表为空时返回DV_ERR_EMPTY
 */
dv_error_t dv_slist_delete_head(dv_slist_t *slist);

/**
 * @brief 反转链表
 * 
 * @param slist 单链表实例指针
 * @return dv_error_t 错误码
 */
dv_error_t dv_slist_reverse(dv_slist_t *slist);
```

**示例：**

```c
#include "ds4viz.h"

int main(void)
{
    dv_slist_t *slist = dv_slist_create("demo_slist", NULL);
    if (slist == NULL) return 1;
    
    int32_t node_a, node_c;
    dv_slist_insert_head_int(slist, 10, &node_a);
    dv_slist_insert_tail_int(slist, 20, NULL);
    dv_slist_insert_tail_int(slist, 30, &node_c);
    
    dv_slist_insert_after_int(slist, node_a, 15, NULL);
    
    dv_slist_delete(slist, node_c);
    dv_slist_reverse(slist);
    
    dv_slist_commit(slist);
    dv_slist_destroy(slist);
    
    return 0;
}
```

#### `dv_dlist`（双向链表）

```c
/**
 * @brief 创建双向链表实例
 */
dv_dlist_t *dv_dlist_create(const char *label, const char *output);

/**
 * @brief 销毁双向链表实例
 */
void dv_dlist_destroy(dv_dlist_t *dlist);

/**
 * @brief 提交并写入trace文件
 */
dv_error_t dv_dlist_commit(dv_dlist_t *dlist);
```

**双向链表操作函数：**

```c
/**
 * @brief 在链表头部插入节点
 */
dv_error_t dv_dlist_insert_head_int(dv_dlist_t *dlist, int64_t value, int32_t *out_node_id);

/**
 * @brief 在链表尾部插入节点
 */
dv_error_t dv_dlist_insert_tail_int(dv_dlist_t *dlist, int64_t value, int32_t *out_node_id);

/**
 * @brief 在指定节点前插入新节点
 */
dv_error_t dv_dlist_insert_before_int(dv_dlist_t *dlist, int32_t node_id, int64_t value, int32_t *out_node_id);

/**
 * @brief 在指定节点后插入新节点
 */
dv_error_t dv_dlist_insert_after_int(dv_dlist_t *dlist, int32_t node_id, int64_t value, int32_t *out_node_id);

/* 同样提供 _float, _string, _bool 变体 */

/**
 * @brief 删除指定节点
 */
dv_error_t dv_dlist_delete(dv_dlist_t *dlist, int32_t node_id);

/**
 * @brief 删除头节点
 */
dv_error_t dv_dlist_delete_head(dv_dlist_t *dlist);

/**
 * @brief 删除尾节点
 */
dv_error_t dv_dlist_delete_tail(dv_dlist_t *dlist);

/**
 * @brief 反转链表
 */
dv_error_t dv_dlist_reverse(dv_dlist_t *dlist);
```

**示例：**

```c
#include "ds4viz.h"

int main(void)
{
    dv_dlist_t *dlist = dv_dlist_create("demo_dlist", NULL);
    if (dlist == NULL) return 1;
    
    int32_t node_a;
    dv_dlist_insert_head_int(dlist, 10, &node_a);
    dv_dlist_insert_tail_int(dlist, 30, NULL);
    
    dv_dlist_insert_after_int(dlist, node_a, 20, NULL);
    
    dv_dlist_delete_tail(dlist);
    dv_dlist_reverse(dlist);
    
    dv_dlist_commit(dlist);
    dv_dlist_destroy(dlist);
    
    return 0;
}
```

---

### 树结构

#### `dv_binary_tree`

```c
/**
 * @brief 创建二叉树实例
 * 
 * @param label 二叉树的标签，传入NULL使用默认值 "binary_tree"
 * @param output 输出文件路径，传入NULL使用全局配置
 * @return dv_binary_tree_t* 二叉树实例指针，失败返回NULL
 */
dv_binary_tree_t *dv_binary_tree_create(const char *label, const char *output);

/**
 * @brief 销毁二叉树实例
 */
void dv_binary_tree_destroy(dv_binary_tree_t *tree);

/**
 * @brief 提交并写入trace文件
 */
dv_error_t dv_binary_tree_commit(dv_binary_tree_t *tree);
```

**二叉树操作函数：**

```c
/**
 * @brief 插入根节点
 * 
 * @param tree 二叉树实例指针
 * @param value 根节点的值（整数）
 * @param out_node_id 输出参数，根节点的id，可为NULL
 * @return dv_error_t 错误码，根节点已存在时返回DV_ERR_ROOT_EXISTS
 */
dv_error_t dv_binary_tree_insert_root_int(dv_binary_tree_t *tree, int64_t value, int32_t *out_node_id);

/**
 * @brief 在指定父节点的左侧插入子节点
 * 
 * @param tree 二叉树实例指针
 * @param parent_id 父节点的id
 * @param value 要插入的子节点值
 * @param out_node_id 输出参数，新插入节点的id，可为NULL
 * @return dv_error_t 错误码
 */
dv_error_t dv_binary_tree_insert_left_int(dv_binary_tree_t *tree, int32_t parent_id, int64_t value, int32_t *out_node_id);

/**
 * @brief 在指定父节点的右侧插入子节点
 */
dv_error_t dv_binary_tree_insert_right_int(dv_binary_tree_t *tree, int32_t parent_id, int64_t value, int32_t *out_node_id);

/* 同样提供 _float, _string, _bool 变体 */

/**
 * @brief 删除指定节点及其所有子树
 * 
 * @param tree 二叉树实例指针
 * @param node_id 要删除的节点id
 * @return dv_error_t 错误码，节点不存在时返回DV_ERR_NOT_FOUND
 */
dv_error_t dv_binary_tree_delete(dv_binary_tree_t *tree, int32_t node_id);

/**
 * @brief 更新节点的值
 * 
 * @param tree 二叉树实例指针
 * @param node_id 节点id
 * @param value 新的值
 * @return dv_error_t 错误码，节点不存在时返回DV_ERR_NOT_FOUND
 */
dv_error_t dv_binary_tree_update_value_int(dv_binary_tree_t *tree, int32_t node_id, int64_t value);
```

**示例：**

```c
#include "ds4viz.h"

int main(void)
{
    dv_binary_tree_t *tree = dv_binary_tree_create("demo_tree", NULL);
    if (tree == NULL) return 1;
    
    int32_t root, left, right;
    dv_binary_tree_insert_root_int(tree, 10, &root);
    dv_binary_tree_insert_left_int(tree, root, 5, &left);
    dv_binary_tree_insert_right_int(tree, root, 15, &right);
    
    dv_binary_tree_insert_left_int(tree, left, 3, NULL);
    dv_binary_tree_insert_right_int(tree, left, 7, NULL);
    
    dv_binary_tree_update_value_int(tree, right, 20);
    dv_binary_tree_delete(tree, left);
    
    dv_binary_tree_commit(tree);
    dv_binary_tree_destroy(tree);
    
    return 0;
}
```

#### `dv_bst`（二叉搜索树）

```c
/**
 * @brief 创建二叉搜索树实例
 */
dv_bst_t *dv_bst_create(const char *label, const char *output);

/**
 * @brief 销毁二叉搜索树实例
 */
void dv_bst_destroy(dv_bst_t *bst);

/**
 * @brief 提交并写入trace文件
 */
dv_error_t dv_bst_commit(dv_bst_t *bst);
```

**二叉搜索树操作函数：**

```c
/**
 * @brief 插入节点，自动维护二叉搜索树性质
 * 
 * @param bst 二叉搜索树实例指针
 * @param value 要插入的值
 * @param out_node_id 输出参数，新插入节点的id，可为NULL
 * @return dv_error_t 错误码
 */
dv_error_t dv_bst_insert_int(dv_bst_t *bst, int64_t value, int32_t *out_node_id);

dv_error_t dv_bst_insert_float(dv_bst_t *bst, double value, int32_t *out_node_id);

/**
 * @brief 删除节点，自动维护二叉搜索树性质
 * 
 * @param bst 二叉搜索树实例指针
 * @param value 要删除的节点值
 * @return dv_error_t 错误码，节点不存在时返回DV_ERR_NOT_FOUND
 */
dv_error_t dv_bst_delete_int(dv_bst_t *bst, int64_t value);

dv_error_t dv_bst_delete_float(dv_bst_t *bst, double value);
```

**示例：**

```c
#include "ds4viz.h"

int main(void)
{
    dv_bst_t *bst = dv_bst_create("demo_bst", NULL);
    if (bst == NULL) return 1;
    
    dv_bst_insert_int(bst, 10, NULL);
    dv_bst_insert_int(bst, 5, NULL);
    dv_bst_insert_int(bst, 15, NULL);
    dv_bst_insert_int(bst, 3, NULL);
    dv_bst_insert_int(bst, 7, NULL);
    
    dv_bst_delete_int(bst, 5);
    dv_bst_insert_int(bst, 6, NULL);
    
    dv_bst_commit(bst);
    dv_bst_destroy(bst);
    
    return 0;
}
```

#### `dv_heap`（堆）

```c
/**
 * @brief 堆类型枚举
 */
typedef enum dv_heap_type_e
{
    DV_HEAP_MIN,    /**< 最小堆 */
    DV_HEAP_MAX     /**< 最大堆 */
} dv_heap_type_t;

/**
 * @brief 创建堆实例
 * 
 * @param label 堆的标签，传入NULL使用默认值 "heap"
 * @param heap_type 堆类型，DV_HEAP_MIN或DV_HEAP_MAX
 * @param output 输出文件路径，传入NULL使用全局配置
 * @return dv_heap_t* 堆实例指针，失败返回NULL
 */
dv_heap_t *dv_heap_create(const char *label, dv_heap_type_t heap_type, const char *output);

/**
 * @brief 销毁堆实例
 */
void dv_heap_destroy(dv_heap_t *heap);

/**
 * @brief 提交并写入trace文件
 */
dv_error_t dv_heap_commit(dv_heap_t *heap);
```

**堆操作函数：**

```c
/**
 * @brief 插入元素，自动维护堆性质
 * 
 * @param heap 堆实例指针
 * @param value 要插入的值
 * @return dv_error_t 错误码
 */
dv_error_t dv_heap_insert_int(dv_heap_t *heap, int64_t value);

dv_error_t dv_heap_insert_float(dv_heap_t *heap, double value);

/**
 * @brief 提取堆顶元素，自动维护堆性质
 * 
 * @param heap 堆实例指针
 * @return dv_error_t 错误码，堆为空时返回DV_ERR_EMPTY
 */
dv_error_t dv_heap_extract(dv_heap_t *heap);

/**
 * @brief 清空堆
 * 
 * @param heap 堆实例指针
 * @return dv_error_t 错误码
 */
dv_error_t dv_heap_clear(dv_heap_t *heap);
```

**示例：**

```c
#include "ds4viz.h"

int main(void)
{
    /* 最小堆 */
    dv_heap_t *min_heap = dv_heap_create("demo_min_heap", DV_HEAP_MIN, "min_heap.toml");
    if (min_heap == NULL) return 1;
    
    dv_heap_insert_int(min_heap, 10);
    dv_heap_insert_int(min_heap, 5);
    dv_heap_insert_int(min_heap, 15);
    dv_heap_insert_int(min_heap, 3);
    
    dv_heap_extract(min_heap);  /* 提取最小值 3 */
    
    dv_heap_commit(min_heap);
    dv_heap_destroy(min_heap);
    
    /* 最大堆 */
    dv_heap_t *max_heap = dv_heap_create("demo_max_heap", DV_HEAP_MAX, "max_heap.toml");
    if (max_heap == NULL) return 1;
    
    dv_heap_insert_int(max_heap, 10);
    dv_heap_insert_int(max_heap, 5);
    dv_heap_insert_int(max_heap, 15);
    dv_heap_insert_int(max_heap, 3);
    
    dv_heap_extract(max_heap);  /* 提取最大值 15 */
    
    dv_heap_commit(max_heap);
    dv_heap_destroy(max_heap);
    
    return 0;
}
```

---

### 图结构

#### `dv_graph_undirected`（无向图）

```c
/**
 * @brief 创建无向图实例
 * 
 * @param label 无向图的标签，传入NULL使用默认值 "graph"
 * @param output 输出文件路径，传入NULL使用全局配置
 * @return dv_graph_undirected_t* 无向图实例指针，失败返回NULL
 */
dv_graph_undirected_t *dv_graph_undirected_create(const char *label, const char *output);

/**
 * @brief 销毁无向图实例
 */
void dv_graph_undirected_destroy(dv_graph_undirected_t *graph);

/**
 * @brief 提交并写入trace文件
 */
dv_error_t dv_graph_undirected_commit(dv_graph_undirected_t *graph);
```

**无向图操作函数：**

```c
/**
 * @brief 添加节点
 * 
 * @param graph 无向图实例指针
 * @param node_id 节点id
 * @param label 节点标签，长度限制为1-32字符
 * @return dv_error_t 错误码，节点已存在时返回DV_ERR_ALREADY_EXISTS
 */
dv_error_t dv_graph_undirected_add_node(dv_graph_undirected_t *graph, int32_t node_id, const char *label);

/**
 * @brief 添加无向边，内部自动规范化为from_id < to_id
 * 
 * @param graph 无向图实例指针
 * @param from_id 起始节点id
 * @param to_id 终止节点id
 * @return dv_error_t 错误码
 */
dv_error_t dv_graph_undirected_add_edge(dv_graph_undirected_t *graph, int32_t from_id, int32_t to_id);

/**
 * @brief 删除节点及其相关的所有边
 * 
 * @param graph 无向图实例指针
 * @param node_id 要删除的节点id
 * @return dv_error_t 错误码，节点不存在时返回DV_ERR_NOT_FOUND
 */
dv_error_t dv_graph_undirected_remove_node(dv_graph_undirected_t *graph, int32_t node_id);

/**
 * @brief 删除边
 * 
 * @param graph 无向图实例指针
 * @param from_id 起始节点id
 * @param to_id 终止节点id
 * @return dv_error_t 错误码，边不存在时返回DV_ERR_NOT_FOUND
 */
dv_error_t dv_graph_undirected_remove_edge(dv_graph_undirected_t *graph, int32_t from_id, int32_t to_id);

/**
 * @brief 更新节点标签
 * 
 * @param graph 无向图实例指针
 * @param node_id 节点id
 * @param label 新的节点标签，长度限制为1-32字符
 * @return dv_error_t 错误码，节点不存在时返回DV_ERR_NOT_FOUND
 */
dv_error_t dv_graph_undirected_update_node_label(dv_graph_undirected_t *graph, int32_t node_id, const char *label);
```

**示例：**

```c
#include "ds4viz.h"

int main(void)
{
    dv_graph_undirected_t *graph = dv_graph_undirected_create("demo_graph", NULL);
    if (graph == NULL) return 1;
    
    dv_graph_undirected_add_node(graph, 0, "A");
    dv_graph_undirected_add_node(graph, 1, "B");
    dv_graph_undirected_add_node(graph, 2, "C");
    
    dv_graph_undirected_add_edge(graph, 0, 1);
    dv_graph_undirected_add_edge(graph, 1, 2);
    
    dv_graph_undirected_update_node_label(graph, 1, "B_updated");
    dv_graph_undirected_remove_edge(graph, 0, 1);
    
    dv_graph_undirected_commit(graph);
    dv_graph_undirected_destroy(graph);
    
    return 0;
}
```

#### `dv_graph_directed`（有向图）

```c
/**
 * @brief 创建有向图实例
 */
dv_graph_directed_t *dv_graph_directed_create(const char *label, const char *output);

/**
 * @brief 销毁有向图实例
 */
void dv_graph_directed_destroy(dv_graph_directed_t *graph);

/**
 * @brief 提交并写入trace文件
 */
dv_error_t dv_graph_directed_commit(dv_graph_directed_t *graph);
```

**有向图操作函数：**

```c
/**
 * @brief 添加节点
 */
dv_error_t dv_graph_directed_add_node(dv_graph_directed_t *graph, int32_t node_id, const char *label);

/**
 * @brief 添加有向边
 */
dv_error_t dv_graph_directed_add_edge(dv_graph_directed_t *graph, int32_t from_id, int32_t to_id);

/**
 * @brief 删除节点及其相关的所有边
 */
dv_error_t dv_graph_directed_remove_node(dv_graph_directed_t *graph, int32_t node_id);

/**
 * @brief 删除边
 */
dv_error_t dv_graph_directed_remove_edge(dv_graph_directed_t *graph, int32_t from_id, int32_t to_id);

/**
 * @brief 更新节点标签
 */
dv_error_t dv_graph_directed_update_node_label(dv_graph_directed_t *graph, int32_t node_id, const char *label);
```

**示例：**

```c
#include "ds4viz.h"

int main(void)
{
    dv_graph_directed_t *graph = dv_graph_directed_create("demo_digraph", NULL);
    if (graph == NULL) return 1;
    
    dv_graph_directed_add_node(graph, 0, "A");
    dv_graph_directed_add_node(graph, 1, "B");
    dv_graph_directed_add_node(graph, 2, "C");
    
    dv_graph_directed_add_edge(graph, 0, 1);
    dv_graph_directed_add_edge(graph, 1, 2);
    dv_graph_directed_add_edge(graph, 2, 0);
    
    dv_graph_directed_update_node_label(graph, 1, "B_updated");
    dv_graph_directed_remove_edge(graph, 2, 0);
    
    dv_graph_directed_commit(graph);
    dv_graph_directed_destroy(graph);
    
    return 0;
}
```

#### `dv_graph_weighted`（带权图）

```c
/**
 * @brief 创建带权图实例
 * 
 * @param label 带权图的标签，传入NULL使用默认值 "graph"
 * @param directed 是否为有向图，true表示有向，false表示无向
 * @param output 输出文件路径，传入NULL使用全局配置
 * @return dv_graph_weighted_t* 带权图实例指针，失败返回NULL
 */
dv_graph_weighted_t *dv_graph_weighted_create(const char *label, bool directed, const char *output);

/**
 * @brief 销毁带权图实例
 */
void dv_graph_weighted_destroy(dv_graph_weighted_t *graph);

/**
 * @brief 提交并写入trace文件
 */
dv_error_t dv_graph_weighted_commit(dv_graph_weighted_t *graph);
```

**带权图操作函数：**

```c
/**
 * @brief 添加节点
 */
dv_error_t dv_graph_weighted_add_node(dv_graph_weighted_t *graph, int32_t node_id, const char *label);

/**
 * @brief 添加带权边
 * 
 * @param graph 带权图实例指针
 * @param from_id 起始节点id
 * @param to_id 终止节点id
 * @param weight 边的权重
 * @return dv_error_t 错误码
 */
dv_error_t dv_graph_weighted_add_edge(dv_graph_weighted_t *graph, int32_t from_id, int32_t to_id, double weight);

/**
 * @brief 删除节点及其相关的所有边
 */
dv_error_t dv_graph_weighted_remove_node(dv_graph_weighted_t *graph, int32_t node_id);

/**
 * @brief 删除边
 */
dv_error_t dv_graph_weighted_remove_edge(dv_graph_weighted_t *graph, int32_t from_id, int32_t to_id);

/**
 * @brief 更新边的权重
 * 
 * @param graph 带权图实例指针
 * @param from_id 起始节点id
 * @param to_id 终止节点id
 * @param weight 新的权重值
 * @return dv_error_t 错误码，边不存在时返回DV_ERR_NOT_FOUND
 */
dv_error_t dv_graph_weighted_update_weight(dv_graph_weighted_t *graph, int32_t from_id, int32_t to_id, double weight);

/**
 * @brief 更新节点标签
 */
dv_error_t dv_graph_weighted_update_node_label(dv_graph_weighted_t *graph, int32_t node_id, const char *label);
```

**示例：**

```c
#include "ds4viz.h"

int main(void)
{
    /* 无向带权图 */
    dv_graph_weighted_t *graph = dv_graph_weighted_create("demo_weighted_graph", false, NULL);
    if (graph == NULL) return 1;
    
    dv_graph_weighted_add_node(graph, 0, "A");
    dv_graph_weighted_add_node(graph, 1, "B");
    dv_graph_weighted_add_node(graph, 2, "C");
    
    dv_graph_weighted_add_edge(graph, 0, 1, 3.5);
    dv_graph_weighted_add_edge(graph, 1, 2, 2.0);
    dv_graph_weighted_add_edge(graph, 2, 0, 4.2);
    
    dv_graph_weighted_update_weight(graph, 0, 1, 5.0);
    dv_graph_weighted_remove_edge(graph, 2, 0);
    
    dv_graph_weighted_commit(graph);
    dv_graph_weighted_destroy(graph);
    
    /* 有向带权图 */
    dv_graph_weighted_t *digraph = dv_graph_weighted_create("demo_directed_weighted", true, "directed.toml");
    if (digraph == NULL) return 1;
    
    dv_graph_weighted_add_node(digraph, 0, "Start");
    dv_graph_weighted_add_node(digraph, 1, "Middle");
    dv_graph_weighted_add_node(digraph, 2, "End");
    
    dv_graph_weighted_add_edge(digraph, 0, 1, 1.5);
    dv_graph_weighted_add_edge(digraph, 1, 2, 2.5);
    
    dv_graph_weighted_commit(digraph);
    dv_graph_weighted_destroy(digraph);
    
    return 0;
}
```

---

### 错误处理

所有数据结构在遇到错误时都会返回对应的错误码。在调用`commit`时，如果之前发生过错误，将生成包含`[error]`部分的`.toml`文件。

**示例：**

```c
#include "ds4viz.h"
#include <stdio.h>

int main(void)
{
    dv_binary_tree_t *tree = dv_binary_tree_create("demo_tree", NULL);
    if (tree == NULL)
    {
        fprintf(stderr, "Failed to create tree\n");
        return 1;
    }
    
    dv_error_t err;
    int32_t root;
    
    err = dv_binary_tree_insert_root_int(tree, 10, &root);
    if (err != DV_OK)
    {
        fprintf(stderr, "Failed to insert root\n");
        goto cleanup;
    }
    
    /* 父节点不存在，将返回错误 */
    err = dv_binary_tree_insert_left_int(tree, 999, 5, NULL);
    if (err != DV_OK)
    {
        fprintf(stderr, "Error: %d\n", err);
        /* 继续执行cleanup，将生成包含[error]的trace文件 */
    }

cleanup:
    dv_binary_tree_commit(tree);
    dv_binary_tree_destroy(tree);
    
    return (err == DV_OK) ? 0 : 1;
}
```

生成的`trace.toml`将包含：

```toml
[error]
type = "runtime"
message = "Parent node not found: 999"
step = 1
last_state = 1
```

---

### 辅助宏

为了方便错误处理，库提供了以下辅助宏：

```c
/**
 * @brief 检查错误并跳转到清理标签
 * 
 * @param expr 要检查的表达式
 * @param label 错误时跳转的标签
 */
#define DV_CHECK_GOTO(expr, label) \
    do \
    { \
        if ((expr) != DV_OK) \
        { \
            goto label; \
        } \
    } while (0)

/**
 * @brief 检查错误并返回
 * 
 * @param expr 要检查的表达式
 */
#define DV_CHECK_RETURN(expr) \
    do \
    { \
        dv_error_t _err = (expr); \
        if (_err != DV_OK) \
        { \
            return _err; \
        } \
    } while (0)
```

**使用示例：**

```c
#include "ds4viz.h"

int main(void)
{
    dv_stack_t *s = dv_stack_create("demo", NULL);
    if (s == NULL) return 1;
    
    DV_CHECK_GOTO(dv_stack_push_int(s, 10), cleanup);
    DV_CHECK_GOTO(dv_stack_push_int(s, 20), cleanup);
    DV_CHECK_GOTO(dv_stack_pop(s), cleanup);

cleanup:
    dv_stack_commit(s);
    dv_stack_destroy(s);
    
    return 0;
}
```

---

## 类型约束

根据IR定义，`value`字段仅支持以下类型：

- `int64_t`：整数（通过`_int`后缀函数）
- `double`：浮点数（通过`_float`后缀函数）
- `const char *`：字符串（通过`_string`后缀函数）
- `bool`：布尔值（通过`_bool`后缀函数）

所有API通过函数名后缀区分值类型，确保类型安全。
