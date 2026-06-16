//! 图数据结构实现
//!
//! 包含无向图、有向图、带权图的实现
//!
//! # 作者
//!
//! WaterRun
//!
//! # 文件路径
//!
//! src/structures/graph.rs
//!
//! # 日期
//!
//! 2025-12-25

use crate::error::{Result, StructureError};
use crate::session::Session;
use crate::structures::base::BaseStructure;
use crate::trace::Value;
use std::collections::{HashMap, HashSet};

/// 无向图数据结构
pub struct GraphUndirected<'a> {
    /// 会话引用
    session: &'a mut Session,
    /// 节点映射（id -> label）
    nodes: HashMap<i64, String>,
    /// 边集合（规范化为 from < to）
    edges: HashSet<(i64, i64)>,
}

impl<'a> GraphUndirected<'a> {
    /// 创建新的无向图实例
    #[must_use]
    pub fn new(session: &'a mut Session) -> Self {
        let mut graph = Self {
            session,
            nodes: HashMap::new(),
            edges: HashSet::new(),
        };
        graph.initialize();
        graph
    }

    /// 规范化无向边，确保 from < to
    const fn normalize_edge(from_id: i64, to_id: i64) -> (i64, i64) {
        if from_id < to_id {
            (from_id, to_id)
        } else {
            (to_id, from_id)
        }
    }

    /// 添加节点
    ///
    /// # 错误
    ///
    /// * 当节点已存在时返回 `StructureError::NodeExists`
    /// * 当标签长度无效时返回 `StructureError::InvalidLabel`
    pub fn add_node(&mut self, node_id: i64, label: &str) -> Result<()> {
        if self.nodes.contains_key(&node_id) {
            self.session.set_failed_step_id(self.session.step_counter());
            return Err(StructureError::NodeExists(node_id).into());
        }

        if label.is_empty() || label.len() > 32 {
            self.session.set_failed_step_id(self.session.step_counter());
            return Err(StructureError::InvalidLabel(label.len()).into());
        }

        self.nodes.insert(node_id, label.to_string());

        let mut args = HashMap::new();
        args.insert("id".to_string(), Value::Integer(node_id));
        args.insert("label".to_string(), Value::String(label.to_string()));
        self.record_step("add_node", args, None, None)?;

        Ok(())
    }

    /// 添加无向边
    ///
    /// # 错误
    ///
    /// * 当节点不存在时返回 `StructureError::NodeNotFound`
    /// * 当边已存在时返回 `StructureError::EdgeExists`
    /// * 当形成自环时返回 `StructureError::SelfLoop`
    pub fn add_edge(&mut self, from_id: i64, to_id: i64) -> Result<()> {
        if !self.nodes.contains_key(&from_id) {
            self.session.set_failed_step_id(self.session.step_counter());
            return Err(StructureError::NodeNotFound(from_id).into());
        }

        if !self.nodes.contains_key(&to_id) {
            self.session.set_failed_step_id(self.session.step_counter());
            return Err(StructureError::NodeNotFound(to_id).into());
        }

        if from_id == to_id {
            self.session.set_failed_step_id(self.session.step_counter());
            return Err(StructureError::SelfLoop(from_id).into());
        }

        let edge = Self::normalize_edge(from_id, to_id);
        if self.edges.contains(&edge) {
            self.session.set_failed_step_id(self.session.step_counter());
            return Err(StructureError::EdgeExists(from_id, to_id).into());
        }

        self.edges.insert(edge);

        let mut args = HashMap::new();
        args.insert("from".to_string(), Value::Integer(from_id));
        args.insert("to".to_string(), Value::Integer(to_id));
        self.record_step("add_edge", args, None, None)?;

        Ok(())
    }

    /// 删除节点及其相关的所有边
    ///
    /// # 错误
    ///
    /// 当节点不存在时返回 `StructureError::NodeNotFound`
    pub fn remove_node(&mut self, node_id: i64) -> Result<()> {
        if !self.nodes.contains_key(&node_id) {
            self.session.set_failed_step_id(self.session.step_counter());
            return Err(StructureError::NodeNotFound(node_id).into());
        }

        // 删除相关的边
        self.edges.retain(|(f, t)| *f != node_id && *t != node_id);
        self.nodes.remove(&node_id);

        let mut args = HashMap::new();
        args.insert("node_id".to_string(), Value::Integer(node_id));
        self.record_step("remove_node", args, None, None)?;

        Ok(())
    }

