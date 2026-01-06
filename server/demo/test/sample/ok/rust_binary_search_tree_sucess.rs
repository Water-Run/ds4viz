use ds4viz::prelude::*;

fn main() -> ds4viz::Result<()> {
    ds4viz::binary_search_tree("demo_bst_ok", |bst| {
        bst.insert(10)?;
        bst.insert(5)?;
        bst.insert(15)?;
        bst.insert(3)?;
        bst.insert(7)?;

        bst.delete(5)?;
        bst.insert(6)?;
        Ok(())
    })
}