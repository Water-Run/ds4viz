//! 线性数据结构实现
//!
//! 包含栈、队列、单链表、双向链表的实现
//!
//! # 作者
//!
//! WaterRun
//!
//! # 文件路径
//!
//! src/structures/linear.rs
//!
//! # 日期
//!
//! 2025-12-25

use crate::error::{Result, StructureError};
use crate::session::Session;
use crate::structures::base::BaseStructure;
use crate::trace::Value;
use std::collections::HashMap;

/// 栈数据结构
pub struct Stack<'a> {
    /// 会话引用
    session: &'a mut Session,
    /// 栈元素
    items: Vec<Value>,
}

impl<'a> Stack<'a> {
    /// 创建新的栈实例
    ///
    /// # 参数
    ///
    /// * `session` - 会话引用
    #[must_use]
    pub fn new(session: &'a mut Session) -> Self {
        let mut stack = Self {
            session,
            items: Vec::new(),
        };
        stack.initialize();
        stack
    }

    /// 压栈操作
    ///
    /// # 参数
    ///
    /// * `value` - 要压入的值
    ///
    /// # 错误
    ///
    /// 此操作不会失败
    pub fn push(&mut self, value: impl Into<Value>) -> Result<()> {
        let value = value.into();
        let mut args = HashMap::new();
        args.insert("value".to_string(), value.clone());

        self.items.push(value);
        self.record_step("push", args, None, None)?;
        Ok(())
    }

    /// 弹栈操作
    ///
    /// # 错误
    ///
    /// 当栈为空时返回 `StructureError::EmptyStructure`
    pub fn pop(&mut self) -> Result<()> {
        if self.items.is_empty() {
            self.session.set_failed_step_id(self.session.step_counter());
            return Err(StructureError::EmptyStructure.into());
        }

        let popped = self.items.pop();
        let args = HashMap::new();
        self.record_step("pop", args, popped, None)?;
        Ok(())
    }

    /// 清空栈
    pub fn clear(&mut self) -> Result<()> {
        self.items.clear();
        let args = HashMap::new();
        self.record_step("clear", args, None, None)?;
        Ok(())
    }
}

impl<'a> BaseStructure for Stack<'a> {
    fn build_data(&self) -> HashMap<String, Value> {
        let mut data = HashMap::new();
        data.insert(
            "items".to_string(),
            Value::Array(self.items.clone()),
        );
        data.insert(
            "top".to_string(),
            Value::Integer(if self.items.is_empty() {
                -1
            } else {
                (self.items.len() - 1) as i64
            }),
        );
        data
    }

    fn session(&self) -> &Session {
        self.session
    }

    fn session_mut(&mut self) -> &mut Session {
        self.session
    }
}

/// 队列数据结构
pub struct Queue<'a> {
    /// 会话引用
    session: &'a mut Session,
    /// 队列元素
    items: Vec<Value>,
}

impl<'a> Queue<'a> {
    /// 创建新的队列实例
    #[must_use]
    pub fn new(session: &'a mut Session) -> Self {
        let mut queue = Self {
            session,
            items: Vec::new(),
        };
        queue.initialize();
        queue
    }

    /// 入队操作
    ///
    /// # 参数
    ///
    /// * `value` - 要入队的值
    pub fn enqueue(&mut self, value: impl Into<Value>) -> Result<()> {
        let value = value.into();
        let mut args = HashMap::new();
        args.insert("value".to_string(), value.clone());

        self.items.push(value);
        self.record_step("enqueue", args, None, None)?;
        Ok(())
    }

    /// 出队操作
    ///
    /// # 错误
    ///
    /// 当队列为空时返回 `StructureError::EmptyStructure`
    pub fn dequeue(&mut self) -> Result<()> {
        if self.items.is_empty() {
            self.session.set_failed_step_id(self.session.step_counter());
            return Err(StructureError::EmptyStructure.into());
        }

        let dequeued = self.items.remove(0);
        let args = HashMap::new();
        self.record_step("dequeue", args, Some(dequeued), None)?;
        Ok(())
    }

