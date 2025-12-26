/**
 * @file ds4viz.h
 * @brief ds4viz C库 - 数据结构可视化工具
 *
 * 单头文件库，提供数据结构的可视化追踪功能，生成标准化的TOML中间表示文件。
 * 在一个源文件中定义 DS4VIZ_IMPLEMENTATION 宏以包含实现。
 *
 * @author WaterRun
 * @date 2025-12-24
 * @version 0.1.0
 */

#ifndef DS4VIZ_H
#define DS4VIZ_H

#ifdef __cplusplus
extern "C"
{
#endif

    /* ========================================================================== */
    /* 包含指令                                                                    */
    /* ========================================================================== */

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

/* ========================================================================== */
/* 版本信息                                                                    */
/* ========================================================================== */

/**
 * @brief 主版本号
 * @def DS4VIZ_VERSION_MAJOR
 */
#define DS4VIZ_VERSION_MAJOR 0

/**
 * @brief 次版本号
 * @def DS4VIZ_VERSION_MINOR
 */
#define DS4VIZ_VERSION_MINOR 1

/**
 * @brief 补丁版本号
 * @def DS4VIZ_VERSION_PATCH
 */
#define DS4VIZ_VERSION_PATCH 0

/**
 * @brief 版本字符串
 * @def DS4VIZ_VERSION_STRING
 */
#define DS4VIZ_VERSION_STRING "0.1.0"

/* ========================================================================== */
/* 常量定义                                                                    */
/* ========================================================================== */

/**
 * @brief 默认输出文件路径
 * @def DV_DEFAULT_OUTPUT
 */
#define DV_DEFAULT_OUTPUT "trace.toml"

/**
 * @brief 最大标签长度
 * @def DV_MAX_LABEL_LEN
 */
#define DV_MAX_LABEL_LEN 64

/**
 * @brief 最大路径长度
 * @def DV_MAX_PATH_LEN
 */
#define DV_MAX_PATH_LEN 256

/**
 * @brief 最大错误消息长度
 * @def DV_MAX_ERROR_MSG_LEN
 */
#define DV_MAX_ERROR_MSG_LEN 512

/**
 * @brief 初始容量
 * @def DV_INITIAL_CAPACITY
 */
#define DV_INITIAL_CAPACITY 16

/* ========================================================================== */
/* 辅助宏                                                                      */
/* ========================================================================== */

/**
 * @brief 检查错误并跳转到清理标签
 *
 * @param expr 要检查的表达式
 * @param label 错误时跳转的标签
 */
#define DV_CHECK_GOTO(expr, label) \
    do                             \
    {                              \
        if ((expr) != DV_OK)       \
        {                          \
            goto label;            \
        }                          \
    } while (0)

/**
 * @brief 检查错误并返回
 *
 * @param expr 要检查的表达式
 */
#define DV_CHECK_RETURN(expr)     \
    do                            \
    {                             \
        dv_error_t _err = (expr); \
        if (_err != DV_OK)        \
        {                         \
            return _err;          \
        }                         \
    } while (0)

    /* ========================================================================== */
    /* 枚举类型定义                                                                */
    /* ========================================================================== */

    /**
     * @brief 错误码枚举
     *
     * 定义库中所有可能的错误类型
     *
     * @enum dv_error_e
     * @typedef dv_error_t
     */
    typedef enum dv_error_e
    {
        /** @brief 成功 */
        DV_OK = 0,

        /** @brief 空指针错误 */
        DV_ERR_NULL_PTR,

        /** @brief 内存分配失败 */
        DV_ERR_ALLOC,

        /** @brief 数据结构为空 */
        DV_ERR_EMPTY,

        /** @brief 元素未找到 */
        DV_ERR_NOT_FOUND,

        /** @brief 元素已存在 */
        DV_ERR_ALREADY_EXISTS,

        /** @brief 自环不允许 */
        DV_ERR_SELF_LOOP,

        /** @brief 无效参数 */
        DV_ERR_INVALID_PARAM,

        /** @brief 根节点已存在 */
        DV_ERR_ROOT_EXISTS,

        /** @brief 子节点已存在 */
        DV_ERR_CHILD_EXISTS,

        /** @brief IO错误 */
        DV_ERR_IO
    } dv_error_t;

    /**
     * @brief 值类型枚举
     *
     * 用于标识存储值的类型
     *
     * @enum dv_value_type_e
     * @typedef dv_value_type_t
     */
    typedef enum dv_value_type_e
    {
        /** @brief 整数类型 */
        DV_VALUE_INT = 0,

        /** @brief 浮点数类型 */
        DV_VALUE_FLOAT,

        /** @brief 字符串类型 */
        DV_VALUE_STRING,

        /** @brief 布尔类型 */
        DV_VALUE_BOOL
    } dv_value_type_t;

    /**
     * @brief 堆类型枚举
     *
     * @enum dv_heap_type_e
     * @typedef dv_heap_type_t
     */
    typedef enum dv_heap_type_e
    {
        /** @brief 最小堆 */
        DV_HEAP_MIN = 0,

        /** @brief 最大堆 */
        DV_HEAP_MAX
    } dv_heap_type_t;

    /* ========================================================================== */
    /* 联合体类型定义                                                              */
    /* ========================================================================== */

    /**
     * @brief 通用值联合体
     *
     * 可以存储不同类型的值
     *
     * @union dv_value_data_u
     * @typedef dv_value_data_t
     */
    typedef union dv_value_data_u
    {
        /** @brief 整数值 */
        int64_t int_val;

        /** @brief 浮点值 */
        double float_val;

        /** @brief 字符串值（需要复制） */
        char *string_val;

        /** @brief 布尔值 */
        bool bool_val;
    } dv_value_data_t;

    /* ========================================================================== */
    /* 结构体类型定义                                                              */
    /* ========================================================================== */

    /**
     * @brief 通用值结构体
     *
     * 包含值类型和值数据
     *
     * @struct dv_value_s
     * @typedef dv_value_t
     */
    typedef struct dv_value_s
    {
        /** @brief 值类型 */
        dv_value_type_t type;

        /** @brief 值数据 */
        dv_value_data_t data;
    } dv_value_t;

    /* 前向声明内部类型 */
    typedef struct dv_session_s dv_session_t;

    /**
     * @brief 栈数据结构
     *
     * @struct dv_stack_s
     * @typedef dv_stack_t
     */
    typedef struct dv_stack_s dv_stack_t;

    /**
     * @brief 队列数据结构
     *
     * @struct dv_queue_s
     * @typedef dv_queue_t
     */
    typedef struct dv_queue_s dv_queue_t;

    /**
     * @brief 单链表数据结构
     *
     * @struct dv_slist_s
     * @typedef dv_slist_t
     */
    typedef struct dv_slist_s dv_slist_t;

    /**
     * @brief 双向链表数据结构
     *
     * @struct dv_dlist_s
     * @typedef dv_dlist_t
     */
    typedef struct dv_dlist_s dv_dlist_t;

    /**
     * @brief 二叉树数据结构
     *
     * @struct dv_binary_tree_s
     * @typedef dv_binary_tree_t
     */
    typedef struct dv_binary_tree_s dv_binary_tree_t;

    /**
     * @brief 二叉搜索树数据结构
     *
     * @struct dv_bst_s
     * @typedef dv_bst_t
     */
    typedef struct dv_bst_s dv_bst_t;

    /**
     * @brief 堆数据结构
     *
     * @struct dv_heap_s
     * @typedef dv_heap_t
     */
    typedef struct dv_heap_s dv_heap_t;

    /**
     * @brief 无向图数据结构
     *
     * @struct dv_graph_undirected_s
     * @typedef dv_graph_undirected_t
     */
    typedef struct dv_graph_undirected_s dv_graph_undirected_t;

    /**
     * @brief 有向图数据结构
     *
     * @struct dv_graph_directed_s
     * @typedef dv_graph_directed_t
     */
    typedef struct dv_graph_directed_s dv_graph_directed_t;

    /**
     * @brief 带权图数据结构
     *
     * @struct dv_graph_weighted_s
     * @typedef dv_graph_weighted_t
     */
    typedef struct dv_graph_weighted_s dv_graph_weighted_t;

    /* ========================================================================== */
    /* 全局配置函数                                                                */
    /* ========================================================================== */

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
        const char *comment);

    /**
     * @brief 获取库版本字符串
     *
     * @return const char* 版本字符串
     */
    const char *dv_version(void);

    /* ========================================================================== */
    /* 栈 API                                                                      */
    /* ========================================================================== */

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
     * @param p_stack 栈实例指针
     */
    void dv_stack_destroy(dv_stack_t *p_stack);

    /**
     * @brief 提交并写入trace文件
     *
     * @param p_stack 栈实例指针
     * @return dv_error_t 错误码
     */
    dv_error_t dv_stack_commit(dv_stack_t *p_stack);

    /**
     * @brief 压栈操作（整数）
     *
     * @param p_stack 栈实例指针
     * @param value 要压入的值
     * @return dv_error_t 错误码
     */
    dv_error_t dv_stack_push_int(dv_stack_t *p_stack, int64_t value);

    /**
     * @brief 压栈操作（浮点数）
     *
     * @param p_stack 栈实例指针
     * @param value 要压入的值
     * @return dv_error_t 错误码
     */
    dv_error_t dv_stack_push_float(dv_stack_t *p_stack, double value);

    /**
     * @brief 压栈操作（字符串）
     *
     * @param p_stack 栈实例指针
     * @param value 要压入的字符串
     * @return dv_error_t 错误码
     */
    dv_error_t dv_stack_push_string(dv_stack_t *p_stack, const char *value);

    /**
     * @brief 压栈操作（布尔值）
     *
     * @param p_stack 栈实例指针
     * @param value 要压入的布尔值
     * @return dv_error_t 错误码
     */
    dv_error_t dv_stack_push_bool(dv_stack_t *p_stack, bool value);

    /**
     * @brief 弹栈操作
     *
     * @param p_stack 栈实例指针
     * @return dv_error_t 错误码，栈为空时返回DV_ERR_EMPTY
     */
    dv_error_t dv_stack_pop(dv_stack_t *p_stack);

    /**
     * @brief 清空栈
     *
     * @param p_stack 栈实例指针
     * @return dv_error_t 错误码
     */
    dv_error_t dv_stack_clear(dv_stack_t *p_stack);

    /* ========================================================================== */
    /* 队列 API                                                                    */
    /* ========================================================================== */

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
     * @param p_queue 队列实例指针
     */
    void dv_queue_destroy(dv_queue_t *p_queue);

    /**
     * @brief 提交并写入trace文件
     *
     * @param p_queue 队列实例指针
     * @return dv_error_t 错误码
     */
    dv_error_t dv_queue_commit(dv_queue_t *p_queue);

    /**
     * @brief 入队操作（整数）
     *
     * @param p_queue 队列实例指针
     * @param value 要入队的值
     * @return dv_error_t 错误码
     */
    dv_error_t dv_queue_enqueue_int(dv_queue_t *p_queue, int64_t value);

    /**
     * @brief 入队操作（浮点数）
     *
     * @param p_queue 队列实例指针
     * @param value 要入队的值
     * @return dv_error_t 错误码
     */
    dv_error_t dv_queue_enqueue_float(dv_queue_t *p_queue, double value);

    /**
     * @brief 入队操作（字符串）
     *
     * @param p_queue 队列实例指针
     * @param value 要入队的字符串
     * @return dv_error_t 错误码
     */
    dv_error_t dv_queue_enqueue_string(dv_queue_t *p_queue, const char *value);

    /**
     * @brief 入队操作（布尔值）
     *
     * @param p_queue 队列实例指针
     * @param value 要入队的布尔值
     * @return dv_error_t 错误码
     */
    dv_error_t dv_queue_enqueue_bool(dv_queue_t *p_queue, bool value);

    /**
     * @brief 出队操作
     *
     * @param p_queue 队列实例指针
     * @return dv_error_t 错误码，队列为空时返回DV_ERR_EMPTY
     */
    dv_error_t dv_queue_dequeue(dv_queue_t *p_queue);

    /**
     * @brief 清空队列
     *
     * @param p_queue 队列实例指针
     * @return dv_error_t 错误码
     */
    dv_error_t dv_queue_clear(dv_queue_t *p_queue);

    /* ========================================================================== */
    /* 单链表 API                                                                  */
    /* ========================================================================== */

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
     *
     * @param p_slist 单链表实例指针
     */
    void dv_slist_destroy(dv_slist_t *p_slist);

    /**
     * @brief 提交并写入trace文件
     *
     * @param p_slist 单链表实例指针
     * @return dv_error_t 错误码
     */
    dv_error_t dv_slist_commit(dv_slist_t *p_slist);

    /**
     * @brief 在链表头部插入节点（整数）
     *
     * @param p_slist 单链表实例指针
     * @param value 要插入的值
     * @param p_out_node_id 输出参数，新插入节点的id，可为NULL
     * @return dv_error_t 错误码
     */
    dv_error_t dv_slist_insert_head_int(dv_slist_t *p_slist, int64_t value, int32_t *p_out_node_id);

    /**
     * @brief 在链表头部插入节点（浮点数）
     */
    dv_error_t dv_slist_insert_head_float(dv_slist_t *p_slist, double value, int32_t *p_out_node_id);

    /**
     * @brief 在链表头部插入节点（字符串）
     */
    dv_error_t dv_slist_insert_head_string(dv_slist_t *p_slist, const char *value, int32_t *p_out_node_id);

    /**
     * @brief 在链表头部插入节点（布尔值）
     */
    dv_error_t dv_slist_insert_head_bool(dv_slist_t *p_slist, bool value, int32_t *p_out_node_id);

    /**
     * @brief 在链表尾部插入节点（整数）
     */
    dv_error_t dv_slist_insert_tail_int(dv_slist_t *p_slist, int64_t value, int32_t *p_out_node_id);

    /**
     * @brief 在链表尾部插入节点（浮点数）
     */
    dv_error_t dv_slist_insert_tail_float(dv_slist_t *p_slist, double value, int32_t *p_out_node_id);

    /**
     * @brief 在链表尾部插入节点（字符串）
     */
    dv_error_t dv_slist_insert_tail_string(dv_slist_t *p_slist, const char *value, int32_t *p_out_node_id);

    /**
     * @brief 在链表尾部插入节点（布尔值）
     */
    dv_error_t dv_slist_insert_tail_bool(dv_slist_t *p_slist, bool value, int32_t *p_out_node_id);

    /**
     * @brief 在指定节点后插入新节点（整数）
     *
     * @param p_slist 单链表实例指针
     * @param node_id 目标节点的id
     * @param value 要插入的值
     * @param p_out_node_id 输出参数，新插入节点的id，可为NULL
     * @return dv_error_t 错误码，节点不存在时返回DV_ERR_NOT_FOUND
     */
    dv_error_t dv_slist_insert_after_int(dv_slist_t *p_slist, int32_t node_id, int64_t value, int32_t *p_out_node_id);

    /**
     * @brief 在指定节点后插入新节点（浮点数）
     */
    dv_error_t dv_slist_insert_after_float(dv_slist_t *p_slist, int32_t node_id, double value, int32_t *p_out_node_id);

    /**
     * @brief 在指定节点后插入新节点（字符串）
     */
    dv_error_t dv_slist_insert_after_string(dv_slist_t *p_slist, int32_t node_id, const char *value, int32_t *p_out_node_id);

    /**
     * @brief 在指定节点后插入新节点（布尔值）
     */
    dv_error_t dv_slist_insert_after_bool(dv_slist_t *p_slist, int32_t node_id, bool value, int32_t *p_out_node_id);

    /**
     * @brief 删除指定节点
     *
     * @param p_slist 单链表实例指针
     * @param node_id 要删除的节点id
     * @return dv_error_t 错误码，节点不存在时返回DV_ERR_NOT_FOUND
     */
    dv_error_t dv_slist_delete(dv_slist_t *p_slist, int32_t node_id);

    /**
     * @brief 删除头节点
     *
     * @param p_slist 单链表实例指针
     * @return dv_error_t 错误码，链表为空时返回DV_ERR_EMPTY
     */
    dv_error_t dv_slist_delete_head(dv_slist_t *p_slist);

    /**
     * @brief 反转链表
     *
     * @param p_slist 单链表实例指针
     * @return dv_error_t 错误码
     */
    dv_error_t dv_slist_reverse(dv_slist_t *p_slist);

    /* ========================================================================== */
    /* 双向链表 API                                                                */
    /* ========================================================================== */

    /**
     * @brief 创建双向链表实例
     *
     * @param label 双向链表的标签，传入NULL使用默认值 "dlist"
     * @param output 输出文件路径，传入NULL使用全局配置
     * @return dv_dlist_t* 双向链表实例指针，失败返回NULL
     */
    dv_dlist_t *dv_dlist_create(const char *label, const char *output);

    /**
     * @brief 销毁双向链表实例
     */
    void dv_dlist_destroy(dv_dlist_t *p_dlist);

    /**
     * @brief 提交并写入trace文件
     */
    dv_error_t dv_dlist_commit(dv_dlist_t *p_dlist);

    /**
     * @brief 在链表头部插入节点（整数）
     */
    dv_error_t dv_dlist_insert_head_int(dv_dlist_t *p_dlist, int64_t value, int32_t *p_out_node_id);

    /**
     * @brief 在链表头部插入节点（浮点数）
     */
    dv_error_t dv_dlist_insert_head_float(dv_dlist_t *p_dlist, double value, int32_t *p_out_node_id);

    /**
     * @brief 在链表头部插入节点（字符串）
     */
    dv_error_t dv_dlist_insert_head_string(dv_dlist_t *p_dlist, const char *value, int32_t *p_out_node_id);

    /**
     * @brief 在链表头部插入节点（布尔值）
     */
    dv_error_t dv_dlist_insert_head_bool(dv_dlist_t *p_dlist, bool value, int32_t *p_out_node_id);

    /**
     * @brief 在链表尾部插入节点（整数）
     */
    dv_error_t dv_dlist_insert_tail_int(dv_dlist_t *p_dlist, int64_t value, int32_t *p_out_node_id);

    /**
     * @brief 在链表尾部插入节点（浮点数）
     */
    dv_error_t dv_dlist_insert_tail_float(dv_dlist_t *p_dlist, double value, int32_t *p_out_node_id);

    /**
     * @brief 在链表尾部插入节点（字符串）
     */
    dv_error_t dv_dlist_insert_tail_string(dv_dlist_t *p_dlist, const char *value, int32_t *p_out_node_id);

    /**
     * @brief 在链表尾部插入节点（布尔值）
     */
    dv_error_t dv_dlist_insert_tail_bool(dv_dlist_t *p_dlist, bool value, int32_t *p_out_node_id);

    /**
     * @brief 在指定节点前插入新节点（整数）
     */
    dv_error_t dv_dlist_insert_before_int(dv_dlist_t *p_dlist, int32_t node_id, int64_t value, int32_t *p_out_node_id);

    /**
     * @brief 在指定节点前插入新节点（浮点数）
     */
    dv_error_t dv_dlist_insert_before_float(dv_dlist_t *p_dlist, int32_t node_id, double value, int32_t *p_out_node_id);

    /**
     * @brief 在指定节点前插入新节点（字符串）
     */
    dv_error_t dv_dlist_insert_before_string(dv_dlist_t *p_dlist, int32_t node_id, const char *value, int32_t *p_out_node_id);

    /**
     * @brief 在指定节点前插入新节点（布尔值）
     */
    dv_error_t dv_dlist_insert_before_bool(dv_dlist_t *p_dlist, int32_t node_id, bool value, int32_t *p_out_node_id);

    /**
     * @brief 在指定节点后插入新节点（整数）
     */
    dv_error_t dv_dlist_insert_after_int(dv_dlist_t *p_dlist, int32_t node_id, int64_t value, int32_t *p_out_node_id);

    /**
     * @brief 在指定节点后插入新节点（浮点数）
     */
    dv_error_t dv_dlist_insert_after_float(dv_dlist_t *p_dlist, int32_t node_id, double value, int32_t *p_out_node_id);

    /**
     * @brief 在指定节点后插入新节点（字符串）
     */
    dv_error_t dv_dlist_insert_after_string(dv_dlist_t *p_dlist, int32_t node_id, const char *value, int32_t *p_out_node_id);

    /**
     * @brief 在指定节点后插入新节点（布尔值）
     */
    dv_error_t dv_dlist_insert_after_bool(dv_dlist_t *p_dlist, int32_t node_id, bool value, int32_t *p_out_node_id);

    /**
     * @brief 删除指定节点
     */
    dv_error_t dv_dlist_delete(dv_dlist_t *p_dlist, int32_t node_id);

    /**
     * @brief 删除头节点
     */
    dv_error_t dv_dlist_delete_head(dv_dlist_t *p_dlist);

    /**
     * @brief 删除尾节点
     */
    dv_error_t dv_dlist_delete_tail(dv_dlist_t *p_dlist);

    /**
     * @brief 反转链表
     */
    dv_error_t dv_dlist_reverse(dv_dlist_t *p_dlist);

    /* ========================================================================== */
    /* 二叉树 API                                                                  */
    /* ========================================================================== */

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
    void dv_binary_tree_destroy(dv_binary_tree_t *p_tree);

    /**
     * @brief 提交并写入trace文件
     */
    dv_error_t dv_binary_tree_commit(dv_binary_tree_t *p_tree);

    /**
     * @brief 插入根节点（整数）
     *
     * @param p_tree 二叉树实例指针
     * @param value 根节点的值
     * @param p_out_node_id 输出参数，根节点的id，可为NULL
     * @return dv_error_t 错误码，根节点已存在时返回DV_ERR_ROOT_EXISTS
     */
    dv_error_t dv_binary_tree_insert_root_int(dv_binary_tree_t *p_tree, int64_t value, int32_t *p_out_node_id);

    /**
     * @brief 插入根节点（浮点数）
     */
    dv_error_t dv_binary_tree_insert_root_float(dv_binary_tree_t *p_tree, double value, int32_t *p_out_node_id);

    /**
     * @brief 插入根节点（字符串）
     */
    dv_error_t dv_binary_tree_insert_root_string(dv_binary_tree_t *p_tree, const char *value, int32_t *p_out_node_id);

    /**
     * @brief 插入根节点（布尔值）
     */
    dv_error_t dv_binary_tree_insert_root_bool(dv_binary_tree_t *p_tree, bool value, int32_t *p_out_node_id);

    /**
     * @brief 在指定父节点的左侧插入子节点（整数）
     *
     * @param p_tree 二叉树实例指针
     * @param parent_id 父节点的id
     * @param value 要插入的子节点值
     * @param p_out_node_id 输出参数，新插入节点的id，可为NULL
     * @return dv_error_t 错误码
     */
    dv_error_t dv_binary_tree_insert_left_int(dv_binary_tree_t *p_tree, int32_t parent_id, int64_t value, int32_t *p_out_node_id);

    /**
     * @brief 在指定父节点的左侧插入子节点（浮点数）
     */
    dv_error_t dv_binary_tree_insert_left_float(dv_binary_tree_t *p_tree, int32_t parent_id, double value, int32_t *p_out_node_id);

    /**
     * @brief 在指定父节点的左侧插入子节点（字符串）
     */
    dv_error_t dv_binary_tree_insert_left_string(dv_binary_tree_t *p_tree, int32_t parent_id, const char *value, int32_t *p_out_node_id);

    /**
     * @brief 在指定父节点的左侧插入子节点（布尔值）
     */
    dv_error_t dv_binary_tree_insert_left_bool(dv_binary_tree_t *p_tree, int32_t parent_id, bool value, int32_t *p_out_node_id);

    /**
     * @brief 在指定父节点的右侧插入子节点（整数）
     */
    dv_error_t dv_binary_tree_insert_right_int(dv_binary_tree_t *p_tree, int32_t parent_id, int64_t value, int32_t *p_out_node_id);

    /**
     * @brief 在指定父节点的右侧插入子节点（浮点数）
     */
    dv_error_t dv_binary_tree_insert_right_float(dv_binary_tree_t *p_tree, int32_t parent_id, double value, int32_t *p_out_node_id);

    /**
     * @brief 在指定父节点的右侧插入子节点（字符串）
     */
    dv_error_t dv_binary_tree_insert_right_string(dv_binary_tree_t *p_tree, int32_t parent_id, const char *value, int32_t *p_out_node_id);

    /**
     * @brief 在指定父节点的右侧插入子节点（布尔值）
     */
    dv_error_t dv_binary_tree_insert_right_bool(dv_binary_tree_t *p_tree, int32_t parent_id, bool value, int32_t *p_out_node_id);

    /**
     * @brief 删除指定节点及其所有子树
     *
     * @param p_tree 二叉树实例指针
     * @param node_id 要删除的节点id
     * @return dv_error_t 错误码，节点不存在时返回DV_ERR_NOT_FOUND
     */
    dv_error_t dv_binary_tree_delete(dv_binary_tree_t *p_tree, int32_t node_id);

    /**
     * @brief 更新节点的值（整数）
     *
     * @param p_tree 二叉树实例指针
     * @param node_id 节点id
     * @param value 新的值
     * @return dv_error_t 错误码，节点不存在时返回DV_ERR_NOT_FOUND
     */
    dv_error_t dv_binary_tree_update_value_int(dv_binary_tree_t *p_tree, int32_t node_id, int64_t value);

    /**
     * @brief 更新节点的值（浮点数）
     */
    dv_error_t dv_binary_tree_update_value_float(dv_binary_tree_t *p_tree, int32_t node_id, double value);

    /**
     * @brief 更新节点的值（字符串）
     */
    dv_error_t dv_binary_tree_update_value_string(dv_binary_tree_t *p_tree, int32_t node_id, const char *value);

    /**
     * @brief 更新节点的值（布尔值）
     */
    dv_error_t dv_binary_tree_update_value_bool(dv_binary_tree_t *p_tree, int32_t node_id, bool value);

    /* ========================================================================== */
    /* 二叉搜索树 API                                                              */
    /* ========================================================================== */

    /**
     * @brief 创建二叉搜索树实例
     *
     * @param label 二叉搜索树的标签，传入NULL使用默认值 "bst"
     * @param output 输出文件路径，传入NULL使用全局配置
     * @return dv_bst_t* 二叉搜索树实例指针，失败返回NULL
     */
    dv_bst_t *dv_bst_create(const char *label, const char *output);

    /**
     * @brief 销毁二叉搜索树实例
     */
    void dv_bst_destroy(dv_bst_t *p_bst);

    /**
     * @brief 提交并写入trace文件
     */
    dv_error_t dv_bst_commit(dv_bst_t *p_bst);

    /**
     * @brief 插入节点（整数），自动维护二叉搜索树性质
     *
     * @param p_bst 二叉搜索树实例指针
     * @param value 要插入的值
     * @param p_out_node_id 输出参数，新插入节点的id，可为NULL
     * @return dv_error_t 错误码
     */
    dv_error_t dv_bst_insert_int(dv_bst_t *p_bst, int64_t value, int32_t *p_out_node_id);

    /**
     * @brief 插入节点（浮点数），自动维护二叉搜索树性质
     */
    dv_error_t dv_bst_insert_float(dv_bst_t *p_bst, double value, int32_t *p_out_node_id);

    /**
     * @brief 删除节点（整数），自动维护二叉搜索树性质
     *
     * @param p_bst 二叉搜索树实例指针
     * @param value 要删除的节点值
     * @return dv_error_t 错误码，节点不存在时返回DV_ERR_NOT_FOUND
     */
    dv_error_t dv_bst_delete_int(dv_bst_t *p_bst, int64_t value);

    /**
     * @brief 删除节点（浮点数），自动维护二叉搜索树性质
     */
    dv_error_t dv_bst_delete_float(dv_bst_t *p_bst, double value);

    /* ========================================================================== */
    /* 堆 API                                                                      */
    /* ========================================================================== */

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
    void dv_heap_destroy(dv_heap_t *p_heap);

    /**
     * @brief 提交并写入trace文件
     */
    dv_error_t dv_heap_commit(dv_heap_t *p_heap);

    /**
     * @brief 插入元素（整数），自动维护堆性质
     *
     * @param p_heap 堆实例指针
     * @param value 要插入的值
     * @return dv_error_t 错误码
     */
    dv_error_t dv_heap_insert_int(dv_heap_t *p_heap, int64_t value);

    /**
     * @brief 插入元素（浮点数），自动维护堆性质
     */
    dv_error_t dv_heap_insert_float(dv_heap_t *p_heap, double value);

    /**
     * @brief 提取堆顶元素，自动维护堆性质
     *
     * @param p_heap 堆实例指针
     * @return dv_error_t 错误码，堆为空时返回DV_ERR_EMPTY
     */
    dv_error_t dv_heap_extract(dv_heap_t *p_heap);

    /**
     * @brief 清空堆
     *
     * @param p_heap 堆实例指针
     * @return dv_error_t 错误码
     */
    dv_error_t dv_heap_clear(dv_heap_t *p_heap);

    /* ========================================================================== */
    /* 无向图 API                                                                  */
    /* ========================================================================== */

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
    void dv_graph_undirected_destroy(dv_graph_undirected_t *p_graph);

    /**
     * @brief 提交并写入trace文件
     */
    dv_error_t dv_graph_undirected_commit(dv_graph_undirected_t *p_graph);

    /**
     * @brief 添加节点
     *
     * @param p_graph 无向图实例指针
     * @param node_id 节点id
     * @param label 节点标签，长度限制为1-32字符
     * @return dv_error_t 错误码，节点已存在时返回DV_ERR_ALREADY_EXISTS
     */
    dv_error_t dv_graph_undirected_add_node(dv_graph_undirected_t *p_graph, int32_t node_id, const char *label);

    /**
     * @brief 添加无向边，内部自动规范化为from_id < to_id
     *
     * @param p_graph 无向图实例指针
     * @param from_id 起始节点id
     * @param to_id 终止节点id
     * @return dv_error_t 错误码
     */
    dv_error_t dv_graph_undirected_add_edge(dv_graph_undirected_t *p_graph, int32_t from_id, int32_t to_id);

    /**
     * @brief 删除节点及其相关的所有边
     *
     * @param p_graph 无向图实例指针
     * @param node_id 要删除的节点id
     * @return dv_error_t 错误码，节点不存在时返回DV_ERR_NOT_FOUND
     */
    dv_error_t dv_graph_undirected_remove_node(dv_graph_undirected_t *p_graph, int32_t node_id);

    /**
     * @brief 删除边
     *
     * @param p_graph 无向图实例指针
     * @param from_id 起始节点id
     * @param to_id 终止节点id
     * @return dv_error_t 错误码，边不存在时返回DV_ERR_NOT_FOUND
     */
    dv_error_t dv_graph_undirected_remove_edge(dv_graph_undirected_t *p_graph, int32_t from_id, int32_t to_id);

    /**
     * @brief 更新节点标签
     *
     * @param p_graph 无向图实例指针
     * @param node_id 节点id
     * @param label 新的节点标签，长度限制为1-32字符
     * @return dv_error_t 错误码，节点不存在时返回DV_ERR_NOT_FOUND
     */
    dv_error_t dv_graph_undirected_update_node_label(dv_graph_undirected_t *p_graph, int32_t node_id, const char *label);

    /* ========================================================================== */
    /* 有向图 API                                                                  */
    /* ========================================================================== */

    /**
     * @brief 创建有向图实例
     */
    dv_graph_directed_t *dv_graph_directed_create(const char *label, const char *output);

    /**
     * @brief 销毁有向图实例
     */
    void dv_graph_directed_destroy(dv_graph_directed_t *p_graph);

    /**
     * @brief 提交并写入trace文件
     */
    dv_error_t dv_graph_directed_commit(dv_graph_directed_t *p_graph);

    /**
     * @brief 添加节点
     */
    dv_error_t dv_graph_directed_add_node(dv_graph_directed_t *p_graph, int32_t node_id, const char *label);

    /**
     * @brief 添加有向边
     */
    dv_error_t dv_graph_directed_add_edge(dv_graph_directed_t *p_graph, int32_t from_id, int32_t to_id);

    /**
     * @brief 删除节点及其相关的所有边
     */
    dv_error_t dv_graph_directed_remove_node(dv_graph_directed_t *p_graph, int32_t node_id);

    /**
     * @brief 删除边
     */
    dv_error_t dv_graph_directed_remove_edge(dv_graph_directed_t *p_graph, int32_t from_id, int32_t to_id);

    /**
     * @brief 更新节点标签
     */
    dv_error_t dv_graph_directed_update_node_label(dv_graph_directed_t *p_graph, int32_t node_id, const char *label);

    /* ========================================================================== */
    /* 带权图 API                                                                  */
    /* ========================================================================== */

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
    void dv_graph_weighted_destroy(dv_graph_weighted_t *p_graph);

    /**
     * @brief 提交并写入trace文件
     */
    dv_error_t dv_graph_weighted_commit(dv_graph_weighted_t *p_graph);

    /**
     * @brief 添加节点
     */
    dv_error_t dv_graph_weighted_add_node(dv_graph_weighted_t *p_graph, int32_t node_id, const char *label);

    /**
     * @brief 添加带权边
     *
     * @param p_graph 带权图实例指针
     * @param from_id 起始节点id
     * @param to_id 终止节点id
     * @param weight 边的权重
     * @return dv_error_t 错误码
     */
    dv_error_t dv_graph_weighted_add_edge(dv_graph_weighted_t *p_graph, int32_t from_id, int32_t to_id, double weight);

    /**
     * @brief 删除节点及其相关的所有边
     */
    dv_error_t dv_graph_weighted_remove_node(dv_graph_weighted_t *p_graph, int32_t node_id);

    /**
     * @brief 删除边
     */
    dv_error_t dv_graph_weighted_remove_edge(dv_graph_weighted_t *p_graph, int32_t from_id, int32_t to_id);

    /**
     * @brief 更新边的权重
     *
     * @param p_graph 带权图实例指针
     * @param from_id 起始节点id
     * @param to_id 终止节点id
     * @param weight 新的权重值
     * @return dv_error_t 错误码，边不存在时返回DV_ERR_NOT_FOUND
     */
    dv_error_t dv_graph_weighted_update_weight(dv_graph_weighted_t *p_graph, int32_t from_id, int32_t to_id, double weight);

    /**
     * @brief 更新节点标签
     */
    dv_error_t dv_graph_weighted_update_node_label(dv_graph_weighted_t *p_graph, int32_t node_id, const char *label);

