-- =====================================================
-- ds4viz 模板初始化数据（完整版）
-- 50 个模板 × 2 种语言 (Python / C) = 100 条代码记录
-- 适用于 PostgreSQL
-- =====================================================

BEGIN;

TRUNCATE template_codes, templates RESTART IDENTITY CASCADE;

-- =====================================================
-- 分类: 栈 (6)
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
    s.pop()
    s.pop()
$C$, $E$### Python 实现

基础 push / pop 操作，三个元素依次入栈再依次出栈，体现后进先出。$E$);

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
        dvStackPop(s);
        dvStackPop(s);
    }
    return 0;
}
$C$, $E$### C 实现

使用 `dvStack` 块作用域，退出时自动输出 `.toml` 文件。$E$);

-- [2] 栈的操作可视化
INSERT INTO templates (title, category, description) VALUES (
'栈的操作可视化', '栈',
$D$在 push / pop 的同时使用高亮标记，直观展示栈顶变化。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'python', $C$import ds4viz as dv

dv.config(title="栈的操作可视化")

with dv.stack(label="vis_stack") as s:
    s.push(10)
    s.amend(note="压入 10", highlights=[dv.item(0, "focus", level=3)])

    s.push(20)
    s.amend(note="压入 20", highlights=[
        dv.item(0, "visited"),
        dv.item(1, "focus", level=3)
    ])

    s.push(30)
    s.amend(note="压入 30", highlights=[
        dv.item(0, "visited"),
        dv.item(1, "visited"),
        dv.item(2, "focus", level=3)
    ])

    val = s.pop()
    s.amend(note=f"弹出 {val}", highlights=[
        dv.item(0, "visited"),
        dv.item(1, "focus", level=3)
    ])

    val = s.pop()
    s.amend(note=f"弹出 {val}", highlights=[
        dv.item(0, "focus", level=3)
    ])
$C$, $E$### Python 实现

使用 `amend()` 为每一步添加说明和高亮，`dv.item()` 标记栈中元素位置。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'c', $C$#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"

int main(void) {
    dvConfig((dvConfigOptions){ .title = "栈的操作可视化" });

    dvStack(s, "vis_stack") {
        dvStackPush(s, 10);
        dvAmend(s, "压入 10", dvItem(0, DV_FOCUS, 3));

        dvStackPush(s, 20);
        dvAmend(s, "压入 20",
            dvItem(0, DV_VISITED),
            dvItem(1, DV_FOCUS, 3));

        dvStackPush(s, 30);
        dvAmend(s, "压入 30",
            dvItem(0, DV_VISITED),
            dvItem(1, DV_VISITED),
            dvItem(2, DV_FOCUS, 3));

        dvStackPop(s);
        dvAmend(s, "弹出 30",
            dvItem(0, DV_VISITED),
            dvItem(1, DV_FOCUS, 3));

        dvStackPop(s);
        dvAmend(s, "弹出 20",
            dvItem(0, DV_FOCUS, 3));
    }
    return 0;
}
$C$, $E$### C 实现

`dvAmend` 为上一步补充说明与高亮，`dvItem` 构造元素高亮标记。$E$);

-- [3] 括号匹配检验
INSERT INTO templates (title, category, description) VALUES (
'括号匹配检验', '栈',
$D$使用栈检查括号字符串 `(())` 是否合法：左括号入栈，右括号出栈匹配。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'python', $C$import ds4viz as dv

dv.config(title="括号匹配: (())")

with dv.stack(label="bracket_check") as s:
    with s.phase("扫描"):
        s.push("(")
        s.amend(note="读入 '(', 压栈", highlights=[dv.item(0, "focus", level=3)])

        s.push("(")
        s.amend(note="读入 '(', 压栈", highlights=[
            dv.item(0, "visited"),
            dv.item(1, "focus", level=3)
        ])

        val = s.pop()
        s.amend(note=f"读入 ')', 弹出 '{val}' 匹配成功", highlights=[
            dv.item(0, "focus", level=3)
        ])

        val = s.pop()
        s.amend(note=f"读入 ')', 弹出 '{val}' 匹配成功")

    s.step(note="栈为空, 括号合法")
$C$, $E$### Python 实现

使用 `phase()` 划分"扫描"和"结论"两阶段，高亮跟踪栈顶变化。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'c', $C$#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"

int main(void) {
    dvConfig((dvConfigOptions){ .title = "括号匹配: (())" });

    dvStack(s, "bracket_check") {
        dvPhase(s, "扫描") {
            dvStackPush(s, "(");
            dvAmend(s, "读入 '(', 压栈", dvItem(0, DV_FOCUS, 3));

            dvStackPush(s, "(");
            dvAmend(s, "读入 '(', 压栈",
                dvItem(0, DV_VISITED),
                dvItem(1, DV_FOCUS, 3));

            dvStackPop(s);
            dvAmend(s, "读入 ')', 弹出匹配成功",
                dvItem(0, DV_FOCUS, 3));

            dvStackPop(s);
            dvAmend(s, "读入 ')', 弹出匹配成功");
        }

        dvStep(s, "栈为空, 括号合法");
    }
    return 0;
}
$C$, $E$### C 实现

`dvPhase` 块将扫描过程标记为独立阶段，渲染器可分段展示。$E$);

-- [4] 十进制转二进制
INSERT INTO templates (title, category, description) VALUES (
'十进制转二进制', '栈',
$D$利用"取余入栈、依次出栈"将十进制数 42 转换为二进制表示。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'python', $C$import ds4viz as dv

dv.config(title="十进制转二进制: 42")

n = 42
with dv.stack(label="dec_to_bin") as s:
    with s.phase("取余入栈"):
        cnt = 0
        while n > 0:
            rem = n % 2
            s.push(rem)
            s.amend(note=f"{n} % 2 = {rem}, 压栈")
            n //= 2
            cnt += 1

    with s.phase("依次出栈"):
        for _ in range(cnt):
            val = s.pop()
            s.amend(note=f"弹出 {val}")
$C$, $E$### Python 实现

两阶段：先"取余入栈"将低位到高位依次压入；再"依次出栈"得到高位在前的二进制序列。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'c', $C$#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"

int main(void) {
    dvConfig((dvConfigOptions){ .title = "十进制转二进制: 42" });

    int n = 42;
    dvStack(s, "dec_to_bin") {
        int cnt = 0;
        dvPhase(s, "取余入栈") {
            while (n > 0) {
                dvStackPush(s, n % 2);
                dvAmend(s, "取余并压栈");
                n /= 2;
                cnt++;
            }
        }

        dvPhase(s, "依次出栈") {
            for (int i = 0; i < cnt; i++) {
                dvStackPop(s);
                dvAmend(s, "弹出位");
            }
        }
    }
    return 0;
}
$C$, $E$### C 实现

经典"取余 + 出栈"进制转换流程。$E$);

-- [5] 逆序字符串
INSERT INTO templates (title, category, description) VALUES (
'逆序字符串', '栈',
$D$利用栈的 LIFO 特性将字符串 "hello" 逆序为 "olleh"。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'python', $C$import ds4viz as dv

dv.config(title="逆序字符串: hello")

word = "hello"
with dv.stack(label="reverse_str") as s:
    with s.phase("逐字符入栈"):
        for i, ch in enumerate(word):
            s.push(ch)
            s.amend(note=f"压入 '{ch}'",
                    highlights=[dv.item(i, "focus", level=3)])

    with s.phase("逐字符出栈"):
        for i in range(len(word)):
            val = s.pop()
            s.amend(note=f"弹出 '{val}'")
$C$, $E$### Python 实现

两阶段演示：入栈时高亮最新元素，出栈时依次弹出实现逆序。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'c', $C$#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"

int main(void) {
    dvConfig((dvConfigOptions){ .title = "逆序字符串: hello" });

    const char* word = "hello";
    int len = 5;
    dvStack(s, "reverse_str") {
        dvPhase(s, "逐字符入栈") {
            for (int i = 0; i < len; i++) {
                char buf[2] = { word[i], '\0' };
                dvStackPush(s, buf);
                dvAmend(s, "压入字符", dvItem(i, DV_FOCUS, 3));
            }
        }

        dvPhase(s, "逐字符出栈") {
            for (int i = 0; i < len; i++) {
                dvStackPop(s);
                dvAmend(s, "弹出字符");
            }
        }
    }
    return 0;
}
$C$, $E$### C 实现

栈天然实现序列反转，字符逐个入栈后弹出即为倒序。$E$);

-- [6] 后缀表达式求值
INSERT INTO templates (title, category, description) VALUES (
'后缀表达式求值', '栈',
$D$用栈演示后缀表达式 `3 5 + 2 *`（即 (3+5)*2 = 16）的求值过程。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'python', $C$import ds4viz as dv

dv.config(title="后缀表达式: 3 5 + 2 *")

with dv.stack(label="postfix_eval") as s:
    with s.phase("求值"):
        s.push(3)
        s.amend(note="读入 3, 压栈", highlights=[dv.item(0, "focus", level=3)])

        s.push(5)
        s.amend(note="读入 5, 压栈", highlights=[
            dv.item(0, "visited"),
            dv.item(1, "focus", level=3)
        ])

        b = s.pop()
        a = s.pop()
        s.push(a + b)
        s.amend(note=f"读入 '+', 弹出 {a} 和 {b}, 压入 {a+b}",
                highlights=[dv.item(0, "active", level=3)])

        s.push(2)
        s.amend(note="读入 2, 压栈", highlights=[
            dv.item(0, "visited"),
            dv.item(1, "focus", level=3)
        ])

        b = s.pop()
        a = s.pop()
        s.push(a * b)
        s.amend(note=f"读入 '*', 弹出 {a} 和 {b}, 压入 {a*b}",
                highlights=[dv.item(0, "found", level=3)])

    s.step(note="求值完毕, 结果为 16")
$C$, $E$### Python 实现

操作数入栈，遇到运算符弹出两个操作数计算后将结果压回，最终栈中即为结果。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'c', $C$#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"

int main(void) {
    dvConfig((dvConfigOptions){ .title = "后缀表达式: 3 5 + 2 *" });

    dvStack(s, "postfix_eval") {
        dvPhase(s, "求值") {
            dvStackPush(s, 3);
            dvAmend(s, "读入 3, 压栈", dvItem(0, DV_FOCUS, 3));

            dvStackPush(s, 5);
            dvAmend(s, "读入 5, 压栈",
                dvItem(0, DV_VISITED),
                dvItem(1, DV_FOCUS, 3));

            dvValue b = dvStackPop(s);
            dvValue a = dvStackPop(s);
            dvStackPush(s, dvInt(a) + dvInt(b));
            dvAmend(s, "读入 '+', 3+5=8 压栈",
                dvItem(0, DV_ACTIVE, 3));

            dvStackPush(s, 2);
            dvAmend(s, "读入 2, 压栈",
                dvItem(0, DV_VISITED),
                dvItem(1, DV_FOCUS, 3));

            b = dvStackPop(s);
            a = dvStackPop(s);
            dvStackPush(s, dvInt(a) * dvInt(b));
            dvAmend(s, "读入 '*', 8*2=16 压栈",
                dvItem(0, DV_FOUND, 3));
        }

        dvStep(s, "求值完毕, 结果为 16");
    }
    return 0;
}
$C$, $E$### C 实现

`dvInt()` 从 `dvValue` 提取整数值，`DV_ACTIVE` 标记中间结果，`DV_FOUND` 标记最终结果。$E$);

-- =====================================================
-- 分类: 队列 (5)
-- =====================================================

-- [7] 队列的基本操作
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
    q.dequeue()
    q.dequeue()
$C$, $E$### Python 实现

三个元素依次入队再依次出队，先入先出。$E$);

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
        dvQueueDequeue(q);
        dvQueueDequeue(q);
    }
    return 0;
}
$C$, $E$### C 实现

FIFO 基本操作模板。$E$);

-- [8] 队列的操作可视化
INSERT INTO templates (title, category, description) VALUES (
'队列的操作可视化', '队列',
$D$在入队和出队时使用高亮标记，直观展示队首和队尾的变化。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'python', $C$import ds4viz as dv

dv.config(title="队列操作可视化")

with dv.queue(label="vis_queue") as q:
    q.enqueue("甲")
    q.amend(note="甲 入队", highlights=[dv.item(0, "focus", level=3)])

    q.enqueue("乙")
    q.amend(note="乙 入队", highlights=[
        dv.item(0, "visited"),
        dv.item(1, "focus", level=3)
    ])

    q.enqueue("丙")
    q.amend(note="丙 入队", highlights=[
        dv.item(0, "visited"),
        dv.item(1, "visited"),
        dv.item(2, "focus", level=3)
    ])

    val = q.dequeue()
    q.amend(note=f"出队: {val}", highlights=[dv.item(0, "focus", level=3)])

    val = q.dequeue()
    q.amend(note=f"出队: {val}", highlights=[dv.item(0, "focus", level=3)])
$C$, $E$### Python 实现

`dv.item()` 高亮队列中指定索引的元素，出队后索引自动重排。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'c', $C$#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"

int main(void) {
    dvConfig((dvConfigOptions){ .title = "队列操作可视化" });

    dvQueue(q, "vis_queue") {
        dvQueueEnqueue(q, "甲");
        dvAmend(q, "甲 入队", dvItem(0, DV_FOCUS, 3));

        dvQueueEnqueue(q, "乙");
        dvAmend(q, "乙 入队",
            dvItem(0, DV_VISITED),
            dvItem(1, DV_FOCUS, 3));

        dvQueueEnqueue(q, "丙");
        dvAmend(q, "丙 入队",
            dvItem(0, DV_VISITED),
            dvItem(1, DV_VISITED),
            dvItem(2, DV_FOCUS, 3));

        dvQueueDequeue(q);
        dvAmend(q, "出队: 甲", dvItem(0, DV_FOCUS, 3));

        dvQueueDequeue(q);
        dvAmend(q, "出队: 乙", dvItem(0, DV_FOCUS, 3));
    }
    return 0;
}
$C$, $E$### C 实现

`dvItem` 配合 `dvAmend` 为每步操作附加高亮。$E$);

