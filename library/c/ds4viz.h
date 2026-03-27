/**
 * @file ds4viz.h
 * @brief ds4viz - 数据结构可视化 C23 单头文件库
 *
 * 生成与 Python ds4viz 库完全对齐的 TOML IR 文件,
 * 用于可扩展的数据结构可视化教学平台.
 *
 * 用法:
 *     #define DS4VIZ_IMPLEMENTATION
 *     #include "ds4viz.h"
 *
 * 可选:
 *     #define DS4VIZ_SHORT_NAMES
 *
 * @author WaterRun
 * @date 2026-03-27
 * @see https://github.com/Water-Run/ds4viz
 */

#ifndef DS4VIZ_H
#define DS4VIZ_H

#include <assert.h>
#include <math.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <time.h>

#ifdef __cplusplus
extern "C"
{
#endif

/* ================================================================
 * 公开宏定义
 * ================================================================ */

/**
 * @brief 预定义高亮样式: 当前焦点
 *
 * @def DS4VIZ_FOCUS
 */
#define DS4VIZ_FOCUS "focus"

/**
 * @brief 预定义高亮样式: 已访问
 *
 * @def DS4VIZ_VISITED
 */
#define DS4VIZ_VISITED "visited"

/**
 * @brief 预定义高亮样式: 参与当前计算
 *
 * @def DS4VIZ_ACTIVE
 */
#define DS4VIZ_ACTIVE "active"

/**
 * @brief 预定义高亮样式: 正在比较
 *
 * @def DS4VIZ_COMPARING
 */
#define DS4VIZ_COMPARING "comparing"

/**
 * @brief 预定义高亮样式: 查找命中
 *
 * @def DS4VIZ_FOUND
 */
#define DS4VIZ_FOUND "found"

/**
 * @brief 预定义高亮样式: 异常状态
 *
 * @def DS4VIZ_ERROR_STY
 */
#define DS4VIZ_ERROR_STY "error"

    /* ================================================================
     * 公开类型定义
     * ================================================================ */

    /**
     * @brief 全局配置选项
     *
     * 用于设置 TOML 输出路径及元数据.
     * 未指定的字段自动零初始化为 NULL, 使用默认值.
     *
     * @struct ds4viz_config_options_s
     */
    typedef struct ds4viz_config_options_s
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
     * @brief 公开值类型 (Pop/Dequeue 返回值)
     *
     * 带标签联合体, 通过 ds4vizInt / ds4vizFloat / ds4vizStr / ds4vizBool 提取值.
     *
     * @struct ds4viz_value_s
     */
    typedef struct ds4viz_value_s
    {
        /** @brief 值类型标签 */
        enum
        {
            DS4VIZ_INT = 0,
            DS4VIZ_FLOAT = 1,
            DS4VIZ_STR = 2,
            DS4VIZ_BOOL = 3
        } tag;
        /** @brief 值联合体 */
        union
        {
            int64_t i;
            double f;
            const char *p_s;
            bool b;
        };
    } ds4vizValue;

    /**
     * @brief 从 ds4vizValue 提取 int64_t
     *
     * @param v ds4vizValue 值
     * @return int64_t 存储的整数值
     * @note debug 模式下若 tag 不匹配则触发 assert
     */
    static inline int64_t
    ds4vizInt(ds4vizValue v)
    {
        assert(v.tag == DS4VIZ_INT);
        return v.i;
    }

    /**
     * @brief 从 ds4vizValue 提取 double
     *
     * @param v ds4vizValue 值
     * @return double 存储的浮点值
     */
    static inline double
    ds4vizFloat(ds4vizValue v)
    {
        assert(v.tag == DS4VIZ_FLOAT);
        return v.f;
    }

    /**
     * @brief 从 ds4vizValue 提取 const char*
     *
     * @param v ds4vizValue 值
     * @return const char* 存储的字符串指针, 生命周期与所属结构作用域相同
     */
    static inline const char *
    ds4vizStr(ds4vizValue v)
    {
        assert(v.tag == DS4VIZ_STR);
        return v.p_s;
    }

    /**
     * @brief 从 ds4vizValue 提取 bool
     *
     * @param v ds4vizValue 值
     * @return bool 存储的布尔值
     */
    static inline bool
    ds4vizBool(ds4vizValue v)
    {
        assert(v.tag == DS4VIZ_BOOL);
        return v.b;
    }

    /* ================================================================
     * 公开函数声明
     * ================================================================ */

    /**
     * @brief 配置全局参数
     *
     * 未指定的字段自动零初始化 (NULL), 使用默认值.
     * 调用 ds4vizConfig 是可选的; 若不调用, 输出路径默认为 "trace.toml".
     *
     * @param options 配置选项
     */
    void ds4vizConfig(ds4vizConfigOptions options);

    /* ================================================================
     * 内部类型定义
     * ================================================================ */

    /**
     * @brief 内部值类型标签
     *
     * @def DS4VIZ_P_VINT_
     * @def DS4VIZ_P_VDBL_
     * @def DS4VIZ_P_VSTR_
     * @def DS4VIZ_P_VBOOL_
     */
    enum
    {
        DS4VIZ_P_VINT_ = 0,
        DS4VIZ_P_VDBL_ = 1,
        DS4VIZ_P_VSTR_ = 2,
        DS4VIZ_P_VBOOL_ = 3
    };

    /**
     * @brief 内部值联合体
     *
     * @struct ds4viz_p_val_s
     * @typedef ds4viz_p_val_t
     */
    typedef struct ds4viz_p_val_s
    {
        /** @brief 值类型标签 */
        int type;
        /** @brief 值联合体 */
        union
        {
            long long i;
            double d;
            const char *p_s;
            bool b;
        };
    } ds4viz_p_val_t;

    /**
     * @brief 动态缓冲区
     *
     * @struct ds4viz_p_buf_s
     * @typedef ds4viz_p_buf_t
     */
    typedef struct ds4viz_p_buf_s
    {
        /** @brief 缓冲区数据指针 */
        char *p_data;
        /** @brief 当前长度 */
        int len;
        /** @brief 容量 */
        int cap;
    } ds4viz_p_buf_t;

    /**
     * @brief 高亮标记
     *
     * @struct ds4viz_p_hl_s
     * @typedef ds4viz_p_hl_t
     */
    typedef struct ds4viz_p_hl_s
    {
        /** @brief 高亮类型: 0=node, 1=item, 2=edge */
        int kind;
        /** @brief 目标节点/元素 id (node/item) */
        int target;
        /** @brief 边起始 (edge) */
        int from;
        /** @brief 边终止 (edge) */
        int to;
        /** @brief 样式名称 */
        const char *p_style;
        /** @brief 视觉强度等级 */
        int level;
    } ds4viz_p_hl_t;

    /**
     * @brief 别名条目
     *
     * @struct ds4viz_p_alias_s
     * @typedef ds4viz_p_alias_t
     */
    typedef struct ds4viz_p_alias_s
    {
        /** @brief 节点 id */
        int id;
        /** @brief 别名字符串 */
        char name[65];
    } ds4viz_p_alias_t;

    /**
     * @brief 待刷新步骤
     *
     * @struct ds4viz_p_pstep_s
     * @typedef ds4viz_p_pstep_t
     */
    typedef struct ds4viz_p_pstep_s
    {
        /** @brief 步骤 id */
        int id;
        /** @brief 操作名称 */
        char op[65];
        /** @brief 操作前状态 id */
        int before;
        /** @brief 操作后状态 id, -1 表示不存在 */
        int after;
        /** @brief 已格式化参数字符串 (堆分配, NULL 表示无参数) */
        char *p_args;
        /** @brief 已格式化返回值字符串 (堆分配, NULL 表示无返回值) */
        char *p_ret_str;
        /** @brief 代码行号 */
        int code_line;
        /** @brief 阶段名称 (堆分配, NULL 表示无 phase) */
        char *p_phase;
        /** @brief 步骤说明 (堆分配, NULL 表示无 note) */
        char *p_note;
        /** @brief 高亮标记数组 (堆分配) */
        ds4viz_p_hl_t *p_hls;
        /** @brief 高亮标记数量 */
        int nhl;
        /** @brief 是否有效 */
        bool valid;
    } ds4viz_p_pstep_t;

    /**
     * @brief 已返回的字符串指针追踪 (用于作用域结束时统一释放)
     *
     * @struct ds4viz_p_strpool_s
     * @typedef ds4viz_p_strpool_t
     */
    typedef struct ds4viz_p_strpool_s
    {
        /** @brief 字符串指针数组 */
        char **p_ptrs;
        /** @brief 当前数量 */
        int cnt;
        /** @brief 容量 */
        int cap;
    } ds4viz_p_strpool_t;

    /**
     * @brief 写状态回调函数指针类型
     *
     * @param p_self 数据结构对象指针
     *
     * @typedef ds4viz_p_ws_fn_t
     */
    typedef void (*ds4viz_p_ws_fn_t)(void *p_self);

/**
 * @brief 阶段栈最大嵌套深度
 *
 * @def DS4VIZ_P_PHASE_DEPTH_MAX_
 */
#define DS4VIZ_P_PHASE_DEPTH_MAX_ 16

/**
 * @brief 所有数据结构共享的公共字段
 *
 * 通过宏嵌入每个结构体的开头, 保证偏移量一致,
 * 使通用函数可通过 offsetof 访问.
 *
 * @def DS4VIZ_P_COMMON_FIELDS_
 */
#define DS4VIZ_P_COMMON_FIELDS_                           \
    ds4viz_p_buf_t states_buf;                            \
    ds4viz_p_buf_t steps_buf;                             \
    int state_id;                                         \
    int step_id;                                          \
    bool err;                                             \
    char errmsg[512];                                     \
    int err_step;                                         \
    int err_last_state;                                   \
    int err_line;                                         \
    bool done;                                            \
    const char *p_label;                                  \
    const char *p_kind;                                   \
    int scope_line;                                       \
    const char *p_phase_stack[DS4VIZ_P_PHASE_DEPTH_MAX_]; \
    int phase_depth;                                      \
    ds4viz_p_alias_t *p_aliases;                          \
    int alias_cnt;                                        \
    int alias_cap;                                        \
    ds4viz_p_pstep_t pending;                             \
    ds4viz_p_ws_fn_t ws_fn;                               \
    ds4viz_p_strpool_t ret_pool;                          \
    int last_user_line

    /* ---- 栈 ---- */

    /**
     * @brief 栈内部结构
     *
     * @struct ds4viz_p_stack_s
     * @typedef ds4viz_p_stack_t
     */
    typedef struct ds4viz_p_stack_s
    {
        DS4VIZ_P_COMMON_FIELDS_;
        /** @brief 元素数组 */
        ds4viz_p_val_t *p_items;
        /** @brief 当前元素数量 */
        int count;
        /** @brief 数组容量 */
        int cap;
    } ds4viz_p_stack_t;

    ds4viz_p_stack_t ds4viz_p_stack_open_(const char *p_label, int line);
    void ds4viz_p_stack_close_(ds4viz_p_stack_t *p_s);
    void ds4viz_p_stack_push_(ds4viz_p_stack_t *p_s, ds4viz_p_val_t v, int line);
    ds4vizValue ds4viz_p_stack_pop_(ds4viz_p_stack_t *p_s, int line);
    void ds4viz_p_stack_clear_(ds4viz_p_stack_t *p_s, int line);

    /* ---- 队列 ---- */

    /**
     * @brief 队列内部结构
     *
     * @struct ds4viz_p_queue_s
     * @typedef ds4viz_p_queue_t
     */
    typedef struct ds4viz_p_queue_s
    {
        DS4VIZ_P_COMMON_FIELDS_;
        /** @brief 元素数组 */
        ds4viz_p_val_t *p_items;
        /** @brief 当前元素数量 */
        int count;
        /** @brief 数组容量 */
        int cap;
    } ds4viz_p_queue_t;

    ds4viz_p_queue_t ds4viz_p_queue_open_(const char *p_label, int line);
    void ds4viz_p_queue_close_(ds4viz_p_queue_t *p_q);
    void ds4viz_p_queue_enqueue_(ds4viz_p_queue_t *p_q, ds4viz_p_val_t v, int line);
    ds4vizValue ds4viz_p_queue_dequeue_(ds4viz_p_queue_t *p_q, int line);
    void ds4viz_p_queue_clear_(ds4viz_p_queue_t *p_q, int line);

    /* ---- 单链表 ---- */

    /**
     * @brief 单链表节点
     *
     * @struct ds4viz_p_slnode_s
     * @typedef ds4viz_p_slnode_t
     */
    typedef struct ds4viz_p_slnode_s
    {
        int id;
        ds4viz_p_val_t value;
        int next;
        bool alive;
    } ds4viz_p_slnode_t;

    /**
     * @brief 单链表内部结构
     *
     * @struct ds4viz_p_slist_s
     * @typedef ds4viz_p_slist_t
     */
    typedef struct ds4viz_p_slist_s
    {
        DS4VIZ_P_COMMON_FIELDS_;
        ds4viz_p_slnode_t *p_nodes;
        int ncnt;
        int ncap;
        int head;
        int nid;
    } ds4viz_p_slist_t;

    ds4viz_p_slist_t ds4viz_p_slist_open_(const char *p_label, int line);
    void ds4viz_p_slist_close_(ds4viz_p_slist_t *p_l);
    int ds4viz_p_slist_insert_head_(ds4viz_p_slist_t *p_l, ds4viz_p_val_t v, int line);
    int ds4viz_p_slist_insert_tail_(ds4viz_p_slist_t *p_l, ds4viz_p_val_t v, int line);
    int ds4viz_p_slist_insert_after_(ds4viz_p_slist_t *p_l, int nid, ds4viz_p_val_t v, int line);
    void ds4viz_p_slist_delete_(ds4viz_p_slist_t *p_l, int nid, int line);
    void ds4viz_p_slist_delete_head_(ds4viz_p_slist_t *p_l, int line);
    void ds4viz_p_slist_reverse_(ds4viz_p_slist_t *p_l, int line);

    /* ---- 双向链表 ---- */

    /**
     * @brief 双向链表节点
     *
     * @struct ds4viz_p_dlnode_s
     * @typedef ds4viz_p_dlnode_t
     */
    typedef struct ds4viz_p_dlnode_s
    {
        int id;
        ds4viz_p_val_t value;
        int prev;
        int next;
        bool alive;
    } ds4viz_p_dlnode_t;

    /**
     * @brief 双向链表内部结构
     *
     * @struct ds4viz_p_dlist_s
     * @typedef ds4viz_p_dlist_t
     */
    typedef struct ds4viz_p_dlist_s
    {
        DS4VIZ_P_COMMON_FIELDS_;
        ds4viz_p_dlnode_t *p_nodes;
        int ncnt;
        int ncap;
        int head;
        int tail;
        int nid;
    } ds4viz_p_dlist_t;

    ds4viz_p_dlist_t ds4viz_p_dlist_open_(const char *p_label, int line);
    void ds4viz_p_dlist_close_(ds4viz_p_dlist_t *p_l);
    int ds4viz_p_dlist_insert_head_(ds4viz_p_dlist_t *p_l, ds4viz_p_val_t v, int line);
    int ds4viz_p_dlist_insert_tail_(ds4viz_p_dlist_t *p_l, ds4viz_p_val_t v, int line);
    int ds4viz_p_dlist_insert_before_(ds4viz_p_dlist_t *p_l, int nid, ds4viz_p_val_t v, int line);
    int ds4viz_p_dlist_insert_after_(ds4viz_p_dlist_t *p_l, int nid, ds4viz_p_val_t v, int line);
    void ds4viz_p_dlist_delete_(ds4viz_p_dlist_t *p_l, int nid, int line);
    void ds4viz_p_dlist_delete_head_(ds4viz_p_dlist_t *p_l, int line);
    void ds4viz_p_dlist_delete_tail_(ds4viz_p_dlist_t *p_l, int line);
    void ds4viz_p_dlist_reverse_(ds4viz_p_dlist_t *p_l, int line);

    /* ---- 二叉树 / BST ---- */

    /**
     * @brief 二叉树节点
     *
     * @struct ds4viz_p_tnode_s
     * @typedef ds4viz_p_tnode_t
     */
    typedef struct ds4viz_p_tnode_s
    {
        int id;
        ds4viz_p_val_t value;
        int left;
        int right;
        bool alive;
    } ds4viz_p_tnode_t;

    /**
     * @brief 二叉树内部结构
     *
     * @struct ds4viz_p_bt_s
     * @typedef ds4viz_p_bt_t
     */
    typedef struct ds4viz_p_bt_s
    {
        DS4VIZ_P_COMMON_FIELDS_;
        ds4viz_p_tnode_t *p_nodes;
        int ncnt;
        int ncap;
        int root;
        int nid;
    } ds4viz_p_bt_t;

    ds4viz_p_bt_t ds4viz_p_bt_open_(const char *p_kind, const char *p_label, int line);
    void ds4viz_p_bt_close_(ds4viz_p_bt_t *p_t);
    int ds4viz_p_bt_insert_root_(ds4viz_p_bt_t *p_t, ds4viz_p_val_t v, int line);
    int ds4viz_p_bt_insert_left_(ds4viz_p_bt_t *p_t, int pid, ds4viz_p_val_t v, int line);
    int ds4viz_p_bt_insert_right_(ds4viz_p_bt_t *p_t, int pid, ds4viz_p_val_t v, int line);
    void ds4viz_p_bt_delete_(ds4viz_p_bt_t *p_t, int nid, int line);
    void ds4viz_p_bt_update_value_(ds4viz_p_bt_t *p_t, int nid, ds4viz_p_val_t v, int line);
    int ds4viz_p_bst_insert_(ds4viz_p_bt_t *p_t, ds4viz_p_val_t v, int line);
    void ds4viz_p_bst_delete_(ds4viz_p_bt_t *p_t, ds4viz_p_val_t v, int line);

    /* ---- 图 ---- */

    /**
     * @brief 图节点
     *
     * @struct ds4viz_p_gnode_s
     * @typedef ds4viz_p_gnode_t
     */
    typedef struct ds4viz_p_gnode_s
    {
        int id;
        char label[33];
        bool alive;
    } ds4viz_p_gnode_t;

    /**
     * @brief 图边
     *
     * @struct ds4viz_p_gedge_s
     * @typedef ds4viz_p_gedge_t
     */
    typedef struct ds4viz_p_gedge_s
    {
        int from;
        int to;
        double weight;
        bool alive;
    } ds4viz_p_gedge_t;

    /**
     * @brief 图内部结构
     *
     * @struct ds4viz_p_graph_s
     * @typedef ds4viz_p_graph_t
     */
    typedef struct ds4viz_p_graph_s
    {
        DS4VIZ_P_COMMON_FIELDS_;
        ds4viz_p_gnode_t *p_gnodes;
        int ncnt;
        int ncap;
        ds4viz_p_gedge_t *p_edges;
        int ecnt;
        int ecap;
        bool directed;
        bool weighted;
    } ds4viz_p_graph_t;

    ds4viz_p_graph_t ds4viz_p_graph_open_(const char *p_kind, const char *p_label,
                                          bool directed, bool weighted, int line);
    void ds4viz_p_graph_close_(ds4viz_p_graph_t *p_g);
    void ds4viz_p_graph_add_node_(ds4viz_p_graph_t *p_g, int nid, const char *p_lbl, int line);
    void ds4viz_p_graph_add_edge_(ds4viz_p_graph_t *p_g, int from, int to, double w, int line);
    void ds4viz_p_graph_remove_node_(ds4viz_p_graph_t *p_g, int nid, int line);
    void ds4viz_p_graph_remove_edge_(ds4viz_p_graph_t *p_g, int from, int to, int line);
    void ds4viz_p_graph_update_node_label_(ds4viz_p_graph_t *p_g, int nid,
                                           const char *p_lbl, int line);
    void ds4viz_p_graph_update_weight_(ds4viz_p_graph_t *p_g, int from, int to,
                                       double w, int line);

    /* ---- 通用 step/amend/phase/alias ---- */
    void ds4viz_p_step_fn_(void *p_obj, const char *p_note,
                           const ds4viz_p_hl_t *p_hls, int nhl, int line);
    void ds4viz_p_amend_fn_(void *p_obj, const char *p_note,
                            const ds4viz_p_hl_t *p_hls, int nhl, int line);
    void ds4viz_p_amend_hl_fn_(void *p_obj, const ds4viz_p_hl_t *p_hls, int nhl, int line);
    void ds4viz_p_amend_clear_hl_fn_(void *p_obj, int line);
    void ds4viz_p_phase_push_(void *p_obj, const char *p_name);
    void ds4viz_p_phase_pop_(void *p_obj);
    void ds4viz_p_alias_fn_(void *p_obj, int nid, const char *p_name, int line);

    /* ================================================================
     * 值转换内联函数与宏
     * ================================================================ */

    /**
     * @brief 布尔值转内部值
     */
    static inline ds4viz_p_val_t
    ds4viz_p_mb_(bool v)
    {
        return (ds4viz_p_val_t){DS4VIZ_P_VBOOL_, .b = v};
    }

    /**
     * @brief 有符号整数转内部值
     */
    static inline ds4viz_p_val_t
    ds4viz_p_mi_(long long v)
    {
        return (ds4viz_p_val_t){DS4VIZ_P_VINT_, .i = v};
    }

    /**
     * @brief 无符号整数转内部值
     */
    static inline ds4viz_p_val_t
    ds4viz_p_mu_(unsigned long long v)
    {
        return (ds4viz_p_val_t){DS4VIZ_P_VINT_, .i = (long long)v};
    }

    /**
     * @brief 浮点数转内部值
     */
    static inline ds4viz_p_val_t
    ds4viz_p_md_(double v)
    {
        return (ds4viz_p_val_t){DS4VIZ_P_VDBL_, .d = v};
    }

    /**
     * @brief 字符串转内部值
     */
    static inline ds4viz_p_val_t
    ds4viz_p_ms_(const char *p_v)
    {
        return (ds4viz_p_val_t){DS4VIZ_P_VSTR_, .p_s = p_v};
    }

/**
 * @brief 通用类型 -> 内部值 (支持全部类型)
 *
 * @param x 任意受支持的 C 标量值
 *
 * @def DS4VIZ_P_VAL_
 */
#define DS4VIZ_P_VAL_(x) _Generic((x), \
    bool: ds4viz_p_mb_,                \
    char: ds4viz_p_mi_,                \
    signed char: ds4viz_p_mi_,         \
    unsigned char: ds4viz_p_mi_,       \
    short: ds4viz_p_mi_,               \
    unsigned short: ds4viz_p_mi_,      \
    int: ds4viz_p_mi_,                 \
    unsigned int: ds4viz_p_mi_,        \
    long: ds4viz_p_mi_,                \
    unsigned long: ds4viz_p_mi_,       \
    long long: ds4viz_p_mi_,           \
    unsigned long long: ds4viz_p_mu_,  \
    float: ds4viz_p_md_,               \
    double: ds4viz_p_md_,              \
    char *: ds4viz_p_ms_,              \
    const char *: ds4viz_p_ms_)(x)