    /// 删除边
    ///
    /// # 错误
    ///
    /// 当边不存在时返回 `StructureError::EdgeNotFound`
    pub fn remove_edge(&mut self, from_id: i64, to_id: i64) -> Result<()> {
        let edge = Self::normalize_edge(from_id, to_id);
        if !self.edges.contains(&edge) {
            self.session.set_failed_step_id(self.session.step_counter());
            return Err(StructureError::EdgeNotFound(from_id, to_id).into());
        }

        self.edges.remove(&edge);

        let mut args = HashMap::new();
        args.insert("from".to_string(), Value::Integer(from_id));
        args.insert("to".to_string(), Value::Integer(to_id));
        self.record_step("remove_edge", args, None, None)?;

        Ok(())
    }

    /// 更新节点标签
    ///
    /// # 错误
    ///
    /// * 当节点不存在时返回 `StructureError::NodeNotFound`
    /// * 当标签长度无效时返回 `StructureError::InvalidLabel`
    pub fn update_node_label(&mut self, node_id: i64, label: &str) -> Result<()> {
        if !self.nodes.contains_key(&node_id) {
            self.session.set_failed_step_id(self.session.step_counter());
            return Err(StructureError::NodeNotFound(node_id).into());
        }

        if label.is_empty() || label.len() > 32 {
            self.session.set_failed_step_id(self.session.step_counter());
            return Err(StructureError::InvalidLabel(label.len()).into());
        }

        let old_label = self.nodes.get(&node_id).cloned();
        self.nodes.insert(node_id, label.to_string());

        let mut args = HashMap::new();
        args.insert("node_id".to_string(), Value::Integer(node_id));
        args.insert("label".to_string(), Value::String(label.to_string()));
        self.record_step(
            "update_node_label",
            args,
            old_label.map(Value::String),
            None,
        )?;

        Ok(())
    }
}

impl<'a> BaseStructure for GraphUndirected<'a> {
    fn build_data(&self) -> HashMap<String, Value> {
        let mut nodes_list: Vec<Value> = self
            .nodes
            .iter()
            .map(|(id, label)| {
                let mut table = HashMap::new();
                table.insert("id".to_string(), Value::Integer(*id));
                table.insert("label".to_string(), Value::String(label.clone()));
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

        let mut edges_list: Vec<Value> = self
            .edges
            .iter()
            .map(|(from, to)| {
                let mut table = HashMap::new();
                table.insert("from".to_string(), Value::Integer(*from));
                table.insert("to".to_string(), Value::Integer(*to));
                Value::Table(table)
            })
            .collect();

        edges_list.sort_by(|a, b| {
            if let (Value::Table(ta), Value::Table(tb)) = (a, b) {
                if let (Some(Value::Integer(fa)), Some(Value::Integer(fb))) =
                    (ta.get("from"), tb.get("from"))
                {
                    let cmp = fa.cmp(fb);
                    if cmp != std::cmp::Ordering::Equal {
                        return cmp;
                    }
                }
                if let (Some(Value::Integer(ta_val)), Some(Value::Integer(tb_val))) =
                    (ta.get("to"), tb.get("to"))
                {
                    return ta_val.cmp(tb_val);
                }
            }
            std::cmp::Ordering::Equal
        });

        let mut data = HashMap::new();
        data.insert("nodes".to_string(), Value::Array(nodes_list));
        data.insert("edges".to_string(), Value::Array(edges_list));
        data
    }

    fn session(&self) -> &Session {
        self.session
    }

    fn session_mut(&mut self) -> &mut Session {
        self.session
    }
}

/// 有向图数据结构
pub struct GraphDirected<'a> {
    /// 会话引用
    session: &'a mut Session,
    /// 节点映射
    nodes: HashMap<i64, String>,
    /// 边集合
    edges: HashSet<(i64, i64)>,
}

impl<'a> GraphDirected<'a> {
    /// 创建新的有向图实例
    #[must_use]
    pub fn new(session: &'a mut Session) -> Self {
        let mut graph = Self {
            session,
            nodes: HashMap::new(),
            edges: HashSet::new(),
        };
        graph.initialize();
        graph
    }

