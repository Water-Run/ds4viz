/*
 * ds4viz.h - Data Structure Visualization - Single Header C23 Library
 * https://github.com/Water-Run/ds4viz
 *
 * Usage:
 *     // In exactly ONE .c file:
 *     #define DS4VIZ_IMPLEMENTATION
 *     #include "ds4viz.h"
 *
 *     // In all other files that use the API:
 *     #include "ds4viz.h"
 *
 * Options:
 *     #define DS4VIZ_SHORT_NAMES   // before #include, enables dv... prefix
 */

#ifndef DS4VIZ_H
#define DS4VIZ_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>
#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ================================================================
 * PUBLIC TYPES
 * ================================================================ */

typedef struct {
    const char *output_path;
    const char *title;
    const char *author;
    const char *comment;
} ds4vizConfigOptions;

typedef enum {
    ds4vizHeapOrderMin = 0,
    ds4vizHeapOrderMax = 1
} ds4vizHeapOrder;

/* ================================================================
 * PUBLIC FUNCTION
 * ================================================================ */

void ds4vizConfig(ds4vizConfigOptions options);

/* ================================================================
 * INTERNAL HELPERS  (used by macros – do not call directly)
 * ================================================================ */

/* ---- value type ---- */
enum { DS4VIZ_VINT_=0, DS4VIZ_VDBL_=1, DS4VIZ_VSTR_=2, DS4VIZ_VBOOL_=3 };

typedef struct {
    int type;
    union { long long i; double d; const char *s; bool b; };
} ds4vizP_val;

/* ---- dynamic buffer ---- */
typedef struct { char *data; int len, cap; } ds4vizP_buf;

/* ---- common fields macro ---- */
#define DS4VIZ_COMMON_FIELDS_ \
    ds4vizP_buf states; ds4vizP_buf steps; \
    int state_id; int step_id; \
    bool err; char errmsg[256]; int err_step; int err_last_state; int err_line; \
    bool done; const char *label; const char *type_name

/* ---- stack ---- */
typedef struct { DS4VIZ_COMMON_FIELDS_; ds4vizP_val *items; int count, cap; } ds4vizP_stack;

ds4vizP_stack ds4vizP_stack_open(const char *label);
void ds4vizP_stack_close(ds4vizP_stack *s);
void ds4vizP_stack_push(ds4vizP_stack *s, ds4vizP_val v, int line);
void ds4vizP_stack_pop(ds4vizP_stack *s, int line);
void ds4vizP_stack_clear(ds4vizP_stack *s, int line);

/* ---- queue ---- */
typedef struct { DS4VIZ_COMMON_FIELDS_; ds4vizP_val *items; int count, cap; } ds4vizP_queue;

ds4vizP_queue ds4vizP_queue_open(const char *label);
void ds4vizP_queue_close(ds4vizP_queue *s);
void ds4vizP_queue_enqueue(ds4vizP_queue *s, ds4vizP_val v, int line);
void ds4vizP_queue_dequeue(ds4vizP_queue *s, int line);
void ds4vizP_queue_clear(ds4vizP_queue *s, int line);

/* ---- singly linked list ---- */
typedef struct { int id; ds4vizP_val value; int next; bool alive; } ds4vizP_slnode;
typedef struct { DS4VIZ_COMMON_FIELDS_; ds4vizP_slnode *nodes; int ncnt,ncap; int head; int nid; } ds4vizP_slist;

ds4vizP_slist ds4vizP_slist_open(const char *label);
void ds4vizP_slist_close(ds4vizP_slist *l);
int  ds4vizP_slist_insert_head(ds4vizP_slist *l, ds4vizP_val v, int line);
int  ds4vizP_slist_insert_tail(ds4vizP_slist *l, ds4vizP_val v, int line);
int  ds4vizP_slist_insert_after(ds4vizP_slist *l, int nid, ds4vizP_val v, int line);
void ds4vizP_slist_delete(ds4vizP_slist *l, int nid, int line);
void ds4vizP_slist_delete_head(ds4vizP_slist *l, int line);
void ds4vizP_slist_reverse(ds4vizP_slist *l, int line);

/* ---- doubly linked list ---- */
typedef struct { int id; ds4vizP_val value; int prev,next; bool alive; } ds4vizP_dlnode;
typedef struct { DS4VIZ_COMMON_FIELDS_; ds4vizP_dlnode *nodes; int ncnt,ncap; int head,tail; int nid; } ds4vizP_dlist;

ds4vizP_dlist ds4vizP_dlist_open(const char *label);
void ds4vizP_dlist_close(ds4vizP_dlist *l);
int  ds4vizP_dlist_insert_head(ds4vizP_dlist *l, ds4vizP_val v, int line);
int  ds4vizP_dlist_insert_tail(ds4vizP_dlist *l, ds4vizP_val v, int line);
int  ds4vizP_dlist_insert_before(ds4vizP_dlist *l, int nid, ds4vizP_val v, int line);
int  ds4vizP_dlist_insert_after(ds4vizP_dlist *l, int nid, ds4vizP_val v, int line);
void ds4vizP_dlist_delete(ds4vizP_dlist *l, int nid, int line);
void ds4vizP_dlist_delete_head(ds4vizP_dlist *l, int line);
void ds4vizP_dlist_delete_tail(ds4vizP_dlist *l, int line);
void ds4vizP_dlist_reverse(ds4vizP_dlist *l, int line);

/* ---- binary tree / bst ---- */
typedef struct { int id; ds4vizP_val value; int left,right,parent; bool alive; } ds4vizP_tnode;
typedef struct { DS4VIZ_COMMON_FIELDS_; ds4vizP_tnode *nodes; int ncnt,ncap; int root; int nid; } ds4vizP_bt;

ds4vizP_bt ds4vizP_bt_open(const char *type_nm, const char *label);
void ds4vizP_bt_close(ds4vizP_bt *t);
int  ds4vizP_bt_insert_root(ds4vizP_bt *t, ds4vizP_val v, int line);
int  ds4vizP_bt_insert_left(ds4vizP_bt *t, int pid, ds4vizP_val v, int line);
int  ds4vizP_bt_insert_right(ds4vizP_bt *t, int pid, ds4vizP_val v, int line);
void ds4vizP_bt_delete(ds4vizP_bt *t, int nid, int line);
void ds4vizP_bt_update_value(ds4vizP_bt *t, int nid, ds4vizP_val v, int line);

int  ds4vizP_bst_insert(ds4vizP_bt *t, ds4vizP_val v, int line);
void ds4vizP_bst_delete(ds4vizP_bt *t, ds4vizP_val v, int line);

/* ---- heap ---- */
typedef struct { DS4VIZ_COMMON_FIELDS_; ds4vizP_val *items; int count,cap; int order; } ds4vizP_heap;

ds4vizP_heap ds4vizP_heap_open(const char *label, int order);
void ds4vizP_heap_close(ds4vizP_heap *h);
void ds4vizP_heap_insert(ds4vizP_heap *h, ds4vizP_val v, int line);
void ds4vizP_heap_extract(ds4vizP_heap *h, int line);
void ds4vizP_heap_clear(ds4vizP_heap *h, int line);

/* ---- graph (unified) ---- */
typedef struct { int id; char label[33]; bool alive; } ds4vizP_gnode;
typedef struct { int from,to; double weight; bool alive; } ds4vizP_gedge;
typedef struct {
    DS4VIZ_COMMON_FIELDS_;
    ds4vizP_gnode *nodes; int ncnt,ncap;
    ds4vizP_gedge *edges; int ecnt,ecap;
    bool directed; bool weighted;
} ds4vizP_graph;

ds4vizP_graph ds4vizP_graph_open(const char *type_nm, const char *label, bool directed, bool weighted);
void ds4vizP_graph_close(ds4vizP_graph *g);
void ds4vizP_graph_add_node(ds4vizP_graph *g, int nid, const char *lbl, int line);
void ds4vizP_graph_add_edge(ds4vizP_graph *g, int from, int to, double w, int line);
void ds4vizP_graph_remove_node(ds4vizP_graph *g, int nid, int line);
void ds4vizP_graph_remove_edge(ds4vizP_graph *g, int from, int to, int line);
void ds4vizP_graph_update_node_label(ds4vizP_graph *g, int nid, const char *lbl, int line);
void ds4vizP_graph_update_weight(ds4vizP_graph *g, int from, int to, double w, int line);

/* ================================================================
 * INTERNAL MACROS
 * ================================================================ */

/* ---- value conversion ---- */
#define DS4VIZ_VAL_(x) _Generic((x), \
    bool:               (ds4vizP_val){.type=DS4VIZ_VBOOL_, .b=(x)}, \
    char:               (ds4vizP_val){.type=DS4VIZ_VINT_,  .i=(long long)(x)}, \
    signed char:        (ds4vizP_val){.type=DS4VIZ_VINT_,  .i=(long long)(x)}, \
    unsigned char:      (ds4vizP_val){.type=DS4VIZ_VINT_,  .i=(long long)(x)}, \
    short:              (ds4vizP_val){.type=DS4VIZ_VINT_,  .i=(long long)(x)}, \
    unsigned short:     (ds4vizP_val){.type=DS4VIZ_VINT_,  .i=(long long)(x)}, \
    int:                (ds4vizP_val){.type=DS4VIZ_VINT_,  .i=(long long)(x)}, \
    unsigned int:       (ds4vizP_val){.type=DS4VIZ_VINT_,  .i=(long long)(x)}, \
    long:               (ds4vizP_val){.type=DS4VIZ_VINT_,  .i=(long long)(x)}, \
    unsigned long:      (ds4vizP_val){.type=DS4VIZ_VINT_,  .i=(long long)(x)}, \
    long long:          (ds4vizP_val){.type=DS4VIZ_VINT_,  .i=(long long)(x)}, \
    unsigned long long: (ds4vizP_val){.type=DS4VIZ_VINT_,  .i=(long long)(x)}, \
    float:              (ds4vizP_val){.type=DS4VIZ_VDBL_,  .d=(double)(x)}, \
    double:             (ds4vizP_val){.type=DS4VIZ_VDBL_,  .d=(double)(x)}, \
    char*:              (ds4vizP_val){.type=DS4VIZ_VSTR_,  .s=(const char*)(x)}, \
    const char*:        (ds4vizP_val){.type=DS4VIZ_VSTR_,  .s=(x)} \
)

