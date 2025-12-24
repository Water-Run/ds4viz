/**
 * @file test_ds4viz.c
 * @brief ds4viz库测试文件
 *
 * 包含约70个测试用例，覆盖：
 * - 基本功能测试
 * - 边缘情况测试
 * - 错误处理测试
 * - TOML输出格式验证
 * - IR规范契合度检查
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
#include <stdbool.h>

/* ========================================================================== */
/* 测试辅助宏和函数                                                            */
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

/**
 * @brief 读取文件内容到字符串
 */
static char *read_file_content(const char *filename)
{
    FILE *fp = NULL;
    long file_size;
    char *content = NULL;
    size_t read_size;

    fp = fopen(filename, "rb");
    if (fp == NULL)
    {
        return NULL;
    }

    fseek(fp, 0, SEEK_END);
    file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    content = (char *)malloc(file_size + 1);
    if (content == NULL)
    {
        fclose(fp);
        return NULL;
    }

    read_size = fread(content, 1, file_size, fp);
    content[read_size] = '\0';

    fclose(fp);
    return content;
}

/**
 * @brief 检查字符串是否包含子串
 */
static bool str_contains(const char *haystack, const char *needle)
{
    if (haystack == NULL || needle == NULL)
    {
        return false;
    }
    return strstr(haystack, needle) != NULL;
}

/**
 * @brief 删除测试生成的文件
 */
static void cleanup_file(const char *filename)
{
    remove(filename);
}

/* ========================================================================== */
/* 版本和配置测试                                                              */
/* ========================================================================== */

static int test_version(void)
{
    const char *version = NULL;

    version = dv_version();
    TEST_ASSERT(version != NULL, "dv_version() should return non-NULL");
    TEST_ASSERT(strcmp(version, DS4VIZ_VERSION_STRING) == 0, "Version string mismatch");
    TEST_ASSERT(strcmp(version, "0.1.0") == 0, "Version should be 0.1.0");

    return 0;
}

static int test_version_macros(void)
{
    TEST_ASSERT(DS4VIZ_VERSION_MAJOR == 0, "Major version should be 0");
    TEST_ASSERT(DS4VIZ_VERSION_MINOR == 1, "Minor version should be 1");
    TEST_ASSERT(DS4VIZ_VERSION_PATCH == 0, "Patch version should be 0");

    return 0;
}

static int test_config_basic(void)
{
    dv_config("test_output.toml", "Test Title", "Test Author", "Test Comment");
    dv_config(NULL, NULL, NULL, NULL);

    return 0;
}

static int test_config_with_stack_output(void)
{
    dv_stack_t *p_stack = NULL;
    char *content = NULL;
    const char *filename = "test_config_remarks.toml";

    dv_config(NULL, "Config Title", "Config Author", "Config Comment");

    p_stack = dv_stack_create("stack", filename);
    TEST_ASSERT(p_stack != NULL, "Failed to create stack");

    dv_stack_push_int(p_stack, 42);
    dv_stack_commit(p_stack);
    dv_stack_destroy(p_stack);

    content = read_file_content(filename);
    TEST_ASSERT(content != NULL, "Failed to read output file");

    TEST_ASSERT(str_contains(content, "[remarks]"), "Should contain [remarks] section");
    TEST_ASSERT(str_contains(content, "title = \"Config Title\""), "Should contain title");
    TEST_ASSERT(str_contains(content, "author = \"Config Author\""), "Should contain author");
    TEST_ASSERT(str_contains(content, "comment = \"Config Comment\""), "Should contain comment");

    free(content);
    cleanup_file(filename);
    dv_config(NULL, NULL, NULL, NULL);

    return 0;
}

/* ========================================================================== */
/* 栈测试                                                                      */
/* ========================================================================== */

static int test_stack_create_destroy(void)
{
    dv_stack_t *p_stack = NULL;

    p_stack = dv_stack_create(NULL, NULL);
    TEST_ASSERT(p_stack != NULL, "Should create stack with NULL label");
    dv_stack_destroy(p_stack);

    p_stack = dv_stack_create("custom_label", NULL);
    TEST_ASSERT(p_stack != NULL, "Should create stack with custom label");
    dv_stack_destroy(p_stack);

    p_stack = dv_stack_create("stack", "custom_output.toml");
    TEST_ASSERT(p_stack != NULL, "Should create stack with custom output");
    dv_stack_destroy(p_stack);

    return 0;
}

static int test_stack_push_all_types(void)
{
    dv_stack_t *p_stack = NULL;
    dv_error_t err;
    char *content = NULL;
    const char *filename = "test_stack_types.toml";

    p_stack = dv_stack_create("stack", filename);
    TEST_ASSERT(p_stack != NULL, "Failed to create stack");

    err = dv_stack_push_int(p_stack, 42);
    TEST_ASSERT(err == DV_OK, "Failed to push int");

    err = dv_stack_push_int(p_stack, -100);
    TEST_ASSERT(err == DV_OK, "Failed to push negative int");

    err = dv_stack_push_int(p_stack, 0);
    TEST_ASSERT(err == DV_OK, "Failed to push zero");

    err = dv_stack_push_float(p_stack, 3.14159);
    TEST_ASSERT(err == DV_OK, "Failed to push float");

    err = dv_stack_push_float(p_stack, -2.71828);
    TEST_ASSERT(err == DV_OK, "Failed to push negative float");

    err = dv_stack_push_string(p_stack, "hello");
    TEST_ASSERT(err == DV_OK, "Failed to push string");

    err = dv_stack_push_string(p_stack, "");
    TEST_ASSERT(err == DV_OK, "Failed to push empty string");

    err = dv_stack_push_bool(p_stack, true);
    TEST_ASSERT(err == DV_OK, "Failed to push true");

    err = dv_stack_push_bool(p_stack, false);
    TEST_ASSERT(err == DV_OK, "Failed to push false");

    err = dv_stack_commit(p_stack);
    TEST_ASSERT(err == DV_OK, "Failed to commit");

    content = read_file_content(filename);
    TEST_ASSERT(content != NULL, "Failed to read output file");

    TEST_ASSERT(str_contains(content, "42"), "Should contain int value");
    TEST_ASSERT(str_contains(content, "-100"), "Should contain negative int");
    TEST_ASSERT(str_contains(content, "3.14159"), "Should contain float value");
    TEST_ASSERT(str_contains(content, "\"hello\""), "Should contain string value");
    TEST_ASSERT(str_contains(content, "true"), "Should contain true");
    TEST_ASSERT(str_contains(content, "false"), "Should contain false");

    free(content);
    dv_stack_destroy(p_stack);
    cleanup_file(filename);

    return 0;
}

static int test_stack_pop_sequence(void)
{
    dv_stack_t *p_stack = NULL;
    dv_error_t err;
    char *content = NULL;
    const char *filename = "test_stack_pop.toml";

    p_stack = dv_stack_create("stack", filename);
    TEST_ASSERT(p_stack != NULL, "Failed to create stack");

    dv_stack_push_int(p_stack, 1);
    dv_stack_push_int(p_stack, 2);
    dv_stack_push_int(p_stack, 3);

    err = dv_stack_pop(p_stack);
    TEST_ASSERT(err == DV_OK, "Failed to pop");

    err = dv_stack_pop(p_stack);
    TEST_ASSERT(err == DV_OK, "Failed to pop");

    err = dv_stack_pop(p_stack);
    TEST_ASSERT(err == DV_OK, "Failed to pop");

    err = dv_stack_commit(p_stack);
    TEST_ASSERT(err == DV_OK, "Failed to commit");

    content = read_file_content(filename);
    TEST_ASSERT(content != NULL, "Failed to read output file");

    /* 验证最终状态为空栈 */
    TEST_ASSERT(str_contains(content, "items = []"), "Final state should be empty");
    TEST_ASSERT(str_contains(content, "top = -1"), "Top should be -1 for empty stack");

    free(content);
    dv_stack_destroy(p_stack);
    cleanup_file(filename);

    return 0;
}

static int test_stack_empty_pop(void)
{
    dv_stack_t *p_stack = NULL;
    dv_error_t err;
    char *content = NULL;
    const char *filename = "test_stack_empty.toml";

    p_stack = dv_stack_create("stack", filename);
    TEST_ASSERT(p_stack != NULL, "Failed to create stack");

    err = dv_stack_pop(p_stack);
    TEST_ASSERT(err == DV_ERR_EMPTY, "Should return DV_ERR_EMPTY");

    err = dv_stack_commit(p_stack);
    TEST_ASSERT(err == DV_OK, "Failed to commit");

    content = read_file_content(filename);
    TEST_ASSERT(content != NULL, "Failed to read output file");

    /* 验证包含error section */
    TEST_ASSERT(str_contains(content, "[error]"), "Should contain [error] section");
    TEST_ASSERT(str_contains(content, "type = \"runtime\""), "Error type should be runtime");
    TEST_ASSERT(str_contains(content, "empty stack"), "Error message should mention empty stack");

    free(content);
    dv_stack_destroy(p_stack);
    cleanup_file(filename);

    return 0;
}

