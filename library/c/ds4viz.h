/**
 * @file ds4viz.h
 * @brief ds4viz - 数据结构可视化 C23 单头文件库
 *
 * 生成与 Python ds4viz 库完全对齐的 TOML IR 文件,
 * 用于可扩展的数据结构可视化教学平台.
 *
 * 用法:
 *     // 在恰好一个 .c 文件中:
 *     #define DS4VIZ_IMPLEMENTATION
 *     #include "ds4viz.h"
 *
 *     // 在其他使用 API 的文件中:
 *     #include "ds4viz.h"
 *
 * 可选:
 *     #define DS4VIZ_SHORT_NAMES   // 启用 dv... 短名前缀
 *
 * @author WaterRun
 * @date 2025-06-15
 * @version 0.1.0
 * @see https://github.com/Water-Run/ds4viz
 */

#ifndef DS4VIZ_H
#define DS4VIZ_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>
#include <math.h>
#include <time.h>

#ifdef __cplusplus
extern "C"
{
#endif

    /* ================================================================
     * 公开类型
     * ================================================================ */

    /**
     * @brief 全局配置选项
     *
     * @struct ds4vizConfigOptions
     */
    typedef struct
    {
        /** @brief 输出文件路径, NULL 则默认为 "trace.toml" */
        const char *output_path;
        /** @brief 可视化标题, NULL 则不输出 */
        const char *title;
        /** @brief 作者信息, NULL 则不输出 */
        const char *author;
        /** @brief 注释说明, NULL 则不输出 */
        const char *comment;
    } ds4vizConfigOptions;

    /**
     * @brief 堆序类型枚举
     *
     * @enum ds4vizHeapOrder
     */
    typedef enum
    {
        /** @brief 最小堆 */
        ds4vizHeapOrderMin = 0,
        /** @brief 最大堆 */
        ds4vizHeapOrderMax = 1
    } ds4vizHeapOrder;

    /* ================================================================
     * 公开函数
     * ================================================================ */

    /**
     * @brief 配置全局参数
     *
     * @param options 配置选项
     */
    void ds4vizConfig(ds4vizConfigOptions options);

    /* ================================================================
     * 内部类型 (宏使用, 请勿直接调用)
     * ================================================================ */

    /**
     * @brief 值类型标识
     *
     * @def DS4VIZ_VINT_
     * @def DS4VIZ_VDBL_
     * @def DS4VIZ_VSTR_
     * @def DS4VIZ_VBOOL_
     */
    enum
    {
        DS4VIZ_VINT_ = 0,
        DS4VIZ_VDBL_ = 1,
        DS4VIZ_VSTR_ = 2,
        DS4VIZ_VBOOL_ = 3
    };

    /**
     * @brief 通用值类型, 支持 int/double/string/bool
     */
    typedef struct
    {
        /** @brief 类型标识 */
        int type;
        union
        {
            /** @brief 整数值 */
            long long i;
            /** @brief 浮点值 */
            double d;
            /** @brief 字符串值 */
            const char *s;
            /** @brief 布尔值 */
            bool b;
        };
    } ds4vizP_val_;

    /**
     * @brief 动态字符缓冲区
     */
    typedef struct
    {
        /** @brief 数据指针 */
        char *data;
        /** @brief 当前长度 */
        int len;
        /** @brief 容量 */
        int cap;
    } ds4vizP_buf_;

/**
 * @brief 所有数据结构共有的会话字段宏
 *
 * @def DS4VIZ_COMMON_FIELDS_
 */
#define DS4VIZ_COMMON_FIELDS_ \
    ds4vizP_buf_ states_buf;  \
    ds4vizP_buf_ steps_buf;   \
    int state_id;             \
    int step_id;              \
    bool err;                 \
    char errmsg[512];         \
    int err_step;             \
    int err_last_state;       \
    int err_line;             \
    bool done;                \
    const char *label;        \
    const char *kind;         \
    int scope_line

    /* ---- 栈 ---- */

    /**
     * @brief 栈内部结构
     */
    typedef struct
    {
        DS4VIZ_COMMON_FIELDS_;
        /** @brief 元素数组 */
        ds4vizP_val_ *items;
        /** @brief 元素个数 */
        int count;
        /** @brief 容量 */
        int cap;
    } ds4vizP_stack_;

    ds4vizP_stack_ ds4vizP_stack_open_(const char *label, int line);
    void ds4vizP_stack_close_(ds4vizP_stack_ *p_s);
    void ds4vizP_stack_push_(ds4vizP_stack_ *p_s, ds4vizP_val_ v, int line);
    void ds4vizP_stack_pop_(ds4vizP_stack_ *p_s, int line);
    void ds4vizP_stack_clear_(ds4vizP_stack_ *p_s, int line);

    /* ---- 队列 ---- */

    /**
     * @brief 队列内部结构
     */
    typedef struct
    {
        DS4VIZ_COMMON_FIELDS_;
        ds4vizP_val_ *items;
        int count;
        int cap;
    } ds4vizP_queue_;

    ds4vizP_queue_ ds4vizP_queue_open_(const char *label, int line);
    void ds4vizP_queue_close_(ds4vizP_queue_ *p_q);
    void ds4vizP_queue_enqueue_(ds4vizP_queue_ *p_q, ds4vizP_val_ v, int line);
    void ds4vizP_queue_dequeue_(ds4vizP_queue_ *p_q, int line);
    void ds4vizP_queue_clear_(ds4vizP_queue_ *p_q, int line);

    /* ---- 单链表 ---- */

    /**
     * @brief 单链表节点
     */
    typedef struct
    {
        int id;
        ds4vizP_val_ value;
        int next;
        bool alive;
    } ds4vizP_slnode_;

    /**
     * @brief 单链表内部结构
     */
    typedef struct
    {
        DS4VIZ_COMMON_FIELDS_;
        ds4vizP_slnode_ *nodes;
        int ncnt;
        int ncap;
        int head;
        int nid;
    } ds4vizP_slist_;

    ds4vizP_slist_ ds4vizP_slist_open_(const char *label, int line);
    void ds4vizP_slist_close_(ds4vizP_slist_ *p_l);
    int ds4vizP_slist_insert_head_(ds4vizP_slist_ *p_l, ds4vizP_val_ v, int line);
    int ds4vizP_slist_insert_tail_(ds4vizP_slist_ *p_l, ds4vizP_val_ v, int line);
    int ds4vizP_slist_insert_after_(ds4vizP_slist_ *p_l, int nid, ds4vizP_val_ v, int line);
    void ds4vizP_slist_delete_(ds4vizP_slist_ *p_l, int nid, int line);
    void ds4vizP_slist_delete_head_(ds4vizP_slist_ *p_l, int line);
    void ds4vizP_slist_reverse_(ds4vizP_slist_ *p_l, int line);

    /* ---- 双向链表 ---- */

    /**
     * @brief 双向链表节点
     */
    typedef struct
    {
        int id;
        ds4vizP_val_ value;
        int prev;
        int next;
        bool alive;
    } ds4vizP_dlnode_;

    /**
     * @brief 双向链表内部结构
     */
    typedef struct
    {
        DS4VIZ_COMMON_FIELDS_;
        ds4vizP_dlnode_ *nodes;
        int ncnt;
        int ncap;
        int head;
        int tail;
        int nid;
    } ds4vizP_dlist_;

    ds4vizP_dlist_ ds4vizP_dlist_open_(const char *label, int line);
    void ds4vizP_dlist_close_(ds4vizP_dlist_ *p_l);
    int ds4vizP_dlist_insert_head_(ds4vizP_dlist_ *p_l, ds4vizP_val_ v, int line);
    int ds4vizP_dlist_insert_tail_(ds4vizP_dlist_ *p_l, ds4vizP_val_ v, int line);
    int ds4vizP_dlist_insert_before_(ds4vizP_dlist_ *p_l, int nid, ds4vizP_val_ v, int line);
    int ds4vizP_dlist_insert_after_(ds4vizP_dlist_ *p_l, int nid, ds4vizP_val_ v, int line);
    void ds4vizP_dlist_delete_(ds4vizP_dlist_ *p_l, int nid, int line);
    void ds4vizP_dlist_delete_head_(ds4vizP_dlist_ *p_l, int line);
    void ds4vizP_dlist_delete_tail_(ds4vizP_dlist_ *p_l, int line);
    void ds4vizP_dlist_reverse_(ds4vizP_dlist_ *p_l, int line);

    /* ---- 二叉树 / 二叉搜索树 ---- */

    /**
     * @brief 二叉树节点
     */
    typedef struct
    {
        int id;
        ds4vizP_val_ value;
        int left;
        int right;
        bool alive;
    } ds4vizP_tnode_;

    /**
     * @brief 二叉树内部结构 (同时用于 BST)
     */
    typedef struct
    {
        DS4VIZ_COMMON_FIELDS_;
        ds4vizP_tnode_ *nodes;
        int ncnt;
        int ncap;
        int root;
        int nid;
    } ds4vizP_bt_;

    ds4vizP_bt_ ds4vizP_bt_open_(const char *kind, const char *label, int line);
    void ds4vizP_bt_close_(ds4vizP_bt_ *p_t);
    int ds4vizP_bt_insert_root_(ds4vizP_bt_ *p_t, ds4vizP_val_ v, int line);
    int ds4vizP_bt_insert_left_(ds4vizP_bt_ *p_t, int pid, ds4vizP_val_ v, int line);
    int ds4vizP_bt_insert_right_(ds4vizP_bt_ *p_t, int pid, ds4vizP_val_ v, int line);
    void ds4vizP_bt_delete_(ds4vizP_bt_ *p_t, int nid, int line);
    void ds4vizP_bt_update_value_(ds4vizP_bt_ *p_t, int nid, ds4vizP_val_ v, int line);

    int ds4vizP_bst_insert_(ds4vizP_bt_ *p_t, ds4vizP_val_ v, int line);
    void ds4vizP_bst_delete_(ds4vizP_bt_ *p_t, ds4vizP_val_ v, int line);

    /* ---- 堆 ---- */

    /**
     * @brief 堆内部结构 (序列化为二叉树形式)
     */
    typedef struct
    {
        DS4VIZ_COMMON_FIELDS_;
        ds4vizP_val_ *items;
        int count;
        int cap;
        int order;
    } ds4vizP_heap_;

    ds4vizP_heap_ ds4vizP_heap_open_(const char *label, int order, int line);
    void ds4vizP_heap_close_(ds4vizP_heap_ *p_h);
    void ds4vizP_heap_insert_(ds4vizP_heap_ *p_h, ds4vizP_val_ v, int line);
    void ds4vizP_heap_extract_(ds4vizP_heap_ *p_h, int line);
    void ds4vizP_heap_clear_(ds4vizP_heap_ *p_h, int line);

    /* ---- 图 (统一: 无向/有向/带权) ---- */

    /**
     * @brief 图节点
     */
    typedef struct
    {
        int id;
        char label[33];
        bool alive;
    } ds4vizP_gnode_;

    /**
     * @brief 图边
     */
    typedef struct
    {
        int from;
        int to;
        double weight;
        bool alive;
    } ds4vizP_gedge_;

    /**
     * @brief 图内部结构
     */
    typedef struct
    {
        DS4VIZ_COMMON_FIELDS_;
        ds4vizP_gnode_ *nodes;
        int ncnt;
        int ncap;
        ds4vizP_gedge_ *edges;
        int ecnt;
        int ecap;
        bool directed;
        bool weighted;
    } ds4vizP_graph_;

    ds4vizP_graph_ ds4vizP_graph_open_(const char *kind, const char *label,
                                       bool directed, bool weighted, int line);
    void ds4vizP_graph_close_(ds4vizP_graph_ *p_g);
    void ds4vizP_graph_add_node_(ds4vizP_graph_ *p_g, int nid, const char *lbl, int line);
    void ds4vizP_graph_add_edge_(ds4vizP_graph_ *p_g, int from, int to, double w, int line);
    void ds4vizP_graph_remove_node_(ds4vizP_graph_ *p_g, int nid, int line);
    void ds4vizP_graph_remove_edge_(ds4vizP_graph_ *p_g, int from, int to, int line);
    void ds4vizP_graph_update_node_label_(ds4vizP_graph_ *p_g, int nid, const char *lbl, int line);
    void ds4vizP_graph_update_weight_(ds4vizP_graph_ *p_g, int from, int to, double w, int line);

/* ================================================================
 * 值转换宏 (_Generic)
 * ================================================================ */

    /* ================================================================
     * 值转换辅助函数 + 宏 (_Generic)
     * ================================================================ */

    static inline ds4vizP_val_ ds4vizP_make_bool_(bool v)
    {
        return (ds4vizP_val_){.type = DS4VIZ_VBOOL_, .b = v};
    }

    static inline ds4vizP_val_ ds4vizP_make_i64_(long long v)
    {
        return (ds4vizP_val_){.type = DS4VIZ_VINT_, .i = v};
    }

    static inline ds4vizP_val_ ds4vizP_make_u64_(unsigned long long v)
    {
        return (ds4vizP_val_){.type = DS4VIZ_VINT_, .i = (long long)v};
    }

    static inline ds4vizP_val_ ds4vizP_make_dbl_(double v)
    {
        return (ds4vizP_val_){.type = DS4VIZ_VDBL_, .d = v};
    }

    static inline ds4vizP_val_ ds4vizP_make_cstr_(const char *v)
    {
        return (ds4vizP_val_){.type = DS4VIZ_VSTR_, .s = v};
    }

#define DS4VIZ_VAL_(x) _Generic((x),       \
    bool: ds4vizP_make_bool_,              \
    char: ds4vizP_make_i64_,               \
    signed char: ds4vizP_make_i64_,        \
    unsigned char: ds4vizP_make_i64_,      \
    short: ds4vizP_make_i64_,              \
    unsigned short: ds4vizP_make_i64_,     \
    int: ds4vizP_make_i64_,                \
    unsigned int: ds4vizP_make_i64_,       \
    long: ds4vizP_make_i64_,               \
    unsigned long: ds4vizP_make_i64_,      \
    long long: ds4vizP_make_i64_,          \
    unsigned long long: ds4vizP_make_u64_, \
    float: ds4vizP_make_dbl_,              \
    double: ds4vizP_make_dbl_,             \
    char *: ds4vizP_make_cstr_,            \
    const char *: ds4vizP_make_cstr_)(x)

