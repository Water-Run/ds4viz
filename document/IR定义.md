# IR定义

中间语言采用`.toml`模式, 记录数据结构的元数据及每步骤的变迁.
渲染端仅依赖`.toml`: 一个`.toml`足以记录整个操作流. `.toml` IR平台无关, 完全独立.
一个`.toml`文件必须且仅描述一个数据结构实例的状态变迁.

![示意图](../assets/TOML%20IR中间协议架构图.png)  

## 顶层结构总览(固定顺序)

```toml
[meta]
[package]
[remarks]        # 可选

[object]         # 必需, 且唯一: 描述本 trace 对应的数据结构实例类型

[[states]]       # 1..N, 每个 state 是完整快照
[[steps]]        # 0..N, 每个 step 对应一次操作

[result]         # 成功时存在
[error]          # 失败时存在
```

## 字段定义与约束

### `[meta]` (必需)

`.toml`作为通用中间语言, `meta`仅描述 trace 自身的生成时间与"产生该 trace 的语言上下文".

```toml
[meta]
created_at = "2025-12-19T00:00:00Z"

# 生成该 trace 的语言上下文(用于解释语义, 例如: 数值类型, 以及代码行号来源)
lang = "python"
lang_version = "3.14.0"
```

**约束**

| 字段                | 类型   | 是否必需 | 约束(唯一确定)                                                                                       |
|---------------------|--------|---------:|------------------------------------------------------------------------------------------------------|
| `meta.created_at`   | string |       是 | RFC3339 格式时间戳(UTC 必须以 `Z` 结尾)                                                              |
| `meta.lang`         | string |       是 | 小写 ASCII 标识符, 取值必须属于: `python`, `c`, `zig`, `rust`, `java`, `csharp`, `typescript`, `lua` |
| `meta.lang_version` | string |       是 | 语言版本字符串, 仅允许字符集: `0-9`, `a-z`, `.`, `+`, `-`                                            |

### `[package]` (必需)

描述"用户调用的 ds4viz 库"信息.

```toml
[package]
name = "ds4viz"
lang = "python"
version = "1.0.0"
```

**约束**

| 字段                          | 类型   | 是否必需 | 约束(唯一确定)                                                                                       |
|-------------------------------|--------|---------:|------------------------------------------------------------------------------------------------------|
| `package.name`                | string |       是 | 小写 ASCII 标识符, 仅允许字符集: `0-9`, `a-z`, `_`, `-`; 且长度 1..64                                |
| `package.lang`                | string |       是 | 小写 ASCII 标识符, 取值必须属于: `python`, `c`, `zig`, `rust`, `java`, `csharp`, `typescript`, `lua` |
| `package.version`             | string |       是 | 语义化版本字符串, 格式必须匹配: `MAJOR.MINOR.PATCH` (例如 `1.0.0`), 仅允许数字与 `.`                 |
| `package.lang` 与 `meta.lang` | -      |       是 | 必须相等                                                                                             |

### `[remarks]` (可选)

用于描述该 trace 的展示信息(标题/作者/备注). 渲染器可直接用于 UI 展示, 不参与语义计算.

`title` 为整个可视化过程的完整名称, 与 `steps.phase` 形成两级展示层次: `title` 描述整个 trace, `phase` 标记每个步骤所属阶段.

```toml
[remarks]
title = "BST 插入与前序遍历"
author = "WaterRun"
comment = "Python BST example"
```

**约束**

| 字段              | 类型   | 是否必需 | 约束(唯一确定)                                                                       |
|-------------------|--------|---------:|--------------------------------------------------------------------------------------|
| `remarks`         | table  |       否 | 若存在, 则必须至少包含字段之一: `title`, `author`, `comment`; 且仅允许包含这三个字段 |
| `remarks.title`   | string |       否 | 长度 1..128; 不允许包含换行符                                                        |
| `remarks.author`  | string |       否 | 长度 1..64; 不允许包含换行符                                                         |
| `remarks.comment` | string |       否 | 长度 1..256; 不允许包含换行符                                                        |