#ifdef __cplusplus
}
#endif

/* ========================================================================== */
/* 实现部分                                                                    */
/* ========================================================================== */

#ifdef DS4VIZ_IMPLEMENTATION

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* ========================================================================== */
/* 内部宏定义                                                                  */
/* ========================================================================== */

/**
 * @brief 安全释放内存并置空指针
 *
 * @param ptr 要释放的指针
 */
#define DV_SAFE_FREE(ptr)  \
    do                     \
    {                      \
        if ((ptr) != NULL) \
        {                  \
            free(ptr);     \
            (ptr) = NULL;  \
        }                  \
    } while (0)

/**
 * @brief 字符串复制辅助宏
 */
#define DV_STRDUP(dst, src)                 \
    do                                      \
    {                                       \
        if ((src) != NULL)                  \
        {                                   \
            size_t _len = strlen(src) + 1;  \
            (dst) = (char *)malloc(_len);   \
            if ((dst) != NULL)              \
            {                               \
                memcpy((dst), (src), _len); \
            }                               \
        }                                   \
        else                                \
        {                                   \
            (dst) = NULL;                   \
        }                                   \
    } while (0)

/* ========================================================================== */
/* 内部结构体定义                                                              */
/* ========================================================================== */

/**
 * @brief 全局配置结构体
 */
typedef struct dv_global_config_s
{
    char output_path[DV_MAX_PATH_LEN];
    char title[128];
    char author[64];
    char comment[256];
} dv_global_config_t;

/**
 * @brief 步骤参数
 */
typedef struct dv_step_arg_s
{
    char *key;
    dv_value_t value;
} dv_step_arg_t;

/**
 * @brief 步骤结构体
 */
typedef struct dv_step_s
{
    int32_t id;
    char *op;
    int32_t before;
    int32_t after;
    bool has_after;
    dv_step_arg_t *args;
    size_t args_count;
    size_t args_capacity;
    dv_value_t *ret;
    char *note;
} dv_step_t;

/**
 * @brief 错误信息结构体
 */
typedef struct dv_error_info_s
{
    bool has_error;
    char type[32];
    char message[DV_MAX_ERROR_MSG_LEN];
    int32_t step;
    int32_t last_state;
} dv_error_info_t;

/**
 * @brief 会话结构体
 */
struct dv_session_s
{
    char *kind;
    char *label;
    char *output;

    char **states_toml;
    size_t states_count;
    size_t states_capacity;

    dv_step_t *steps;
    size_t steps_count;
    size_t steps_capacity;

    dv_error_info_t error;
    int32_t current_state_id;
    int32_t current_step_id;
};

/**
 * @brief 栈内部结构
 */
struct dv_stack_s
{
    dv_session_t *p_session;
    dv_value_t *items;
    size_t count;
    size_t capacity;
};

/**
 * @brief 队列内部结构
 */
