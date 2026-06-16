//! 冒烟测试
//!
//! 基本功能测试，确保库可以正常编译和运行
//!
//! # 作者
//!
//! WaterRun
//!
//! # 文件路径
//!
//! tests/smoke.rs
//!
//! # 日期
//!
//! 2025-12-25

use ds4viz::{
    config, heap, queue, stack, Config, HeapType,
};
use std::fs;
use tempfile::tempdir;

// ============================================================================
// 栈基础测试
// ============================================================================

#[test]
fn test_stack_basic_push_pop() {
    let dir = tempdir().unwrap();
    let path = dir.path().join("stack_basic.toml");
    let path_str = path.to_str().unwrap();

    let result = ds4viz::stack_with_output("test_stack", Some(path_str), |s| {
        s.push(10)?;
        s.push(20)?;
        s.push(30)?;
        s.pop()?;
        Ok(())
    });

    assert!(result.is_ok());
    assert!(path.exists());

    let content = fs::read_to_string(&path).unwrap();
    assert!(content.contains("[meta]"));
    assert!(content.contains("[package]"));
    assert!(content.contains("[object]"));
    assert!(content.contains("kind = \"stack\""));
    assert!(content.contains("[result]"));
    assert!(content.contains("final_state"));
}

#[test]
fn test_stack_empty_operations() {
    let dir = tempdir().unwrap();
    let path = dir.path().join("stack_empty.toml");
    let path_str = path.to_str().unwrap();

    let result = ds4viz::stack_with_output("empty_stack", Some(path_str), |_s| {
        // 不进行任何操作
        Ok(())
    });

    assert!(result.is_ok());

    let content = fs::read_to_string(&path).unwrap();
    assert!(content.contains("items = []"));
    assert!(content.contains("top = -1"));
}

#[test]
fn test_stack_clear_operation() {
    let dir = tempdir().unwrap();
    let path = dir.path().join("stack_clear.toml");
    let path_str = path.to_str().unwrap();

    let result = ds4viz::stack_with_output("clear_stack", Some(path_str), |s| {
        s.push(1)?;
        s.push(2)?;
        s.push(3)?;
        s.clear()?;
        s.push(100)?;
        Ok(())
    });

    assert!(result.is_ok());

    let content = fs::read_to_string(&path).unwrap();
    assert!(content.contains("op = \"clear\""));
}

#[test]
fn test_stack_convenience_function() {
    // 测试不带输出路径的便捷函数
    let result = stack("convenience_stack", |s| {
        s.push(42)?;
        Ok(())
    });

    // 默认输出到 trace.toml
    assert!(result.is_ok());
    
    // 清理默认文件
    let _ = fs::remove_file("trace.toml");
}

// ============================================================================
// 队列基础测试
// ============================================================================

#[test]
fn test_queue_basic_enqueue_dequeue() {
    let dir = tempdir().unwrap();
    let path = dir.path().join("queue_basic.toml");
    let path_str = path.to_str().unwrap();

    let result = ds4viz::queue_with_output("test_queue", Some(path_str), |q| {
        q.enqueue(10)?;
        q.enqueue(20)?;
        q.dequeue()?;
        Ok(())
    });

    assert!(result.is_ok());
    assert!(path.exists());

    let content = fs::read_to_string(&path).unwrap();
    assert!(content.contains("kind = \"queue\""));
    assert!(content.contains("op = \"enqueue\""));
    assert!(content.contains("op = \"dequeue\""));
}

#[test]
fn test_queue_empty_state() {
    let dir = tempdir().unwrap();
    let path = dir.path().join("queue_empty.toml");
    let path_str = path.to_str().unwrap();

    let result = ds4viz::queue_with_output("empty_queue", Some(path_str), |_q| Ok(()));

    assert!(result.is_ok());

    let content = fs::read_to_string(&path).unwrap();
    assert!(content.contains("front = -1"));
    assert!(content.contains("rear = -1"));
}