/**
 * @brief 数值类型 -> 内部值 (仅整数/浮点)
 *
 * @param x 整数或浮点值
 *
 * @def DS4VIZ_P_NUMVAL_
 */
#define DS4VIZ_P_NUMVAL_(x) _Generic((x), \
    char: ds4viz_p_mi_,                   \
    signed char: ds4viz_p_mi_,            \
    unsigned char: ds4viz_p_mi_,          \
    short: ds4viz_p_mi_,                  \
    unsigned short: ds4viz_p_mi_,         \
    int: ds4viz_p_mi_,                    \
    unsigned int: ds4viz_p_mi_,           \
    long: ds4viz_p_mi_,                   \
    unsigned long: ds4viz_p_mi_,          \
    long long: ds4viz_p_mi_,              \
    unsigned long long: ds4viz_p_mu_,     \
    float: ds4viz_p_md_,                  \
    double: ds4viz_p_md_)(x)

/* ================================================================
 * 可选参数 + 参数计数宏
 * ================================================================ */

/**
 * @brief 从 __VA_ARGS__ 中取第一个参数, 若无则使用 def
 *
 * @def DS4VIZ_P_OPT1_
 */
#define DS4VIZ_P_OPT1_(def, ...) DS4VIZ_P_PICK1_(__VA_ARGS__ __VA_OPT__(, ) def)
#define DS4VIZ_P_PICK1_(a, ...) a

/**
 * @brief 从 __VA_ARGS__ 中取第二个参数, 若无则使用 def
 *
 * @def DS4VIZ_P_OPT2_
 */
#define DS4VIZ_P_OPT2_(def, ...) DS4VIZ_P_PICK2_(__VA_ARGS__ __VA_OPT__(, ) def, def)
#define DS4VIZ_P_PICK2_(a, b, ...) b

/**
 * @brief 标记粘贴辅助
 *
 * @def DS4VIZ_P_CAT_
 */
#define DS4VIZ_P_CAT_(a, b) DS4VIZ_P_CAT2_(a, b)
#define DS4VIZ_P_CAT2_(a, b) a##b

/**
 * @brief 计算可变参数个数 (0..16)
 *
 * @def DS4VIZ_P_CNTX_
 */
#define DS4VIZ_P_CNTX_(...) \
    DS4VIZ_P_CNTX_I_(__VA_ARGS__ __VA_OPT__(, ) 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)
#define DS4VIZ_P_CNTX_I_(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, N, ...) N

    /* ================================================================
     * 高亮标记构造宏与辅助内联函数
     * ================================================================ */

#define DS4VIZ_P_SEL3_(p, _1, _2, _3, N, ...) DS4VIZ_P_CAT_(DS4VIZ_P_CAT_(p, N), _)
#define DS4VIZ_P_SEL4_(p, _1, _2, _3, _4, N, ...) DS4VIZ_P_CAT_(DS4VIZ_P_CAT_(p, N), _)

    static inline ds4viz_p_hl_t ds4viz_p_n1_(int t) { return (ds4viz_p_hl_t){0, t, 0, 0, "focus", 1}; }
    static inline ds4viz_p_hl_t ds4viz_p_n2_(int t, const char *p_s) { return (ds4viz_p_hl_t){0, t, 0, 0, p_s, 1}; }
    static inline ds4viz_p_hl_t ds4viz_p_n3_(int t, const char *p_s, int l) { return (ds4viz_p_hl_t){0, t, 0, 0, p_s, l}; }
    static inline ds4viz_p_hl_t ds4viz_p_i1_(int t) { return (ds4viz_p_hl_t){1, t, 0, 0, "focus", 1}; }
    static inline ds4viz_p_hl_t ds4viz_p_i2_(int t, const char *p_s) { return (ds4viz_p_hl_t){1, t, 0, 0, p_s, 1}; }
    static inline ds4viz_p_hl_t ds4viz_p_i3_(int t, const char *p_s, int l) { return (ds4viz_p_hl_t){1, t, 0, 0, p_s, l}; }
    static inline ds4viz_p_hl_t ds4viz_p_e2_(int f, int t) { return (ds4viz_p_hl_t){2, 0, f, t, "focus", 1}; }
    static inline ds4viz_p_hl_t ds4viz_p_e3_(int f, int t, const char *p_s) { return (ds4viz_p_hl_t){2, 0, f, t, p_s, 1}; }
    static inline ds4viz_p_hl_t ds4viz_p_e4_(int f, int t, const char *p_s, int l) { return (ds4viz_p_hl_t){2, 0, f, t, p_s, l}; }

/**
 * @brief 创建节点高亮标记
 *
 * @param target 节点 id
 * @param style 高亮样式 (可选, 默认 "focus")
 * @param level 视觉强度 (可选, 默认 1)
 */
#define ds4vizNode(...) DS4VIZ_P_SEL3_(ds4viz_p_n, __VA_ARGS__, 3, 2, 1)(__VA_ARGS__)

/**
 * @brief 创建元素高亮标记
 *
 * @param target 元素索引
 * @param style 高亮样式 (可选, 默认 "focus")
 * @param level 视觉强度 (可选, 默认 1)
 */
#define ds4vizItem(...) DS4VIZ_P_SEL3_(ds4viz_p_i, __VA_ARGS__, 3, 2, 1)(__VA_ARGS__)

/**
 * @brief 创建边高亮标记
 *
 * @param from_id 起始节点 id
 * @param to_id 终止节点 id
 * @param style 高亮样式 (可选, 默认 "focus")
 * @param level 视觉强度 (可选, 默认 1)
 */
#define ds4vizEdge(...) DS4VIZ_P_SEL4_(ds4viz_p_e, __VA_ARGS__, 4, 3, 2, 1)(__VA_ARGS__)

/* ================================================================
 * ds4vizStep 宏 (0..16 个额外参数)
 * ================================================================ */

/**
 * @brief 记录一次观察步骤
 *
 * 不改变数据结构状态, 在 IR 中生成 op = "observe", before == after 的 step.
 *
 * @param obj 数据结构对象
 * @param note 步骤说明 (可选)
 * @param ... 高亮标记 (可选)
 */
#define ds4vizStep(obj, ...) \
    DS4VIZ_P_CAT_(DS4VIZ_P_STP_, DS4VIZ_P_CNTX_(__VA_ARGS__))(obj __VA_OPT__(, ) __VA_ARGS__)

#define DS4VIZ_P_STP_0(obj) ds4viz_p_step_fn_((void *)&(obj), NULL, NULL, 0, __LINE__)
#define DS4VIZ_P_STP_1(obj, note) ds4viz_p_step_fn_((void *)&(obj), (note), NULL, 0, __LINE__)
#define DS4VIZ_P_STP_HL_(obj, note, ...)                    \
    ds4viz_p_step_fn_((void *)&(obj), (note),               \
                      (const ds4viz_p_hl_t[]){__VA_ARGS__}, \
                      (int)(sizeof((const ds4viz_p_hl_t[]){__VA_ARGS__}) / sizeof(ds4viz_p_hl_t)), __LINE__)
#define DS4VIZ_P_STP_2(obj, note, ...) DS4VIZ_P_STP_HL_(obj, note, __VA_ARGS__)
#define DS4VIZ_P_STP_3(obj, note, ...) DS4VIZ_P_STP_HL_(obj, note, __VA_ARGS__)
#define DS4VIZ_P_STP_4(obj, note, ...) DS4VIZ_P_STP_HL_(obj, note, __VA_ARGS__)
#define DS4VIZ_P_STP_5(obj, note, ...) DS4VIZ_P_STP_HL_(obj, note, __VA_ARGS__)
#define DS4VIZ_P_STP_6(obj, note, ...) DS4VIZ_P_STP_HL_(obj, note, __VA_ARGS__)
#define DS4VIZ_P_STP_7(obj, note, ...) DS4VIZ_P_STP_HL_(obj, note, __VA_ARGS__)
#define DS4VIZ_P_STP_8(obj, note, ...) DS4VIZ_P_STP_HL_(obj, note, __VA_ARGS__)
#define DS4VIZ_P_STP_9(obj, note, ...) DS4VIZ_P_STP_HL_(obj, note, __VA_ARGS__)
#define DS4VIZ_P_STP_10(obj, note, ...) DS4VIZ_P_STP_HL_(obj, note, __VA_ARGS__)
#define DS4VIZ_P_STP_11(obj, note, ...) DS4VIZ_P_STP_HL_(obj, note, __VA_ARGS__)
#define DS4VIZ_P_STP_12(obj, note, ...) DS4VIZ_P_STP_HL_(obj, note, __VA_ARGS__)
#define DS4VIZ_P_STP_13(obj, note, ...) DS4VIZ_P_STP_HL_(obj, note, __VA_ARGS__)
#define DS4VIZ_P_STP_14(obj, note, ...) DS4VIZ_P_STP_HL_(obj, note, __VA_ARGS__)
#define DS4VIZ_P_STP_15(obj, note, ...) DS4VIZ_P_STP_HL_(obj, note, __VA_ARGS__)
#define DS4VIZ_P_STP_16(obj, note, ...) DS4VIZ_P_STP_HL_(obj, note, __VA_ARGS__)

/* ================================================================
 * ds4vizStepAt 宏 (显式代码行号)
 * ================================================================ */

/**
 * @brief 记录一次观察步骤 (显式指定代码行号)
 *
 * 不改变数据结构状态, 在 IR 中生成 op = "observe", before == after 的 step.
 *
 * @param obj 数据结构对象
 * @param line 步骤对应源码行号
 * @param note 步骤说明 (可选)
 * @param ... 高亮标记 (可选)
 */
#define ds4vizStepAt(obj, line, ...) \
    DS4VIZ_P_CAT_(DS4VIZ_P_STPA_, DS4VIZ_P_CNTX_(__VA_ARGS__))(obj, line __VA_OPT__(, ) __VA_ARGS__)

#define DS4VIZ_P_STPA_0(obj, line) ds4viz_p_step_fn_((void *)&(obj), NULL, NULL, 0, (line))
#define DS4VIZ_P_STPA_1(obj, line, note) ds4viz_p_step_fn_((void *)&(obj), (note), NULL, 0, (line))
#define DS4VIZ_P_STPA_HL_(obj, line, note, ...)             \
    ds4viz_p_step_fn_((void *)&(obj), (note),               \
                      (const ds4viz_p_hl_t[]){__VA_ARGS__}, \
                      (int)(sizeof((const ds4viz_p_hl_t[]){__VA_ARGS__}) / sizeof(ds4viz_p_hl_t)), (line))
#define DS4VIZ_P_STPA_2(obj, line, note, ...) DS4VIZ_P_STPA_HL_(obj, line, note, __VA_ARGS__)
#define DS4VIZ_P_STPA_3(obj, line, note, ...) DS4VIZ_P_STPA_HL_(obj, line, note, __VA_ARGS__)
#define DS4VIZ_P_STPA_4(obj, line, note, ...) DS4VIZ_P_STPA_HL_(obj, line, note, __VA_ARGS__)
#define DS4VIZ_P_STPA_5(obj, line, note, ...) DS4VIZ_P_STPA_HL_(obj, line, note, __VA_ARGS__)
#define DS4VIZ_P_STPA_6(obj, line, note, ...) DS4VIZ_P_STPA_HL_(obj, line, note, __VA_ARGS__)
#define DS4VIZ_P_STPA_7(obj, line, note, ...) DS4VIZ_P_STPA_HL_(obj, line, note, __VA_ARGS__)
#define DS4VIZ_P_STPA_8(obj, line, note, ...) DS4VIZ_P_STPA_HL_(obj, line, note, __VA_ARGS__)
#define DS4VIZ_P_STPA_9(obj, line, note, ...) DS4VIZ_P_STPA_HL_(obj, line, note, __VA_ARGS__)
#define DS4VIZ_P_STPA_10(obj, line, note, ...) DS4VIZ_P_STPA_HL_(obj, line, note, __VA_ARGS__)
#define DS4VIZ_P_STPA_11(obj, line, note, ...) DS4VIZ_P_STPA_HL_(obj, line, note, __VA_ARGS__)
#define DS4VIZ_P_STPA_12(obj, line, note, ...) DS4VIZ_P_STPA_HL_(obj, line, note, __VA_ARGS__)
#define DS4VIZ_P_STPA_13(obj, line, note, ...) DS4VIZ_P_STPA_HL_(obj, line, note, __VA_ARGS__)
#define DS4VIZ_P_STPA_14(obj, line, note, ...) DS4VIZ_P_STPA_HL_(obj, line, note, __VA_ARGS__)
#define DS4VIZ_P_STPA_15(obj, line, note, ...) DS4VIZ_P_STPA_HL_(obj, line, note, __VA_ARGS__)
#define DS4VIZ_P_STPA_16(obj, line, note, ...) DS4VIZ_P_STPA_HL_(obj, line, note, __VA_ARGS__)

/* ================================================================
 * ds4vizAmend 宏
 * ================================================================ */

/**
 * @brief 修改上一步的 note, 可同时设置 highlights
 *
 * @param obj 数据结构对象
 * @param note 步骤说明
 * @param ... 高亮标记 (可选)
 */
#define ds4vizAmend(obj, note, ...) \
    DS4VIZ_P_CAT_(DS4VIZ_P_AMD_, DS4VIZ_P_CNTX_(__VA_ARGS__))(obj, note __VA_OPT__(, ) __VA_ARGS__)

#define DS4VIZ_P_AMD_0(obj, note) ds4viz_p_amend_fn_((void *)&(obj), (note), NULL, 0, __LINE__)
#define DS4VIZ_P_AMD_HL_(obj, note, ...)                     \
    ds4viz_p_amend_fn_((void *)&(obj), (note),               \
                       (const ds4viz_p_hl_t[]){__VA_ARGS__}, \
                       (int)(sizeof((const ds4viz_p_hl_t[]){__VA_ARGS__}) / sizeof(ds4viz_p_hl_t)), __LINE__)
#define DS4VIZ_P_AMD_1(obj, note, ...) DS4VIZ_P_AMD_HL_(obj, note, __VA_ARGS__)
#define DS4VIZ_P_AMD_2(obj, note, ...) DS4VIZ_P_AMD_HL_(obj, note, __VA_ARGS__)
#define DS4VIZ_P_AMD_3(obj, note, ...) DS4VIZ_P_AMD_HL_(obj, note, __VA_ARGS__)
#define DS4VIZ_P_AMD_4(obj, note, ...) DS4VIZ_P_AMD_HL_(obj, note, __VA_ARGS__)
#define DS4VIZ_P_AMD_5(obj, note, ...) DS4VIZ_P_AMD_HL_(obj, note, __VA_ARGS__)
#define DS4VIZ_P_AMD_6(obj, note, ...) DS4VIZ_P_AMD_HL_(obj, note, __VA_ARGS__)
#define DS4VIZ_P_AMD_7(obj, note, ...) DS4VIZ_P_AMD_HL_(obj, note, __VA_ARGS__)
#define DS4VIZ_P_AMD_8(obj, note, ...) DS4VIZ_P_AMD_HL_(obj, note, __VA_ARGS__)
#define DS4VIZ_P_AMD_9(obj, note, ...) DS4VIZ_P_AMD_HL_(obj, note, __VA_ARGS__)
#define DS4VIZ_P_AMD_10(obj, note, ...) DS4VIZ_P_AMD_HL_(obj, note, __VA_ARGS__)
#define DS4VIZ_P_AMD_11(obj, note, ...) DS4VIZ_P_AMD_HL_(obj, note, __VA_ARGS__)
#define DS4VIZ_P_AMD_12(obj, note, ...) DS4VIZ_P_AMD_HL_(obj, note, __VA_ARGS__)
#define DS4VIZ_P_AMD_13(obj, note, ...) DS4VIZ_P_AMD_HL_(obj, note, __VA_ARGS__)
#define DS4VIZ_P_AMD_14(obj, note, ...) DS4VIZ_P_AMD_HL_(obj, note, __VA_ARGS__)
#define DS4VIZ_P_AMD_15(obj, note, ...) DS4VIZ_P_AMD_HL_(obj, note, __VA_ARGS__)
#define DS4VIZ_P_AMD_16(obj, note, ...) DS4VIZ_P_AMD_HL_(obj, note, __VA_ARGS__)

