-- =====================================================
-- ds4viz 模板初始化数据（完整版）
-- 40 个模板 × 4 种语言 (Python / Lua / Rust / C) = 160 条代码记录
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
$D$演示栈的 push / pop / clear 基本操作，体现 LIFO（后进先出）特性。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'python', $C$import ds4viz as dv

with dv.stack(label="demo_stack") as s:
    s.push(1)
    s.push(2)
    s.push(3)
    s.pop()
    s.clear()
$C$, $E$### Python 实现

基础 push / pop / clear。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'lua', $C$local ds4viz = require("ds4viz")

ds4viz.withContext(ds4viz.stack("demo_stack"), function(s)
    s:push(1)
    s:push(2)
    s:push(3)
    s:pop()
    s:clear()
end)
$C$, $E$### Lua 实现

使用 `withContext` 包裹栈作用域。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'rust', $C$use ds4viz::prelude::*;

fn main() -> ds4viz::Result<()> {
    ds4viz::stack("demo_stack", |s| {
        s.push(1)?;
        s.push(2)?;
        s.push(3)?;
        s.pop()?;
        s.clear()?;
        Ok(())
    })
}
$C$, $E$### Rust 实现

所有操作返回 `Result`。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'c', $C$#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"

int main(void) {
    dvStack(s, "demo_stack") {
        dvStackPush(s, 1);
        dvStackPush(s, 2);
        dvStackPush(s, 3);
        dvStackPop(s);
        dvStackClear(s);
    }
    return 0;
}
$C$, $E$### C 实现

使用单头文件 C API。$E$);

-- [2] 括号匹配检验
INSERT INTO templates (title, category, description) VALUES (
'括号匹配检验', '栈',
$D$使用栈模拟括号匹配：左括号入栈，右括号出栈。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'python', $C$import ds4viz as dv

brackets = "({[]})[({})]"

with dv.stack(label="bracket_check") as s:
    for ch in brackets:
        if ch in "({[":
            s.push(ch)
        elif ch in ")}]":
            s.pop()
$C$, $E$### Python 实现

典型括号匹配栈模型。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'lua', $C$local ds4viz = require("ds4viz")
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
end)
$C$, $E$### Lua 实现

逐字符判断并执行 push/pop。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'rust', $C$use ds4viz::prelude::*;

fn main() -> ds4viz::Result<()> {
    let brackets = "({[]})[({})]";
    ds4viz::stack("bracket_check", |s| {
        for ch in brackets.chars() {
            match ch {
                '(' | '{' | '[' => s.push(ch.to_string())?,
                ')' | '}' | ']' => s.pop()?,
                _ => {}
            }
        }
        Ok(())
    })
}
$C$, $E$### Rust 实现

`match` 分派括号类型。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'c', $C$#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"

int main(void) {
    const char* brackets = "({[]})[({})]";
    dvStack(s, "bracket_check") {
        for (const char* p = brackets; *p; ++p) {
            char ch = *p;
            if (ch == '(' || ch == '{' || ch == '[') {
                char buf[2] = { ch, '\0' };
                dvStackPush(s, buf);
            } else if (ch == ')' || ch == '}' || ch == ']') {
                dvStackPop(s);
            }
        }
    }
    return 0;
}
$C$, $E$### C 实现

左括号入栈，右括号弹栈。$E$);

-- [3] 十进制转二进制
INSERT INTO templates (title, category, description) VALUES (
'十进制转二进制', '栈',
$D$利用“取余入栈、依次出栈”演示二进制转换过程。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'python', $C$import ds4viz as dv

n = 42
with dv.stack(label="dec_to_bin") as s:
    cnt = 0
    while n > 0:
        s.push(n % 2)
        n //= 2
        cnt += 1
    for _ in range(cnt):
        s.pop()
$C$, $E$### Python 实现

低位先入栈，高位后弹出。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'lua', $C$local ds4viz = require("ds4viz")
local n = 42

ds4viz.withContext(ds4viz.stack("dec_to_bin"), function(s)
    local cnt = 0
    while n > 0 do
        s:push(n % 2)
        n = math.floor(n / 2)
        cnt = cnt + 1
    end
    for _ = 1, cnt do
        s:pop()
    end
end)
$C$, $E$### Lua 实现

使用 `math.floor` 实现整除。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'rust', $C$use ds4viz::prelude::*;
fn main() -> ds4viz::Result<()> {
    let mut n: i64 = 42;
    ds4viz::stack("dec_to_bin", |s| {
        let mut cnt = 0;
        while n > 0 {
            s.push(n % 2)?;
            n /= 2;
            cnt += 1;
        }
        for _ in 0..cnt {
            s.pop()?;
        }
        Ok(())
    })
}
$C$, $E$### Rust 实现

整除与取余完成进制拆解。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'c', $C$#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"

int main(void) {
    int n = 42, cnt = 0;
    dvStack(s, "dec_to_bin") {
        while (n > 0) {
            dvStackPush(s, n % 2);
            n /= 2;
            cnt++;
        }
        for (int i = 0; i < cnt; i++) {
            dvStackPop(s);
        }
    }
    return 0;
}
$C$, $E$### C 实现

经典“取余 + 出栈”转换。$E$);

-- =====================================================
-- 分类: 队列
-- =====================================================

-- [4] 队列的基本操作
INSERT INTO templates (title, category, description) VALUES (
'队列的基本操作', '队列',
$D$演示 enqueue / dequeue / clear，体现 FIFO（先进先出）。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'python', $C$import ds4viz as dv

with dv.queue(label="demo_queue") as q:
    q.enqueue(10)
    q.enqueue(20)
    q.enqueue(30)
    q.dequeue()
    q.clear()
$C$, $E$### Python 实现

入队、出队、清空。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'lua', $C$local ds4viz = require("ds4viz")
ds4viz.withContext(ds4viz.queue("demo_queue"), function(q)
    q:enqueue(10)
    q:enqueue(20)
    q:enqueue(30)
    q:dequeue()
    q:clear()
end)
$C$, $E$### Lua 实现

FIFO 基本操作。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'rust', $C$use ds4viz::prelude::*;
fn main() -> ds4viz::Result<()> {
    ds4viz::queue("demo_queue", |q| {
        q.enqueue(10)?;
        q.enqueue(20)?;
        q.enqueue(30)?;
        q.dequeue()?;
        q.clear()?;
        Ok(())
    })
}
$C$, $E$### Rust 实现

基础队列操作模板。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'c', $C$#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"

int main(void) {
    dvQueue(q, "demo_queue") {
        dvQueueEnqueue(q, 10);
        dvQueueEnqueue(q, 20);
        dvQueueEnqueue(q, 30);
        dvQueueDequeue(q);
        dvQueueClear(q);
    }
    return 0;
}
$C$, $E$### C 实现

队列 API 对应 C 版本。$E$);

-- [5] 约瑟夫环问题
INSERT INTO templates (title, category, description) VALUES (
'约瑟夫环问题', '队列',
$D$使用队列模拟“报数淘汰”的轮转过程。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'python', $C$import ds4viz as dv

with dv.queue(label="josephus") as q:
    for i in range(1, 7):
        q.enqueue(i)

    for _ in range(5):
        q.dequeue(); q.enqueue(0)  # 占位演示跳过
        q.dequeue(); q.enqueue(0)
        q.dequeue()                # 淘汰
$C$, $E$### Python 实现

以队列轮转演示约瑟夫思想。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'lua', $C$local ds4viz = require("ds4viz")
ds4viz.withContext(ds4viz.queue("josephus"), function(q)
    for i = 1, 6 do q:enqueue(i) end
    for _ = 1, 5 do
        q:dequeue(); q:enqueue(0)
        q:dequeue(); q:enqueue(0)
        q:dequeue()
    end
end)
$C$, $E$### Lua 实现

轮转 + 淘汰流程。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'rust', $C$use ds4viz::prelude::*;
fn main() -> ds4viz::Result<()> {
    ds4viz::queue("josephus", |q| {
        for i in 1..=6 { q.enqueue(i)?; }
        for _ in 0..5 {
            q.dequeue()?; q.enqueue(0)?;
            q.dequeue()?; q.enqueue(0)?;
            q.dequeue()?;
        }
        Ok(())
    })
}
$C$, $E$### Rust 实现

简化版约瑟夫过程。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'c', $C$#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"

int main(void) {
    dvQueue(q, "josephus") {
        for (int i = 1; i <= 6; i++) dvQueueEnqueue(q, i);
        for (int k = 0; k < 5; k++) {
            dvQueueDequeue(q); dvQueueEnqueue(q, 0);
            dvQueueDequeue(q); dvQueueEnqueue(q, 0);
            dvQueueDequeue(q);
        }
    }
    return 0;
}
$C$, $E$### C 实现

队列轮转淘汰示意。$E$);

-- [6] 打印任务队列
INSERT INTO templates (title, category, description) VALUES (
'打印任务队列', '队列',
$D$模拟打印任务到达与处理过程，展示 FCFS 调度。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'python', $C$import ds4viz as dv

with dv.queue(label="print_queue") as q:
    q.enqueue("report.pdf")
    q.enqueue("photo.png")
    q.dequeue()
    q.enqueue("slides.pptx")
    q.dequeue()
    q.dequeue()
$C$, $E$### Python 实现

字符串任务队列。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'lua', $C$local ds4viz = require("ds4viz")
ds4viz.withContext(ds4viz.queue("print_queue"), function(q)
    q:enqueue("report.pdf")
    q:enqueue("photo.png")
    q:dequeue()
    q:enqueue("slides.pptx")
    q:dequeue()
    q:dequeue()
end)
$C$, $E$### Lua 实现

打印任务排队示例。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'rust', $C$use ds4viz::prelude::*;
fn main() -> ds4viz::Result<()> {
    ds4viz::queue("print_queue", |q| {
        q.enqueue("report.pdf")?;
        q.enqueue("photo.png")?;
        q.dequeue()?;
        q.enqueue("slides.pptx")?;
        q.dequeue()?;
        q.dequeue()?;
        Ok(())
    })
}
$C$, $E$### Rust 实现

任务入队与出队。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'c', $C$#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"

int main(void) {
    dvQueue(q, "print_queue") {
        dvQueueEnqueue(q, "report.pdf");
        dvQueueEnqueue(q, "photo.png");
        dvQueueDequeue(q);
        dvQueueEnqueue(q, "slides.pptx");
        dvQueueDequeue(q);
        dvQueueDequeue(q);
    }
    return 0;
}
$C$, $E$### C 实现

队列在任务调度中的应用。$E$);

-- =====================================================
-- 分类: 单链表
-- =====================================================

