//! 集成测试
//!
//! 测试完整的使用场景和边缘情况
//!
//! # 作者
//!
//! WaterRun
//!
//! # 文件路径
//!
//! tests/integration.rs
//!
//! # 日期
//!
//! 2025-12-25

use ds4viz::HeapType;
use std::fs;
use tempfile::tempdir;

// ============================================================================
// 栈完整测试
// ============================================================================

#[test]
fn test_stack_full_operations() {
    let dir = tempdir().unwrap();
    let path = dir.path().join("stack_full.toml");
    let path_str = path.to_str().unwrap();

    let result = ds4viz::stack_with_output("full_stack", Some(path_str), |s| {
        // 多次 push
        s.push(10)?;
        s.push(20)?;
        s.push(30)?;
        s.push(40)?;
        s.push(50)?;

        // 多次 pop
        s.pop()?;
        s.pop()?;

        // 混合操作
        s.push(60)?;
        s.pop()?;
        s.pop()?;

        // clear 后继续操作
        s.clear()?;
        s.push(100)?;
        s.push(200)?;
        s.pop()?;

        Ok(())
    });

    assert!(result.is_ok());

    let content = fs::read_to_string(&path).unwrap();
    
    // 统计操作数量
    let push_count = content.matches("op = \"push\"").count();
    let pop_count = content.matches("op = \"pop\"").count();
    let clear_count = content.matches("op = \"clear\"").count();

    assert_eq!(push_count, 8);
    assert_eq!(pop_count, 5);
    assert_eq!(clear_count, 1);
}

#[test]
fn test_stack_with_different_value_types() {
    let dir = tempdir().unwrap();
    let path = dir.path().join("stack_types.toml");
    let path_str = path.to_str().unwrap();

    let result = ds4viz::stack_with_output("typed_stack", Some(path_str), |s| {
        // 整数类型
        s.push(42_i64)?;
        s.push(100_i32)?;

        // 浮点数类型
        s.push(3.14159_f64)?;
        s.push(2.71828_f32)?;

        // 字符串类型
        s.push("hello world")?;
        s.push(String::from("Rust"))?;

        // 布尔类型
        s.push(true)?;
        s.push(false)?;

        Ok(())
    });

    assert!(result.is_ok());

    let content = fs::read_to_string(&path).unwrap();
    assert!(content.contains("42"));
    assert!(content.contains("100"));
    assert!(content.contains("3.14159"));
    assert!(content.contains("\"hello world\""));
    assert!(content.contains("\"Rust\""));
    assert!(content.contains("true"));
    assert!(content.contains("false"));
}

#[test]
fn test_stack_pop_until_empty_then_error() {
    let dir = tempdir().unwrap();
    let path = dir.path().join("stack_pop_error.toml");
    let path_str = path.to_str().unwrap();

    let result = ds4viz::stack_with_output("pop_error_stack", Some(path_str), |s| {
        s.push(1)?;
        s.push(2)?;
        s.push(3)?;
        s.pop()?;
        s.pop()?;
        s.pop()?;
        s.pop()?; // 这里应该报错
        Ok(())
    });

    assert!(result.is_err());

    let content = fs::read_to_string(&path).unwrap();
    assert!(content.contains("[error]"));
    assert!(content.contains("step = 6")); // 第7步（索引6）失败
}

#[test]
fn test_stack_large_number_of_operations() {
    let dir = tempdir().unwrap();
    let path = dir.path().join("stack_large.toml");
    let path_str = path.to_str().unwrap();

    let result = ds4viz::stack_with_output("large_stack", Some(path_str), |s| {
        for i in 0..100 {
            s.push(i)?;
        }
        for _ in 0..50 {
            s.pop()?;
        }
        Ok(())
    });

    assert!(result.is_ok());

    let content = fs::read_to_string(&path).unwrap();
    let push_count = content.matches("op = \"push\"").count();
    let pop_count = content.matches("op = \"pop\"").count();

    assert_eq!(push_count, 100);
    assert_eq!(pop_count, 50);
}

// ============================================================================
// 队列完整测试
// ============================================================================

#[test]
fn test_queue_full_operations() {
    let dir = tempdir().unwrap();
    let path = dir.path().join("queue_full.toml");
    let path_str = path.to_str().unwrap();

    let result = ds4viz::queue_with_output("full_queue", Some(path_str), |q| {
        q.enqueue(10)?;
        q.enqueue(20)?;
        q.enqueue(30)?;
        q.dequeue()?;
        q.enqueue(40)?;
        q.dequeue()?;
        q.dequeue()?;
        q.enqueue(50)?;
        q.clear()?;
        q.enqueue(100)?;
        Ok(())
    });

    assert!(result.is_ok());

    let content = fs::read_to_string(&path).unwrap();
    assert!(content.contains("op = \"enqueue\""));
    assert!(content.contains("op = \"dequeue\""));
    assert!(content.contains("op = \"clear\""));
}

#[test]
fn test_queue_front_rear_indices() {
    let dir = tempdir().unwrap();
    let path = dir.path().join("queue_indices.toml");
    let path_str = path.to_str().unwrap();

    let result = ds4viz::queue_with_output("index_queue", Some(path_str), |q| {
        // 初始状态 front = rear = -1
        q.enqueue(10)?;
        // 现在 front = 0, rear = 0
        q.enqueue(20)?;
        // front = 0, rear = 1
        q.dequeue()?;
        // front = 0, rear = 0 (数组调整后)
        Ok(())
    });

    assert!(result.is_ok());

    let content = fs::read_to_string(&path).unwrap();
    assert!(content.contains("front"));
    assert!(content.contains("rear"));
}

#[test]
fn test_queue_dequeue_error() {
    let dir = tempdir().unwrap();
    let path = dir.path().join("queue_error.toml");
    let path_str = path.to_str().unwrap();

    let result = ds4viz::queue_with_output("error_queue", Some(path_str), |q| {
        q.enqueue(1)?;
        q.dequeue()?;
        q.dequeue()?; // 空队列出队
        Ok(())
    });

    assert!(result.is_err());

    let content = fs::read_to_string(&path).unwrap();
    assert!(content.contains("[error]"));
    assert!(content.contains("Structure is empty"));
}

// ============================================================================
// 单链表完整测试
// ============================================================================

