//! 树形数据结构实现
//!
//! 包含二叉树、二叉搜索树、堆的实现
//!
//! # 作者
//!
//! WaterRun
//!
//! # 文件路径
//!
//! src/structures/tree.rs
//!
//! # 日期
//!
//! 2025-12-25

use crate::error::{Result, StructureError};
use crate::session::Session;
use crate::structures::base::BaseStructure;
use crate::trace::{NumericValue, Value};
use std::collections::HashMap;

/// 二叉树节点
#[derive(Debug, Clone)]
struct TreeNode {
    /// 节点 ID
    id: i64,
    /// 节点值
    value: Value,
    /// 左子节点 ID
    left: i64,
    /// 右子节点 ID
    right: i64,
}

/// 二叉树数据结构
pub struct BinaryTree<'a> {
    /// 会话引用
    session: &'a mut Session,
    /// 节点映射
    nodes: HashMap<i64, TreeNode>,
    /// 根节点 ID
    root: i64,
    /// 下一个节点 ID
    next_id: i64,
}

impl<'a> BinaryTree<'a> {
    /// 创建新的二叉树实例
    #[must_use]
    pub fn new(session: &'a mut Session) -> Self {
        let mut tree = Self {
            session,
            nodes: HashMap::new(),
            root: -1,
            next_id: 0,
        };
        tree.initialize();
        tree
    }

    /// 插入根节点
    ///
    /// # 错误
    ///
    /// 当根节点已存在时返回 `StructureError::RootExists`
    pub fn insert_root(&mut self, value: impl Into<Value>) -> Result<i64> {
        if self.root != -1 {
            self.session.set_failed_step_id(self.session.step_counter());
            return Err(StructureError::RootExists.into());
        }

        let value = value.into();
        let new_id = self.next_id;
        self.next_id += 1;

        let node = TreeNode {
            id: new_id,
            value: value.clone(),
            left: -1,
            right: -1,
        };
        self.nodes.insert(new_id, node);
        self.root = new_id;

        let mut args = HashMap::new();
        args.insert("value".to_string(), value);
        self.record_step("insert_root", args, Some(Value::Integer(new_id)), None)?;

        Ok(new_id)
    }

    /// 在指定父节点的左侧插入子节点
    ///
    /// # 错误
    ///
    /// * 当父节点不存在时返回 `StructureError::NodeNotFound`
    /// * 当左子节点已存在时返回 `StructureError::ChildExists`
    pub fn insert_left(&mut self, parent_id: i64, value: impl Into<Value>) -> Result<i64> {
        if !self.nodes.contains_key(&parent_id) {
            self.session.set_failed_step_id(self.session.step_counter());
            return Err(StructureError::NodeNotFound(parent_id).into());
        }

        if self.nodes.get(&parent_id).map_or(false, |n| n.left != -1) {
            self.session.set_failed_step_id(self.session.step_counter());
            return Err(StructureError::ChildExists.into());
        }

        let value = value.into();
        let new_id = self.next_id;
        self.next_id += 1;

        let node = TreeNode {
            id: new_id,
            value: value.clone(),
            left: -1,
            right: -1,
        };
        self.nodes.insert(new_id, node);

        if let Some(parent) = self.nodes.get_mut(&parent_id) {
            parent.left = new_id;
        }

        let mut args = HashMap::new();
        args.insert("parent".to_string(), Value::Integer(parent_id));
        args.insert("value".to_string(), value);
        self.record_step("insert_left", args, Some(Value::Integer(new_id)), None)?;

        Ok(new_id)
    }

