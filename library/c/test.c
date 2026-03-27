/**
 * @file test.c
 * @brief ds4viz C 库集成测试
 *
 * 与 Python ds4viz 库的 integration_test.py 和 unit_test.py 对齐,
 * 覆盖所有数据结构的基本操作、错误处理、高亮标记、阶段管理、别名等功能.
 *
 * 编译: gcc -std=c2x -Wall -Wextra -Wpedantic -O2 -o test test.c -lm
 * 运行: ./test
 *
 * @author WaterRun
 * @date 2026-03-27
 * @see https://github.com/Water-Run/ds4viz
 */

#define DS4VIZ_IMPLEMENTATION
#include "ds4viz.h"

#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ================================================================
 * 测试框架
 * ================================================================ */

/**
 * @brief 已运行测试数
 */
static int g_tests_run = 0;

/**
 * @brief 已通过测试数
 */
static int g_tests_passed = 0;

/**
 * @brief 已失败测试数
 */
static int g_tests_failed = 0;

/**
 * @brief 当前测试名
 */
static const char *g_current_test = NULL;

/**
 * @brief 断言条件为真, 否则报告失败并返回
 *
 * @param cond 断言条件
 * @param msg 失败消息
 *
 * @def T_ASSERT
 */
#define T_ASSERT(cond, msg)                                \
    do                                                     \
    {                                                      \
        if (!(cond))                                       \
        {                                                  \
            fprintf(stderr, "    FAIL [%s] line %d: %s\n", \
                    g_current_test, __LINE__, (msg));      \
            g_tests_failed++;                              \
            return;                                        \
        }                                                  \
    } while (0)

/**
 * @brief 断言字符串包含子串
 *
 * @param hay 被搜索字符串
 * @param ndl 目标子串
 *
 * @def T_CONTAINS
 */
#define T_CONTAINS(hay, ndl)             \
    T_ASSERT(str_contains((hay), (ndl)), \
             "Expected to contain: " ndl)

/**
 * @brief 断言字符串不包含子串
 *
 * @param hay 被搜索字符串
 * @param ndl 目标子串
 *
 * @def T_NOT_CONTAINS
 */
#define T_NOT_CONTAINS(hay, ndl)          \
    T_ASSERT(!str_contains((hay), (ndl)), \
             "Expected NOT to contain: " ndl)

/**
 * @brief 断言整数相等
 *
 * @param a 期望值
 * @param b 实际值
 *
 * @def T_INT_EQ
 */
#define T_INT_EQ(a, b) \
    T_ASSERT((a) == (b), "Integer mismatch")

/**
 * @brief 运行测试函数并报告结果
 *
 * @param fn 测试函数名
 *
 * @def RUN_TEST
 */
