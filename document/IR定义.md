# IR定义

中间语言采用`.toml`模式, 记录数据结构的元数据及每步骤的变迁.
渲染端仅依赖`.toml`: 一个`.toml`足以记录整个操作流. `.toml` IR平台无关, 完全独立.
一个`.toml`文件必须且仅描述一个数据结构实例的状态变迁.

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

`.toml`作为通用中间语言, `meta`仅描述 trace 自身的生成时间与“产生该 trace 的语言上下文”.

```toml
[meta]
created_at = "2025-12-19T00:00:00Z"

# 生成该 trace 的语言上下文(用于解释语义, 例如: 数值类型, 以及代码行号来源)
lang = "python"
lang_version = "3.14.0"
```

**约束**

| 字段                  | 类型     | 是否必需 | 约束(唯一确定)                                                                                  |
| ------------------- | ------ | ----:| ----------------------------------------------------------------------------------------- |
| `meta.created_at`   | string | 是    | RFC3339 格式时间戳(UTC 必须以 `Z` 结尾)                                                             |
| `meta.lang`         | string | 是    | 小写 ASCII 标识符, 取值必须属于: `python`, `c`, `zig`, `rust`, `java`, `csharp`, `typescript`, `lua` |
| `meta.lang_version` | string | 是    | 语言版本字符串, 仅允许字符集: `0-9`, `a-z`, `.`, `+`, `-`                                              |

### `[package]` (必需)

描述“用户调用的 ds4viz 库”信息.

```toml
[package]
name = "ds4viz"
lang = "python"
version = "1.0.0"
```

**约束**

| 字段                           | 类型     | 是否必需 | 约束(唯一确定)                                                                                  |
| ---------------------------- | ------ | ----:| ----------------------------------------------------------------------------------------- |
| `package.name`               | string | 是    | 小写 ASCII 标识符, 仅允许字符集: `0-9`, `a-z`, `_`, `-`; 且长度 1..64                                   |
| `package.lang`               | string | 是    | 小写 ASCII 标识符, 取值必须属于: `python`, `c`, `zig`, `rust`, `java`, `csharp`, `typescript`, `lua` |
| `package.version`            | string | 是    | 语义化版本字符串, 格式必须匹配: `MAJOR.MINOR.PATCH` (例如 `1.0.0`), 仅允许数字与 `.`                            |
| `package.lang` 与 `meta.lang` | -      | 是    | 必须相等                                                                                      |

### `[remarks]` (可选)

用于描述该 trace 的展示信息(标题/作者/备注). 渲染器可直接用于 UI 展示, 不参与语义计算.

```toml
[remarks]
title = "Stack demo"
author = "WaterRun"
comment = "Python stack example"
```

**约束**

| 字段                | 类型     | 是否必需 | 约束(唯一确定)                                                    |
| ----------------- | ------ | ----:| ----------------------------------------------------------- |
| `remarks`         | table  | 否    | 若存在, 则必须至少包含字段之一: `title`, `author`, `comment`; 且仅允许包含这三个字段 |
| `remarks.title`   | string | 否    | 长度 1..128; 不允许包含换行符                                         |
| `remarks.author`  | string | 否    | 长度 1..64; 不允许包含换行符                                          |
| `remarks.comment` | string | 否    | 长度 1..256; 不允许包含换行符                                         |

---

### `[object]` (必需, 且唯一)

描述本`.toml`对应的唯一数据结构实例.

```toml
[object]
kind = "stack"
label = "stack"
```

**约束**

| 字段             | 类型     | 是否必需 | 约束(唯一确定)                                                                                                                 |
| -------------- | ------ | ----:| ------------------------------------------------------------------------------------------------------------------------ |
| `object`       | table  | 是    | 必须存在且仅出现一次                                                                                                               |
| `object.kind`  | string | 是    | 取值必须属于: `stack`, `queue`, `slist`, `dlist`, `binary_tree`, `bst`, `graph_undirected`, `graph_directed`, `graph_weighted` |
| `object.label` | string | 否    | 长度 1..64; 不允许包含换行符                                                                                                       |

---

### `states`: 状态快照 (必需, 1..N)

每个 state 是一个“完整可渲染快照”. 渲染器必须能够在不依赖其他 state 的情况下渲染任意 state.
状态数据统一存放在 `states.data`, 且其结构由 `object.kind` 唯一确定.

#### 统一结构示例

```toml
[[states]]
id = 0

[states.data]
# data 的字段由 object.kind 唯一确定
```

**约束**