    /// 在指定父节点的右侧插入子节点
    ///
    /// # 错误
    ///
    /// * 当父节点不存在时返回 `StructureError::NodeNotFound`
    /// * 当右子节点已存在时返回 `StructureError::ChildExists`
    pub fn insert_right(&mut self, parent_id: i64, value: impl Into<Value>) -> Result<i64> {
        if !self.nodes.contains_key(&parent_id) {
            self.session.set_failed_step_id(self.session.step_counter());
            return Err(StructureError::NodeNotFound(parent_id).into());
        }

        if self.nodes.get(&parent_id).map_or(false, |n| n.right != -1) {
            self.session.set_failed_step_id(self.session.step_counter());
            return Err(StructureError::ChildExists.into());
        }

        let value = value.into();
        let new_id = self.next_id;
        self.next_id += 1;

        let node = TreeNode {
            id: new_id,
            value: value.clone(),
            left: -1,
            right: -1,
        };
        self.nodes.insert(new_id, node);

        if let Some(parent) = self.nodes.get_mut(&parent_id) {
            parent.right = new_id;
        }

        let mut args = HashMap::new();
        args.insert("parent".to_string(), Value::Integer(parent_id));
        args.insert("value".to_string(), value);
        self.record_step("insert_right", args, Some(Value::Integer(new_id)), None)?;

        Ok(new_id)
    }

    /// 删除指定节点及其所有子树
    ///
    /// # 错误
    ///
    /// 当节点不存在时返回 `StructureError::NodeNotFound`
    pub fn delete(&mut self, node_id: i64) -> Result<()> {
        if !self.nodes.contains_key(&node_id) {
            self.session.set_failed_step_id(self.session.step_counter());
            return Err(StructureError::NodeNotFound(node_id).into());
        }

        let subtree_ids = self.collect_subtree(node_id);
        let parent_id = self.find_parent(node_id);

        if parent_id != -1 {
            if let Some(parent) = self.nodes.get_mut(&parent_id) {
                if parent.left == node_id {
                    parent.left = -1;
                } else if parent.right == node_id {
                    parent.right = -1;
                }
            }
        } else {
            self.root = -1;
        }

        for id in subtree_ids {
            self.nodes.remove(&id);
        }

        let mut args = HashMap::new();
        args.insert("node_id".to_string(), Value::Integer(node_id));
        self.record_step("delete", args, None, None)?;

        Ok(())
    }

    /// 更新节点的值
    ///
    /// # 错误
    ///
    /// 当节点不存在时返回 `StructureError::NodeNotFound`
    pub fn update_value(&mut self, node_id: i64, value: impl Into<Value>) -> Result<()> {
        if !self.nodes.contains_key(&node_id) {
            self.session.set_failed_step_id(self.session.step_counter());
            return Err(StructureError::NodeNotFound(node_id).into());
        }

        let value = value.into();
        let old_value = self.nodes.get(&node_id).map(|n| n.value.clone());

        if let Some(node) = self.nodes.get_mut(&node_id) {
            node.value = value.clone();
        }

        let mut args = HashMap::new();
        args.insert("node_id".to_string(), Value::Integer(node_id));
        args.insert("value".to_string(), value);
        self.record_step("update_value", args, old_value, None)?;

        Ok(())
    }

    /// 收集以指定节点为根的子树中所有节点 ID
    fn collect_subtree(&self, node_id: i64) -> Vec<i64> {
        if node_id == -1 {
            return Vec::new();
        }

        let mut result = vec![node_id];
        if let Some(node) = self.nodes.get(&node_id) {
            result.extend(self.collect_subtree(node.left));
            result.extend(self.collect_subtree(node.right));
        }
        result
    }

    /// 查找指定节点的父节点 ID
    fn find_parent(&self, node_id: i64) -> i64 {
        for (id, node) in &self.nodes {
            if node.left == node_id || node.right == node_id {
                return *id;
            }
        }
        -1
    }
}