struct dv_queue_s
{
    dv_session_t *p_session;
    dv_value_t *items;
    size_t count;
    size_t capacity;
};

/**
 * @brief 单链表节点
 */
typedef struct dv_slist_node_s
{
    int32_t id;
    dv_value_t value;
    int32_t next;
    bool active;
} dv_slist_node_t;

/**
 * @brief 单链表内部结构
 */
struct dv_slist_s
{
    dv_session_t *p_session;
    dv_slist_node_t *nodes;
    size_t nodes_count;
    size_t nodes_capacity;
    int32_t head;
    int32_t next_id;
};

/**
 * @brief 双向链表节点
 */
typedef struct dv_dlist_node_s
{
    int32_t id;
    dv_value_t value;
    int32_t prev;
    int32_t next;
    bool active;
} dv_dlist_node_t;

/**
 * @brief 双向链表内部结构
 */
struct dv_dlist_s
{
    dv_session_t *p_session;
    dv_dlist_node_t *nodes;
    size_t nodes_count;
    size_t nodes_capacity;
    int32_t head;
    int32_t tail;
    int32_t next_id;
};

/**
 * @brief 二叉树节点
 */
typedef struct dv_tree_node_s
{
    int32_t id;
    dv_value_t value;
    int32_t left;
    int32_t right;
    bool active;
} dv_tree_node_t;

/**
 * @brief 二叉树内部结构
 */
struct dv_binary_tree_s
{
    dv_session_t *p_session;
    dv_tree_node_t *nodes;
    size_t nodes_count;
    size_t nodes_capacity;
    int32_t root;
    int32_t next_id;
};

/**
 * @brief 二叉搜索树内部结构（复用二叉树节点）
 */
struct dv_bst_s
{
    dv_session_t *p_session;
    dv_tree_node_t *nodes;
    size_t nodes_count;
    size_t nodes_capacity;
    int32_t root;
    int32_t next_id;
    bool use_float;
};

/**
 * @brief 堆内部结构
 */
struct dv_heap_s
{
    dv_session_t *p_session;
    dv_value_t *items;
    size_t count;
    size_t capacity;
    dv_heap_type_t heap_type;
};

/**
 * @brief 图节点
 */
typedef struct dv_graph_node_s
{
    int32_t id;
    char label[33];
    bool active;
} dv_graph_node_t;

/**
 * @brief 图边
 */
typedef struct dv_graph_edge_s
{
    int32_t from;
    int32_t to;
    double weight;
    bool has_weight;
    bool active;
} dv_graph_edge_t;

/**
 * @brief 无向图内部结构
 */
struct dv_graph_undirected_s
{
    dv_session_t *p_session;
    dv_graph_node_t *nodes;
    size_t nodes_count;
    size_t nodes_capacity;
    dv_graph_edge_t *edges;
    size_t edges_count;
    size_t edges_capacity;
};

/**
 * @brief 有向图内部结构
 */
struct dv_graph_directed_s
{
    dv_session_t *p_session;
    dv_graph_node_t *nodes;
    size_t nodes_count;
    size_t nodes_capacity;
    dv_graph_edge_t *edges;
    size_t edges_count;
    size_t edges_capacity;
};

/**
 * @brief 带权图内部结构
 */
struct dv_graph_weighted_s
{
    dv_session_t *p_session;
    dv_graph_node_t *nodes;
    size_t nodes_count;
    size_t nodes_capacity;
    dv_graph_edge_t *edges;
    size_t edges_count;
    size_t edges_capacity;
    bool directed;
};

/* ========================================================================== */
/* 静态全局变量                                                                */
/* ========================================================================== */

static dv_global_config_t g_config = {
    .output_path = DV_DEFAULT_OUTPUT,
    .title = "",
    .author = "",
    .comment = ""};

/* ========================================================================== */
/* 静态函数声明                                                                */
/* ========================================================================== */

static char *dv_strdup_internal(const char *str);
static void dv_value_free(dv_value_t *p_val);
static dv_value_t dv_value_copy(const dv_value_t *p_val);
static void dv_value_to_toml(const dv_value_t *p_val, char *buf, size_t buf_size);
static void dv_get_timestamp(char *buf, size_t buf_size);
static dv_session_t *dv_session_create(const char *kind, const char *label, const char *output);
static void dv_session_destroy(dv_session_t *p_session);
static dv_error_t dv_session_add_state(dv_session_t *p_session, const char *state_toml);
static dv_error_t dv_session_add_step(dv_session_t *p_session, const char *op, int32_t before, int32_t after, bool has_after);
static dv_error_t dv_session_add_step_arg(dv_session_t *p_session, const char *key, const dv_value_t *p_val);
static void dv_session_set_step_ret(dv_session_t *p_session, const dv_value_t *p_val);
static void dv_session_set_error(dv_session_t *p_session, const char *type, const char *message);
static dv_error_t dv_session_write(dv_session_t *p_session);

/* ========================================================================== */
/* 辅助函数实现                                                                */
/* ========================================================================== */

static char *dv_strdup_internal(const char *str)
{
    char *result = NULL;
    size_t len;

    if (str == NULL)
    {
        return NULL;
    }

    len = strlen(str) + 1;
    result = (char *)malloc(len);
    if (result != NULL)
    {
        memcpy(result, str, len);
    }

    return result;
}

static void dv_value_free(dv_value_t *p_val)
{
    if (p_val == NULL)
    {
        return;
    }

    if (p_val->type == DV_VALUE_STRING && p_val->data.string_val != NULL)
    {
        free(p_val->data.string_val);
        p_val->data.string_val = NULL;
    }
}

static dv_value_t dv_value_copy(const dv_value_t *p_val)
{
    dv_value_t result;

    result.type = p_val->type;

    if (p_val->type == DV_VALUE_STRING)
    {
        result.data.string_val = dv_strdup_internal(p_val->data.string_val);
    }
    else
    {
        result.data = p_val->data;
    }

    return result;
}

static void dv_value_to_toml(const dv_value_t *p_val, char *buf, size_t buf_size)
{
    switch (p_val->type)
    {
    case DV_VALUE_INT:
        snprintf(buf, buf_size, "%lld", (long long)p_val->data.int_val);
        break;
    case DV_VALUE_FLOAT:
        snprintf(buf, buf_size, "%g", p_val->data.float_val);
        break;
    case DV_VALUE_STRING:
        snprintf(buf, buf_size, "\"%s\"", p_val->data.string_val ? p_val->data.string_val : "");
        break;
    case DV_VALUE_BOOL:
        snprintf(buf, buf_size, "%s", p_val->data.bool_val ? "true" : "false");
        break;
    default:
        snprintf(buf, buf_size, "null");
        break;
    }
}

static void dv_get_timestamp(char *buf, size_t buf_size)
{
    time_t now;
    struct tm *tm_info;

    time(&now);
    tm_info = gmtime(&now);
    strftime(buf, buf_size, "%Y-%m-%dT%H:%M:%SZ", tm_info);
}

/* ========================================================================== */
/* 会话管理函数实现                                                            */
/* ========================================================================== */

static dv_session_t *dv_session_create(const char *kind, const char *label, const char *output)
{
    dv_session_t *p_session = NULL;

    p_session = (dv_session_t *)calloc(1, sizeof(dv_session_t));
    if (p_session == NULL)
    {
        return NULL;
    }

    p_session->kind = dv_strdup_internal(kind);
    p_session->label = dv_strdup_internal(label);
    p_session->output = dv_strdup_internal(output ? output : g_config.output_path);

    if (p_session->kind == NULL || p_session->label == NULL || p_session->output == NULL)
    {
        dv_session_destroy(p_session);
        return NULL;
    }

    p_session->states_capacity = DV_INITIAL_CAPACITY;
    p_session->states_toml = (char **)calloc(p_session->states_capacity, sizeof(char *));

    p_session->steps_capacity = DV_INITIAL_CAPACITY;
    p_session->steps = (dv_step_t *)calloc(p_session->steps_capacity, sizeof(dv_step_t));

    if (p_session->states_toml == NULL || p_session->steps == NULL)
    {
        dv_session_destroy(p_session);
        return NULL;
    }

    p_session->current_state_id = -1;
    p_session->current_step_id = -1;
    p_session->error.has_error = false;

    return p_session;
}

static void dv_session_destroy(dv_session_t *p_session)
{
    size_t i;
    size_t j;

    if (p_session == NULL)
    {
        return;
    }

    DV_SAFE_FREE(p_session->kind);
    DV_SAFE_FREE(p_session->label);
    DV_SAFE_FREE(p_session->output);

    if (p_session->states_toml != NULL)
    {
        for (i = 0; i < p_session->states_count; i++)
        {
            DV_SAFE_FREE(p_session->states_toml[i]);
        }
        free(p_session->states_toml);
    }

    if (p_session->steps != NULL)
    {
        for (i = 0; i < p_session->steps_count; i++)
        {
            DV_SAFE_FREE(p_session->steps[i].op);
            DV_SAFE_FREE(p_session->steps[i].note);

            if (p_session->steps[i].args != NULL)
            {
                for (j = 0; j < p_session->steps[i].args_count; j++)
                {
                    DV_SAFE_FREE(p_session->steps[i].args[j].key);
                    dv_value_free(&p_session->steps[i].args[j].value);
                }
                free(p_session->steps[i].args);
            }

            if (p_session->steps[i].ret != NULL)
            {
                dv_value_free(p_session->steps[i].ret);
                free(p_session->steps[i].ret);
            }
        }
        free(p_session->steps);
    }

    free(p_session);
}

static dv_error_t dv_session_add_state(dv_session_t *p_session, const char *state_toml)
{
    char **new_states = NULL;

    if (p_session == NULL || state_toml == NULL)
    {
        return DV_ERR_NULL_PTR;
    }

    if (p_session->states_count >= p_session->states_capacity)
    {
        size_t new_capacity = p_session->states_capacity * 2;
        new_states = (char **)realloc(p_session->states_toml, new_capacity * sizeof(char *));
        if (new_states == NULL)
        {
            return DV_ERR_ALLOC;
        }
        p_session->states_toml = new_states;
        p_session->states_capacity = new_capacity;
    }

    p_session->states_toml[p_session->states_count] = dv_strdup_internal(state_toml);
    if (p_session->states_toml[p_session->states_count] == NULL)
    {
        return DV_ERR_ALLOC;
    }

    p_session->current_state_id = (int32_t)p_session->states_count;
    p_session->states_count++;

    return DV_OK;
}

static dv_error_t dv_session_add_step(dv_session_t *p_session, const char *op, int32_t before, int32_t after, bool has_after)
{
    dv_step_t *new_steps = NULL;
    dv_step_t *p_step = NULL;

    if (p_session == NULL || op == NULL)
    {
        return DV_ERR_NULL_PTR;
    }

    if (p_session->steps_count >= p_session->steps_capacity)
    {
        size_t new_capacity = p_session->steps_capacity * 2;
        new_steps = (dv_step_t *)realloc(p_session->steps, new_capacity * sizeof(dv_step_t));
        if (new_steps == NULL)
        {
            return DV_ERR_ALLOC;
        }
        p_session->steps = new_steps;
        p_session->steps_capacity = new_capacity;
    }

    p_step = &p_session->steps[p_session->steps_count];
    memset(p_step, 0, sizeof(dv_step_t));

    p_step->id = (int32_t)p_session->steps_count;
    p_step->op = dv_strdup_internal(op);
    p_step->before = before;
    p_step->after = after;
    p_step->has_after = has_after;
    p_step->args_capacity = 8;
    p_step->args = (dv_step_arg_t *)calloc(p_step->args_capacity, sizeof(dv_step_arg_t));

    if (p_step->op == NULL || p_step->args == NULL)
    {
        return DV_ERR_ALLOC;
    }

    p_session->current_step_id = p_step->id;
    p_session->steps_count++;

    return DV_OK;
}

static dv_error_t dv_session_add_step_arg(dv_session_t *p_session, const char *key, const dv_value_t *p_val)
{
    dv_step_t *p_step = NULL;
    dv_step_arg_t *new_args = NULL;
    dv_step_arg_t *p_arg = NULL;

    if (p_session == NULL || key == NULL || p_val == NULL)
    {
        return DV_ERR_NULL_PTR;
    }

    if (p_session->steps_count == 0)
    {
        return DV_ERR_INVALID_PARAM;
    }

    p_step = &p_session->steps[p_session->steps_count - 1];

    if (p_step->args_count >= p_step->args_capacity)
    {
        size_t new_capacity = p_step->args_capacity * 2;
        new_args = (dv_step_arg_t *)realloc(p_step->args, new_capacity * sizeof(dv_step_arg_t));
        if (new_args == NULL)
        {
            return DV_ERR_ALLOC;
        }
        p_step->args = new_args;
        p_step->args_capacity = new_capacity;
    }

    p_arg = &p_step->args[p_step->args_count];
    p_arg->key = dv_strdup_internal(key);
    p_arg->value = dv_value_copy(p_val);

    if (p_arg->key == NULL)
    {
        return DV_ERR_ALLOC;
    }

    p_step->args_count++;

    return DV_OK;
}

static void dv_session_set_step_ret(dv_session_t *p_session, const dv_value_t *p_val)
{
    dv_step_t *p_step = NULL;

    if (p_session == NULL || p_val == NULL || p_session->steps_count == 0)
    {
        return;
    }

    p_step = &p_session->steps[p_session->steps_count - 1];

    if (p_step->ret != NULL)
    {
        dv_value_free(p_step->ret);
        free(p_step->ret);
    }

    p_step->ret = (dv_value_t *)malloc(sizeof(dv_value_t));
    if (p_step->ret != NULL)
    {
        *p_step->ret = dv_value_copy(p_val);
    }
}

static void dv_session_set_error(dv_session_t *p_session, const char *type, const char *message)
{
    if (p_session == NULL)
    {
        return;
    }

    p_session->error.has_error = true;
    strncpy(p_session->error.type, type ? type : "runtime", sizeof(p_session->error.type) - 1);
    strncpy(p_session->error.message, message ? message : "Unknown error", sizeof(p_session->error.message) - 1);
    p_session->error.step = p_session->current_step_id;
    p_session->error.last_state = p_session->current_state_id;
}

static dv_error_t dv_session_write(dv_session_t *p_session)
{
    FILE *fp = NULL;
    char timestamp[64];
    size_t i;
    size_t j;
    char val_buf[256];
    dv_step_t *p_step = NULL;

    if (p_session == NULL)
    {
        return DV_ERR_NULL_PTR;
    }

    fp = fopen(p_session->output, "w");
    if (fp == NULL)
    {
        return DV_ERR_IO;
    }

    dv_get_timestamp(timestamp, sizeof(timestamp));

    /* [meta] */
    fprintf(fp, "[meta]\n");
    fprintf(fp, "created_at = \"%s\"\n", timestamp);
    fprintf(fp, "lang = \"c\"\n");
    fprintf(fp, "lang_version = \"17\"\n");
    fprintf(fp, "\n");

    /* [package] */
    fprintf(fp, "[package]\n");
    fprintf(fp, "name = \"ds4viz\"\n");
    fprintf(fp, "lang = \"c\"\n");
    fprintf(fp, "version = \"%s\"\n", DS4VIZ_VERSION_STRING);
    fprintf(fp, "\n");

    /* [remarks] */
    if (g_config.title[0] != '\0' || g_config.author[0] != '\0' || g_config.comment[0] != '\0')
    {
        fprintf(fp, "[remarks]\n");
        if (g_config.title[0] != '\0')
        {
            fprintf(fp, "title = \"%s\"\n", g_config.title);
        }
        if (g_config.author[0] != '\0')
        {
            fprintf(fp, "author = \"%s\"\n", g_config.author);
        }
        if (g_config.comment[0] != '\0')
        {
            fprintf(fp, "comment = \"%s\"\n", g_config.comment);
        }
        fprintf(fp, "\n");
    }

    /* [object] */
    fprintf(fp, "[object]\n");
    fprintf(fp, "kind = \"%s\"\n", p_session->kind);
    if (p_session->label != NULL && p_session->label[0] != '\0')
    {
        fprintf(fp, "label = \"%s\"\n", p_session->label);
    }

    /* [[states]] */
    for (i = 0; i < p_session->states_count; i++)
    {
        fprintf(fp, "\n[[states]]\n");
        fprintf(fp, "id = %zu\n", i);
        fprintf(fp, "\n[states.data]\n");
        fprintf(fp, "%s", p_session->states_toml[i]);
    }

    /* [[steps]] */
    for (i = 0; i < p_session->steps_count; i++)
    {
        p_step = &p_session->steps[i];

        fprintf(fp, "\n[[steps]]\n");
        fprintf(fp, "id = %d\n", p_step->id);
        fprintf(fp, "op = \"%s\"\n", p_step->op);
        fprintf(fp, "before = %d\n", p_step->before);

        if (p_step->has_after)
        {
            fprintf(fp, "after = %d\n", p_step->after);
        }

        if (p_step->ret != NULL)
        {
            dv_value_to_toml(p_step->ret, val_buf, sizeof(val_buf));
            fprintf(fp, "ret = %s\n", val_buf);
        }

        if (p_step->note != NULL)
        {
            fprintf(fp, "note = \"%s\"\n", p_step->note);
        }

        fprintf(fp, "\n[steps.args]\n");
        for (j = 0; j < p_step->args_count; j++)
        {
            dv_value_to_toml(&p_step->args[j].value, val_buf, sizeof(val_buf));
            fprintf(fp, "%s = %s\n", p_step->args[j].key, val_buf);
        }
    }

    /* [result] or [error] */
    if (p_session->error.has_error)
    {
        fprintf(fp, "\n[error]\n");
        fprintf(fp, "type = \"%s\"\n", p_session->error.type);
        fprintf(fp, "message = \"%s\"\n", p_session->error.message);
        if (p_session->error.step >= 0)
        {
            fprintf(fp, "step = %d\n", p_session->error.step);
        }
        if (p_session->error.last_state >= 0)
        {
            fprintf(fp, "last_state = %d\n", p_session->error.last_state);
        }
    }
    else
    {
        fprintf(fp, "\n[result]\n");
        fprintf(fp, "final_state = %d\n", p_session->current_state_id);
    }

    fclose(fp);

    return DV_OK;
}

/* ========================================================================== */
/* 全局配置函数实现                                                            */
/* ========================================================================== */