-- [9] 任务调度模拟
INSERT INTO templates (title, category, description) VALUES (
'任务调度模拟', '队列',
$D$模拟先来先服务 (FCFS) 的任务调度：任务入队、处理、新任务追加。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'python', $C$import ds4viz as dv

dv.config(title="任务调度")

with dv.queue(label="task_queue") as q:
    with q.phase("初始任务入队"):
        q.enqueue("下载文件")
        q.enqueue("解压缩")
        q.enqueue("编译")

    with q.phase("处理任务"):
        val = q.dequeue()
        q.amend(note=f"执行: {val}", highlights=[dv.item(0, "focus", level=3)])

        q.enqueue("测试")
        q.amend(note="新增任务: 测试")

        val = q.dequeue()
        q.amend(note=f"执行: {val}", highlights=[dv.item(0, "focus", level=3)])

        val = q.dequeue()
        q.amend(note=f"执行: {val}", highlights=[dv.item(0, "focus", level=3)])

        val = q.dequeue()
        q.amend(note=f"执行: {val}")

    q.step(note="所有任务已完成")
$C$, $E$### Python 实现

`phase()` 区分"入队"和"处理"阶段，展示 FCFS 调度流程。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'c', $C$#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"

int main(void) {
    dvConfig((dvConfigOptions){ .title = "任务调度" });

    dvQueue(q, "task_queue") {
        dvPhase(q, "初始任务入队") {
            dvQueueEnqueue(q, "下载文件");
            dvQueueEnqueue(q, "解压缩");
            dvQueueEnqueue(q, "编译");
        }

        dvPhase(q, "处理任务") {
            dvQueueDequeue(q);
            dvAmend(q, "执行: 下载文件", dvItem(0, DV_FOCUS, 3));

            dvQueueEnqueue(q, "测试");
            dvAmend(q, "新增任务: 测试");

            dvQueueDequeue(q);
            dvAmend(q, "执行: 解压缩", dvItem(0, DV_FOCUS, 3));

            dvQueueDequeue(q);
            dvAmend(q, "执行: 编译", dvItem(0, DV_FOCUS, 3));

            dvQueueDequeue(q);
            dvAmend(q, "执行: 测试");
        }

        dvStep(q, "所有任务已完成");
    }
    return 0;
}
$C$, $E$### C 实现

阶段划分 + 高亮跟踪当前处理的队首任务。$E$);

-- [10] 约瑟夫环问题
INSERT INTO templates (title, category, description) VALUES (
'约瑟夫环问题', '队列',
$D$用队列模拟 6 人围坐、每隔 2 人淘汰 1 人的约瑟夫环过程。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'python', $C$import ds4viz as dv

dv.config(title="约瑟夫环 (n=6, k=3)")

with dv.queue(label="josephus") as q:
    with q.phase("初始化"):
        for i in range(1, 7):
            q.enqueue(i)

    with q.phase("淘汰过程"):
        for round_num in range(5):
            val = q.dequeue()
            q.enqueue(val)
            q.amend(note=f"跳过 {val}")

            val = q.dequeue()
            q.enqueue(val)
            q.amend(note=f"跳过 {val}")

            val = q.dequeue()
            q.amend(note=f"淘汰 {val}", highlights=[dv.item(0, "focus", level=3)])
$C$, $E$### Python 实现

队列轮转模拟约瑟夫过程：跳过 2 人后淘汰第 3 人。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'c', $C$#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"

int main(void) {
    dvConfig((dvConfigOptions){ .title = "约瑟夫环 (n=6, k=3)" });

    dvQueue(q, "josephus") {
        dvPhase(q, "初始化") {
            for (int i = 1; i <= 6; i++) dvQueueEnqueue(q, i);
        }

        dvPhase(q, "淘汰过程") {
            for (int r = 0; r < 5; r++) {
                dvValue v1 = dvQueueDequeue(q);
                dvQueueEnqueue(q, dvInt(v1));
                dvAmend(q, "跳过");

                dvValue v2 = dvQueueDequeue(q);
                dvQueueEnqueue(q, dvInt(v2));
                dvAmend(q, "跳过");

                dvQueueDequeue(q);
                dvAmend(q, "淘汰", dvItem(0, DV_FOCUS, 3));
            }
        }
    }
    return 0;
}
$C$, $E$### C 实现

出队重入队模拟跳过，直接出队模拟淘汰。$E$);

-- [11] 打印队列模拟
INSERT INTO templates (title, category, description) VALUES (
'打印队列模拟', '队列',
$D$模拟打印机任务队列：多个任务到达并按序处理。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'python', $C$import ds4viz as dv

dv.config(title="打印队列")

with dv.queue(label="print_queue") as q:
    with q.phase("任务到达"):
        q.enqueue("report.pdf")
        q.amend(note="report.pdf 加入队列")
        q.enqueue("photo.png")
        q.amend(note="photo.png 加入队列")

    with q.phase("处理并追加"):
        val = q.dequeue()
        q.amend(note=f"打印完成: {val}", highlights=[dv.item(0, "active", level=3)])

        q.enqueue("slides.pptx")
        q.amend(note="slides.pptx 加入队列",
                highlights=[dv.item(1, "focus", level=3)])

        val = q.dequeue()
        q.amend(note=f"打印完成: {val}")

        val = q.dequeue()
        q.amend(note=f"打印完成: {val}")

    q.step(note="队列清空, 所有任务完成")
$C$, $E$### Python 实现

分阶段展示任务到达与处理，高亮当前正在打印的文件。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'c', $C$#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"

int main(void) {
    dvConfig((dvConfigOptions){ .title = "打印队列" });

    dvQueue(q, "print_queue") {
        dvPhase(q, "任务到达") {
            dvQueueEnqueue(q, "report.pdf");
            dvAmend(q, "report.pdf 加入队列");
            dvQueueEnqueue(q, "photo.png");
            dvAmend(q, "photo.png 加入队列");
        }

        dvPhase(q, "处理并追加") {
            dvQueueDequeue(q);
            dvAmend(q, "打印完成: report.pdf",
                dvItem(0, DV_ACTIVE, 3));

            dvQueueEnqueue(q, "slides.pptx");
            dvAmend(q, "slides.pptx 加入队列",
                dvItem(1, DV_FOCUS, 3));

            dvQueueDequeue(q);
            dvAmend(q, "打印完成: photo.png");

            dvQueueDequeue(q);
            dvAmend(q, "打印完成: slides.pptx");
        }

        dvStep(q, "队列清空, 所有任务完成");
    }
    return 0;
}
$C$, $E$### C 实现

打印任务 FCFS 调度演示。$E$);

-- =====================================================
-- 分类: 单链表 (5)
-- =====================================================

-- [12] 单链表的构建
INSERT INTO templates (title, category, description) VALUES (
'单链表的构建', '单链表',
$D$演示头插法与尾插法构建单链表。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'python', $C$import ds4viz as dv

with dv.single_linked_list(label="build_slist") as l:
    l.insert_head(30)
    l.insert_head(20)
    l.insert_head(10)
    l.insert_tail(40)
    l.insert_tail(50)
$C$, $E$### Python 实现

头插三次后尾插两次，链表最终为 10→20→30→40→50。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'c', $C$#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"

int main(void) {
    dvSingleLinkedList(l, "build_slist") {
        dvSlInsertHead(l, 30);
        dvSlInsertHead(l, 20);
        dvSlInsertHead(l, 10);
        dvSlInsertTail(l, 40);
        dvSlInsertTail(l, 50);
    }
    return 0;
}
$C$, $E$### C 实现

`dvSlInsertHead` 头插、`dvSlInsertTail` 尾插。$E$);

-- [13] 单链表的插入与删除
INSERT INTO templates (title, category, description) VALUES (
'单链表的插入与删除', '单链表',
$D$演示 insert_after、delete、delete_head 等动态操作，配合别名和高亮。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'python', $C$import ds4viz as dv

dv.config(title="单链表插入与删除")

with dv.single_linked_list(label="modify_slist") as l:
    a = l.insert_head(10)
    l.alias(a, "head")
    b = l.insert_tail(20)
    c = l.insert_tail(40)

    d = l.insert_after(b, 30)
    l.amend(note="在 20 后插入 30", highlights=[
        dv.node(b, "active"),
        dv.node(d, "focus", level=3)
    ])

    l.delete(c)
    l.amend(note="删除节点 40", highlights=[
        dv.node(a, "visited")
    ])

    l.delete_head()
    l.amend(note="删除头节点 10")
$C$, $E$### Python 实现

`alias()` 为头节点设置别名，`insert_after` 在指定节点后插入，高亮展示操作目标。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'c', $C$#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"

int main(void) {
    dvConfig((dvConfigOptions){ .title = "单链表插入与删除" });

    dvSingleLinkedList(l, "modify_slist") {
        int a = dvSlInsertHead(l, 10);
        dvAlias(l, a, "head");
        int b = dvSlInsertTail(l, 20);
        int c = dvSlInsertTail(l, 40);

        int d = dvSlInsertAfter(l, b, 30);
        dvAmend(l, "在 20 后插入 30",
            dvNode(b, DV_ACTIVE),
            dvNode(d, DV_FOCUS, 3));

        dvSlDelete(l, c);
        dvAmend(l, "删除节点 40",
            dvNode(a, DV_VISITED));

        dvSlDeleteHead(l);
        dvAmend(l, "删除头节点 10");
    }
    return 0;
}
$C$, $E$### C 实现

`dvAlias` 设置别名，`dvNode` 高亮节点标记操作位置。$E$);

-- [14] 单链表反转
INSERT INTO templates (title, category, description) VALUES (
'单链表反转', '单链表',
$D$构建有序链表后执行 reverse，使用阶段和高亮展示反转前后的状态。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'python', $C$import ds4viz as dv

dv.config(title="单链表遍历")

with dv.single_linked_list(label="traverse_slist") as l:
    with l.phase("构建"):
        a = l.insert_head("甲")
        b = l.insert_tail("乙")
        c = l.insert_tail("丙")
        d = l.insert_tail("丁")

    visited = []

    def visit(nid, label):
        l.step(note=f"访问 {label}", highlights=[
            *[dv.node(v, "visited") for v in visited],
            dv.node(nid, "focus", level=3)
        ])
        visited.append(nid)

    with l.phase("遍历"):
        visit(a, "甲")
        visit(b, "乙")
        visit(c, "丙")
        visit(d, "丁")
$C$, $E$### Python 实现

阶段划分"构建"与"反转"，高亮标识头尾节点位置的互换。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'c', $C$#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"
#include <stdio.h>

#define SLIST_VISIT_LINE 8

#define VISIT(obj, label, ...)                                           \
    do {                                                                 \
        char _dv_note[64];                                               \
        snprintf(_dv_note, sizeof _dv_note, "访问 %s", (label));         \
        dvStepAt((obj), SLIST_VISIT_LINE, _dv_note, __VA_ARGS__);       \
    } while (0)

int main(void) {
    dvConfig((dvConfigOptions){ .title = "单链表遍历" });

    dvSingleLinkedList(l, "traverse_slist") {
        int a, b, c, d;
        dvPhase(l, "构建") {
            a = dvSlInsertHead(l, "甲");
            b = dvSlInsertTail(l, "乙");
            c = dvSlInsertTail(l, "丙");
            d = dvSlInsertTail(l, "丁");
        }

        dvPhase(l, "遍历") {
            VISIT(l, "甲",
                dvNode(a, DV_FOCUS, 3));
            VISIT(l, "乙",
                dvNode(a, DV_VISITED),
                dvNode(b, DV_FOCUS, 3));
            VISIT(l, "丙",
                dvNode(a, DV_VISITED), dvNode(b, DV_VISITED),
                dvNode(c, DV_FOCUS, 3));
            VISIT(l, "丁",
                dvNode(a, DV_VISITED), dvNode(b, DV_VISITED), dvNode(c, DV_VISITED),
                dvNode(d, DV_FOCUS, 3));
        }
    }
    return 0;
}
$C$, $E$### C 实现

`dvSlReverse` 一步完成反转，高亮显示头尾互换。$E$);

-- [15] 单链表遍历
INSERT INTO templates (title, category, description) VALUES (
'单链表遍历', '单链表',
$D$逐节点遍历单链表，使用 step 和高亮展示遍历进度。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'python', $C$import ds4viz as dv

dv.config(title="单链表遍历")

with dv.single_linked_list(label="traverse_slist") as l:
    with l.phase("构建"):
        a = l.insert_head("甲")
        b = l.insert_tail("乙")
        c = l.insert_tail("丙")
        d = l.insert_tail("丁")

    visited = []
    nodes = [(a, "甲"), (b, "乙"), (c, "丙"), (d, "丁")]

    with l.phase("遍历"):
        for node_id, label in nodes:
            l.step(
                note=f"访问 {label}",
                highlights=[
                    *[dv.node(v, "visited") for v in visited],
                    dv.node(node_id, "focus", level=3)
                ]
            )
            visited.append(node_id)
$C$, $E$### Python 实现

`step()` 不修改结构，仅记录观察步骤。每步高亮已访问节点（visited）和当前节点（focus）。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'c', $C$#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"

int main(void) {
    dvConfig((dvConfigOptions){ .title = "单链表遍历" });

    dvSingleLinkedList(l, "traverse_slist") {
        int a, b, c, d;
        dvPhase(l, "构建") {
            a = dvSlInsertHead(l, "甲");
            b = dvSlInsertTail(l, "乙");
            c = dvSlInsertTail(l, "丙");
            d = dvSlInsertTail(l, "丁");
        }

        dvPhase(l, "遍历") {
            dvStep(l, "访问 甲",
                dvNode(a, DV_FOCUS, 3));

            dvStep(l, "访问 乙",
                dvNode(a, DV_VISITED),
                dvNode(b, DV_FOCUS, 3));

            dvStep(l, "访问 丙",
                dvNode(a, DV_VISITED),
                dvNode(b, DV_VISITED),
                dvNode(c, DV_FOCUS, 3));

            dvStep(l, "访问 丁",
                dvNode(a, DV_VISITED),
                dvNode(b, DV_VISITED),
                dvNode(c, DV_VISITED),
                dvNode(d, DV_FOCUS, 3));
        }
    }
    return 0;
}
$C$, $E$### C 实现

`dvStep` 记录观察步骤，逐步展示遍历进度。$E$);