static int test_stack_clear(void)
{
    dv_stack_t *p_stack = NULL;
    dv_error_t err;
    char *content = NULL;
    const char *filename = "test_stack_clear.toml";

    p_stack = dv_stack_create("stack", filename);
    TEST_ASSERT(p_stack != NULL, "Failed to create stack");

    dv_stack_push_int(p_stack, 1);
    dv_stack_push_int(p_stack, 2);
    dv_stack_push_int(p_stack, 3);

    err = dv_stack_clear(p_stack);
    TEST_ASSERT(err == DV_OK, "Failed to clear");

    err = dv_stack_pop(p_stack);
    TEST_ASSERT(err == DV_ERR_EMPTY, "Should be empty after clear");

    dv_stack_commit(p_stack);

    content = read_file_content(filename);
    TEST_ASSERT(content != NULL, "Failed to read output file");

    TEST_ASSERT(str_contains(content, "op = \"clear\""), "Should contain clear operation");

    free(content);
    dv_stack_destroy(p_stack);
    cleanup_file(filename);

    return 0;
}

static int test_stack_toml_structure(void)
{
    dv_stack_t *p_stack = NULL;
    char *content = NULL;
    const char *filename = "test_stack_structure.toml";

    p_stack = dv_stack_create("my_stack", filename);
    TEST_ASSERT(p_stack != NULL, "Failed to create stack");

    dv_stack_push_int(p_stack, 10);
    dv_stack_push_int(p_stack, 20);
    dv_stack_pop(p_stack);
    dv_stack_commit(p_stack);

    content = read_file_content(filename);
    TEST_ASSERT(content != NULL, "Failed to read output file");

    /* 验证必需的顶层结构 */
    TEST_ASSERT(str_contains(content, "[meta]"), "Should contain [meta] section");
    TEST_ASSERT(str_contains(content, "[package]"), "Should contain [package] section");
    TEST_ASSERT(str_contains(content, "[object]"), "Should contain [object] section");
    TEST_ASSERT(str_contains(content, "[[states]]"), "Should contain [[states]] section");
    TEST_ASSERT(str_contains(content, "[[steps]]"), "Should contain [[steps]] section");
    TEST_ASSERT(str_contains(content, "[result]"), "Should contain [result] section");

    /* 验证meta字段 */
    TEST_ASSERT(str_contains(content, "created_at = "), "Should contain created_at");
    TEST_ASSERT(str_contains(content, "lang = \"c\""), "Should contain lang = c");
    TEST_ASSERT(str_contains(content, "lang_version = \"17\""), "Should contain lang_version");

    /* 验证package字段 */
    TEST_ASSERT(str_contains(content, "name = \"ds4viz\""), "Should contain package name");
    TEST_ASSERT(str_contains(content, "version = \"0.1.0\""), "Should contain package version");

    /* 验证object字段 */
    TEST_ASSERT(str_contains(content, "kind = \"stack\""), "Should contain kind = stack");
    TEST_ASSERT(str_contains(content, "label = \"my_stack\""), "Should contain label");

    /* 验证steps字段 */
    TEST_ASSERT(str_contains(content, "op = \"push\""), "Should contain push operation");
    TEST_ASSERT(str_contains(content, "op = \"pop\""), "Should contain pop operation");
    TEST_ASSERT(str_contains(content, "[steps.args]"), "Should contain [steps.args]");

    /* 验证result字段 */
    TEST_ASSERT(str_contains(content, "final_state = "), "Should contain final_state");

    free(content);
    dv_stack_destroy(p_stack);
    cleanup_file(filename);

    return 0;
}

static int test_stack_state_ids_sequential(void)
{
    dv_stack_t *p_stack = NULL;
    char *content = NULL;
    const char *filename = "test_stack_state_ids.toml";

    p_stack = dv_stack_create("stack", filename);
    dv_stack_push_int(p_stack, 1);
    dv_stack_push_int(p_stack, 2);
    dv_stack_push_int(p_stack, 3);
    dv_stack_commit(p_stack);

    content = read_file_content(filename);
    TEST_ASSERT(content != NULL, "Failed to read output file");

    /* 验证state id从0开始连续递增 */
    TEST_ASSERT(str_contains(content, "id = 0"), "Should have state id 0");
    TEST_ASSERT(str_contains(content, "id = 1"), "Should have state id 1");
    TEST_ASSERT(str_contains(content, "id = 2"), "Should have state id 2");
    TEST_ASSERT(str_contains(content, "id = 3"), "Should have state id 3");

    free(content);
    dv_stack_destroy(p_stack);
    cleanup_file(filename);

    return 0;
}

static int test_stack_large_capacity(void)
{
    dv_stack_t *p_stack = NULL;
    dv_error_t err;
    int i;
    const char *filename = "test_stack_large.toml";

    p_stack = dv_stack_create("stack", filename);
    TEST_ASSERT(p_stack != NULL, "Failed to create stack");

    /* 测试超过初始容量 */
    for (i = 0; i < 100; i++)
    {
        err = dv_stack_push_int(p_stack, i);
        TEST_ASSERT(err == DV_OK, "Failed to push during capacity test");
    }

    for (i = 0; i < 50; i++)
    {
        err = dv_stack_pop(p_stack);
        TEST_ASSERT(err == DV_OK, "Failed to pop during capacity test");
    }

    err = dv_stack_commit(p_stack);
    TEST_ASSERT(err == DV_OK, "Failed to commit");

    dv_stack_destroy(p_stack);
    cleanup_file(filename);

    return 0;
}

/* ========================================================================== */
/* 队列测试                                                                    */
/* ========================================================================== */

static int test_queue_create_destroy(void)
{
    dv_queue_t *p_queue = NULL;

    p_queue = dv_queue_create(NULL, NULL);
    TEST_ASSERT(p_queue != NULL, "Should create queue with NULL label");
    dv_queue_destroy(p_queue);

    p_queue = dv_queue_create("custom_queue", "custom_queue.toml");
    TEST_ASSERT(p_queue != NULL, "Should create queue with custom params");
    dv_queue_destroy(p_queue);

    return 0;
}

static int test_queue_enqueue_all_types(void)
{
    dv_queue_t *p_queue = NULL;
    dv_error_t err;
    char *content = NULL;
    const char *filename = "test_queue_types.toml";

    p_queue = dv_queue_create("queue", filename);
    TEST_ASSERT(p_queue != NULL, "Failed to create queue");

    err = dv_queue_enqueue_int(p_queue, 42);
    TEST_ASSERT(err == DV_OK, "Failed to enqueue int");

    err = dv_queue_enqueue_float(p_queue, 3.14);
    TEST_ASSERT(err == DV_OK, "Failed to enqueue float");

    err = dv_queue_enqueue_string(p_queue, "test");
    TEST_ASSERT(err == DV_OK, "Failed to enqueue string");

    err = dv_queue_enqueue_bool(p_queue, true);
    TEST_ASSERT(err == DV_OK, "Failed to enqueue bool");

    err = dv_queue_commit(p_queue);
    TEST_ASSERT(err == DV_OK, "Failed to commit");

    content = read_file_content(filename);
    TEST_ASSERT(content != NULL, "Failed to read output file");

    TEST_ASSERT(str_contains(content, "kind = \"queue\""), "Should have kind = queue");
    TEST_ASSERT(str_contains(content, "op = \"enqueue\""), "Should have enqueue operation");

    free(content);
    dv_queue_destroy(p_queue);
    cleanup_file(filename);

    return 0;
}

static int test_queue_dequeue_sequence(void)
{
    dv_queue_t *p_queue = NULL;
    dv_error_t err;
    char *content = NULL;
    const char *filename = "test_queue_dequeue.toml";

    p_queue = dv_queue_create("queue", filename);
    TEST_ASSERT(p_queue != NULL, "Failed to create queue");

    dv_queue_enqueue_int(p_queue, 1);
    dv_queue_enqueue_int(p_queue, 2);
    dv_queue_enqueue_int(p_queue, 3);

    err = dv_queue_dequeue(p_queue);
    TEST_ASSERT(err == DV_OK, "Failed to dequeue");

    err = dv_queue_dequeue(p_queue);
    TEST_ASSERT(err == DV_OK, "Failed to dequeue");

    dv_queue_commit(p_queue);

    content = read_file_content(filename);
    TEST_ASSERT(content != NULL, "Failed to read output file");

    TEST_ASSERT(str_contains(content, "op = \"dequeue\""), "Should have dequeue operation");

    free(content);
    dv_queue_destroy(p_queue);
    cleanup_file(filename);

    return 0;
}

