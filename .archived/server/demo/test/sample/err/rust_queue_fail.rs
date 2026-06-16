use ds4viz::prelude::*;
use std::fs;

fn emit_toml(path: &str) {
    match fs::read_to_string(path) {
        Ok(s) => print!("{s}"),
        Err(_) => print!(
            "[error]\ntype = \"internal\"\nmessage = \"trace file missing: {}\"\n",
            path
        ),
    }
}

fn main() {
    let out = "trace.toml";

    // 触发 EmptyStructure：空队列 dequeue
    let _ = ds4viz::queue_with_output("demo_queue_fail_empty", out, |q| {
        q.dequeue()?;
        Ok(())
    });

    // 无论成功/失败，都尝试输出 toml（失败时应包含 [error]）
    emit_toml(out);
    let _ = std::fs::remove_file(out);
}
