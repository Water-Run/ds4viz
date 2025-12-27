//! ```cargo
//! [dependencies]
//! ds4viz = "0.1.0"
//! ```

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
    let out = "/tmp/ds4viz_rust_heap_sucess.toml";

    let _ = ds4viz::heap_with_output("demo_min_heap_ok", out, HeapType::Min, |h| {
        h.insert(10)?;
        h.insert(5)?;
        h.insert(15)?;
        h.insert(3)?;

        h.extract()?; // 取 3
        h.extract()?; // 取 5

        h.clear()?;
        h.insert(7)?;
        Ok(())
    });

    emit_toml(out);
    let _ = std::fs::remove_file(out);
}