    /// 清空队列
    pub fn clear(&mut self) -> Result<()> {
        self.items.clear();
        let args = HashMap::new();
        self.record_step("clear", args, None, None)?;
        Ok(())
    }
}

impl<'a> BaseStructure for Queue<'a> {
    fn build_data(&self) -> HashMap<String, Value> {
        let mut data = HashMap::new();
        data.insert("items".to_string(), Value::Array(self.items.clone()));

        if self.items.is_empty() {
            data.insert("front".to_string(), Value::Integer(-1));
            data.insert("rear".to_string(), Value::Integer(-1));
        } else {
            data.insert("front".to_string(), Value::Integer(0));
            data.insert(
                "rear".to_string(),
                Value::Integer((self.items.len() - 1) as i64),
            );
        }
        data
    }

    fn session(&self) -> &Session {
        self.session
    }

    fn session_mut(&mut self) -> &mut Session {
        self.session
    }
}

/// 单链表节点
#[derive(Debug, Clone)]
struct SListNode {
    /// 节点 ID
    id: i64,
    /// 节点值
    value: Value,
    /// 下一个节点 ID
    next: i64,
}

/// 单链表数据结构
pub struct SingleLinkedList<'a> {
    /// 会话引用
    session: &'a mut Session,
    /// 节点映射
    nodes: HashMap<i64, SListNode>,
    /// 头节点 ID
    head: i64,
    /// 下一个节点 ID
    next_id: i64,
}

impl<'a> SingleLinkedList<'a> {
    /// 创建新的单链表实例
    #[must_use]
    pub fn new(session: &'a mut Session) -> Self {
        let mut list = Self {
            session,
            nodes: HashMap::new(),
            head: -1,
            next_id: 0,
        };
        list.initialize();
        list
    }

    /// 在链表头部插入节点
    ///
    /// # 返回
    ///
    /// 返回新插入节点的 id
    pub fn insert_head(&mut self, value: impl Into<Value>) -> Result<i64> {
        let value = value.into();
        let new_id = self.next_id;
        self.next_id += 1;

        let node = SListNode {
            id: new_id,
            value: value.clone(),
            next: self.head,
        };
        self.nodes.insert(new_id, node);
        self.head = new_id;

        let mut args = HashMap::new();
        args.insert("value".to_string(), value);
        self.record_step("insert_head", args, Some(Value::Integer(new_id)), None)?;

        Ok(new_id)
    }

    /// 在链表尾部插入节点
    ///
    /// # 返回
    ///
    /// 返回新插入节点的 id
    pub fn insert_tail(&mut self, value: impl Into<Value>) -> Result<i64> {
        let value = value.into();
        let new_id = self.next_id;
        self.next_id += 1;

        let node = SListNode {
            id: new_id,
            value: value.clone(),
            next: -1,
        };
        self.nodes.insert(new_id, node);

        if self.head == -1 {
            self.head = new_id;
        } else {
            let tail = self.find_tail();
            if let Some(tail_node) = self.nodes.get_mut(&tail) {
                tail_node.next = new_id;
            }
        }

        let mut args = HashMap::new();
        args.insert("value".to_string(), value);
        self.record_step("insert_tail", args, Some(Value::Integer(new_id)), None)?;

        Ok(new_id)
    }

    /// 在指定节点后插入新节点
    ///
    /// # 错误
    ///
    /// 当指定节点不存在时返回 `StructureError::NodeNotFound`
    pub fn insert_after(&mut self, node_id: i64, value: impl Into<Value>) -> Result<i64> {
        if !self.nodes.contains_key(&node_id) {
            self.session.set_failed_step_id(self.session.step_counter());
            return Err(StructureError::NodeNotFound(node_id).into());
        }

        let value = value.into();
        let new_id = self.next_id;
        self.next_id += 1;

        let old_next = self.nodes.get(&node_id).map_or(-1, |n| n.next);
        let node = SListNode {
            id: new_id,
            value: value.clone(),
            next: old_next,
        };
        self.nodes.insert(new_id, node);

        if let Some(target) = self.nodes.get_mut(&node_id) {
            target.next = new_id;
        }

        let mut args = HashMap::new();
        args.insert("node_id".to_string(), Value::Integer(node_id));
        args.insert("value".to_string(), value);
        self.record_step("insert_after", args, Some(Value::Integer(new_id)), None)?;

        Ok(new_id)
    }