-- [7] 单链表的构建
INSERT INTO templates (title, category, description) VALUES (
'单链表的构建', '单链表',
$D$演示头插法与尾插法构建单链表。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'python', $C$import ds4viz as dv
with dv.single_linked_list(label="build_slist") as l:
    l.insert_head(30)
    l.insert_head(20)
    l.insert_tail(40)
    l.insert_tail(50)
$C$, $E$### Python 实现

头插 + 尾插组合。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'lua', $C$local ds4viz = require("ds4viz")
ds4viz.withContext(ds4viz.singleLinkedList("build_slist"), function(l)
    l:insertHead(30)
    l:insertHead(20)
    l:insertTail(40)
    l:insertTail(50)
end)
$C$, $E$### Lua 实现

构建链表示例。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'rust', $C$use ds4viz::prelude::*;
fn main() -> ds4viz::Result<()> {
    ds4viz::single_linked_list("build_slist", |l| {
        l.insert_head(30)?;
        l.insert_head(20)?;
        l.insert_tail(40)?;
        l.insert_tail(50)?;
        Ok(())
    })
}
$C$, $E$### Rust 实现

基本构建流程。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'c', $C$#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"
int main(void){
    dvSingleLinkedList(l, "build_slist"){
        dvSlInsertHead(l, 30);
        dvSlInsertHead(l, 20);
        dvSlInsertTail(l, 40);
        dvSlInsertTail(l, 50);
    }
    return 0;
}
$C$, $E$### C 实现

单链表构建。$E$);

-- [8] 单链表的插入与删除
INSERT INTO templates (title, category, description) VALUES (
'单链表的插入与删除', '单链表',
$D$演示 insert_after、delete、delete_head 等动态操作。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'python', $C$import ds4viz as dv
with dv.single_linked_list(label="modify_slist") as l:
    a = l.insert_head(10)
    b = l.insert_tail(20)
    c = l.insert_tail(40)
    l.insert_after(b, 30)
    l.delete(c)
    l.delete_head()
$C$, $E$### Python 实现

插入与删除组合操作。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'lua', $C$local ds4viz = require("ds4viz")
ds4viz.withContext(ds4viz.singleLinkedList("modify_slist"), function(l)
    local a = l:insertHead(10)
    local b = l:insertTail(20)
    local c = l:insertTail(40)
    l:insertAfter(b, 30)
    l:delete(c)
    l:deleteHead()
end)
$C$, $E$### Lua 实现

动态增删示意。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'rust', $C$use ds4viz::prelude::*;
fn main() -> ds4viz::Result<()> {
    ds4viz::single_linked_list("modify_slist", |l| {
        let _a = l.insert_head(10)?;
        let b = l.insert_tail(20)?;
        let c = l.insert_tail(40)?;
        l.insert_after(b, 30)?;
        l.delete(c)?;
        l.delete_head()?;
        Ok(())
    })
}
$C$, $E$### Rust 实现

链表节点按 id 操作。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'c', $C$#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"
int main(void){
    dvSingleLinkedList(l, "modify_slist"){
        int a = dvSlInsertHead(l, 10);
        int b = dvSlInsertTail(l, 20);
        int c = dvSlInsertTail(l, 40);
        (void)a;
        dvSlInsertAfter(l, b, 30);
        dvSlDelete(l, c);
        dvSlDeleteHead(l);
    }
    return 0;
}
$C$, $E$### C 实现

节点 ID 驱动的增删。$E$);

-- [9] 单链表反转
INSERT INTO templates (title, category, description) VALUES (
'单链表反转', '单链表',
$D$构建有序单链表后执行 reverse。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'python', $C$import ds4viz as dv
with dv.single_linked_list(label="reverse_slist") as l:
    l.insert_tail(1)
    l.insert_tail(2)
    l.insert_tail(3)
    l.insert_tail(4)
    l.reverse()
$C$, $E$### Python 实现

链表指向整体反转。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'lua', $C$local ds4viz = require("ds4viz")
ds4viz.withContext(ds4viz.singleLinkedList("reverse_slist"), function(l)
    l:insertTail(1)
    l:insertTail(2)
    l:insertTail(3)
    l:insertTail(4)
    l:reverse()
end)
$C$, $E$### Lua 实现

反转操作演示。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'rust', $C$use ds4viz::prelude::*;
fn main() -> ds4viz::Result<()> {
    ds4viz::single_linked_list("reverse_slist", |l| {
        l.insert_tail(1)?;
        l.insert_tail(2)?;
        l.insert_tail(3)?;
        l.insert_tail(4)?;
        l.reverse()?;
        Ok(())
    })
}
$C$, $E$### Rust 实现

`reverse()` 一步完成。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'c', $C$#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"
int main(void){
    dvSingleLinkedList(l, "reverse_slist"){
        dvSlInsertTail(l, 1);
        dvSlInsertTail(l, 2);
        dvSlInsertTail(l, 3);
        dvSlInsertTail(l, 4);
        dvSlReverse(l);
    }
    return 0;
}
$C$, $E$### C 实现

单链表反转。$E$);

-- =====================================================
-- 分类: 双向链表
-- =====================================================

-- [10] 双向链表的基本操作
INSERT INTO templates (title, category, description) VALUES (
'双向链表的基本操作', '双向链表',
$D$演示头尾插入、前后插入、头尾删除与按节点删除。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'python', $C$import ds4viz as dv
with dv.double_linked_list(label="demo_dlist") as l:
    a = l.insert_head(30)
    l.insert_head(10)
    l.insert_tail(50)
    l.insert_before(a, 20)
    l.insert_after(a, 40)
    l.delete(a)
    l.delete_head()
    l.delete_tail()
$C$, $E$### Python 实现

覆盖双向链表常见操作。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'lua', $C$local ds4viz = require("ds4viz")
ds4viz.withContext(ds4viz.doubleLinkedList("demo_dlist"), function(l)
    local a = l:insertHead(30)
    l:insertHead(10)
    l:insertTail(50)
    l:insertBefore(a, 20)
    l:insertAfter(a, 40)
    l:delete(a)
    l:deleteHead()
    l:deleteTail()
end)
$C$, $E$### Lua 实现

双向链表完整操作链。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'rust', $C$use ds4viz::prelude::*;
fn main() -> ds4viz::Result<()> {
    ds4viz::double_linked_list("demo_dlist", |l| {
        let a = l.insert_head(30)?;
        l.insert_head(10)?;
        l.insert_tail(50)?;
        l.insert_before(a, 20)?;
        l.insert_after(a, 40)?;
        l.delete(a)?;
        l.delete_head()?;
        l.delete_tail()?;
        Ok(())
    })
}
$C$, $E$### Rust 实现

操作函数均返回 `Result`。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'c', $C$#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"
int main(void){
    dvDoubleLinkedList(l, "demo_dlist"){
        int a = dvDlInsertHead(l, 30);
        dvDlInsertHead(l, 10);
        dvDlInsertTail(l, 50);
        dvDlInsertBefore(l, a, 20);
        dvDlInsertAfter(l, a, 40);
        dvDlDelete(l, a);
        dvDlDeleteHead(l);
        dvDlDeleteTail(l);
    }
    return 0;
}
$C$, $E$### C 实现

双向链表基础教学模板。$E$);

-- [11] 双向链表反转
INSERT INTO templates (title, category, description) VALUES (
'双向链表反转', '双向链表',
$D$构建链表后执行 reverse，观察 prev/next 对调效果。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'python', $C$import ds4viz as dv
with dv.double_linked_list(label="reverse_dlist") as l:
    l.insert_tail(1)
    l.insert_tail(2)
    l.insert_tail(3)
    l.insert_tail(4)
    l.reverse()
$C$, $E$### Python 实现

双向链表反转。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'lua', $C$local ds4viz = require("ds4viz")
ds4viz.withContext(ds4viz.doubleLinkedList("reverse_dlist"), function(l)
    l:insertTail(1)
    l:insertTail(2)
    l:insertTail(3)
    l:insertTail(4)
    l:reverse()
end)
$C$, $E$### Lua 实现

反转示例。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'rust', $C$use ds4viz::prelude::*;
fn main() -> ds4viz::Result<()> {
    ds4viz::double_linked_list("reverse_dlist", |l| {
        l.insert_tail(1)?;
        l.insert_tail(2)?;
        l.insert_tail(3)?;
        l.insert_tail(4)?;
        l.reverse()?;
        Ok(())
    })
}
$C$, $E$### Rust 实现

调用 `reverse()`。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'c', $C$#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"
int main(void){
    dvDoubleLinkedList(l, "reverse_dlist"){
        dvDlInsertTail(l, 1);
        dvDlInsertTail(l, 2);
        dvDlInsertTail(l, 3);
        dvDlInsertTail(l, 4);
        dvDlReverse(l);
    }
    return 0;
}
$C$, $E$### C 实现

双向链表反转操作。$E$);

-- [12] 双端队列模拟
INSERT INTO templates (title, category, description) VALUES (
'双端队列模拟', '双向链表',
$D$利用双向链表头尾 O(1) 操作模拟 deque。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'python', $C$import ds4viz as dv
with dv.double_linked_list(label="deque_sim") as l:
    l.insert_tail(1)
    l.insert_tail(2)
    l.insert_head(0)
    l.delete_head()
    l.delete_tail()
$C$, $E$### Python 实现

头尾双端操作。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'lua', $C$local ds4viz = require("ds4viz")
ds4viz.withContext(ds4viz.doubleLinkedList("deque_sim"), function(l)
    l:insertTail(1)
    l:insertTail(2)
    l:insertHead(0)
    l:deleteHead()
    l:deleteTail()
end)
$C$, $E$### Lua 实现

deque 行为模拟。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'rust', $C$use ds4viz::prelude::*;
fn main() -> ds4viz::Result<()> {
    ds4viz::double_linked_list("deque_sim", |l| {
        l.insert_tail(1)?;
        l.insert_tail(2)?;
        l.insert_head(0)?;
        l.delete_head()?;
        l.delete_tail()?;
        Ok(())
    })
}
$C$, $E$### Rust 实现

双端入队/出队。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'c', $C$#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"
int main(void){
    dvDoubleLinkedList(l, "deque_sim"){
        dvDlInsertTail(l, 1);
        dvDlInsertTail(l, 2);
        dvDlInsertHead(l, 0);
        dvDlDeleteHead(l);
        dvDlDeleteTail(l);
    }
    return 0;
}
$C$, $E$### C 实现

双端队列概念演示。$E$);

-- =====================================================
-- 分类: 二叉树
-- =====================================================

