/**
 * @file test.c
 * @brief ds4viz C 库集成测试
 *
 * 覆盖所有数据结构的基本操作、错误处理、类型推导、
 * 边缘情况与复杂场景, 并验证 TOML IR 输出的结构正确性.
 * 测试覆盖范围与 Python ds4viz 的集成测试完全对齐.
 *
 * @author WaterRun
 * @date 2025-06-15
 * @version 0.1.0
 */

#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/* ================================================================
 * 测试框架
 * ================================================================ */

static int g_tests_run = 0;
static int g_tests_passed = 0;
static int g_tests_failed = 0;
static int g_asserts = 0;
static bool g_cur_failed = false;

/**
 * @brief 断言: 条件为真
 *
 * @def ASSERT_TRUE
 */
#define ASSERT_TRUE(cond)                                       \
    do                                                          \
    {                                                           \
        g_asserts++;                                            \
        if (!(cond))                                            \
        {                                                       \
            fprintf(stderr, "    ASSERT_TRUE FAIL %s:%d: %s\n", \
                    __FILE__, __LINE__, #cond);                 \
            g_cur_failed = true;                                \
        }                                                       \
    } while (0)

/**
 * @brief 断言: 字符串 hay 包含子串 needle
 *
 * @def ASSERT_CONTAINS
 */
#define ASSERT_CONTAINS(hay, needle)                                  \
    do                                                                \
    {                                                                 \
        g_asserts++;                                                  \
        if (strstr((hay), (needle)) == NULL)                          \
        {                                                             \
            fprintf(stderr, "    ASSERT_CONTAINS FAIL %s:%d: '%s'\n", \
                    __FILE__, __LINE__, (needle));                    \
            g_cur_failed = true;                                      \
        }                                                             \
    } while (0)

/**
 * @brief 断言: 字符串 hay 不包含子串 needle
 *
 * @def ASSERT_NOT_CONTAINS
 */
#define ASSERT_NOT_CONTAINS(hay, needle)                                  \
    do                                                                    \
    {                                                                     \
        g_asserts++;                                                      \
        if (strstr((hay), (needle)) != NULL)                              \
        {                                                                 \
            fprintf(stderr, "    ASSERT_NOT_CONTAINS FAIL %s:%d: '%s'\n", \
                    __FILE__, __LINE__, (needle));                        \
            g_cur_failed = true;                                          \
        }                                                                 \
    } while (0)

/**
 * @brief 断言: 两个整数相等
 *
 * @def ASSERT_INT_EQ
 */
#define ASSERT_INT_EQ(a, b)                                             \
    do                                                                  \
    {                                                                   \
        g_asserts++;                                                    \
        int _a = (a);                                                   \
        int _b = (b);                                                   \
        if (_a != _b)                                                   \
        {                                                               \
            fprintf(stderr, "    ASSERT_INT_EQ FAIL %s:%d: %d != %d\n", \
                    __FILE__, __LINE__, _a, _b);                        \
            g_cur_failed = true;                                        \
        }                                                               \
    } while (0)

/**
 * @brief 运行单个测试函数
 *
 * @def RUN_TEST
 */
#define RUN_TEST(fn)            \
    do                          \
    {                           \
        g_cur_failed = false;   \
        printf("  %-58s", #fn); \
        fn();                   \
        g_tests_run++;          \
        if (g_cur_failed)       \
        {                       \
            g_tests_failed++;   \
            printf("[FAIL]\n"); \
        }                       \
        else                    \
        {                       \
            g_tests_passed++;   \
            printf("[PASS]\n"); \
        }                       \
    } while (0)

/**
 * @brief 输出测试分节标题
 *
 * @def SECTION
 */
#define SECTION(name) printf("\n--- %s ---\n", (name))

/* ================================================================
 * 辅助函数
 * ================================================================ */

/**
 * @brief 读取文件全部内容到堆分配字符串
 *
 * @param path 文件路径
 * @return 成功返回堆分配字符串 (调用者需 free), 失败返回 NULL
 */
static char *read_file(const char *path)
{
    FILE *p_f = fopen(path, "rb");
    long len;
    char *p_buf;

    if (!p_f)
    {
        return NULL;
    }
    fseek(p_f, 0, SEEK_END);
    len = ftell(p_f);
    fseek(p_f, 0, SEEK_SET);
    p_buf = (char *)malloc((size_t)(len + 1));
    if (!p_buf)
    {
        fclose(p_f);
        return NULL;
    }
    fread(p_buf, 1, (size_t)len, p_f);
    p_buf[len] = '\0';
    fclose(p_f);
    return p_buf;
}

/**
 * @brief 统计子串在字符串中出现的次数
 *
 * @param hay 被搜索字符串
 * @param needle 子串
 * @return 出现次数
 */
static int count_str(const char *hay, const char *needle)
{
    int n = 0;
    size_t nlen = strlen(needle);
    const char *p = hay;

    while ((p = strstr(p, needle)) != NULL)
    {
        n++;
        p += nlen;
    }
    return n;
}

/**
 * @brief 重置全局配置
 */
static void reset_config(void)
{
    dvConfig((dvConfigOptions){0});
}

/**
 * @brief 设置仅输出路径的配置
 *
 * @param path 输出路径
 */
static void set_output(const char *path)
{
    dvConfig((dvConfigOptions){.output_path = path});
}

/**
 * @brief 清理测试输出文件
 *
 * @param path 文件路径
 */
static void cleanup(const char *path)
{
    remove(path);
}

/* ================================================================
 * TOML 验证函数
 * ================================================================ */

/**
 * @brief 验证 TOML IR 通用结构 (meta/package/object/states/result|error)
 *
 * @param c TOML 文件内容
 */
static void validate_toml(const char *c)
{
    bool has_result;
    bool has_error;

    ASSERT_CONTAINS(c, "[meta]");
    ASSERT_CONTAINS(c, "created_at = ");
    ASSERT_CONTAINS(c, "lang = \"c\"");
    ASSERT_CONTAINS(c, "lang_version = ");
    ASSERT_CONTAINS(c, "[package]");
    ASSERT_CONTAINS(c, "name = \"ds4viz\"");
    ASSERT_CONTAINS(c, "lang = \"c\"");
    ASSERT_CONTAINS(c, "version = ");
    ASSERT_CONTAINS(c, "[object]");
    ASSERT_CONTAINS(c, "kind = ");
    ASSERT_CONTAINS(c, "[[states]]");
    ASSERT_CONTAINS(c, "[states.data]");

    has_result = strstr(c, "[result]") != NULL;
    has_error = strstr(c, "[error]") != NULL;
    ASSERT_TRUE(has_result || has_error);
    ASSERT_TRUE(!(has_result && has_error));

    {
        int ns = count_str(c, "[[states]]");
        int nd = count_str(c, "[states.data]");
        ASSERT_INT_EQ(ns, nd);
    }
}

/**
 * @brief 验证成功结果段
 *
 * @param c TOML 文件内容
 */
static void validate_result(const char *c)
{
    ASSERT_CONTAINS(c, "[result]");
    ASSERT_CONTAINS(c, "final_state = ");
    ASSERT_NOT_CONTAINS(c, "[error]");
}

/**
 * @brief 验证错误段
 *
 * @param c TOML 文件内容
 */
static void validate_error(const char *c)
{
    ASSERT_CONTAINS(c, "[error]");
    ASSERT_CONTAINS(c, "type = \"runtime\"");
    ASSERT_CONTAINS(c, "message = ");
    ASSERT_NOT_CONTAINS(c, "[result]");
}

/**
 * @brief 验证栈数据字段
 *
 * @param c TOML 文件内容
 */
static void validate_stack_data(const char *c)
{
    ASSERT_CONTAINS(c, "items = ");
    ASSERT_CONTAINS(c, "top = ");
}

/**
 * @brief 验证队列数据字段
 *
 * @param c TOML 文件内容
 */
static void validate_queue_data(const char *c)
{
    ASSERT_CONTAINS(c, "items = ");
    ASSERT_CONTAINS(c, "front = ");
    ASSERT_CONTAINS(c, "rear = ");
}

/**
 * @brief 验证单链表数据字段
 *
 * @param c TOML 文件内容
 */
static void validate_slist_data(const char *c)
{
    ASSERT_CONTAINS(c, "head = ");
    ASSERT_CONTAINS(c, "nodes = ");
}

/**
 * @brief 验证双向链表数据字段
 *
 * @param c TOML 文件内容
 */
static void validate_dlist_data(const char *c)
{
    ASSERT_CONTAINS(c, "head = ");
    ASSERT_CONTAINS(c, "tail = ");
    ASSERT_CONTAINS(c, "nodes = ");
}

/**
 * @brief 验证二叉树数据字段
 *
 * @param c TOML 文件内容
 */
static void validate_bt_data(const char *c)
{
    ASSERT_CONTAINS(c, "root = ");
    ASSERT_CONTAINS(c, "nodes = ");
}

/**
 * @brief 验证图数据字段
 *
 * @param c TOML 文件内容
 */
static void validate_graph_data(const char *c)
{
    ASSERT_CONTAINS(c, "nodes = ");
    ASSERT_CONTAINS(c, "edges = ");
}

/**
 * @brief 验证步骤结构字段
 *
 * @param c TOML 文件内容
 */
static void validate_steps(const char *c)
{
    int ns = count_str(c, "[[steps]]");
    int na = count_str(c, "[steps.args]");
    int nc = count_str(c, "[steps.code]");

    if (ns > 0)
    {
        ASSERT_INT_EQ(ns, na);
        ASSERT_INT_EQ(ns, nc);
    }
}

/* ================================================================
 * 栈测试
 * ================================================================ */

/**
 * @brief 测试栈基本操作 (push/pop)
 */
static void test_stack_basic(void)
{
    const char *path = "t_stack_basic.toml";
    char *c;

    set_output(path);
    dvStack(s, "test_stack")
    {
        dvStackPush(s, 10);
        dvStackPush(s, 20);
        dvStackPush(s, 30);
        dvStackPop(s);
        dvStackPush(s, 40);
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_result(c);
    validate_steps(c);
    ASSERT_CONTAINS(c, "kind = \"stack\"");
    validate_stack_data(c);
    free(c);
    cleanup(path);
}

/**
 * @brief 测试空栈弹出 (应产生错误)
 */
static void test_stack_empty_pop_error(void)
{
    const char *path = "t_stack_epop.toml";
    char *c;

    set_output(path);
    dvStack(s)
    {
        dvStackPop(s);
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_error(c);
    free(c);
    cleanup(path);
}

/**
 * @brief 测试栈清空操作
 */
static void test_stack_clear(void)
{
    const char *path = "t_stack_clear.toml";
    char *c;

    set_output(path);
    dvStack(s)
    {
        dvStackPush(s, 1);
        dvStackPush(s, 2);
        dvStackPush(s, 3);
        dvStackClear(s);
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_result(c);
    ASSERT_CONTAINS(c, "items = []");
    free(c);
    cleanup(path);
}

/**
 * @brief 测试栈支持多种值类型 (int/double/string/bool)
 */
static void test_stack_various_types(void)
{
    const char *path = "t_stack_types.toml";
    char *c;

    set_output(path);
    dvStack(s)
    {
        dvStackPush(s, 42);
        dvStackPush(s, 3.14);
        dvStackPush(s, "hello");
        dvStackPush(s, true);
        dvStackPush(s, false);
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_result(c);
    ASSERT_CONTAINS(c, "42");
    ASSERT_CONTAINS(c, "\"hello\"");
    ASSERT_CONTAINS(c, "true");
    ASSERT_CONTAINS(c, "false");
    free(c);
    cleanup(path);
}

/**
 * @brief 测试栈处理负数
 */
static void test_stack_negative(void)
{
    const char *path = "t_stack_neg.toml";
    char *c;

    set_output(path);
    dvStack(s)
    {
        dvStackPush(s, -10);
        dvStackPush(s, -20);
        dvStackPop(s);
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_result(c);
    free(c);
    cleanup(path);
}

/**
 * @brief 测试栈混合操作 (push/pop/clear/push)
 */
static void test_stack_mixed(void)
{
    const char *path = "t_stack_mixed.toml";
    char *c;

    set_output(path);
    dvStack(s)
    {
        dvStackPush(s, 1);
        dvStackPush(s, 2);
        dvStackPop(s);
        dvStackPush(s, 3);
        dvStackClear(s);
        dvStackPush(s, 4);
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_result(c);
    free(c);
    cleanup(path);
}

/**
 * @brief 测试单元素栈
 */
static void test_stack_single(void)
{
    const char *path = "t_stack_single.toml";
    char *c;

    set_output(path);
    dvStack(s)
    {
        dvStackPush(s, 100);
        dvStackPop(s);
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_result(c);
    free(c);
    cleanup(path);
}

/**
 * @brief 测试清空后再压入
 */
static void test_stack_push_after_clear(void)
{
    const char *path = "t_stack_pac.toml";
    char *c;

    set_output(path);
    dvStack(s)
    {
        dvStackPush(s, 1);
        dvStackPush(s, 2);
        dvStackClear(s);
        dvStackPush(s, 10);
        dvStackPush(s, 20);
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_result(c);
    free(c);
    cleanup(path);
}

/**
 * @brief 测试连续弹出
 */
static void test_stack_multi_pop(void)
{
    const char *path = "t_stack_mpop.toml";
    char *c;

    set_output(path);
    dvStack(s)
    {
        for (int i = 0; i < 5; i++)
        {
            dvStackPush(s, i);
        }
        for (int i = 0; i < 3; i++)
        {
            dvStackPop(s);
        }
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_result(c);
    free(c);
    cleanup(path);
}

/**
 * @brief 测试浮点数值
 */
static void test_stack_float(void)
{
    const char *path = "t_stack_float.toml";
    char *c;

    set_output(path);
    dvStack(s)
    {
        dvStackPush(s, 1.5);
        dvStackPush(s, 2.7);
        dvStackPush(s, -3.14);
        dvStackPop(s);
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_result(c);
    free(c);
    cleanup(path);
}

/**
 * @brief 测试字符串值
 */
static void test_stack_string(void)
{
    const char *path = "t_stack_str.toml";
    char *c;

    set_output(path);
    dvStack(s)
    {
        dvStackPush(s, "first");
        dvStackPush(s, "second");
        dvStackPush(s, "third");
        dvStackPop(s);
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_result(c);
    ASSERT_CONTAINS(c, "\"first\"");
    ASSERT_CONTAINS(c, "\"second\"");
    free(c);
    cleanup(path);
}

/**
 * @brief 测试布尔值
 */
static void test_stack_bool(void)
{
    const char *path = "t_stack_bool.toml";
    char *c;

    set_output(path);
    dvStack(s)
    {
        dvStackPush(s, true);
        dvStackPush(s, false);
        dvStackPush(s, true);
        dvStackPop(s);
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_result(c);
    free(c);
    cleanup(path);
}

/**
 * @brief 测试零值
 */
static void test_stack_zero(void)
{
    const char *path = "t_stack_zero.toml";
    char *c;

    set_output(path);
    dvStack(s)
    {
        dvStackPush(s, 0);
        dvStackPush(s, 0.0);
        dvStackPop(s);
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_result(c);
    free(c);
    cleanup(path);
}

/* ================================================================
 * 队列测试
 * ================================================================ */

/**
 * @brief 测试队列基本操作
 */
static void test_queue_basic(void)
{
    const char *path = "t_queue_basic.toml";
    char *c;

    set_output(path);
    dvQueue(q, "test_queue")
    {
        dvQueueEnqueue(q, 10);
        dvQueueEnqueue(q, 20);
        dvQueueDequeue(q);
        dvQueueEnqueue(q, 30);
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_result(c);
    ASSERT_CONTAINS(c, "kind = \"queue\"");
    validate_queue_data(c);
    free(c);
    cleanup(path);
}

/**
 * @brief 测试空队列出队错误
 */
static void test_queue_empty_dequeue_error(void)
{
    const char *path = "t_queue_edeq.toml";
    char *c;

    set_output(path);
    dvQueue(q)
    {
        dvQueueDequeue(q);
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_error(c);
    free(c);
    cleanup(path);
}

/**
 * @brief 测试队列清空
 */
static void test_queue_clear(void)
{
    const char *path = "t_queue_clear.toml";
    char *c;

    set_output(path);
    dvQueue(q)
    {
        dvQueueEnqueue(q, 1);
        dvQueueEnqueue(q, 2);
        dvQueueEnqueue(q, 3);
        dvQueueClear(q);
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_result(c);
    ASSERT_CONTAINS(c, "items = []");
    free(c);
    cleanup(path);
}

/**
 * @brief 测试队列 front/rear 一致性
 */
static void test_queue_consistency(void)
{
    const char *path = "t_queue_cons.toml";
    char *c;

    set_output(path);
    dvQueue(q)
    {
        dvQueueEnqueue(q, 1);
        dvQueueEnqueue(q, 2);
        dvQueueEnqueue(q, 3);
        dvQueueDequeue(q);
        dvQueueDequeue(q);
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_result(c);
    validate_queue_data(c);
    free(c);
    cleanup(path);
}

/**
 * @brief 测试单元素队列
 */
static void test_queue_single(void)
{
    const char *path = "t_queue_single.toml";
    char *c;

    set_output(path);
    dvQueue(q)
    {
        dvQueueEnqueue(q, 100);
        dvQueueDequeue(q);
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_result(c);
    free(c);
    cleanup(path);
}

/**
 * @brief 测试清空后再入队
 */
static void test_queue_enqueue_after_clear(void)
{
    const char *path = "t_queue_eac.toml";
    char *c;

    set_output(path);
    dvQueue(q)
    {
        dvQueueEnqueue(q, 1);
        dvQueueEnqueue(q, 2);
        dvQueueClear(q);
        dvQueueEnqueue(q, 10);
        dvQueueEnqueue(q, 20);
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_result(c);
    free(c);
    cleanup(path);
}

/**
 * @brief 测试连续出队
 */
static void test_queue_multi_dequeue(void)
{
    const char *path = "t_queue_mdeq.toml";
    char *c;

    set_output(path);
    dvQueue(q)
    {
        for (int i = 0; i < 5; i++)
        {
            dvQueueEnqueue(q, i);
        }
        for (int i = 0; i < 3; i++)
        {
            dvQueueDequeue(q);
        }
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_result(c);
    free(c);
    cleanup(path);
}

/**
 * @brief 测试交替入队出队
 */
static void test_queue_alternating(void)
{
    const char *path = "t_queue_alt.toml";
    char *c;

    set_output(path);
    dvQueue(q)
    {
        dvQueueEnqueue(q, 1);
        dvQueueEnqueue(q, 2);
        dvQueueDequeue(q);
        dvQueueEnqueue(q, 3);
        dvQueueDequeue(q);
        dvQueueEnqueue(q, 4);
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_result(c);
    free(c);
    cleanup(path);
}

/**
 * @brief 测试出队直到空
 */
static void test_queue_until_empty(void)
{
    const char *path = "t_queue_ue.toml";
    char *c;

    set_output(path);
    dvQueue(q)
    {
        for (int i = 0; i < 3; i++)
        {
            dvQueueEnqueue(q, i);
        }
        for (int i = 0; i < 3; i++)
        {
            dvQueueDequeue(q);
        }
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_result(c);
    ASSERT_CONTAINS(c, "items = []");
    free(c);
    cleanup(path);
}

/**
 * @brief 测试队列字符串值
 */
static void test_queue_string(void)
{
    const char *path = "t_queue_str.toml";
    char *c;

    set_output(path);
    dvQueue(q)
    {
        dvQueueEnqueue(q, "first");
        dvQueueEnqueue(q, "second");
        dvQueueDequeue(q);
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_result(c);
    free(c);
    cleanup(path);
}

/* ================================================================
 * 单链表测试
 * ================================================================ */

/**
 * @brief 测试单链表基本操作
 */
static void test_slist_basic(void)
{
    const char *path = "t_slist_basic.toml";
    char *c;
    int n1;

    set_output(path);
    dvSingleLinkedList(l, "test_slist")
    {
        n1 = dvSlInsertHead(l, 10);
        dvSlInsertTail(l, 20);
        dvSlInsertAfter(l, n1, 15);
        dvSlDeleteHead(l);
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_result(c);
    ASSERT_CONTAINS(c, "kind = \"slist\"");
    validate_slist_data(c);
    free(c);
    cleanup(path);
}

/**
 * @brief 测试单链表反转
 */
static void test_slist_reverse(void)
{
    const char *path = "t_slist_rev.toml";
    char *c;

    set_output(path);
    dvSingleLinkedList(l)
    {
        dvSlInsertTail(l, 1);
        dvSlInsertTail(l, 2);
        dvSlInsertTail(l, 3);
        dvSlReverse(l);
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_result(c);
    free(c);
    cleanup(path);
}

/**
 * @brief 测试删除不存在的节点 (错误)
 */
static void test_slist_delete_nonexistent(void)
{
    const char *path = "t_slist_dne.toml";
    char *c;

    set_output(path);
    dvSingleLinkedList(l)
    {
        dvSlInsertHead(l, 10);
        dvSlDelete(l, 999);
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_error(c);
    free(c);
    cleanup(path);
}

/**
 * @brief 测试空链表删除头节点 (错误)
 */
static void test_slist_empty_delete_head(void)
{
    const char *path = "t_slist_edh.toml";
    char *c;

    set_output(path);
    dvSingleLinkedList(l)
    {
        dvSlDeleteHead(l);
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_error(c);
    free(c);
    cleanup(path);
}

/**
 * @brief 测试在不存在的节点后插入 (错误)
 */
static void test_slist_insert_after_nonexist(void)
{
    const char *path = "t_slist_iane.toml";
    char *c;

    set_output(path);
    dvSingleLinkedList(l)
    {
        dvSlInsertHead(l, 10);
        dvSlInsertAfter(l, 999, 20);
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_error(c);
    free(c);
    cleanup(path);
}

/**
 * @brief 测试删除尾节点
 */
static void test_slist_delete_tail(void)
{
    const char *path = "t_slist_dt.toml";
    char *c;
    int n3;

    set_output(path);
    dvSingleLinkedList(l)
    {
        dvSlInsertTail(l, 1);
        dvSlInsertTail(l, 2);
        n3 = dvSlInsertTail(l, 3);
        dvSlDelete(l, n3);
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_result(c);
    free(c);
    cleanup(path);
}

/**
 * @brief 测试单节点链表
 */
static void test_slist_single_node(void)
{
    const char *path = "t_slist_sn.toml";
    char *c;

    set_output(path);
    dvSingleLinkedList(l)
    {
        dvSlInsertHead(l, 100);
        dvSlDeleteHead(l);
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_result(c);
    free(c);
    cleanup(path);
}

/**
 * @brief 测试反转空链表
 */
static void test_slist_reverse_empty(void)
{
    const char *path = "t_slist_re.toml";
    char *c;

    set_output(path);
    dvSingleLinkedList(l)
    {
        dvSlReverse(l);
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_result(c);
    free(c);
    cleanup(path);
}

/**
 * @brief 测试多次插入
 */
static void test_slist_multi_insert(void)
{
    const char *path = "t_slist_mi.toml";
    char *c;

    set_output(path);
    dvSingleLinkedList(l)
    {
        for (int i = 0; i < 5; i++)
        {
            dvSlInsertTail(l, i);
        }
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_result(c);
    free(c);
    cleanup(path);
}

/* ================================================================
 * 双向链表测试
 * ================================================================ */

/**
 * @brief 测试双向链表基本操作
 */
static void test_dlist_basic(void)
{
    const char *path = "t_dlist_basic.toml";
    char *c;
    int n1;
    int n2;

    set_output(path);
    dvDoubleLinkedList(l, "test_dlist")
    {
        n1 = dvDlInsertHead(l, 10);
        n2 = dvDlInsertTail(l, 30);
        dvDlInsertBefore(l, n2, 20);
        dvDlInsertAfter(l, n1, 15);
        dvDlDeleteTail(l);
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_result(c);
    ASSERT_CONTAINS(c, "kind = \"dlist\"");
    validate_dlist_data(c);
    free(c);
    cleanup(path);
}

/**
 * @brief 测试双向链表反转
 */
static void test_dlist_reverse(void)
{
    const char *path = "t_dlist_rev.toml";
    char *c;

    set_output(path);
    dvDoubleLinkedList(l)
    {
        dvDlInsertTail(l, 1);
        dvDlInsertTail(l, 2);
        dvDlInsertTail(l, 3);
        dvDlReverse(l);
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_result(c);
    free(c);
    cleanup(path);
}

/**
 * @brief 测试双向链表 head/tail 一致性
 */
static void test_dlist_consistency(void)
{
    const char *path = "t_dlist_cons.toml";
    char *c;

    set_output(path);
    dvDoubleLinkedList(l)
    {
        dvDlInsertHead(l, 1);
        dvDlDeleteHead(l);
        dvDlInsertTail(l, 2);
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_result(c);
    validate_dlist_data(c);
    free(c);
    cleanup(path);
}

/**
 * @brief 测试空链表删除头节点 (错误)
 */
static void test_dlist_delete_head_error(void)
{
    const char *path = "t_dlist_dhe.toml";
    char *c;

    set_output(path);
    dvDoubleLinkedList(l)
    {
        dvDlDeleteHead(l);
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_error(c);
    free(c);
    cleanup(path);
}

/**
 * @brief 测试空链表删除尾节点 (错误)
 */
static void test_dlist_delete_tail_error(void)
{
    const char *path = "t_dlist_dte.toml";
    char *c;

    set_output(path);
    dvDoubleLinkedList(l)
    {
        dvDlDeleteTail(l);
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_error(c);
    free(c);
    cleanup(path);
}

/**
 * @brief 测试在不存在的节点前插入 (错误)
 */
static void test_dlist_insert_before_nonexist(void)
{
    const char *path = "t_dlist_ibne.toml";
    char *c;

    set_output(path);
    dvDoubleLinkedList(l)
    {
        dvDlInsertHead(l, 10);
        dvDlInsertBefore(l, 999, 20);
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_error(c);
    free(c);
    cleanup(path);
}

/**
 * @brief 测试在不存在的节点后插入 (错误)
 */
static void test_dlist_insert_after_nonexist(void)
{
    const char *path = "t_dlist_iane.toml";
    char *c;

    set_output(path);
    dvDoubleLinkedList(l)
    {
        dvDlInsertHead(l, 10);
        dvDlInsertAfter(l, 999, 20);
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_error(c);
    free(c);
    cleanup(path);
}

/**
 * @brief 测试删除不存在的节点 (错误)
 */
static void test_dlist_delete_nonexist(void)
{
    const char *path = "t_dlist_dne.toml";
    char *c;

    set_output(path);
    dvDoubleLinkedList(l)
    {
        dvDlInsertHead(l, 10);
        dvDlDelete(l, 999);
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_error(c);
    free(c);
    cleanup(path);
}

/**
 * @brief 测试双向链表清空
 */
static void test_dlist_clear(void)
{
    const char *path = "t_dlist_clear.toml";
    char *c;

    set_output(path);
    dvDoubleLinkedList(l)
    {
        dvDlInsertTail(l, 1);
        dvDlInsertTail(l, 2);
        dvDlDeleteHead(l);
        dvDlDeleteHead(l);
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_result(c);
    free(c);
    cleanup(path);
}

/**
 * @brief 测试单节点双向链表
 */
static void test_dlist_single_node(void)
{
    const char *path = "t_dlist_sn.toml";
    char *c;

    set_output(path);
    dvDoubleLinkedList(l)
    {
        dvDlInsertHead(l, 100);
        dvDlDeleteHead(l);
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_result(c);
    free(c);
    cleanup(path);
}

/**
 * @brief 测试反转空链表
 */
static void test_dlist_reverse_empty(void)
{
    const char *path = "t_dlist_re.toml";
    char *c;

    set_output(path);
    dvDoubleLinkedList(l)
    {
        dvDlReverse(l);
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_result(c);
    free(c);
    cleanup(path);
}

/**
 * @brief 测试多种操作组合
 */
static void test_dlist_multi_ops(void)
{
    const char *path = "t_dlist_mops.toml";
    char *c;
    int n1;
    int n2;

    set_output(path);
    dvDoubleLinkedList(l)
    {
        n1 = dvDlInsertHead(l, 1);
        n2 = dvDlInsertTail(l, 5);
        dvDlInsertAfter(l, n1, 2);
        dvDlInsertBefore(l, n2, 4);
        dvDlDelete(l, n1);
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_result(c);
    free(c);
    cleanup(path);
}

/* ================================================================
 * 二叉树测试
 * ================================================================ */

/**
 * @brief 测试二叉树基本操作
 */
static void test_bt_basic(void)
{
    const char *path = "t_bt_basic.toml";
    char *c;
    int root;
    int left;

    set_output(path);
    dvBinaryTree(t, "test_btree")
    {
        root = dvBtInsertRoot(t, 10);
        left = dvBtInsertLeft(t, root, 5);
        dvBtInsertRight(t, root, 15);
        dvBtInsertLeft(t, left, 3);
        dvBtInsertRight(t, left, 7);
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_result(c);
    ASSERT_CONTAINS(c, "kind = \"binary_tree\"");
    validate_bt_data(c);
    free(c);
    cleanup(path);
}

/**
 * @brief 测试删除子树
 */
static void test_bt_delete_subtree(void)
{
    const char *path = "t_bt_delsub.toml";
    char *c;
    int root;
    int left;

    set_output(path);
    dvBinaryTree(t)
    {
        root = dvBtInsertRoot(t, 10);
        left = dvBtInsertLeft(t, root, 5);
        dvBtInsertLeft(t, left, 3);
        dvBtInsertRight(t, left, 7);
        dvBtDelete(t, left);
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_result(c);
    free(c);
    cleanup(path);
}

/**
 * @brief 测试更新节点值
 */
static void test_bt_update(void)
{
    const char *path = "t_bt_update.toml";
    char *c;
    int root;

    set_output(path);
    dvBinaryTree(t)
    {
        root = dvBtInsertRoot(t, 10);
        dvBtUpdateValue(t, root, 100);
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_result(c);
    free(c);
    cleanup(path);
}

/**
 * @brief 测试重复插入根节点 (错误)
 */
static void test_bt_dup_root_error(void)
{
    const char *path = "t_bt_duproot.toml";
    char *c;

    set_output(path);
    dvBinaryTree(t)
    {
        dvBtInsertRoot(t, 10);
        dvBtInsertRoot(t, 20);
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_error(c);
    free(c);
    cleanup(path);
}

/**
 * @brief 测试重复插入左子节点 (错误)
 */
static void test_bt_dup_child_error(void)
{
    const char *path = "t_bt_dupchild.toml";
    char *c;
    int root;

    set_output(path);
    dvBinaryTree(t)
    {
        root = dvBtInsertRoot(t, 10);
        dvBtInsertLeft(t, root, 5);
        dvBtInsertLeft(t, root, 6);
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_error(c);
    free(c);
    cleanup(path);
}

/**
 * @brief 测试不存在的父节点 (错误)
 */
static void test_bt_no_parent_error(void)
{
    const char *path = "t_bt_nopar.toml";
    char *c;

    set_output(path);
    dvBinaryTree(t)
    {
        dvBtInsertRoot(t, 10);
        dvBtInsertLeft(t, 999, 5);
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_error(c);
    free(c);
    cleanup(path);
}

/**
 * @brief 测试删除根节点
 */
static void test_bt_delete_root(void)
{
    const char *path = "t_bt_delroot.toml";
    char *c;
    int root;

    set_output(path);
    dvBinaryTree(t)
    {
        root = dvBtInsertRoot(t, 10);
        dvBtInsertLeft(t, root, 5);
        dvBtDelete(t, root);
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_result(c);
    free(c);
    cleanup(path);
}

/**
 * @brief 测试删除不存在的节点 (错误)
 */
static void test_bt_delete_nonexist_error(void)
{
    const char *path = "t_bt_delne.toml";
    char *c;

    set_output(path);
    dvBinaryTree(t)
    {
        dvBtInsertRoot(t, 10);
        dvBtDelete(t, 999);
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_error(c);
    free(c);
    cleanup(path);
}

/**
 * @brief 测试更新不存在的节点 (错误)
 */
static void test_bt_update_nonexist_error(void)
{
    const char *path = "t_bt_updne.toml";
    char *c;

    set_output(path);
    dvBinaryTree(t)
    {
        dvBtInsertRoot(t, 10);
        dvBtUpdateValue(t, 999, 100);
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_error(c);
    free(c);
    cleanup(path);
}

/**
 * @brief 测试完全二叉树
 */
static void test_bt_complete(void)
{
    const char *path = "t_bt_complete.toml";
    char *c;
    int root;
    int left;
    int right;

    set_output(path);
    dvBinaryTree(t)
    {
        root = dvBtInsertRoot(t, 1);
        left = dvBtInsertLeft(t, root, 2);
        right = dvBtInsertRight(t, root, 3);
        dvBtInsertLeft(t, left, 4);
        dvBtInsertRight(t, left, 5);
        dvBtInsertLeft(t, right, 6);
        dvBtInsertRight(t, right, 7);
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_result(c);
    ASSERT_INT_EQ(count_str(c, "[[states]]"), 8);
    ASSERT_INT_EQ(count_str(c, "[[steps]]"), 7);
    free(c);
    cleanup(path);
}

/* ================================================================
 * 二叉搜索树测试
 * ================================================================ */

/**
 * @brief 测试 BST 基本操作
 */
static void test_bst_basic(void)
{
    const char *path = "t_bst_basic.toml";
    char *c;

    set_output(path);
    dvBinarySearchTree(b, "test_bst")
    {
        dvBstInsert(b, 10);
        dvBstInsert(b, 5);
        dvBstInsert(b, 15);
        dvBstInsert(b, 3);
        dvBstInsert(b, 7);
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_result(c);
    ASSERT_CONTAINS(c, "kind = \"bst\"");
    validate_bt_data(c);
    free(c);
    cleanup(path);
}

/**
 * @brief 测试 BST 删除叶节点
 */
static void test_bst_delete_leaf(void)
{
    const char *path = "t_bst_dl.toml";
    char *c;

    set_output(path);
    dvBinarySearchTree(b)
    {
        dvBstInsert(b, 10);
        dvBstInsert(b, 5);
        dvBstInsert(b, 15);
        dvBstDelete(b, 5);
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_result(c);
    free(c);
    cleanup(path);
}

/**
 * @brief 测试 BST 删除单子节点的节点
 */
static void test_bst_delete_one_child(void)
{
    const char *path = "t_bst_doc.toml";
    char *c;

    set_output(path);
    dvBinarySearchTree(b)
    {
        dvBstInsert(b, 10);
        dvBstInsert(b, 5);
        dvBstInsert(b, 3);
        dvBstDelete(b, 5);
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_result(c);
    free(c);
    cleanup(path);
}

/**
 * @brief 测试 BST 删除双子节点的节点
 */
static void test_bst_delete_two_children(void)
{
    const char *path = "t_bst_dtc.toml";
    char *c;

    set_output(path);
    dvBinarySearchTree(b)
    {
        dvBstInsert(b, 10);
        dvBstInsert(b, 5);
        dvBstInsert(b, 15);
        dvBstInsert(b, 3);
        dvBstInsert(b, 7);
        dvBstDelete(b, 5);
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_result(c);
    free(c);
    cleanup(path);
}

/**
 * @brief 测试 BST 删除不存在的值 (错误)
 */
static void test_bst_delete_nonexist_error(void)
{
    const char *path = "t_bst_dne.toml";
    char *c;

    set_output(path);
    dvBinarySearchTree(b)
    {
        dvBstInsert(b, 10);
        dvBstDelete(b, 999);
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_error(c);
    free(c);
    cleanup(path);
}

/**
 * @brief 测试 BST 删除根节点
 */
static void test_bst_delete_root(void)
{
    const char *path = "t_bst_dr.toml";
    char *c;

    set_output(path);
    dvBinarySearchTree(b)
    {
        dvBstInsert(b, 10);
        dvBstInsert(b, 5);
        dvBstInsert(b, 15);
        dvBstDelete(b, 10);
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_result(c);
    free(c);
    cleanup(path);
}

/**
 * @brief 测试 BST 插入重复值
 */
static void test_bst_duplicate(void)
{
    const char *path = "t_bst_dup.toml";
    char *c;

    set_output(path);
    dvBinarySearchTree(b)
    {
        dvBstInsert(b, 10);
        dvBstInsert(b, 5);
        dvBstInsert(b, 10);
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_result(c);
    free(c);
    cleanup(path);
}

/**
 * @brief 测试 BST 全部删除
 */
static void test_bst_clear(void)
{
    const char *path = "t_bst_clear.toml";
    char *c;

    set_output(path);
    dvBinarySearchTree(b)
    {
        dvBstInsert(b, 10);
        dvBstInsert(b, 5);
        dvBstDelete(b, 10);
        dvBstDelete(b, 5);
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_result(c);
    ASSERT_CONTAINS(c, "root = -1");
    free(c);
    cleanup(path);
}

/**
 * @brief 测试左偏树
 */
static void test_bst_left_skewed(void)
{
    const char *path = "t_bst_ls.toml";
    char *c;

    set_output(path);
    dvBinarySearchTree(b)
    {
        for (int i = 5; i >= 1; i--)
        {
            dvBstInsert(b, i);
        }
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_result(c);
    free(c);
    cleanup(path);
}

/**
 * @brief 测试右偏树
 */
static void test_bst_right_skewed(void)
{
    const char *path = "t_bst_rs.toml";
    char *c;

    set_output(path);
    dvBinarySearchTree(b)
    {
        for (int i = 1; i <= 5; i++)
        {
            dvBstInsert(b, i);
        }
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_result(c);
    free(c);
    cleanup(path);
}

/* ================================================================
 * 堆测试
 * ================================================================ */

/**
 * @brief 测试最小堆基本操作
 */
static void test_heap_min_basic(void)
{
    const char *path = "t_heap_min.toml";
    char *c;

    set_output(path);
    dvHeap(h, "test_heap", dvHeapOrderMin)
    {
        dvHeapInsert(h, 10);
        dvHeapInsert(h, 5);
        dvHeapInsert(h, 15);
        dvHeapInsert(h, 3);
        dvHeapExtract(h);
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_result(c);
    ASSERT_CONTAINS(c, "kind = \"binary_tree\"");
    validate_bt_data(c);
    free(c);
    cleanup(path);
}

/**
 * @brief 测试最大堆基本操作
 */
static void test_heap_max_basic(void)
{
    const char *path = "t_heap_max.toml";
    char *c;

    set_output(path);
    dvHeap(h, "max_heap", dvHeapOrderMax)
    {
        dvHeapInsert(h, 10);
        dvHeapInsert(h, 20);
        dvHeapInsert(h, 5);
        dvHeapExtract(h);
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_result(c);
    free(c);
    cleanup(path);
}

/**
 * @brief 测试空堆提取 (错误)
 */
static void test_heap_extract_empty_error(void)
{
    const char *path = "t_heap_eex.toml";
    char *c;

    set_output(path);
    dvHeap(h)
    {
        dvHeapExtract(h);
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_error(c);
    free(c);
    cleanup(path);
}

/**
 * @brief 测试堆清空
 */
static void test_heap_clear(void)
{
    const char *path = "t_heap_clear.toml";
    char *c;

    set_output(path);
    dvHeap(h)
    {
        dvHeapInsert(h, 1);
        dvHeapInsert(h, 2);
        dvHeapInsert(h, 3);
        dvHeapClear(h);
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_result(c);
    ASSERT_CONTAINS(c, "root = -1");
    ASSERT_CONTAINS(c, "nodes = []");
    free(c);
    cleanup(path);
}

/**
 * @brief 测试多次提取
 */
static void test_heap_multi_extract(void)
{
    const char *path = "t_heap_mex.toml";
    char *c;

    set_output(path);
    dvHeap(h, "h", dvHeapOrderMin)
    {
        dvHeapInsert(h, 10);
        dvHeapInsert(h, 5);
        dvHeapInsert(h, 15);
        dvHeapInsert(h, 3);
        dvHeapExtract(h);
        dvHeapExtract(h);
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_result(c);
    free(c);
    cleanup(path);
}

/**
 * @brief 测试单元素堆
 */
static void test_heap_single(void)
{
    const char *path = "t_heap_single.toml";
    char *c;

    set_output(path);
    dvHeap(h)
    {
        dvHeapInsert(h, 100);
        dvHeapExtract(h);
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_result(c);
    free(c);
    cleanup(path);
}

/**
 * @brief 测试提取后再插入
 */
static void test_heap_insert_after_extract(void)
{
    const char *path = "t_heap_iae.toml";
    char *c;

    set_output(path);
    dvHeap(h, "h", dvHeapOrderMin)
    {
        dvHeapInsert(h, 10);
        dvHeapInsert(h, 5);
        dvHeapExtract(h);
        dvHeapInsert(h, 3);
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_result(c);
    free(c);
    cleanup(path);
}

/**
 * @brief 测试最小堆性质 (多值)
 */
static void test_heap_min_property(void)
{
    const char *path = "t_heap_minp.toml";
    char *c;
    int vals[] = {15, 10, 20, 8, 25, 12};

    set_output(path);
    dvHeap(h, "minp", dvHeapOrderMin)
    {
        for (int i = 0; i < 6; i++)
        {
            dvHeapInsert(h, vals[i]);
        }
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_result(c);
    free(c);
    cleanup(path);
}

/**
 * @brief 测试最大堆性质 (多值)
 */
static void test_heap_max_property(void)
{
    const char *path = "t_heap_maxp.toml";
    char *c;
    int vals[] = {15, 10, 20, 8, 25, 12};

    set_output(path);
    dvHeap(h, "maxp", dvHeapOrderMax)
    {
        for (int i = 0; i < 6; i++)
        {
            dvHeapInsert(h, vals[i]);
        }
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_result(c);
    free(c);
    cleanup(path);
}

/* ================================================================
 * 无向图测试
 * ================================================================ */

/**
 * @brief 测试无向图基本操作
 */
static void test_gu_basic(void)
{
    const char *path = "t_gu_basic.toml";
    char *c;

    set_output(path);
    dvGraphUndirected(g, "test_graph")
    {
        dvGuAddNode(g, 0, "A");
        dvGuAddNode(g, 1, "B");
        dvGuAddNode(g, 2, "C");
        dvGuAddEdge(g, 0, 1);
        dvGuAddEdge(g, 1, 2);
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_result(c);
    ASSERT_CONTAINS(c, "kind = \"graph_undirected\"");
    validate_graph_data(c);
    free(c);
    cleanup(path);
}

/**
 * @brief 测试无向图边规范化 (from < to)
 */
static void test_gu_edge_normalization(void)
{
    const char *path = "t_gu_norm.toml";
    char *c;

    set_output(path);
    dvGraphUndirected(g)
    {
        dvGuAddNode(g, 0, "A");
        dvGuAddNode(g, 1, "B");
        dvGuAddEdge(g, 1, 0);
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    ASSERT_CONTAINS(c, "from = 0");
    ASSERT_CONTAINS(c, "to = 1");
    free(c);
    cleanup(path);
}

/**
 * @brief 测试无向图自环 (错误)
 */
static void test_gu_self_loop_error(void)
{
    const char *path = "t_gu_selfloop.toml";
    char *c;

    set_output(path);
    dvGraphUndirected(g)
    {
        dvGuAddNode(g, 0, "A");
        dvGuAddEdge(g, 0, 0);
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_error(c);
    free(c);
    cleanup(path);
}

/**
 * @brief 测试无向图重复边 (错误)
 */
static void test_gu_dup_edge_error(void)
{
    const char *path = "t_gu_dupedge.toml";
    char *c;

    set_output(path);
    dvGraphUndirected(g)
    {
        dvGuAddNode(g, 0, "A");
        dvGuAddNode(g, 1, "B");
        dvGuAddEdge(g, 0, 1);
        dvGuAddEdge(g, 0, 1);
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_error(c);
    free(c);
    cleanup(path);
}

/**
 * @brief 测试删除节点 (同时删除相关边)
 */
static void test_gu_remove_node(void)
{
    const char *path = "t_gu_rn.toml";
    char *c;

    set_output(path);
    dvGraphUndirected(g)
    {
        dvGuAddNode(g, 0, "A");
        dvGuAddNode(g, 1, "B");
        dvGuAddNode(g, 2, "C");
        dvGuAddEdge(g, 0, 1);
        dvGuAddEdge(g, 1, 2);
        dvGuRemoveNode(g, 1);
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_result(c);
    free(c);
    cleanup(path);
}

/**
 * @brief 测试删除边
 */
static void test_gu_remove_edge(void)
{
    const char *path = "t_gu_re.toml";
    char *c;

    set_output(path);
    dvGraphUndirected(g)
    {
        dvGuAddNode(g, 0, "A");
        dvGuAddNode(g, 1, "B");
        dvGuAddEdge(g, 0, 1);
        dvGuRemoveEdge(g, 0, 1);
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_result(c);
    ASSERT_CONTAINS(c, "edges = []");
    free(c);
    cleanup(path);
}

/**
 * @brief 测试添加重复节点 (错误)
 */
static void test_gu_dup_node_error(void)
{
    const char *path = "t_gu_dupnode.toml";
    char *c;

    set_output(path);
    dvGraphUndirected(g)
    {
        dvGuAddNode(g, 0, "A");
        dvGuAddNode(g, 0, "B");
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_error(c);
    free(c);
    cleanup(path);
}

/**
 * @brief 测试删除不存在的节点 (错误)
 */
static void test_gu_remove_nonexist_node(void)
{
    const char *path = "t_gu_rnn.toml";
    char *c;

    set_output(path);
    dvGraphUndirected(g)
    {
        dvGuAddNode(g, 0, "A");
        dvGuRemoveNode(g, 999);
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_error(c);
    free(c);
    cleanup(path);
}

/**
 * @brief 测试删除不存在的边 (错误)
 */
static void test_gu_remove_nonexist_edge(void)
{
    const char *path = "t_gu_rne.toml";
    char *c;

    set_output(path);
    dvGraphUndirected(g)
    {
        dvGuAddNode(g, 0, "A");
        dvGuAddNode(g, 1, "B");
        dvGuRemoveEdge(g, 0, 1);
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_error(c);
    free(c);
    cleanup(path);
}

/**
 * @brief 测试更新节点标签
 */
static void test_gu_update_label(void)
{
    const char *path = "t_gu_ulbl.toml";
    char *c;

    set_output(path);
    dvGraphUndirected(g)
    {
        dvGuAddNode(g, 0, "A");
        dvGuUpdateNodeLabel(g, 0, "X");
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_result(c);
    ASSERT_CONTAINS(c, "\"X\"");
    free(c);
    cleanup(path);
}

/* ================================================================
 * 有向图测试
 * ================================================================ */

/**
 * @brief 测试有向图基本操作
 */
static void test_gd_basic(void)
{
    const char *path = "t_gd_basic.toml";
    char *c;

    set_output(path);
    dvGraphDirected(g, "test_digraph")
    {
        dvGdAddNode(g, 0, "A");
        dvGdAddNode(g, 1, "B");
        dvGdAddEdge(g, 0, 1);
        dvGdAddEdge(g, 1, 0);
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_result(c);
    ASSERT_CONTAINS(c, "kind = \"graph_directed\"");
    free(c);
    cleanup(path);
}

/**
 * @brief 测试有向图不进行边规范化
 */
static void test_gd_no_normalization(void)
{
    const char *path = "t_gd_nonorm.toml";
    char *c;

    set_output(path);
    dvGraphDirected(g)
    {
        dvGdAddNode(g, 0, "A");
        dvGdAddNode(g, 1, "B");
        dvGdAddEdge(g, 1, 0);
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    ASSERT_CONTAINS(c, "from = 1");
    ASSERT_CONTAINS(c, "to = 0");
    free(c);
    cleanup(path);
}

/**
 * @brief 测试有向图自环 (错误)
 */
static void test_gd_self_loop_error(void)
{
    const char *path = "t_gd_selfloop.toml";
    char *c;

    set_output(path);
    dvGraphDirected(g)
    {
        dvGdAddNode(g, 0, "A");
        dvGdAddEdge(g, 0, 0);
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_error(c);
    free(c);
    cleanup(path);
}

/**
 * @brief 测试有向图重复边 (错误)
 */
static void test_gd_dup_edge_error(void)
{
    const char *path = "t_gd_dupedge.toml";
    char *c;

    set_output(path);
    dvGraphDirected(g)
    {
        dvGdAddNode(g, 0, "A");
        dvGdAddNode(g, 1, "B");
        dvGdAddEdge(g, 0, 1);
        dvGdAddEdge(g, 0, 1);
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_error(c);
    free(c);
    cleanup(path);
}

/**
 * @brief 测试有向图删除节点
 */
static void test_gd_remove_node(void)
{
    const char *path = "t_gd_rn.toml";
    char *c;

    set_output(path);
    dvGraphDirected(g)
    {
        dvGdAddNode(g, 0, "A");
        dvGdAddNode(g, 1, "B");
        dvGdAddNode(g, 2, "C");
        dvGdAddEdge(g, 0, 1);
        dvGdAddEdge(g, 1, 2);
        dvGdRemoveNode(g, 1);
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_result(c);
    free(c);
    cleanup(path);
}

/**
 * @brief 测试有向图删除边
 */
static void test_gd_remove_edge(void)
{
    const char *path = "t_gd_re.toml";
    char *c;

    set_output(path);
    dvGraphDirected(g)
    {
        dvGdAddNode(g, 0, "A");
        dvGdAddNode(g, 1, "B");
        dvGdAddEdge(g, 0, 1);
        dvGdRemoveEdge(g, 0, 1);
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_result(c);
    free(c);
    cleanup(path);
}

/**
 * @brief 测试有向图环
 */
static void test_gd_cycle(void)
{
    const char *path = "t_gd_cycle.toml";
    char *c;

    set_output(path);
    dvGraphDirected(g)
    {
        for (int i = 0; i < 4; i++)
        {
            char lbl[8];
            snprintf(lbl, sizeof(lbl), "N%d", i);
            dvGdAddNode(g, i, lbl);
        }
        dvGdAddEdge(g, 0, 1);
        dvGdAddEdge(g, 1, 2);
        dvGdAddEdge(g, 2, 3);
        dvGdAddEdge(g, 3, 1);
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_result(c);
    free(c);
    cleanup(path);
}

/* ================================================================
 * 带权图测试
 * ================================================================ */

/**
 * @brief 测试无向带权图
 */
static void test_gw_undirected(void)
{
    const char *path = "t_gw_undir.toml";
    char *c;

    set_output(path);
    dvGraphWeighted(g, "test_wgraph")
    {
        dvGwAddNode(g, 0, "A");
        dvGwAddNode(g, 1, "B");
        dvGwAddEdge(g, 0, 1, 3.5);
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_result(c);
    ASSERT_CONTAINS(c, "kind = \"graph_weighted\"");
    ASSERT_CONTAINS(c, "weight = ");
    free(c);
    cleanup(path);
}

/**
 * @brief 测试有向带权图
 */
static void test_gw_directed(void)
{
    const char *path = "t_gw_dir.toml";
    char *c;

    set_output(path);
    dvGraphWeighted(g, "dw", true)
    {
        dvGwAddNode(g, 0, "A");
        dvGwAddNode(g, 1, "B");
        dvGwAddEdge(g, 0, 1, 2.0);
        dvGwAddEdge(g, 1, 0, 3.0);
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_result(c);
    free(c);
    cleanup(path);
}

/**
 * @brief 测试更新边权重
 */
static void test_gw_update_weight(void)
{
    const char *path = "t_gw_uw.toml";
    char *c;

    set_output(path);
    dvGraphWeighted(g)
    {
        dvGwAddNode(g, 0, "A");
        dvGwAddNode(g, 1, "B");
        dvGwAddEdge(g, 0, 1, 1.0);
        dvGwUpdateWeight(g, 0, 1, 5.0);
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_result(c);
    free(c);
    cleanup(path);
}

/**
 * @brief 测试更新节点标签
 */
static void test_gw_update_label(void)
{
    const char *path = "t_gw_ul.toml";
    char *c;

    set_output(path);
    dvGraphWeighted(g)
    {
        dvGwAddNode(g, 0, "A");
        dvGwUpdateNodeLabel(g, 0, "X");
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_result(c);
    ASSERT_CONTAINS(c, "\"X\"");
    free(c);
    cleanup(path);
}

/**
 * @brief 测试负权重
 */
static void test_gw_negative_weight(void)
{
    const char *path = "t_gw_nw.toml";
    char *c;

    set_output(path);
    dvGraphWeighted(g)
    {
        dvGwAddNode(g, 0, "A");
        dvGwAddNode(g, 1, "B");
        dvGwAddEdge(g, 0, 1, -5.5);
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_result(c);
    ASSERT_CONTAINS(c, "-5.5");
    free(c);
    cleanup(path);
}

/**
 * @brief 测试零权重
 */
static void test_gw_zero_weight(void)
{
    const char *path = "t_gw_zw.toml";
    char *c;

    set_output(path);
    dvGraphWeighted(g)
    {
        dvGwAddNode(g, 0, "A");
        dvGwAddNode(g, 1, "B");
        dvGwAddEdge(g, 0, 1, 0.0);
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_result(c);
    free(c);
    cleanup(path);
}

/**
 * @brief 测试更新不存在的边权重 (错误)
 */
static void test_gw_update_nonexist_error(void)
{
    const char *path = "t_gw_une.toml";
    char *c;

    set_output(path);
    dvGraphWeighted(g)
    {
        dvGwAddNode(g, 0, "A");
        dvGwAddNode(g, 1, "B");
        dvGwUpdateWeight(g, 0, 1, 5.0);
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_error(c);
    free(c);
    cleanup(path);
}

/**
 * @brief 测试删除带权边
 */
static void test_gw_remove_edge(void)
{
    const char *path = "t_gw_re.toml";
    char *c;

    set_output(path);
    dvGraphWeighted(g)
    {
        dvGwAddNode(g, 0, "A");
        dvGwAddNode(g, 1, "B");
        dvGwAddEdge(g, 0, 1, 1.0);
        dvGwRemoveEdge(g, 0, 1);
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_result(c);
    free(c);
    cleanup(path);
}

/**
 * @brief 测试带权图清空
 */
static void test_gw_clear(void)
{
    const char *path = "t_gw_clear.toml";
    char *c;

    set_output(path);
    dvGraphWeighted(g)
    {
        dvGwAddNode(g, 0, "A");
        dvGwAddNode(g, 1, "B");
        dvGwRemoveNode(g, 0);
        dvGwRemoveNode(g, 1);
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_result(c);
    free(c);
    cleanup(path);
}

/* ================================================================
 * 全局配置测试
 * ================================================================ */

/**
 * @brief 测试配置 remarks
 */
static void test_config_remarks(void)
{
    const char *path = "t_cfg_remarks.toml";
    char *c;

    dvConfig((dvConfigOptions){
        .output_path = path,
        .title = "Test Title",
        .author = "Test Author",
        .comment = "Test Comment"});
    dvStack(s)
    {
        dvStackPush(s, 1);
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    ASSERT_CONTAINS(c, "[remarks]");
    ASSERT_CONTAINS(c, "title = \"Test Title\"");
    ASSERT_CONTAINS(c, "author = \"Test Author\"");
    ASSERT_CONTAINS(c, "comment = \"Test Comment\"");
    free(c);
    cleanup(path);
    reset_config();
}

/**
 * @brief 测试配置输出路径
 */
static void test_config_output_path(void)
{
    const char *path = "t_cfg_outpath.toml";
    char *c;

    dvConfig((dvConfigOptions){.output_path = path});
    dvStack(s)
    {
        dvStackPush(s, 1);
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_result(c);
    free(c);
    cleanup(path);
    reset_config();
}

/**
 * @brief 测试只配置 title
 */
static void test_config_only_title(void)
{
    const char *path = "t_cfg_title.toml";
    char *c;

    dvConfig((dvConfigOptions){.output_path = path, .title = "Only Title"});
    dvStack(s)
    {
        dvStackPush(s, 1);
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    ASSERT_CONTAINS(c, "title = \"Only Title\"");
    free(c);
    cleanup(path);
    reset_config();
}

/**
 * @brief 测试只配置 author
 */
static void test_config_only_author(void)
{
    const char *path = "t_cfg_author.toml";
    char *c;

    dvConfig((dvConfigOptions){.output_path = path, .author = "Only Author"});
    dvStack(s)
    {
        dvStackPush(s, 1);
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    ASSERT_CONTAINS(c, "author = \"Only Author\"");
    free(c);
    cleanup(path);
    reset_config();
}

/**
 * @brief 测试只配置 comment
 */
static void test_config_only_comment(void)
{
    const char *path = "t_cfg_comment.toml";
    char *c;

    dvConfig((dvConfigOptions){.output_path = path, .comment = "Only Comment"});
    dvStack(s)
    {
        dvStackPush(s, 1);
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    ASSERT_CONTAINS(c, "comment = \"Only Comment\"");
    free(c);
    cleanup(path);
    reset_config();
}

/* ================================================================
 * 边缘情况测试
 * ================================================================ */

/**
 * @brief 测试空数据结构 (无操作)
 */
static void test_edge_empty_structure(void)
{
    const char *path = "t_edge_empty.toml";
    char *c;

    set_output(path);
    dvStack(s)
    {
        /* 不执行任何操作 */
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_result(c);
    ASSERT_INT_EQ(count_str(c, "[[states]]"), 1);
    free(c);
    cleanup(path);
}

/**
 * @brief 测试字符串中的特殊字符
 */
static void test_edge_special_chars(void)
{
    const char *path = "t_edge_special.toml";
    char *c;

    set_output(path);
    dvStack(s)
    {
        dvStackPush(s, "hello\"world");
        dvStackPush(s, "line1\\nline2");
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_result(c);
    free(c);
    cleanup(path);
}

/**
 * @brief 测试大数字
 */
static void test_edge_large_numbers(void)
{
    const char *path = "t_edge_large.toml";
    char *c;

    set_output(path);
    dvStack(s)
    {
        dvStackPush(s, (long long)1000000000000000000LL);
        dvStackPush(s, (long long)-1000000000000000000LL);
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_result(c);
    free(c);
    cleanup(path);
}

/**
 * @brief 测试大量操作 (100 次 push + 50 次 pop)
 */
static void test_edge_many_operations(void)
{
    const char *path = "t_edge_many.toml";
    char *c;

    set_output(path);
    dvStack(s)
    {
        for (int i = 0; i < 100; i++)
        {
            dvStackPush(s, i);
        }
        for (int i = 0; i < 50; i++)
        {
            dvStackPop(s);
        }
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_result(c);
    ASSERT_INT_EQ(count_str(c, "[[states]]"), 151);
    ASSERT_INT_EQ(count_str(c, "[[steps]]"), 150);
    free(c);
    cleanup(path);
}

/**
 * @brief 测试节点标签边界 (长度 1 和 32)
 */
static void test_edge_label_boundary(void)
{
    const char *path = "t_edge_lblbd.toml";
    char *c;

    set_output(path);
    dvGraphUndirected(g)
    {
        dvGuAddNode(g, 0, "A");
        dvGuAddNode(g, 1, "ABCDEFGHIJKLMNOPQRSTUVWXYZ012345");
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_result(c);
    free(c);
    cleanup(path);
}

/**
 * @brief 测试节点标签过长 (错误)
 */
static void test_edge_label_too_long(void)
{
    const char *path = "t_edge_lbllong.toml";
    char *c;

    set_output(path);
    dvGraphUndirected(g)
    {
        dvGuAddNode(g, 0, "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456");
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_error(c);
    free(c);
    cleanup(path);
}

/**
 * @brief 测试节点标签为空 (错误)
 */
static void test_edge_label_empty(void)
{
    const char *path = "t_edge_lblempty.toml";
    char *c;

    set_output(path);
    dvGraphUndirected(g)
    {
        dvGuAddNode(g, 0, "");
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_error(c);
    free(c);
    cleanup(path);
}

/**
 * @brief 测试 Unicode 字符串
 */
static void test_edge_unicode(void)
{
    const char *path = "t_edge_unicode.toml";
    char *c;

    set_output(path);
    dvStack(s)
    {
        dvStackPush(s, "你好");
        dvStackPush(s, "世界");
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_result(c);
    free(c);
    cleanup(path);
}

/**
 * @brief 测试混合类型值
 */
static void test_edge_mixed_types(void)
{
    const char *path = "t_edge_mixed.toml";
    char *c;

    set_output(path);
    dvStack(s)
    {
        dvStackPush(s, 1);
        dvStackPush(s, 2.5);
        dvStackPush(s, "text");
        dvStackPush(s, true);
        dvStackPush(s, false);
        dvStackPop(s);
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_result(c);
    free(c);
    cleanup(path);
}

/**
 * @brief 测试错误后后续操作静默跳过
 */
static void test_edge_error_skips(void)
{
    const char *path = "t_edge_errskip.toml";
    char *c;
    int root;

    set_output(path);
    dvBinaryTree(t)
    {
        root = dvBtInsertRoot(t, 10);
        dvBtInsertLeft(t, 999, 5);
        /* 以下操作应被跳过 */
        dvBtInsertRight(t, root, 15);
    }
    (void)root;
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_error(c);
    /* 只应有 2 个状态 (初始 + insert_root 后) */
    ASSERT_INT_EQ(count_str(c, "[[states]]"), 2);
    free(c);
    cleanup(path);
}

/* ================================================================
 * 复杂场景测试
 * ================================================================ */

/**
 * @brief 测试 BST 复杂删除场景
 */
static void test_complex_bst_deletion(void)
{
    const char *path = "t_cplx_bst.toml";
    char *c;
    int vals[] = {50, 30, 70, 20, 40, 60, 80, 35, 45};

    set_output(path);
    dvBinarySearchTree(b)
    {
        for (int i = 0; i < 9; i++)
        {
            dvBstInsert(b, vals[i]);
        }
        dvBstDelete(b, 30);
        dvBstDelete(b, 50);
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_result(c);
    free(c);
    cleanup(path);
}

/**
 * @brief 测试图的复杂操作
 */
static void test_complex_graph(void)
{
    const char *path = "t_cplx_graph.toml";
    char *c;

    set_output(path);
    dvGraphWeighted(g, "complex", true)
    {
        for (int i = 0; i < 5; i++)
        {
            char lbl[4];
            lbl[0] = (char)('A' + i);
            lbl[1] = '\0';
            dvGwAddNode(g, i, lbl);
        }
        dvGwAddEdge(g, 0, 1, 1.0);
        dvGwAddEdge(g, 0, 2, 2.0);
        dvGwAddEdge(g, 1, 2, 1.5);
        dvGwAddEdge(g, 2, 3, 2.5);
        dvGwAddEdge(g, 3, 4, 1.0);
        dvGwAddEdge(g, 4, 0, 3.0);
        dvGwUpdateWeight(g, 0, 1, 10.0);
        dvGwRemoveNode(g, 2);
        dvGwUpdateNodeLabel(g, 0, "Start");
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_result(c);
    free(c);
    cleanup(path);
}

/**
 * @brief 测试链表的复杂操作
 */
static void test_complex_linked_list(void)
{
    const char *path = "t_cplx_dlist.toml";
    char *c;
    int nodes[5];

    set_output(path);
    dvDoubleLinkedList(l)
    {
        for (int i = 0; i < 5; i++)
        {
            nodes[i] = dvDlInsertTail(l, i);
        }
        dvDlInsertAfter(l, nodes[2], 100);
        dvDlInsertBefore(l, nodes[2], 200);
        dvDlDeleteHead(l);
        dvDlDeleteTail(l);
        dvDlReverse(l);
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_result(c);
    free(c);
    cleanup(path);
}

/**
 * @brief 测试堆排序模拟
 */
static void test_complex_heap_sort(void)
{
    const char *path = "t_cplx_hsort.toml";
    char *c;
    int vals[] = {5, 2, 8, 1, 9, 3};

    set_output(path);
    dvHeap(h, "sort", dvHeapOrderMin)
    {
        for (int i = 0; i < 6; i++)
        {
            dvHeapInsert(h, vals[i]);
        }
        for (int i = 0; i < 6; i++)
        {
            dvHeapExtract(h);
        }
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_result(c);
    /* 6 inserts + 6 extracts = 12 steps, 13 states */
    ASSERT_INT_EQ(count_str(c, "[[steps]]"), 12);
    ASSERT_INT_EQ(count_str(c, "[[states]]"), 13);
    free(c);
    cleanup(path);
}

/**
 * @brief 测试快速上手示例 (文档中的 quick start)
 */
static void test_complex_quickstart(void)
{
    const char *path = "t_cplx_qs.toml";
    char *c;

    dvConfig((dvConfigOptions){
        .output_path = path,
        .title = "无向图演示",
        .author = "WaterRun",
        .comment = "C quick start"});
    dvGraphUndirected(graph, "graph")
    {
        dvGuAddNode(graph, 0, "A");
        dvGuAddNode(graph, 1, "B");
        dvGuAddNode(graph, 2, "C");
        dvGuAddEdge(graph, 0, 1);
        dvGuAddEdge(graph, 1, 2);
    }
    c = read_file(path);
    ASSERT_TRUE(c != NULL);
    validate_toml(c);
    validate_result(c);
    ASSERT_CONTAINS(c, "[remarks]");
    ASSERT_CONTAINS(c, "\"WaterRun\"");
    free(c);
    cleanup(path);
    reset_config();
}

/* ================================================================
 * 主函数
 * ================================================================ */

int main(void)
{
    printf("ds4viz C library test suite\n");
    printf("========================================\n");

    SECTION("Stack");
    RUN_TEST(test_stack_basic);
    RUN_TEST(test_stack_empty_pop_error);
    RUN_TEST(test_stack_clear);
    RUN_TEST(test_stack_various_types);
    RUN_TEST(test_stack_negative);
    RUN_TEST(test_stack_mixed);
    RUN_TEST(test_stack_single);
    RUN_TEST(test_stack_push_after_clear);
    RUN_TEST(test_stack_multi_pop);
    RUN_TEST(test_stack_float);
    RUN_TEST(test_stack_string);
    RUN_TEST(test_stack_bool);
    RUN_TEST(test_stack_zero);

    SECTION("Queue");
    RUN_TEST(test_queue_basic);
    RUN_TEST(test_queue_empty_dequeue_error);
    RUN_TEST(test_queue_clear);
    RUN_TEST(test_queue_consistency);
    RUN_TEST(test_queue_single);
    RUN_TEST(test_queue_enqueue_after_clear);
    RUN_TEST(test_queue_multi_dequeue);
    RUN_TEST(test_queue_alternating);
    RUN_TEST(test_queue_until_empty);
    RUN_TEST(test_queue_string);

    SECTION("SingleLinkedList");
    RUN_TEST(test_slist_basic);
    RUN_TEST(test_slist_reverse);
    RUN_TEST(test_slist_delete_nonexistent);
    RUN_TEST(test_slist_empty_delete_head);
    RUN_TEST(test_slist_insert_after_nonexist);
    RUN_TEST(test_slist_delete_tail);
    RUN_TEST(test_slist_single_node);
    RUN_TEST(test_slist_reverse_empty);
    RUN_TEST(test_slist_multi_insert);

    SECTION("DoubleLinkedList");
    RUN_TEST(test_dlist_basic);
    RUN_TEST(test_dlist_reverse);
    RUN_TEST(test_dlist_consistency);
    RUN_TEST(test_dlist_delete_head_error);
    RUN_TEST(test_dlist_delete_tail_error);
    RUN_TEST(test_dlist_insert_before_nonexist);
    RUN_TEST(test_dlist_insert_after_nonexist);
    RUN_TEST(test_dlist_delete_nonexist);
    RUN_TEST(test_dlist_clear);
    RUN_TEST(test_dlist_single_node);
    RUN_TEST(test_dlist_reverse_empty);
    RUN_TEST(test_dlist_multi_ops);

    SECTION("BinaryTree");
    RUN_TEST(test_bt_basic);
    RUN_TEST(test_bt_delete_subtree);
    RUN_TEST(test_bt_update);
    RUN_TEST(test_bt_dup_root_error);
    RUN_TEST(test_bt_dup_child_error);
    RUN_TEST(test_bt_no_parent_error);
    RUN_TEST(test_bt_delete_root);
    RUN_TEST(test_bt_delete_nonexist_error);
    RUN_TEST(test_bt_update_nonexist_error);
    RUN_TEST(test_bt_complete);

    SECTION("BinarySearchTree");
    RUN_TEST(test_bst_basic);
    RUN_TEST(test_bst_delete_leaf);
    RUN_TEST(test_bst_delete_one_child);
    RUN_TEST(test_bst_delete_two_children);
    RUN_TEST(test_bst_delete_nonexist_error);
    RUN_TEST(test_bst_delete_root);
    RUN_TEST(test_bst_duplicate);
    RUN_TEST(test_bst_clear);
    RUN_TEST(test_bst_left_skewed);
    RUN_TEST(test_bst_right_skewed);

    SECTION("Heap");
    RUN_TEST(test_heap_min_basic);
    RUN_TEST(test_heap_max_basic);
    RUN_TEST(test_heap_extract_empty_error);
    RUN_TEST(test_heap_clear);
    RUN_TEST(test_heap_multi_extract);
    RUN_TEST(test_heap_single);
    RUN_TEST(test_heap_insert_after_extract);
    RUN_TEST(test_heap_min_property);
    RUN_TEST(test_heap_max_property);

    SECTION("GraphUndirected");
    RUN_TEST(test_gu_basic);
    RUN_TEST(test_gu_edge_normalization);
    RUN_TEST(test_gu_self_loop_error);
    RUN_TEST(test_gu_dup_edge_error);
    RUN_TEST(test_gu_remove_node);
    RUN_TEST(test_gu_remove_edge);
    RUN_TEST(test_gu_dup_node_error);
    RUN_TEST(test_gu_remove_nonexist_node);
    RUN_TEST(test_gu_remove_nonexist_edge);
    RUN_TEST(test_gu_update_label);

    SECTION("GraphDirected");
    RUN_TEST(test_gd_basic);
    RUN_TEST(test_gd_no_normalization);
    RUN_TEST(test_gd_self_loop_error);
    RUN_TEST(test_gd_dup_edge_error);
    RUN_TEST(test_gd_remove_node);
    RUN_TEST(test_gd_remove_edge);
    RUN_TEST(test_gd_cycle);

    SECTION("GraphWeighted");
    RUN_TEST(test_gw_undirected);
    RUN_TEST(test_gw_directed);
    RUN_TEST(test_gw_update_weight);
    RUN_TEST(test_gw_update_label);
    RUN_TEST(test_gw_negative_weight);
    RUN_TEST(test_gw_zero_weight);
    RUN_TEST(test_gw_update_nonexist_error);
    RUN_TEST(test_gw_remove_edge);
    RUN_TEST(test_gw_clear);

    SECTION("GlobalConfig");
    RUN_TEST(test_config_remarks);
    RUN_TEST(test_config_output_path);
    RUN_TEST(test_config_only_title);
    RUN_TEST(test_config_only_author);
    RUN_TEST(test_config_only_comment);

    SECTION("EdgeCases");
    RUN_TEST(test_edge_empty_structure);
    RUN_TEST(test_edge_special_chars);
    RUN_TEST(test_edge_large_numbers);
    RUN_TEST(test_edge_many_operations);
    RUN_TEST(test_edge_label_boundary);
    RUN_TEST(test_edge_label_too_long);
    RUN_TEST(test_edge_label_empty);
    RUN_TEST(test_edge_unicode);
    RUN_TEST(test_edge_mixed_types);
    RUN_TEST(test_edge_error_skips);

    SECTION("ComplexScenarios");
    RUN_TEST(test_complex_bst_deletion);
    RUN_TEST(test_complex_graph);
    RUN_TEST(test_complex_linked_list);
    RUN_TEST(test_complex_heap_sort);
    RUN_TEST(test_complex_quickstart);

    printf("\n========================================\n");
    printf("Results: %d passed, %d failed, %d total (%d asserts)\n",
           g_tests_passed, g_tests_failed, g_tests_run, g_asserts);
    printf("========================================\n");

    return g_tests_failed > 0 ? 1 : 0;
}