| 字段            | 类型             | 是否必需 | 约束(唯一确定)                              |
| ------------- | -------------- | ----:| ------------------------------------- |
| `states`      | array of table | 是    | 数组长度 1..N                             |
| `states.id`   | integer        | 是    | 必须为非负整数; 在整个文档内唯一; 且从 0 开始连续递增(步长为 1) |
| `states.data` | table          | 是    | 结构由 `object.kind` 唯一确定(见下文)           |

---

## `data` 规范(按 `object.kind` 唯一确定)

通用约定:

* 空指针必须使用 `-1` 表示.
* 节点 id 必须为非负整数, 并且在同一个 `data.nodes` 内唯一.
* `value`字段仅允许 TOML 标量类型: integer, float, string, boolean.

为确保空数组可表示为 `[]`, `nodes`/`edges` 在`data`中使用“array of inline table”表达.

### `stack`

```toml
[states.data]
items = [10, 20]
top = 1
```

**约束**

| 字段/规则   | 约束(唯一确定)                                                                   |
| ------- | -------------------------------------------------------------------------- |
| 顶层字段    | 必须包含且仅包含字段: `items`, `top`                                                 |
| `items` | 必须为 array; 元素类型必须属于: integer, float, string, boolean                       |
| `top`   | 必须为 integer; 且满足: 空栈 `items = []` 时 `top = -1`, 非空时 `top = len(items) - 1` |

### `queue`

```toml
[states.data]
items = [10, 20]
front = 0
rear = 1
```

**约束**

| 字段/规则        | 约束(唯一确定)                                                                                          |
| ------------ | ------------------------------------------------------------------------------------------------- |
| 顶层字段         | 必须包含且仅包含字段: `items`, `front`, `rear`                                                              |
| `items`      | 必须为 array; 元素类型必须属于: integer, float, string, boolean                                              |
| `front/rear` | 必须为 integer; 且满足: `items = []` 时 `front = -1` 且 `rear = -1`, 否则 `0 <= front <= rear < len(items)` |

### `slist` (单链表)

```toml
[states.data]
head = 0
nodes = [
  { id = 0, value = 10, next = 1 },
  { id = 1, value = 20, next = -1 }
]
```

**约束**

| 字段/规则   | 约束(唯一确定)                                                     |
| ------- | ------------------------------------------------------------ |
| 顶层字段    | 必须包含且仅包含字段: `nodes`, `head`                                  |
| `nodes` | 必须为 array; 元素必须为 inline table, 且仅包含字段: `id`, `value`, `next` |
| `head`  | 必须为 `-1` 或引用一个存在的 `nodes.id`                                 |
| `next`  | 必须为 `-1` 或引用一个存在的 `nodes.id`                                 |

### `dlist` (双向链表)

```toml
[states.data]
head = 0
tail = 1
nodes = [
  { id = 0, value = 10, prev = -1, next = 1 },
  { id = 1, value = 20, prev = 0,  next = -1 }
]
```

**约束**

| 字段/规则       | 约束(唯一确定)                                                             |
| ----------- | -------------------------------------------------------------------- |
| 顶层字段        | 必须包含且仅包含字段: `nodes`, `head`, `tail`                                  |
| `nodes`     | 必须为 array; 元素必须为 inline table, 且仅包含字段: `id`, `value`, `prev`, `next` |
| `head/tail` | 必须同时为 `-1` 或同时不为 `-1`; 且若不为 `-1` 则必须引用一个存在的 `nodes.id`               |
| `prev/next` | 必须为 `-1` 或引用一个存在的 `nodes.id`                                         |

### `binary_tree`

```toml
[states.data]
root = 0
nodes = [
  { id = 0, value = 10, left = 1, right = 2 },
  { id = 1, value = 5,  left = -1, right = -1 },
  { id = 2, value = 15, left = -1, right = -1 }
]
```

**约束**

| 字段/规则        | 约束(唯一确定)                                                              |
| ------------ | --------------------------------------------------------------------- |
| 顶层字段         | 必须包含且仅包含字段: `nodes`, `root`                                           |
| `nodes`      | 必须为 array; 元素必须为 inline table, 且仅包含字段: `id`, `value`, `left`, `right` |
| `root`       | 必须为 `-1` 或引用一个存在的 `nodes.id`                                          |
| `left/right` | 必须为 `-1` 或引用一个存在的 `nodes.id`                                          |

### `bst` (二叉搜索树)

结构与 `binary_tree` 完全一致. 本 IR 不在`.toml`层面表达 BST 有序性约束.

```toml
[states.data]
root = 0
nodes = [
  { id = 0, value = 10, left = 1, right = 2 },
  { id = 1, value = 5,  left = -1, right = -1 },
  { id = 2, value = 15, left = -1, right = -1 }
]
```