#[test]
fn test_single_linked_list_full_operations() {
    let dir = tempdir().unwrap();
    let path = dir.path().join("slist_full.toml");
    let path_str = path.to_str().unwrap();

    let result = ds4viz::single_linked_list_with_output("full_slist", Some(path_str), |list| {
        // 头部插入
        let node1 = list.insert_head(10)?;
        let node2 = list.insert_head(20)?;

        // 尾部插入
        let node3 = list.insert_tail(30)?;
        list.insert_tail(40)?;

        // 在指定节点后插入
        list.insert_after(node1, 15)?;
        list.insert_after(node2, 25)?;

        // 删除操作
        list.delete(node3)?;
        list.delete_head()?;

        // 反转
        list.reverse()?;

        Ok(())
    });

    assert!(result.is_ok());

    let content = fs::read_to_string(&path).unwrap();
    assert!(content.contains("op = \"insert_head\""));
    assert!(content.contains("op = \"insert_tail\""));
    assert!(content.contains("op = \"insert_after\""));
    assert!(content.contains("op = \"delete\""));
    assert!(content.contains("op = \"delete_head\""));
    assert!(content.contains("op = \"reverse\""));
}

#[test]
fn test_single_linked_list_node_not_found() {
    let dir = tempdir().unwrap();
    let path = dir.path().join("slist_error.toml");
    let path_str = path.to_str().unwrap();

    let result = ds4viz::single_linked_list_with_output("error_slist", Some(path_str), |list| {
        list.insert_head(10)?;
        list.delete(999)?; // 不存在的节点
        Ok(())
    });

    assert!(result.is_err());

    let content = fs::read_to_string(&path).unwrap();
    assert!(content.contains("[error]"));
    assert!(content.contains("Node not found: 999"));
}

#[test]
fn test_single_linked_list_insert_after_invalid() {
    let dir = tempdir().unwrap();
    let path = dir.path().join("slist_insert_error.toml");
    let path_str = path.to_str().unwrap();

    let result = ds4viz::single_linked_list_with_output("insert_error_slist", Some(path_str), |list| {
        list.insert_head(10)?;
        list.insert_after(100, 20)?; // 无效的父节点
        Ok(())
    });

    assert!(result.is_err());

    let content = fs::read_to_string(&path).unwrap();
    assert!(content.contains("[error]"));
}

#[test]
fn test_single_linked_list_delete_head_empty() {
    let dir = tempdir().unwrap();
    let path = dir.path().join("slist_delete_head_error.toml");
    let path_str = path.to_str().unwrap();

    let result = ds4viz::single_linked_list_with_output("delete_head_error", Some(path_str), |list| {
        list.delete_head()?; // 空链表删除头节点
        Ok(())
    });

    assert!(result.is_err());

    let content = fs::read_to_string(&path).unwrap();
    assert!(content.contains("[error]"));
    assert!(content.contains("Structure is empty"));
}

#[test]
fn test_single_linked_list_reverse_empty() {
    let dir = tempdir().unwrap();
    let path = dir.path().join("slist_reverse_empty.toml");
    let path_str = path.to_str().unwrap();

    let result = ds4viz::single_linked_list_with_output("reverse_empty", Some(path_str), |list| {
        list.reverse()?; // 空链表反转应该成功
        Ok(())
    });

    assert!(result.is_ok());
}

#[test]
fn test_single_linked_list_reverse_single() {
    let dir = tempdir().unwrap();
    let path = dir.path().join("slist_reverse_single.toml");
    let path_str = path.to_str().unwrap();

    let result = ds4viz::single_linked_list_with_output("reverse_single", Some(path_str), |list| {
        list.insert_head(10)?;
        list.reverse()?; // 单节点反转
        Ok(())
    });

    assert!(result.is_ok());
}

// ============================================================================
// 双向链表完整测试
// ============================================================================

#[test]
fn test_double_linked_list_full_operations() {
    let dir = tempdir().unwrap();
    let path = dir.path().join("dlist_full.toml");
    let path_str = path.to_str().unwrap();

    let result = ds4viz::double_linked_list_with_output("full_dlist", Some(path_str), |list| {
        // 头部和尾部插入
        let node1 = list.insert_head(10)?;
        list.insert_tail(30)?;
        let node3 = list.insert_head(5)?;

        // 在指定位置插入
        list.insert_after(node1, 20)?;
        list.insert_before(node3, 1)?;

        // 删除操作
        list.delete(node1)?;
        list.delete_head()?;
        list.delete_tail()?;

        // 反转
        list.reverse()?;

        Ok(())
    });

    assert!(result.is_ok());

    let content = fs::read_to_string(&path).unwrap();
    assert!(content.contains("op = \"insert_head\""));
    assert!(content.contains("op = \"insert_tail\""));
    assert!(content.contains("op = \"insert_after\""));
    assert!(content.contains("op = \"insert_before\""));
    assert!(content.contains("op = \"delete\""));
    assert!(content.contains("op = \"reverse\""));
}

#[test]
fn test_double_linked_list_prev_next_pointers() {
    let dir = tempdir().unwrap();
    let path = dir.path().join("dlist_pointers.toml");
    let path_str = path.to_str().unwrap();

    let result = ds4viz::double_linked_list_with_output("pointer_dlist", Some(path_str), |list| {
        list.insert_head(10)?;
        list.insert_tail(20)?;
        list.insert_tail(30)?;
        Ok(())
    });

    assert!(result.is_ok());

    let content = fs::read_to_string(&path).unwrap();
    assert!(content.contains("prev"));
    assert!(content.contains("next"));
}

#[test]
fn test_double_linked_list_delete_tail_empty() {
    let dir = tempdir().unwrap();
    let path = dir.path().join("dlist_delete_tail_error.toml");
    let path_str = path.to_str().unwrap();

    let result = ds4viz::double_linked_list_with_output("delete_tail_error", Some(path_str), |list| {
        list.delete_tail()?;
        Ok(())
    });

    assert!(result.is_err());

    let content = fs::read_to_string(&path).unwrap();
    assert!(content.contains("[error]"));
}