impl<'a> BaseStructure for BinaryTree<'a> {
    fn build_data(&self) -> HashMap<String, Value> {
        let mut nodes_list: Vec<Value> = self
            .nodes
            .values()
            .map(|n| {
                let mut table = HashMap::new();
                table.insert("id".to_string(), Value::Integer(n.id));
                table.insert("value".to_string(), n.value.clone());
                table.insert("left".to_string(), Value::Integer(n.left));
                table.insert("right".to_string(), Value::Integer(n.right));
                Value::Table(table)
            })
            .collect();

        nodes_list.sort_by(|a, b| {
            if let (Value::Table(ta), Value::Table(tb)) = (a, b) {
                if let (Some(Value::Integer(ia)), Some(Value::Integer(ib))) =
                    (ta.get("id"), tb.get("id"))
                {
                    return ia.cmp(ib);
                }
            }
            std::cmp::Ordering::Equal
        });

        let mut data = HashMap::new();
        data.insert("root".to_string(), Value::Integer(self.root));
        data.insert("nodes".to_string(), Value::Array(nodes_list));
        data
    }

    fn session(&self) -> &Session {
        self.session
    }

    fn session_mut(&mut self) -> &mut Session {
        self.session
    }
}

/// BST 节点
#[derive(Debug, Clone)]
struct BstNode {
    /// 节点 ID
    id: i64,
    /// 节点值
    value: NumericValue,
    /// 左子节点 ID
    left: i64,
    /// 右子节点 ID
    right: i64,
}

/// 二叉搜索树数据结构
pub struct BinarySearchTree<'a> {
    /// 会话引用
    session: &'a mut Session,
    /// 节点映射
    nodes: HashMap<i64, BstNode>,
    /// 根节点 ID
    root: i64,
    /// 下一个节点 ID
    next_id: i64,
}

impl<'a> BinarySearchTree<'a> {
    /// 创建新的二叉搜索树实例
    #[must_use]
    pub fn new(session: &'a mut Session) -> Self {
        let mut tree = Self {
            session,
            nodes: HashMap::new(),
            root: -1,
            next_id: 0,
        };
        tree.initialize();
        tree
    }

    /// 插入节点，自动维护二叉搜索树性质
    ///
    /// # 返回
    ///
    /// 返回新插入节点的 id
    pub fn insert(&mut self, value: impl Into<NumericValue>) -> Result<i64> {
        let value = value.into();
        let new_id = self.next_id;
        self.next_id += 1;

        let node = BstNode {
            id: new_id,
            value: value.clone(),
            left: -1,
            right: -1,
        };
        self.nodes.insert(new_id, node);

        if self.root == -1 {
            self.root = new_id;
        } else {
            let mut current = self.root;
            loop {
                let current_value = self.nodes.get(&current).map(|n| n.value.clone()).unwrap();
                if value.cmp_value(&current_value) == std::cmp::Ordering::Less {
                    let left = self.nodes.get(&current).map_or(-1, |n| n.left);
                    if left == -1 {
                        if let Some(node) = self.nodes.get_mut(&current) {
                            node.left = new_id;
                        }
                        break;
                    }
                    current = left;
                } else {
                    let right = self.nodes.get(&current).map_or(-1, |n| n.right);
                    if right == -1 {
                        if let Some(node) = self.nodes.get_mut(&current) {
                            node.right = new_id;
                        }
                        break;
                    }
                    current = right;
                }
            }
        }

        let mut args = HashMap::new();
        args.insert("value".to_string(), value.into());
        self.record_step("insert", args, Some(Value::Integer(new_id)), None)?;

        Ok(new_id)
    }

    /// 删除节点，自动维护二叉搜索树性质
    ///
    /// # 错误
    ///
    /// 当节点不存在时返回 `StructureError::ValueNotFound`
    pub fn delete(&mut self, value: impl Into<NumericValue>) -> Result<()> {
        let value = value.into();
        let node_id = self.find_node_by_value(&value);

        if node_id == -1 {
            self.session.set_failed_step_id(self.session.step_counter());
            return Err(StructureError::ValueNotFound(value.to_string()).into());
        }

        self.delete_node(node_id);

        let mut args = HashMap::new();
        args.insert("value".to_string(), value.into());
        self.record_step("delete", args, None, None)?;

        Ok(())
    }

    /// 根据值查找节点 ID
    fn find_node_by_value(&self, value: &NumericValue) -> i64 {
        for (id, node) in &self.nodes {
            if node.value.cmp_value(value) == std::cmp::Ordering::Equal {
                return *id;
            }
        }
        -1
    }

