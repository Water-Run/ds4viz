// ts_binary_search_tree_sucess.ts
// 目标：覆盖 binary_search_tree（insert/delete），避免 console 输出

import * as ds4viz from "ds4viz";

ds4viz.binary_search_tree("demo_bst_ok", (bst: any) => {
    bst.insert(10);
    bst.insert(5);
    bst.insert(15);
    bst.insert(3);
    bst.insert(7);

    // 删除后再插入，验证性质维护
    bst.delete(5);
    bst.insert(6);
});