    /// 删除指定节点
    ///
    /// # 错误
    ///
    /// 当指定节点不存在时返回 `StructureError::NodeNotFound`
    pub fn delete(&mut self, node_id: i64) -> Result<()> {
        if !self.nodes.contains_key(&node_id) {
            self.session.set_failed_step_id(self.session.step_counter());
            return Err(StructureError::NodeNotFound(node_id).into());
        }

        let deleted_value = self.nodes.get(&node_id).map(|n| n.value.clone());

        if self.head == node_id {
            self.head = self.nodes.get(&node_id).map_or(-1, |n| n.next);
        } else {
            let prev = self.find_prev(node_id);
            if prev != -1 {
                let next = self.nodes.get(&node_id).map_or(-1, |n| n.next);
                if let Some(prev_node) = self.nodes.get_mut(&prev) {
                    prev_node.next = next;
                }
            }
        }

        self.nodes.remove(&node_id);

        let mut args = HashMap::new();
        args.insert("node_id".to_string(), Value::Integer(node_id));
        self.record_step("delete", args, deleted_value, None)?;

        Ok(())
    }

    /// 删除头节点
    ///
    /// # 错误
    ///
    /// 当链表为空时返回 `StructureError::EmptyStructure`
    pub fn delete_head(&mut self) -> Result<()> {
        if self.head == -1 {
            self.session.set_failed_step_id(self.session.step_counter());
            return Err(StructureError::EmptyStructure.into());
        }

        let old_head = self.head;
        let deleted_value = self.nodes.get(&old_head).map(|n| n.value.clone());
        self.head = self.nodes.get(&old_head).map_or(-1, |n| n.next);
        self.nodes.remove(&old_head);

        let args = HashMap::new();
        self.record_step("delete_head", args, deleted_value, None)?;

        Ok(())
    }

    /// 反转链表
    pub fn reverse(&mut self) -> Result<()> {
        let mut prev: i64 = -1;
        let mut current = self.head;

        while current != -1 {
            let next = self.nodes.get(&current).map_or(-1, |n| n.next);
            if let Some(node) = self.nodes.get_mut(&current) {
                node.next = prev;
            }
            prev = current;
            current = next;
        }
        self.head = prev;

        let args = HashMap::new();
        self.record_step("reverse", args, None, None)?;

        Ok(())
    }

    /// 查找尾节点 ID
    fn find_tail(&self) -> i64 {
        if self.head == -1 {
            return -1;
        }

        let mut current = self.head;
        while let Some(node) = self.nodes.get(&current) {
            if node.next == -1 {
                return current;
            }
            current = node.next;
        }
        current
    }

    /// 查找指定节点的前驱节点 ID
    fn find_prev(&self, node_id: i64) -> i64 {
        if self.head == -1 || self.head == node_id {
            return -1;
        }

        let mut current = self.head;
        while let Some(node) = self.nodes.get(&current) {
            if node.next == node_id {
                return current;
            }
            current = node.next;
            if current == -1 {
                break;
            }
        }
        -1
    }
}