#define DS4VIZ_NUMVAL_(x) _Generic((x),    \
    char: ds4vizP_make_i64_,               \
    signed char: ds4vizP_make_i64_,        \
    unsigned char: ds4vizP_make_i64_,      \
    short: ds4vizP_make_i64_,              \
    unsigned short: ds4vizP_make_i64_,     \
    int: ds4vizP_make_i64_,                \
    unsigned int: ds4vizP_make_i64_,       \
    long: ds4vizP_make_i64_,               \
    unsigned long: ds4vizP_make_i64_,      \
    long long: ds4vizP_make_i64_,          \
    unsigned long long: ds4vizP_make_u64_, \
    float: ds4vizP_make_dbl_,              \
    double: ds4vizP_make_dbl_)(x)
    
/* ---- 可选参数选择器 (C23 __VA_OPT__) ---- */

/**
 * @brief 选取第一个可选参数, 若无则使用默认值
 */
#define DS4VIZ_OPT1_(def, ...) DS4VIZ_PICK1_(__VA_ARGS__ __VA_OPT__(, ) def)
#define DS4VIZ_PICK1_(a, ...) a

/**
 * @brief 选取第二个可选参数, 若无则使用默认值
 */
#define DS4VIZ_OPT2_(def, ...) DS4VIZ_PICK2_(__VA_ARGS__ __VA_OPT__(, ) def, def)
#define DS4VIZ_PICK2_(a, b, ...) b

/* ================================================================
 * 作用域宏 (公开 API)
 * ================================================================ */

/* ---- 栈 ---- */
#define ds4vizStack(s, ...)                                               \
    for (ds4vizP_stack_ s = ds4vizP_stack_open_(                          \
             DS4VIZ_OPT1_("stack" __VA_OPT__(, ) __VA_ARGS__), __LINE__); \
         !s.done; ds4vizP_stack_close_(&s))

#define ds4vizStackPush(s, v) ds4vizP_stack_push_(&(s), DS4VIZ_VAL_(v), __LINE__)
#define ds4vizStackPop(s) ds4vizP_stack_pop_(&(s), __LINE__)
#define ds4vizStackClear(s) ds4vizP_stack_clear_(&(s), __LINE__)

/* ---- 队列 ---- */
#define ds4vizQueue(q, ...)                                               \
    for (ds4vizP_queue_ q = ds4vizP_queue_open_(                          \
             DS4VIZ_OPT1_("queue" __VA_OPT__(, ) __VA_ARGS__), __LINE__); \
         !q.done; ds4vizP_queue_close_(&q))

#define ds4vizQueueEnqueue(q, v) ds4vizP_queue_enqueue_(&(q), DS4VIZ_VAL_(v), __LINE__)
#define ds4vizQueueDequeue(q) ds4vizP_queue_dequeue_(&(q), __LINE__)
#define ds4vizQueueClear(q) ds4vizP_queue_clear_(&(q), __LINE__)

/* ---- 单链表 ---- */
#define ds4vizSingleLinkedList(l, ...)                                    \
    for (ds4vizP_slist_ l = ds4vizP_slist_open_(                          \
             DS4VIZ_OPT1_("slist" __VA_OPT__(, ) __VA_ARGS__), __LINE__); \
         !l.done; ds4vizP_slist_close_(&l))

#define ds4vizSlInsertHead(l, v) ds4vizP_slist_insert_head_(&(l), DS4VIZ_VAL_(v), __LINE__)
#define ds4vizSlInsertTail(l, v) ds4vizP_slist_insert_tail_(&(l), DS4VIZ_VAL_(v), __LINE__)
#define ds4vizSlInsertAfter(l, nid, v) ds4vizP_slist_insert_after_(&(l), (nid), DS4VIZ_VAL_(v), __LINE__)
#define ds4vizSlDelete(l, nid) ds4vizP_slist_delete_(&(l), (nid), __LINE__)
#define ds4vizSlDeleteHead(l) ds4vizP_slist_delete_head_(&(l), __LINE__)
#define ds4vizSlReverse(l) ds4vizP_slist_reverse_(&(l), __LINE__)

/* ---- 双向链表 ---- */
#define ds4vizDoubleLinkedList(l, ...)                                    \
    for (ds4vizP_dlist_ l = ds4vizP_dlist_open_(                          \
             DS4VIZ_OPT1_("dlist" __VA_OPT__(, ) __VA_ARGS__), __LINE__); \
         !l.done; ds4vizP_dlist_close_(&l))

#define ds4vizDlInsertHead(l, v) ds4vizP_dlist_insert_head_(&(l), DS4VIZ_VAL_(v), __LINE__)
#define ds4vizDlInsertTail(l, v) ds4vizP_dlist_insert_tail_(&(l), DS4VIZ_VAL_(v), __LINE__)
#define ds4vizDlInsertBefore(l, nid, v) ds4vizP_dlist_insert_before_(&(l), (nid), DS4VIZ_VAL_(v), __LINE__)
#define ds4vizDlInsertAfter(l, nid, v) ds4vizP_dlist_insert_after_(&(l), (nid), DS4VIZ_VAL_(v), __LINE__)
#define ds4vizDlDelete(l, nid) ds4vizP_dlist_delete_(&(l), (nid), __LINE__)
#define ds4vizDlDeleteHead(l) ds4vizP_dlist_delete_head_(&(l), __LINE__)
#define ds4vizDlDeleteTail(l) ds4vizP_dlist_delete_tail_(&(l), __LINE__)
#define ds4vizDlReverse(l) ds4vizP_dlist_reverse_(&(l), __LINE__)

/* ---- 二叉树 ---- */
#define ds4vizBinaryTree(t, ...)                                                                             \
    for (ds4vizP_bt_ t = ds4vizP_bt_open_("binary_tree",                                                     \
                                          DS4VIZ_OPT1_("binary_tree" __VA_OPT__(, ) __VA_ARGS__), __LINE__); \
         !t.done; ds4vizP_bt_close_(&t))

#define ds4vizBtInsertRoot(t, v) ds4vizP_bt_insert_root_(&(t), DS4VIZ_VAL_(v), __LINE__)
#define ds4vizBtInsertLeft(t, pid, v) ds4vizP_bt_insert_left_(&(t), (pid), DS4VIZ_VAL_(v), __LINE__)
#define ds4vizBtInsertRight(t, pid, v) ds4vizP_bt_insert_right_(&(t), (pid), DS4VIZ_VAL_(v), __LINE__)
#define ds4vizBtDelete(t, nid) ds4vizP_bt_delete_(&(t), (nid), __LINE__)
#define ds4vizBtUpdateValue(t, nid, v) ds4vizP_bt_update_value_(&(t), (nid), DS4VIZ_VAL_(v), __LINE__)

/* ---- 二叉搜索树 ---- */
#define ds4vizBinarySearchTree(b, ...)                                                               \
    for (ds4vizP_bt_ b = ds4vizP_bt_open_("bst",                                                     \
                                          DS4VIZ_OPT1_("bst" __VA_OPT__(, ) __VA_ARGS__), __LINE__); \
         !b.done; ds4vizP_bt_close_(&b))

#define ds4vizBstInsert(b, v) ds4vizP_bst_insert_(&(b), DS4VIZ_NUMVAL_(v), __LINE__)
#define ds4vizBstDelete(b, v) ds4vizP_bst_delete_(&(b), DS4VIZ_NUMVAL_(v), __LINE__)

/* ---- 堆 ---- */
#define ds4vizHeap(h, ...)                                               \
    for (ds4vizP_heap_ h = ds4vizP_heap_open_(                           \
             DS4VIZ_OPT1_("heap" __VA_OPT__(, ) __VA_ARGS__),            \
             (int)DS4VIZ_OPT2_(0 __VA_OPT__(, ) __VA_ARGS__), __LINE__); \
         !h.done; ds4vizP_heap_close_(&h))

#define ds4vizHeapInsert(h, v) ds4vizP_heap_insert_(&(h), DS4VIZ_NUMVAL_(v), __LINE__)
#define ds4vizHeapExtract(h) ds4vizP_heap_extract_(&(h), __LINE__)
#define ds4vizHeapClear(h) ds4vizP_heap_clear_(&(h), __LINE__)

/* ---- 无向图 ---- */
#define ds4vizGraphUndirected(g, ...)                                                             \
    for (ds4vizP_graph_ g = ds4vizP_graph_open_("graph_undirected",                               \
                                                DS4VIZ_OPT1_("graph" __VA_OPT__(, ) __VA_ARGS__), \
                                                false, false, __LINE__);                          \
         !g.done; ds4vizP_graph_close_(&g))

#define ds4vizGuAddNode(g, nid, lbl) ds4vizP_graph_add_node_(&(g), (nid), (lbl), __LINE__)
#define ds4vizGuAddEdge(g, f, t) ds4vizP_graph_add_edge_(&(g), (f), (t), 0.0, __LINE__)
#define ds4vizGuRemoveNode(g, nid) ds4vizP_graph_remove_node_(&(g), (nid), __LINE__)
#define ds4vizGuRemoveEdge(g, f, t) ds4vizP_graph_remove_edge_(&(g), (f), (t), __LINE__)
#define ds4vizGuUpdateNodeLabel(g, n, l) ds4vizP_graph_update_node_label_(&(g), (n), (l), __LINE__)

/* ---- 有向图 ---- */
#define ds4vizGraphDirected(g, ...)                                                               \
    for (ds4vizP_graph_ g = ds4vizP_graph_open_("graph_directed",                                 \
                                                DS4VIZ_OPT1_("graph" __VA_OPT__(, ) __VA_ARGS__), \
                                                true, false, __LINE__);                           \
         !g.done; ds4vizP_graph_close_(&g))

#define ds4vizGdAddNode(g, nid, lbl) ds4vizP_graph_add_node_(&(g), (nid), (lbl), __LINE__)
#define ds4vizGdAddEdge(g, f, t) ds4vizP_graph_add_edge_(&(g), (f), (t), 0.0, __LINE__)
#define ds4vizGdRemoveNode(g, nid) ds4vizP_graph_remove_node_(&(g), (nid), __LINE__)
#define ds4vizGdRemoveEdge(g, f, t) ds4vizP_graph_remove_edge_(&(g), (f), (t), __LINE__)
#define ds4vizGdUpdateNodeLabel(g, n, l) ds4vizP_graph_update_node_label_(&(g), (n), (l), __LINE__)

/* ---- 带权图 ---- */
#define ds4vizGraphWeighted(g, ...)                                                                   \
    for (ds4vizP_graph_ g = ds4vizP_graph_open_("graph_weighted",                                     \
                                                DS4VIZ_OPT1_("graph" __VA_OPT__(, ) __VA_ARGS__),     \
                                                (bool)DS4VIZ_OPT2_(false __VA_OPT__(, ) __VA_ARGS__), \
                                                true, __LINE__);                                      \
         !g.done; ds4vizP_graph_close_(&g))

#define ds4vizGwAddNode(g, nid, lbl) ds4vizP_graph_add_node_(&(g), (nid), (lbl), __LINE__)
#define ds4vizGwAddEdge(g, f, t, w) ds4vizP_graph_add_edge_(&(g), (f), (t), (double)(w), __LINE__)
#define ds4vizGwRemoveNode(g, nid) ds4vizP_graph_remove_node_(&(g), (nid), __LINE__)
#define ds4vizGwRemoveEdge(g, f, t) ds4vizP_graph_remove_edge_(&(g), (f), (t), __LINE__)
#define ds4vizGwUpdateWeight(g, f, t, w) ds4vizP_graph_update_weight_(&(g), (f), (t), (double)(w), __LINE__)
#define ds4vizGwUpdateNodeLabel(g, n, l) ds4vizP_graph_update_node_label_(&(g), (n), (l), __LINE__)

/* ================================================================
 * 短名映射
 * ================================================================ */
#ifdef DS4VIZ_SHORT_NAMES

#define dvConfigOptions ds4vizConfigOptions
#define dvHeapOrder ds4vizHeapOrder
#define dvHeapOrderMin ds4vizHeapOrderMin
#define dvHeapOrderMax ds4vizHeapOrderMax
#define dvConfig ds4vizConfig

#define dvStack ds4vizStack
#define dvStackPush ds4vizStackPush
#define dvStackPop ds4vizStackPop
#define dvStackClear ds4vizStackClear

#define dvQueue ds4vizQueue
#define dvQueueEnqueue ds4vizQueueEnqueue
#define dvQueueDequeue ds4vizQueueDequeue
#define dvQueueClear ds4vizQueueClear

#define dvSingleLinkedList ds4vizSingleLinkedList
#define dvSlInsertHead ds4vizSlInsertHead
#define dvSlInsertTail ds4vizSlInsertTail
#define dvSlInsertAfter ds4vizSlInsertAfter
#define dvSlDelete ds4vizSlDelete
#define dvSlDeleteHead ds4vizSlDeleteHead
#define dvSlReverse ds4vizSlReverse

#define dvDoubleLinkedList ds4vizDoubleLinkedList
#define dvDlInsertHead ds4vizDlInsertHead
#define dvDlInsertTail ds4vizDlInsertTail
#define dvDlInsertBefore ds4vizDlInsertBefore
#define dvDlInsertAfter ds4vizDlInsertAfter
#define dvDlDelete ds4vizDlDelete
#define dvDlDeleteHead ds4vizDlDeleteHead
#define dvDlDeleteTail ds4vizDlDeleteTail
#define dvDlReverse ds4vizDlReverse

#define dvBinaryTree ds4vizBinaryTree
#define dvBtInsertRoot ds4vizBtInsertRoot
#define dvBtInsertLeft ds4vizBtInsertLeft
#define dvBtInsertRight ds4vizBtInsertRight
#define dvBtDelete ds4vizBtDelete
#define dvBtUpdateValue ds4vizBtUpdateValue

#define dvBinarySearchTree ds4vizBinarySearchTree
#define dvBstInsert ds4vizBstInsert
#define dvBstDelete ds4vizBstDelete

#define dvHeap ds4vizHeap
#define dvHeapInsert ds4vizHeapInsert
#define dvHeapExtract ds4vizHeapExtract
#define dvHeapClear ds4vizHeapClear