---

### `[object]` (必需, 且唯一)

描述本`.toml`对应的唯一数据结构实例.

```toml
[object]
kind = "stack"
label = "stack"
```

**约束**

| 字段           | 类型   | 是否必需 | 约束(唯一确定)                                                                                                                 |
|----------------|--------|---------:|--------------------------------------------------------------------------------------------------------------------------------|
| `object`       | table  |       是 | 必须存在且仅出现一次                                                                                                           |
| `object.kind`  | string |       是 | 取值必须属于: `stack`, `queue`, `slist`, `dlist`, `binary_tree`, `bst`, `graph_undirected`, `graph_directed`, `graph_weighted` |
| `object.label` | string |       否 | 长度 1..64; 不允许包含换行符                                                                                                   |

---

### `states`: 状态快照 (必需, 1..N)

每个 state 是一个"完整可渲染快照". 渲染器必须能够在不依赖其他 state 的情况下渲染任意 state.
状态数据统一存放在 `states.data`, 且其结构由 `object.kind` 唯一确定.

#### 统一结构示例

```toml
[[states]]
id = 0

[states.data]
# data 的字段由 object.kind 唯一确定
```

**约束**

| 字段          | 类型           | 是否必需 | 约束(唯一确定)                                                  |
|---------------|----------------|---------:|-----------------------------------------------------------------|
| `states`      | array of table |       是 | 数组长度 1..N                                                   |
| `states.id`   | integer        |       是 | 必须为非负整数; 在整个文档内唯一; 且从 0 开始连续递增(步长为 1) |
| `states.data` | table          |       是 | 结构由 `object.kind` 唯一确定(见下文)                           |

---

## `data` 规范(按 `object.kind` 唯一确定)

通用约定:

* 空指针必须使用 `-1` 表示.
* 节点 `id` 必须为非负整数, 并且在同一个 `data.nodes` 内唯一.
* `value` 字段仅允许 TOML 标量类型: integer, float, string, boolean.
* `alias` 为所有含 `nodes` 的结构(即 `stack` 和 `queue` 以外的全部 kind)的通用可选字段, 用于为节点设置人类可读的标记(如 `"root"`, `"head"`, `"source"`), 供渲染器展示或步骤描述中引用.

| 字段            | 类型   | 是否必需 | 约束(唯一确定)                                                                        |
|-----------------|--------|---------:|---------------------------------------------------------------------------------------|
| `nodes[].alias` | string |       否 | 长度 1..64; 仅允许字符集: `a-z`, `A-Z`, `0-9`, `_`, `-`; 在同一 state 的 nodes 内唯一 |

为确保空数组可表示为 `[]`, `nodes`/`edges` 在 `data` 中使用"array of inline table"表达.

### `stack`

```toml
[states.data]
items = [10, 20]
top = 1
```

**约束**

| 字段/规则 | 约束(唯一确定)                                                                         |
|-----------|----------------------------------------------------------------------------------------|
| 顶层字段  | 必须包含且仅包含字段: `items`, `top`                                                   |
| `items`   | 必须为 array; 元素类型必须属于: integer, float, string, boolean                        |
| `top`     | 必须为 integer; 且满足: 空栈 `items = []` 时 `top = -1`, 非空时 `top = len(items) - 1` |

### `queue`

```toml
[states.data]
items = [10, 20]
front = 0
rear = 1
```

**约束**

| 字段/规则    | 约束(唯一确定)                                                                                              |
|--------------|-------------------------------------------------------------------------------------------------------------|
| 顶层字段     | 必须包含且仅包含字段: `items`, `front`, `rear`                                                              |
| `items`      | 必须为 array; 元素类型必须属于: integer, float, string, boolean                                             |
| `front/rear` | 必须为 integer; 且满足: `items = []` 时 `front = -1` 且 `rear = -1`, 否则 `0 <= front <= rear < len(items)` |