#define DS4VIZ_NUMVAL_(x) _Generic((x), \
    char:               (ds4vizP_val){.type=DS4VIZ_VINT_, .i=(long long)(x)}, \
    signed char:        (ds4vizP_val){.type=DS4VIZ_VINT_, .i=(long long)(x)}, \
    unsigned char:      (ds4vizP_val){.type=DS4VIZ_VINT_, .i=(long long)(x)}, \
    short:              (ds4vizP_val){.type=DS4VIZ_VINT_, .i=(long long)(x)}, \
    unsigned short:     (ds4vizP_val){.type=DS4VIZ_VINT_, .i=(long long)(x)}, \
    int:                (ds4vizP_val){.type=DS4VIZ_VINT_, .i=(long long)(x)}, \
    unsigned int:       (ds4vizP_val){.type=DS4VIZ_VINT_, .i=(long long)(x)}, \
    long:               (ds4vizP_val){.type=DS4VIZ_VINT_, .i=(long long)(x)}, \
    unsigned long:      (ds4vizP_val){.type=DS4VIZ_VINT_, .i=(long long)(x)}, \
    long long:          (ds4vizP_val){.type=DS4VIZ_VINT_, .i=(long long)(x)}, \
    unsigned long long: (ds4vizP_val){.type=DS4VIZ_VINT_, .i=(long long)(x)}, \
    float:              (ds4vizP_val){.type=DS4VIZ_VDBL_, .d=(double)(x)}, \
    double:             (ds4vizP_val){.type=DS4VIZ_VDBL_, .d=(double)(x)} \
)

/* ---- optional argument selectors (C23 __VA_OPT__) ---- */
#define DS4VIZ_OPT1_(def, ...) DS4VIZ_PICK1_(__VA_ARGS__ __VA_OPT__(,) def)
#define DS4VIZ_PICK1_(a, ...) a

#define DS4VIZ_OPT2_(def, ...) DS4VIZ_PICK2_(__VA_ARGS__ __VA_OPT__(,) def, def)
#define DS4VIZ_PICK2_(a, b, ...) b

/* ================================================================
 * SCOPE MACROS  (public API)
 * ================================================================ */

/* ---- stack ---- */
#define ds4vizStack(s, ...) \
    for (ds4vizP_stack s = ds4vizP_stack_open( \
            DS4VIZ_OPT1_("stack" __VA_OPT__(,) __VA_ARGS__)); \
         !s.done; ds4vizP_stack_close(&s))

#define ds4vizStackPush(s, v)  ds4vizP_stack_push(&(s), DS4VIZ_VAL_(v), __LINE__)
#define ds4vizStackPop(s)      ds4vizP_stack_pop(&(s), __LINE__)
#define ds4vizStackClear(s)    ds4vizP_stack_clear(&(s), __LINE__)

/* ---- queue ---- */
#define ds4vizQueue(q, ...) \
    for (ds4vizP_queue q = ds4vizP_queue_open( \
            DS4VIZ_OPT1_("queue" __VA_OPT__(,) __VA_ARGS__)); \
         !q.done; ds4vizP_queue_close(&q))

#define ds4vizQueueEnqueue(q, v) ds4vizP_queue_enqueue(&(q), DS4VIZ_VAL_(v), __LINE__)
#define ds4vizQueueDequeue(q)    ds4vizP_queue_dequeue(&(q), __LINE__)
#define ds4vizQueueClear(q)      ds4vizP_queue_clear(&(q), __LINE__)

/* ---- singly linked list ---- */
#define ds4vizSingleLinkedList(l, ...) \
    for (ds4vizP_slist l = ds4vizP_slist_open( \
            DS4VIZ_OPT1_("slist" __VA_OPT__(,) __VA_ARGS__)); \
         !l.done; ds4vizP_slist_close(&l))

#define ds4vizSlInsertHead(l, v)       ds4vizP_slist_insert_head(&(l), DS4VIZ_VAL_(v), __LINE__)
#define ds4vizSlInsertTail(l, v)       ds4vizP_slist_insert_tail(&(l), DS4VIZ_VAL_(v), __LINE__)
#define ds4vizSlInsertAfter(l, nid, v) ds4vizP_slist_insert_after(&(l), (nid), DS4VIZ_VAL_(v), __LINE__)
#define ds4vizSlDelete(l, nid)         ds4vizP_slist_delete(&(l), (nid), __LINE__)
#define ds4vizSlDeleteHead(l)          ds4vizP_slist_delete_head(&(l), __LINE__)
#define ds4vizSlReverse(l)             ds4vizP_slist_reverse(&(l), __LINE__)

/* ---- doubly linked list ---- */
#define ds4vizDoubleLinkedList(l, ...) \
    for (ds4vizP_dlist l = ds4vizP_dlist_open( \
            DS4VIZ_OPT1_("dlist" __VA_OPT__(,) __VA_ARGS__)); \
         !l.done; ds4vizP_dlist_close(&l))

#define ds4vizDlInsertHead(l, v)        ds4vizP_dlist_insert_head(&(l), DS4VIZ_VAL_(v), __LINE__)
#define ds4vizDlInsertTail(l, v)        ds4vizP_dlist_insert_tail(&(l), DS4VIZ_VAL_(v), __LINE__)
#define ds4vizDlInsertBefore(l, nid, v) ds4vizP_dlist_insert_before(&(l), (nid), DS4VIZ_VAL_(v), __LINE__)
#define ds4vizDlInsertAfter(l, nid, v)  ds4vizP_dlist_insert_after(&(l), (nid), DS4VIZ_VAL_(v), __LINE__)
#define ds4vizDlDelete(l, nid)          ds4vizP_dlist_delete(&(l), (nid), __LINE__)
#define ds4vizDlDeleteHead(l)           ds4vizP_dlist_delete_head(&(l), __LINE__)
#define ds4vizDlDeleteTail(l)           ds4vizP_dlist_delete_tail(&(l), __LINE__)
#define ds4vizDlReverse(l)              ds4vizP_dlist_reverse(&(l), __LINE__)

/* ---- binary tree ---- */
#define ds4vizBinaryTree(t, ...) \
    for (ds4vizP_bt t = ds4vizP_bt_open("binary_tree", \
            DS4VIZ_OPT1_("binary_tree" __VA_OPT__(,) __VA_ARGS__)); \
         !t.done; ds4vizP_bt_close(&t))

#define ds4vizBtInsertRoot(t, v)       ds4vizP_bt_insert_root(&(t), DS4VIZ_VAL_(v), __LINE__)
#define ds4vizBtInsertLeft(t, pid, v)  ds4vizP_bt_insert_left(&(t), (pid), DS4VIZ_VAL_(v), __LINE__)
#define ds4vizBtInsertRight(t, pid, v) ds4vizP_bt_insert_right(&(t), (pid), DS4VIZ_VAL_(v), __LINE__)
#define ds4vizBtDelete(t, nid)         ds4vizP_bt_delete(&(t), (nid), __LINE__)
#define ds4vizBtUpdateValue(t, nid, v) ds4vizP_bt_update_value(&(t), (nid), DS4VIZ_VAL_(v), __LINE__)

/* ---- binary search tree ---- */
#define ds4vizBinarySearchTree(b, ...) \
    for (ds4vizP_bt b = ds4vizP_bt_open("bst", \
            DS4VIZ_OPT1_("bst" __VA_OPT__(,) __VA_ARGS__)); \
         !b.done; ds4vizP_bt_close(&b))

#define ds4vizBstInsert(b, v) ds4vizP_bst_insert(&(b), DS4VIZ_NUMVAL_(v), __LINE__)
#define ds4vizBstDelete(b, v) ds4vizP_bst_delete(&(b), DS4VIZ_NUMVAL_(v), __LINE__)

/* ---- heap ---- */
#define ds4vizHeap(h, ...) \
    for (ds4vizP_heap h = ds4vizP_heap_open( \
            DS4VIZ_OPT1_("heap" __VA_OPT__(,) __VA_ARGS__), \
            (int)DS4VIZ_OPT2_(0 __VA_OPT__(,) __VA_ARGS__)); \
         !h.done; ds4vizP_heap_close(&h))

#define ds4vizHeapInsert(h, v)  ds4vizP_heap_insert(&(h), DS4VIZ_NUMVAL_(v), __LINE__)
#define ds4vizHeapExtract(h)    ds4vizP_heap_extract(&(h), __LINE__)
#define ds4vizHeapClear(h)      ds4vizP_heap_clear(&(h), __LINE__)

/* ---- undirected graph ---- */
#define ds4vizGraphUndirected(g, ...) \
    for (ds4vizP_graph g = ds4vizP_graph_open("graph_undirected", \
            DS4VIZ_OPT1_("graph" __VA_OPT__(,) __VA_ARGS__), false, false); \
         !g.done; ds4vizP_graph_close(&g))

#define ds4vizGuAddNode(g, nid, lbl)    ds4vizP_graph_add_node(&(g), (nid), (lbl), __LINE__)
#define ds4vizGuAddEdge(g, f, t)        ds4vizP_graph_add_edge(&(g), (f), (t), 0.0, __LINE__)
#define ds4vizGuRemoveNode(g, nid)      ds4vizP_graph_remove_node(&(g), (nid), __LINE__)
#define ds4vizGuRemoveEdge(g, f, t)     ds4vizP_graph_remove_edge(&(g), (f), (t), __LINE__)
#define ds4vizGuUpdateNodeLabel(g,n,l)  ds4vizP_graph_update_node_label(&(g),(n),(l),__LINE__)

/* ---- directed graph ---- */
#define ds4vizGraphDirected(g, ...) \
    for (ds4vizP_graph g = ds4vizP_graph_open("graph_directed", \
            DS4VIZ_OPT1_("graph" __VA_OPT__(,) __VA_ARGS__), true, false); \
         !g.done; ds4vizP_graph_close(&g))