#define dvGraphUndirected ds4vizGraphUndirected
#define dvGuAddNode ds4vizGuAddNode
#define dvGuAddEdge ds4vizGuAddEdge
#define dvGuRemoveNode ds4vizGuRemoveNode
#define dvGuRemoveEdge ds4vizGuRemoveEdge
#define dvGuUpdateNodeLabel ds4vizGuUpdateNodeLabel

#define dvGraphDirected ds4vizGraphDirected
#define dvGdAddNode ds4vizGdAddNode
#define dvGdAddEdge ds4vizGdAddEdge
#define dvGdRemoveNode ds4vizGdRemoveNode
#define dvGdRemoveEdge ds4vizGdRemoveEdge
#define dvGdUpdateNodeLabel ds4vizGdUpdateNodeLabel

#define dvGraphWeighted ds4vizGraphWeighted
#define dvGwAddNode ds4vizGwAddNode
#define dvGwAddEdge ds4vizGwAddEdge
#define dvGwRemoveNode ds4vizGwRemoveNode
#define dvGwRemoveEdge ds4vizGwRemoveEdge
#define dvGwUpdateWeight ds4vizGwUpdateWeight
#define dvGwUpdateNodeLabel ds4vizGwUpdateNodeLabel

#endif /* DS4VIZ_SHORT_NAMES */

#ifdef __cplusplus
}
#endif

#endif /* DS4VIZ_H */

/* ================================================================
 *
 *                      实现部分
 *
 * ================================================================ */
#ifdef DS4VIZ_IMPLEMENTATION

/**
 * @brief 库版本字符串
 */
#define DS4VIZ_VERSION_ "0.1.0"

/* ----------------------------------------------------------------
 * 全局配置
 * ---------------------------------------------------------------- */

/**
 * @brief 全局配置单例
 */
static struct
{
    const char *output_path;
    const char *title;
    const char *author;
    const char *comment;
} ds4vizP_g_cfg_ = {0};

void ds4vizConfig(ds4vizConfigOptions o)
{
    ds4vizP_g_cfg_.output_path = o.output_path;
    ds4vizP_g_cfg_.title = o.title;
    ds4vizP_g_cfg_.author = o.author;
    ds4vizP_g_cfg_.comment = o.comment;
}

/**
 * @brief 获取输出路径, 未配置则返回默认值
 *
 * @return 输出文件路径
 */
static const char *ds4vizP_outpath_(void)
{
    return ds4vizP_g_cfg_.output_path ? ds4vizP_g_cfg_.output_path : "trace.toml";
}

/* ----------------------------------------------------------------
 * 动态缓冲区辅助函数
 * ---------------------------------------------------------------- */

/**
 * @brief 确保缓冲区有足够空间
 *
 * @param p_b 缓冲区指针
 * @param need 额外需要的字节数
 */
static void ds4vizP_buf_grow_(ds4vizP_buf_ *p_b, int need)
{
    if (p_b->len + need + 1 > p_b->cap)
    {
        int nc = p_b->cap ? p_b->cap * 2 : 256;
        while (nc < p_b->len + need + 1)
        {
            nc *= 2;
        }
        p_b->data = (char *)realloc(p_b->data, (size_t)nc);
        p_b->cap = nc;
    }
}

/**
 * @brief 追加单个字符
 *
 * @param p_b 缓冲区指针
 * @param c 要追加的字符
 */
static void ds4vizP_buf_push_(ds4vizP_buf_ *p_b, char c)
{
    ds4vizP_buf_grow_(p_b, 1);
    p_b->data[p_b->len++] = c;
    p_b->data[p_b->len] = '\0';
}

/**
 * @brief 追加字符串
 *
 * @param p_b 缓冲区指针
 * @param s 要追加的字符串
 */
static void ds4vizP_buf_cat_(ds4vizP_buf_ *p_b, const char *s)
{
    int n = (int)strlen(s);
    ds4vizP_buf_grow_(p_b, n);
    memcpy(p_b->data + p_b->len, s, (size_t)n);
    p_b->len += n;
    p_b->data[p_b->len] = '\0';
}

/**
 * @brief 格式化追加
 *
 * @param p_b 缓冲区指针
 * @param fmt 格式字符串
 */
static void ds4vizP_buf_printf_(ds4vizP_buf_ *p_b, const char *fmt, ...)
{
    va_list ap;
    va_list ap2;
    va_start(ap, fmt);
    va_copy(ap2, ap);
    int n = vsnprintf(NULL, 0, fmt, ap);
    va_end(ap);
    ds4vizP_buf_grow_(p_b, n + 1);
    vsnprintf(p_b->data + p_b->len, (size_t)(n + 1), fmt, ap2);
    va_end(ap2);
    p_b->len += n;
}

/**
 * @brief 释放缓冲区
 *
 * @param p_b 缓冲区指针
 */
static void ds4vizP_buf_free_(ds4vizP_buf_ *p_b)
{
    free(p_b->data);
    p_b->data = NULL;
    p_b->len = p_b->cap = 0;
}

/* ----------------------------------------------------------------
 * TOML 序列化辅助函数
 * ---------------------------------------------------------------- */

/**
 * @brief 写入 TOML 转义字符串 (带双引号)
 *
 * @param p_b 缓冲区指针
 * @param s 原始字符串
 */
static void ds4vizP_toml_str_(ds4vizP_buf_ *p_b, const char *s)
{
    ds4vizP_buf_push_(p_b, '"');
    if (s)
    {
        for (; *s; s++)
        {
            switch (*s)
            {
            case '\\':
                ds4vizP_buf_cat_(p_b, "\\\\");
                break;
            case '"':
                ds4vizP_buf_cat_(p_b, "\\\"");
                break;
            case '\n':
                ds4vizP_buf_cat_(p_b, "\\n");
                break;
            case '\t':
                ds4vizP_buf_cat_(p_b, "\\t");
                break;
            case '\r':
                ds4vizP_buf_cat_(p_b, "\\r");
                break;
            default:
                ds4vizP_buf_push_(p_b, *s);
                break;
            }
        }
    }
    ds4vizP_buf_push_(p_b, '"');
}

/**
 * @brief 写入 TOML 值 (根据类型自动序列化)
 *
 * @param p_b 缓冲区指针
 * @param v 值
 */
static void ds4vizP_toml_val_(ds4vizP_buf_ *p_b, ds4vizP_val_ v)
{
    switch (v.type)
    {
    case DS4VIZ_VINT_:
        ds4vizP_buf_printf_(p_b, "%lld", v.i);
        break;
    case DS4VIZ_VDBL_:
    {
        char tmp[64];
        snprintf(tmp, sizeof(tmp), "%.17g", v.d);
        ds4vizP_buf_cat_(p_b, tmp);
        if (!strchr(tmp, '.') && !strchr(tmp, 'e') && !strchr(tmp, 'E') && !strchr(tmp, 'n') && !strchr(tmp, 'i'))
        {
            ds4vizP_buf_cat_(p_b, ".0");
        }
        break;
    }
    case DS4VIZ_VSTR_:
        ds4vizP_toml_str_(p_b, v.s ? v.s : "");
        break;
    case DS4VIZ_VBOOL_:
        ds4vizP_buf_cat_(p_b, v.b ? "true" : "false");
        break;
    }
}

/**
 * @brief 将值序列化为独立的 TOML 值字符串 (用于 ret 字段)
 *
 * @param buf 输出缓冲区
 * @param size 缓冲区大小
 * @param v 值
 */
static void ds4vizP_val_to_str_(char *buf, int size, ds4vizP_val_ v)
{
    ds4vizP_buf_ tmp = {0};
    ds4vizP_toml_val_(&tmp, v);
    if (tmp.data)
    {
        snprintf(buf, (size_t)size, "%s", tmp.data);
    }
    else
    {
        buf[0] = '\0';
    }
    ds4vizP_buf_free_(&tmp);
}

/* ---- 值复制与释放 ---- */

/**
 * @brief 复制值 (字符串类型会分配新内存)
 *
 * @param v 原始值
 * @return 复制后的值
 */
static ds4vizP_val_ ds4vizP_val_dup_(ds4vizP_val_ v)
{
    if (v.type == DS4VIZ_VSTR_ && v.s)
    {
        ds4vizP_val_ r = v;
        size_t n = strlen(v.s);
        char *c = (char *)malloc(n + 1);
        memcpy(c, v.s, n + 1);
        r.s = c;
        return r;
    }
    return v;
}

/**
 * @brief 释放值 (仅释放字符串类型的内存)
 *
 * @param p_v 值指针
 */
static void ds4vizP_val_free_(ds4vizP_val_ *p_v)
{
    if (p_v->type == DS4VIZ_VSTR_ && p_v->s)
    {
        free((void *)p_v->s);
        p_v->s = NULL;
    }
}

/**
 * @brief 将值转换为 double (用于 BST/堆比较)
 *
 * @param v 值
 * @return double 数值
 */
static double ds4vizP_val_num_(ds4vizP_val_ v)
{
    return v.type == DS4VIZ_VDBL_ ? v.d : (double)v.i;
}

/* ---- 数组容量增长宏 ---- */

/**
 * @brief 动态数组扩容
 *
 * @param arr 数组指针
 * @param cnt 当前计数
 * @param cap 当前容量
 * @param type 元素类型
 */
#define DS4VIZ_GROW_(arr, cnt, cap, type)                                 \
    do                                                                    \
    {                                                                     \
        if ((cnt) >= (cap))                                               \
        {                                                                 \
            (cap) = (cap) ? (cap) * 2 : 8;                                \
            (arr) = (type *)realloc((arr), (size_t)(cap) * sizeof(type)); \
        }                                                                 \
    } while (0)

/* ---- 错误记录宏 ---- */

/**
 * @brief 记录错误信息到结构体
 */
#define DS4VIZ_ERR_(p_s, ln, ...)                                    \
    do                                                               \
    {                                                                \
        (p_s)->err = true;                                           \
        snprintf((p_s)->errmsg, sizeof((p_s)->errmsg), __VA_ARGS__); \
        (p_s)->err_step = (p_s)->step_id;                            \
        (p_s)->err_last_state = (p_s)->state_id - 1;                 \
        (p_s)->err_line = ln;                                        \
    } while (0)

/* ----------------------------------------------------------------
 * 状态/步骤写入辅助函数
 * ---------------------------------------------------------------- */

/**
 * @brief 开始写入一个状态段 (写入 [[states]] 头和 [states.data] 头)
 *
 * @param p_b 状态缓冲区
 * @param sid 状态 ID
 */
static void ds4vizP_begin_state_(ds4vizP_buf_ *p_b, int sid)
{
    ds4vizP_buf_printf_(p_b, "\n[[states]]\nid = %d\n\n[states.data]\n", sid);
}

/**
 * @brief 写入简单值数组 (如 items = [10, 20])
 *
 * @param p_b 缓冲区
 * @param key 键名
 * @param items 值数组
 * @param count 元素个数
 */
static void ds4vizP_write_simple_array_(ds4vizP_buf_ *p_b, const char *key,
                                        ds4vizP_val_ *items, int count)
{
    ds4vizP_buf_cat_(p_b, key);
    ds4vizP_buf_cat_(p_b, " = [");
    for (int i = 0; i < count; i++)
    {
        if (i)
        {
            ds4vizP_buf_cat_(p_b, ", ");
        }
        ds4vizP_toml_val_(p_b, items[i]);
    }
    ds4vizP_buf_cat_(p_b, "]\n");
}

/**
 * @brief 写入完整步骤段到缓冲区
 *
 * @param p_b 步骤缓冲区
 * @param sid 步骤 ID
 * @param op 操作名称
 * @param before 操作前状态 ID
 * @param after 操作后状态 ID (-1 表示无 after, 即错误情况)
 * @param args 参数内容字符串 (已格式化的 key = val 行)
 * @param ret_str 返回值字符串, NULL 表示无返回值
 * @param code_line 源码行号
 */
static void ds4vizP_write_step_(ds4vizP_buf_ *p_b, int sid, const char *op,
                                int before, int after, const char *args,
                                const char *ret_str, int code_line)
{
    ds4vizP_buf_printf_(p_b, "\n[[steps]]\nid = %d\n", sid);
    ds4vizP_buf_cat_(p_b, "op = ");
    ds4vizP_toml_str_(p_b, op);
    ds4vizP_buf_printf_(p_b, "\nbefore = %d\n", before);
    if (after >= 0)
    {
        ds4vizP_buf_printf_(p_b, "after = %d\n", after);
    }
    if (ret_str)
    {
        ds4vizP_buf_printf_(p_b, "ret = %s\n", ret_str);
    }
    ds4vizP_buf_cat_(p_b, "\n[steps.args]\n");
    if (args && args[0])
    {
        ds4vizP_buf_cat_(p_b, args);
    }
    ds4vizP_buf_printf_(p_b, "\n[steps.code]\nline = %d\n", code_line);
}

/* ----------------------------------------------------------------
 * 时间戳辅助函数
 * ---------------------------------------------------------------- */

/**
 * @brief 获取当前 UTC 时间戳 (RFC3339 格式)
 *
 * @param buf 输出缓冲区
 * @param size 缓冲区大小
 */
static void ds4vizP_timestamp_(char *buf, int size)
{
    time_t now = time(NULL);
    struct tm *p_utc = gmtime(&now);
    if (p_utc)
    {
        strftime(buf, (size_t)size, "%Y-%m-%dT%H:%M:%SZ", p_utc);
    }
    else
    {
        snprintf(buf, (size_t)size, "1970-01-01T00:00:00Z");
    }
}

/**
 * @brief 获取 C 语言版本字符串 (从 __STDC_VERSION__ 推导)
 *
 * @param buf 输出缓冲区
 * @param size 缓冲区大小
 */
static void ds4vizP_c_version_(char *buf, int size)
{
#if defined(__STDC_VERSION__)
    snprintf(buf, (size_t)size, "%ld", (long)__STDC_VERSION__);
#else
    snprintf(buf, (size_t)size, "unknown");
#endif
}

/* ----------------------------------------------------------------
 * 最终 TOML 组装与写入
 * ---------------------------------------------------------------- */

/**
 * @brief 将完整 trace 写入文件
 *
 * @param kind 数据结构类型
 * @param label 数据结构标签
 * @param p_states 状态缓冲区
 * @param p_steps 步骤缓冲区
 * @param err 是否有错误
 * @param errmsg 错误消息
 * @param err_step 错误发生时的步骤 ID
 * @param err_last_state 错误发生时的最后状态 ID
 * @param err_line 错误行号
 * @param final_state 最终状态 ID
 * @param scope_line 作用域行号 (用于 commit)
 */