static int test_queue_empty_dequeue(void)
{
    dv_queue_t *p_queue = NULL;
    dv_error_t err;
    char *content = NULL;
    const char *filename = "test_queue_empty.toml";

    p_queue = dv_queue_create("queue", filename);
    TEST_ASSERT(p_queue != NULL, "Failed to create queue");

    err = dv_queue_dequeue(p_queue);
    TEST_ASSERT(err == DV_ERR_EMPTY, "Should return DV_ERR_EMPTY");

    dv_queue_commit(p_queue);

    content = read_file_content(filename);
    TEST_ASSERT(content != NULL, "Failed to read output file");

    TEST_ASSERT(str_contains(content, "[error]"), "Should contain error section");

    free(content);
    dv_queue_destroy(p_queue);
    cleanup_file(filename);

    return 0;
}

static int test_queue_clear(void)
{
    dv_queue_t *p_queue = NULL;
    dv_error_t err;
    const char *filename = "test_queue_clear.toml";

    p_queue = dv_queue_create("queue", filename);
    TEST_ASSERT(p_queue != NULL, "Failed to create queue");

    dv_queue_enqueue_int(p_queue, 1);
    dv_queue_enqueue_int(p_queue, 2);

    err = dv_queue_clear(p_queue);
    TEST_ASSERT(err == DV_OK, "Failed to clear");

    err = dv_queue_dequeue(p_queue);
    TEST_ASSERT(err == DV_ERR_EMPTY, "Should be empty after clear");

    dv_queue_commit(p_queue);
    dv_queue_destroy(p_queue);
    cleanup_file(filename);

    return 0;
}

static int test_queue_toml_front_rear(void)
{
    dv_queue_t *p_queue = NULL;
    char *content = NULL;
    const char *filename = "test_queue_frontrear.toml";

    p_queue = dv_queue_create("queue", filename);
    dv_queue_enqueue_int(p_queue, 10);
    dv_queue_enqueue_int(p_queue, 20);
    dv_queue_commit(p_queue);

    content = read_file_content(filename);
    TEST_ASSERT(content != NULL, "Failed to read output file");

    /* 验证非空队列的front/rear */
    TEST_ASSERT(str_contains(content, "front = 0"), "Should have front = 0");
    TEST_ASSERT(str_contains(content, "rear = 1"), "Should have rear = 1");

    free(content);
    dv_queue_destroy(p_queue);
    cleanup_file(filename);

    return 0;
}

static int test_queue_empty_state(void)
{
    dv_queue_t *p_queue = NULL;
    char *content = NULL;
    const char *filename = "test_queue_emptystate.toml";

    p_queue = dv_queue_create("queue", filename);
    dv_queue_commit(p_queue);

    content = read_file_content(filename);
    TEST_ASSERT(content != NULL, "Failed to read output file");

    /* 验证空队列的front/rear都是-1 */
    TEST_ASSERT(str_contains(content, "items = []"), "Should have empty items");
    TEST_ASSERT(str_contains(content, "front = -1"), "Should have front = -1");
    TEST_ASSERT(str_contains(content, "rear = -1"), "Should have rear = -1");

    free(content);
    dv_queue_destroy(p_queue);
    cleanup_file(filename);

    return 0;
}

/* ========================================================================== */
/* 单链表测试                                                                  */
/* ========================================================================== */

static int test_slist_create_destroy(void)
{
    dv_slist_t *p_slist = NULL;

    p_slist = dv_slist_create(NULL, NULL);
    TEST_ASSERT(p_slist != NULL, "Should create slist with NULL params");
    dv_slist_destroy(p_slist);

    p_slist = dv_slist_create("my_list", "my_list.toml");
    TEST_ASSERT(p_slist != NULL, "Should create slist with custom params");
    dv_slist_destroy(p_slist);

    return 0;
}

static int test_slist_insert_head(void)
{
    dv_slist_t *p_slist = NULL;
    dv_error_t err;
    int32_t node_id;
    char *content = NULL;
    const char *filename = "test_slist_head.toml";

    p_slist = dv_slist_create("slist", filename);
    TEST_ASSERT(p_slist != NULL, "Failed to create slist");

    err = dv_slist_insert_head_int(p_slist, 10, &node_id);
    TEST_ASSERT(err == DV_OK, "Failed to insert head");
    TEST_ASSERT(node_id == 0, "First node id should be 0");

    err = dv_slist_insert_head_int(p_slist, 20, &node_id);
    TEST_ASSERT(err == DV_OK, "Failed to insert head");
    TEST_ASSERT(node_id == 1, "Second node id should be 1");

    dv_slist_commit(p_slist);

    content = read_file_content(filename);
    TEST_ASSERT(content != NULL, "Failed to read output file");

    TEST_ASSERT(str_contains(content, "kind = \"slist\""), "Should have kind = slist");
    TEST_ASSERT(str_contains(content, "head = 1"), "Head should be the last inserted");
    TEST_ASSERT(str_contains(content, "op = \"insert_head\""), "Should have insert_head op");

    free(content);
    dv_slist_destroy(p_slist);
    cleanup_file(filename);

    return 0;
}

static int test_slist_insert_tail(void)
{
    dv_slist_t *p_slist = NULL;
    dv_error_t err;
    int32_t node_a, node_b;
    char *content = NULL;
    const char *filename = "test_slist_tail.toml";

    p_slist = dv_slist_create("slist", filename);

    err = dv_slist_insert_tail_int(p_slist, 10, &node_a);
    TEST_ASSERT(err == DV_OK, "Failed to insert tail");

    err = dv_slist_insert_tail_int(p_slist, 20, &node_b);
    TEST_ASSERT(err == DV_OK, "Failed to insert tail");

    dv_slist_commit(p_slist);

    content = read_file_content(filename);
    TEST_ASSERT(content != NULL, "Failed to read output file");

    TEST_ASSERT(str_contains(content, "head = 0"), "Head should be first inserted");
    TEST_ASSERT(str_contains(content, "op = \"insert_tail\""), "Should have insert_tail op");

    free(content);
    dv_slist_destroy(p_slist);
    cleanup_file(filename);

    return 0;
}

static int test_slist_insert_after(void)
{
    dv_slist_t *p_slist = NULL;
    dv_error_t err;
    int32_t node_a, node_new;
    const char *filename = "test_slist_after.toml";

    p_slist = dv_slist_create("slist", filename);

    dv_slist_insert_head_int(p_slist, 10, &node_a);
    dv_slist_insert_tail_int(p_slist, 30, NULL);

    err = dv_slist_insert_after_int(p_slist, node_a, 20, &node_new);
    TEST_ASSERT(err == DV_OK, "Failed to insert after");

    dv_slist_commit(p_slist);
    dv_slist_destroy(p_slist);
    cleanup_file(filename);

    return 0;
}

static int test_slist_insert_after_not_found(void)
{
    dv_slist_t *p_slist = NULL;
    dv_error_t err;
    char *content = NULL;
    const char *filename = "test_slist_notfound.toml";

    p_slist = dv_slist_create("slist", filename);

    dv_slist_insert_head_int(p_slist, 10, NULL);

    err = dv_slist_insert_after_int(p_slist, 999, 20, NULL);
    TEST_ASSERT(err == DV_ERR_NOT_FOUND, "Should return NOT_FOUND");

    dv_slist_commit(p_slist);

    content = read_file_content(filename);
    TEST_ASSERT(content != NULL, "Failed to read output file");

    TEST_ASSERT(str_contains(content, "[error]"), "Should contain error section");
    TEST_ASSERT(str_contains(content, "Node not found"), "Should mention node not found");

    free(content);
    dv_slist_destroy(p_slist);
    cleanup_file(filename);

    return 0;
}

static int test_slist_delete(void)
{
    dv_slist_t *p_slist = NULL;
    dv_error_t err;
    int32_t node_a, node_b;
    const char *filename = "test_slist_delete.toml";

    p_slist = dv_slist_create("slist", filename);

    dv_slist_insert_head_int(p_slist, 10, &node_a);
    dv_slist_insert_tail_int(p_slist, 20, &node_b);

    err = dv_slist_delete(p_slist, node_b);
    TEST_ASSERT(err == DV_OK, "Failed to delete");

    err = dv_slist_delete(p_slist, node_b);
    TEST_ASSERT(err == DV_ERR_NOT_FOUND, "Should fail for deleted node");

    dv_slist_commit(p_slist);
    dv_slist_destroy(p_slist);
    cleanup_file(filename);

    return 0;
}