#[test]
fn test_double_linked_list_insert_before_invalid() {
    let dir = tempdir().unwrap();
    let path = dir.path().join("dlist_insert_before_error.toml");
    let path_str = path.to_str().unwrap();

    let result = ds4viz::double_linked_list_with_output("insert_before_error", Some(path_str), |list| {
        list.insert_head(10)?;
        list.insert_before(999, 5)?;
        Ok(())
    });

    assert!(result.is_err());

    let content = fs::read_to_string(&path).unwrap();
    assert!(content.contains("[error]"));
    assert!(content.contains("Node not found: 999"));
}

// ============================================================================
// 二叉树完整测试
// ============================================================================

#[test]
fn test_binary_tree_full_operations() {
    let dir = tempdir().unwrap();
    let path = dir.path().join("btree_full.toml");
    let path_str = path.to_str().unwrap();

    let result = ds4viz::binary_tree_with_output("full_btree", Some(path_str), |tree| {
        let root = tree.insert_root(10)?;
        let left = tree.insert_left(root, 5)?;
        let right = tree.insert_right(root, 15)?;

        tree.insert_left(left, 3)?;
        tree.insert_right(left, 7)?;
        tree.insert_left(right, 12)?;
        tree.insert_right(right, 18)?;

        tree.update_value(left, 6)?;
        tree.delete(right)?; // 删除子树

        Ok(())
    });

    assert!(result.is_ok());

    let content = fs::read_to_string(&path).unwrap();
    assert!(content.contains("op = \"insert_root\""));
    assert!(content.contains("op = \"insert_left\""));
    assert!(content.contains("op = \"insert_right\""));
    assert!(content.contains("op = \"update_value\""));
    assert!(content.contains("op = \"delete\""));
}

#[test]
fn test_binary_tree_root_exists_error() {
    let dir = tempdir().unwrap();
    let path = dir.path().join("btree_root_error.toml");
    let path_str = path.to_str().unwrap();

    let result = ds4viz::binary_tree_with_output("root_error_btree", Some(path_str), |tree| {
        tree.insert_root(10)?;
        tree.insert_root(20)?; // 根节点已存在
        Ok(())
    });

    assert!(result.is_err());

    let content = fs::read_to_string(&path).unwrap();
    assert!(content.contains("[error]"));
    assert!(content.contains("Root node already exists"));
}

#[test]
fn test_binary_tree_child_exists_error() {
    let dir = tempdir().unwrap();
    let path = dir.path().join("btree_child_error.toml");
    let path_str = path.to_str().unwrap();

    let result = ds4viz::binary_tree_with_output("child_error_btree", Some(path_str), |tree| {
        let root = tree.insert_root(10)?;
        tree.insert_left(root, 5)?;
        tree.insert_left(root, 6)?; // 左子节点已存在
        Ok(())
    });

    assert!(result.is_err());

    let content = fs::read_to_string(&path).unwrap();
    assert!(content.contains("[error]"));
    assert!(content.contains("Child node already exists"));
}

#[test]
fn test_binary_tree_node_not_found_error() {
    let dir = tempdir().unwrap();
    let path = dir.path().join("btree_node_error.toml");
    let path_str = path.to_str().unwrap();

    let result = ds4viz::binary_tree_with_output("node_error_btree", Some(path_str), |tree| {
        tree.insert_root(10)?;
        tree.insert_left(999, 5)?; // 父节点不存在
        Ok(())
    });

    assert!(result.is_err());

    let content = fs::read_to_string(&path).unwrap();
    assert!(content.contains("[error]"));
    assert!(content.contains("Node not found: 999"));
}

#[test]
fn test_binary_tree_update_not_found() {
    let dir = tempdir().unwrap();
    let path = dir.path().join("btree_update_error.toml");
    let path_str = path.to_str().unwrap();

    let result = ds4viz::binary_tree_with_output("update_error_btree", Some(path_str), |tree| {
        tree.insert_root(10)?;
        tree.update_value(999, 20)?;
        Ok(())
    });

    assert!(result.is_err());
}

#[test]
fn test_binary_tree_delete_not_found() {
    let dir = tempdir().unwrap();
    let path = dir.path().join("btree_delete_error.toml");
    let path_str = path.to_str().unwrap();

    let result = ds4viz::binary_tree_with_output("delete_error_btree", Some(path_str), |tree| {
        tree.insert_root(10)?;
        tree.delete(999)?;
        Ok(())
    });

    assert!(result.is_err());
}

// ============================================================================
// 二叉搜索树完整测试
// ============================================================================

#[test]
fn test_bst_full_operations() {
    let dir = tempdir().unwrap();
    let path = dir.path().join("bst_full.toml");
    let path_str = path.to_str().unwrap();

    let result = ds4viz::binary_search_tree_with_output("full_bst", Some(path_str), |bst| {
        // 插入多个值
        bst.insert(50)?;
        bst.insert(30)?;
        bst.insert(70)?;
        bst.insert(20)?;
        bst.insert(40)?;
        bst.insert(60)?;
        bst.insert(80)?;

        // 删除叶节点
        bst.delete(20)?;

        // 删除只有一个子节点的节点
        bst.delete(70)?;

        // 删除有两个子节点的节点
        bst.delete(50)?;

        Ok(())
    });

    assert!(result.is_ok());

    let content = fs::read_to_string(&path).unwrap();
    assert!(content.contains("kind = \"bst\""));
}

#[test]
fn test_bst_with_floats() {
    let dir = tempdir().unwrap();
    let path = dir.path().join("bst_float.toml");
    let path_str = path.to_str().unwrap();

    let result = ds4viz::binary_search_tree_with_output("float_bst", Some(path_str), |bst| {
        bst.insert(3.14)?;
        bst.insert(2.71)?;
        bst.insert(1.41)?;
        bst.delete(2.71)?;
        Ok(())
    });

    assert!(result.is_ok());
}

#[test]
fn test_bst_delete_not_found() {
    let dir = tempdir().unwrap();
    let path = dir.path().join("bst_delete_error.toml");
    let path_str = path.to_str().unwrap();

    let result = ds4viz::binary_search_tree_with_output("delete_error_bst", Some(path_str), |bst| {
        bst.insert(10)?;
        bst.insert(5)?;
        bst.delete(100)?; // 值不存在
        Ok(())
    });

    assert!(result.is_err());

    let content = fs::read_to_string(&path).unwrap();
    assert!(content.contains("[error]"));
    assert!(content.contains("Value not found"));
}