static void ds4vizP_flush_(const char *kind, const char *label,
                           ds4vizP_buf_ *p_states, ds4vizP_buf_ *p_steps,
                           bool err, const char *errmsg,
                           int err_step, int err_last_state, int err_line,
                           int final_state, int scope_line)
{
    char ts[64];
    char cv[32];
    ds4vizP_buf_ out = {0};

    ds4vizP_timestamp_(ts, sizeof(ts));
    ds4vizP_c_version_(cv, sizeof(cv));

    /* [meta] */
    ds4vizP_buf_cat_(&out, "[meta]\ncreated_at = ");
    ds4vizP_toml_str_(&out, ts);
    ds4vizP_buf_cat_(&out, "\nlang = \"c\"\nlang_version = ");
    ds4vizP_toml_str_(&out, cv);
    ds4vizP_buf_push_(&out, '\n');

    /* [package] */
    ds4vizP_buf_cat_(&out, "\n[package]\nname = \"ds4viz\"\nlang = \"c\"\nversion = \"");
    ds4vizP_buf_cat_(&out, DS4VIZ_VERSION_);
    ds4vizP_buf_cat_(&out, "\"\n");

    /* [remarks] (可选) */
    if (ds4vizP_g_cfg_.title || ds4vizP_g_cfg_.author || ds4vizP_g_cfg_.comment)
    {
        ds4vizP_buf_cat_(&out, "\n[remarks]\n");
        if (ds4vizP_g_cfg_.title)
        {
            ds4vizP_buf_cat_(&out, "title = ");
            ds4vizP_toml_str_(&out, ds4vizP_g_cfg_.title);
            ds4vizP_buf_push_(&out, '\n');
        }
        if (ds4vizP_g_cfg_.author)
        {
            ds4vizP_buf_cat_(&out, "author = ");
            ds4vizP_toml_str_(&out, ds4vizP_g_cfg_.author);
            ds4vizP_buf_push_(&out, '\n');
        }
        if (ds4vizP_g_cfg_.comment)
        {
            ds4vizP_buf_cat_(&out, "comment = ");
            ds4vizP_toml_str_(&out, ds4vizP_g_cfg_.comment);
            ds4vizP_buf_push_(&out, '\n');
        }
    }

    /* [object] */
    ds4vizP_buf_cat_(&out, "\n[object]\nkind = ");
    ds4vizP_toml_str_(&out, kind);
    ds4vizP_buf_push_(&out, '\n');
    if (label && label[0])
    {
        ds4vizP_buf_cat_(&out, "label = ");
        ds4vizP_toml_str_(&out, label);
        ds4vizP_buf_push_(&out, '\n');
    }

    /* [[states]] */
    if (p_states->data)
    {
        ds4vizP_buf_cat_(&out, p_states->data);
    }

    /* [[steps]] */
    if (p_steps->data)
    {
        ds4vizP_buf_cat_(&out, p_steps->data);
    }

    /* [result] 或 [error] */
    if (err)
    {
        ds4vizP_buf_cat_(&out, "\n[error]\ntype = \"runtime\"\nmessage = ");
        ds4vizP_toml_str_(&out, errmsg);
        ds4vizP_buf_push_(&out, '\n');
        if (err_line > 0)
        {
            ds4vizP_buf_printf_(&out, "line = %d\n", err_line);
        }
        ds4vizP_buf_printf_(&out, "step = %d\n", err_step);
        if (err_last_state >= 0)
        {
            ds4vizP_buf_printf_(&out, "last_state = %d\n", err_last_state);
        }
    }
    else
    {
        ds4vizP_buf_printf_(&out, "\n[result]\nfinal_state = %d\n", final_state);
        ds4vizP_buf_printf_(&out, "\n[result.commit]\nop = \"commit\"\nline = %d\n",
                            scope_line);
    }

    /* 写入文件 */
    {
        FILE *p_f = fopen(ds4vizP_outpath_(), "w");
        if (p_f)
        {
            if (out.data)
            {
                fwrite(out.data, 1, (size_t)out.len, p_f);
            }
            fclose(p_f);
        }
    }
    ds4vizP_buf_free_(&out);
}

/* ---- 参数构建辅助 ---- */

/**
 * @brief 向参数缓冲区追加值参数行
 *
 * @param p_b 缓冲区
 * @param key 键名
 * @param v 值
 */
static void ds4vizP_arg_val_(ds4vizP_buf_ *p_b, const char *key, ds4vizP_val_ v)
{
    ds4vizP_buf_cat_(p_b, key);
    ds4vizP_buf_cat_(p_b, " = ");
    ds4vizP_toml_val_(p_b, v);
    ds4vizP_buf_push_(p_b, '\n');
}

/**
 * @brief 向参数缓冲区追加整数参数行
 *
 * @param p_b 缓冲区
 * @param key 键名
 * @param v 整数值
 */
static void ds4vizP_arg_int_(ds4vizP_buf_ *p_b, const char *key, int v)
{
    ds4vizP_buf_printf_(p_b, "%s = %d\n", key, v);
}

/**
 * @brief 向参数缓冲区追加字符串参数行
 *
 * @param p_b 缓冲区
 * @param key 键名
 * @param v 字符串值
 */
static void ds4vizP_arg_str_(ds4vizP_buf_ *p_b, const char *key, const char *v)
{
    ds4vizP_buf_cat_(p_b, key);
    ds4vizP_buf_cat_(p_b, " = ");
    ds4vizP_toml_str_(p_b, v);
    ds4vizP_buf_push_(p_b, '\n');
}

/**
 * @brief 向参数缓冲区追加浮点参数行
 *
 * @param p_b 缓冲区
 * @param key 键名
 * @param v 浮点值
 */
static void ds4vizP_arg_dbl_(ds4vizP_buf_ *p_b, const char *key, double v)
{
    ds4vizP_arg_val_(p_b, key, (ds4vizP_val_){.type = DS4VIZ_VDBL_, .d = v});
}

/* ================================================================
 *  栈
 * ================================================================ */

/**
 * @brief 写入栈状态快照
 *
 * @param p_s 栈指针
 */
static void ds4vizP_stack_ws_(ds4vizP_stack_ *p_s)
{
    ds4vizP_buf_ *p_b = &p_s->states_buf;
    ds4vizP_begin_state_(p_b, p_s->state_id);
    ds4vizP_write_simple_array_(p_b, "items", p_s->items, p_s->count);
    ds4vizP_buf_printf_(p_b, "top = %d\n", p_s->count > 0 ? p_s->count - 1 : -1);
}

ds4vizP_stack_ ds4vizP_stack_open_(const char *label, int line)
{
    ds4vizP_stack_ s = {0};
    s.label = label;
    s.kind = "stack";
    s.scope_line = line;
    ds4vizP_stack_ws_(&s);
    s.state_id++;
    return s;
}

void ds4vizP_stack_close_(ds4vizP_stack_ *p_s)
{
    ds4vizP_flush_(p_s->kind, p_s->label, &p_s->states_buf, &p_s->steps_buf,
                   p_s->err, p_s->errmsg, p_s->err_step, p_s->err_last_state,
                   p_s->err_line, p_s->state_id - 1, p_s->scope_line);
    for (int i = 0; i < p_s->count; i++)
    {
        ds4vizP_val_free_(&p_s->items[i]);
    }
    free(p_s->items);
    ds4vizP_buf_free_(&p_s->states_buf);
    ds4vizP_buf_free_(&p_s->steps_buf);
    p_s->done = true;
}

void ds4vizP_stack_push_(ds4vizP_stack_ *p_s, ds4vizP_val_ v, int line)
{
    if (p_s->err)
    {
        return;
    }
    v = ds4vizP_val_dup_(v);
    DS4VIZ_GROW_(p_s->items, p_s->count, p_s->cap, ds4vizP_val_);
    p_s->items[p_s->count++] = v;
    int before = p_s->state_id - 1;
    ds4vizP_stack_ws_(p_s);
    int after = p_s->state_id++;
    ds4vizP_buf_ args = {0};
    ds4vizP_arg_val_(&args, "value", v);
    ds4vizP_write_step_(&p_s->steps_buf, p_s->step_id++, "push",
                        before, after, args.data, NULL, line);
    ds4vizP_buf_free_(&args);
}

void ds4vizP_stack_pop_(ds4vizP_stack_ *p_s, int line)
{
    if (p_s->err)
    {
        return;
    }
    if (p_s->count == 0)
    {
        DS4VIZ_ERR_(p_s, line, "Cannot pop from empty stack");
        return;
    }
    ds4vizP_val_ popped = p_s->items[--p_s->count];
    int before = p_s->state_id - 1;
    ds4vizP_stack_ws_(p_s);
    int after = p_s->state_id++;
    char ret_str[256];
    ds4vizP_val_to_str_(ret_str, sizeof(ret_str), popped);
    ds4vizP_write_step_(&p_s->steps_buf, p_s->step_id++, "pop",
                        before, after, NULL, ret_str, line);
    ds4vizP_val_free_(&popped);
}

void ds4vizP_stack_clear_(ds4vizP_stack_ *p_s, int line)
{
    if (p_s->err)
    {
        return;
    }
    for (int i = 0; i < p_s->count; i++)
    {
        ds4vizP_val_free_(&p_s->items[i]);
    }
    p_s->count = 0;
    int before = p_s->state_id - 1;
    ds4vizP_stack_ws_(p_s);
    int after = p_s->state_id++;
    ds4vizP_write_step_(&p_s->steps_buf, p_s->step_id++, "clear",
                        before, after, NULL, NULL, line);
}

/* ================================================================
 *  队列
 * ================================================================ */

/**
 * @brief 写入队列状态快照
 */
static void ds4vizP_queue_ws_(ds4vizP_queue_ *p_q)
{
    ds4vizP_buf_ *p_b = &p_q->states_buf;
    ds4vizP_begin_state_(p_b, p_q->state_id);
    ds4vizP_write_simple_array_(p_b, "items", p_q->items, p_q->count);
    if (p_q->count == 0)
    {
        ds4vizP_buf_cat_(p_b, "front = -1\nrear = -1\n");
    }
    else
    {
        ds4vizP_buf_printf_(p_b, "front = 0\nrear = %d\n", p_q->count - 1);
    }
}

ds4vizP_queue_ ds4vizP_queue_open_(const char *label, int line)
{
    ds4vizP_queue_ q = {0};
    q.label = label;
    q.kind = "queue";
    q.scope_line = line;
    ds4vizP_queue_ws_(&q);
    q.state_id++;
    return q;
}

void ds4vizP_queue_close_(ds4vizP_queue_ *p_q)
{
    ds4vizP_flush_(p_q->kind, p_q->label, &p_q->states_buf, &p_q->steps_buf,
                   p_q->err, p_q->errmsg, p_q->err_step, p_q->err_last_state,
                   p_q->err_line, p_q->state_id - 1, p_q->scope_line);
    for (int i = 0; i < p_q->count; i++)
    {
        ds4vizP_val_free_(&p_q->items[i]);
    }
    free(p_q->items);
    ds4vizP_buf_free_(&p_q->states_buf);
    ds4vizP_buf_free_(&p_q->steps_buf);
    p_q->done = true;
}

void ds4vizP_queue_enqueue_(ds4vizP_queue_ *p_q, ds4vizP_val_ v, int line)
{
    if (p_q->err)
    {
        return;
    }
    v = ds4vizP_val_dup_(v);
    DS4VIZ_GROW_(p_q->items, p_q->count, p_q->cap, ds4vizP_val_);
    p_q->items[p_q->count++] = v;
    int before = p_q->state_id - 1;
    ds4vizP_queue_ws_(p_q);
    int after = p_q->state_id++;
    ds4vizP_buf_ args = {0};
    ds4vizP_arg_val_(&args, "value", v);
    ds4vizP_write_step_(&p_q->steps_buf, p_q->step_id++, "enqueue",
                        before, after, args.data, NULL, line);
    ds4vizP_buf_free_(&args);
}

void ds4vizP_queue_dequeue_(ds4vizP_queue_ *p_q, int line)
{
    if (p_q->err)
    {
        return;
    }
    if (p_q->count == 0)
    {
        DS4VIZ_ERR_(p_q, line, "Cannot dequeue from empty queue");
        return;
    }
    ds4vizP_val_ dequeued = p_q->items[0];
    memmove(p_q->items, p_q->items + 1, (size_t)(p_q->count - 1) * sizeof(ds4vizP_val_));
    p_q->count--;
    int before = p_q->state_id - 1;
    ds4vizP_queue_ws_(p_q);
    int after = p_q->state_id++;
    char ret_str[256];
    ds4vizP_val_to_str_(ret_str, sizeof(ret_str), dequeued);
    ds4vizP_write_step_(&p_q->steps_buf, p_q->step_id++, "dequeue",
                        before, after, NULL, ret_str, line);
    ds4vizP_val_free_(&dequeued);
}

void ds4vizP_queue_clear_(ds4vizP_queue_ *p_q, int line)
{
    if (p_q->err)
    {
        return;
    }
    for (int i = 0; i < p_q->count; i++)
    {
        ds4vizP_val_free_(&p_q->items[i]);
    }
    p_q->count = 0;
    int before = p_q->state_id - 1;
    ds4vizP_queue_ws_(p_q);
    int after = p_q->state_id++;
    ds4vizP_write_step_(&p_q->steps_buf, p_q->step_id++, "clear",
                        before, after, NULL, NULL, line);
}

/* ================================================================
 *  单链表
 * ================================================================ */

/**
 * @brief 写入单链表状态快照
 */
static void ds4vizP_slist_ws_(ds4vizP_slist_ *p_l)
{
    ds4vizP_buf_ *p_b = &p_l->states_buf;
    ds4vizP_begin_state_(p_b, p_l->state_id);
    ds4vizP_buf_printf_(p_b, "head = %d\n", p_l->head);

    /* 统计存活节点数 */
    int alive_count = 0;
    for (int i = 0; i < p_l->ncnt; i++)
    {
        if (p_l->nodes[i].alive)
        {
            alive_count++;
        }
    }

    if (alive_count == 0)
    {
        ds4vizP_buf_cat_(p_b, "nodes = []\n");
    }
    else
    {
        ds4vizP_buf_cat_(p_b, "nodes = [\n");
        bool first = true;
        for (int i = 0; i < p_l->ncnt; i++)
        {
            if (!p_l->nodes[i].alive)
            {
                continue;
            }
            if (!first)
            {
                ds4vizP_buf_cat_(p_b, ",\n");
            }
            first = false;
            ds4vizP_buf_printf_(p_b, "  { id = %d, value = ", p_l->nodes[i].id);
            ds4vizP_toml_val_(p_b, p_l->nodes[i].value);
            ds4vizP_buf_printf_(p_b, ", next = %d }", p_l->nodes[i].next);
        }
        ds4vizP_buf_cat_(p_b, "\n]\n");
    }
}

