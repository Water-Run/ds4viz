//! 数据结构基础 trait 定义
//!
//! 定义所有数据结构共享的基础行为
//!
//! # 作者
//!
//! WaterRun
//!
//! # 文件路径
//!
//! src/structures/base.rs
//!
//! # 日期
//!
//! 2025-12-25

use crate::error::Result;
use crate::session::Session;
use crate::trace::Value;
use std::collections::HashMap;

/// 数据结构基础 trait
///
/// 定义所有数据结构必须实现的基础行为
pub trait BaseStructure {
    /// 构建当前状态的数据字典
    ///
    /// # 返回
    ///
    /// 状态数据字典
    fn build_data(&self) -> HashMap<String, Value>;

    /// 获取会话引用
    fn session(&self) -> &Session;

    /// 获取会话可变引用
    fn session_mut(&mut self) -> &mut Session;

    /// 初始化数据结构，添加初始状态
    fn initialize(&mut self) {
        let data = self.build_data();
        self.session_mut().add_state(data);
    }

    /// 记录操作步骤并添加新状态
    ///
    /// # 参数
    ///
    /// * `op` - 操作名称
    /// * `args` - 操作参数
    /// * `ret` - 返回值
    /// * `note` - 备注
    fn record_step(
        &mut self,
        op: &str,
        args: HashMap<String, Value>,
        ret: Option<Value>,
        note: Option<String>,
    ) -> Result<i64> {
        let before = self.session().get_last_state_id();
        let new_data = self.build_data();
        let after = self.session_mut().add_state(new_data);
        let step_id = self.session_mut().add_step(op, before, Some(after), args, ret, note);
        Ok(step_id)
    }
}