#[test]
fn test_bst_ascending_insert() {
    let dir = tempdir().unwrap();
    let path = dir.path().join("bst_ascending.toml");
    let path_str = path.to_str().unwrap();

    let result = ds4viz::binary_search_tree_with_output("ascending_bst", Some(path_str), |bst| {
        for i in 1..=10 {
            bst.insert(i)?;
        }
        Ok(())
    });

    assert!(result.is_ok());
}

#[test]
fn test_bst_descending_insert() {
    let dir = tempdir().unwrap();
    let path = dir.path().join("bst_descending.toml");
    let path_str = path.to_str().unwrap();

    let result = ds4viz::binary_search_tree_with_output("descending_bst", Some(path_str), |bst| {
        for i in (1..=10).rev() {
            bst.insert(i)?;
        }
        Ok(())
    });

    assert!(result.is_ok());
}

// ============================================================================
// 堆完整测试
// ============================================================================

#[test]
fn test_heap_min_operations() {
    let dir = tempdir().unwrap();
    let path = dir.path().join("heap_min_full.toml");
    let path_str = path.to_str().unwrap();

    let result = ds4viz::heap_with_output("full_min_heap", HeapType::Min, Some(path_str), |heap| {
        heap.insert(50)?;
        heap.insert(30)?;
        heap.insert(70)?;
        heap.insert(10)?;
        heap.insert(40)?;
        
        heap.extract()?; // 应该提取 10
        heap.extract()?; // 应该提取 30
        
        heap.insert(5)?;
        heap.extract()?; // 应该提取 5

        Ok(())
    });

    assert!(result.is_ok());

    let content = fs::read_to_string(&path).unwrap();
    assert!(content.contains("op = \"insert\""));
    assert!(content.contains("op = \"extract\""));
}

#[test]
fn test_heap_max_operations() {
    let dir = tempdir().unwrap();
    let path = dir.path().join("heap_max_full.toml");
    let path_str = path.to_str().unwrap();

    let result = ds4viz::heap_with_output("full_max_heap", HeapType::Max, Some(path_str), |heap| {
        heap.insert(50)?;
        heap.insert(30)?;
        heap.insert(70)?;
        heap.insert(10)?;
        heap.insert(90)?;
        
        heap.extract()?; // 应该提取 90
        heap.extract()?; // 应该提取 70

        Ok(())
    });

    assert!(result.is_ok());
}

#[test]
fn test_heap_extract_empty() {
    let dir = tempdir().unwrap();
    let path = dir.path().join("heap_extract_error.toml");
    let path_str = path.to_str().unwrap();

    let result = ds4viz::heap_with_output("extract_error_heap", HeapType::Min, Some(path_str), |heap| {
        heap.extract()?; // 空堆提取
        Ok(())
    });

    assert!(result.is_err());

    let content = fs::read_to_string(&path).unwrap();
    assert!(content.contains("[error]"));
    assert!(content.contains("Structure is empty"));
}

#[test]
fn test_heap_clear() {
    let dir = tempdir().unwrap();
    let path = dir.path().join("heap_clear.toml");
    let path_str = path.to_str().unwrap();

    let result = ds4viz::heap_with_output("clear_heap", HeapType::Min, Some(path_str), |heap| {
        heap.insert(10)?;
        heap.insert(20)?;
        heap.insert(30)?;
        heap.clear()?;
        heap.insert(5)?;
        Ok(())
    });

    assert!(result.is_ok());

    let content = fs::read_to_string(&path).unwrap();
    assert!(content.contains("op = \"clear\""));
}

#[test]
fn test_heap_with_floats() {
    let dir = tempdir().unwrap();
    let path = dir.path().join("heap_float.toml");
    let path_str = path.to_str().unwrap();

    let result = ds4viz::heap_with_output("float_heap", HeapType::Min, Some(path_str), |heap| {
        heap.insert(3.14)?;
        heap.insert(2.71)?;
        heap.insert(1.41)?;
        heap.extract()?;
        Ok(())
    });

    assert!(result.is_ok());
}

// ============================================================================
// 无向图完整测试
// ============================================================================

#[test]
fn test_graph_undirected_full_operations() {
    let dir = tempdir().unwrap();
    let path = dir.path().join("graph_undirected_full.toml");
    let path_str = path.to_str().unwrap();

    let result = ds4viz::graph_undirected_with_output("full_undirected", Some(path_str), |graph| {
        // 添加节点
        graph.add_node(0, "A")?;
        graph.add_node(1, "B")?;
        graph.add_node(2, "C")?;
        graph.add_node(3, "D")?;

        // 添加边
        graph.add_edge(0, 1)?;
        graph.add_edge(1, 2)?;
        graph.add_edge(2, 3)?;
        graph.add_edge(0, 3)?;

        // 更新标签
        graph.update_node_label(1, "B_updated")?;

        // 删除边
        graph.remove_edge(0, 3)?;

        // 删除节点
        graph.remove_node(2)?;

        Ok(())
    });

    assert!(result.is_ok());

    let content = fs::read_to_string(&path).unwrap();
    assert!(content.contains("op = \"add_node\""));
    assert!(content.contains("op = \"add_edge\""));
    assert!(content.contains("op = \"update_node_label\""));
    assert!(content.contains("op = \"remove_edge\""));
    assert!(content.contains("op = \"remove_node\""));
}

#[test]
fn test_graph_undirected_edge_normalization() {
    let dir = tempdir().unwrap();
    let path = dir.path().join("graph_undirected_norm.toml");
    let path_str = path.to_str().unwrap();

    let result = ds4viz::graph_undirected_with_output("norm_undirected", Some(path_str), |graph| {
        graph.add_node(0, "A")?;
        graph.add_node(1, "B")?;
        
        // 添加边 (1, 0)，应该被规范化为 (0, 1)
        graph.add_edge(1, 0)?;
        
        // 尝试删除 (0, 1)，应该成功
        graph.remove_edge(0, 1)?;
        
        Ok(())
    });

    assert!(result.is_ok());
}

#[test]
fn test_graph_undirected_node_exists() {
    let dir = tempdir().unwrap();
    let path = dir.path().join("graph_node_exists.toml");
    let path_str = path.to_str().unwrap();

    let result = ds4viz::graph_undirected_with_output("node_exists", Some(path_str), |graph| {
        graph.add_node(0, "A")?;
        graph.add_node(0, "B")?; // 节点已存在
        Ok(())
    });

    assert!(result.is_err());

    let content = fs::read_to_string(&path).unwrap();
    assert!(content.contains("[error]"));
    assert!(content.contains("Node already exists: 0"));
}