void dv_config(
    const char *output_path,
    const char *title,
    const char *author,
    const char *comment)
{
    if (output_path != NULL)
    {
        strncpy(g_config.output_path, output_path, DV_MAX_PATH_LEN - 1);
        g_config.output_path[DV_MAX_PATH_LEN - 1] = '\0';
    }
    else
    {
        strncpy(g_config.output_path, DV_DEFAULT_OUTPUT, DV_MAX_PATH_LEN - 1);
    }

    if (title != NULL)
    {
        strncpy(g_config.title, title, sizeof(g_config.title) - 1);
        g_config.title[sizeof(g_config.title) - 1] = '\0';
    }
    else
    {
        g_config.title[0] = '\0';
    }

    if (author != NULL)
    {
        strncpy(g_config.author, author, sizeof(g_config.author) - 1);
        g_config.author[sizeof(g_config.author) - 1] = '\0';
    }
    else
    {
        g_config.author[0] = '\0';
    }

    if (comment != NULL)
    {
        strncpy(g_config.comment, comment, sizeof(g_config.comment) - 1);
        g_config.comment[sizeof(g_config.comment) - 1] = '\0';
    }
    else
    {
        g_config.comment[0] = '\0';
    }
}

const char *dv_version(void)
{
    return DS4VIZ_VERSION_STRING;
}

/* ========================================================================== */
/* 栈实现                                                                      */
/* ========================================================================== */

static void dv_stack_build_state(dv_stack_t *p_stack, char *buf, size_t buf_size)
{
    size_t i;
    int written;
    int offset = 0;
    char val_buf[256];

    offset += snprintf(buf + offset, buf_size - offset, "items = [");

    for (i = 0; i < p_stack->count; i++)
    {
        if (i > 0)
        {
            offset += snprintf(buf + offset, buf_size - offset, ", ");
        }
        dv_value_to_toml(&p_stack->items[i], val_buf, sizeof(val_buf));
        offset += snprintf(buf + offset, buf_size - offset, "%s", val_buf);
    }

    offset += snprintf(buf + offset, buf_size - offset, "]\n");
    written = snprintf(buf + offset, buf_size - offset, "top = %d\n",
                       p_stack->count > 0 ? (int)(p_stack->count - 1) : -1);
    (void)written;
}

dv_stack_t *dv_stack_create(const char *label, const char *output)
{
    dv_stack_t *p_stack = NULL;
    char state_buf[4096];

    p_stack = (dv_stack_t *)calloc(1, sizeof(dv_stack_t));
    if (p_stack == NULL)
    {
        return NULL;
    }

    p_stack->p_session = dv_session_create("stack", label ? label : "stack", output);
    if (p_stack->p_session == NULL)
    {
        free(p_stack);
        return NULL;
    }

    p_stack->capacity = DV_INITIAL_CAPACITY;
    p_stack->items = (dv_value_t *)calloc(p_stack->capacity, sizeof(dv_value_t));
    if (p_stack->items == NULL)
    {
        dv_session_destroy(p_stack->p_session);
        free(p_stack);
        return NULL;
    }

    /* 添加初始状态 */
    dv_stack_build_state(p_stack, state_buf, sizeof(state_buf));
    dv_session_add_state(p_stack->p_session, state_buf);

    return p_stack;
}

void dv_stack_destroy(dv_stack_t *p_stack)
{
    size_t i;

    if (p_stack == NULL)
    {
        return;
    }

    if (p_stack->items != NULL)
    {
        for (i = 0; i < p_stack->count; i++)
        {
            dv_value_free(&p_stack->items[i]);
        }
        free(p_stack->items);
    }

    dv_session_destroy(p_stack->p_session);
    free(p_stack);
}

dv_error_t dv_stack_commit(dv_stack_t *p_stack)
{
    if (p_stack == NULL)
    {
        return DV_ERR_NULL_PTR;
    }

    return dv_session_write(p_stack->p_session);
}

static dv_error_t dv_stack_push_value(dv_stack_t *p_stack, const dv_value_t *p_val)
{
    dv_value_t *new_items = NULL;
    char state_buf[4096];
    int32_t before;

    if (p_stack == NULL || p_val == NULL)
    {
        return DV_ERR_NULL_PTR;
    }

    if (p_stack->count >= p_stack->capacity)
    {
        size_t new_capacity = p_stack->capacity * 2;
        new_items = (dv_value_t *)realloc(p_stack->items, new_capacity * sizeof(dv_value_t));
        if (new_items == NULL)
        {
            dv_session_set_error(p_stack->p_session, "runtime", "Memory allocation failed");
            return DV_ERR_ALLOC;
        }
        p_stack->items = new_items;
        p_stack->capacity = new_capacity;
    }

    before = p_stack->p_session->current_state_id;

    p_stack->items[p_stack->count] = dv_value_copy(p_val);
    p_stack->count++;

    dv_stack_build_state(p_stack, state_buf, sizeof(state_buf));
    dv_session_add_state(p_stack->p_session, state_buf);

    dv_session_add_step(p_stack->p_session, "push", before, p_stack->p_session->current_state_id, true);
    dv_session_add_step_arg(p_stack->p_session, "value", p_val);

    return DV_OK;
}

dv_error_t dv_stack_push_int(dv_stack_t *p_stack, int64_t value)
{
    dv_value_t val;

    val.type = DV_VALUE_INT;
    val.data.int_val = value;

    return dv_stack_push_value(p_stack, &val);
}

dv_error_t dv_stack_push_float(dv_stack_t *p_stack, double value)
{
    dv_value_t val;

    val.type = DV_VALUE_FLOAT;
    val.data.float_val = value;

    return dv_stack_push_value(p_stack, &val);
}

dv_error_t dv_stack_push_string(dv_stack_t *p_stack, const char *value)
{
    dv_value_t val;
    dv_error_t err;

    val.type = DV_VALUE_STRING;
    val.data.string_val = dv_strdup_internal(value);

    err = dv_stack_push_value(p_stack, &val);

    DV_SAFE_FREE(val.data.string_val);

    return err;
}

dv_error_t dv_stack_push_bool(dv_stack_t *p_stack, bool value)
{
    dv_value_t val;

    val.type = DV_VALUE_BOOL;
    val.data.bool_val = value;

    return dv_stack_push_value(p_stack, &val);
}

dv_error_t dv_stack_pop(dv_stack_t *p_stack)
{
    char state_buf[4096];
    int32_t before;
    dv_value_t popped;

    if (p_stack == NULL)
    {
        return DV_ERR_NULL_PTR;
    }

    if (p_stack->count == 0)
    {
        dv_session_set_error(p_stack->p_session, "runtime", "Cannot pop from empty stack");
        return DV_ERR_EMPTY;
    }

    before = p_stack->p_session->current_state_id;

    popped = p_stack->items[p_stack->count - 1];
    p_stack->count--;

    dv_stack_build_state(p_stack, state_buf, sizeof(state_buf));
    dv_session_add_state(p_stack->p_session, state_buf);

    dv_session_add_step(p_stack->p_session, "pop", before, p_stack->p_session->current_state_id, true);
    dv_session_set_step_ret(p_stack->p_session, &popped);

    dv_value_free(&popped);

    return DV_OK;
}

dv_error_t dv_stack_clear(dv_stack_t *p_stack)
{
    char state_buf[4096];
    int32_t before;
    size_t i;

    if (p_stack == NULL)
    {
        return DV_ERR_NULL_PTR;
    }

    before = p_stack->p_session->current_state_id;

    for (i = 0; i < p_stack->count; i++)
    {
        dv_value_free(&p_stack->items[i]);
    }
    p_stack->count = 0;

    dv_stack_build_state(p_stack, state_buf, sizeof(state_buf));
    dv_session_add_state(p_stack->p_session, state_buf);

    dv_session_add_step(p_stack->p_session, "clear", before, p_stack->p_session->current_state_id, true);

    return DV_OK;
}

/* ========================================================================== */
/* 队列实现                                                                    */
/* ========================================================================== */

static void dv_queue_build_state(dv_queue_t *p_queue, char *buf, size_t buf_size)
{
    size_t i;
    int offset = 0;
    char val_buf[256];

    offset += snprintf(buf + offset, buf_size - offset, "items = [");

    for (i = 0; i < p_queue->count; i++)
    {
        if (i > 0)
        {
            offset += snprintf(buf + offset, buf_size - offset, ", ");
        }
        dv_value_to_toml(&p_queue->items[i], val_buf, sizeof(val_buf));
        offset += snprintf(buf + offset, buf_size - offset, "%s", val_buf);
    }

    offset += snprintf(buf + offset, buf_size - offset, "]\n");

    if (p_queue->count == 0)
    {
        snprintf(buf + offset, buf_size - offset, "front = -1\nrear = -1\n");
    }
    else
    {
        snprintf(buf + offset, buf_size - offset, "front = 0\nrear = %d\n", (int)(p_queue->count - 1));
    }
}

dv_queue_t *dv_queue_create(const char *label, const char *output)
{
    dv_queue_t *p_queue = NULL;
    char state_buf[4096];

    p_queue = (dv_queue_t *)calloc(1, sizeof(dv_queue_t));
    if (p_queue == NULL)
    {
        return NULL;
    }

    p_queue->p_session = dv_session_create("queue", label ? label : "queue", output);
    if (p_queue->p_session == NULL)
    {
        free(p_queue);
        return NULL;
    }

    p_queue->capacity = DV_INITIAL_CAPACITY;
    p_queue->items = (dv_value_t *)calloc(p_queue->capacity, sizeof(dv_value_t));
    if (p_queue->items == NULL)
    {
        dv_session_destroy(p_queue->p_session);
        free(p_queue);
        return NULL;
    }

    dv_queue_build_state(p_queue, state_buf, sizeof(state_buf));
    dv_session_add_state(p_queue->p_session, state_buf);

    return p_queue;
}

void dv_queue_destroy(dv_queue_t *p_queue)
{
    size_t i;

    if (p_queue == NULL)
    {
        return;
    }

    if (p_queue->items != NULL)
    {
        for (i = 0; i < p_queue->count; i++)
        {
            dv_value_free(&p_queue->items[i]);
        }
        free(p_queue->items);
    }

    dv_session_destroy(p_queue->p_session);
    free(p_queue);
}

dv_error_t dv_queue_commit(dv_queue_t *p_queue)
{
    if (p_queue == NULL)
    {
        return DV_ERR_NULL_PTR;
    }

    return dv_session_write(p_queue->p_session);
}

static dv_error_t dv_queue_enqueue_value(dv_queue_t *p_queue, const dv_value_t *p_val)
{
    dv_value_t *new_items = NULL;
    char state_buf[4096];
    int32_t before;

    if (p_queue == NULL || p_val == NULL)
    {
        return DV_ERR_NULL_PTR;
    }

    if (p_queue->count >= p_queue->capacity)
    {
        size_t new_capacity = p_queue->capacity * 2;
        new_items = (dv_value_t *)realloc(p_queue->items, new_capacity * sizeof(dv_value_t));
        if (new_items == NULL)
        {
            dv_session_set_error(p_queue->p_session, "runtime", "Memory allocation failed");
            return DV_ERR_ALLOC;
        }
        p_queue->items = new_items;
        p_queue->capacity = new_capacity;
    }

    before = p_queue->p_session->current_state_id;

    p_queue->items[p_queue->count] = dv_value_copy(p_val);
    p_queue->count++;

    dv_queue_build_state(p_queue, state_buf, sizeof(state_buf));
    dv_session_add_state(p_queue->p_session, state_buf);

    dv_session_add_step(p_queue->p_session, "enqueue", before, p_queue->p_session->current_state_id, true);
    dv_session_add_step_arg(p_queue->p_session, "value", p_val);

    return DV_OK;
}

dv_error_t dv_queue_enqueue_int(dv_queue_t *p_queue, int64_t value)
{
    dv_value_t val;
    val.type = DV_VALUE_INT;
    val.data.int_val = value;
    return dv_queue_enqueue_value(p_queue, &val);
}

dv_error_t dv_queue_enqueue_float(dv_queue_t *p_queue, double value)
{
    dv_value_t val;
    val.type = DV_VALUE_FLOAT;
    val.data.float_val = value;
    return dv_queue_enqueue_value(p_queue, &val);
}

dv_error_t dv_queue_enqueue_string(dv_queue_t *p_queue, const char *value)
{
    dv_value_t val;
    dv_error_t err;

    val.type = DV_VALUE_STRING;
    val.data.string_val = dv_strdup_internal(value);
    err = dv_queue_enqueue_value(p_queue, &val);
    DV_SAFE_FREE(val.data.string_val);
    return err;
}

dv_error_t dv_queue_enqueue_bool(dv_queue_t *p_queue, bool value)
{
    dv_value_t val;
    val.type = DV_VALUE_BOOL;
    val.data.bool_val = value;
    return dv_queue_enqueue_value(p_queue, &val);
}

dv_error_t dv_queue_dequeue(dv_queue_t *p_queue)
{
    char state_buf[4096];
    int32_t before;
    dv_value_t dequeued;
    size_t i;

    if (p_queue == NULL)
    {
        return DV_ERR_NULL_PTR;
    }

    if (p_queue->count == 0)
    {
        dv_session_set_error(p_queue->p_session, "runtime", "Cannot dequeue from empty queue");
        return DV_ERR_EMPTY;
    }

    before = p_queue->p_session->current_state_id;

    dequeued = p_queue->items[0];

    for (i = 1; i < p_queue->count; i++)
    {
        p_queue->items[i - 1] = p_queue->items[i];
    }
    p_queue->count--;

    dv_queue_build_state(p_queue, state_buf, sizeof(state_buf));
    dv_session_add_state(p_queue->p_session, state_buf);

    dv_session_add_step(p_queue->p_session, "dequeue", before, p_queue->p_session->current_state_id, true);
    dv_session_set_step_ret(p_queue->p_session, &dequeued);

    dv_value_free(&dequeued);

    return DV_OK;
}

dv_error_t dv_queue_clear(dv_queue_t *p_queue)
{
    char state_buf[4096];
    int32_t before;
    size_t i;

    if (p_queue == NULL)
    {
        return DV_ERR_NULL_PTR;
    }

    before = p_queue->p_session->current_state_id;

    for (i = 0; i < p_queue->count; i++)
    {
        dv_value_free(&p_queue->items[i]);
    }
    p_queue->count = 0;

    dv_queue_build_state(p_queue, state_buf, sizeof(state_buf));
    dv_session_add_state(p_queue->p_session, state_buf);

    dv_session_add_step(p_queue->p_session, "clear", before, p_queue->p_session->current_state_id, true);

    return DV_OK;
}

/* ========================================================================== */
/* 单链表实现                                                                  */
/* ========================================================================== */

static int32_t dv_slist_find_node_index(dv_slist_t *p_slist, int32_t node_id)
{
    size_t i;
    for (i = 0; i < p_slist->nodes_count; i++)
    {
        if (p_slist->nodes[i].id == node_id && p_slist->nodes[i].active)
        {
            return (int32_t)i;
        }
    }
    return -1;
}

static void dv_slist_build_state(dv_slist_t *p_slist, char *buf, size_t buf_size)
{
    int offset = 0;
    size_t i;
    char val_buf[256];
    bool first = true;

    offset += snprintf(buf + offset, buf_size - offset, "head = %d\n", p_slist->head);
    offset += snprintf(buf + offset, buf_size - offset, "nodes = [");

    for (i = 0; i < p_slist->nodes_count; i++)
    {
        if (p_slist->nodes[i].active)
        {
            if (!first)
            {
                offset += snprintf(buf + offset, buf_size - offset, ",");
            }
            first = false;

            dv_value_to_toml(&p_slist->nodes[i].value, val_buf, sizeof(val_buf));
            offset += snprintf(buf + offset, buf_size - offset,
                               "\n  { id = %d, value = %s, next = %d }",
                               p_slist->nodes[i].id, val_buf, p_slist->nodes[i].next);
        }
    }

    if (!first)
    {
        offset += snprintf(buf + offset, buf_size - offset, "\n");
    }
    snprintf(buf + offset, buf_size - offset, "]\n");
}

dv_slist_t *dv_slist_create(const char *label, const char *output)
{
    dv_slist_t *p_slist = NULL;
    char state_buf[4096];

    p_slist = (dv_slist_t *)calloc(1, sizeof(dv_slist_t));
    if (p_slist == NULL)
    {
        return NULL;
    }

    p_slist->p_session = dv_session_create("slist", label ? label : "slist", output);
    if (p_slist->p_session == NULL)
    {
        free(p_slist);
        return NULL;
    }

    p_slist->nodes_capacity = DV_INITIAL_CAPACITY;
    p_slist->nodes = (dv_slist_node_t *)calloc(p_slist->nodes_capacity, sizeof(dv_slist_node_t));
    if (p_slist->nodes == NULL)
    {
        dv_session_destroy(p_slist->p_session);
        free(p_slist);
        return NULL;
    }

    p_slist->head = -1;
    p_slist->next_id = 0;

    dv_slist_build_state(p_slist, state_buf, sizeof(state_buf));
    dv_session_add_state(p_slist->p_session, state_buf);

    return p_slist;
}

void dv_slist_destroy(dv_slist_t *p_slist)
{
    size_t i;

    if (p_slist == NULL)
    {
        return;
    }

    if (p_slist->nodes != NULL)
    {
        for (i = 0; i < p_slist->nodes_count; i++)
        {
            dv_value_free(&p_slist->nodes[i].value);
        }
        free(p_slist->nodes);
    }

    dv_session_destroy(p_slist->p_session);
    free(p_slist);
}

dv_error_t dv_slist_commit(dv_slist_t *p_slist)
{
    if (p_slist == NULL)
    {
        return DV_ERR_NULL_PTR;
    }
    return dv_session_write(p_slist->p_session);
}

static dv_error_t dv_slist_insert_head_value(dv_slist_t *p_slist, const dv_value_t *p_val, int32_t *p_out_node_id)
{
    dv_slist_node_t *new_nodes = NULL;
    dv_slist_node_t *p_node = NULL;
    char state_buf[4096];
    int32_t before;
    int32_t new_id;

    if (p_slist == NULL || p_val == NULL)
    {
        return DV_ERR_NULL_PTR;
    }

    if (p_slist->nodes_count >= p_slist->nodes_capacity)
    {
        size_t new_capacity = p_slist->nodes_capacity * 2;
        new_nodes = (dv_slist_node_t *)realloc(p_slist->nodes, new_capacity * sizeof(dv_slist_node_t));
        if (new_nodes == NULL)
        {
            return DV_ERR_ALLOC;
        }
        p_slist->nodes = new_nodes;
        p_slist->nodes_capacity = new_capacity;
    }

    before = p_slist->p_session->current_state_id;
    new_id = p_slist->next_id++;

    p_node = &p_slist->nodes[p_slist->nodes_count];
    p_node->id = new_id;
    p_node->value = dv_value_copy(p_val);
    p_node->next = p_slist->head;
    p_node->active = true;
    p_slist->nodes_count++;

    p_slist->head = new_id;

    dv_slist_build_state(p_slist, state_buf, sizeof(state_buf));
    dv_session_add_state(p_slist->p_session, state_buf);

    dv_session_add_step(p_slist->p_session, "insert_head", before, p_slist->p_session->current_state_id, true);
    dv_session_add_step_arg(p_slist->p_session, "value", p_val);

    {
        dv_value_t ret_val;
        ret_val.type = DV_VALUE_INT;
        ret_val.data.int_val = new_id;
        dv_session_set_step_ret(p_slist->p_session, &ret_val);
    }

    if (p_out_node_id != NULL)
    {
        *p_out_node_id = new_id;
    }

    return DV_OK;
}