-- [13] 二叉树的手动构建
INSERT INTO templates (title, category, description) VALUES (
'二叉树的手动构建', '二叉树',
$D$通过 insert_root / insert_left / insert_right 手动构建满二叉树。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'python', $C$import ds4viz as dv
with dv.binary_tree(label="manual_tree") as t:
    r = t.insert_root(1)
    l = t.insert_left(r, 2)
    rr = t.insert_right(r, 3)
    t.insert_left(l, 4)
    t.insert_right(l, 5)
    t.insert_left(rr, 6)
    t.insert_right(rr, 7)
$C$, $E$### Python 实现

手动构建三层二叉树。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'lua', $C$local ds4viz = require("ds4viz")
ds4viz.withContext(ds4viz.binaryTree("manual_tree"), function(t)
    local r = t:insertRoot(1)
    local l = t:insertLeft(r, 2)
    local rr = t:insertRight(r, 3)
    t:insertLeft(l, 4)
    t:insertRight(l, 5)
    t:insertLeft(rr, 6)
    t:insertRight(rr, 7)
end)
$C$, $E$### Lua 实现

固定结构树。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'rust', $C$use ds4viz::prelude::*;
fn main() -> ds4viz::Result<()> {
    ds4viz::binary_tree("manual_tree", |t| {
        let r = t.insert_root(1)?;
        let l = t.insert_left(r, 2)?;
        let rr = t.insert_right(r, 3)?;
        t.insert_left(l, 4)?;
        t.insert_right(l, 5)?;
        t.insert_left(rr, 6)?;
        t.insert_right(rr, 7)?;
        Ok(())
    })
}
$C$, $E$### Rust 实现

层次构建。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'c', $C$#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"
int main(void){
    dvBinaryTree(t, "manual_tree"){
        int r = dvBtInsertRoot(t, 1);
        int l = dvBtInsertLeft(t, r, 2);
        int rr = dvBtInsertRight(t, r, 3);
        dvBtInsertLeft(t, l, 4);
        dvBtInsertRight(t, l, 5);
        dvBtInsertLeft(t, rr, 6);
        dvBtInsertRight(t, rr, 7);
    }
    return 0;
}
$C$, $E$### C 实现

二叉树手动构建模板。$E$);

-- [14] 表达式树
INSERT INTO templates (title, category, description) VALUES (
'表达式树', '二叉树',
$D$构建表达式 `(3 + 5) * 2` 对应的二叉表达式树。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'python', $C$import ds4viz as dv
with dv.binary_tree(label="expr_tree") as t:
    r = t.insert_root("*")
    p = t.insert_left(r, "+")
    t.insert_right(r, 2)
    t.insert_left(p, 3)
    t.insert_right(p, 5)
$C$, $E$### Python 实现

运算符作为内部节点。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'lua', $C$local ds4viz = require("ds4viz")
ds4viz.withContext(ds4viz.binaryTree("expr_tree"), function(t)
    local r = t:insertRoot("*")
    local p = t:insertLeft(r, "+")
    t:insertRight(r, 2)
    t:insertLeft(p, 3)
    t:insertRight(p, 5)
end)
$C$, $E$### Lua 实现

表达式树教学。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'rust', $C$use ds4viz::prelude::*;
fn main() -> ds4viz::Result<()> {
    ds4viz::binary_tree("expr_tree", |t| {
        let r = t.insert_root("*")?;
        let p = t.insert_left(r, "+")?;
        t.insert_right(r, 2)?;
        t.insert_left(p, 3)?;
        t.insert_right(p, 5)?;
        Ok(())
    })
}
$C$, $E$### Rust 实现

字符串与数字混合值。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'c', $C$#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"
int main(void){
    dvBinaryTree(t, "expr_tree"){
        int r = dvBtInsertRoot(t, "*");
        int p = dvBtInsertLeft(t, r, "+");
        dvBtInsertRight(t, r, 2);
        dvBtInsertLeft(t, p, 3);
        dvBtInsertRight(t, p, 5);
    }
    return 0;
}
$C$, $E$### C 实现

表达式树 C 版本。$E$);

-- [15] 二叉树的节点修改与删除
INSERT INTO templates (title, category, description) VALUES (
'二叉树的节点修改与删除', '二叉树',
$D$演示 update_value 与 delete（删除子树）操作。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'python', $C$import ds4viz as dv
with dv.binary_tree(label="modify_tree") as t:
    r = t.insert_root(1)
    a = t.insert_left(r, 2)
    b = t.insert_right(r, 3)
    t.insert_left(a, 4)
    t.insert_right(a, 5)
    t.update_value(b, 30)
    t.delete(a)
$C$, $E$### Python 实现

更新值并删除左子树。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'lua', $C$local ds4viz = require("ds4viz")
ds4viz.withContext(ds4viz.binaryTree("modify_tree"), function(t)
    local r = t:insertRoot(1)
    local a = t:insertLeft(r, 2)
    local b = t:insertRight(r, 3)
    t:insertLeft(a, 4)
    t:insertRight(a, 5)
    t:updateValue(b, 30)
    t:delete(a)
end)
$C$, $E$### Lua 实现

修改与子树删除。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'rust', $C$use ds4viz::prelude::*;
fn main() -> ds4viz::Result<()> {
    ds4viz::binary_tree("modify_tree", |t| {
        let r = t.insert_root(1)?;
        let a = t.insert_left(r, 2)?;
        let b = t.insert_right(r, 3)?;
        t.insert_left(a, 4)?;
        t.insert_right(a, 5)?;
        t.update_value(b, 30)?;
        t.delete(a)?;
        Ok(())
    })
}
$C$, $E$### Rust 实现

典型动态修改场景。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'c', $C$#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"
int main(void){
    dvBinaryTree(t, "modify_tree"){
        int r = dvBtInsertRoot(t, 1);
        int a = dvBtInsertLeft(t, r, 2);
        int b = dvBtInsertRight(t, r, 3);
        dvBtInsertLeft(t, a, 4);
        dvBtInsertRight(t, a, 5);
        dvBtUpdateValue(t, b, 30);
        dvBtDelete(t, a);
    }
    return 0;
}
$C$, $E$### C 实现

节点值修改与子树删除。$E$);

-- =====================================================
-- 分类: 二叉搜索树
-- =====================================================

-- [16] BST 的逐步构建
INSERT INTO templates (title, category, description) VALUES (
'BST的逐步构建', '二叉搜索树',
$D$按序插入节点，观察 BST 自动保持有序性质。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'python', $C$import ds4viz as dv
with dv.binary_search_tree(label="build_bst") as b:
    for v in [50, 30, 70, 20, 40, 60, 80]:
        b.insert(v)
$C$, $E$### Python 实现

标准 BST 构建序列。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'lua', $C$local ds4viz = require("ds4viz")
ds4viz.withContext(ds4viz.binarySearchTree("build_bst"), function(b)
    b:insert(50); b:insert(30); b:insert(70)
    b:insert(20); b:insert(40); b:insert(60); b:insert(80)
end)
$C$, $E$### Lua 实现

按值插入。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'rust', $C$use ds4viz::prelude::*;
fn main() -> ds4viz::Result<()> {
    ds4viz::binary_search_tree("build_bst", |b| {
        for v in [50,30,70,20,40,60,80] {
            b.insert(v)?;
        }
        Ok(())
    })
}
$C$, $E$### Rust 实现

`insert` 仅接收数值。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'c', $C$#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"
int main(void){
    dvBinarySearchTree(b, "build_bst"){
        dvBstInsert(b, 50); dvBstInsert(b, 30); dvBstInsert(b, 70);
        dvBstInsert(b, 20); dvBstInsert(b, 40); dvBstInsert(b, 60); dvBstInsert(b, 80);
    }
    return 0;
}
$C$, $E$### C 实现

BST 插入构建。$E$);

-- [17] BST 的删除操作
INSERT INTO templates (title, category, description) VALUES (
'BST的删除操作', '二叉搜索树',
$D$演示删除叶子、单子节点、双子节点三种情况。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'python', $C$import ds4viz as dv
with dv.binary_search_tree(label="delete_bst") as b:
    for v in [50, 30, 70, 20, 40, 60, 80]:
        b.insert(v)
    b.delete(20)
    b.delete(70)
    b.delete(30)
$C$, $E$### Python 实现

覆盖常见删除分支。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'lua', $C$local ds4viz = require("ds4viz")
ds4viz.withContext(ds4viz.binarySearchTree("delete_bst"), function(b)
    b:insert(50); b:insert(30); b:insert(70)
    b:insert(20); b:insert(40); b:insert(60); b:insert(80)
    b:delete(20); b:delete(70); b:delete(30)
end)
$C$, $E$### Lua 实现

BST 删除教学示例。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'rust', $C$use ds4viz::prelude::*;
fn main() -> ds4viz::Result<()> {
    ds4viz::binary_search_tree("delete_bst", |b| {
        for v in [50,30,70,20,40,60,80] { b.insert(v)?; }
        b.delete(20)?;
        b.delete(70)?;
        b.delete(30)?;
        Ok(())
    })
}
$C$, $E$### Rust 实现

按值删除节点。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'c', $C$#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"
int main(void){
    dvBinarySearchTree(b, "delete_bst"){
        dvBstInsert(b, 50); dvBstInsert(b, 30); dvBstInsert(b, 70);
        dvBstInsert(b, 20); dvBstInsert(b, 40); dvBstInsert(b, 60); dvBstInsert(b, 80);
        dvBstDelete(b, 20);
        dvBstDelete(b, 70);
        dvBstDelete(b, 30);
    }
    return 0;
}
$C$, $E$### C 实现

BST 删除三类场景。$E$);

-- [18] BST 的有序插入退化
INSERT INTO templates (title, category, description) VALUES (
'BST的有序插入退化', '二叉搜索树',
$D$按升序连续插入，展示 BST 退化为链表。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'python', $C$import ds4viz as dv
with dv.binary_search_tree(label="degenerate_bst") as b:
    for i in range(1, 9):
        b.insert(i)
$C$, $E$### Python 实现

升序插入导致高度退化。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'lua', $C$local ds4viz = require("ds4viz")
ds4viz.withContext(ds4viz.binarySearchTree("degenerate_bst"), function(b)
    for i = 1, 8 do b:insert(i) end
end)
$C$, $E$### Lua 实现

退化示意。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'rust', $C$use ds4viz::prelude::*;
fn main() -> ds4viz::Result<()> {
    ds4viz::binary_search_tree("degenerate_bst", |b| {
        for i in 1..=8 { b.insert(i)?; }
        Ok(())
    })
}
$C$, $E$### Rust 实现

右斜树形态。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'c', $C$#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"
int main(void){
    dvBinarySearchTree(b, "degenerate_bst"){
        for (int i = 1; i <= 8; i++) dvBstInsert(b, i);
    }
    return 0;
}
$C$, $E$### C 实现

BST 退化示意。$E$);

-- =====================================================
-- 分类: 堆
-- =====================================================