#define ds4vizGdAddNode(g, nid, lbl)    ds4vizP_graph_add_node(&(g), (nid), (lbl), __LINE__)
#define ds4vizGdAddEdge(g, f, t)        ds4vizP_graph_add_edge(&(g), (f), (t), 0.0, __LINE__)
#define ds4vizGdRemoveNode(g, nid)      ds4vizP_graph_remove_node(&(g), (nid), __LINE__)
#define ds4vizGdRemoveEdge(g, f, t)     ds4vizP_graph_remove_edge(&(g), (f), (t), __LINE__)
#define ds4vizGdUpdateNodeLabel(g,n,l)  ds4vizP_graph_update_node_label(&(g),(n),(l),__LINE__)

/* ---- weighted graph ---- */
#define ds4vizGraphWeighted(g, ...) \
    for (ds4vizP_graph g = ds4vizP_graph_open("graph_weighted", \
            DS4VIZ_OPT1_("graph" __VA_OPT__(,) __VA_ARGS__), \
            (bool)DS4VIZ_OPT2_(false __VA_OPT__(,) __VA_ARGS__), true); \
         !g.done; ds4vizP_graph_close(&g))

#define ds4vizGwAddNode(g, nid, lbl)      ds4vizP_graph_add_node(&(g), (nid), (lbl), __LINE__)
#define ds4vizGwAddEdge(g, f, t, w)       ds4vizP_graph_add_edge(&(g), (f), (t), (double)(w), __LINE__)
#define ds4vizGwRemoveNode(g, nid)        ds4vizP_graph_remove_node(&(g), (nid), __LINE__)
#define ds4vizGwRemoveEdge(g, f, t)       ds4vizP_graph_remove_edge(&(g), (f), (t), __LINE__)
#define ds4vizGwUpdateWeight(g, f, t, w)  ds4vizP_graph_update_weight(&(g),(f),(t),(double)(w),__LINE__)
#define ds4vizGwUpdateNodeLabel(g,n,l)    ds4vizP_graph_update_node_label(&(g),(n),(l),__LINE__)

/* ================================================================
 * SHORT NAMES
 * ================================================================ */
#ifdef DS4VIZ_SHORT_NAMES

#define dvConfigOptions         ds4vizConfigOptions
#define dvHeapOrder             ds4vizHeapOrder
#define dvHeapOrderMin          ds4vizHeapOrderMin
#define dvHeapOrderMax          ds4vizHeapOrderMax
#define dvConfig                ds4vizConfig

#define dvStack                 ds4vizStack
#define dvStackPush             ds4vizStackPush
#define dvStackPop              ds4vizStackPop
#define dvStackClear            ds4vizStackClear

#define dvQueue                 ds4vizQueue
#define dvQueueEnqueue          ds4vizQueueEnqueue
#define dvQueueDequeue          ds4vizQueueDequeue
#define dvQueueClear            ds4vizQueueClear

#define dvSingleLinkedList      ds4vizSingleLinkedList
#define dvSlInsertHead          ds4vizSlInsertHead
#define dvSlInsertTail          ds4vizSlInsertTail
#define dvSlInsertAfter         ds4vizSlInsertAfter
#define dvSlDelete              ds4vizSlDelete
#define dvSlDeleteHead          ds4vizSlDeleteHead
#define dvSlReverse             ds4vizSlReverse

#define dvDoubleLinkedList      ds4vizDoubleLinkedList
#define dvDlInsertHead          ds4vizDlInsertHead
#define dvDlInsertTail          ds4vizDlInsertTail
#define dvDlInsertBefore        ds4vizDlInsertBefore
#define dvDlInsertAfter         ds4vizDlInsertAfter
#define dvDlDelete              ds4vizDlDelete
#define dvDlDeleteHead          ds4vizDlDeleteHead
#define dvDlDeleteTail          ds4vizDlDeleteTail
#define dvDlReverse             ds4vizDlReverse

#define dvBinaryTree            ds4vizBinaryTree
#define dvBtInsertRoot          ds4vizBtInsertRoot
#define dvBtInsertLeft          ds4vizBtInsertLeft
#define dvBtInsertRight         ds4vizBtInsertRight
#define dvBtDelete              ds4vizBtDelete
#define dvBtUpdateValue         ds4vizBtUpdateValue

#define dvBinarySearchTree      ds4vizBinarySearchTree
#define dvBstInsert             ds4vizBstInsert
#define dvBstDelete             ds4vizBstDelete

#define dvHeap                  ds4vizHeap
#define dvHeapInsert            ds4vizHeapInsert
#define dvHeapExtract           ds4vizHeapExtract
#define dvHeapClear             ds4vizHeapClear

#define dvGraphUndirected       ds4vizGraphUndirected
#define dvGuAddNode             ds4vizGuAddNode
#define dvGuAddEdge             ds4vizGuAddEdge
#define dvGuRemoveNode          ds4vizGuRemoveNode
#define dvGuRemoveEdge          ds4vizGuRemoveEdge
#define dvGuUpdateNodeLabel     ds4vizGuUpdateNodeLabel

#define dvGraphDirected         ds4vizGraphDirected
#define dvGdAddNode             ds4vizGdAddNode
#define dvGdAddEdge             ds4vizGdAddEdge
#define dvGdRemoveNode          ds4vizGdRemoveNode
#define dvGdRemoveEdge          ds4vizGdRemoveEdge
#define dvGdUpdateNodeLabel     ds4vizGdUpdateNodeLabel

#define dvGraphWeighted         ds4vizGraphWeighted
#define dvGwAddNode             ds4vizGwAddNode
#define dvGwAddEdge             ds4vizGwAddEdge
#define dvGwRemoveNode          ds4vizGwRemoveNode
#define dvGwRemoveEdge          ds4vizGwRemoveEdge
#define dvGwUpdateWeight        ds4vizGwUpdateWeight
#define dvGwUpdateNodeLabel     ds4vizGwUpdateNodeLabel

#endif /* DS4VIZ_SHORT_NAMES */

#ifdef __cplusplus
}
#endif

#endif /* DS4VIZ_H */

/* ================================================================
 *
 *                      IMPLEMENTATION
 *
 * ================================================================ */
#ifdef DS4VIZ_IMPLEMENTATION

/* ----------------------------------------------------------------
 * Global configuration
 * ---------------------------------------------------------------- */
static struct {
    const char *output_path;
    const char *title;
    const char *author;
    const char *comment;
} ds4vizP_g_cfg = {0};

void ds4vizConfig(ds4vizConfigOptions o) {
    ds4vizP_g_cfg.output_path = o.output_path;
    ds4vizP_g_cfg.title       = o.title;
    ds4vizP_g_cfg.author      = o.author;
    ds4vizP_g_cfg.comment     = o.comment;
}

static const char *ds4vizP_outpath(void) {
    return ds4vizP_g_cfg.output_path ? ds4vizP_g_cfg.output_path : "trace.toml";
}

/* ----------------------------------------------------------------
 * Dynamic buffer helpers
 * ---------------------------------------------------------------- */
static void ds4vizP_buf_grow(ds4vizP_buf *b, int need) {
    if (b->len + need + 1 > b->cap) {
        int nc = b->cap ? b->cap * 2 : 256;
        while (nc < b->len + need + 1) nc *= 2;
        b->data = (char *)realloc(b->data, nc);
        b->cap = nc;
    }
}

static void ds4vizP_buf_push(ds4vizP_buf *b, char c) {
    ds4vizP_buf_grow(b, 1);
    b->data[b->len++] = c;
    b->data[b->len] = '\0';
}

static void ds4vizP_buf_cat(ds4vizP_buf *b, const char *s) {
    int n = (int)strlen(s);
    ds4vizP_buf_grow(b, n);
    memcpy(b->data + b->len, s, n);
    b->len += n;
    b->data[b->len] = '\0';
}

static void ds4vizP_buf_printf(ds4vizP_buf *b, const char *fmt, ...) {
    va_list ap, ap2;
    va_start(ap, fmt);
    va_copy(ap2, ap);
    int n = vsnprintf(NULL, 0, fmt, ap);
    va_end(ap);
    ds4vizP_buf_grow(b, n + 1);
    vsnprintf(b->data + b->len, n + 1, fmt, ap2);
    va_end(ap2);
    b->len += n;
}

static void ds4vizP_buf_free(ds4vizP_buf *b) {
    free(b->data);
    b->data = NULL;
    b->len = b->cap = 0;
}

/* ----------------------------------------------------------------
 * TOML writing helpers
 * ---------------------------------------------------------------- */
static void ds4vizP_toml_str(ds4vizP_buf *b, const char *s) {
    ds4vizP_buf_push(b, '"');
    if (s) {
        for (; *s; s++) {
            switch (*s) {
            case '\\': ds4vizP_buf_cat(b, "\\\\"); break;
            case '"':  ds4vizP_buf_cat(b, "\\\""); break;
            case '\n': ds4vizP_buf_cat(b, "\\n"); break;
            case '\t': ds4vizP_buf_cat(b, "\\t"); break;
            case '\r': ds4vizP_buf_cat(b, "\\r"); break;
            default:   ds4vizP_buf_push(b, *s); break;
            }
        }
    }
    ds4vizP_buf_push(b, '"');
}

static void ds4vizP_toml_val(ds4vizP_buf *b, ds4vizP_val v) {
    switch (v.type) {
    case DS4VIZ_VINT_:
        ds4vizP_buf_printf(b, "%lld", v.i);
        break;
    case DS4VIZ_VDBL_: {
        char tmp[64];
        snprintf(tmp, sizeof tmp, "%.17g", v.d);
        ds4vizP_buf_cat(b, tmp);
        if (!strchr(tmp, '.') && !strchr(tmp, 'e') && !strchr(tmp, 'E')
            && !strchr(tmp, 'n') && !strchr(tmp, 'i'))
            ds4vizP_buf_cat(b, ".0");
        break;
    }
    case DS4VIZ_VSTR_:
        ds4vizP_toml_str(b, v.s ? v.s : "");
        break;
    case DS4VIZ_VBOOL_:
        ds4vizP_buf_cat(b, v.b ? "true" : "false");
        break;
    }
}

/* ---- value dup / free ---- */
static ds4vizP_val ds4vizP_val_dup(ds4vizP_val v) {
    if (v.type == DS4VIZ_VSTR_ && v.s) {
        ds4vizP_val r = v;
        size_t n = strlen(v.s);
        char *c = (char *)malloc(n + 1);
        memcpy(c, v.s, n + 1);
        r.s = c;
        return r;
    }
    return v;
}

