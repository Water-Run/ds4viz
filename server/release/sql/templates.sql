-- =====================================================
-- ds4viz 模板初始化数据
-- 30 个模板 × 3 种语言 (Python / Lua / Rust) = 90 条代码记录
-- 适用于 PostgreSQL
-- =====================================================

BEGIN;

-- 清理已有模板数据
TRUNCATE template_codes, templates RESTART IDENTITY CASCADE;

-- =====================================================
-- 分类: 栈
-- =====================================================

-- [1] 栈的基本操作
INSERT INTO templates (title, category, description) VALUES (
'栈的基本操作', '栈',
$D$栈(Stack)是一种后进先出(LIFO)的线性数据结构, 只允许在栈顶进行插入和删除操作。

本模板完整演示栈的三种基本操作: push(压栈)、pop(弹栈)和 clear(清空)。依次将多个元素压入栈中, 再按后进先出的顺序逐一弹出, 最后清空剩余元素, 展示栈从空到满再到空的全生命周期。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES (
currval('templates_id_seq'), 'python',
$C$import ds4viz as dv

with dv.stack(label="demo_stack") as s:
    # 依次压入 5 个元素
    s.push(1)
    s.push(2)
    s.push(3)
    s.push(4)
    s.push(5)
    # 按后进先出顺序逐一弹出
    s.pop()    # 弹出 5
    s.pop()    # 弹出 4
    s.pop()    # 弹出 3
    # 清空栈中剩余的 2 和 1
    s.clear()
$C$,
$E$### Python 实现

使用 `with` 上下文管理器创建栈实例, 确保无论执行是否成功都会生成 `.toml` 文件。

依次将 1~5 压入栈顶, 再按后进先出的顺序弹出 5、4、3, 最后 `clear()` 清空栈中剩余的 2 和 1。`push` 和 `pop` 的时间复杂度均为 O(1)。$E$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES (
currval('templates_id_seq'), 'lua',
$C$local ds4viz = require("ds4viz")

ds4viz.withContext(ds4viz.stack("demo_stack"), function(s)
    -- 依次压入 5 个元素
    s:push(1)
    s:push(2)
    s:push(3)
    s:push(4)
    s:push(5)
    -- 按后进先出顺序逐一弹出
    s:pop()    -- 弹出 5
    s:pop()    -- 弹出 4
    s:pop()    -- 弹出 3
    -- 清空栈中剩余的 2 和 1
    s:clear()
end)
$C$,
$E$### Lua 实现

使用 `withContext` 包裹操作, 模拟上下文管理。Lua 中通过冒号语法 `s:push()` 调用实例方法, 等价于 `s.push(s, ...)`。

`withContext` 返回 `boolean, any` 两个值, 分别表示执行是否成功以及可能的错误信息。$E$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES (
currval('templates_id_seq'), 'rust',
$C$use ds4viz::prelude::*;

fn main() -> ds4viz::Result<()> {
    ds4viz::stack("demo_stack", |s| {
        // 依次压入 5 个元素
        s.push(1)?;
        s.push(2)?;
        s.push(3)?;
        s.push(4)?;
        s.push(5)?;
        // 按后进先出顺序逐一弹出
        s.pop()?;    // 弹出 5
        s.pop()?;    // 弹出 4
        s.pop()?;    // 弹出 3
        // 清空栈中剩余的 2 和 1
        s.clear()?;
        Ok(())
    })
}
$C$,
$E$### Rust 实现

使用闭包上下文操作栈, 每个操作返回 `Result`, 通过 `?` 运算符传播错误。`push` 接受 `impl Into<Value>`, 支持 `i64`、`f64`、`&str`、`bool` 等类型。

对空栈执行 `pop` 会返回 `StructureError::EmptyStructure` 错误。闭包最后需要返回 `Ok(())`。$E$
);

-- [2] 括号匹配检验
INSERT INTO templates (title, category, description) VALUES (
'括号匹配检验', '栈',
$D$括号匹配是栈的经典应用之一。给定一个包含 ()、{}、[] 的字符串, 利用栈来检验括号是否合法配对。

算法思路: 遍历字符串, 遇到左括号压栈, 遇到右括号弹栈。若整个过程中栈操作合法且最终栈为空, 则括号匹配成功。本模板使用括号串 "({[]})[({})]" 进行演示, 该串包含嵌套和并列两种匹配模式。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES (
currval('templates_id_seq'), 'python',
$C$import ds4viz as dv

# 检验括号串是否合法
brackets = "({[]})[({})]"

with dv.stack(label="bracket_check") as s:
    for ch in brackets:
        if ch in "({[":
            s.push(ch)
        elif ch in ")}]":
            s.pop()
    # 遍历结束后栈为空, 说明括号完全匹配
$C$,
$E$### Python 实现

遍历字符串中的每个字符, 左括号执行 `push`, 右括号执行 `pop`。

括号串 `"({[]})[({})]"` 包含两段独立的合法嵌套: 前半段 `({[]})` 和后半段 `[({})]`。可视化清晰展示了栈在匹配过程中的两次"涨落"周期。$E$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES (
currval('templates_id_seq'), 'lua',
$C$local ds4viz = require("ds4viz")

-- 检验括号串是否合法
local brackets = "({[]})[({})]"

ds4viz.withContext(ds4viz.stack("bracket_check"), function(s)
    for i = 1, #brackets do
        local ch = brackets:sub(i, i)
        if ch == "(" or ch == "{" or ch == "[" then
            s:push(ch)
        elseif ch == ")" or ch == "}" or ch == "]" then
            s:pop()
        end
    end
    -- 遍历结束后栈为空, 说明括号完全匹配
end)
$C$,
$E$### Lua 实现

Lua 没有内置的字符迭代器, 因此使用 `string.sub(i, i)` 逐字符遍历。左括号通过字符比较判断后压栈, 右括号弹栈。$E$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES (
currval('templates_id_seq'), 'rust',
$C$use ds4viz::prelude::*;

fn main() -> ds4viz::Result<()> {
    // 检验括号串是否合法
    let brackets = "({[]})[({})]";

    ds4viz::stack("bracket_check", |s| {
        for ch in brackets.chars() {
            match ch {
                '(' | '{' | '[' => s.push(ch.to_string())?,
                ')' | '}' | ']' => s.pop()?,
                _ => {}
            }
        }
        // 遍历结束后栈为空, 说明括号完全匹配
        Ok(())
    })
}
$C$,
$E$### Rust 实现

使用 `chars()` 迭代字符, `match` 表达式匹配括号类型。`push` 接受 `impl Into<Value>`, 此处将 `char` 通过 `to_string()` 转为 `String` 后压栈。$E$
);

-- [3] 十进制转二进制
INSERT INTO templates (title, category, description) VALUES (
'十进制转二进制', '栈',
$D$利用栈实现十进制整数到二进制的转换, 是栈的经典应用场景。

算法思路: 反复对目标数取模 2 并将余数压栈, 然后整除 2, 直到商为 0。最后依次弹出栈中元素, 即为二进制表示(高位在前)。本模板将十进制数 42 转换为二进制 101010。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES (
currval('templates_id_seq'), 'python',
$C$import ds4viz as dv

# 将十进制数 42 转换为二进制: 101010
number = 42

with dv.stack(label="dec_to_bin") as s:
    n = number
    digits = 0
    while n > 0:
        s.push(n % 2)
        n = n // 2
        digits += 1
    # 依次弹出, 从高位到低位: 1, 0, 1, 0, 1, 0
    for _ in range(digits):
        s.pop()
$C$,
$E$### Python 实现

变量 `digits` 记录压栈次数, 便于后续逐一弹出。42 的二进制为 101010, 压栈顺序为 0、1、0、1、0、1(低位先入), 弹栈时自然得到高位在前的正确顺序。$E$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES (
currval('templates_id_seq'), 'lua',
$C$local ds4viz = require("ds4viz")

-- 将十进制数 42 转换为二进制: 101010
local number = 42

ds4viz.withContext(ds4viz.stack("dec_to_bin"), function(s)
    local n = number
    local digits = 0
    while n > 0 do
        s:push(n % 2)
        n = math.floor(n / 2)
        digits = digits + 1
    end
    -- 依次弹出, 从高位到低位: 1, 0, 1, 0, 1, 0
    for _ = 1, digits do
        s:pop()
    end
end)
$C$,
$E$### Lua 实现

Lua 中整除需使用 `math.floor(n / 2)`。数值循环使用 `while ... do ... end` 语法, 弹栈使用 `for` 计数循环。$E$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES (
currval('templates_id_seq'), 'rust',
$C$use ds4viz::prelude::*;

fn main() -> ds4viz::Result<()> {
    // 将十进制数 42 转换为二进制: 101010
    let number: i64 = 42;

    ds4viz::stack("dec_to_bin", |s| {
        let mut n = number;
        let mut digits = 0;
        while n > 0 {
            s.push(n % 2)?;
            n /= 2;
            digits += 1;
        }
        // 依次弹出, 从高位到低位: 1, 0, 1, 0, 1, 0
        for _ in 0..digits {
            s.pop()?;
        }
        Ok(())
    })
}
$C$,
$E$### Rust 实现

使用 `mut` 声明可变变量。Rust 的整数除法 `/=` 默认为整除, 循环使用 `for _ in 0..n` 范围表达式。$E$
);

-- =====================================================
-- 分类: 队列
-- =====================================================

-- [4] 队列的基本操作
INSERT INTO templates (title, category, description) VALUES (
'队列的基本操作', '队列',
$D$队列(Queue)是一种先进先出(FIFO)的线性数据结构, 元素从队尾入队、从队头出队。

本模板完整演示队列的三种基本操作: enqueue(入队)、dequeue(出队)和 clear(清空)。依次将多个元素入队, 再按先进先出的顺序逐一出队, 最后清空剩余元素, 直观展示 FIFO 特性。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES (
currval('templates_id_seq'), 'python',
$C$import ds4viz as dv

with dv.queue(label="demo_queue") as q:
    # 依次入队 5 个元素
    q.enqueue(10)
    q.enqueue(20)
    q.enqueue(30)
    q.enqueue(40)
    q.enqueue(50)
    # 按先进先出顺序逐一出队
    q.dequeue()  # 出队 10
    q.dequeue()  # 出队 20
    q.dequeue()  # 出队 30
    # 清空队列中剩余的 40 和 50
    q.clear()
$C$,
$E$### Python 实现

依次将 10~50 入队, 然后按先进先出顺序出队。最先入队的 10 最先出队, 最后 `clear()` 清空队列中剩余的 40 和 50。$E$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES (
currval('templates_id_seq'), 'lua',
$C$local ds4viz = require("ds4viz")

ds4viz.withContext(ds4viz.queue("demo_queue"), function(q)
    -- 依次入队 5 个元素
    q:enqueue(10)
    q:enqueue(20)
    q:enqueue(30)
    q:enqueue(40)
    q:enqueue(50)
    -- 按先进先出顺序逐一出队
    q:dequeue()  -- 出队 10
    q:dequeue()  -- 出队 20
    q:dequeue()  -- 出队 30
    -- 清空队列中剩余的 40 和 50
    q:clear()
end)
$C$,
$E$### Lua 实现

使用 `withContext` 管理队列生命周期。队列的 `enqueue` 和 `dequeue` 操作通过冒号语法调用。$E$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES (
currval('templates_id_seq'), 'rust',
$C$use ds4viz::prelude::*;

fn main() -> ds4viz::Result<()> {
    ds4viz::queue("demo_queue", |q| {
        // 依次入队 5 个元素
        q.enqueue(10)?;
        q.enqueue(20)?;
        q.enqueue(30)?;
        q.enqueue(40)?;
        q.enqueue(50)?;
        // 按先进先出顺序逐一出队
        q.dequeue()?;  // 出队 10
        q.dequeue()?;  // 出队 20
        q.dequeue()?;  // 出队 30
        // 清空队列中剩余的 40 和 50
        q.clear()?;
        Ok(())
    })
}
$C$,
$E$### Rust 实现

闭包接收队列的可变引用, 每个操作通过 `?` 传播可能的错误。对空队列执行 `dequeue` 会返回 `StructureError::EmptyStructure`。$E$
);

-- [5] 约瑟夫环问题
INSERT INTO templates (title, category, description) VALUES (
'约瑟夫环问题', '队列',
$D$约瑟夫环(Josephus Problem)是经典的数学与计算机科学问题: N 个人围成一圈, 从第一个人开始报数, 每报到第 K 个人将其淘汰, 然后从下一个人重新报数, 直到只剩一人。

本模板使用队列模拟约瑟夫环: 未被淘汰者出队后重新入队到队尾, 被淘汰者仅出队。此处 N=6, K=3, 淘汰顺序为 3、6、4、2、5, 最终幸存者为 1 号。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES (
currval('templates_id_seq'), 'python',
$C$import ds4viz as dv

# 约瑟夫环: 6人, 每报到3淘汰
with dv.queue(label="josephus") as q:
    # 6人入队
    q.enqueue(1)
    q.enqueue(2)
    q.enqueue(3)
    q.enqueue(4)
    q.enqueue(5)
    q.enqueue(6)

    # 第1轮: 跳过1, 跳过2, 淘汰3
    q.dequeue()
    q.enqueue(1)
    q.dequeue()
    q.enqueue(2)
    q.dequeue()    # 3 被淘汰

    # 第2轮: 跳过4, 跳过5, 淘汰6
    q.dequeue()
    q.enqueue(4)
    q.dequeue()
    q.enqueue(5)
    q.dequeue()    # 6 被淘汰

    # 第3轮: 跳过1, 跳过2, 淘汰4
    q.dequeue()
    q.enqueue(1)
    q.dequeue()
    q.enqueue(2)
    q.dequeue()    # 4 被淘汰

    # 第4轮: 跳过5, 跳过1, 淘汰2
    q.dequeue()
    q.enqueue(5)
    q.dequeue()
    q.enqueue(1)
    q.dequeue()    # 2 被淘汰

    # 第5轮: 跳过5, 跳过1, 淘汰5
    q.dequeue()
    q.enqueue(5)
    q.dequeue()
    q.enqueue(1)
    q.dequeue()    # 5 被淘汰

    # 1号是最终幸存者
    q.dequeue()
$C$,
$E$### Python 实现

每轮中, 非淘汰者执行 `dequeue` 后立即 `enqueue` 回队尾, 模拟"跳过"; 被淘汰者仅执行 `dequeue`。

6 人报数 K=3 共需 5 轮淘汰, 淘汰顺序为 3→6→4→2→5, 最终 1 号幸存。可视化清晰展示了每轮队列的变化过程。$E$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES (
currval('templates_id_seq'), 'lua',
$C$local ds4viz = require("ds4viz")

-- 约瑟夫环: 6人, 每报到3淘汰
ds4viz.withContext(ds4viz.queue("josephus"), function(q)
    -- 6人入队
    q:enqueue(1)
    q:enqueue(2)
    q:enqueue(3)
    q:enqueue(4)
    q:enqueue(5)
    q:enqueue(6)

    -- 第1轮: 跳过1, 跳过2, 淘汰3
    q:dequeue()
    q:enqueue(1)
    q:dequeue()
    q:enqueue(2)
    q:dequeue()

    -- 第2轮: 跳过4, 跳过5, 淘汰6
    q:dequeue()
    q:enqueue(4)
    q:dequeue()
    q:enqueue(5)
    q:dequeue()

    -- 第3轮: 跳过1, 跳过2, 淘汰4
    q:dequeue()
    q:enqueue(1)
    q:dequeue()
    q:enqueue(2)
    q:dequeue()

    -- 第4轮: 跳过5, 跳过1, 淘汰2
    q:dequeue()
    q:enqueue(5)
    q:dequeue()
    q:enqueue(1)
    q:dequeue()

    -- 第5轮: 跳过5, 跳过1, 淘汰5
    q:dequeue()
    q:enqueue(5)
    q:dequeue()
    q:enqueue(1)
    q:dequeue()

    -- 1号是最终幸存者
    q:dequeue()
end)
$C$,
$E$### Lua 实现

与 Python 版本逻辑一致, 通过冒号语法调用队列方法。每轮的"跳过"操作体现为 `dequeue` + `enqueue` 的组合。$E$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES (
currval('templates_id_seq'), 'rust',
$C$use ds4viz::prelude::*;

fn main() -> ds4viz::Result<()> {
    // 约瑟夫环: 6人, 每报到3淘汰
    ds4viz::queue("josephus", |q| {
        // 6人入队
        q.enqueue(1)?;
        q.enqueue(2)?;
        q.enqueue(3)?;
        q.enqueue(4)?;
        q.enqueue(5)?;
        q.enqueue(6)?;

        // 第1轮: 跳过1, 跳过2, 淘汰3
        q.dequeue()?;
        q.enqueue(1)?;
        q.dequeue()?;
        q.enqueue(2)?;
        q.dequeue()?;

        // 第2轮: 跳过4, 跳过5, 淘汰6
        q.dequeue()?;
        q.enqueue(4)?;
        q.dequeue()?;
        q.enqueue(5)?;
        q.dequeue()?;

        // 第3轮: 跳过1, 跳过2, 淘汰4
        q.dequeue()?;
        q.enqueue(1)?;
        q.dequeue()?;
        q.enqueue(2)?;
        q.dequeue()?;

        // 第4轮: 跳过5, 跳过1, 淘汰2
        q.dequeue()?;
        q.enqueue(5)?;
        q.dequeue()?;
        q.enqueue(1)?;
        q.dequeue()?;

        // 第5轮: 跳过5, 跳过1, 淘汰5
        q.dequeue()?;
        q.enqueue(5)?;
        q.dequeue()?;
        q.enqueue(1)?;
        q.dequeue()?;

        // 1号是最终幸存者
        q.dequeue()?;
        Ok(())
    })
}
$C$,
$E$### Rust 实现

Rust 版本通过 `?` 运算符串联所有操作。若任何步骤中队列为空(不应发生), 会自动传播 `StructureError::EmptyStructure` 错误。$E$
);

-- [6] 打印任务队列
INSERT INTO templates (title, category, description) VALUES (
'打印任务队列', '队列',
$D$模拟打印机的任务调度场景。多个打印任务按到达顺序入队, 打印机按先到先服务(FCFS)的策略依次处理。

在处理过程中可能有新任务到达并加入队列尾部, 体现了队列在任务调度中的典型应用。本模板模拟 5 个文件的打印过程, 其中有 2 个文件在处理途中到达。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES (
currval('templates_id_seq'), 'python',
$C$import ds4viz as dv

with dv.queue(label="print_queue") as q:
    # 三个任务依次到达
    q.enqueue("report.pdf")
    q.enqueue("photo.png")
    q.enqueue("resume.docx")
    # 开始处理
    q.dequeue()              # 打印 report.pdf
    q.enqueue("slides.pptx") # 新任务到达
    q.dequeue()              # 打印 photo.png
    q.enqueue("chart.xlsx")  # 新任务到达
    q.dequeue()              # 打印 resume.docx
    q.dequeue()              # 打印 slides.pptx
    q.dequeue()              # 打印 chart.xlsx
$C$,
$E$### Python 实现

用字符串表示文件名, 模拟打印任务的入队和出队。中途新任务 `slides.pptx` 和 `chart.xlsx` 加入队尾, 但仍然排在已有任务之后处理, 体现了 FIFO 原则。$E$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES (
currval('templates_id_seq'), 'lua',
$C$local ds4viz = require("ds4viz")

ds4viz.withContext(ds4viz.queue("print_queue"), function(q)
    -- 三个任务依次到达
    q:enqueue("report.pdf")
    q:enqueue("photo.png")
    q:enqueue("resume.docx")
    -- 开始处理
    q:dequeue()              -- 打印 report.pdf
    q:enqueue("slides.pptx") -- 新任务到达
    q:dequeue()              -- 打印 photo.png
    q:enqueue("chart.xlsx")  -- 新任务到达
    q:dequeue()              -- 打印 resume.docx
    q:dequeue()              -- 打印 slides.pptx
    q:dequeue()              -- 打印 chart.xlsx
end)
$C$,
$E$### Lua 实现

Lua 中字符串使用双引号, 与 Python 版本逻辑完全一致。队列支持任意类型的值, 包括字符串。$E$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES (
currval('templates_id_seq'), 'rust',
$C$use ds4viz::prelude::*;

fn main() -> ds4viz::Result<()> {
    ds4viz::queue("print_queue", |q| {
        // 三个任务依次到达
        q.enqueue("report.pdf")?;
        q.enqueue("photo.png")?;
        q.enqueue("resume.docx")?;
        // 开始处理
        q.dequeue()?;               // 打印 report.pdf
        q.enqueue("slides.pptx")?;  // 新任务到达
        q.dequeue()?;               // 打印 photo.png
        q.enqueue("chart.xlsx")?;   // 新任务到达
        q.dequeue()?;               // 打印 resume.docx
        q.dequeue()?;               // 打印 slides.pptx
        q.dequeue()?;               // 打印 chart.xlsx
        Ok(())
    })
}
$C$,
$E$### Rust 实现

`enqueue` 接受 `impl Into<Value>`, 字符串字面量 `&str` 会自动转换为 `Value::String`。$E$
);

-- =====================================================
-- 分类: 单链表
-- =====================================================

-- [7] 单链表的构建
INSERT INTO templates (title, category, description) VALUES (
'单链表的构建', '单链表',
$D$单链表(Singly Linked List)由节点组成, 每个节点包含数据和指向下一个节点的指针。

本模板演示通过头插法和尾插法混合构建单链表的过程。头插法将新节点插入链表头部, 已有节点依次后移; 尾插法将新节点追加到链表末尾。两种方式对节点排列顺序的影响截然不同。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES (
currval('templates_id_seq'), 'python',
$C$import ds4viz as dv

with dv.single_linked_list(label="build_slist") as slist:
    # 头插法: 每次插入到链表头部
    slist.insert_head(30)   # 链表: 30
    slist.insert_head(20)   # 链表: 20 -> 30
    slist.insert_head(10)   # 链表: 10 -> 20 -> 30
    # 尾插法: 每次追加到链表末尾
    slist.insert_tail(40)   # 链表: 10 -> 20 -> 30 -> 40
    slist.insert_tail(50)   # 链表: 10 -> 20 -> 30 -> 40 -> 50
    slist.insert_tail(60)   # 链表: 10 -> 20 -> 30 -> 40 -> 50 -> 60
$C$,
$E$### Python 实现

`insert_head` 和 `insert_tail` 都返回新节点的 ID(整数), 后续操作可通过该 ID 引用节点。头插法的顺序为 30→20→10, 结合尾插法后最终构建出有序链表 10→20→30→40→50→60。$E$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES (
currval('templates_id_seq'), 'lua',
$C$local ds4viz = require("ds4viz")

ds4viz.withContext(ds4viz.singleLinkedList("build_slist"), function(slist)
    -- 头插法: 每次插入到链表头部
    slist:insertHead(30)
    slist:insertHead(20)
    slist:insertHead(10)
    -- 尾插法: 每次追加到链表末尾
    slist:insertTail(40)
    slist:insertTail(50)
    slist:insertTail(60)
end)
$C$,
$E$### Lua 实现

Lua 中方法名使用驼峰命名(`insertHead`、`insertTail`), 与 Python 的下划线命名风格不同。返回值为节点 ID(number 类型)。$E$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES (
currval('templates_id_seq'), 'rust',
$C$use ds4viz::prelude::*;

fn main() -> ds4viz::Result<()> {
    ds4viz::single_linked_list("build_slist", |slist| {
        // 头插法: 每次插入到链表头部
        slist.insert_head(30)?;
        slist.insert_head(20)?;
        slist.insert_head(10)?;
        // 尾插法: 每次追加到链表末尾
        slist.insert_tail(40)?;
        slist.insert_tail(50)?;
        slist.insert_tail(60)?;
        Ok(())
    })
}
$C$,
$E$### Rust 实现

`insert_head` 和 `insert_tail` 返回 `Result<i64>`, 其中 `i64` 是新节点的 ID。此处用 `?` 忽略返回的节点 ID。$E$
);

-- [8] 单链表的插入与删除
INSERT INTO templates (title, category, description) VALUES (
'单链表的插入与删除', '单链表',
$D$在单链表的指定位置插入或删除节点是链表操作的核心技能。由于单链表只能单向遍历, 在某个节点之后插入较为高效, 但删除操作需要知道前驱节点。

本模板综合演示 insert_head、insert_tail、insert_after(指定节点后插入)、delete(删除指定节点)和 delete_head(删除头节点)五种操作, 展示单链表的完整动态修改能力。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES (
currval('templates_id_seq'), 'python',
$C$import ds4viz as dv

with dv.single_linked_list(label="modify_slist") as slist:
    # 构建初始链表
    n1 = slist.insert_head(10)     # 10
    n2 = slist.insert_tail(20)     # 10 -> 20
    n3 = slist.insert_tail(40)     # 10 -> 20 -> 40
    n4 = slist.insert_tail(50)     # 10 -> 20 -> 40 -> 50

    # 在节点20之后插入30
    slist.insert_after(n2, 30)     # 10 -> 20 -> 30 -> 40 -> 50

    # 删除节点40
    slist.delete(n3)               # 10 -> 20 -> 30 -> 50

    # 删除头节点
    slist.delete_head()            # 20 -> 30 -> 50

    # 在节点50之后插入60
    slist.insert_after(n4, 60)     # 20 -> 30 -> 50 -> 60
$C$,
$E$### Python 实现

`insert_head` 和 `insert_tail` 返回新节点的 ID, 保存在变量中以便后续引用。`insert_after(n2, 30)` 在节点 20 之后插入 30, `delete(n3)` 删除值为 40 的节点, `delete_head()` 删除当前头节点。$E$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES (
currval('templates_id_seq'), 'lua',
$C$local ds4viz = require("ds4viz")

ds4viz.withContext(ds4viz.singleLinkedList("modify_slist"), function(slist)
    -- 构建初始链表
    local n1 = slist:insertHead(10)
    local n2 = slist:insertTail(20)
    local n3 = slist:insertTail(40)
    local n4 = slist:insertTail(50)

    -- 在节点20之后插入30
    slist:insertAfter(n2, 30)

    -- 删除节点40
    slist:delete(n3)

    -- 删除头节点
    slist:deleteHead()

    -- 在节点50之后插入60
    slist:insertAfter(n4, 60)
end)
$C$,
$E$### Lua 实现

插入方法返回节点 ID(number 类型), 保存在局部变量中以便后续引用。`deleteHead` 删除链表的第一个节点。$E$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES (
currval('templates_id_seq'), 'rust',
$C$use ds4viz::prelude::*;

fn main() -> ds4viz::Result<()> {
    ds4viz::single_linked_list("modify_slist", |slist| {
        // 构建初始链表
        let _n1 = slist.insert_head(10)?;
        let n2 = slist.insert_tail(20)?;
        let n3 = slist.insert_tail(40)?;
        let n4 = slist.insert_tail(50)?;

        // 在节点20之后插入30
        slist.insert_after(n2, 30)?;

        // 删除节点40
        slist.delete(n3)?;

        // 删除头节点
        slist.delete_head()?;

        // 在节点50之后插入60
        slist.insert_after(n4, 60)?;
        Ok(())
    })
}
$C$,
$E$### Rust 实现

使用 `let` 绑定节点 ID。未使用的变量用 `_n1` 前缀下划线避免编译器警告。删除不存在的节点会返回 `StructureError::NodeNotFound`。$E$
);

-- [9] 单链表反转
INSERT INTO templates (title, category, description) VALUES (
'单链表反转', '单链表',
$D$链表反转是面试和算法竞赛中的高频题目。反转操作将链表中所有节点的指针方向逆转, 使原来的尾节点变为头节点。

本模板先构建一个 1->2->3->4->5 的链表, 然后执行 reverse 操作, 观察指针方向的变化。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES (
currval('templates_id_seq'), 'python',
$C$import ds4viz as dv

with dv.single_linked_list(label="reverse_slist") as slist:
    # 使用尾插法构建有序链表
    slist.insert_tail(1)
    slist.insert_tail(2)
    slist.insert_tail(3)
    slist.insert_tail(4)
    slist.insert_tail(5)
    # 链表: 1 -> 2 -> 3 -> 4 -> 5

    # 反转链表
    slist.reverse()
    # 反转后: 5 -> 4 -> 3 -> 2 -> 1
$C$,
$E$### Python 实现

先使用 `insert_tail` 按序构建链表, 然后调用 `reverse()` 一步完成反转。可视化将展示反转前后节点链接关系的变化。$E$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES (
currval('templates_id_seq'), 'lua',
$C$local ds4viz = require("ds4viz")

ds4viz.withContext(ds4viz.singleLinkedList("reverse_slist"), function(slist)
    -- 使用尾插法构建有序链表
    slist:insertTail(1)
    slist:insertTail(2)
    slist:insertTail(3)
    slist:insertTail(4)
    slist:insertTail(5)

    -- 反转链表
    slist:reverse()
end)
$C$,
$E$### Lua 实现

调用 `reverse()` 后, 链表内部的指针方向全部逆转。可视化会展示每个步骤的状态变化。$E$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES (
currval('templates_id_seq'), 'rust',
$C$use ds4viz::prelude::*;

fn main() -> ds4viz::Result<()> {
    ds4viz::single_linked_list("reverse_slist", |slist| {
        // 使用尾插法构建有序链表
        slist.insert_tail(1)?;
        slist.insert_tail(2)?;
        slist.insert_tail(3)?;
        slist.insert_tail(4)?;
        slist.insert_tail(5)?;

        // 反转链表
        slist.reverse()?;
        Ok(())
    })
}
$C$,
$E$### Rust 实现

`reverse()` 返回 `Result<()>`, 反转操作在库内部完成, 所有中间状态均被记录到 trace 中。$E$
);

-- =====================================================
-- 分类: 双向链表
-- =====================================================

-- [10] 双向链表的基本操作
INSERT INTO templates (title, category, description) VALUES (
'双向链表的基本操作', '双向链表',
$D$双向链表(Doubly Linked List)的每个节点同时持有前驱和后继指针, 支持双向遍历。

本模板综合演示头插、尾插、前插、后插四种插入方式以及按节点删除、删除头节点、删除尾节点三种删除方式, 完整覆盖双向链表的全部七种基本操作。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES (
currval('templates_id_seq'), 'python',
$C$import ds4viz as dv

with dv.double_linked_list(label="demo_dlist") as dlist:
    # 头插和尾插构建初始链表
    n1 = dlist.insert_head(30)     # 30
    n2 = dlist.insert_head(10)     # 10 <-> 30
    n3 = dlist.insert_tail(50)     # 10 <-> 30 <-> 50

    # 利用 insert_after 和 insert_before 精确插入
    dlist.insert_after(n1, 40)     # 10 <-> 30 <-> 40 <-> 50
    dlist.insert_before(n1, 20)    # 10 <-> 20 <-> 30 <-> 40 <-> 50

    # 三种删除方式
    dlist.delete(n1)               # 删除30: 10 <-> 20 <-> 40 <-> 50
    dlist.delete_head()            # 删除10: 20 <-> 40 <-> 50
    dlist.delete_tail()            # 删除50: 20 <-> 40
$C$,
$E$### Python 实现

`insert_before` 和 `insert_after` 利用双向链表的前驱指针实现高效的任意位置插入。三种删除方式分别适用于不同场景: `delete(node_id)` 精确删除、`delete_head()` 删除头部、`delete_tail()` 删除尾部, 时间复杂度均为 O(1)。$E$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES (
currval('templates_id_seq'), 'lua',
$C$local ds4viz = require("ds4viz")

ds4viz.withContext(ds4viz.doubleLinkedList("demo_dlist"), function(dlist)
    -- 头插和尾插构建初始链表
    local n1 = dlist:insertHead(30)
    local n2 = dlist:insertHead(10)
    local n3 = dlist:insertTail(50)

    -- 利用 insertAfter 和 insertBefore 精确插入
    dlist:insertAfter(n1, 40)
    dlist:insertBefore(n1, 20)

    -- 三种删除方式
    dlist:delete(n1)
    dlist:deleteHead()
    dlist:deleteTail()
end)
$C$,
$E$### Lua 实现

`insertBefore` 和 `insertAfter` 是双向链表独有的操作, 在单链表中仅支持 `insertAfter`。`deleteHead` 和 `deleteTail` 从两端删除节点。$E$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES (
currval('templates_id_seq'), 'rust',
$C$use ds4viz::prelude::*;

fn main() -> ds4viz::Result<()> {
    ds4viz::double_linked_list("demo_dlist", |dlist| {
        // 头插和尾插构建初始链表
        let n1 = dlist.insert_head(30)?;
        let _n2 = dlist.insert_head(10)?;
        let _n3 = dlist.insert_tail(50)?;

        // 利用 insert_after 和 insert_before 精确插入
        dlist.insert_after(n1, 40)?;
        dlist.insert_before(n1, 20)?;

        // 三种删除方式
        dlist.delete(n1)?;
        dlist.delete_head()?;
        dlist.delete_tail()?;
        Ok(())
    })
}
$C$,
$E$### Rust 实现

`insert_before` 和 `insert_after` 都接受目标节点 ID 和新值, 返回新节点 ID。空链表执行 `delete_head` 或 `delete_tail` 会返回 `StructureError::EmptyStructure`。$E$
);

-- [11] 双向链表反转
INSERT INTO templates (title, category, description) VALUES (
'双向链表反转', '双向链表',
$D$双向链表的反转需要交换每个节点的前驱和后继指针, 同时交换头尾指针。

本模板构建一个 1<->2<->3<->4<->5 的双向链表后执行 reverse 操作, 观察节点间双向链接关系的变化。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES (
currval('templates_id_seq'), 'python',
$C$import ds4viz as dv

with dv.double_linked_list(label="reverse_dlist") as dlist:
    dlist.insert_tail(1)
    dlist.insert_tail(2)
    dlist.insert_tail(3)
    dlist.insert_tail(4)
    dlist.insert_tail(5)
    # 链表: 1 <-> 2 <-> 3 <-> 4 <-> 5

    dlist.reverse()
    # 反转后: 5 <-> 4 <-> 3 <-> 2 <-> 1
$C$,
$E$### Python 实现

`reverse()` 将所有节点的前驱和后继指针互换。可视化展示反转前后双向指针的变化。$E$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES (
currval('templates_id_seq'), 'lua',
$C$local ds4viz = require("ds4viz")

ds4viz.withContext(ds4viz.doubleLinkedList("reverse_dlist"), function(dlist)
    dlist:insertTail(1)
    dlist:insertTail(2)
    dlist:insertTail(3)
    dlist:insertTail(4)
    dlist:insertTail(5)

    dlist:reverse()
end)
$C$,
$E$### Lua 实现

构建链表后一步调用 `reverse()` 完成反转。与单链表反转的区别在于每个节点有两个指针需要交换。$E$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES (
currval('templates_id_seq'), 'rust',
$C$use ds4viz::prelude::*;

fn main() -> ds4viz::Result<()> {
    ds4viz::double_linked_list("reverse_dlist", |dlist| {
        dlist.insert_tail(1)?;
        dlist.insert_tail(2)?;
        dlist.insert_tail(3)?;
        dlist.insert_tail(4)?;
        dlist.insert_tail(5)?;

        dlist.reverse()?;
        Ok(())
    })
}
$C$,
$E$### Rust 实现

与单链表反转 API 一致, 调用 `reverse()` 即可。库内部会自动处理前驱和后继指针的交换。$E$
);

-- [12] 双端队列模拟
INSERT INTO templates (title, category, description) VALUES (
'双端队列模拟', '双向链表',
$D$双端队列(Deque)允许在两端进行插入和删除操作。双向链表天然支持头尾双端操作, 因此是实现双端队列的理想数据结构。

本模板用双向链表模拟双端队列, 演示从两端入队和出队的过程, 所有操作的时间复杂度均为 O(1)。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES (
currval('templates_id_seq'), 'python',
$C$import ds4viz as dv

# 用双向链表模拟双端队列
with dv.double_linked_list(label="deque_sim") as dlist:
    # 从尾部入队
    dlist.insert_tail(1)
    dlist.insert_tail(2)
    dlist.insert_tail(3)
    # 从头部入队
    dlist.insert_head(0)
    # 双端队列: 0 <-> 1 <-> 2 <-> 3

    dlist.delete_head()   # 头部出队: 0
    dlist.delete_tail()   # 尾部出队: 3
    dlist.insert_tail(4)  # 尾部入队: 4
    dlist.insert_head(-1) # 头部入队: -1
    dlist.delete_head()   # 头部出队: -1
    dlist.delete_tail()   # 尾部出队: 4
$C$,
$E$### Python 实现

`insert_head`/`delete_head` 对应双端队列的前端操作, `insert_tail`/`delete_tail` 对应后端操作。两端操作的时间复杂度均为 O(1)。$E$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES (
currval('templates_id_seq'), 'lua',
$C$local ds4viz = require("ds4viz")

ds4viz.withContext(ds4viz.doubleLinkedList("deque_sim"), function(dlist)
    -- 从尾部入队
    dlist:insertTail(1)
    dlist:insertTail(2)
    dlist:insertTail(3)
    -- 从头部入队
    dlist:insertHead(0)

    dlist:deleteHead()   -- 头部出队: 0
    dlist:deleteTail()   -- 尾部出队: 3
    dlist:insertTail(4)  -- 尾部入队: 4
    dlist:insertHead(-1) -- 头部入队: -1
    dlist:deleteHead()   -- 头部出队: -1
    dlist:deleteTail()   -- 尾部出队: 4
end)
$C$,
$E$### Lua 实现

双向链表提供 `deleteHead` 和 `deleteTail` 两个方法, 分别从两端删除节点, 天然适合实现双端队列。$E$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES (
currval('templates_id_seq'), 'rust',
$C$use ds4viz::prelude::*;

fn main() -> ds4viz::Result<()> {
    ds4viz::double_linked_list("deque_sim", |dlist| {
        // 从尾部入队
        dlist.insert_tail(1)?;
        dlist.insert_tail(2)?;
        dlist.insert_tail(3)?;
        // 从头部入队
        dlist.insert_head(0)?;

        dlist.delete_head()?;   // 头部出队: 0
        dlist.delete_tail()?;   // 尾部出队: 3
        dlist.insert_tail(4)?;  // 尾部入队: 4
        dlist.insert_head(-1)?; // 头部入队: -1
        dlist.delete_head()?;   // 头部出队: -1
        dlist.delete_tail()?;   // 尾部出队: 4
        Ok(())
    })
}
$C$,
$E$### Rust 实现

`delete_head` 和 `delete_tail` 在链表为空时返回 `StructureError::EmptyStructure` 错误。$E$
);

-- =====================================================
-- 分类: 二叉树
-- =====================================================

-- [13] 二叉树的手动构建
INSERT INTO templates (title, category, description) VALUES (
'二叉树的手动构建', '二叉树',
$D$二叉树(Binary Tree)是每个节点最多有两个子节点的树结构, 分别称为左子节点和右子节点。

本模板从根节点出发, 逐步通过 insert_left 和 insert_right 手动构建一棵三层满二叉树, 共 7 个节点, 节点值为 1~7 的层序编号。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES (
currval('templates_id_seq'), 'python',
$C$import ds4viz as dv

with dv.binary_tree(label="manual_tree") as tree:
    # 第1层: 根节点
    root = tree.insert_root(1)
    # 第2层
    l = tree.insert_left(root, 2)
    r = tree.insert_right(root, 3)
    # 第3层
    tree.insert_left(l, 4)
    tree.insert_right(l, 5)
    tree.insert_left(r, 6)
    tree.insert_right(r, 7)
    #         1
    #        / \
    #       2   3
    #      / \ / \
    #     4  5 6  7
$C$,
$E$### Python 实现

`insert_root` 创建根节点并返回其 ID, 后续通过父节点 ID 调用 `insert_left` 和 `insert_right` 插入子节点。每个插入操作都返回新节点的 ID, 可用于进一步扩展子树。$E$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES (
currval('templates_id_seq'), 'lua',
$C$local ds4viz = require("ds4viz")

ds4viz.withContext(ds4viz.binaryTree("manual_tree"), function(tree)
    -- 第1层: 根节点
    local root = tree:insertRoot(1)
    -- 第2层
    local l = tree:insertLeft(root, 2)
    local r = tree:insertRight(root, 3)
    -- 第3层
    tree:insertLeft(l, 4)
    tree:insertRight(l, 5)
    tree:insertLeft(r, 6)
    tree:insertRight(r, 7)
end)
$C$,
$E$### Lua 实现

使用 `local` 保存节点 ID 到局部变量, 通过 ID 指定父节点进行子树构建。$E$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES (
currval('templates_id_seq'), 'rust',
$C$use ds4viz::prelude::*;

fn main() -> ds4viz::Result<()> {
    ds4viz::binary_tree("manual_tree", |tree| {
        // 第1层: 根节点
        let root = tree.insert_root(1)?;
        // 第2层
        let l = tree.insert_left(root, 2)?;
        let r = tree.insert_right(root, 3)?;
        // 第3层
        tree.insert_left(l, 4)?;
        tree.insert_right(l, 5)?;
        tree.insert_left(r, 6)?;
        tree.insert_right(r, 7)?;
        Ok(())
    })
}
$C$,
$E$### Rust 实现

`insert_root`、`insert_left`、`insert_right` 均返回 `Result<i64>`, 其中 `i64` 为新节点 ID。重复插入根节点会返回 `StructureError::RootExists`, 对已有左/右子节点的位置插入会返回 `StructureError::ChildExists`。$E$
);

-- [14] 表达式树
INSERT INTO templates (title, category, description) VALUES (
'表达式树', '二叉树',
$D$表达式树(Expression Tree)是一种用二叉树表示算术表达式的方法: 叶节点为操作数, 内部节点为运算符。

本模板构建表达式 (3 + 5) * 2 对应的表达式树。对该树进行中序遍历可还原中缀表达式, 后序遍历可得到后缀表达式 3 5 + 2 *。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES (
currval('templates_id_seq'), 'python',
$C$import ds4viz as dv

# 表达式树: (3 + 5) * 2
with dv.binary_tree(label="expr_tree") as tree:
    root = tree.insert_root("*")
    plus = tree.insert_left(root, "+")
    tree.insert_right(root, 2)
    tree.insert_left(plus, 3)
    tree.insert_right(plus, 5)
    #       *
    #      / \
    #     +   2
    #    / \
    #   3   5
$C$,
$E$### Python 实现

运算符 `*` 和 `+` 作为字符串存入内部节点, 操作数 3、5、2 作为整数存入叶节点。ds4viz 的 `value` 字段支持混合类型(int、float、str、bool)。$E$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES (
currval('templates_id_seq'), 'lua',
$C$local ds4viz = require("ds4viz")

-- 表达式树: (3 + 5) * 2
ds4viz.withContext(ds4viz.binaryTree("expr_tree"), function(tree)
    local root = tree:insertRoot("*")
    local plus = tree:insertLeft(root, "+")
    tree:insertRight(root, 2)
    tree:insertLeft(plus, 3)
    tree:insertRight(plus, 5)
end)
$C$,
$E$### Lua 实现

Lua 中字符串和数值均可作为节点值。中序遍历该树可得到中缀表达式 `3 + 5 * 2`(需加括号), 后序遍历可得到后缀表达式 `3 5 + 2 *`。$E$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES (
currval('templates_id_seq'), 'rust',
$C$use ds4viz::prelude::*;

fn main() -> ds4viz::Result<()> {
    // 表达式树: (3 + 5) * 2
    ds4viz::binary_tree("expr_tree", |tree| {
        let root = tree.insert_root("*")?;
        let plus = tree.insert_left(root, "+")?;
        tree.insert_right(root, 2)?;
        tree.insert_left(plus, 3)?;
        tree.insert_right(plus, 5)?;
        Ok(())
    })
}
$C$,
$E$### Rust 实现

字符串字面量 `"*"` 和 `"+"` 通过 `Into<Value>` trait 自动转换为 `Value::String`, 整数自动转换为 `Value::Integer`。$E$
);

-- [15] 二叉树的节点修改与删除
INSERT INTO templates (title, category, description) VALUES (
'二叉树的节点修改与删除', '二叉树',
$D$二叉树在使用过程中经常需要动态修改节点值或删除子树。

本模板先构建一棵三层满二叉树, 然后演示 update_value(修改节点值)和 delete(删除节点及其所有子树)操作, 展示二叉树的动态修改能力。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES (
currval('templates_id_seq'), 'python',
$C$import ds4viz as dv

with dv.binary_tree(label="modify_tree") as tree:
    # 构建三层满二叉树
    root = tree.insert_root(1)
    n2 = tree.insert_left(root, 2)
    n3 = tree.insert_right(root, 3)
    n4 = tree.insert_left(n2, 4)
    tree.insert_right(n2, 5)
    tree.insert_left(n3, 6)
    tree.insert_right(n3, 7)
    #         1
    #        / \
    #       2   3
    #      / \ / \
    #     4  5 6  7

    # 修改节点值: 将节点3的值改为30, 节点4的值改为40
    tree.update_value(n3, 30)
    tree.update_value(n4, 40)

    # 删除节点2及其所有子树(节点40和5也会被级联删除)
    tree.delete(n2)
    # 最终树:
    #     1
    #      \
    #      30
    #     / \
    #    6   7
$C$,
$E$### Python 实现

`update_value(node_id, value)` 修改指定节点的值, 不影响树的结构。`delete(node_id)` 删除节点及其整棵子树, 级联删除所有后代节点。$E$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES (
currval('templates_id_seq'), 'lua',
$C$local ds4viz = require("ds4viz")

ds4viz.withContext(ds4viz.binaryTree("modify_tree"), function(tree)
    -- 构建三层满二叉树
    local root = tree:insertRoot(1)
    local n2 = tree:insertLeft(root, 2)
    local n3 = tree:insertRight(root, 3)
    local n4 = tree:insertLeft(n2, 4)
    tree:insertRight(n2, 5)
    tree:insertLeft(n3, 6)
    tree:insertRight(n3, 7)

    -- 修改节点值
    tree:updateValue(n3, 30)
    tree:updateValue(n4, 40)

    -- 删除节点2及其所有子树
    tree:delete(n2)
end)
$C$,
$E$### Lua 实现

`updateValue` 用于修改节点值, `delete` 级联删除节点及其所有后代。操作不存在的节点会抛出异常。$E$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES (
currval('templates_id_seq'), 'rust',
$C$use ds4viz::prelude::*;

fn main() -> ds4viz::Result<()> {
    ds4viz::binary_tree("modify_tree", |tree| {
        // 构建三层满二叉树
        let root = tree.insert_root(1)?;
        let n2 = tree.insert_left(root, 2)?;
        let n3 = tree.insert_right(root, 3)?;
        let n4 = tree.insert_left(n2, 4)?;
        tree.insert_right(n2, 5)?;
        tree.insert_left(n3, 6)?;
        tree.insert_right(n3, 7)?;

        // 修改节点值
        tree.update_value(n3, 30)?;
        tree.update_value(n4, 40)?;

        // 删除节点2及其所有子树
        tree.delete(n2)?;
        Ok(())
    })
}
$C$,
$E$### Rust 实现

`update_value` 接受 `impl Into<Value>`, 可以将节点值修改为任意支持的类型。`delete` 删除不存在的节点会返回 `StructureError::NodeNotFound`。$E$
);

-- =====================================================
-- 分类: 二叉搜索树
-- =====================================================

-- [16] BST的逐步构建
INSERT INTO templates (title, category, description) VALUES (
'BST的逐步构建', '二叉搜索树',
$D$二叉搜索树(BST)的核心性质: 对于任意节点, 左子树所有节点的值小于该节点, 右子树所有节点的值大于该节点。

本模板依次插入序列 [50, 30, 70, 20, 40, 60, 80], 观察 BST 如何在每次插入时自动维护有序性质。该序列恰好构建出一棵完美平衡的三层满二叉搜索树。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES (
currval('templates_id_seq'), 'python',
$C$import ds4viz as dv

with dv.binary_search_tree(label="build_bst") as bst:
    bst.insert(50)
    bst.insert(30)
    bst.insert(70)
    bst.insert(20)
    bst.insert(40)
    bst.insert(60)
    bst.insert(80)
    #        50
    #       /  \
    #     30    70
    #    / \   / \
    #   20 40 60 80
$C$,
$E$### Python 实现

每次 `insert` 时, BST 自动比较值的大小, 将新节点放置在正确的位置。例如 30 < 50 故放在根的左子树, 70 > 50 故放在根的右子树。中序遍历该树可得到升序序列 20、30、40、50、60、70、80。$E$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES (
currval('templates_id_seq'), 'lua',
$C$local ds4viz = require("ds4viz")

ds4viz.withContext(ds4viz.binarySearchTree("build_bst"), function(bst)
    bst:insert(50)
    bst:insert(30)
    bst:insert(70)
    bst:insert(20)
    bst:insert(40)
    bst:insert(60)
    bst:insert(80)
end)
$C$,
$E$### Lua 实现

BST 的 `insert` 方法只需传入值, 树会自动维护搜索树性质, 无需手动指定插入位置。$E$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES (
currval('templates_id_seq'), 'rust',
$C$use ds4viz::prelude::*;

fn main() -> ds4viz::Result<()> {
    ds4viz::binary_search_tree("build_bst", |bst| {
        bst.insert(50)?;
        bst.insert(30)?;
        bst.insert(70)?;
        bst.insert(20)?;
        bst.insert(40)?;
        bst.insert(60)?;
        bst.insert(80)?;
        Ok(())
    })
}
$C$,
$E$### Rust 实现

BST 的 `insert` 接受 `impl Into<NumericValue>`, 仅支持数值类型(`i64` 或 `f64`), 因为需要进行大小比较。$E$
);

-- [17] BST的删除操作
INSERT INTO templates (title, category, description) VALUES (
'BST的删除操作', '二叉搜索树',
$D$BST 的删除操作需要考虑三种情况: 删除叶节点(直接删除)、删除只有一个子节点的节点(用子节点替代)、删除有两个子节点的节点(用中序后继替代)。

本模板先构建与"BST的逐步构建"相同的树, 然后依次演示这三种删除场景: 删除叶节点 20、删除双子节点 70(被中序后继 80 替代)、删除单子节点 30(被子节点 40 替代)。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES (
currval('templates_id_seq'), 'python',
$C$import ds4viz as dv

with dv.binary_search_tree(label="delete_bst") as bst:
    # 构建 BST
    bst.insert(50)
    bst.insert(30)
    bst.insert(70)
    bst.insert(20)
    bst.insert(40)
    bst.insert(60)
    bst.insert(80)

    # 情况1: 删除叶节点 20
    bst.delete(20)

    # 情况3: 删除有两个子节点的节点 70 (被中序后继 80 替代)
    bst.delete(70)

    # 情况2: 删除只有一个子节点的节点 30 (被子节点 40 替代)
    bst.delete(30)
$C$,
$E$### Python 实现

`delete(20)` 直接移除叶节点; `delete(70)` 找到中序后继 80 替代 70 的位置; `delete(30)` 此时 30 仅有右子节点 40(因为 20 已被删除), 用 40 替代 30。三种情况展示了 BST 删除的完整逻辑。$E$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES (
currval('templates_id_seq'), 'lua',
$C$local ds4viz = require("ds4viz")

ds4viz.withContext(ds4viz.binarySearchTree("delete_bst"), function(bst)
    -- 构建 BST
    bst:insert(50)
    bst:insert(30)
    bst:insert(70)
    bst:insert(20)
    bst:insert(40)
    bst:insert(60)
    bst:insert(80)

    -- 情况1: 删除叶节点
    bst:delete(20)
    -- 情况3: 删除双子节点(中序后继替代)
    bst:delete(70)
    -- 情况2: 删除单子节点(子节点替代)
    bst:delete(30)
end)
$C$,
$E$### Lua 实现

`delete` 方法按值删除节点, 库内部自动处理三种删除情况并维护 BST 性质。删除不存在的值会抛出异常。$E$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES (
currval('templates_id_seq'), 'rust',
$C$use ds4viz::prelude::*;

fn main() -> ds4viz::Result<()> {
    ds4viz::binary_search_tree("delete_bst", |bst| {
        // 构建 BST
        bst.insert(50)?;
        bst.insert(30)?;
        bst.insert(70)?;
        bst.insert(20)?;
        bst.insert(40)?;
        bst.insert(60)?;
        bst.insert(80)?;

        // 情况1: 删除叶节点
        bst.delete(20)?;
        // 情况3: 删除双子节点(中序后继替代)
        bst.delete(70)?;
        // 情况2: 删除单子节点(子节点替代)
        bst.delete(30)?;
        Ok(())
    })
}
$C$,
$E$### Rust 实现

`delete` 接受 `impl Into<NumericValue>`, 如果目标值不存在会返回 `StructureError::ValueNotFound` 错误。$E$
);

-- [18] BST的有序插入退化
INSERT INTO templates (title, category, description) VALUES (
'BST的有序插入退化', '二叉搜索树',
$D$当数据按升序或降序依次插入 BST 时, 树会退化为一条链表(斜树), 查找效率从 O(log n) 降至 O(n)。

本模板按升序插入 1~8, 直观展示 BST 退化为右斜链表的过程。这也是引入平衡树(如 AVL 树、红黑树)的重要动机。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES (
currval('templates_id_seq'), 'python',
$C$import ds4viz as dv

# 按升序插入, BST 退化为右斜链表
with dv.binary_search_tree(label="degenerate_bst") as bst:
    bst.insert(1)
    bst.insert(2)
    bst.insert(3)
    bst.insert(4)
    bst.insert(5)
    bst.insert(6)
    bst.insert(7)
    bst.insert(8)
$C$,
$E$### Python 实现

每个新值都大于已有所有值, 因此总是插入到最右侧, 形成一条只有右子节点的链状结构。树的高度等于节点数 8, 对比"BST的逐步构建"中 7 个节点仅需高度 3 的平衡效果。$E$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES (
currval('templates_id_seq'), 'lua',
$C$local ds4viz = require("ds4viz")

-- 按升序插入, BST 退化为右斜链表
ds4viz.withContext(ds4viz.binarySearchTree("degenerate_bst"), function(bst)
    for i = 1, 8 do
        bst:insert(i)
    end
end)
$C$,
$E$### Lua 实现

使用 `for` 循环按 1~8 顺序插入, 简洁地展示退化过程。$E$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES (
currval('templates_id_seq'), 'rust',
$C$use ds4viz::prelude::*;

fn main() -> ds4viz::Result<()> {
    // 按升序插入, BST 退化为右斜链表
    ds4viz::binary_search_tree("degenerate_bst", |bst| {
        for i in 1..=8 {
            bst.insert(i)?;
        }
        Ok(())
    })
}
$C$,
$E$### Rust 实现

使用 `1..=8` 闭区间范围循环插入。每次插入都走最右路径, 树的高度等于节点数。$E$
);

-- =====================================================
-- 分类: 堆
-- =====================================================

-- [19] 最小堆操作演示
INSERT INTO templates (title, category, description) VALUES (
'最小堆操作演示', '堆',
$D$最小堆(Min Heap)是一种完全二叉树, 满足堆性质: 每个节点的值不大于其子节点的值, 因此根节点始终是最小值。

本模板演示最小堆的插入(上浮)和提取最小值(下沉)操作, 观察每次操作后堆结构的自动调整过程。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES (
currval('templates_id_seq'), 'python',
$C$import ds4viz as dv

with dv.heap(label="min_heap", heap_type="min") as h:
    h.insert(10)
    h.insert(5)
    h.insert(15)
    h.insert(3)
    h.insert(8)
    h.insert(20)
    h.insert(1)
    h.extract()  # 提取最小值 1
    h.extract()  # 提取最小值 3
    h.insert(2)
    h.extract()  # 提取最小值 2
    h.extract()  # 提取最小值 5
$C$,
$E$### Python 实现

每次 `insert` 后, 新元素上浮到合适位置以维护堆性质。`extract` 移除堆顶最小值后, 末尾元素移至堆顶并下沉调整。插入和提取的时间复杂度均为 O(log n)。$E$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES (
currval('templates_id_seq'), 'lua',
$C$local ds4viz = require("ds4viz")

ds4viz.withContext(ds4viz.heap("min_heap", "min"), function(h)
    h:insert(10)
    h:insert(5)
    h:insert(15)
    h:insert(3)
    h:insert(8)
    h:insert(20)
    h:insert(1)
    h:extract()  -- 提取最小值 1
    h:extract()  -- 提取最小值 3
    h:insert(2)
    h:extract()  -- 提取最小值 2
    h:extract()  -- 提取最小值 5
end)
$C$,
$E$### Lua 实现

Lua 中堆类型通过第二个参数 `"min"` 指定。操作逻辑与 Python 版本一致。$E$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES (
currval('templates_id_seq'), 'rust',
$C$use ds4viz::prelude::*;

fn main() -> ds4viz::Result<()> {
    ds4viz::heap("min_heap", HeapType::Min, |h| {
        h.insert(10)?;
        h.insert(5)?;
        h.insert(15)?;
        h.insert(3)?;
        h.insert(8)?;
        h.insert(20)?;
        h.insert(1)?;
        h.extract()?;  // 提取最小值 1
        h.extract()?;  // 提取最小值 3
        h.insert(2)?;
        h.extract()?;  // 提取最小值 2
        h.extract()?;  // 提取最小值 5
        Ok(())
    })
}
$C$,
$E$### Rust 实现

使用 `HeapType::Min` 枚举值指定最小堆。`insert` 和 `extract` 均为 O(log n) 操作。空堆执行 `extract` 会返回 `StructureError::EmptyStructure`。$E$
);

-- [20] 最大堆操作演示
INSERT INTO templates (title, category, description) VALUES (
'最大堆操作演示', '堆',
$D$最大堆(Max Heap)的堆性质与最小堆相反: 每个节点的值不小于其子节点的值, 根节点始终是最大值。

本模板演示最大堆的插入和提取最大值操作, 观察堆的结构变化。最大堆常用于实现优先队列和堆排序。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES (
currval('templates_id_seq'), 'python',
$C$import ds4viz as dv

with dv.heap(label="max_heap", heap_type="max") as h:
    h.insert(10)
    h.insert(5)
    h.insert(15)
    h.insert(3)
    h.insert(8)
    h.insert(20)
    h.insert(25)
    h.extract()  # 提取最大值 25
    h.extract()  # 提取最大值 20
    h.insert(30)
    h.extract()  # 提取最大值 30
    h.extract()  # 提取最大值 15
$C$,
$E$### Python 实现

与最小堆类似, 但 `heap_type="max"` 使得堆顶始终为最大值。`extract` 每次移除并返回当前最大值。$E$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES (
currval('templates_id_seq'), 'lua',
$C$local ds4viz = require("ds4viz")

ds4viz.withContext(ds4viz.heap("max_heap", "max"), function(h)
    h:insert(10)
    h:insert(5)
    h:insert(15)
    h:insert(3)
    h:insert(8)
    h:insert(20)
    h:insert(25)
    h:extract()  -- 提取最大值 25
    h:extract()  -- 提取最大值 20
    h:insert(30)
    h:extract()  -- 提取最大值 30
    h:extract()  -- 提取最大值 15
end)
$C$,
$E$### Lua 实现

堆类型通过第二个参数 `"max"` 指定, 其余操作与最小堆完全一致。$E$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES (
currval('templates_id_seq'), 'rust',
$C$use ds4viz::prelude::*;

fn main() -> ds4viz::Result<()> {
    ds4viz::heap("max_heap", HeapType::Max, |h| {
        h.insert(10)?;
        h.insert(5)?;
        h.insert(15)?;
        h.insert(3)?;
        h.insert(8)?;
        h.insert(20)?;
        h.insert(25)?;
        h.extract()?;  // 提取最大值 25
        h.extract()?;  // 提取最大值 20
        h.insert(30)?;
        h.extract()?;  // 提取最大值 30
        h.extract()?;  // 提取最大值 15
        Ok(())
    })
}
$C$,
$E$### Rust 实现

使用 `HeapType::Max` 枚举值指定最大堆。最大堆常用于实现优先队列和堆排序的降序版本。$E$
);

-- [21] 堆排序过程
INSERT INTO templates (title, category, description) VALUES (
'堆排序过程', '堆',
$D$堆排序(Heap Sort)利用堆的性质实现排序: 将所有元素依次插入堆中(建堆), 再逐一提取堆顶元素即可得到有序序列。

本模板使用最小堆对数组 [38, 27, 43, 3, 9, 82, 10] 进行排序, 逐一提取后得到升序结果 [3, 9, 10, 27, 38, 43, 82]。总时间复杂度为 O(n log n)。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES (
currval('templates_id_seq'), 'python',
$C$import ds4viz as dv

data = [38, 27, 43, 3, 9, 82, 10]

with dv.heap(label="heap_sort", heap_type="min") as h:
    # 建堆阶段: 依次插入所有元素
    for val in data:
        h.insert(val)
    # 排序阶段: 逐一提取最小值
    # 提取顺序: 3, 9, 10, 27, 38, 43, 82
    for _ in range(len(data)):
        h.extract()
$C$,
$E$### Python 实现

分为两个阶段: 建堆阶段将无序数组中的元素依次插入最小堆; 排序阶段逐一提取堆顶最小值, 得到升序序列。总时间复杂度 O(n log n)。$E$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES (
currval('templates_id_seq'), 'lua',
$C$local ds4viz = require("ds4viz")

local data = {38, 27, 43, 3, 9, 82, 10}

ds4viz.withContext(ds4viz.heap("heap_sort", "min"), function(h)
    -- 建堆阶段: 依次插入所有元素
    for _, val in ipairs(data) do
        h:insert(val)
    end
    -- 排序阶段: 逐一提取最小值
    for _ = 1, #data do
        h:extract()
    end
end)
$C$,
$E$### Lua 实现

使用 `ipairs` 遍历数组进行建堆, `#data` 获取数组长度用于提取循环。$E$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES (
currval('templates_id_seq'), 'rust',
$C$use ds4viz::prelude::*;

fn main() -> ds4viz::Result<()> {
    let data = [38, 27, 43, 3, 9, 82, 10];

    ds4viz::heap("heap_sort", HeapType::Min, |h| {
        // 建堆阶段: 依次插入所有元素
        for &val in &data {
            h.insert(val)?;
        }
        // 排序阶段: 逐一提取最小值
        for _ in 0..data.len() {
            h.extract()?;
        }
        Ok(())
    })
}
$C$,
$E$### Rust 实现

使用 `&data` 引用数组并通过 `&val` 解引用每个元素。`data.len()` 获取数组长度控制提取次数。$E$
);

-- =====================================================
-- 分类: 无向图
-- =====================================================

-- [22] 无向图的构建
INSERT INTO templates (title, category, description) VALUES (
'无向图的构建', '无向图',
$D$无向图(Undirected Graph)由节点和无方向的边组成, 每条边连接两个节点且双向可达。

本模板构建一个包含 6 个节点和 7 条边的连通无向图, 展示 add_node 和 add_edge 的基本用法。该图包含多个环路, 是一个典型的稠密连通图。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES (
currval('templates_id_seq'), 'python',
$C$import ds4viz as dv

with dv.graph_undirected(label="simple_graph") as g:
    # 添加 6 个节点
    g.add_node(0, "A")
    g.add_node(1, "B")
    g.add_node(2, "C")
    g.add_node(3, "D")
    g.add_node(4, "E")
    g.add_node(5, "F")
    # 添加 7 条无向边
    g.add_edge(0, 1)  # A -- B
    g.add_edge(0, 2)  # A -- C
    g.add_edge(1, 2)  # B -- C
    g.add_edge(1, 3)  # B -- D
    g.add_edge(2, 4)  # C -- E
    g.add_edge(3, 5)  # D -- F
    g.add_edge(4, 5)  # E -- F
$C$,
$E$### Python 实现

先通过 `add_node` 添加所有节点(指定 ID 和标签), 再通过 `add_edge` 添加无向边。无向图中 `add_edge(0, 1)` 和 `add_edge(1, 0)` 等价, 库内部会自动规范化为 `from_id < to_id`。不允许自环, `add_edge(0, 0)` 会抛出异常。$E$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES (
currval('templates_id_seq'), 'lua',
$C$local ds4viz = require("ds4viz")

ds4viz.withContext(ds4viz.graphUndirected("simple_graph"), function(g)
    -- 添加 6 个节点
    g:addNode(0, "A")
    g:addNode(1, "B")
    g:addNode(2, "C")
    g:addNode(3, "D")
    g:addNode(4, "E")
    g:addNode(5, "F")
    -- 添加 7 条无向边
    g:addEdge(0, 1)
    g:addEdge(0, 2)
    g:addEdge(1, 2)
    g:addEdge(1, 3)
    g:addEdge(2, 4)
    g:addEdge(3, 5)
    g:addEdge(4, 5)
end)
$C$,
$E$### Lua 实现

Lua 中图的方法使用驼峰命名: `addNode`、`addEdge`。节点 ID 为 number 类型, 标签为 string 类型。$E$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES (
currval('templates_id_seq'), 'rust',
$C$use ds4viz::prelude::*;

fn main() -> ds4viz::Result<()> {
    ds4viz::graph_undirected("simple_graph", |g| {
        // 添加 6 个节点
        g.add_node(0, "A")?;
        g.add_node(1, "B")?;
        g.add_node(2, "C")?;
        g.add_node(3, "D")?;
        g.add_node(4, "E")?;
        g.add_node(5, "F")?;
        // 添加 7 条无向边
        g.add_edge(0, 1)?;
        g.add_edge(0, 2)?;
        g.add_edge(1, 2)?;
        g.add_edge(1, 3)?;
        g.add_edge(2, 4)?;
        g.add_edge(3, 5)?;
        g.add_edge(4, 5)?;
        Ok(())
    })
}
$C$,
$E$### Rust 实现

`add_node` 的标签参数为 `&str`, 长度限制 1~32 字符。重复添加同一节点会返回 `StructureError::NodeExists`, 自环会返回 `StructureError::SelfLoop`。$E$
);

-- [23] 无向图的动态修改
INSERT INTO templates (title, category, description) VALUES (
'无向图的动态修改', '无向图',
$D$图结构在使用过程中经常需要动态增删节点和边。

本模板演示构建无向图后进行的一系列动态操作: 更新节点标签、删除边、添加新节点和新边, 以及删除节点(会同时移除关联的所有边)。覆盖了无向图全部五种 API 方法。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES (
currval('templates_id_seq'), 'python',
$C$import ds4viz as dv

with dv.graph_undirected(label="dynamic_graph") as g:
    # 构建初始图
    g.add_node(0, "A")
    g.add_node(1, "B")
    g.add_node(2, "C")
    g.add_node(3, "D")
    g.add_edge(0, 1)  # A -- B
    g.add_edge(1, 2)  # B -- C
    g.add_edge(2, 3)  # C -- D
    g.add_edge(0, 3)  # A -- D

    # 修改节点标签
    g.update_node_label(0, "Alpha")
    g.update_node_label(2, "Gamma")

    # 删除一条边
    g.remove_edge(0, 3)

    # 添加新节点和新边
    g.add_node(4, "E")
    g.add_edge(3, 4)
    g.add_edge(0, 4)
    g.add_edge(1, 4)

    # 删除节点1(B)及其所有关联边
    g.remove_node(1)
$C$,
$E$### Python 实现

本模板覆盖了无向图的全部五种 API: `add_node`、`add_edge`、`update_node_label`、`remove_edge`、`remove_node`。`remove_node` 会级联删除与该节点关联的所有边, 避免产生悬空边。$E$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES (
currval('templates_id_seq'), 'lua',
$C$local ds4viz = require("ds4viz")

ds4viz.withContext(ds4viz.graphUndirected("dynamic_graph"), function(g)
    -- 构建初始图
    g:addNode(0, "A")
    g:addNode(1, "B")
    g:addNode(2, "C")
    g:addNode(3, "D")
    g:addEdge(0, 1)
    g:addEdge(1, 2)
    g:addEdge(2, 3)
    g:addEdge(0, 3)

    -- 修改节点标签
    g:updateNodeLabel(0, "Alpha")
    g:updateNodeLabel(2, "Gamma")

    -- 删除一条边
    g:removeEdge(0, 3)

    -- 添加新节点和新边
    g:addNode(4, "E")
    g:addEdge(3, 4)
    g:addEdge(0, 4)
    g:addEdge(1, 4)

    -- 删除节点1(B)及其所有关联边
    g:removeNode(1)
end)
$C$,
$E$### Lua 实现

`removeNode` 级联删除所有关联边, 避免产生悬空边。`updateNodeLabel` 可修改已有节点的标签。$E$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES (
currval('templates_id_seq'), 'rust',
$C$use ds4viz::prelude::*;

fn main() -> ds4viz::Result<()> {
    ds4viz::graph_undirected("dynamic_graph", |g| {
        // 构建初始图
        g.add_node(0, "A")?;
        g.add_node(1, "B")?;
        g.add_node(2, "C")?;
        g.add_node(3, "D")?;
        g.add_edge(0, 1)?;
        g.add_edge(1, 2)?;
        g.add_edge(2, 3)?;
        g.add_edge(0, 3)?;

        // 修改节点标签
        g.update_node_label(0, "Alpha")?;
        g.update_node_label(2, "Gamma")?;

        // 删除一条边
        g.remove_edge(0, 3)?;

        // 添加新节点和新边
        g.add_node(4, "E")?;
        g.add_edge(3, 4)?;
        g.add_edge(0, 4)?;
        g.add_edge(1, 4)?;

        // 删除节点1(B)及其所有关联边
        g.remove_node(1)?;
        Ok(())
    })
}
$C$,
$E$### Rust 实现

删除不存在的节点或边会返回 `StructureError::NodeNotFound` 或 `StructureError::EdgeNotFound` 错误。$E$
);

-- [24] 彼得森图
INSERT INTO templates (title, category, description) VALUES (
'彼得森图', '无向图',
$D$彼得森图(Petersen Graph)是图论中最著名的经典小型图之一, 由 10 个节点和 15 条边组成。它的结构包含一个外圈五边形、一个内圈五角星以及连接内外圈对应节点的 5 条辐条边。

彼得森图是 3-正则图(每个节点恰好连接 3 条边), 同时也是许多图论性质的经典反例: 它不是平面图, 也不含哈密顿回路。本模板构建完整的彼得森图, 展示较复杂的无向图结构。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES (
currval('templates_id_seq'), 'python',
$C$import ds4viz as dv

with dv.graph_undirected(label="petersen_graph") as g:
    # 外圈五边形: o0 ~ o4
    g.add_node(0, "o0")
    g.add_node(1, "o1")
    g.add_node(2, "o2")
    g.add_node(3, "o3")
    g.add_node(4, "o4")
    # 内圈五角星: i0 ~ i4
    g.add_node(5, "i0")
    g.add_node(6, "i1")
    g.add_node(7, "i2")
    g.add_node(8, "i3")
    g.add_node(9, "i4")

    # 外圈边(五边形)
    g.add_edge(0, 1)
    g.add_edge(1, 2)
    g.add_edge(2, 3)
    g.add_edge(3, 4)
    g.add_edge(4, 0)

    # 辐条边(连接内外圈)
    g.add_edge(0, 5)
    g.add_edge(1, 6)
    g.add_edge(2, 7)
    g.add_edge(3, 8)
    g.add_edge(4, 9)

    # 内圈边(五角星)
    g.add_edge(5, 7)
    g.add_edge(7, 9)
    g.add_edge(9, 6)
    g.add_edge(6, 8)
    g.add_edge(8, 5)
$C$,
$E$### Python 实现

彼得森图共 10 个节点 15 条边。外圈节点 o0~o4 构成五边形, 内圈节点 i0~i4 构成五角星(每隔一个节点相连), 辐条边连接外圈第 k 个节点和内圈第 k 个节点。每个节点恰好连接 3 条边, 是 3-正则图。$E$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES (
currval('templates_id_seq'), 'lua',
$C$local ds4viz = require("ds4viz")

ds4viz.withContext(ds4viz.graphUndirected("petersen_graph"), function(g)
    -- 外圈五边形: o0 ~ o4
    g:addNode(0, "o0")
    g:addNode(1, "o1")
    g:addNode(2, "o2")
    g:addNode(3, "o3")
    g:addNode(4, "o4")
    -- 内圈五角星: i0 ~ i4
    g:addNode(5, "i0")
    g:addNode(6, "i1")
    g:addNode(7, "i2")
    g:addNode(8, "i3")
    g:addNode(9, "i4")

    -- 外圈边(五边形)
    g:addEdge(0, 1)
    g:addEdge(1, 2)
    g:addEdge(2, 3)
    g:addEdge(3, 4)
    g:addEdge(4, 0)

    -- 辐条边(连接内外圈)
    g:addEdge(0, 5)
    g:addEdge(1, 6)
    g:addEdge(2, 7)
    g:addEdge(3, 8)
    g:addEdge(4, 9)

    -- 内圈边(五角星)
    g:addEdge(5, 7)
    g:addEdge(7, 9)
    g:addEdge(9, 6)
    g:addEdge(6, 8)
    g:addEdge(8, 5)
end)
$C$,
$E$### Lua 实现

10 个节点 15 条边构成完整的彼得森图。该图是图论研究中极为重要的反例图, 常用于检验图论猜想。$E$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES (
currval('templates_id_seq'), 'rust',
$C$use ds4viz::prelude::*;

fn main() -> ds4viz::Result<()> {
    ds4viz::graph_undirected("petersen_graph", |g| {
        // 外圈五边形: o0 ~ o4
        g.add_node(0, "o0")?;
        g.add_node(1, "o1")?;
        g.add_node(2, "o2")?;
        g.add_node(3, "o3")?;
        g.add_node(4, "o4")?;
        // 内圈五角星: i0 ~ i4
        g.add_node(5, "i0")?;
        g.add_node(6, "i1")?;
        g.add_node(7, "i2")?;
        g.add_node(8, "i3")?;
        g.add_node(9, "i4")?;

        // 外圈边(五边形)
        g.add_edge(0, 1)?;
        g.add_edge(1, 2)?;
        g.add_edge(2, 3)?;
        g.add_edge(3, 4)?;
        g.add_edge(4, 0)?;

        // 辐条边(连接内外圈)
        g.add_edge(0, 5)?;
        g.add_edge(1, 6)?;
        g.add_edge(2, 7)?;
        g.add_edge(3, 8)?;
        g.add_edge(4, 9)?;

        // 内圈边(五角星)
        g.add_edge(5, 7)?;
        g.add_edge(7, 9)?;
        g.add_edge(9, 6)?;
        g.add_edge(6, 8)?;
        g.add_edge(8, 5)?;
        Ok(())
    })
}
$C$,
$E$### Rust 实现

彼得森图的色数为 3、边色数为 4、围长为 5。它是最小的不含哈密顿回路的 hypohamiltonian 图。$E$
);

-- =====================================================
-- 分类: 有向图
-- =====================================================

-- [25] 有向图的构建
INSERT INTO templates (title, category, description) VALUES (
'有向图的构建', '有向图',
$D$有向图(Directed Graph)的边具有方向性, 从起点指向终点, A->B 不等同于 B->A。

本模板构建一个包含 5 个节点的有向图, 其中包含一个环路 A->B->D->E->A, 展示有向图的基本结构和环路特征。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES (
currval('templates_id_seq'), 'python',
$C$import ds4viz as dv

with dv.graph_directed(label="simple_digraph") as g:
    g.add_node(0, "A")
    g.add_node(1, "B")
    g.add_node(2, "C")
    g.add_node(3, "D")
    g.add_node(4, "E")
    g.add_edge(0, 1)  # A -> B
    g.add_edge(0, 2)  # A -> C
    g.add_edge(1, 3)  # B -> D
    g.add_edge(2, 3)  # C -> D
    g.add_edge(3, 4)  # D -> E
    g.add_edge(4, 0)  # E -> A (形成环)
$C$,
$E$### Python 实现

有向图的 `add_edge(from_id, to_id)` 创建从 `from_id` 到 `to_id` 的单向边。边 `(4, 0)` 形成了从 E 回到 A 的环路。与无向图不同, 有向图中 `add_edge(0, 1)` 和 `add_edge(1, 0)` 是两条不同的边。$E$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES (
currval('templates_id_seq'), 'lua',
$C$local ds4viz = require("ds4viz")

ds4viz.withContext(ds4viz.graphDirected("simple_digraph"), function(g)
    g:addNode(0, "A")
    g:addNode(1, "B")
    g:addNode(2, "C")
    g:addNode(3, "D")
    g:addNode(4, "E")
    g:addEdge(0, 1)  -- A -> B
    g:addEdge(0, 2)  -- A -> C
    g:addEdge(1, 3)  -- B -> D
    g:addEdge(2, 3)  -- C -> D
    g:addEdge(3, 4)  -- D -> E
    g:addEdge(4, 0)  -- E -> A (形成环)
end)
$C$,
$E$### Lua 实现

有向图中 `addEdge(0, 1)` 和 `addEdge(1, 0)` 是两条不同的边, 分别表示 A->B 和 B->A。$E$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES (
currval('templates_id_seq'), 'rust',
$C$use ds4viz::prelude::*;

fn main() -> ds4viz::Result<()> {
    ds4viz::graph_directed("simple_digraph", |g| {
        g.add_node(0, "A")?;
        g.add_node(1, "B")?;
        g.add_node(2, "C")?;
        g.add_node(3, "D")?;
        g.add_node(4, "E")?;
        g.add_edge(0, 1)?;  // A -> B
        g.add_edge(0, 2)?;  // A -> C
        g.add_edge(1, 3)?;  // B -> D
        g.add_edge(2, 3)?;  // C -> D
        g.add_edge(3, 4)?;  // D -> E
        g.add_edge(4, 0)?;  // E -> A (形成环)
        Ok(())
    })
}
$C$,
$E$### Rust 实现

有向图允许环路, 但不允许自环(`add_edge(0, 0)` 会返回 `StructureError::SelfLoop` 错误)。$E$
);

-- [26] 课程先修关系图
INSERT INTO templates (title, category, description) VALUES (
'课程先修关系图', '有向图',
$D$大学课程之间存在先修关系: 必须先完成前置课程才能选修后续课程。这种依赖关系天然构成有向无环图(DAG)。

本模板用有向图表示 7 门计算机科学课程的先修关系, 边 A->B 表示课程 A 是课程 B 的先修课。对该图进行拓扑排序即可得到合理的选课顺序。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES (
currval('templates_id_seq'), 'python',
$C$import ds4viz as dv

with dv.graph_directed(label="course_prereqs") as g:
    g.add_node(0, "Math")
    g.add_node(1, "C-Lang")
    g.add_node(2, "DataStruct")
    g.add_node(3, "Algorithm")
    g.add_node(4, "OS")
    g.add_node(5, "Compiler")
    g.add_node(6, "Database")
    g.add_edge(0, 2)  # 数学 -> 数据结构
    g.add_edge(1, 2)  # C语言 -> 数据结构
    g.add_edge(2, 3)  # 数据结构 -> 算法
    g.add_edge(2, 4)  # 数据结构 -> 操作系统
    g.add_edge(2, 6)  # 数据结构 -> 数据库
    g.add_edge(3, 5)  # 算法 -> 编译原理
    g.add_edge(4, 5)  # 操作系统 -> 编译原理
$C$,
$E$### Python 实现

这是一个 DAG(有向无环图), 没有环路。节点 `DataStruct`(数据结构) 的出度最高, 是后续多门课程的先修课。一个合法的拓扑排序为: Math -> C-Lang -> DataStruct -> Algorithm -> OS -> Database -> Compiler。$E$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES (
currval('templates_id_seq'), 'lua',
$C$local ds4viz = require("ds4viz")

ds4viz.withContext(ds4viz.graphDirected("course_prereqs"), function(g)
    g:addNode(0, "Math")
    g:addNode(1, "C-Lang")
    g:addNode(2, "DataStruct")
    g:addNode(3, "Algorithm")
    g:addNode(4, "OS")
    g:addNode(5, "Compiler")
    g:addNode(6, "Database")
    g:addEdge(0, 2)  -- 数学 -> 数据结构
    g:addEdge(1, 2)  -- C语言 -> 数据结构
    g:addEdge(2, 3)  -- 数据结构 -> 算法
    g:addEdge(2, 4)  -- 数据结构 -> 操作系统
    g:addEdge(2, 6)  -- 数据结构 -> 数据库
    g:addEdge(3, 5)  -- 算法 -> 编译原理
    g:addEdge(4, 5)  -- 操作系统 -> 编译原理
end)
$C$,
$E$### Lua 实现

DAG 在拓扑排序、任务调度等场景中广泛应用。编译原理(Compiler)需要算法和操作系统两门先修课, 入度为 2。$E$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES (
currval('templates_id_seq'), 'rust',
$C$use ds4viz::prelude::*;

fn main() -> ds4viz::Result<()> {
    ds4viz::graph_directed("course_prereqs", |g| {
        g.add_node(0, "Math")?;
        g.add_node(1, "C-Lang")?;
        g.add_node(2, "DataStruct")?;
        g.add_node(3, "Algorithm")?;
        g.add_node(4, "OS")?;
        g.add_node(5, "Compiler")?;
        g.add_node(6, "Database")?;
        g.add_edge(0, 2)?;  // 数学 -> 数据结构
        g.add_edge(1, 2)?;  // C语言 -> 数据结构
        g.add_edge(2, 3)?;  // 数据结构 -> 算法
        g.add_edge(2, 4)?;  // 数据结构 -> 操作系统
        g.add_edge(2, 6)?;  // 数据结构 -> 数据库
        g.add_edge(3, 5)?;  // 算法 -> 编译原理
        g.add_edge(4, 5)?;  // 操作系统 -> 编译原理
        Ok(())
    })
}
$C$,
$E$### Rust 实现

该图为 DAG(有向无环图), 不存在环路, 适合用于演示拓扑排序算法。$E$
);

-- [27] 状态转换图
INSERT INTO templates (title, category, description) VALUES (
'状态转换图', '有向图',
$D$有限状态机(FSM)的状态转换关系可以用有向图表示: 节点为状态, 有向边为状态转换。

本模板用有向图表示一个简单的订单处理流程状态机: 从创建到付款、发货、送达, 或在过程中取消。边的方向限定了状态转换只能单向进行。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES (
currval('templates_id_seq'), 'python',
$C$import ds4viz as dv

# 订单处理状态机
with dv.graph_directed(label="order_fsm") as g:
    g.add_node(0, "Created")
    g.add_node(1, "Paid")
    g.add_node(2, "Shipped")
    g.add_node(3, "Delivered")
    g.add_node(4, "Cancelled")
    g.add_node(5, "Refunded")
    g.add_edge(0, 1)  # 创建 -> 已付款
    g.add_edge(1, 2)  # 已付款 -> 已发货
    g.add_edge(2, 3)  # 已发货 -> 已送达
    g.add_edge(0, 4)  # 创建 -> 已取消
    g.add_edge(1, 4)  # 已付款 -> 已取消
    g.add_edge(3, 5)  # 已送达 -> 已退款
$C$,
$E$### Python 实现

每个节点代表订单的一个状态, 有向边表示合法的状态转换。注意不是所有状态间都可以转换, 例如已发货后不能直接取消, 只有送达后才能退款。这体现了有向图在约束建模中的作用。$E$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES (
currval('templates_id_seq'), 'lua',
$C$local ds4viz = require("ds4viz")

-- 订单处理状态机
ds4viz.withContext(ds4viz.graphDirected("order_fsm"), function(g)
    g:addNode(0, "Created")
    g:addNode(1, "Paid")
    g:addNode(2, "Shipped")
    g:addNode(3, "Delivered")
    g:addNode(4, "Cancelled")
    g:addNode(5, "Refunded")
    g:addEdge(0, 1)  -- 创建 -> 已付款
    g:addEdge(1, 2)  -- 已付款 -> 已发货
    g:addEdge(2, 3)  -- 已发货 -> 已送达
    g:addEdge(0, 4)  -- 创建 -> 已取消
    g:addEdge(1, 4)  -- 已付款 -> 已取消
    g:addEdge(3, 5)  -- 已送达 -> 已退款
end)
$C$,
$E$### Lua 实现

有向图天然适合表示状态机: 边的方向限定了状态转换的方向, 不存在反向边意味着转换不可逆。$E$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES (
currval('templates_id_seq'), 'rust',
$C$use ds4viz::prelude::*;

fn main() -> ds4viz::Result<()> {
    // 订单处理状态机
    ds4viz::graph_directed("order_fsm", |g| {
        g.add_node(0, "Created")?;
        g.add_node(1, "Paid")?;
        g.add_node(2, "Shipped")?;
        g.add_node(3, "Delivered")?;
        g.add_node(4, "Cancelled")?;
        g.add_node(5, "Refunded")?;
        g.add_edge(0, 1)?;  // 创建 -> 已付款
        g.add_edge(1, 2)?;  // 已付款 -> 已发货
        g.add_edge(2, 3)?;  // 已发货 -> 已送达
        g.add_edge(0, 4)?;  // 创建 -> 已取消
        g.add_edge(1, 4)?;  // 已付款 -> 已取消
        g.add_edge(3, 5)?;  // 已送达 -> 已退款
        Ok(())
    })
}
$C$,
$E$### Rust 实现

状态机是有向图的经典应用。ds4viz 不允许自环, 因此无法直接表示"停留在当前状态"的转换。$E$
);

-- =====================================================
-- 分类: 带权图
-- =====================================================

-- [28] 城市交通网络
INSERT INTO templates (title, category, description) VALUES (
'城市交通网络', '带权图',
$D$城市间的交通网络适合用带权无向图建模: 城市为节点, 公路为边, 距离(公里)为权重。

本模板构建一个 5 城市交通网络, 展示带权无向图的基本构建方式。权重可用于最短路径、最小生成树等经典图算法。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES (
currval('templates_id_seq'), 'python',
$C$import ds4viz as dv

with dv.graph_weighted(label="city_roads", directed=False) as g:
    g.add_node(0, "Beijing")
    g.add_node(1, "Shanghai")
    g.add_node(2, "Guangzhou")
    g.add_node(3, "Shenzhen")
    g.add_node(4, "Chengdu")
    g.add_edge(0, 1, 1213.0)  # 北京 -- 上海
    g.add_edge(0, 4, 1816.0)  # 北京 -- 成都
    g.add_edge(1, 2, 1462.0)  # 上海 -- 广州
    g.add_edge(2, 3, 147.0)   # 广州 -- 深圳
    g.add_edge(2, 4, 1947.0)  # 广州 -- 成都
    g.add_edge(3, 4, 1900.0)  # 深圳 -- 成都
$C$,
$E$### Python 实现

`directed=False` 创建无向带权图。`add_edge` 的第三个参数为边的权重(float 类型), 这里表示城市间的公路距离(单位: 公里)。$E$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES (
currval('templates_id_seq'), 'lua',
$C$local ds4viz = require("ds4viz")

ds4viz.withContext(ds4viz.graphWeighted("city_roads", false), function(g)
    g:addNode(0, "Beijing")
    g:addNode(1, "Shanghai")
    g:addNode(2, "Guangzhou")
    g:addNode(3, "Shenzhen")
    g:addNode(4, "Chengdu")
    g:addEdge(0, 1, 1213.0)
    g:addEdge(0, 4, 1816.0)
    g:addEdge(1, 2, 1462.0)
    g:addEdge(2, 3, 147.0)
    g:addEdge(2, 4, 1947.0)
    g:addEdge(3, 4, 1900.0)
end)
$C$,
$E$### Lua 实现

`graphWeighted` 的第二个参数 `false` 表示无向图。`addEdge` 的第三个参数为权重(number 类型)。$E$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES (
currval('templates_id_seq'), 'rust',
$C$use ds4viz::prelude::*;

fn main() -> ds4viz::Result<()> {
    ds4viz::graph_weighted("city_roads", false, |g| {
        g.add_node(0, "Beijing")?;
        g.add_node(1, "Shanghai")?;
        g.add_node(2, "Guangzhou")?;
        g.add_node(3, "Shenzhen")?;
        g.add_node(4, "Chengdu")?;
        g.add_edge(0, 1, 1213.0)?;
        g.add_edge(0, 4, 1816.0)?;
        g.add_edge(1, 2, 1462.0)?;
        g.add_edge(2, 3, 147.0)?;
        g.add_edge(2, 4, 1947.0)?;
        g.add_edge(3, 4, 1900.0)?;
        Ok(())
    })
}
$C$,
$E$### Rust 实现

`graph_weighted` 第二个参数 `false` 表示无向图。权重类型为 `f64`。可通过 `update_weight` 方法动态调整边权重。$E$
);

-- [29] 网络拓扑图
INSERT INTO templates (title, category, description) VALUES (
'网络拓扑图', '带权图',
$D$计算机网络的拓扑结构适合用有向带权图表示: 服务器为节点, 网络连接为有向边, 延迟(毫秒)为权重。

本模板构建一个从客户端到数据库的典型多层网络拓扑, 展示请求的流经路径和各段延迟。负载均衡器将流量分发到两台服务器, 体现了有向带权图在网络建模中的应用。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES (
currval('templates_id_seq'), 'python',
$C$import ds4viz as dv

# 有向带权图: 网络延迟(ms)
with dv.graph_weighted(label="network_topo", directed=True) as g:
    g.add_node(0, "Client")
    g.add_node(1, "CDN")
    g.add_node(2, "LB")
    g.add_node(3, "Server-A")
    g.add_node(4, "Server-B")
    g.add_node(5, "Cache")
    g.add_node(6, "Database")
    g.add_edge(0, 1, 5.0)   # 客户端 -> CDN
    g.add_edge(1, 2, 2.0)   # CDN -> 负载均衡
    g.add_edge(2, 3, 1.0)   # LB -> 服务器A
    g.add_edge(2, 4, 1.5)   # LB -> 服务器B
    g.add_edge(3, 5, 0.5)   # 服务器A -> 缓存
    g.add_edge(4, 5, 0.5)   # 服务器B -> 缓存
    g.add_edge(5, 6, 3.0)   # 缓存 -> 数据库
$C$,
$E$### Python 实现

`directed=True` 创建有向带权图。请求从 Client 出发, 经 CDN、负载均衡(LB)分发到两台服务器, 经缓存层最终访问数据库。通过 Server-A 的总延迟为 5+2+1+0.5+3=11.5ms, 通过 Server-B 为 5+2+1.5+0.5+3=12ms。$E$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES (
currval('templates_id_seq'), 'lua',
$C$local ds4viz = require("ds4viz")

-- 有向带权图: 网络延迟(ms)
ds4viz.withContext(ds4viz.graphWeighted("network_topo", true), function(g)
    g:addNode(0, "Client")
    g:addNode(1, "CDN")
    g:addNode(2, "LB")
    g:addNode(3, "Server-A")
    g:addNode(4, "Server-B")
    g:addNode(5, "Cache")
    g:addNode(6, "Database")
    g:addEdge(0, 1, 5.0)
    g:addEdge(1, 2, 2.0)
    g:addEdge(2, 3, 1.0)
    g:addEdge(2, 4, 1.5)
    g:addEdge(3, 5, 0.5)
    g:addEdge(4, 5, 0.5)
    g:addEdge(5, 6, 3.0)
end)
$C$,
$E$### Lua 实现

`graphWeighted` 第二个参数 `true` 表示有向图。该图展示了典型的多层网络架构。$E$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES (
currval('templates_id_seq'), 'rust',
$C$use ds4viz::prelude::*;

fn main() -> ds4viz::Result<()> {
    // 有向带权图: 网络延迟(ms)
    ds4viz::graph_weighted("network_topo", true, |g| {
        g.add_node(0, "Client")?;
        g.add_node(1, "CDN")?;
        g.add_node(2, "LB")?;
        g.add_node(3, "Server-A")?;
        g.add_node(4, "Server-B")?;
        g.add_node(5, "Cache")?;
        g.add_node(6, "Database")?;
        g.add_edge(0, 1, 5.0)?;
        g.add_edge(1, 2, 2.0)?;
        g.add_edge(2, 3, 1.0)?;
        g.add_edge(2, 4, 1.5)?;
        g.add_edge(3, 5, 0.5)?;
        g.add_edge(4, 5, 0.5)?;
        g.add_edge(5, 6, 3.0)?;
        Ok(())
    })
}
$C$,
$E$### Rust 实现

有向带权图适合建模网络流量分析、延迟优化等场景。可使用 `update_weight` 模拟网络延迟波动。$E$
);

-- [30] Dijkstra最短路径示例图
INSERT INTO templates (title, category, description) VALUES (
'Dijkstra最短路径示例图', '带权图',
$D$最短路径问题是图论中最经典的问题之一: 给定带权图和起点, 求到各个节点的最短路径。

本模板构建 Dijkstra 算法教材中的经典示例图, 包含 6 个节点(S 为起点, T 为终点)和 10 条有向带权边。从 S 到 T 的最短路径为 S->B->A->C->T, 总权重为 13。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES (
currval('templates_id_seq'), 'python',
$C$import ds4viz as dv

with dv.graph_weighted(label="shortest_path", directed=True) as g:
    g.add_node(0, "S")
    g.add_node(1, "A")
    g.add_node(2, "B")
    g.add_node(3, "C")
    g.add_node(4, "D")
    g.add_node(5, "T")
    g.add_edge(0, 1, 10.0)  # S -> A
    g.add_edge(0, 2, 5.0)   # S -> B
    g.add_edge(1, 2, 2.0)   # A -> B
    g.add_edge(1, 3, 1.0)   # A -> C
    g.add_edge(2, 1, 3.0)   # B -> A
    g.add_edge(2, 3, 9.0)   # B -> C
    g.add_edge(2, 4, 2.0)   # B -> D
    g.add_edge(3, 5, 4.0)   # C -> T
    g.add_edge(4, 3, 6.0)   # D -> C
    g.add_edge(4, 5, 7.0)   # D -> T
$C$,
$E$### Python 实现

这是 Dijkstra 算法的经典示例图。手动推演最短路径:

- S->B: 权重 5
- S->B->A: 权重 5+3=8 (优于 S->A 的 10)
- S->B->A->C: 权重 8+1=9 (优于 S->B->C 的 14 和 S->B->D->C 的 13)
- S->B->A->C->T: 权重 9+4=13 (优于 S->B->D->T 的 14)

因此 S 到 T 的最短路径为 S->B->A->C->T, 总权重 13。注意 B->A 和 A->B 权重不同, 体现了有向图的非对称性。$E$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES (
currval('templates_id_seq'), 'lua',
$C$local ds4viz = require("ds4viz")

ds4viz.withContext(ds4viz.graphWeighted("shortest_path", true), function(g)
    g:addNode(0, "S")
    g:addNode(1, "A")
    g:addNode(2, "B")
    g:addNode(3, "C")
    g:addNode(4, "D")
    g:addNode(5, "T")
    g:addEdge(0, 1, 10.0)  -- S -> A
    g:addEdge(0, 2, 5.0)   -- S -> B
    g:addEdge(1, 2, 2.0)   -- A -> B
    g:addEdge(1, 3, 1.0)   -- A -> C
    g:addEdge(2, 1, 3.0)   -- B -> A
    g:addEdge(2, 3, 9.0)   -- B -> C
    g:addEdge(2, 4, 2.0)   -- B -> D
    g:addEdge(3, 5, 4.0)   -- C -> T
    g:addEdge(4, 3, 6.0)   -- D -> C
    g:addEdge(4, 5, 7.0)   -- D -> T
end)
$C$,
$E$### Lua 实现

该图包含反向边(A->B 权重 2, B->A 权重 3), 适合演示 Dijkstra、Bellman-Ford 等最短路径算法。最短路径 S->B->A->C->T 总权重为 13。$E$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES (
currval('templates_id_seq'), 'rust',
$C$use ds4viz::prelude::*;

fn main() -> ds4viz::Result<()> {
    ds4viz::graph_weighted("shortest_path", true, |g| {
        g.add_node(0, "S")?;
        g.add_node(1, "A")?;
        g.add_node(2, "B")?;
        g.add_node(3, "C")?;
        g.add_node(4, "D")?;
        g.add_node(5, "T")?;
        g.add_edge(0, 1, 10.0)?;  // S -> A
        g.add_edge(0, 2, 5.0)?;   // S -> B
        g.add_edge(1, 2, 2.0)?;   // A -> B
        g.add_edge(1, 3, 1.0)?;   // A -> C
        g.add_edge(2, 1, 3.0)?;   // B -> A
        g.add_edge(2, 3, 9.0)?;   // B -> C
        g.add_edge(2, 4, 2.0)?;   // B -> D
        g.add_edge(3, 5, 4.0)?;   // C -> T
        g.add_edge(4, 3, 6.0)?;   // D -> C
        g.add_edge(4, 5, 7.0)?;   // D -> T
        Ok(())
    })
}
$C$,
$E$### Rust 实现

该图是 Dijkstra 算法教材中的经典例题。`update_weight` 方法可在构建后动态调整边权重以探索不同场景。$E$
);

-- =====================================================
-- 更新序列值, 使后续插入从正确的 ID 继续
-- =====================================================
SELECT setval('templates_id_seq', (SELECT COALESCE(MAX(id), 0) FROM templates));
SELECT setval('template_codes_id_seq', (SELECT COALESCE(MAX(id), 0) FROM template_codes));

COMMIT;