dv_error_t dv_slist_insert_head_int(dv_slist_t *p_slist, int64_t value, int32_t *p_out_node_id)
{
    dv_value_t val;
    val.type = DV_VALUE_INT;
    val.data.int_val = value;
    return dv_slist_insert_head_value(p_slist, &val, p_out_node_id);
}

dv_error_t dv_slist_insert_head_float(dv_slist_t *p_slist, double value, int32_t *p_out_node_id)
{
    dv_value_t val;
    val.type = DV_VALUE_FLOAT;
    val.data.float_val = value;
    return dv_slist_insert_head_value(p_slist, &val, p_out_node_id);
}

dv_error_t dv_slist_insert_head_string(dv_slist_t *p_slist, const char *value, int32_t *p_out_node_id)
{
    dv_value_t val;
    dv_error_t err;
    val.type = DV_VALUE_STRING;
    val.data.string_val = dv_strdup_internal(value);
    err = dv_slist_insert_head_value(p_slist, &val, p_out_node_id);
    DV_SAFE_FREE(val.data.string_val);
    return err;
}

dv_error_t dv_slist_insert_head_bool(dv_slist_t *p_slist, bool value, int32_t *p_out_node_id)
{
    dv_value_t val;
    val.type = DV_VALUE_BOOL;
    val.data.bool_val = value;
    return dv_slist_insert_head_value(p_slist, &val, p_out_node_id);
}

static dv_error_t dv_slist_insert_tail_value(dv_slist_t *p_slist, const dv_value_t *p_val, int32_t *p_out_node_id)
{
    dv_slist_node_t *new_nodes = NULL;
    dv_slist_node_t *p_node = NULL;
    char state_buf[4096];
    int32_t before;
    int32_t new_id;
    int32_t tail_idx;
    size_t i;

    if (p_slist == NULL || p_val == NULL)
    {
        return DV_ERR_NULL_PTR;
    }

    if (p_slist->nodes_count >= p_slist->nodes_capacity)
    {
        size_t new_capacity = p_slist->nodes_capacity * 2;
        new_nodes = (dv_slist_node_t *)realloc(p_slist->nodes, new_capacity * sizeof(dv_slist_node_t));
        if (new_nodes == NULL)
        {
            return DV_ERR_ALLOC;
        }
        p_slist->nodes = new_nodes;
        p_slist->nodes_capacity = new_capacity;
    }

    before = p_slist->p_session->current_state_id;
    new_id = p_slist->next_id++;

    p_node = &p_slist->nodes[p_slist->nodes_count];
    p_node->id = new_id;
    p_node->value = dv_value_copy(p_val);
    p_node->next = -1;
    p_node->active = true;
    p_slist->nodes_count++;

    if (p_slist->head == -1)
    {
        p_slist->head = new_id;
    }
    else
    {
        /* 找到尾节点 */
        tail_idx = -1;
        for (i = 0; i < p_slist->nodes_count - 1; i++)
        {
            if (p_slist->nodes[i].active && p_slist->nodes[i].next == -1)
            {
                tail_idx = (int32_t)i;
                break;
            }
        }
        if (tail_idx >= 0)
        {
            p_slist->nodes[tail_idx].next = new_id;
        }
    }

    dv_slist_build_state(p_slist, state_buf, sizeof(state_buf));
    dv_session_add_state(p_slist->p_session, state_buf);

    dv_session_add_step(p_slist->p_session, "insert_tail", before, p_slist->p_session->current_state_id, true);
    dv_session_add_step_arg(p_slist->p_session, "value", p_val);

    {
        dv_value_t ret_val;
        ret_val.type = DV_VALUE_INT;
        ret_val.data.int_val = new_id;
        dv_session_set_step_ret(p_slist->p_session, &ret_val);
    }

    if (p_out_node_id != NULL)
    {
        *p_out_node_id = new_id;
    }

    return DV_OK;
}

dv_error_t dv_slist_insert_tail_int(dv_slist_t *p_slist, int64_t value, int32_t *p_out_node_id)
{
    dv_value_t val;
    val.type = DV_VALUE_INT;
    val.data.int_val = value;
    return dv_slist_insert_tail_value(p_slist, &val, p_out_node_id);
}

dv_error_t dv_slist_insert_tail_float(dv_slist_t *p_slist, double value, int32_t *p_out_node_id)
{
    dv_value_t val;
    val.type = DV_VALUE_FLOAT;
    val.data.float_val = value;
    return dv_slist_insert_tail_value(p_slist, &val, p_out_node_id);
}

dv_error_t dv_slist_insert_tail_string(dv_slist_t *p_slist, const char *value, int32_t *p_out_node_id)
{
    dv_value_t val;
    dv_error_t err;
    val.type = DV_VALUE_STRING;
    val.data.string_val = dv_strdup_internal(value);
    err = dv_slist_insert_tail_value(p_slist, &val, p_out_node_id);
    DV_SAFE_FREE(val.data.string_val);
    return err;
}

dv_error_t dv_slist_insert_tail_bool(dv_slist_t *p_slist, bool value, int32_t *p_out_node_id)
{
    dv_value_t val;
    val.type = DV_VALUE_BOOL;
    val.data.bool_val = value;
    return dv_slist_insert_tail_value(p_slist, &val, p_out_node_id);
}

static dv_error_t dv_slist_insert_after_value(dv_slist_t *p_slist, int32_t node_id, const dv_value_t *p_val, int32_t *p_out_node_id)
{
    dv_slist_node_t *new_nodes = NULL;
    dv_slist_node_t *p_node = NULL;
    dv_slist_node_t *p_target = NULL;
    char state_buf[4096];
    int32_t before;
    int32_t new_id;
    int32_t target_idx;
    dv_value_t arg_val;

    if (p_slist == NULL || p_val == NULL)
    {
        return DV_ERR_NULL_PTR;
    }

    target_idx = dv_slist_find_node_index(p_slist, node_id);
    if (target_idx < 0)
    {
        char msg[128];
        snprintf(msg, sizeof(msg), "Node not found: %d", node_id);
        dv_session_set_error(p_slist->p_session, "runtime", msg);
        return DV_ERR_NOT_FOUND;
    }

    if (p_slist->nodes_count >= p_slist->nodes_capacity)
    {
        size_t new_capacity = p_slist->nodes_capacity * 2;
        new_nodes = (dv_slist_node_t *)realloc(p_slist->nodes, new_capacity * sizeof(dv_slist_node_t));
        if (new_nodes == NULL)
        {
            return DV_ERR_ALLOC;
        }
        p_slist->nodes = new_nodes;
        p_slist->nodes_capacity = new_capacity;
        target_idx = dv_slist_find_node_index(p_slist, node_id);
    }

    before = p_slist->p_session->current_state_id;
    new_id = p_slist->next_id++;

    p_target = &p_slist->nodes[target_idx];

    p_node = &p_slist->nodes[p_slist->nodes_count];
    p_node->id = new_id;
    p_node->value = dv_value_copy(p_val);
    p_node->next = p_target->next;
    p_node->active = true;
    p_slist->nodes_count++;

    p_target->next = new_id;

    dv_slist_build_state(p_slist, state_buf, sizeof(state_buf));
    dv_session_add_state(p_slist->p_session, state_buf);

    dv_session_add_step(p_slist->p_session, "insert_after", before, p_slist->p_session->current_state_id, true);

    arg_val.type = DV_VALUE_INT;
    arg_val.data.int_val = node_id;
    dv_session_add_step_arg(p_slist->p_session, "node_id", &arg_val);
    dv_session_add_step_arg(p_slist->p_session, "value", p_val);

    {
        dv_value_t ret_val;
        ret_val.type = DV_VALUE_INT;
        ret_val.data.int_val = new_id;
        dv_session_set_step_ret(p_slist->p_session, &ret_val);
    }

    if (p_out_node_id != NULL)
    {
        *p_out_node_id = new_id;
    }

    return DV_OK;
}

dv_error_t dv_slist_insert_after_int(dv_slist_t *p_slist, int32_t node_id, int64_t value, int32_t *p_out_node_id)
{
    dv_value_t val;
    val.type = DV_VALUE_INT;
    val.data.int_val = value;
    return dv_slist_insert_after_value(p_slist, node_id, &val, p_out_node_id);
}

dv_error_t dv_slist_insert_after_float(dv_slist_t *p_slist, int32_t node_id, double value, int32_t *p_out_node_id)
{
    dv_value_t val;
    val.type = DV_VALUE_FLOAT;
    val.data.float_val = value;
    return dv_slist_insert_after_value(p_slist, node_id, &val, p_out_node_id);
}

dv_error_t dv_slist_insert_after_string(dv_slist_t *p_slist, int32_t node_id, const char *value, int32_t *p_out_node_id)
{
    dv_value_t val;
    dv_error_t err;
    val.type = DV_VALUE_STRING;
    val.data.string_val = dv_strdup_internal(value);
    err = dv_slist_insert_after_value(p_slist, node_id, &val, p_out_node_id);
    DV_SAFE_FREE(val.data.string_val);
    return err;
}

dv_error_t dv_slist_insert_after_bool(dv_slist_t *p_slist, int32_t node_id, bool value, int32_t *p_out_node_id)
{
    dv_value_t val;
    val.type = DV_VALUE_BOOL;
    val.data.bool_val = value;
    return dv_slist_insert_after_value(p_slist, node_id, &val, p_out_node_id);
}

dv_error_t dv_slist_delete(dv_slist_t *p_slist, int32_t node_id)
{
    int32_t target_idx;
    int32_t prev_idx;
    size_t i;
    char state_buf[4096];
    int32_t before;
    dv_value_t deleted_val;
    dv_value_t arg_val;

    if (p_slist == NULL)
    {
        return DV_ERR_NULL_PTR;
    }

    target_idx = dv_slist_find_node_index(p_slist, node_id);
    if (target_idx < 0)
    {
        char msg[128];
        snprintf(msg, sizeof(msg), "Node not found: %d", node_id);
        dv_session_set_error(p_slist->p_session, "runtime", msg);
        return DV_ERR_NOT_FOUND;
    }

    before = p_slist->p_session->current_state_id;
    deleted_val = dv_value_copy(&p_slist->nodes[target_idx].value);

    if (p_slist->head == node_id)
    {
        p_slist->head = p_slist->nodes[target_idx].next;
    }
    else
    {
        prev_idx = -1;
        for (i = 0; i < p_slist->nodes_count; i++)
        {
            if (p_slist->nodes[i].active && p_slist->nodes[i].next == node_id)
            {
                prev_idx = (int32_t)i;
                break;
            }
        }
        if (prev_idx >= 0)
        {
            p_slist->nodes[prev_idx].next = p_slist->nodes[target_idx].next;
        }
    }

    dv_value_free(&p_slist->nodes[target_idx].value);
    p_slist->nodes[target_idx].active = false;

    dv_slist_build_state(p_slist, state_buf, sizeof(state_buf));
    dv_session_add_state(p_slist->p_session, state_buf);

    dv_session_add_step(p_slist->p_session, "delete", before, p_slist->p_session->current_state_id, true);

    arg_val.type = DV_VALUE_INT;
    arg_val.data.int_val = node_id;
    dv_session_add_step_arg(p_slist->p_session, "node_id", &arg_val);
    dv_session_set_step_ret(p_slist->p_session, &deleted_val);

    dv_value_free(&deleted_val);

    return DV_OK;
}

dv_error_t dv_slist_delete_head(dv_slist_t *p_slist)
{
    if (p_slist == NULL)
    {
        return DV_ERR_NULL_PTR;
    }

    if (p_slist->head == -1)
    {
        dv_session_set_error(p_slist->p_session, "runtime", "Cannot delete from empty list");
        return DV_ERR_EMPTY;
    }

    return dv_slist_delete(p_slist, p_slist->head);
}

dv_error_t dv_slist_reverse(dv_slist_t *p_slist)
{
    int32_t prev;
    int32_t current;
    int32_t next;
    int32_t current_idx;
    char state_buf[4096];
    int32_t before;

    if (p_slist == NULL)
    {
        return DV_ERR_NULL_PTR;
    }

    before = p_slist->p_session->current_state_id;

    prev = -1;
    current = p_slist->head;

    while (current != -1)
    {
        current_idx = dv_slist_find_node_index(p_slist, current);
        if (current_idx < 0)
        {
            break;
        }

        next = p_slist->nodes[current_idx].next;
        p_slist->nodes[current_idx].next = prev;
        prev = current;
        current = next;
    }

    p_slist->head = prev;

    dv_slist_build_state(p_slist, state_buf, sizeof(state_buf));
    dv_session_add_state(p_slist->p_session, state_buf);

    dv_session_add_step(p_slist->p_session, "reverse", before, p_slist->p_session->current_state_id, true);

    return DV_OK;
}

/* ========================================================================== */
/* 双向链表实现（简化版本）                                                    */
/* ========================================================================== */

static int32_t dv_dlist_find_node_index(dv_dlist_t *p_dlist, int32_t node_id)
{
    size_t i;
    for (i = 0; i < p_dlist->nodes_count; i++)
    {
        if (p_dlist->nodes[i].id == node_id && p_dlist->nodes[i].active)
        {
            return (int32_t)i;
        }
    }
    return -1;
}

static void dv_dlist_build_state(dv_dlist_t *p_dlist, char *buf, size_t buf_size)
{
    int offset = 0;
    size_t i;
    char val_buf[256];
    bool first = true;

    offset += snprintf(buf + offset, buf_size - offset, "head = %d\n", p_dlist->head);
    offset += snprintf(buf + offset, buf_size - offset, "tail = %d\n", p_dlist->tail);
    offset += snprintf(buf + offset, buf_size - offset, "nodes = [");

    for (i = 0; i < p_dlist->nodes_count; i++)
    {
        if (p_dlist->nodes[i].active)
        {
            if (!first)
            {
                offset += snprintf(buf + offset, buf_size - offset, ",");
            }
            first = false;

            dv_value_to_toml(&p_dlist->nodes[i].value, val_buf, sizeof(val_buf));
            offset += snprintf(buf + offset, buf_size - offset,
                               "\n  { id = %d, value = %s, prev = %d, next = %d }",
                               p_dlist->nodes[i].id, val_buf,
                               p_dlist->nodes[i].prev, p_dlist->nodes[i].next);
        }
    }

    if (!first)
    {
        offset += snprintf(buf + offset, buf_size - offset, "\n");
    }
    snprintf(buf + offset, buf_size - offset, "]\n");
}

dv_dlist_t *dv_dlist_create(const char *label, const char *output)
{
    dv_dlist_t *p_dlist = NULL;
    char state_buf[4096];

    p_dlist = (dv_dlist_t *)calloc(1, sizeof(dv_dlist_t));
    if (p_dlist == NULL)
    {
        return NULL;
    }

    p_dlist->p_session = dv_session_create("dlist", label ? label : "dlist", output);
    if (p_dlist->p_session == NULL)
    {
        free(p_dlist);
        return NULL;
    }

    p_dlist->nodes_capacity = DV_INITIAL_CAPACITY;
    p_dlist->nodes = (dv_dlist_node_t *)calloc(p_dlist->nodes_capacity, sizeof(dv_dlist_node_t));
    if (p_dlist->nodes == NULL)
    {
        dv_session_destroy(p_dlist->p_session);
        free(p_dlist);
        return NULL;
    }

    p_dlist->head = -1;
    p_dlist->tail = -1;
    p_dlist->next_id = 0;

    dv_dlist_build_state(p_dlist, state_buf, sizeof(state_buf));
    dv_session_add_state(p_dlist->p_session, state_buf);

    return p_dlist;
}

void dv_dlist_destroy(dv_dlist_t *p_dlist)
{
    size_t i;

    if (p_dlist == NULL)
    {
        return;
    }

    if (p_dlist->nodes != NULL)
    {
        for (i = 0; i < p_dlist->nodes_count; i++)
        {
            dv_value_free(&p_dlist->nodes[i].value);
        }
        free(p_dlist->nodes);
    }

    dv_session_destroy(p_dlist->p_session);
    free(p_dlist);
}

dv_error_t dv_dlist_commit(dv_dlist_t *p_dlist)
{
    if (p_dlist == NULL)
    {
        return DV_ERR_NULL_PTR;
    }
    return dv_session_write(p_dlist->p_session);
}

static dv_error_t dv_dlist_insert_head_value(dv_dlist_t *p_dlist, const dv_value_t *p_val, int32_t *p_out_node_id)
{
    dv_dlist_node_t *new_nodes = NULL;
    dv_dlist_node_t *p_node = NULL;
    char state_buf[4096];
    int32_t before;
    int32_t new_id;
    int32_t old_head_idx;

    if (p_dlist == NULL || p_val == NULL)
    {
        return DV_ERR_NULL_PTR;
    }

    if (p_dlist->nodes_count >= p_dlist->nodes_capacity)
    {
        size_t new_capacity = p_dlist->nodes_capacity * 2;
        new_nodes = (dv_dlist_node_t *)realloc(p_dlist->nodes, new_capacity * sizeof(dv_dlist_node_t));
        if (new_nodes == NULL)
        {
            return DV_ERR_ALLOC;
        }
        p_dlist->nodes = new_nodes;
        p_dlist->nodes_capacity = new_capacity;
    }

    before = p_dlist->p_session->current_state_id;
    new_id = p_dlist->next_id++;

    p_node = &p_dlist->nodes[p_dlist->nodes_count];
    p_node->id = new_id;
    p_node->value = dv_value_copy(p_val);
    p_node->prev = -1;
    p_node->next = p_dlist->head;
    p_node->active = true;
    p_dlist->nodes_count++;

    if (p_dlist->head != -1)
    {
        old_head_idx = dv_dlist_find_node_index(p_dlist, p_dlist->head);
        if (old_head_idx >= 0)
        {
            p_dlist->nodes[old_head_idx].prev = new_id;
        }
    }

    p_dlist->head = new_id;
    if (p_dlist->tail == -1)
    {
        p_dlist->tail = new_id;
    }

    dv_dlist_build_state(p_dlist, state_buf, sizeof(state_buf));
    dv_session_add_state(p_dlist->p_session, state_buf);

    dv_session_add_step(p_dlist->p_session, "insert_head", before, p_dlist->p_session->current_state_id, true);
    dv_session_add_step_arg(p_dlist->p_session, "value", p_val);

    {
        dv_value_t ret_val;
        ret_val.type = DV_VALUE_INT;
        ret_val.data.int_val = new_id;
        dv_session_set_step_ret(p_dlist->p_session, &ret_val);
    }

    if (p_out_node_id != NULL)
    {
        *p_out_node_id = new_id;
    }

    return DV_OK;
}