/* ================================================================
 * ds4vizAmendHL / ds4vizAmendClearHL 宏
 * ================================================================ */

/**
 * @brief 仅修改上一步的 highlights
 *
 * @param obj 数据结构对象
 * @param ... 高亮标记
 */
#define ds4vizAmendHL(obj, ...)                                 \
    ds4viz_p_amend_hl_fn_((void *)&(obj),                       \
                          (const ds4viz_p_hl_t[]){__VA_ARGS__}, \
                          (int)(sizeof((const ds4viz_p_hl_t[]){__VA_ARGS__}) / sizeof(ds4viz_p_hl_t)), __LINE__)

/**
 * @brief 清除上一步的所有高亮标记
 *
 * @param obj 数据结构对象
 */
#define ds4vizAmendClearHL(obj) ds4viz_p_amend_clear_hl_fn_((void *)&(obj), __LINE__)

/* ================================================================
 * ds4vizPhase 宏 (支持嵌套, 内层优先)
 * ================================================================ */

/**
 * @brief 阶段作用域块
 *
 * 块内产生的所有步骤都会标记为指定阶段.
 * 支持嵌套, 内层 phase 优先.
 *
 * @param obj 数据结构对象
 * @param name 阶段名称
 */
#define ds4vizPhase(obj, name)                                                                 \
    for (int DS4VIZ_P_CAT_(pf_, __LINE__) = (ds4viz_p_phase_push_((void *)&(obj), (name)), 0); \
         !DS4VIZ_P_CAT_(pf_, __LINE__);                                                        \
         DS4VIZ_P_CAT_(pf_, __LINE__) = 1, ds4viz_p_phase_pop_((void *)&(obj)))

/* ================================================================
 * ds4vizAlias 宏
 * ================================================================ */

/**
 * @brief 设置或清除节点别名
 *
 * @param obj 数据结构对象
 * @param nid 节点 id
 * @param name 别名, NULL 表示清除
 */
#define ds4vizAlias(obj, nid, name) ds4viz_p_alias_fn_((void *)&(obj), (nid), (name), __LINE__)

/* ================================================================
 * 结构作用域宏
 * ================================================================ */

/**
 * @brief 创建栈实例
 *
 * @param s 栈对象的变量名
 * @param ... 可选 label (默认 "stack")
 */
#define ds4vizStack(s, ...)                                                 \
    for (ds4viz_p_stack_t s = ds4viz_p_stack_open_(                         \
             DS4VIZ_P_OPT1_("stack" __VA_OPT__(, ) __VA_ARGS__), __LINE__); \
         !s.done; ds4viz_p_stack_close_(&s))

#define ds4vizStackPush(s, v) ds4viz_p_stack_push_(&(s), DS4VIZ_P_VAL_(v), __LINE__)
#define ds4vizStackPop(s) ds4viz_p_stack_pop_(&(s), __LINE__)
#define ds4vizStackClear(s) ds4viz_p_stack_clear_(&(s), __LINE__)

/**
 * @brief 创建队列实例
 *
 * @param q 队列对象的变量名
 * @param ... 可选 label (默认 "queue")
 */
#define ds4vizQueue(q, ...)                                                 \
    for (ds4viz_p_queue_t q = ds4viz_p_queue_open_(                         \
             DS4VIZ_P_OPT1_("queue" __VA_OPT__(, ) __VA_ARGS__), __LINE__); \
         !q.done; ds4viz_p_queue_close_(&q))

#define ds4vizQueueEnqueue(q, v) ds4viz_p_queue_enqueue_(&(q), DS4VIZ_P_VAL_(v), __LINE__)
#define ds4vizQueueDequeue(q) ds4viz_p_queue_dequeue_(&(q), __LINE__)
#define ds4vizQueueClear(q) ds4viz_p_queue_clear_(&(q), __LINE__)

/**
 * @brief 创建单链表实例
 *
 * @param l 单链表对象的变量名
 * @param ... 可选 label (默认 "slist")
 */
#define ds4vizSingleLinkedList(l, ...)                                      \
    for (ds4viz_p_slist_t l = ds4viz_p_slist_open_(                         \
             DS4VIZ_P_OPT1_("slist" __VA_OPT__(, ) __VA_ARGS__), __LINE__); \
         !l.done; ds4viz_p_slist_close_(&l))

#define ds4vizSlInsertHead(l, v) ds4viz_p_slist_insert_head_(&(l), DS4VIZ_P_VAL_(v), __LINE__)
#define ds4vizSlInsertTail(l, v) ds4viz_p_slist_insert_tail_(&(l), DS4VIZ_P_VAL_(v), __LINE__)
#define ds4vizSlInsertAfter(l, nid, v) ds4viz_p_slist_insert_after_(&(l), (nid), DS4VIZ_P_VAL_(v), __LINE__)
#define ds4vizSlDelete(l, nid) ds4viz_p_slist_delete_(&(l), (nid), __LINE__)
#define ds4vizSlDeleteHead(l) ds4viz_p_slist_delete_head_(&(l), __LINE__)
#define ds4vizSlReverse(l) ds4viz_p_slist_reverse_(&(l), __LINE__)

/**
 * @brief 创建双向链表实例
 *
 * @param l 双向链表对象的变量名
 * @param ... 可选 label (默认 "dlist")
 */
#define ds4vizDoubleLinkedList(l, ...)                                      \
    for (ds4viz_p_dlist_t l = ds4viz_p_dlist_open_(                         \
             DS4VIZ_P_OPT1_("dlist" __VA_OPT__(, ) __VA_ARGS__), __LINE__); \
         !l.done; ds4viz_p_dlist_close_(&l))

#define ds4vizDlInsertHead(l, v) ds4viz_p_dlist_insert_head_(&(l), DS4VIZ_P_VAL_(v), __LINE__)
#define ds4vizDlInsertTail(l, v) ds4viz_p_dlist_insert_tail_(&(l), DS4VIZ_P_VAL_(v), __LINE__)
#define ds4vizDlInsertBefore(l, nid, v) ds4viz_p_dlist_insert_before_(&(l), (nid), DS4VIZ_P_VAL_(v), __LINE__)
#define ds4vizDlInsertAfter(l, nid, v) ds4viz_p_dlist_insert_after_(&(l), (nid), DS4VIZ_P_VAL_(v), __LINE__)
#define ds4vizDlDelete(l, nid) ds4viz_p_dlist_delete_(&(l), (nid), __LINE__)
#define ds4vizDlDeleteHead(l) ds4viz_p_dlist_delete_head_(&(l), __LINE__)
#define ds4vizDlDeleteTail(l) ds4viz_p_dlist_delete_tail_(&(l), __LINE__)
#define ds4vizDlReverse(l) ds4viz_p_dlist_reverse_(&(l), __LINE__)

/**
 * @brief 创建二叉树实例
 *
 * @param t 二叉树对象的变量名
 * @param ... 可选 label (默认 "binary_tree")
 */
#define ds4vizBinaryTree(t, ...)                                                                       \
    for (ds4viz_p_bt_t t = ds4viz_p_bt_open_("binary_tree",                                            \
                                             DS4VIZ_P_OPT1_("binary_tree" __VA_OPT__(, ) __VA_ARGS__), \
                                             __LINE__);                                                \
         !t.done; ds4viz_p_bt_close_(&t))

#define ds4vizBtInsertRoot(t, v) ds4viz_p_bt_insert_root_(&(t), DS4VIZ_P_VAL_(v), __LINE__)
#define ds4vizBtInsertLeft(t, pid, v) ds4viz_p_bt_insert_left_(&(t), (pid), DS4VIZ_P_VAL_(v), __LINE__)
#define ds4vizBtInsertRight(t, pid, v) ds4viz_p_bt_insert_right_(&(t), (pid), DS4VIZ_P_VAL_(v), __LINE__)
#define ds4vizBtDelete(t, nid) ds4viz_p_bt_delete_(&(t), (nid), __LINE__)
#define ds4vizBtUpdateValue(t, nid, v) ds4viz_p_bt_update_value_(&(t), (nid), DS4VIZ_P_VAL_(v), __LINE__)

/**
 * @brief 创建二叉搜索树实例
 *
 * @param b 二叉搜索树对象的变量名
 * @param ... 可选 label (默认 "bst")
 */
#define ds4vizBinarySearchTree(b, ...)                                                                    \
    for (ds4viz_p_bt_t b = ds4viz_p_bt_open_("bst",                                                       \
                                             DS4VIZ_P_OPT1_("bst" __VA_OPT__(, ) __VA_ARGS__), __LINE__); \
         !b.done; ds4viz_p_bt_close_(&b))

#define ds4vizBstInsert(b, v) ds4viz_p_bst_insert_(&(b), DS4VIZ_P_NUMVAL_(v), __LINE__)
#define ds4vizBstDelete(b, v) ds4viz_p_bst_delete_(&(b), DS4VIZ_P_NUMVAL_(v), __LINE__)

/**
 * @brief 创建无向图实例
 *
 * @param g 无向图对象的变量名
 * @param ... 可选 label (默认 "graph")
 */
#define ds4vizGraphUndirected(g, ...)                                                                  \
    for (ds4viz_p_graph_t g = ds4viz_p_graph_open_("graph_undirected",                                 \
                                                   DS4VIZ_P_OPT1_("graph" __VA_OPT__(, ) __VA_ARGS__), \
                                                   false, false, __LINE__);                            \
         !g.done; ds4viz_p_graph_close_(&g))

#define ds4vizGuAddNode(g, nid, lbl) ds4viz_p_graph_add_node_(&(g), (nid), (lbl), __LINE__)
#define ds4vizGuAddEdge(g, f, t) ds4viz_p_graph_add_edge_(&(g), (f), (t), 0.0, __LINE__)
#define ds4vizGuRemoveNode(g, nid) ds4viz_p_graph_remove_node_(&(g), (nid), __LINE__)
#define ds4vizGuRemoveEdge(g, f, t) ds4viz_p_graph_remove_edge_(&(g), (f), (t), __LINE__)
#define ds4vizGuUpdateNodeLabel(g, n, l) ds4viz_p_graph_update_node_label_(&(g), (n), (l), __LINE__)

/**
 * @brief 创建有向图实例
 *
 * @param g 有向图对象的变量名
 * @param ... 可选 label (默认 "graph")
 */
#define ds4vizGraphDirected(g, ...)                                                                    \
    for (ds4viz_p_graph_t g = ds4viz_p_graph_open_("graph_directed",                                   \
                                                   DS4VIZ_P_OPT1_("graph" __VA_OPT__(, ) __VA_ARGS__), \
                                                   true, false, __LINE__);                             \
         !g.done; ds4viz_p_graph_close_(&g))

#define ds4vizGdAddNode(g, nid, lbl) ds4viz_p_graph_add_node_(&(g), (nid), (lbl), __LINE__)
#define ds4vizGdAddEdge(g, f, t) ds4viz_p_graph_add_edge_(&(g), (f), (t), 0.0, __LINE__)
#define ds4vizGdRemoveNode(g, nid) ds4viz_p_graph_remove_node_(&(g), (nid), __LINE__)
#define ds4vizGdRemoveEdge(g, f, t) ds4viz_p_graph_remove_edge_(&(g), (f), (t), __LINE__)
#define ds4vizGdUpdateNodeLabel(g, n, l) ds4viz_p_graph_update_node_label_(&(g), (n), (l), __LINE__)

/**
 * @brief 创建带权图实例
 *
 * 默认为有向图. 若需要无向, 使用 ds4vizGraphUndirected 并为边手动指定权重.
 *
 * @param g 带权图对象的变量名
 * @param ... 可选 label (默认 "graph")
 */
#define ds4vizGraphWeighted(g, ...)                                                                    \
    for (ds4viz_p_graph_t g = ds4viz_p_graph_open_("graph_weighted",                                   \
                                                   DS4VIZ_P_OPT1_("graph" __VA_OPT__(, ) __VA_ARGS__), \
                                                   true, true, __LINE__);                              \
         !g.done; ds4viz_p_graph_close_(&g))

#define ds4vizGwAddNode(g, nid, lbl) ds4viz_p_graph_add_node_(&(g), (nid), (lbl), __LINE__)
#define ds4vizGwAddEdge(g, f, t, w) ds4viz_p_graph_add_edge_(&(g), (f), (t), (double)(w), __LINE__)
#define ds4vizGwRemoveNode(g, nid) ds4viz_p_graph_remove_node_(&(g), (nid), __LINE__)
#define ds4vizGwRemoveEdge(g, f, t) ds4viz_p_graph_remove_edge_(&(g), (f), (t), __LINE__)
#define ds4vizGwUpdateWeight(g, f, t, w) ds4viz_p_graph_update_weight_(&(g), (f), (t), (double)(w), __LINE__)
#define ds4vizGwUpdateNodeLabel(g, n, l) ds4viz_p_graph_update_node_label_(&(g), (n), (l), __LINE__)

/* ================================================================
 * 短名映射
 * ================================================================ */
#ifdef DS4VIZ_SHORT_NAMES
#define dvConfigOptions ds4vizConfigOptions
#define dvValue ds4vizValue
#define DV_INT DS4VIZ_INT
#define DV_FLOAT DS4VIZ_FLOAT
#define DV_STR DS4VIZ_STR
#define DV_BOOL DS4VIZ_BOOL
#define dvInt ds4vizInt
#define dvFloat ds4vizFloat
#define dvStr ds4vizStr
#define dvBool ds4vizBool
#define DV_FOCUS DS4VIZ_FOCUS
#define DV_VISITED DS4VIZ_VISITED
#define DV_ACTIVE DS4VIZ_ACTIVE
#define DV_COMPARING DS4VIZ_COMPARING
#define DV_FOUND DS4VIZ_FOUND
#define DV_ERROR_STY DS4VIZ_ERROR_STY
#define dvConfig ds4vizConfig
#define dvNode ds4vizNode
#define dvItem ds4vizItem
#define dvEdge ds4vizEdge
#define dvStep ds4vizStep
#define dvStepAt ds4vizStepAt
#define dvAmend ds4vizAmend
#define dvAmendHL ds4vizAmendHL
#define dvAmendClearHL ds4vizAmendClearHL
#define dvPhase ds4vizPhase
#define dvAlias ds4vizAlias
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
 *                      实现部分
 * ================================================================ */
#ifdef DS4VIZ_IMPLEMENTATION

/**
 * @brief 库版本号
 *
 * @def DS4VIZ_P_VERSION_
 */
#define DS4VIZ_P_VERSION_ "0.1.0"

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
} ds4viz_p_g_ = {0};

void ds4vizConfig(ds4vizConfigOptions o)
{
    ds4viz_p_g_.output_path = o.output_path;
    ds4viz_p_g_.title = o.title;
    ds4viz_p_g_.author = o.author;
    ds4viz_p_g_.comment = o.comment;
}

/**
 * @brief 获取输出路径, 未配置时返回默认值
 *
 * @return const char* 输出文件路径
 */
static const char *
ds4viz_p_outpath_(void)
{
    return ds4viz_p_g_.output_path ? ds4viz_p_g_.output_path : "trace.toml";
}

/* ----------------------------------------------------------------
 * 缓冲区操作
 * ---------------------------------------------------------------- */

/**
 * @brief 确保缓冲区有足够空间
 */
static void
ds4viz_p_bg_(ds4viz_p_buf_t *p_b, int need)
{
    int nc;

    if (p_b->len + need + 1 <= p_b->cap)
    {
        return;
    }
    nc = p_b->cap ? p_b->cap * 2 : 256;
    while (nc < p_b->len + need + 1)
    {
        nc *= 2;
    }
    p_b->p_data = (char *)realloc(p_b->p_data, (size_t)nc);
    p_b->cap = nc;
}

/**
 * @brief 追加单个字符
 */
static void
ds4viz_p_bc_(ds4viz_p_buf_t *p_b, char c)
{
    ds4viz_p_bg_(p_b, 1);
    p_b->p_data[p_b->len++] = c;
    p_b->p_data[p_b->len] = 0;
}

/**
 * @brief 追加字符串
 */
static void
ds4viz_p_bs_(ds4viz_p_buf_t *p_b, const char *p_s)
{
    int n = (int)strlen(p_s);

    ds4viz_p_bg_(p_b, n);
    memcpy(p_b->p_data + p_b->len, p_s, (size_t)n);
    p_b->len += n;
    p_b->p_data[p_b->len] = 0;
}

/**
 * @brief 格式化追加
 */
static void
ds4viz_p_bf_(ds4viz_p_buf_t *p_b, const char *p_fmt, ...)
{
    va_list a;
    va_list a2;
    int n;

    va_start(a, p_fmt);
    va_copy(a2, a);
    n = vsnprintf(NULL, 0, p_fmt, a);
    va_end(a);
    ds4viz_p_bg_(p_b, n + 1);
    vsnprintf(p_b->p_data + p_b->len, (size_t)(n + 1), p_fmt, a2);
    va_end(a2);
    p_b->len += n;
}

/**
 * @brief 释放缓冲区
 */
static void
ds4viz_p_bfree_(ds4viz_p_buf_t *p_b)
{
    free(p_b->p_data);
    p_b->p_data = NULL;
    p_b->len = p_b->cap = 0;
}

/* ----------------------------------------------------------------
 * 返回字符串池
 * ---------------------------------------------------------------- */

/**
 * @brief 将字符串指针加入池, 以便作用域结束时统一释放
 */
static void
ds4viz_p_pool_add_(ds4viz_p_strpool_t *p_pool, char *p_str)
{
    if (!p_str)
    {
        return;
    }
    if (p_pool->cnt >= p_pool->cap)
    {
        p_pool->cap = p_pool->cap ? p_pool->cap * 2 : 8;
        p_pool->p_ptrs = (char **)realloc(p_pool->p_ptrs, (size_t)p_pool->cap * sizeof(char *));
    }
    p_pool->p_ptrs[p_pool->cnt++] = p_str;
}

/**
 * @brief 释放池中所有字符串并销毁池
 */
static void
ds4viz_p_pool_free_(ds4viz_p_strpool_t *p_pool)
{
    int i;

    for (i = 0; i < p_pool->cnt; i++)
    {
        free(p_pool->p_ptrs[i]);
    }
    free(p_pool->p_ptrs);
    p_pool->p_ptrs = NULL;
    p_pool->cnt = p_pool->cap = 0;
}

/* ----------------------------------------------------------------
 * TOML 序列化
 * ---------------------------------------------------------------- */