    /// 添加节点
    pub fn add_node(&mut self, node_id: i64, label: &str) -> Result<()> {
        if self.nodes.contains_key(&node_id) {
            self.session.set_failed_step_id(self.session.step_counter());
            return Err(StructureError::NodeExists(node_id).into());
        }

        if label.is_empty() || label.len() > 32 {
            self.session.set_failed_step_id(self.session.step_counter());
            return Err(StructureError::InvalidLabel(label.len()).into());
        }

        self.nodes.insert(node_id, label.to_string());

        let mut args = HashMap::new();
        args.insert("id".to_string(), Value::Integer(node_id));
        args.insert("label".to_string(), Value::String(label.to_string()));
        self.record_step("add_node", args, None, None)?;

        Ok(())
    }

    /// 添加有向边
    pub fn add_edge(&mut self, from_id: i64, to_id: i64) -> Result<()> {
        if !self.nodes.contains_key(&from_id) {
            self.session.set_failed_step_id(self.session.step_counter());
            return Err(StructureError::NodeNotFound(from_id).into());
        }

        if !self.nodes.contains_key(&to_id) {
            self.session.set_failed_step_id(self.session.step_counter());
            return Err(StructureError::NodeNotFound(to_id).into());
        }

        if from_id == to_id {
            self.session.set_failed_step_id(self.session.step_counter());
            return Err(StructureError::SelfLoop(from_id).into());
        }

        let edge = (from_id, to_id);
        if self.edges.contains(&edge) {
            self.session.set_failed_step_id(self.session.step_counter());
            return Err(StructureError::EdgeExists(from_id, to_id).into());
        }

        self.edges.insert(edge);

        let mut args = HashMap::new();
        args.insert("from".to_string(), Value::Integer(from_id));
        args.insert("to".to_string(), Value::Integer(to_id));
        self.record_step("add_edge", args, None, None)?;

        Ok(())
    }

    /// 删除节点及其相关的所有边
    pub fn remove_node(&mut self, node_id: i64) -> Result<()> {
        if !self.nodes.contains_key(&node_id) {
            self.session.set_failed_step_id(self.session.step_counter());
            return Err(StructureError::NodeNotFound(node_id).into());
        }

        self.edges.retain(|(f, t)| *f != node_id && *t != node_id);
        self.nodes.remove(&node_id);

        let mut args = HashMap::new();
        args.insert("node_id".to_string(), Value::Integer(node_id));
        self.record_step("remove_node", args, None, None)?;

        Ok(())
    }

    /// 删除边
    pub fn remove_edge(&mut self, from_id: i64, to_id: i64) -> Result<()> {
        let edge = (from_id, to_id);
        if !self.edges.contains(&edge) {
            self.session.set_failed_step_id(self.session.step_counter());
            return Err(StructureError::EdgeNotFound(from_id, to_id).into());
        }

        self.edges.remove(&edge);

        let mut args = HashMap::new();
        args.insert("from".to_string(), Value::Integer(from_id));
        args.insert("to".to_string(), Value::Integer(to_id));
        self.record_step("remove_edge", args, None, None)?;

        Ok(())
    }