### `slist` (单链表)

```toml
[states.data]
head = 0
nodes = [
  { id = 0, alias = "head", value = 10, next = 1 },
  { id = 1, value = 20, next = -1 }
]
```

**约束**

| 字段/规则 | 约束(唯一确定)                                                                                            |
|-----------|-----------------------------------------------------------------------------------------------------------|
| 顶层字段  | 必须包含且仅包含字段: `nodes`, `head`                                                                     |
| `nodes`   | 必须为 array; 元素必须为 inline table; 必需字段: `id`, `value`, `next`; 可选字段: `alias`; 不允许其它字段 |
| `head`    | 必须为 `-1` 或引用一个存在的 `nodes.id`                                                                   |
| `next`    | 必须为 `-1` 或引用一个存在的 `nodes.id`                                                                   |

### `dlist` (双向链表)

```toml
[states.data]
head = 0
tail = 1
nodes = [
  { id = 0, alias = "head", value = 10, prev = -1, next = 1 },
  { id = 1, value = 20, prev = 0,  next = -1 }
]
```

**约束**

| 字段/规则   | 约束(唯一确定)                                                                                                    |
|-------------|-------------------------------------------------------------------------------------------------------------------|
| 顶层字段    | 必须包含且仅包含字段: `nodes`, `head`, `tail`                                                                     |
| `nodes`     | 必须为 array; 元素必须为 inline table; 必需字段: `id`, `value`, `prev`, `next`; 可选字段: `alias`; 不允许其它字段 |
| `head/tail` | 必须同时为 `-1` 或同时不为 `-1`; 且若不为 `-1` 则必须引用一个存在的 `nodes.id`                                    |
| `prev/next` | 必须为 `-1` 或引用一个存在的 `nodes.id`                                                                           |

### `binary_tree`

```toml
[states.data]
root = 0
nodes = [
  { id = 0, alias = "root", value = 10, left = 1, right = 2 },
  { id = 1, value = 5,  left = -1, right = -1 },
  { id = 2, value = 15, left = -1, right = -1 }
]
```

**约束**

| 字段/规则    | 约束(唯一确定)                                                                                                     |
|--------------|--------------------------------------------------------------------------------------------------------------------|
| 顶层字段     | 必须包含且仅包含字段: `nodes`, `root`                                                                              |
| `nodes`      | 必须为 array; 元素必须为 inline table; 必需字段: `id`, `value`, `left`, `right`; 可选字段: `alias`; 不允许其它字段 |
| `root`       | 必须为 `-1` 或引用一个存在的 `nodes.id`                                                                            |
| `left/right` | 必须为 `-1` 或引用一个存在的 `nodes.id`                                                                            |

### `bst` (二叉搜索树)

结构与 `binary_tree` 完全一致. 本 IR 不在`.toml`层面表达 BST 有序性约束.

```toml
[states.data]
root = 0
nodes = [
  { id = 0, alias = "root", value = 10, left = 1, right = 2 },
  { id = 1, value = 5,  left = -1, right = -1 },
  { id = 2, value = 15, left = -1, right = -1 }
]
```

**约束**

| 字段/规则    | 约束(唯一确定)                        |
|--------------|---------------------------------------|
| 顶层字段     | 必须包含且仅包含字段: `nodes`, `root` |
| 其余字段规则 | 与 `binary_tree` 完全一致             |

### `graph_undirected` (无向图)

```toml
[states.data]
nodes = [
  { id = 0, alias = "source", label = "A" },
  { id = 1, label = "B" }
]
edges = [
  { from = 0, to = 1 }
]
```

**约束**

