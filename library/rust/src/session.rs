//! 会话管理模块
//!
//! 提供上下文管理和全局配置功能
//!
//! # 作者
//!
//! WaterRun
//!
//! # 文件路径
//!
//! src/session.rs
//!
//! # 日期
//!
//! 2025-12-25

use crate::error::{Ds4VizError, ErrorType, Result};
use crate::trace::{
    Commit, Error as TraceError, Meta, Object, Package, Remarks, Result as TraceResult, State,
    Step, Trace, Value,
};
use crate::writer::TomlWriter;
use chrono::Utc;
use std::collections::HashMap;
use std::sync::RwLock;

/// 全局配置
static GLOBAL_CONFIG: RwLock<GlobalConfig> = RwLock::new(GlobalConfig::new());

/// 全局配置结构体
#[derive(Debug, Clone)]
struct GlobalConfig {
    /// 输出文件路径
    output_path: String,
    /// 可视化标题
    title: String,
    /// 作者信息
    author: String,
    /// 注释说明
    comment: String,
}

impl GlobalConfig {
    /// 创建默认配置
    const fn new() -> Self {
        Self {
            output_path: String::new(),
            title: String::new(),
            author: String::new(),
            comment: String::new(),
        }
    }

    /// 获取输出路径，如果为空则返回默认值
    fn get_output_path(&self) -> String {
        if self.output_path.is_empty() {
            "trace.toml".to_string()
        } else {
            self.output_path.clone()
        }
    }
}

/// 配置构建器
#[derive(Debug, Clone, Default)]
pub struct Config {
    /// 输出文件路径
    pub output_path: String,
    /// 可视化标题
    pub title: String,
    /// 作者信息
    pub author: String,
    /// 注释说明
    pub comment: String,
}

impl Config {
    /// 创建配置构建器
    #[must_use]
    pub fn builder() -> ConfigBuilder {
        ConfigBuilder::default()
    }
}

/// 配置构建器
#[derive(Debug, Clone, Default)]
pub struct ConfigBuilder {
    output_path: Option<String>,
    title: Option<String>,
    author: Option<String>,
    comment: Option<String>,
}

impl ConfigBuilder {
    /// 设置输出路径
    #[must_use]
    pub fn output_path(mut self, path: impl Into<String>) -> Self {
        self.output_path = Some(path.into());
        self
    }

    /// 设置标题
    #[must_use]
    pub fn title(mut self, title: impl Into<String>) -> Self {
        self.title = Some(title.into());
        self
    }

    /// 设置作者
    #[must_use]
    pub fn author(mut self, author: impl Into<String>) -> Self {
        self.author = Some(author.into());
        self
    }

    /// 设置注释
    #[must_use]
    pub fn comment(mut self, comment: impl Into<String>) -> Self {
        self.comment = Some(comment.into());
        self
    }

    /// 构建配置
    #[must_use]
    pub fn build(self) -> Config {
        Config {
            output_path: self.output_path.unwrap_or_default(),
            title: self.title.unwrap_or_default(),
            author: self.author.unwrap_or_default(),
            comment: self.comment.unwrap_or_default(),
        }
    }
}

/// 配置全局参数
///
/// # 参数
///
/// * `settings` - 配置对象
///
/// # 示例
///
/// ```
/// use ds4viz::{config, Config};
///
/// config(Config {
///     output_path: "my_trace.toml".into(),
///     title: "栈操作演示".into(),
///     author: "WaterRun".into(),
///     comment: "演示基本的栈操作".into(),
/// });
/// ```
pub fn config(settings: Config) {
    if let Ok(mut global) = GLOBAL_CONFIG.write() {
        global.output_path = settings.output_path;
        global.title = settings.title;
        global.author = settings.author;
        global.comment = settings.comment;
    }
}

/// 获取全局配置
fn get_global_config() -> GlobalConfig {
    GLOBAL_CONFIG
        .read()
        .map(|g| g.clone())
        .unwrap_or_else(|_| GlobalConfig::new())
}

/// 获取 Rust 版本字符串
fn get_rust_version() -> String {
    env!("CARGO_PKG_RUST_VERSION").to_string()
}

/// 会话管理器
///
/// 用于管理数据结构的操作轨迹记录
pub struct Session {
    /// 数据结构类型
    kind: String,
    /// 数据结构标签
    label: String,
    /// 输出文件路径
    output: String,
    /// 状态列表
    states: Vec<State>,
    /// 步骤列表
    steps: Vec<Step>,
    /// 状态计数器
    state_counter: i64,
    /// 步骤计数器
    step_counter: i64,
    /// 错误信息
    error: Option<TraceError>,
    /// 失败的步骤 ID
    failed_step_id: Option<i64>,
}