#[test]
fn test_queue_clear_operation() {
    let dir = tempdir().unwrap();
    let path = dir.path().join("queue_clear.toml");
    let path_str = path.to_str().unwrap();

    let result = ds4viz::queue_with_output("clear_queue", Some(path_str), |q| {
        q.enqueue(1)?;
        q.enqueue(2)?;
        q.clear()?;
        q.enqueue(99)?;
        Ok(())
    });

    assert!(result.is_ok());

    let content = fs::read_to_string(&path).unwrap();
    assert!(content.contains("op = \"clear\""));
}

#[test]
fn test_queue_convenience_function() {
    let result = queue("convenience_queue", |q| {
        q.enqueue(42)?;
        Ok(())
    });

    assert!(result.is_ok());
    let _ = fs::remove_file("trace.toml");
}

// ============================================================================
// 图基础测试
// ============================================================================

#[test]
fn test_graph_undirected_basic() {
    let dir = tempdir().unwrap();
    let path = dir.path().join("graph_undirected.toml");
    let path_str = path.to_str().unwrap();

    let result = ds4viz::graph_undirected_with_output("test_graph", Some(path_str), |g| {
        g.add_node(0, "A")?;
        g.add_node(1, "B")?;
        g.add_node(2, "C")?;
        g.add_edge(0, 1)?;
        g.add_edge(1, 2)?;
        Ok(())
    });

    assert!(result.is_ok());
    assert!(path.exists());

    let content = fs::read_to_string(&path).unwrap();
    assert!(content.contains("kind = \"graph_undirected\""));
}

#[test]
fn test_graph_directed_basic() {
    let dir = tempdir().unwrap();
    let path = dir.path().join("graph_directed.toml");
    let path_str = path.to_str().unwrap();

    let result = ds4viz::graph_directed_with_output("test_graph", Some(path_str), |g| {
        g.add_node(0, "A")?;
        g.add_node(1, "B")?;
        g.add_edge(0, 1)?;
        Ok(())
    });

    assert!(result.is_ok());

    let content = fs::read_to_string(&path).unwrap();
    assert!(content.contains("kind = \"graph_directed\""));
}

#[test]
fn test_graph_weighted_basic() {
    let dir = tempdir().unwrap();
    let path = dir.path().join("graph_weighted.toml");
    let path_str = path.to_str().unwrap();

    let result = ds4viz::graph_weighted_with_output("test_graph", false, Some(path_str), |g| {
        g.add_node(0, "A")?;
        g.add_node(1, "B")?;
        g.add_edge(0, 1, 5.5)?;
        Ok(())
    });

    assert!(result.is_ok());

    let content = fs::read_to_string(&path).unwrap();
    assert!(content.contains("kind = \"graph_weighted\""));
    assert!(content.contains("weight"));
}

// ============================================================================
// 二叉树基础测试
// ============================================================================

#[test]
fn test_binary_tree_basic() {
    let dir = tempdir().unwrap();
    let path = dir.path().join("binary_tree.toml");
    let path_str = path.to_str().unwrap();

    let result = ds4viz::binary_tree_with_output("test_tree", Some(path_str), |tree| {
        let root = tree.insert_root(10)?;
        tree.insert_left(root, 5)?;
        tree.insert_right(root, 15)?;
        Ok(())
    });

    assert!(result.is_ok());
    assert!(path.exists());

    let content = fs::read_to_string(&path).unwrap();
    assert!(content.contains("kind = \"binary_tree\""));
    assert!(content.contains("root = 0"));
}

#[test]
fn test_binary_search_tree_basic() {
    let dir = tempdir().unwrap();
    let path = dir.path().join("bst.toml");
    let path_str = path.to_str().unwrap();

    let result = ds4viz::binary_search_tree_with_output("test_bst", Some(path_str), |bst| {
        bst.insert(10)?;
        bst.insert(5)?;
        bst.insert(15)?;
        Ok(())
    });

    assert!(result.is_ok());

    let content = fs::read_to_string(&path).unwrap();
    assert!(content.contains("kind = \"bst\""));
}