    /// 查找指定节点的父节点 ID
    fn find_parent(&self, node_id: i64) -> i64 {
        for (id, node) in &self.nodes {
            if node.left == node_id || node.right == node_id {
                return *id;
            }
        }
        -1
    }

    /// 查找以指定节点为根的子树中的最小值节点
    fn find_min_node(&self, node_id: i64) -> i64 {
        let mut current = node_id;
        while let Some(node) = self.nodes.get(&current) {
            if node.left == -1 {
                return current;
            }
            current = node.left;
        }
        current
    }

    /// 删除指定节点并维护 BST 性质
    fn delete_node(&mut self, node_id: i64) {
        let (left_child, right_child) = {
            let node = self.nodes.get(&node_id).unwrap();
            (node.left, node.right)
        };
        let parent_id = self.find_parent(node_id);

        if left_child == -1 && right_child == -1 {
            // 叶节点
            self.replace_in_parent(parent_id, node_id, -1);
            self.nodes.remove(&node_id);
        } else if left_child == -1 {
            // 只有右子节点
            self.replace_in_parent(parent_id, node_id, right_child);
            self.nodes.remove(&node_id);
        } else if right_child == -1 {
            // 只有左子节点
            self.replace_in_parent(parent_id, node_id, left_child);
            self.nodes.remove(&node_id);
        } else {
            // 有两个子节点
            let successor_id = self.find_min_node(right_child);
            let successor_value = self.nodes.get(&successor_id).map(|n| n.value.clone()).unwrap();
            self.delete_node(successor_id);
            if let Some(node) = self.nodes.get_mut(&node_id) {
                node.value = successor_value;
            }
        }
    }

    /// 在父节点中替换子节点引用
    fn replace_in_parent(&mut self, parent_id: i64, old_child: i64, new_child: i64) {
        if parent_id == -1 {
            self.root = new_child;
        } else if let Some(parent) = self.nodes.get_mut(&parent_id) {
            if parent.left == old_child {
                parent.left = new_child;
            } else {
                parent.right = new_child;
            }
        }
    }
}

impl<'a> BaseStructure for BinarySearchTree<'a> {
    fn build_data(&self) -> HashMap<String, Value> {
        let mut nodes_list: Vec<Value> = self
            .nodes
            .values()
            .map(|n| {
                let mut table = HashMap::new();
                table.insert("id".to_string(), Value::Integer(n.id));
                table.insert("value".to_string(), n.value.clone().into());
                table.insert("left".to_string(), Value::Integer(n.left));
                table.insert("right".to_string(), Value::Integer(n.right));
                Value::Table(table)
            })
            .collect();

        nodes_list.sort_by(|a, b| {
            if let (Value::Table(ta), Value::Table(tb)) = (a, b) {
                if let (Some(Value::Integer(ia)), Some(Value::Integer(ib))) =
                    (ta.get("id"), tb.get("id"))
                {
                    return ia.cmp(ib);
                }
            }
            std::cmp::Ordering::Equal
        });

        let mut data = HashMap::new();
        data.insert("root".to_string(), Value::Integer(self.root));
        data.insert("nodes".to_string(), Value::Array(nodes_list));
        data
    }

    fn session(&self) -> &Session {
        self.session
    }

    fn session_mut(&mut self) -> &mut Session {
        self.session
    }
}

/// 堆类型
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum HeapType {
    /// 最小堆
    Min,
    /// 最大堆
    Max,
}

/// 堆数据结构（使用完全二叉树表示）
pub struct Heap<'a> {
    /// 会话引用
    session: &'a mut Session,
    /// 堆类型
    heap_type: HeapType,
    /// 堆元素
    items: Vec<NumericValue>,
}