-- [16] 单链表查找节点
INSERT INTO templates (title, category, description) VALUES (
'单链表查找节点', '单链表',
$D$在单链表中查找值为 30 的节点，使用 comparing 和 found 高亮展示查找路径。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'python', $C$import ds4viz as dv

dv.config(title="单链表查找: 目标值 30")

with dv.single_linked_list(label="search_slist") as l:
    with l.phase("构建"):
        n10 = l.insert_tail(10)
        n20 = l.insert_tail(20)
        n30 = l.insert_tail(30)
        n40 = l.insert_tail(40)

    with l.phase("查找"):
        l.step(note="比较 10 != 30", highlights=[
            dv.node(n10, "comparing", level=3)
        ])
        l.step(note="比较 20 != 30", highlights=[
            dv.node(n10, "visited"),
            dv.node(n20, "comparing", level=3)
        ])
        l.step(note="比较 30 == 30, 找到!", highlights=[
            dv.node(n10, "visited"),
            dv.node(n20, "visited"),
            dv.node(n30, "found", level=3)
        ])
$C$, $E$### Python 实现

使用 `comparing` 表示当前正在比较的节点，`found` 表示查找命中。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'c', $C$#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"

int main(void) {
    dvConfig((dvConfigOptions){ .title = "单链表查找: 目标值 30" });

    dvSingleLinkedList(l, "search_slist") {
        int n10, n20, n30, n40;
        dvPhase(l, "构建") {
            n10 = dvSlInsertTail(l, 10);
            n20 = dvSlInsertTail(l, 20);
            n30 = dvSlInsertTail(l, 30);
            n40 = dvSlInsertTail(l, 40);
        }

        dvPhase(l, "查找") {
            dvStep(l, "比较 10 != 30",
                dvNode(n10, DV_COMPARING, 3));

            dvStep(l, "比较 20 != 30",
                dvNode(n10, DV_VISITED),
                dvNode(n20, DV_COMPARING, 3));

            dvStep(l, "比较 30 == 30, 找到!",
                dvNode(n10, DV_VISITED),
                dvNode(n20, DV_VISITED),
                dvNode(n30, DV_FOUND, 3));
        }
    }
    return 0;
}
$C$, $E$### C 实现

`DV_COMPARING` 标识当前比较节点，`DV_FOUND` 标识命中节点。$E$);

-- =====================================================
-- 分类: 双向链表 (4)
-- =====================================================