/**
 * @brief 序列化 TOML 字符串 (带转义)
 */
static void
ds4viz_p_tstr_(ds4viz_p_buf_t *p_b, const char *p_s)
{
    ds4viz_p_bc_(p_b, '"');
    if (p_s)
    {
        for (; *p_s; p_s++)
        {
            switch (*p_s)
            {
            case '\\':
                ds4viz_p_bs_(p_b, "\\\\");
                break;
            case '"':
                ds4viz_p_bs_(p_b, "\\\"");
                break;
            case '\n':
                ds4viz_p_bs_(p_b, "\\n");
                break;
            case '\r':
                ds4viz_p_bs_(p_b, "\\r");
                break;
            case '\t':
                ds4viz_p_bs_(p_b, "\\t");
                break;
            default:
                ds4viz_p_bc_(p_b, *p_s);
            }
        }
    }
    ds4viz_p_bc_(p_b, '"');
}

/**
 * @brief 序列化 TOML 值
 */
static void
ds4viz_p_tval_(ds4viz_p_buf_t *p_b, ds4viz_p_val_t v)
{
    char tmp[64];

    switch (v.type)
    {
    case DS4VIZ_P_VINT_:
        ds4viz_p_bf_(p_b, "%lld", v.i);
        break;
    case DS4VIZ_P_VDBL_:
        snprintf(tmp, sizeof tmp, "%.17g", v.d);
        ds4viz_p_bs_(p_b, tmp);
        if (!strchr(tmp, '.') && !strchr(tmp, 'e') && !strchr(tmp, 'E') && !strchr(tmp, 'n') && !strchr(tmp, 'i'))
        {
            ds4viz_p_bs_(p_b, ".0");
        }
        break;
    case DS4VIZ_P_VSTR_:
        ds4viz_p_tstr_(p_b, v.p_s ? v.p_s : "");
        break;
    case DS4VIZ_P_VBOOL_:
        ds4viz_p_bs_(p_b, v.b ? "true" : "false");
        break;
    }
}

/**
 * @brief 将值格式化到固定缓冲区
 */
static void
ds4viz_p_vstr_(char *p_buf, int sz, ds4viz_p_val_t v)
{
    ds4viz_p_buf_t tmp = {0};

    ds4viz_p_tval_(&tmp, v);
    if (tmp.p_data)
    {
        snprintf(p_buf, (size_t)sz, "%s", tmp.p_data);
    }
    else
    {
        p_buf[0] = 0;
    }
    ds4viz_p_bfree_(&tmp);
}

/* ---- 值复制/释放 ---- */

/**
 * @brief 深拷贝值 (字符串类型会 malloc 副本)
 */
static ds4viz_p_val_t
ds4viz_p_vdup_(ds4viz_p_val_t v)
{
    size_t n;
    char *p_copy;

    if (v.type == DS4VIZ_P_VSTR_ && v.p_s)
    {
        n = strlen(v.p_s);
        p_copy = (char *)malloc(n + 1);
        memcpy(p_copy, v.p_s, n + 1);
        v.p_s = p_copy;
    }
    return v;
}

/**
 * @brief 释放值中的堆内存
 */
static void
ds4viz_p_vfree_(ds4viz_p_val_t *p_v)
{
    if (p_v->type == DS4VIZ_P_VSTR_ && p_v->p_s)
    {
        free((void *)p_v->p_s);
        p_v->p_s = NULL;
    }
}

/**
 * @brief 提取值的数值部分
 */
static double
ds4viz_p_vnum_(ds4viz_p_val_t v)
{
    return v.type == DS4VIZ_P_VDBL_ ? v.d : (double)v.i;
}

/**
 * @brief 内部值转公开值
 */
static ds4vizValue
ds4viz_p_to_pub_(ds4viz_p_val_t v)
{
    ds4vizValue r = {0};

    switch (v.type)
    {
    case DS4VIZ_P_VINT_:
        r.tag = DS4VIZ_INT;
        r.i = v.i;
        break;
    case DS4VIZ_P_VDBL_:
        r.tag = DS4VIZ_FLOAT;
        r.f = v.d;
        break;
    case DS4VIZ_P_VSTR_:
        r.tag = DS4VIZ_STR;
        r.p_s = v.p_s;
        break;
    case DS4VIZ_P_VBOOL_:
        r.tag = DS4VIZ_BOOL;
        r.b = v.b;
        break;
    }
    return r;
}

/* ---- 容量增长 ---- */

/**
 * @brief 动态数组扩容
 *
 * @def DS4VIZ_P_GROW_
 */
#define DS4VIZ_P_GROW_(arr, cnt, cap, type)                               \
    do                                                                    \
    {                                                                     \
        if ((cnt) >= (cap))                                               \
        {                                                                 \
            (cap) = (cap) ? (cap) * 2 : 8;                                \
            (arr) = (type *)realloc((arr), (size_t)(cap) * sizeof(type)); \
        }                                                                 \
    } while (0)

/* ---- 错误记录 ---- */

/**
 * @brief 记录错误信息
 *
 * @def DS4VIZ_P_ERR_
 */
#define DS4VIZ_P_ERR_(p, ln, ...)                                \
    do                                                           \
    {                                                            \
        (p)->err = true;                                         \
        snprintf((p)->errmsg, sizeof((p)->errmsg), __VA_ARGS__); \
        (p)->err_step = (p)->step_id - 1;                        \
        (p)->err_last_state = (p)->state_id - 1;                 \
        (p)->err_line = ln;                                      \
    } while (0)

/* ----------------------------------------------------------------
 * 时间戳与编译器版本
 * ---------------------------------------------------------------- */

/**
 * @brief 生成 RFC3339 UTC 时间戳
 */
static void
ds4viz_p_ts_(char *p_buf, int sz)
{
    time_t now = time(NULL);
    struct tm *p_u = gmtime(&now);

    if (p_u)
    {
        strftime(p_buf, (size_t)sz, "%Y-%m-%dT%H:%M:%SZ", p_u);
    }
    else
    {
        snprintf(p_buf, (size_t)sz, "1970-01-01T00:00:00Z");
    }
}

/**
 * @brief 获取 C 语言标准版本字符串
 */
static void
ds4viz_p_cv_(char *p_buf, int sz)
{
#if defined(__STDC_VERSION__)
    snprintf(p_buf, (size_t)sz, "%ld", (long)__STDC_VERSION__);
#else
    snprintf(p_buf, (size_t)sz, "unknown");
#endif
}

/* ----------------------------------------------------------------
 * 别名查找
 * ---------------------------------------------------------------- */

/**
 * @brief 在对象的别名列表中查找指定节点的别名
 *
 * @param p_obj 数据结构对象指针
 * @param nid 节点 id
 * @return const char* 别名字符串, 未找到返回 NULL
 */
static const char *
ds4viz_p_alias_get_(void *p_obj, int nid)
{
    ds4viz_p_alias_t *p_a;
    int cnt;
    int i;

    p_a = *(ds4viz_p_alias_t **)((char *)p_obj + offsetof(ds4viz_p_stack_t, p_aliases));
    cnt = *(int *)((char *)p_obj + offsetof(ds4viz_p_stack_t, alias_cnt));
    for (i = 0; i < cnt; i++)
    {
        if (p_a[i].id == nid)
        {
            return p_a[i].name;
        }
    }
    return NULL;
}

/**
 * @brief 更新对象最近一次用户调用行号
 */
static void
ds4viz_p_touch_line_(void *p_obj, int line)
{
    int *p_last_user_line;

    if (line <= 0)
    {
        return;
    }
    p_last_user_line = (int *)((char *)p_obj + offsetof(ds4viz_p_stack_t, last_user_line));
    *p_last_user_line = line;
}

/**
 * @brief 从后向前查找子串
 */
static char *
ds4viz_p_rfind_(char *p_hay, const char *p_ndl)
{
    char *p_cur;
    char *p_last;

    if (!p_hay || !p_ndl || !p_ndl[0])
    {
        return NULL;
    }
    p_cur = strstr(p_hay, p_ndl);
    p_last = NULL;
    while (p_cur)
    {
        p_last = p_cur;
        p_cur = strstr(p_cur + 1, p_ndl);
    }
    return p_last;
}

/**
 * @brief 重写最后一个 state (保持 state_id 不变)
 */
static void
ds4viz_p_rewrite_last_state_(void *p_obj)
{
    ds4viz_p_buf_t *p_sb;
    int *p_state_id;
    ds4viz_p_ws_fn_t *p_ws;
    char *p_pos;
    int sid_saved;

    p_sb = (ds4viz_p_buf_t *)((char *)p_obj + offsetof(ds4viz_p_stack_t, states_buf));
    p_state_id = (int *)((char *)p_obj + offsetof(ds4viz_p_stack_t, state_id));
    p_ws = (ds4viz_p_ws_fn_t *)((char *)p_obj + offsetof(ds4viz_p_stack_t, ws_fn));

    if (!p_sb->p_data || *p_state_id <= 0 || !(*p_ws))
    {
        return;
    }

    p_pos = ds4viz_p_rfind_(p_sb->p_data, "\n[[states]]\n");
    if (!p_pos)
    {
        return;
    }

    p_sb->len = (int)(p_pos - p_sb->p_data);
    p_sb->p_data[p_sb->len] = '\0';

    sid_saved = *p_state_id;
    *p_state_id = sid_saved - 1;
    (*p_ws)(p_obj);
    *p_state_id = sid_saved;
}

/* ----------------------------------------------------------------
 * 高亮序列化
 * ---------------------------------------------------------------- */

/**
 * @brief 将高亮标记数组序列化为 TOML
 */
static void
ds4viz_p_ser_hls_(ds4viz_p_buf_t *p_b, const ds4viz_p_hl_t *p_hls, int nhl)
{
    int i;
    const ds4viz_p_hl_t *p_h;

    if (!p_hls || nhl <= 0)
    {
        ds4viz_p_bs_(p_b, "highlights = []\n");
        return;
    }
    ds4viz_p_bs_(p_b, "highlights = [\n");
    for (i = 0; i < nhl; i++)
    {
        if (i)
        {
            ds4viz_p_bs_(p_b, ",\n");
        }
        p_h = &p_hls[i];
        if (p_h->kind == 0)
        {
            ds4viz_p_bf_(p_b, "  { kind = \"node\", target = %d, style = ", p_h->target);
            ds4viz_p_tstr_(p_b, p_h->p_style);
            ds4viz_p_bf_(p_b, ", level = %d }", p_h->level);
        }
        else if (p_h->kind == 1)
        {
            ds4viz_p_bf_(p_b, "  { kind = \"item\", target = %d, style = ", p_h->target);
            ds4viz_p_tstr_(p_b, p_h->p_style);
            ds4viz_p_bf_(p_b, ", level = %d }", p_h->level);
        }
        else
        {
            ds4viz_p_bf_(p_b, "  { kind = \"edge\", from = %d, to = %d, style = ",
                         p_h->from, p_h->to);
            ds4viz_p_tstr_(p_b, p_h->p_style);
            ds4viz_p_bf_(p_b, ", level = %d }", p_h->level);
        }
    }
    ds4viz_p_bs_(p_b, "\n]\n");
}

/* ----------------------------------------------------------------
 * 待刷新步骤管理
 * ---------------------------------------------------------------- */

/**
 * @brief 释放待刷新步骤的堆内存
 */
static void
ds4viz_p_pstep_free_(ds4viz_p_pstep_t *p_p)
{
    free(p_p->p_args);
    free(p_p->p_ret_str);
    free(p_p->p_phase);
    free(p_p->p_note);
    free(p_p->p_hls);
    memset(p_p, 0, sizeof(*p_p));
}

/**
 * @brief 将待刷新步骤序列化到步骤缓冲区
 */
static void
ds4viz_p_pstep_flush_(ds4viz_p_buf_t *p_sb, ds4viz_p_pstep_t *p_p)
{
    ds4viz_p_buf_t tmp;

    if (!p_p->valid)
    {
        return;
    }
    ds4viz_p_bf_(p_sb, "\n[[steps]]\nid = %d\nop = ", p_p->id);
    tmp = (ds4viz_p_buf_t){0};
    ds4viz_p_tstr_(&tmp, p_p->op);
    ds4viz_p_bs_(p_sb, tmp.p_data);
    ds4viz_p_bfree_(&tmp);
    ds4viz_p_bc_(p_sb, '\n');
    if (p_p->p_phase)
    {
        ds4viz_p_bs_(p_sb, "phase = ");
        tmp = (ds4viz_p_buf_t){0};
        ds4viz_p_tstr_(&tmp, p_p->p_phase);
        ds4viz_p_bs_(p_sb, tmp.p_data);
        ds4viz_p_bfree_(&tmp);
        ds4viz_p_bc_(p_sb, '\n');
    }
    ds4viz_p_bf_(p_sb, "before = %d\n", p_p->before);
    if (p_p->after >= 0)
    {
        ds4viz_p_bf_(p_sb, "after = %d\n", p_p->after);
    }
    if (p_p->p_ret_str)
    {
        ds4viz_p_bf_(p_sb, "ret = %s\n", p_p->p_ret_str);
    }
    if (p_p->p_note)
    {
        ds4viz_p_bs_(p_sb, "note = ");
        tmp = (ds4viz_p_buf_t){0};
        ds4viz_p_tstr_(&tmp, p_p->p_note);
        ds4viz_p_bs_(p_sb, tmp.p_data);
        ds4viz_p_bfree_(&tmp);
        ds4viz_p_bc_(p_sb, '\n');
    }
    if (p_p->p_hls && p_p->nhl > 0)
    {
        ds4viz_p_ser_hls_(p_sb, p_p->p_hls, p_p->nhl);
    }
    else if (p_p->p_hls && p_p->nhl == 0)
    {
        ds4viz_p_bs_(p_sb, "highlights = []\n");
    }
    ds4viz_p_bs_(p_sb, "\n[steps.args]\n");
    if (p_p->p_args && p_p->p_args[0])
    {
        ds4viz_p_bs_(p_sb, p_p->p_args);
    }
    ds4viz_p_bf_(p_sb, "\n[steps.code]\nline = %d\n", p_p->code_line);
    ds4viz_p_pstep_free_(p_p);
}

/**
 * @brief 创建新待刷新步骤, 先刷新旧的
 */
static void
ds4viz_p_new_step_(void *p_obj, const char *p_op, int before, int after,
                   const char *p_args, const char *p_ret_str, int line)
{
    ds4viz_p_buf_t *p_sb;
    ds4viz_p_pstep_t *p_p;
    int *p_sid;
    int *p_last_user_line;
    int phase_depth;
    const char **p_phase_stack;

    p_sb = (ds4viz_p_buf_t *)((char *)p_obj + offsetof(ds4viz_p_stack_t, steps_buf));
    p_p = (ds4viz_p_pstep_t *)((char *)p_obj + offsetof(ds4viz_p_stack_t, pending));
    p_sid = (int *)((char *)p_obj + offsetof(ds4viz_p_stack_t, step_id));
    p_last_user_line = (int *)((char *)p_obj + offsetof(ds4viz_p_stack_t, last_user_line));
    if (line > 0)
    {
        *p_last_user_line = line;
    }
    phase_depth = *(int *)((char *)p_obj + offsetof(ds4viz_p_stack_t, phase_depth));
    p_phase_stack = (const char **)((char *)p_obj + offsetof(ds4viz_p_stack_t, p_phase_stack));

    ds4viz_p_pstep_flush_(p_sb, p_p);
    memset(p_p, 0, sizeof(*p_p));
    p_p->valid = true;
    p_p->id = (*p_sid)++;
    snprintf(p_p->op, sizeof p_p->op, "%s", p_op);
    p_p->before = before;
    p_p->after = after;
    if (p_args && p_args[0])
    {
        p_p->p_args = strdup(p_args);
    }
    if (p_ret_str)
    {
        p_p->p_ret_str = strdup(p_ret_str);
    }
    p_p->code_line = line;
    if (phase_depth > 0)
    {
        p_p->p_phase = strdup(p_phase_stack[phase_depth - 1]);
    }
}

/* ----------------------------------------------------------------
 * 状态写入辅助
 * ---------------------------------------------------------------- */

/**
 * @brief 写入状态头
 */
static void
ds4viz_p_bstate_(ds4viz_p_buf_t *p_b, int sid)
{
    ds4viz_p_bf_(p_b, "\n[[states]]\nid = %d\n\n[states.data]\n", sid);
}

/**
 * @brief 写入值数组字段
 */
static void
ds4viz_p_wsa_(ds4viz_p_buf_t *p_b, const char *p_key,
              ds4viz_p_val_t *p_items, int count)
{
    int i;

    ds4viz_p_bs_(p_b, p_key);
    ds4viz_p_bs_(p_b, " = [");
    for (i = 0; i < count; i++)
    {
        if (i)
        {
            ds4viz_p_bs_(p_b, ", ");
        }
        ds4viz_p_tval_(p_b, p_items[i]);
    }
    ds4viz_p_bs_(p_b, "]\n");
}

/* ----------------------------------------------------------------
 * 通用 step/amend/phase/alias 实现
 * ---------------------------------------------------------------- */

void ds4viz_p_step_fn_(void *p_obj, const char *p_note,
                       const ds4viz_p_hl_t *p_hls, int nhl, int line)
{
    bool *p_err;
    int state;
    ds4viz_p_pstep_t *p_p;

    p_err = (bool *)((char *)p_obj + offsetof(ds4viz_p_stack_t, err));
    if (*p_err)
    {
        return;
    }
    state = *(int *)((char *)p_obj + offsetof(ds4viz_p_stack_t, state_id)) - 1;
    ds4viz_p_new_step_(p_obj, "observe", state, state, NULL, NULL, line);

    p_p = (ds4viz_p_pstep_t *)((char *)p_obj + offsetof(ds4viz_p_stack_t, pending));
    if (p_note)
    {
        p_p->p_note = strdup(p_note);
    }
    if (p_hls && nhl > 0)
    {
        p_p->p_hls = (ds4viz_p_hl_t *)malloc((size_t)nhl * sizeof(ds4viz_p_hl_t));
        memcpy(p_p->p_hls, p_hls, (size_t)nhl * sizeof(ds4viz_p_hl_t));
        p_p->nhl = nhl;
    }
}

void ds4viz_p_amend_fn_(void *p_obj, const char *p_note,
                        const ds4viz_p_hl_t *p_hls, int nhl, int line)
{
    bool *p_err;
    ds4viz_p_pstep_t *p_p;

    p_err = (bool *)((char *)p_obj + offsetof(ds4viz_p_stack_t, err));
    if (*p_err)
    {
        return;
    }

    ds4viz_p_touch_line_(p_obj, line);

    p_p = (ds4viz_p_pstep_t *)((char *)p_obj + offsetof(ds4viz_p_stack_t, pending));
    if (!p_p->valid)
    {
        DS4VIZ_P_ERR_((ds4viz_p_stack_t *)p_obj, line, "No previous step to amend");
        return;
    }
    if (p_note)
    {
        free(p_p->p_note);
        p_p->p_note = strdup(p_note);
    }
    if (p_hls)
    {
        free(p_p->p_hls);
        if (nhl > 0)
        {
            p_p->p_hls = (ds4viz_p_hl_t *)malloc((size_t)nhl * sizeof(ds4viz_p_hl_t));
            memcpy(p_p->p_hls, p_hls, (size_t)nhl * sizeof(ds4viz_p_hl_t));
            p_p->nhl = nhl;
        }
        else
        {
            p_p->p_hls = (ds4viz_p_hl_t *)malloc(sizeof(ds4viz_p_hl_t));
            p_p->nhl = 0;
        }
    }
}