#[test]
fn test_heap_basic_min() {
    let dir = tempdir().unwrap();
    let path = dir.path().join("heap_min.toml");
    let path_str = path.to_str().unwrap();

    let result = ds4viz::heap_with_output("min_heap", HeapType::Min, Some(path_str), |h| {
        h.insert(10)?;
        h.insert(5)?;
        h.insert(15)?;
        h.extract()?;
        Ok(())
    });

    assert!(result.is_ok());
}

#[test]
fn test_heap_basic_max() {
    let dir = tempdir().unwrap();
    let path = dir.path().join("heap_max.toml");
    let path_str = path.to_str().unwrap();

    let result = ds4viz::heap_with_output("max_heap", HeapType::Max, Some(path_str), |h| {
        h.insert(10)?;
        h.insert(5)?;
        h.insert(15)?;
        h.extract()?;
        Ok(())
    });

    assert!(result.is_ok());
}

#[test]
fn test_heap_convenience_function() {
    let result = heap("convenience_heap", HeapType::Min, |h| {
        h.insert(42)?;
        Ok(())
    });

    assert!(result.is_ok());
    let _ = fs::remove_file("trace.toml");
}

// ============================================================================
// 链表基础测试
// ============================================================================

#[test]
fn test_single_linked_list_basic() {
    let dir = tempdir().unwrap();
    let path = dir.path().join("slist.toml");
    let path_str = path.to_str().unwrap();

    let result = ds4viz::single_linked_list_with_output("test_slist", Some(path_str), |list| {
        list.insert_head(10)?;
        list.insert_tail(20)?;
        Ok(())
    });

    assert!(result.is_ok());

    let content = fs::read_to_string(&path).unwrap();
    assert!(content.contains("kind = \"slist\""));
    assert!(content.contains("head"));
}

#[test]
fn test_double_linked_list_basic() {
    let dir = tempdir().unwrap();
    let path = dir.path().join("dlist.toml");
    let path_str = path.to_str().unwrap();

    let result = ds4viz::double_linked_list_with_output("test_dlist", Some(path_str), |list| {
        list.insert_head(10)?;
        list.insert_tail(20)?;
        Ok(())
    });

    assert!(result.is_ok());

    let content = fs::read_to_string(&path).unwrap();
    assert!(content.contains("kind = \"dlist\""));
    assert!(content.contains("head"));
    assert!(content.contains("tail"));
}

// ============================================================================
// 错误处理基础测试
// ============================================================================

#[test]
fn test_error_stack_pop_empty() {
    let dir = tempdir().unwrap();
    let path = dir.path().join("error_stack.toml");
    let path_str = path.to_str().unwrap();

    let result = ds4viz::stack_with_output("error_stack", Some(path_str), |s| {
        s.pop()?;
        Ok(())
    });

    assert!(result.is_err());
    assert!(path.exists());

    let content = fs::read_to_string(&path).unwrap();
    assert!(content.contains("[error]"));
    assert!(content.contains("type = \"runtime\""));
    assert!(content.contains("Structure is empty"));
}

#[test]
fn test_error_queue_dequeue_empty() {
    let dir = tempdir().unwrap();
    let path = dir.path().join("error_queue.toml");
    let path_str = path.to_str().unwrap();

    let result = ds4viz::queue_with_output("error_queue", Some(path_str), |q| {
        q.dequeue()?;
        Ok(())
    });

    assert!(result.is_err());

    let content = fs::read_to_string(&path).unwrap();
    assert!(content.contains("[error]"));
}

#[test]
fn test_error_graph_self_loop() {
    let dir = tempdir().unwrap();
    let path = dir.path().join("error_self_loop.toml");
    let path_str = path.to_str().unwrap();

    let result = ds4viz::graph_undirected_with_output("error_graph", Some(path_str), |g| {
        g.add_node(0, "A")?;
        g.add_edge(0, 0)?;
        Ok(())
    });

    assert!(result.is_err());

    let content = fs::read_to_string(&path).unwrap();
    assert!(content.contains("[error]"));
    assert!(content.contains("Self-loop"));
}

