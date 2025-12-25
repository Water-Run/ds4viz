//! TOML 格式写入器实现
//!
//! 将 Trace 对象序列化为 TOML 格式并写入文件
//!
//! # 作者
//!
//! WaterRun
//!
//! # 文件路径
//!
//! src/writer/toml_writer.rs
//!
//! # 日期
//!
//! 2025-12-25

use crate::error::Result;
use crate::trace::{State, Step, Trace, Value};
use std::fs::File;
use std::io::Write;

/// TOML 格式写入器
pub struct TomlWriter<'a> {
    /// 要序列化的 Trace 对象
    trace: &'a Trace,
}

impl<'a> TomlWriter<'a> {
    /// 创建新的写入器
    ///
    /// # 参数
    ///
    /// * `trace` - 要序列化的 Trace 对象
    #[must_use]
    pub const fn new(trace: &'a Trace) -> Self {
        Self { trace }
    }

    /// 将 Trace 写入指定路径的 TOML 文件
    ///
    /// # 参数
    ///
    /// * `path` - 输出文件路径
    ///
    /// # 错误
    ///
    /// 当文件写入失败时返回 IO 错误
    pub fn write(&self, path: &str) -> Result<()> {
        let content = self.serialize();
        let mut file = File::create(path)?;
        file.write_all(content.as_bytes())?;
        Ok(())
    }

    /// 将 Trace 序列化为 TOML 字符串
    #[must_use]
    pub fn serialize(&self) -> String {
        let mut lines = Vec::new();

        lines.extend(self.serialize_meta());
        lines.push(String::new());
        lines.extend(self.serialize_package());

        if let Some(remarks) = &self.trace.remarks {
            if !remarks.is_empty() {
                lines.push(String::new());
                lines.extend(self.serialize_remarks(remarks));
            }
        }

        lines.push(String::new());
        lines.extend(self.serialize_object());

        for state in &self.trace.states {
            lines.push(String::new());
            lines.extend(self.serialize_state(state));
        }

        for step in &self.trace.steps {
            lines.push(String::new());
            lines.extend(self.serialize_step(step));
        }

        if let Some(result) = &self.trace.result {
            lines.push(String::new());
            lines.extend(self.serialize_result(result));
        }

        if let Some(error) = &self.trace.error {
            lines.push(String::new());
            lines.extend(self.serialize_error(error));
        }

        lines.join("\n")
    }

    /// 序列化 meta 部分
    fn serialize_meta(&self) -> Vec<String> {
        vec![
            "[meta]".to_string(),
            format!("created_at = \"{}\"", self.trace.meta.created_at),
            format!("lang = \"{}\"", self.trace.meta.lang),
            format!("lang_version = \"{}\"", self.trace.meta.lang_version),
        ]
    }

    /// 序列化 package 部分
    fn serialize_package(&self) -> Vec<String> {
        vec![
            "[package]".to_string(),
            format!("name = \"{}\"", self.trace.package.name),
            format!("lang = \"{}\"", self.trace.package.lang),
            format!("version = \"{}\"", self.trace.package.version),
        ]
    }

    /// 序列化 remarks 部分
    fn serialize_remarks(&self, remarks: &crate::trace::Remarks) -> Vec<String> {
        let mut lines = vec!["[remarks]".to_string()];

        if let Some(title) = &remarks.title {
            lines.push(format!("title = \"{}\"", Self::escape_string(title)));
        }
        if let Some(author) = &remarks.author {
            lines.push(format!("author = \"{}\"", Self::escape_string(author)));
        }
        if let Some(comment) = &remarks.comment {
            lines.push(format!("comment = \"{}\"", Self::escape_string(comment)));
        }

        lines
    }

    /// 序列化 object 部分
    fn serialize_object(&self) -> Vec<String> {
        let mut lines = vec![
            "[object]".to_string(),
            format!("kind = \"{}\"", self.trace.object.kind),
        ];

        if let Some(label) = &self.trace.object.label {
            lines.push(format!("label = \"{}\"", Self::escape_string(label)));
        }

        lines
    }

    /// 序列化单个 state
    fn serialize_state(&self, state: &State) -> Vec<String> {
        let mut lines = vec![
            "[[states]]".to_string(),
            format!("id = {}", state.id),
            String::new(),
            "[states.data]".to_string(),
        ];

        lines.extend(self.serialize_state_data(&state.data));
        lines
    }

    /// 序列化 state.data 部分
    fn serialize_state_data(&self, data: &std::collections::HashMap<String, Value>) -> Vec<String> {
        let mut lines = Vec::new();

        // 按照固定顺序输出字段
        let ordered_keys = [
            "items", "top", "front", "rear", "head", "tail", "root", "nodes", "edges",
        ];

        for key in ordered_keys {
            if let Some(value) = data.get(key) {
                match key {
                    "nodes" | "edges" => {
                        if let Value::Array(arr) = value {
                            lines.push(format!(
                                "{} = {}",
                                key,
                                self.serialize_inline_table_array(arr)
                            ));
                        }
                    }
                    "items" => {
                        if let Value::Array(arr) = value {
                            lines.push(format!("{} = {}", key, self.serialize_simple_array(arr)));
                        }
                    }
                    _ => {
                        lines.push(format!("{} = {}", key, Self::serialize_value(value)));
                    }
                }
            }
        }

        lines
    }