**约束**

| 字段/规则  | 约束(唯一确定)                    |
| ------ | --------------------------- |
| 顶层字段   | 必须包含且仅包含字段: `nodes`, `root` |
| 其余字段规则 | 与 `binary_tree` 完全一致        |

### `graph_undirected` (无向图)

```toml
[states.data]
nodes = [
  { id = 0, label = "A" },
  { id = 1, label = "B" }
]
edges = [
  { from = 0, to = 1 }
]
```

**约束**

| 字段/规则   | 约束(唯一确定)                                             |
| ------- | ---------------------------------------------------- |
| 顶层字段    | 必须包含且仅包含字段: `nodes`, `edges`                         |
| `nodes` | 必须为 array; 元素必须为 inline table, 且仅包含字段: `id`, `label` |
| `label` | 必须为 string; 长度 1..32; 不允许包含换行符                       |
| `edges` | 必须为 array; 元素必须为 inline table, 且仅包含字段: `from`, `to`  |
| 无向边规范化  | 每条边必须满足 `from < to`                                  |
| 多重边     | 同一 state 内不允许出现重复的 `(from,to)`                       |

### `graph_directed` (有向图)

```toml
[states.data]
nodes = [
  { id = 0, label = "A" },
  { id = 1, label = "B" }
]
edges = [
  { from = 0, to = 1 }
]
```

**约束**

| 字段/规则   | 约束(唯一确定)                                             |
| ------- | ---------------------------------------------------- |
| 顶层字段    | 必须包含且仅包含字段: `nodes`, `edges`                         |
| `nodes` | 必须为 array; 元素必须为 inline table, 且仅包含字段: `id`, `label` |
| `label` | 必须为 string; 长度 1..32; 不允许包含换行符                       |
| `edges` | 必须为 array; 元素必须为 inline table, 且仅包含字段: `from`, `to`  |
| 自环      | 每条边必须满足 `from != to`                                 |
| 多重边     | 同一 state 内不允许出现重复的 `(from,to)`                       |

### `graph_weighted` (带权图)

```toml
[states.data]
nodes = [
  { id = 0, label = "A" },
  { id = 1, label = "B" }
]
edges = [
  { from = 0, to = 1, weight = 3.2 }
]
```

**约束**

| 字段/规则    | 约束(唯一确定)                                                      |
| -------- | ------------------------------------------------------------- |
| 顶层字段     | 必须包含且仅包含字段: `nodes`, `edges`                                  |
| `nodes`  | 必须为 array; 元素必须为 inline table, 且仅包含字段: `id`, `label`          |
| `label`  | 必须为 string; 长度 1..32; 不允许包含换行符                                |
| `edges`  | 必须为 array; 元素必须为 inline table, 且仅包含字段: `from`, `to`, `weight` |
| 自环       | 每条边必须满足 `from != to`                                          |
| `weight` | 必须存在; 类型必须为 number                                            |
| 多重边      | 同一 state 内不允许出现重复的 `(from,to)`                                |

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
before = 0
after = 1
ret = 10
note = "optional"

[steps.args]
value = 10