    /// 更新节点标签
    pub fn update_node_label(&mut self, node_id: i64, label: &str) -> Result<()> {
        if !self.nodes.contains_key(&node_id) {
            self.session.set_failed_step_id(self.session.step_counter());
            return Err(StructureError::NodeNotFound(node_id).into());
        }

        if label.is_empty() || label.len() > 32 {
            self.session.set_failed_step_id(self.session.step_counter());
            return Err(StructureError::InvalidLabel(label.len()).into());
        }

        let old_label = self.nodes.get(&node_id).cloned();
        self.nodes.insert(node_id, label.to_string());

        let mut args = HashMap::new();
        args.insert("node_id".to_string(), Value::Integer(node_id));
        args.insert("label".to_string(), Value::String(label.to_string()));
        self.record_step(
            "update_node_label",
            args,
            old_label.map(Value::String),
            None,
        )?;

        Ok(())
    }
}

impl<'a> BaseStructure for GraphDirected<'a> {
    fn build_data(&self) -> HashMap<String, Value> {
        let mut nodes_list: Vec<Value> = self
            .nodes
            .iter()
            .map(|(id, label)| {
                let mut table = HashMap::new();
                table.insert("id".to_string(), Value::Integer(*id));
                table.insert("label".to_string(), Value::String(label.clone()));
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

        let mut edges_list: Vec<Value> = self
            .edges
            .iter()
            .map(|(from, to)| {
                let mut table = HashMap::new();
                table.insert("from".to_string(), Value::Integer(*from));
                table.insert("to".to_string(), Value::Integer(*to));
                Value::Table(table)
            })
            .collect();

        edges_list.sort_by(|a, b| {
            if let (Value::Table(ta), Value::Table(tb)) = (a, b) {
                if let (Some(Value::Integer(fa)), Some(Value::Integer(fb))) =
                    (ta.get("from"), tb.get("from"))
                {
                    let cmp = fa.cmp(fb);
                    if cmp != std::cmp::Ordering::Equal {
                        return cmp;
                    }
                }
                if let (Some(Value::Integer(ta_val)), Some(Value::Integer(tb_val))) =
                    (ta.get("to"), tb.get("to"))
                {
                    return ta_val.cmp(tb_val);
                }
            }
            std::cmp::Ordering::Equal
        });

        let mut data = HashMap::new();
        data.insert("nodes".to_string(), Value::Array(nodes_list));
        data.insert("edges".to_string(), Value::Array(edges_list));
        data
    }

    fn session(&self) -> &Session {
        self.session
    }

    fn session_mut(&mut self) -> &mut Session {
        self.session
    }
}

/// 带权图数据结构
pub struct GraphWeighted<'a> {
    /// 会话引用
    session: &'a mut Session,
    /// 是否为有向图
    directed: bool,
    /// 节点映射
    nodes: HashMap<i64, String>,
    /// 边映射（(from, to) -> weight）
    edges: HashMap<(i64, i64), f64>,
}

impl<'a> GraphWeighted<'a> {
    /// 创建新的带权图实例
    #[must_use]
    pub fn new(session: &'a mut Session, directed: bool) -> Self {
        let mut graph = Self {
            session,
            directed,
            nodes: HashMap::new(),
            edges: HashMap::new(),
        };
        graph.initialize();
        graph
    }

    /// 规范化边（无向图时确保 from < to）
    fn normalize_edge(&self, from_id: i64, to_id: i64) -> (i64, i64) {
        if self.directed {
            (from_id, to_id)
        } else if from_id < to_id {
            (from_id, to_id)
        } else {
            (to_id, from_id)
        }
    }

    /// 添加节点
    pub fn add_node(&mut self, node_id: i64, label: &str) -> Result<()> {
        if self.nodes.contains_key(&node_id) {
            self.session.set_failed_step_id(self.session.step_counter());
            return Err(StructureError::NodeExists(node_id).into());
        }

        if label.is_empty() || label.len() > 32 {
            self.session.set_failed_step_id(self.session.step_counter());
            return Err(StructureError::InvalidLabel(label.len()).into());
        }

        self.nodes.insert(node_id, label.to_string());

        let mut args = HashMap::new();
        args.insert("id".to_string(), Value::Integer(node_id));
        args.insert("label".to_string(), Value::String(label.to_string()));
        self.record_step("add_node", args, None, None)?;

        Ok(())
    }

    /// 添加带权边
    pub fn add_edge(&mut self, from_id: i64, to_id: i64, weight: f64) -> Result<()> {
        if !self.nodes.contains_key(&from_id) {
            self.session.set_failed_step_id(self.session.step_counter());
            return Err(StructureError::NodeNotFound(from_id).into());
        }

        if !self.nodes.contains_key(&to_id) {
            self.session.set_failed_step_id(self.session.step_counter());
            return Err(StructureError::NodeNotFound(to_id).into());
        }

        if from_id == to_id {
            self.session.set_failed_step_id(self.session.step_counter());
            return Err(StructureError::SelfLoop(from_id).into());
        }

        let edge = self.normalize_edge(from_id, to_id);
        if self.edges.contains_key(&edge) {
            self.session.set_failed_step_id(self.session.step_counter());
            return Err(StructureError::EdgeExists(from_id, to_id).into());
        }

        self.edges.insert(edge, weight);

        let mut args = HashMap::new();
        args.insert("from".to_string(), Value::Integer(from_id));
        args.insert("to".to_string(), Value::Integer(to_id));
        args.insert("weight".to_string(), Value::Float(weight));
        self.record_step("add_edge", args, None, None)?;

        Ok(())
    }

    /// 删除节点及其相关的所有边
    pub fn remove_node(&mut self, node_id: i64) -> Result<()> {
        if !self.nodes.contains_key(&node_id) {
            self.session.set_failed_step_id(self.session.step_counter());
            return Err(StructureError::NodeNotFound(node_id).into());
        }

        self.edges
            .retain(|(f, t), _| *f != node_id && *t != node_id);
        self.nodes.remove(&node_id);

        let mut args = HashMap::new();
        args.insert("node_id".to_string(), Value::Integer(node_id));
        self.record_step("remove_node", args, None, None)?;

        Ok(())
    }