| 字段/规则    | 约束(唯一确定)                                                                                    |
|--------------|---------------------------------------------------------------------------------------------------|
| 顶层字段     | 必须包含且仅包含字段: `nodes`, `edges`                                                            |
| `nodes`      | 必须为 array; 元素必须为 inline table; 必需字段: `id`, `label`; 可选字段: `alias`; 不允许其它字段 |
| `label`      | 必须为 string; 长度 1..32; 不允许包含换行符                                                       |
| `edges`      | 必须为 array; 元素必须为 inline table, 且仅包含字段: `from`, `to`                                 |
| 无向边规范化 | 每条边必须满足 `from < to`                                                                        |
| 多重边       | 同一 state 内不允许出现重复的 `(from,to)`                                                         |

### `graph_directed` (有向图)

```toml
[states.data]
nodes = [
  { id = 0, alias = "source", label = "A" },
  { id = 1, label = "B" }
]
edges = [
  { from = 0, to = 1 }
]
```

**约束**

| 字段/规则 | 约束(唯一确定)                                                                                    |
|-----------|---------------------------------------------------------------------------------------------------|
| 顶层字段  | 必须包含且仅包含字段: `nodes`, `edges`                                                            |
| `nodes`   | 必须为 array; 元素必须为 inline table; 必需字段: `id`, `label`; 可选字段: `alias`; 不允许其它字段 |
| `label`   | 必须为 string; 长度 1..32; 不允许包含换行符                                                       |
| `edges`   | 必须为 array; 元素必须为 inline table, 且仅包含字段: `from`, `to`                                 |
| 自环      | 每条边必须满足 `from != to`                                                                       |
| 多重边    | 同一 state 内不允许出现重复的 `(from,to)`                                                         |

### `graph_weighted` (带权图)

```toml
[states.data]
nodes = [
  { id = 0, alias = "source", label = "A" },
  { id = 1, label = "B" }
]
edges = [
  { from = 0, to = 1, weight = 3.2 }
]
```

**约束**

| 字段/规则 | 约束(唯一确定)                                                                                    |
|-----------|---------------------------------------------------------------------------------------------------|
| 顶层字段  | 必须包含且仅包含字段: `nodes`, `edges`                                                            |
| `nodes`   | 必须为 array; 元素必须为 inline table; 必需字段: `id`, `label`; 可选字段: `alias`; 不允许其它字段 |
| `label`   | 必须为 string; 长度 1..32; 不允许包含换行符                                                       |
| `edges`   | 必须为 array; 元素必须为 inline table, 且仅包含字段: `from`, `to`, `weight`                       |
| 自环      | 每条边必须满足 `from != to`                                                                       |
| `weight`  | 必须存在; 类型必须为 number                                                                       |
| 多重边    | 同一 state 内不允许出现重复的 `(from,to)`                                                         |

---

### `steps`: 操作序列 (可选, 0..N)

`steps` 描述每一次操作, 并通过 `before/after` 引用 `states.id` 完成前进/后退/跳转.
源码映射仅通过 `steps.code.line`/`steps.code.col` 提供.
由于文档仅描述一个对象, `steps` 不包含对象引用字段.

#### 统一结构示例

```toml
[[steps]]
id = 0
op = "push"
phase = "build"
before = 0
after = 1
ret = 10
note = "optional"
highlights = [
  { kind = "item", target = 0, style = "focus", level = 3 }
]

[steps.args]
value = 10

[steps.code]
line = 5
col = 1
```

**约束**

