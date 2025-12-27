/**
 * js_stack_sucess.js
 * Simple success case: stack push/pop
 * Output: TOML only (stdout)
 */

const fs = require("node:fs");
const path = require("node:path");
const { pathToFileURL } = require("node:url");

async function loadDs4viz() {
    // 1) If installed as a package
    try {
        return require("ds4viz");
    } catch (_) { }

    // 2) Try common file locations relative to server cwd
    const candidates = [
        path.join(process.cwd(), "ds4viz.js"),
        path.join(process.cwd(), "../ds4viz.js"),
        path.join(process.cwd(), "../../library/javascript/ds4viz.js"),
        path.join(process.cwd(), "../../library/javascript/dist/ds4viz.js"),
        path.join(process.cwd(), "../../library/javascript/src/ds4viz.js"),
    ];

    for (const p of candidates) {
        if (!fs.existsSync(p)) continue;

        // Try CJS/UMD
        try {
            return require(p);
        } catch (_) { }

        // Try ESM
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
    try {
        const ds4viz = await loadDs4viz();
        const { config, withContext, stack } = ds4viz;

        const outDir = "/tmp/ds4viz";
        fs.mkdirSync(outDir, { recursive: true });

        const outPath = path.join(outDir, "trace_js_stack_sucess.toml");

        if (typeof config === "function") {
            config({
                outputPath: outPath,
                title: "JS Stack Simple",
                author: "WaterRun",
                comment: "Simple success: stack push/pop",
            });
        }

        await withContext(stack({ label: "demo_stack" }), async (s) => {
            s.push(10);
            s.push(20);
            s.pop();
            s.push(true);
        });

        const toml = fs.readFileSync(outPath, "utf8");
        process.stdout.write(toml);
    } catch (err) {
        process.stdout.write(writeSandboxErrorToml(err));
        process.exit(1);
    }
})();