#define RUN_TEST(fn)                       \
    do                                     \
    {                                      \
        int _old_f = g_tests_failed;       \
        g_current_test = #fn;              \
        g_tests_run++;                     \
        fn();                              \
        if (g_tests_failed == _old_f)      \
        {                                  \
            g_tests_passed++;              \
            printf("    PASS  %s\n", #fn); \
        }                                  \
    } while (0)

/**
 * @brief 验证 TOML 内容并断言非空
 *
 * @param p 内容指针
 *
 * @def T_VALID
 */
#define T_VALID(p)                                            \
    do                                                        \
    {                                                         \
        T_ASSERT((p) != NULL, "Output file missing");         \
        T_ASSERT(validate_toml(p), "Invalid TOML structure"); \
    } while (0)

/**
 * @brief 释放内容指针并清理测试环境
 *
 * @param p 内容指针
 *
 * @def T_TEARDOWN
 */
#define T_TEARDOWN(p)   \
    do                  \
    {                   \
        free(p);        \
        cleanup_test(); \
    } while (0)

/* ================================================================
 * 辅助函数
 * ================================================================ */

/**
 * @brief 全局测试输出路径
 */
static char g_test_path[256];

/**
 * @brief 读取文件全部内容
 *
 * @param p_path 文件路径
 * @return char* 堆分配的内容, 调用者负责释放; 失败返回 NULL
 */
static char *
read_file(const char *p_path)
{
    FILE *p_f;
    long sz;
    char *p_buf;

    p_f = fopen(p_path, "rb");
    if (!p_f)
    {
        return NULL;
    }
    fseek(p_f, 0, SEEK_END);
    sz = ftell(p_f);
    fseek(p_f, 0, SEEK_SET);
    p_buf = (char *)malloc((size_t)sz + 1);
    if (p_buf)
    {
        fread(p_buf, 1, (size_t)sz, p_f);
        p_buf[sz] = '\0';
    }
    fclose(p_f);
    return p_buf;
}

/**
 * @brief 检查字符串是否包含子串
 *
 * @param p_hay 被搜索字符串
 * @param p_ndl 目标子串
 * @return bool 包含返回 true
 */
static bool
str_contains(const char *p_hay, const char *p_ndl)
{
    if (!p_hay || !p_ndl)
    {
        return false;
    }
    return strstr(p_hay, p_ndl) != NULL;
}

/**
 * @brief 统计子串在字符串中出现的次数
 *
 * @param p_hay 被搜索字符串
 * @param p_ndl 目标子串
 * @return int 出现次数
 */
static int
str_count(const char *p_hay, const char *p_ndl)
{
    int count = 0;
    size_t nlen;
    const char *p_pos;

    if (!p_hay || !p_ndl)
    {
        return 0;
    }
    nlen = strlen(p_ndl);
    if (nlen == 0)
    {
        return 0;
    }
    p_pos = p_hay;
    while ((p_pos = strstr(p_pos, p_ndl)) != NULL)
    {
        count++;
        p_pos += nlen;
    }
    return count;
}

/**
 * @brief 简易 TOML IR 验证
 *
 * @param p_c TOML 内容
 * @return bool 通过验证返回 true
 */
static bool
validate_toml(const char *p_c)
{
    bool has_result;
    bool has_error;

    if (!p_c)
    {
        return false;
    }
    if (!str_contains(p_c, "[meta]"))
    {
        return false;
    }
    if (!str_contains(p_c, "[package]"))
    {
        return false;
    }
    if (!str_contains(p_c, "[object]"))
    {
        return false;
    }
    if (!str_contains(p_c, "[[states]]"))
    {
        return false;
    }
    if (!str_contains(p_c, "lang = \"c\""))
    {
        return false;
    }
    if (!str_contains(p_c, "version = \"0.1.0\""))
    {
        return false;
    }

    has_result = str_contains(p_c, "[result]");
    has_error = str_contains(p_c, "[error]");
    if (has_result && has_error)
    {
        return false;
    }
    if (!has_result && !has_error)
    {
        return false;
    }
    return true;
}

/**
 * @brief 读取 [result.commit] 的 line 值
 *
 * @param p_c TOML 内容
 * @param p_line 输出行号
 * @return bool 读取成功返回 true
 */
static bool
toml_get_commit_line(const char *p_c, int *p_line)
{
    const char *p_pos;
    char *p_end;
    long v;

    if (!p_c || !p_line)
    {
        return false;
    }
    p_pos = strstr(p_c, "[result.commit]");
    if (!p_pos)
    {
        return false;
    }
    p_pos = strstr(p_pos, "line = ");
    if (!p_pos)
    {
        return false;
    }
    p_pos += 7;
    v = strtol(p_pos, &p_end, 10);
    if (p_end == p_pos)
    {
        return false;
    }
    *p_line = (int)v;
    return true;
}

/**
 * @brief 在指定 token 之后读取最近的 [steps.code].line
 *
 * @param p_c TOML 内容
 * @param p_token 锚点 token
 * @param p_line 输出行号
 * @return bool 读取成功返回 true
 */
static bool
toml_get_step_line_after_token(const char *p_c, const char *p_token, int *p_line)
{
    const char *p_pos;
    char *p_end;
    long v;

    if (!p_c || !p_token || !p_line)
    {
        return false;
    }
    p_pos = strstr(p_c, p_token);
    if (!p_pos)
    {
        return false;
    }
    p_pos = strstr(p_pos, "[steps.code]");
    if (!p_pos)
    {
        return false;
    }
    p_pos = strstr(p_pos, "line = ");
    if (!p_pos)
    {
        return false;
    }
    p_pos += 7;
    v = strtol(p_pos, &p_end, 10);
    if (p_end == p_pos)
    {
        return false;
    }
    *p_line = (int)v;
    return true;
}

/**
 * @brief 设置测试环境: 配置唯一输出路径
 *
 * @param p_name 测试名
 */
static void
setup_test(const char *p_name)
{
    snprintf(g_test_path, sizeof(g_test_path), "test_%s.toml", p_name);
    ds4vizConfig((ds4vizConfigOptions){.output_path = g_test_path});
}

/**
 * @brief 清理测试环境: 删除输出文件并重置全局配置
 */
static void
cleanup_test(void)
{
    remove(g_test_path);
    ds4vizConfig((ds4vizConfigOptions){0});
}

/**
 * @brief 获取当前测试的输出内容
 *
 * @return char* 堆分配的内容, 调用者负责释放
 */
static char *
get_output(void)
{
    return read_file(g_test_path);
}

/* ================================================================
 * TestStackIntegration: 栈集成测试
 * ================================================================ */

/**
 * @brief 测试栈基本操作
 */
static void
test_stack_basic_operations(void)
{
    char *p_c;
    bool pop_ok = false;

    setup_test("stack_basic");
    ds4vizStack(s, "test")
    {
        ds4vizValue val;

        ds4vizStackPush(s, 10);
        ds4vizStackPush(s, 20);
        val = ds4vizStackPop(s);
        pop_ok = (val.tag == DS4VIZ_INT && ds4vizInt(val) == 20);
        ds4vizStackPush(s, 30);
    }
    T_ASSERT(pop_ok, "Pop should return 20");
    p_c = get_output();
    T_VALID(p_c);
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试空栈弹出错误
 */
static void
test_stack_empty_pop_error(void)
{
    char *p_c;

    setup_test("stack_empty_pop");
    ds4vizStack(s)
    {
        ds4vizStackPop(s);
    }
    p_c = get_output();
    T_VALID(p_c);
    T_CONTAINS(p_c, "[error]");
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试栈清空
 */
static void
test_stack_clear(void)
{
    char *p_c;

    setup_test("stack_clear");
    ds4vizStack(s)
    {
        ds4vizStackPush(s, 1);
        ds4vizStackPush(s, 2);
        ds4vizStackClear(s);
    }
    p_c = get_output();
    T_VALID(p_c);
    T_CONTAINS(p_c, "items = []");
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试栈各种值类型
 */
static void
test_stack_various_types(void)
{
    char *p_c;

    setup_test("stack_types");
    ds4vizStack(s)
    {
        ds4vizStackPush(s, 42);
        ds4vizStackPush(s, 3.14);
        ds4vizStackPush(s, "hello");
        ds4vizStackPush(s, true);
    }
    p_c = get_output();
    T_VALID(p_c);
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试 step 和 amend
 */
static void
test_stack_step_and_amend(void)
{
    char *p_c;

    setup_test("stack_step_amend");
    ds4vizStack(s)
    {
        ds4vizStackPush(s, 10);
        ds4vizAmend(s, "push 10",
                    ds4vizItem(0, DS4VIZ_FOCUS, 3));
        ds4vizStackPush(s, 20);
        ds4vizStep(s, "observe stack",
                   ds4vizItem(0, DS4VIZ_VISITED),
                   ds4vizItem(1, DS4VIZ_FOCUS, 3));
    }
    p_c = get_output();
    T_VALID(p_c);
    T_CONTAINS(p_c, "note = \"push 10\"");
    T_CONTAINS(p_c, "op = \"observe\"");
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试 phase
 */
static void
test_stack_phase(void)
{
    char *p_c;

    setup_test("stack_phase");
    ds4vizStack(s)
    {
        ds4vizPhase(s, "build")
        {
            ds4vizStackPush(s, 10);
            ds4vizStackPush(s, 20);
        }
        ds4vizPhase(s, "pop")
        {
            ds4vizStackPop(s);
        }
    }
    p_c = get_output();
    T_VALID(p_c);
    T_CONTAINS(p_c, "phase = \"build\"");
    T_CONTAINS(p_c, "phase = \"pop\"");
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试 push-pop-push 循环操作
 */
static void
test_stack_push_pop_push_cycle(void)
{
    char *p_c;
    bool ok1 = false;
    bool ok2 = false;
    bool ok3 = false;

    setup_test("stack_cycle");
    ds4vizStack(s)
    {
        ds4vizValue v;

        ds4vizStackPush(s, 10);
        ds4vizStackPush(s, 20);
        v = ds4vizStackPop(s);
        ok1 = (ds4vizInt(v) == 20);
        ds4vizStackPush(s, 30);
        ds4vizStackPush(s, 40);
        v = ds4vizStackPop(s);
        ok2 = (ds4vizInt(v) == 40);
        v = ds4vizStackPop(s);
        ok3 = (ds4vizInt(v) == 30);
    }
    T_ASSERT(ok1 && ok2 && ok3, "LIFO cycle values");
    p_c = get_output();
    T_VALID(p_c);
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试连续弹出遵循 LIFO 顺序
 */
static void
test_stack_sequential_pop_lifo(void)
{
    char *p_c;
    int results[5];
    bool ok = true;
    int expected[5] = {5, 4, 3, 2, 1};

    setup_test("stack_lifo");
    ds4vizStack(s)
    {
        int i;
        ds4vizValue v;

        for (i = 1; i <= 5; i++)
        {
            ds4vizStackPush(s, i);
        }
        for (i = 0; i < 5; i++)
        {
            v = ds4vizStackPop(s);
            results[i] = (int)ds4vizInt(v);
        }
    }
    {
        int i;
        for (i = 0; i < 5; i++)
        {
            if (results[i] != expected[i])
            {
                ok = false;
            }
        }
    }
    T_ASSERT(ok, "LIFO order 5,4,3,2,1");
    p_c = get_output();
    T_VALID(p_c);
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试无前置步骤时 amend 报错
 */
static void
test_stack_amend_no_prev_error(void)
{
    char *p_c;

    setup_test("stack_amend_noprev");
    ds4vizStack(s)
    {
        ds4vizAmend(s, "no previous step");
    }
    p_c = get_output();
    T_VALID(p_c);
    T_CONTAINS(p_c, "[error]");
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试嵌套 phase, 内层优先
 */
static void
test_stack_nested_phases(void)
{
    char *p_c;

    setup_test("stack_nested_phase");
    ds4vizStack(s)
    {
        ds4vizPhase(s, "outer")
        {
            ds4vizStackPush(s, 10);
            ds4vizPhase(s, "inner")
            {
                ds4vizStackPush(s, 20);
            }
            ds4vizStackPush(s, 30);
        }
    }
    p_c = get_output();
    T_VALID(p_c);
    T_CONTAINS(p_c, "phase = \"inner\"");
    T_CONTAINS(p_c, "phase = \"outer\"");
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试清空后继续操作
 */
static void
test_stack_clear_then_push(void)
{
    char *p_c;
    bool ok = false;

    setup_test("stack_clear_push");
    ds4vizStack(s)
    {
        ds4vizValue v;

        ds4vizStackPush(s, 1);
        ds4vizStackPush(s, 2);
        ds4vizStackClear(s);
        ds4vizStackPush(s, 3);
        v = ds4vizStackPop(s);
        ok = (ds4vizInt(v) == 3);
    }
    T_ASSERT(ok, "Pop after clear+push should return 3");
    p_c = get_output();
    T_VALID(p_c);
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试单次 push 和 pop
 */
static void
test_stack_single_push_pop(void)
{
    char *p_c;
    bool ok = false;

    setup_test("stack_single");
    ds4vizStack(s)
    {
        ds4vizValue v;

        ds4vizStackPush(s, 42);
        v = ds4vizStackPop(s);
        ok = (ds4vizInt(v) == 42);
    }
    T_ASSERT(ok, "Pop should return 42");
    p_c = get_output();
    T_VALID(p_c);
    T_CONTAINS(p_c, "[result]");
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试清空后弹出报错
 */
static void
test_stack_pop_after_clear_error(void)
{
    char *p_c;

    setup_test("stack_pop_clear");
    ds4vizStack(s)
    {
        ds4vizStackPush(s, 1);
        ds4vizStackClear(s);
        ds4vizStackPop(s);
    }
    p_c = get_output();
    T_VALID(p_c);
    T_CONTAINS(p_c, "[error]");
    T_TEARDOWN(p_c);
}

/* ================================================================
 * TestQueueIntegration: 队列集成测试
 * ================================================================ */

/**
 * @brief 测试队列基本操作
 */
static void
test_queue_basic_operations(void)
{
    char *p_c;
    bool ok = false;

    setup_test("queue_basic");
    ds4vizQueue(q)
    {
        ds4vizValue v;

        ds4vizQueueEnqueue(q, 10);
        ds4vizQueueEnqueue(q, 20);
        v = ds4vizQueueDequeue(q);
        ok = (ds4vizInt(v) == 10);
    }
    T_ASSERT(ok, "Dequeue should return 10 (FIFO)");
    p_c = get_output();
    T_VALID(p_c);
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试空队列出队错误
 */
static void
test_queue_empty_dequeue_error(void)
{
    char *p_c;

    setup_test("queue_empty_deq");
    ds4vizQueue(q)
    {
        ds4vizQueueDequeue(q);
    }
    p_c = get_output();
    T_VALID(p_c);
    T_CONTAINS(p_c, "[error]");
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试队列 phase 和 amend
 */
static void
test_queue_phase_and_amend(void)
{
    char *p_c;

    setup_test("queue_phase_amend");
    ds4vizQueue(q)
    {
        ds4vizPhase(q, "enqueue")
        {
            ds4vizQueueEnqueue(q, "A");
            ds4vizQueueEnqueue(q, "B");
        }
        ds4vizPhase(q, "process")
        {
            ds4vizQueueDequeue(q);
            ds4vizAmend(q, "processed A");
        }
    }
    p_c = get_output();
    T_VALID(p_c);
    T_CONTAINS(p_c, "phase = \"enqueue\"");
    T_CONTAINS(p_c, "note = \"processed A\"");
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试队列清空
 */
static void
test_queue_clear(void)
{
    char *p_c;

    setup_test("queue_clear");
    ds4vizQueue(q)
    {
        ds4vizQueueEnqueue(q, 1);
        ds4vizQueueEnqueue(q, 2);
        ds4vizQueueEnqueue(q, 3);
        ds4vizQueueClear(q);
    }
    p_c = get_output();
    T_VALID(p_c);
    T_CONTAINS(p_c, "items = []");
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试队列各种值类型
 */
static void
test_queue_various_types(void)
{
    char *p_c;

    setup_test("queue_types");
    ds4vizQueue(q)
    {
        ds4vizQueueEnqueue(q, 42);
        ds4vizQueueEnqueue(q, 2.718);
        ds4vizQueueEnqueue(q, "world");
        ds4vizQueueEnqueue(q, false);
    }
    p_c = get_output();
    T_VALID(p_c);
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试出队遵循 FIFO 顺序
 */
static void
test_queue_fifo_order(void)
{
    char *p_c;
    bool ok = true;

    setup_test("queue_fifo");
    ds4vizQueue(q)
    {
        ds4vizValue v;

        ds4vizQueueEnqueue(q, 1);
        ds4vizQueueEnqueue(q, 2);
        ds4vizQueueEnqueue(q, 3);
        ds4vizQueueEnqueue(q, 4);
        v = ds4vizQueueDequeue(q);
        if (ds4vizInt(v) != 1)
            ok = false;
        v = ds4vizQueueDequeue(q);
        if (ds4vizInt(v) != 2)
            ok = false;
        v = ds4vizQueueDequeue(q);
        if (ds4vizInt(v) != 3)
            ok = false;
        v = ds4vizQueueDequeue(q);
        if (ds4vizInt(v) != 4)
            ok = false;
    }
    T_ASSERT(ok, "FIFO order 1,2,3,4");
    p_c = get_output();
    T_VALID(p_c);
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试队列 step 与高亮
 */
static void
test_queue_step_with_highlights(void)
{
    char *p_c;

    setup_test("queue_step_hl");
    ds4vizQueue(q)
    {
        ds4vizQueueEnqueue(q, 10);
        ds4vizQueueEnqueue(q, 20);
        ds4vizStep(q, "observe queue",
                   ds4vizItem(0, DS4VIZ_FOCUS, 3),
                   ds4vizItem(1, DS4VIZ_VISITED));
    }
    p_c = get_output();
    T_VALID(p_c);
    T_CONTAINS(p_c, "op = \"observe\"");
    T_CONTAINS(p_c, "note = \"observe queue\"");
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试清空后出队报错
 */
static void
test_queue_dequeue_after_clear_error(void)
{
    char *p_c;

    setup_test("queue_deq_clear");
    ds4vizQueue(q)
    {
        ds4vizQueueEnqueue(q, 1);
        ds4vizQueueClear(q);
        ds4vizQueueDequeue(q);
    }
    p_c = get_output();
    T_VALID(p_c);
    T_CONTAINS(p_c, "[error]");
    T_TEARDOWN(p_c);
}

/* ================================================================
 * TestSingleLinkedListIntegration: 单链表集成测试
 * ================================================================ */

/**
 * @brief 测试单链表基本操作
 */
static void
test_slist_basic_operations(void)
{
    char *p_c;

    setup_test("slist_basic");
    ds4vizSingleLinkedList(l)
    {
        int a;

        a = ds4vizSlInsertHead(l, 10);
        ds4vizAlias(l, a, "head");
        ds4vizSlInsertTail(l, 20);
        ds4vizSlInsertAfter(l, a, 15);
        ds4vizSlDeleteHead(l);
    }
    p_c = get_output();
    T_VALID(p_c);
    T_CONTAINS(p_c, "kind = \"slist\"");
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试单链表反转
 */
static void
test_slist_reverse(void)
{
    char *p_c;

    setup_test("slist_reverse");
    ds4vizSingleLinkedList(l)
    {
        ds4vizSlInsertTail(l, 1);
        ds4vizSlInsertTail(l, 2);
        ds4vizSlInsertTail(l, 3);
        ds4vizSlReverse(l);
    }
    p_c = get_output();
    T_VALID(p_c);
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试删除不存在的节点
 */
static void
test_slist_delete_nonexistent_error(void)
{
    char *p_c;

    setup_test("slist_del_ne");
    ds4vizSingleLinkedList(l)
    {
        ds4vizSlInsertHead(l, 10);
        ds4vizSlDelete(l, 999);
    }
    p_c = get_output();
    T_VALID(p_c);
    T_CONTAINS(p_c, "[error]");
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试别名和 step
 */
static void
test_slist_alias_and_step(void)
{
    char *p_c;

    setup_test("slist_alias_step");
    ds4vizSingleLinkedList(l)
    {
        int a;
        int b;

        a = ds4vizSlInsertHead(l, 10);
        b = ds4vizSlInsertTail(l, 20);
        ds4vizAlias(l, a, "start");
        ds4vizStep(l, "traverse begin",
                   ds4vizNode(a, DS4VIZ_FOCUS, 3));
        ds4vizStep(l, "traverse end",
                   ds4vizNode(a, DS4VIZ_VISITED),
                   ds4vizNode(b, DS4VIZ_FOCUS, 3));
    }
    p_c = get_output();
    T_VALID(p_c);
    T_CONTAINS(p_c, "alias = \"start\"");
    T_CONTAINS(p_c, "op = \"observe\"");
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试删除唯一节点的头
 */
static void
test_slist_delete_head_single(void)
{
    char *p_c;

    setup_test("slist_del_head_single");
    ds4vizSingleLinkedList(l)
    {
        ds4vizSlInsertHead(l, 10);
        ds4vizSlDeleteHead(l);
    }
    p_c = get_output();
    T_VALID(p_c);
    T_CONTAINS(p_c, "head = -1");
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试在不存在的节点后插入
 */
static void
test_slist_insert_after_nonexistent_error(void)
{
    char *p_c;

    setup_test("slist_ia_ne");
    ds4vizSingleLinkedList(l)
    {
        ds4vizSlInsertHead(l, 10);
        ds4vizSlInsertAfter(l, 999, 20);
    }
    p_c = get_output();
    T_VALID(p_c);
    T_CONTAINS(p_c, "[error]");
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试空链表删除头
 */
static void
test_slist_delete_head_empty_error(void)
{
    char *p_c;

    setup_test("slist_dh_empty");
    ds4vizSingleLinkedList(l)
    {
        ds4vizSlDeleteHead(l);
    }
    p_c = get_output();
    T_VALID(p_c);
    T_CONTAINS(p_c, "[error]");
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试多次头插入
 */
static void
test_slist_multiple_head_inserts(void)
{
    char *p_c;

    setup_test("slist_multi_head");
    ds4vizSingleLinkedList(l)
    {
        int a;
        int b;
        int c;

        a = ds4vizSlInsertHead(l, 30);
        b = ds4vizSlInsertHead(l, 20);
        c = ds4vizSlInsertHead(l, 10);
        ds4vizStep(l, "check order",
                   ds4vizNode(c, DS4VIZ_FOCUS, 3),
                   ds4vizNode(b, DS4VIZ_VISITED),
                   ds4vizNode(a, DS4VIZ_VISITED));
    }
    p_c = get_output();
    T_VALID(p_c);
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试单节点反转
 */
static void
test_slist_reverse_single(void)
{
    char *p_c;

    setup_test("slist_rev_single");
    ds4vizSingleLinkedList(l)
    {
        ds4vizSlInsertHead(l, 42);
        ds4vizSlReverse(l);
    }
    p_c = get_output();
    T_VALID(p_c);
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试空链表反转
 */
static void
test_slist_reverse_empty(void)
{
    char *p_c;

    setup_test("slist_rev_empty");
    ds4vizSingleLinkedList(l)
    {
        ds4vizSlReverse(l);
    }
    p_c = get_output();
    T_VALID(p_c);
    T_TEARDOWN(p_c);
}

/* ================================================================
 * TestDoubleLinkedListIntegration: 双向链表集成测试
 * ================================================================ */

/**
 * @brief 测试双向链表基本操作
 */
static void
test_dlist_basic_operations(void)
{
    char *p_c;

    setup_test("dlist_basic");
    ds4vizDoubleLinkedList(l)
    {
        int a;
        int b;

        a = ds4vizDlInsertHead(l, 10);
        b = ds4vizDlInsertTail(l, 30);
        ds4vizDlInsertBefore(l, b, 20);
        ds4vizDlInsertAfter(l, a, 15);
        ds4vizDlDeleteTail(l);
    }
    p_c = get_output();
    T_VALID(p_c);
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试双向链表反转
 */
static void
test_dlist_reverse(void)
{
    char *p_c;

    setup_test("dlist_reverse");
    ds4vizDoubleLinkedList(l)
    {
        ds4vizDlInsertTail(l, 1);
        ds4vizDlInsertTail(l, 2);
        ds4vizDlInsertTail(l, 3);
        ds4vizDlReverse(l);
    }
    p_c = get_output();
    T_VALID(p_c);
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试空双向链表删除头报错
 */
static void
test_dlist_delete_head_empty_error(void)
{
    char *p_c;

    setup_test("dlist_dh_empty");
    ds4vizDoubleLinkedList(l)
    {
        ds4vizDlDeleteHead(l);
    }
    p_c = get_output();
    T_VALID(p_c);
    T_CONTAINS(p_c, "[error]");
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试删除中间节点
 */
static void
test_dlist_delete_specific_node(void)
{
    char *p_c;

    setup_test("dlist_del_mid");
    ds4vizDoubleLinkedList(l)
    {
        int b;

        ds4vizDlInsertTail(l, 10);
        b = ds4vizDlInsertTail(l, 20);
        ds4vizDlInsertTail(l, 30);
        ds4vizDlDelete(l, b);
    }
    p_c = get_output();
    T_VALID(p_c);
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试在不存在的节点前插入
 */
static void
test_dlist_insert_before_nonexistent_error(void)
{
    char *p_c;

    setup_test("dlist_ib_ne");
    ds4vizDoubleLinkedList(l)
    {
        ds4vizDlInsertTail(l, 10);
        ds4vizDlInsertBefore(l, 999, 20);
    }
    p_c = get_output();
    T_VALID(p_c);
    T_CONTAINS(p_c, "[error]");
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试双向链表别名和 step
 */
static void
test_dlist_alias_and_step(void)
{
    char *p_c;

    setup_test("dlist_alias_step");
    ds4vizDoubleLinkedList(l)
    {
        int a;
        int b;

        a = ds4vizDlInsertHead(l, 10);
        b = ds4vizDlInsertTail(l, 20);
        ds4vizAlias(l, a, "head");
        ds4vizAlias(l, b, "tail");
        ds4vizStep(l, "view dlist",
                   ds4vizNode(a, DS4VIZ_FOCUS, 3),
                   ds4vizNode(b, DS4VIZ_ACTIVE));
    }
    p_c = get_output();
    T_VALID(p_c);
    T_CONTAINS(p_c, "alias = \"head\"");
    T_CONTAINS(p_c, "alias = \"tail\"");
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试单节点时删除头
 */
static void
test_dlist_delete_head_single(void)
{
    char *p_c;

    setup_test("dlist_dh_single");
    ds4vizDoubleLinkedList(l)
    {
        ds4vizDlInsertHead(l, 10);
        ds4vizDlDeleteHead(l);
    }
    p_c = get_output();
    T_VALID(p_c);
    T_CONTAINS(p_c, "head = -1");
    T_CONTAINS(p_c, "tail = -1");
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试空链表删除尾
 */
static void
test_dlist_delete_tail_empty_error(void)
{
    char *p_c;

    setup_test("dlist_dt_empty");
    ds4vizDoubleLinkedList(l)
    {
        ds4vizDlDeleteTail(l);
    }
    p_c = get_output();
    T_VALID(p_c);
    T_CONTAINS(p_c, "[error]");
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试删除不存在的节点
 */
static void
test_dlist_delete_nonexistent_error(void)
{
    char *p_c;

    setup_test("dlist_del_ne");
    ds4vizDoubleLinkedList(l)
    {
        ds4vizDlInsertHead(l, 10);
        ds4vizDlDelete(l, 999);
    }
    p_c = get_output();
    T_VALID(p_c);
    T_CONTAINS(p_c, "[error]");
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试在不存在的节点后插入
 */
static void
test_dlist_insert_after_nonexistent_error(void)
{
    char *p_c;

    setup_test("dlist_ia_ne");
    ds4vizDoubleLinkedList(l)
    {
        ds4vizDlInsertHead(l, 10);
        ds4vizDlInsertAfter(l, 999, 20);
    }
    p_c = get_output();
    T_VALID(p_c);
    T_CONTAINS(p_c, "[error]");
    T_TEARDOWN(p_c);
}

/* ================================================================
 * TestBinaryTreeIntegration: 二叉树集成测试
 * ================================================================ */

/**
 * @brief 测试二叉树基本操作
 */
static void
test_bt_basic_operations(void)
{
    char *p_c;

    setup_test("bt_basic");
    ds4vizBinaryTree(t)
    {
        int root;
        int left;

        root = ds4vizBtInsertRoot(t, 10);
        ds4vizAlias(t, root, "root");
        left = ds4vizBtInsertLeft(t, root, 5);
        ds4vizBtInsertRight(t, root, 15);
        ds4vizBtInsertLeft(t, left, 3);
    }
    p_c = get_output();
    T_VALID(p_c);
    T_CONTAINS(p_c, "kind = \"binary_tree\"");
    T_CONTAINS(p_c, "alias = \"root\"");
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试删除子树
 */
static void
test_bt_delete_subtree(void)
{
    char *p_c;

    setup_test("bt_del_sub");
    ds4vizBinaryTree(t)
    {
        int root;
        int left;

        root = ds4vizBtInsertRoot(t, 10);
        left = ds4vizBtInsertLeft(t, root, 5);
        ds4vizBtInsertLeft(t, left, 3);
        ds4vizBtDelete(t, left);
    }
    p_c = get_output();
    T_VALID(p_c);
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试更新值
 */
static void
test_bt_update_value(void)
{
    char *p_c;

    setup_test("bt_update");
    ds4vizBinaryTree(t)
    {
        int root;

        root = ds4vizBtInsertRoot(t, 10);
        ds4vizBtUpdateValue(t, root, 100);
    }
    p_c = get_output();
    T_VALID(p_c);
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试重复插入根节点
 */
static void
test_bt_duplicate_root_error(void)
{
    char *p_c;

    setup_test("bt_dup_root");
    ds4vizBinaryTree(t)
    {
        ds4vizBtInsertRoot(t, 10);
        ds4vizBtInsertRoot(t, 20);
    }
    p_c = get_output();
    T_VALID(p_c);
    T_CONTAINS(p_c, "[error]");
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试使用 step 模拟遍历
 */
static void
test_bt_step_traversal(void)
{
    char *p_c;

    setup_test("bt_traversal");
    ds4vizBinaryTree(t)
    {
        int root;
        int left;
        int right;

        root = ds4vizBtInsertRoot(t, 10);
        left = ds4vizBtInsertLeft(t, root, 5);
        right = ds4vizBtInsertRight(t, root, 15);
        ds4vizPhase(t, "preorder")
        {
            ds4vizStep(t, "visit 10",
                       ds4vizNode(root, DS4VIZ_FOCUS, 3));
            ds4vizStep(t, "visit 5",
                       ds4vizNode(root, DS4VIZ_VISITED),
                       ds4vizNode(left, DS4VIZ_FOCUS, 3));
            ds4vizStep(t, "visit 15",
                       ds4vizNode(root, DS4VIZ_VISITED),
                       ds4vizNode(left, DS4VIZ_VISITED),
                       ds4vizNode(right, DS4VIZ_FOCUS, 3));
        }
    }
    p_c = get_output();
    T_VALID(p_c);
    T_CONTAINS(p_c, "phase = \"preorder\"");
    T_INT_EQ(str_count(p_c, "op = \"observe\""), 3);
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试删除根节点及其所有子树
 */
static void
test_bt_delete_root(void)
{
    char *p_c;

    setup_test("bt_del_root");
    ds4vizBinaryTree(t)
    {
        int root;

        root = ds4vizBtInsertRoot(t, 10);
        ds4vizBtInsertLeft(t, root, 5);
        ds4vizBtInsertRight(t, root, 15);
        ds4vizBtDelete(t, root);
    }
    p_c = get_output();
    T_VALID(p_c);
    T_CONTAINS(p_c, "root = -1");
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试删除叶节点
 */
static void
test_bt_delete_leaf(void)
{
    char *p_c;

    setup_test("bt_del_leaf");
    ds4vizBinaryTree(t)
    {
        int root;
        int left;

        root = ds4vizBtInsertRoot(t, 10);
        left = ds4vizBtInsertLeft(t, root, 5);
        ds4vizBtInsertRight(t, root, 15);
        ds4vizBtDelete(t, left);
    }
    p_c = get_output();
    T_VALID(p_c);
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试左子节点已存在时插入报错
 */
static void
test_bt_insert_left_occupied_error(void)
{
    char *p_c;

    setup_test("bt_il_occ");
    ds4vizBinaryTree(t)
    {
        int root;

        root = ds4vizBtInsertRoot(t, 10);
        ds4vizBtInsertLeft(t, root, 5);
        ds4vizBtInsertLeft(t, root, 3);
    }
    p_c = get_output();
    T_VALID(p_c);
    T_CONTAINS(p_c, "[error]");
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试父节点不存在时插入报错
 */
static void
test_bt_insert_nonexistent_parent_error(void)
{
    char *p_c;

    setup_test("bt_ip_ne");
    ds4vizBinaryTree(t)
    {
        ds4vizBtInsertRoot(t, 10);
        ds4vizBtInsertLeft(t, 999, 5);
    }
    p_c = get_output();
    T_VALID(p_c);
    T_CONTAINS(p_c, "[error]");
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试更新不存在的节点报错
 */
static void
test_bt_update_nonexistent_error(void)
{
    char *p_c;

    setup_test("bt_upd_ne");
    ds4vizBinaryTree(t)
    {
        ds4vizBtInsertRoot(t, 10);
        ds4vizBtUpdateValue(t, 999, 100);
    }
    p_c = get_output();
    T_VALID(p_c);
    T_CONTAINS(p_c, "[error]");
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试右子节点已存在时插入报错
 */
static void
test_bt_insert_right_occupied_error(void)
{
    char *p_c;

    setup_test("bt_ir_occ");
    ds4vizBinaryTree(t)
    {
        int root;

        root = ds4vizBtInsertRoot(t, 10);
        ds4vizBtInsertRight(t, root, 15);
        ds4vizBtInsertRight(t, root, 20);
    }
    p_c = get_output();
    T_VALID(p_c);
    T_CONTAINS(p_c, "[error]");
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试删除不存在的节点报错
 */
static void
test_bt_delete_nonexistent_error(void)
{
    char *p_c;

    setup_test("bt_del_ne");
    ds4vizBinaryTree(t)
    {
        ds4vizBtInsertRoot(t, 10);
        ds4vizBtDelete(t, 999);
    }
    p_c = get_output();
    T_VALID(p_c);
    T_CONTAINS(p_c, "[error]");
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试字符串值节点
 */
static void
test_bt_string_values(void)
{
    char *p_c;

    setup_test("bt_string");
    ds4vizBinaryTree(t)
    {
        int root;

        root = ds4vizBtInsertRoot(t, "A");
        ds4vizBtInsertLeft(t, root, "B");
        ds4vizBtInsertRight(t, root, "C");
    }
    p_c = get_output();
    T_VALID(p_c);
    T_TEARDOWN(p_c);
}

/* ================================================================
 * TestBinarySearchTreeIntegration: BST 集成测试
 * ================================================================ */

/**
 * @brief 测试 BST 基本操作
 */
static void
test_bst_basic_operations(void)
{
    char *p_c;

    setup_test("bst_basic");
    ds4vizBinarySearchTree(b)
    {
        ds4vizBstInsert(b, 10);
        ds4vizBstInsert(b, 5);
        ds4vizBstInsert(b, 15);
        ds4vizBstInsert(b, 3);
        ds4vizBstInsert(b, 7);
    }
    p_c = get_output();
    T_VALID(p_c);
    T_CONTAINS(p_c, "kind = \"bst\"");
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试删除叶节点
 */
static void
test_bst_delete_leaf(void)
{
    char *p_c;

    setup_test("bst_del_leaf");
    ds4vizBinarySearchTree(b)
    {
        ds4vizBstInsert(b, 10);
        ds4vizBstInsert(b, 5);
        ds4vizBstInsert(b, 15);
        ds4vizBstDelete(b, 5);
    }
    p_c = get_output();
    T_VALID(p_c);
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试删除有两个子节点的节点
 */
static void
test_bst_delete_two_children(void)
{
    char *p_c;

    setup_test("bst_del_2c");
    ds4vizBinarySearchTree(b)
    {
        ds4vizBstInsert(b, 10);
        ds4vizBstInsert(b, 5);
        ds4vizBstInsert(b, 15);
        ds4vizBstInsert(b, 3);
        ds4vizBstInsert(b, 7);
        ds4vizBstDelete(b, 5);
    }
    p_c = get_output();
    T_VALID(p_c);
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试删除不存在的值
 */
static void
test_bst_delete_nonexistent_error(void)
{
    char *p_c;

    setup_test("bst_del_ne");
    ds4vizBinarySearchTree(b)
    {
        ds4vizBstInsert(b, 10);
        ds4vizBstDelete(b, 999);
    }
    p_c = get_output();
    T_VALID(p_c);
    T_CONTAINS(p_c, "[error]");
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试删除根节点
 */
static void
test_bst_delete_root(void)
{
    char *p_c;

    setup_test("bst_del_root");
    ds4vizBinarySearchTree(b)
    {
        ds4vizBstInsert(b, 10);
        ds4vizBstInsert(b, 5);
        ds4vizBstInsert(b, 15);
        ds4vizBstDelete(b, 10);
    }
    p_c = get_output();
    T_VALID(p_c);
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试删除只有一个子节点的节点
 */
static void
test_bst_delete_one_child(void)
{
    char *p_c;

    setup_test("bst_del_1c");
    ds4vizBinarySearchTree(b)
    {
        ds4vizBstInsert(b, 10);
        ds4vizBstInsert(b, 5);
        ds4vizBstInsert(b, 3);
        ds4vizBstDelete(b, 5);
    }
    p_c = get_output();
    T_VALID(p_c);
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试 BST phase 与 alias
 */
static void
test_bst_phase_and_alias(void)
{
    char *p_c;

    setup_test("bst_phase_alias");
    ds4vizBinarySearchTree(b)
    {
        int n10;

        ds4vizPhase(b, "insert")
        {
            n10 = ds4vizBstInsert(b, 10);
            ds4vizAlias(b, n10, "root");
            ds4vizBstInsert(b, 5);
            ds4vizBstInsert(b, 15);
        }
        ds4vizPhase(b, "search")
        {
            ds4vizStep(b, "search 5",
                       ds4vizNode(n10, DS4VIZ_COMPARING, 3));
        }
    }
    p_c = get_output();
    T_VALID(p_c);
    T_CONTAINS(p_c, "phase = \"insert\"");
    T_CONTAINS(p_c, "phase = \"search\"");
    T_CONTAINS(p_c, "alias = \"root\"");
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试顺序插入产生退化树
 */
static void
test_bst_sequential_insert(void)
{
    char *p_c;

    setup_test("bst_seq");
    ds4vizBinarySearchTree(b)
    {
        int i;
        for (i = 1; i <= 5; i++)
        {
            ds4vizBstInsert(b, i);
        }
    }
    p_c = get_output();
    T_VALID(p_c);
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试依次删除所有节点
 */
static void
test_bst_delete_all(void)
{
    char *p_c;

    setup_test("bst_del_all");
    ds4vizBinarySearchTree(b)
    {
        ds4vizBstInsert(b, 10);
        ds4vizBstInsert(b, 5);
        ds4vizBstInsert(b, 15);
        ds4vizBstDelete(b, 5);
        ds4vizBstDelete(b, 15);
        ds4vizBstDelete(b, 10);
    }
    p_c = get_output();
    T_VALID(p_c);
    T_CONTAINS(p_c, "root = -1");
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试降序插入
 */
static void
test_bst_insert_descending(void)
{
    char *p_c;

    setup_test("bst_desc");
    ds4vizBinarySearchTree(b)
    {
        int i;
        for (i = 5; i >= 1; i--)
        {
            ds4vizBstInsert(b, i);
        }
    }
    p_c = get_output();
    T_VALID(p_c);
    T_TEARDOWN(p_c);
}

/* ================================================================
 * TestGraphUndirectedIntegration: 无向图集成测试
 * ================================================================ */

/**
 * @brief 测试无向图基本操作
 */
static void
test_gu_basic(void)
{
    char *p_c;

    setup_test("gu_basic");
    ds4vizGraphUndirected(g)
    {
        ds4vizGuAddNode(g, 0, "A");
        ds4vizGuAddNode(g, 1, "B");
        ds4vizGuAddEdge(g, 0, 1);
    }
    p_c = get_output();
    T_VALID(p_c);
    T_CONTAINS(p_c, "kind = \"graph_undirected\"");
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试无向边规范化
 */
static void
test_gu_edge_normalization(void)
{
    char *p_c;

    setup_test("gu_norm");
    ds4vizGraphUndirected(g)
    {
        ds4vizGuAddNode(g, 0, "A");
        ds4vizGuAddNode(g, 1, "B");
        ds4vizGuAddEdge(g, 1, 0);
    }
    p_c = get_output();
    T_VALID(p_c);
    T_CONTAINS(p_c, "from = 0");
    T_CONTAINS(p_c, "to = 1");
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试自环错误
 */
static void
test_gu_self_loop_error(void)
{
    char *p_c;

    setup_test("gu_selfloop");
    ds4vizGraphUndirected(g)
    {
        ds4vizGuAddNode(g, 0, "A");
        ds4vizGuAddEdge(g, 0, 0);
    }
    p_c = get_output();
    T_VALID(p_c);
    T_CONTAINS(p_c, "[error]");
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试重复边错误
 */
static void
test_gu_duplicate_edge_error(void)
{
    char *p_c;

    setup_test("gu_dup_edge");
    ds4vizGraphUndirected(g)
    {
        ds4vizGuAddNode(g, 0, "A");
        ds4vizGuAddNode(g, 1, "B");
        ds4vizGuAddEdge(g, 0, 1);
        ds4vizGuAddEdge(g, 0, 1);
    }
    p_c = get_output();
    T_VALID(p_c);
    T_CONTAINS(p_c, "[error]");
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试删除节点同时删除边
 */
static void
test_gu_remove_node_and_edges(void)
{
    char *p_c;

    setup_test("gu_rm_node");
    ds4vizGraphUndirected(g)
    {
        ds4vizGuAddNode(g, 0, "A");
        ds4vizGuAddNode(g, 1, "B");
        ds4vizGuAddNode(g, 2, "C");
        ds4vizGuAddEdge(g, 0, 1);
        ds4vizGuAddEdge(g, 1, 2);
        ds4vizGuRemoveNode(g, 1);
    }
    p_c = get_output();
    T_VALID(p_c);
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试边高亮
 */
static void
test_gu_step_edge_highlight(void)
{
    char *p_c;

    setup_test("gu_edge_hl");
    ds4vizGraphUndirected(g)
    {
        ds4vizGuAddNode(g, 0, "A");
        ds4vizGuAddNode(g, 1, "B");
        ds4vizGuAddEdge(g, 0, 1);
        ds4vizStep(g, NULL,
                   ds4vizNode(0, DS4VIZ_VISITED),
                   ds4vizNode(1, DS4VIZ_FOCUS, 3),
                   ds4vizEdge(0, 1, DS4VIZ_ACTIVE));
    }
    p_c = get_output();
    T_VALID(p_c);
    T_CONTAINS(p_c, "kind = \"edge\"");
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试删除边
 */
static void
test_gu_remove_edge(void)
{
    char *p_c;

    setup_test("gu_rm_edge");
    ds4vizGraphUndirected(g)
    {
        ds4vizGuAddNode(g, 0, "A");
        ds4vizGuAddNode(g, 1, "B");
        ds4vizGuAddNode(g, 2, "C");
        ds4vizGuAddEdge(g, 0, 1);
        ds4vizGuAddEdge(g, 0, 2);
        ds4vizGuRemoveEdge(g, 0, 1);
    }
    p_c = get_output();
    T_VALID(p_c);
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试更新节点标签
 */
static void
test_gu_update_node_label(void)
{
    char *p_c;

    setup_test("gu_upd_lbl");
    ds4vizGraphUndirected(g)
    {
        ds4vizGuAddNode(g, 0, "A");
        ds4vizGuUpdateNodeLabel(g, 0, "A-updated");
    }
    p_c = get_output();
    T_VALID(p_c);
    T_CONTAINS(p_c, "label = \"A-updated\"");
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试删除不存在的节点
 */
static void
test_gu_remove_nonexistent_node_error(void)
{
    char *p_c;

    setup_test("gu_rm_ne_node");
    ds4vizGraphUndirected(g)
    {
        ds4vizGuAddNode(g, 0, "A");
        ds4vizGuRemoveNode(g, 999);
    }
    p_c = get_output();
    T_VALID(p_c);
    T_CONTAINS(p_c, "[error]");
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试添加重复节点
 */
static void
test_gu_add_duplicate_node_error(void)
{
    char *p_c;

    setup_test("gu_dup_node");
    ds4vizGraphUndirected(g)
    {
        ds4vizGuAddNode(g, 0, "A");
        ds4vizGuAddNode(g, 0, "B");
    }
    p_c = get_output();
    T_VALID(p_c);
    T_CONTAINS(p_c, "[error]");
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试在不存在的节点间添加边
 */
static void
test_gu_add_edge_nonexistent_error(void)
{
    char *p_c;

    setup_test("gu_ae_ne");
    ds4vizGraphUndirected(g)
    {
        ds4vizGuAddNode(g, 0, "A");
        ds4vizGuAddEdge(g, 0, 1);
    }
    p_c = get_output();
    T_VALID(p_c);
    T_CONTAINS(p_c, "[error]");
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试反向添加重复边
 */
static void
test_gu_duplicate_edge_reversed_error(void)
{
    char *p_c;

    setup_test("gu_dup_rev");
    ds4vizGraphUndirected(g)
    {
        ds4vizGuAddNode(g, 0, "A");
        ds4vizGuAddNode(g, 1, "B");
        ds4vizGuAddEdge(g, 0, 1);
        ds4vizGuAddEdge(g, 1, 0);
    }
    p_c = get_output();
    T_VALID(p_c);
    T_CONTAINS(p_c, "[error]");
    T_TEARDOWN(p_c);
}

/* ================================================================
 * TestGraphDirectedIntegration: 有向图集成测试
 * ================================================================ */

/**
 * @brief 测试有向图基本操作
 */
static void
test_gd_basic(void)
{
    char *p_c;

    setup_test("gd_basic");
    ds4vizGraphDirected(g)
    {
        ds4vizGdAddNode(g, 0, "A");
        ds4vizGdAddNode(g, 1, "B");
        ds4vizGdAddEdge(g, 0, 1);
        ds4vizGdAddEdge(g, 1, 0);
    }
    p_c = get_output();
    T_VALID(p_c);
    T_CONTAINS(p_c, "kind = \"graph_directed\"");
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试有向图不规范化
 */
static void
test_gd_no_normalization(void)
{
    char *p_c;

    setup_test("gd_no_norm");
    ds4vizGraphDirected(g)
    {
        ds4vizGdAddNode(g, 0, "A");
        ds4vizGdAddNode(g, 1, "B");
        ds4vizGdAddEdge(g, 1, 0);
    }
    p_c = get_output();
    T_VALID(p_c);
    T_CONTAINS(p_c, "from = 1");
    T_CONTAINS(p_c, "to = 0");
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试有向图自环错误
 */
static void
test_gd_self_loop_error(void)
{
    char *p_c;

    setup_test("gd_selfloop");
    ds4vizGraphDirected(g)
    {
        ds4vizGdAddNode(g, 0, "A");
        ds4vizGdAddEdge(g, 0, 0);
    }
    p_c = get_output();
    T_VALID(p_c);
    T_CONTAINS(p_c, "[error]");
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试删除节点同时删除关联入边与出边
 */
static void
test_gd_remove_node_and_edges(void)
{
    char *p_c;

    setup_test("gd_rm_node");
    ds4vizGraphDirected(g)
    {
        ds4vizGdAddNode(g, 0, "A");
        ds4vizGdAddNode(g, 1, "B");
        ds4vizGdAddNode(g, 2, "C");
        ds4vizGdAddEdge(g, 0, 1);
        ds4vizGdAddEdge(g, 1, 2);
        ds4vizGdAddEdge(g, 2, 1);
        ds4vizGdRemoveNode(g, 1);
    }
    p_c = get_output();
    T_VALID(p_c);
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试删除有向边
 */
static void
test_gd_remove_edge(void)
{
    char *p_c;

    setup_test("gd_rm_edge");
    ds4vizGraphDirected(g)
    {
        ds4vizGdAddNode(g, 0, "A");
        ds4vizGdAddNode(g, 1, "B");
        ds4vizGdAddEdge(g, 0, 1);
        ds4vizGdAddEdge(g, 1, 0);
        ds4vizGdRemoveEdge(g, 0, 1);
    }
    p_c = get_output();
    T_VALID(p_c);
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试重复有向边错误
 */
static void
test_gd_duplicate_edge_error(void)
{
    char *p_c;

    setup_test("gd_dup_edge");
    ds4vizGraphDirected(g)
    {
        ds4vizGdAddNode(g, 0, "A");
        ds4vizGdAddNode(g, 1, "B");
        ds4vizGdAddEdge(g, 0, 1);
        ds4vizGdAddEdge(g, 0, 1);
    }
    p_c = get_output();
    T_VALID(p_c);
    T_CONTAINS(p_c, "[error]");
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试添加重复节点
 */
static void
test_gd_add_duplicate_node_error(void)
{
    char *p_c;

    setup_test("gd_dup_node");
    ds4vizGraphDirected(g)
    {
        ds4vizGdAddNode(g, 0, "A");
        ds4vizGdAddNode(g, 0, "A");
    }
    p_c = get_output();
    T_VALID(p_c);
    T_CONTAINS(p_c, "[error]");
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试更新有向图节点标签
 */
static void
test_gd_update_node_label(void)
{
    char *p_c;

    setup_test("gd_upd_lbl");
    ds4vizGraphDirected(g)
    {
        ds4vizGdAddNode(g, 0, "X");
        ds4vizGdUpdateNodeLabel(g, 0, "X-mod");
    }
    p_c = get_output();
    T_VALID(p_c);
    T_CONTAINS(p_c, "label = \"X-mod\"");
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试在不存在的节点间添加有向边
 */
static void
test_gd_add_edge_nonexistent_error(void)
{
    char *p_c;

    setup_test("gd_ae_ne");
    ds4vizGraphDirected(g)
    {
        ds4vizGdAddNode(g, 0, "A");
        ds4vizGdAddEdge(g, 0, 1);
    }
    p_c = get_output();
    T_VALID(p_c);
    T_CONTAINS(p_c, "[error]");
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试删除不存在的有向边
 */
static void
test_gd_remove_nonexistent_edge_error(void)
{
    char *p_c;

    setup_test("gd_re_ne");
    ds4vizGraphDirected(g)
    {
        ds4vizGdAddNode(g, 0, "A");
        ds4vizGdAddNode(g, 1, "B");
        ds4vizGdRemoveEdge(g, 0, 1);
    }
    p_c = get_output();
    T_VALID(p_c);
    T_CONTAINS(p_c, "[error]");
    T_TEARDOWN(p_c);
}

/* ================================================================
 * TestGraphWeightedIntegration: 带权图集成测试
 * ================================================================ */

/**
 * @brief 测试带权图基本操作
 */
static void
test_gw_basic(void)
{
    char *p_c;

    setup_test("gw_basic");
    ds4vizGraphWeighted(g)
    {
        ds4vizGwAddNode(g, 0, "A");
        ds4vizGwAddNode(g, 1, "B");
        ds4vizGwAddEdge(g, 0, 1, 3.5);
    }
    p_c = get_output();
    T_VALID(p_c);
    T_CONTAINS(p_c, "kind = \"graph_weighted\"");
    T_CONTAINS(p_c, "weight = 3.5");
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试更新权重
 */
static void
test_gw_update_weight(void)
{
    char *p_c;

    setup_test("gw_upd_w");
    ds4vizGraphWeighted(g)
    {
        ds4vizGwAddNode(g, 0, "A");
        ds4vizGwAddNode(g, 1, "B");
        ds4vizGwAddEdge(g, 0, 1, 1.0);
        ds4vizGwUpdateWeight(g, 0, 1, 5.0);
    }
    p_c = get_output();
    T_VALID(p_c);
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试别名和高亮
 */
static void
test_gw_alias_and_highlights(void)
{
    char *p_c;

    setup_test("gw_alias_hl");
    ds4vizGraphWeighted(g)
    {
        ds4vizGwAddNode(g, 0, "src");
        ds4vizAlias(g, 0, "source");
        ds4vizGwAddNode(g, 1, "dst");
        ds4vizGwAddEdge(g, 0, 1, 2.0);
        ds4vizAmend(g, NULL,
                    ds4vizNode(0, DS4VIZ_ACTIVE),
                    ds4vizNode(1, DS4VIZ_ACTIVE),
                    ds4vizEdge(0, 1, DS4VIZ_FOCUS, 3));
    }
    p_c = get_output();
    T_VALID(p_c);
    T_CONTAINS(p_c, "alias = \"source\"");
    T_CONTAINS(p_c, "highlights");
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试删除节点
 */
static void
test_gw_remove_node(void)
{
    char *p_c;

    setup_test("gw_rm_node");
    ds4vizGraphWeighted(g)
    {
        ds4vizGwAddNode(g, 0, "A");
        ds4vizGwAddNode(g, 1, "B");
        ds4vizGwAddEdge(g, 0, 1, 1.0);
        ds4vizGwRemoveNode(g, 1);
    }
    p_c = get_output();
    T_VALID(p_c);
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试更新不存在的边
 */
static void
test_gw_update_nonexistent_edge_error(void)
{
    char *p_c;

    setup_test("gw_upd_ne");
    ds4vizGraphWeighted(g)
    {
        ds4vizGwAddNode(g, 0, "A");
        ds4vizGwAddNode(g, 1, "B");
        ds4vizGwUpdateWeight(g, 0, 1, 5.0);
    }
    p_c = get_output();
    T_VALID(p_c);
    T_CONTAINS(p_c, "[error]");
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试负权重
 */
static void
test_gw_negative_weight(void)
{
    char *p_c;

    setup_test("gw_neg_w");
    ds4vizGraphWeighted(g)
    {
        ds4vizGwAddNode(g, 0, "A");
        ds4vizGwAddNode(g, 1, "B");
        ds4vizGwAddEdge(g, 0, 1, -5.5);
    }
    p_c = get_output();
    T_VALID(p_c);
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试带权图自环错误
 */
static void
test_gw_self_loop_error(void)
{
    char *p_c;

    setup_test("gw_selfloop");
    ds4vizGraphWeighted(g)
    {
        ds4vizGwAddNode(g, 0, "A");
        ds4vizGwAddEdge(g, 0, 0, 1.0);
    }
    p_c = get_output();
    T_VALID(p_c);
    T_CONTAINS(p_c, "[error]");
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试重复边错误
 */
static void
test_gw_duplicate_edge_error(void)
{
    char *p_c;

    setup_test("gw_dup_edge");
    ds4vizGraphWeighted(g)
    {
        ds4vizGwAddNode(g, 0, "A");
        ds4vizGwAddNode(g, 1, "B");
        ds4vizGwAddEdge(g, 0, 1, 1.0);
        ds4vizGwAddEdge(g, 0, 1, 2.0);
    }
    p_c = get_output();
    T_VALID(p_c);
    T_CONTAINS(p_c, "[error]");
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试添加重复节点
 */
static void
test_gw_add_duplicate_node_error(void)
{
    char *p_c;

    setup_test("gw_dup_node");
    ds4vizGraphWeighted(g)
    {
        ds4vizGwAddNode(g, 0, "A");
        ds4vizGwAddNode(g, 0, "B");
    }
    p_c = get_output();
    T_VALID(p_c);
    T_CONTAINS(p_c, "[error]");
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试更新节点标签
 */
static void
test_gw_update_node_label(void)
{
    char *p_c;

    setup_test("gw_upd_lbl");
    ds4vizGraphWeighted(g)
    {
        ds4vizGwAddNode(g, 0, "Old");
        ds4vizGwUpdateNodeLabel(g, 0, "New");
    }
    p_c = get_output();
    T_VALID(p_c);
    T_CONTAINS(p_c, "label = \"New\"");
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试整数权重
 */
static void
test_gw_integer_weight(void)
{
    char *p_c;

    setup_test("gw_int_w");
    ds4vizGraphWeighted(g)
    {
        ds4vizGwAddNode(g, 0, "A");
        ds4vizGwAddNode(g, 1, "B");
        ds4vizGwAddEdge(g, 0, 1, 7);
    }
    p_c = get_output();
    T_VALID(p_c);
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试删除带权边
 */
static void
test_gw_remove_edge(void)
{
    char *p_c;

    setup_test("gw_rm_edge");
    ds4vizGraphWeighted(g)
    {
        ds4vizGwAddNode(g, 0, "A");
        ds4vizGwAddNode(g, 1, "B");
        ds4vizGwAddNode(g, 2, "C");
        ds4vizGwAddEdge(g, 0, 1, 1.0);
        ds4vizGwAddEdge(g, 1, 2, 2.0);
        ds4vizGwRemoveEdge(g, 0, 1);
    }
    p_c = get_output();
    T_VALID(p_c);
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试零权重
 */
static void
test_gw_zero_weight(void)
{
    char *p_c;

    setup_test("gw_zero_w");
    ds4vizGraphWeighted(g)
    {
        ds4vizGwAddNode(g, 0, "A");
        ds4vizGwAddNode(g, 1, "B");
        ds4vizGwAddEdge(g, 0, 1, 0);
    }
    p_c = get_output();
    T_VALID(p_c);
    T_TEARDOWN(p_c);
}

/* ================================================================
 * TestGlobalConfigIntegration: 全局配置集成测试
 * ================================================================ */

/**
 * @brief 测试配置 remarks
 */
static void
test_config_remarks(void)
{
    char *p_c;

    snprintf(g_test_path, sizeof(g_test_path), "test_config_remarks.toml");
    ds4vizConfig((ds4vizConfigOptions){
        .output_path = g_test_path,
        .title = "Title",
        .author = "Author",
        .comment = "Comment"});
    ds4vizStack(s)
    {
        ds4vizStackPush(s, 1);
    }
    p_c = get_output();
    T_VALID(p_c);
    T_CONTAINS(p_c, "title = \"Title\"");
    T_CONTAINS(p_c, "author = \"Author\"");
    T_CONTAINS(p_c, "comment = \"Comment\"");
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试配置输出路径
 */
static void
test_config_output_path(void)
{
    char *p_c;

    snprintf(g_test_path, sizeof(g_test_path), "test_config_path.toml");
    ds4vizConfig((ds4vizConfigOptions){.output_path = g_test_path});
    ds4vizStack(s)
    {
        ds4vizStackPush(s, 1);
    }
    p_c = read_file(g_test_path);
    T_ASSERT(p_c != NULL, "Custom output path file should exist");
    T_VALID(p_c);
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试仅设置 title
 */
static void
test_config_title_only(void)
{
    char *p_c;

    snprintf(g_test_path, sizeof(g_test_path), "test_config_title.toml");
    ds4vizConfig((ds4vizConfigOptions){
        .output_path = g_test_path,
        .title = "Only Title"});
    ds4vizStack(s)
    {
        ds4vizStackPush(s, 1);
    }
    p_c = get_output();
    T_VALID(p_c);
    T_CONTAINS(p_c, "title = \"Only Title\"");
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试无 remarks 参数时输出有效
 */
static void
test_config_no_remarks(void)
{
    char *p_c;

    setup_test("config_no_remarks");
    ds4vizStack(s)
    {
        ds4vizStackPush(s, 1);
    }
    p_c = get_output();
    T_VALID(p_c);
    T_NOT_CONTAINS(p_c, "[remarks]");
    T_TEARDOWN(p_c);
}

/* ================================================================
 * TestEdgeCases: 边缘情况测试
 * ================================================================ */

/**
 * @brief 测试空数据结构
 */
static void
test_edge_empty_structure(void)
{
    char *p_c;

    setup_test("edge_empty");
    ds4vizStack(s)
    {
        (void)s;
    }
    p_c = get_output();
    T_VALID(p_c);
    T_INT_EQ(str_count(p_c, "[[states]]"), 1);
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试特殊字符
 */
static void
test_edge_special_characters(void)
{
    char *p_c;

    setup_test("edge_special");
    ds4vizStack(s)
    {
        ds4vizStackPush(s, "hello\"world");
        ds4vizStackPush(s, "line1\\nline2");
    }
    p_c = get_output();
    T_VALID(p_c);
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试大量操作
 */
static void
test_edge_many_operations(void)
{
    char *p_c;

    setup_test("edge_many");
    ds4vizStack(s)
    {
        int i;

        for (i = 0; i < 100; i++)
        {
            ds4vizStackPush(s, i);
        }
        for (i = 0; i < 50; i++)
        {
            ds4vizStackPop(s);
        }
    }
    p_c = get_output();
    T_VALID(p_c);
    T_INT_EQ(str_count(p_c, "[[states]]"), 151);
    T_INT_EQ(str_count(p_c, "[[steps]]"), 150);
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试标签边界长度
 */
static void
test_edge_label_boundary(void)
{
    char *p_c;

    setup_test("edge_lbl_ok");
    ds4vizGraphUndirected(g)
    {
        ds4vizGuAddNode(g, 0, "A");
        ds4vizGuAddNode(g, 1, "ABCDEFGHIJKLMNOPQRSTUVWXYZ012345");
    }
    p_c = get_output();
    T_VALID(p_c);
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试标签过长
 */
static void
test_edge_label_too_long(void)
{
    char *p_c;

    setup_test("edge_lbl_long");
    ds4vizGraphUndirected(g)
    {
        ds4vizGuAddNode(g, 0, "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456");
    }
    p_c = get_output();
    T_VALID(p_c);
    T_CONTAINS(p_c, "[error]");
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试空标签
 */
static void
test_edge_label_empty(void)
{
    char *p_c;

    setup_test("edge_lbl_empty");
    ds4vizGraphUndirected(g)
    {
        ds4vizGuAddNode(g, 0, "");
    }
    p_c = get_output();
    T_VALID(p_c);
    T_CONTAINS(p_c, "[error]");
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试 Unicode 字符串
 */
static void
test_edge_unicode_strings(void)
{
    char *p_c;

    setup_test("edge_unicode");
    ds4vizStack(s)
    {
        ds4vizStackPush(s, "hello");
    }
    p_c = get_output();
    T_VALID(p_c);
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试嵌套 phase 内层优先
 */
static void
test_edge_nested_phases_priority(void)
{
    char *p_c;

    setup_test("edge_nested_phase");
    ds4vizQueue(q)
    {
        ds4vizPhase(q, "outer")
        {
            ds4vizQueueEnqueue(q, 1);
            ds4vizPhase(q, "inner")
            {
                ds4vizQueueEnqueue(q, 2);
            }
            ds4vizQueueEnqueue(q, 3);
        }
    }
    p_c = get_output();
    T_VALID(p_c);
    T_CONTAINS(p_c, "phase = \"inner\"");
    T_CONTAINS(p_c, "phase = \"outer\"");
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试所有预定义高亮样式
 */
static void
test_edge_all_highlight_styles(void)
{
    char *p_c;

    setup_test("edge_all_styles");
    ds4vizBinaryTree(t)
    {
        int root;

        root = ds4vizBtInsertRoot(t, 10);
        ds4vizStep(t, "focus",
                   ds4vizNode(root, DS4VIZ_FOCUS));
        ds4vizStep(t, "visited",
                   ds4vizNode(root, DS4VIZ_VISITED));
        ds4vizStep(t, "active",
                   ds4vizNode(root, DS4VIZ_ACTIVE));
        ds4vizStep(t, "comparing",
                   ds4vizNode(root, DS4VIZ_COMPARING));
        ds4vizStep(t, "found",
                   ds4vizNode(root, DS4VIZ_FOUND));
        ds4vizStep(t, "error",
                   ds4vizNode(root, DS4VIZ_ERROR_STY));
    }
    p_c = get_output();
    T_VALID(p_c);
    T_CONTAINS(p_c, "style = \"focus\"");
    T_CONTAINS(p_c, "style = \"visited\"");
    T_CONTAINS(p_c, "style = \"active\"");
    T_CONTAINS(p_c, "style = \"comparing\"");
    T_CONTAINS(p_c, "style = \"found\"");
    T_CONTAINS(p_c, "style = \"error\"");
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试连续两次 amend 覆盖
 */
static void
test_edge_amend_overwrite(void)
{
    char *p_c;

    setup_test("edge_amend_ow");
    ds4vizStack(s)
    {
        ds4vizStackPush(s, 10);
        ds4vizAmend(s, "first");
        ds4vizAmend(s, "second");
    }
    p_c = get_output();
    T_VALID(p_c);
    T_CONTAINS(p_c, "note = \"second\"");
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试高亮 level 边界值 1 和 9
 */
static void
test_edge_highlight_level_boundaries(void)
{
    char *p_c;

    setup_test("edge_hl_levels");
    ds4vizBinaryTree(t)
    {
        int root;

        root = ds4vizBtInsertRoot(t, 10);
        ds4vizStep(t, "level1",
                   ds4vizNode(root, DS4VIZ_FOCUS, 1));
        ds4vizStep(t, "level9",
                   ds4vizNode(root, DS4VIZ_FOCUS, 9));
    }
    p_c = get_output();
    T_VALID(p_c);
    T_CONTAINS(p_c, "level = 1");
    T_CONTAINS(p_c, "level = 9");
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试空二叉树
 */
static void
test_edge_empty_binary_tree(void)
{
    char *p_c;

    setup_test("edge_empty_bt");
    ds4vizBinaryTree(t)
    {
        (void)t;
    }
    p_c = get_output();
    T_VALID(p_c);
    T_CONTAINS(p_c, "root = -1");
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试空单链表
 */
static void
test_edge_empty_slist(void)
{
    char *p_c;

    setup_test("edge_empty_slist");
    ds4vizSingleLinkedList(l)
    {
        (void)l;
    }
    p_c = get_output();
    T_VALID(p_c);
    T_CONTAINS(p_c, "head = -1");
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试空双向链表
 */
static void
test_edge_empty_dlist(void)
{
    char *p_c;

    setup_test("edge_empty_dlist");
    ds4vizDoubleLinkedList(l)
    {
        (void)l;
    }
    p_c = get_output();
    T_VALID(p_c);
    T_CONTAINS(p_c, "head = -1");
    T_CONTAINS(p_c, "tail = -1");
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试空图结构
 */
static void
test_edge_empty_graph(void)
{
    char *p_c;

    setup_test("edge_empty_graph");
    ds4vizGraphDirected(g)
    {
        (void)g;
    }
    p_c = get_output();
    T_VALID(p_c);
    T_INT_EQ(str_count(p_c, "[[states]]"), 1);
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试 amend 传入空列表清除高亮
 */
static void
test_edge_amend_clear_highlights(void)
{
    char *p_c;

    setup_test("edge_amend_clr_hl");
    ds4vizStack(s)
    {
        ds4vizStackPush(s, 10);
        ds4vizAmendHL(s, ds4vizItem(0, DS4VIZ_FOCUS, 3));
        ds4vizAmendClearHL(s);
    }
    p_c = get_output();
    T_VALID(p_c);
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试 alias 不新增 state / step, 且会回写最后 state
 */
static void
test_ir_alias_no_extra_state(void)
{
    char *p_c;

    setup_test("ir_alias_no_extra_state");
    ds4vizBinaryTree(t)
    {
        int root;

        root = ds4vizBtInsertRoot(t, 10);
        ds4vizAlias(t, root, "root");
        ds4vizBtInsertLeft(t, root, 5);
    }
    p_c = get_output();
    T_VALID(p_c);

    T_INT_EQ(str_count(p_c, "[[states]]"), 3);
    T_INT_EQ(str_count(p_c, "[[steps]]"), 2);
    T_CONTAINS(p_c, "alias = \"root\"");

    T_TEARDOWN(p_c);
}

/**
 * @brief 测试 result.commit.line 跟随最后一次成功用户调用
 */
static void
test_ir_commit_line_last_user_call(void)
{
    char *p_c;
    int expected;
    int commit_line;
    bool ok;

    setup_test("ir_commit_line");
    expected = 0;
    ds4vizStack(s)
    {
        expected = __LINE__ + 1;
        ds4vizStackPush(s, 1);
    }

    p_c = get_output();
    T_VALID(p_c);

    ok = toml_get_commit_line(p_c, &commit_line);
    T_ASSERT(ok, "commit line should be readable");
    T_INT_EQ(commit_line, expected);

    T_TEARDOWN(p_c);
}

/**
 * @brief 测试 StepAt 显式行号覆盖
 */
static void
test_ir_stepat_line_override(void)
{
    char *p_c;
    int marker;
    int line_read;
    bool ok;

    setup_test("ir_stepat");
    marker = 0;
    ds4vizBinaryTree(t)
    {
        int root;

        root = ds4vizBtInsertRoot(t, 10);
        marker = __LINE__ + 1;
        ds4vizStepAt(t, marker, "manual line",
                     ds4vizNode(root, DS4VIZ_FOCUS, 3));
    }

    p_c = get_output();
    T_VALID(p_c);

    ok = toml_get_step_line_after_token(p_c, "note = \"manual line\"", &line_read);
    T_ASSERT(ok, "observe line should be readable");
    T_INT_EQ(line_read, marker);

    T_TEARDOWN(p_c);
}

/* ================================================================
 * TestComplexScenarios: 复杂场景测试
 * ================================================================ */

/**
 * @brief 测试 BST 复杂删除
 */
static void
test_complex_bst_deletion(void)
{
    char *p_c;

    setup_test("complex_bst_del");
    ds4vizBinarySearchTree(b)
    {
        int vals[] = {50, 30, 70, 20, 40, 60, 80, 35, 45};
        int i;

        for (i = 0; i < 9; i++)
        {
            ds4vizBstInsert(b, vals[i]);
        }
        ds4vizBstDelete(b, 30);
        ds4vizBstDelete(b, 50);
    }
    p_c = get_output();
    T_VALID(p_c);
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试图复杂操作
 */
static void
test_complex_graph(void)
{
    char *p_c;

    setup_test("complex_graph");
    ds4vizGraphWeighted(g)
    {
        int i;

        for (i = 0; i < 5; i++)
        {
            char lbl[2] = {(char)('A' + i), '\0'};
            ds4vizGwAddNode(g, i, lbl);
        }
        ds4vizGwAddEdge(g, 0, 1, 1.0);
        ds4vizGwAddEdge(g, 1, 2, 1.5);
        ds4vizGwAddEdge(g, 2, 3, 2.5);
        ds4vizGwAddEdge(g, 3, 4, 1.0);
        ds4vizGwUpdateWeight(g, 0, 1, 10.0);
        ds4vizGwRemoveNode(g, 2);
        ds4vizGwUpdateNodeLabel(g, 0, "Start");
    }
    p_c = get_output();
    T_VALID(p_c);
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试双向链表复杂操作
 */
static void
test_complex_dlist(void)
{
    char *p_c;

    setup_test("complex_dlist");
    ds4vizDoubleLinkedList(l)
    {
        int nodes[5];
        int i;

        for (i = 0; i < 5; i++)
        {
            nodes[i] = ds4vizDlInsertTail(l, i);
        }
        ds4vizDlInsertAfter(l, nodes[2], 100);
        ds4vizDlInsertBefore(l, nodes[2], 200);
        ds4vizDlDeleteHead(l);
        ds4vizDlDeleteTail(l);
        ds4vizDlReverse(l);
    }
    p_c = get_output();
    T_VALID(p_c);
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试所有数据结构均可正常产出
 */
static void
test_complex_all_structures(void)
{
    char *p_c;

    setup_test("complex_all_stack");
    ds4vizStack(s) { (void)s; }
    p_c = get_output();
    T_VALID(p_c);
    free(p_c);
    cleanup_test();

    setup_test("complex_all_queue");
    ds4vizQueue(q) { (void)q; }
    p_c = get_output();
    T_VALID(p_c);
    free(p_c);
    cleanup_test();

    setup_test("complex_all_slist");
    ds4vizSingleLinkedList(l) { (void)l; }
    p_c = get_output();
    T_VALID(p_c);
    free(p_c);
    cleanup_test();

    setup_test("complex_all_dlist");
    ds4vizDoubleLinkedList(d) { (void)d; }
    p_c = get_output();
    T_VALID(p_c);
    free(p_c);
    cleanup_test();

    setup_test("complex_all_bt");
    ds4vizBinaryTree(bt) { (void)bt; }
    p_c = get_output();
    T_VALID(p_c);
    free(p_c);
    cleanup_test();

    setup_test("complex_all_bst");
    ds4vizBinarySearchTree(b) { (void)b; }
    p_c = get_output();
    T_VALID(p_c);
    free(p_c);
    cleanup_test();

    setup_test("complex_all_gu");
    ds4vizGraphUndirected(gu) { (void)gu; }
    p_c = get_output();
    T_VALID(p_c);
    free(p_c);
    cleanup_test();

    setup_test("complex_all_gd");
    ds4vizGraphDirected(gd) { (void)gd; }
    p_c = get_output();
    T_VALID(p_c);
    free(p_c);
    cleanup_test();

    setup_test("complex_all_gw");
    ds4vizGraphWeighted(gw) { (void)gw; }
    p_c = get_output();
    T_VALID(p_c);
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试有向图 BFS 模拟
 */
static void
test_complex_bfs_simulation(void)
{
    char *p_c;

    setup_test("complex_bfs");
    ds4vizGraphDirected(g)
    {
        int i;

        ds4vizPhase(g, "build")
        {
            for (i = 0; i < 4; i++)
            {
                char lbl[2] = {(char)('A' + i), '\0'};
                ds4vizGdAddNode(g, i, lbl);
            }
            ds4vizGdAddEdge(g, 0, 1);
            ds4vizGdAddEdge(g, 0, 2);
            ds4vizGdAddEdge(g, 1, 3);
            ds4vizGdAddEdge(g, 2, 3);
        }
        ds4vizPhase(g, "BFS")
        {
            ds4vizStep(g, "start A",
                       ds4vizNode(0, DS4VIZ_FOCUS, 3));
            ds4vizStep(g, "visit B,C",
                       ds4vizNode(0, DS4VIZ_VISITED),
                       ds4vizNode(1, DS4VIZ_FOCUS, 2),
                       ds4vizNode(2, DS4VIZ_FOCUS, 2));
            ds4vizStep(g, "visit D",
                       ds4vizNode(0, DS4VIZ_VISITED),
                       ds4vizNode(1, DS4VIZ_VISITED),
                       ds4vizNode(2, DS4VIZ_VISITED),
                       ds4vizNode(3, DS4VIZ_FOCUS, 2));
            ds4vizStep(g, "BFS done",
                       ds4vizNode(0, DS4VIZ_VISITED),
                       ds4vizNode(1, DS4VIZ_VISITED),
                       ds4vizNode(2, DS4VIZ_VISITED),
                       ds4vizNode(3, DS4VIZ_VISITED));
        }
    }
    p_c = get_output();
    T_VALID(p_c);
    T_CONTAINS(p_c, "phase = \"BFS\"");
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试链表循环操作: 构建, 删除, 重建
 */
static void
test_complex_linked_list_cycle(void)
{
    char *p_c;

    setup_test("complex_ll_cycle");
    ds4vizSingleLinkedList(l)
    {
        int a;
        int b;
        int c;
        int d;
        int e;

        ds4vizPhase(l, "round1")
        {
            a = ds4vizSlInsertHead(l, 1);
            b = ds4vizSlInsertTail(l, 2);
            c = ds4vizSlInsertTail(l, 3);
            ds4vizSlDelete(l, b);
            ds4vizSlDelete(l, c);
            ds4vizSlDelete(l, a);
        }
        ds4vizPhase(l, "round2")
        {
            d = ds4vizSlInsertHead(l, 10);
            e = ds4vizSlInsertTail(l, 20);
            ds4vizStep(l, "new list",
                       ds4vizNode(d, DS4VIZ_FOCUS, 3),
                       ds4vizNode(e, DS4VIZ_VISITED));
        }
    }
    p_c = get_output();
    T_VALID(p_c);
    T_CONTAINS(p_c, "phase = \"round1\"");
    T_CONTAINS(p_c, "phase = \"round2\"");
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试深层二叉树 (10 层, 全左子树)
 */
static void
test_complex_deep_binary_tree(void)
{
    char *p_c;

    setup_test("complex_deep_bt");
    ds4vizBinaryTree(t)
    {
        int parent;
        int i;

        parent = ds4vizBtInsertRoot(t, 0);
        for (i = 1; i < 10; i++)
        {
            parent = ds4vizBtInsertLeft(t, parent, i);
        }
    }
    p_c = get_output();
    T_VALID(p_c);
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试完全无向图 (K5)
 */
static void
test_complex_complete_graph_k5(void)
{
    char *p_c;

    setup_test("complex_k5");
    ds4vizGraphUndirected(g)
    {
        int i;
        int j;

        for (i = 0; i < 5; i++)
        {
            char lbl[2] = {(char)('A' + i), '\0'};
            ds4vizGuAddNode(g, i, lbl);
        }
        for (i = 0; i < 5; i++)
        {
            for (j = i + 1; j < 5; j++)
            {
                ds4vizGuAddEdge(g, i, j);
            }
        }
    }
    p_c = get_output();
    T_VALID(p_c);
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试 Dijkstra 松弛过程模拟
 */
static void
test_complex_dijkstra(void)
{
    char *p_c;

    setup_test("complex_dijkstra");
    ds4vizGraphWeighted(g)
    {
        ds4vizPhase(g, "build")
        {
            ds4vizGwAddNode(g, 0, "src");
            ds4vizAlias(g, 0, "source");
            ds4vizGwAddNode(g, 1, "A");
            ds4vizGwAddNode(g, 2, "B");
            ds4vizGwAddNode(g, 3, "dst");
            ds4vizGwAddEdge(g, 0, 1, 1);
            ds4vizGwAddEdge(g, 0, 2, 4);
            ds4vizGwAddEdge(g, 1, 2, 2);
            ds4vizGwAddEdge(g, 2, 3, 3);
        }
        ds4vizPhase(g, "relax")
        {
            ds4vizStep(g, "from src",
                       ds4vizNode(0, DS4VIZ_VISITED),
                       ds4vizNode(1, DS4VIZ_FOCUS, 2),
                       ds4vizEdge(0, 1, DS4VIZ_ACTIVE, 3));
            ds4vizStep(g, "from A",
                       ds4vizNode(0, DS4VIZ_VISITED),
                       ds4vizNode(1, DS4VIZ_VISITED),
                       ds4vizNode(2, DS4VIZ_FOCUS, 2),
                       ds4vizEdge(1, 2, DS4VIZ_ACTIVE, 3));
            ds4vizStep(g, "done",
                       ds4vizNode(0, DS4VIZ_FOUND),
                       ds4vizNode(1, DS4VIZ_FOUND),
                       ds4vizNode(2, DS4VIZ_FOUND),
                       ds4vizNode(3, DS4VIZ_FOUND),
                       ds4vizEdge(0, 1, DS4VIZ_FOUND, 3),
                       ds4vizEdge(1, 2, DS4VIZ_FOUND, 3),
                       ds4vizEdge(2, 3, DS4VIZ_FOUND, 3));
        }
    }
    p_c = get_output();
    T_VALID(p_c);
    T_CONTAINS(p_c, "alias = \"source\"");
    T_CONTAINS(p_c, "phase = \"relax\"");
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试二叉树构建后完整前序遍历
 */
static void
test_complex_bt_build_and_traverse(void)
{
    char *p_c;

    setup_test("complex_bt_trav");
    ds4vizBinaryTree(t)
    {
        int root;
        int n2;
        int n3;
        int n4;
        int n5;

        ds4vizPhase(t, "build")
        {
            root = ds4vizBtInsertRoot(t, 1);
            n2 = ds4vizBtInsertLeft(t, root, 2);
            n3 = ds4vizBtInsertRight(t, root, 3);
            n4 = ds4vizBtInsertLeft(t, n2, 4);
            n5 = ds4vizBtInsertRight(t, n2, 5);
        }
        ds4vizPhase(t, "preorder")
        {
            ds4vizStep(t, "visit 1",
                       ds4vizNode(root, DS4VIZ_FOCUS, 3));
            ds4vizStep(t, "visit 2",
                       ds4vizNode(root, DS4VIZ_VISITED),
                       ds4vizNode(n2, DS4VIZ_FOCUS, 3));
            ds4vizStep(t, "visit 4",
                       ds4vizNode(root, DS4VIZ_VISITED),
                       ds4vizNode(n2, DS4VIZ_VISITED),
                       ds4vizNode(n4, DS4VIZ_FOCUS, 3));
            ds4vizStep(t, "visit 5",
                       ds4vizNode(root, DS4VIZ_VISITED),
                       ds4vizNode(n2, DS4VIZ_VISITED),
                       ds4vizNode(n4, DS4VIZ_VISITED),
                       ds4vizNode(n5, DS4VIZ_FOCUS, 3));
            ds4vizStep(t, "visit 3",
                       ds4vizNode(root, DS4VIZ_VISITED),
                       ds4vizNode(n2, DS4VIZ_VISITED),
                       ds4vizNode(n4, DS4VIZ_VISITED),
                       ds4vizNode(n5, DS4VIZ_VISITED),
                       ds4vizNode(n3, DS4VIZ_FOCUS, 3));
        }
    }
    p_c = get_output();
    T_VALID(p_c);
    T_INT_EQ(str_count(p_c, "op = \"observe\""), 5);
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试图构建-删除-重建
 */
static void
test_complex_graph_build_remove_rebuild(void)
{
    char *p_c;

    setup_test("complex_grr");
    ds4vizGraphDirected(g)
    {
        ds4vizPhase(g, "build")
        {
            ds4vizGdAddNode(g, 0, "A");
            ds4vizGdAddNode(g, 1, "B");
            ds4vizGdAddNode(g, 2, "C");
            ds4vizGdAddEdge(g, 0, 1);
            ds4vizGdAddEdge(g, 1, 2);
        }
        ds4vizPhase(g, "cleanup")
        {
            ds4vizGdRemoveNode(g, 1);
        }
        ds4vizPhase(g, "rebuild")
        {
            ds4vizGdAddNode(g, 3, "D");
            ds4vizGdAddEdge(g, 0, 3);
            ds4vizGdAddEdge(g, 3, 2);
        }
    }
    p_c = get_output();
    T_VALID(p_c);
    T_CONTAINS(p_c, "phase = \"build\"");
    T_CONTAINS(p_c, "phase = \"cleanup\"");
    T_CONTAINS(p_c, "phase = \"rebuild\"");
    T_TEARDOWN(p_c);
}

/**
 * @brief 测试单链表遍历中使用 amend
 */
static void
test_complex_slist_traversal_amend(void)
{
    char *p_c;

    setup_test("complex_sla");
    ds4vizSingleLinkedList(l)
    {
        int a;
        int b;
        int c;

        a = ds4vizSlInsertTail(l, 10);
        b = ds4vizSlInsertTail(l, 20);
        c = ds4vizSlInsertTail(l, 30);
        ds4vizStep(l, "begin traverse");
        ds4vizAmendHL(l, ds4vizNode(a, DS4VIZ_FOCUS, 3));
        ds4vizStep(l, "next node",
                   ds4vizNode(a, DS4VIZ_VISITED),
                   ds4vizNode(b, DS4VIZ_FOCUS, 3));
        ds4vizStep(l, "last node",
                   ds4vizNode(a, DS4VIZ_VISITED),
                   ds4vizNode(b, DS4VIZ_VISITED),
                   ds4vizNode(c, DS4VIZ_FOCUS, 3));
    }
    p_c = get_output();
    T_VALID(p_c);
    T_INT_EQ(str_count(p_c, "op = \"observe\""), 3);
    T_TEARDOWN(p_c);
}

/* ================================================================
 * main
 * ================================================================ */

/**
 * @brief 测试入口
 *
 * @return int 全部通过返回 0, 有失败返回 1
 */
int main(void)
{
    printf("=== ds4viz C Library Tests ===\n\n");

    printf("--- Stack Integration ---\n");
    RUN_TEST(test_stack_basic_operations);
    RUN_TEST(test_stack_empty_pop_error);
    RUN_TEST(test_stack_clear);
    RUN_TEST(test_stack_various_types);
    RUN_TEST(test_stack_step_and_amend);
    RUN_TEST(test_stack_phase);
    RUN_TEST(test_stack_push_pop_push_cycle);
    RUN_TEST(test_stack_sequential_pop_lifo);
    RUN_TEST(test_stack_amend_no_prev_error);
    RUN_TEST(test_stack_nested_phases);
    RUN_TEST(test_stack_clear_then_push);
    RUN_TEST(test_stack_single_push_pop);
    RUN_TEST(test_stack_pop_after_clear_error);

    printf("\n--- Queue Integration ---\n");
    RUN_TEST(test_queue_basic_operations);
    RUN_TEST(test_queue_empty_dequeue_error);
    RUN_TEST(test_queue_phase_and_amend);
    RUN_TEST(test_queue_clear);
    RUN_TEST(test_queue_various_types);
    RUN_TEST(test_queue_fifo_order);
    RUN_TEST(test_queue_step_with_highlights);
    RUN_TEST(test_queue_dequeue_after_clear_error);

    printf("\n--- Single Linked List Integration ---\n");
    RUN_TEST(test_slist_basic_operations);
    RUN_TEST(test_slist_reverse);
    RUN_TEST(test_slist_delete_nonexistent_error);
    RUN_TEST(test_slist_alias_and_step);
    RUN_TEST(test_slist_delete_head_single);
    RUN_TEST(test_slist_insert_after_nonexistent_error);
    RUN_TEST(test_slist_delete_head_empty_error);
    RUN_TEST(test_slist_multiple_head_inserts);
    RUN_TEST(test_slist_reverse_single);
    RUN_TEST(test_slist_reverse_empty);

    printf("\n--- Double Linked List Integration ---\n");
    RUN_TEST(test_dlist_basic_operations);
    RUN_TEST(test_dlist_reverse);
    RUN_TEST(test_dlist_delete_head_empty_error);
    RUN_TEST(test_dlist_delete_specific_node);
    RUN_TEST(test_dlist_insert_before_nonexistent_error);
    RUN_TEST(test_dlist_alias_and_step);
    RUN_TEST(test_dlist_delete_head_single);
    RUN_TEST(test_dlist_delete_tail_empty_error);
    RUN_TEST(test_dlist_delete_nonexistent_error);
    RUN_TEST(test_dlist_insert_after_nonexistent_error);

    printf("\n--- Binary Tree Integration ---\n");
    RUN_TEST(test_bt_basic_operations);
    RUN_TEST(test_bt_delete_subtree);
    RUN_TEST(test_bt_update_value);
    RUN_TEST(test_bt_duplicate_root_error);
    RUN_TEST(test_bt_step_traversal);
    RUN_TEST(test_bt_delete_root);
    RUN_TEST(test_bt_delete_leaf);
    RUN_TEST(test_bt_insert_left_occupied_error);
    RUN_TEST(test_bt_insert_nonexistent_parent_error);
    RUN_TEST(test_bt_update_nonexistent_error);
    RUN_TEST(test_bt_insert_right_occupied_error);
    RUN_TEST(test_bt_delete_nonexistent_error);
    RUN_TEST(test_bt_string_values);

    printf("\n--- BST Integration ---\n");
    RUN_TEST(test_bst_basic_operations);
    RUN_TEST(test_bst_delete_leaf);
    RUN_TEST(test_bst_delete_two_children);
    RUN_TEST(test_bst_delete_nonexistent_error);
    RUN_TEST(test_bst_delete_root);
    RUN_TEST(test_bst_delete_one_child);
    RUN_TEST(test_bst_phase_and_alias);
    RUN_TEST(test_bst_sequential_insert);
    RUN_TEST(test_bst_delete_all);
    RUN_TEST(test_bst_insert_descending);

    printf("\n--- Graph Undirected Integration ---\n");
    RUN_TEST(test_gu_basic);
    RUN_TEST(test_gu_edge_normalization);
    RUN_TEST(test_gu_self_loop_error);
    RUN_TEST(test_gu_duplicate_edge_error);
    RUN_TEST(test_gu_remove_node_and_edges);
    RUN_TEST(test_gu_step_edge_highlight);
    RUN_TEST(test_gu_remove_edge);
    RUN_TEST(test_gu_update_node_label);
    RUN_TEST(test_gu_remove_nonexistent_node_error);
    RUN_TEST(test_gu_add_duplicate_node_error);
    RUN_TEST(test_gu_add_edge_nonexistent_error);
    RUN_TEST(test_gu_duplicate_edge_reversed_error);

    printf("\n--- Graph Directed Integration ---\n");
    RUN_TEST(test_gd_basic);
    RUN_TEST(test_gd_no_normalization);
    RUN_TEST(test_gd_self_loop_error);
    RUN_TEST(test_gd_remove_node_and_edges);
    RUN_TEST(test_gd_remove_edge);
    RUN_TEST(test_gd_duplicate_edge_error);
    RUN_TEST(test_gd_add_duplicate_node_error);
    RUN_TEST(test_gd_update_node_label);
    RUN_TEST(test_gd_add_edge_nonexistent_error);
    RUN_TEST(test_gd_remove_nonexistent_edge_error);

    printf("\n--- Graph Weighted Integration ---\n");
    RUN_TEST(test_gw_basic);
    RUN_TEST(test_gw_update_weight);
    RUN_TEST(test_gw_alias_and_highlights);
    RUN_TEST(test_gw_remove_node);
    RUN_TEST(test_gw_update_nonexistent_edge_error);
    RUN_TEST(test_gw_negative_weight);
    RUN_TEST(test_gw_self_loop_error);
    RUN_TEST(test_gw_duplicate_edge_error);
    RUN_TEST(test_gw_add_duplicate_node_error);
    RUN_TEST(test_gw_update_node_label);
    RUN_TEST(test_gw_integer_weight);
    RUN_TEST(test_gw_remove_edge);
    RUN_TEST(test_gw_zero_weight);

    printf("\n--- Global Config ---\n");
    RUN_TEST(test_config_remarks);
    RUN_TEST(test_config_output_path);
    RUN_TEST(test_config_title_only);
    RUN_TEST(test_config_no_remarks);

    printf("\n--- Edge Cases ---\n");
    RUN_TEST(test_edge_empty_structure);
    RUN_TEST(test_edge_special_characters);
    RUN_TEST(test_edge_many_operations);
    RUN_TEST(test_edge_label_boundary);
    RUN_TEST(test_edge_label_too_long);
    RUN_TEST(test_edge_label_empty);
    RUN_TEST(test_edge_unicode_strings);
    RUN_TEST(test_edge_nested_phases_priority);
    RUN_TEST(test_edge_all_highlight_styles);
    RUN_TEST(test_edge_amend_overwrite);
    RUN_TEST(test_edge_highlight_level_boundaries);
    RUN_TEST(test_edge_empty_binary_tree);
    RUN_TEST(test_edge_empty_slist);
    RUN_TEST(test_edge_empty_dlist);
    RUN_TEST(test_edge_empty_graph);
    RUN_TEST(test_edge_amend_clear_highlights);
    RUN_TEST(test_ir_alias_no_extra_state);
    RUN_TEST(test_ir_commit_line_last_user_call);
    RUN_TEST(test_ir_stepat_line_override);

    printf("\n--- Complex Scenarios ---\n");
    RUN_TEST(test_complex_bst_deletion);
    RUN_TEST(test_complex_graph);
    RUN_TEST(test_complex_dlist);
    RUN_TEST(test_complex_all_structures);
    RUN_TEST(test_complex_bfs_simulation);
    RUN_TEST(test_complex_linked_list_cycle);
    RUN_TEST(test_complex_deep_binary_tree);
    RUN_TEST(test_complex_complete_graph_k5);
    RUN_TEST(test_complex_dijkstra);
    RUN_TEST(test_complex_bt_build_and_traverse);
    RUN_TEST(test_complex_graph_build_remove_rebuild);
    RUN_TEST(test_complex_slist_traversal_amend);

    printf("\n========================================\n");
    printf("  Total: %d | Passed: %d | Failed: %d\n",
           g_tests_run, g_tests_passed, g_tests_failed);
    printf("========================================\n");

    return g_tests_failed > 0 ? 1 : 0;
}