#[test]
fn test_graph_undirected_edge_exists() {
    let dir = tempdir().unwrap();
    let path = dir.path().join("graph_edge_exists.toml");
    let path_str = path.to_str().unwrap();

    let result = ds4viz::graph_undirected_with_output("edge_exists", Some(path_str), |graph| {
        graph.add_node(0, "A")?;
        graph.add_node(1, "B")?;
        graph.add_edge(0, 1)?;
        graph.add_edge(0, 1)?; // 边已存在
        Ok(())
    });

    assert!(result.is_err());

    let content = fs::read_to_string(&path).unwrap();
    assert!(content.contains("[error]"));
    assert!(content.contains("Edge already exists"));
}

#[test]
fn test_graph_undirected_edge_not_found() {
    let dir = tempdir().unwrap();
    let path = dir.path().join("graph_edge_not_found.toml");
    let path_str = path.to_str().unwrap();

    let result = ds4viz::graph_undirected_with_output("edge_not_found", Some(path_str), |graph| {
        graph.add_node(0, "A")?;
        graph.add_node(1, "B")?;
        graph.remove_edge(0, 1)?; // 边不存在
        Ok(())
    });

    assert!(result.is_err());

    let content = fs::read_to_string(&path).unwrap();
    assert!(content.contains("[error]"));
    assert!(content.contains("Edge not found"));
}

#[test]
fn test_graph_undirected_self_loop() {
    let dir = tempdir().unwrap();
    let path = dir.path().join("graph_self_loop.toml");
    let path_str = path.to_str().unwrap();

    let result = ds4viz::graph_undirected_with_output("self_loop", Some(path_str), |graph| {
        graph.add_node(0, "A")?;
        graph.add_edge(0, 0)?; // 自环
        Ok(())
    });

    assert!(result.is_err());

    let content = fs::read_to_string(&path).unwrap();
    assert!(content.contains("[error]"));
    assert!(content.contains("Self-loop not allowed"));
}

#[test]
fn test_graph_undirected_invalid_label() {
    let dir = tempdir().unwrap();
    let path = dir.path().join("graph_invalid_label.toml");
    let path_str = path.to_str().unwrap();

    // 空标签
    let result = ds4viz::graph_undirected_with_output("invalid_label1", Some(path_str), |graph| {
        graph.add_node(0, "")?;
        Ok(())
    });

    assert!(result.is_err());

    let content = fs::read_to_string(&path).unwrap();
    assert!(content.contains("[error]"));
    assert!(content.contains("Invalid label length"));
}

#[test]
fn test_graph_undirected_label_too_long() {
    let dir = tempdir().unwrap();
    let path = dir.path().join("graph_label_long.toml");
    let path_str = path.to_str().unwrap();

    // 超长标签（超过32个字符）
    let long_label = "a".repeat(33);
    let result = ds4viz::graph_undirected_with_output("label_too_long", Some(path_str), |graph| {
        graph.add_node(0, &long_label)?;
        Ok(())
    });

    assert!(result.is_err());
}

#[test]
fn test_graph_undirected_add_edge_node_not_found() {
    let dir = tempdir().unwrap();
    let path = dir.path().join("graph_edge_node_error.toml");
    let path_str = path.to_str().unwrap();

    let result = ds4viz::graph_undirected_with_output("edge_node_error", Some(path_str), |graph| {
        graph.add_node(0, "A")?;
        graph.add_edge(0, 1)?; // 节点1不存在
        Ok(())
    });

    assert!(result.is_err());

    let content = fs::read_to_string(&path).unwrap();
    assert!(content.contains("[error]"));
    assert!(content.contains("Node not found: 1"));
}

// ============================================================================
// 有向图完整测试
// ============================================================================

#[test]
fn test_graph_directed_full_operations() {
    let dir = tempdir().unwrap();
    let path = dir.path().join("graph_directed_full.toml");
    let path_str = path.to_str().unwrap();

    let result = ds4viz::graph_directed_with_output("full_directed", Some(path_str), |graph| {
        graph.add_node(0, "A")?;
        graph.add_node(1, "B")?;
        graph.add_node(2, "C")?;

        // 有向边
        graph.add_edge(0, 1)?;
        graph.add_edge(1, 2)?;
        graph.add_edge(2, 0)?; // 环

        graph.update_node_label(1, "B_new")?;
        graph.remove_edge(2, 0)?;
        graph.remove_node(2)?;

        Ok(())
    });

    assert!(result.is_ok());

    let content = fs::read_to_string(&path).unwrap();
    assert!(content.contains("kind = \"graph_directed\""));
}

#[test]
fn test_graph_directed_asymmetric_edges() {
    let dir = tempdir().unwrap();
    let path = dir.path().join("graph_directed_asym.toml");
    let path_str = path.to_str().unwrap();

    let result = ds4viz::graph_directed_with_output("asym_directed", Some(path_str), |graph| {
        graph.add_node(0, "A")?;
        graph.add_node(1, "B")?;
        
        // 可以添加两条方向不同的边
        graph.add_edge(0, 1)?;
        graph.add_edge(1, 0)?;
        
        Ok(())
    });

    assert!(result.is_ok());
}

#[test]
fn test_graph_directed_duplicate_edge() {
    let dir = tempdir().unwrap();
    let path = dir.path().join("graph_directed_dup.toml");
    let path_str = path.to_str().unwrap();

    let result = ds4viz::graph_directed_with_output("dup_directed", Some(path_str), |graph| {
        graph.add_node(0, "A")?;
        graph.add_node(1, "B")?;
        graph.add_edge(0, 1)?;
        graph.add_edge(0, 1)?; // 重复边
        Ok(())
    });

    assert!(result.is_err());

    let content = fs::read_to_string(&path).unwrap();
    assert!(content.contains("[error]"));
}

// ============================================================================
// 带权图完整测试
// ============================================================================

