#define DS4VIZ_IMPLEMENTATION
#include "ds4viz.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#ifdef _WIN32
#include <process.h>
#define getpid _getpid
#else
#include <unistd.h>
#endif

static char *read_all(const char *path, long *out_len)
{
    FILE *f = fopen(path, "rb");
    if (!f)
        return NULL;
    if (fseek(f, 0, SEEK_END) != 0)
    {
        fclose(f);
        return NULL;
    }
    long n = ftell(f);
    if (n < 0)
    {
        fclose(f);
        return NULL;
    }
    rewind(f);

    char *buf = (char *)malloc((size_t)n + 1);
    if (!buf)
    {
        fclose(f);
        return NULL;
    }

    size_t r = fread(buf, 1, (size_t)n, f);
    fclose(f);
    if ((long)r != n)
    {
        free(buf);
        return NULL;
    }

    buf[n] = '\0';
    if (out_len)
        *out_len = n;
    return buf;
}

static void print_and_remove(const char *path)
{
    long n = 0;
    char *s = read_all(path, &n);
    if (!s)
    {
        fprintf(stderr, "failed to read toml: %s\n", path);
        return;
    }
    fwrite(s, 1, (size_t)n, stdout);
    fflush(stdout);
    free(s);
    remove(path);
}

int main(void)
{
    char out_path[256];
    snprintf(out_path, sizeof(out_path),
             "/tmp/ds4viz_btree_fail_%ld_%d.toml",
             (long)time(NULL), (int)getpid());

    dv_config(out_path, "C Binary Tree Fail Demo", "WaterRun",
              "fail case: insert child with non-existent parent");

    dv_binary_tree_t *t = dv_binary_tree_create("demo_btree", NULL);
    if (!t)
        return 1;

    dv_error_t err = DV_OK;
    int32_t root = -1;

    err = dv_binary_tree_insert_root_int(t, 10, &root);
    if (err != DV_OK)
        goto cleanup;

    /* 故意制造错误：父节点不存在 */
    err = dv_binary_tree_insert_left_int(t, 999, 5, NULL);

    /* 这里不 goto，让库把错误状态带进 commit 的 TOML 里 */
cleanup:
    (void)dv_binary_tree_commit(t);
    dv_binary_tree_destroy(t);

    /* 输出 TOML（供 Demo Server 直接返回） */
    print_and_remove(out_path);

    /* 失败用例：返回非 0 */
    return (err == DV_OK) ? 0 : 1;
}