| 字段               | 类型                  |                 是否必需 | 约束(唯一确定)                                                                                                 |
|--------------------|-----------------------|-------------------------:|----------------------------------------------------------------------------------------------------------------|
| `steps`            | array of table        |                       否 | 若存在, 数组长度 0..N                                                                                          |
| `steps.id`         | integer               |                       是 | 必须为非负整数; 在整个文档内唯一; 且从 0 开始连续递增(步长为 1)                                                |
| `steps.op`         | string                |                       是 | 小写 ASCII 标识符, 仅允许字符集: `0-9`, `a-z`, `_`, `-`; 且长度 1..64                                          |
| `steps.phase`      | string                |                       否 | 长度 1..64; 不允许包含换行符                                                                                   |
| `steps.before`     | integer               |                       是 | 必须引用已存在的 `states.id`                                                                                   |
| `steps.after`      | integer               |                       否 | 若存在, 必须引用已存在的 `states.id`; 若不存在, 则文档必须存在 `[error]` 且 `error.step` 必须等于该 `steps.id` |
| `steps.args`       | table                 |                       是 | 必须存在(允许为空 table); key 必须为小写 ASCII 标识符(字符集: `0-9`, `a-z`, `_`, `-`)                          |
| `steps.code`       | table                 |                       否 | 若存在, 则必须包含字段 `line`; 且仅允许字段: `line`, `col`                                                     |
| `steps.code.line`  | integer               | 是(当 `steps.code` 存在) | 必须为正整数(从 1 开始)                                                                                        |
| `steps.code.col`   | integer               |                       否 | 必须为正整数(从 1 开始)                                                                                        |
| `steps.ret`        | any                   |                       否 | 允许任意 TOML 值(标量/数组/table), 用于记录返回值                                                              |
| `steps.note`       | string                |                       否 | 长度 1..256; 不允许包含换行符                                                                                  |
| `steps.highlights` | array of inline table |                       否 | 若存在, 数组长度 0..N; 详见 highlights 约束                                                                    |

#### `steps.phase`: 阶段标记 (可选)

用于标记当前步骤所属的阶段. 渲染器可根据 `phase` 对步骤分组展示(如在时间线上显示分隔标签). 同一 `phase` 值的连续 steps 属于同一阶段.

`phase` 与 `remarks.title` 形成两级展示层次:

```
remarks.title = "BST 插入与前序遍历"      ← 顶层, 描述整个 trace
  steps[0..2].phase = "construction"     ← 阶段级
  steps[3..5].phase = "preorder"         ← 阶段级
```

#### `steps.highlights`: 高亮标记 (可选)

每个 step 可携带一个 `highlights` 数组, 用于标记该步骤中需要视觉强调的元素. `highlights` 为全量快照语义: 每个 step 的 `highlights` 完整描述该步骤的所有高亮, 渲染器无需回溯之前的 step 来计算累积状态.

`highlights` 中的引用基于该 step 关联的 state: 若 `after` 存在则引用 `after` 对应的 state, 否则引用 `before` 对应的 state.

```toml
highlights = [
  { kind = "node", target = 0, style = "focus", level = 3 },
  { kind = "node", target = 1, style = "comparing", level = 1 },
  { kind = "edge", from = 0, to = 1, style = "active" }
]
```

**highlights 元素约束**

| 字段                  | 类型    | 是否必需 | 约束(唯一确定)                                                                                                                                              |
|-----------------------|---------|---------:|-------------------------------------------------------------------------------------------------------------------------------------------------------------|
| `highlights[].kind`   | string  |       是 | 取值必须属于: `"node"`, `"item"`, `"edge"`                                                                                                                  |
| `highlights[].target` | integer | 条件必需 | `kind = "node"` 时必须存在, 引用关联 state 中存在的 `nodes[].id`; `kind = "item"` 时必须存在, 为关联 state 中 `items` 的有效索引                            |
| `highlights[].from`   | integer | 条件必需 | `kind = "edge"` 时必须存在, 引用关联 state 中存在的 `nodes[].id`                                                                                            |
| `highlights[].to`     | integer | 条件必需 | `kind = "edge"` 时必须存在, 引用关联 state 中存在的 `nodes[].id`; `(from, to)` 必须匹配关联 state 中存在的一条边                                            |
| `highlights[].style`  | string  |       是 | 长度 1..32; 仅允许字符集: `a-z`, `0-9`, `_`, `-`                                                                                                            |
| `highlights[].level`  | integer |       否 | 正整数, 1..9; 默认为 1; 数值越大越醒目                                                                                                                      |
| 字段互斥              | -       |        - | `kind = "node"` 或 `kind = "item"` 时: 仅允许字段 `kind`, `target`, `style`, `level`; `kind = "edge"` 时: 仅允许字段 `kind`, `from`, `to`, `style`, `level` |