void ds4viz_p_amend_hl_fn_(void *p_obj, const ds4viz_p_hl_t *p_hls, int nhl, int line)
{
    ds4viz_p_amend_fn_(p_obj, NULL, p_hls, nhl, line);
}

void ds4viz_p_amend_clear_hl_fn_(void *p_obj, int line)
{
    bool *p_err;
    ds4viz_p_pstep_t *p_p;

    p_err = (bool *)((char *)p_obj + offsetof(ds4viz_p_stack_t, err));
    if (*p_err)
    {
        return;
    }

    ds4viz_p_touch_line_(p_obj, line);

    p_p = (ds4viz_p_pstep_t *)((char *)p_obj + offsetof(ds4viz_p_stack_t, pending));
    if (!p_p->valid)
    {
        return;
    }
    free(p_p->p_hls);
    p_p->p_hls = (ds4viz_p_hl_t *)malloc(sizeof(ds4viz_p_hl_t));
    p_p->nhl = 0;
}

void ds4viz_p_phase_push_(void *p_obj, const char *p_name)
{
    int *p_depth;
    const char **p_stack;

    p_depth = (int *)((char *)p_obj + offsetof(ds4viz_p_stack_t, phase_depth));
    p_stack = (const char **)((char *)p_obj + offsetof(ds4viz_p_stack_t, p_phase_stack));
    if (*p_depth < DS4VIZ_P_PHASE_DEPTH_MAX_)
    {
        p_stack[(*p_depth)++] = p_name;
    }
}

void ds4viz_p_phase_pop_(void *p_obj)
{
    int *p_depth;

    p_depth = (int *)((char *)p_obj + offsetof(ds4viz_p_stack_t, phase_depth));
    if (*p_depth > 0)
    {
        (*p_depth)--;
    }
}

void ds4viz_p_alias_fn_(void *p_obj, int nid, const char *p_name, int line)
{
    bool *p_err;
    ds4viz_p_alias_t **pp_a;
    int *p_cnt;
    int *p_cap;
    int i;
    bool changed;

    p_err = (bool *)((char *)p_obj + offsetof(ds4viz_p_stack_t, err));
    if (*p_err)
    {
        return;
    }

    ds4viz_p_touch_line_(p_obj, line);

    pp_a = (ds4viz_p_alias_t **)((char *)p_obj + offsetof(ds4viz_p_stack_t, p_aliases));
    p_cnt = (int *)((char *)p_obj + offsetof(ds4viz_p_stack_t, alias_cnt));
    p_cap = (int *)((char *)p_obj + offsetof(ds4viz_p_stack_t, alias_cap));

    changed = false;

    if (!p_name)
    {
        for (i = 0; i < *p_cnt; i++)
        {
            if ((*pp_a)[i].id == nid)
            {
                (*pp_a)[i] = (*pp_a)[--(*p_cnt)];
                changed = true;
                break;
            }
        }
        if (changed)
        {
            ds4viz_p_rewrite_last_state_(p_obj);
        }
        return;
    }

    for (i = 0; i < *p_cnt; i++)
    {
        if ((*pp_a)[i].id == nid)
        {
            snprintf((*pp_a)[i].name, 65, "%s", p_name);
            changed = true;
            break;
        }
    }

    if (!changed)
    {
        if (*p_cnt >= *p_cap)
        {
            *p_cap = *p_cap ? *p_cap * 2 : 8;
            *pp_a = (ds4viz_p_alias_t *)realloc(*pp_a, (size_t)*p_cap * sizeof(ds4viz_p_alias_t));
        }
        (*pp_a)[*p_cnt].id = nid;
        snprintf((*pp_a)[*p_cnt].name, 65, "%s", p_name);
        (*p_cnt)++;
        changed = true;
    }

    if (changed)
    {
        ds4viz_p_rewrite_last_state_(p_obj);
    }
}

/* ----------------------------------------------------------------
 * record_step 辅助
 * ---------------------------------------------------------------- */

/**
 * @brief 为数据结构操作记录步骤
 */
static void
ds4viz_p_rec_step_(void *p_obj, const char *p_op, int before, int after,
                   const char *p_args, const char *p_ret_str, int line)
{
    ds4viz_p_new_step_(p_obj, p_op, before, after, p_args, p_ret_str, line);
}

/* ----------------------------------------------------------------
 * 参数构建辅助
 * ---------------------------------------------------------------- */

static void
ds4viz_p_av_(ds4viz_p_buf_t *p_b, const char *p_k, ds4viz_p_val_t v)
{
    ds4viz_p_bs_(p_b, p_k);
    ds4viz_p_bs_(p_b, " = ");
    ds4viz_p_tval_(p_b, v);
    ds4viz_p_bc_(p_b, '\n');
}

static void
ds4viz_p_ai_(ds4viz_p_buf_t *p_b, const char *p_k, int v)
{
    ds4viz_p_bf_(p_b, "%s = %d\n", p_k, v);
}

static void
ds4viz_p_as_(ds4viz_p_buf_t *p_b, const char *p_k, const char *p_v)
{
    ds4viz_p_bs_(p_b, p_k);
    ds4viz_p_bs_(p_b, " = ");
    ds4viz_p_tstr_(p_b, p_v);
    ds4viz_p_bc_(p_b, '\n');
}

static void
ds4viz_p_ad_(ds4viz_p_buf_t *p_b, const char *p_k, double v)
{
    ds4viz_p_av_(p_b, p_k, (ds4viz_p_val_t){DS4VIZ_P_VDBL_, .d = v});
}

/* ----------------------------------------------------------------
 * 最终写入
 * ---------------------------------------------------------------- */

/**
 * @brief 将完整 TOML 输出写入文件
 */
static void
ds4viz_p_flush_(const char *p_kind, const char *p_label,
                ds4viz_p_buf_t *p_sb_st, ds4viz_p_buf_t *p_sb_sp,
                ds4viz_p_pstep_t *p_pend,
                bool err, const char *p_errmsg,
                int err_step, int err_ls, int err_line,
                int final_state, int scope_line, int commit_line)
{
    char ts[64];
    char cv[32];
    ds4viz_p_buf_t out = {0};
    FILE *p_f;

    ds4viz_p_ts_(ts, sizeof ts);
    ds4viz_p_cv_(cv, sizeof cv);
    ds4viz_p_pstep_flush_(p_sb_sp, p_pend);

    /* [meta] */
    ds4viz_p_bs_(&out, "[meta]\ncreated_at = ");
    ds4viz_p_tstr_(&out, ts);
    ds4viz_p_bs_(&out, "\nlang = \"c\"\nlang_version = ");
    ds4viz_p_tstr_(&out, cv);
    ds4viz_p_bc_(&out, '\n');

    /* [package] */
    ds4viz_p_bs_(&out, "\n[package]\nname = \"ds4viz\"\nlang = \"c\"\nversion = \"" DS4VIZ_P_VERSION_ "\"\n");

    /* [remarks] */
    if (ds4viz_p_g_.title || ds4viz_p_g_.author || ds4viz_p_g_.comment)
    {
        ds4viz_p_bs_(&out, "\n[remarks]\n");
        if (ds4viz_p_g_.title)
        {
            ds4viz_p_bs_(&out, "title = ");
            ds4viz_p_tstr_(&out, ds4viz_p_g_.title);
            ds4viz_p_bc_(&out, '\n');
        }
        if (ds4viz_p_g_.author)
        {
            ds4viz_p_bs_(&out, "author = ");
            ds4viz_p_tstr_(&out, ds4viz_p_g_.author);
            ds4viz_p_bc_(&out, '\n');
        }
        if (ds4viz_p_g_.comment)
        {
            ds4viz_p_bs_(&out, "comment = ");
            ds4viz_p_tstr_(&out, ds4viz_p_g_.comment);
            ds4viz_p_bc_(&out, '\n');
        }
    }

    /* [object] */
    ds4viz_p_bs_(&out, "\n[object]\nkind = ");
    ds4viz_p_tstr_(&out, p_kind);
    ds4viz_p_bc_(&out, '\n');
    if (p_label && p_label[0])
    {
        ds4viz_p_bs_(&out, "label = ");
        ds4viz_p_tstr_(&out, p_label);
        ds4viz_p_bc_(&out, '\n');
    }

    /* states + steps */
    if (p_sb_st->p_data)
    {
        ds4viz_p_bs_(&out, p_sb_st->p_data);
    }
    if (p_sb_sp->p_data)
    {
        ds4viz_p_bs_(&out, p_sb_sp->p_data);
    }

    /* result/error */
    if (err)
    {
        ds4viz_p_bs_(&out, "\n[error]\ntype = \"runtime\"\nmessage = ");
        ds4viz_p_tstr_(&out, p_errmsg);
        ds4viz_p_bc_(&out, '\n');
        if (err_line > 0)
        {
            ds4viz_p_bf_(&out, "line = %d\n", err_line);
        }
        if (err_step >= 0)
        {
            ds4viz_p_bf_(&out, "step = %d\n", err_step);
        }
        if (err_ls >= 0)
        {
            ds4viz_p_bf_(&out, "last_state = %d\n", err_ls);
        }
    }
    else
    {
        if (commit_line <= 0)
        {
            commit_line = scope_line > 0 ? scope_line : 1;
        }
        ds4viz_p_bf_(&out,
                     "\n[result]\nfinal_state = %d\n\n[result.commit]\nop = \"commit\"\nline = %d\n",
                     final_state, commit_line);
    }

    p_f = fopen(ds4viz_p_outpath_(), "w");
    if (p_f)
    {
        if (out.p_data)
        {
            fwrite(out.p_data, 1, (size_t)out.len, p_f);
        }
        fclose(p_f);
    }
    ds4viz_p_bfree_(&out);
}

/**
 * @brief 通用 close 宏: 刷新 TOML 并释放公共资源
 *
 * @def DS4VIZ_P_CLOSE_
 */
#define DS4VIZ_P_CLOSE_(p)                                                     \
    ds4viz_p_flush_((p)->p_kind, (p)->p_label,                                 \
                    &(p)->states_buf, &(p)->steps_buf, &(p)->pending,          \
                    (p)->err, (p)->errmsg, (p)->err_step, (p)->err_last_state, \
                    (p)->err_line, (p)->state_id - 1, (p)->scope_line,         \
                    (p)->last_user_line);                                      \
    free((p)->p_aliases);                                                      \
    ds4viz_p_bfree_(&(p)->states_buf);                                         \
    ds4viz_p_bfree_(&(p)->steps_buf);                                          \
    ds4viz_p_pool_free_(&(p)->ret_pool);                                       \
    (p)->done = true

/* ================================================================
 * 栈
 * ================================================================ */

/**
 * @brief 栈状态写入回调
 */
static void
ds4viz_p_stack_ws_(void *p_self)
{
    ds4viz_p_stack_t *p_s = (ds4viz_p_stack_t *)p_self;

    ds4viz_p_bstate_(&p_s->states_buf, p_s->state_id);
    ds4viz_p_wsa_(&p_s->states_buf, "items", p_s->p_items, p_s->count);
    ds4viz_p_bf_(&p_s->states_buf, "top = %d\n", p_s->count > 0 ? p_s->count - 1 : -1);
}

ds4viz_p_stack_t
ds4viz_p_stack_open_(const char *p_label, int line)
{
    ds4viz_p_stack_t s = {0};

    s.p_label = p_label;
    s.p_kind = "stack";
    s.scope_line = line;
    s.ws_fn = ds4viz_p_stack_ws_;
    s.err_step = -1;
    ds4viz_p_stack_ws_(&s);
    s.state_id++;
    return s;
}

void ds4viz_p_stack_close_(ds4viz_p_stack_t *p_s)
{
    int i;

    DS4VIZ_P_CLOSE_(p_s);
    for (i = 0; i < p_s->count; i++)
    {
        ds4viz_p_vfree_(&p_s->p_items[i]);
    }
    free(p_s->p_items);
}

void ds4viz_p_stack_push_(ds4viz_p_stack_t *p_s, ds4viz_p_val_t v, int line)
{
    int before;
    int after;
    ds4viz_p_buf_t a = {0};

    if (p_s->err)
    {
        return;
    }
    v = ds4viz_p_vdup_(v);
    DS4VIZ_P_GROW_(p_s->p_items, p_s->count, p_s->cap, ds4viz_p_val_t);
    p_s->p_items[p_s->count++] = v;

    before = p_s->state_id - 1;
    ds4viz_p_stack_ws_(p_s);
    after = p_s->state_id++;

    ds4viz_p_av_(&a, "value", v);
    ds4viz_p_rec_step_(p_s, "push", before, after, a.p_data, NULL, line);
    ds4viz_p_bfree_(&a);
}

ds4vizValue
ds4viz_p_stack_pop_(ds4viz_p_stack_t *p_s, int line)
{
    ds4vizValue rv = {0};
    ds4viz_p_val_t popped;
    int before;
    int after;
    char rs[256];

    if (p_s->err)
    {
        return rv;
    }
    if (p_s->count == 0)
    {
        ds4viz_p_rec_step_(p_s, "pop", p_s->state_id - 1, -1, NULL, NULL, line);
        DS4VIZ_P_ERR_(p_s, line, "Cannot pop from empty stack");
        return rv;
    }
    popped = p_s->p_items[--p_s->count];
    rv = ds4viz_p_to_pub_(popped);
    if (popped.type == DS4VIZ_P_VSTR_ && popped.p_s)
    {
        ds4viz_p_pool_add_(&p_s->ret_pool, (char *)popped.p_s);
    }

    before = p_s->state_id - 1;
    ds4viz_p_stack_ws_(p_s);
    after = p_s->state_id++;

    ds4viz_p_vstr_(rs, sizeof rs, popped);
    ds4viz_p_rec_step_(p_s, "pop", before, after, NULL, rs, line);
    return rv;
}

void ds4viz_p_stack_clear_(ds4viz_p_stack_t *p_s, int line)
{
    int i;
    int before;
    int after;

    if (p_s->err)
    {
        return;
    }
    for (i = 0; i < p_s->count; i++)
    {
        ds4viz_p_vfree_(&p_s->p_items[i]);
    }
    p_s->count = 0;

    before = p_s->state_id - 1;
    ds4viz_p_stack_ws_(p_s);
    after = p_s->state_id++;
    ds4viz_p_rec_step_(p_s, "clear", before, after, NULL, NULL, line);
}

/* ================================================================
 * 队列
 * ================================================================ */

static void
ds4viz_p_queue_ws_(void *p_self)
{
    ds4viz_p_queue_t *p_q = (ds4viz_p_queue_t *)p_self;

    ds4viz_p_bstate_(&p_q->states_buf, p_q->state_id);
    ds4viz_p_wsa_(&p_q->states_buf, "items", p_q->p_items, p_q->count);
    if (p_q->count == 0)
    {
        ds4viz_p_bs_(&p_q->states_buf, "front = -1\nrear = -1\n");
    }
    else
    {
        ds4viz_p_bf_(&p_q->states_buf, "front = 0\nrear = %d\n", p_q->count - 1);
    }
}

ds4viz_p_queue_t
ds4viz_p_queue_open_(const char *p_label, int line)
{
    ds4viz_p_queue_t q = {0};

    q.p_label = p_label;
    q.p_kind = "queue";
    q.scope_line = line;
    q.ws_fn = ds4viz_p_queue_ws_;
    q.err_step = -1;
    ds4viz_p_queue_ws_(&q);
    q.state_id++;
    return q;
}

void ds4viz_p_queue_close_(ds4viz_p_queue_t *p_q)
{
    int i;

    DS4VIZ_P_CLOSE_(p_q);
    for (i = 0; i < p_q->count; i++)
    {
        ds4viz_p_vfree_(&p_q->p_items[i]);
    }
    free(p_q->p_items);
}

void ds4viz_p_queue_enqueue_(ds4viz_p_queue_t *p_q, ds4viz_p_val_t v, int line)
{
    int before;
    int after;
    ds4viz_p_buf_t a = {0};

    if (p_q->err)
    {
        return;
    }
    v = ds4viz_p_vdup_(v);
    DS4VIZ_P_GROW_(p_q->p_items, p_q->count, p_q->cap, ds4viz_p_val_t);
    p_q->p_items[p_q->count++] = v;

    before = p_q->state_id - 1;
    ds4viz_p_queue_ws_(p_q);
    after = p_q->state_id++;

    ds4viz_p_av_(&a, "value", v);
    ds4viz_p_rec_step_(p_q, "enqueue", before, after, a.p_data, NULL, line);
    ds4viz_p_bfree_(&a);
}

ds4vizValue
ds4viz_p_queue_dequeue_(ds4viz_p_queue_t *p_q, int line)
{
    ds4vizValue rv = {0};
    ds4viz_p_val_t deq;
    int before;
    int after;
    char rs[256];

    if (p_q->err)
    {
        return rv;
    }
    if (p_q->count == 0)
    {
        ds4viz_p_rec_step_(p_q, "dequeue", p_q->state_id - 1, -1, NULL, NULL, line);
        DS4VIZ_P_ERR_(p_q, line, "Cannot dequeue from empty queue");
        return rv;
    }
    deq = p_q->p_items[0];
    rv = ds4viz_p_to_pub_(deq);
    if (deq.type == DS4VIZ_P_VSTR_ && deq.p_s)
    {
        ds4viz_p_pool_add_(&p_q->ret_pool, (char *)deq.p_s);
    }
    memmove(p_q->p_items, p_q->p_items + 1, (size_t)(p_q->count - 1) * sizeof(ds4viz_p_val_t));
    p_q->count--;

    before = p_q->state_id - 1;
    ds4viz_p_queue_ws_(p_q);
    after = p_q->state_id++;

    ds4viz_p_vstr_(rs, sizeof rs, deq);
    ds4viz_p_rec_step_(p_q, "dequeue", before, after, NULL, rs, line);
    return rv;
}

void ds4viz_p_queue_clear_(ds4viz_p_queue_t *p_q, int line)
{
    int i;
    int before;
    int after;

    if (p_q->err)
    {
        return;
    }
    for (i = 0; i < p_q->count; i++)
    {
        ds4viz_p_vfree_(&p_q->p_items[i]);
    }
    p_q->count = 0;

    before = p_q->state_id - 1;
    ds4viz_p_queue_ws_(p_q);
    after = p_q->state_id++;
    ds4viz_p_rec_step_(p_q, "clear", before, after, NULL, NULL, line);
}

/* ================================================================
 * 单链表
 * ================================================================ */