static int test_slist_delete_head(void)
{
    dv_slist_t *p_slist = NULL;
    dv_error_t err;
    const char *filename = "test_slist_delhead.toml";

    p_slist = dv_slist_create("slist", filename);

    dv_slist_insert_head_int(p_slist, 10, NULL);
    dv_slist_insert_head_int(p_slist, 20, NULL);

    err = dv_slist_delete_head(p_slist);
    TEST_ASSERT(err == DV_OK, "Failed to delete head");

    dv_slist_commit(p_slist);
    dv_slist_destroy(p_slist);
    cleanup_file(filename);

    return 0;
}

static int test_slist_delete_head_empty(void)
{
    dv_slist_t *p_slist = NULL;
    dv_error_t err;
    const char *filename = "test_slist_delempty.toml";

    p_slist = dv_slist_create("slist", filename);

    err = dv_slist_delete_head(p_slist);
    TEST_ASSERT(err == DV_ERR_EMPTY, "Should return EMPTY");

    dv_slist_commit(p_slist);
    dv_slist_destroy(p_slist);
    cleanup_file(filename);

    return 0;
}

static int test_slist_reverse(void)
{
    dv_slist_t *p_slist = NULL;
    dv_error_t err;
    char *content = NULL;
    const char *filename = "test_slist_reverse.toml";

    p_slist = dv_slist_create("slist", filename);

    dv_slist_insert_tail_int(p_slist, 1, NULL);
    dv_slist_insert_tail_int(p_slist, 2, NULL);
    dv_slist_insert_tail_int(p_slist, 3, NULL);

    err = dv_slist_reverse(p_slist);
    TEST_ASSERT(err == DV_OK, "Failed to reverse");

    dv_slist_commit(p_slist);

    content = read_file_content(filename);
    TEST_ASSERT(content != NULL, "Failed to read output file");

    TEST_ASSERT(str_contains(content, "op = \"reverse\""), "Should have reverse op");

    free(content);
    dv_slist_destroy(p_slist);
    cleanup_file(filename);

    return 0;
}

static int test_slist_all_types(void)
{
    dv_slist_t *p_slist = NULL;
    dv_error_t err;
    const char *filename = "test_slist_types.toml";

    p_slist = dv_slist_create("slist", filename);

    err = dv_slist_insert_head_int(p_slist, 42, NULL);
    TEST_ASSERT(err == DV_OK, "Failed to insert int");

    err = dv_slist_insert_head_float(p_slist, 3.14, NULL);
    TEST_ASSERT(err == DV_OK, "Failed to insert float");

    err = dv_slist_insert_head_string(p_slist, "hello", NULL);
    TEST_ASSERT(err == DV_OK, "Failed to insert string");

    err = dv_slist_insert_head_bool(p_slist, true, NULL);
    TEST_ASSERT(err == DV_OK, "Failed to insert bool");

    dv_slist_commit(p_slist);
    dv_slist_destroy(p_slist);
    cleanup_file(filename);

    return 0;
}

static int test_slist_toml_node_format(void)
{
    dv_slist_t *p_slist = NULL;
    char *content = NULL;
    const char *filename = "test_slist_format.toml";

    p_slist = dv_slist_create("slist", filename);

    dv_slist_insert_head_int(p_slist, 10, NULL);
    dv_slist_insert_tail_int(p_slist, 20, NULL);

    dv_slist_commit(p_slist);

    content = read_file_content(filename);
    TEST_ASSERT(content != NULL, "Failed to read output file");

    /* 验证节点格式包含id, value, next */
    TEST_ASSERT(str_contains(content, "id = 0"), "Should have node id");
    TEST_ASSERT(str_contains(content, "value = "), "Should have node value");
    TEST_ASSERT(str_contains(content, "next = "), "Should have node next");

    free(content);
    dv_slist_destroy(p_slist);
    cleanup_file(filename);

    return 0;
}

/* ========================================================================== */
/* 双向链表测试                                                                */
/* ========================================================================== */

static int test_dlist_create_destroy(void)
{
    dv_dlist_t *p_dlist = NULL;

    p_dlist = dv_dlist_create(NULL, NULL);
    TEST_ASSERT(p_dlist != NULL, "Should create dlist");
    dv_dlist_destroy(p_dlist);

    return 0;
}

static int test_dlist_insert_head_tail(void)
{
    dv_dlist_t *p_dlist = NULL;
    dv_error_t err;
    int32_t head_id, tail_id;
    char *content = NULL;
    const char *filename = "test_dlist_basic.toml";

    p_dlist = dv_dlist_create("dlist", filename);

    err = dv_dlist_insert_head_int(p_dlist, 10, &head_id);
    TEST_ASSERT(err == DV_OK, "Failed to insert head");

    err = dv_dlist_insert_tail_int(p_dlist, 20, &tail_id);
    TEST_ASSERT(err == DV_OK, "Failed to insert tail");

    dv_dlist_commit(p_dlist);

    content = read_file_content(filename);
    TEST_ASSERT(content != NULL, "Failed to read output file");

    TEST_ASSERT(str_contains(content, "kind = \"dlist\""), "Should have kind = dlist");
    TEST_ASSERT(str_contains(content, "head = "), "Should have head field");
    TEST_ASSERT(str_contains(content, "tail = "), "Should have tail field");
    TEST_ASSERT(str_contains(content, "prev = "), "Should have prev in nodes");

    free(content);
    dv_dlist_destroy(p_dlist);
    cleanup_file(filename);

    return 0;
}

static int test_dlist_delete_operations(void)
{
    dv_dlist_t *p_dlist = NULL;
    dv_error_t err;
    int32_t node_id;
    const char *filename = "test_dlist_delete.toml";

    p_dlist = dv_dlist_create("dlist", filename);

    dv_dlist_insert_head_int(p_dlist, 10, &node_id);
    dv_dlist_insert_tail_int(p_dlist, 20, NULL);
    dv_dlist_insert_tail_int(p_dlist, 30, NULL);

    err = dv_dlist_delete(p_dlist, node_id);
    TEST_ASSERT(err == DV_OK, "Failed to delete node");

    err = dv_dlist_delete_tail(p_dlist);
    TEST_ASSERT(err == DV_OK, "Failed to delete tail");

    dv_dlist_commit(p_dlist);
    dv_dlist_destroy(p_dlist);
    cleanup_file(filename);

    return 0;
}

static int test_dlist_delete_empty(void)
{
    dv_dlist_t *p_dlist = NULL;
    dv_error_t err;
    const char *filename = "test_dlist_empty.toml";

    p_dlist = dv_dlist_create("dlist", filename);

    err = dv_dlist_delete_head(p_dlist);
    TEST_ASSERT(err == DV_ERR_EMPTY, "Should return EMPTY");

    err = dv_dlist_delete_tail(p_dlist);
    TEST_ASSERT(err == DV_ERR_EMPTY, "Should return EMPTY");

    dv_dlist_commit(p_dlist);
    dv_dlist_destroy(p_dlist);
    cleanup_file(filename);

    return 0;
}

static int test_dlist_reverse(void)
{
    dv_dlist_t *p_dlist = NULL;
    dv_error_t err;
    char *content = NULL;
    const char *filename = "test_dlist_reverse.toml";

    p_dlist = dv_dlist_create("dlist", filename);

    dv_dlist_insert_tail_int(p_dlist, 1, NULL);
    dv_dlist_insert_tail_int(p_dlist, 2, NULL);
    dv_dlist_insert_tail_int(p_dlist, 3, NULL);

    err = dv_dlist_reverse(p_dlist);
    TEST_ASSERT(err == DV_OK, "Failed to reverse");

    dv_dlist_commit(p_dlist);

    content = read_file_content(filename);
    TEST_ASSERT(content != NULL, "Failed to read output file");

    TEST_ASSERT(str_contains(content, "op = \"reverse\""), "Should have reverse op");

    free(content);
    dv_dlist_destroy(p_dlist);
    cleanup_file(filename);

    return 0;
}

static int test_dlist_all_types(void)
{
    dv_dlist_t *p_dlist = NULL;
    const char *filename = "test_dlist_types.toml";

    p_dlist = dv_dlist_create("dlist", filename);

    dv_dlist_insert_head_int(p_dlist, 42, NULL);
    dv_dlist_insert_head_float(p_dlist, 3.14, NULL);
    dv_dlist_insert_head_string(p_dlist, "test", NULL);
    dv_dlist_insert_head_bool(p_dlist, false, NULL);

    dv_dlist_insert_tail_int(p_dlist, 100, NULL);
    dv_dlist_insert_tail_float(p_dlist, 2.71, NULL);
    dv_dlist_insert_tail_string(p_dlist, "hello", NULL);
    dv_dlist_insert_tail_bool(p_dlist, true, NULL);

    dv_dlist_commit(p_dlist);
    dv_dlist_destroy(p_dlist);
    cleanup_file(filename);

    return 0;
}

/* ========================================================================== */
/* 二叉树测试                                                                  */
/* ========================================================================== */