static ds4vizP_slnode_ *ds4vizP_slist_find_(ds4vizP_slist_ *p_l, int nid)
{
    if (nid >= 0 && nid < p_l->ncnt && p_l->nodes[nid].alive)
    {
        return &p_l->nodes[nid];
    }
    return NULL;
}

static int ds4vizP_slist_alloc_(ds4vizP_slist_ *p_l, ds4vizP_val_ v)
{
    int id = p_l->nid++;
    if (id >= p_l->ncap)
    {
        int nc = p_l->ncap ? p_l->ncap * 2 : 8;
        while (nc <= id)
        {
            nc *= 2;
        }
        p_l->nodes = (ds4vizP_slnode_ *)realloc(p_l->nodes, (size_t)nc * sizeof(ds4vizP_slnode_));
        p_l->ncap = nc;
    }
    p_l->nodes[id] = (ds4vizP_slnode_){.id = id, .value = v, .next = -1, .alive = true};
    p_l->ncnt = p_l->nid;
    return id;
}

ds4vizP_slist_ ds4vizP_slist_open_(const char *label, int line)
{
    ds4vizP_slist_ l = {0};
    l.label = label;
    l.kind = "slist";
    l.scope_line = line;
    l.head = -1;
    ds4vizP_slist_ws_(&l);
    l.state_id++;
    return l;
}

void ds4vizP_slist_close_(ds4vizP_slist_ *p_l)
{
    ds4vizP_flush_(p_l->kind, p_l->label, &p_l->states_buf, &p_l->steps_buf,
                   p_l->err, p_l->errmsg, p_l->err_step, p_l->err_last_state,
                   p_l->err_line, p_l->state_id - 1, p_l->scope_line);
    for (int i = 0; i < p_l->ncnt; i++)
    {
        if (p_l->nodes[i].alive)
        {
            ds4vizP_val_free_(&p_l->nodes[i].value);
        }
    }
    free(p_l->nodes);
    ds4vizP_buf_free_(&p_l->states_buf);
    ds4vizP_buf_free_(&p_l->steps_buf);
    p_l->done = true;
}

int ds4vizP_slist_insert_head_(ds4vizP_slist_ *p_l, ds4vizP_val_ v, int line)
{
    if (p_l->err)
    {
        return -1;
    }
    v = ds4vizP_val_dup_(v);
    int id = ds4vizP_slist_alloc_(p_l, v);
    p_l->nodes[id].next = p_l->head;
    p_l->head = id;
    int before = p_l->state_id - 1;
    ds4vizP_slist_ws_(p_l);
    int after = p_l->state_id++;
    ds4vizP_buf_ args = {0};
    ds4vizP_arg_val_(&args, "value", v);
    char ret_str[32];
    snprintf(ret_str, sizeof(ret_str), "%d", id);
    ds4vizP_write_step_(&p_l->steps_buf, p_l->step_id++, "insert_head",
                        before, after, args.data, ret_str, line);
    ds4vizP_buf_free_(&args);
    return id;
}

int ds4vizP_slist_insert_tail_(ds4vizP_slist_ *p_l, ds4vizP_val_ v, int line)
{
    if (p_l->err)
    {
        return -1;
    }
    v = ds4vizP_val_dup_(v);
    int id = ds4vizP_slist_alloc_(p_l, v);
    if (p_l->head < 0)
    {
        p_l->head = id;
    }
    else
    {
        int cur = p_l->head;
        while (p_l->nodes[cur].next >= 0)
        {
            cur = p_l->nodes[cur].next;
        }
        p_l->nodes[cur].next = id;
    }
    int before = p_l->state_id - 1;
    ds4vizP_slist_ws_(p_l);
    int after = p_l->state_id++;
    ds4vizP_buf_ args = {0};
    ds4vizP_arg_val_(&args, "value", v);
    char ret_str[32];
    snprintf(ret_str, sizeof(ret_str), "%d", id);
    ds4vizP_write_step_(&p_l->steps_buf, p_l->step_id++, "insert_tail",
                        before, after, args.data, ret_str, line);
    ds4vizP_buf_free_(&args);
    return id;
}

int ds4vizP_slist_insert_after_(ds4vizP_slist_ *p_l, int nid, ds4vizP_val_ v, int line)
{
    if (p_l->err)
    {
        return -1;
    }
    ds4vizP_slnode_ *p_n = ds4vizP_slist_find_(p_l, nid);
    if (!p_n)
    {
        DS4VIZ_ERR_(p_l, line, "Node not found: %d", nid);
        return -1;
    }
    v = ds4vizP_val_dup_(v);
    int id = ds4vizP_slist_alloc_(p_l, v);
    p_l->nodes[id].next = p_n->next;
    p_n->next = id;
    int before = p_l->state_id - 1;
    ds4vizP_slist_ws_(p_l);
    int after = p_l->state_id++;
    ds4vizP_buf_ args = {0};
    ds4vizP_arg_int_(&args, "node_id", nid);
    ds4vizP_arg_val_(&args, "value", v);
    char ret_str[32];
    snprintf(ret_str, sizeof(ret_str), "%d", id);
    ds4vizP_write_step_(&p_l->steps_buf, p_l->step_id++, "insert_after",
                        before, after, args.data, ret_str, line);
    ds4vizP_buf_free_(&args);
    return id;
}

void ds4vizP_slist_delete_(ds4vizP_slist_ *p_l, int nid, int line)
{
    if (p_l->err)
    {
        return;
    }
    ds4vizP_slnode_ *p_n = ds4vizP_slist_find_(p_l, nid);
    if (!p_n)
    {
        DS4VIZ_ERR_(p_l, line, "Node not found: %d", nid);
        return;
    }
    ds4vizP_val_ deleted_val = p_n->value;
    if (p_l->head == nid)
    {
        p_l->head = p_n->next;
    }
    else
    {
        int cur = p_l->head;
        while (cur >= 0 && p_l->nodes[cur].next != nid)
        {
            cur = p_l->nodes[cur].next;
        }
        if (cur >= 0)
        {
            p_l->nodes[cur].next = p_n->next;
        }
    }
    p_n->alive = false;
    int before = p_l->state_id - 1;
    ds4vizP_slist_ws_(p_l);
    int after = p_l->state_id++;
    ds4vizP_buf_ args = {0};
    ds4vizP_arg_int_(&args, "node_id", nid);
    char ret_str[256];
    ds4vizP_val_to_str_(ret_str, sizeof(ret_str), deleted_val);
    ds4vizP_write_step_(&p_l->steps_buf, p_l->step_id++, "delete",
                        before, after, args.data, ret_str, line);
    ds4vizP_buf_free_(&args);
    ds4vizP_val_free_(&deleted_val);
}

void ds4vizP_slist_delete_head_(ds4vizP_slist_ *p_l, int line)
{
    if (p_l->err)
    {
        return;
    }
    if (p_l->head < 0)
    {
        DS4VIZ_ERR_(p_l, line, "Cannot delete from empty list");
        return;
    }
    int old_head = p_l->head;
    ds4vizP_val_ deleted_val = p_l->nodes[old_head].value;
    p_l->head = p_l->nodes[old_head].next;
    p_l->nodes[old_head].alive = false;
    int before = p_l->state_id - 1;
    ds4vizP_slist_ws_(p_l);
    int after = p_l->state_id++;
    char ret_str[256];
    ds4vizP_val_to_str_(ret_str, sizeof(ret_str), deleted_val);
    ds4vizP_write_step_(&p_l->steps_buf, p_l->step_id++, "delete_head",
                        before, after, NULL, ret_str, line);
    ds4vizP_val_free_(&deleted_val);
}

void ds4vizP_slist_reverse_(ds4vizP_slist_ *p_l, int line)
{
    if (p_l->err)
    {
        return;
    }
    int prev = -1;
    int cur = p_l->head;
    while (cur >= 0)
    {
        int nxt = p_l->nodes[cur].next;
        p_l->nodes[cur].next = prev;
        prev = cur;
        cur = nxt;
    }
    p_l->head = prev;
    int before = p_l->state_id - 1;
    ds4vizP_slist_ws_(p_l);
    int after = p_l->state_id++;
    ds4vizP_write_step_(&p_l->steps_buf, p_l->step_id++, "reverse",
                        before, after, NULL, NULL, line);
}

/* ================================================================
 *  双向链表
 * ================================================================ */

/**
 * @brief 写入双向链表状态快照
 */
static void ds4vizP_dlist_ws_(ds4vizP_dlist_ *p_l)
{
    ds4vizP_buf_ *p_b = &p_l->states_buf;
    ds4vizP_begin_state_(p_b, p_l->state_id);
    ds4vizP_buf_printf_(p_b, "head = %d\ntail = %d\n", p_l->head, p_l->tail);

    int alive_count = 0;
    for (int i = 0; i < p_l->ncnt; i++)
    {
        if (p_l->nodes[i].alive)
        {
            alive_count++;
        }
    }

    if (alive_count == 0)
    {
        ds4vizP_buf_cat_(p_b, "nodes = []\n");
    }
    else
    {
        ds4vizP_buf_cat_(p_b, "nodes = [\n");
        bool first = true;
        for (int i = 0; i < p_l->ncnt; i++)
        {
            if (!p_l->nodes[i].alive)
            {
                continue;
            }
            if (!first)
            {
                ds4vizP_buf_cat_(p_b, ",\n");
            }
            first = false;
            ds4vizP_buf_printf_(p_b, "  { id = %d, value = ", p_l->nodes[i].id);
            ds4vizP_toml_val_(p_b, p_l->nodes[i].value);
            ds4vizP_buf_printf_(p_b, ", prev = %d, next = %d }",
                                p_l->nodes[i].prev, p_l->nodes[i].next);
        }
        ds4vizP_buf_cat_(p_b, "\n]\n");
    }
}

static ds4vizP_dlnode_ *ds4vizP_dlist_find_(ds4vizP_dlist_ *p_l, int nid)
{
    if (nid >= 0 && nid < p_l->ncnt && p_l->nodes[nid].alive)
    {
        return &p_l->nodes[nid];
    }
    return NULL;
}

static int ds4vizP_dlist_alloc_(ds4vizP_dlist_ *p_l, ds4vizP_val_ v)
{
    int id = p_l->nid++;
    if (id >= p_l->ncap)
    {
        int nc = p_l->ncap ? p_l->ncap * 2 : 8;
        while (nc <= id)
        {
            nc *= 2;
        }
        p_l->nodes = (ds4vizP_dlnode_ *)realloc(p_l->nodes, (size_t)nc * sizeof(ds4vizP_dlnode_));
        p_l->ncap = nc;
    }
    p_l->nodes[id] = (ds4vizP_dlnode_){.id = id, .value = v, .prev = -1, .next = -1, .alive = true};
    p_l->ncnt = p_l->nid;
    return id;
}

ds4vizP_dlist_ ds4vizP_dlist_open_(const char *label, int line)
{
    ds4vizP_dlist_ l = {0};
    l.label = label;
    l.kind = "dlist";
    l.scope_line = line;
    l.head = l.tail = -1;
    ds4vizP_dlist_ws_(&l);
    l.state_id++;
    return l;
}

void ds4vizP_dlist_close_(ds4vizP_dlist_ *p_l)
{
    ds4vizP_flush_(p_l->kind, p_l->label, &p_l->states_buf, &p_l->steps_buf,
                   p_l->err, p_l->errmsg, p_l->err_step, p_l->err_last_state,
                   p_l->err_line, p_l->state_id - 1, p_l->scope_line);
    for (int i = 0; i < p_l->ncnt; i++)
    {
        if (p_l->nodes[i].alive)
        {
            ds4vizP_val_free_(&p_l->nodes[i].value);
        }
    }
    free(p_l->nodes);
    ds4vizP_buf_free_(&p_l->states_buf);
    ds4vizP_buf_free_(&p_l->steps_buf);
    p_l->done = true;
}

int ds4vizP_dlist_insert_head_(ds4vizP_dlist_ *p_l, ds4vizP_val_ v, int line)
{
    if (p_l->err)
    {
        return -1;
    }
    v = ds4vizP_val_dup_(v);
    int id = ds4vizP_dlist_alloc_(p_l, v);
    p_l->nodes[id].next = p_l->head;
    if (p_l->head >= 0)
    {
        p_l->nodes[p_l->head].prev = id;
    }
    p_l->head = id;
    if (p_l->tail < 0)
    {
        p_l->tail = id;
    }
    int before = p_l->state_id - 1;
    ds4vizP_dlist_ws_(p_l);
    int after = p_l->state_id++;
    ds4vizP_buf_ args = {0};
    ds4vizP_arg_val_(&args, "value", v);
    char ret_str[32];
    snprintf(ret_str, sizeof(ret_str), "%d", id);
    ds4vizP_write_step_(&p_l->steps_buf, p_l->step_id++, "insert_head",
                        before, after, args.data, ret_str, line);
    ds4vizP_buf_free_(&args);
    return id;
}

int ds4vizP_dlist_insert_tail_(ds4vizP_dlist_ *p_l, ds4vizP_val_ v, int line)
{
    if (p_l->err)
    {
        return -1;
    }
    v = ds4vizP_val_dup_(v);
    int id = ds4vizP_dlist_alloc_(p_l, v);
    p_l->nodes[id].prev = p_l->tail;
    if (p_l->tail >= 0)
    {
        p_l->nodes[p_l->tail].next = id;
    }
    p_l->tail = id;
    if (p_l->head < 0)
    {
        p_l->head = id;
    }
    int before = p_l->state_id - 1;
    ds4vizP_dlist_ws_(p_l);
    int after = p_l->state_id++;
    ds4vizP_buf_ args = {0};
    ds4vizP_arg_val_(&args, "value", v);
    char ret_str[32];
    snprintf(ret_str, sizeof(ret_str), "%d", id);
    ds4vizP_write_step_(&p_l->steps_buf, p_l->step_id++, "insert_tail",
                        before, after, args.data, ret_str, line);
    ds4vizP_buf_free_(&args);
    return id;
}