    /// 序列化 inline table 数组
    fn serialize_inline_table_array(&self, arr: &[Value]) -> String {
        if arr.is_empty() {
            return "[]".to_string();
        }

        let items: Vec<String> = arr
            .iter()
            .filter_map(|v| {
                if let Value::Table(table) = v {
                    Some(self.serialize_inline_table(table))
                } else {
                    None
                }
            })
            .collect();

        format!("[\n  {}\n]", items.join(",\n  "))
    }

    /// 序列化单个 inline table
    fn serialize_inline_table(&self, table: &std::collections::HashMap<String, Value>) -> String {
        // 按照固定顺序输出字段
        let ordered_keys = [
            "id", "value", "label", "prev", "next", "left", "right", "from", "to", "weight",
        ];

        let pairs: Vec<String> = ordered_keys
            .iter()
            .filter_map(|key| {
                table
                    .get(*key)
                    .map(|v| format!("{} = {}", key, Self::serialize_value(v)))
            })
            .collect();

        format!("{{ {} }}", pairs.join(", "))
    }

    /// 序列化简单数组
    fn serialize_simple_array(&self, arr: &[Value]) -> String {
        if arr.is_empty() {
            return "[]".to_string();
        }

        let items: Vec<String> = arr.iter().map(Self::serialize_value).collect();
        format!("[{}]", items.join(", "))
    }

    /// 序列化单个 step
    fn serialize_step(&self, step: &Step) -> Vec<String> {
        let mut lines = vec![
            "[[steps]]".to_string(),
            format!("id = {}", step.id),
            format!("op = \"{}\"", step.op),
            format!("before = {}", step.before),
        ];

        if let Some(after) = step.after {
            lines.push(format!("after = {after}"));
        }

        if let Some(ret) = &step.ret {
            lines.push(format!("ret = {}", Self::serialize_value(ret)));
        }

        if let Some(note) = &step.note {
            lines.push(format!("note = \"{}\"", Self::escape_string(note)));
        }

        lines.push(String::new());
        lines.push("[steps.args]".to_string());

        for (key, value) in &step.args {
            lines.push(format!("{} = {}", key, Self::serialize_value(value)));
        }

        if let Some(code) = &step.code {
            lines.push(String::new());
            lines.push("[steps.code]".to_string());
            lines.push(format!("line = {}", code.line));
            if let Some(col) = code.col {
                lines.push(format!("col = {col}"));
            }
        }

        lines
    }

    /// 序列化 result 部分
    fn serialize_result(&self, result: &crate::trace::Result) -> Vec<String> {
        let mut lines = vec![
            "[result]".to_string(),
            format!("final_state = {}", result.final_state),
        ];

        if let Some(commit) = &result.commit {
            lines.push(String::new());
            lines.push("[result.commit]".to_string());
            lines.push(format!("op = \"{}\"", commit.op));
            lines.push(format!("line = {}", commit.line));
        }

        lines
    }

    /// 序列化 error 部分
    fn serialize_error(&self, error: &crate::trace::Error) -> Vec<String> {
        let mut lines = vec![
            "[error]".to_string(),
            format!("type = \"{}\"", error.error_type),
            format!("message = \"{}\"", Self::escape_string(&error.message)),
        ];

        if let Some(line) = error.line {
            lines.push(format!("line = {line}"));
        }

        if let Some(step) = error.step {
            lines.push(format!("step = {step}"));
        }

        if let Some(last_state) = error.last_state {
            lines.push(format!("last_state = {last_state}"));
        }

        lines
    }

    /// 序列化单个值
    fn serialize_value(value: &Value) -> String {
        match value {
            Value::Boolean(b) => {
                if *b {
                    "true".to_string()
                } else {
                    "false".to_string()
                }
            }
            Value::Integer(i) => i.to_string(),
            Value::Float(f) => {
                if f.fract() == 0.0 {
                    format!("{f:.1}")
                } else {
                    f.to_string()
                }
            }
            Value::String(s) => format!("\"{}\"", Self::escape_string(s)),
            Value::Array(arr) => {
                let items: Vec<String> = arr.iter().map(Self::serialize_value).collect();
                format!("[{}]", items.join(", "))
            }
            Value::Table(table) => {
                let pairs: Vec<String> = table
                    .iter()
                    .map(|(k, v)| format!("{k} = {}", Self::serialize_value(v)))
                    .collect();
                format!("{{ {} }}", pairs.join(", "))
            }
        }
    }

    /// 转义字符串中的特殊字符
    fn escape_string(s: &str) -> String {
        s.replace('\\', "\\\\")
            .replace('"', "\\\"")
            .replace('\n', "\\n")
            .replace('\r', "\\r")
    }
}