-- [19] 最小堆操作演示
INSERT INTO templates (title, category, description) VALUES (
'最小堆操作演示', '堆',
$D$最小堆的插入与提取最小值过程。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'python', $C$import ds4viz as dv
with dv.heap(label="min_heap", heap_type="min") as h:
    for v in [10, 5, 15, 3, 8]:
        h.insert(v)
    h.extract()
    h.extract()
$C$, $E$### Python 实现

最小堆提取堆顶。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'lua', $C$local ds4viz = require("ds4viz")
ds4viz.withContext(ds4viz.heap("min_heap", "min"), function(h)
    h:insert(10); h:insert(5); h:insert(15); h:insert(3); h:insert(8)
    h:extract()
    h:extract()
end)
$C$, $E$### Lua 实现

最小堆。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'rust', $C$use ds4viz::prelude::*;
fn main() -> ds4viz::Result<()> {
    ds4viz::heap("min_heap", HeapType::Min, |h| {
        for v in [10,5,15,3,8] { h.insert(v)?; }
        h.extract()?;
        h.extract()?;
        Ok(())
    })
}
$C$, $E$### Rust 实现

`HeapType::Min`。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'c', $C$#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"
int main(void){
    dvHeap(h, "min_heap", dvHeapOrderMin){
        dvHeapInsert(h, 10);
        dvHeapInsert(h, 5);
        dvHeapInsert(h, 15);
        dvHeapInsert(h, 3);
        dvHeapInsert(h, 8);
        dvHeapExtract(h);
        dvHeapExtract(h);
    }
    return 0;
}
$C$, $E$### C 实现

最小堆示例。$E$);

-- [20] 最大堆操作演示
INSERT INTO templates (title, category, description) VALUES (
'最大堆操作演示', '堆',
$D$最大堆的插入与提取最大值过程。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'python', $C$import ds4viz as dv
with dv.heap(label="max_heap", heap_type="max") as h:
    for v in [10, 5, 15, 3, 20]:
        h.insert(v)
    h.extract()
    h.extract()
$C$, $E$### Python 实现

最大堆提取堆顶。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'lua', $C$local ds4viz = require("ds4viz")
ds4viz.withContext(ds4viz.heap("max_heap", "max"), function(h)
    h:insert(10); h:insert(5); h:insert(15); h:insert(3); h:insert(20)
    h:extract()
    h:extract()
end)
$C$, $E$### Lua 实现

最大堆。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'rust', $C$use ds4viz::prelude::*;
fn main() -> ds4viz::Result<()> {
    ds4viz::heap("max_heap", HeapType::Max, |h| {
        for v in [10,5,15,3,20] { h.insert(v)?; }
        h.extract()?;
        h.extract()?;
        Ok(())
    })
}
$C$, $E$### Rust 实现

`HeapType::Max`。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'c', $C$#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"
int main(void){
    dvHeap(h, "max_heap", dvHeapOrderMax){
        dvHeapInsert(h, 10);
        dvHeapInsert(h, 5);
        dvHeapInsert(h, 15);
        dvHeapInsert(h, 3);
        dvHeapInsert(h, 20);
        dvHeapExtract(h);
        dvHeapExtract(h);
    }
    return 0;
}
$C$, $E$### C 实现

最大堆示例。$E$);

-- [21] 堆排序过程
INSERT INTO templates (title, category, description) VALUES (
'堆排序过程', '堆',
$D$先建最小堆，再反复 extract，演示升序输出。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'python', $C$import ds4viz as dv
data = [38, 27, 43, 3, 9, 82, 10]
with dv.heap(label="heap_sort", heap_type="min") as h:
    for v in data:
        h.insert(v)
    for _ in data:
        h.extract()
$C$, $E$### Python 实现

最小堆排序示意。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'lua', $C$local ds4viz = require("ds4viz")
local data = {38, 27, 43, 3, 9, 82, 10}
ds4viz.withContext(ds4viz.heap("heap_sort", "min"), function(h)
    for _, v in ipairs(data) do h:insert(v) end
    for _ = 1, #data do h:extract() end
end)
$C$, $E$### Lua 实现

建堆 + 提取。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'rust', $C$use ds4viz::prelude::*;
fn main() -> ds4viz::Result<()> {
    let data = [38, 27, 43, 3, 9, 82, 10];
    ds4viz::heap("heap_sort", HeapType::Min, |h| {
        for &v in &data { h.insert(v)?; }
        for _ in 0..data.len() { h.extract()?; }
        Ok(())
    })
}
$C$, $E$### Rust 实现

堆排序过程可视化。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'c', $C$#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"
int main(void){
    int data[] = {38, 27, 43, 3, 9, 82, 10};
    int n = (int)(sizeof(data)/sizeof(data[0]));
    dvHeap(h, "heap_sort", dvHeapOrderMin){
        for (int i = 0; i < n; i++) dvHeapInsert(h, data[i]);
        for (int i = 0; i < n; i++) dvHeapExtract(h);
    }
    return 0;
}
$C$, $E$### C 实现

最小堆排序流程。$E$);

-- =====================================================
-- 分类: 无向图
-- =====================================================

-- [22] 无向图的构建
INSERT INTO templates (title, category, description) VALUES (
'无向图的构建', '无向图',
$D$添加节点与无向边，构建连通图。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'python', $C$import ds4viz as dv
with dv.graph_undirected(label="simple_graph") as g:
    g.add_node(0, "A"); g.add_node(1, "B"); g.add_node(2, "C")
    g.add_node(3, "D"); g.add_node(4, "E"); g.add_node(5, "F")
    g.add_edge(0, 1); g.add_edge(0, 2); g.add_edge(1, 2)
    g.add_edge(1, 3); g.add_edge(2, 4); g.add_edge(3, 5); g.add_edge(4, 5)
$C$, $E$### Python 实现

标准无向图构建。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'lua', $C$local ds4viz = require("ds4viz")
ds4viz.withContext(ds4viz.graphUndirected("simple_graph"), function(g)
    g:addNode(0, "A"); g:addNode(1, "B"); g:addNode(2, "C")
    g:addNode(3, "D"); g:addNode(4, "E"); g:addNode(5, "F")
    g:addEdge(0, 1); g:addEdge(0, 2); g:addEdge(1, 2)
    g:addEdge(1, 3); g:addEdge(2, 4); g:addEdge(3, 5); g:addEdge(4, 5)
end)
$C$, $E$### Lua 实现

无向边示例。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'rust', $C$use ds4viz::prelude::*;
fn main() -> ds4viz::Result<()> {
    ds4viz::graph_undirected("simple_graph", |g| {
        g.add_node(0, "A")?; g.add_node(1, "B")?; g.add_node(2, "C")?;
        g.add_node(3, "D")?; g.add_node(4, "E")?; g.add_node(5, "F")?;
        g.add_edge(0, 1)?; g.add_edge(0, 2)?; g.add_edge(1, 2)?;
        g.add_edge(1, 3)?; g.add_edge(2, 4)?; g.add_edge(3, 5)?; g.add_edge(4, 5)?;
        Ok(())
    })
}
$C$, $E$### Rust 实现

连通无向图。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'c', $C$#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"
int main(void){
    dvGraphUndirected(g, "simple_graph"){
        dvGuAddNode(g, 0, "A"); dvGuAddNode(g, 1, "B"); dvGuAddNode(g, 2, "C");
        dvGuAddNode(g, 3, "D"); dvGuAddNode(g, 4, "E"); dvGuAddNode(g, 5, "F");
        dvGuAddEdge(g, 0, 1); dvGuAddEdge(g, 0, 2); dvGuAddEdge(g, 1, 2);
        dvGuAddEdge(g, 1, 3); dvGuAddEdge(g, 2, 4); dvGuAddEdge(g, 3, 5); dvGuAddEdge(g, 4, 5);
    }
    return 0;
}
$C$, $E$### C 实现

无向图 C API。$E$);

-- [23] 无向图的动态修改
INSERT INTO templates (title, category, description) VALUES (
'无向图的动态修改', '无向图',
$D$演示更新标签、删边、删点与增边。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'python', $C$import ds4viz as dv
with dv.graph_undirected(label="dynamic_graph") as g:
    g.add_node(0, "A"); g.add_node(1, "B"); g.add_node(2, "C"); g.add_node(3, "D")
    g.add_edge(0, 1); g.add_edge(1, 2); g.add_edge(2, 3); g.add_edge(0, 3)
    g.update_node_label(0, "Alpha")
    g.remove_edge(0, 3)
    g.add_node(4, "E")
    g.add_edge(3, 4)
    g.remove_node(1)
$C$, $E$### Python 实现

图结构动态维护。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'lua', $C$local ds4viz = require("ds4viz")
ds4viz.withContext(ds4viz.graphUndirected("dynamic_graph"), function(g)
    g:addNode(0, "A"); g:addNode(1, "B"); g:addNode(2, "C"); g:addNode(3, "D")
    g:addEdge(0, 1); g:addEdge(1, 2); g:addEdge(2, 3); g:addEdge(0, 3)
    g:updateNodeLabel(0, "Alpha")
    g:removeEdge(0, 3)
    g:addNode(4, "E")
    g:addEdge(3, 4)
    g:removeNode(1)
end)
$C$, $E$### Lua 实现

增删改全覆盖。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'rust', $C$use ds4viz::prelude::*;
fn main() -> ds4viz::Result<()> {
    ds4viz::graph_undirected("dynamic_graph", |g| {
        g.add_node(0, "A")?; g.add_node(1, "B")?; g.add_node(2, "C")?; g.add_node(3, "D")?;
        g.add_edge(0, 1)?; g.add_edge(1, 2)?; g.add_edge(2, 3)?; g.add_edge(0, 3)?;
        g.update_node_label(0, "Alpha")?;
        g.remove_edge(0, 3)?;
        g.add_node(4, "E")?;
        g.add_edge(3, 4)?;
        g.remove_node(1)?;
        Ok(())
    })
}
$C$, $E$### Rust 实现

动态图模板。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'c', $C$#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"
int main(void){
    dvGraphUndirected(g, "dynamic_graph"){
        dvGuAddNode(g, 0, "A"); dvGuAddNode(g, 1, "B"); dvGuAddNode(g, 2, "C"); dvGuAddNode(g, 3, "D");
        dvGuAddEdge(g, 0, 1); dvGuAddEdge(g, 1, 2); dvGuAddEdge(g, 2, 3); dvGuAddEdge(g, 0, 3);
        dvGuUpdateNodeLabel(g, 0, "Alpha");
        dvGuRemoveEdge(g, 0, 3);
        dvGuAddNode(g, 4, "E");
        dvGuAddEdge(g, 3, 4);
        dvGuRemoveNode(g, 1);
    }
    return 0;
}
$C$, $E$### C 实现

无向图动态变更。$E$);