#[test]
fn test_graph_weighted_undirected_full() {
    let dir = tempdir().unwrap();
    let path = dir.path().join("graph_weighted_undirected.toml");
    let path_str = path.to_str().unwrap();

    let result = ds4viz::graph_weighted_with_output("weighted_undirected", false, Some(path_str), |graph| {
        graph.add_node(0, "A")?;
        graph.add_node(1, "B")?;
        graph.add_node(2, "C")?;

        graph.add_edge(0, 1, 3.5)?;
        graph.add_edge(1, 2, 2.0)?;
        graph.add_edge(0, 2, 5.5)?;

        graph.update_weight(0, 1, 4.0)?;
        graph.update_node_label(0, "A_new")?;
        graph.remove_edge(0, 2)?;

        Ok(())
    });

    assert!(result.is_ok());

    let content = fs::read_to_string(&path).unwrap();
    assert!(content.contains("kind = \"graph_weighted\""));
    assert!(content.contains("weight"));
    assert!(content.contains("op = \"update_weight\""));
}

#[test]
fn test_graph_weighted_directed_full() {
    let dir = tempdir().unwrap();
    let path = dir.path().join("graph_weighted_directed.toml");
    let path_str = path.to_str().unwrap();

    let result = ds4viz::graph_weighted_with_output("weighted_directed", true, Some(path_str), |graph| {
        graph.add_node(0, "A")?;
        graph.add_node(1, "B")?;

        graph.add_edge(0, 1, 3.5)?;
        graph.add_edge(1, 0, 2.0)?; // 反向边

        Ok(())
    });

    assert!(result.is_ok());
}

#[test]
fn test_graph_weighted_update_not_found() {
    let dir = tempdir().unwrap();
    let path = dir.path().join("graph_weighted_update_error.toml");
    let path_str = path.to_str().unwrap();

    let result = ds4viz::graph_weighted_with_output("update_error", false, Some(path_str), |graph| {
        graph.add_node(0, "A")?;
        graph.add_node(1, "B")?;
        graph.update_weight(0, 1, 5.0)?; // 边不存在
        Ok(())
    });

    assert!(result.is_err());

    let content = fs::read_to_string(&path).unwrap();
    assert!(content.contains("[error]"));
    assert!(content.contains("Edge not found"));
}

#[test]
fn test_graph_weighted_negative_weight() {
    let dir = tempdir().unwrap();
    let path = dir.path().join("graph_weighted_negative.toml");
    let path_str = path.to_str().unwrap();

    let result = ds4viz::graph_weighted_with_output("negative_weight", false, Some(path_str), |graph| {
        graph.add_node(0, "A")?;
        graph.add_node(1, "B")?;
        graph.add_edge(0, 1, -5.0)?; // 负权边
        Ok(())
    });

    assert!(result.is_ok()); // 应该允许负权边
}

#[test]
fn test_graph_weighted_zero_weight() {
    let dir = tempdir().unwrap();
    let path = dir.path().join("graph_weighted_zero.toml");
    let path_str = path.to_str().unwrap();

    let result = ds4viz::graph_weighted_with_output("zero_weight", false, Some(path_str), |graph| {
        graph.add_node(0, "A")?;
        graph.add_node(1, "B")?;
        graph.add_edge(0, 1, 0.0)?;
        Ok(())
    });

    assert!(result.is_ok());
}

// ============================================================================
// 值类型测试
// ============================================================================

#[test]
fn test_value_integer_types() {
    let dir = tempdir().unwrap();
    let path = dir.path().join("value_int.toml");
    let path_str = path.to_str().unwrap();

    let result = ds4viz::stack_with_output("int_stack", Some(path_str), |s| {
        s.push(0_i64)?;
        s.push(-100_i64)?;
        s.push(i64::MAX)?;
        s.push(i64::MIN)?;
        s.push(42_i32)?;
        Ok(())
    });

    assert!(result.is_ok());
}

#[test]
fn test_value_float_types() {
    let dir = tempdir().unwrap();
    let path = dir.path().join("value_float.toml");
    let path_str = path.to_str().unwrap();

    let result = ds4viz::stack_with_output("float_stack", Some(path_str), |s| {
        s.push(0.0_f64)?;
        s.push(-3.14_f64)?;
        s.push(f64::MAX)?;
        s.push(1.5_f32)?;
        Ok(())
    });

    assert!(result.is_ok());

    let content = fs::read_to_string(&path).unwrap();
    assert!(content.contains("-3.14"));
}

#[test]
fn test_value_string_types() {
    let dir = tempdir().unwrap();
    let path = dir.path().join("value_string.toml");
    let path_str = path.to_str().unwrap();

    let result = ds4viz::stack_with_output("string_stack", Some(path_str), |s| {
        s.push("")?; // 空字符串
        s.push("hello")?;
        s.push("包含中文")?;
        s.push("with\nnewline")?;
        s.push("with \"quotes\"")?;
        s.push(String::from("owned string"))?;
        Ok(())
    });

    assert!(result.is_ok());

    let content = fs::read_to_string(&path).unwrap();
    assert!(content.contains("\"\""));
    assert!(content.contains("\"hello\""));
    assert!(content.contains("包含中文"));
    assert!(content.contains("\\n")); // 换行符应该被转义
    assert!(content.contains("\\\"quotes\\\"")); // 引号应该被转义
}

#[test]
fn test_value_boolean_types() {
    let dir = tempdir().unwrap();
    let path = dir.path().join("value_bool.toml");
    let path_str = path.to_str().unwrap();

    let result = ds4viz::stack_with_output("bool_stack", Some(path_str), |s| {
        s.push(true)?;
        s.push(false)?;
        Ok(())
    });

    assert!(result.is_ok());

    let content = fs::read_to_string(&path).unwrap();
    assert!(content.contains("true"));
    assert!(content.contains("false"));
}

// ============================================================================
// TOML 输出结构验证测试
// ============================================================================

#[test]
fn test_toml_meta_section() {
    let dir = tempdir().unwrap();
    let path = dir.path().join("toml_meta.toml");
    let path_str = path.to_str().unwrap();

    let result = ds4viz::stack_with_output("meta_test", Some(path_str), |s| {
        s.push(1)?;
        Ok(())
    });

    assert!(result.is_ok());

    let content = fs::read_to_string(&path).unwrap();
    assert!(content.contains("[meta]"));
    assert!(content.contains("created_at = \""));
    assert!(content.contains("lang = \"rust\""));
    assert!(content.contains("lang_version = \""));
}