static void ds4vizP_val_free(ds4vizP_val *v) {
    if (v->type == DS4VIZ_VSTR_ && v->s) {
        free((void *)v->s);
        v->s = NULL;
    }
}

static double ds4vizP_val_num(ds4vizP_val v) {
    return v.type == DS4VIZ_VDBL_ ? v.d : (double)v.i;
}

/* ---- meta writing ---- */
static void ds4vizP_write_meta(ds4vizP_buf *b) {
    ds4vizP_buf_cat(b, "[meta]\n");
    if (ds4vizP_g_cfg.title) {
        ds4vizP_buf_cat(b, "title = ");
        ds4vizP_toml_str(b, ds4vizP_g_cfg.title);
        ds4vizP_buf_push(b, '\n');
    }
    if (ds4vizP_g_cfg.author) {
        ds4vizP_buf_cat(b, "author = ");
        ds4vizP_toml_str(b, ds4vizP_g_cfg.author);
        ds4vizP_buf_push(b, '\n');
    }
    if (ds4vizP_g_cfg.comment) {
        ds4vizP_buf_cat(b, "comment = ");
        ds4vizP_toml_str(b, ds4vizP_g_cfg.comment);
        ds4vizP_buf_push(b, '\n');
    }
    ds4vizP_buf_push(b, '\n');
}

/* ---- step writing ---- */
static void ds4vizP_write_step(ds4vizP_buf *b, int sid, const char *op,
                                const char *args, int before, int after) {
    ds4vizP_buf_printf(b, "[[steps]]\nid = %d\n", sid);
    ds4vizP_buf_cat(b, "operation = ");
    ds4vizP_toml_str(b, op);
    ds4vizP_buf_push(b, '\n');
    ds4vizP_buf_printf(b, "args = {%s}\n", args ? args : "");
    ds4vizP_buf_printf(b, "before_state = %d\nafter_state = %d\n\n", before, after);
}

/* ---- final flush ---- */
static void ds4vizP_flush(const char *type_nm, const char *label,
                           const char *extra,
                           ds4vizP_buf *states, ds4vizP_buf *steps,
                           bool err, const char *errmsg,
                           int err_step, int err_last_state, int err_line,
                           int final_state, int total_steps) {
    ds4vizP_buf out = {0};
    ds4vizP_write_meta(&out);
    ds4vizP_buf_cat(&out, "[structure]\ntype = ");
    ds4vizP_toml_str(&out, type_nm);
    ds4vizP_buf_cat(&out, "\nlabel = ");
    ds4vizP_toml_str(&out, label);
    ds4vizP_buf_push(&out, '\n');
    if (extra && extra[0]) ds4vizP_buf_cat(&out, extra);
    ds4vizP_buf_push(&out, '\n');
    if (states->data) ds4vizP_buf_cat(&out, states->data);
    if (steps->data)  ds4vizP_buf_cat(&out, steps->data);
    if (err) {
        ds4vizP_buf_cat(&out, "[error]\ntype = \"runtime\"\nmessage = ");
        ds4vizP_toml_str(&out, errmsg);
        ds4vizP_buf_push(&out, '\n');
        ds4vizP_buf_printf(&out, "line = %d\nstep = %d\nlast_state = %d\n",
                           err_line, err_step, err_last_state);
    } else {
        ds4vizP_buf_printf(&out, "[result]\nfinal_state = %d\ntotal_steps = %d\n",
                           final_state, total_steps);
    }
    FILE *f = fopen(ds4vizP_outpath(), "w");
    if (f) {
        if (out.data) fwrite(out.data, 1, out.len, f);
        fclose(f);
    }
    ds4vizP_buf_free(&out);
}

/* ---- error macro ---- */
#define DS4VIZ_ERR_(s, ln, ...) do { \
    (s)->err = true; \
    snprintf((s)->errmsg, sizeof((s)->errmsg), __VA_ARGS__); \
    (s)->err_step = (s)->step_id; \
    (s)->err_last_state = (s)->state_id; \
    (s)->err_line = ln; \
} while(0)

/* ---- args buffer helper ---- */
static void ds4vizP_args_val(ds4vizP_buf *b, const char *k, ds4vizP_val v) {
    if (b->len > 0) ds4vizP_buf_cat(b, ", ");
    ds4vizP_buf_cat(b, k);
    ds4vizP_buf_cat(b, " = ");
    ds4vizP_toml_val(b, v);
}

static void ds4vizP_args_int(ds4vizP_buf *b, const char *k, int v) {
    if (b->len > 0) ds4vizP_buf_cat(b, ", ");
    ds4vizP_buf_printf(b, "%s = %d", k, v);
}

static void ds4vizP_args_str(ds4vizP_buf *b, const char *k, const char *v) {
    if (b->len > 0) ds4vizP_buf_cat(b, ", ");
    ds4vizP_buf_cat(b, k);
    ds4vizP_buf_cat(b, " = ");
    ds4vizP_toml_str(b, v);
}

static void ds4vizP_args_dbl(ds4vizP_buf *b, const char *k, double v) {
    ds4vizP_args_val(b, k, (ds4vizP_val){.type=DS4VIZ_VDBL_, .d=v});
}

/* ---- capacity helper ---- */
#define DS4VIZ_GROW_(arr, cnt, cap, type) do { \
    if ((cnt) >= (cap)) { \
        (cap) = (cap) ? (cap)*2 : 8; \
        (arr) = (type *)realloc((arr), (cap)*sizeof(type)); \
    } \
} while(0)

/* ================================================================
 *  STACK
 * ================================================================ */

static void ds4vizP_stack_ws(ds4vizP_stack *s) {
    ds4vizP_buf *b = &s->states;
    ds4vizP_buf_printf(b, "[[states]]\nid = %d\nelements = [", s->state_id);
    for (int i = 0; i < s->count; i++) {
        if (i) ds4vizP_buf_cat(b, ", ");
        ds4vizP_toml_val(b, s->items[i]);
    }
    ds4vizP_buf_cat(b, "]\n\n");
}

ds4vizP_stack ds4vizP_stack_open(const char *label) {
    ds4vizP_stack s = {0};
    s.label = label;
    s.type_name = "stack";
    ds4vizP_stack_ws(&s);
    return s;
}

void ds4vizP_stack_close(ds4vizP_stack *s) {
    ds4vizP_flush(s->type_name, s->label, NULL, &s->states, &s->steps,
                  s->err, s->errmsg, s->err_step, s->err_last_state, s->err_line,
                  s->state_id, s->step_id);
    for (int i = 0; i < s->count; i++) ds4vizP_val_free(&s->items[i]);
    free(s->items);
    ds4vizP_buf_free(&s->states);
    ds4vizP_buf_free(&s->steps);
    s->done = true;
}

void ds4vizP_stack_push(ds4vizP_stack *s, ds4vizP_val v, int line) {
    if (s->err) return;
    v = ds4vizP_val_dup(v);
    DS4VIZ_GROW_(s->items, s->count, s->cap, ds4vizP_val);
    s->items[s->count++] = v;
    int before = s->state_id;
    s->state_id++;
    ds4vizP_stack_ws(s);
    ds4vizP_buf args = {0};
    ds4vizP_args_val(&args, "value", v);
    ds4vizP_write_step(&s->steps, s->step_id++, "push", args.data, before, s->state_id);
    ds4vizP_buf_free(&args);
    (void)line;
}

void ds4vizP_stack_pop(ds4vizP_stack *s, int line) {
    if (s->err) return;
    if (s->count == 0) { DS4VIZ_ERR_(s, line, "Stack is empty"); return; }
    ds4vizP_val_free(&s->items[--s->count]);
    int before = s->state_id;
    s->state_id++;
    ds4vizP_stack_ws(s);
    ds4vizP_write_step(&s->steps, s->step_id++, "pop", "", before, s->state_id);
}

void ds4vizP_stack_clear(ds4vizP_stack *s, int line) {
    if (s->err) return;
    for (int i = 0; i < s->count; i++) ds4vizP_val_free(&s->items[i]);
    s->count = 0;
    int before = s->state_id;
    s->state_id++;
    ds4vizP_stack_ws(s);
    ds4vizP_write_step(&s->steps, s->step_id++, "clear", "", before, s->state_id);
    (void)line;
}

/* ================================================================
 *  QUEUE
 * ================================================================ */

static void ds4vizP_queue_ws(ds4vizP_queue *q) {
    ds4vizP_buf *b = &q->states;
    ds4vizP_buf_printf(b, "[[states]]\nid = %d\nelements = [", q->state_id);
    for (int i = 0; i < q->count; i++) {
        if (i) ds4vizP_buf_cat(b, ", ");
        ds4vizP_toml_val(b, q->items[i]);
    }
    ds4vizP_buf_cat(b, "]\n\n");
}

ds4vizP_queue ds4vizP_queue_open(const char *label) {
    ds4vizP_queue q = {0};
    q.label = label;
    q.type_name = "queue";
    ds4vizP_queue_ws(&q);
    return q;
}

void ds4vizP_queue_close(ds4vizP_queue *q) {
    ds4vizP_flush(q->type_name, q->label, NULL, &q->states, &q->steps,
                  q->err, q->errmsg, q->err_step, q->err_last_state, q->err_line,
                  q->state_id, q->step_id);
    for (int i = 0; i < q->count; i++) ds4vizP_val_free(&q->items[i]);
    free(q->items);
    ds4vizP_buf_free(&q->states);
    ds4vizP_buf_free(&q->steps);
    q->done = true;
}

void ds4vizP_queue_enqueue(ds4vizP_queue *q, ds4vizP_val v, int line) {
    if (q->err) return;
    v = ds4vizP_val_dup(v);
    DS4VIZ_GROW_(q->items, q->count, q->cap, ds4vizP_val);
    q->items[q->count++] = v;
    int before = q->state_id;
    q->state_id++;
    ds4vizP_queue_ws(q);
    ds4vizP_buf args = {0};
    ds4vizP_args_val(&args, "value", v);
    ds4vizP_write_step(&q->steps, q->step_id++, "enqueue", args.data, before, q->state_id);
    ds4vizP_buf_free(&args);
    (void)line;
}