impl Session {
    /// 创建新会话
    ///
    /// # 参数
    ///
    /// * `kind` - 数据结构类型
    /// * `label` - 数据结构标签
    /// * `output` - 输出文件路径，`None` 使用全局配置
    pub fn new(kind: &str, label: &str, output: Option<&str>) -> Self {
        let global = get_global_config();
        let output_path = output
            .map(ToString::to_string)
            .unwrap_or_else(|| global.get_output_path());

        Self {
            kind: kind.to_string(),
            label: label.to_string(),
            output: output_path,
            states: Vec::new(),
            steps: Vec::new(),
            state_counter: 0,
            step_counter: 0,
            error: None,
            failed_step_id: None,
        }
    }

    /// 添加状态快照
    ///
    /// # 参数
    ///
    /// * `data` - 状态数据
    ///
    /// # 返回
    ///
    /// 新状态的 ID
    pub fn add_state(&mut self, data: HashMap<String, Value>) -> i64 {
        let state_id = self.state_counter;
        self.states.push(State {
            id: state_id,
            data,
        });
        self.state_counter += 1;
        state_id
    }

    /// 添加操作步骤
    ///
    /// # 参数
    ///
    /// * `op` - 操作名称
    /// * `before` - 操作前状态 ID
    /// * `after` - 操作后状态 ID
    /// * `args` - 操作参数
    /// * `ret` - 返回值
    /// * `note` - 备注
    ///
    /// # 返回
    ///
    /// 新步骤的 ID
    #[allow(clippy::too_many_arguments)]
    pub fn add_step(
        &mut self,
        op: &str,
        before: i64,
        after: Option<i64>,
        args: HashMap<String, Value>,
        ret: Option<Value>,
        note: Option<String>,
    ) -> i64 {
        let step_id = self.step_counter;
        self.steps.push(Step {
            id: step_id,
            op: op.to_string(),
            before,
            after,
            args,
            code: None,
            ret,
            note,
        });
        self.step_counter += 1;
        step_id
    }

    /// 获取最后一个状态的 ID
    ///
    /// # 返回
    ///
    /// 最后一个状态的 ID，若无状态则返回 -1
    #[must_use]
    pub fn get_last_state_id(&self) -> i64 {
        self.states.last().map_or(-1, |s| s.id)
    }

    /// 获取当前步骤计数器
    #[must_use]
    pub fn step_counter(&self) -> i64 {
        self.step_counter
    }

    /// 设置失败的步骤 ID
    pub fn set_failed_step_id(&mut self, step_id: i64) {
        self.failed_step_id = Some(step_id);
    }

    /// 成功完成会话
    pub fn finalize_success(&mut self) -> Result<()> {
        let trace = self.build_trace(true)?;
        let writer = TomlWriter::new(&trace);
        writer.write(&self.output)?;
        Ok(())
    }

    /// 错误完成会话
    pub fn finalize_error(&mut self, err: &Ds4VizError) -> Result<()> {
        let error_type = ErrorType::from(err);
        let message = err.to_string();
        let last_state = self.states.last().map(|s| s.id);

        self.error = Some(TraceError {
            error_type: error_type.as_str().to_string(),
            message: if message.len() > 512 {
                message[..512].to_string()
            } else {
                message
            },
            line: None,
            step: self.failed_step_id,
            last_state,
        });

        let trace = self.build_trace(false)?;
        let writer = TomlWriter::new(&trace);
        writer.write(&self.output)?;
        Ok(())
    }

    /// 构建 Trace 对象
    fn build_trace(&self, success: bool) -> Result<Trace> {
        let global = get_global_config();
        let now = Utc::now().format("%Y-%m-%dT%H:%M:%SZ").to_string();

        let meta = Meta {
            created_at: now,
            lang: "rust".to_string(),
            lang_version: get_rust_version(),
        };

        let package = Package {
            name: "ds4viz".to_string(),
            lang: "rust".to_string(),
            version: env!("CARGO_PKG_VERSION").to_string(),
        };

        let remarks = if global.title.is_empty()
            && global.author.is_empty()
            && global.comment.is_empty()
        {
            None
        } else {
            Some(Remarks {
                title: if global.title.is_empty() {
                    None
                } else {
                    Some(global.title.clone())
                },
                author: if global.author.is_empty() {
                    None
                } else {
                    Some(global.author.clone())
                },
                comment: if global.comment.is_empty() {
                    None
                } else {
                    Some(global.comment.clone())
                },
            })
        };

        let object = Object {
            kind: self.kind.clone(),
            label: if self.label.is_empty() {
                None
            } else {
                Some(self.label.clone())
            },
        };

        let result = if success {
            let final_state = self.states.last().map_or(0, |s| s.id);
            Some(TraceResult {
                final_state,
                commit: Some(Commit {
                    op: "commit".to_string(),
                    line: 0,
                }),
            })
        } else {
            None
        };

        Ok(Trace {
            meta,
            package,
            remarks,
            object,
            states: self.states.clone(),
            steps: self.steps.clone(),
            result,
            error: self.error.clone(),
        })
    }
}