-- [17] 双向链表的基本操作
INSERT INTO templates (title, category, description) VALUES (
'双向链表的基本操作', '双向链表',
$D$演示双向链表头尾插入、前后插入、头尾删除与按节点删除。$D$
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

覆盖双向链表所有核心操作。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'c', $C$#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"

int main(void) {
    dvDoubleLinkedList(l, "demo_dlist") {
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

双向链表全操作演示模板。$E$);

-- [18] 双向链表的动态操作
INSERT INTO templates (title, category, description) VALUES (
'双向链表的动态操作', '双向链表',
$D$分阶段构建和修改双向链表，使用别名和高亮展示操作位置。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'python', $C$import ds4viz as dv

dv.config(title="双向链表动态操作")

with dv.double_linked_list(label="dynamic_dlist") as l:
    with l.phase("构建"):
        a = l.insert_head(10)
        l.alias(a, "head")
        b = l.insert_tail(30)
        c = l.insert_before(b, 20)
        l.amend(note="在 30 前插入 20", highlights=[
            dv.node(c, "focus", level=3),
            dv.node(b, "active")
        ])

    with l.phase("修改"):
        d = l.insert_after(a, 15)
        l.amend(note="在 10 后插入 15", highlights=[
            dv.node(a, "active"),
            dv.node(d, "focus", level=3)
        ])
        l.delete_tail()
        l.amend(note="删除尾节点 30")
$C$, $E$### Python 实现

`alias()` 标识头节点，`phase()` 划分构建与修改阶段。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'c', $C$#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"

int main(void) {
    dvConfig((dvConfigOptions){ .title = "双向链表动态操作" });

    dvDoubleLinkedList(l, "dynamic_dlist") {
        int a, b, c, d;
        dvPhase(l, "构建") {
            a = dvDlInsertHead(l, 10);
            dvAlias(l, a, "head");
            b = dvDlInsertTail(l, 30);
            c = dvDlInsertBefore(l, b, 20);
            dvAmend(l, "在 30 前插入 20",
                dvNode(c, DV_FOCUS, 3),
                dvNode(b, DV_ACTIVE));
        }

        dvPhase(l, "修改") {
            d = dvDlInsertAfter(l, a, 15);
            dvAmend(l, "在 10 后插入 15",
                dvNode(a, DV_ACTIVE),
                dvNode(d, DV_FOCUS, 3));
            dvDlDeleteTail(l);
            dvAmend(l, "删除尾节点 30");
        }
    }
    return 0;
}
$C$, $E$### C 实现

`dvAlias` 设置别名供渲染器展示，`dvPhase` 划分操作阶段。$E$);

-- [19] 双向链表反转
INSERT INTO templates (title, category, description) VALUES (
'双向链表反转', '双向链表',
$D$构建双向链表后执行 reverse，观察 prev/next 对调效果。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'python', $C$import ds4viz as dv

dv.config(title="双向链表反转")

with dv.double_linked_list(label="reverse_dlist") as l:
    with l.phase("构建"):
        n1 = l.insert_tail(1)
        n2 = l.insert_tail(2)
        n3 = l.insert_tail(3)
        n4 = l.insert_tail(4)

    with l.phase("反转"):
        l.reverse()
        l.amend(note="反转完成: 4<->3<->2<->1")
$C$, $E$### Python 实现

`reverse()` 一步完成所有指针对调。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'c', $C$#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"

int main(void) {
    dvConfig((dvConfigOptions){ .title = "双向链表反转" });

    dvDoubleLinkedList(l, "reverse_dlist") {
        dvPhase(l, "构建") {
            dvDlInsertTail(l, 1);
            dvDlInsertTail(l, 2);
            dvDlInsertTail(l, 3);
            dvDlInsertTail(l, 4);
        }

        dvPhase(l, "反转") {
            dvDlReverse(l);
            dvAmend(l, "反转完成: 4<->3<->2<->1");
        }
    }
    return 0;
}
$C$, $E$### C 实现

双向链表反转，前驱/后继指针全部对调。$E$);

-- [20] 双端队列模拟
INSERT INTO templates (title, category, description) VALUES (
'双端队列模拟', '双向链表',
$D$利用双向链表头尾 O(1) 操作模拟双端队列 (deque)。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'python', $C$import ds4viz as dv

dv.config(title="双端队列模拟")

with dv.double_linked_list(label="deque_sim") as l:
    with l.phase("尾部入队"):
        l.insert_tail(1)
        l.insert_tail(2)
        l.insert_tail(3)

    with l.phase("头部入队"):
        l.insert_head(0)
        l.amend(note="从头部插入 0")

    with l.phase("双端出队"):
        l.delete_head()
        l.amend(note="从头部弹出",
                highlights=[dv.node(l.insert_tail(99), "error")])
        l.delete_tail()
        l.amend(note="从尾部弹出")
        l.delete_tail()
        l.amend(note="从尾部弹出")
$C$, $E$### Python 实现

双向链表天然支持头尾双端操作，模拟 deque 行为。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'c', $C$#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"

int main(void) {
    dvConfig((dvConfigOptions){ .title = "双端队列模拟" });

    dvDoubleLinkedList(l, "deque_sim") {
        dvPhase(l, "尾部入队") {
            dvDlInsertTail(l, 1);
            dvDlInsertTail(l, 2);
            dvDlInsertTail(l, 3);
        }

        dvPhase(l, "头部入队") {
            dvDlInsertHead(l, 0);
            dvAmend(l, "从头部插入 0");
        }

        dvPhase(l, "双端出队") {
            dvDlDeleteHead(l);
            dvAmend(l, "从头部弹出");
            dvDlDeleteTail(l);
            dvAmend(l, "从尾部弹出");
            dvDlDeleteHead(l);
            dvAmend(l, "从头部弹出");
        }
    }
    return 0;
}
$C$, $E$### C 实现

头尾插入/删除均为常量时间，完美模拟双端队列。$E$);

-- =====================================================
-- 分类: 二叉树 (7)
-- =====================================================

-- [21] 二叉树的手动构建
INSERT INTO templates (title, category, description) VALUES (
'二叉树的手动构建', '二叉树',
$D$通过 insert_root / insert_left / insert_right 手动构建满二叉树，并使用别名标注根节点。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'python', $C$import ds4viz as dv

with dv.binary_tree(label="manual_tree") as t:
    r = t.insert_root(1)
    t.alias(r, "root")
    l = t.insert_left(r, 2)
    rr = t.insert_right(r, 3)
    t.insert_left(l, 4)
    t.insert_right(l, 5)
    t.insert_left(rr, 6)
    t.insert_right(rr, 7)
$C$, $E$### Python 实现

手动构建三层满二叉树，`alias()` 标注根节点。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'c', $C$#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"

int main(void) {
    dvBinaryTree(t, "manual_tree") {
        int r = dvBtInsertRoot(t, 1);
        dvAlias(t, r, "root");
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

层次构建完全二叉树。$E$);

-- [22] 表达式树
INSERT INTO templates (title, category, description) VALUES (
'表达式树', '二叉树',
$D$构建表达式 (3 + 5) * 2 对应的二叉表达式树，运算符为内部节点。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'python', $C$import ds4viz as dv

dv.config(title="表达式树: (3+5)*2")

with dv.binary_tree(label="expr_tree") as t:
    mul = t.insert_root("*")
    t.alias(mul, "root")
    plus = t.insert_left(mul, "+")
    t.insert_right(mul, 2)
    t.insert_left(plus, 3)
    t.insert_right(plus, 5)

    t.step(note="后序求值: 3, 5, +, 2, * => 16", highlights=[
        dv.node(mul, "found", level=3)
    ])
$C$, $E$### Python 实现

运算符作为内部节点，操作数作为叶节点，后序遍历即为后缀表达式。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'c', $C$#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"

int main(void) {
    dvConfig((dvConfigOptions){ .title = "表达式树: (3+5)*2" });

    dvBinaryTree(t, "expr_tree") {
        int mul = dvBtInsertRoot(t, "*");
        dvAlias(t, mul, "root");
        int plus = dvBtInsertLeft(t, mul, "+");
        dvBtInsertRight(t, mul, 2);
        dvBtInsertLeft(t, plus, 3);
        dvBtInsertRight(t, plus, 5);

        dvStep(t, "后序求值: 3, 5, +, 2, * => 16",
            dvNode(mul, DV_FOUND, 3));
    }
    return 0;
}
$C$, $E$### C 实现

表达式树：字符串与数字混合值，`DV_FOUND` 标记最终结果节点。$E$);

-- [23] 二叉树的修改与删除
INSERT INTO templates (title, category, description) VALUES (
'二叉树的修改与删除', '二叉树',
$D$演示 update_value 修改节点值和 delete 删除子树的操作。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'python', $C$import ds4viz as dv

dv.config(title="二叉树修改与删除")

with dv.binary_tree(label="modify_tree") as t:
    r = t.insert_root(1)
    t.alias(r, "root")
    a = t.insert_left(r, 2)
    b = t.insert_right(r, 3)
    t.insert_left(a, 4)
    t.insert_right(a, 5)

    t.update_value(b, 30)
    t.amend(note="将节点 3 的值更新为 30", highlights=[
        dv.node(b, "focus", level=3)
    ])

    t.delete(a)
    t.amend(note="删除节点 2 及其整棵子树", highlights=[
        dv.node(r, "active")
    ])
$C$, $E$### Python 实现

`update_value()` 修改值、`delete()` 级联删除子树。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'c', $C$#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"

int main(void) {
    dvConfig((dvConfigOptions){ .title = "二叉树修改与删除" });

    dvBinaryTree(t, "modify_tree") {
        int r = dvBtInsertRoot(t, 1);
        dvAlias(t, r, "root");
        int a = dvBtInsertLeft(t, r, 2);
        int b = dvBtInsertRight(t, r, 3);
        dvBtInsertLeft(t, a, 4);
        dvBtInsertRight(t, a, 5);

        dvBtUpdateValue(t, b, 30);
        dvAmend(t, "将节点 3 的值更新为 30",
            dvNode(b, DV_FOCUS, 3));

        dvBtDelete(t, a);
        dvAmend(t, "删除节点 2 及其整棵子树",
            dvNode(r, DV_ACTIVE));
    }
    return 0;
}
$C$, $E$### C 实现

`dvBtUpdateValue` 修改节点值，`dvBtDelete` 删除子树。$E$);

-- [24] 二叉树前序遍历
INSERT INTO templates (title, category, description) VALUES (
'二叉树前序遍历', '二叉树',
$D$构建二叉树后使用 step 逐步记录前序遍历（根→左→右）的访问过程。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'python', $C$import ds4viz as dv

dv.config(title="二叉树前序遍历", author="ds4viz")

with dv.binary_tree(label="preorder") as tree:
    with tree.phase("构建"):
        root = tree.insert_root(10)
        tree.alias(root, "root")
        n5 = tree.insert_left(root, 5)
        n15 = tree.insert_right(root, 15)
        n3 = tree.insert_left(n5, 3)
        n7 = tree.insert_right(n5, 7)
        n12 = tree.insert_left(n15, 12)

    visited = []
    def visit(nid, val):
        tree.step(note=f"访问节点 {val}", highlights=[
            *[dv.node(v, "visited") for v in visited],
            dv.node(nid, "focus", level=3)
        ])
        visited.append(nid)

    with tree.phase("前序遍历"):
        visit(root, 10)
        visit(n5, 5)
        visit(n3, 3)
        visit(n7, 7)
        visit(n15, 15)
        visit(n12, 12)
$C$, $E$### Python 实现

前序遍历顺序：根→左→右。使用辅助函数 `visit()` 逐步记录并累积已访问高亮。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'c', $C$#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"
#include <stdio.h>

/* 固定逻辑行号，便于与封装调用场景对齐 */
#define PREORDER_VISIT_LINE 17

#define VISIT(obj, nid, val, ...)                                  \
    do {                                                           \
        char _dv_note[64];                                         \
        snprintf(_dv_note, sizeof _dv_note, "访问节点 %d", (val)); \
        dvStepAt((obj), PREORDER_VISIT_LINE, _dv_note, __VA_ARGS__); \
    } while (0)

int main(void) {
    dvConfig((dvConfigOptions){ .title = "二叉树前序遍历", .author = "ds4viz" });

    dvBinaryTree(tree, "preorder") {
        int root, n5, n15, n3, n7, n12;

        dvPhase(tree, "构建") {
            root = dvBtInsertRoot(tree, 10);
            dvAlias(tree, root, "root");
            n5   = dvBtInsertLeft(tree, root, 5);
            n15  = dvBtInsertRight(tree, root, 15);
            n3   = dvBtInsertLeft(tree, n5, 3);
            n7   = dvBtInsertRight(tree, n5, 7);
            n12  = dvBtInsertLeft(tree, n15, 12);
        }

        dvPhase(tree, "前序遍历") {
            VISIT(tree, root, 10,
                dvNode(root, DV_FOCUS, 3));

            VISIT(tree, n5, 5,
                dvNode(root, DV_VISITED),
                dvNode(n5, DV_FOCUS, 3));

            VISIT(tree, n3, 3,
                dvNode(root, DV_VISITED),
                dvNode(n5, DV_VISITED),
                dvNode(n3, DV_FOCUS, 3));

            VISIT(tree, n7, 7,
                dvNode(root, DV_VISITED),
                dvNode(n5, DV_VISITED),
                dvNode(n3, DV_VISITED),
                dvNode(n7, DV_FOCUS, 3));

            VISIT(tree, n15, 15,
                dvNode(root, DV_VISITED),
                dvNode(n5, DV_VISITED),
                dvNode(n3, DV_VISITED),
                dvNode(n7, DV_VISITED),
                dvNode(n15, DV_FOCUS, 3));

            VISIT(tree, n12, 12,
                dvNode(root, DV_VISITED),
                dvNode(n5, DV_VISITED),
                dvNode(n3, DV_VISITED),
                dvNode(n7, DV_VISITED),
                dvNode(n15, DV_VISITED),
                dvNode(n12, DV_FOCUS, 3));
        }
    }
    return 0;
}
$C$, $E$### C 实现

使用封装宏 `VISIT` + `dvStepAt` 固定 `steps.code.line`，便于在封装调用场景下保持行号语义稳定；
同时每步仍累积 `DV_VISITED`，当前节点标记 `DV_FOCUS`。$E$);

-- [25] 二叉树中序遍历
INSERT INTO templates (title, category, description) VALUES (
'二叉树中序遍历', '二叉树',
$D$中序遍历（左→根→右），对 BST 可得到有序序列。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'python', $C$import ds4viz as dv


dv.config(title="二叉树中序遍历")

with dv.binary_tree(label="inorder") as tree:
    with tree.phase("构建"):
        root = tree.insert_root("A")
        tree.alias(root, "root")
        b = tree.insert_left(root, "B")
        c = tree.insert_right(root, "C")
        d = tree.insert_left(b, "D")
        e = tree.insert_right(b, "E")

    # 中序: D, B, E, A, C
    visited = []

    def visit(nid, val):
        tree.step(note=f"访问 {val}", highlights=[
            *[dv.node(v, "visited") for v in visited],
            dv.node(nid, "focus", level=3)
        ])
        visited.append(nid)

    with tree.phase("中序遍历"):
        visit(d, "D")
        visit(b, "B")
        visit(e, "E")
        visit(root, "A")
        visit(c, "C")
$C$, $E$### Python 实现

中序遍历：左→根→右。遍历顺序 D→B→E→A→C。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'c', $C$#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"
#include <stdio.h>

#define INORDER_VISIT_LINE 8

#define VISIT(obj, val, ...)                                             \
    do {                                                                 \
        char _dv_note[64];                                               \
        snprintf(_dv_note, sizeof _dv_note, "访问 %s", (val));           \
        dvStepAt((obj), INORDER_VISIT_LINE, _dv_note, __VA_ARGS__);     \
    } while (0)

int main(void) {
    dvConfig((dvConfigOptions){ .title = "二叉树中序遍历" });

    dvBinaryTree(tree, "inorder") {
        int root, b, c, d, e;
        dvPhase(tree, "构建") {
            root = dvBtInsertRoot(tree, "A");
            dvAlias(tree, root, "root");
            b = dvBtInsertLeft(tree, root, "B");
            c = dvBtInsertRight(tree, root, "C");
            d = dvBtInsertLeft(tree, b, "D");
            e = dvBtInsertRight(tree, b, "E");
        }

        dvPhase(tree, "中序遍历") {
            VISIT(tree, "D",
                dvNode(d, DV_FOCUS, 3));
            VISIT(tree, "B",
                dvNode(d, DV_VISITED),
                dvNode(b, DV_FOCUS, 3));
            VISIT(tree, "E",
                dvNode(d, DV_VISITED), dvNode(b, DV_VISITED),
                dvNode(e, DV_FOCUS, 3));
            VISIT(tree, "A",
                dvNode(d, DV_VISITED), dvNode(b, DV_VISITED), dvNode(e, DV_VISITED),
                dvNode(root, DV_FOCUS, 3));
            VISIT(tree, "C",
                dvNode(d, DV_VISITED), dvNode(b, DV_VISITED), dvNode(e, DV_VISITED),
                dvNode(root, DV_VISITED),
                dvNode(c, DV_FOCUS, 3));
        }
    }
    return 0;
}

$C$, $E$### C 实现

中序遍历逐步展示，最终所有节点标记为 `DV_VISITED`。$E$);

-- [26] 二叉树后序遍历
INSERT INTO templates (title, category, description) VALUES (
'二叉树后序遍历', '二叉树',
$D$后序遍历（左→右→根），常用于释放树节点或计算子树属性。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'python', $C$import ds4viz as dv

dv.config(title="二叉树后序遍历")

with dv.binary_tree(label="postorder") as tree:
    with tree.phase("构建"):
        root = tree.insert_root("A")
        tree.alias(root, "root")
        b = tree.insert_left(root, "B")
        c = tree.insert_right(root, "C")
        d = tree.insert_left(b, "D")
        e = tree.insert_right(b, "E")

    # 后序: D, E, B, C, A
    visited = []

    def visit(nid, val):
        tree.step(note=f"访问 {val}", highlights=[
            *[dv.node(v, "visited") for v in visited],
            dv.node(nid, "focus", level=3)
        ])
        visited.append(nid)

    with tree.phase("后序遍历"):
        visit(d, "D")
        visit(e, "E")
        visit(b, "B")
        visit(c, "C")
        visit(root, "A")

$C$, $E$### Python 实现

后序遍历：左→右→根。根节点最后被访问。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'c', $C$#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"
#include <stdio.h>

#define POSTORDER_VISIT_LINE 8

#define VISIT(obj, val, ...)                                               \
    do {                                                                   \
        char _dv_note[64];                                                 \
        snprintf(_dv_note, sizeof _dv_note, "访问 %s", (val));             \
        dvStepAt((obj), POSTORDER_VISIT_LINE, _dv_note, __VA_ARGS__);     \
    } while (0)

int main(void) {
    dvConfig((dvConfigOptions){ .title = "二叉树后序遍历" });

    dvBinaryTree(tree, "postorder") {
        int root, b, c, d, e;
        dvPhase(tree, "构建") {
            root = dvBtInsertRoot(tree, "A");
            dvAlias(tree, root, "root");
            b = dvBtInsertLeft(tree, root, "B");
            c = dvBtInsertRight(tree, root, "C");
            d = dvBtInsertLeft(tree, b, "D");
            e = dvBtInsertRight(tree, b, "E");
        }

        dvPhase(tree, "后序遍历") {
            VISIT(tree, "D",
                dvNode(d, DV_FOCUS, 3));
            VISIT(tree, "E",
                dvNode(d, DV_VISITED),
                dvNode(e, DV_FOCUS, 3));
            VISIT(tree, "B",
                dvNode(d, DV_VISITED), dvNode(e, DV_VISITED),
                dvNode(b, DV_FOCUS, 3));
            VISIT(tree, "C",
                dvNode(d, DV_VISITED), dvNode(e, DV_VISITED), dvNode(b, DV_VISITED),
                dvNode(c, DV_FOCUS, 3));
            VISIT(tree, "A",
                dvNode(d, DV_VISITED), dvNode(e, DV_VISITED), dvNode(b, DV_VISITED),
                dvNode(c, DV_VISITED),
                dvNode(root, DV_FOCUS, 3));
        }
    }
    return 0;
}
$C$, $E$### C 实现

后序遍历适用于需要先处理子树再处理根的场景。$E$);

-- [27] 二叉树层序遍历
INSERT INTO templates (title, category, description) VALUES (
'二叉树层序遍历', '二叉树',
$D$逐层从左到右访问所有节点（广度优先遍历）。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'python', $C$import ds4viz as dv


dv.config(title="二叉树层序遍历")

with dv.binary_tree(label="levelorder") as tree:
    with tree.phase("构建"):
        root = tree.insert_root(1)
        n2 = tree.insert_left(root, 2)
        n3 = tree.insert_right(root, 3)
        n4 = tree.insert_left(n2, 4)
        n5 = tree.insert_right(n2, 5)
        n6 = tree.insert_left(n3, 6)

    visited = []

    def visit_level(nodes, desc):
        tree.step(note=desc, highlights=[
            *[dv.node(v, "visited") for v in visited],
            *[dv.node(n, "focus", level=3) for n in nodes]
        ])
        visited.extend(nodes)

    with tree.phase("层序遍历"):
        visit_level([root], "第 1 层: [1]")
        visit_level([n2, n3], "第 2 层: [2, 3]")
        visit_level([n4, n5, n6], "第 3 层: [4, 5, 6]")
$C$, $E$### Python 实现

每层所有节点同时标记为 `focus`，已遍历层标记为 `visited`。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'c', $C$#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"

int main(void) {
    dvConfig((dvConfigOptions){ .title = "二叉树层序遍历" });

    dvBinaryTree(tree, "levelorder") {
        int root, n2, n3, n4, n5, n6;
        dvPhase(tree, "构建") {
            root = dvBtInsertRoot(tree, 1);
            n2   = dvBtInsertLeft(tree, root, 2);
            n3   = dvBtInsertRight(tree, root, 3);
            n4   = dvBtInsertLeft(tree, n2, 4);
            n5   = dvBtInsertRight(tree, n2, 5);
            n6   = dvBtInsertLeft(tree, n3, 6);
        }

        dvPhase(tree, "层序遍历") {
            dvStep(tree, "第 1 层: [1]",
                dvNode(root, DV_FOCUS, 3));

            dvStep(tree, "第 2 层: [2, 3]",
                dvNode(root, DV_VISITED),
                dvNode(n2, DV_FOCUS, 3),
                dvNode(n3, DV_FOCUS, 3));

            dvStep(tree, "第 3 层: [4, 5, 6]",
                dvNode(root, DV_VISITED), dvNode(n2, DV_VISITED), dvNode(n3, DV_VISITED),
                dvNode(n4, DV_FOCUS, 3),
                dvNode(n5, DV_FOCUS, 3),
                dvNode(n6, DV_FOCUS, 3));
        }
    }
    return 0;
}
$C$, $E$### C 实现

层序遍历逐层展示，同一层的所有节点同时高亮。$E$);

-- =====================================================
-- 分类: 二叉搜索树 (5)
-- =====================================================

-- [28] BST 的逐步构建
INSERT INTO templates (title, category, description) VALUES (
'BST的逐步构建', '二叉搜索树',
$D$按序插入节点，观察 BST 自动维护有序性质。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'python', $C$import ds4viz as dv

with dv.binary_search_tree(label="build_bst") as b:
    for v in [50, 30, 70, 20, 40, 60, 80]:
        b.insert(v)
$C$, $E$### Python 实现

标准 BST 构建序列，每次插入自动维护左小右大的性质。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'c', $C$#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"

int main(void) {
    dvBinarySearchTree(b, "build_bst") {
        dvBstInsert(b, 50);
        dvBstInsert(b, 30);
        dvBstInsert(b, 70);
        dvBstInsert(b, 20);
        dvBstInsert(b, 40);
        dvBstInsert(b, 60);
        dvBstInsert(b, 80);
    }
    return 0;
}
$C$, $E$### C 实现

`dvBstInsert` 自动按值大小决定插入位置。$E$);

-- [29] BST 的删除操作
INSERT INTO templates (title, category, description) VALUES (
'BST的删除操作', '二叉搜索树',
$D$演示删除叶子节点、单子节点、双子节点三种情况。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'python', $C$import ds4viz as dv

dv.config(title="BST 删除操作")

with dv.binary_search_tree(label="delete_bst") as b:
    with b.phase("构建"):
        n50 = b.insert(50)
        b.alias(n50, "root")
        b.insert(30)
        b.insert(70)
        b.insert(20)
        b.insert(40)
        b.insert(60)
        b.insert(80)

    with b.phase("删除"):
        b.delete(20)
        b.amend(note="删除叶子节点 20")

        b.delete(70)
        b.amend(note="删除双子节点 70, 用中序后继替代")

        b.delete(30)
        b.amend(note="删除单子节点 30")
$C$, $E$### Python 实现

三种典型删除场景：叶子、单子、双子，分阶段展示。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'c', $C$#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"

int main(void) {
    dvConfig((dvConfigOptions){ .title = "BST 删除操作" });

    dvBinarySearchTree(b, "delete_bst") {
        dvPhase(b, "构建") {
            int n50 = dvBstInsert(b, 50);
            dvAlias(b, n50, "root");
            dvBstInsert(b, 30); dvBstInsert(b, 70);
            dvBstInsert(b, 20); dvBstInsert(b, 40);
            dvBstInsert(b, 60); dvBstInsert(b, 80);
        }

        dvPhase(b, "删除") {
            dvBstDelete(b, 20);
            dvAmend(b, "删除叶子节点 20");

            dvBstDelete(b, 70);
            dvAmend(b, "删除双子节点 70, 用中序后继替代");

            dvBstDelete(b, 30);
            dvAmend(b, "删除单子节点 30");
        }
    }
    return 0;
}
$C$, $E$### C 实现

覆盖 BST 删除的三类分支场景。$E$);

-- [30] BST 的有序插入退化
INSERT INTO templates (title, category, description) VALUES (
'BST的有序插入退化', '二叉搜索树',
$D$按升序插入 1~8，展示 BST 退化为链表的过程。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'python', $C$import ds4viz as dv

dv.config(title="BST 退化: 有序插入")

with dv.binary_search_tree(label="degenerate_bst") as b:
    for i in range(1, 9):
        b.insert(i)
$C$, $E$### Python 实现

升序插入导致树高度退化为 O(n)，形成右斜链。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'c', $C$#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"

int main(void) {
    dvConfig((dvConfigOptions){ .title = "BST 退化: 有序插入" });

    dvBinarySearchTree(b, "degenerate_bst") {
        for (int i = 1; i <= 8; i++)
            dvBstInsert(b, i);
    }
    return 0;
}
$C$, $E$### C 实现

顺序插入使 BST 退化为链表，对比说明平衡树的必要性。$E$);

-- [31] BST 查找过程
INSERT INTO templates (title, category, description) VALUES (
'BST查找过程', '二叉搜索树',
$D$在 BST 中查找值 7，逐步展示比较路径，使用 comparing 和 found 高亮。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'python', $C$import ds4viz as dv

dv.config(title="BST 查找: 目标值 7")

with dv.binary_search_tree(label="search_bst") as bst:
    with bst.phase("构建"):
        n10 = bst.insert(10)
        bst.alias(n10, "root")
        n5  = bst.insert(5)
        n15 = bst.insert(15)
        n3  = bst.insert(3)
        n7  = bst.insert(7)
        n12 = bst.insert(12)
        n20 = bst.insert(20)

    with bst.phase("查找 7"):
        bst.step(note="7 < 10, 向左", highlights=[
            dv.node(n10, "comparing", level=3)
        ])
        bst.step(note="7 > 5, 向右", highlights=[
            dv.node(n10, "visited"),
            dv.node(n5, "comparing", level=3)
        ])
        bst.step(note="7 == 7, 找到!", highlights=[
            dv.node(n10, "visited"),
            dv.node(n5, "visited"),
            dv.node(n7, "found", level=3)
        ])
$C$, $E$### Python 实现

`comparing` 标记当前正在比较的节点，`found` 标记命中节点，展示二分查找路径。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'c', $C$#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"

int main(void) {
    dvConfig((dvConfigOptions){ .title = "BST 查找: 目标值 7" });

    dvBinarySearchTree(bst, "search_bst") {
        int n10, n5, n15, n3, n7, n12, n20;

        dvPhase(bst, "构建") {
            n10 = dvBstInsert(bst, 10);
            dvAlias(bst, n10, "root");
            n5  = dvBstInsert(bst, 5);
            n15 = dvBstInsert(bst, 15);
            n3  = dvBstInsert(bst, 3);
            n7  = dvBstInsert(bst, 7);
            n12 = dvBstInsert(bst, 12);
            n20 = dvBstInsert(bst, 20);
        }

        dvPhase(bst, "查找 7") {
            dvStep(bst, "7 < 10, 向左",
                dvNode(n10, DV_COMPARING, 3));

            dvStep(bst, "7 > 5, 向右",
                dvNode(n10, DV_VISITED),
                dvNode(n5, DV_COMPARING, 3));

            dvStep(bst, "7 == 7, 找到!",
                dvNode(n10, DV_VISITED),
                dvNode(n5, DV_VISITED),
                dvNode(n7, DV_FOUND, 3));
        }
    }
    return 0;
}
$C$, $E$### C 实现

BST 查找：每步比较缩小一半范围，时间复杂度 O(h)。$E$);

-- [32] BST 中序遍历
INSERT INTO templates (title, category, description) VALUES (
'BST中序遍历', '二叉搜索树',
$D$BST 的中序遍历可得到有序序列，逐步展示遍历过程。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'python', $C$import ds4viz as dv

dv.config(title="BST 中序遍历 (有序输出)")

with dv.binary_search_tree(label="inorder_bst") as bst:
    with bst.phase("构建"):
        n8  = bst.insert(8)
        bst.alias(n8, "root")
        n3  = bst.insert(3)
        n10 = bst.insert(10)
        n1  = bst.insert(1)
        n6  = bst.insert(6)
        n14 = bst.insert(14)

    # 中序: 1, 3, 6, 8, 10, 14
    visited = []

    def visit(nid, val):
        bst.step(note=f"访问 {val}", highlights=[
            *[dv.node(v, "visited") for v in visited],
            dv.node(nid, "focus", level=3)
        ])
        visited.append(nid)

    with bst.phase("中序遍历"):
        visit(n1, 1)
        visit(n3, 3)
        visit(n6, 6)
        visit(n8, 8)
        visit(n10, 10)
        visit(n14, 14)
$C$, $E$### Python 实现

中序遍历 BST 输出有序序列：1→3→6→8→10→14。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'c', $C$#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"
#include <stdio.h>

#define BST_INORDER_VISIT_LINE 8

#define VISIT(obj, val, ...)                                                  \
    do {                                                                      \
        char _dv_note[64];                                                    \
        snprintf(_dv_note, sizeof _dv_note, "访问 %d", (val));                \
        dvStepAt((obj), BST_INORDER_VISIT_LINE, _dv_note, __VA_ARGS__);      \
    } while (0)

int main(void) {
    dvConfig((dvConfigOptions){ .title = "BST 中序遍历 (有序输出)" });

    dvBinarySearchTree(bst, "inorder_bst") {
        int n8, n3, n10, n1, n6, n14;
        dvPhase(bst, "构建") {
            n8  = dvBstInsert(bst, 8);
            dvAlias(bst, n8, "root");
            n3  = dvBstInsert(bst, 3);
            n10 = dvBstInsert(bst, 10);
            n1  = dvBstInsert(bst, 1);
            n6  = dvBstInsert(bst, 6);
            n14 = dvBstInsert(bst, 14);
        }

        dvPhase(bst, "中序遍历") {
            VISIT(bst, 1,
                dvNode(n1, DV_FOCUS, 3));
            VISIT(bst, 3,
                dvNode(n1, DV_VISITED),
                dvNode(n3, DV_FOCUS, 3));
            VISIT(bst, 6,
                dvNode(n1, DV_VISITED), dvNode(n3, DV_VISITED),
                dvNode(n6, DV_FOCUS, 3));
            VISIT(bst, 8,
                dvNode(n1, DV_VISITED), dvNode(n3, DV_VISITED), dvNode(n6, DV_VISITED),
                dvNode(n8, DV_FOCUS, 3));
            VISIT(bst, 10,
                dvNode(n1, DV_VISITED), dvNode(n3, DV_VISITED), dvNode(n6, DV_VISITED),
                dvNode(n8, DV_VISITED),
                dvNode(n10, DV_FOCUS, 3));
            VISIT(bst, 14,
                dvNode(n1, DV_VISITED), dvNode(n3, DV_VISITED), dvNode(n6, DV_VISITED),
                dvNode(n8, DV_VISITED), dvNode(n10, DV_VISITED),
                dvNode(n14, DV_FOCUS, 3));
        }
    }
    return 0;
}

$C$, $E$### C 实现

BST 中序遍历天然输出升序，验证 BST 的有序性质。$E$);

-- =====================================================
-- 分类: 无向图 (5)
-- =====================================================

-- [33] 无向图的构建
INSERT INTO templates (title, category, description) VALUES (
'无向图的构建', '无向图',
$D$添加节点与无向边，构建连通图。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'python', $C$import ds4viz as dv

with dv.graph_undirected(label="simple_graph") as g:
    g.add_node(0, "A")
    g.add_node(1, "B")
    g.add_node(2, "C")
    g.add_node(3, "D")
    g.add_node(4, "E")
    g.add_edge(0, 1)
    g.add_edge(0, 2)
    g.add_edge(1, 3)
    g.add_edge(2, 3)
    g.add_edge(3, 4)
$C$, $E$### Python 实现

标准无向图构建，`add_edge` 内部自动规范化为 from < to。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'c', $C$#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"

int main(void) {
    dvGraphUndirected(g, "simple_graph") {
        dvGuAddNode(g, 0, "A");
        dvGuAddNode(g, 1, "B");
        dvGuAddNode(g, 2, "C");
        dvGuAddNode(g, 3, "D");
        dvGuAddNode(g, 4, "E");
        dvGuAddEdge(g, 0, 1);
        dvGuAddEdge(g, 0, 2);
        dvGuAddEdge(g, 1, 3);
        dvGuAddEdge(g, 2, 3);
        dvGuAddEdge(g, 3, 4);
    }
    return 0;
}
$C$, $E$### C 实现

无向图 API：`dvGuAddNode` 添加节点、`dvGuAddEdge` 添加边。$E$);

-- [34] 无向图的动态修改
INSERT INTO templates (title, category, description) VALUES (
'无向图的动态修改', '无向图',
$D$演示更新标签、删边、删点与增边等图的动态维护操作。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'python', $C$import ds4viz as dv

dv.config(title="无向图动态修改")

with dv.graph_undirected(label="dynamic_graph") as g:
    with g.phase("构建"):
        g.add_node(0, "A")
        g.add_node(1, "B")
        g.add_node(2, "C")
        g.add_node(3, "D")
        g.add_edge(0, 1)
        g.add_edge(1, 2)
        g.add_edge(2, 3)
        g.add_edge(0, 3)

    with g.phase("修改"):
        g.update_node_label(0, "Alpha")
        g.amend(note="节点 A 更名为 Alpha")
        g.remove_edge(0, 3)
        g.amend(note="删除 Alpha-D 的边")
        g.add_node(4, "E")
        g.add_edge(3, 4)
        g.amend(note="新增节点 E 并连接 D")
        g.remove_node(1)
        g.amend(note="删除节点 B 及其所有关联边")
$C$, $E$### Python 实现

分阶段展示图结构的增删改操作。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'c', $C$#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"

int main(void) {
    dvConfig((dvConfigOptions){ .title = "无向图动态修改" });

    dvGraphUndirected(g, "dynamic_graph") {
        dvPhase(g, "构建") {
            dvGuAddNode(g, 0, "A"); dvGuAddNode(g, 1, "B");
            dvGuAddNode(g, 2, "C"); dvGuAddNode(g, 3, "D");
            dvGuAddEdge(g, 0, 1); dvGuAddEdge(g, 1, 2);
            dvGuAddEdge(g, 2, 3); dvGuAddEdge(g, 0, 3);
        }

        dvPhase(g, "修改") {
            dvGuUpdateNodeLabel(g, 0, "Alpha");
            dvAmend(g, "节点 A 更名为 Alpha");
            dvGuRemoveEdge(g, 0, 3);
            dvAmend(g, "删除 Alpha-D 的边");
            dvGuAddNode(g, 4, "E");
            dvGuAddEdge(g, 3, 4);
            dvAmend(g, "新增节点 E 并连接 D");
            dvGuRemoveNode(g, 1);
            dvAmend(g, "删除节点 B 及其所有关联边");
        }
    }
    return 0;
}
$C$, $E$### C 实现

图的动态变更：标签更新、边删除、节点级联删除。$E$);

-- [35] 无向图 BFS 遍历
INSERT INTO templates (title, category, description) VALUES (
'无向图BFS遍历', '无向图',
$D$从节点 A 出发广度优先遍历无向图，逐步展示队列出入和邻居发现过程。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'python', $C$import ds4viz as dv

dv.config(title="无向图 BFS")

with dv.graph_undirected(label="bfs_demo") as g:
    g.add_node(0, "A")
    g.add_node(1, "B")
    g.add_node(2, "C")
    g.add_node(3, "D")
    g.add_node(4, "E")
    g.add_edge(0, 1)
    g.add_edge(0, 2)
    g.add_edge(1, 3)
    g.add_edge(2, 3)
    g.add_edge(3, 4)

    with g.phase("BFS"):
        g.step(note="起点 A 入队", highlights=[
            dv.node(0, "focus", level=3)
        ])
        g.step(note="出队 A, 发现邻居 B, C", highlights=[
            dv.node(0, "visited"),
            dv.node(1, "focus", level=2),
            dv.node(2, "focus", level=2),
            dv.edge(0, 1, "active"),
            dv.edge(0, 2, "active")
        ])
        g.step(note="出队 B, 发现邻居 D", highlights=[
            dv.node(0, "visited"), dv.node(1, "visited"),
            dv.node(2, "active"),
            dv.node(3, "focus", level=2),
            dv.edge(1, 3, "active")
        ])
        g.step(note="出队 C, 邻居 D 已发现", highlights=[
            dv.node(0, "visited"), dv.node(1, "visited"), dv.node(2, "visited"),
            dv.node(3, "active")
        ])
        g.step(note="出队 D, 发现邻居 E", highlights=[
            dv.node(0, "visited"), dv.node(1, "visited"),
            dv.node(2, "visited"), dv.node(3, "visited"),
            dv.node(4, "focus", level=2),
            dv.edge(3, 4, "active")
        ])
        g.step(note="出队 E, BFS 完成", highlights=[
            dv.node(0, "visited"), dv.node(1, "visited"),
            dv.node(2, "visited"), dv.node(3, "visited"), dv.node(4, "visited")
        ])
$C$, $E$### Python 实现

BFS 逐步展示：节点出队、边探索、邻居入队，使用 `edge()` 高亮当前探索的边。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'c', $C$#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"

int main(void) {
    dvConfig((dvConfigOptions){ .title = "无向图 BFS" });

    dvGraphUndirected(g, "bfs_demo") {
        dvGuAddNode(g, 0, "A"); dvGuAddNode(g, 1, "B"); dvGuAddNode(g, 2, "C");
        dvGuAddNode(g, 3, "D"); dvGuAddNode(g, 4, "E");
        dvGuAddEdge(g, 0, 1); dvGuAddEdge(g, 0, 2);
        dvGuAddEdge(g, 1, 3); dvGuAddEdge(g, 2, 3); dvGuAddEdge(g, 3, 4);

        dvPhase(g, "BFS") {
            dvStep(g, "起点 A 入队",
                dvNode(0, DV_FOCUS, 3));

            dvStep(g, "出队 A, 发现邻居 B, C",
                dvNode(0, DV_VISITED),
                dvNode(1, DV_FOCUS, 2), dvNode(2, DV_FOCUS, 2),
                dvEdge(0, 1, DV_ACTIVE), dvEdge(0, 2, DV_ACTIVE));

            dvStep(g, "出队 B, 发现邻居 D",
                dvNode(0, DV_VISITED), dvNode(1, DV_VISITED),
                dvNode(2, DV_ACTIVE), dvNode(3, DV_FOCUS, 2),
                dvEdge(1, 3, DV_ACTIVE));

            dvStep(g, "出队 C, 邻居 D 已发现",
                dvNode(0, DV_VISITED), dvNode(1, DV_VISITED), dvNode(2, DV_VISITED),
                dvNode(3, DV_ACTIVE));

            dvStep(g, "出队 D, 发现邻居 E",
                dvNode(0, DV_VISITED), dvNode(1, DV_VISITED),
                dvNode(2, DV_VISITED), dvNode(3, DV_VISITED),
                dvNode(4, DV_FOCUS, 2), dvEdge(3, 4, DV_ACTIVE));

            dvStep(g, "出队 E, BFS 完成",
                dvNode(0, DV_VISITED), dvNode(1, DV_VISITED), dvNode(2, DV_VISITED),
                dvNode(3, DV_VISITED), dvNode(4, DV_VISITED));
        }
    }
    return 0;
}
$C$, $E$### C 实现

`dvEdge` 高亮正在探索的边，完整展示 BFS 的广度优先搜索过程。$E$);

-- [36] 无向图 DFS 遍历
INSERT INTO templates (title, category, description) VALUES (
'无向图DFS遍历', '无向图',
$D$从节点 A 出发深度优先遍历无向图，展示递归深入和回溯过程。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'python', $C$import ds4viz as dv

dv.config(title="无向图 DFS")

with dv.graph_undirected(label="dfs_demo") as g:
    g.add_node(0, "A")
    g.add_node(1, "B")
    g.add_node(2, "C")
    g.add_node(3, "D")
    g.add_node(4, "E")
    g.add_edge(0, 1)
    g.add_edge(0, 2)
    g.add_edge(1, 3)
    g.add_edge(2, 4)
    g.add_edge(3, 4)

    # DFS: A -> B -> D -> E -> (回溯) -> C (E已访问)
    visited = []
    def dfs_step(nid, label):
        g.step(note=f"进入 {label}", highlights=[
            *[dv.node(v, "visited") for v in visited],
            dv.node(nid, "focus", level=3)
        ], line_offset=1)
        visited.append(nid)

    with g.phase("DFS"):
        dfs_step(0, "A")
        dfs_step(1, "B")
        dfs_step(3, "D")
        dfs_step(4, "E")
        g.step(note="E 的邻居都已访问, 回溯", highlights=[
            *[dv.node(v, "visited") for v in visited],
            dv.node(0, "active", level=2)
        ])
        dfs_step(2, "C")
        g.step(note="DFS 完成", highlights=[
            *[dv.node(v, "visited") for v in visited]
        ])
$C$, $E$### Python 实现

DFS 逐步深入，遇到死胡同回溯，使用 `active` 标记回溯点。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'c', $C$#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"

int main(void) {
    dvConfig((dvConfigOptions){ .title = "无向图 DFS" });

    dvGraphUndirected(g, "dfs_demo") {
        dvGuAddNode(g, 0, "A"); dvGuAddNode(g, 1, "B"); dvGuAddNode(g, 2, "C");
        dvGuAddNode(g, 3, "D"); dvGuAddNode(g, 4, "E");
        dvGuAddEdge(g, 0, 1); dvGuAddEdge(g, 0, 2);
        dvGuAddEdge(g, 1, 3); dvGuAddEdge(g, 2, 4); dvGuAddEdge(g, 3, 4);

        dvPhase(g, "DFS") {
            dvStep(g, "进入 A", dvNode(0, DV_FOCUS, 3));

            dvStep(g, "进入 B",
                dvNode(0, DV_VISITED), dvNode(1, DV_FOCUS, 3));

            dvStep(g, "进入 D",
                dvNode(0, DV_VISITED), dvNode(1, DV_VISITED),
                dvNode(3, DV_FOCUS, 3));

            dvStep(g, "进入 E",
                dvNode(0, DV_VISITED), dvNode(1, DV_VISITED), dvNode(3, DV_VISITED),
                dvNode(4, DV_FOCUS, 3));

            dvStep(g, "回溯到 A",
                dvNode(0, DV_ACTIVE, 2), dvNode(1, DV_VISITED),
                dvNode(3, DV_VISITED), dvNode(4, DV_VISITED));

            dvStep(g, "进入 C",
                dvNode(0, DV_VISITED), dvNode(1, DV_VISITED),
                dvNode(3, DV_VISITED), dvNode(4, DV_VISITED),
                dvNode(2, DV_FOCUS, 3));

            dvStep(g, "DFS 完成",
                dvNode(0, DV_VISITED), dvNode(1, DV_VISITED), dvNode(2, DV_VISITED),
                dvNode(3, DV_VISITED), dvNode(4, DV_VISITED));
        }
    }
    return 0;
}
$C$, $E$### C 实现

DFS 深度优先搜索：先深入再回溯，`DV_ACTIVE` 标记回溯节点。$E$);

-- [37] 彼得森图
INSERT INTO templates (title, category, description) VALUES (
'彼得森图', '无向图',
$D$构建经典彼得森图（10 个节点、15 条边），常用于图论教学。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'python', $C$import ds4viz as dv

with dv.graph_undirected(label="petersen") as g:
    for i in range(5):
        g.add_node(i, f"o{i}")
    for i in range(5):
        g.add_node(i + 5, f"i{i}")

    # 外圈
    for i in range(5):
        g.add_edge(i, (i + 1) % 5)
    # 辐条
    for i in range(5):
        g.add_edge(i, i + 5)
    # 内星
    for i in range(5):
        g.add_edge(i + 5, (i + 2) % 5 + 5)
$C$, $E$### Python 实现

彼得森图：外圈五边形 + 内圈五角星 + 辐条连接。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'c', $C$#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"

int main(void) {
    dvGraphUndirected(g, "petersen") {
        dvGuAddNode(g,0,"o0"); dvGuAddNode(g,1,"o1"); dvGuAddNode(g,2,"o2");
        dvGuAddNode(g,3,"o3"); dvGuAddNode(g,4,"o4");
        dvGuAddNode(g,5,"i0"); dvGuAddNode(g,6,"i1"); dvGuAddNode(g,7,"i2");
        dvGuAddNode(g,8,"i3"); dvGuAddNode(g,9,"i4");

        dvGuAddEdge(g,0,1); dvGuAddEdge(g,1,2); dvGuAddEdge(g,2,3);
        dvGuAddEdge(g,3,4); dvGuAddEdge(g,0,4);
        dvGuAddEdge(g,0,5); dvGuAddEdge(g,1,6); dvGuAddEdge(g,2,7);
        dvGuAddEdge(g,3,8); dvGuAddEdge(g,4,9);
        dvGuAddEdge(g,5,7); dvGuAddEdge(g,7,9); dvGuAddEdge(g,6,9);
        dvGuAddEdge(g,6,8); dvGuAddEdge(g,5,8);
    }
    return 0;
}
$C$, $E$### C 实现

经典图论基准图，常用于三着色、哈密顿回路等问题的教学。$E$);

-- =====================================================
-- 分类: 有向图 (5)
-- =====================================================

-- [38] 有向图的构建
INSERT INTO templates (title, category, description) VALUES (
'有向图的构建', '有向图',
$D$构建带环的有向图，体现边的方向性。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'python', $C$import ds4viz as dv

with dv.graph_directed(label="simple_digraph") as g:
    g.add_node(0, "A")
    g.add_node(1, "B")
    g.add_node(2, "C")
    g.add_node(3, "D")
    g.add_edge(0, 1)
    g.add_edge(0, 2)
    g.add_edge(1, 3)
    g.add_edge(2, 3)
    g.add_edge(3, 0)
$C$, $E$### Python 实现

有向边构成环路 A→B→D→A 和 A→C→D→A。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'c', $C$#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"

int main(void) {
    dvGraphDirected(g, "simple_digraph") {
        dvGdAddNode(g, 0, "A"); dvGdAddNode(g, 1, "B");
        dvGdAddNode(g, 2, "C"); dvGdAddNode(g, 3, "D");
        dvGdAddEdge(g, 0, 1); dvGdAddEdge(g, 0, 2);
        dvGdAddEdge(g, 1, 3); dvGdAddEdge(g, 2, 3);
        dvGdAddEdge(g, 3, 0);
    }
    return 0;
}
$C$, $E$### C 实现

有向图：边具有方向性，`dvGdAddEdge(g, 3, 0)` 表示 D→A。$E$);

-- [39] 课程先修关系图
INSERT INTO templates (title, category, description) VALUES (
'课程先修关系图', '有向图',
$D$用 DAG 表示课程先修依赖关系，适合拓扑排序教学。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'python', $C$import ds4viz as dv

dv.config(title="课程先修关系")

with dv.graph_directed(label="course_prereqs") as g:
    g.add_node(0, "Math")
    g.alias(0, "math")
    g.add_node(1, "C-Lang")
    g.alias(1, "clang")
    g.add_node(2, "DataStruct")
    g.add_node(3, "Algorithm")
    g.add_node(4, "OS")
    g.add_node(5, "Compiler")

    g.add_edge(0, 2)  # Math -> DataStruct
    g.add_edge(1, 2)  # C-Lang -> DataStruct
    g.add_edge(2, 3)  # DataStruct -> Algorithm
    g.add_edge(2, 4)  # DataStruct -> OS
    g.add_edge(3, 5)  # Algorithm -> Compiler
    g.add_edge(4, 5)  # OS -> Compiler
$C$, $E$### Python 实现

DAG 表示先修关系，`alias()` 标注基础课程，可用于拓扑排序讲解。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'c', $C$#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"

int main(void) {
    dvConfig((dvConfigOptions){ .title = "课程先修关系" });

    dvGraphDirected(g, "course_prereqs") {
        dvGdAddNode(g, 0, "Math");    dvAlias(g, 0, "math");
        dvGdAddNode(g, 1, "C-Lang");  dvAlias(g, 1, "clang");
        dvGdAddNode(g, 2, "DataStruct");
        dvGdAddNode(g, 3, "Algorithm");
        dvGdAddNode(g, 4, "OS");
        dvGdAddNode(g, 5, "Compiler");

        dvGdAddEdge(g, 0, 2); dvGdAddEdge(g, 1, 2);
        dvGdAddEdge(g, 2, 3); dvGdAddEdge(g, 2, 4);
        dvGdAddEdge(g, 3, 5); dvGdAddEdge(g, 4, 5);
    }
    return 0;
}
$C$, $E$### C 实现

有向无环图 (DAG) 表示课程依赖关系。$E$);

-- [40] 有向图 DFS 遍历
INSERT INTO templates (title, category, description) VALUES (
'有向图DFS遍历', '有向图',
$D$从节点 A 出发深度优先遍历有向图，展示有向边约束下的搜索路径。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'python', $C$import ds4viz as dv

dv.config(title="有向图 DFS")

with dv.graph_directed(label="dfs_digraph") as g:
    g.add_node(0, "A")
    g.add_node(1, "B")
    g.add_node(2, "C")
    g.add_node(3, "D")
    g.add_edge(0, 1)
    g.add_edge(0, 2)
    g.add_edge(1, 3)
    g.add_edge(2, 3)

    visited = []
    def dfs_step(nid, label):
        g.step(note=f"进入 {label}", highlights=[
            *[dv.node(v, "visited") for v in visited],
            dv.node(nid, "focus", level=3)
        ], line_offset=1)
        visited.append(nid)

    with g.phase("DFS"):
        dfs_step(0, "A")
        dfs_step(1, "B")
        dfs_step(3, "D")
        g.step(note="D 无出边, 回溯", highlights=[
            *[dv.node(v, "visited") for v in visited],
            dv.node(0, "active", level=2)
        ])
        dfs_step(2, "C")
        g.step(note="C->D 已访问, DFS 完成", highlights=[
            *[dv.node(v, "visited") for v in visited]
        ])
$C$, $E$### Python 实现

有向图 DFS：只能沿出边方向前进，`active` 标记回溯点。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'c', $C$#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"

int main(void) {
    dvConfig((dvConfigOptions){ .title = "有向图 DFS" });

    dvGraphDirected(g, "dfs_digraph") {
        dvGdAddNode(g, 0, "A"); dvGdAddNode(g, 1, "B");
        dvGdAddNode(g, 2, "C"); dvGdAddNode(g, 3, "D");
        dvGdAddEdge(g, 0, 1); dvGdAddEdge(g, 0, 2);
        dvGdAddEdge(g, 1, 3); dvGdAddEdge(g, 2, 3);

        dvPhase(g, "DFS") {
            dvStep(g, "进入 A", dvNode(0, DV_FOCUS, 3));
            dvStep(g, "进入 B",
                dvNode(0, DV_VISITED), dvNode(1, DV_FOCUS, 3));
            dvStep(g, "进入 D",
                dvNode(0, DV_VISITED), dvNode(1, DV_VISITED),
                dvNode(3, DV_FOCUS, 3));
            dvStep(g, "D 无出边, 回溯",
                dvNode(0, DV_ACTIVE, 2), dvNode(1, DV_VISITED), dvNode(3, DV_VISITED));
            dvStep(g, "进入 C",
                dvNode(0, DV_VISITED), dvNode(1, DV_VISITED), dvNode(3, DV_VISITED),
                dvNode(2, DV_FOCUS, 3));
            dvStep(g, "C->D 已访问, DFS 完成",
                dvNode(0, DV_VISITED), dvNode(1, DV_VISITED),
                dvNode(2, DV_VISITED), dvNode(3, DV_VISITED));
        }
    }
    return 0;
}
$C$, $E$### C 实现

有向图约束搜索方向，只能沿出边前进。$E$);

-- [41] 状态转换图
INSERT INTO templates (title, category, description) VALUES (
'状态转换图', '有向图',
$D$用有向图表示订单状态机的合法迁移路径。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'python', $C$import ds4viz as dv

dv.config(title="订单状态机")

with dv.graph_directed(label="order_fsm") as g:
    g.add_node(0, "Created")
    g.alias(0, "start")
    g.add_node(1, "Paid")
    g.add_node(2, "Shipped")
    g.add_node(3, "Delivered")
    g.alias(3, "end")
    g.add_node(4, "Cancelled")

    g.add_edge(0, 1)  # Created -> Paid
    g.add_edge(1, 2)  # Paid -> Shipped
    g.add_edge(2, 3)  # Shipped -> Delivered
    g.add_edge(0, 4)  # Created -> Cancelled
    g.add_edge(1, 4)  # Paid -> Cancelled
$C$, $E$### Python 实现

有向图天然适合表示状态机，边表示合法的状态迁移。`alias` 标注起止状态。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'c', $C$#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"

int main(void) {
    dvConfig((dvConfigOptions){ .title = "订单状态机" });

    dvGraphDirected(g, "order_fsm") {
        dvGdAddNode(g, 0, "Created");   dvAlias(g, 0, "start");
        dvGdAddNode(g, 1, "Paid");
        dvGdAddNode(g, 2, "Shipped");
        dvGdAddNode(g, 3, "Delivered"); dvAlias(g, 3, "end");
        dvGdAddNode(g, 4, "Cancelled");

        dvGdAddEdge(g, 0, 1); dvGdAddEdge(g, 1, 2);
        dvGdAddEdge(g, 2, 3); dvGdAddEdge(g, 0, 4); dvGdAddEdge(g, 1, 4);
    }
    return 0;
}
$C$, $E$### C 实现

状态机有向图，`dvAlias` 标注起始与终止状态。$E$);

-- [42] 拓扑排序示意
INSERT INTO templates (title, category, description) VALUES (
'拓扑排序示意', '有向图',
$D$在 DAG 上逐步展示拓扑排序过程（Kahn 算法思路）。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'python', $C$import ds4viz as dv

dv.config(title="拓扑排序")

with dv.graph_directed(label="topo_sort") as g:
    g.add_node(0, "A")
    g.add_node(1, "B")
    g.add_node(2, "C")
    g.add_node(3, "D")
    g.add_node(4, "E")
    g.add_edge(0, 2)
    g.add_edge(1, 2)
    g.add_edge(2, 3)
    g.add_edge(2, 4)

    visited = []
    def topo_step(nid, label, note):
        g.step(note=note, highlights=[
            *[dv.node(v, "visited") for v in visited],
            dv.node(nid, "focus", level=3)
        ], line_offset=1)
        visited.append(nid)

    with g.phase("拓扑排序"):
        topo_step(0, "A", "入度为 0, 输出 A")
        topo_step(1, "B", "入度为 0, 输出 B")
        topo_step(2, "C", "A,B 已移除, C 入度变 0, 输出 C")
        topo_step(3, "D", "C 已移除, D 入度变 0, 输出 D")
        topo_step(4, "E", "C 已移除, E 入度变 0, 输出 E")

    g.step(note="拓扑序: A, B, C, D, E", highlights=[
        *[dv.node(v, "found") for v in visited]
    ])
$C$, $E$### Python 实现

Kahn 算法思路：每次选入度为 0 的节点输出，然后移除该节点的出边。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'c', $C$#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"

int main(void) {
    dvConfig((dvConfigOptions){ .title = "拓扑排序" });

    dvGraphDirected(g, "topo_sort") {
        dvGdAddNode(g, 0, "A"); dvGdAddNode(g, 1, "B"); dvGdAddNode(g, 2, "C");
        dvGdAddNode(g, 3, "D"); dvGdAddNode(g, 4, "E");
        dvGdAddEdge(g, 0, 2); dvGdAddEdge(g, 1, 2);
        dvGdAddEdge(g, 2, 3); dvGdAddEdge(g, 2, 4);

        dvPhase(g, "拓扑排序") {
            dvStep(g, "入度为 0, 输出 A",
                dvNode(0, DV_FOCUS, 3));
            dvStep(g, "入度为 0, 输出 B",
                dvNode(0, DV_VISITED), dvNode(1, DV_FOCUS, 3));
            dvStep(g, "C 入度变 0, 输出 C",
                dvNode(0, DV_VISITED), dvNode(1, DV_VISITED),
                dvNode(2, DV_FOCUS, 3));
            dvStep(g, "D 入度变 0, 输出 D",
                dvNode(0, DV_VISITED), dvNode(1, DV_VISITED), dvNode(2, DV_VISITED),
                dvNode(3, DV_FOCUS, 3));
            dvStep(g, "E 入度变 0, 输出 E",
                dvNode(0, DV_VISITED), dvNode(1, DV_VISITED), dvNode(2, DV_VISITED),
                dvNode(3, DV_VISITED), dvNode(4, DV_FOCUS, 3));
        }

        dvStep(g, "拓扑序: A, B, C, D, E",
            dvNode(0, DV_FOUND), dvNode(1, DV_FOUND), dvNode(2, DV_FOUND),
            dvNode(3, DV_FOUND), dvNode(4, DV_FOUND));
    }
    return 0;
}
$C$, $E$### C 实现

逐步移除入度为 0 的节点，`DV_FOUND` 标记最终拓扑序。$E$);

-- =====================================================
-- 分类: 带权图 (5)
-- =====================================================

-- [43] 带权图的构建
INSERT INTO templates (title, category, description) VALUES (
'带权图的构建', '带权图',
$D$构建带权有向图，节点为城市，边权为距离。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'python', $C$import ds4viz as dv

with dv.graph_weighted(label="city_routes") as g:
    g.add_node(0, "Beijing")
    g.add_node(1, "Shanghai")
    g.add_node(2, "Guangzhou")
    g.add_node(3, "Chengdu")
    g.add_edge(0, 1, 1213.0)
    g.add_edge(1, 2, 1462.0)
    g.add_edge(0, 3, 1816.0)
    g.add_edge(2, 3, 1947.0)
$C$, $E$### Python 实现

`add_edge` 的第三个参数为边的权重（距离），构建有向带权图。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'c', $C$#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"

int main(void) {
    dvGraphWeighted(g, "city_routes") {
        dvGwAddNode(g, 0, "Beijing");
        dvGwAddNode(g, 1, "Shanghai");
        dvGwAddNode(g, 2, "Guangzhou");
        dvGwAddNode(g, 3, "Chengdu");
        dvGwAddEdge(g, 0, 1, 1213.0);
        dvGwAddEdge(g, 1, 2, 1462.0);
        dvGwAddEdge(g, 0, 3, 1816.0);
        dvGwAddEdge(g, 2, 3, 1947.0);
    }
    return 0;
}
$C$, $E$### C 实现

`dvGwAddEdge` 的第四个参数为 `double` 类型的权重。$E$);

-- [44] 网络拓扑图
INSERT INTO templates (title, category, description) VALUES (
'网络拓扑图', '带权图',
$D$有向带权图：节点为网络设备，边权为延迟（ms），使用别名标注关键节点。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'python', $C$import ds4viz as dv

dv.config(title="网络拓扑")

with dv.graph_weighted(label="network_topo") as g:
    g.add_node(0, "Client")
    g.alias(0, "source")
    g.add_node(1, "CDN")
    g.add_node(2, "LB")
    g.add_node(3, "Server-A")
    g.add_node(4, "Server-B")
    g.add_node(5, "Database")
    g.alias(5, "target")

    g.add_edge(0, 1, 5.0)
    g.add_edge(1, 2, 2.0)
    g.add_edge(2, 3, 1.0)
    g.add_edge(2, 4, 1.5)
    g.add_edge(3, 5, 3.0)
    g.add_edge(4, 5, 3.2)
$C$, $E$### Python 实现

网络延迟模型，`alias()` 标注请求源和目标。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'c', $C$#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"

int main(void) {
    dvConfig((dvConfigOptions){ .title = "网络拓扑" });

    dvGraphWeighted(g, "network_topo") {
        dvGwAddNode(g, 0, "Client");   dvAlias(g, 0, "source");
        dvGwAddNode(g, 1, "CDN");
        dvGwAddNode(g, 2, "LB");
        dvGwAddNode(g, 3, "Server-A");
        dvGwAddNode(g, 4, "Server-B");
        dvGwAddNode(g, 5, "Database");  dvAlias(g, 5, "target");

        dvGwAddEdge(g, 0, 1, 5.0);
        dvGwAddEdge(g, 1, 2, 2.0);
        dvGwAddEdge(g, 2, 3, 1.0);
        dvGwAddEdge(g, 2, 4, 1.5);
        dvGwAddEdge(g, 3, 5, 3.0);
        dvGwAddEdge(g, 4, 5, 3.2);
    }
    return 0;
}
$C$, $E$### C 实现

网络拓扑有向带权图，`dvAlias` 标注关键设备。$E$);

-- [45] Dijkstra 最短路径演示
INSERT INTO templates (title, category, description) VALUES (
'Dijkstra最短路径演示', '带权图',
$D$完整演示 Dijkstra 算法的松弛过程，逐步更新最短距离。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'python', $C$import ds4viz as dv

dv.config(title="Dijkstra 最短路径")

with dv.graph_weighted(label="dijkstra") as g:
    with g.phase("构建"):
        g.add_node(0, "S")
        g.alias(0, "source")
        g.add_node(1, "A")
        g.add_node(2, "B")
        g.add_node(3, "T")
        g.add_edge(0, 1, 2.0)
        g.add_edge(0, 2, 5.0)
        g.add_edge(1, 2, 1.0)
        g.add_edge(1, 3, 4.0)
        g.add_edge(2, 3, 1.0)

    with g.phase("松弛"):
        g.step(note="从 S 松弛: dist[A]=2, dist[B]=5", highlights=[
            dv.node(0, "visited"),
            dv.node(1, "focus", level=2),
            dv.node(2, "focus", level=2),
            dv.edge(0, 1, "active", level=3),
            dv.edge(0, 2, "active")
        ])

        g.step(note="从 A 松弛: dist[B]=5->3, dist[T]=6", highlights=[
            dv.node(0, "visited"), dv.node(1, "visited"),
            dv.node(2, "focus", level=2),
            dv.node(3, "focus", level=2),
            dv.edge(1, 2, "active", level=3),
            dv.edge(1, 3, "active")
        ])

        g.step(note="从 B 松弛: dist[T]=6->4", highlights=[
            dv.node(0, "visited"), dv.node(1, "visited"), dv.node(2, "visited"),
            dv.node(3, "focus", level=3),
            dv.edge(2, 3, "active", level=3)
        ])

        g.step(note="完成! 最短路径 S->A->B->T, 距离=4", highlights=[
            dv.node(0, "found"), dv.node(1, "found"),
            dv.node(2, "found"), dv.node(3, "found"),
            dv.edge(0, 1, "found", level=3),
            dv.edge(1, 2, "found", level=3),
            dv.edge(2, 3, "found", level=3)
        ])
$C$, $E$### Python 实现

完整 Dijkstra 松弛过程：每步选最小距离节点，更新邻居。`found` 标记最终最短路径。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'c', $C$#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"

int main(void) {
    dvConfig((dvConfigOptions){ .title = "Dijkstra 最短路径" });

    dvGraphWeighted(g, "dijkstra") {
        dvPhase(g, "构建") {
            dvGwAddNode(g, 0, "S");  dvAlias(g, 0, "source");
            dvGwAddNode(g, 1, "A");
            dvGwAddNode(g, 2, "B");
            dvGwAddNode(g, 3, "T");
            dvGwAddEdge(g, 0, 1, 2.0);
            dvGwAddEdge(g, 0, 2, 5.0);
            dvGwAddEdge(g, 1, 2, 1.0);
            dvGwAddEdge(g, 1, 3, 4.0);
            dvGwAddEdge(g, 2, 3, 1.0);
        }

        dvPhase(g, "松弛") {
            dvStep(g, "从 S 松弛: dist[A]=2, dist[B]=5",
                dvNode(0, DV_VISITED),
                dvNode(1, DV_FOCUS, 2), dvNode(2, DV_FOCUS, 2),
                dvEdge(0, 1, DV_ACTIVE, 3), dvEdge(0, 2, DV_ACTIVE));

            dvStep(g, "从 A 松弛: dist[B]=5->3, dist[T]=6",
                dvNode(0, DV_VISITED), dvNode(1, DV_VISITED),
                dvNode(2, DV_FOCUS, 2), dvNode(3, DV_FOCUS, 2),
                dvEdge(1, 2, DV_ACTIVE, 3), dvEdge(1, 3, DV_ACTIVE));

            dvStep(g, "从 B 松弛: dist[T]=6->4",
                dvNode(0, DV_VISITED), dvNode(1, DV_VISITED), dvNode(2, DV_VISITED),
                dvNode(3, DV_FOCUS, 3), dvEdge(2, 3, DV_ACTIVE, 3));

            dvStep(g, "完成! 最短路径 S->A->B->T, 距离=4",
                dvNode(0, DV_FOUND), dvNode(1, DV_FOUND),
                dvNode(2, DV_FOUND), dvNode(3, DV_FOUND),
                dvEdge(0, 1, DV_FOUND, 3), dvEdge(1, 2, DV_FOUND, 3),
                dvEdge(2, 3, DV_FOUND, 3));
        }
    }
    return 0;
}
$C$, $E$### C 实现

Dijkstra 完整演示：`DV_ACTIVE` 标记松弛中的边，`DV_FOUND` 标记最终路径。$E$);

-- [46] 带权图边权更新
INSERT INTO templates (title, category, description) VALUES (
'带权图边权更新', '带权图',
$D$演示 update_weight 更新边权重，配合高亮展示变更前后的对比。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'python', $C$import ds4viz as dv

dv.config(title="边权更新")

with dv.graph_weighted(label="update_weight") as g:
    g.add_node(0, "A")
    g.add_node(1, "B")
    g.add_node(2, "C")
    g.add_edge(0, 1, 10.0)
    g.add_edge(0, 2, 3.0)
    g.add_edge(1, 2, 5.0)

    g.update_weight(0, 1, 4.0)
    g.amend(note="A->B 权重 10.0 -> 4.0", highlights=[
        dv.edge(0, 1, "focus", level=3)
    ])

    g.update_weight(1, 2, 2.0)
    g.amend(note="B->C 权重 5.0 -> 2.0", highlights=[
        dv.edge(0, 1, "visited"),
        dv.edge(1, 2, "focus", level=3)
    ])
$C$, $E$### Python 实现

`update_weight()` 更新边权，`dv.edge()` 高亮变更的边。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'c', $C$#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"

int main(void) {
    dvConfig((dvConfigOptions){ .title = "边权更新" });

    dvGraphWeighted(g, "update_weight") {
        dvGwAddNode(g, 0, "A"); dvGwAddNode(g, 1, "B"); dvGwAddNode(g, 2, "C");
        dvGwAddEdge(g, 0, 1, 10.0);
        dvGwAddEdge(g, 0, 2, 3.0);
        dvGwAddEdge(g, 1, 2, 5.0);

        dvGwUpdateWeight(g, 0, 1, 4.0);
        dvAmend(g, "A->B 权重 10.0 -> 4.0",
            dvEdge(0, 1, DV_FOCUS, 3));

        dvGwUpdateWeight(g, 1, 2, 2.0);
        dvAmend(g, "B->C 权重 5.0 -> 2.0",
            dvEdge(0, 1, DV_VISITED),
            dvEdge(1, 2, DV_FOCUS, 3));
    }
    return 0;
}
$C$, $E$### C 实现

`dvGwUpdateWeight` 更新权重，`dvEdge` 高亮变更的边。$E$);

-- [47] 最短路径完整示例
INSERT INTO templates (title, category, description) VALUES (
'最短路径完整示例', '带权图',
$D$更大规模的 Dijkstra 示例，含 6 个节点的完整松弛过程。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'python', $C$import ds4viz as dv

dv.config(title="Dijkstra 松弛过程")

with dv.graph_weighted(label="shortest_path") as g:
    with g.phase("构建"):
        g.add_node(0, "起点")
        g.alias(0, "source")
        g.add_node(1, "甲")
        g.add_node(2, "乙")
        g.add_node(3, "终点")
        g.add_edge(0, 1, 1.0)
        g.add_edge(0, 2, 4.0)
        g.add_edge(1, 2, 2.0)
        g.add_edge(1, 3, 6.0)
        g.add_edge(2, 3, 3.0)

    with g.phase("松弛"):
        g.step(note="从 起点 松弛: dist[甲]=1, dist[乙]=4", highlights=[
            dv.node(0, "visited"),
            dv.node(1, "focus", level=2), dv.node(2, "focus", level=2),
            dv.edge(0, 1, "active", level=3), dv.edge(0, 2, "active")
        ])

        g.step(note="从 甲 松弛: dist[乙]=4->3, dist[终点]=7", highlights=[
            dv.node(0, "visited"), dv.node(1, "visited"),
            dv.node(2, "focus", level=2), dv.node(3, "focus", level=2),
            dv.edge(1, 2, "active", level=3), dv.edge(1, 3, "active")
        ])

        g.step(note="从 乙 松弛: dist[终点]=7->6", highlights=[
            dv.node(0, "visited"), dv.node(1, "visited"), dv.node(2, "visited"),
            dv.node(3, "focus", level=3),
            dv.edge(2, 3, "active", level=3)
        ])

        g.step(note="完成: 起点->甲->乙->终点, 距离=6", highlights=[
            dv.node(0, "found"), dv.node(1, "found"),
            dv.node(2, "found"), dv.node(3, "found"),
            dv.edge(0, 1, "found", level=3),
            dv.edge(1, 2, "found", level=3),
            dv.edge(2, 3, "found", level=3)
        ])
$C$, $E$### Python 实现

松弛过程详解：每步贪心选取最近未确定节点，更新其所有邻居的距离。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'c', $C$#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"

int main(void) {
    dvConfig((dvConfigOptions){ .title = "Dijkstra 松弛过程" });

    dvGraphWeighted(g, "shortest_path") {
        dvPhase(g, "构建") {
            dvGwAddNode(g, 0, "起点"); dvAlias(g, 0, "source");
            dvGwAddNode(g, 1, "甲");
            dvGwAddNode(g, 2, "乙");
            dvGwAddNode(g, 3, "终点");
            dvGwAddEdge(g, 0, 1, 1.0);
            dvGwAddEdge(g, 0, 2, 4.0);
            dvGwAddEdge(g, 1, 2, 2.0);
            dvGwAddEdge(g, 1, 3, 6.0);
            dvGwAddEdge(g, 2, 3, 3.0);
        }

        dvPhase(g, "松弛") {
            dvStep(g, "从 起点 松弛: dist[甲]=1, dist[乙]=4",
                dvNode(0, DV_VISITED),
                dvNode(1, DV_FOCUS, 2), dvNode(2, DV_FOCUS, 2),
                dvEdge(0, 1, DV_ACTIVE, 3), dvEdge(0, 2, DV_ACTIVE));

            dvStep(g, "从 甲 松弛: dist[乙]=4->3, dist[终点]=7",
                dvNode(0, DV_VISITED), dvNode(1, DV_VISITED),
                dvNode(2, DV_FOCUS, 2), dvNode(3, DV_FOCUS, 2),
                dvEdge(1, 2, DV_ACTIVE, 3), dvEdge(1, 3, DV_ACTIVE));

            dvStep(g, "从 乙 松弛: dist[终点]=7->6",
                dvNode(0, DV_VISITED), dvNode(1, DV_VISITED), dvNode(2, DV_VISITED),
                dvNode(3, DV_FOCUS, 3), dvEdge(2, 3, DV_ACTIVE, 3));

            dvStep(g, "完成: 起点->甲->乙->终点, 距离=6",
                dvNode(0, DV_FOUND), dvNode(1, DV_FOUND),
                dvNode(2, DV_FOUND), dvNode(3, DV_FOUND),
                dvEdge(0, 1, DV_FOUND, 3), dvEdge(1, 2, DV_FOUND, 3),
                dvEdge(2, 3, DV_FOUND, 3));
        }
    }
    return 0;
}
$C$, $E$### C 实现

`DV_ACTIVE` 标记当前松弛的边，`DV_FOUND` 标记最终最短路径树上的边。$E$);

-- =====================================================
-- 分类: 概念与综合 (3)
-- =====================================================

-- [48] 哈希表拉链法示意
INSERT INTO templates (title, category, description) VALUES (
'哈希表拉链法示意', '概念示意',
$D$使用有向图表示哈希表桶到冲突链节点的指向关系。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'python', $C$import ds4viz as dv

dv.config(title="哈希表拉链法")

with dv.graph_directed(label="hash_chaining") as g:
    g.add_node(0, "bucket[0]")
    g.add_node(1, "bucket[1]")
    g.add_node(2, "bucket[2]")

    g.add_node(10, "k=10")
    g.add_node(13, "k=13")
    g.add_node(16, "k=16")
    g.add_node(21, "k=21")

    g.add_edge(1, 10)
    g.add_edge(10, 13)
    g.add_edge(13, 16)
    g.add_edge(0, 21)

    g.step(note="bucket[1] 发生三次冲突, 形成链表", highlights=[
        dv.node(1, "active"),
        dv.node(10, "focus"), dv.node(13, "focus"), dv.node(16, "focus")
    ])
$C$, $E$### Python 实现

用有向图模拟哈希表结构：桶指向冲突链头节点，链节点串联。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'c', $C$#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"

int main(void) {
    dvConfig((dvConfigOptions){ .title = "哈希表拉链法" });

    dvGraphDirected(g, "hash_chaining") {
        dvGdAddNode(g, 0, "bucket[0]");
        dvGdAddNode(g, 1, "bucket[1]");
        dvGdAddNode(g, 2, "bucket[2]");
        dvGdAddNode(g, 10, "k=10");
        dvGdAddNode(g, 13, "k=13");
        dvGdAddNode(g, 16, "k=16");
        dvGdAddNode(g, 21, "k=21");

        dvGdAddEdge(g, 1, 10);
        dvGdAddEdge(g, 10, 13);
        dvGdAddEdge(g, 13, 16);
        dvGdAddEdge(g, 0, 21);

        dvStep(g, "bucket[1] 发生三次冲突, 形成链表",
            dvNode(1, DV_ACTIVE),
            dvNode(10, DV_FOCUS), dvNode(13, DV_FOCUS), dvNode(16, DV_FOCUS));
    }
    return 0;
}
$C$, $E$### C 实现

利用有向图的灵活性表达哈希表的内部结构。$E$);

-- [49] 概念示意：AVL 旋转结果
INSERT INTO templates (title, category, description) VALUES (
'AVL旋转结果示意', '概念示意',
$D$用二叉树展示 AVL 树经 LL 旋转后的平衡形态，辅助理解旋转操作效果。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'python', $C$import ds4viz as dv

dv.config(title="AVL 树 LL 旋转后形态")

with dv.binary_tree(label="avl_after_rotate") as t:
    with t.phase("旋转前（不平衡）"):
        r = t.insert_root(30)
        a = t.insert_left(r, 20)
        t.insert_left(a, 10)
        t.step(note="左左失衡: 30 的左子树过高", highlights=[
            dv.node(r, "error", level=3)
        ])

    with t.phase("旋转后（平衡）"):
        t.delete(r)
        r2 = t.insert_root(20)
        t.alias(r2, "new-root")
        t.insert_left(r2, 10)
        t.insert_right(r2, 30)
        t.step(note="LL 旋转: 20 成为新根, 树高平衡", highlights=[
            dv.node(r2, "found", level=3)
        ])
$C$, $E$### Python 实现

分两阶段展示旋转前后的树形态对比，`error` 标记不平衡节点，`found` 标记新根。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'c', $C$#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"

int main(void) {
    dvConfig((dvConfigOptions){ .title = "AVL 树 LL 旋转后形态" });

    dvBinaryTree(t, "avl_after_rotate") {
        int r, a, r2;

        dvPhase(t, "旋转前") {
            r = dvBtInsertRoot(t, 30);
            a = dvBtInsertLeft(t, r, 20);
            dvBtInsertLeft(t, a, 10);
            dvStep(t, "左左失衡: 30 的左子树过高",
                dvNode(r, DV_ERROR_STY, 3));
        }

        dvPhase(t, "旋转后") {
            dvBtDelete(t, r);
            r2 = dvBtInsertRoot(t, 20);
            dvAlias(t, r2, "new-root");
            dvBtInsertLeft(t, r2, 10);
            dvBtInsertRight(t, r2, 30);
            dvStep(t, "LL 旋转: 20 成为新根, 树高平衡",
                dvNode(r2, DV_FOUND, 3));
        }
    }
    return 0;
}
$C$, $E$### C 实现

`DV_ERROR_STY` 标记不平衡节点，通过删除重建模拟旋转结果。$E$);

-- [50] 错误处理演示
INSERT INTO templates (title, category, description) VALUES (
'错误处理演示', '概念示意',
$D$演示对空栈执行 pop 时的错误处理机制：异常后自动生成包含 [error] 的 .toml 文件。$D$
);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'python', $C$import ds4viz as dv

dv.config(title="错误处理演示")

try:
    with dv.stack(label="error_demo") as s:
        s.push(10)
        s.push(20)
        s.pop()
        s.pop()
        s.pop()  # 栈已空, 抛出 RuntimeError
except RuntimeError as e:
    print(f"捕获错误: {e}")

# trace.toml 已生成, 包含前 4 步正常记录 + [error] 信息
$C$, $E$### Python 实现

`with` 上下文管理器自动捕获异常并生成包含 `[error]` 的 `.toml`。即使代码出错，可视化文件仍然有效。$E$);

INSERT INTO template_codes (template_id, language, code, explanation) VALUES
(currval('templates_id_seq'), 'c', $C$#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"

int main(void) {
    dvConfig((dvConfigOptions){ .title = "错误处理演示" });

    dvStack(s, "error_demo") {
        dvStackPush(s, 10);
        dvStackPush(s, 20);
        dvStackPop(s);
        dvStackPop(s);
        dvStackPop(s);     /* 栈已空, 记录错误 */
        dvStackPush(s, 30); /* 不执行 (静默跳过) */
    }
    /* trace.toml 已生成, 包含前 4 步 + [error] */
    return 0;
}
$C$, $E$### C 实现

C 版本错误后静默跳过后续操作，退出作用域时输出 `[error]`。与 Python 的异常机制不同，但生成的 `.toml` 结构一致。$E$);

-- =====================================================
-- 更新序列值
-- =====================================================
SELECT setval('templates_id_seq', (SELECT COALESCE(MAX(id), 0) FROM templates));
SELECT setval('template_codes_id_seq', (SELECT COALESCE(MAX(id), 0) FROM template_codes));

COMMIT;