static int test_binary_tree_create_destroy(void)
{
    dv_binary_tree_t *p_tree = NULL;

    p_tree = dv_binary_tree_create(NULL, NULL);
    TEST_ASSERT(p_tree != NULL, "Should create tree");
    dv_binary_tree_destroy(p_tree);

    p_tree = dv_binary_tree_create("my_tree", "my_tree.toml");
    TEST_ASSERT(p_tree != NULL, "Should create tree with params");
    dv_binary_tree_destroy(p_tree);

    return 0;
}

static int test_binary_tree_insert_root(void)
{
    dv_binary_tree_t *p_tree = NULL;
    dv_error_t err;
    int32_t root_id;
    char *content = NULL;
    const char *filename = "test_tree_root.toml";

    p_tree = dv_binary_tree_create("tree", filename);

    err = dv_binary_tree_insert_root_int(p_tree, 10, &root_id);
    TEST_ASSERT(err == DV_OK, "Failed to insert root");
    TEST_ASSERT(root_id == 0, "Root id should be 0");

    dv_binary_tree_commit(p_tree);

    content = read_file_content(filename);
    TEST_ASSERT(content != NULL, "Failed to read output file");

    TEST_ASSERT(str_contains(content, "kind = \"binary_tree\""), "Should have kind = binary_tree");
    TEST_ASSERT(str_contains(content, "root = 0"), "Should have root = 0");
    TEST_ASSERT(str_contains(content, "op = \"insert_root\""), "Should have insert_root op");

    free(content);
    dv_binary_tree_destroy(p_tree);
    cleanup_file(filename);

    return 0;
}

static int test_binary_tree_insert_root_twice(void)
{
    dv_binary_tree_t *p_tree = NULL;
    dv_error_t err;
    char *content = NULL;
    const char *filename = "test_tree_roottwice.toml";

    p_tree = dv_binary_tree_create("tree", filename);

    dv_binary_tree_insert_root_int(p_tree, 10, NULL);

    err = dv_binary_tree_insert_root_int(p_tree, 20, NULL);
    TEST_ASSERT(err == DV_ERR_ROOT_EXISTS, "Should return ROOT_EXISTS");

    dv_binary_tree_commit(p_tree);

    content = read_file_content(filename);
    TEST_ASSERT(content != NULL, "Failed to read output file");

    TEST_ASSERT(str_contains(content, "[error]"), "Should contain error section");
    TEST_ASSERT(str_contains(content, "Root node already exists"), "Should mention root exists");

    free(content);
    dv_binary_tree_destroy(p_tree);
    cleanup_file(filename);

    return 0;
}

static int test_binary_tree_insert_children(void)
{
    dv_binary_tree_t *p_tree = NULL;
    dv_error_t err;
    int32_t root, left, right;
    char *content = NULL;
    const char *filename = "test_tree_children.toml";

    p_tree = dv_binary_tree_create("tree", filename);

    dv_binary_tree_insert_root_int(p_tree, 10, &root);

    err = dv_binary_tree_insert_left_int(p_tree, root, 5, &left);
    TEST_ASSERT(err == DV_OK, "Failed to insert left");

    err = dv_binary_tree_insert_right_int(p_tree, root, 15, &right);
    TEST_ASSERT(err == DV_OK, "Failed to insert right");

    dv_binary_tree_commit(p_tree);

    content = read_file_content(filename);
    TEST_ASSERT(content != NULL, "Failed to read output file");

    TEST_ASSERT(str_contains(content, "op = \"insert_left\""), "Should have insert_left op");
    TEST_ASSERT(str_contains(content, "op = \"insert_right\""), "Should have insert_right op");
    TEST_ASSERT(str_contains(content, "left = "), "Nodes should have left field");
    TEST_ASSERT(str_contains(content, "right = "), "Nodes should have right field");

    free(content);
    dv_binary_tree_destroy(p_tree);
    cleanup_file(filename);

    return 0;
}

static int test_binary_tree_insert_child_exists(void)
{
    dv_binary_tree_t *p_tree = NULL;
    dv_error_t err;
    int32_t root;
    const char *filename = "test_tree_childexists.toml";

    p_tree = dv_binary_tree_create("tree", filename);

    dv_binary_tree_insert_root_int(p_tree, 10, &root);
    dv_binary_tree_insert_left_int(p_tree, root, 5, NULL);

    err = dv_binary_tree_insert_left_int(p_tree, root, 3, NULL);
    TEST_ASSERT(err == DV_ERR_CHILD_EXISTS, "Should return CHILD_EXISTS");

    dv_binary_tree_commit(p_tree);
    dv_binary_tree_destroy(p_tree);
    cleanup_file(filename);

    return 0;
}

static int test_binary_tree_insert_parent_not_found(void)
{
    dv_binary_tree_t *p_tree = NULL;
    dv_error_t err;
    char *content = NULL;
    const char *filename = "test_tree_parentnf.toml";

    p_tree = dv_binary_tree_create("tree", filename);

    dv_binary_tree_insert_root_int(p_tree, 10, NULL);

    err = dv_binary_tree_insert_left_int(p_tree, 999, 5, NULL);
    TEST_ASSERT(err == DV_ERR_NOT_FOUND, "Should return NOT_FOUND");

    dv_binary_tree_commit(p_tree);

    content = read_file_content(filename);
    TEST_ASSERT(content != NULL, "Failed to read output file");

    TEST_ASSERT(str_contains(content, "[error]"), "Should contain error section");
    TEST_ASSERT(str_contains(content, "Parent node not found"), "Should mention parent not found");

    free(content);
    dv_binary_tree_destroy(p_tree);
    cleanup_file(filename);

    return 0;
}

static int test_binary_tree_delete(void)
{
    dv_binary_tree_t *p_tree = NULL;
    dv_error_t err;
    int32_t root, left;
    char *content = NULL;
    const char *filename = "test_tree_delete.toml";

    p_tree = dv_binary_tree_create("tree", filename);

    dv_binary_tree_insert_root_int(p_tree, 10, &root);
    dv_binary_tree_insert_left_int(p_tree, root, 5, &left);
    dv_binary_tree_insert_right_int(p_tree, root, 15, NULL);
    dv_binary_tree_insert_left_int(p_tree, left, 3, NULL);

    err = dv_binary_tree_delete(p_tree, left);
    TEST_ASSERT(err == DV_OK, "Failed to delete subtree");

    dv_binary_tree_commit(p_tree);

    content = read_file_content(filename);
    TEST_ASSERT(content != NULL, "Failed to read output file");

    TEST_ASSERT(str_contains(content, "op = \"delete\""), "Should have delete op");

    free(content);
    dv_binary_tree_destroy(p_tree);
    cleanup_file(filename);

    return 0;
}

static int test_binary_tree_delete_not_found(void)
{
    dv_binary_tree_t *p_tree = NULL;
    dv_error_t err;
    const char *filename = "test_tree_delnf.toml";

    p_tree = dv_binary_tree_create("tree", filename);

    dv_binary_tree_insert_root_int(p_tree, 10, NULL);

    err = dv_binary_tree_delete(p_tree, 999);
    TEST_ASSERT(err == DV_ERR_NOT_FOUND, "Should return NOT_FOUND");

    dv_binary_tree_commit(p_tree);
    dv_binary_tree_destroy(p_tree);
    cleanup_file(filename);

    return 0;
}

static int test_binary_tree_update_value(void)
{
    dv_binary_tree_t *p_tree = NULL;
    dv_error_t err;
    int32_t root;
    char *content = NULL;
    const char *filename = "test_tree_update.toml";

    p_tree = dv_binary_tree_create("tree", filename);

    dv_binary_tree_insert_root_int(p_tree, 10, &root);

    err = dv_binary_tree_update_value_int(p_tree, root, 100);
    TEST_ASSERT(err == DV_OK, "Failed to update value");

    dv_binary_tree_commit(p_tree);

    content = read_file_content(filename);
    TEST_ASSERT(content != NULL, "Failed to read output file");

    TEST_ASSERT(str_contains(content, "op = \"update_value\""), "Should have update_value op");

    free(content);
    dv_binary_tree_destroy(p_tree);
    cleanup_file(filename);

    return 0;
}

static int test_binary_tree_update_not_found(void)
{
    dv_binary_tree_t *p_tree = NULL;
    dv_error_t err;
    const char *filename = "test_tree_updnf.toml";

    p_tree = dv_binary_tree_create("tree", filename);

    dv_binary_tree_insert_root_int(p_tree, 10, NULL);

    err = dv_binary_tree_update_value_int(p_tree, 999, 100);
    TEST_ASSERT(err == DV_ERR_NOT_FOUND, "Should return NOT_FOUND");

    dv_binary_tree_commit(p_tree);
    dv_binary_tree_destroy(p_tree);
    cleanup_file(filename);

    return 0;
}