**`kind` 与 `object.kind` 的交叉约束**

| `object.kind`                                          | 允许的 `highlights[].kind` |
|--------------------------------------------------------|----------------------------|
| `stack`, `queue`                                       | `"item"`                   |
| `slist`, `dlist`, `binary_tree`, `bst`                 | `"node"`                   |
| `graph_undirected`, `graph_directed`, `graph_weighted` | `"node"`, `"edge"`         |

**边高亮规范化约束**: 当 `object.kind = "graph_undirected"` 且 `kind = "edge"` 时, 必须满足 `from < to`(与无向边规范化一致). 当 `object.kind` 为有向图或带权图时, `from != to`(与自环约束一致).

**预定义 `style` 值**(渲染器必须支持; 自定义值可回退为 `"focus"` 表现):

| style       | 语义         |
|-------------|--------------|
| `focus`     | 当前焦点     |
| `visited`   | 已访问       |
| `active`    | 参与当前计算 |
| `comparing` | 正在比较     |
| `found`     | 查找命中     |
| `error`     | 异常状态     |

**`level` 的渲染器解释**: `level` 是无颜色语义的视觉分级. 渲染器自行决定如何映射(透明度, 边框粗细, 动画强度等). 同一 step 内同一 style 的不同 level 允许渲染器区分主次.

---

### `[result]` 成功结果 (成功时必需, Option 风格)

`[result]` 的存在表示成功, 不需要 `ok = true`.
`commit` 信息为可选项.

```toml
[result]
final_state = 4

[result.commit]
op = "commit"
line = 10
```

**约束**

| 字段                 | 类型    | 是否必需 | 约束(唯一确定)                                                        |
|----------------------|---------|---------:|-----------------------------------------------------------------------|
| `result`             | table   |       否 | 若存在, 则文档不得存在 `[error]`                                      |
| `result.final_state` | integer |       是 | 必须引用已存在的 `states.id`                                          |
| `result.commit`      | table   |       否 | 若存在, 则必须包含且仅包含字段: `op`, `line`                          |
| `result.commit.op`   | string  |       是 | 小写 ASCII 标识符, 仅允许字符集: `0-9`, `a-z`, `_`, `-`; 且长度 1..64 |
| `result.commit.line` | integer |       是 | 必须为正整数(从 1 开始)                                               |

---

### `[error]` 执行/验证错误 (失败时必需)

`[error]` 的存在表示失败, 同时不得存在 `[result]`.

```toml
[error]
type = "runtime"
message = "Parent node not found: 999"
line = 20
step = 3
last_state = 3
```

**约束**

| 字段               | 类型    | 是否必需 | 约束(唯一确定)                                                         |
|--------------------|---------|---------:|------------------------------------------------------------------------|
| `error`            | table   |       否 | 若存在, 则文档不得存在 `[result]`                                      |
| `error.type`       | string  |       是 | 取值必须属于: `runtime`, `timeout`, `validation`, `sandbox`, `unknown` |
| `error.message`    | string  |       是 | 长度 1..512; 不允许包含换行符                                          |
| `error.line`       | integer |       否 | 必须为正整数(从 1 开始)                                                |
| `error.step`       | integer |       否 | 若存在, 必须引用已存在的 `steps.id`                                    |
| `error.last_state` | integer |       否 | 若存在, 必须引用已存在的 `states.id`                                   |

---

## 文档级约束