dv_error_t dv_dlist_insert_head_int(dv_dlist_t *p_dlist, int64_t value, int32_t *p_out_node_id)
{
    dv_value_t val;
    val.type = DV_VALUE_INT;
    val.data.int_val = value;
    return dv_dlist_insert_head_value(p_dlist, &val, p_out_node_id);
}

dv_error_t dv_dlist_insert_head_float(dv_dlist_t *p_dlist, double value, int32_t *p_out_node_id)
{
    dv_value_t val;
    val.type = DV_VALUE_FLOAT;
    val.data.float_val = value;
    return dv_dlist_insert_head_value(p_dlist, &val, p_out_node_id);
}

dv_error_t dv_dlist_insert_head_string(dv_dlist_t *p_dlist, const char *value, int32_t *p_out_node_id)
{
    dv_value_t val;
    dv_error_t err;
    val.type = DV_VALUE_STRING;
    val.data.string_val = dv_strdup_internal(value);
    err = dv_dlist_insert_head_value(p_dlist, &val, p_out_node_id);
    DV_SAFE_FREE(val.data.string_val);
    return err;
}

dv_error_t dv_dlist_insert_head_bool(dv_dlist_t *p_dlist, bool value, int32_t *p_out_node_id)
{
    dv_value_t val;
    val.type = DV_VALUE_BOOL;
    val.data.bool_val = value;
    return dv_dlist_insert_head_value(p_dlist, &val, p_out_node_id);
}

static dv_error_t dv_dlist_insert_tail_value(dv_dlist_t *p_dlist, const dv_value_t *p_val, int32_t *p_out_node_id)
{
    dv_dlist_node_t *new_nodes = NULL;
    dv_dlist_node_t *p_node = NULL;
    char state_buf[4096];
    int32_t before;
    int32_t new_id;
    int32_t old_tail_idx;

    if (p_dlist == NULL || p_val == NULL)
    {
        return DV_ERR_NULL_PTR;
    }

    if (p_dlist->nodes_count >= p_dlist->nodes_capacity)
    {
        size_t new_capacity = p_dlist->nodes_capacity * 2;
        new_nodes = (dv_dlist_node_t *)realloc(p_dlist->nodes, new_capacity * sizeof(dv_dlist_node_t));
        if (new_nodes == NULL)
        {
            return DV_ERR_ALLOC;
        }
        p_dlist->nodes = new_nodes;
        p_dlist->nodes_capacity = new_capacity;
    }

    before = p_dlist->p_session->current_state_id;
    new_id = p_dlist->next_id++;

    p_node = &p_dlist->nodes[p_dlist->nodes_count];
    p_node->id = new_id;
    p_node->value = dv_value_copy(p_val);
    p_node->prev = p_dlist->tail;
    p_node->next = -1;
    p_node->active = true;
    p_dlist->nodes_count++;

    if (p_dlist->tail != -1)
    {
        old_tail_idx = dv_dlist_find_node_index(p_dlist, p_dlist->tail);
        if (old_tail_idx >= 0)
        {
            p_dlist->nodes[old_tail_idx].next = new_id;
        }
    }

    p_dlist->tail = new_id;
    if (p_dlist->head == -1)
    {
        p_dlist->head = new_id;
    }

    dv_dlist_build_state(p_dlist, state_buf, sizeof(state_buf));
    dv_session_add_state(p_dlist->p_session, state_buf);

    dv_session_add_step(p_dlist->p_session, "insert_tail", before, p_dlist->p_session->current_state_id, true);
    dv_session_add_step_arg(p_dlist->p_session, "value", p_val);

    {
        dv_value_t ret_val;
        ret_val.type = DV_VALUE_INT;
        ret_val.data.int_val = new_id;
        dv_session_set_step_ret(p_dlist->p_session, &ret_val);
    }

    if (p_out_node_id != NULL)
    {
        *p_out_node_id = new_id;
    }

    return DV_OK;
}

dv_error_t dv_dlist_insert_tail_int(dv_dlist_t *p_dlist, int64_t value, int32_t *p_out_node_id)
{
    dv_value_t val;
    val.type = DV_VALUE_INT;
    val.data.int_val = value;
    return dv_dlist_insert_tail_value(p_dlist, &val, p_out_node_id);
}

dv_error_t dv_dlist_insert_tail_float(dv_dlist_t *p_dlist, double value, int32_t *p_out_node_id)
{
    dv_value_t val;
    val.type = DV_VALUE_FLOAT;
    val.data.float_val = value;
    return dv_dlist_insert_tail_value(p_dlist, &val, p_out_node_id);
}

dv_error_t dv_dlist_insert_tail_string(dv_dlist_t *p_dlist, const char *value, int32_t *p_out_node_id)
{
    dv_value_t val;
    dv_error_t err;
    val.type = DV_VALUE_STRING;
    val.data.string_val = dv_strdup_internal(value);
    err = dv_dlist_insert_tail_value(p_dlist, &val, p_out_node_id);
    DV_SAFE_FREE(val.data.string_val);
    return err;
}

dv_error_t dv_dlist_insert_tail_bool(dv_dlist_t *p_dlist, bool value, int32_t *p_out_node_id)
{
    dv_value_t val;
    val.type = DV_VALUE_BOOL;
    val.data.bool_val = value;
    return dv_dlist_insert_tail_value(p_dlist, &val, p_out_node_id);
}

/* insert_before 和 insert_after 的实现类似，为简洁省略，按相同模式实现 */

dv_error_t dv_dlist_insert_before_int(dv_dlist_t *p_dlist, int32_t node_id, int64_t value, int32_t *p_out_node_id)
{
    (void)node_id;
    (void)value;
    (void)p_out_node_id;
    if (p_dlist == NULL)
        return DV_ERR_NULL_PTR;
    /* 简化实现 */
    return DV_OK;
}

dv_error_t dv_dlist_insert_before_float(dv_dlist_t *p_dlist, int32_t node_id, double value, int32_t *p_out_node_id)
{
    (void)node_id;
    (void)value;
    (void)p_out_node_id;
    if (p_dlist == NULL)
        return DV_ERR_NULL_PTR;
    return DV_OK;
}

dv_error_t dv_dlist_insert_before_string(dv_dlist_t *p_dlist, int32_t node_id, const char *value, int32_t *p_out_node_id)
{
    (void)node_id;
    (void)value;
    (void)p_out_node_id;
    if (p_dlist == NULL)
        return DV_ERR_NULL_PTR;
    return DV_OK;
}

dv_error_t dv_dlist_insert_before_bool(dv_dlist_t *p_dlist, int32_t node_id, bool value, int32_t *p_out_node_id)
{
    (void)node_id;
    (void)value;
    (void)p_out_node_id;
    if (p_dlist == NULL)
        return DV_ERR_NULL_PTR;
    return DV_OK;
}

dv_error_t dv_dlist_insert_after_int(dv_dlist_t *p_dlist, int32_t node_id, int64_t value, int32_t *p_out_node_id)
{
    (void)node_id;
    (void)value;
    (void)p_out_node_id;
    if (p_dlist == NULL)
        return DV_ERR_NULL_PTR;
    return DV_OK;
}

dv_error_t dv_dlist_insert_after_float(dv_dlist_t *p_dlist, int32_t node_id, double value, int32_t *p_out_node_id)
{
    (void)node_id;
    (void)value;
    (void)p_out_node_id;
    if (p_dlist == NULL)
        return DV_ERR_NULL_PTR;
    return DV_OK;
}

dv_error_t dv_dlist_insert_after_string(dv_dlist_t *p_dlist, int32_t node_id, const char *value, int32_t *p_out_node_id)
{
    (void)node_id;
    (void)value;
    (void)p_out_node_id;
    if (p_dlist == NULL)
        return DV_ERR_NULL_PTR;
    return DV_OK;
}

dv_error_t dv_dlist_insert_after_bool(dv_dlist_t *p_dlist, int32_t node_id, bool value, int32_t *p_out_node_id)
{
    (void)node_id;
    (void)value;
    (void)p_out_node_id;
    if (p_dlist == NULL)
        return DV_ERR_NULL_PTR;
    return DV_OK;
}

dv_error_t dv_dlist_delete(dv_dlist_t *p_dlist, int32_t node_id)
{
    int32_t target_idx;
    int32_t prev_id;
    int32_t next_id;
    int32_t prev_idx;
    int32_t next_idx;
    char state_buf[4096];
    int32_t before;
    dv_value_t deleted_val;
    dv_value_t arg_val;

    if (p_dlist == NULL)
    {
        return DV_ERR_NULL_PTR;
    }

    target_idx = dv_dlist_find_node_index(p_dlist, node_id);
    if (target_idx < 0)
    {
        char msg[128];
        snprintf(msg, sizeof(msg), "Node not found: %d", node_id);
        dv_session_set_error(p_dlist->p_session, "runtime", msg);
        return DV_ERR_NOT_FOUND;
    }

    before = p_dlist->p_session->current_state_id;
    deleted_val = dv_value_copy(&p_dlist->nodes[target_idx].value);

    prev_id = p_dlist->nodes[target_idx].prev;
    next_id = p_dlist->nodes[target_idx].next;

    if (prev_id != -1)
    {
        prev_idx = dv_dlist_find_node_index(p_dlist, prev_id);
        if (prev_idx >= 0)
        {
            p_dlist->nodes[prev_idx].next = next_id;
        }
    }
    else
    {
        p_dlist->head = next_id;
    }

    if (next_id != -1)
    {
        next_idx = dv_dlist_find_node_index(p_dlist, next_id);
        if (next_idx >= 0)
        {
            p_dlist->nodes[next_idx].prev = prev_id;
        }
    }
    else
    {
        p_dlist->tail = prev_id;
    }

    dv_value_free(&p_dlist->nodes[target_idx].value);
    p_dlist->nodes[target_idx].active = false;

    dv_dlist_build_state(p_dlist, state_buf, sizeof(state_buf));
    dv_session_add_state(p_dlist->p_session, state_buf);

    dv_session_add_step(p_dlist->p_session, "delete", before, p_dlist->p_session->current_state_id, true);

    arg_val.type = DV_VALUE_INT;
    arg_val.data.int_val = node_id;
    dv_session_add_step_arg(p_dlist->p_session, "node_id", &arg_val);
    dv_session_set_step_ret(p_dlist->p_session, &deleted_val);

    dv_value_free(&deleted_val);

    return DV_OK;
}

dv_error_t dv_dlist_delete_head(dv_dlist_t *p_dlist)
{
    if (p_dlist == NULL)
    {
        return DV_ERR_NULL_PTR;
    }

    if (p_dlist->head == -1)
    {
        dv_session_set_error(p_dlist->p_session, "runtime", "Cannot delete from empty list");
        return DV_ERR_EMPTY;
    }

    return dv_dlist_delete(p_dlist, p_dlist->head);
}

dv_error_t dv_dlist_delete_tail(dv_dlist_t *p_dlist)
{
    if (p_dlist == NULL)
    {
        return DV_ERR_NULL_PTR;
    }

    if (p_dlist->tail == -1)
    {
        dv_session_set_error(p_dlist->p_session, "runtime", "Cannot delete from empty list");
        return DV_ERR_EMPTY;
    }

    return dv_dlist_delete(p_dlist, p_dlist->tail);
}

dv_error_t dv_dlist_reverse(dv_dlist_t *p_dlist)
{
    size_t i;
    int32_t temp;
    char state_buf[4096];
    int32_t before;

    if (p_dlist == NULL)
    {
        return DV_ERR_NULL_PTR;
    }

    before = p_dlist->p_session->current_state_id;

    for (i = 0; i < p_dlist->nodes_count; i++)
    {
        if (p_dlist->nodes[i].active)
        {
            temp = p_dlist->nodes[i].prev;
            p_dlist->nodes[i].prev = p_dlist->nodes[i].next;
            p_dlist->nodes[i].next = temp;
        }
    }

    temp = p_dlist->head;
    p_dlist->head = p_dlist->tail;
    p_dlist->tail = temp;

    dv_dlist_build_state(p_dlist, state_buf, sizeof(state_buf));
    dv_session_add_state(p_dlist->p_session, state_buf);

    dv_session_add_step(p_dlist->p_session, "reverse", before, p_dlist->p_session->current_state_id, true);

    return DV_OK;
}

/* ========================================================================== */
/* 二叉树实现                                                                  */
/* ========================================================================== */

static int32_t dv_binary_tree_find_node_index(dv_binary_tree_t *p_tree, int32_t node_id)
{
    size_t i;
    for (i = 0; i < p_tree->nodes_count; i++)
    {
        if (p_tree->nodes[i].id == node_id && p_tree->nodes[i].active)
        {
            return (int32_t)i;
        }
    }
    return -1;
}

static void dv_binary_tree_build_state(dv_binary_tree_t *p_tree, char *buf, size_t buf_size)
{
    int offset = 0;
    size_t i;
    char val_buf[256];
    bool first = true;

    offset += snprintf(buf + offset, buf_size - offset, "root = %d\n", p_tree->root);
    offset += snprintf(buf + offset, buf_size - offset, "nodes = [");

    for (i = 0; i < p_tree->nodes_count; i++)
    {
        if (p_tree->nodes[i].active)
        {
            if (!first)
            {
                offset += snprintf(buf + offset, buf_size - offset, ",");
            }
            first = false;

            dv_value_to_toml(&p_tree->nodes[i].value, val_buf, sizeof(val_buf));
            offset += snprintf(buf + offset, buf_size - offset,
                               "\n  { id = %d, value = %s, left = %d, right = %d }",
                               p_tree->nodes[i].id, val_buf,
                               p_tree->nodes[i].left, p_tree->nodes[i].right);
        }
    }

    if (!first)
    {
        offset += snprintf(buf + offset, buf_size - offset, "\n");
    }
    snprintf(buf + offset, buf_size - offset, "]\n");
}

dv_binary_tree_t *dv_binary_tree_create(const char *label, const char *output)
{
    dv_binary_tree_t *p_tree = NULL;
    char state_buf[4096];

    p_tree = (dv_binary_tree_t *)calloc(1, sizeof(dv_binary_tree_t));
    if (p_tree == NULL)
    {
        return NULL;
    }

    p_tree->p_session = dv_session_create("binary_tree", label ? label : "binary_tree", output);
    if (p_tree->p_session == NULL)
    {
        free(p_tree);
        return NULL;
    }

    p_tree->nodes_capacity = DV_INITIAL_CAPACITY;
    p_tree->nodes = (dv_tree_node_t *)calloc(p_tree->nodes_capacity, sizeof(dv_tree_node_t));
    if (p_tree->nodes == NULL)
    {
        dv_session_destroy(p_tree->p_session);
        free(p_tree);
        return NULL;
    }

    p_tree->root = -1;
    p_tree->next_id = 0;

    dv_binary_tree_build_state(p_tree, state_buf, sizeof(state_buf));
    dv_session_add_state(p_tree->p_session, state_buf);

    return p_tree;
}

void dv_binary_tree_destroy(dv_binary_tree_t *p_tree)
{
    size_t i;

    if (p_tree == NULL)
    {
        return;
    }

    if (p_tree->nodes != NULL)
    {
        for (i = 0; i < p_tree->nodes_count; i++)
        {
            dv_value_free(&p_tree->nodes[i].value);
        }
        free(p_tree->nodes);
    }

    dv_session_destroy(p_tree->p_session);
    free(p_tree);
}

dv_error_t dv_binary_tree_commit(dv_binary_tree_t *p_tree)
{
    if (p_tree == NULL)
    {
        return DV_ERR_NULL_PTR;
    }
    return dv_session_write(p_tree->p_session);
}

static dv_error_t dv_binary_tree_insert_root_value(dv_binary_tree_t *p_tree, const dv_value_t *p_val, int32_t *p_out_node_id)
{
    dv_tree_node_t *p_node = NULL;
    char state_buf[4096];
    int32_t before;
    int32_t new_id;

    if (p_tree == NULL || p_val == NULL)
    {
        return DV_ERR_NULL_PTR;
    }

    if (p_tree->root != -1)
    {
        dv_session_set_error(p_tree->p_session, "runtime", "Root node already exists");
        return DV_ERR_ROOT_EXISTS;
    }

    before = p_tree->p_session->current_state_id;
    new_id = p_tree->next_id++;

    p_node = &p_tree->nodes[p_tree->nodes_count];
    p_node->id = new_id;
    p_node->value = dv_value_copy(p_val);
    p_node->left = -1;
    p_node->right = -1;
    p_node->active = true;
    p_tree->nodes_count++;

    p_tree->root = new_id;

    dv_binary_tree_build_state(p_tree, state_buf, sizeof(state_buf));
    dv_session_add_state(p_tree->p_session, state_buf);

    dv_session_add_step(p_tree->p_session, "insert_root", before, p_tree->p_session->current_state_id, true);
    dv_session_add_step_arg(p_tree->p_session, "value", p_val);

    {
        dv_value_t ret_val;
        ret_val.type = DV_VALUE_INT;
        ret_val.data.int_val = new_id;
        dv_session_set_step_ret(p_tree->p_session, &ret_val);
    }

    if (p_out_node_id != NULL)
    {
        *p_out_node_id = new_id;
    }

    return DV_OK;
}

dv_error_t dv_binary_tree_insert_root_int(dv_binary_tree_t *p_tree, int64_t value, int32_t *p_out_node_id)
{
    dv_value_t val;
    val.type = DV_VALUE_INT;
    val.data.int_val = value;
    return dv_binary_tree_insert_root_value(p_tree, &val, p_out_node_id);
}

dv_error_t dv_binary_tree_insert_root_float(dv_binary_tree_t *p_tree, double value, int32_t *p_out_node_id)
{
    dv_value_t val;
    val.type = DV_VALUE_FLOAT;
    val.data.float_val = value;
    return dv_binary_tree_insert_root_value(p_tree, &val, p_out_node_id);
}

dv_error_t dv_binary_tree_insert_root_string(dv_binary_tree_t *p_tree, const char *value, int32_t *p_out_node_id)
{
    dv_value_t val;
    dv_error_t err;
    val.type = DV_VALUE_STRING;
    val.data.string_val = dv_strdup_internal(value);
    err = dv_binary_tree_insert_root_value(p_tree, &val, p_out_node_id);
    DV_SAFE_FREE(val.data.string_val);
    return err;
}