// ============================================================================
// 配置测试
// ============================================================================

#[test]
fn test_config_with_remarks() {
    let dir = tempdir().unwrap();
    let path = dir.path().join("config_test.toml");
    let path_str = path.to_str().unwrap();

    config(Config {
        output_path: String::new(),
        title: "测试标题".into(),
        author: "WaterRun".into(),
        comment: "这是一个测试".into(),
    });

    let result = ds4viz::stack_with_output("config_stack", Some(path_str), |s| {
        s.push(1)?;
        Ok(())
    });

    assert!(result.is_ok());

    let content = fs::read_to_string(&path).unwrap();
    assert!(content.contains("[remarks]"));
    assert!(content.contains("title = \"测试标题\""));
    assert!(content.contains("author = \"WaterRun\""));

    // 重置配置
    config(Config {
        output_path: String::new(),
        title: String::new(),
        author: String::new(),
        comment: String::new(),
    });
}

#[test]
fn test_config_builder() {
    let cfg = Config::builder()
        .output_path("test.toml")
        .title("Title")
        .author("Author")
        .comment("Comment")
        .build();

    assert_eq!(cfg.output_path, "test.toml");
    assert_eq!(cfg.title, "Title");
    assert_eq!(cfg.author, "Author");
    assert_eq!(cfg.comment, "Comment");
}

// ============================================================================
// TOML 结构验证测试
// ============================================================================

#[test]
fn test_toml_structure_complete() {
    let dir = tempdir().unwrap();
    let path = dir.path().join("structure.toml");
    let path_str = path.to_str().unwrap();

    let result = ds4viz::stack_with_output("structure_stack", Some(path_str), |s| {
        s.push(10)?;
        s.push(20)?;
        s.pop()?;
        Ok(())
    });

    assert!(result.is_ok());

    let content = fs::read_to_string(&path).unwrap();

    // 验证 meta 部分
    assert!(content.contains("[meta]"));
    assert!(content.contains("created_at"));
    assert!(content.contains("lang = \"rust\""));
    assert!(content.contains("lang_version"));

    // 验证 package 部分
    assert!(content.contains("[package]"));
    assert!(content.contains("name = \"ds4viz\""));
    assert!(content.contains("lang = \"rust\""));
    assert!(content.contains("version"));

    // 验证 object 部分
    assert!(content.contains("[object]"));
    assert!(content.contains("kind = \"stack\""));
    assert!(content.contains("label = \"structure_stack\""));

    // 验证 states 部分
    assert!(content.contains("[[states]]"));
    assert!(content.contains("[states.data]"));
    assert!(content.contains("items"));
    assert!(content.contains("top"));

    // 验证 steps 部分
    assert!(content.contains("[[steps]]"));
    assert!(content.contains("[steps.args]"));
    assert!(content.contains("op = \"push\""));
    assert!(content.contains("op = \"pop\""));
    assert!(content.contains("before"));
    assert!(content.contains("after"));

    // 验证 result 部分
    assert!(content.contains("[result]"));
    assert!(content.contains("final_state"));
    assert!(content.contains("[result.commit]"));
}

#[test]
fn test_toml_error_structure() {
    let dir = tempdir().unwrap();
    let path = dir.path().join("error_structure.toml");
    let path_str = path.to_str().unwrap();

    let result = ds4viz::stack_with_output("error_structure", Some(path_str), |s| {
        s.push(10)?;
        s.pop()?;
        s.pop()?; // 错误
        Ok(())
    });

    assert!(result.is_err());

    let content = fs::read_to_string(&path).unwrap();

    // 验证有 error 部分
    assert!(content.contains("[error]"));
    assert!(content.contains("type"));
    assert!(content.contains("message"));

    // 验证没有 result 部分
    assert!(!content.contains("[result]"));
}