[steps.code]
line = 5
col = 1
```

**约束**

| 字段                | 类型             | 是否必需                 | 约束(唯一确定)                                                                           |
| ----------------- | -------------- | --------------------:| ---------------------------------------------------------------------------------- |
| `steps`           | array of table | 否                    | 若存在, 数组长度 0..N                                                                     |
| `steps.id`        | integer        | 是                    | 必须为非负整数; 在整个文档内唯一; 且从 0 开始连续递增(步长为 1)                                              |
| `steps.op`        | string         | 是                    | 小写 ASCII 标识符, 仅允许字符集: `0-9`, `a-z`, `_`, `-`; 且长度 1..64                            |
| `steps.before`    | integer        | 是                    | 必须引用已存在的 `states.id`                                                               |
| `steps.after`     | integer        | 否                    | 若存在, 必须引用已存在的 `states.id`; 若不存在, 则文档必须存在 `[error]` 且 `error.step` 必须等于该 `steps.id` |
| `steps.args`      | table          | 是                    | 必须存在(允许为空 table); key 必须为小写 ASCII 标识符(字符集: `0-9`, `a-z`, `_`, `-`)                 |
| `steps.code`      | table          | 否                    | 若存在, 则必须包含字段 `line`; 且仅允许字段: `line`, `col`                                         |
| `steps.code.line` | integer        | 是(当 `steps.code` 存在) | 必须为正整数(从 1 开始)                                                                     |
| `steps.code.col`  | integer        | 否                    | 必须为正整数(从 1 开始)                                                                     |
| `steps.ret`       | any            | 否                    | 允许任意 TOML 值(标量/数组/table), 用于记录返回值                                                  |
| `steps.note`      | string         | 否                    | 长度 1..256; 不允许包含换行符                                                                |

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

| 字段                   | 类型      | 是否必需 | 约束(唯一确定)                                                |
| -------------------- | ------- | ----:| ------------------------------------------------------- |
| `result`             | table   | 否    | 若存在, 则文档不得存在 `[error]`                                  |
| `result.final_state` | integer | 是    | 必须引用已存在的 `states.id`                                    |
| `result.commit`      | table   | 否    | 若存在, 则必须包含且仅包含字段: `op`, `line`                          |
| `result.commit.op`   | string  | 是    | 小写 ASCII 标识符, 仅允许字符集: `0-9`, `a-z`, `_`, `-`; 且长度 1..64 |
| `result.commit.line` | integer | 是    | 必须为正整数(从 1 开始)                                          |

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

| 字段                 | 类型      | 是否必需 | 约束(唯一确定)                                                         |
| ------------------ | ------- | ----:| ---------------------------------------------------------------- |
| `error`            | table   | 否    | 若存在, 则文档不得存在 `[result]`                                          |
| `error.type`       | string  | 是    | 取值必须属于: `runtime`, `timeout`, `validation`, `sandbox`, `unknown` |
| `error.message`    | string  | 是    | 长度 1..512; 不允许包含换行符                                              |
| `error.line`       | integer | 否    | 必须为正整数(从 1 开始)                                                   |
| `error.step`       | integer | 否    | 若存在, 必须引用已存在的 `steps.id`                                         |
| `error.last_state` | integer | 否    | 若存在, 必须引用已存在的 `states.id`                                        |

---

## 文档级约束

| 约束项     | 约束(唯一确定)                              |
| ------- | ------------------------------------- |
| 顶层结局唯一性 | 文档必须且仅能出现 `[result]` 或 `[error]` 其中之一 |

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
title = "Stack demo"
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
before = 0
after = 1

[steps.args]
value = 10

[steps.code]
line = 6

[[steps]]
id = 1
op = "push"
before = 1
after = 2

[steps.args]
value = 20

[steps.code]
line = 7

[[steps]]
id = 2
op = "pop"
before = 2
after = 3
ret = 20

[steps.args]

[steps.code]
line = 8

[[steps]]
id = 3
op = "push"
before = 3
after = 4

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
title = "BinaryTree demo"
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
  { id = 0, value = 10, left = -1, right = -1 }
]

[[states]]
id = 2

[states.data]
root = 0
nodes = [
  { id = 0, value = 10, left = 1, right = -1 },
  { id = 1, value = 5,  left = -1, right = -1 }
]

[[states]]
id = 3

[states.data]
root = 0
nodes = [
  { id = 0, value = 10, left = 1, right = 2 },
  { id = 1, value = 5,  left = -1, right = -1 },
  { id = 2, value = 15, left = -1, right = -1 }
]

[[steps]]
id = 0
op = "insert_root"
before = 0
after = 1

[steps.args]
value = 10

[steps.code]
line = 12

[[steps]]
id = 1
op = "insert_left"
before = 1
after = 2

[steps.args]
parent = 10
value = 5

[steps.code]
line = 13

[[steps]]
id = 2
op = "insert_right"
before = 2
after = 3

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
title = "Weighted graph demo"
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
  { id = 0, label = "A" }
]
edges = []

[[states]]
id = 2

[states.data]
nodes = [
  { id = 0, label = "A" },
  { id = 1, label = "B" }
]
edges = []

[[states]]
id = 3

[states.data]
nodes = [
  { id = 0, label = "A" },
  { id = 1, label = "B" }
]
edges = [
  { from = 0, to = 1, weight = 3.2 }
]

[[states]]
id = 4

[states.data]
nodes = [
  { id = 0, label = "A" },
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
  { id = 0, label = "A" },
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
before = 0
after = 1

[steps.args]
id = 0
label = "A"

[steps.code]
line = 8

[[steps]]
id = 1
op = "add_node"
before = 1
after = 2

[steps.args]
id = 1
label = "B"

[steps.code]
line = 9

[[steps]]
id = 2
op = "add_edge"
before = 2
after = 3

[steps.args]
from = 0
to = 1
weight = 3.2

[steps.code]
line = 10

[[steps]]
id = 3
op = "add_node"
before = 3
after = 4

[steps.args]
id = 2
label = "C"

[steps.code]
line = 11

[[steps]]
id = 4
op = "add_edge"
before = 4
after = 5

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