impl<'a> BaseStructure for SingleLinkedList<'a> {
    fn build_data(&self) -> HashMap<String, Value> {
        let mut nodes_list: Vec<Value> = self
            .nodes
            .values()
            .map(|n| {
                let mut table = HashMap::new();
                table.insert("id".to_string(), Value::Integer(n.id));
                table.insert("value".to_string(), n.value.clone());
                table.insert("next".to_string(), Value::Integer(n.next));
                Value::Table(table)
            })
            .collect();

        // 按 id 排序
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
        data.insert("head".to_string(), Value::Integer(self.head));
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

/// 双向链表节点
#[derive(Debug, Clone)]
struct DListNode {
    /// 节点 ID
    id: i64,
    /// 节点值
    value: Value,
    /// 前一个节点 ID
    prev: i64,
    /// 下一个节点 ID
    next: i64,
}

/// 双向链表数据结构
pub struct DoubleLinkedList<'a> {
    /// 会话引用
    session: &'a mut Session,
    /// 节点映射
    nodes: HashMap<i64, DListNode>,
    /// 头节点 ID
    head: i64,
    /// 尾节点 ID
    tail: i64,
    /// 下一个节点 ID
    next_id: i64,
}

impl<'a> DoubleLinkedList<'a> {
    /// 创建新的双向链表实例
    #[must_use]
    pub fn new(session: &'a mut Session) -> Self {
        let mut list = Self {
            session,
            nodes: HashMap::new(),
            head: -1,
            tail: -1,
            next_id: 0,
        };
        list.initialize();
        list
    }

    /// 在链表头部插入节点
    pub fn insert_head(&mut self, value: impl Into<Value>) -> Result<i64> {
        let value = value.into();
        let new_id = self.next_id;
        self.next_id += 1;

        let node = DListNode {
            id: new_id,
            value: value.clone(),
            prev: -1,
            next: self.head,
        };

        if self.head != -1 {
            if let Some(head_node) = self.nodes.get_mut(&self.head) {
                head_node.prev = new_id;
            }
        }

        self.nodes.insert(new_id, node);
        self.head = new_id;

        if self.tail == -1 {
            self.tail = new_id;
        }

        let mut args = HashMap::new();
        args.insert("value".to_string(), value);
        self.record_step("insert_head", args, Some(Value::Integer(new_id)), None)?;

        Ok(new_id)
    }

    /// 在链表尾部插入节点
    pub fn insert_tail(&mut self, value: impl Into<Value>) -> Result<i64> {
        let value = value.into();
        let new_id = self.next_id;
        self.next_id += 1;

        let node = DListNode {
            id: new_id,
            value: value.clone(),
            prev: self.tail,
            next: -1,
        };

        if self.tail != -1 {
            if let Some(tail_node) = self.nodes.get_mut(&self.tail) {
                tail_node.next = new_id;
            }
        }

        self.nodes.insert(new_id, node);
        self.tail = new_id;

        if self.head == -1 {
            self.head = new_id;
        }

        let mut args = HashMap::new();
        args.insert("value".to_string(), value);
        self.record_step("insert_tail", args, Some(Value::Integer(new_id)), None)?;

        Ok(new_id)
    }

    /// 在指定节点前插入新节点
    pub fn insert_before(&mut self, node_id: i64, value: impl Into<Value>) -> Result<i64> {
        if !self.nodes.contains_key(&node_id) {
            self.session.set_failed_step_id(self.session.step_counter());
            return Err(StructureError::NodeNotFound(node_id).into());
        }

        let value = value.into();
        let new_id = self.next_id;
        self.next_id += 1;

        let old_prev = self.nodes.get(&node_id).map_or(-1, |n| n.prev);

        let node = DListNode {
            id: new_id,
            value: value.clone(),
            prev: old_prev,
            next: node_id,
        };

        self.nodes.insert(new_id, node);

        if let Some(target) = self.nodes.get_mut(&node_id) {
            target.prev = new_id;
        }

        if old_prev != -1 {
            if let Some(prev_node) = self.nodes.get_mut(&old_prev) {
                prev_node.next = new_id;
            }
        } else {
            self.head = new_id;
        }

        let mut args = HashMap::new();
        args.insert("node_id".to_string(), Value::Integer(node_id));
        args.insert("value".to_string(), value);
        self.record_step("insert_before", args, Some(Value::Integer(new_id)), None)?;

        Ok(new_id)
    }