static int test_binary_tree_all_types(void)
{
    dv_binary_tree_t *p_tree = NULL;
    int32_t root;
    const char *filename = "test_tree_types.toml";

    p_tree = dv_binary_tree_create("tree", filename);

    dv_binary_tree_insert_root_int(p_tree, 10, &root);
    dv_binary_tree_insert_left_float(p_tree, root, 3.14, NULL);
    dv_binary_tree_insert_right_string(p_tree, root, "hello", NULL);

    dv_binary_tree_commit(p_tree);
    dv_binary_tree_destroy(p_tree);
    cleanup_file(filename);

    return 0;
}

static int test_binary_tree_empty_initial_state(void)
{
    dv_binary_tree_t *p_tree = NULL;
    char *content = NULL;
    const char *filename = "test_tree_empty.toml";

    p_tree = dv_binary_tree_create("tree", filename);
    dv_binary_tree_commit(p_tree);

    content = read_file_content(filename);
    TEST_ASSERT(content != NULL, "Failed to read output file");

    /* 验证空树的初始状态 */
    TEST_ASSERT(str_contains(content, "root = -1"), "Empty tree should have root = -1");
    TEST_ASSERT(str_contains(content, "nodes = []"), "Empty tree should have empty nodes");

    free(content);
    dv_binary_tree_destroy(p_tree);
    cleanup_file(filename);

    return 0;
}

/* ========================================================================== */
/* 无向图测试                                                                  */
/* ========================================================================== */

static int test_graph_undirected_create_destroy(void)
{
    dv_graph_undirected_t *p_graph = NULL;

    p_graph = dv_graph_undirected_create(NULL, NULL);
    TEST_ASSERT(p_graph != NULL, "Should create graph");
    dv_graph_undirected_destroy(p_graph);

    p_graph = dv_graph_undirected_create("my_graph", "my_graph.toml");
    TEST_ASSERT(p_graph != NULL, "Should create graph with params");
    dv_graph_undirected_destroy(p_graph);

    return 0;
}

static int test_graph_undirected_add_nodes(void)
{
    dv_graph_undirected_t *p_graph = NULL;
    dv_error_t err;
    char *content = NULL;
    const char *filename = "test_graph_nodes.toml";

    p_graph = dv_graph_undirected_create("graph", filename);

    err = dv_graph_undirected_add_node(p_graph, 0, "A");
    TEST_ASSERT(err == DV_OK, "Failed to add node A");

    err = dv_graph_undirected_add_node(p_graph, 1, "B");
    TEST_ASSERT(err == DV_OK, "Failed to add node B");

    err = dv_graph_undirected_add_node(p_graph, 2, "C");
    TEST_ASSERT(err == DV_OK, "Failed to add node C");

    dv_graph_undirected_commit(p_graph);

    content = read_file_content(filename);
    TEST_ASSERT(content != NULL, "Failed to read output file");

    TEST_ASSERT(str_contains(content, "kind = \"graph_undirected\""), "Should have kind = graph_undirected");
    TEST_ASSERT(str_contains(content, "op = \"add_node\""), "Should have add_node op");
    TEST_ASSERT(str_contains(content, "label = \"A\""), "Should have node label");

    free(content);
    dv_graph_undirected_destroy(p_graph);
    cleanup_file(filename);

    return 0;
}

static int test_graph_undirected_add_node_duplicate(void)
{
    dv_graph_undirected_t *p_graph = NULL;
    dv_error_t err;
    char *content = NULL;
    const char *filename = "test_graph_dup.toml";

    p_graph = dv_graph_undirected_create("graph", filename);

    dv_graph_undirected_add_node(p_graph, 0, "A");

    err = dv_graph_undirected_add_node(p_graph, 0, "A2");
    TEST_ASSERT(err == DV_ERR_ALREADY_EXISTS, "Should return ALREADY_EXISTS");

    dv_graph_undirected_commit(p_graph);

    content = read_file_content(filename);
    TEST_ASSERT(content != NULL, "Failed to read output file");

    TEST_ASSERT(str_contains(content, "[error]"), "Should contain error section");

    free(content);
    dv_graph_undirected_destroy(p_graph);
    cleanup_file(filename);

    return 0;
}

static int test_graph_undirected_add_edges(void)
{
    dv_graph_undirected_t *p_graph = NULL;
    dv_error_t err;
    char *content = NULL;
    const char *filename = "test_graph_edges.toml";

    p_graph = dv_graph_undirected_create("graph", filename);

    dv_graph_undirected_add_node(p_graph, 0, "A");
    dv_graph_undirected_add_node(p_graph, 1, "B");
    dv_graph_undirected_add_node(p_graph, 2, "C");

    err = dv_graph_undirected_add_edge(p_graph, 0, 1);
    TEST_ASSERT(err == DV_OK, "Failed to add edge 0-1");

    err = dv_graph_undirected_add_edge(p_graph, 1, 2);
    TEST_ASSERT(err == DV_OK, "Failed to add edge 1-2");

    /* 测试边规范化：2,0 应该规范化为 0,2 */
    err = dv_graph_undirected_add_edge(p_graph, 2, 0);
    TEST_ASSERT(err == DV_OK, "Failed to add edge 2-0");

    dv_graph_undirected_commit(p_graph);

    content = read_file_content(filename);
    TEST_ASSERT(content != NULL, "Failed to read output file");

    TEST_ASSERT(str_contains(content, "op = \"add_edge\""), "Should have add_edge op");
    /* 验证边被规范化存储 */
    TEST_ASSERT(str_contains(content, "from = 0"), "Edge should have normalized from");

    free(content);
    dv_graph_undirected_destroy(p_graph);
    cleanup_file(filename);

    return 0;
}

static int test_graph_undirected_self_loop(void)
{
    dv_graph_undirected_t *p_graph = NULL;
    dv_error_t err;
    char *content = NULL;
    const char *filename = "test_graph_selfloop.toml";

    p_graph = dv_graph_undirected_create("graph", filename);

    dv_graph_undirected_add_node(p_graph, 0, "A");

    err = dv_graph_undirected_add_edge(p_graph, 0, 0);
    TEST_ASSERT(err == DV_ERR_SELF_LOOP, "Should return SELF_LOOP");

    dv_graph_undirected_commit(p_graph);

    content = read_file_content(filename);
    TEST_ASSERT(content != NULL, "Failed to read output file");

    TEST_ASSERT(str_contains(content, "[error]"), "Should contain error section");
    TEST_ASSERT(str_contains(content, "Self-loop"), "Should mention self-loop");

    free(content);
    dv_graph_undirected_destroy(p_graph);
    cleanup_file(filename);

    return 0;
}

static int test_graph_undirected_edge_node_not_found(void)
{
    dv_graph_undirected_t *p_graph = NULL;
    dv_error_t err;
    const char *filename = "test_graph_edgenf.toml";

    p_graph = dv_graph_undirected_create("graph", filename);

    dv_graph_undirected_add_node(p_graph, 0, "A");

    err = dv_graph_undirected_add_edge(p_graph, 0, 999);
    TEST_ASSERT(err == DV_ERR_NOT_FOUND, "Should return NOT_FOUND");

    dv_graph_undirected_commit(p_graph);
    dv_graph_undirected_destroy(p_graph);
    cleanup_file(filename);

    return 0;
}

static int test_graph_undirected_edge_duplicate(void)
{
    dv_graph_undirected_t *p_graph = NULL;
    dv_error_t err;
    const char *filename = "test_graph_edgedup.toml";

    p_graph = dv_graph_undirected_create("graph", filename);

    dv_graph_undirected_add_node(p_graph, 0, "A");
    dv_graph_undirected_add_node(p_graph, 1, "B");

    dv_graph_undirected_add_edge(p_graph, 0, 1);

    err = dv_graph_undirected_add_edge(p_graph, 0, 1);
    TEST_ASSERT(err == DV_ERR_ALREADY_EXISTS, "Should return ALREADY_EXISTS for duplicate edge");

    /* 测试反向也被检测为重复 */
    err = dv_graph_undirected_add_edge(p_graph, 1, 0);
    TEST_ASSERT(err == DV_ERR_ALREADY_EXISTS, "Should return ALREADY_EXISTS for reverse edge");

    dv_graph_undirected_commit(p_graph);
    dv_graph_undirected_destroy(p_graph);
    cleanup_file(filename);

    return 0;
}