| 约束项                 | 约束(唯一确定)                                                                                                                                                                                                      |
|------------------------|---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| 顶层结局唯一性         | 文档必须且仅能出现 `[result]` 或 `[error]` 其中之一                                                                                                                                                                 |
| 跨状态节点 `id` 稳定性 | 同一 `id` 值在所有 states 中表示同一个逻辑节点. 若节点在 state N 中存在但在 state N+1 中不存在, 表示该节点被删除. 已删除节点的 `id` 不得在后续 states 中被分配给新节点. 此约束适用于所有含 `nodes` 的 `object.kind` |

---

# 完整`.toml`示例

下面分别给出线性/树/图的完整 trace 示例(均为成功案例, 使用 `[result]`).

## 线性结构示例: Stack

```toml
[meta]
created_at = "2025-12-19T00:00:00Z"
lang = "python"
lang_version = "3.14.0"

[package]
name = "ds4viz"
lang = "python"
version = "1.0.0"

[remarks]
title = "Stack push/pop demo"
author = "WaterRun"
comment = "push/pop"

[object]
kind = "stack"
label = "stack"

[[states]]
id = 0

[states.data]
items = []
top = -1

[[states]]
id = 1

[states.data]
items = [10]
top = 0

[[states]]
id = 2

[states.data]
items = [10, 20]
top = 1

[[states]]
id = 3

[states.data]
items = [10]
top = 0

[[states]]
id = 4

[states.data]
items = [10, 30]
top = 1

[[steps]]
id = 0
op = "push"
phase = "build"
before = 0
after = 1
highlights = [
  { kind = "item", target = 0, style = "focus", level = 3 }
]

[steps.args]
value = 10

[steps.code]
line = 6

[[steps]]
id = 1
op = "push"
phase = "build"
before = 1
after = 2
highlights = [
  { kind = "item", target = 0, style = "visited", level = 1 },
  { kind = "item", target = 1, style = "focus", level = 3 }
]

[steps.args]
value = 20

[steps.code]
line = 7

[[steps]]
id = 2
op = "pop"
phase = "modify"
before = 2
after = 3
ret = 20
highlights = [
  { kind = "item", target = 0, style = "focus", level = 3 }
]

[steps.args]

[steps.code]
line = 8

[[steps]]
id = 3
op = "push"
phase = "modify"
before = 3
after = 4
highlights = [
  { kind = "item", target = 0, style = "visited", level = 1 },
  { kind = "item", target = 1, style = "focus", level = 3 }
]

[steps.args]
value = 30

[steps.code]
line = 9

[result]
final_state = 4

[result.commit]
op = "commit"
line = 11
```

## 树结构示例: BinaryTree

```toml
[meta]
created_at = "2025-12-19T00:00:00Z"
lang = "csharp"
lang_version = "12.0"

[package]
name = "ds4viz"
lang = "csharp"
version = "1.0.0"

[remarks]
title = "BinaryTree 构建与遍历"
author = "WaterRun"
comment = "insert root/left/right"

[object]
kind = "binary_tree"
label = "binary_tree"

[[states]]
id = 0

[states.data]
root = -1
nodes = []

[[states]]
id = 1

[states.data]
root = 0
nodes = [
  { id = 0, alias = "root", value = 10, left = -1, right = -1 }
]

[[states]]
id = 2

[states.data]
root = 0
nodes = [
  { id = 0, alias = "root", value = 10, left = 1, right = -1 },
  { id = 1, value = 5,  left = -1, right = -1 }
]

[[states]]
id = 3

[states.data]
root = 0
nodes = [
  { id = 0, alias = "root", value = 10, left = 1, right = 2 },
  { id = 1, value = 5,  left = -1, right = -1 },
  { id = 2, value = 15, left = -1, right = -1 }
]

[[steps]]
id = 0
op = "insert_root"
phase = "construction"
before = 0
after = 1
highlights = [
  { kind = "node", target = 0, style = "focus", level = 3 }
]

[steps.args]
value = 10

[steps.code]
line = 12

[[steps]]
id = 1
op = "insert_left"
phase = "construction"
before = 1
after = 2
highlights = [
  { kind = "node", target = 0, style = "active", level = 1 },
  { kind = "node", target = 1, style = "focus", level = 3 }
]

[steps.args]
parent = 10
value = 5

[steps.code]
line = 13

[[steps]]
id = 2
op = "insert_right"
phase = "construction"
before = 2
after = 3
highlights = [
  { kind = "node", target = 0, style = "active", level = 1 },
  { kind = "node", target = 2, style = "focus", level = 3 }
]

[steps.args]
parent = 10
value = 15

[steps.code]
line = 14

[result]
final_state = 3

[result.commit]
op = "commit"
line = 16
```