void ds4vizP_queue_dequeue(ds4vizP_queue *q, int line) {
    if (q->err) return;
    if (q->count == 0) { DS4VIZ_ERR_(q, line, "Queue is empty"); return; }
    ds4vizP_val_free(&q->items[0]);
    memmove(q->items, q->items + 1, (q->count - 1) * sizeof(ds4vizP_val));
    q->count--;
    int before = q->state_id;
    q->state_id++;
    ds4vizP_queue_ws(q);
    ds4vizP_write_step(&q->steps, q->step_id++, "dequeue", "", before, q->state_id);
}

void ds4vizP_queue_clear(ds4vizP_queue *q, int line) {
    if (q->err) return;
    for (int i = 0; i < q->count; i++) ds4vizP_val_free(&q->items[i]);
    q->count = 0;
    int before = q->state_id;
    q->state_id++;
    ds4vizP_queue_ws(q);
    ds4vizP_write_step(&q->steps, q->step_id++, "clear", "", before, q->state_id);
    (void)line;
}

/* ================================================================
 *  SINGLY LINKED LIST
 * ================================================================ */

static void ds4vizP_slist_ws(ds4vizP_slist *l) {
    ds4vizP_buf *b = &l->states;
    ds4vizP_buf_printf(b, "[[states]]\nid = %d\nhead = %d\nnodes = [", l->state_id, l->head);
    bool first = true;
    for (int i = 0; i < l->ncnt; i++) {
        if (!l->nodes[i].alive) continue;
        if (!first) ds4vizP_buf_cat(b, ", ");
        first = false;
        ds4vizP_buf_printf(b, "{id = %d, value = ", l->nodes[i].id);
        ds4vizP_toml_val(b, l->nodes[i].value);
        ds4vizP_buf_printf(b, ", next = %d}", l->nodes[i].next);
    }
    ds4vizP_buf_cat(b, "]\n\n");
}

static ds4vizP_slnode *ds4vizP_slist_find(ds4vizP_slist *l, int nid) {
    if (nid >= 0 && nid < l->ncnt && l->nodes[nid].alive) return &l->nodes[nid];
    return NULL;
}

static int ds4vizP_slist_alloc(ds4vizP_slist *l, ds4vizP_val v) {
    int id = l->nid++;
    if (id >= l->ncap) {
        int nc = l->ncap ? l->ncap * 2 : 8;
        while (nc <= id) nc *= 2;
        l->nodes = (ds4vizP_slnode *)realloc(l->nodes, nc * sizeof(ds4vizP_slnode));
        l->ncap = nc;
    }
    l->nodes[id] = (ds4vizP_slnode){.id = id, .value = v, .next = -1, .alive = true};
    l->ncnt = l->nid;
    return id;
}

ds4vizP_slist ds4vizP_slist_open(const char *label) {
    ds4vizP_slist l = {0};
    l.label = label;
    l.type_name = "slist";
    l.head = -1;
    ds4vizP_slist_ws(&l);
    return l;
}

void ds4vizP_slist_close(ds4vizP_slist *l) {
    ds4vizP_flush(l->type_name, l->label, NULL, &l->states, &l->steps,
                  l->err, l->errmsg, l->err_step, l->err_last_state, l->err_line,
                  l->state_id, l->step_id);
    for (int i = 0; i < l->ncnt; i++)
        if (l->nodes[i].alive) ds4vizP_val_free(&l->nodes[i].value);
    free(l->nodes);
    ds4vizP_buf_free(&l->states);
    ds4vizP_buf_free(&l->steps);
    l->done = true;
}

int ds4vizP_slist_insert_head(ds4vizP_slist *l, ds4vizP_val v, int line) {
    if (l->err) return -1;
    v = ds4vizP_val_dup(v);
    int id = ds4vizP_slist_alloc(l, v);
    l->nodes[id].next = l->head;
    l->head = id;
    int before = l->state_id++;
    ds4vizP_slist_ws(l);
    ds4vizP_buf a = {0};
    ds4vizP_args_val(&a, "value", v);
    ds4vizP_write_step(&l->steps, l->step_id++, "insert_head", a.data, before, l->state_id);
    ds4vizP_buf_free(&a);
    (void)line;
    return id;
}

int ds4vizP_slist_insert_tail(ds4vizP_slist *l, ds4vizP_val v, int line) {
    if (l->err) return -1;
    v = ds4vizP_val_dup(v);
    int id = ds4vizP_slist_alloc(l, v);
    if (l->head < 0) {
        l->head = id;
    } else {
        int cur = l->head;
        while (l->nodes[cur].next >= 0) cur = l->nodes[cur].next;
        l->nodes[cur].next = id;
    }
    int before = l->state_id++;
    ds4vizP_slist_ws(l);
    ds4vizP_buf a = {0};
    ds4vizP_args_val(&a, "value", v);
    ds4vizP_write_step(&l->steps, l->step_id++, "insert_tail", a.data, before, l->state_id);
    ds4vizP_buf_free(&a);
    (void)line;
    return id;
}

int ds4vizP_slist_insert_after(ds4vizP_slist *l, int nid, ds4vizP_val v, int line) {
    if (l->err) return -1;
    ds4vizP_slnode *n = ds4vizP_slist_find(l, nid);
    if (!n) { DS4VIZ_ERR_(l, line, "Node not found: %d", nid); return -1; }
    v = ds4vizP_val_dup(v);
    int id = ds4vizP_slist_alloc(l, v);
    l->nodes[id].next = n->next;
    n->next = id;
    int before = l->state_id++;
    ds4vizP_slist_ws(l);
    ds4vizP_buf a = {0};
    ds4vizP_args_int(&a, "node_id", nid);
    ds4vizP_args_val(&a, "value", v);
    ds4vizP_write_step(&l->steps, l->step_id++, "insert_after", a.data, before, l->state_id);
    ds4vizP_buf_free(&a);
    return id;
}

void ds4vizP_slist_delete(ds4vizP_slist *l, int nid, int line) {
    if (l->err) return;
    ds4vizP_slnode *n = ds4vizP_slist_find(l, nid);
    if (!n) { DS4VIZ_ERR_(l, line, "Node not found: %d", nid); return; }
    if (l->head == nid) {
        l->head = n->next;
    } else {
        int cur = l->head;
        while (cur >= 0 && l->nodes[cur].next != nid) cur = l->nodes[cur].next;
        if (cur >= 0) l->nodes[cur].next = n->next;
    }
    n->alive = false;
    ds4vizP_val_free(&n->value);
    int before = l->state_id++;
    ds4vizP_slist_ws(l);
    ds4vizP_buf a = {0};
    ds4vizP_args_int(&a, "node_id", nid);
    ds4vizP_write_step(&l->steps, l->step_id++, "delete", a.data, before, l->state_id);
    ds4vizP_buf_free(&a);
}

void ds4vizP_slist_delete_head(ds4vizP_slist *l, int line) {
    if (l->err) return;
    if (l->head < 0) { DS4VIZ_ERR_(l, line, "List is empty"); return; }
    int old = l->head;
    l->head = l->nodes[old].next;
    l->nodes[old].alive = false;
    ds4vizP_val_free(&l->nodes[old].value);
    int before = l->state_id++;
    ds4vizP_slist_ws(l);
    ds4vizP_write_step(&l->steps, l->step_id++, "delete_head", "", before, l->state_id);
}

void ds4vizP_slist_reverse(ds4vizP_slist *l, int line) {
    if (l->err) return;
    int prev = -1, cur = l->head;
    while (cur >= 0) {
        int nxt = l->nodes[cur].next;
        l->nodes[cur].next = prev;
        prev = cur;
        cur = nxt;
    }
    l->head = prev;
    int before = l->state_id++;
    ds4vizP_slist_ws(l);
    ds4vizP_write_step(&l->steps, l->step_id++, "reverse", "", before, l->state_id);
    (void)line;
}

/* ================================================================
 *  DOUBLY LINKED LIST
 * ================================================================ */

static void ds4vizP_dlist_ws(ds4vizP_dlist *l) {
    ds4vizP_buf *b = &l->states;
    ds4vizP_buf_printf(b, "[[states]]\nid = %d\nhead = %d\ntail = %d\nnodes = [",
                       l->state_id, l->head, l->tail);
    bool first = true;
    for (int i = 0; i < l->ncnt; i++) {
        if (!l->nodes[i].alive) continue;
        if (!first) ds4vizP_buf_cat(b, ", ");
        first = false;
        ds4vizP_buf_printf(b, "{id = %d, value = ", l->nodes[i].id);
        ds4vizP_toml_val(b, l->nodes[i].value);
        ds4vizP_buf_printf(b, ", prev = %d, next = %d}", l->nodes[i].prev, l->nodes[i].next);
    }
    ds4vizP_buf_cat(b, "]\n\n");
}

static ds4vizP_dlnode *ds4vizP_dlist_find(ds4vizP_dlist *l, int nid) {
    if (nid >= 0 && nid < l->ncnt && l->nodes[nid].alive) return &l->nodes[nid];
    return NULL;
}

static int ds4vizP_dlist_alloc(ds4vizP_dlist *l, ds4vizP_val v) {
    int id = l->nid++;
    if (id >= l->ncap) {
        int nc = l->ncap ? l->ncap * 2 : 8;
        while (nc <= id) nc *= 2;
        l->nodes = (ds4vizP_dlnode *)realloc(l->nodes, nc * sizeof(ds4vizP_dlnode));
        l->ncap = nc;
    }
    l->nodes[id] = (ds4vizP_dlnode){.id = id, .value = v, .prev = -1, .next = -1, .alive = true};
    l->ncnt = l->nid;
    return id;
}

ds4vizP_dlist ds4vizP_dlist_open(const char *label) {
    ds4vizP_dlist l = {0};
    l.label = label;
    l.type_name = "dlist";
    l.head = l.tail = -1;
    ds4vizP_dlist_ws(&l);
    return l;
}

void ds4vizP_dlist_close(ds4vizP_dlist *l) {
    ds4vizP_flush(l->type_name, l->label, NULL, &l->states, &l->steps,
                  l->err, l->errmsg, l->err_step, l->err_last_state, l->err_line,
                  l->state_id, l->step_id);
    for (int i = 0; i < l->ncnt; i++)
        if (l->nodes[i].alive) ds4vizP_val_free(&l->nodes[i].value);
    free(l->nodes);
    ds4vizP_buf_free(&l->states);
    ds4vizP_buf_free(&l->steps);
    l->done = true;
}