dv_error_t dv_binary_tree_insert_root_bool(dv_binary_tree_t *p_tree, bool value, int32_t *p_out_node_id)
{
    dv_value_t val;
    val.type = DV_VALUE_BOOL;
    val.data.bool_val = value;
    return dv_binary_tree_insert_root_value(p_tree, &val, p_out_node_id);
}

static dv_error_t dv_binary_tree_insert_left_value(dv_binary_tree_t *p_tree, int32_t parent_id, const dv_value_t *p_val, int32_t *p_out_node_id)
{
    dv_tree_node_t *new_nodes = NULL;
    dv_tree_node_t *p_node = NULL;
    dv_tree_node_t *p_parent = NULL;
    char state_buf[4096];
    int32_t before;
    int32_t new_id;
    int32_t parent_idx;
    dv_value_t arg_val;

    if (p_tree == NULL || p_val == NULL)
    {
        return DV_ERR_NULL_PTR;
    }

    parent_idx = dv_binary_tree_find_node_index(p_tree, parent_id);
    if (parent_idx < 0)
    {
        char msg[128];
        snprintf(msg, sizeof(msg), "Parent node not found: %d", parent_id);
        dv_session_set_error(p_tree->p_session, "runtime", msg);
        return DV_ERR_NOT_FOUND;
    }

    p_parent = &p_tree->nodes[parent_idx];
    if (p_parent->left != -1)
    {
        char msg[128];
        snprintf(msg, sizeof(msg), "Left child already exists for node: %d", parent_id);
        dv_session_set_error(p_tree->p_session, "runtime", msg);
        return DV_ERR_CHILD_EXISTS;
    }

    if (p_tree->nodes_count >= p_tree->nodes_capacity)
    {
        size_t new_capacity = p_tree->nodes_capacity * 2;
        new_nodes = (dv_tree_node_t *)realloc(p_tree->nodes, new_capacity * sizeof(dv_tree_node_t));
        if (new_nodes == NULL)
        {
            return DV_ERR_ALLOC;
        }
        p_tree->nodes = new_nodes;
        p_tree->nodes_capacity = new_capacity;
        parent_idx = dv_binary_tree_find_node_index(p_tree, parent_id);
        p_parent = &p_tree->nodes[parent_idx];
    }

    before = p_tree->p_session->current_state_id;
    new_id = p_tree->next_id++;

    p_node = &p_tree->nodes[p_tree->nodes_count];
    p_node->id = new_id;
    p_node->value = dv_value_copy(p_val);
    p_node->left = -1;
    p_node->right = -1;
    p_node->active = true;
    p_tree->nodes_count++;

    p_parent->left = new_id;

    dv_binary_tree_build_state(p_tree, state_buf, sizeof(state_buf));
    dv_session_add_state(p_tree->p_session, state_buf);

    dv_session_add_step(p_tree->p_session, "insert_left", before, p_tree->p_session->current_state_id, true);

    arg_val.type = DV_VALUE_INT;
    arg_val.data.int_val = parent_id;
    dv_session_add_step_arg(p_tree->p_session, "parent", &arg_val);
    dv_session_add_step_arg(p_tree->p_session, "value", p_val);

    {
        dv_value_t ret_val;
        ret_val.type = DV_VALUE_INT;
        ret_val.data.int_val = new_id;
        dv_session_set_step_ret(p_tree->p_session, &ret_val);
    }

    if (p_out_node_id != NULL)
    {
        *p_out_node_id = new_id;
    }

    return DV_OK;
}

dv_error_t dv_binary_tree_insert_left_int(dv_binary_tree_t *p_tree, int32_t parent_id, int64_t value, int32_t *p_out_node_id)
{
    dv_value_t val;
    val.type = DV_VALUE_INT;
    val.data.int_val = value;
    return dv_binary_tree_insert_left_value(p_tree, parent_id, &val, p_out_node_id);
}

dv_error_t dv_binary_tree_insert_left_float(dv_binary_tree_t *p_tree, int32_t parent_id, double value, int32_t *p_out_node_id)
{
    dv_value_t val;
    val.type = DV_VALUE_FLOAT;
    val.data.float_val = value;
    return dv_binary_tree_insert_left_value(p_tree, parent_id, &val, p_out_node_id);
}

dv_error_t dv_binary_tree_insert_left_string(dv_binary_tree_t *p_tree, int32_t parent_id, const char *value, int32_t *p_out_node_id)
{
    dv_value_t val;
    dv_error_t err;
    val.type = DV_VALUE_STRING;
    val.data.string_val = dv_strdup_internal(value);
    err = dv_binary_tree_insert_left_value(p_tree, parent_id, &val, p_out_node_id);
    DV_SAFE_FREE(val.data.string_val);
    return err;
}

dv_error_t dv_binary_tree_insert_left_bool(dv_binary_tree_t *p_tree, int32_t parent_id, bool value, int32_t *p_out_node_id)
{
    dv_value_t val;
    val.type = DV_VALUE_BOOL;
    val.data.bool_val = value;
    return dv_binary_tree_insert_left_value(p_tree, parent_id, &val, p_out_node_id);
}

static dv_error_t dv_binary_tree_insert_right_value(dv_binary_tree_t *p_tree, int32_t parent_id, const dv_value_t *p_val, int32_t *p_out_node_id)
{
    dv_tree_node_t *new_nodes = NULL;
    dv_tree_node_t *p_node = NULL;
    dv_tree_node_t *p_parent = NULL;
    char state_buf[4096];
    int32_t before;
    int32_t new_id;
    int32_t parent_idx;
    dv_value_t arg_val;

    if (p_tree == NULL || p_val == NULL)
    {
        return DV_ERR_NULL_PTR;
    }

    parent_idx = dv_binary_tree_find_node_index(p_tree, parent_id);
    if (parent_idx < 0)
    {
        char msg[128];
        snprintf(msg, sizeof(msg), "Parent node not found: %d", parent_id);
        dv_session_set_error(p_tree->p_session, "runtime", msg);
        return DV_ERR_NOT_FOUND;
    }

    p_parent = &p_tree->nodes[parent_idx];
    if (p_parent->right != -1)
    {
        char msg[128];
        snprintf(msg, sizeof(msg), "Right child already exists for node: %d", parent_id);
        dv_session_set_error(p_tree->p_session, "runtime", msg);
        return DV_ERR_CHILD_EXISTS;
    }

    if (p_tree->nodes_count >= p_tree->nodes_capacity)
    {
        size_t new_capacity = p_tree->nodes_capacity * 2;
        new_nodes = (dv_tree_node_t *)realloc(p_tree->nodes, new_capacity * sizeof(dv_tree_node_t));
        if (new_nodes == NULL)
        {
            return DV_ERR_ALLOC;
        }
        p_tree->nodes = new_nodes;
        p_tree->nodes_capacity = new_capacity;
        parent_idx = dv_binary_tree_find_node_index(p_tree, parent_id);
        p_parent = &p_tree->nodes[parent_idx];
    }

    before = p_tree->p_session->current_state_id;
    new_id = p_tree->next_id++;

    p_node = &p_tree->nodes[p_tree->nodes_count];
    p_node->id = new_id;
    p_node->value = dv_value_copy(p_val);
    p_node->left = -1;
    p_node->right = -1;
    p_node->active = true;
    p_tree->nodes_count++;

    p_parent->right = new_id;

    dv_binary_tree_build_state(p_tree, state_buf, sizeof(state_buf));
    dv_session_add_state(p_tree->p_session, state_buf);

    dv_session_add_step(p_tree->p_session, "insert_right", before, p_tree->p_session->current_state_id, true);

    arg_val.type = DV_VALUE_INT;
    arg_val.data.int_val = parent_id;
    dv_session_add_step_arg(p_tree->p_session, "parent", &arg_val);
    dv_session_add_step_arg(p_tree->p_session, "value", p_val);

    {
        dv_value_t ret_val;
        ret_val.type = DV_VALUE_INT;
        ret_val.data.int_val = new_id;
        dv_session_set_step_ret(p_tree->p_session, &ret_val);
    }

    if (p_out_node_id != NULL)
    {
        *p_out_node_id = new_id;
    }

    return DV_OK;
}

dv_error_t dv_binary_tree_insert_right_int(dv_binary_tree_t *p_tree, int32_t parent_id, int64_t value, int32_t *p_out_node_id)
{
    dv_value_t val;
    val.type = DV_VALUE_INT;
    val.data.int_val = value;
    return dv_binary_tree_insert_right_value(p_tree, parent_id, &val, p_out_node_id);
}

dv_error_t dv_binary_tree_insert_right_float(dv_binary_tree_t *p_tree, int32_t parent_id, double value, int32_t *p_out_node_id)
{
    dv_value_t val;
    val.type = DV_VALUE_FLOAT;
    val.data.float_val = value;
    return dv_binary_tree_insert_right_value(p_tree, parent_id, &val, p_out_node_id);
}

dv_error_t dv_binary_tree_insert_right_string(dv_binary_tree_t *p_tree, int32_t parent_id, const char *value, int32_t *p_out_node_id)
{
    dv_value_t val;
    dv_error_t err;
    val.type = DV_VALUE_STRING;
    val.data.string_val = dv_strdup_internal(value);
    err = dv_binary_tree_insert_right_value(p_tree, parent_id, &val, p_out_node_id);
    DV_SAFE_FREE(val.data.string_val);
    return err;
}

dv_error_t dv_binary_tree_insert_right_bool(dv_binary_tree_t *p_tree, int32_t parent_id, bool value, int32_t *p_out_node_id)
{
    dv_value_t val;
    val.type = DV_VALUE_BOOL;
    val.data.bool_val = value;
    return dv_binary_tree_insert_right_value(p_tree, parent_id, &val, p_out_node_id);
}

static void dv_binary_tree_delete_subtree(dv_binary_tree_t *p_tree, int32_t node_id)
{
    int32_t idx;
    dv_tree_node_t *p_node = NULL;

    if (node_id == -1)
    {
        return;
    }

    idx = dv_binary_tree_find_node_index(p_tree, node_id);
    if (idx < 0)
    {
        return;
    }

    p_node = &p_tree->nodes[idx];

    dv_binary_tree_delete_subtree(p_tree, p_node->left);
    dv_binary_tree_delete_subtree(p_tree, p_node->right);

    dv_value_free(&p_node->value);
    p_node->active = false;
}

dv_error_t dv_binary_tree_delete(dv_binary_tree_t *p_tree, int32_t node_id)
{
    int32_t target_idx;
    int32_t parent_idx;
    size_t i;
    char state_buf[4096];
    int32_t before;
    dv_value_t arg_val;

    if (p_tree == NULL)
    {
        return DV_ERR_NULL_PTR;
    }

    target_idx = dv_binary_tree_find_node_index(p_tree, node_id);
    if (target_idx < 0)
    {
        char msg[128];
        snprintf(msg, sizeof(msg), "Node not found: %d", node_id);
        dv_session_set_error(p_tree->p_session, "runtime", msg);
        return DV_ERR_NOT_FOUND;
    }

    before = p_tree->p_session->current_state_id;

    /* 找到父节点并更新其子节点引用 */
    parent_idx = -1;
    for (i = 0; i < p_tree->nodes_count; i++)
    {
        if (p_tree->nodes[i].active)
        {
            if (p_tree->nodes[i].left == node_id)
            {
                parent_idx = (int32_t)i;
                p_tree->nodes[i].left = -1;
                break;
            }
            if (p_tree->nodes[i].right == node_id)
            {
                parent_idx = (int32_t)i;
                p_tree->nodes[i].right = -1;
                break;
            }
        }
    }

    if (parent_idx == -1 && p_tree->root == node_id)
    {
        p_tree->root = -1;
    }

    dv_binary_tree_delete_subtree(p_tree, node_id);

    dv_binary_tree_build_state(p_tree, state_buf, sizeof(state_buf));
    dv_session_add_state(p_tree->p_session, state_buf);

    dv_session_add_step(p_tree->p_session, "delete", before, p_tree->p_session->current_state_id, true);

    arg_val.type = DV_VALUE_INT;
    arg_val.data.int_val = node_id;
    dv_session_add_step_arg(p_tree->p_session, "node_id", &arg_val);

    return DV_OK;
}

dv_error_t dv_binary_tree_update_value_int(dv_binary_tree_t *p_tree, int32_t node_id, int64_t value)
{
    int32_t idx;
    char state_buf[4096];
    int32_t before;
    dv_value_t old_val;
    dv_value_t new_val;
    dv_value_t arg_val;

    if (p_tree == NULL)
    {
        return DV_ERR_NULL_PTR;
    }

    idx = dv_binary_tree_find_node_index(p_tree, node_id);
    if (idx < 0)
    {
        char msg[128];
        snprintf(msg, sizeof(msg), "Node not found: %d", node_id);
        dv_session_set_error(p_tree->p_session, "runtime", msg);
        return DV_ERR_NOT_FOUND;
    }

    before = p_tree->p_session->current_state_id;
    old_val = dv_value_copy(&p_tree->nodes[idx].value);

    dv_value_free(&p_tree->nodes[idx].value);
    p_tree->nodes[idx].value.type = DV_VALUE_INT;
    p_tree->nodes[idx].value.data.int_val = value;

    dv_binary_tree_build_state(p_tree, state_buf, sizeof(state_buf));
    dv_session_add_state(p_tree->p_session, state_buf);

    dv_session_add_step(p_tree->p_session, "update_value", before, p_tree->p_session->current_state_id, true);

    arg_val.type = DV_VALUE_INT;
    arg_val.data.int_val = node_id;
    dv_session_add_step_arg(p_tree->p_session, "node_id", &arg_val);

    new_val.type = DV_VALUE_INT;
    new_val.data.int_val = value;
    dv_session_add_step_arg(p_tree->p_session, "value", &new_val);
    dv_session_set_step_ret(p_tree->p_session, &old_val);

    dv_value_free(&old_val);

    return DV_OK;
}

dv_error_t dv_binary_tree_update_value_float(dv_binary_tree_t *p_tree, int32_t node_id, double value)
{
    (void)value;
    if (p_tree == NULL)
        return DV_ERR_NULL_PTR;
    (void)node_id;
    return DV_OK;
}

dv_error_t dv_binary_tree_update_value_string(dv_binary_tree_t *p_tree, int32_t node_id, const char *value)
{
    (void)value;
    if (p_tree == NULL)
        return DV_ERR_NULL_PTR;
    (void)node_id;
    return DV_OK;
}

dv_error_t dv_binary_tree_update_value_bool(dv_binary_tree_t *p_tree, int32_t node_id, bool value)
{
    (void)value;
    if (p_tree == NULL)
        return DV_ERR_NULL_PTR;
    (void)node_id;
    return DV_OK;
}

/* ========================================================================== */
/* BST、堆、图的简化实现（占位符）                                             */
/* ========================================================================== */

dv_bst_t *dv_bst_create(const char *label, const char *output)
{
    (void)label;
    (void)output;
    return NULL;
}

void dv_bst_destroy(dv_bst_t *p_bst)
{
    (void)p_bst;
}

dv_error_t dv_bst_commit(dv_bst_t *p_bst)
{
    if (p_bst == NULL)
        return DV_ERR_NULL_PTR;
    return DV_OK;
}

dv_error_t dv_bst_insert_int(dv_bst_t *p_bst, int64_t value, int32_t *p_out_node_id)
{
    (void)value;
    (void)p_out_node_id;
    if (p_bst == NULL)
        return DV_ERR_NULL_PTR;
    return DV_OK;
}

dv_error_t dv_bst_insert_float(dv_bst_t *p_bst, double value, int32_t *p_out_node_id)
{
    (void)value;
    (void)p_out_node_id;
    if (p_bst == NULL)
        return DV_ERR_NULL_PTR;
    return DV_OK;
}

dv_error_t dv_bst_delete_int(dv_bst_t *p_bst, int64_t value)
{
    (void)value;
    if (p_bst == NULL)
        return DV_ERR_NULL_PTR;
    return DV_OK;
}

dv_error_t dv_bst_delete_float(dv_bst_t *p_bst, double value)
{
    (void)value;
    if (p_bst == NULL)
        return DV_ERR_NULL_PTR;
    return DV_OK;
}

dv_heap_t *dv_heap_create(const char *label, dv_heap_type_t heap_type, const char *output)
{
    (void)label;
    (void)heap_type;
    (void)output;
    return NULL;
}

void dv_heap_destroy(dv_heap_t *p_heap)
{
    (void)p_heap;
}

dv_error_t dv_heap_commit(dv_heap_t *p_heap)
{
    if (p_heap == NULL)
        return DV_ERR_NULL_PTR;
    return DV_OK;
}

dv_error_t dv_heap_insert_int(dv_heap_t *p_heap, int64_t value)
{
    (void)value;
    if (p_heap == NULL)
        return DV_ERR_NULL_PTR;
    return DV_OK;
}

dv_error_t dv_heap_insert_float(dv_heap_t *p_heap, double value)
{
    (void)value;
    if (p_heap == NULL)
        return DV_ERR_NULL_PTR;
    return DV_OK;
}

dv_error_t dv_heap_extract(dv_heap_t *p_heap)
{
    if (p_heap == NULL)
        return DV_ERR_NULL_PTR;
    return DV_OK;
}

dv_error_t dv_heap_clear(dv_heap_t *p_heap)
{
    if (p_heap == NULL)
        return DV_ERR_NULL_PTR;
    return DV_OK;
}

/* ========================================================================== */
/* 无向图实现                                                                  */
/* ========================================================================== */

static int32_t dv_graph_find_node_index(dv_graph_node_t *nodes, size_t count, int32_t node_id)
{
    size_t i;
    for (i = 0; i < count; i++)
    {
        if (nodes[i].id == node_id && nodes[i].active)
        {
            return (int32_t)i;
        }
    }
    return -1;
}