## 图结构示例: GraphWeighted

```toml
[meta]
created_at = "2025-12-19T00:00:00Z"
lang = "rust"
lang_version = "1.80.0"

[package]
name = "ds4viz"
lang = "rust"
version = "1.0.0"

[remarks]
title = "Weighted graph construction"
author = "WaterRun"
comment = "add nodes/edges"

[object]
kind = "graph_weighted"
label = "graph_weighted"

[[states]]
id = 0

[states.data]
nodes = []
edges = []

[[states]]
id = 1

[states.data]
nodes = [
  { id = 0, alias = "source", label = "A" }
]
edges = []

[[states]]
id = 2

[states.data]
nodes = [
  { id = 0, alias = "source", label = "A" },
  { id = 1, label = "B" }
]
edges = []

[[states]]
id = 3

[states.data]
nodes = [
  { id = 0, alias = "source", label = "A" },
  { id = 1, label = "B" }
]
edges = [
  { from = 0, to = 1, weight = 3.2 }
]

[[states]]
id = 4

[states.data]
nodes = [
  { id = 0, alias = "source", label = "A" },
  { id = 1, label = "B" },
  { id = 2, label = "C" }
]
edges = [
  { from = 0, to = 1, weight = 3.2 }
]

[[states]]
id = 5

[states.data]
nodes = [
  { id = 0, alias = "source", label = "A" },
  { id = 1, label = "B" },
  { id = 2, label = "C" }
]
edges = [
  { from = 0, to = 1, weight = 3.2 },
  { from = 1, to = 2, weight = 1.5 }
]

[[steps]]
id = 0
op = "add_node"
phase = "build-nodes"
before = 0
after = 1
highlights = [
  { kind = "node", target = 0, style = "focus", level = 3 }
]

[steps.args]
id = 0
label = "A"

[steps.code]
line = 8

[[steps]]
id = 1
op = "add_node"
phase = "build-nodes"
before = 1
after = 2
highlights = [
  { kind = "node", target = 0, style = "visited", level = 1 },
  { kind = "node", target = 1, style = "focus", level = 3 }
]

[steps.args]
id = 1
label = "B"

[steps.code]
line = 9

[[steps]]
id = 2
op = "add_edge"
phase = "build-edges"
before = 2
after = 3
highlights = [
  { kind = "node", target = 0, style = "active", level = 1 },
  { kind = "node", target = 1, style = "active", level = 1 },
  { kind = "edge", from = 0, to = 1, style = "focus", level = 3 }
]

[steps.args]
from = 0
to = 1
weight = 3.2

[steps.code]
line = 10

[[steps]]
id = 3
op = "add_node"
phase = "build-nodes"
before = 3
after = 4
highlights = [
  { kind = "node", target = 2, style = "focus", level = 3 }
]

[steps.args]
id = 2
label = "C"

[steps.code]
line = 11

[[steps]]
id = 4
op = "add_edge"
phase = "build-edges"
before = 4
after = 5
highlights = [
  { kind = "node", target = 1, style = "active", level = 1 },
  { kind = "node", target = 2, style = "active", level = 1 },
  { kind = "edge", from = 1, to = 2, style = "focus", level = 3 }
]

[steps.args]
from = 1
to = 2
weight = 1.5

[steps.code]
line = 12

[result]
final_state = 5

[result.commit]
op = "commit"
line = 14
```