-- [24] 彼得森图
INSERT INTO templates (title, category, description) VALUES (
'彼得森图', '无向图',
$D$构建经典彼得森图（10 节点、15 边）用于图论教学。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'python', $C$import ds4viz as dv
with dv.graph_undirected(label="petersen_graph") as g:
    for i in range(5):
        g.add_node(i, f"o{i}")
    for i in range(5):
        g.add_node(i + 5, f"i{i}")
    g.add_edge(0,1); g.add_edge(1,2); g.add_edge(2,3); g.add_edge(3,4); g.add_edge(4,0)
    g.add_edge(0,5); g.add_edge(1,6); g.add_edge(2,7); g.add_edge(3,8); g.add_edge(4,9)
    g.add_edge(5,7); g.add_edge(7,9); g.add_edge(9,6); g.add_edge(6,8); g.add_edge(8,5)
$C$, $E$### Python 实现

经典图论基准图。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'lua', $C$local ds4viz = require("ds4viz")
ds4viz.withContext(ds4viz.graphUndirected("petersen_graph"), function(g)
    for i = 0, 4 do g:addNode(i, "o" .. i) end
    for i = 0, 4 do g:addNode(i + 5, "i" .. i) end
    g:addEdge(0,1); g:addEdge(1,2); g:addEdge(2,3); g:addEdge(3,4); g:addEdge(4,0)
    g:addEdge(0,5); g:addEdge(1,6); g:addEdge(2,7); g:addEdge(3,8); g:addEdge(4,9)
    g:addEdge(5,7); g:addEdge(7,9); g:addEdge(9,6); g:addEdge(6,8); g:addEdge(8,5)
end)
$C$, $E$### Lua 实现

彼得森图全边集。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'rust', $C$use ds4viz::prelude::*;
fn main() -> ds4viz::Result<()> {
    ds4viz::graph_undirected("petersen_graph", |g| {
        for i in 0..5 { g.add_node(i, &format!("o{}", i))?; }
        for i in 0..5 { g.add_node(i + 5, &format!("i{}", i))?; }
        g.add_edge(0,1)?; g.add_edge(1,2)?; g.add_edge(2,3)?; g.add_edge(3,4)?; g.add_edge(4,0)?;
        g.add_edge(0,5)?; g.add_edge(1,6)?; g.add_edge(2,7)?; g.add_edge(3,8)?; g.add_edge(4,9)?;
        g.add_edge(5,7)?; g.add_edge(7,9)?; g.add_edge(9,6)?; g.add_edge(6,8)?; g.add_edge(8,5)?;
        Ok(())
    })
}
$C$, $E$### Rust 实现

图论经典示例。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'c', $C$#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"
int main(void){
    dvGraphUndirected(g, "petersen_graph"){
        dvGuAddNode(g,0,"o0"); dvGuAddNode(g,1,"o1"); dvGuAddNode(g,2,"o2"); dvGuAddNode(g,3,"o3"); dvGuAddNode(g,4,"o4");
        dvGuAddNode(g,5,"i0"); dvGuAddNode(g,6,"i1"); dvGuAddNode(g,7,"i2"); dvGuAddNode(g,8,"i3"); dvGuAddNode(g,9,"i4");
        dvGuAddEdge(g,0,1); dvGuAddEdge(g,1,2); dvGuAddEdge(g,2,3); dvGuAddEdge(g,3,4); dvGuAddEdge(g,4,0);
        dvGuAddEdge(g,0,5); dvGuAddEdge(g,1,6); dvGuAddEdge(g,2,7); dvGuAddEdge(g,3,8); dvGuAddEdge(g,4,9);
        dvGuAddEdge(g,5,7); dvGuAddEdge(g,7,9); dvGuAddEdge(g,9,6); dvGuAddEdge(g,6,8); dvGuAddEdge(g,8,5);
    }
    return 0;
}
$C$, $E$### C 实现

彼得森图 C 版本。$E$);

-- =====================================================
-- 分类: 有向图
-- =====================================================

-- [25] 有向图的构建
INSERT INTO templates (title, category, description) VALUES (
'有向图的构建', '有向图',
$D$构建带有环路的有向图，体现方向性。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'python', $C$import ds4viz as dv
with dv.graph_directed(label="simple_digraph") as g:
    g.add_node(0, "A"); g.add_node(1, "B"); g.add_node(2, "C"); g.add_node(3, "D"); g.add_node(4, "E")
    g.add_edge(0, 1); g.add_edge(0, 2); g.add_edge(1, 3); g.add_edge(2, 3); g.add_edge(3, 4); g.add_edge(4, 0)
$C$, $E$### Python 实现

有向边 + 环路。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'lua', $C$local ds4viz = require("ds4viz")
ds4viz.withContext(ds4viz.graphDirected("simple_digraph"), function(g)
    g:addNode(0, "A"); g:addNode(1, "B"); g:addNode(2, "C"); g:addNode(3, "D"); g:addNode(4, "E")
    g:addEdge(0, 1); g:addEdge(0, 2); g:addEdge(1, 3); g:addEdge(2, 3); g:addEdge(3, 4); g:addEdge(4, 0)
end)
$C$, $E$### Lua 实现

方向敏感边集。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'rust', $C$use ds4viz::prelude::*;
fn main() -> ds4viz::Result<()> {
    ds4viz::graph_directed("simple_digraph", |g| {
        g.add_node(0, "A")?; g.add_node(1, "B")?; g.add_node(2, "C")?; g.add_node(3, "D")?; g.add_node(4, "E")?;
        g.add_edge(0, 1)?; g.add_edge(0, 2)?; g.add_edge(1, 3)?; g.add_edge(2, 3)?; g.add_edge(3, 4)?; g.add_edge(4, 0)?;
        Ok(())
    })
}
$C$, $E$### Rust 实现

有向图基础模板。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'c', $C$#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"
int main(void){
    dvGraphDirected(g, "simple_digraph"){
        dvGdAddNode(g,0,"A"); dvGdAddNode(g,1,"B"); dvGdAddNode(g,2,"C"); dvGdAddNode(g,3,"D"); dvGdAddNode(g,4,"E");
        dvGdAddEdge(g,0,1); dvGdAddEdge(g,0,2); dvGdAddEdge(g,1,3); dvGdAddEdge(g,2,3); dvGdAddEdge(g,3,4); dvGdAddEdge(g,4,0);
    }
    return 0;
}
$C$, $E$### C 实现

有向图 C API 示例。$E$);

-- [26] 课程先修关系图
INSERT INTO templates (title, category, description) VALUES (
'课程先修关系图', '有向图',
$D$用 DAG 表示先修依赖关系，适合拓扑排序教学。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'python', $C$import ds4viz as dv
with dv.graph_directed(label="course_prereqs") as g:
    g.add_node(0, "Math"); g.add_node(1, "C-Lang"); g.add_node(2, "DataStruct")
    g.add_node(3, "Algorithm"); g.add_node(4, "OS"); g.add_node(5, "Compiler")
    g.add_edge(0,2); g.add_edge(1,2); g.add_edge(2,3); g.add_edge(2,4); g.add_edge(3,5); g.add_edge(4,5)
$C$, $E$### Python 实现

先修课依赖图。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'lua', $C$local ds4viz = require("ds4viz")
ds4viz.withContext(ds4viz.graphDirected("course_prereqs"), function(g)
    g:addNode(0, "Math"); g:addNode(1, "C-Lang"); g:addNode(2, "DataStruct")
    g:addNode(3, "Algorithm"); g:addNode(4, "OS"); g:addNode(5, "Compiler")
    g:addEdge(0,2); g:addEdge(1,2); g:addEdge(2,3); g:addEdge(2,4); g:addEdge(3,5); g:addEdge(4,5)
end)
$C$, $E$### Lua 实现

DAG 依赖关系。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'rust', $C$use ds4viz::prelude::*;
fn main() -> ds4viz::Result<()> {
    ds4viz::graph_directed("course_prereqs", |g| {
        g.add_node(0, "Math")?; g.add_node(1, "C-Lang")?; g.add_node(2, "DataStruct")?;
        g.add_node(3, "Algorithm")?; g.add_node(4, "OS")?; g.add_node(5, "Compiler")?;
        g.add_edge(0,2)?; g.add_edge(1,2)?; g.add_edge(2,3)?; g.add_edge(2,4)?; g.add_edge(3,5)?; g.add_edge(4,5)?;
        Ok(())
    })
}
$C$, $E$### Rust 实现

课程依赖 DAG。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'c', $C$#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"
int main(void){
    dvGraphDirected(g, "course_prereqs"){
        dvGdAddNode(g,0,"Math"); dvGdAddNode(g,1,"C-Lang"); dvGdAddNode(g,2,"DataStruct");
        dvGdAddNode(g,3,"Algorithm"); dvGdAddNode(g,4,"OS"); dvGdAddNode(g,5,"Compiler");
        dvGdAddEdge(g,0,2); dvGdAddEdge(g,1,2); dvGdAddEdge(g,2,3); dvGdAddEdge(g,2,4); dvGdAddEdge(g,3,5); dvGdAddEdge(g,4,5);
    }
    return 0;
}
$C$, $E$### C 实现

先修关系图 C 版。$E$);

-- [27] 状态转换图
INSERT INTO templates (title, category, description) VALUES (
'状态转换图', '有向图',
$D$用有向图表示订单状态机的合法迁移路径。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'python', $C$import ds4viz as dv
with dv.graph_directed(label="order_fsm") as g:
    g.add_node(0, "Created"); g.add_node(1, "Paid"); g.add_node(2, "Shipped")
    g.add_node(3, "Delivered"); g.add_node(4, "Cancelled")
    g.add_edge(0,1); g.add_edge(1,2); g.add_edge(2,3); g.add_edge(0,4); g.add_edge(1,4)
$C$, $E$### Python 实现

状态机边表示“允许迁移”。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'lua', $C$local ds4viz = require("ds4viz")
ds4viz.withContext(ds4viz.graphDirected("order_fsm"), function(g)
    g:addNode(0, "Created"); g:addNode(1, "Paid"); g:addNode(2, "Shipped")
    g:addNode(3, "Delivered"); g:addNode(4, "Cancelled")
    g:addEdge(0,1); g:addEdge(1,2); g:addEdge(2,3); g:addEdge(0,4); g:addEdge(1,4)
end)
$C$, $E$### Lua 实现

FSM 可视化示意。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'rust', $C$use ds4viz::prelude::*;
fn main() -> ds4viz::Result<()> {
    ds4viz::graph_directed("order_fsm", |g| {
        g.add_node(0, "Created")?; g.add_node(1, "Paid")?; g.add_node(2, "Shipped")?;
        g.add_node(3, "Delivered")?; g.add_node(4, "Cancelled")?;
        g.add_edge(0,1)?; g.add_edge(1,2)?; g.add_edge(2,3)?; g.add_edge(0,4)?; g.add_edge(1,4)?;
        Ok(())
    })
}
$C$, $E$### Rust 实现

订单流转状态图。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'c', $C$#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"
int main(void){
    dvGraphDirected(g, "order_fsm"){
        dvGdAddNode(g,0,"Created"); dvGdAddNode(g,1,"Paid"); dvGdAddNode(g,2,"Shipped");
        dvGdAddNode(g,3,"Delivered"); dvGdAddNode(g,4,"Cancelled");
        dvGdAddEdge(g,0,1); dvGdAddEdge(g,1,2); dvGdAddEdge(g,2,3); dvGdAddEdge(g,0,4); dvGdAddEdge(g,1,4);
    }
    return 0;
}
$C$, $E$### C 实现

状态机有向图。$E$);

-- =====================================================
-- 分类: 带权图
-- =====================================================