impl<'a> Heap<'a> {
    /// 创建新的堆实例
    #[must_use]
    pub fn new(session: &'a mut Session, heap_type: HeapType) -> Self {
        let mut heap = Self {
            session,
            heap_type,
            items: Vec::new(),
        };
        heap.initialize();
        heap
    }

    /// 插入元素，自动维护堆性质
    pub fn insert(&mut self, value: impl Into<NumericValue>) -> Result<()> {
        let value = value.into();
        self.items.push(value.clone());
        self.sift_up(self.items.len() - 1);

        let mut args = HashMap::new();
        args.insert("value".to_string(), value.into());
        self.record_step("insert", args, None, None)?;

        Ok(())
    }

    /// 提取堆顶元素，自动维护堆性质
    ///
    /// # 错误
    ///
    /// 当堆为空时返回 `StructureError::EmptyStructure`
    pub fn extract(&mut self) -> Result<()> {
        if self.items.is_empty() {
            self.session.set_failed_step_id(self.session.step_counter());
            return Err(StructureError::EmptyStructure.into());
        }

        let extracted = self.items[0].clone();

        if self.items.len() == 1 {
            self.items.pop();
        } else {
            let last = self.items.pop().unwrap();
            self.items[0] = last;
            self.sift_down(0);
        }

        let args = HashMap::new();
        self.record_step("extract", args, Some(extracted.into()), None)?;

        Ok(())
    }

    /// 清空堆
    pub fn clear(&mut self) -> Result<()> {
        self.items.clear();

        let args = HashMap::new();
        self.record_step("clear", args, None, None)?;

        Ok(())
    }

    /// 比较两个值
    fn compare(&self, a: &NumericValue, b: &NumericValue) -> bool {
        match self.heap_type {
            HeapType::Min => a.cmp_value(b) == std::cmp::Ordering::Less,
            HeapType::Max => a.cmp_value(b) == std::cmp::Ordering::Greater,
        }
    }

    /// 向上调整堆
    fn sift_up(&mut self, mut index: usize) {
        while index > 0 {
            let parent = (index - 1) / 2;
            if self.compare(&self.items[index], &self.items[parent]) {
                self.items.swap(index, parent);
                index = parent;
            } else {
                break;
            }
        }
    }

    /// 向下调整堆
    fn sift_down(&mut self, mut index: usize) {
        let size = self.items.len();
        loop {
            let mut target = index;
            let left = 2 * index + 1;
            let right = 2 * index + 2;

            if left < size && self.compare(&self.items[left], &self.items[target]) {
                target = left;
            }
            if right < size && self.compare(&self.items[right], &self.items[target]) {
                target = right;
            }

            if target == index {
                break;
            }

            self.items.swap(index, target);
            index = target;
        }
    }
}

impl<'a> BaseStructure for Heap<'a> {
    fn build_data(&self) -> HashMap<String, Value> {
        if self.items.is_empty() {
            let mut data = HashMap::new();
            data.insert("root".to_string(), Value::Integer(-1));
            data.insert("nodes".to_string(), Value::Array(Vec::new()));
            return data;
        }

        let nodes_list: Vec<Value> = self
            .items
            .iter()
            .enumerate()
            .map(|(i, v)| {
                let mut table = HashMap::new();
                table.insert("id".to_string(), Value::Integer(i as i64));
                table.insert("value".to_string(), v.clone().into());

                let left = 2 * i + 1;
                let right = 2 * i + 2;
                table.insert(
                    "left".to_string(),
                    Value::Integer(if left < self.items.len() {
                        left as i64
                    } else {
                        -1
                    }),
                );
                table.insert(
                    "right".to_string(),
                    Value::Integer(if right < self.items.len() {
                        right as i64
                    } else {
                        -1
                    }),
                );

                Value::Table(table)
            })
            .collect();

        let mut data = HashMap::new();
        data.insert("root".to_string(), Value::Integer(0));
        data.insert("nodes".to_string(), Value::Array(nodes_list));
        data
    }

    fn session(&self) -> &Session {
        self.session
    }

    fn session_mut(&mut self) -> &mut Session {
        self.session
    }
}