int ds4vizP_dlist_insert_before_(ds4vizP_dlist_ *p_l, int nid, ds4vizP_val_ v, int line)
{
    if (p_l->err)
    {
        return -1;
    }
    ds4vizP_dlnode_ *p_n = ds4vizP_dlist_find_(p_l, nid);
    if (!p_n)
    {
        DS4VIZ_ERR_(p_l, line, "Node not found: %d", nid);
        return -1;
    }
    v = ds4vizP_val_dup_(v);
    int id = ds4vizP_dlist_alloc_(p_l, v);
    p_l->nodes[id].next = nid;
    p_l->nodes[id].prev = p_n->prev;
    if (p_n->prev >= 0)
    {
        p_l->nodes[p_n->prev].next = id;
    }
    else
    {
        p_l->head = id;
    }
    p_n->prev = id;
    int before = p_l->state_id - 1;
    ds4vizP_dlist_ws_(p_l);
    int after = p_l->state_id++;
    ds4vizP_buf_ args = {0};
    ds4vizP_arg_int_(&args, "node_id", nid);
    ds4vizP_arg_val_(&args, "value", v);
    char ret_str[32];
    snprintf(ret_str, sizeof(ret_str), "%d", id);
    ds4vizP_write_step_(&p_l->steps_buf, p_l->step_id++, "insert_before",
                        before, after, args.data, ret_str, line);
    ds4vizP_buf_free_(&args);
    return id;
}

int ds4vizP_dlist_insert_after_(ds4vizP_dlist_ *p_l, int nid, ds4vizP_val_ v, int line)
{
    if (p_l->err)
    {
        return -1;
    }
    ds4vizP_dlnode_ *p_n = ds4vizP_dlist_find_(p_l, nid);
    if (!p_n)
    {
        DS4VIZ_ERR_(p_l, line, "Node not found: %d", nid);
        return -1;
    }
    v = ds4vizP_val_dup_(v);
    int id = ds4vizP_dlist_alloc_(p_l, v);
    p_l->nodes[id].prev = nid;
    p_l->nodes[id].next = p_n->next;
    if (p_n->next >= 0)
    {
        p_l->nodes[p_n->next].prev = id;
    }
    else
    {
        p_l->tail = id;
    }
    p_n->next = id;
    int before = p_l->state_id - 1;
    ds4vizP_dlist_ws_(p_l);
    int after = p_l->state_id++;
    ds4vizP_buf_ args = {0};
    ds4vizP_arg_int_(&args, "node_id", nid);
    ds4vizP_arg_val_(&args, "value", v);
    char ret_str[32];
    snprintf(ret_str, sizeof(ret_str), "%d", id);
    ds4vizP_write_step_(&p_l->steps_buf, p_l->step_id++, "insert_after",
                        before, after, args.data, ret_str, line);
    ds4vizP_buf_free_(&args);
    return id;
}

void ds4vizP_dlist_delete_(ds4vizP_dlist_ *p_l, int nid, int line)
{
    if (p_l->err)
    {
        return;
    }
    ds4vizP_dlnode_ *p_n = ds4vizP_dlist_find_(p_l, nid);
    if (!p_n)
    {
        DS4VIZ_ERR_(p_l, line, "Node not found: %d", nid);
        return;
    }
    ds4vizP_val_ deleted_val = p_n->value;
    if (p_n->prev >= 0)
    {
        p_l->nodes[p_n->prev].next = p_n->next;
    }
    else
    {
        p_l->head = p_n->next;
    }
    if (p_n->next >= 0)
    {
        p_l->nodes[p_n->next].prev = p_n->prev;
    }
    else
    {
        p_l->tail = p_n->prev;
    }
    p_n->alive = false;
    int before = p_l->state_id - 1;
    ds4vizP_dlist_ws_(p_l);
    int after = p_l->state_id++;
    ds4vizP_buf_ args = {0};
    ds4vizP_arg_int_(&args, "node_id", nid);
    char ret_str[256];
    ds4vizP_val_to_str_(ret_str, sizeof(ret_str), deleted_val);
    ds4vizP_write_step_(&p_l->steps_buf, p_l->step_id++, "delete",
                        before, after, args.data, ret_str, line);
    ds4vizP_buf_free_(&args);
    ds4vizP_val_free_(&deleted_val);
}

void ds4vizP_dlist_delete_head_(ds4vizP_dlist_ *p_l, int line)
{
    if (p_l->err)
    {
        return;
    }
    if (p_l->head < 0)
    {
        DS4VIZ_ERR_(p_l, line, "Cannot delete from empty list");
        return;
    }
    ds4vizP_dlist_delete_(p_l, p_l->head, line);
}

void ds4vizP_dlist_delete_tail_(ds4vizP_dlist_ *p_l, int line)
{
    if (p_l->err)
    {
        return;
    }
    if (p_l->tail < 0)
    {
        DS4VIZ_ERR_(p_l, line, "Cannot delete from empty list");
        return;
    }
    ds4vizP_dlist_delete_(p_l, p_l->tail, line);
}

void ds4vizP_dlist_reverse_(ds4vizP_dlist_ *p_l, int line)
{
    if (p_l->err)
    {
        return;
    }
    int cur = p_l->head;
    while (cur >= 0)
    {
        ds4vizP_dlnode_ *p_n = &p_l->nodes[cur];
        int nxt = p_n->next;
        int tmp = p_n->prev;
        p_n->prev = p_n->next;
        p_n->next = tmp;
        cur = nxt;
    }
    int tmp = p_l->head;
    p_l->head = p_l->tail;
    p_l->tail = tmp;
    int before = p_l->state_id - 1;
    ds4vizP_dlist_ws_(p_l);
    int after = p_l->state_id++;
    ds4vizP_write_step_(&p_l->steps_buf, p_l->step_id++, "reverse",
                        before, after, NULL, NULL, line);
}

/* ================================================================
 *  二叉树 (同时用于 BST)
 * ================================================================ */

/**
 * @brief 写入二叉树状态快照 (不含 parent 字段, 匹配 Python IR)
 */
static void ds4vizP_bt_ws_(ds4vizP_bt_ *p_t)
{
    ds4vizP_buf_ *p_b = &p_t->states_buf;
    ds4vizP_begin_state_(p_b, p_t->state_id);
    ds4vizP_buf_printf_(p_b, "root = %d\n", p_t->root);

    int alive_count = 0;
    for (int i = 0; i < p_t->ncnt; i++)
    {
        if (p_t->nodes[i].alive)
        {
            alive_count++;
        }
    }

    if (alive_count == 0)
    {
        ds4vizP_buf_cat_(p_b, "nodes = []\n");
    }
    else
    {
        ds4vizP_buf_cat_(p_b, "nodes = [\n");
        bool first = true;
        for (int i = 0; i < p_t->ncnt; i++)
        {
            if (!p_t->nodes[i].alive)
            {
                continue;
            }
            if (!first)
            {
                ds4vizP_buf_cat_(p_b, ",\n");
            }
            first = false;
            ds4vizP_buf_printf_(p_b, "  { id = %d, value = ", p_t->nodes[i].id);
            ds4vizP_toml_val_(p_b, p_t->nodes[i].value);
            ds4vizP_buf_printf_(p_b, ", left = %d, right = %d }",
                                p_t->nodes[i].left, p_t->nodes[i].right);
        }
        ds4vizP_buf_cat_(p_b, "\n]\n");
    }
}

static ds4vizP_tnode_ *ds4vizP_bt_find_(ds4vizP_bt_ *p_t, int nid)
{
    if (nid >= 0 && nid < p_t->ncnt && p_t->nodes[nid].alive)
    {
        return &p_t->nodes[nid];
    }
    return NULL;
}

static int ds4vizP_bt_alloc_(ds4vizP_bt_ *p_t, ds4vizP_val_ v)
{
    int id = p_t->nid++;
    if (id >= p_t->ncap)
    {
        int nc = p_t->ncap ? p_t->ncap * 2 : 8;
        while (nc <= id)
        {
            nc *= 2;
        }
        p_t->nodes = (ds4vizP_tnode_ *)realloc(p_t->nodes, (size_t)nc * sizeof(ds4vizP_tnode_));
        p_t->ncap = nc;
    }
    p_t->nodes[id] = (ds4vizP_tnode_){.id = id, .value = v, .left = -1, .right = -1, .alive = true};
    p_t->ncnt = p_t->nid;
    return id;
}

/**
 * @brief 查找节点的父节点 ID
 */
static int ds4vizP_bt_find_parent_(ds4vizP_bt_ *p_t, int nid)
{
    for (int i = 0; i < p_t->ncnt; i++)
    {
        if (p_t->nodes[i].alive &&
            (p_t->nodes[i].left == nid || p_t->nodes[i].right == nid))
        {
            return i;
        }
    }
    return -1;
}

/**
 * @brief 递归收集子树所有节点 ID
 */
static void ds4vizP_bt_collect_subtree_(ds4vizP_bt_ *p_t, int nid, int *ids, int *cnt)
{
    if (nid < 0 || nid >= p_t->ncnt || !p_t->nodes[nid].alive)
    {
        return;
    }
    ids[(*cnt)++] = nid;
    ds4vizP_bt_collect_subtree_(p_t, p_t->nodes[nid].left, ids, cnt);
    ds4vizP_bt_collect_subtree_(p_t, p_t->nodes[nid].right, ids, cnt);
}

ds4vizP_bt_ ds4vizP_bt_open_(const char *kind, const char *label, int line)
{
    ds4vizP_bt_ t = {0};
    t.label = label;
    t.kind = kind;
    t.scope_line = line;
    t.root = -1;
    ds4vizP_bt_ws_(&t);
    t.state_id++;
    return t;
}

void ds4vizP_bt_close_(ds4vizP_bt_ *p_t)
{
    ds4vizP_flush_(p_t->kind, p_t->label, &p_t->states_buf, &p_t->steps_buf,
                   p_t->err, p_t->errmsg, p_t->err_step, p_t->err_last_state,
                   p_t->err_line, p_t->state_id - 1, p_t->scope_line);
    for (int i = 0; i < p_t->ncnt; i++)
    {
        if (p_t->nodes[i].alive)
        {
            ds4vizP_val_free_(&p_t->nodes[i].value);
        }
    }
    free(p_t->nodes);
    ds4vizP_buf_free_(&p_t->states_buf);
    ds4vizP_buf_free_(&p_t->steps_buf);
    p_t->done = true;
}

int ds4vizP_bt_insert_root_(ds4vizP_bt_ *p_t, ds4vizP_val_ v, int line)
{
    if (p_t->err)
    {
        return -1;
    }
    if (p_t->root >= 0)
    {
        DS4VIZ_ERR_(p_t, line, "Root node already exists");
        return -1;
    }
    v = ds4vizP_val_dup_(v);
    int id = ds4vizP_bt_alloc_(p_t, v);
    p_t->root = id;
    int before = p_t->state_id - 1;
    ds4vizP_bt_ws_(p_t);
    int after = p_t->state_id++;
    ds4vizP_buf_ args = {0};
    ds4vizP_arg_val_(&args, "value", v);
    char ret_str[32];
    snprintf(ret_str, sizeof(ret_str), "%d", id);
    ds4vizP_write_step_(&p_t->steps_buf, p_t->step_id++, "insert_root",
                        before, after, args.data, ret_str, line);
    ds4vizP_buf_free_(&args);
    return id;
}

int ds4vizP_bt_insert_left_(ds4vizP_bt_ *p_t, int pid, ds4vizP_val_ v, int line)
{
    if (p_t->err)
    {
        return -1;
    }
    ds4vizP_tnode_ *p_p = ds4vizP_bt_find_(p_t, pid);
    if (!p_p)
    {
        DS4VIZ_ERR_(p_t, line, "Parent node not found: %d", pid);
        return -1;
    }
    if (p_p->left >= 0)
    {
        DS4VIZ_ERR_(p_t, line, "Left child already exists for node: %d", pid);
        return -1;
    }
    v = ds4vizP_val_dup_(v);
    int id = ds4vizP_bt_alloc_(p_t, v);
    p_p->left = id;
    int before = p_t->state_id - 1;
    ds4vizP_bt_ws_(p_t);
    int after = p_t->state_id++;
    ds4vizP_buf_ args = {0};
    ds4vizP_arg_int_(&args, "parent", pid);
    ds4vizP_arg_val_(&args, "value", v);
    char ret_str[32];
    snprintf(ret_str, sizeof(ret_str), "%d", id);
    ds4vizP_write_step_(&p_t->steps_buf, p_t->step_id++, "insert_left",
                        before, after, args.data, ret_str, line);
    ds4vizP_buf_free_(&args);
    return id;
}

int ds4vizP_bt_insert_right_(ds4vizP_bt_ *p_t, int pid, ds4vizP_val_ v, int line)
{
    if (p_t->err)
    {
        return -1;
    }
    ds4vizP_tnode_ *p_p = ds4vizP_bt_find_(p_t, pid);
    if (!p_p)
    {
        DS4VIZ_ERR_(p_t, line, "Parent node not found: %d", pid);
        return -1;
    }
    if (p_p->right >= 0)
    {
        DS4VIZ_ERR_(p_t, line, "Right child already exists for node: %d", pid);
        return -1;
    }
    v = ds4vizP_val_dup_(v);
    int id = ds4vizP_bt_alloc_(p_t, v);
    p_p->right = id;
    int before = p_t->state_id - 1;
    ds4vizP_bt_ws_(p_t);
    int after = p_t->state_id++;
    ds4vizP_buf_ args = {0};
    ds4vizP_arg_int_(&args, "parent", pid);
    ds4vizP_arg_val_(&args, "value", v);
    char ret_str[32];
    snprintf(ret_str, sizeof(ret_str), "%d", id);
    ds4vizP_write_step_(&p_t->steps_buf, p_t->step_id++, "insert_right",
                        before, after, args.data, ret_str, line);
    ds4vizP_buf_free_(&args);
    return id;
}