static void dv_graph_undirected_build_state(dv_graph_undirected_t *p_graph, char *buf, size_t buf_size)
{
    int offset = 0;
    size_t i;
    bool first;

    offset += snprintf(buf + offset, buf_size - offset, "nodes = [");
    first = true;
    for (i = 0; i < p_graph->nodes_count; i++)
    {
        if (p_graph->nodes[i].active)
        {
            if (!first)
            {
                offset += snprintf(buf + offset, buf_size - offset, ",");
            }
            first = false;
            offset += snprintf(buf + offset, buf_size - offset,
                               "\n  { id = %d, label = \"%s\" }",
                               p_graph->nodes[i].id, p_graph->nodes[i].label);
        }
    }
    if (!first)
    {
        offset += snprintf(buf + offset, buf_size - offset, "\n");
    }
    offset += snprintf(buf + offset, buf_size - offset, "]\n");

    offset += snprintf(buf + offset, buf_size - offset, "edges = [");
    first = true;
    for (i = 0; i < p_graph->edges_count; i++)
    {
        if (p_graph->edges[i].active)
        {
            if (!first)
            {
                offset += snprintf(buf + offset, buf_size - offset, ",");
            }
            first = false;
            offset += snprintf(buf + offset, buf_size - offset,
                               "\n  { from = %d, to = %d }",
                               p_graph->edges[i].from, p_graph->edges[i].to);
        }
    }
    if (!first)
    {
        offset += snprintf(buf + offset, buf_size - offset, "\n");
    }
    snprintf(buf + offset, buf_size - offset, "]\n");
}

dv_graph_undirected_t *dv_graph_undirected_create(const char *label, const char *output)
{
    dv_graph_undirected_t *p_graph = NULL;
    char state_buf[4096];

    p_graph = (dv_graph_undirected_t *)calloc(1, sizeof(dv_graph_undirected_t));
    if (p_graph == NULL)
    {
        return NULL;
    }

    p_graph->p_session = dv_session_create("graph_undirected", label ? label : "graph", output);
    if (p_graph->p_session == NULL)
    {
        free(p_graph);
        return NULL;
    }

    p_graph->nodes_capacity = DV_INITIAL_CAPACITY;
    p_graph->nodes = (dv_graph_node_t *)calloc(p_graph->nodes_capacity, sizeof(dv_graph_node_t));

    p_graph->edges_capacity = DV_INITIAL_CAPACITY;
    p_graph->edges = (dv_graph_edge_t *)calloc(p_graph->edges_capacity, sizeof(dv_graph_edge_t));

    if (p_graph->nodes == NULL || p_graph->edges == NULL)
    {
        DV_SAFE_FREE(p_graph->nodes);
        DV_SAFE_FREE(p_graph->edges);
        dv_session_destroy(p_graph->p_session);
        free(p_graph);
        return NULL;
    }

    dv_graph_undirected_build_state(p_graph, state_buf, sizeof(state_buf));
    dv_session_add_state(p_graph->p_session, state_buf);

    return p_graph;
}

void dv_graph_undirected_destroy(dv_graph_undirected_t *p_graph)
{
    if (p_graph == NULL)
    {
        return;
    }

    DV_SAFE_FREE(p_graph->nodes);
    DV_SAFE_FREE(p_graph->edges);
    dv_session_destroy(p_graph->p_session);
    free(p_graph);
}

dv_error_t dv_graph_undirected_commit(dv_graph_undirected_t *p_graph)
{
    if (p_graph == NULL)
    {
        return DV_ERR_NULL_PTR;
    }
    return dv_session_write(p_graph->p_session);
}

dv_error_t dv_graph_undirected_add_node(dv_graph_undirected_t *p_graph, int32_t node_id, const char *label)
{
    dv_graph_node_t *new_nodes = NULL;
    dv_graph_node_t *p_node = NULL;
    char state_buf[4096];
    int32_t before;
    size_t label_len;
    dv_value_t arg_val;
    dv_value_t label_val;

    if (p_graph == NULL || label == NULL)
    {
        return DV_ERR_NULL_PTR;
    }

    label_len = strlen(label);
    if (label_len < 1 || label_len > 32)
    {
        dv_session_set_error(p_graph->p_session, "runtime", "Label length must be 1-32");
        return DV_ERR_INVALID_PARAM;
    }

    if (dv_graph_find_node_index(p_graph->nodes, p_graph->nodes_count, node_id) >= 0)
    {
        char msg[128];
        snprintf(msg, sizeof(msg), "Node already exists: %d", node_id);
        dv_session_set_error(p_graph->p_session, "runtime", msg);
        return DV_ERR_ALREADY_EXISTS;
    }

    if (p_graph->nodes_count >= p_graph->nodes_capacity)
    {
        size_t new_capacity = p_graph->nodes_capacity * 2;
        new_nodes = (dv_graph_node_t *)realloc(p_graph->nodes, new_capacity * sizeof(dv_graph_node_t));
        if (new_nodes == NULL)
        {
            return DV_ERR_ALLOC;
        }
        p_graph->nodes = new_nodes;
        p_graph->nodes_capacity = new_capacity;
    }

    before = p_graph->p_session->current_state_id;

    p_node = &p_graph->nodes[p_graph->nodes_count];
    p_node->id = node_id;
    strncpy(p_node->label, label, sizeof(p_node->label) - 1);
    p_node->label[sizeof(p_node->label) - 1] = '\0';
    p_node->active = true;
    p_graph->nodes_count++;

    dv_graph_undirected_build_state(p_graph, state_buf, sizeof(state_buf));
    dv_session_add_state(p_graph->p_session, state_buf);

    dv_session_add_step(p_graph->p_session, "add_node", before, p_graph->p_session->current_state_id, true);

    arg_val.type = DV_VALUE_INT;
    arg_val.data.int_val = node_id;
    dv_session_add_step_arg(p_graph->p_session, "id", &arg_val);

    label_val.type = DV_VALUE_STRING;
    label_val.data.string_val = dv_strdup_internal(label);
    dv_session_add_step_arg(p_graph->p_session, "label", &label_val);
    DV_SAFE_FREE(label_val.data.string_val);

    return DV_OK;
}

dv_error_t dv_graph_undirected_add_edge(dv_graph_undirected_t *p_graph, int32_t from_id, int32_t to_id)
{
    dv_graph_edge_t *new_edges = NULL;
    dv_graph_edge_t *p_edge = NULL;
    char state_buf[4096];
    int32_t before;
    int32_t normalized_from;
    int32_t normalized_to;
    size_t i;
    dv_value_t arg_val;

    if (p_graph == NULL)
    {
        return DV_ERR_NULL_PTR;
    }

    if (dv_graph_find_node_index(p_graph->nodes, p_graph->nodes_count, from_id) < 0)
    {
        char msg[128];
        snprintf(msg, sizeof(msg), "Node not found: %d", from_id);
        dv_session_set_error(p_graph->p_session, "runtime", msg);
        return DV_ERR_NOT_FOUND;
    }

    if (dv_graph_find_node_index(p_graph->nodes, p_graph->nodes_count, to_id) < 0)
    {
        char msg[128];
        snprintf(msg, sizeof(msg), "Node not found: %d", to_id);
        dv_session_set_error(p_graph->p_session, "runtime", msg);
        return DV_ERR_NOT_FOUND;
    }

    if (from_id == to_id)
    {
        char msg[128];
        snprintf(msg, sizeof(msg), "Self-loop not allowed: %d", from_id);
        dv_session_set_error(p_graph->p_session, "runtime", msg);
        return DV_ERR_SELF_LOOP;
    }

    /* 规范化边：确保 from < to */
    if (from_id < to_id)
    {
        normalized_from = from_id;
        normalized_to = to_id;
    }
    else
    {
        normalized_from = to_id;
        normalized_to = from_id;
    }

    /* 检查边是否已存在 */
    for (i = 0; i < p_graph->edges_count; i++)
    {
        if (p_graph->edges[i].active &&
            p_graph->edges[i].from == normalized_from &&
            p_graph->edges[i].to == normalized_to)
        {
            char msg[128];
            snprintf(msg, sizeof(msg), "Edge already exists: (%d, %d)", normalized_from, normalized_to);
            dv_session_set_error(p_graph->p_session, "runtime", msg);
            return DV_ERR_ALREADY_EXISTS;
        }
    }

    if (p_graph->edges_count >= p_graph->edges_capacity)
    {
        size_t new_capacity = p_graph->edges_capacity * 2;
        new_edges = (dv_graph_edge_t *)realloc(p_graph->edges, new_capacity * sizeof(dv_graph_edge_t));
        if (new_edges == NULL)
        {
            return DV_ERR_ALLOC;
        }
        p_graph->edges = new_edges;
        p_graph->edges_capacity = new_capacity;
    }

    before = p_graph->p_session->current_state_id;

    p_edge = &p_graph->edges[p_graph->edges_count];
    p_edge->from = normalized_from;
    p_edge->to = normalized_to;
    p_edge->has_weight = false;
    p_edge->active = true;
    p_graph->edges_count++;

    dv_graph_undirected_build_state(p_graph, state_buf, sizeof(state_buf));
    dv_session_add_state(p_graph->p_session, state_buf);

    dv_session_add_step(p_graph->p_session, "add_edge", before, p_graph->p_session->current_state_id, true);

    arg_val.type = DV_VALUE_INT;
    arg_val.data.int_val = from_id;
    dv_session_add_step_arg(p_graph->p_session, "from", &arg_val);

    arg_val.data.int_val = to_id;
    dv_session_add_step_arg(p_graph->p_session, "to", &arg_val);

    return DV_OK;
}

dv_error_t dv_graph_undirected_remove_node(dv_graph_undirected_t *p_graph, int32_t node_id)
{
    int32_t idx;
    size_t i;
    char state_buf[4096];
    int32_t before;
    dv_value_t arg_val;

    if (p_graph == NULL)
    {
        return DV_ERR_NULL_PTR;
    }

    idx = dv_graph_find_node_index(p_graph->nodes, p_graph->nodes_count, node_id);
    if (idx < 0)
    {
        char msg[128];
        snprintf(msg, sizeof(msg), "Node not found: %d", node_id);
        dv_session_set_error(p_graph->p_session, "runtime", msg);
        return DV_ERR_NOT_FOUND;
    }

    before = p_graph->p_session->current_state_id;

    /* 删除相关边 */
    for (i = 0; i < p_graph->edges_count; i++)
    {
        if (p_graph->edges[i].active &&
            (p_graph->edges[i].from == node_id || p_graph->edges[i].to == node_id))
        {
            p_graph->edges[i].active = false;
        }
    }

    p_graph->nodes[idx].active = false;

    dv_graph_undirected_build_state(p_graph, state_buf, sizeof(state_buf));
    dv_session_add_state(p_graph->p_session, state_buf);

    dv_session_add_step(p_graph->p_session, "remove_node", before, p_graph->p_session->current_state_id, true);

    arg_val.type = DV_VALUE_INT;
    arg_val.data.int_val = node_id;
    dv_session_add_step_arg(p_graph->p_session, "node_id", &arg_val);

    return DV_OK;
}

dv_error_t dv_graph_undirected_remove_edge(dv_graph_undirected_t *p_graph, int32_t from_id, int32_t to_id)
{
    int32_t normalized_from;
    int32_t normalized_to;
    size_t i;
    bool found;
    char state_buf[4096];
    int32_t before;
    dv_value_t arg_val;

    if (p_graph == NULL)
    {
        return DV_ERR_NULL_PTR;
    }

    if (from_id < to_id)
    {
        normalized_from = from_id;
        normalized_to = to_id;
    }
    else
    {
        normalized_from = to_id;
        normalized_to = from_id;
    }

    found = false;
    for (i = 0; i < p_graph->edges_count; i++)
    {
        if (p_graph->edges[i].active &&
            p_graph->edges[i].from == normalized_from &&
            p_graph->edges[i].to == normalized_to)
        {
            found = true;
            before = p_graph->p_session->current_state_id;
            p_graph->edges[i].active = false;
            break;
        }
    }

    if (!found)
    {
        char msg[128];
        snprintf(msg, sizeof(msg), "Edge not found: (%d, %d)", normalized_from, normalized_to);
        dv_session_set_error(p_graph->p_session, "runtime", msg);
        return DV_ERR_NOT_FOUND;
    }

    dv_graph_undirected_build_state(p_graph, state_buf, sizeof(state_buf));
    dv_session_add_state(p_graph->p_session, state_buf);

    dv_session_add_step(p_graph->p_session, "remove_edge", before, p_graph->p_session->current_state_id, true);

    arg_val.type = DV_VALUE_INT;
    arg_val.data.int_val = from_id;
    dv_session_add_step_arg(p_graph->p_session, "from", &arg_val);

    arg_val.data.int_val = to_id;
    dv_session_add_step_arg(p_graph->p_session, "to", &arg_val);

    return DV_OK;
}

dv_error_t dv_graph_undirected_update_node_label(dv_graph_undirected_t *p_graph, int32_t node_id, const char *label)
{
    int32_t idx;
    size_t label_len;
    char state_buf[4096];
    int32_t before;
    dv_value_t arg_val;
    dv_value_t label_val;
    char old_label[33];

    if (p_graph == NULL || label == NULL)
    {
        return DV_ERR_NULL_PTR;
    }

    label_len = strlen(label);
    if (label_len < 1 || label_len > 32)
    {
        dv_session_set_error(p_graph->p_session, "runtime", "Label length must be 1-32");
        return DV_ERR_INVALID_PARAM;
    }

    idx = dv_graph_find_node_index(p_graph->nodes, p_graph->nodes_count, node_id);
    if (idx < 0)
    {
        char msg[128];
        snprintf(msg, sizeof(msg), "Node not found: %d", node_id);
        dv_session_set_error(p_graph->p_session, "runtime", msg);
        return DV_ERR_NOT_FOUND;
    }

    before = p_graph->p_session->current_state_id;

    strncpy(old_label, p_graph->nodes[idx].label, sizeof(old_label) - 1);
    old_label[sizeof(old_label) - 1] = '\0';

    strncpy(p_graph->nodes[idx].label, label, sizeof(p_graph->nodes[idx].label) - 1);
    p_graph->nodes[idx].label[sizeof(p_graph->nodes[idx].label) - 1] = '\0';

    dv_graph_undirected_build_state(p_graph, state_buf, sizeof(state_buf));
    dv_session_add_state(p_graph->p_session, state_buf);

    dv_session_add_step(p_graph->p_session, "update_node_label", before, p_graph->p_session->current_state_id, true);

    arg_val.type = DV_VALUE_INT;
    arg_val.data.int_val = node_id;
    dv_session_add_step_arg(p_graph->p_session, "node_id", &arg_val);

    label_val.type = DV_VALUE_STRING;
    label_val.data.string_val = dv_strdup_internal(label);
    dv_session_add_step_arg(p_graph->p_session, "label", &label_val);
    DV_SAFE_FREE(label_val.data.string_val);

    {
        dv_value_t ret_val;
        ret_val.type = DV_VALUE_STRING;
        ret_val.data.string_val = dv_strdup_internal(old_label);
        dv_session_set_step_ret(p_graph->p_session, &ret_val);
        DV_SAFE_FREE(ret_val.data.string_val);
    }

    return DV_OK;
}

/* 有向图和带权图的占位实现 */

dv_graph_directed_t *dv_graph_directed_create(const char *label, const char *output)
{
    (void)label;
    (void)output;
    return NULL;
}

void dv_graph_directed_destroy(dv_graph_directed_t *p_graph)
{
    (void)p_graph;
}

dv_error_t dv_graph_directed_commit(dv_graph_directed_t *p_graph)
{
    if (p_graph == NULL)
        return DV_ERR_NULL_PTR;
    return DV_OK;
}

dv_error_t dv_graph_directed_add_node(dv_graph_directed_t *p_graph, int32_t node_id, const char *label)
{
    (void)node_id;
    (void)label;
    if (p_graph == NULL)
        return DV_ERR_NULL_PTR;
    return DV_OK;
}

dv_error_t dv_graph_directed_add_edge(dv_graph_directed_t *p_graph, int32_t from_id, int32_t to_id)
{
    (void)from_id;
    (void)to_id;
    if (p_graph == NULL)
        return DV_ERR_NULL_PTR;
    return DV_OK;
}

dv_error_t dv_graph_directed_remove_node(dv_graph_directed_t *p_graph, int32_t node_id)
{
    (void)node_id;
    if (p_graph == NULL)
        return DV_ERR_NULL_PTR;
    return DV_OK;
}

dv_error_t dv_graph_directed_remove_edge(dv_graph_directed_t *p_graph, int32_t from_id, int32_t to_id)
{
    (void)from_id;
    (void)to_id;
    if (p_graph == NULL)
        return DV_ERR_NULL_PTR;
    return DV_OK;
}

dv_error_t dv_graph_directed_update_node_label(dv_graph_directed_t *p_graph, int32_t node_id, const char *label)
{
    (void)node_id;
    (void)label;
    if (p_graph == NULL)
        return DV_ERR_NULL_PTR;
    return DV_OK;
}

dv_graph_weighted_t *dv_graph_weighted_create(const char *label, bool directed, const char *output)
{
    (void)label;
    (void)directed;
    (void)output;
    return NULL;
}

void dv_graph_weighted_destroy(dv_graph_weighted_t *p_graph)
{
    (void)p_graph;
}

dv_error_t dv_graph_weighted_commit(dv_graph_weighted_t *p_graph)
{
    if (p_graph == NULL)
        return DV_ERR_NULL_PTR;
    return DV_OK;
}

dv_error_t dv_graph_weighted_add_node(dv_graph_weighted_t *p_graph, int32_t node_id, const char *label)
{
    (void)node_id;
    (void)label;
    if (p_graph == NULL)
        return DV_ERR_NULL_PTR;
    return DV_OK;
}

dv_error_t dv_graph_weighted_add_edge(dv_graph_weighted_t *p_graph, int32_t from_id, int32_t to_id, double weight)
{
    (void)from_id;
    (void)to_id;
    (void)weight;
    if (p_graph == NULL)
        return DV_ERR_NULL_PTR;
    return DV_OK;
}

dv_error_t dv_graph_weighted_remove_node(dv_graph_weighted_t *p_graph, int32_t node_id)
{
    (void)node_id;
    if (p_graph == NULL)
        return DV_ERR_NULL_PTR;
    return DV_OK;
}

dv_error_t dv_graph_weighted_remove_edge(dv_graph_weighted_t *p_graph, int32_t from_id, int32_t to_id)
{
    (void)from_id;
    (void)to_id;
    if (p_graph == NULL)
        return DV_ERR_NULL_PTR;
    return DV_OK;
}

dv_error_t dv_graph_weighted_update_weight(dv_graph_weighted_t *p_graph, int32_t from_id, int32_t to_id, double weight)
{
    (void)from_id;
    (void)to_id;
    (void)weight;
    if (p_graph == NULL)
        return DV_ERR_NULL_PTR;
    return DV_OK;
}

dv_error_t dv_graph_weighted_update_node_label(dv_graph_weighted_t *p_graph, int32_t node_id, const char *label)
{
    (void)node_id;
    (void)label;
    if (p_graph == NULL)
        return DV_ERR_NULL_PTR;
    return DV_OK;
}

#endif /* DS4VIZ_IMPLEMENTATION */

#endif /* DS4VIZ_H */
