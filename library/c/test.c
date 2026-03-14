/* test.c — Comprehensive tests for ds4viz.h (C23)
 *
 * Build: see Makefile
 * Coverage mirrors the Python integration_test.py + unit_test.py
 */

#define DS4VIZ_IMPLEMENTATION
#include "ds4viz.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

/* ================================================================
 *  MINI TEST FRAMEWORK
 * ================================================================ */

static int g_run, g_pass, g_fail;
static int _tf;

#define TEST(name) static void test_##name(void)

#define RUN(name)                          \
    do                                     \
    {                                      \
        _tf = 0;                           \
        test_##name();                     \
        g_run++;                           \
        if (_tf)                           \
        {                                  \
            printf("  FAIL  %s\n", #name); \
            g_fail++;                      \
        }                                  \
        else                               \
        {                                  \
            printf("  OK    %s\n", #name); \
            g_pass++;                      \
        }                                  \
    } while (0)

#define ASSERT(c)                                      \
    do                                                 \
    {                                                  \
        if (!(c))                                      \
        {                                              \
            printf("    line %d: %s\n", __LINE__, #c); \
            _tf = 1;                                   \
            return;                                    \
        }                                              \
    } while (0)

#define SECTION(t) printf("\n=== %s ===\n", (t))

/* ================================================================
 *  HELPERS
 * ================================================================ */

static char *read_file(const char *p)
{
    FILE *f = fopen(p, "rb");
    if (!f)
        return NULL;
    fseek(f, 0, SEEK_END);
    long n = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *b = (char *)malloc((size_t)n + 1);
    fread(b, 1, (size_t)n, f);
    b[n] = '\0';
    fclose(f);
    return b;
}

static bool has(const char *s, const char *sub)
{
    return s && sub && strstr(s, sub) != NULL;
}

static int count_str(const char *s, const char *sub)
{
    if (!s || !sub)
        return 0;
    int c = 0;
    size_t len = strlen(sub);
    for (const char *p = s; (p = strstr(p, sub)) != NULL; p += len)
        c++;
    return c;
}

/* Set output path, clearing other config fields */
static void cfg(const char *path)
{
    ds4vizConfig((ds4vizConfigOptions){
        .output_path = path, .title = NULL, .author = NULL, .comment = NULL});
}

/* Set output path with full config */
static void cfg_full(const char *path, const char *title,
                     const char *author, const char *comment)
{
    ds4vizConfig((ds4vizConfigOptions){
        .output_path = path, .title = title, .author = author, .comment = comment});
}

/* Validate: has [structure], [[states]], and exactly one of [result]/[error] */
static bool valid_basic(const char *c)
{
    if (!c)
        return false;
    if (!has(c, "[structure]"))
        return false;
    if (!has(c, "[[states]]"))
        return false;
    bool r = has(c, "\n[result]\n") || has(c, "[result]\n");
    bool e = has(c, "\n[error]\n") || has(c, "[error]\n");
    if (r == e)
        return false; /* must be XOR */
    return true;
}

static bool valid_ok(const char *c)
{
    return valid_basic(c) && has(c, "[result]") && !has(c, "[error]");
}

static bool valid_err(const char *c)
{
    return valid_basic(c) && has(c, "[error]") && !has(c, "[result]");
}

/* ================================================================
 *  STACK TESTS
 * ================================================================ */

TEST(stack_basic_operations)
{
    cfg("_t_s01.toml");
    ds4vizStack(s, "test_stack")
    {
        ds4vizStackPush(s, 10);
        ds4vizStackPush(s, 20);
        ds4vizStackPush(s, 30);
        ds4vizStackPop(s);
        ds4vizStackPush(s, 40);
    }
    char *c = read_file("_t_s01.toml");
    ASSERT(c);
    ASSERT(valid_ok(c));
    ASSERT(has(c, "type = \"stack\""));
    ASSERT(has(c, "label = \"test_stack\""));
    ASSERT(count_str(c, "[[states]]") == 6);
    ASSERT(count_str(c, "[[steps]]") == 5);
    free(c);
    remove("_t_s01.toml");
}

TEST(stack_empty_pop_error)
{
    cfg("_t_s02.toml");
    ds4vizStack(s)
    {
        ds4vizStackPop(s);
    }
    char *c = read_file("_t_s02.toml");
    ASSERT(c);
    ASSERT(valid_err(c));
    ASSERT(has(c, "type = \"runtime\""));
    free(c);
    remove("_t_s02.toml");
}

TEST(stack_clear)
{
    cfg("_t_s03.toml");
    ds4vizStack(s)
    {
        ds4vizStackPush(s, 1);
        ds4vizStackPush(s, 2);
        ds4vizStackPush(s, 3);
        ds4vizStackClear(s);
    }
    char *c = read_file("_t_s03.toml");
    ASSERT(c);
    ASSERT(valid_ok(c));
    ASSERT(has(c, "elements = []"));
    free(c);
    remove("_t_s03.toml");
}

TEST(stack_various_value_types)
{
    cfg("_t_s04.toml");
    ds4vizStack(s)
    {
        ds4vizStackPush(s, 42);
        ds4vizStackPush(s, 3.14);
        ds4vizStackPush(s, "hello");
        ds4vizStackPush(s, true);
        ds4vizStackPush(s, false);
    }
    char *c = read_file("_t_s04.toml");
    ASSERT(c);
    ASSERT(valid_ok(c));
    ASSERT(has(c, "42"));
    ASSERT(has(c, "\"hello\""));
    ASSERT(has(c, "true"));
    ASSERT(has(c, "false"));
    free(c);
    remove("_t_s04.toml");
}

TEST(stack_negative_numbers)
{
    cfg("_t_s05.toml");
    ds4vizStack(s)
    {
        ds4vizStackPush(s, -10);
        ds4vizStackPush(s, -20);
        ds4vizStackPop(s);
    }
    char *c = read_file("_t_s05.toml");
    ASSERT(c);
    ASSERT(valid_ok(c));
    ASSERT(has(c, "-10"));
    free(c);
    remove("_t_s05.toml");
}

TEST(stack_mixed_operations)
{
    cfg("_t_s06.toml");
    ds4vizStack(s)
    {
        ds4vizStackPush(s, 1);
        ds4vizStackPush(s, 2);
        ds4vizStackPop(s);
        ds4vizStackPush(s, 3);
        ds4vizStackClear(s);
        ds4vizStackPush(s, 4);
    }
    char *c = read_file("_t_s06.toml");
    ASSERT(c);
    ASSERT(valid_ok(c));
    ASSERT(count_str(c, "[[steps]]") == 6);
    free(c);
    remove("_t_s06.toml");
}

TEST(stack_single_element)
{
    cfg("_t_s07.toml");
    ds4vizStack(s)
    {
        ds4vizStackPush(s, 100);
        ds4vizStackPop(s);
    }
    char *c = read_file("_t_s07.toml");
    ASSERT(c);
    ASSERT(valid_ok(c));
    free(c);
    remove("_t_s07.toml");
}

TEST(stack_push_after_clear)
{
    cfg("_t_s08.toml");
    ds4vizStack(s)
    {
        ds4vizStackPush(s, 1);
        ds4vizStackPush(s, 2);
        ds4vizStackClear(s);
        ds4vizStackPush(s, 10);
        ds4vizStackPush(s, 20);
    }
    char *c = read_file("_t_s08.toml");
    ASSERT(c);
    ASSERT(valid_ok(c));
    free(c);
    remove("_t_s08.toml");
}

TEST(stack_multiple_pops)
{
    cfg("_t_s09.toml");
    ds4vizStack(s)
    {
        for (int i = 0; i < 5; i++)
            ds4vizStackPush(s, i);
        for (int i = 0; i < 3; i++)
            ds4vizStackPop(s);
    }
    char *c = read_file("_t_s09.toml");
    ASSERT(c);
    ASSERT(valid_ok(c));
    ASSERT(count_str(c, "[[steps]]") == 8);
    free(c);
    remove("_t_s09.toml");
}

TEST(stack_float_values)
{
    cfg("_t_s10.toml");
    ds4vizStack(s)
    {
        ds4vizStackPush(s, 1.5);
        ds4vizStackPush(s, 2.7);
        ds4vizStackPush(s, -3.14);
        ds4vizStackPop(s);
    }
    char *c = read_file("_t_s10.toml");
    ASSERT(c);
    ASSERT(valid_ok(c));
    free(c);
    remove("_t_s10.toml");
}

TEST(stack_string_values)
{
    cfg("_t_s11.toml");
    ds4vizStack(s)
    {
        ds4vizStackPush(s, "first");
        ds4vizStackPush(s, "second");
        ds4vizStackPush(s, "third");
        ds4vizStackPop(s);
    }
    char *c = read_file("_t_s11.toml");
    ASSERT(c);
    ASSERT(valid_ok(c));
    ASSERT(has(c, "\"first\""));
    ASSERT(has(c, "\"second\""));
    free(c);
    remove("_t_s11.toml");
}

TEST(stack_boolean_values)
{
    cfg("_t_s12.toml");
    ds4vizStack(s)
    {
        ds4vizStackPush(s, true);
        ds4vizStackPush(s, false);
        ds4vizStackPush(s, true);
        ds4vizStackPop(s);
    }
    char *c = read_file("_t_s12.toml");
    ASSERT(c);
    ASSERT(valid_ok(c));
    free(c);
    remove("_t_s12.toml");
}

TEST(stack_zero_value)
{
    cfg("_t_s13.toml");
    ds4vizStack(s)
    {
        ds4vizStackPush(s, 0);
        ds4vizStackPush(s, 0.0);
        ds4vizStackPop(s);
    }
    char *c = read_file("_t_s13.toml");
    ASSERT(c);
    ASSERT(valid_ok(c));
    free(c);
    remove("_t_s13.toml");
}

/* ================================================================
 *  QUEUE TESTS
 * ================================================================ */

TEST(queue_basic_operations)
{
    cfg("_t_q01.toml");
    ds4vizQueue(q, "test_queue")
    {
        ds4vizQueueEnqueue(q, 10);
        ds4vizQueueEnqueue(q, 20);
        ds4vizQueueDequeue(q);
        ds4vizQueueEnqueue(q, 30);
    }
    char *c = read_file("_t_q01.toml");
    ASSERT(c);
    ASSERT(valid_ok(c));
    ASSERT(has(c, "type = \"queue\""));
    ASSERT(has(c, "label = \"test_queue\""));
    ASSERT(count_str(c, "[[steps]]") == 4);
    free(c);
    remove("_t_q01.toml");
}

TEST(queue_empty_dequeue_error)
{
    cfg("_t_q02.toml");
    ds4vizQueue(q)
    {
        ds4vizQueueDequeue(q);
    }
    char *c = read_file("_t_q02.toml");
    ASSERT(c);
    ASSERT(valid_err(c));
    ASSERT(has(c, "type = \"runtime\""));
    free(c);
    remove("_t_q02.toml");
}

TEST(queue_clear)
{
    cfg("_t_q03.toml");
    ds4vizQueue(q)
    {
        ds4vizQueueEnqueue(q, 1);
        ds4vizQueueEnqueue(q, 2);
        ds4vizQueueEnqueue(q, 3);
        ds4vizQueueClear(q);
    }
    char *c = read_file("_t_q03.toml");
    ASSERT(c);
    ASSERT(valid_ok(c));
    ASSERT(has(c, "elements = []"));
    free(c);
    remove("_t_q03.toml");
}

TEST(queue_single_element)
{
    cfg("_t_q04.toml");
    ds4vizQueue(q)
    {
        ds4vizQueueEnqueue(q, 100);
        ds4vizQueueDequeue(q);
    }
    char *c = read_file("_t_q04.toml");
    ASSERT(c);
    ASSERT(valid_ok(c));
    free(c);
    remove("_t_q04.toml");
}

TEST(queue_enqueue_after_clear)
{
    cfg("_t_q05.toml");
    ds4vizQueue(q)
    {
        ds4vizQueueEnqueue(q, 1);
        ds4vizQueueEnqueue(q, 2);
        ds4vizQueueClear(q);
        ds4vizQueueEnqueue(q, 10);
        ds4vizQueueEnqueue(q, 20);
    }
    char *c = read_file("_t_q05.toml");
    ASSERT(c);
    ASSERT(valid_ok(c));
    free(c);
    remove("_t_q05.toml");
}

TEST(queue_multiple_dequeues)
{
    cfg("_t_q06.toml");
    ds4vizQueue(q)
    {
        for (int i = 0; i < 5; i++)
            ds4vizQueueEnqueue(q, i);
        for (int i = 0; i < 3; i++)
            ds4vizQueueDequeue(q);
    }
    char *c = read_file("_t_q06.toml");
    ASSERT(c);
    ASSERT(valid_ok(c));
    ASSERT(count_str(c, "[[steps]]") == 8);
    free(c);
    remove("_t_q06.toml");
}

TEST(queue_alternating_operations)
{
    cfg("_t_q07.toml");
    ds4vizQueue(q)
    {
        ds4vizQueueEnqueue(q, 1);
        ds4vizQueueEnqueue(q, 2);
        ds4vizQueueDequeue(q);
        ds4vizQueueEnqueue(q, 3);
        ds4vizQueueDequeue(q);
        ds4vizQueueEnqueue(q, 4);
    }
    char *c = read_file("_t_q07.toml");
    ASSERT(c);
    ASSERT(valid_ok(c));
    free(c);
    remove("_t_q07.toml");
}

TEST(queue_negative_numbers)
{
    cfg("_t_q08.toml");
    ds4vizQueue(q)
    {
        ds4vizQueueEnqueue(q, -10);
        ds4vizQueueEnqueue(q, -20);
        ds4vizQueueDequeue(q);
    }
    char *c = read_file("_t_q08.toml");
    ASSERT(c);
    ASSERT(valid_ok(c));
    free(c);
    remove("_t_q08.toml");
}

TEST(queue_float_values)
{
    cfg("_t_q09.toml");
    ds4vizQueue(q)
    {
        ds4vizQueueEnqueue(q, 1.5);
        ds4vizQueueEnqueue(q, 2.7);
        ds4vizQueueDequeue(q);
    }
    char *c = read_file("_t_q09.toml");
    ASSERT(c);
    ASSERT(valid_ok(c));
    free(c);
    remove("_t_q09.toml");
}

TEST(queue_string_values)
{
    cfg("_t_q10.toml");
    ds4vizQueue(q)
    {
        ds4vizQueueEnqueue(q, "first");
        ds4vizQueueEnqueue(q, "second");
        ds4vizQueueDequeue(q);
    }
    char *c = read_file("_t_q10.toml");
    ASSERT(c);
    ASSERT(valid_ok(c));
    free(c);
    remove("_t_q10.toml");
}

TEST(queue_dequeue_until_empty)
{
    cfg("_t_q11.toml");
    ds4vizQueue(q)
    {
        for (int i = 0; i < 3; i++)
            ds4vizQueueEnqueue(q, i);
        for (int i = 0; i < 3; i++)
            ds4vizQueueDequeue(q);
    }
    char *c = read_file("_t_q11.toml");
    ASSERT(c);
    ASSERT(valid_ok(c));
    ASSERT(has(c, "elements = []"));
    free(c);
    remove("_t_q11.toml");
}

/* ================================================================
 *  SINGLY LINKED LIST TESTS
 * ================================================================ */

TEST(slist_basic_operations)
{
    cfg("_t_sl01.toml");
    ds4vizSingleLinkedList(l, "test_slist")
    {
        int n1 = ds4vizSlInsertHead(l, 10);
        ds4vizSlInsertTail(l, 20);
        ds4vizSlInsertAfter(l, n1, 15);
        ds4vizSlDeleteHead(l);
    }
    char *c = read_file("_t_sl01.toml");
    ASSERT(c);
    ASSERT(valid_ok(c));
    ASSERT(has(c, "type = \"slist\""));
    ASSERT(has(c, "label = \"test_slist\""));
    free(c);
    remove("_t_sl01.toml");
}

TEST(slist_reverse)
{
    cfg("_t_sl02.toml");
    ds4vizSingleLinkedList(l)
    {
        ds4vizSlInsertTail(l, 1);
        ds4vizSlInsertTail(l, 2);
        ds4vizSlInsertTail(l, 3);
        ds4vizSlReverse(l);
    }
    char *c = read_file("_t_sl02.toml");
    ASSERT(c);
    ASSERT(valid_ok(c));
    ASSERT(has(c, "\"reverse\""));
    free(c);
    remove("_t_sl02.toml");
}

TEST(slist_delete_nonexistent_error)
{
    cfg("_t_sl03.toml");
    ds4vizSingleLinkedList(l)
    {
        ds4vizSlInsertHead(l, 10);
        ds4vizSlDelete(l, 999);
    }
    char *c = read_file("_t_sl03.toml");
    ASSERT(c);
    ASSERT(valid_err(c));
    free(c);
    remove("_t_sl03.toml");
}

TEST(slist_empty_delete_head_error)
{
    cfg("_t_sl04.toml");
    ds4vizSingleLinkedList(l)
    {
        ds4vizSlDeleteHead(l);
    }
    char *c = read_file("_t_sl04.toml");
    ASSERT(c);
    ASSERT(valid_err(c));
    free(c);
    remove("_t_sl04.toml");
}

TEST(slist_insert_after_nonexistent_error)
{
    cfg("_t_sl05.toml");
    ds4vizSingleLinkedList(l)
    {
        ds4vizSlInsertHead(l, 10);
        ds4vizSlInsertAfter(l, 999, 20);
    }
    char *c = read_file("_t_sl05.toml");
    ASSERT(c);
    ASSERT(valid_err(c));
    free(c);
    remove("_t_sl05.toml");
}

TEST(slist_delete_tail)
{
    cfg("_t_sl06.toml");
    ds4vizSingleLinkedList(l)
    {
        ds4vizSlInsertTail(l, 1);
        ds4vizSlInsertTail(l, 2);
        int n3 = ds4vizSlInsertTail(l, 3);
        ds4vizSlDelete(l, n3);
    }
    char *c = read_file("_t_sl06.toml");
    ASSERT(c);
    ASSERT(valid_ok(c));
    free(c);
    remove("_t_sl06.toml");
}

TEST(slist_clear)
{
    cfg("_t_sl07.toml");
    ds4vizSingleLinkedList(l)
    {
        ds4vizSlInsertTail(l, 1);
        ds4vizSlInsertTail(l, 2);
        ds4vizSlDeleteHead(l);
        ds4vizSlDeleteHead(l);
    }
    char *c = read_file("_t_sl07.toml");
    ASSERT(c);
    ASSERT(valid_ok(c));
    ASSERT(has(c, "head = -1"));
    free(c);
    remove("_t_sl07.toml");
}

TEST(slist_single_node)
{
    cfg("_t_sl08.toml");
    ds4vizSingleLinkedList(l)
    {
        ds4vizSlInsertHead(l, 100);
        ds4vizSlDeleteHead(l);
    }
    char *c = read_file("_t_sl08.toml");
    ASSERT(c);
    ASSERT(valid_ok(c));
    free(c);
    remove("_t_sl08.toml");
}

TEST(slist_reverse_empty)
{
    cfg("_t_sl09.toml");
    ds4vizSingleLinkedList(l)
    {
        ds4vizSlReverse(l);
    }
    char *c = read_file("_t_sl09.toml");
    ASSERT(c);
    ASSERT(valid_ok(c));
    free(c);
    remove("_t_sl09.toml");
}

TEST(slist_reverse_single)
{
    cfg("_t_sl10.toml");
    ds4vizSingleLinkedList(l)
    {
        ds4vizSlInsertHead(l, 1);
        ds4vizSlReverse(l);
    }
    char *c = read_file("_t_sl10.toml");
    ASSERT(c);
    ASSERT(valid_ok(c));
    free(c);
    remove("_t_sl10.toml");
}

TEST(slist_multiple_inserts)
{
    cfg("_t_sl11.toml");
    ds4vizSingleLinkedList(l)
    {
        for (int i = 0; i < 5; i++)
            ds4vizSlInsertTail(l, i);
    }
    char *c = read_file("_t_sl11.toml");
    ASSERT(c);
    ASSERT(valid_ok(c));
    ASSERT(count_str(c, "[[steps]]") == 5);
    free(c);
    remove("_t_sl11.toml");
}

/* ================================================================
 *  DOUBLY LINKED LIST TESTS
 * ================================================================ */

TEST(dlist_basic_operations)
{
    cfg("_t_dl01.toml");
    ds4vizDoubleLinkedList(l, "test_dlist")
    {
        int n1 = ds4vizDlInsertHead(l, 10);
        int n2 = ds4vizDlInsertTail(l, 30);
        ds4vizDlInsertBefore(l, n2, 20);
        ds4vizDlInsertAfter(l, n1, 15);
        ds4vizDlDeleteTail(l);
    }
    char *c = read_file("_t_dl01.toml");
    ASSERT(c);
    ASSERT(valid_ok(c));
    ASSERT(has(c, "type = \"dlist\""));
    ASSERT(has(c, "label = \"test_dlist\""));
    free(c);
    remove("_t_dl01.toml");
}

TEST(dlist_reverse)
{
    cfg("_t_dl02.toml");
    ds4vizDoubleLinkedList(l)
    {
        ds4vizDlInsertTail(l, 1);
        ds4vizDlInsertTail(l, 2);
        ds4vizDlInsertTail(l, 3);
        ds4vizDlReverse(l);
    }
    char *c = read_file("_t_dl02.toml");
    ASSERT(c);
    ASSERT(valid_ok(c));
    ASSERT(has(c, "\"reverse\""));
    free(c);
    remove("_t_dl02.toml");
}

TEST(dlist_head_tail_consistency)
{
    cfg("_t_dl03.toml");
    ds4vizDoubleLinkedList(l)
    {
        ds4vizDlInsertHead(l, 1);
        ds4vizDlDeleteHead(l);
        ds4vizDlInsertTail(l, 2);
    }
    char *c = read_file("_t_dl03.toml");
    ASSERT(c);
    ASSERT(valid_ok(c));
    /* After delete_head of single node, head==tail==-1.
       Then insert_tail makes head==tail==new node. */
    free(c);
    remove("_t_dl03.toml");
}

TEST(dlist_delete_head_error)
{
    cfg("_t_dl04.toml");
    ds4vizDoubleLinkedList(l)
    {
        ds4vizDlDeleteHead(l);
    }
    char *c = read_file("_t_dl04.toml");
    ASSERT(c);
    ASSERT(valid_err(c));
    free(c);
    remove("_t_dl04.toml");
}

TEST(dlist_delete_tail_error)
{
    cfg("_t_dl05.toml");
    ds4vizDoubleLinkedList(l)
    {
        ds4vizDlDeleteTail(l);
    }
    char *c = read_file("_t_dl05.toml");
    ASSERT(c);
    ASSERT(valid_err(c));
    free(c);
    remove("_t_dl05.toml");
}

TEST(dlist_insert_before_nonexistent_error)
{
    cfg("_t_dl06.toml");
    ds4vizDoubleLinkedList(l)
    {
        ds4vizDlInsertHead(l, 10);
        ds4vizDlInsertBefore(l, 999, 20);
    }
    char *c = read_file("_t_dl06.toml");
    ASSERT(c);
    ASSERT(valid_err(c));
    free(c);
    remove("_t_dl06.toml");
}

TEST(dlist_insert_after_nonexistent_error)
{
    cfg("_t_dl07.toml");
    ds4vizDoubleLinkedList(l)
    {
        ds4vizDlInsertHead(l, 10);
        ds4vizDlInsertAfter(l, 999, 20);
    }
    char *c = read_file("_t_dl07.toml");
    ASSERT(c);
    ASSERT(valid_err(c));
    free(c);
    remove("_t_dl07.toml");
}

TEST(dlist_delete_nonexistent_error)
{
    cfg("_t_dl08.toml");
    ds4vizDoubleLinkedList(l)
    {
        ds4vizDlInsertHead(l, 10);
        ds4vizDlDelete(l, 999);
    }
    char *c = read_file("_t_dl08.toml");
    ASSERT(c);
    ASSERT(valid_err(c));
    free(c);
    remove("_t_dl08.toml");
}

TEST(dlist_clear)
{
    cfg("_t_dl09.toml");
    ds4vizDoubleLinkedList(l)
    {
        ds4vizDlInsertTail(l, 1);
        ds4vizDlInsertTail(l, 2);
        ds4vizDlDeleteHead(l);
        ds4vizDlDeleteHead(l);
    }
    char *c = read_file("_t_dl09.toml");
    ASSERT(c);
    ASSERT(valid_ok(c));
    ASSERT(has(c, "head = -1"));
    ASSERT(has(c, "tail = -1"));
    free(c);
    remove("_t_dl09.toml");
}

TEST(dlist_single_node)
{
    cfg("_t_dl10.toml");
    ds4vizDoubleLinkedList(l)
    {
        ds4vizDlInsertHead(l, 100);
        ds4vizDlDeleteHead(l);
    }
    char *c = read_file("_t_dl10.toml");
    ASSERT(c);
    ASSERT(valid_ok(c));
    free(c);
    remove("_t_dl10.toml");
}

TEST(dlist_reverse_empty)
{
    cfg("_t_dl11.toml");
    ds4vizDoubleLinkedList(l)
    {
        ds4vizDlReverse(l);
    }
    char *c = read_file("_t_dl11.toml");
    ASSERT(c);
    ASSERT(valid_ok(c));
    free(c);
    remove("_t_dl11.toml");
}

TEST(dlist_multiple_operations)
{
    cfg("_t_dl12.toml");
    ds4vizDoubleLinkedList(l)
    {
        int n1 = ds4vizDlInsertHead(l, 1);
        int n2 = ds4vizDlInsertTail(l, 5);
        ds4vizDlInsertAfter(l, n1, 2);
        ds4vizDlInsertBefore(l, n2, 4);
        ds4vizDlDelete(l, n1);
    }
    char *c = read_file("_t_dl12.toml");
    ASSERT(c);
    ASSERT(valid_ok(c));
    ASSERT(count_str(c, "[[steps]]") == 5);
    free(c);
    remove("_t_dl12.toml");
}

/* ================================================================
 *  BINARY TREE TESTS
 * ================================================================ */

TEST(bt_basic_operations)
{
    cfg("_t_bt01.toml");
    ds4vizBinaryTree(t, "test_btree")
    {
        int root = ds4vizBtInsertRoot(t, 10);
        int left = ds4vizBtInsertLeft(t, root, 5);
        ds4vizBtInsertRight(t, root, 15);
        ds4vizBtInsertLeft(t, left, 3);
        ds4vizBtInsertRight(t, left, 7);
    }
    char *c = read_file("_t_bt01.toml");
    ASSERT(c);
    ASSERT(valid_ok(c));
    ASSERT(has(c, "type = \"binary_tree\""));
    ASSERT(has(c, "label = \"test_btree\""));
    free(c);
    remove("_t_bt01.toml");
}

TEST(bt_delete_subtree)
{
    cfg("_t_bt02.toml");
    ds4vizBinaryTree(t)
    {
        int root = ds4vizBtInsertRoot(t, 10);
        int left = ds4vizBtInsertLeft(t, root, 5);
        ds4vizBtInsertLeft(t, left, 3);
        ds4vizBtInsertRight(t, left, 7);
        ds4vizBtDelete(t, left);
    }
    char *c = read_file("_t_bt02.toml");
    ASSERT(c);
    ASSERT(valid_ok(c));
    free(c);
    remove("_t_bt02.toml");
}

TEST(bt_update_value)
{
    cfg("_t_bt03.toml");
    ds4vizBinaryTree(t)
    {
        int root = ds4vizBtInsertRoot(t, 10);
        ds4vizBtUpdateValue(t, root, 100);
    }
    char *c = read_file("_t_bt03.toml");
    ASSERT(c);
    ASSERT(valid_ok(c));
    ASSERT(has(c, "100"));
    free(c);
    remove("_t_bt03.toml");
}

TEST(bt_duplicate_root_error)
{
    cfg("_t_bt04.toml");
    ds4vizBinaryTree(t)
    {
        ds4vizBtInsertRoot(t, 10);
        ds4vizBtInsertRoot(t, 20);
    }
    char *c = read_file("_t_bt04.toml");
    ASSERT(c);
    ASSERT(valid_err(c));
    free(c);
    remove("_t_bt04.toml");
}

TEST(bt_duplicate_child_error)
{
    cfg("_t_bt05.toml");
    ds4vizBinaryTree(t)
    {
        int root = ds4vizBtInsertRoot(t, 10);
        ds4vizBtInsertLeft(t, root, 5);
        ds4vizBtInsertLeft(t, root, 6);
    }
    char *c = read_file("_t_bt05.toml");
    ASSERT(c);
    ASSERT(valid_err(c));
    free(c);
    remove("_t_bt05.toml");
}

TEST(bt_nonexistent_parent_error)
{
    cfg("_t_bt06.toml");
    ds4vizBinaryTree(t)
    {
        ds4vizBtInsertRoot(t, 10);
        ds4vizBtInsertLeft(t, 999, 5);
    }
    char *c = read_file("_t_bt06.toml");
    ASSERT(c);
    ASSERT(valid_err(c));
    free(c);
    remove("_t_bt06.toml");
}

TEST(bt_delete_root)
{
    cfg("_t_bt07.toml");
    ds4vizBinaryTree(t)
    {
        int root = ds4vizBtInsertRoot(t, 10);
        ds4vizBtInsertLeft(t, root, 5);
        ds4vizBtDelete(t, root);
    }
    char *c = read_file("_t_bt07.toml");
    ASSERT(c);
    ASSERT(valid_ok(c));
    ASSERT(has(c, "root = -1"));
    free(c);
    remove("_t_bt07.toml");
}

TEST(bt_delete_nonexistent_error)
{
    cfg("_t_bt08.toml");
    ds4vizBinaryTree(t)
    {
        ds4vizBtInsertRoot(t, 10);
        ds4vizBtDelete(t, 999);
    }
    char *c = read_file("_t_bt08.toml");
    ASSERT(c);
    ASSERT(valid_err(c));
    free(c);
    remove("_t_bt08.toml");
}

TEST(bt_update_nonexistent_error)
{
    cfg("_t_bt09.toml");
    ds4vizBinaryTree(t)
    {
        ds4vizBtInsertRoot(t, 10);
        ds4vizBtUpdateValue(t, 999, 100);
    }
    char *c = read_file("_t_bt09.toml");
    ASSERT(c);
    ASSERT(valid_err(c));
    free(c);
    remove("_t_bt09.toml");
}

TEST(bt_clear)
{
    cfg("_t_bt10.toml");
    ds4vizBinaryTree(t)
    {
        int root = ds4vizBtInsertRoot(t, 10);
        ds4vizBtInsertLeft(t, root, 5);
        ds4vizBtDelete(t, root);
    }
    char *c = read_file("_t_bt10.toml");
    ASSERT(c);
    ASSERT(valid_ok(c));
    free(c);
    remove("_t_bt10.toml");
}

TEST(bt_complete_tree)
{
    cfg("_t_bt11.toml");
    ds4vizBinaryTree(t)
    {
        int root = ds4vizBtInsertRoot(t, 1);
        int left = ds4vizBtInsertLeft(t, root, 2);
        int right = ds4vizBtInsertRight(t, root, 3);
        ds4vizBtInsertLeft(t, left, 4);
        ds4vizBtInsertRight(t, left, 5);
        ds4vizBtInsertLeft(t, right, 6);
        ds4vizBtInsertRight(t, right, 7);
    }
    char *c = read_file("_t_bt11.toml");
    ASSERT(c);
    ASSERT(valid_ok(c));
    ASSERT(count_str(c, "[[steps]]") == 7);
    free(c);
    remove("_t_bt11.toml");
}

/* ================================================================
 *  BST TESTS
 * ================================================================ */

TEST(bst_basic_operations)
{
    cfg("_t_bst01.toml");
    ds4vizBinarySearchTree(b, "test_bst")
    {
        ds4vizBstInsert(b, 10);
        ds4vizBstInsert(b, 5);
        ds4vizBstInsert(b, 15);
        ds4vizBstInsert(b, 3);
        ds4vizBstInsert(b, 7);
    }
    char *c = read_file("_t_bst01.toml");
    ASSERT(c);
    ASSERT(valid_ok(c));
    ASSERT(has(c, "type = \"bst\""));
    ASSERT(has(c, "label = \"test_bst\""));
    free(c);
    remove("_t_bst01.toml");
}

TEST(bst_delete_leaf)
{
    cfg("_t_bst02.toml");
    ds4vizBinarySearchTree(b)
    {
        ds4vizBstInsert(b, 10);
        ds4vizBstInsert(b, 5);
        ds4vizBstInsert(b, 15);
        ds4vizBstDelete(b, 5);
    }
    char *c = read_file("_t_bst02.toml");
    ASSERT(c);
    ASSERT(valid_ok(c));
    free(c);
    remove("_t_bst02.toml");
}

TEST(bst_delete_one_child)
{
    cfg("_t_bst03.toml");
    ds4vizBinarySearchTree(b)
    {
        ds4vizBstInsert(b, 10);
        ds4vizBstInsert(b, 5);
        ds4vizBstInsert(b, 3);
        ds4vizBstDelete(b, 5);
    }
    char *c = read_file("_t_bst03.toml");
    ASSERT(c);
    ASSERT(valid_ok(c));
    free(c);
    remove("_t_bst03.toml");
}

TEST(bst_delete_two_children)
{
    cfg("_t_bst04.toml");
    ds4vizBinarySearchTree(b)
    {
        ds4vizBstInsert(b, 10);
        ds4vizBstInsert(b, 5);
        ds4vizBstInsert(b, 15);
        ds4vizBstInsert(b, 3);
        ds4vizBstInsert(b, 7);
        ds4vizBstDelete(b, 5);
    }
    char *c = read_file("_t_bst04.toml");
    ASSERT(c);
    ASSERT(valid_ok(c));
    free(c);
    remove("_t_bst04.toml");
}

TEST(bst_delete_nonexistent_error)
{
    cfg("_t_bst05.toml");
    ds4vizBinarySearchTree(b)
    {
        ds4vizBstInsert(b, 10);
        ds4vizBstDelete(b, 999);
    }
    char *c = read_file("_t_bst05.toml");
    ASSERT(c);
    ASSERT(valid_err(c));
    free(c);
    remove("_t_bst05.toml");
}

TEST(bst_delete_root)
{
    cfg("_t_bst06.toml");
    ds4vizBinarySearchTree(b)
    {
        ds4vizBstInsert(b, 10);
        ds4vizBstInsert(b, 5);
        ds4vizBstInsert(b, 15);
        ds4vizBstDelete(b, 10);
    }
    char *c = read_file("_t_bst06.toml");
    ASSERT(c);
    ASSERT(valid_ok(c));
    free(c);
    remove("_t_bst06.toml");
}

TEST(bst_insert_duplicate)
{
    cfg("_t_bst07.toml");
    ds4vizBinarySearchTree(b)
    {
        ds4vizBstInsert(b, 10);
        ds4vizBstInsert(b, 5);
        ds4vizBstInsert(b, 10);
    }
    char *c = read_file("_t_bst07.toml");
    ASSERT(c);
    ASSERT(valid_ok(c));
    free(c);
    remove("_t_bst07.toml");
}

TEST(bst_clear)
{
    cfg("_t_bst08.toml");
    ds4vizBinarySearchTree(b)
    {
        ds4vizBstInsert(b, 10);
        ds4vizBstInsert(b, 5);
        ds4vizBstDelete(b, 10);
        ds4vizBstDelete(b, 5);
    }
    char *c = read_file("_t_bst08.toml");
    ASSERT(c);
    ASSERT(valid_ok(c));
    ASSERT(has(c, "root = -1"));
    free(c);
    remove("_t_bst08.toml");
}

TEST(bst_left_skewed)
{
    cfg("_t_bst09.toml");
    ds4vizBinarySearchTree(b)
    {
        for (int i = 5; i >= 1; i--)
            ds4vizBstInsert(b, i);
    }
    char *c = read_file("_t_bst09.toml");
    ASSERT(c);
    ASSERT(valid_ok(c));
    ASSERT(count_str(c, "[[steps]]") == 5);
    free(c);
    remove("_t_bst09.toml");
}

TEST(bst_right_skewed)
{
    cfg("_t_bst10.toml");
    ds4vizBinarySearchTree(b)
    {
        for (int i = 1; i <= 5; i++)
            ds4vizBstInsert(b, i);
    }
    char *c = read_file("_t_bst10.toml");
    ASSERT(c);
    ASSERT(valid_ok(c));
    ASSERT(count_str(c, "[[steps]]") == 5);
    free(c);
    remove("_t_bst10.toml");
}

/* ================================================================
 *  HEAP TESTS
 * ================================================================ */

TEST(heap_min_basic)
{
    cfg("_t_h01.toml");
    ds4vizHeap(h, "test_heap", ds4vizHeapOrderMin)
    {
        ds4vizHeapInsert(h, 10);
        ds4vizHeapInsert(h, 5);
        ds4vizHeapInsert(h, 15);
        ds4vizHeapInsert(h, 3);
        ds4vizHeapExtract(h);
    }
    char *c = read_file("_t_h01.toml");
    ASSERT(c);
    ASSERT(valid_ok(c));
    ASSERT(has(c, "type = \"heap\""));
    ASSERT(has(c, "label = \"test_heap\""));
    ASSERT(has(c, "order = \"min\""));
    free(c);
    remove("_t_h01.toml");
}

TEST(heap_max_basic)
{
    cfg("_t_h02.toml");
    ds4vizHeap(h, "heap", ds4vizHeapOrderMax)
    {
        ds4vizHeapInsert(h, 10);
        ds4vizHeapInsert(h, 20);
        ds4vizHeapInsert(h, 5);
        ds4vizHeapExtract(h);
    }
    char *c = read_file("_t_h02.toml");
    ASSERT(c);
    ASSERT(valid_ok(c));
    ASSERT(has(c, "order = \"max\""));
    free(c);
    remove("_t_h02.toml");
}

TEST(heap_extract_empty_error)
{
    cfg("_t_h03.toml");
    ds4vizHeap(h)
    {
        ds4vizHeapExtract(h);
    }
    char *c = read_file("_t_h03.toml");
    ASSERT(c);
    ASSERT(valid_err(c));
    free(c);
    remove("_t_h03.toml");
}

TEST(heap_clear)
{
    cfg("_t_h04.toml");
    ds4vizHeap(h)
    {
        ds4vizHeapInsert(h, 1);
        ds4vizHeapInsert(h, 2);
        ds4vizHeapInsert(h, 3);
        ds4vizHeapClear(h);
    }
    char *c = read_file("_t_h04.toml");
    ASSERT(c);
    ASSERT(valid_ok(c));
    ASSERT(has(c, "elements = []"));
    free(c);
    remove("_t_h04.toml");
}

TEST(heap_multiple_extract)
{
    cfg("_t_h05.toml");
    ds4vizHeap(h, "heap", ds4vizHeapOrderMin)
    {
        ds4vizHeapInsert(h, 10);
        ds4vizHeapInsert(h, 5);
        ds4vizHeapInsert(h, 15);
        ds4vizHeapInsert(h, 3);
        ds4vizHeapExtract(h);
        ds4vizHeapExtract(h);
    }
    char *c = read_file("_t_h05.toml");
    ASSERT(c);
    ASSERT(valid_ok(c));
    ASSERT(count_str(c, "[[steps]]") == 6);
    free(c);
    remove("_t_h05.toml");
}

TEST(heap_single_element)
{
    cfg("_t_h06.toml");
    ds4vizHeap(h)
    {
        ds4vizHeapInsert(h, 100);
        ds4vizHeapExtract(h);
    }
    char *c = read_file("_t_h06.toml");
    ASSERT(c);
    ASSERT(valid_ok(c));
    free(c);
    remove("_t_h06.toml");
}

TEST(heap_insert_after_extract)
{
    cfg("_t_h07.toml");
    ds4vizHeap(h, "heap", ds4vizHeapOrderMin)
    {
        ds4vizHeapInsert(h, 10);
        ds4vizHeapInsert(h, 5);
        ds4vizHeapExtract(h);
        ds4vizHeapInsert(h, 3);
    }
    char *c = read_file("_t_h07.toml");
    ASSERT(c);
    ASSERT(valid_ok(c));
    free(c);
    remove("_t_h07.toml");
}

TEST(heap_min_property)
{
    cfg("_t_h08.toml");
    ds4vizHeap(h, "heap", ds4vizHeapOrderMin)
    {
        int vals[] = {15, 10, 20, 8, 25, 12};
        for (int i = 0; i < 6; i++)
            ds4vizHeapInsert(h, vals[i]);
    }
    char *c = read_file("_t_h08.toml");
    ASSERT(c);
    ASSERT(valid_ok(c));
    free(c);
    remove("_t_h08.toml");
}

TEST(heap_max_property)
{
    cfg("_t_h09.toml");
    ds4vizHeap(h, "heap", ds4vizHeapOrderMax)
    {
        int vals[] = {15, 10, 20, 8, 25, 12};
        for (int i = 0; i < 6; i++)
            ds4vizHeapInsert(h, vals[i]);
    }
    char *c = read_file("_t_h09.toml");
    ASSERT(c);
    ASSERT(valid_ok(c));
    free(c);
    remove("_t_h09.toml");
}

/* ================================================================
 *  GRAPH UNDIRECTED TESTS
 * ================================================================ */

TEST(gu_basic)
{
    cfg("_t_gu01.toml");
    ds4vizGraphUndirected(g, "test_graph")
    {
        ds4vizGuAddNode(g, 0, "A");
        ds4vizGuAddNode(g, 1, "B");
        ds4vizGuAddNode(g, 2, "C");
        ds4vizGuAddEdge(g, 0, 1);
        ds4vizGuAddEdge(g, 1, 2);
    }
    char *c = read_file("_t_gu01.toml");
    ASSERT(c);
    ASSERT(valid_ok(c));
    ASSERT(has(c, "type = \"graph_undirected\""));
    ASSERT(has(c, "label = \"test_graph\""));
    free(c);
    remove("_t_gu01.toml");
}

TEST(gu_edge_normalization)
{
    cfg("_t_gu02.toml");
    ds4vizGraphUndirected(g)
    {
        ds4vizGuAddNode(g, 0, "A");
        ds4vizGuAddNode(g, 1, "B");
        ds4vizGuAddEdge(g, 1, 0); /* reversed — should normalize */
    }
    char *c = read_file("_t_gu02.toml");
    ASSERT(c);
    ASSERT(valid_ok(c));
    ASSERT(has(c, "from = 0"));
    ASSERT(has(c, "to = 1"));
    free(c);
    remove("_t_gu02.toml");
}

TEST(gu_self_loop_error)
{
    cfg("_t_gu03.toml");
    ds4vizGraphUndirected(g)
    {
        ds4vizGuAddNode(g, 0, "A");
        ds4vizGuAddEdge(g, 0, 0);
    }
    char *c = read_file("_t_gu03.toml");
    ASSERT(c);
    ASSERT(valid_err(c));
    free(c);
    remove("_t_gu03.toml");
}

TEST(gu_duplicate_edge_error)
{
    cfg("_t_gu04.toml");
    ds4vizGraphUndirected(g)
    {
        ds4vizGuAddNode(g, 0, "A");
        ds4vizGuAddNode(g, 1, "B");
        ds4vizGuAddEdge(g, 0, 1);
        ds4vizGuAddEdge(g, 0, 1);
    }
    char *c = read_file("_t_gu04.toml");
    ASSERT(c);
    ASSERT(valid_err(c));
    free(c);
    remove("_t_gu04.toml");
}

TEST(gu_remove_node)
{
    cfg("_t_gu05.toml");
    ds4vizGraphUndirected(g)
    {
        ds4vizGuAddNode(g, 0, "A");
        ds4vizGuAddNode(g, 1, "B");
        ds4vizGuAddNode(g, 2, "C");
        ds4vizGuAddEdge(g, 0, 1);
        ds4vizGuAddEdge(g, 1, 2);
        ds4vizGuRemoveNode(g, 1);
    }
    char *c = read_file("_t_gu05.toml");
    ASSERT(c);
    ASSERT(valid_ok(c));
    free(c);
    remove("_t_gu05.toml");
}

TEST(gu_remove_edge)
{
    cfg("_t_gu06.toml");
    ds4vizGraphUndirected(g)
    {
        ds4vizGuAddNode(g, 0, "A");
        ds4vizGuAddNode(g, 1, "B");
        ds4vizGuAddEdge(g, 0, 1);
        ds4vizGuRemoveEdge(g, 0, 1);
    }
    char *c = read_file("_t_gu06.toml");
    ASSERT(c);
    ASSERT(valid_ok(c));
    free(c);
    remove("_t_gu06.toml");
}

TEST(gu_duplicate_node_error)
{
    cfg("_t_gu07.toml");
    ds4vizGraphUndirected(g)
    {
        ds4vizGuAddNode(g, 0, "A");
        ds4vizGuAddNode(g, 0, "B");
    }
    char *c = read_file("_t_gu07.toml");
    ASSERT(c);
    ASSERT(valid_err(c));
    free(c);
    remove("_t_gu07.toml");
}

TEST(gu_remove_nonexistent_node_error)
{
    cfg("_t_gu08.toml");
    ds4vizGraphUndirected(g)
    {
        ds4vizGuAddNode(g, 0, "A");
        ds4vizGuRemoveNode(g, 999);
    }
    char *c = read_file("_t_gu08.toml");
    ASSERT(c);
    ASSERT(valid_err(c));
    free(c);
    remove("_t_gu08.toml");
}

TEST(gu_remove_nonexistent_edge_error)
{
    cfg("_t_gu09.toml");
    ds4vizGraphUndirected(g)
    {
        ds4vizGuAddNode(g, 0, "A");
        ds4vizGuAddNode(g, 1, "B");
        ds4vizGuRemoveEdge(g, 0, 1);
    }
    char *c = read_file("_t_gu09.toml");
    ASSERT(c);
    ASSERT(valid_err(c));
    free(c);
    remove("_t_gu09.toml");
}

TEST(gu_clear)
{
    cfg("_t_gu10.toml");
    ds4vizGraphUndirected(g)
    {
        ds4vizGuAddNode(g, 0, "A");
        ds4vizGuAddNode(g, 1, "B");
        ds4vizGuRemoveNode(g, 0);
        ds4vizGuRemoveNode(g, 1);
    }
    char *c = read_file("_t_gu10.toml");
    ASSERT(c);
    ASSERT(valid_ok(c));
    ASSERT(has(c, "nodes = []"));
    ASSERT(has(c, "edges = []"));
    free(c);
    remove("_t_gu10.toml");
}

/* ================================================================
 *  GRAPH DIRECTED TESTS
 * ================================================================ */

TEST(gd_basic)
{
    cfg("_t_gd01.toml");
    ds4vizGraphDirected(g, "test_digraph")
    {
        ds4vizGdAddNode(g, 0, "A");
        ds4vizGdAddNode(g, 1, "B");
        ds4vizGdAddEdge(g, 0, 1);
        ds4vizGdAddEdge(g, 1, 0);
    }
    char *c = read_file("_t_gd01.toml");
    ASSERT(c);
    ASSERT(valid_ok(c));
    ASSERT(has(c, "type = \"graph_directed\""));
    ASSERT(has(c, "label = \"test_digraph\""));
    free(c);
    remove("_t_gd01.toml");
}

TEST(gd_no_edge_normalization)
{
    cfg("_t_gd02.toml");
    ds4vizGraphDirected(g)
    {
        ds4vizGdAddNode(g, 0, "A");
        ds4vizGdAddNode(g, 1, "B");
        ds4vizGdAddEdge(g, 1, 0);
    }
    char *c = read_file("_t_gd02.toml");
    ASSERT(c);
    ASSERT(valid_ok(c));
    ASSERT(has(c, "from = 1"));
    ASSERT(has(c, "to = 0"));
    free(c);
    remove("_t_gd02.toml");
}

TEST(gd_self_loop_error)
{
    cfg("_t_gd03.toml");
    ds4vizGraphDirected(g)
    {
        ds4vizGdAddNode(g, 0, "A");
        ds4vizGdAddEdge(g, 0, 0);
    }
    char *c = read_file("_t_gd03.toml");
    ASSERT(c);
    ASSERT(valid_err(c));
    free(c);
    remove("_t_gd03.toml");
}

TEST(gd_duplicate_edge_error)
{
    cfg("_t_gd04.toml");
    ds4vizGraphDirected(g)
    {
        ds4vizGdAddNode(g, 0, "A");
        ds4vizGdAddNode(g, 1, "B");
        ds4vizGdAddEdge(g, 0, 1);
        ds4vizGdAddEdge(g, 0, 1);
    }
    char *c = read_file("_t_gd04.toml");
    ASSERT(c);
    ASSERT(valid_err(c));
    free(c);
    remove("_t_gd04.toml");
}

TEST(gd_remove_node)
{
    cfg("_t_gd05.toml");
    ds4vizGraphDirected(g)
    {
        ds4vizGdAddNode(g, 0, "A");
        ds4vizGdAddNode(g, 1, "B");
        ds4vizGdAddNode(g, 2, "C");
        ds4vizGdAddEdge(g, 0, 1);
        ds4vizGdAddEdge(g, 1, 2);
        ds4vizGdRemoveNode(g, 1);
    }
    char *c = read_file("_t_gd05.toml");
    ASSERT(c);
    ASSERT(valid_ok(c));
    free(c);
    remove("_t_gd05.toml");
}

TEST(gd_remove_edge)
{
    cfg("_t_gd06.toml");
    ds4vizGraphDirected(g)
    {
        ds4vizGdAddNode(g, 0, "A");
        ds4vizGdAddNode(g, 1, "B");
        ds4vizGdAddEdge(g, 0, 1);
        ds4vizGdRemoveEdge(g, 0, 1);
    }
    char *c = read_file("_t_gd06.toml");
    ASSERT(c);
    ASSERT(valid_ok(c));
    free(c);
    remove("_t_gd06.toml");
}

TEST(gd_clear)
{
    cfg("_t_gd07.toml");
    ds4vizGraphDirected(g)
    {
        ds4vizGdAddNode(g, 0, "A");
        ds4vizGdAddNode(g, 1, "B");
        ds4vizGdRemoveNode(g, 0);
        ds4vizGdRemoveNode(g, 1);
    }
    char *c = read_file("_t_gd07.toml");
    ASSERT(c);
    ASSERT(valid_ok(c));
    free(c);
    remove("_t_gd07.toml");
}

/* ================================================================
 *  GRAPH WEIGHTED TESTS
 * ================================================================ */

TEST(gw_undirected)
{
    cfg("_t_gw01.toml");
    ds4vizGraphWeighted(g, "test_wgraph")
    {
        ds4vizGwAddNode(g, 0, "A");
        ds4vizGwAddNode(g, 1, "B");
        ds4vizGwAddEdge(g, 0, 1, 3.5);
    }
    char *c = read_file("_t_gw01.toml");
    ASSERT(c);
    ASSERT(valid_ok(c));
    ASSERT(has(c, "type = \"graph_weighted\""));
    ASSERT(has(c, "label = \"test_wgraph\""));
    ASSERT(has(c, "3.5"));
    free(c);
    remove("_t_gw01.toml");
}

TEST(gw_directed)
{
    cfg("_t_gw02.toml");
    ds4vizGraphWeighted(g, "graph", true)
    {
        ds4vizGwAddNode(g, 0, "A");
        ds4vizGwAddNode(g, 1, "B");
        ds4vizGwAddEdge(g, 0, 1, 2.0);
        ds4vizGwAddEdge(g, 1, 0, 3.0);
    }
    char *c = read_file("_t_gw02.toml");
    ASSERT(c);
    ASSERT(valid_ok(c));
    ASSERT(has(c, "directed = true"));
    free(c);
    remove("_t_gw02.toml");
}

TEST(gw_update_weight)
{
    cfg("_t_gw03.toml");
    ds4vizGraphWeighted(g)
    {
        ds4vizGwAddNode(g, 0, "A");
        ds4vizGwAddNode(g, 1, "B");
        ds4vizGwAddEdge(g, 0, 1, 1.0);
        ds4vizGwUpdateWeight(g, 0, 1, 5.0);
    }
    char *c = read_file("_t_gw03.toml");
    ASSERT(c);
    ASSERT(valid_ok(c));
    free(c);
    remove("_t_gw03.toml");
}

TEST(gw_update_node_label)
{
    cfg("_t_gw04.toml");
    ds4vizGraphWeighted(g)
    {
        ds4vizGwAddNode(g, 0, "A");
        ds4vizGwUpdateNodeLabel(g, 0, "X");
    }
    char *c = read_file("_t_gw04.toml");
    ASSERT(c);
    ASSERT(valid_ok(c));
    ASSERT(has(c, "\"X\""));
    free(c);
    remove("_t_gw04.toml");
}

TEST(gw_negative_weight)
{
    cfg("_t_gw05.toml");
    ds4vizGraphWeighted(g)
    {
        ds4vizGwAddNode(g, 0, "A");
        ds4vizGwAddNode(g, 1, "B");
        ds4vizGwAddEdge(g, 0, 1, -5.5);
    }
    char *c = read_file("_t_gw05.toml");
    ASSERT(c);
    ASSERT(valid_ok(c));
    ASSERT(has(c, "-5.5"));
    free(c);
    remove("_t_gw05.toml");
}

TEST(gw_zero_weight)
{
    cfg("_t_gw06.toml");
    ds4vizGraphWeighted(g)
    {
        ds4vizGwAddNode(g, 0, "A");
        ds4vizGwAddNode(g, 1, "B");
        ds4vizGwAddEdge(g, 0, 1, 0.0);
    }
    char *c = read_file("_t_gw06.toml");
    ASSERT(c);
    ASSERT(valid_ok(c));
    free(c);
    remove("_t_gw06.toml");
}

TEST(gw_update_nonexistent_edge_error)
{
    cfg("_t_gw07.toml");
    ds4vizGraphWeighted(g)
    {
        ds4vizGwAddNode(g, 0, "A");
        ds4vizGwAddNode(g, 1, "B");
        ds4vizGwUpdateWeight(g, 0, 1, 5.0);
    }
    char *c = read_file("_t_gw07.toml");
    ASSERT(c);
    ASSERT(valid_err(c));
    free(c);
    remove("_t_gw07.toml");
}

TEST(gw_remove_edge)
{
    cfg("_t_gw08.toml");
    ds4vizGraphWeighted(g)
    {
        ds4vizGwAddNode(g, 0, "A");
        ds4vizGwAddNode(g, 1, "B");
        ds4vizGwAddEdge(g, 0, 1, 1.0);
        ds4vizGwRemoveEdge(g, 0, 1);
    }
    char *c = read_file("_t_gw08.toml");
    ASSERT(c);
    ASSERT(valid_ok(c));
    free(c);
    remove("_t_gw08.toml");
}

TEST(gw_clear)
{
    cfg("_t_gw09.toml");
    ds4vizGraphWeighted(g)
    {
        ds4vizGwAddNode(g, 0, "A");
        ds4vizGwAddNode(g, 1, "B");
        ds4vizGwRemoveNode(g, 0);
        ds4vizGwRemoveNode(g, 1);
    }
    char *c = read_file("_t_gw09.toml");
    ASSERT(c);
    ASSERT(valid_ok(c));
    free(c);
    remove("_t_gw09.toml");
}

/* ================================================================
 *  CONFIG TESTS
 * ================================================================ */

TEST(config_remarks)
{
    cfg_full("_t_cfg01.toml", "Test Title", "Test Author", "Test Comment");
    ds4vizStack(s)
    {
        ds4vizStackPush(s, 1);
    }
    char *c = read_file("_t_cfg01.toml");
    ASSERT(c);
    ASSERT(valid_ok(c));
    ASSERT(has(c, "title = \"Test Title\""));
    ASSERT(has(c, "author = \"Test Author\""));
    ASSERT(has(c, "comment = \"Test Comment\""));
    free(c);
    remove("_t_cfg01.toml");
}

TEST(config_output_path)
{
    cfg("_t_cfg02.toml");
    ds4vizStack(s)
    {
        ds4vizStackPush(s, 1);
    }
    char *c = read_file("_t_cfg02.toml");
    ASSERT(c);
    ASSERT(valid_ok(c));
    free(c);
    remove("_t_cfg02.toml");
}

TEST(config_only_title)
{
    cfg_full("_t_cfg03.toml", "Only Title", NULL, NULL);
    ds4vizStack(s)
    {
        ds4vizStackPush(s, 1);
    }
    char *c = read_file("_t_cfg03.toml");
    ASSERT(c);
    ASSERT(has(c, "title = \"Only Title\""));
    ASSERT(!has(c, "author ="));
    ASSERT(!has(c, "comment ="));
    free(c);
    remove("_t_cfg03.toml");
}

TEST(config_only_author)
{
    cfg_full("_t_cfg04.toml", NULL, "Only Author", NULL);
    ds4vizStack(s)
    {
        ds4vizStackPush(s, 1);
    }
    char *c = read_file("_t_cfg04.toml");
    ASSERT(c);
    ASSERT(has(c, "author = \"Only Author\""));
    ASSERT(!has(c, "title ="));
    ASSERT(!has(c, "comment ="));
    free(c);
    remove("_t_cfg04.toml");
}

TEST(config_only_comment)
{
    cfg_full("_t_cfg05.toml", NULL, NULL, "Only Comment");
    ds4vizStack(s)
    {
        ds4vizStackPush(s, 1);
    }
    char *c = read_file("_t_cfg05.toml");
    ASSERT(c);
    ASSERT(has(c, "comment = \"Only Comment\""));
    ASSERT(!has(c, "title ="));
    ASSERT(!has(c, "author ="));
    free(c);
    remove("_t_cfg05.toml");
}

/* ================================================================
 *  EDGE CASE TESTS
 * ================================================================ */

TEST(edge_empty_structure)
{
    cfg("_t_ec01.toml");
    ds4vizStack(s)
    {
        /* nothing */
    }
    char *c = read_file("_t_ec01.toml");
    ASSERT(c);
    ASSERT(valid_ok(c));
    ASSERT(count_str(c, "[[states]]") == 1);
    ASSERT(count_str(c, "[[steps]]") == 0);
    free(c);
    remove("_t_ec01.toml");
}

TEST(edge_special_chars_string)
{
    cfg("_t_ec02.toml");
    ds4vizStack(s)
    {
        ds4vizStackPush(s, "hello\"world");
        ds4vizStackPush(s, "line1\\nline2");
    }
    char *c = read_file("_t_ec02.toml");
    ASSERT(c);
    ASSERT(valid_ok(c));
    /* Escaped quotes and backslashes should be in the TOML */
    ASSERT(has(c, "\\\""));
    ASSERT(has(c, "\\\\"));
    free(c);
    remove("_t_ec02.toml");
}

TEST(edge_large_numbers)
{
    cfg("_t_ec03.toml");
    ds4vizStack(s)
    {
        ds4vizStackPush(s, (long long)1000000000000000000LL);
        ds4vizStackPush(s, (long long)-1000000000000000000LL);
        ds4vizStackPush(s, 1.7976931348623157e308);
    }
    char *c = read_file("_t_ec03.toml");
    ASSERT(c);
    ASSERT(valid_ok(c));
    free(c);
    remove("_t_ec03.toml");
}

TEST(edge_many_operations)
{
    cfg("_t_ec04.toml");
    ds4vizStack(s)
    {
        for (int i = 0; i < 100; i++)
            ds4vizStackPush(s, i);
        for (int i = 0; i < 50; i++)
            ds4vizStackPop(s);
    }
    char *c = read_file("_t_ec04.toml");
    ASSERT(c);
    ASSERT(valid_ok(c));
    ASSERT(count_str(c, "[[states]]") == 151);
    ASSERT(count_str(c, "[[steps]]") == 150);
    free(c);
    remove("_t_ec04.toml");
}

TEST(edge_node_label_boundary)
{
    cfg("_t_ec05.toml");
    ds4vizGraphUndirected(g)
    {
        ds4vizGuAddNode(g, 0, "A");
        ds4vizGuAddNode(g, 1, "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"); /* 32 A's */
    }
    char *c = read_file("_t_ec05.toml");
    ASSERT(c);
    ASSERT(valid_ok(c));
    free(c);
    remove("_t_ec05.toml");
}

TEST(edge_mixed_types)
{
    cfg("_t_ec06.toml");
    ds4vizStack(s)
    {
        ds4vizStackPush(s, 1);
        ds4vizStackPush(s, 2.5);
        ds4vizStackPush(s, "text");
        ds4vizStackPush(s, true);
        ds4vizStackPush(s, false);
        ds4vizStackPop(s);
    }
    char *c = read_file("_t_ec06.toml");
    ASSERT(c);
    ASSERT(valid_ok(c));
    free(c);
    remove("_t_ec06.toml");
}

/* ================================================================
 *  COMPLEX SCENARIO TESTS
 * ================================================================ */

TEST(complex_bst_deletion)
{
    cfg("_t_cx01.toml");
    ds4vizBinarySearchTree(b)
    {
        int vals[] = {50, 30, 70, 20, 40, 60, 80, 35, 45};
        for (int i = 0; i < 9; i++)
            ds4vizBstInsert(b, vals[i]);
        ds4vizBstDelete(b, 30);
        ds4vizBstDelete(b, 50);
    }
    char *c = read_file("_t_cx01.toml");
    ASSERT(c);
    ASSERT(valid_ok(c));
    ASSERT(count_str(c, "[[steps]]") == 11);
    free(c);
    remove("_t_cx01.toml");
}

TEST(complex_graph_operations)
{
    cfg("_t_cx02.toml");
    ds4vizGraphWeighted(g, "graph", true)
    {
        for (int i = 0; i < 5; i++)
        {
            char lbl[2] = {(char)('A' + i), '\0'};
            ds4vizGwAddNode(g, i, lbl);
        }
        ds4vizGwAddEdge(g, 0, 1, 1.0);
        ds4vizGwAddEdge(g, 0, 2, 2.0);
        ds4vizGwAddEdge(g, 1, 2, 1.5);
        ds4vizGwAddEdge(g, 2, 3, 2.5);
        ds4vizGwAddEdge(g, 3, 4, 1.0);
        ds4vizGwAddEdge(g, 4, 0, 3.0);
        ds4vizGwUpdateWeight(g, 0, 1, 10.0);
        ds4vizGwRemoveNode(g, 2);
        ds4vizGwUpdateNodeLabel(g, 0, "Start");
    }
    char *c = read_file("_t_cx02.toml");
    ASSERT(c);
    ASSERT(valid_ok(c));
    ASSERT(has(c, "\"Start\""));
    free(c);
    remove("_t_cx02.toml");
}

TEST(complex_linked_list_operations)
{
    cfg("_t_cx03.toml");
    ds4vizDoubleLinkedList(l)
    {
        int nodes[5];
        for (int i = 0; i < 5; i++)
            nodes[i] = ds4vizDlInsertTail(l, i);
        ds4vizDlInsertAfter(l, nodes[2], 100);
        ds4vizDlInsertBefore(l, nodes[2], 200);
        ds4vizDlDeleteHead(l);
        ds4vizDlDeleteTail(l);
        ds4vizDlReverse(l);
    }
    char *c = read_file("_t_cx03.toml");
    ASSERT(c);
    ASSERT(valid_ok(c));
    ASSERT(count_str(c, "[[steps]]") == 10);
    free(c);
    remove("_t_cx03.toml");
}

TEST(complex_heap_sort)
{
    cfg("_t_cx04.toml");
    ds4vizHeap(h, "heap", ds4vizHeapOrderMin)
    {
        int vals[] = {5, 2, 8, 1, 9, 3};
        for (int i = 0; i < 6; i++)
            ds4vizHeapInsert(h, vals[i]);
        for (int i = 0; i < 6; i++)
            ds4vizHeapExtract(h);
    }
    char *c = read_file("_t_cx04.toml");
    ASSERT(c);
    ASSERT(valid_ok(c));
    ASSERT(count_str(c, "[[steps]]") == 12);
    ASSERT(has(c, "elements = []"));
    free(c);
    remove("_t_cx04.toml");
}

TEST(complex_graph_cycle)
{
    cfg("_t_cx05.toml");
    ds4vizGraphDirected(g)
    {
        for (int i = 0; i < 4; i++)
        {
            char lbl[8];
            snprintf(lbl, sizeof lbl, "Node%d", i);
            ds4vizGdAddNode(g, i, lbl);
        }
        ds4vizGdAddEdge(g, 0, 1);
        ds4vizGdAddEdge(g, 1, 2);
        ds4vizGdAddEdge(g, 2, 3);
        ds4vizGdAddEdge(g, 3, 1);
    }
    char *c = read_file("_t_cx05.toml");
    ASSERT(c);
    ASSERT(valid_ok(c));
    free(c);
    remove("_t_cx05.toml");
}

/* ================================================================
 *  SHORT NAMES SMOKE TEST
 *  (compile-time check — just make sure the aliases expand)
 * ================================================================ */

#define DS4VIZ_SHORT_NAMES
/* Re-expand short-name macros manually since we can't re-include
   the header; the aliases are already defined from the include above
   if DS4VIZ_SHORT_NAMES was defined before include.
   Instead, we test via the long names which are always available. */
#undef DS4VIZ_SHORT_NAMES

/* ================================================================
 *  ADDITIONAL TESTS: operations after error are ignored
 * ================================================================ */

TEST(error_stops_further_ops)
{
    cfg("_t_es01.toml");
    ds4vizStack(s)
    {
        ds4vizStackPop(s);       /* error */
        ds4vizStackPush(s, 999); /* should be ignored */
    }
    char *c = read_file("_t_es01.toml");
    ASSERT(c);
    ASSERT(valid_err(c));
    /* Only the initial state should exist — no push happened */
    ASSERT(count_str(c, "[[states]]") == 1);
    ASSERT(count_str(c, "[[steps]]") == 0);
    free(c);
    remove("_t_es01.toml");
}

TEST(error_stops_queue_ops)
{
    cfg("_t_es02.toml");
    ds4vizQueue(q)
    {
        ds4vizQueueDequeue(q);     /* error */
        ds4vizQueueEnqueue(q, 42); /* ignored */
    }
    char *c = read_file("_t_es02.toml");
    ASSERT(c);
    ASSERT(valid_err(c));
    ASSERT(count_str(c, "[[states]]") == 1);
    free(c);
    remove("_t_es02.toml");
}

TEST(error_stops_slist_ops)
{
    cfg("_t_es03.toml");
    ds4vizSingleLinkedList(l)
    {
        ds4vizSlDeleteHead(l);    /* error */
        ds4vizSlInsertHead(l, 1); /* ignored */
    }
    char *c = read_file("_t_es03.toml");
    ASSERT(c);
    ASSERT(valid_err(c));
    ASSERT(count_str(c, "[[states]]") == 1);
    free(c);
    remove("_t_es03.toml");
}

/* ================================================================
 *  ADDITIONAL TESTS: undirected graph reverse-order edge removal
 * ================================================================ */

TEST(gu_remove_edge_reversed)
{
    cfg("_t_gur01.toml");
    ds4vizGraphUndirected(g)
    {
        ds4vizGuAddNode(g, 0, "A");
        ds4vizGuAddNode(g, 1, "B");
        ds4vizGuAddEdge(g, 0, 1);
        ds4vizGuRemoveEdge(g, 1, 0); /* reversed args */
    }
    char *c = read_file("_t_gur01.toml");
    ASSERT(c);
    ASSERT(valid_ok(c));
    free(c);
    remove("_t_gur01.toml");
}

/* ================================================================
 *  ADDITIONAL TESTS: graph update_node_label for all graph types
 * ================================================================ */

TEST(gu_update_node_label)
{
    cfg("_t_gunl.toml");
    ds4vizGraphUndirected(g)
    {
        ds4vizGuAddNode(g, 0, "A");
        ds4vizGuUpdateNodeLabel(g, 0, "Z");
    }
    char *c = read_file("_t_gunl.toml");
    ASSERT(c);
    ASSERT(valid_ok(c));
    ASSERT(has(c, "\"Z\""));
    free(c);
    remove("_t_gunl.toml");
}

TEST(gd_update_node_label)
{
    cfg("_t_gdnl.toml");
    ds4vizGraphDirected(g)
    {
        ds4vizGdAddNode(g, 0, "A");
        ds4vizGdUpdateNodeLabel(g, 0, "Z");
    }
    char *c = read_file("_t_gdnl.toml");
    ASSERT(c);
    ASSERT(valid_ok(c));
    ASSERT(has(c, "\"Z\""));
    free(c);
    remove("_t_gdnl.toml");
}

/* ================================================================
 *  ADDITIONAL TESTS: graph edge to nonexistent node
 * ================================================================ */

TEST(gu_edge_to_nonexistent_node_error)
{
    cfg("_t_guen.toml");
    ds4vizGraphUndirected(g)
    {
        ds4vizGuAddNode(g, 0, "A");
        ds4vizGuAddEdge(g, 0, 99);
    }
    char *c = read_file("_t_guen.toml");
    ASSERT(c);
    ASSERT(valid_err(c));
    free(c);
    remove("_t_guen.toml");
}

TEST(gd_edge_to_nonexistent_node_error)
{
    cfg("_t_gden.toml");
    ds4vizGraphDirected(g)
    {
        ds4vizGdAddNode(g, 0, "A");
        ds4vizGdAddEdge(g, 0, 99);
    }
    char *c = read_file("_t_gden.toml");
    ASSERT(c);
    ASSERT(valid_err(c));
    free(c);
    remove("_t_gden.toml");
}

/* ================================================================
 *  MAIN
 * ================================================================ */

int main(void)
{
    printf("ds4viz.h test suite\n");

    SECTION("Stack");
    RUN(stack_basic_operations);
    RUN(stack_empty_pop_error);
    RUN(stack_clear);
    RUN(stack_various_value_types);
    RUN(stack_negative_numbers);
    RUN(stack_mixed_operations);
    RUN(stack_single_element);
    RUN(stack_push_after_clear);
    RUN(stack_multiple_pops);
    RUN(stack_float_values);
    RUN(stack_string_values);
    RUN(stack_boolean_values);
    RUN(stack_zero_value);

    SECTION("Queue");
    RUN(queue_basic_operations);
    RUN(queue_empty_dequeue_error);
    RUN(queue_clear);
    RUN(queue_single_element);
    RUN(queue_enqueue_after_clear);
    RUN(queue_multiple_dequeues);
    RUN(queue_alternating_operations);
    RUN(queue_negative_numbers);
    RUN(queue_float_values);
    RUN(queue_string_values);
    RUN(queue_dequeue_until_empty);

    SECTION("Singly Linked List");
    RUN(slist_basic_operations);
    RUN(slist_reverse);
    RUN(slist_delete_nonexistent_error);
    RUN(slist_empty_delete_head_error);
    RUN(slist_insert_after_nonexistent_error);
    RUN(slist_delete_tail);
    RUN(slist_clear);
    RUN(slist_single_node);
    RUN(slist_reverse_empty);
    RUN(slist_reverse_single);
    RUN(slist_multiple_inserts);

    SECTION("Doubly Linked List");
    RUN(dlist_basic_operations);
    RUN(dlist_reverse);
    RUN(dlist_head_tail_consistency);
    RUN(dlist_delete_head_error);
    RUN(dlist_delete_tail_error);
    RUN(dlist_insert_before_nonexistent_error);
    RUN(dlist_insert_after_nonexistent_error);
    RUN(dlist_delete_nonexistent_error);
    RUN(dlist_clear);
    RUN(dlist_single_node);
    RUN(dlist_reverse_empty);
    RUN(dlist_multiple_operations);

    SECTION("Binary Tree");
    RUN(bt_basic_operations);
    RUN(bt_delete_subtree);
    RUN(bt_update_value);
    RUN(bt_duplicate_root_error);
    RUN(bt_duplicate_child_error);
    RUN(bt_nonexistent_parent_error);
    RUN(bt_delete_root);
    RUN(bt_delete_nonexistent_error);
    RUN(bt_update_nonexistent_error);
    RUN(bt_clear);
    RUN(bt_complete_tree);

    SECTION("Binary Search Tree");
    RUN(bst_basic_operations);
    RUN(bst_delete_leaf);
    RUN(bst_delete_one_child);
    RUN(bst_delete_two_children);
    RUN(bst_delete_nonexistent_error);
    RUN(bst_delete_root);
    RUN(bst_insert_duplicate);
    RUN(bst_clear);
    RUN(bst_left_skewed);
    RUN(bst_right_skewed);

    SECTION("Heap");
    RUN(heap_min_basic);
    RUN(heap_max_basic);
    RUN(heap_extract_empty_error);
    RUN(heap_clear);
    RUN(heap_multiple_extract);
    RUN(heap_single_element);
    RUN(heap_insert_after_extract);
    RUN(heap_min_property);
    RUN(heap_max_property);

    SECTION("Graph Undirected");
    RUN(gu_basic);
    RUN(gu_edge_normalization);
    RUN(gu_self_loop_error);
    RUN(gu_duplicate_edge_error);
    RUN(gu_remove_node);
    RUN(gu_remove_edge);
    RUN(gu_duplicate_node_error);
    RUN(gu_remove_nonexistent_node_error);
    RUN(gu_remove_nonexistent_edge_error);
    RUN(gu_clear);
    RUN(gu_update_node_label);
    RUN(gu_remove_edge_reversed);
    RUN(gu_edge_to_nonexistent_node_error);

    SECTION("Graph Directed");
    RUN(gd_basic);
    RUN(gd_no_edge_normalization);
    RUN(gd_self_loop_error);
    RUN(gd_duplicate_edge_error);
    RUN(gd_remove_node);
    RUN(gd_remove_edge);
    RUN(gd_clear);
    RUN(gd_update_node_label);
    RUN(gd_edge_to_nonexistent_node_error);

    SECTION("Graph Weighted");
    RUN(gw_undirected);
    RUN(gw_directed);
    RUN(gw_update_weight);
    RUN(gw_update_node_label);
    RUN(gw_negative_weight);
    RUN(gw_zero_weight);
    RUN(gw_update_nonexistent_edge_error);
    RUN(gw_remove_edge);
    RUN(gw_clear);

    SECTION("Config");
    RUN(config_remarks);
    RUN(config_output_path);
    RUN(config_only_title);
    RUN(config_only_author);
    RUN(config_only_comment);

    SECTION("Edge Cases");
    RUN(edge_empty_structure);
    RUN(edge_special_chars_string);
    RUN(edge_large_numbers);
    RUN(edge_many_operations);
    RUN(edge_node_label_boundary);
    RUN(edge_mixed_types);

    SECTION("Complex Scenarios");
    RUN(complex_bst_deletion);
    RUN(complex_graph_operations);
    RUN(complex_linked_list_operations);
    RUN(complex_heap_sort);
    RUN(complex_graph_cycle);

    SECTION("Error-stops-ops");
    RUN(error_stops_further_ops);
    RUN(error_stops_queue_ops);
    RUN(error_stops_slist_ops);

    /* Summary */
    printf("\n========================================\n");
    printf("  Total: %d  |  Passed: %d  |  Failed: %d\n", g_run, g_pass, g_fail);
    printf("========================================\n");

    return g_fail ? 1 : 0;
}