int ds4vizP_dlist_insert_head(ds4vizP_dlist *l, ds4vizP_val v, int line) {
    if (l->err) return -1;
    v = ds4vizP_val_dup(v);
    int id = ds4vizP_dlist_alloc(l, v);
    l->nodes[id].next = l->head;
    if (l->head >= 0) l->nodes[l->head].prev = id;
    l->head = id;
    if (l->tail < 0) l->tail = id;
    int before = l->state_id++;
    ds4vizP_dlist_ws(l);
    ds4vizP_buf a = {0};
    ds4vizP_args_val(&a, "value", v);
    ds4vizP_write_step(&l->steps, l->step_id++, "insert_head", a.data, before, l->state_id);
    ds4vizP_buf_free(&a);
    (void)line;
    return id;
}

int ds4vizP_dlist_insert_tail(ds4vizP_dlist *l, ds4vizP_val v, int line) {
    if (l->err) return -1;
    v = ds4vizP_val_dup(v);
    int id = ds4vizP_dlist_alloc(l, v);
    l->nodes[id].prev = l->tail;
    if (l->tail >= 0) l->nodes[l->tail].next = id;
    l->tail = id;
    if (l->head < 0) l->head = id;
    int before = l->state_id++;
    ds4vizP_dlist_ws(l);
    ds4vizP_buf a = {0};
    ds4vizP_args_val(&a, "value", v);
    ds4vizP_write_step(&l->steps, l->step_id++, "insert_tail", a.data, before, l->state_id);
    ds4vizP_buf_free(&a);
    (void)line;
    return id;
}

int ds4vizP_dlist_insert_before(ds4vizP_dlist *l, int nid, ds4vizP_val v, int line) {
    if (l->err) return -1;
    ds4vizP_dlnode *n = ds4vizP_dlist_find(l, nid);
    if (!n) { DS4VIZ_ERR_(l, line, "Node not found: %d", nid); return -1; }
    v = ds4vizP_val_dup(v);
    int id = ds4vizP_dlist_alloc(l, v);
    l->nodes[id].next = nid;
    l->nodes[id].prev = n->prev;
    if (n->prev >= 0) l->nodes[n->prev].next = id;
    else l->head = id;
    n->prev = id;
    int before = l->state_id++;
    ds4vizP_dlist_ws(l);
    ds4vizP_buf a = {0};
    ds4vizP_args_int(&a, "node_id", nid);
    ds4vizP_args_val(&a, "value", v);
    ds4vizP_write_step(&l->steps, l->step_id++, "insert_before", a.data, before, l->state_id);
    ds4vizP_buf_free(&a);
    return id;
}

int ds4vizP_dlist_insert_after(ds4vizP_dlist *l, int nid, ds4vizP_val v, int line) {
    if (l->err) return -1;
    ds4vizP_dlnode *n = ds4vizP_dlist_find(l, nid);
    if (!n) { DS4VIZ_ERR_(l, line, "Node not found: %d", nid); return -1; }
    v = ds4vizP_val_dup(v);
    int id = ds4vizP_dlist_alloc(l, v);
    l->nodes[id].prev = nid;
    l->nodes[id].next = n->next;
    if (n->next >= 0) l->nodes[n->next].prev = id;
    else l->tail = id;
    n->next = id;
    int before = l->state_id++;
    ds4vizP_dlist_ws(l);
    ds4vizP_buf a = {0};
    ds4vizP_args_int(&a, "node_id", nid);
    ds4vizP_args_val(&a, "value", v);
    ds4vizP_write_step(&l->steps, l->step_id++, "insert_after", a.data, before, l->state_id);
    ds4vizP_buf_free(&a);
    return id;
}

void ds4vizP_dlist_delete(ds4vizP_dlist *l, int nid, int line) {
    if (l->err) return;
    ds4vizP_dlnode *n = ds4vizP_dlist_find(l, nid);
    if (!n) { DS4VIZ_ERR_(l, line, "Node not found: %d", nid); return; }
    if (n->prev >= 0) l->nodes[n->prev].next = n->next; else l->head = n->next;
    if (n->next >= 0) l->nodes[n->next].prev = n->prev; else l->tail = n->prev;
    n->alive = false;
    ds4vizP_val_free(&n->value);
    int before = l->state_id++;
    ds4vizP_dlist_ws(l);
    ds4vizP_buf a = {0};
    ds4vizP_args_int(&a, "node_id", nid);
    ds4vizP_write_step(&l->steps, l->step_id++, "delete", a.data, before, l->state_id);
    ds4vizP_buf_free(&a);
}

void ds4vizP_dlist_delete_head(ds4vizP_dlist *l, int line) {
    if (l->err) return;
    if (l->head < 0) { DS4VIZ_ERR_(l, line, "List is empty"); return; }
    ds4vizP_dlist_delete(l, l->head, line);
    /* step already written by delete; overwrite operation name */
    /* Actually, let's do it directly instead */
}

void ds4vizP_dlist_delete_tail(ds4vizP_dlist *l, int line) {
    if (l->err) return;
    if (l->tail < 0) { DS4VIZ_ERR_(l, line, "List is empty"); return; }
    ds4vizP_dlist_delete(l, l->tail, line);
}

void ds4vizP_dlist_reverse(ds4vizP_dlist *l, int line) {
    if (l->err) return;
    int cur = l->head;
    while (cur >= 0) {
        ds4vizP_dlnode *n = &l->nodes[cur];
        int nxt = n->next;
        int tmp = n->prev;
        n->prev = n->next;
        n->next = tmp;
        cur = nxt;
    }
    int tmp = l->head;
    l->head = l->tail;
    l->tail = tmp;
    int before = l->state_id++;
    ds4vizP_dlist_ws(l);
    ds4vizP_write_step(&l->steps, l->step_id++, "reverse", "", before, l->state_id);
    (void)line;
}

/* ================================================================
 *  BINARY TREE  (also used by BST)
 * ================================================================ */

static void ds4vizP_bt_ws(ds4vizP_bt *t) {
    ds4vizP_buf *b = &t->states;
    ds4vizP_buf_printf(b, "[[states]]\nid = %d\nroot = %d\nnodes = [", t->state_id, t->root);
    bool first = true;
    for (int i = 0; i < t->ncnt; i++) {
        if (!t->nodes[i].alive) continue;
        if (!first) ds4vizP_buf_cat(b, ", ");
        first = false;
        ds4vizP_buf_printf(b, "{id = %d, value = ", t->nodes[i].id);
        ds4vizP_toml_val(b, t->nodes[i].value);
        ds4vizP_buf_printf(b, ", parent = %d, left = %d, right = %d}",
                           t->nodes[i].parent, t->nodes[i].left, t->nodes[i].right);
    }
    ds4vizP_buf_cat(b, "]\n\n");
}

static ds4vizP_tnode *ds4vizP_bt_find(ds4vizP_bt *t, int nid) {
    if (nid >= 0 && nid < t->ncnt && t->nodes[nid].alive) return &t->nodes[nid];
    return NULL;
}

static int ds4vizP_bt_alloc(ds4vizP_bt *t, ds4vizP_val v) {
    int id = t->nid++;
    if (id >= t->ncap) {
        int nc = t->ncap ? t->ncap * 2 : 8;
        while (nc <= id) nc *= 2;
        t->nodes = (ds4vizP_tnode *)realloc(t->nodes, nc * sizeof(ds4vizP_tnode));
        t->ncap = nc;
    }
    t->nodes[id] = (ds4vizP_tnode){.id=id,.value=v,.left=-1,.right=-1,.parent=-1,.alive=true};
    t->ncnt = t->nid;
    return id;
}

ds4vizP_bt ds4vizP_bt_open(const char *type_nm, const char *label) {
    ds4vizP_bt t = {0};
    t.label = label;
    t.type_name = type_nm;
    t.root = -1;
    ds4vizP_bt_ws(&t);
    return t;
}

void ds4vizP_bt_close(ds4vizP_bt *t) {
    ds4vizP_flush(t->type_name, t->label, NULL, &t->states, &t->steps,
                  t->err, t->errmsg, t->err_step, t->err_last_state, t->err_line,
                  t->state_id, t->step_id);
    for (int i = 0; i < t->ncnt; i++)
        if (t->nodes[i].alive) ds4vizP_val_free(&t->nodes[i].value);
    free(t->nodes);
    ds4vizP_buf_free(&t->states);
    ds4vizP_buf_free(&t->steps);
    t->done = true;
}

int ds4vizP_bt_insert_root(ds4vizP_bt *t, ds4vizP_val v, int line) {
    if (t->err) return -1;
    if (t->root >= 0) { DS4VIZ_ERR_(t, line, "Root already exists"); return -1; }
    v = ds4vizP_val_dup(v);
    int id = ds4vizP_bt_alloc(t, v);
    t->root = id;
    int before = t->state_id++;
    ds4vizP_bt_ws(t);
    ds4vizP_buf a = {0};
    ds4vizP_args_val(&a, "value", v);
    ds4vizP_write_step(&t->steps, t->step_id++, "insert_root", a.data, before, t->state_id);
    ds4vizP_buf_free(&a);
    return id;
}

int ds4vizP_bt_insert_left(ds4vizP_bt *t, int pid, ds4vizP_val v, int line) {
    if (t->err) return -1;
    ds4vizP_tnode *p = ds4vizP_bt_find(t, pid);
    if (!p) { DS4VIZ_ERR_(t, line, "Parent node not found: %d", pid); return -1; }
    if (p->left >= 0) { DS4VIZ_ERR_(t, line, "Left child already exists for node %d", pid); return -1; }
    v = ds4vizP_val_dup(v);
    int id = ds4vizP_bt_alloc(t, v);
    p->left = id;
    t->nodes[id].parent = pid;
    int before = t->state_id++;
    ds4vizP_bt_ws(t);
    ds4vizP_buf a = {0};
    ds4vizP_args_int(&a, "parent_id", pid);
    ds4vizP_args_val(&a, "value", v);
    ds4vizP_write_step(&t->steps, t->step_id++, "insert_left", a.data, before, t->state_id);
    ds4vizP_buf_free(&a);
    return id;
}