#[test]
fn test_toml_package_section() {
    let dir = tempdir().unwrap();
    let path = dir.path().join("toml_package.toml");
    let path_str = path.to_str().unwrap();

    let result = ds4viz::stack_with_output("package_test", Some(path_str), |s| {
        s.push(1)?;
        Ok(())
    });

    assert!(result.is_ok());

    let content = fs::read_to_string(&path).unwrap();
    assert!(content.contains("[package]"));
    assert!(content.contains("name = \"ds4viz\""));
    assert!(content.contains("lang = \"rust\""));
    assert!(content.contains("version = \""));
}

#[test]
fn test_toml_states_structure() {
    let dir = tempdir().unwrap();
    let path = dir.path().join("toml_states.toml");
    let path_str = path.to_str().unwrap();

    let result = ds4viz::stack_with_output("states_test", Some(path_str), |s| {
        s.push(10)?;
        s.push(20)?;
        s.pop()?;
        Ok(())
    });

    assert!(result.is_ok());

    let content = fs::read_to_string(&path).unwrap();
    
    // 应该有4个状态（初始状态 + 3个操作后的状态）
    let state_count = content.matches("[[states]]").count();
    assert_eq!(state_count, 4);
    
    // 每个状态都应该有 id 和 data
    assert!(content.contains("id = 0"));
    assert!(content.contains("id = 1"));
    assert!(content.contains("id = 2"));
    assert!(content.contains("id = 3"));
}

#[test]
fn test_toml_steps_structure() {
    let dir = tempdir().unwrap();
    let path = dir.path().join("toml_steps.toml");
    let path_str = path.to_str().unwrap();

    let result = ds4viz::stack_with_output("steps_test", Some(path_str), |s| {
        s.push(10)?;
        s.push(20)?;
        s.pop()?;
        Ok(())
    });

    assert!(result.is_ok());

    let content = fs::read_to_string(&path).unwrap();
    
    // 应该有3个步骤
    let step_count = content.matches("[[steps]]").count();
    assert_eq!(step_count, 3);
    
    // 每个步骤都应该有必需字段
    assert!(content.contains("op = "));
    assert!(content.contains("before = "));
    assert!(content.contains("after = "));
    assert!(content.contains("[steps.args]"));
}

#[test]
fn test_toml_result_section() {
    let dir = tempdir().unwrap();
    let path = dir.path().join("toml_result.toml");
    let path_str = path.to_str().unwrap();

    let result = ds4viz::stack_with_output("result_test", Some(path_str), |s| {
        s.push(10)?;
        Ok(())
    });

    assert!(result.is_ok());

    let content = fs::read_to_string(&path).unwrap();
    assert!(content.contains("[result]"));
    assert!(content.contains("final_state = 1"));
    assert!(content.contains("[result.commit]"));
    assert!(content.contains("op = \"commit\""));
}

#[test]
fn test_toml_error_section_has_step() {
    let dir = tempdir().unwrap();
    let path = dir.path().join("toml_error_step.toml");
    let path_str = path.to_str().unwrap();

    let result = ds4viz::stack_with_output("error_step_test", Some(path_str), |s| {
        s.push(10)?;
        s.push(20)?;
        s.pop()?;
        s.pop()?;
        s.pop()?; // 这里失败
        Ok(())
    });

    assert!(result.is_err());

    let content = fs::read_to_string(&path).unwrap();
    assert!(content.contains("[error]"));
    assert!(content.contains("type = \"runtime\""));
    assert!(content.contains("message = "));
    assert!(content.contains("step = 4")); // 第5步（索引4）失败
    assert!(content.contains("last_state = 4")); // 最后一个成功的状态
}

// ============================================================================
// 边缘情况测试
// ============================================================================

#[test]
fn test_empty_label() {
    let dir = tempdir().unwrap();
    let path = dir.path().join("empty_label.toml");
    let path_str = path.to_str().unwrap();

    let result = ds4viz::stack_with_output("", Some(path_str), |s| {
        s.push(1)?;
        Ok(())
    });

    assert!(result.is_ok());

    let content = fs::read_to_string(&path).unwrap();
    // 空标签应该不输出 label 字段或输出空字符串
    assert!(content.contains("kind = \"stack\""));
}

#[test]
fn test_unicode_in_values() {
    let dir = tempdir().unwrap();
    let path = dir.path().join("unicode.toml");
    let path_str = path.to_str().unwrap();

    let result = ds4viz::stack_with_output("unicode_test", Some(path_str), |s| {
        s.push("你好世界")?;
        s.push("🎉🎊🎁")?;
        s.push("日本語")?;
        s.push("العربية")?;
        Ok(())
    });

    assert!(result.is_ok());

    let content = fs::read_to_string(&path).unwrap();
    assert!(content.contains("你好世界"));
    assert!(content.contains("🎉🎊🎁"));
    assert!(content.contains("日本語"));
    assert!(content.contains("العربية"));
}

#[test]
fn test_special_characters_in_strings() {
    let dir = tempdir().unwrap();
    let path = dir.path().join("special_chars.toml");
    let path_str = path.to_str().unwrap();

    let result = ds4viz::stack_with_output("special_test", Some(path_str), |s| {
        s.push("line1\nline2")?;
        s.push("tab\there")?;
        s.push("quote\"here")?;
        s.push("backslash\\here")?;
        s.push("carriage\rreturn")?;
        Ok(())
    });

    assert!(result.is_ok());

    let content = fs::read_to_string(&path).unwrap();
    assert!(content.contains("\\n")); // 换行符被转义
    assert!(content.contains("\\\"")); // 引号被转义
    assert!(content.contains("\\\\")); // 反斜杠被转义
}

#[test]
fn test_many_operations_sequence() {
    let dir = tempdir().unwrap();
    let path = dir.path().join("many_ops.toml");
    let path_str = path.to_str().unwrap();

    let result = ds4viz::binary_tree_with_output("many_ops_tree", Some(path_str), |tree| {
        // 构建完整的二叉树
        let root = tree.insert_root(50)?;
        
        let left = tree.insert_left(root, 25)?;
        let right = tree.insert_right(root, 75)?;
        
        tree.insert_left(left, 12)?;
        tree.insert_right(left, 37)?;
        
        let rl = tree.insert_left(right, 62)?;
        tree.insert_right(right, 87)?;
        
        // 更新一些值
        tree.update_value(left, 30)?;
        tree.update_value(rl, 65)?;
        
        // 删除子树
        tree.delete(left)?;
        
        Ok(())
    });

    assert!(result.is_ok());
}