    /// 在指定节点后插入新节点
    pub fn insert_after(&mut self, node_id: i64, value: impl Into<Value>) -> Result<i64> {
        if !self.nodes.contains_key(&node_id) {
            self.session.set_failed_step_id(self.session.step_counter());
            return Err(StructureError::NodeNotFound(node_id).into());
        }

        let value = value.into();
        let new_id = self.next_id;
        self.next_id += 1;

        let old_next = self.nodes.get(&node_id).map_or(-1, |n| n.next);

        let node = DListNode {
            id: new_id,
            value: value.clone(),
            prev: node_id,
            next: old_next,
        };

        self.nodes.insert(new_id, node);

        if let Some(target) = self.nodes.get_mut(&node_id) {
            target.next = new_id;
        }

        if old_next != -1 {
            if let Some(next_node) = self.nodes.get_mut(&old_next) {
                next_node.prev = new_id;
            }
        } else {
            self.tail = new_id;
        }

        let mut args = HashMap::new();
        args.insert("node_id".to_string(), Value::Integer(node_id));
        args.insert("value".to_string(), value);
        self.record_step("insert_after", args, Some(Value::Integer(new_id)), None)?;

        Ok(new_id)
    }

    /// 删除指定节点
    pub fn delete(&mut self, node_id: i64) -> Result<()> {
        if !self.nodes.contains_key(&node_id) {
            self.session.set_failed_step_id(self.session.step_counter());
            return Err(StructureError::NodeNotFound(node_id).into());
        }

        let (prev_id, next_id, deleted_value) = {
            let node = self.nodes.get(&node_id).unwrap();
            (node.prev, node.next, node.value.clone())
        };

        if prev_id != -1 {
            if let Some(prev_node) = self.nodes.get_mut(&prev_id) {
                prev_node.next = next_id;
            }
        } else {
            self.head = next_id;
        }

        if next_id != -1 {
            if let Some(next_node) = self.nodes.get_mut(&next_id) {
                next_node.prev = prev_id;
            }
        } else {
            self.tail = prev_id;
        }

        self.nodes.remove(&node_id);

        let mut args = HashMap::new();
        args.insert("node_id".to_string(), Value::Integer(node_id));
        self.record_step("delete", args, Some(deleted_value), None)?;

        Ok(())
    }

    /// 删除头节点
    pub fn delete_head(&mut self) -> Result<()> {
        if self.head == -1 {
            self.session.set_failed_step_id(self.session.step_counter());
            return Err(StructureError::EmptyStructure.into());
        }

        self.delete(self.head)
    }

    /// 删除尾节点
    pub fn delete_tail(&mut self) -> Result<()> {
        if self.tail == -1 {
            self.session.set_failed_step_id(self.session.step_counter());
            return Err(StructureError::EmptyStructure.into());
        }

        self.delete(self.tail)
    }

    /// 反转链表
    pub fn reverse(&mut self) -> Result<()> {
        let mut current = self.head;
        while current != -1 {
            if let Some(node) = self.nodes.get_mut(&current) {
                std::mem::swap(&mut node.prev, &mut node.next);
                current = node.prev; // 原来的 next
            } else {
                break;
            }
        }
        std::mem::swap(&mut self.head, &mut self.tail);

        let args = HashMap::new();
        self.record_step("reverse", args, None, None)?;

        Ok(())
    }
}

impl<'a> BaseStructure for DoubleLinkedList<'a> {
    fn build_data(&self) -> HashMap<String, Value> {
        let mut nodes_list: Vec<Value> = self
            .nodes
            .values()
            .map(|n| {
                let mut table = HashMap::new();
                table.insert("id".to_string(), Value::Integer(n.id));
                table.insert("value".to_string(), n.value.clone());
                table.insert("prev".to_string(), Value::Integer(n.prev));
                table.insert("next".to_string(), Value::Integer(n.next));
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
        data.insert("head".to_string(), Value::Integer(self.head));
        data.insert("tail".to_string(), Value::Integer(self.tail));
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