static int test_graph_undirected_remove_node(void)
{
    dv_graph_undirected_t *p_graph = NULL;
    dv_error_t err;
    char *content = NULL;
    const char *filename = "test_graph_rmnode.toml";

    p_graph = dv_graph_undirected_create("graph", filename);

    dv_graph_undirected_add_node(p_graph, 0, "A");
    dv_graph_undirected_add_node(p_graph, 1, "B");
    dv_graph_undirected_add_edge(p_graph, 0, 1);

    err = dv_graph_undirected_remove_node(p_graph, 0);
    TEST_ASSERT(err == DV_OK, "Failed to remove node");

    dv_graph_undirected_commit(p_graph);

    content = read_file_content(filename);
    TEST_ASSERT(content != NULL, "Failed to read output file");

    TEST_ASSERT(str_contains(content, "op = \"remove_node\""), "Should have remove_node op");

    free(content);
    dv_graph_undirected_destroy(p_graph);
    cleanup_file(filename);

    return 0;
}

static int test_graph_undirected_remove_edge(void)
{
    dv_graph_undirected_t *p_graph = NULL;
    dv_error_t err;
    char *content = NULL;
    const char *filename = "test_graph_rmedge.toml";

    p_graph = dv_graph_undirected_create("graph", filename);

    dv_graph_undirected_add_node(p_graph, 0, "A");
    dv_graph_undirected_add_node(p_graph, 1, "B");
    dv_graph_undirected_add_edge(p_graph, 0, 1);

    err = dv_graph_undirected_remove_edge(p_graph, 0, 1);
    TEST_ASSERT(err == DV_OK, "Failed to remove edge");

    dv_graph_undirected_commit(p_graph);

    content = read_file_content(filename);
    TEST_ASSERT(content != NULL, "Failed to read output file");

    TEST_ASSERT(str_contains(content, "op = \"remove_edge\""), "Should have remove_edge op");

    free(content);
    dv_graph_undirected_destroy(p_graph);
    cleanup_file(filename);

    return 0;
}

static int test_graph_undirected_update_label(void)
{
    dv_graph_undirected_t *p_graph = NULL;
    dv_error_t err;
    char *content = NULL;
    const char *filename = "test_graph_updlabel.toml";

    p_graph = dv_graph_undirected_create("graph", filename);

    dv_graph_undirected_add_node(p_graph, 0, "A");

    err = dv_graph_undirected_update_node_label(p_graph, 0, "A_new");
    TEST_ASSERT(err == DV_OK, "Failed to update label");

    dv_graph_undirected_commit(p_graph);

    content = read_file_content(filename);
    TEST_ASSERT(content != NULL, "Failed to read output file");

    TEST_ASSERT(str_contains(content, "op = \"update_node_label\""), "Should have update_node_label op");
    TEST_ASSERT(str_contains(content, "A_new"), "Should contain new label");

    free(content);
    dv_graph_undirected_destroy(p_graph);
    cleanup_file(filename);

    return 0;
}

static int test_graph_undirected_label_length(void)
{
    dv_graph_undirected_t *p_graph = NULL;
    dv_error_t err;
    const char *filename = "test_graph_labellen.toml";

    p_graph = dv_graph_undirected_create("graph", filename);

    /* 空标签应失败 */
    err = dv_graph_undirected_add_node(p_graph, 0, "");
    TEST_ASSERT(err == DV_ERR_INVALID_PARAM, "Empty label should fail");

    /* 超长标签应失败（超过32字符） */
    err = dv_graph_undirected_add_node(p_graph, 1, "This_is_a_very_long_label_that_exceeds_32_characters");
    TEST_ASSERT(err == DV_ERR_INVALID_PARAM, "Long label should fail");

    /* 正常长度标签应成功 */
    err = dv_graph_undirected_add_node(p_graph, 2, "ValidLabel");
    TEST_ASSERT(err == DV_OK, "Valid label should succeed");

    dv_graph_undirected_commit(p_graph);
    dv_graph_undirected_destroy(p_graph);
    cleanup_file(filename);

    return 0;
}

/* ========================================================================== */
/* 空指针测试                                                                  */
/* ========================================================================== */

static int test_null_stack_operations(void)
{
    dv_error_t err;

    err = dv_stack_push_int(NULL, 10);
    TEST_ASSERT(err == DV_ERR_NULL_PTR, "Should return NULL_PTR");

    err = dv_stack_push_float(NULL, 3.14);
    TEST_ASSERT(err == DV_ERR_NULL_PTR, "Should return NULL_PTR");

    err = dv_stack_push_string(NULL, "test");
    TEST_ASSERT(err == DV_ERR_NULL_PTR, "Should return NULL_PTR");

    err = dv_stack_push_bool(NULL, true);
    TEST_ASSERT(err == DV_ERR_NULL_PTR, "Should return NULL_PTR");

    err = dv_stack_pop(NULL);
    TEST_ASSERT(err == DV_ERR_NULL_PTR, "Should return NULL_PTR");

    err = dv_stack_clear(NULL);
    TEST_ASSERT(err == DV_ERR_NULL_PTR, "Should return NULL_PTR");

    err = dv_stack_commit(NULL);
    TEST_ASSERT(err == DV_ERR_NULL_PTR, "Should return NULL_PTR");

    dv_stack_destroy(NULL); /* 不应崩溃 */

    return 0;
}

static int test_null_queue_operations(void)
{
    dv_error_t err;

    err = dv_queue_enqueue_int(NULL, 10);
    TEST_ASSERT(err == DV_ERR_NULL_PTR, "Should return NULL_PTR");

    err = dv_queue_dequeue(NULL);
    TEST_ASSERT(err == DV_ERR_NULL_PTR, "Should return NULL_PTR");

    err = dv_queue_clear(NULL);
    TEST_ASSERT(err == DV_ERR_NULL_PTR, "Should return NULL_PTR");

    err = dv_queue_commit(NULL);
    TEST_ASSERT(err == DV_ERR_NULL_PTR, "Should return NULL_PTR");

    dv_queue_destroy(NULL);

    return 0;
}

static int test_null_slist_operations(void)
{
    dv_error_t err;

    err = dv_slist_insert_head_int(NULL, 10, NULL);
    TEST_ASSERT(err == DV_ERR_NULL_PTR, "Should return NULL_PTR");

    err = dv_slist_insert_tail_int(NULL, 10, NULL);
    TEST_ASSERT(err == DV_ERR_NULL_PTR, "Should return NULL_PTR");

    err = dv_slist_delete(NULL, 0);
    TEST_ASSERT(err == DV_ERR_NULL_PTR, "Should return NULL_PTR");

    err = dv_slist_reverse(NULL);
    TEST_ASSERT(err == DV_ERR_NULL_PTR, "Should return NULL_PTR");

    err = dv_slist_commit(NULL);
    TEST_ASSERT(err == DV_ERR_NULL_PTR, "Should return NULL_PTR");

    dv_slist_destroy(NULL);

    return 0;
}

static int test_null_dlist_operations(void)
{
    dv_error_t err;

    err = dv_dlist_insert_head_int(NULL, 10, NULL);
    TEST_ASSERT(err == DV_ERR_NULL_PTR, "Should return NULL_PTR");

    err = dv_dlist_delete(NULL, 0);
    TEST_ASSERT(err == DV_ERR_NULL_PTR, "Should return NULL_PTR");

    err = dv_dlist_reverse(NULL);
    TEST_ASSERT(err == DV_ERR_NULL_PTR, "Should return NULL_PTR");

    err = dv_dlist_commit(NULL);
    TEST_ASSERT(err == DV_ERR_NULL_PTR, "Should return NULL_PTR");

    dv_dlist_destroy(NULL);

    return 0;
}

static int test_null_tree_operations(void)
{
    dv_error_t err;

    err = dv_binary_tree_insert_root_int(NULL, 10, NULL);
    TEST_ASSERT(err == DV_ERR_NULL_PTR, "Should return NULL_PTR");

    err = dv_binary_tree_insert_left_int(NULL, 0, 5, NULL);
    TEST_ASSERT(err == DV_ERR_NULL_PTR, "Should return NULL_PTR");

    err = dv_binary_tree_delete(NULL, 0);
    TEST_ASSERT(err == DV_ERR_NULL_PTR, "Should return NULL_PTR");

    err = dv_binary_tree_commit(NULL);
    TEST_ASSERT(err == DV_ERR_NULL_PTR, "Should return NULL_PTR");

    dv_binary_tree_destroy(NULL);

    return 0;
}

static int test_null_graph_operations(void)
{
    dv_error_t err;

    err = dv_graph_undirected_add_node(NULL, 0, "A");
    TEST_ASSERT(err == DV_ERR_NULL_PTR, "Should return NULL_PTR");

    err = dv_graph_undirected_add_edge(NULL, 0, 1);
    TEST_ASSERT(err == DV_ERR_NULL_PTR, "Should return NULL_PTR");

    err = dv_graph_undirected_commit(NULL);
    TEST_ASSERT(err == DV_ERR_NULL_PTR, "Should return NULL_PTR");

    dv_graph_undirected_destroy(NULL);

    return 0;
}

/* ========================================================================== */
/* TOML格式合规性测试                                                          */
/* ========================================================================== */

