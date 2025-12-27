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
    let out = "/tmp/ds4viz_rust_bst_sucess.toml";

    let _ = ds4viz::binary_search_tree_with_output("demo_bst_ok", out, |bst| {
        bst.insert(10)?;
        bst.insert(5)?;
        bst.insert(15)?;
        bst.insert(3)?;
        bst.insert(7)?;

        bst.delete(5)?; // 删除后再插入
        bst.insert(6)?;
        Ok(())
    });

    emit_toml(out);
    let _ = std::fs::remove_file(out);
}