void ds4vizP_bt_delete_(ds4vizP_bt_ *p_t, int nid, int line)
{
    if (p_t->err)
    {
        return;
    }
    ds4vizP_tnode_ *p_n = ds4vizP_bt_find_(p_t, nid);
    if (!p_n)
    {
        DS4VIZ_ERR_(p_t, line, "Node not found: %d", nid);
        return;
    }

    /* 收集子树 */
    int *subtree_ids = (int *)malloc((size_t)p_t->ncnt * sizeof(int));
    int sub_cnt = 0;
    ds4vizP_bt_collect_subtree_(p_t, nid, subtree_ids, &sub_cnt);

    /* 断开父链接 */
    int par = ds4vizP_bt_find_parent_(p_t, nid);
    if (par >= 0)
    {
        if (p_t->nodes[par].left == nid)
        {
            p_t->nodes[par].left = -1;
        }
        if (p_t->nodes[par].right == nid)
        {
            p_t->nodes[par].right = -1;
        }
    }
    else
    {
        p_t->root = -1;
    }

    /* 删除子树所有节点 */
    for (int i = 0; i < sub_cnt; i++)
    {
        int sid = subtree_ids[i];
        p_t->nodes[sid].alive = false;
        ds4vizP_val_free_(&p_t->nodes[sid].value);
    }
    free(subtree_ids);

    int before = p_t->state_id - 1;
    ds4vizP_bt_ws_(p_t);
    int after = p_t->state_id++;
    ds4vizP_buf_ args = {0};
    ds4vizP_arg_int_(&args, "node_id", nid);
    ds4vizP_write_step_(&p_t->steps_buf, p_t->step_id++, "delete",
                        before, after, args.data, NULL, line);
    ds4vizP_buf_free_(&args);
}

void ds4vizP_bt_update_value_(ds4vizP_bt_ *p_t, int nid, ds4vizP_val_ v, int line)
{
    if (p_t->err)
    {
        return;
    }
    ds4vizP_tnode_ *p_n = ds4vizP_bt_find_(p_t, nid);
    if (!p_n)
    {
        DS4VIZ_ERR_(p_t, line, "Node not found: %d", nid);
        return;
    }
    v = ds4vizP_val_dup_(v);
    ds4vizP_val_ old_val = p_n->value;
    p_n->value = v;
    int before = p_t->state_id - 1;
    ds4vizP_bt_ws_(p_t);
    int after = p_t->state_id++;
    ds4vizP_buf_ args = {0};
    ds4vizP_arg_int_(&args, "node_id", nid);
    ds4vizP_arg_val_(&args, "value", v);
    char ret_str[256];
    ds4vizP_val_to_str_(ret_str, sizeof(ret_str), old_val);
    ds4vizP_write_step_(&p_t->steps_buf, p_t->step_id++, "update_value",
                        before, after, args.data, ret_str, line);
    ds4vizP_buf_free_(&args);
    ds4vizP_val_free_(&old_val);
}

/* ================================================================
 *  二叉搜索树 (复用 ds4vizP_bt_, 值复制语义匹配 Python)
 * ================================================================ */

int ds4vizP_bst_insert_(ds4vizP_bt_ *p_t, ds4vizP_val_ v, int line)
{
    if (p_t->err)
    {
        return -1;
    }
    v = ds4vizP_val_dup_(v);
    double val = ds4vizP_val_num_(v);
    int id = ds4vizP_bt_alloc_(p_t, v);
    if (p_t->root < 0)
    {
        p_t->root = id;
    }
    else
    {
        int cur = p_t->root;
        for (;;)
        {
            double cv = ds4vizP_val_num_(p_t->nodes[cur].value);
            if (val < cv)
            {
                if (p_t->nodes[cur].left < 0)
                {
                    p_t->nodes[cur].left = id;
                    break;
                }
                cur = p_t->nodes[cur].left;
            }
            else
            {
                if (p_t->nodes[cur].right < 0)
                {
                    p_t->nodes[cur].right = id;
                    break;
                }
                cur = p_t->nodes[cur].right;
            }
        }
    }
    int before = p_t->state_id - 1;
    ds4vizP_bt_ws_(p_t);
    int after = p_t->state_id++;
    ds4vizP_buf_ args = {0};
    ds4vizP_arg_val_(&args, "value", v);
    char ret_str[32];
    snprintf(ret_str, sizeof(ret_str), "%d", id);
    ds4vizP_write_step_(&p_t->steps_buf, p_t->step_id++, "insert",
                        before, after, args.data, ret_str, line);
    ds4vizP_buf_free_(&args);
    return id;
}

/**
 * @brief 在 BST 中按值查找节点 ID
 */
static int ds4vizP_bst_findval_(ds4vizP_bt_ *p_t, double target)
{
    for (int i = 0; i < p_t->ncnt; i++)
    {
        if (p_t->nodes[i].alive && fabs(ds4vizP_val_num_(p_t->nodes[i].value) - target) < 1e-12)
        {
            return i;
        }
    }
    return -1;
}

/**
 * @brief BST 查找最小值节点
 */
static int ds4vizP_bst_min_(ds4vizP_bt_ *p_t, int nid)
{
    while (p_t->nodes[nid].left >= 0)
    {
        nid = p_t->nodes[nid].left;
    }
    return nid;
}

/**
 * @brief 在父节点中替换子节点引用
 */
static void ds4vizP_bst_replace_in_parent_(ds4vizP_bt_ *p_t, int par, int old_child, int new_child)
{
    if (par < 0)
    {
        p_t->root = new_child;
    }
    else if (p_t->nodes[par].left == old_child)
    {
        p_t->nodes[par].left = new_child;
    }
    else
    {
        p_t->nodes[par].right = new_child;
    }
}

/**
 * @brief BST 删除节点 (值复制语义, 匹配 Python)
 */
static void ds4vizP_bst_delete_node_(ds4vizP_bt_ *p_t, int nid)
{
    ds4vizP_tnode_ *p_n = &p_t->nodes[nid];
    int par = ds4vizP_bt_find_parent_(p_t, nid);
    int left_child = p_n->left;
    int right_child = p_n->right;

    if (left_child < 0 && right_child < 0)
    {
        ds4vizP_bst_replace_in_parent_(p_t, par, nid, -1);
        p_n->alive = false;
        ds4vizP_val_free_(&p_n->value);
    }
    else if (left_child < 0)
    {
        ds4vizP_bst_replace_in_parent_(p_t, par, nid, right_child);
        p_n->alive = false;
        ds4vizP_val_free_(&p_n->value);
    }
    else if (right_child < 0)
    {
        ds4vizP_bst_replace_in_parent_(p_t, par, nid, left_child);
        p_n->alive = false;
        ds4vizP_val_free_(&p_n->value);
    }
    else
    {
        int suc = ds4vizP_bst_min_(p_t, right_child);
        ds4vizP_val_ suc_val = ds4vizP_val_dup_(p_t->nodes[suc].value);
        ds4vizP_bst_delete_node_(p_t, suc);
        ds4vizP_val_free_(&p_t->nodes[nid].value);
        p_t->nodes[nid].value = suc_val;
    }
}

void ds4vizP_bst_delete_(ds4vizP_bt_ *p_t, ds4vizP_val_ v, int line)
{
    if (p_t->err)
    {
        return;
    }
    double target = ds4vizP_val_num_(v);
    int nid = ds4vizP_bst_findval_(p_t, target);
    if (nid < 0)
    {
        DS4VIZ_ERR_(p_t, line, "Node with value %s not found",
                    (v.type == DS4VIZ_VINT_) ? "int" : "float");
        /* 生成更友好的错误消息 */
        if (v.type == DS4VIZ_VINT_)
        {
            snprintf(p_t->errmsg, sizeof(p_t->errmsg), "Node with value %lld not found", v.i);
        }
        else
        {
            snprintf(p_t->errmsg, sizeof(p_t->errmsg), "Node with value %g not found", v.d);
        }
        return;
    }
    ds4vizP_bst_delete_node_(p_t, nid);
    int before = p_t->state_id - 1;
    ds4vizP_bt_ws_(p_t);
    int after = p_t->state_id++;
    ds4vizP_buf_ args = {0};
    ds4vizP_arg_val_(&args, "value", v);
    ds4vizP_write_step_(&p_t->steps_buf, p_t->step_id++, "delete",
                        before, after, args.data, NULL, line);
    ds4vizP_buf_free_(&args);
}

/* ================================================================
 *  堆 (存储为扁平数组, 序列化为二叉树节点, kind="binary_tree")
 * ================================================================ */

/**
 * @brief 写入堆状态快照 (序列化为二叉树节点, 匹配 Python)
 */
static void ds4vizP_heap_ws_(ds4vizP_heap_ *p_h)
{
    ds4vizP_buf_ *p_b = &p_h->states_buf;
    ds4vizP_begin_state_(p_b, p_h->state_id);

    if (p_h->count == 0)
    {
        ds4vizP_buf_cat_(p_b, "root = -1\nnodes = []\n");
    }
    else
    {
        ds4vizP_buf_cat_(p_b, "root = 0\nnodes = [\n");
        for (int i = 0; i < p_h->count; i++)
        {
            if (i > 0)
            {
                ds4vizP_buf_cat_(p_b, ",\n");
            }
            int left = (2 * i + 1 < p_h->count) ? 2 * i + 1 : -1;
            int right = (2 * i + 2 < p_h->count) ? 2 * i + 2 : -1;
            ds4vizP_buf_printf_(p_b, "  { id = %d, value = ", i);
            ds4vizP_toml_val_(p_b, p_h->items[i]);
            ds4vizP_buf_printf_(p_b, ", left = %d, right = %d }", left, right);
        }
        ds4vizP_buf_cat_(p_b, "\n]\n");
    }
}

static bool ds4vizP_heap_cmp_(ds4vizP_heap_ *p_h, int a, int b)
{
    double va = ds4vizP_val_num_(p_h->items[a]);
    double vb = ds4vizP_val_num_(p_h->items[b]);
    return p_h->order == 0 ? (va < vb) : (va > vb);
}

static void ds4vizP_heap_swap_(ds4vizP_heap_ *p_h, int a, int b)
{
    ds4vizP_val_ tmp = p_h->items[a];
    p_h->items[a] = p_h->items[b];
    p_h->items[b] = tmp;
}

static void ds4vizP_heap_up_(ds4vizP_heap_ *p_h, int i)
{
    while (i > 0)
    {
        int p = (i - 1) / 2;
        if (ds4vizP_heap_cmp_(p_h, i, p))
        {
            ds4vizP_heap_swap_(p_h, i, p);
            i = p;
        }
        else
        {
            break;
        }
    }
}

static void ds4vizP_heap_down_(ds4vizP_heap_ *p_h, int i)
{
    for (;;)
    {
        int best = i;
        int l = 2 * i + 1;
        int r = 2 * i + 2;
        if (l < p_h->count && ds4vizP_heap_cmp_(p_h, l, best))
        {
            best = l;
        }
        if (r < p_h->count && ds4vizP_heap_cmp_(p_h, r, best))
        {
            best = r;
        }
        if (best != i)
        {
            ds4vizP_heap_swap_(p_h, i, best);
            i = best;
        }
        else
        {
            break;
        }
    }
}

ds4vizP_heap_ ds4vizP_heap_open_(const char *label, int order, int line)
{
    ds4vizP_heap_ h = {0};
    h.label = label;
    h.kind = "binary_tree";
    h.scope_line = line;
    h.order = order;
    ds4vizP_heap_ws_(&h);
    h.state_id++;
    return h;
}

void ds4vizP_heap_close_(ds4vizP_heap_ *p_h)
{
    ds4vizP_flush_(p_h->kind, p_h->label, &p_h->states_buf, &p_h->steps_buf,
                   p_h->err, p_h->errmsg, p_h->err_step, p_h->err_last_state,
                   p_h->err_line, p_h->state_id - 1, p_h->scope_line);
    for (int i = 0; i < p_h->count; i++)
    {
        ds4vizP_val_free_(&p_h->items[i]);
    }
    free(p_h->items);
    ds4vizP_buf_free_(&p_h->states_buf);
    ds4vizP_buf_free_(&p_h->steps_buf);
    p_h->done = true;
}

void ds4vizP_heap_insert_(ds4vizP_heap_ *p_h, ds4vizP_val_ v, int line)
{
    if (p_h->err)
    {
        return;
    }
    v = ds4vizP_val_dup_(v);
    DS4VIZ_GROW_(p_h->items, p_h->count, p_h->cap, ds4vizP_val_);
    p_h->items[p_h->count++] = v;
    ds4vizP_heap_up_(p_h, p_h->count - 1);
    int before = p_h->state_id - 1;
    ds4vizP_heap_ws_(p_h);
    int after = p_h->state_id++;
    ds4vizP_buf_ args = {0};
    ds4vizP_arg_val_(&args, "value", v);
    ds4vizP_write_step_(&p_h->steps_buf, p_h->step_id++, "insert",
                        before, after, args.data, NULL, line);
    ds4vizP_buf_free_(&args);
}

void ds4vizP_heap_extract_(ds4vizP_heap_ *p_h, int line)
{
    if (p_h->err)
    {
        return;
    }
    if (p_h->count == 0)
    {
        DS4VIZ_ERR_(p_h, line, "Cannot extract from empty heap");
        return;
    }
    ds4vizP_val_ extracted = p_h->items[0];
    if (p_h->count == 1)
    {
        p_h->count = 0;
    }
    else
    {
        p_h->items[0] = p_h->items[--p_h->count];
        ds4vizP_heap_down_(p_h, 0);
    }
    int before = p_h->state_id - 1;
    ds4vizP_heap_ws_(p_h);
    int after = p_h->state_id++;
    char ret_str[256];
    ds4vizP_val_to_str_(ret_str, sizeof(ret_str), extracted);
    ds4vizP_write_step_(&p_h->steps_buf, p_h->step_id++, "extract",
                        before, after, NULL, ret_str, line);
    ds4vizP_val_free_(&extracted);
}

void ds4vizP_heap_clear_(ds4vizP_heap_ *p_h, int line)
{
    if (p_h->err)
    {
        return;
    }
    for (int i = 0; i < p_h->count; i++)
    {
        ds4vizP_val_free_(&p_h->items[i]);
    }
    p_h->count = 0;
    int before = p_h->state_id - 1;
    ds4vizP_heap_ws_(p_h);
    int after = p_h->state_id++;
    ds4vizP_write_step_(&p_h->steps_buf, p_h->step_id++, "clear",
                        before, after, NULL, NULL, line);
}

