use ds4viz::prelude::*;

fn main() -> ds4viz::Result<()> {
    ds4viz::heap("demo_min_heap_ok", HeapType::Min, |h| {
        h.insert(10)?;
        h.insert(5)?;
        h.insert(15)?;
        h.insert(3)?;

        h.extract()?; // 取 3
        h.extract()?; // 取 5

        h.clear()?;
        h.insert(7)?;
        Ok(())
    })
}