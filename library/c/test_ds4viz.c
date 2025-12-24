/**
 * @file test_ds4viz.c
 * @brief ds4viz库测试文件
 *
 * @author WaterRun
 * @date 2025-12-24
 * @version 0.1.0
 */

#define DS4VIZ_IMPLEMENTATION
#include "ds4viz.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* ========================================================================== */
/* 测试辅助宏                                                                  */
/* ========================================================================== */

#define TEST_ASSERT(cond, msg)                                                              \
    do                                                                                      \
    {                                                                                       \
        if (!(cond))                                                                        \
        {                                                                                   \
            fprintf(stderr, "FAILED: %s\n  File: %s, Line: %d\n", msg, __FILE__, __LINE__); \
            return 1;                                                                       \
        }                                                                                   \
    } while (0)

#define RUN_TEST(test_func)                   \
    do                                        \
    {                                         \
        printf("Running %s... ", #test_func); \
        fflush(stdout);                       \
        if (test_func() == 0)                 \
        {                                     \
            printf("PASSED\n");               \
            passed++;                         \
        }                                     \
        else                                  \
        {                                     \
            printf("FAILED\n");               \
            failed++;                         \
        }                                     \
        total++;                              \
    } while (0)

/* ========================================================================== */
/* 测试函数                                                                    */
/* ========================================================================== */

static int test_version(void)
{
    const char *version = NULL;

    version = dv_version();
    TEST_ASSERT(version != NULL, "dv_version() should return non-NULL");
    TEST_ASSERT(strcmp(version, DS4VIZ_VERSION_STRING) == 0, "Version string mismatch");

    return 0;
}

static int test_config(void)
{
    dv_config("test_output.toml", "Test Title", "Test Author", "Test Comment");
    dv_config(NULL, NULL, NULL, NULL);

    return 0;
}

static int test_stack_basic(void)
{
    dv_stack_t *p_stack = NULL;
    dv_error_t err;

    p_stack = dv_stack_create("test_stack", "test_stack.toml");
    TEST_ASSERT(p_stack != NULL, "Failed to create stack");

    err = dv_stack_push_int(p_stack, 10);
    TEST_ASSERT(err == DV_OK, "Failed to push int");

    err = dv_stack_push_int(p_stack, 20);
    TEST_ASSERT(err == DV_OK, "Failed to push int");

    err = dv_stack_push_float(p_stack, 3.14);
    TEST_ASSERT(err == DV_OK, "Failed to push float");

    err = dv_stack_push_string(p_stack, "hello");
    TEST_ASSERT(err == DV_OK, "Failed to push string");

    err = dv_stack_push_bool(p_stack, true);
    TEST_ASSERT(err == DV_OK, "Failed to push bool");

    err = dv_stack_pop(p_stack);
    TEST_ASSERT(err == DV_OK, "Failed to pop");

    err = dv_stack_pop(p_stack);
    TEST_ASSERT(err == DV_OK, "Failed to pop");

    err = dv_stack_commit(p_stack);
    TEST_ASSERT(err == DV_OK, "Failed to commit");

    dv_stack_destroy(p_stack);

    return 0;
}

static int test_stack_empty_pop(void)
{
    dv_stack_t *p_stack = NULL;
    dv_error_t err;

    p_stack = dv_stack_create("empty_stack", "test_stack_empty.toml");
    TEST_ASSERT(p_stack != NULL, "Failed to create stack");

    err = dv_stack_pop(p_stack);
    TEST_ASSERT(err == DV_ERR_EMPTY, "Should return DV_ERR_EMPTY for empty stack");

    err = dv_stack_commit(p_stack);
    TEST_ASSERT(err == DV_OK, "Failed to commit");

    dv_stack_destroy(p_stack);

    return 0;
}

static int test_stack_clear(void)
{
    dv_stack_t *p_stack = NULL;
    dv_error_t err;

    p_stack = dv_stack_create("clear_stack", "test_stack_clear.toml");
    TEST_ASSERT(p_stack != NULL, "Failed to create stack");

    dv_stack_push_int(p_stack, 1);
    dv_stack_push_int(p_stack, 2);
    dv_stack_push_int(p_stack, 3);

    err = dv_stack_clear(p_stack);
    TEST_ASSERT(err == DV_OK, "Failed to clear stack");

    err = dv_stack_pop(p_stack);
    TEST_ASSERT(err == DV_ERR_EMPTY, "Stack should be empty after clear");

    dv_stack_commit(p_stack);
    dv_stack_destroy(p_stack);

    return 0;
}

static int test_queue_basic(void)
{
    dv_queue_t *p_queue = NULL;
    dv_error_t err;

    p_queue = dv_queue_create("test_queue", "test_queue.toml");
    TEST_ASSERT(p_queue != NULL, "Failed to create queue");

    err = dv_queue_enqueue_int(p_queue, 10);
    TEST_ASSERT(err == DV_OK, "Failed to enqueue int");

    err = dv_queue_enqueue_int(p_queue, 20);
    TEST_ASSERT(err == DV_OK, "Failed to enqueue int");

    err = dv_queue_enqueue_float(p_queue, 3.14);
    TEST_ASSERT(err == DV_OK, "Failed to enqueue float");

    err = dv_queue_dequeue(p_queue);
    TEST_ASSERT(err == DV_OK, "Failed to dequeue");

    err = dv_queue_commit(p_queue);
    TEST_ASSERT(err == DV_OK, "Failed to commit");

    dv_queue_destroy(p_queue);

    return 0;
}

static int test_slist_basic(void)
{
    dv_slist_t *p_slist = NULL;
    dv_error_t err;
    int32_t node_a;
    int32_t node_b;

    p_slist = dv_slist_create("test_slist", "test_slist.toml");
    TEST_ASSERT(p_slist != NULL, "Failed to create slist");

    err = dv_slist_insert_head_int(p_slist, 10, &node_a);
    TEST_ASSERT(err == DV_OK, "Failed to insert head");
    TEST_ASSERT(node_a == 0, "First node id should be 0");

    err = dv_slist_insert_tail_int(p_slist, 20, &node_b);
    TEST_ASSERT(err == DV_OK, "Failed to insert tail");

    err = dv_slist_insert_after_int(p_slist, node_a, 15, NULL);
    TEST_ASSERT(err == DV_OK, "Failed to insert after");

    err = dv_slist_delete(p_slist, node_b);
    TEST_ASSERT(err == DV_OK, "Failed to delete node");

    err = dv_slist_reverse(p_slist);
    TEST_ASSERT(err == DV_OK, "Failed to reverse");

    err = dv_slist_commit(p_slist);
    TEST_ASSERT(err == DV_OK, "Failed to commit");

    dv_slist_destroy(p_slist);

    return 0;
}

static int test_dlist_basic(void)
{
    dv_dlist_t *p_dlist = NULL;
    dv_error_t err;
    int32_t node_a;

    p_dlist = dv_dlist_create("test_dlist", "test_dlist.toml");
    TEST_ASSERT(p_dlist != NULL, "Failed to create dlist");

    err = dv_dlist_insert_head_int(p_dlist, 10, &node_a);
    TEST_ASSERT(err == DV_OK, "Failed to insert head");

    err = dv_dlist_insert_tail_int(p_dlist, 20, NULL);
    TEST_ASSERT(err == DV_OK, "Failed to insert tail");

    err = dv_dlist_insert_tail_int(p_dlist, 30, NULL);
    TEST_ASSERT(err == DV_OK, "Failed to insert tail");

    err = dv_dlist_delete_tail(p_dlist);
    TEST_ASSERT(err == DV_OK, "Failed to delete tail");

    err = dv_dlist_reverse(p_dlist);
    TEST_ASSERT(err == DV_OK, "Failed to reverse");

    err = dv_dlist_commit(p_dlist);
    TEST_ASSERT(err == DV_OK, "Failed to commit");

    dv_dlist_destroy(p_dlist);

    return 0;
}

static int test_binary_tree_basic(void)
{
    dv_binary_tree_t *p_tree = NULL;
    dv_error_t err;
    int32_t root;
    int32_t left;
    int32_t right;

    p_tree = dv_binary_tree_create("test_tree", "test_binary_tree.toml");
    TEST_ASSERT(p_tree != NULL, "Failed to create binary tree");

    err = dv_binary_tree_insert_root_int(p_tree, 10, &root);
    TEST_ASSERT(err == DV_OK, "Failed to insert root");
    TEST_ASSERT(root == 0, "Root id should be 0");

    err = dv_binary_tree_insert_left_int(p_tree, root, 5, &left);
    TEST_ASSERT(err == DV_OK, "Failed to insert left");

    err = dv_binary_tree_insert_right_int(p_tree, root, 15, &right);
    TEST_ASSERT(err == DV_OK, "Failed to insert right");

    err = dv_binary_tree_insert_left_int(p_tree, left, 3, NULL);
    TEST_ASSERT(err == DV_OK, "Failed to insert left child");

    err = dv_binary_tree_update_value_int(p_tree, right, 20);
    TEST_ASSERT(err == DV_OK, "Failed to update value");

    err = dv_binary_tree_delete(p_tree, left);
    TEST_ASSERT(err == DV_OK, "Failed to delete subtree");

    err = dv_binary_tree_commit(p_tree);
    TEST_ASSERT(err == DV_OK, "Failed to commit");

    dv_binary_tree_destroy(p_tree);

    return 0;
}

static int test_binary_tree_errors(void)
{
    dv_binary_tree_t *p_tree = NULL;
    dv_error_t err;
    int32_t root;

    p_tree = dv_binary_tree_create("error_tree", "test_binary_tree_error.toml");
    TEST_ASSERT(p_tree != NULL, "Failed to create binary tree");

    err = dv_binary_tree_insert_root_int(p_tree, 10, &root);
    TEST_ASSERT(err == DV_OK, "Failed to insert root");

    /* 尝试再次插入根节点 */
    err = dv_binary_tree_insert_root_int(p_tree, 20, NULL);
    TEST_ASSERT(err == DV_ERR_ROOT_EXISTS, "Should fail when root exists");

    /* 尝试在不存在的父节点插入 */
    err = dv_binary_tree_insert_left_int(p_tree, 999, 5, NULL);
    TEST_ASSERT(err == DV_ERR_NOT_FOUND, "Should fail for non-existent parent");

    err = dv_binary_tree_commit(p_tree);
    TEST_ASSERT(err == DV_OK, "Failed to commit");

    dv_binary_tree_destroy(p_tree);

    return 0;
}

static int test_graph_undirected_basic(void)
{
    dv_graph_undirected_t *p_graph = NULL;
    dv_error_t err;

    p_graph = dv_graph_undirected_create("test_graph", "test_graph_undirected.toml");
    TEST_ASSERT(p_graph != NULL, "Failed to create graph");

    err = dv_graph_undirected_add_node(p_graph, 0, "A");
    TEST_ASSERT(err == DV_OK, "Failed to add node A");

    err = dv_graph_undirected_add_node(p_graph, 1, "B");
    TEST_ASSERT(err == DV_OK, "Failed to add node B");

    err = dv_graph_undirected_add_node(p_graph, 2, "C");
    TEST_ASSERT(err == DV_OK, "Failed to add node C");

    err = dv_graph_undirected_add_edge(p_graph, 0, 1);
    TEST_ASSERT(err == DV_OK, "Failed to add edge 0-1");

    err = dv_graph_undirected_add_edge(p_graph, 1, 2);
    TEST_ASSERT(err == DV_OK, "Failed to add edge 1-2");

    err = dv_graph_undirected_update_node_label(p_graph, 1, "B_updated");
    TEST_ASSERT(err == DV_OK, "Failed to update label");

    err = dv_graph_undirected_remove_edge(p_graph, 0, 1);
    TEST_ASSERT(err == DV_OK, "Failed to remove edge");

    err = dv_graph_undirected_commit(p_graph);
    TEST_ASSERT(err == DV_OK, "Failed to commit");

    dv_graph_undirected_destroy(p_graph);

    return 0;
}

static int test_graph_errors(void)
{
    dv_graph_undirected_t *p_graph = NULL;
    dv_error_t err;

    p_graph = dv_graph_undirected_create("error_graph", "test_graph_error.toml");
    TEST_ASSERT(p_graph != NULL, "Failed to create graph");

    err = dv_graph_undirected_add_node(p_graph, 0, "A");
    TEST_ASSERT(err == DV_OK, "Failed to add node");

    /* 重复添加节点 */
    err = dv_graph_undirected_add_node(p_graph, 0, "A2");
    TEST_ASSERT(err == DV_ERR_ALREADY_EXISTS, "Should fail for duplicate node");

    /* 自环 */
    err = dv_graph_undirected_add_edge(p_graph, 0, 0);
    TEST_ASSERT(err == DV_ERR_SELF_LOOP, "Should fail for self-loop");

    /* 不存在的节点 */
    err = dv_graph_undirected_add_edge(p_graph, 0, 999);
    TEST_ASSERT(err == DV_ERR_NOT_FOUND, "Should fail for non-existent node");

    dv_graph_undirected_commit(p_graph);
    dv_graph_undirected_destroy(p_graph);

    return 0;
}

static int test_null_pointers(void)
{
    dv_error_t err;

    err = dv_stack_push_int(NULL, 10);
    TEST_ASSERT(err == DV_ERR_NULL_PTR, "Should return DV_ERR_NULL_PTR");

    err = dv_stack_pop(NULL);
    TEST_ASSERT(err == DV_ERR_NULL_PTR, "Should return DV_ERR_NULL_PTR");

    err = dv_queue_enqueue_int(NULL, 10);
    TEST_ASSERT(err == DV_ERR_NULL_PTR, "Should return DV_ERR_NULL_PTR");

    err = dv_binary_tree_commit(NULL);
    TEST_ASSERT(err == DV_ERR_NULL_PTR, "Should return DV_ERR_NULL_PTR");

    dv_stack_destroy(NULL);
    dv_queue_destroy(NULL);
    dv_slist_destroy(NULL);
    dv_dlist_destroy(NULL);
    dv_binary_tree_destroy(NULL);
    dv_graph_undirected_destroy(NULL);

    return 0;
}

/* ========================================================================== */
/* 主函数                                                                      */
/* ========================================================================== */

int main(void)
{
    int passed = 0;
    int failed = 0;
    int total = 0;

    printf("=== ds4viz C Library Tests ===\n\n");

    RUN_TEST(test_version);
    RUN_TEST(test_config);
    RUN_TEST(test_stack_basic);
    RUN_TEST(test_stack_empty_pop);
    RUN_TEST(test_stack_clear);
    RUN_TEST(test_queue_basic);
    RUN_TEST(test_slist_basic);
    RUN_TEST(test_dlist_basic);
    RUN_TEST(test_binary_tree_basic);
    RUN_TEST(test_binary_tree_errors);
    RUN_TEST(test_graph_undirected_basic);
    RUN_TEST(test_graph_errors);
    RUN_TEST(test_null_pointers);

    printf("\n=== Test Summary ===\n");
    printf("Total: %d, Passed: %d, Failed: %d\n", total, passed, failed);

    return failed > 0 ? 1 : 0;
}