    /// 删除边
    pub fn remove_edge(&mut self, from_id: i64, to_id: i64) -> Result<()> {
        let edge = self.normalize_edge(from_id, to_id);
        if !self.edges.contains_key(&edge) {
            self.session.set_failed_step_id(self.session.step_counter());
            return Err(StructureError::EdgeNotFound(from_id, to_id).into());
        }

        self.edges.remove(&edge);

        let mut args = HashMap::new();
        args.insert("from".to_string(), Value::Integer(from_id));
        args.insert("to".to_string(), Value::Integer(to_id));
        self.record_step("remove_edge", args, None, None)?;

        Ok(())
    }

    /// 更新边的权重
    pub fn update_weight(&mut self, from_id: i64, to_id: i64, weight: f64) -> Result<()> {
        let edge = self.normalize_edge(from_id, to_id);
        if !self.edges.contains_key(&edge) {
            self.session.set_failed_step_id(self.session.step_counter());
            return Err(StructureError::EdgeNotFound(from_id, to_id).into());
        }

        let old_weight = self.edges.get(&edge).copied();
        self.edges.insert(edge, weight);

        let mut args = HashMap::new();
        args.insert("from".to_string(), Value::Integer(from_id));
        args.insert("to".to_string(), Value::Integer(to_id));
        args.insert("weight".to_string(), Value::Float(weight));
        self.record_step("update_weight", args, old_weight.map(Value::Float), None)?;

        Ok(())
    }

    /// 更新节点标签
    pub fn update_node_label(&mut self, node_id: i64, label: &str) -> Result<()> {
        if !self.nodes.contains_key(&node_id) {
            self.session.set_failed_step_id(self.session.step_counter());
            return Err(StructureError::NodeNotFound(node_id).into());
        }

        if label.is_empty() || label.len() > 32 {
            self.session.set_failed_step_id(self.session.step_counter());
            return Err(StructureError::InvalidLabel(label.len()).into());
        }

        let old_label = self.nodes.get(&node_id).cloned();
        self.nodes.insert(node_id, label.to_string());

        let mut args = HashMap::new();
        args.insert("node_id".to_string(), Value::Integer(node_id));
        args.insert("label".to_string(), Value::String(label.to_string()));
        self.record_step(
            "update_node_label",
            args,
            old_label.map(Value::String),
            None,
        )?;

        Ok(())
    }
}

impl<'a> BaseStructure for GraphWeighted<'a> {
    fn build_data(&self) -> HashMap<String, Value> {
        let mut nodes_list: Vec<Value> = self
            .nodes
            .iter()
            .map(|(id, label)| {
                let mut table = HashMap::new();
                table.insert("id".to_string(), Value::Integer(*id));
                table.insert("label".to_string(), Value::String(label.clone()));
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

        let mut edges_list: Vec<Value> = self
            .edges
            .iter()
            .map(|((from, to), weight)| {
                let mut table = HashMap::new();
                table.insert("from".to_string(), Value::Integer(*from));
                table.insert("to".to_string(), Value::Integer(*to));
                table.insert("weight".to_string(), Value::Float(*weight));
                Value::Table(table)
            })
            .collect();

        edges_list.sort_by(|a, b| {
            if let (Value::Table(ta), Value::Table(tb)) = (a, b) {
                if let (Some(Value::Integer(fa)), Some(Value::Integer(fb))) =
                    (ta.get("from"), tb.get("from"))
                {
                    let cmp = fa.cmp(fb);
                    if cmp != std::cmp::Ordering::Equal {
                        return cmp;
                    }
                }
                if let (Some(Value::Integer(ta_val)), Some(Value::Integer(tb_val))) =
                    (ta.get("to"), tb.get("to"))
                {
                    return ta_val.cmp(tb_val);
                }
            }
            std::cmp::Ordering::Equal
        });

        let mut data = HashMap::new();
        data.insert("nodes".to_string(), Value::Array(nodes_list));
        data.insert("edges".to_string(), Value::Array(edges_list));
        data
    }

    fn session(&self) -> &Session {
        self.session
    }

    fn session_mut(&mut self) -> &mut Session {
        self.session
    }
}