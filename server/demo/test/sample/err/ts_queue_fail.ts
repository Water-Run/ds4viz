// ts_queue_fail.ts
// 目标：触发 Queue 错误（dequeue 空队列），仍生成包含 [error] 的 trace.toml，并仅输出 TOML 到 stdout

const fs = require("fs");
const dv: any = require("ds4viz");

function ensureTmpDir() {
    try {
        fs.mkdirSync("/tmp/ds4viz", { recursive: true });
    } catch { }
}

function fileExists(p: string): boolean {
    try {
        fs.accessSync(p);
        return true;
    } catch {
        return false;
    }
}

function main() {
    ensureTmpDir();

    const outPath = `/tmp/ds4viz/trace_ts_queue_fail_${Date.now()}_${Math.floor(
        Math.random() * 1_000_000
    )}.toml`;

    if (typeof dv.config === "function") {
        dv.config({
            output_path: outPath,
            title: "TypeScript Queue Fail",
            author: "WaterRun",
            comment: "expect: dequeue on empty queue",
        });
    }

    // 尝试形态 A：queue(label, fn)
    try {
        if (typeof dv.queue === "function") {
            dv.queue("demo_queue_fail", (q: any) => {
                // 故意失败：空队列 dequeue
                q.dequeue();
            });
        }
    } catch {
        // 忽略异常，继续读取 trace
    }

    // 如果形态 A 没生成文件，尝试形态 B：withContext(queue(...), fn)
    if (!fileExists(outPath)) {
        try {
            if (typeof dv.withContext === "function" && typeof dv.queue === "function") {
                const q = dv.queue("demo_queue_fail");
                dv.withContext(q, (qq: any) => {
                    qq.dequeue();
                });
            }
        } catch {
            // 忽略异常
        }
    }

    // 只输出 TOML
    if (fileExists(outPath)) {
        const toml = fs.readFileSync(outPath, "utf8");
        process.stdout.write(toml);
    }
}

main();