static void
ds4viz_p_slist_ws_(void *p_self)
{
    ds4viz_p_slist_t *p_l = (ds4viz_p_slist_t *)p_self;
    ds4viz_p_buf_t *p_b = &p_l->states_buf;
    int ac = 0;
    int i;
    bool first;
    const char *p_al;

    ds4viz_p_bstate_(p_b, p_l->state_id);
    ds4viz_p_bf_(p_b, "head = %d\n", p_l->head);
    for (i = 0; i < p_l->ncnt; i++)
    {
        if (p_l->p_nodes[i].alive)
        {
            ac++;
        }
    }
    if (ac == 0)
    {
        ds4viz_p_bs_(p_b, "nodes = []\n");
        return;
    }
    ds4viz_p_bs_(p_b, "nodes = [\n");
    first = true;
    for (i = 0; i < p_l->ncnt; i++)
    {
        if (!p_l->p_nodes[i].alive)
        {
            continue;
        }
        if (!first)
        {
            ds4viz_p_bs_(p_b, ",\n");
        }
        first = false;
        ds4viz_p_bf_(p_b, "  { id = %d", p_l->p_nodes[i].id);
        p_al = ds4viz_p_alias_get_(p_l, p_l->p_nodes[i].id);
        if (p_al)
        {
            ds4viz_p_bs_(p_b, ", alias = ");
            ds4viz_p_tstr_(p_b, p_al);
        }
        ds4viz_p_bs_(p_b, ", value = ");
        ds4viz_p_tval_(p_b, p_l->p_nodes[i].value);
        ds4viz_p_bf_(p_b, ", next = %d }", p_l->p_nodes[i].next);
    }
    ds4viz_p_bs_(p_b, "\n]\n");
}

static ds4viz_p_slnode_t *
ds4viz_p_slf_(ds4viz_p_slist_t *p_l, int nid)
{
    if (nid >= 0 && nid < p_l->ncnt && p_l->p_nodes[nid].alive)
    {
        return &p_l->p_nodes[nid];
    }
    return NULL;
}

static int
ds4viz_p_sla_(ds4viz_p_slist_t *p_l, ds4viz_p_val_t v)
{
    int id = p_l->nid++;
    int nc;

    if (id >= p_l->ncap)
    {
        nc = p_l->ncap ? p_l->ncap * 2 : 8;
        while (nc <= id)
        {
            nc *= 2;
        }
        p_l->p_nodes = (ds4viz_p_slnode_t *)realloc(p_l->p_nodes, (size_t)nc * sizeof(ds4viz_p_slnode_t));
        p_l->ncap = nc;
    }
    p_l->p_nodes[id] = (ds4viz_p_slnode_t){id, v, -1, true};
    p_l->ncnt = p_l->nid;
    return id;
}

ds4viz_p_slist_t
ds4viz_p_slist_open_(const char *p_label, int line)
{
    ds4viz_p_slist_t l = {0};

    l.p_label = p_label;
    l.p_kind = "slist";
    l.scope_line = line;
    l.head = -1;
    l.ws_fn = ds4viz_p_slist_ws_;
    l.err_step = -1;
    ds4viz_p_slist_ws_(&l);
    l.state_id++;
    return l;
}

void ds4viz_p_slist_close_(ds4viz_p_slist_t *p_l)
{
    int i;

    DS4VIZ_P_CLOSE_(p_l);
    for (i = 0; i < p_l->ncnt; i++)
    {
        if (p_l->p_nodes[i].alive)
        {
            ds4viz_p_vfree_(&p_l->p_nodes[i].value);
        }
    }
    free(p_l->p_nodes);
}

int ds4viz_p_slist_insert_head_(ds4viz_p_slist_t *p_l, ds4viz_p_val_t v, int line)
{
    int id;
    int bf;
    int af;
    ds4viz_p_buf_t a = {0};
    char rs[32];

    if (p_l->err)
    {
        return -1;
    }
    v = ds4viz_p_vdup_(v);
    id = ds4viz_p_sla_(p_l, v);
    p_l->p_nodes[id].next = p_l->head;
    p_l->head = id;

    bf = p_l->state_id - 1;
    ds4viz_p_slist_ws_(p_l);
    af = p_l->state_id++;

    ds4viz_p_av_(&a, "value", v);
    snprintf(rs, 32, "%d", id);
    ds4viz_p_rec_step_(p_l, "insert_head", bf, af, a.p_data, rs, line);
    ds4viz_p_bfree_(&a);
    return id;
}

int ds4viz_p_slist_insert_tail_(ds4viz_p_slist_t *p_l, ds4viz_p_val_t v, int line)
{
    int id;
    int c;
    int bf;
    int af;
    ds4viz_p_buf_t a = {0};
    char rs[32];

    if (p_l->err)
    {
        return -1;
    }
    v = ds4viz_p_vdup_(v);
    id = ds4viz_p_sla_(p_l, v);
    if (p_l->head < 0)
    {
        p_l->head = id;
    }
    else
    {
        c = p_l->head;
        while (p_l->p_nodes[c].next >= 0)
        {
            c = p_l->p_nodes[c].next;
        }
        p_l->p_nodes[c].next = id;
    }

    bf = p_l->state_id - 1;
    ds4viz_p_slist_ws_(p_l);
    af = p_l->state_id++;

    ds4viz_p_av_(&a, "value", v);
    snprintf(rs, 32, "%d", id);
    ds4viz_p_rec_step_(p_l, "insert_tail", bf, af, a.p_data, rs, line);
    ds4viz_p_bfree_(&a);
    return id;
}

int ds4viz_p_slist_insert_after_(ds4viz_p_slist_t *p_l, int nid, ds4viz_p_val_t v, int line)
{
    ds4viz_p_slnode_t *p_n;
    int id;
    int bf;
    int af;
    ds4viz_p_buf_t a = {0};
    char rs[32];

    if (p_l->err)
    {
        return -1;
    }
    p_n = ds4viz_p_slf_(p_l, nid);
    if (!p_n)
    {
        ds4viz_p_ai_(&a, "node_id", nid);
        ds4viz_p_rec_step_(p_l, "insert_after", p_l->state_id - 1, -1, a.p_data, NULL, line);
        ds4viz_p_bfree_(&a);
        DS4VIZ_P_ERR_(p_l, line, "Node not found: %d", nid);
        return -1;
    }
    v = ds4viz_p_vdup_(v);
    id = ds4viz_p_sla_(p_l, v);
    p_l->p_nodes[id].next = p_n->next;
    p_n->next = id;

    bf = p_l->state_id - 1;
    ds4viz_p_slist_ws_(p_l);
    af = p_l->state_id++;

    ds4viz_p_ai_(&a, "node_id", nid);
    ds4viz_p_av_(&a, "value", v);
    snprintf(rs, 32, "%d", id);
    ds4viz_p_rec_step_(p_l, "insert_after", bf, af, a.p_data, rs, line);
    ds4viz_p_bfree_(&a);
    return id;
}

void ds4viz_p_slist_delete_(ds4viz_p_slist_t *p_l, int nid, int line)
{
    ds4viz_p_slnode_t *p_n;
    ds4viz_p_val_t dv;
    int c;
    int bf;
    int af;
    int i;
    ds4viz_p_buf_t a = {0};
    char rs[256];

    if (p_l->err)
    {
        return;
    }
    p_n = ds4viz_p_slf_(p_l, nid);
    if (!p_n)
    {
        ds4viz_p_ai_(&a, "node_id", nid);
        ds4viz_p_rec_step_(p_l, "delete", p_l->state_id - 1, -1, a.p_data, NULL, line);
        ds4viz_p_bfree_(&a);
        DS4VIZ_P_ERR_(p_l, line, "Node not found: %d", nid);
        return;
    }
    dv = p_n->value;
    if (p_l->head == nid)
    {
        p_l->head = p_n->next;
    }
    else
    {
        c = p_l->head;
        while (c >= 0 && p_l->p_nodes[c].next != nid)
        {
            c = p_l->p_nodes[c].next;
        }
        if (c >= 0)
        {
            p_l->p_nodes[c].next = p_n->next;
        }
    }
    p_n->alive = false;
    for (i = 0; i < p_l->alias_cnt; i++)
    {
        if (p_l->p_aliases[i].id == nid)
        {
            p_l->p_aliases[i] = p_l->p_aliases[--p_l->alias_cnt];
            break;
        }
    }

    bf = p_l->state_id - 1;
    ds4viz_p_slist_ws_(p_l);
    af = p_l->state_id++;

    ds4viz_p_ai_(&a, "node_id", nid);
    ds4viz_p_vstr_(rs, sizeof rs, dv);
    ds4viz_p_rec_step_(p_l, "delete", bf, af, a.p_data, rs, line);
    ds4viz_p_bfree_(&a);
    ds4viz_p_vfree_(&dv);
}

void ds4viz_p_slist_delete_head_(ds4viz_p_slist_t *p_l, int line)
{
    int oh;
    ds4viz_p_val_t dv;
    int bf;
    int af;
    int i;
    char rs[256];

    if (p_l->err)
    {
        return;
    }
    if (p_l->head < 0)
    {
        ds4viz_p_rec_step_(p_l, "delete_head", p_l->state_id - 1, -1, NULL, NULL, line);
        DS4VIZ_P_ERR_(p_l, line, "Cannot delete from empty list");
        return;
    }
    oh = p_l->head;
    dv = p_l->p_nodes[oh].value;
    p_l->head = p_l->p_nodes[oh].next;
    p_l->p_nodes[oh].alive = false;
    for (i = 0; i < p_l->alias_cnt; i++)
    {
        if (p_l->p_aliases[i].id == oh)
        {
            p_l->p_aliases[i] = p_l->p_aliases[--p_l->alias_cnt];
            break;
        }
    }

    bf = p_l->state_id - 1;
    ds4viz_p_slist_ws_(p_l);
    af = p_l->state_id++;

    ds4viz_p_vstr_(rs, sizeof rs, dv);
    ds4viz_p_rec_step_(p_l, "delete_head", bf, af, NULL, rs, line);
    ds4viz_p_vfree_(&dv);
}

void ds4viz_p_slist_reverse_(ds4viz_p_slist_t *p_l, int line)
{
    int prev;
    int cur;
    int nxt;
    int bf;
    int af;

    if (p_l->err)
    {
        return;
    }
    prev = -1;
    cur = p_l->head;
    while (cur >= 0)
    {
        nxt = p_l->p_nodes[cur].next;
        p_l->p_nodes[cur].next = prev;
        prev = cur;
        cur = nxt;
    }
    p_l->head = prev;

    bf = p_l->state_id - 1;
    ds4viz_p_slist_ws_(p_l);
    af = p_l->state_id++;
    ds4viz_p_rec_step_(p_l, "reverse", bf, af, NULL, NULL, line);
}

/* ================================================================
 * 双向链表
 * ================================================================ */

static void
ds4viz_p_dlist_ws_(void *p_self)
{
    ds4viz_p_dlist_t *p_l = (ds4viz_p_dlist_t *)p_self;
    ds4viz_p_buf_t *p_b = &p_l->states_buf;
    int ac = 0;
    int i;
    bool first;
    const char *p_al;

    ds4viz_p_bstate_(p_b, p_l->state_id);
    ds4viz_p_bf_(p_b, "head = %d\ntail = %d\n", p_l->head, p_l->tail);
    for (i = 0; i < p_l->ncnt; i++)
    {
        if (p_l->p_nodes[i].alive)
        {
            ac++;
        }
    }
    if (ac == 0)
    {
        ds4viz_p_bs_(p_b, "nodes = []\n");
        return;
    }
    ds4viz_p_bs_(p_b, "nodes = [\n");
    first = true;
    for (i = 0; i < p_l->ncnt; i++)
    {
        if (!p_l->p_nodes[i].alive)
        {
            continue;
        }
        if (!first)
        {
            ds4viz_p_bs_(p_b, ",\n");
        }
        first = false;
        ds4viz_p_bf_(p_b, "  { id = %d", p_l->p_nodes[i].id);
        p_al = ds4viz_p_alias_get_(p_l, p_l->p_nodes[i].id);
        if (p_al)
        {
            ds4viz_p_bs_(p_b, ", alias = ");
            ds4viz_p_tstr_(p_b, p_al);
        }
        ds4viz_p_bs_(p_b, ", value = ");
        ds4viz_p_tval_(p_b, p_l->p_nodes[i].value);
        ds4viz_p_bf_(p_b, ", prev = %d, next = %d }", p_l->p_nodes[i].prev, p_l->p_nodes[i].next);
    }
    ds4viz_p_bs_(p_b, "\n]\n");
}

static ds4viz_p_dlnode_t *
ds4viz_p_dlf_(ds4viz_p_dlist_t *p_l, int nid)
{
    if (nid >= 0 && nid < p_l->ncnt && p_l->p_nodes[nid].alive)
    {
        return &p_l->p_nodes[nid];
    }
    return NULL;
}

static int
ds4viz_p_dla_(ds4viz_p_dlist_t *p_l, ds4viz_p_val_t v)
{
    int id = p_l->nid++;
    int nc;

    if (id >= p_l->ncap)
    {
        nc = p_l->ncap ? p_l->ncap * 2 : 8;
        while (nc <= id)
        {
            nc *= 2;
        }
        p_l->p_nodes = (ds4viz_p_dlnode_t *)realloc(p_l->p_nodes, (size_t)nc * sizeof(ds4viz_p_dlnode_t));
        p_l->ncap = nc;
    }
    p_l->p_nodes[id] = (ds4viz_p_dlnode_t){id, v, -1, -1, true};
    p_l->ncnt = p_l->nid;
    return id;
}

ds4viz_p_dlist_t
ds4viz_p_dlist_open_(const char *p_label, int line)
{
    ds4viz_p_dlist_t l = {0};

    l.p_label = p_label;
    l.p_kind = "dlist";
    l.scope_line = line;
    l.head = l.tail = -1;
    l.ws_fn = ds4viz_p_dlist_ws_;
    l.err_step = -1;
    ds4viz_p_dlist_ws_(&l);
    l.state_id++;
    return l;
}

void ds4viz_p_dlist_close_(ds4viz_p_dlist_t *p_l)
{
    int i;

    DS4VIZ_P_CLOSE_(p_l);
    for (i = 0; i < p_l->ncnt; i++)
    {
        if (p_l->p_nodes[i].alive)
        {
            ds4viz_p_vfree_(&p_l->p_nodes[i].value);
        }
    }
    free(p_l->p_nodes);
}

int ds4viz_p_dlist_insert_head_(ds4viz_p_dlist_t *p_l, ds4viz_p_val_t v, int line)
{
    int id, bf, af;
    ds4viz_p_buf_t a = {0};
    char rs[32];

    if (p_l->err)
    {
        return -1;
    }
    v = ds4viz_p_vdup_(v);
    id = ds4viz_p_dla_(p_l, v);
    p_l->p_nodes[id].next = p_l->head;
    if (p_l->head >= 0)
    {
        p_l->p_nodes[p_l->head].prev = id;
    }
    p_l->head = id;
    if (p_l->tail < 0)
    {
        p_l->tail = id;
    }

    bf = p_l->state_id - 1;
    ds4viz_p_dlist_ws_(p_l);
    af = p_l->state_id++;

    ds4viz_p_av_(&a, "value", v);
    snprintf(rs, 32, "%d", id);
    ds4viz_p_rec_step_(p_l, "insert_head", bf, af, a.p_data, rs, line);
    ds4viz_p_bfree_(&a);
    return id;
}

int ds4viz_p_dlist_insert_tail_(ds4viz_p_dlist_t *p_l, ds4viz_p_val_t v, int line)
{
    int id, bf, af;
    ds4viz_p_buf_t a = {0};
    char rs[32];

    if (p_l->err)
    {
        return -1;
    }
    v = ds4viz_p_vdup_(v);
    id = ds4viz_p_dla_(p_l, v);
    p_l->p_nodes[id].prev = p_l->tail;
    if (p_l->tail >= 0)
    {
        p_l->p_nodes[p_l->tail].next = id;
    }
    p_l->tail = id;
    if (p_l->head < 0)
    {
        p_l->head = id;
    }

    bf = p_l->state_id - 1;
    ds4viz_p_dlist_ws_(p_l);
    af = p_l->state_id++;

    ds4viz_p_av_(&a, "value", v);
    snprintf(rs, 32, "%d", id);
    ds4viz_p_rec_step_(p_l, "insert_tail", bf, af, a.p_data, rs, line);
    ds4viz_p_bfree_(&a);
    return id;
}

int ds4viz_p_dlist_insert_before_(ds4viz_p_dlist_t *p_l, int nid, ds4viz_p_val_t v, int line)
{
    ds4viz_p_dlnode_t *p_n;
    int id, bf, af;
    ds4viz_p_buf_t a = {0};
    char rs[32];

    if (p_l->err)
    {
        return -1;
    }
    p_n = ds4viz_p_dlf_(p_l, nid);
    if (!p_n)
    {
        ds4viz_p_ai_(&a, "node_id", nid);
        ds4viz_p_rec_step_(p_l, "insert_before", p_l->state_id - 1, -1, a.p_data, NULL, line);
        ds4viz_p_bfree_(&a);
        DS4VIZ_P_ERR_(p_l, line, "Node not found: %d", nid);
        return -1;
    }
    v = ds4viz_p_vdup_(v);
    id = ds4viz_p_dla_(p_l, v);
    p_l->p_nodes[id].next = nid;
    p_l->p_nodes[id].prev = p_n->prev;
    if (p_n->prev >= 0)
    {
        p_l->p_nodes[p_n->prev].next = id;
    }
    else
    {
        p_l->head = id;
    }
    p_n->prev = id;

    bf = p_l->state_id - 1;
    ds4viz_p_dlist_ws_(p_l);
    af = p_l->state_id++;

    ds4viz_p_ai_(&a, "node_id", nid);
    ds4viz_p_av_(&a, "value", v);
    snprintf(rs, 32, "%d", id);
    ds4viz_p_rec_step_(p_l, "insert_before", bf, af, a.p_data, rs, line);
    ds4viz_p_bfree_(&a);
    return id;
}

int ds4viz_p_dlist_insert_after_(ds4viz_p_dlist_t *p_l, int nid, ds4viz_p_val_t v, int line)
{
    ds4viz_p_dlnode_t *p_n;
    int id, bf, af;
    ds4viz_p_buf_t a = {0};
    char rs[32];

    if (p_l->err)
    {
        return -1;
    }
    p_n = ds4viz_p_dlf_(p_l, nid);
    if (!p_n)
    {
        ds4viz_p_ai_(&a, "node_id", nid);
        ds4viz_p_rec_step_(p_l, "insert_after", p_l->state_id - 1, -1, a.p_data, NULL, line);
        ds4viz_p_bfree_(&a);
        DS4VIZ_P_ERR_(p_l, line, "Node not found: %d", nid);
        return -1;
    }
    v = ds4viz_p_vdup_(v);
    id = ds4viz_p_dla_(p_l, v);
    p_l->p_nodes[id].prev = nid;
    p_l->p_nodes[id].next = p_n->next;
    if (p_n->next >= 0)
    {
        p_l->p_nodes[p_n->next].prev = id;
    }
    else
    {
        p_l->tail = id;
    }
    p_n->next = id;

    bf = p_l->state_id - 1;
    ds4viz_p_dlist_ws_(p_l);
    af = p_l->state_id++;

    ds4viz_p_ai_(&a, "node_id", nid);
    ds4viz_p_av_(&a, "value", v);
    snprintf(rs, 32, "%d", id);
    ds4viz_p_rec_step_(p_l, "insert_after", bf, af, a.p_data, rs, line);
    ds4viz_p_bfree_(&a);
    return id;
}