-- [28] 城市交通网络
INSERT INTO templates (title, category, description) VALUES (
'城市交通网络', '带权图',
$D$无向带权图：节点为城市，权重为距离。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'python', $C$import ds4viz as dv
with dv.graph_weighted(label="city_roads", directed=False) as g:
    g.add_node(0, "Beijing"); g.add_node(1, "Shanghai"); g.add_node(2, "Guangzhou")
    g.add_node(3, "Shenzhen"); g.add_node(4, "Chengdu")
    g.add_edge(0,1,1213.0); g.add_edge(0,4,1816.0); g.add_edge(1,2,1462.0)
    g.add_edge(2,3,147.0); g.add_edge(2,4,1947.0); g.add_edge(3,4,1900.0)
$C$, $E$### Python 实现

城市距离图。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'lua', $C$local ds4viz = require("ds4viz")
ds4viz.withContext(ds4viz.graphWeighted("city_roads", false), function(g)
    g:addNode(0, "Beijing"); g:addNode(1, "Shanghai"); g:addNode(2, "Guangzhou")
    g:addNode(3, "Shenzhen"); g:addNode(4, "Chengdu")
    g:addEdge(0,1,1213.0); g:addEdge(0,4,1816.0); g:addEdge(1,2,1462.0)
    g:addEdge(2,3,147.0); g:addEdge(2,4,1947.0); g:addEdge(3,4,1900.0)
end)
$C$, $E$### Lua 实现

无向带权图。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'rust', $C$use ds4viz::prelude::*;
fn main() -> ds4viz::Result<()> {
    ds4viz::graph_weighted("city_roads", false, |g| {
        g.add_node(0, "Beijing")?; g.add_node(1, "Shanghai")?; g.add_node(2, "Guangzhou")?;
        g.add_node(3, "Shenzhen")?; g.add_node(4, "Chengdu")?;
        g.add_edge(0,1,1213.0)?; g.add_edge(0,4,1816.0)?; g.add_edge(1,2,1462.0)?;
        g.add_edge(2,3,147.0)?; g.add_edge(2,4,1947.0)?; g.add_edge(3,4,1900.0)?;
        Ok(())
    })
}
$C$, $E$### Rust 实现

交通网络示例。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'c', $C$#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"
int main(void){
    dvGraphWeighted(g, "city_roads", false){
        dvGwAddNode(g,0,"Beijing"); dvGwAddNode(g,1,"Shanghai"); dvGwAddNode(g,2,"Guangzhou");
        dvGwAddNode(g,3,"Shenzhen"); dvGwAddNode(g,4,"Chengdu");
        dvGwAddEdge(g,0,1,1213.0); dvGwAddEdge(g,0,4,1816.0); dvGwAddEdge(g,1,2,1462.0);
        dvGwAddEdge(g,2,3,147.0); dvGwAddEdge(g,2,4,1947.0); dvGwAddEdge(g,3,4,1900.0);
    }
    return 0;
}
$C$, $E$### C 实现

城市道路带权图。$E$);

-- [29] 网络拓扑图
INSERT INTO templates (title, category, description) VALUES (
'网络拓扑图', '带权图',
$D$有向带权图：边权表示网络延迟。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'python', $C$import ds4viz as dv
with dv.graph_weighted(label="network_topo", directed=True) as g:
    g.add_node(0, "Client"); g.add_node(1, "CDN"); g.add_node(2, "LB")
    g.add_node(3, "Server-A"); g.add_node(4, "Server-B"); g.add_node(5, "Database")
    g.add_edge(0,1,5.0); g.add_edge(1,2,2.0); g.add_edge(2,3,1.0)
    g.add_edge(2,4,1.5); g.add_edge(3,5,3.0); g.add_edge(4,5,3.2)
$C$, $E$### Python 实现

网络路径延迟建模。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'lua', $C$local ds4viz = require("ds4viz")
ds4viz.withContext(ds4viz.graphWeighted("network_topo", true), function(g)
    g:addNode(0, "Client"); g:addNode(1, "CDN"); g:addNode(2, "LB")
    g:addNode(3, "Server-A"); g:addNode(4, "Server-B"); g:addNode(5, "Database")
    g:addEdge(0,1,5.0); g:addEdge(1,2,2.0); g:addEdge(2,3,1.0)
    g:addEdge(2,4,1.5); g:addEdge(3,5,3.0); g:addEdge(4,5,3.2)
end)
$C$, $E$### Lua 实现

有向带权网络图。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'rust', $C$use ds4viz::prelude::*;
fn main() -> ds4viz::Result<()> {
    ds4viz::graph_weighted("network_topo", true, |g| {
        g.add_node(0, "Client")?; g.add_node(1, "CDN")?; g.add_node(2, "LB")?;
        g.add_node(3, "Server-A")?; g.add_node(4, "Server-B")?; g.add_node(5, "Database")?;
        g.add_edge(0,1,5.0)?; g.add_edge(1,2,2.0)?; g.add_edge(2,3,1.0)?;
        g.add_edge(2,4,1.5)?; g.add_edge(3,5,3.0)?; g.add_edge(4,5,3.2)?;
        Ok(())
    })
}
$C$, $E$### Rust 实现

网络拓扑与延迟。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'c', $C$#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"
int main(void){
    dvGraphWeighted(g, "network_topo", true){
        dvGwAddNode(g,0,"Client"); dvGwAddNode(g,1,"CDN"); dvGwAddNode(g,2,"LB");
        dvGwAddNode(g,3,"Server-A"); dvGwAddNode(g,4,"Server-B"); dvGwAddNode(g,5,"Database");
        dvGwAddEdge(g,0,1,5.0); dvGwAddEdge(g,1,2,2.0); dvGwAddEdge(g,2,3,1.0);
        dvGwAddEdge(g,2,4,1.5); dvGwAddEdge(g,3,5,3.0); dvGwAddEdge(g,4,5,3.2);
    }
    return 0;
}
$C$, $E$### C 实现

网络有向带权图。$E$);

-- [30] Dijkstra 最短路径示例图
INSERT INTO templates (title, category, description) VALUES (
'Dijkstra最短路径示例图', '带权图',
$D$经典最短路径教学图，适用于 Dijkstra 算法讲解。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'python', $C$import ds4viz as dv
with dv.graph_weighted(label="shortest_path", directed=True) as g:
    g.add_node(0, "S"); g.add_node(1, "A"); g.add_node(2, "B")
    g.add_node(3, "C"); g.add_node(4, "D"); g.add_node(5, "T")
    g.add_edge(0,1,10.0); g.add_edge(0,2,5.0); g.add_edge(2,1,3.0)
    g.add_edge(1,3,1.0); g.add_edge(2,3,9.0); g.add_edge(2,4,2.0)
    g.add_edge(4,3,6.0); g.add_edge(3,5,4.0); g.add_edge(4,5,7.0)
$C$, $E$### Python 实现

Dijkstra 入门经典图。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'lua', $C$local ds4viz = require("ds4viz")
ds4viz.withContext(ds4viz.graphWeighted("shortest_path", true), function(g)
    g:addNode(0, "S"); g:addNode(1, "A"); g:addNode(2, "B")
    g:addNode(3, "C"); g:addNode(4, "D"); g:addNode(5, "T")
    g:addEdge(0,1,10.0); g:addEdge(0,2,5.0); g:addEdge(2,1,3.0)
    g:addEdge(1,3,1.0); g:addEdge(2,3,9.0); g:addEdge(2,4,2.0)
    g:addEdge(4,3,6.0); g:addEdge(3,5,4.0); g:addEdge(4,5,7.0)
end)
$C$, $E$### Lua 实现

最短路径示例图。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'rust', $C$use ds4viz::prelude::*;
fn main() -> ds4viz::Result<()> {
    ds4viz::graph_weighted("shortest_path", true, |g| {
        g.add_node(0, "S")?; g.add_node(1, "A")?; g.add_node(2, "B")?;
        g.add_node(3, "C")?; g.add_node(4, "D")?; g.add_node(5, "T")?;
        g.add_edge(0,1,10.0)?; g.add_edge(0,2,5.0)?; g.add_edge(2,1,3.0)?;
        g.add_edge(1,3,1.0)?; g.add_edge(2,3,9.0)?; g.add_edge(2,4,2.0)?;
        g.add_edge(4,3,6.0)?; g.add_edge(3,5,4.0)?; g.add_edge(4,5,7.0)?;
        Ok(())
    })
}
$C$, $E$### Rust 实现

有向带权最短路径图。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'c', $C$#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"
int main(void){
    dvGraphWeighted(g, "shortest_path", true){
        dvGwAddNode(g,0,"S"); dvGwAddNode(g,1,"A"); dvGwAddNode(g,2,"B");
        dvGwAddNode(g,3,"C"); dvGwAddNode(g,4,"D"); dvGwAddNode(g,5,"T");
        dvGwAddEdge(g,0,1,10.0); dvGwAddEdge(g,0,2,5.0); dvGwAddEdge(g,2,1,3.0);
        dvGwAddEdge(g,1,3,1.0); dvGwAddEdge(g,2,3,9.0); dvGwAddEdge(g,2,4,2.0);
        dvGwAddEdge(g,4,3,6.0); dvGwAddEdge(g,3,5,4.0); dvGwAddEdge(g,4,5,7.0);
    }
    return 0;
}
$C$, $E$### C 实现

Dijkstra 教学图 C 版。$E$);

-- =====================================================
-- 新增 10 个类别
-- =====================================================

-- [31] 二叉树三种遍历示意
INSERT INTO templates (title, category, description) VALUES (
'二叉树三种遍历示意', '树遍历',
$D$构建固定二叉树，讲解前序/中序/后序遍历序列。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'python', $C$import ds4viz as dv
with dv.binary_tree(label="traversal_tree") as t:
    r = t.insert_root("A")
    b = t.insert_left(r, "B")
    c = t.insert_right(r, "C")
    t.insert_left(b, "D")
    t.insert_right(b, "E")
    t.insert_left(c, "F")
    t.insert_right(c, "G")
$C$, $E$### Python 实现

固定树用于遍历教学。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'lua', $C$local ds4viz = require("ds4viz")
ds4viz.withContext(ds4viz.binaryTree("traversal_tree"), function(t)
    local r = t:insertRoot("A")
    local b = t:insertLeft(r, "B")
    local c = t:insertRight(r, "C")
    t:insertLeft(b, "D"); t:insertRight(b, "E")
    t:insertLeft(c, "F"); t:insertRight(c, "G")
end)
$C$, $E$### Lua 实现

遍历示意树。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'rust', $C$use ds4viz::prelude::*;
fn main() -> ds4viz::Result<()> {
    ds4viz::binary_tree("traversal_tree", |t| {
        let r = t.insert_root("A")?;
        let b = t.insert_left(r, "B")?;
        let c = t.insert_right(r, "C")?;
        t.insert_left(b, "D")?; t.insert_right(b, "E")?;
        t.insert_left(c, "F")?; t.insert_right(c, "G")?;
        Ok(())
    })
}
$C$, $E$### Rust 实现

遍历教学模板。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'c', $C$#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"
int main(void){
    dvBinaryTree(t, "traversal_tree"){
        int r = dvBtInsertRoot(t, "A");
        int b = dvBtInsertLeft(t, r, "B");
        int c = dvBtInsertRight(t, r, "C");
        dvBtInsertLeft(t, b, "D"); dvBtInsertRight(t, b, "E");
        dvBtInsertLeft(t, c, "F"); dvBtInsertRight(t, c, "G");
    }
    return 0;
}
$C$, $E$### C 实现

遍历教学树（C）。$E$);

