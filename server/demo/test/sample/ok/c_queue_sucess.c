#define DS4VIZ_IMPLEMENTATION
#include "ds4viz.h"

#include <stdint.h>

int main(void)
{
    const char *OUT = "/dev/stdout";
    dv_error_t err = DV_OK;

    dv_config(
        OUT,
        "C Queue (simple)",
        "WaterRun",
        "queue enqueue/dequeue/clear");

    dv_queue_t *q = dv_queue_create("c_queue_simple", OUT);
    if (q == NULL)
        return 1;

    err = dv_queue_enqueue_int(q, 10);
    if (err != DV_OK)
        goto cleanup;

    err = dv_queue_enqueue_int(q, 20);
    if (err != DV_OK)
        goto cleanup;

    err = dv_queue_enqueue_int(q, 30);
    if (err != DV_OK)
        goto cleanup;

    err = dv_queue_dequeue(q); /* pop 10 */
    if (err != DV_OK)
        goto cleanup;

    err = dv_queue_enqueue_int(q, 40);
    if (err != DV_OK)
        goto cleanup;

    err = dv_queue_clear(q);
    if (err != DV_OK)
        goto cleanup;

    /* 结束前再放一个元素，便于渲染器看到最终状态非空也可 */
    err = dv_queue_enqueue_int(q, 99);
    if (err != DV_OK)
        goto cleanup;

cleanup:
    dv_queue_commit(q); /* 无论成功/失败都输出 TOML 到 stdout */
    dv_queue_destroy(q);

    return (err == DV_OK) ? 0 : 1;
}