void ds4viz_p_dlist_delete_(ds4viz_p_dlist_t *p_l, int nid, int line)
{
    ds4viz_p_dlnode_t *p_n;
    ds4viz_p_val_t dv;
    int bf, af, i;
    ds4viz_p_buf_t a = {0};
    char rs[256];

    if (p_l->err)
    {
        return;
    }
    p_n = ds4viz_p_dlf_(p_l, nid);
    if (!p_n)
    {
        ds4viz_p_ai_(&a, "node_id", nid);
        ds4viz_p_rec_step_(p_l, "delete", p_l->state_id - 1, -1, a.p_data, NULL, line);
        ds4viz_p_bfree_(&a);
        DS4VIZ_P_ERR_(p_l, line, "Node not found: %d", nid);
        return;
    }
    dv = p_n->value;
    if (p_n->prev >= 0)
    {
        p_l->p_nodes[p_n->prev].next = p_n->next;
    }
    else
    {
        p_l->head = p_n->next;
    }
    if (p_n->next >= 0)
    {
        p_l->p_nodes[p_n->next].prev = p_n->prev;
    }
    else
    {
        p_l->tail = p_n->prev;
    }
    p_n->alive = false;
    for (i = 0; i < p_l->alias_cnt; i++)
    {
        if (p_l->p_aliases[i].id == nid)
        {
            p_l->p_aliases[i] = p_l->p_aliases[--p_l->alias_cnt];
            break;
        }
    }

    bf = p_l->state_id - 1;
    ds4viz_p_dlist_ws_(p_l);
    af = p_l->state_id++;

    ds4viz_p_ai_(&a, "node_id", nid);
    ds4viz_p_vstr_(rs, sizeof rs, dv);
    ds4viz_p_rec_step_(p_l, "delete", bf, af, a.p_data, rs, line);
    ds4viz_p_bfree_(&a);
    ds4viz_p_vfree_(&dv);
}

void ds4viz_p_dlist_delete_head_(ds4viz_p_dlist_t *p_l, int line)
{
    if (p_l->err)
    {
        return;
    }
    if (p_l->head < 0)
    {
        ds4viz_p_rec_step_(p_l, "delete_head", p_l->state_id - 1, -1, NULL, NULL, line);
        DS4VIZ_P_ERR_(p_l, line, "Cannot delete from empty list");
        return;
    }
    ds4viz_p_dlist_delete_(p_l, p_l->head, line);
}

void ds4viz_p_dlist_delete_tail_(ds4viz_p_dlist_t *p_l, int line)
{
    if (p_l->err)
    {
        return;
    }
    if (p_l->tail < 0)
    {
        ds4viz_p_rec_step_(p_l, "delete_tail", p_l->state_id - 1, -1, NULL, NULL, line);
        DS4VIZ_P_ERR_(p_l, line, "Cannot delete from empty list");
        return;
    }
    ds4viz_p_dlist_delete_(p_l, p_l->tail, line);
}

void ds4viz_p_dlist_reverse_(ds4viz_p_dlist_t *p_l, int line)
{
    int cur, nxt, tmp_int, bf, af;
    ds4viz_p_dlnode_t *p_n;

    if (p_l->err)
    {
        return;
    }
    cur = p_l->head;
    while (cur >= 0)
    {
        p_n = &p_l->p_nodes[cur];
        nxt = p_n->next;
        tmp_int = p_n->prev;
        p_n->prev = p_n->next;
        p_n->next = tmp_int;
        cur = nxt;
    }
    tmp_int = p_l->head;
    p_l->head = p_l->tail;
    p_l->tail = tmp_int;

    bf = p_l->state_id - 1;
    ds4viz_p_dlist_ws_(p_l);
    af = p_l->state_id++;
    ds4viz_p_rec_step_(p_l, "reverse", bf, af, NULL, NULL, line);
}

/* ================================================================
 * 二叉树 / BST
 * ================================================================ */

static void
ds4viz_p_bt_ws_(void *p_self)
{
    ds4viz_p_bt_t *p_t = (ds4viz_p_bt_t *)p_self;
    ds4viz_p_buf_t *p_b = &p_t->states_buf;
    int ac = 0;
    int i;
    bool first;
    const char *p_al;

    ds4viz_p_bstate_(p_b, p_t->state_id);
    ds4viz_p_bf_(p_b, "root = %d\n", p_t->root);
    for (i = 0; i < p_t->ncnt; i++)
    {
        if (p_t->p_nodes[i].alive)
        {
            ac++;
        }
    }
    if (ac == 0)
    {
        ds4viz_p_bs_(p_b, "nodes = []\n");
        return;
    }
    ds4viz_p_bs_(p_b, "nodes = [\n");
    first = true;
    for (i = 0; i < p_t->ncnt; i++)
    {
        if (!p_t->p_nodes[i].alive)
        {
            continue;
        }
        if (!first)
        {
            ds4viz_p_bs_(p_b, ",\n");
        }
        first = false;
        ds4viz_p_bf_(p_b, "  { id = %d", p_t->p_nodes[i].id);
        p_al = ds4viz_p_alias_get_(p_t, p_t->p_nodes[i].id);
        if (p_al)
        {
            ds4viz_p_bs_(p_b, ", alias = ");
            ds4viz_p_tstr_(p_b, p_al);
        }
        ds4viz_p_bs_(p_b, ", value = ");
        ds4viz_p_tval_(p_b, p_t->p_nodes[i].value);
        ds4viz_p_bf_(p_b, ", left = %d, right = %d }", p_t->p_nodes[i].left, p_t->p_nodes[i].right);
    }
    ds4viz_p_bs_(p_b, "\n]\n");
}

static ds4viz_p_tnode_t *
ds4viz_p_btf_(ds4viz_p_bt_t *p_t, int nid)
{
    if (nid >= 0 && nid < p_t->ncnt && p_t->p_nodes[nid].alive)
    {
        return &p_t->p_nodes[nid];
    }
    return NULL;
}

static int
ds4viz_p_bta_(ds4viz_p_bt_t *p_t, ds4viz_p_val_t v)
{
    int id = p_t->nid++;
    int nc;

    if (id >= p_t->ncap)
    {
        nc = p_t->ncap ? p_t->ncap * 2 : 8;
        while (nc <= id)
        {
            nc *= 2;
        }
        p_t->p_nodes = (ds4viz_p_tnode_t *)realloc(p_t->p_nodes, (size_t)nc * sizeof(ds4viz_p_tnode_t));
        p_t->ncap = nc;
    }
    p_t->p_nodes[id] = (ds4viz_p_tnode_t){id, v, -1, -1, true};
    p_t->ncnt = p_t->nid;
    return id;
}

static int
ds4viz_p_bt_fp_(ds4viz_p_bt_t *p_t, int nid)
{
    int i;
    for (i = 0; i < p_t->ncnt; i++)
    {
        if (p_t->p_nodes[i].alive && (p_t->p_nodes[i].left == nid || p_t->p_nodes[i].right == nid))
        {
            return i;
        }
    }
    return -1;
}

static void
ds4viz_p_bt_cs_(ds4viz_p_bt_t *p_t, int nid, int *p_ids, int *p_cnt)
{
    if (nid < 0 || nid >= p_t->ncnt || !p_t->p_nodes[nid].alive)
    {
        return;
    }
    p_ids[(*p_cnt)++] = nid;
    ds4viz_p_bt_cs_(p_t, p_t->p_nodes[nid].left, p_ids, p_cnt);
    ds4viz_p_bt_cs_(p_t, p_t->p_nodes[nid].right, p_ids, p_cnt);
}

ds4viz_p_bt_t
ds4viz_p_bt_open_(const char *p_kind, const char *p_label, int line)
{
    ds4viz_p_bt_t t = {0};

    t.p_label = p_label;
    t.p_kind = p_kind;
    t.scope_line = line;
    t.root = -1;
    t.ws_fn = ds4viz_p_bt_ws_;
    t.err_step = -1;
    ds4viz_p_bt_ws_(&t);
    t.state_id++;
    return t;
}

void ds4viz_p_bt_close_(ds4viz_p_bt_t *p_t)
{
    int i;
    DS4VIZ_P_CLOSE_(p_t);
    for (i = 0; i < p_t->ncnt; i++)
    {
        if (p_t->p_nodes[i].alive)
        {
            ds4viz_p_vfree_(&p_t->p_nodes[i].value);
        }
    }
    free(p_t->p_nodes);
}

int ds4viz_p_bt_insert_root_(ds4viz_p_bt_t *p_t, ds4viz_p_val_t v, int line)
{
    int id, bf, af;
    ds4viz_p_buf_t a = {0};
    char rs[32];

    if (p_t->err)
    {
        return -1;
    }
    if (p_t->root >= 0)
    {
        ds4viz_p_av_(&a, "value", v);
        ds4viz_p_rec_step_(p_t, "insert_root", p_t->state_id - 1, -1, a.p_data, NULL, line);
        ds4viz_p_bfree_(&a);
        DS4VIZ_P_ERR_(p_t, line, "Root node already exists");
        return -1;
    }
    v = ds4viz_p_vdup_(v);
    id = ds4viz_p_bta_(p_t, v);
    p_t->root = id;

    bf = p_t->state_id - 1;
    ds4viz_p_bt_ws_(p_t);
    af = p_t->state_id++;

    ds4viz_p_av_(&a, "value", v);
    snprintf(rs, 32, "%d", id);
    ds4viz_p_rec_step_(p_t, "insert_root", bf, af, a.p_data, rs, line);
    ds4viz_p_bfree_(&a);
    return id;
}

int ds4viz_p_bt_insert_left_(ds4viz_p_bt_t *p_t, int pid, ds4viz_p_val_t v, int line)
{
    ds4viz_p_tnode_t *p_p;
    int id, bf, af;
    ds4viz_p_buf_t a = {0};
    char rs[32];

    if (p_t->err)
    {
        return -1;
    }
    p_p = ds4viz_p_btf_(p_t, pid);
    if (!p_p)
    {
        ds4viz_p_ai_(&a, "parent", pid);
        ds4viz_p_rec_step_(p_t, "insert_left", p_t->state_id - 1, -1, a.p_data, NULL, line);
        ds4viz_p_bfree_(&a);
        DS4VIZ_P_ERR_(p_t, line, "Parent node not found: %d", pid);
        return -1;
    }
    if (p_p->left >= 0)
    {
        ds4viz_p_ai_(&a, "parent", pid);
        ds4viz_p_rec_step_(p_t, "insert_left", p_t->state_id - 1, -1, a.p_data, NULL, line);
        ds4viz_p_bfree_(&a);
        DS4VIZ_P_ERR_(p_t, line, "Left child already exists for node: %d", pid);
        return -1;
    }
    v = ds4viz_p_vdup_(v);
    id = ds4viz_p_bta_(p_t, v);
    p_p->left = id;

    bf = p_t->state_id - 1;
    ds4viz_p_bt_ws_(p_t);
    af = p_t->state_id++;

    ds4viz_p_ai_(&a, "parent", pid);
    ds4viz_p_av_(&a, "value", v);
    snprintf(rs, 32, "%d", id);
    ds4viz_p_rec_step_(p_t, "insert_left", bf, af, a.p_data, rs, line);
    ds4viz_p_bfree_(&a);
    return id;
}

int ds4viz_p_bt_insert_right_(ds4viz_p_bt_t *p_t, int pid, ds4viz_p_val_t v, int line)
{
    ds4viz_p_tnode_t *p_p;
    int id, bf, af;
    ds4viz_p_buf_t a = {0};
    char rs[32];

    if (p_t->err)
    {
        return -1;
    }
    p_p = ds4viz_p_btf_(p_t, pid);
    if (!p_p)
    {
        ds4viz_p_ai_(&a, "parent", pid);
        ds4viz_p_rec_step_(p_t, "insert_right", p_t->state_id - 1, -1, a.p_data, NULL, line);
        ds4viz_p_bfree_(&a);
        DS4VIZ_P_ERR_(p_t, line, "Parent node not found: %d", pid);
        return -1;
    }
    if (p_p->right >= 0)
    {
        ds4viz_p_ai_(&a, "parent", pid);
        ds4viz_p_rec_step_(p_t, "insert_right", p_t->state_id - 1, -1, a.p_data, NULL, line);
        ds4viz_p_bfree_(&a);
        DS4VIZ_P_ERR_(p_t, line, "Right child already exists for node: %d", pid);
        return -1;
    }
    v = ds4viz_p_vdup_(v);
    id = ds4viz_p_bta_(p_t, v);
    p_p->right = id;

    bf = p_t->state_id - 1;
    ds4viz_p_bt_ws_(p_t);
    af = p_t->state_id++;

    ds4viz_p_ai_(&a, "parent", pid);
    ds4viz_p_av_(&a, "value", v);
    snprintf(rs, 32, "%d", id);
    ds4viz_p_rec_step_(p_t, "insert_right", bf, af, a.p_data, rs, line);
    ds4viz_p_bfree_(&a);
    return id;
}

void ds4viz_p_bt_delete_(ds4viz_p_bt_t *p_t, int nid, int line)
{
    int *p_si;
    int sc = 0;
    int par, bf, af, i, j, sid;
    ds4viz_p_buf_t a = {0};

    if (p_t->err)
    {
        return;
    }
    if (!ds4viz_p_btf_(p_t, nid))
    {
        ds4viz_p_ai_(&a, "node_id", nid);
        ds4viz_p_rec_step_(p_t, "delete", p_t->state_id - 1, -1, a.p_data, NULL, line);
        ds4viz_p_bfree_(&a);
        DS4VIZ_P_ERR_(p_t, line, "Node not found: %d", nid);
        return;
    }
    p_si = (int *)malloc((size_t)p_t->ncnt * sizeof(int));
    ds4viz_p_bt_cs_(p_t, nid, p_si, &sc);
    par = ds4viz_p_bt_fp_(p_t, nid);
    if (par >= 0)
    {
        if (p_t->p_nodes[par].left == nid)
        {
            p_t->p_nodes[par].left = -1;
        }
        if (p_t->p_nodes[par].right == nid)
        {
            p_t->p_nodes[par].right = -1;
        }
    }
    else
    {
        p_t->root = -1;
    }
    for (i = 0; i < sc; i++)
    {
        sid = p_si[i];
        p_t->p_nodes[sid].alive = false;
        ds4viz_p_vfree_(&p_t->p_nodes[sid].value);
        for (j = 0; j < p_t->alias_cnt; j++)
        {
            if (p_t->p_aliases[j].id == sid)
            {
                p_t->p_aliases[j] = p_t->p_aliases[--p_t->alias_cnt];
                break;
            }
        }
    }
    free(p_si);

    bf = p_t->state_id - 1;
    ds4viz_p_bt_ws_(p_t);
    af = p_t->state_id++;

    ds4viz_p_ai_(&a, "node_id", nid);
    ds4viz_p_rec_step_(p_t, "delete", bf, af, a.p_data, NULL, line);
    ds4viz_p_bfree_(&a);
}

void ds4viz_p_bt_update_value_(ds4viz_p_bt_t *p_t, int nid, ds4viz_p_val_t v, int line)
{
    ds4viz_p_tnode_t *p_n;
    ds4viz_p_val_t old;
    int bf, af;
    ds4viz_p_buf_t a = {0};
    char rs[256];

    if (p_t->err)
    {
        return;
    }
    p_n = ds4viz_p_btf_(p_t, nid);
    if (!p_n)
    {
        ds4viz_p_ai_(&a, "node_id", nid);
        ds4viz_p_rec_step_(p_t, "update_value", p_t->state_id - 1, -1, a.p_data, NULL, line);
        ds4viz_p_bfree_(&a);
        DS4VIZ_P_ERR_(p_t, line, "Node not found: %d", nid);
        return;
    }
    v = ds4viz_p_vdup_(v);
    old = p_n->value;
    p_n->value = v;

    bf = p_t->state_id - 1;
    ds4viz_p_bt_ws_(p_t);
    af = p_t->state_id++;

    ds4viz_p_ai_(&a, "node_id", nid);
    ds4viz_p_av_(&a, "value", v);
    ds4viz_p_vstr_(rs, sizeof rs, old);
    ds4viz_p_rec_step_(p_t, "update_value", bf, af, a.p_data, rs, line);
    ds4viz_p_bfree_(&a);
    ds4viz_p_vfree_(&old);
}

/* BST */

int ds4viz_p_bst_insert_(ds4viz_p_bt_t *p_t, ds4viz_p_val_t v, int line)
{
    double val, cv_d;
    int id, c, bf, af;
    ds4viz_p_buf_t a = {0};
    char rs[32];

    if (p_t->err)
    {
        return -1;
    }
    v = ds4viz_p_vdup_(v);
    val = ds4viz_p_vnum_(v);
    id = ds4viz_p_bta_(p_t, v);
    if (p_t->root < 0)
    {
        p_t->root = id;
    }
    else
    {
        c = p_t->root;
        for (;;)
        {
            cv_d = ds4viz_p_vnum_(p_t->p_nodes[c].value);
            if (val < cv_d)
            {
                if (p_t->p_nodes[c].left < 0)
                {
                    p_t->p_nodes[c].left = id;
                    break;
                }
                c = p_t->p_nodes[c].left;
            }
            else
            {
                if (p_t->p_nodes[c].right < 0)
                {
                    p_t->p_nodes[c].right = id;
                    break;
                }
                c = p_t->p_nodes[c].right;
            }
        }
    }

    bf = p_t->state_id - 1;
    ds4viz_p_bt_ws_(p_t);
    af = p_t->state_id++;

    ds4viz_p_av_(&a, "value", v);
    snprintf(rs, 32, "%d", id);
    ds4viz_p_rec_step_(p_t, "insert", bf, af, a.p_data, rs, line);
    ds4viz_p_bfree_(&a);
    return id;
}

static int
ds4viz_p_bst_fv_(ds4viz_p_bt_t *p_t, double target)
{
    int i;
    for (i = 0; i < p_t->ncnt; i++)
    {
        if (p_t->p_nodes[i].alive && fabs(ds4viz_p_vnum_(p_t->p_nodes[i].value) - target) < 1e-12)
        {
            return i;
        }
    }
    return -1;
}

static int
ds4viz_p_bst_min_(ds4viz_p_bt_t *p_t, int nid)
{
    while (p_t->p_nodes[nid].left >= 0)
    {
        nid = p_t->p_nodes[nid].left;
    }
    return nid;
}

static void
ds4viz_p_bst_rp_(ds4viz_p_bt_t *p_t, int par, int old, int new_id)
{
    if (par < 0)
    {
        p_t->root = new_id;
    }
    else if (p_t->p_nodes[par].left == old)
    {
        p_t->p_nodes[par].left = new_id;
    }
    else
    {
        p_t->p_nodes[par].right = new_id;
    }
}