/* ================================================================
 *  图 (统一: 无向/有向/带权)
 * ================================================================ */

/**
 * @brief 写入图状态快照 (节点和边按排序输出, 匹配 Python)
 */
static void ds4vizP_graph_ws_(ds4vizP_graph_ *p_g)
{
    ds4vizP_buf_ *p_b = &p_g->states_buf;
    ds4vizP_begin_state_(p_b, p_g->state_id);

    /* 收集并排序存活节点 */
    int alive_nodes = 0;
    for (int i = 0; i < p_g->ncnt; i++)
    {
        if (p_g->nodes[i].alive)
        {
            alive_nodes++;
        }
    }

    if (alive_nodes == 0)
    {
        ds4vizP_buf_cat_(p_b, "nodes = []\n");
    }
    else
    {
        /* 按 id 排序的节点索引 */
        int *sorted_ni = (int *)malloc((size_t)alive_nodes * sizeof(int));
        int si = 0;
        for (int i = 0; i < p_g->ncnt; i++)
        {
            if (p_g->nodes[i].alive)
            {
                sorted_ni[si++] = i;
            }
        }
        for (int i = 0; i < si - 1; i++)
        {
            for (int j = 0; j < si - 1 - i; j++)
            {
                if (p_g->nodes[sorted_ni[j]].id > p_g->nodes[sorted_ni[j + 1]].id)
                {
                    int tmp = sorted_ni[j];
                    sorted_ni[j] = sorted_ni[j + 1];
                    sorted_ni[j + 1] = tmp;
                }
            }
        }
        ds4vizP_buf_cat_(p_b, "nodes = [\n");
        for (int i = 0; i < si; i++)
        {
            if (i > 0)
            {
                ds4vizP_buf_cat_(p_b, ",\n");
            }
            ds4vizP_gnode_ *p_n = &p_g->nodes[sorted_ni[i]];
            ds4vizP_buf_printf_(p_b, "  { id = %d, label = ", p_n->id);
            ds4vizP_toml_str_(p_b, p_n->label);
            ds4vizP_buf_cat_(p_b, " }");
        }
        ds4vizP_buf_cat_(p_b, "\n]\n");
        free(sorted_ni);
    }

    /* 收集并排序存活边 */
    int alive_edges = 0;
    for (int i = 0; i < p_g->ecnt; i++)
    {
        if (p_g->edges[i].alive)
        {
            alive_edges++;
        }
    }

    if (alive_edges == 0)
    {
        ds4vizP_buf_cat_(p_b, "edges = []\n");
    }
    else
    {
        int *sorted_ei = (int *)malloc((size_t)alive_edges * sizeof(int));
        int ei = 0;
        for (int i = 0; i < p_g->ecnt; i++)
        {
            if (p_g->edges[i].alive)
            {
                sorted_ei[ei++] = i;
            }
        }
        for (int i = 0; i < ei - 1; i++)
        {
            for (int j = 0; j < ei - 1 - i; j++)
            {
                ds4vizP_gedge_ *p_a = &p_g->edges[sorted_ei[j]];
                ds4vizP_gedge_ *p_b2 = &p_g->edges[sorted_ei[j + 1]];
                if (p_a->from > p_b2->from || (p_a->from == p_b2->from && p_a->to > p_b2->to))
                {
                    int tmp = sorted_ei[j];
                    sorted_ei[j] = sorted_ei[j + 1];
                    sorted_ei[j + 1] = tmp;
                }
            }
        }
        ds4vizP_buf_cat_(p_b, "edges = [\n");
        for (int i = 0; i < ei; i++)
        {
            if (i > 0)
            {
                ds4vizP_buf_cat_(p_b, ",\n");
            }
            ds4vizP_gedge_ *p_e = &p_g->edges[sorted_ei[i]];
            ds4vizP_buf_printf_(p_b, "  { from = %d, to = %d", p_e->from, p_e->to);
            if (p_g->weighted)
            {
                ds4vizP_buf_cat_(p_b, ", weight = ");
                ds4vizP_toml_val_(p_b, (ds4vizP_val_){.type = DS4VIZ_VDBL_, .d = p_e->weight});
            }
            ds4vizP_buf_cat_(p_b, " }");
        }
        ds4vizP_buf_cat_(p_b, "\n]\n");
        free(sorted_ei);
    }
}

static ds4vizP_gnode_ *ds4vizP_graph_findnode_(ds4vizP_graph_ *p_g, int nid)
{
    for (int i = 0; i < p_g->ncnt; i++)
    {
        if (p_g->nodes[i].alive && p_g->nodes[i].id == nid)
        {
            return &p_g->nodes[i];
        }
    }
    return NULL;
}

static ds4vizP_gedge_ *ds4vizP_graph_findedge_(ds4vizP_graph_ *p_g, int from, int to)
{
    for (int i = 0; i < p_g->ecnt; i++)
    {
        if (p_g->edges[i].alive && p_g->edges[i].from == from && p_g->edges[i].to == to)
        {
            return &p_g->edges[i];
        }
    }
    return NULL;
}

ds4vizP_graph_ ds4vizP_graph_open_(const char *kind, const char *label,
                                   bool directed, bool weighted, int line)
{
    ds4vizP_graph_ g = {0};
    g.label = label;
    g.kind = kind;
    g.scope_line = line;
    g.directed = directed;
    g.weighted = weighted;
    ds4vizP_graph_ws_(&g);
    g.state_id++;
    return g;
}

void ds4vizP_graph_close_(ds4vizP_graph_ *p_g)
{
    ds4vizP_flush_(p_g->kind, p_g->label, &p_g->states_buf, &p_g->steps_buf,
                   p_g->err, p_g->errmsg, p_g->err_step, p_g->err_last_state,
                   p_g->err_line, p_g->state_id - 1, p_g->scope_line);
    free(p_g->nodes);
    free(p_g->edges);
    ds4vizP_buf_free_(&p_g->states_buf);
    ds4vizP_buf_free_(&p_g->steps_buf);
    p_g->done = true;
}

void ds4vizP_graph_add_node_(ds4vizP_graph_ *p_g, int nid, const char *lbl, int line)
{
    if (p_g->err)
    {
        return;
    }
    if (ds4vizP_graph_findnode_(p_g, nid))
    {
        DS4VIZ_ERR_(p_g, line, "Node already exists: %d", nid);
        return;
    }
    int lbl_len = lbl ? (int)strlen(lbl) : 0;
    if (lbl_len < 1 || lbl_len > 32)
    {
        DS4VIZ_ERR_(p_g, line, "Label length must be 1-32, got %d", lbl_len);
        return;
    }
    DS4VIZ_GROW_(p_g->nodes, p_g->ncnt, p_g->ncap, ds4vizP_gnode_);
    ds4vizP_gnode_ *p_n = &p_g->nodes[p_g->ncnt++];
    p_n->id = nid;
    p_n->alive = true;
    strncpy(p_n->label, lbl, 32);
    p_n->label[32] = '\0';
    int before = p_g->state_id - 1;
    ds4vizP_graph_ws_(p_g);
    int after = p_g->state_id++;
    ds4vizP_buf_ args = {0};
    ds4vizP_arg_int_(&args, "id", nid);
    ds4vizP_arg_str_(&args, "label", lbl);
    ds4vizP_write_step_(&p_g->steps_buf, p_g->step_id++, "add_node",
                        before, after, args.data, NULL, line);
    ds4vizP_buf_free_(&args);
}

void ds4vizP_graph_add_edge_(ds4vizP_graph_ *p_g, int from, int to, double w, int line)
{
    if (p_g->err)
    {
        return;
    }
    if (from == to)
    {
        DS4VIZ_ERR_(p_g, line, "Self-loop not allowed: %d", from);
        return;
    }
    if (!ds4vizP_graph_findnode_(p_g, from))
    {
        DS4VIZ_ERR_(p_g, line, "Node not found: %d", from);
        return;
    }
    if (!ds4vizP_graph_findnode_(p_g, to))
    {
        DS4VIZ_ERR_(p_g, line, "Node not found: %d", to);
        return;
    }
    int ef = from;
    int et = to;
    if (!p_g->directed && ef > et)
    {
        int tmp = ef;
        ef = et;
        et = tmp;
    }
    if (ds4vizP_graph_findedge_(p_g, ef, et))
    {
        DS4VIZ_ERR_(p_g, line, "Edge already exists: (%d, %d)", ef, et);
        return;
    }
    DS4VIZ_GROW_(p_g->edges, p_g->ecnt, p_g->ecap, ds4vizP_gedge_);
    p_g->edges[p_g->ecnt++] = (ds4vizP_gedge_){.from = ef, .to = et, .weight = w, .alive = true};
    int before = p_g->state_id - 1;
    ds4vizP_graph_ws_(p_g);
    int after = p_g->state_id++;
    ds4vizP_buf_ args = {0};
    ds4vizP_arg_int_(&args, "from", from);
    ds4vizP_arg_int_(&args, "to", to);
    if (p_g->weighted)
    {
        ds4vizP_arg_dbl_(&args, "weight", w);
    }
    ds4vizP_write_step_(&p_g->steps_buf, p_g->step_id++, "add_edge",
                        before, after, args.data, NULL, line);
    ds4vizP_buf_free_(&args);
}

void ds4vizP_graph_remove_node_(ds4vizP_graph_ *p_g, int nid, int line)
{
    if (p_g->err)
    {
        return;
    }
    ds4vizP_gnode_ *p_n = ds4vizP_graph_findnode_(p_g, nid);
    if (!p_n)
    {
        DS4VIZ_ERR_(p_g, line, "Node not found: %d", nid);
        return;
    }
    p_n->alive = false;
    for (int i = 0; i < p_g->ecnt; i++)
    {
        if (p_g->edges[i].alive &&
            (p_g->edges[i].from == nid || p_g->edges[i].to == nid))
        {
            p_g->edges[i].alive = false;
        }
    }
    int before = p_g->state_id - 1;
    ds4vizP_graph_ws_(p_g);
    int after = p_g->state_id++;
    ds4vizP_buf_ args = {0};
    ds4vizP_arg_int_(&args, "node_id", nid);
    ds4vizP_write_step_(&p_g->steps_buf, p_g->step_id++, "remove_node",
                        before, after, args.data, NULL, line);
    ds4vizP_buf_free_(&args);
}

void ds4vizP_graph_remove_edge_(ds4vizP_graph_ *p_g, int from, int to, int line)
{
    if (p_g->err)
    {
        return;
    }
    int ef = from;
    int et = to;
    if (!p_g->directed && ef > et)
    {
        int tmp = ef;
        ef = et;
        et = tmp;
    }
    ds4vizP_gedge_ *p_e = ds4vizP_graph_findedge_(p_g, ef, et);
    if (!p_e)
    {
        DS4VIZ_ERR_(p_g, line, "Edge not found: (%d, %d)", from, to);
        return;
    }
    p_e->alive = false;
    int before = p_g->state_id - 1;
    ds4vizP_graph_ws_(p_g);
    int after = p_g->state_id++;
    ds4vizP_buf_ args = {0};
    ds4vizP_arg_int_(&args, "from", from);
    ds4vizP_arg_int_(&args, "to", to);
    ds4vizP_write_step_(&p_g->steps_buf, p_g->step_id++, "remove_edge",
                        before, after, args.data, NULL, line);
    ds4vizP_buf_free_(&args);
}

void ds4vizP_graph_update_node_label_(ds4vizP_graph_ *p_g, int nid, const char *lbl, int line)
{
    if (p_g->err)
    {
        return;
    }
    ds4vizP_gnode_ *p_n = ds4vizP_graph_findnode_(p_g, nid);
    if (!p_n)
    {
        DS4VIZ_ERR_(p_g, line, "Node not found: %d", nid);
        return;
    }
    int lbl_len = lbl ? (int)strlen(lbl) : 0;
    if (lbl_len < 1 || lbl_len > 32)
    {
        DS4VIZ_ERR_(p_g, line, "Label length must be 1-32, got %d", lbl_len);
        return;
    }
    char old_label[33];
    strncpy(old_label, p_n->label, 33);
    strncpy(p_n->label, lbl, 32);
    p_n->label[32] = '\0';
    int before = p_g->state_id - 1;
    ds4vizP_graph_ws_(p_g);
    int after = p_g->state_id++;
    ds4vizP_buf_ args = {0};
    ds4vizP_arg_int_(&args, "node_id", nid);
    ds4vizP_arg_str_(&args, "label", lbl);
    /* 构建 ret 字符串 */
    ds4vizP_buf_ ret_buf = {0};
    ds4vizP_toml_str_(&ret_buf, old_label);
    ds4vizP_write_step_(&p_g->steps_buf, p_g->step_id++, "update_node_label",
                        before, after, args.data, ret_buf.data, line);
    ds4vizP_buf_free_(&args);
    ds4vizP_buf_free_(&ret_buf);
}

void ds4vizP_graph_update_weight_(ds4vizP_graph_ *p_g, int from, int to, double w, int line)
{
    if (p_g->err)
    {
        return;
    }
    int ef = from;
    int et = to;
    if (!p_g->directed && ef > et)
    {
        int tmp = ef;
        ef = et;
        et = tmp;
    }
    ds4vizP_gedge_ *p_e = ds4vizP_graph_findedge_(p_g, ef, et);
    if (!p_e)
    {
        DS4VIZ_ERR_(p_g, line, "Edge not found: (%d, %d)", from, to);
        return;
    }
    double old_weight = p_e->weight;
    p_e->weight = w;
    int before = p_g->state_id - 1;
    ds4vizP_graph_ws_(p_g);
    int after = p_g->state_id++;
    ds4vizP_buf_ args = {0};
    ds4vizP_arg_int_(&args, "from", from);
    ds4vizP_arg_int_(&args, "to", to);
    ds4vizP_arg_dbl_(&args, "weight", w);
    char ret_str[64];
    ds4vizP_val_to_str_(ret_str, sizeof(ret_str),
                        (ds4vizP_val_){.type = DS4VIZ_VDBL_, .d = old_weight});
    ds4vizP_write_step_(&p_g->steps_buf, p_g->step_id++, "update_weight",
                        before, after, args.data, ret_str, line);
    ds4vizP_buf_free_(&args);
}

#endif /* DS4VIZ_IMPLEMENTATION */