int ds4vizP_bt_insert_right(ds4vizP_bt *t, int pid, ds4vizP_val v, int line) {
    if (t->err) return -1;
    ds4vizP_tnode *p = ds4vizP_bt_find(t, pid);
    if (!p) { DS4VIZ_ERR_(t, line, "Parent node not found: %d", pid); return -1; }
    if (p->right >= 0) { DS4VIZ_ERR_(t, line, "Right child already exists for node %d", pid); return -1; }
    v = ds4vizP_val_dup(v);
    int id = ds4vizP_bt_alloc(t, v);
    p->right = id;
    t->nodes[id].parent = pid;
    int before = t->state_id++;
    ds4vizP_bt_ws(t);
    ds4vizP_buf a = {0};
    ds4vizP_args_int(&a, "parent_id", pid);
    ds4vizP_args_val(&a, "value", v);
    ds4vizP_write_step(&t->steps, t->step_id++, "insert_right", a.data, before, t->state_id);
    ds4vizP_buf_free(&a);
    return id;
}

/* recursive subtree deletion */
static void ds4vizP_bt_kill(ds4vizP_bt *t, int nid) {
    if (nid < 0 || nid >= t->ncnt || !t->nodes[nid].alive) return;
    ds4vizP_bt_kill(t, t->nodes[nid].left);
    ds4vizP_bt_kill(t, t->nodes[nid].right);
    t->nodes[nid].alive = false;
    ds4vizP_val_free(&t->nodes[nid].value);
}

void ds4vizP_bt_delete(ds4vizP_bt *t, int nid, int line) {
    if (t->err) return;
    ds4vizP_tnode *n = ds4vizP_bt_find(t, nid);
    if (!n) { DS4VIZ_ERR_(t, line, "Node not found: %d", nid); return; }
    int par = n->parent;
    if (par >= 0) {
        if (t->nodes[par].left == nid)  t->nodes[par].left = -1;
        if (t->nodes[par].right == nid) t->nodes[par].right = -1;
    } else {
        t->root = -1;
    }
    ds4vizP_bt_kill(t, nid);
    int before = t->state_id++;
    ds4vizP_bt_ws(t);
    ds4vizP_buf a = {0};
    ds4vizP_args_int(&a, "node_id", nid);
    ds4vizP_write_step(&t->steps, t->step_id++, "delete", a.data, before, t->state_id);
    ds4vizP_buf_free(&a);
}

void ds4vizP_bt_update_value(ds4vizP_bt *t, int nid, ds4vizP_val v, int line) {
    if (t->err) return;
    ds4vizP_tnode *n = ds4vizP_bt_find(t, nid);
    if (!n) { DS4VIZ_ERR_(t, line, "Node not found: %d", nid); return; }
    v = ds4vizP_val_dup(v);
    ds4vizP_val_free(&n->value);
    n->value = v;
    int before = t->state_id++;
    ds4vizP_bt_ws(t);
    ds4vizP_buf a = {0};
    ds4vizP_args_int(&a, "node_id", nid);
    ds4vizP_args_val(&a, "value", v);
    ds4vizP_write_step(&t->steps, t->step_id++, "update_value", a.data, before, t->state_id);
    ds4vizP_buf_free(&a);
}

/* ================================================================
 *  BINARY SEARCH TREE  (reuses ds4vizP_bt)
 * ================================================================ */

int ds4vizP_bst_insert(ds4vizP_bt *t, ds4vizP_val v, int line) {
    if (t->err) return -1;
    v = ds4vizP_val_dup(v);
    double val = ds4vizP_val_num(v);
    int id = ds4vizP_bt_alloc(t, v);
    if (t->root < 0) {
        t->root = id;
    } else {
        int cur = t->root;
        for (;;) {
            double cv = ds4vizP_val_num(t->nodes[cur].value);
            if (val < cv) {
                if (t->nodes[cur].left < 0) {
                    t->nodes[cur].left = id;
                    t->nodes[id].parent = cur;
                    break;
                }
                cur = t->nodes[cur].left;
            } else {
                if (t->nodes[cur].right < 0) {
                    t->nodes[cur].right = id;
                    t->nodes[id].parent = cur;
                    break;
                }
                cur = t->nodes[cur].right;
            }
        }
    }
    int before = t->state_id++;
    ds4vizP_bt_ws(t);
    ds4vizP_buf a = {0};
    ds4vizP_args_val(&a, "value", v);
    ds4vizP_write_step(&t->steps, t->step_id++, "insert", a.data, before, t->state_id);
    ds4vizP_buf_free(&a);
    (void)line;
    return id;
}

/* BST find node by numeric value */
static int ds4vizP_bst_findval(ds4vizP_bt *t, double target) {
    int cur = t->root;
    while (cur >= 0 && t->nodes[cur].alive) {
        double cv = ds4vizP_val_num(t->nodes[cur].value);
        if (fabs(target - cv) < 1e-12) return cur;
        cur = (target < cv) ? t->nodes[cur].left : t->nodes[cur].right;
    }
    return -1;
}

/* replace subtree u with subtree v in parent linkage */
static void ds4vizP_bst_transplant(ds4vizP_bt *t, int u, int v) {
    int up = t->nodes[u].parent;
    if (up < 0) {
        t->root = v;
    } else if (t->nodes[up].left == u) {
        t->nodes[up].left = v;
    } else {
        t->nodes[up].right = v;
    }
    if (v >= 0) t->nodes[v].parent = up;
}

static int ds4vizP_bst_min(ds4vizP_bt *t, int nid) {
    while (t->nodes[nid].left >= 0) nid = t->nodes[nid].left;
    return nid;
}

void ds4vizP_bst_delete(ds4vizP_bt *t, ds4vizP_val v, int line) {
    if (t->err) return;
    double target = ds4vizP_val_num(v);
    int nid = ds4vizP_bst_findval(t, target);
    if (nid < 0) { DS4VIZ_ERR_(t, line, "Value not found in BST"); return; }
    ds4vizP_tnode *n = &t->nodes[nid];
    if (n->left < 0) {
        ds4vizP_bst_transplant(t, nid, n->right);
    } else if (n->right < 0) {
        ds4vizP_bst_transplant(t, nid, n->left);
    } else {
        int suc = ds4vizP_bst_min(t, n->right);
        if (t->nodes[suc].parent != nid) {
            ds4vizP_bst_transplant(t, suc, t->nodes[suc].right);
            t->nodes[suc].right = n->right;
            t->nodes[n->right].parent = suc;
        }
        ds4vizP_bst_transplant(t, nid, suc);
        t->nodes[suc].left = n->left;
        t->nodes[n->left].parent = suc;
    }
    n->alive = false;
    ds4vizP_val_free(&n->value);
    int before = t->state_id++;
    ds4vizP_bt_ws(t);
    ds4vizP_buf a = {0};
    ds4vizP_args_val(&a, "value", v);
    ds4vizP_write_step(&t->steps, t->step_id++, "delete", a.data, before, t->state_id);
    ds4vizP_buf_free(&a);
}

/* ================================================================
 *  HEAP
 * ================================================================ */

static void ds4vizP_heap_ws(ds4vizP_heap *h) {
    ds4vizP_buf *b = &h->states;
    ds4vizP_buf_printf(b, "[[states]]\nid = %d\nelements = [", h->state_id);
    for (int i = 0; i < h->count; i++) {
        if (i) ds4vizP_buf_cat(b, ", ");
        ds4vizP_toml_val(b, h->items[i]);
    }
    ds4vizP_buf_cat(b, "]\n\n");
}

static bool ds4vizP_heap_cmp(ds4vizP_heap *h, int a, int b) {
    /* returns true if items[a] should be above items[b] */
    double va = ds4vizP_val_num(h->items[a]);
    double vb = ds4vizP_val_num(h->items[b]);
    return h->order == 0 ? (va < vb) : (va > vb);
}

static void ds4vizP_heap_swap(ds4vizP_heap *h, int a, int b) {
    ds4vizP_val tmp = h->items[a];
    h->items[a] = h->items[b];
    h->items[b] = tmp;
}

static void ds4vizP_heap_up(ds4vizP_heap *h, int i) {
    while (i > 0) {
        int p = (i - 1) / 2;
        if (ds4vizP_heap_cmp(h, i, p)) { ds4vizP_heap_swap(h, i, p); i = p; }
        else break;
    }
}

static void ds4vizP_heap_down(ds4vizP_heap *h, int i) {
    for (;;) {
        int best = i, l = 2*i+1, r = 2*i+2;
        if (l < h->count && ds4vizP_heap_cmp(h, l, best)) best = l;
        if (r < h->count && ds4vizP_heap_cmp(h, r, best)) best = r;
        if (best != i) { ds4vizP_heap_swap(h, i, best); i = best; }
        else break;
    }
}

ds4vizP_heap ds4vizP_heap_open(const char *label, int order) {
    ds4vizP_heap h = {0};
    h.label = label;
    h.type_name = "heap";
    h.order = order;
    ds4vizP_heap_ws(&h);
    return h;
}

void ds4vizP_heap_close(ds4vizP_heap *h) {
    char extra[64];
    snprintf(extra, sizeof extra, "order = \"%s\"\n", h->order == 0 ? "min" : "max");
    ds4vizP_flush(h->type_name, h->label, extra, &h->states, &h->steps,
                  h->err, h->errmsg, h->err_step, h->err_last_state, h->err_line,
                  h->state_id, h->step_id);
    for (int i = 0; i < h->count; i++) ds4vizP_val_free(&h->items[i]);
    free(h->items);
    ds4vizP_buf_free(&h->states);
    ds4vizP_buf_free(&h->steps);
    h->done = true;
}

void ds4vizP_heap_insert(ds4vizP_heap *h, ds4vizP_val v, int line) {
    if (h->err) return;
    v = ds4vizP_val_dup(v);
    DS4VIZ_GROW_(h->items, h->count, h->cap, ds4vizP_val);
    h->items[h->count++] = v;
    ds4vizP_heap_up(h, h->count - 1);
    int before = h->state_id++;
    ds4vizP_heap_ws(h);
    ds4vizP_buf a = {0};
    ds4vizP_args_val(&a, "value", v);
    ds4vizP_write_step(&h->steps, h->step_id++, "insert", a.data, before, h->state_id);
    ds4vizP_buf_free(&a);
    (void)line;
}