static void
ds4viz_p_bst_dn_(ds4viz_p_bt_t *p_t, int nid)
{
    ds4viz_p_tnode_t *p_n = &p_t->p_nodes[nid];
    int par = ds4viz_p_bt_fp_(p_t, nid);
    int suc;
    ds4viz_p_val_t sv;

    if (p_n->left < 0 && p_n->right < 0)
    {
        ds4viz_p_bst_rp_(p_t, par, nid, -1);
        p_n->alive = false;
        ds4viz_p_vfree_(&p_n->value);
    }
    else if (p_n->left < 0)
    {
        ds4viz_p_bst_rp_(p_t, par, nid, p_n->right);
        p_n->alive = false;
        ds4viz_p_vfree_(&p_n->value);
    }
    else if (p_n->right < 0)
    {
        ds4viz_p_bst_rp_(p_t, par, nid, p_n->left);
        p_n->alive = false;
        ds4viz_p_vfree_(&p_n->value);
    }
    else
    {
        suc = ds4viz_p_bst_min_(p_t, p_n->right);
        sv = ds4viz_p_vdup_(p_t->p_nodes[suc].value);
        ds4viz_p_bst_dn_(p_t, suc);
        ds4viz_p_vfree_(&p_t->p_nodes[nid].value);
        p_t->p_nodes[nid].value = sv;
    }
}

void ds4viz_p_bst_delete_(ds4viz_p_bt_t *p_t, ds4viz_p_val_t v, int line)
{
    double target;
    int nid, bf, af;
    ds4viz_p_buf_t a = {0};

    if (p_t->err)
    {
        return;
    }
    target = ds4viz_p_vnum_(v);
    nid = ds4viz_p_bst_fv_(p_t, target);
    if (nid < 0)
    {
        ds4viz_p_av_(&a, "value", v);
        ds4viz_p_rec_step_(p_t, "delete", p_t->state_id - 1, -1, a.p_data, NULL, line);
        ds4viz_p_bfree_(&a);
        if (v.type == DS4VIZ_P_VINT_)
        {
            DS4VIZ_P_ERR_(p_t, line, "Node with value %lld not found", v.i);
        }
        else
        {
            DS4VIZ_P_ERR_(p_t, line, "Node with value %g not found", v.d);
        }
        return;
    }
    ds4viz_p_bst_dn_(p_t, nid);

    bf = p_t->state_id - 1;
    ds4viz_p_bt_ws_(p_t);
    af = p_t->state_id++;

    ds4viz_p_av_(&a, "value", v);
    ds4viz_p_rec_step_(p_t, "delete", bf, af, a.p_data, NULL, line);
    ds4viz_p_bfree_(&a);
}

/* ================================================================
 * 图
 * ================================================================ */

static void
ds4viz_p_graph_ws_(void *p_self)
{
    ds4viz_p_graph_t *p_g = (ds4viz_p_graph_t *)p_self;
    ds4viz_p_buf_t *p_b = &p_g->states_buf;
    int an = 0, ae = 0, i, j, idx, eidx, tmp_i;
    int *p_si;
    int *p_ei;
    ds4viz_p_gnode_t *p_n;
    ds4viz_p_gedge_t *p_ea;
    ds4viz_p_gedge_t *p_eb;
    const char *p_al;

    ds4viz_p_bstate_(p_b, p_g->state_id);

    for (i = 0; i < p_g->ncnt; i++)
    {
        if (p_g->p_gnodes[i].alive)
        {
            an++;
        }
    }
    if (an == 0)
    {
        ds4viz_p_bs_(p_b, "nodes = []\n");
    }
    else
    {
        p_si = (int *)malloc((size_t)an * sizeof(int));
        idx = 0;
        for (i = 0; i < p_g->ncnt; i++)
        {
            if (p_g->p_gnodes[i].alive)
            {
                p_si[idx++] = i;
            }
        }
        for (i = 0; i < idx - 1; i++)
        {
            for (j = 0; j < idx - 1 - i; j++)
            {
                if (p_g->p_gnodes[p_si[j]].id > p_g->p_gnodes[p_si[j + 1]].id)
                {
                    tmp_i = p_si[j];
                    p_si[j] = p_si[j + 1];
                    p_si[j + 1] = tmp_i;
                }
            }
        }
        ds4viz_p_bs_(p_b, "nodes = [\n");
        for (i = 0; i < idx; i++)
        {
            if (i)
            {
                ds4viz_p_bs_(p_b, ",\n");
            }
            p_n = &p_g->p_gnodes[p_si[i]];
            ds4viz_p_bf_(p_b, "  { id = %d", p_n->id);
            p_al = ds4viz_p_alias_get_(p_g, p_n->id);
            if (p_al)
            {
                ds4viz_p_bs_(p_b, ", alias = ");
                ds4viz_p_tstr_(p_b, p_al);
            }
            ds4viz_p_bs_(p_b, ", label = ");
            ds4viz_p_tstr_(p_b, p_n->label);
            ds4viz_p_bs_(p_b, " }");
        }
        ds4viz_p_bs_(p_b, "\n]\n");
        free(p_si);
    }

    for (i = 0; i < p_g->ecnt; i++)
    {
        if (p_g->p_edges[i].alive)
        {
            ae++;
        }
    }
    if (ae == 0)
    {
        ds4viz_p_bs_(p_b, "edges = []\n");
    }
    else
    {
        p_ei = (int *)malloc((size_t)ae * sizeof(int));
        eidx = 0;
        for (i = 0; i < p_g->ecnt; i++)
        {
            if (p_g->p_edges[i].alive)
            {
                p_ei[eidx++] = i;
            }
        }
        for (i = 0; i < eidx - 1; i++)
        {
            for (j = 0; j < eidx - 1 - i; j++)
            {
                p_ea = &p_g->p_edges[p_ei[j]];
                p_eb = &p_g->p_edges[p_ei[j + 1]];
                if (p_ea->from > p_eb->from || (p_ea->from == p_eb->from && p_ea->to > p_eb->to))
                {
                    tmp_i = p_ei[j];
                    p_ei[j] = p_ei[j + 1];
                    p_ei[j + 1] = tmp_i;
                }
            }
        }
        ds4viz_p_bs_(p_b, "edges = [\n");
        for (i = 0; i < eidx; i++)
        {
            if (i)
            {
                ds4viz_p_bs_(p_b, ",\n");
            }
            ds4viz_p_bf_(p_b, "  { from = %d, to = %d", p_g->p_edges[p_ei[i]].from, p_g->p_edges[p_ei[i]].to);
            if (p_g->weighted)
            {
                ds4viz_p_bs_(p_b, ", weight = ");
                ds4viz_p_tval_(p_b, (ds4viz_p_val_t){DS4VIZ_P_VDBL_, .d = p_g->p_edges[p_ei[i]].weight});
            }
            ds4viz_p_bs_(p_b, " }");
        }
        ds4viz_p_bs_(p_b, "\n]\n");
        free(p_ei);
    }
}

static ds4viz_p_gnode_t *
ds4viz_p_gfn_(ds4viz_p_graph_t *p_g, int nid)
{
    int i;
    for (i = 0; i < p_g->ncnt; i++)
    {
        if (p_g->p_gnodes[i].alive && p_g->p_gnodes[i].id == nid)
        {
            return &p_g->p_gnodes[i];
        }
    }
    return NULL;
}

static ds4viz_p_gedge_t *
ds4viz_p_gfe_(ds4viz_p_graph_t *p_g, int from, int to)
{
    int i;
    for (i = 0; i < p_g->ecnt; i++)
    {
        if (p_g->p_edges[i].alive && p_g->p_edges[i].from == from && p_g->p_edges[i].to == to)
        {
            return &p_g->p_edges[i];
        }
    }
    return NULL;
}

ds4viz_p_graph_t
ds4viz_p_graph_open_(const char *p_kind, const char *p_label,
                     bool directed, bool weighted, int line)
{
    ds4viz_p_graph_t g = {0};

    g.p_label = p_label;
    g.p_kind = p_kind;
    g.scope_line = line;
    g.directed = directed;
    g.weighted = weighted;
    g.ws_fn = ds4viz_p_graph_ws_;
    g.err_step = -1;
    ds4viz_p_graph_ws_(&g);
    g.state_id++;
    return g;
}

void ds4viz_p_graph_close_(ds4viz_p_graph_t *p_g)
{
    DS4VIZ_P_CLOSE_(p_g);
    free(p_g->p_gnodes);
    free(p_g->p_edges);
}

void ds4viz_p_graph_add_node_(ds4viz_p_graph_t *p_g, int nid, const char *p_lbl, int line)
{
    int ll, bf, af;
    ds4viz_p_gnode_t *p_n;
    ds4viz_p_buf_t a = {0};

    if (p_g->err)
    {
        return;
    }
    if (ds4viz_p_gfn_(p_g, nid))
    {
        ds4viz_p_ai_(&a, "id", nid);
        ds4viz_p_rec_step_(p_g, "add_node", p_g->state_id - 1, -1, a.p_data, NULL, line);
        ds4viz_p_bfree_(&a);
        DS4VIZ_P_ERR_(p_g, line, "Node already exists: %d", nid);
        return;
    }
    ll = p_lbl ? (int)strlen(p_lbl) : 0;
    if (ll < 1 || ll > 32)
    {
        ds4viz_p_ai_(&a, "id", nid);
        ds4viz_p_rec_step_(p_g, "add_node", p_g->state_id - 1, -1, a.p_data, NULL, line);
        ds4viz_p_bfree_(&a);
        DS4VIZ_P_ERR_(p_g, line, "Label length must be 1-32, got %d", ll);
        return;
    }
    DS4VIZ_P_GROW_(p_g->p_gnodes, p_g->ncnt, p_g->ncap, ds4viz_p_gnode_t);
    p_n = &p_g->p_gnodes[p_g->ncnt++];
    p_n->id = nid;
    p_n->alive = true;
    strncpy(p_n->label, p_lbl, 32);
    p_n->label[32] = 0;

    bf = p_g->state_id - 1;
    ds4viz_p_graph_ws_(p_g);
    af = p_g->state_id++;

    ds4viz_p_ai_(&a, "id", nid);
    ds4viz_p_as_(&a, "label", p_lbl);
    ds4viz_p_rec_step_(p_g, "add_node", bf, af, a.p_data, NULL, line);
    ds4viz_p_bfree_(&a);
}

void ds4viz_p_graph_add_edge_(ds4viz_p_graph_t *p_g, int from, int to, double w, int line)
{
    int ef, et, tmp_i, bf, af;
    ds4viz_p_buf_t a = {0};

    if (p_g->err)
    {
        return;
    }
    if (from == to)
    {
        ds4viz_p_ai_(&a, "from", from);
        ds4viz_p_ai_(&a, "to", to);
        ds4viz_p_rec_step_(p_g, "add_edge", p_g->state_id - 1, -1, a.p_data, NULL, line);
        ds4viz_p_bfree_(&a);
        DS4VIZ_P_ERR_(p_g, line, "Self-loop not allowed: %d", from);
        return;
    }
    if (!ds4viz_p_gfn_(p_g, from))
    {
        ds4viz_p_ai_(&a, "from", from);
        ds4viz_p_rec_step_(p_g, "add_edge", p_g->state_id - 1, -1, a.p_data, NULL, line);
        ds4viz_p_bfree_(&a);
        DS4VIZ_P_ERR_(p_g, line, "Node not found: %d", from);
        return;
    }
    if (!ds4viz_p_gfn_(p_g, to))
    {
        ds4viz_p_ai_(&a, "from", from);
        ds4viz_p_ai_(&a, "to", to);
        ds4viz_p_rec_step_(p_g, "add_edge", p_g->state_id - 1, -1, a.p_data, NULL, line);
        ds4viz_p_bfree_(&a);
        DS4VIZ_P_ERR_(p_g, line, "Node not found: %d", to);
        return;
    }
    ef = from;
    et = to;
    if (!p_g->directed && ef > et)
    {
        tmp_i = ef;
        ef = et;
        et = tmp_i;
    }
    if (ds4viz_p_gfe_(p_g, ef, et))
    {
        ds4viz_p_ai_(&a, "from", from);
        ds4viz_p_ai_(&a, "to", to);
        ds4viz_p_rec_step_(p_g, "add_edge", p_g->state_id - 1, -1, a.p_data, NULL, line);
        ds4viz_p_bfree_(&a);
        DS4VIZ_P_ERR_(p_g, line, "Edge already exists: (%d, %d)", ef, et);
        return;
    }
    DS4VIZ_P_GROW_(p_g->p_edges, p_g->ecnt, p_g->ecap, ds4viz_p_gedge_t);
    p_g->p_edges[p_g->ecnt++] = (ds4viz_p_gedge_t){ef, et, w, true};

    bf = p_g->state_id - 1;
    ds4viz_p_graph_ws_(p_g);
    af = p_g->state_id++;

    ds4viz_p_ai_(&a, "from", from);
    ds4viz_p_ai_(&a, "to", to);
    if (p_g->weighted)
    {
        ds4viz_p_ad_(&a, "weight", w);
    }
    ds4viz_p_rec_step_(p_g, "add_edge", bf, af, a.p_data, NULL, line);
    ds4viz_p_bfree_(&a);
}

void ds4viz_p_graph_remove_node_(ds4viz_p_graph_t *p_g, int nid, int line)
{
    ds4viz_p_gnode_t *p_n;
    int i, bf, af;
    ds4viz_p_buf_t a = {0};

    if (p_g->err)
    {
        return;
    }
    p_n = ds4viz_p_gfn_(p_g, nid);
    if (!p_n)
    {
        ds4viz_p_ai_(&a, "node_id", nid);
        ds4viz_p_rec_step_(p_g, "remove_node", p_g->state_id - 1, -1, a.p_data, NULL, line);
        ds4viz_p_bfree_(&a);
        DS4VIZ_P_ERR_(p_g, line, "Node not found: %d", nid);
        return;
    }
    p_n->alive = false;
    for (i = 0; i < p_g->ecnt; i++)
    {
        if (p_g->p_edges[i].alive && (p_g->p_edges[i].from == nid || p_g->p_edges[i].to == nid))
        {
            p_g->p_edges[i].alive = false;
        }
    }
    for (i = 0; i < p_g->alias_cnt; i++)
    {
        if (p_g->p_aliases[i].id == nid)
        {
            p_g->p_aliases[i] = p_g->p_aliases[--p_g->alias_cnt];
            break;
        }
    }

    bf = p_g->state_id - 1;
    ds4viz_p_graph_ws_(p_g);
    af = p_g->state_id++;

    ds4viz_p_ai_(&a, "node_id", nid);
    ds4viz_p_rec_step_(p_g, "remove_node", bf, af, a.p_data, NULL, line);
    ds4viz_p_bfree_(&a);
}

void ds4viz_p_graph_remove_edge_(ds4viz_p_graph_t *p_g, int from, int to, int line)
{
    int ef, et, tmp_i, bf, af;
    ds4viz_p_gedge_t *p_e;
    ds4viz_p_buf_t a = {0};

    if (p_g->err)
    {
        return;
    }
    ef = from;
    et = to;
    if (!p_g->directed && ef > et)
    {
        tmp_i = ef;
        ef = et;
        et = tmp_i;
    }
    p_e = ds4viz_p_gfe_(p_g, ef, et);
    if (!p_e)
    {
        ds4viz_p_ai_(&a, "from", from);
        ds4viz_p_ai_(&a, "to", to);
        ds4viz_p_rec_step_(p_g, "remove_edge", p_g->state_id - 1, -1, a.p_data, NULL, line);
        ds4viz_p_bfree_(&a);
        DS4VIZ_P_ERR_(p_g, line, "Edge not found: (%d, %d)", from, to);
        return;
    }
    p_e->alive = false;

    bf = p_g->state_id - 1;
    ds4viz_p_graph_ws_(p_g);
    af = p_g->state_id++;

    ds4viz_p_ai_(&a, "from", from);
    ds4viz_p_ai_(&a, "to", to);
    ds4viz_p_rec_step_(p_g, "remove_edge", bf, af, a.p_data, NULL, line);
    ds4viz_p_bfree_(&a);
}

void ds4viz_p_graph_update_node_label_(ds4viz_p_graph_t *p_g, int nid,
                                       const char *p_lbl, int line)
{
    ds4viz_p_gnode_t *p_n;
    int ll, bf, af;
    char old[33];
    ds4viz_p_buf_t a = {0};
    ds4viz_p_buf_t rb = {0};

    if (p_g->err)
    {
        return;
    }
    p_n = ds4viz_p_gfn_(p_g, nid);
    if (!p_n)
    {
        ds4viz_p_ai_(&a, "node_id", nid);
        ds4viz_p_rec_step_(p_g, "update_node_label", p_g->state_id - 1, -1, a.p_data, NULL, line);
        ds4viz_p_bfree_(&a);
        DS4VIZ_P_ERR_(p_g, line, "Node not found: %d", nid);
        return;
    }
    ll = p_lbl ? (int)strlen(p_lbl) : 0;
    if (ll < 1 || ll > 32)
    {
        ds4viz_p_ai_(&a, "node_id", nid);
        ds4viz_p_rec_step_(p_g, "update_node_label", p_g->state_id - 1, -1, a.p_data, NULL, line);
        ds4viz_p_bfree_(&a);
        DS4VIZ_P_ERR_(p_g, line, "Label length must be 1-32, got %d", ll);
        return;
    }
    strncpy(old, p_n->label, 33);
    strncpy(p_n->label, p_lbl, 32);
    p_n->label[32] = 0;

    bf = p_g->state_id - 1;
    ds4viz_p_graph_ws_(p_g);
    af = p_g->state_id++;

    ds4viz_p_ai_(&a, "node_id", nid);
    ds4viz_p_as_(&a, "label", p_lbl);
    ds4viz_p_tstr_(&rb, old);
    ds4viz_p_rec_step_(p_g, "update_node_label", bf, af, a.p_data, rb.p_data, line);
    ds4viz_p_bfree_(&a);
    ds4viz_p_bfree_(&rb);
}

void ds4viz_p_graph_update_weight_(ds4viz_p_graph_t *p_g, int from, int to,
                                   double w, int line)
{
    int ef, et, tmp_i, bf, af;
    double old_w;
    ds4viz_p_gedge_t *p_e;
    ds4viz_p_buf_t a = {0};
    char rs[64];

    if (p_g->err)
    {
        return;
    }
    ef = from;
    et = to;
    if (!p_g->directed && ef > et)
    {
        tmp_i = ef;
        ef = et;
        et = tmp_i;
    }
    p_e = ds4viz_p_gfe_(p_g, ef, et);
    if (!p_e)
    {
        ds4viz_p_ai_(&a, "from", from);
        ds4viz_p_ai_(&a, "to", to);
        ds4viz_p_rec_step_(p_g, "update_weight", p_g->state_id - 1, -1, a.p_data, NULL, line);
        ds4viz_p_bfree_(&a);
        DS4VIZ_P_ERR_(p_g, line, "Edge not found: (%d, %d)", from, to);
        return;
    }
    old_w = p_e->weight;
    p_e->weight = w;

    bf = p_g->state_id - 1;
    ds4viz_p_graph_ws_(p_g);
    af = p_g->state_id++;

    ds4viz_p_ai_(&a, "from", from);
    ds4viz_p_ai_(&a, "to", to);
    ds4viz_p_ad_(&a, "weight", w);
    ds4viz_p_vstr_(rs, sizeof rs, (ds4viz_p_val_t){DS4VIZ_P_VDBL_, .d = old_w});
    ds4viz_p_rec_step_(p_g, "update_weight", bf, af, a.p_data, rs, line);
    ds4viz_p_bfree_(&a);
}

#endif /* DS4VIZ_IMPLEMENTATION */