#[test]
fn test_graph_complete() {
    let dir = tempdir().unwrap();
    let path = dir.path().join("graph_complete.toml");
    let path_str = path.to_str().unwrap();

    let result = ds4viz::graph_undirected_with_output("complete_graph", Some(path_str), |graph| {
        // 创建完全图 K5
        for i in 0..5 {
            graph.add_node(i, &format!("V{}", i))?;
        }
        
        for i in 0..5 {
            for j in (i + 1)..5 {
                graph.add_edge(i, j)?;
            }
        }
        
        Ok(())
    });

    assert!(result.is_ok());

    let content = fs::read_to_string(&path).unwrap();
    // K5 有 5 个节点和 10 条边
    let node_count = content.matches("add_node").count();
    let edge_count = content.matches("add_edge").count();
    assert_eq!(node_count, 5);
    assert_eq!(edge_count, 10);
}

// ============================================================================
// 返回值测试
// ============================================================================

#[test]
fn test_stack_pop_return_value() {
    let dir = tempdir().unwrap();
    let path = dir.path().join("stack_ret.toml");
    let path_str = path.to_str().unwrap();

    let result = ds4viz::stack_with_output("ret_stack", Some(path_str), |s| {
        s.push(42)?;
        s.pop()?;
        Ok(())
    });

    assert!(result.is_ok());

    let content = fs::read_to_string(&path).unwrap();
    assert!(content.contains("ret = 42"));
}

#[test]
fn test_queue_dequeue_return_value() {
    let dir = tempdir().unwrap();
    let path = dir.path().join("queue_ret.toml");
    let path_str = path.to_str().unwrap();

    let result = ds4viz::queue_with_output("ret_queue", Some(path_str), |q| {
        q.enqueue(100)?;
        q.enqueue(200)?;
        q.dequeue()?;
        Ok(())
    });

    assert!(result.is_ok());

    let content = fs::read_to_string(&path).unwrap();
    assert!(content.contains("ret = 100"));
}

#[test]
fn test_linked_list_insert_return_id() {
    let dir = tempdir().unwrap();
    let path = dir.path().join("list_ret.toml");
    let path_str = path.to_str().unwrap();

    let result = ds4viz::single_linked_list_with_output("ret_list", Some(path_str), |list| {
        let id = list.insert_head(10)?;
        assert_eq!(id, 0);
        
        let id = list.insert_head(20)?;
        assert_eq!(id, 1);
        
        Ok(())
    });

    assert!(result.is_ok());

    let content = fs::read_to_string(&path).unwrap();
    assert!(content.contains("ret = 0"));
    assert!(content.contains("ret = 1"));
}

#[test]
fn test_bst_insert_return_id() {
    let dir = tempdir().unwrap();
    let path = dir.path().join("bst_ret.toml");
    let path_str = path.to_str().unwrap();

    let result = ds4viz::binary_search_tree_with_output("ret_bst", Some(path_str), |bst| {
        let id = bst.insert(50)?;
        assert_eq!(id, 0);
        
        let id = bst.insert(30)?;
        assert_eq!(id, 1);
        
        Ok(())
    });

    assert!(result.is_ok());
}

#[test]
fn test_heap_extract_return_value() {
    let dir = tempdir().unwrap();
    let path = dir.path().join("heap_ret.toml");
    let path_str = path.to_str().unwrap();

    let result = ds4viz::heap_with_output("ret_heap", HeapType::Min, Some(path_str), |heap| {
        heap.insert(30)?;
        heap.insert(10)?;
        heap.insert(20)?;
        heap.extract()?;
        Ok(())
    });

    assert!(result.is_ok());

    let content = fs::read_to_string(&path).unwrap();
    assert!(content.contains("ret = 10")); // 最小值
}

#[test]
fn test_graph_update_return_old_value() {
    let dir = tempdir().unwrap();
    let path = dir.path().join("graph_ret.toml");
    let path_str = path.to_str().unwrap();

    let result = ds4viz::graph_undirected_with_output("ret_graph", Some(path_str), |graph| {
        graph.add_node(0, "A")?;
        graph.update_node_label(0, "B")?;
        Ok(())
    });

    assert!(result.is_ok());

    let content = fs::read_to_string(&path).unwrap();
    assert!(content.contains("ret = \"A\"")); // 旧标签
}

// ============================================================================
// 浮点数精度测试
// ============================================================================

#[test]
fn test_float_precision() {
    let dir = tempdir().unwrap();
    let path = dir.path().join("float_precision.toml");
    let path_str = path.to_str().unwrap();

    let result = ds4viz::stack_with_output("float_stack", Some(path_str), |s| {
        s.push(0.1 + 0.2)?; // 典型的浮点精度问题
        s.push(1.0 / 3.0)?;
        s.push(std::f64::consts::PI)?;
        s.push(std::f64::consts::E)?;
        Ok(())
    });

    assert!(result.is_ok());
}

// ============================================================================
// 极端情况测试
// ============================================================================

#[test]
fn test_very_deep_tree() {
    let dir = tempdir().unwrap();
    let path = dir.path().join("deep_tree.toml");
    let path_str = path.to_str().unwrap();

    let result = ds4viz::binary_tree_with_output("deep_tree", Some(path_str), |tree| {
        let mut current = tree.insert_root(0)?;
        for i in 1..20 {
            current = tree.insert_left(current, i)?;
        }
        Ok(())
    });

    assert!(result.is_ok());
}

#[test]
fn test_wide_tree() {
    let dir = tempdir().unwrap();
    let path = dir.path().join("wide_tree.toml");
    let path_str = path.to_str().unwrap();

    let result = ds4viz::binary_tree_with_output("wide_tree", Some(path_str), |tree| {
        let root = tree.insert_root(0)?;
        let left = tree.insert_left(root, 1)?;
        let right = tree.insert_right(root, 2)?;
        
        // 继续扩展
        tree.insert_left(left, 3)?;
        tree.insert_right(left, 4)?;
        tree.insert_left(right, 5)?;
        tree.insert_right(right, 6)?;
        
        Ok(())
    });

    assert!(result.is_ok());
}