void ds4vizP_heap_extract(ds4vizP_heap *h, int line) {
    if (h->err) return;
    if (h->count == 0) { DS4VIZ_ERR_(h, line, "Heap is empty"); return; }
    ds4vizP_val_free(&h->items[0]);
    h->items[0] = h->items[--h->count];
    if (h->count > 0) ds4vizP_heap_down(h, 0);
    int before = h->state_id++;
    ds4vizP_heap_ws(h);
    ds4vizP_write_step(&h->steps, h->step_id++, "extract", "", before, h->state_id);
}

void ds4vizP_heap_clear(ds4vizP_heap *h, int line) {
    if (h->err) return;
    for (int i = 0; i < h->count; i++) ds4vizP_val_free(&h->items[i]);
    h->count = 0;
    int before = h->state_id++;
    ds4vizP_heap_ws(h);
    ds4vizP_write_step(&h->steps, h->step_id++, "clear", "", before, h->state_id);
    (void)line;
}

/* ================================================================
 *  GRAPH  (unified: undirected / directed / weighted)
 * ================================================================ */

static void ds4vizP_graph_ws(ds4vizP_graph *g) {
    ds4vizP_buf *b = &g->states;
    ds4vizP_buf_printf(b, "[[states]]\nid = %d\nnodes = [", g->state_id);
    bool first = true;
    for (int i = 0; i < g->ncnt; i++) {
        if (!g->nodes[i].alive) continue;
        if (!first) ds4vizP_buf_cat(b, ", ");
        first = false;
        ds4vizP_buf_printf(b, "{id = %d, label = ", g->nodes[i].id);
        ds4vizP_toml_str(b, g->nodes[i].label);
        ds4vizP_buf_push(b, '}');
    }
    ds4vizP_buf_cat(b, "]\nedges = [");
    first = true;
    for (int i = 0; i < g->ecnt; i++) {
        if (!g->edges[i].alive) continue;
        if (!first) ds4vizP_buf_cat(b, ", ");
        first = false;
        ds4vizP_buf_printf(b, "{from = %d, to = %d", g->edges[i].from, g->edges[i].to);
        if (g->weighted) {
            ds4vizP_buf_cat(b, ", weight = ");
            ds4vizP_toml_val(b, (ds4vizP_val){.type=DS4VIZ_VDBL_, .d=g->edges[i].weight});
        }
        ds4vizP_buf_push(b, '}');
    }
    ds4vizP_buf_cat(b, "]\n\n");
}

static ds4vizP_gnode *ds4vizP_graph_findnode(ds4vizP_graph *g, int nid) {
    for (int i = 0; i < g->ncnt; i++)
        if (g->nodes[i].alive && g->nodes[i].id == nid) return &g->nodes[i];
    return NULL;
}

static ds4vizP_gedge *ds4vizP_graph_findedge(ds4vizP_graph *g, int from, int to) {
    for (int i = 0; i < g->ecnt; i++)
        if (g->edges[i].alive && g->edges[i].from == from && g->edges[i].to == to)
            return &g->edges[i];
    return NULL;
}

ds4vizP_graph ds4vizP_graph_open(const char *type_nm, const char *label, bool directed, bool weighted) {
    ds4vizP_graph g = {0};
    g.label = label;
    g.type_name = type_nm;
    g.directed = directed;
    g.weighted = weighted;
    ds4vizP_graph_ws(&g);
    return g;
}

void ds4vizP_graph_close(ds4vizP_graph *g) {
    char extra[128] = {0};
    if (g->weighted) {
        snprintf(extra, sizeof extra, "directed = %s\n", g->directed ? "true" : "false");
    }
    ds4vizP_flush(g->type_name, g->label, extra, &g->states, &g->steps,
                  g->err, g->errmsg, g->err_step, g->err_last_state, g->err_line,
                  g->state_id, g->step_id);
    free(g->nodes);
    free(g->edges);
    ds4vizP_buf_free(&g->states);
    ds4vizP_buf_free(&g->steps);
    g->done = true;
}

void ds4vizP_graph_add_node(ds4vizP_graph *g, int nid, const char *lbl, int line) {
    if (g->err) return;
    if (ds4vizP_graph_findnode(g, nid)) {
        DS4VIZ_ERR_(g, line, "Node already exists: %d", nid);
        return;
    }
    DS4VIZ_GROW_(g->nodes, g->ncnt, g->ncap, ds4vizP_gnode);
    ds4vizP_gnode *n = &g->nodes[g->ncnt++];
    n->id = nid;
    n->alive = true;
    strncpy(n->label, lbl ? lbl : "", 32);
    n->label[32] = '\0';
    int before = g->state_id++;
    ds4vizP_graph_ws(g);
    ds4vizP_buf a = {0};
    ds4vizP_args_int(&a, "node_id", nid);
    ds4vizP_args_str(&a, "label", lbl ? lbl : "");
    ds4vizP_write_step(&g->steps, g->step_id++, "add_node", a.data, before, g->state_id);
    ds4vizP_buf_free(&a);
}

void ds4vizP_graph_add_edge(ds4vizP_graph *g, int from, int to, double w, int line) {
    if (g->err) return;
    if (from == to) { DS4VIZ_ERR_(g, line, "Self-loop not allowed: %d", from); return; }
    /* normalize for undirected unweighted / undirected weighted */
    int ef = from, et = to;
    if (!g->directed && ef > et) { int tmp = ef; ef = et; et = tmp; }
    if (!ds4vizP_graph_findnode(g, from)) { DS4VIZ_ERR_(g, line, "Node not found: %d", from); return; }
    if (!ds4vizP_graph_findnode(g, to))   { DS4VIZ_ERR_(g, line, "Node not found: %d", to); return; }
    if (ds4vizP_graph_findedge(g, ef, et)) { DS4VIZ_ERR_(g, line, "Edge already exists: %d -> %d", ef, et); return; }
    DS4VIZ_GROW_(g->edges, g->ecnt, g->ecap, ds4vizP_gedge);
    g->edges[g->ecnt++] = (ds4vizP_gedge){.from=ef, .to=et, .weight=w, .alive=true};
    int before = g->state_id++;
    ds4vizP_graph_ws(g);
    ds4vizP_buf a = {0};
    ds4vizP_args_int(&a, "from_id", from);
    ds4vizP_args_int(&a, "to_id", to);
    if (g->weighted) ds4vizP_args_dbl(&a, "weight", w);
    ds4vizP_write_step(&g->steps, g->step_id++, "add_edge", a.data, before, g->state_id);
    ds4vizP_buf_free(&a);
}

void ds4vizP_graph_remove_node(ds4vizP_graph *g, int nid, int line) {
    if (g->err) return;
    ds4vizP_gnode *n = ds4vizP_graph_findnode(g, nid);
    if (!n) { DS4VIZ_ERR_(g, line, "Node not found: %d", nid); return; }
    n->alive = false;
    /* remove all edges involving this node */
    for (int i = 0; i < g->ecnt; i++) {
        if (g->edges[i].alive && (g->edges[i].from == nid || g->edges[i].to == nid))
            g->edges[i].alive = false;
    }
    int before = g->state_id++;
    ds4vizP_graph_ws(g);
    ds4vizP_buf a = {0};
    ds4vizP_args_int(&a, "node_id", nid);
    ds4vizP_write_step(&g->steps, g->step_id++, "remove_node", a.data, before, g->state_id);
    ds4vizP_buf_free(&a);
}

void ds4vizP_graph_remove_edge(ds4vizP_graph *g, int from, int to, int line) {
    if (g->err) return;
    int ef = from, et = to;
    if (!g->directed && ef > et) { int tmp = ef; ef = et; et = tmp; }
    ds4vizP_gedge *e = ds4vizP_graph_findedge(g, ef, et);
    if (!e) { DS4VIZ_ERR_(g, line, "Edge not found: %d -> %d", from, to); return; }
    e->alive = false;
    int before = g->state_id++;
    ds4vizP_graph_ws(g);
    ds4vizP_buf a = {0};
    ds4vizP_args_int(&a, "from_id", from);
    ds4vizP_args_int(&a, "to_id", to);
    ds4vizP_write_step(&g->steps, g->step_id++, "remove_edge", a.data, before, g->state_id);
    ds4vizP_buf_free(&a);
}

void ds4vizP_graph_update_node_label(ds4vizP_graph *g, int nid, const char *lbl, int line) {
    if (g->err) return;
    ds4vizP_gnode *n = ds4vizP_graph_findnode(g, nid);
    if (!n) { DS4VIZ_ERR_(g, line, "Node not found: %d", nid); return; }
    strncpy(n->label, lbl ? lbl : "", 32);
    n->label[32] = '\0';
    int before = g->state_id++;
    ds4vizP_graph_ws(g);
    ds4vizP_buf a = {0};
    ds4vizP_args_int(&a, "node_id", nid);
    ds4vizP_args_str(&a, "label", lbl ? lbl : "");
    ds4vizP_write_step(&g->steps, g->step_id++, "update_node_label", a.data, before, g->state_id);
    ds4vizP_buf_free(&a);
}

void ds4vizP_graph_update_weight(ds4vizP_graph *g, int from, int to, double w, int line) {
    if (g->err) return;
    int ef = from, et = to;
    if (!g->directed && ef > et) { int tmp = ef; ef = et; et = tmp; }
    ds4vizP_gedge *e = ds4vizP_graph_findedge(g, ef, et);
    if (!e) { DS4VIZ_ERR_(g, line, "Edge not found: %d -> %d", from, to); return; }
    e->weight = w;
    int before = g->state_id++;
    ds4vizP_graph_ws(g);
    ds4vizP_buf a = {0};
    ds4vizP_args_int(&a, "from_id", from);
    ds4vizP_args_int(&a, "to_id", to);
    ds4vizP_args_dbl(&a, "weight", w);
    ds4vizP_write_step(&g->steps, g->step_id++, "update_weight", a.data, before, g->state_id);
    ds4vizP_buf_free(&a);
}

#endif /* DS4VIZ_IMPLEMENTATION */