-- [32] B树节点分裂概念示意
INSERT INTO templates (title, category, description) VALUES (
'B树节点分裂概念示意', 'B树',
$D$ds4viz 现阶段无原生 B 树结构，使用有向图表达多关键字节点关系。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'python', $C$import ds4viz as dv
with dv.graph_directed(label="b_tree_concept") as g:
    g.add_node(0, "root:[20|40]")
    g.add_node(1, "L:[5|10|15]")
    g.add_node(2, "M:[25|30|35]")
    g.add_node(3, "R:[45|50|60]")
    g.add_edge(0, 1); g.add_edge(0, 2); g.add_edge(0, 3)
$C$, $E$### Python 实现

B 树结构概念图。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'lua', $C$local ds4viz = require("ds4viz")
ds4viz.withContext(ds4viz.graphDirected("b_tree_concept"), function(g)
    g:addNode(0, "root:[20|40]")
    g:addNode(1, "L:[5|10|15]")
    g:addNode(2, "M:[25|30|35]")
    g:addNode(3, "R:[45|50|60]")
    g:addEdge(0, 1); g:addEdge(0, 2); g:addEdge(0, 3)
end)
$C$, $E$### Lua 实现

B 树概念示意。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'rust', $C$use ds4viz::prelude::*;
fn main() -> ds4viz::Result<()> {
    ds4viz::graph_directed("b_tree_concept", |g| {
        g.add_node(0, "root:[20|40]")?;
        g.add_node(1, "L:[5|10|15]")?;
        g.add_node(2, "M:[25|30|35]")?;
        g.add_node(3, "R:[45|50|60]")?;
        g.add_edge(0, 1)?; g.add_edge(0, 2)?; g.add_edge(0, 3)?;
        Ok(())
    })
}
$C$, $E$### Rust 实现

B 树课堂示意图。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'c', $C$#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"
int main(void){
    dvGraphDirected(g, "b_tree_concept"){
        dvGdAddNode(g, 0, "root:[20|40]");
        dvGdAddNode(g, 1, "L:[5|10|15]");
        dvGdAddNode(g, 2, "M:[25|30|35]");
        dvGdAddNode(g, 3, "R:[45|50|60]");
        dvGdAddEdge(g, 0, 1); dvGdAddEdge(g, 0, 2); dvGdAddEdge(g, 0, 3);
    }
    return 0;
}
$C$, $E$### C 实现

B 树概念图（C）。$E$);

-- [33] AVL 旋转后形态示意
INSERT INTO templates (title, category, description) VALUES (
'AVL旋转后形态示意', 'AVL树',
$D$展示 AVL 平衡结果树形，用于讲解 LL/RR/LR/RL 旋转效果。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'python', $C$import ds4viz as dv
with dv.binary_tree(label="avl_shape") as t:
    r = t.insert_root(30)
    l = t.insert_left(r, 20)
    t.insert_right(r, 40)
    t.insert_left(l, 10)
    t.insert_right(l, 25)
$C$, $E$### Python 实现

AVL 平衡结构示意。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'lua', $C$local ds4viz = require("ds4viz")
ds4viz.withContext(ds4viz.binaryTree("avl_shape"), function(t)
    local r = t:insertRoot(30)
    local l = t:insertLeft(r, 20)
    t:insertRight(r, 40)
    t:insertLeft(l, 10)
    t:insertRight(l, 25)
end)
$C$, $E$### Lua 实现

AVL 平衡后形态。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'rust', $C$use ds4viz::prelude::*;
fn main() -> ds4viz::Result<()> {
    ds4viz::binary_tree("avl_shape", |t| {
        let r = t.insert_root(30)?;
        let l = t.insert_left(r, 20)?;
        t.insert_right(r, 40)?;
        t.insert_left(l, 10)?;
        t.insert_right(l, 25)?;
        Ok(())
    })
}
$C$, $E$### Rust 实现

AVL 教学树。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'c', $C$#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"
int main(void){
    dvBinaryTree(t, "avl_shape"){
        int r = dvBtInsertRoot(t, 30);
        int l = dvBtInsertLeft(t, r, 20);
        dvBtInsertRight(t, r, 40);
        dvBtInsertLeft(t, l, 10);
        dvBtInsertRight(t, l, 25);
    }
    return 0;
}
$C$, $E$### C 实现

AVL 形态示意（C）。$E$);

-- [34] Trie 前缀树概念示意
INSERT INTO templates (title, category, description) VALUES (
'Trie前缀树概念示意', 'Trie',
$D$以有向图展示前缀扩展路径，节点标签表示前缀。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'python', $C$import ds4viz as dv
with dv.graph_directed(label="trie_concept") as g:
    g.add_node(0, "root")
    g.add_node(1, "a")
    g.add_node(2, "ap")
    g.add_node(3, "app*")
    g.add_node(4, "ape*")
    g.add_edge(0,1); g.add_edge(1,2); g.add_edge(2,3); g.add_edge(2,4)
$C$, $E$### Python 实现

`*` 表示单词结束。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'lua', $C$local ds4viz = require("ds4viz")
ds4viz.withContext(ds4viz.graphDirected("trie_concept"), function(g)
    g:addNode(0, "root")
    g:addNode(1, "a")
    g:addNode(2, "ap")
    g:addNode(3, "app*")
    g:addNode(4, "ape*")
    g:addEdge(0,1); g:addEdge(1,2); g:addEdge(2,3); g:addEdge(2,4)
end)
$C$, $E$### Lua 实现

Trie 概念路径图。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'rust', $C$use ds4viz::prelude::*;
fn main() -> ds4viz::Result<()> {
    ds4viz::graph_directed("trie_concept", |g| {
        g.add_node(0, "root")?;
        g.add_node(1, "a")?;
        g.add_node(2, "ap")?;
        g.add_node(3, "app*")?;
        g.add_node(4, "ape*")?;
        g.add_edge(0,1)?; g.add_edge(1,2)?; g.add_edge(2,3)?; g.add_edge(2,4)?;
        Ok(())
    })
}
$C$, $E$### Rust 实现

前缀扩展示意。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'c', $C$#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"
int main(void){
    dvGraphDirected(g, "trie_concept"){
        dvGdAddNode(g,0,"root");
        dvGdAddNode(g,1,"a");
        dvGdAddNode(g,2,"ap");
        dvGdAddNode(g,3,"app*");
        dvGdAddNode(g,4,"ape*");
        dvGdAddEdge(g,0,1); dvGdAddEdge(g,1,2); dvGdAddEdge(g,2,3); dvGdAddEdge(g,2,4);
    }
    return 0;
}
$C$, $E$### C 实现

Trie 概念图（C）。$E$);

-- [35] 并查集连通分量示意
INSERT INTO templates (title, category, description) VALUES (
'并查集连通分量示意', '并查集',
$D$以无向图连通块表示 Union-Find 的集合划分效果。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'python', $C$import ds4viz as dv
with dv.graph_undirected(label="union_find_sets") as g:
    for i in range(6):
        g.add_node(i, str(i))
    g.add_edge(0,1); g.add_edge(1,2)
    g.add_edge(3,4)
$C$, $E$### Python 实现

两个连通分量示意。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'lua', $C$local ds4viz = require("ds4viz")
ds4viz.withContext(ds4viz.graphUndirected("union_find_sets"), function(g)
    for i = 0, 5 do g:addNode(i, tostring(i)) end
    g:addEdge(0,1); g:addEdge(1,2); g:addEdge(3,4)
end)
$C$, $E$### Lua 实现

并查集结果可视化。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'rust', $C$use ds4viz::prelude::*;
fn main() -> ds4viz::Result<()> {
    ds4viz::graph_undirected("union_find_sets", |g| {
        for i in 0..6 { g.add_node(i, &i.to_string())?; }
        g.add_edge(0,1)?; g.add_edge(1,2)?; g.add_edge(3,4)?;
        Ok(())
    })
}
$C$, $E$### Rust 实现

连通块视角解释并查集。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'c', $C$#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"
int main(void){
    dvGraphUndirected(g, "union_find_sets"){
        dvGuAddNode(g,0,"0"); dvGuAddNode(g,1,"1"); dvGuAddNode(g,2,"2");
        dvGuAddNode(g,3,"3"); dvGuAddNode(g,4,"4"); dvGuAddNode(g,5,"5");
        dvGuAddEdge(g,0,1); dvGuAddEdge(g,1,2); dvGuAddEdge(g,3,4);
    }
    return 0;
}
$C$, $E$### C 实现

并查集连通分量示意（C）。$E$);

-- [36] 哈希表拉链法示意
INSERT INTO templates (title, category, description) VALUES (
'哈希表拉链法示意', '哈希表',
$D$用有向图表示 bucket 到冲突链节点的连接关系。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'python', $C$import ds4viz as dv
with dv.graph_directed(label="hash_chaining") as g:
    g.add_node(0, "bucket0"); g.add_node(1, "bucket1"); g.add_node(2, "bucket2")
    g.add_node(10, "k=10"); g.add_node(13, "k=13"); g.add_node(16, "k=16")
    g.add_edge(1,10); g.add_edge(10,13); g.add_edge(13,16)
$C$, $E$### Python 实现

冲突链（拉链法）示意。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'lua', $C$local ds4viz = require("ds4viz")
ds4viz.withContext(ds4viz.graphDirected("hash_chaining"), function(g)
    g:addNode(0, "bucket0"); g:addNode(1, "bucket1"); g:addNode(2, "bucket2")
    g:addNode(10, "k=10"); g:addNode(13, "k=13"); g:addNode(16, "k=16")
    g:addEdge(1,10); g:addEdge(10,13); g:addEdge(13,16)
end)
$C$, $E$### Lua 实现

桶 + 链式冲突。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'rust', $C$use ds4viz::prelude::*;
fn main() -> ds4viz::Result<()> {
    ds4viz::graph_directed("hash_chaining", |g| {
        g.add_node(0, "bucket0")?; g.add_node(1, "bucket1")?; g.add_node(2, "bucket2")?;
        g.add_node(10, "k=10")?; g.add_node(13, "k=13")?; g.add_node(16, "k=16")?;
        g.add_edge(1,10)?; g.add_edge(10,13)?; g.add_edge(13,16)?;
        Ok(())
    })
}
$C$, $E$### Rust 实现

哈希冲突教学图。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'c', $C$#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"
int main(void){
    dvGraphDirected(g, "hash_chaining"){
        dvGdAddNode(g,0,"bucket0"); dvGdAddNode(g,1,"bucket1"); dvGdAddNode(g,2,"bucket2");
        dvGdAddNode(g,10,"k=10"); dvGdAddNode(g,13,"k=13"); dvGdAddNode(g,16,"k=16");
        dvGdAddEdge(g,1,10); dvGdAddEdge(g,10,13); dvGdAddEdge(g,13,16);
    }
    return 0;
}
$C$, $E$### C 实现

哈希拉链法示意（C）。$E$);