static int test_toml_timestamp_format(void)
{
    dv_stack_t *p_stack = NULL;
    char *content = NULL;
    const char *filename = "test_toml_timestamp.toml";

    p_stack = dv_stack_create("stack", filename);
    dv_stack_push_int(p_stack, 1);
    dv_stack_commit(p_stack);

    content = read_file_content(filename);
    TEST_ASSERT(content != NULL, "Failed to read output file");

    /* 验证时间戳格式：应该包含T和Z（RFC3339 UTC格式） */
    TEST_ASSERT(str_contains(content, "created_at = \""), "Should have created_at");
    TEST_ASSERT(str_contains(content, "T"), "Timestamp should contain T separator");
    TEST_ASSERT(str_contains(content, "Z\""), "Timestamp should end with Z");

    free(content);
    dv_stack_destroy(p_stack);
    cleanup_file(filename);

    return 0;
}

static int test_toml_no_result_and_error(void)
{
    dv_stack_t *p_stack = NULL;
    char *content = NULL;
    const char *filename = "test_toml_exclusive.toml";

    /* 成功案例：应该有result，没有error */
    p_stack = dv_stack_create("stack", filename);
    dv_stack_push_int(p_stack, 1);
    dv_stack_commit(p_stack);

    content = read_file_content(filename);
    TEST_ASSERT(content != NULL, "Failed to read output file");

    TEST_ASSERT(str_contains(content, "[result]"), "Should have [result]");
    TEST_ASSERT(!str_contains(content, "[error]"), "Should NOT have [error] in success case");

    free(content);
    dv_stack_destroy(p_stack);
    cleanup_file(filename);

    /* 失败案例：应该有error，没有result */
    p_stack = dv_stack_create("stack", filename);
    dv_stack_pop(p_stack); /* 空栈弹出会失败 */
    dv_stack_commit(p_stack);

    content = read_file_content(filename);
    TEST_ASSERT(content != NULL, "Failed to read output file");

    TEST_ASSERT(str_contains(content, "[error]"), "Should have [error]");
    TEST_ASSERT(!str_contains(content, "[result]"), "Should NOT have [result] in error case");

    free(content);
    dv_stack_destroy(p_stack);
    cleanup_file(filename);

    return 0;
}

static int test_toml_step_before_after(void)
{
    dv_stack_t *p_stack = NULL;
    char *content = NULL;
    const char *filename = "test_toml_steps.toml";

    p_stack = dv_stack_create("stack", filename);
    dv_stack_push_int(p_stack, 1);
    dv_stack_push_int(p_stack, 2);
    dv_stack_commit(p_stack);

    content = read_file_content(filename);
    TEST_ASSERT(content != NULL, "Failed to read output file");

    /* 验证步骤包含before和after */
    TEST_ASSERT(str_contains(content, "before = "), "Should have before field");
    TEST_ASSERT(str_contains(content, "after = "), "Should have after field");

    free(content);
    dv_stack_destroy(p_stack);
    cleanup_file(filename);

    return 0;
}

static int test_toml_special_characters_in_string(void)
{
    dv_stack_t *p_stack = NULL;
    dv_error_t err;
    const char *filename = "test_toml_special.toml";

    p_stack = dv_stack_create("stack", filename);

    /* 测试包含特殊字符的字符串 */
    err = dv_stack_push_string(p_stack, "hello world");
    TEST_ASSERT(err == DV_OK, "Should handle spaces");

    err = dv_stack_push_string(p_stack, "test123");
    TEST_ASSERT(err == DV_OK, "Should handle alphanumeric");

    dv_stack_commit(p_stack);
    dv_stack_destroy(p_stack);
    cleanup_file(filename);

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

    printf("=== ds4viz C Library Tests ===\n");
    printf("Version: %s\n\n", dv_version());

    /* 版本和配置测试 */
    printf("--- Version and Config Tests ---\n");
    RUN_TEST(test_version);
    RUN_TEST(test_version_macros);
    RUN_TEST(test_config_basic);
    RUN_TEST(test_config_with_stack_output);

    /* 栈测试 */
    printf("\n--- Stack Tests ---\n");
    RUN_TEST(test_stack_create_destroy);
    RUN_TEST(test_stack_push_all_types);
    RUN_TEST(test_stack_pop_sequence);
    RUN_TEST(test_stack_empty_pop);
    RUN_TEST(test_stack_clear);
    RUN_TEST(test_stack_toml_structure);
    RUN_TEST(test_stack_state_ids_sequential);
    RUN_TEST(test_stack_large_capacity);

    /* 队列测试 */
    printf("\n--- Queue Tests ---\n");
    RUN_TEST(test_queue_create_destroy);
    RUN_TEST(test_queue_enqueue_all_types);
    RUN_TEST(test_queue_dequeue_sequence);
    RUN_TEST(test_queue_empty_dequeue);
    RUN_TEST(test_queue_clear);
    RUN_TEST(test_queue_toml_front_rear);
    RUN_TEST(test_queue_empty_state);

    /* 单链表测试 */
    printf("\n--- Single Linked List Tests ---\n");
    RUN_TEST(test_slist_create_destroy);
    RUN_TEST(test_slist_insert_head);
    RUN_TEST(test_slist_insert_tail);
    RUN_TEST(test_slist_insert_after);
    RUN_TEST(test_slist_insert_after_not_found);
    RUN_TEST(test_slist_delete);
    RUN_TEST(test_slist_delete_head);
    RUN_TEST(test_slist_delete_head_empty);
    RUN_TEST(test_slist_reverse);
    RUN_TEST(test_slist_all_types);
    RUN_TEST(test_slist_toml_node_format);

    /* 双向链表测试 */
    printf("\n--- Double Linked List Tests ---\n");
    RUN_TEST(test_dlist_create_destroy);
    RUN_TEST(test_dlist_insert_head_tail);
    RUN_TEST(test_dlist_delete_operations);
    RUN_TEST(test_dlist_delete_empty);
    RUN_TEST(test_dlist_reverse);
    RUN_TEST(test_dlist_all_types);

    /* 二叉树测试 */
    printf("\n--- Binary Tree Tests ---\n");
    RUN_TEST(test_binary_tree_create_destroy);
    RUN_TEST(test_binary_tree_insert_root);
    RUN_TEST(test_binary_tree_insert_root_twice);
    RUN_TEST(test_binary_tree_insert_children);
    RUN_TEST(test_binary_tree_insert_child_exists);
    RUN_TEST(test_binary_tree_insert_parent_not_found);
    RUN_TEST(test_binary_tree_delete);
    RUN_TEST(test_binary_tree_delete_not_found);
    RUN_TEST(test_binary_tree_update_value);
    RUN_TEST(test_binary_tree_update_not_found);
    RUN_TEST(test_binary_tree_all_types);
    RUN_TEST(test_binary_tree_empty_initial_state);

    /* 无向图测试 */
    printf("\n--- Undirected Graph Tests ---\n");
    RUN_TEST(test_graph_undirected_create_destroy);
    RUN_TEST(test_graph_undirected_add_nodes);
    RUN_TEST(test_graph_undirected_add_node_duplicate);
    RUN_TEST(test_graph_undirected_add_edges);
    RUN_TEST(test_graph_undirected_self_loop);
    RUN_TEST(test_graph_undirected_edge_node_not_found);
    RUN_TEST(test_graph_undirected_edge_duplicate);
    RUN_TEST(test_graph_undirected_remove_node);
    RUN_TEST(test_graph_undirected_remove_edge);
    RUN_TEST(test_graph_undirected_update_label);
    RUN_TEST(test_graph_undirected_label_length);

    /* 空指针测试 */
    printf("\n--- Null Pointer Tests ---\n");
    RUN_TEST(test_null_stack_operations);
    RUN_TEST(test_null_queue_operations);
    RUN_TEST(test_null_slist_operations);
    RUN_TEST(test_null_dlist_operations);
    RUN_TEST(test_null_tree_operations);
    RUN_TEST(test_null_graph_operations);

    /* TOML格式合规性测试 */
    printf("\n--- TOML Format Compliance Tests ---\n");
    RUN_TEST(test_toml_timestamp_format);
    RUN_TEST(test_toml_no_result_and_error);
    RUN_TEST(test_toml_step_before_after);
    RUN_TEST(test_toml_special_characters_in_string);

    /* 汇总 */
    printf("\n=== Test Summary ===\n");
    printf("Total: %d, Passed: %d, Failed: %d\n", total, passed, failed);
    printf("Pass Rate: %.1f%%\n", total > 0 ? (100.0 * passed / total) : 0.0);

    if (failed > 0)
    {
        printf("\n*** SOME TESTS FAILED ***\n");
        return 1;
    }
    else
    {
        printf("\n*** ALL TESTS PASSED ***\n");
        return 0;
    }
}
