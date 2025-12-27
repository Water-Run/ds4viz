/**
 * js_stack_fail.js
 * Fail case: pop on empty stack (should produce [error] in TOML)
 * Output: TOML only (stdout), and NO stack trace/no extra logs
 */

const fs = require("node:fs");
const path = require("node:path");
const { pathToFileURL } = require("node:url");

async function loadDs4viz() {
    try {
        return require("ds4viz");
    } catch (_) { }

    const candidates = [
        path.join(process.cwd(), "ds4viz.js"),
        path.join(process.cwd(), "../ds4viz.js"),
        path.join(process.cwd(), "../../library/javascript/ds4viz.js"),
        path.join(process.cwd(), "../../library/javascript/dist/ds4viz.js"),
        path.join(process.cwd(), "../../library/javascript/src/ds4viz.js"),
    ];

    for (const p of candidates) {
        if (!fs.existsSync(p)) continue;
        try {
            return require(p);
        } catch (_) { }
        try {
            return await import(pathToFileURL(p).href);
        } catch (_) { }
    }

    throw new Error(
        "Cannot locate ds4viz library (ds4viz.js). Tried: " + candidates.join(", ")
    );
}

function writeSandboxErrorToml(err) {
    const msg = err && err.message ? err.message : String(err);
    const now = new Date().toISOString();
    return (
        `[meta]\n` +
        `created_at = "${now}"\n` +
        `lang = "javascript"\n\n` +
        `[error]\n` +
        `type = "sandbox"\n` +
        `message = ${JSON.stringify(msg)}\n`
    );
}

(async () => {
    const outDir = "/tmp/ds4viz";
    fs.mkdirSync(outDir, { recursive: true });
    const outPath = path.join(outDir, "trace_js_stack_fail.toml");

    try {
        const ds4viz = await loadDs4viz();
        const { config, withContext, stack } = ds4viz;

        if (typeof config === "function") {
            config({
                outputPath: outPath,
                title: "JS Stack Fail",
                author: "WaterRun",
                comment: "Fail case: pop on empty stack",
            });
        }

        try {
            await withContext(stack({ label: "fail_stack" }), async (s) => {
                // trigger runtime error
                s.pop();
            });
        } catch (_) {
            // swallow to avoid stderr stack traces (your server merges stderr into stdout)
        }

        if (fs.existsSync(outPath)) {
            const toml = fs.readFileSync(outPath, "utf8");
            process.stdout.write(toml);
        } else {
            // If the library failed to write TOML, emit a minimal error TOML for server consistency.
            process.stdout.write(
                `[meta]\ncreated_at = "${new Date().toISOString()}"\nlang = "javascript"\n\n[error]\ntype = "runtime"\nmessage = "Expected TOML not generated"\n`
            );
            process.exit(1);
        }
    } catch (err) {
        process.stdout.write(writeSandboxErrorToml(err));
        process.exit(1);
    }
})();