-- [37] 跳表示意（多层索引）
INSERT INTO templates (title, category, description) VALUES (
'跳表示意（多层索引）', '跳表',
$D$多层索引链 + 底层链，用于解释跳表“快速跳跃”思想。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'python', $C$import ds4viz as dv
with dv.graph_directed(label="skiplist_concept") as g:
    g.add_node(1, "L2:1"); g.add_node(3, "L2:3")
    g.add_node(11, "L1:1"); g.add_node(12, "L1:2"); g.add_node(13, "L1:3")
    g.add_node(21, "B:1"); g.add_node(22, "B:2"); g.add_node(23, "B:3")
    g.add_edge(1,3)
    g.add_edge(11,12); g.add_edge(12,13)
    g.add_edge(21,22); g.add_edge(22,23)
$C$, $E$### Python 实现

跳表层级结构示意。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'lua', $C$local ds4viz = require("ds4viz")
ds4viz.withContext(ds4viz.graphDirected("skiplist_concept"), function(g)
    g:addNode(1, "L2:1"); g:addNode(3, "L2:3")
    g:addNode(11, "L1:1"); g:addNode(12, "L1:2"); g:addNode(13, "L1:3")
    g:addNode(21, "B:1"); g:addNode(22, "B:2"); g:addNode(23, "B:3")
    g:addEdge(1,3)
    g:addEdge(11,12); g:addEdge(12,13)
    g:addEdge(21,22); g:addEdge(22,23)
end)
$C$, $E$### Lua 实现

多层链路示意。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'rust', $C$use ds4viz::prelude::*;
fn main() -> ds4viz::Result<()> {
    ds4viz::graph_directed("skiplist_concept", |g| {
        g.add_node(1, "L2:1")?; g.add_node(3, "L2:3")?;
        g.add_node(11, "L1:1")?; g.add_node(12, "L1:2")?; g.add_node(13, "L1:3")?;
        g.add_node(21, "B:1")?; g.add_node(22, "B:2")?; g.add_node(23, "B:3")?;
        g.add_edge(1,3)?;
        g.add_edge(11,12)?; g.add_edge(12,13)?;
        g.add_edge(21,22)?; g.add_edge(22,23)?;
        Ok(())
    })
}
$C$, $E$### Rust 实现

跳表概念模板。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'c', $C$#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"
int main(void){
    dvGraphDirected(g, "skiplist_concept"){
        dvGdAddNode(g,1,"L2:1"); dvGdAddNode(g,3,"L2:3");
        dvGdAddNode(g,11,"L1:1"); dvGdAddNode(g,12,"L1:2"); dvGdAddNode(g,13,"L1:3");
        dvGdAddNode(g,21,"B:1"); dvGdAddNode(g,22,"B:2"); dvGdAddNode(g,23,"B:3");
        dvGdAddEdge(g,1,3);
        dvGdAddEdge(g,11,12); dvGdAddEdge(g,12,13);
        dvGdAddEdge(g,21,22); dvGdAddEdge(g,22,23);
    }
    return 0;
}
$C$, $E$### C 实现

跳表多层索引示意（C）。$E$);

-- [38] 拓扑排序示例图
INSERT INTO templates (title, category, description) VALUES (
'拓扑排序示例图', '拓扑排序',
$D$构建 DAG，用于 Kahn / DFS 拓扑序教学。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'python', $C$import ds4viz as dv
with dv.graph_directed(label="topo_dag") as g:
    g.add_node(0, "A"); g.add_node(1, "B"); g.add_node(2, "C"); g.add_node(3, "D"); g.add_node(4, "E")
    g.add_edge(0,2); g.add_edge(1,2); g.add_edge(2,3); g.add_edge(2,4)
$C$, $E$### Python 实现

典型无环依赖图。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'lua', $C$local ds4viz = require("ds4viz")
ds4viz.withContext(ds4viz.graphDirected("topo_dag"), function(g)
    g:addNode(0, "A"); g:addNode(1, "B"); g:addNode(2, "C"); g:addNode(3, "D"); g:addNode(4, "E")
    g:addEdge(0,2); g:addEdge(1,2); g:addEdge(2,3); g:addEdge(2,4)
end)
$C$, $E$### Lua 实现

拓扑排序 DAG。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'rust', $C$use ds4viz::prelude::*;
fn main() -> ds4viz::Result<()> {
    ds4viz::graph_directed("topo_dag", |g| {
        g.add_node(0, "A")?; g.add_node(1, "B")?; g.add_node(2, "C")?; g.add_node(3, "D")?; g.add_node(4, "E")?;
        g.add_edge(0,2)?; g.add_edge(1,2)?; g.add_edge(2,3)?; g.add_edge(2,4)?;
        Ok(())
    })
}
$C$, $E$### Rust 实现

拓扑序教学图。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'c', $C$#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"
int main(void){
    dvGraphDirected(g, "topo_dag"){
        dvGdAddNode(g,0,"A"); dvGdAddNode(g,1,"B"); dvGdAddNode(g,2,"C"); dvGdAddNode(g,3,"D"); dvGdAddNode(g,4,"E");
        dvGdAddEdge(g,0,2); dvGdAddEdge(g,1,2); dvGdAddEdge(g,2,3); dvGdAddEdge(g,2,4);
    }
    return 0;
}
$C$, $E$### C 实现

DAG C 版本。$E$);

-- [39] 最短路径入门图
INSERT INTO templates (title, category, description) VALUES (
'最短路径入门图', '最短路径',
$D$有向带权图，用于最短路径算法的入门讲解。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'python', $C$import ds4viz as dv
with dv.graph_weighted(label="dijkstra_intro", directed=True) as g:
    g.add_node(0, "S"); g.add_node(1, "A"); g.add_node(2, "B"); g.add_node(3, "T")
    g.add_edge(0,1,2.0); g.add_edge(0,2,5.0); g.add_edge(1,2,1.0); g.add_edge(1,3,4.0); g.add_edge(2,3,1.0)
$C$, $E$### Python 实现

短路径常见示例。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'lua', $C$local ds4viz = require("ds4viz")
ds4viz.withContext(ds4viz.graphWeighted("dijkstra_intro", true), function(g)
    g:addNode(0, "S"); g:addNode(1, "A"); g:addNode(2, "B"); g:addNode(3, "T")
    g:addEdge(0,1,2.0); g:addEdge(0,2,5.0); g:addEdge(1,2,1.0); g:addEdge(1,3,4.0); g:addEdge(2,3,1.0)
end)
$C$, $E$### Lua 实现

最短路径入门图。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'rust', $C$use ds4viz::prelude::*;
fn main() -> ds4viz::Result<()> {
    ds4viz::graph_weighted("dijkstra_intro", true, |g| {
        g.add_node(0, "S")?; g.add_node(1, "A")?; g.add_node(2, "B")?; g.add_node(3, "T")?;
        g.add_edge(0,1,2.0)?; g.add_edge(0,2,5.0)?; g.add_edge(1,2,1.0)?; g.add_edge(1,3,4.0)?; g.add_edge(2,3,1.0)?;
        Ok(())
    })
}
$C$, $E$### Rust 实现

Dijkstra 入门模板。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'c', $C$#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"
int main(void){
    dvGraphWeighted(g, "dijkstra_intro", true){
        dvGwAddNode(g,0,"S"); dvGwAddNode(g,1,"A"); dvGwAddNode(g,2,"B"); dvGwAddNode(g,3,"T");
        dvGwAddEdge(g,0,1,2.0); dvGwAddEdge(g,0,2,5.0); dvGwAddEdge(g,1,2,1.0); dvGwAddEdge(g,1,3,4.0); dvGwAddEdge(g,2,3,1.0);
    }
    return 0;
}
$C$, $E$### C 实现

最短路径示例（C）。$E$);

-- [40] 最小生成树示例图
INSERT INTO templates (title, category, description) VALUES (
'最小生成树示例图', '最小生成树',
$D$无向带权图，用于 Prim/Kruskal 最小生成树讲解。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'python', $C$import ds4viz as dv
with dv.graph_weighted(label="mst_graph", directed=False) as g:
    g.add_node(0, "A"); g.add_node(1, "B"); g.add_node(2, "C"); g.add_node(3, "D")
    g.add_edge(0,1,1.0); g.add_edge(0,2,4.0); g.add_edge(1,2,2.0); g.add_edge(1,3,5.0); g.add_edge(2,3,3.0)
$C$, $E$### Python 实现

MST 经典小图。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'lua', $C$local ds4viz = require("ds4viz")
ds4viz.withContext(ds4viz.graphWeighted("mst_graph", false), function(g)
    g:addNode(0, "A"); g:addNode(1, "B"); g:addNode(2, "C"); g:addNode(3, "D")
    g:addEdge(0,1,1.0); g:addEdge(0,2,4.0); g:addEdge(1,2,2.0); g:addEdge(1,3,5.0); g:addEdge(2,3,3.0)
end)
$C$, $E$### Lua 实现

最小生成树示意图。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'rust', $C$use ds4viz::prelude::*;
fn main() -> ds4viz::Result<()> {
    ds4viz::graph_weighted("mst_graph", false, |g| {
        g.add_node(0, "A")?; g.add_node(1, "B")?; g.add_node(2, "C")?; g.add_node(3, "D")?;
        g.add_edge(0,1,1.0)?; g.add_edge(0,2,4.0)?; g.add_edge(1,2,2.0)?; g.add_edge(1,3,5.0)?; g.add_edge(2,3,3.0)?;
        Ok(())
    })
}
$C$, $E$### Rust 实现

Prim/Kruskal 练习图。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'c', $C$#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"
int main(void){
    dvGraphWeighted(g, "mst_graph", false){
        dvGwAddNode(g,0,"A"); dvGwAddNode(g,1,"B"); dvGwAddNode(g,2,"C"); dvGwAddNode(g,3,"D");
        dvGwAddEdge(g,0,1,1.0); dvGwAddEdge(g,0,2,4.0); dvGwAddEdge(g,1,2,2.0); dvGwAddEdge(g,1,3,5.0); dvGwAddEdge(g,2,3,3.0);
    }
    return 0;
}
$C$, $E$### C 实现

最小生成树示例（C）。$E$);

-- =====================================================
-- 更新序列值
-- =====================================================
SELECT setval('templates_id_seq', (SELECT COALESCE(MAX(id), 0) FROM templates));
SELECT setval('template_codes_id_seq', (SELECT COALESCE(MAX(id), 0) FROM template_codes));

COMMIT;