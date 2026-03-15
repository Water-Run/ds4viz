<script setup lang="ts">
/**
 * 文档页面
 *
 * @component Docs
 * @date 2026-03-15
 */

import { computed, onMounted, ref } from 'vue'
import { useRouter } from 'vue-router'

import { LANGUAGES, LANGUAGE_LABELS } from '@/types/api'
import MaterialIcon from '@/components/common/MaterialIcon.vue'
import CodeEditor from '@/components/editor/CodeEditor.vue'
import type { Language } from '@/types/api'

/**
 * 文档节点
 *
 * @interface
 */
interface DocNode {
  /** 节点 id */
  id: string
  /** 标题 */
  title: string
  /** 内容 */
  content?: string
  /** 代码块 */
  codeBlocks?: Partial<Record<Language, string>>
  /** 子节点 */
  children?: DocNode[]
}

const router = useRouter()

/**
 * 选中语言（全局共享，切换一处则处处切换）
 */
const selectedLanguage = ref<Language>('python')

/**
 * 展开状态
 */
const expanded = ref<Record<string, boolean>>({
  intro: true,
  online: true,
  syntax: true,
  linear: true,
  tree: true,
  graph: true,
  deploy: true,
})

/**
 * 内容区容器引用
 */
const contentRef = ref<HTMLElement | null>(null)

/**
 * 已复制的代码块 ID
 */
const copiedId = ref<string>('')

/**
 * 切换图标名称
 */
const getToggleIcon = (open: boolean): string => (open ? 'expand_more' : 'chevron_right')

/**
 * 文档树数据
 */
const docTree = computed<DocNode[]>(() => [
  {
    id: 'intro',
    title: '概览',
    content:
      'ds4viz（data-structure-for-visualization）是一个面向教学场景的基础数据结构可视化框架。核心架构通过提供对应语言库，将代码中的结构操作过程记录为可回放、可追踪、可复现的状态序列，并存储在通用的 TOML IR 中。由于各层解耦的设计，ds4viz 支持多种语言以及渲染器。用户可以在单个页面完成从编写代码到观察结构变化的全过程。ds4viz 遵循 GPL v3 协议开放源代码。',
    children: [
      {
        id: 'intro-principle',
        title: '基础原理',
        content:
          'ds4viz 采用 代码 → 中间语言 → 渲染 三层解耦架构。第一层是语言库：用户在 Python、Lua、Rust、C 等语言中以指定语法调用 ds4viz API，每次操作都会记录状态变化。第二层是中间语言 IR：所有语言统一产出 .toml 文件，该文件描述一次完整 trace，渲染器不需要读取源码，只需读取 TOML 即可复原整个过程。第三层是渲染器：按 object.kind 解析结构类型，按 states 和 steps 进行展示，同一份 TOML 可在不同渲染端复用。一类特定的、不设计用于交互的渲染器在 ds4viz 中被称为编译器，例如输出到 PowerPoint 演示文稿等。',
      },
    ],
  },
  {
    id: 'online',
    title: '在线服务',
    content:
      'ds4viz 提供在线服务，这是使用 ds4viz 最方便的方式。你所在的页面即 ds4viz 提供的网页端，使用 Vue SPA 加 FastAPI 后端以及 PostgreSQL 作为技术栈，将代码编辑、远程执行、TOML IR 解析、可视化渲染整合到同一工作流中。除网页端外，在线服务还可用于 ds4viz TUI（Rust 编写）和 ds4viz Windows 桌面端（WinUI3 编写）。在线服务需要登录才可进行访问，你需要注册一个帐号，除此之外没有任何限制。',
    children: [
      {
        id: 'online-pages',
        title: '页面架构',
        content:
          '进入系统后，ds4viz 左侧为导航栏，进行页面跳转，导航栏可折叠。页面包括：关于页（ds4viz 的相关信息，通过点击左上角图标进入）、可视化和编辑器页（最主要的页面，编写、执行代码和可视化结果）、模板库页（展示当前在线服务提供的模板）、文档页（此页面）、个人主页（用户的有关信息）。在个人主页，你可以设置头像、修改密码、查阅统计数据、收藏的模板和执行记录。',
      },
      {
        id: 'online-editor',
        title: '编辑器',
        content:
          '编辑器页采用左右分栏的典型设计。左侧为可视化渲染器，整个系统的核心，支持前进、后退、首尾跳转、自动播放等功能，你还可以拖动和缩放画布，并双击回中。ds4viz 对于不同类别的数据结构有特别的美术设计。右侧为代码编辑区域，嵌入 Monaco，你可以在上方下拉框中进行语言切换，支持基本的语法高亮与补全。需要注意的是，没有 LSP 进行词法分析，你需要自行检查语义的正确性。运行后后端返回 TOML IR，前端立即解析并刷新可视化，失败时将出现对应提示。执行完成代码后，你可以点击下载将 TOML IR 保存到本地；反向的，你也可以上传本地的 TOML，但此时代码行高亮无法对应。',
      },
      {
        id: 'online-renderer',
        title: '可视化渲染器',
        content:
          '渲染器按 object.kind 识别结构类型并绘制，渲染过程基于 states 完整快照，steps 用于展示操作名、参数、返回值和源码行号。你可以自动播放，或逐帧前后步进。可在可视化面板调整运行时配置，包括：显示元数据（是否显示如标签之类的元数据）、平滑动画衔接（两帧之间衔接的动画效果）、联动代码高亮（是否联动高亮执行的对应代码行）、自适应画布（帧切换时是否自适应合适的画布大小）、变更强化（新增/删除的暗示效果）、控制台调试（在控制台输出调试信息）、跳过初始页（跳过生成后的初始引导页面）、生成后自动播放（生成后直接进行自动播放）、播放间隔（播放帧之间的间隔时间）。这些配置仅影响展示行为，不影响 IR 内容。',
      },
      {
        id: 'online-templates',
        title: '模板系统',
        content:
          'ds4viz 在线服务提供模板系统，涵盖各个分类。模板按分类组织，支持搜索和收藏。同一模板可包含、并通常包含多语言版本。在模板详情页可切换语言查看代码，并一键导入编辑器继续运行。',
      },
    ],
  },
  {
    id: 'syntax',
    title: '语法',
    content:
      'ds4viz 支持 Python、Lua、Rust、C 四种语言，遵循大体相同的设计：创建结构对象，在受控上下文中执行操作，自动生成 trace.toml，成功写入 [result]，失败写入 [error]。不同语言保持各自命名习惯，但语义一致。因需要确保 TOML IR 始终写入，需要使用对应语言提供的上下文闭包能力：Python 使用 with，Lua 使用 withContext，Rust 使用闭包 + Result，C 使用结构作用域宏。值类型仅支持 integer、float、string、boolean。',
    children: [
      {
        id: 'syntax-config',
        title: '全局配置',
        content: '可选的全局配置，设定输出路径、标题、作者等元信息。',
        codeBlocks: {
          python: `import ds4viz as dv

dv.config(
    output_path="trace.toml",
    title="Demo",
    author="WaterRun",
    comment="示例"
)`,
          lua: `local ds4viz = require("ds4viz")

ds4viz.config({
    output_path = "trace.toml",
    title = "Demo",
    author = "WaterRun",
    comment = "示例"
})`,
          rust: `use ds4viz::Config;

ds4viz::config(
    Config::builder()
        .output_path("trace.toml")
        .title("Demo")
        .author("WaterRun")
        .comment("示例")
        .build()
);`,
          c: `#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"

int main(void) {
  dvConfig((dvConfigOptions){
    .output_path = "trace.toml",
    .title = "Demo",
    .author = "WaterRun",
    .comment = "示例"
  });

  return 0;
}`,
        },
      },
      {
        id: 'syntax-context',
        title: '上下文管理',
        content:
          '使用上下文管理器确保无论成功或失败，都会生成 trace.toml。成功时写入 [result]，异常时写入 [error]。Python 使用 with 语句，Lua 使用 withContext 函数，Rust 使用闭包 + Result，C 使用结构作用域宏。注意不要在 C 的结构作用域中使用 return 或 goto 跳出。',
        codeBlocks: {
          python: `import ds4viz as dv

with dv.graph_undirected() as graph:
    graph.add_node(0, "A")
    graph.add_node(1, "B")
    graph.add_edge(0, 1)`,
          lua: `local ds4viz = require("ds4viz")

local ok, err = ds4viz.withContext(ds4viz.graphUndirected(), function(graph)
    graph:addNode(0, "A")
    graph:addNode(1, "B")
    graph:addEdge(0, 1)
end)`,
          rust: `use ds4viz::prelude::*;

fn main() -> ds4viz::Result<()> {
    ds4viz::graph_undirected("demo_graph", |graph| {
        graph.add_node(0, "A")?;
        graph.add_node(1, "B")?;
        graph.add_edge(0, 1)?;
        Ok(())
    })
}`,
          c: `#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"

int main(void) {
  dvConfig((dvConfigOptions){ .output_path = "trace.toml" });

  dvGraphUndirected(graph, "demo_graph") {
    dvGuAddNode(graph, 0, "A");
    dvGuAddNode(graph, 1, "B");
    dvGuAddEdge(graph, 0, 1);
  }

  return 0;
}`,
        },
      },
    ],
  },
  {
    id: 'linear',
    title: '线性结构',
    children: [
      {
        id: 'api-stack',
        title: 'Stack',
        content:
          '栈，后进先出线性结构。支持 push（压栈）、pop（弹栈）、clear（清空）操作。pop 在栈为空时抛出异常。',
        codeBlocks: {
          python: `import ds4viz as dv

with dv.stack(label="demo_stack") as s:
    s.push(10)
    s.push(20)
    s.push(30)
    s.pop()
    s.push(40)
    s.pop()
    s.pop()`,
          lua: `local ds4viz = require("ds4viz")

ds4viz.withContext(ds4viz.stack("demo_stack"), function(s)
    s:push(10)
    s:push(20)
    s:push(30)
    s:pop()
    s:push(40)
    s:pop()
    s:pop()
end)`,
          rust: `use ds4viz::prelude::*;

fn main() -> ds4viz::Result<()> {
    ds4viz::stack("demo_stack", |s| {
        s.push(10)?;
        s.push(20)?;
        s.push(30)?;
        s.pop()?;
        s.push(40)?;
        s.pop()?;
        s.pop()?;
        Ok(())
    })
}`,
          c: `#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"

int main(void) {
  dvConfig((dvConfigOptions){ .output_path = "trace.toml" });

  dvStack(s, "demo_stack") {
    dvStackPush(s, 10);
    dvStackPush(s, 20);
    dvStackPush(s, 30);
    dvStackPop(s);
    dvStackPush(s, 40);
    dvStackPop(s);
    dvStackPop(s);
  }

  return 0;
}`,
        },
      },
      {
        id: 'api-queue',
        title: 'Queue',
        content:
          '队列，先进先出线性结构。支持 enqueue（入队）、dequeue（出队）、clear（清空）操作。dequeue 在队列为空时抛出异常。',
        codeBlocks: {
          python: `import ds4viz as dv

with dv.queue(label="demo_queue") as q:
    q.enqueue("A")
    q.enqueue("B")
    q.enqueue("C")
    q.dequeue()
    q.enqueue("D")
    q.dequeue()`,
          lua: `local ds4viz = require("ds4viz")

ds4viz.withContext(ds4viz.queue("demo_queue"), function(q)
    q:enqueue("A")
    q:enqueue("B")
    q:enqueue("C")
    q:dequeue()
    q:enqueue("D")
    q:dequeue()
end)`,
          rust: `use ds4viz::prelude::*;

fn main() -> ds4viz::Result<()> {
    ds4viz::queue("demo_queue", |q| {
        q.enqueue("A")?;
        q.enqueue("B")?;
        q.enqueue("C")?;
        q.dequeue()?;
        q.enqueue("D")?;
        q.dequeue()?;
        Ok(())
    })
}`,
          c: `#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"

int main(void) {
  dvConfig((dvConfigOptions){ .output_path = "trace.toml" });

  dvQueue(q, "demo_queue") {
    dvQueueEnqueue(q, "A");
    dvQueueEnqueue(q, "B");
    dvQueueEnqueue(q, "C");
    dvQueueDequeue(q);
    dvQueueEnqueue(q, "D");
    dvQueueDequeue(q);
  }

  return 0;
}`,
        },
      },
      {
        id: 'api-slist',
        title: 'SingleLinkedList',
        content:
          '单链表。支持 insertHead（头插）、insertTail（尾插）、insertAfter（指定节点后插入）、delete（删除节点）、deleteHead（删除头节点）、reverse（反转）操作。插入操作返回新节点 ID。',
        codeBlocks: {
          python: `import ds4viz as dv

with dv.single_linked_list(label="demo_slist") as slist:
    node_a = slist.insert_head(10)
    slist.insert_tail(20)
    node_c = slist.insert_tail(30)
    slist.insert_after(node_a, 15)
    slist.delete(node_c)
    slist.reverse()`,
          lua: `local ds4viz = require("ds4viz")

ds4viz.withContext(ds4viz.singleLinkedList("demo_slist"), function(slist)
    local node_a = slist:insertHead(10)
    slist:insertTail(20)
    local node_c = slist:insertTail(30)
    slist:insertAfter(node_a, 15)
    slist:delete(node_c)
    slist:reverse()
end)`,
          rust: `use ds4viz::prelude::*;

fn main() -> ds4viz::Result<()> {
    ds4viz::single_linked_list("demo_slist", |slist| {
        let node_a = slist.insert_head(10)?;
        slist.insert_tail(20)?;
        let node_c = slist.insert_tail(30)?;
        slist.insert_after(node_a, 15)?;
        slist.delete(node_c)?;
        slist.reverse()?;
        Ok(())
    })
}`,
          c: `#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"

int main(void) {
  dvConfig((dvConfigOptions){ .output_path = "trace.toml" });

  dvSingleLinkedList(slist, "demo_slist") {
    int node_a = dvSlInsertHead(slist, 10);
    dvSlInsertTail(slist, 20);
    int node_c = dvSlInsertTail(slist, 30);

    dvSlInsertAfter(slist, node_a, 15);

    dvSlDelete(slist, node_c);
    dvSlReverse(slist);
  }

  return 0;
}`,
        },
      },
      {
        id: 'api-dlist',
        title: 'DoubleLinkedList',
        content:
          '双向链表。在单链表基础上增加 insertBefore（指定节点前插入）、deleteTail（删除尾节点）操作，并支持双向遍历。',
        codeBlocks: {
          python: `import ds4viz as dv

with dv.double_linked_list(label="demo_dlist") as dlist:
    node_a = dlist.insert_head(10)
    node_b = dlist.insert_tail(30)
    dlist.insert_after(node_a, 20)
    dlist.insert_before(node_b, 25)
    dlist.delete_tail()
    dlist.reverse()`,
          lua: `local ds4viz = require("ds4viz")

ds4viz.withContext(ds4viz.doubleLinkedList("demo_dlist"), function(dlist)
    local node_a = dlist:insertHead(10)
    local node_b = dlist:insertTail(30)
    dlist:insertAfter(node_a, 20)
    dlist:insertBefore(node_b, 25)
    dlist:deleteTail()
    dlist:reverse()
end)`,
          rust: `use ds4viz::prelude::*;

fn main() -> ds4viz::Result<()> {
    ds4viz::double_linked_list("demo_dlist", |dlist| {
        let node_a = dlist.insert_head(10)?;
        let node_b = dlist.insert_tail(30)?;
        dlist.insert_after(node_a, 20)?;
        dlist.insert_before(node_b, 25)?;
        dlist.delete_tail()?;
        dlist.reverse()?;
        Ok(())
    })
}`,
          c: `#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"

int main(void) {
  dvConfig((dvConfigOptions){ .output_path = "trace.toml" });

  dvDoubleLinkedList(dlist, "demo_dlist") {
    int node_a = dvDlInsertHead(dlist, 10);
    int node_b = dvDlInsertTail(dlist, 30);

    dvDlInsertAfter(dlist, node_a, 20);
    dvDlInsertBefore(dlist, node_b, 25);

    dvDlDeleteTail(dlist);
    dvDlReverse(dlist);
  }

  return 0;
}`,
        },
      },
    ],
  },
  {
    id: 'tree',
    title: '树结构',
    children: [
      {
        id: 'api-binary-tree',
        title: 'BinaryTree',
        content:
          '二叉树。支持 insertRoot（插入根节点）、insertLeft / insertRight（插入左/右子节点）、delete（删除节点及其子树）、updateValue（更新节点值）操作。插入操作返回节点 ID。',
        codeBlocks: {
          python: `import ds4viz as dv

with dv.binary_tree(label="demo_tree") as tree:
    root = tree.insert_root(1)
    left = tree.insert_left(root, 2)
    right = tree.insert_right(root, 3)
    tree.insert_left(left, 4)
    tree.insert_right(left, 5)
    tree.insert_left(right, 6)
    tree.update_value(right, 30)
    tree.delete(left)`,
          lua: `local ds4viz = require("ds4viz")

ds4viz.withContext(ds4viz.binaryTree("demo_tree"), function(tree)
    local root = tree:insertRoot(1)
    local left = tree:insertLeft(root, 2)
    local right = tree:insertRight(root, 3)
    tree:insertLeft(left, 4)
    tree:insertRight(left, 5)
    tree:insertLeft(right, 6)
    tree:updateValue(right, 30)
    tree:delete(left)
end)`,
          rust: `use ds4viz::prelude::*;

fn main() -> ds4viz::Result<()> {
    ds4viz::binary_tree("demo_tree", |tree| {
        let root = tree.insert_root(1)?;
        let left = tree.insert_left(root, 2)?;
        let right = tree.insert_right(root, 3)?;
        tree.insert_left(left, 4)?;
        tree.insert_right(left, 5)?;
        tree.insert_left(right, 6)?;
        tree.update_value(right, 30)?;
        tree.delete(left)?;
        Ok(())
    })
}`,
          c: `#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"

int main(void) {
  dvConfig((dvConfigOptions){ .output_path = "trace.toml" });

  dvBinaryTree(tree, "demo_tree") {
    int root = dvBtInsertRoot(tree, 1);
    int left = dvBtInsertLeft(tree, root, 2);
    int right = dvBtInsertRight(tree, root, 3);

    dvBtInsertLeft(tree, left, 4);
    dvBtInsertRight(tree, left, 5);
    dvBtInsertLeft(tree, right, 6);

    dvBtUpdateValue(tree, right, 30);
    dvBtDelete(tree, left);
  }

  return 0;
}`,
        },
      },
      {
        id: 'api-bst',
        title: 'BinarySearchTree',
        content:
          '二叉搜索树。支持 insert（插入，自动维护 BST 性质）和 delete（删除，自动处理叶子、单子、双子三种情况）操作。值类型限制为数值。',
        codeBlocks: {
          python: `import ds4viz as dv

with dv.binary_search_tree(label="demo_bst") as bst:
    bst.insert(50)
    bst.insert(30)
    bst.insert(70)
    bst.insert(20)
    bst.insert(40)
    bst.insert(60)
    bst.insert(80)
    bst.delete(30)
    bst.insert(35)`,
          lua: `local ds4viz = require("ds4viz")

ds4viz.withContext(ds4viz.binarySearchTree("demo_bst"), function(bst)
    bst:insert(50)
    bst:insert(30)
    bst:insert(70)
    bst:insert(20)
    bst:insert(40)
    bst:insert(60)
    bst:insert(80)
    bst:delete(30)
    bst:insert(35)
end)`,
          rust: `use ds4viz::prelude::*;

fn main() -> ds4viz::Result<()> {
    ds4viz::binary_search_tree("demo_bst", |bst| {
        bst.insert(50)?;
        bst.insert(30)?;
        bst.insert(70)?;
        bst.insert(20)?;
        bst.insert(40)?;
        bst.insert(60)?;
        bst.insert(80)?;
        bst.delete(30)?;
        bst.insert(35)?;
        Ok(())
    })
}`,
          c: `#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"

int main(void) {
  dvConfig((dvConfigOptions){ .output_path = "trace.toml" });

  dvBinarySearchTree(bst, "demo_bst") {
    dvBstInsert(bst, 50);
    dvBstInsert(bst, 30);
    dvBstInsert(bst, 70);
    dvBstInsert(bst, 20);
    dvBstInsert(bst, 40);
    dvBstInsert(bst, 60);
    dvBstInsert(bst, 80);

    dvBstDelete(bst, 30);
    dvBstInsert(bst, 35);
  }

  return 0;
}`,
        },
      },
      {
        id: 'api-heap',
        title: 'Heap',
        content:
          '堆（优先队列）。支持 min（最小堆）和 max（最大堆）两种模式。insert 插入元素并自动上浮，extract 提取堆顶并自动下沉。内部以二叉树形式渲染。',
        codeBlocks: {
          python: `import ds4viz as dv

# 最小堆
with dv.heap(label="demo_min_heap", heap_type="min") as h:
    h.insert(15)
    h.insert(10)
    h.insert(20)
    h.insert(5)
    h.insert(30)
    h.extract()
    h.insert(2)
    h.extract()`,
          lua: `local ds4viz = require("ds4viz")

-- 最小堆
ds4viz.withContext(ds4viz.heap("demo_min_heap", "min"), function(h)
    h:insert(15)
    h:insert(10)
    h:insert(20)
    h:insert(5)
    h:insert(30)
    h:extract()
    h:insert(2)
    h:extract()
end)`,
          rust: `use ds4viz::prelude::*;

fn main() -> ds4viz::Result<()> {
    // 最小堆
    ds4viz::heap("demo_min_heap", HeapType::Min, |h| {
        h.insert(15)?;
        h.insert(10)?;
        h.insert(20)?;
        h.insert(5)?;
        h.insert(30)?;
        h.extract()?;
        h.insert(2)?;
        h.extract()?;
        Ok(())
    })
}`,
          c: `#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"

int main(void) {
  dvConfig((dvConfigOptions){ .output_path = "trace.toml" });

  dvHeap(min_heap, "demo_min_heap", dvHeapOrderMin) {
    dvHeapInsert(min_heap, 15);
    dvHeapInsert(min_heap, 10);
    dvHeapInsert(min_heap, 20);
    dvHeapInsert(min_heap, 5);
    dvHeapInsert(min_heap, 30);

    dvHeapExtract(min_heap);
    dvHeapInsert(min_heap, 2);
    dvHeapExtract(min_heap);
  }

  return 0;
}`,
        },
      },
    ],
  },
  {
    id: 'graph',
    title: '图结构',
    children: [
      {
        id: 'api-graph-undirected',
        title: 'GraphUndirected',
        content:
          '无向图。支持 addNode（添加节点）、addEdge（添加无向边，内部自动规范化方向）、removeNode（删除节点及关联边）、removeEdge（删除边）、updateNodeLabel（更新节点标签）操作。不允许自环。',
        codeBlocks: {
          python: `import ds4viz as dv

with dv.graph_undirected(label="demo_graph") as g:
    g.add_node(0, "A")
    g.add_node(1, "B")
    g.add_node(2, "C")
    g.add_node(3, "D")
    g.add_edge(0, 1)
    g.add_edge(0, 2)
    g.add_edge(1, 2)
    g.add_edge(2, 3)
    g.remove_edge(0, 2)
    g.remove_node(3)`,
          lua: `local ds4viz = require("ds4viz")

ds4viz.withContext(ds4viz.graphUndirected("demo_graph"), function(g)
    g:addNode(0, "A")
    g:addNode(1, "B")
    g:addNode(2, "C")
    g:addNode(3, "D")
    g:addEdge(0, 1)
    g:addEdge(0, 2)
    g:addEdge(1, 2)
    g:addEdge(2, 3)
    g:removeEdge(0, 2)
    g:removeNode(3)
end)`,
          rust: `use ds4viz::prelude::*;

fn main() -> ds4viz::Result<()> {
    ds4viz::graph_undirected("demo_graph", |g| {
        g.add_node(0, "A")?;
        g.add_node(1, "B")?;
        g.add_node(2, "C")?;
        g.add_node(3, "D")?;
        g.add_edge(0, 1)?;
        g.add_edge(0, 2)?;
        g.add_edge(1, 2)?;
        g.add_edge(2, 3)?;
        g.remove_edge(0, 2)?;
        g.remove_node(3)?;
        Ok(())
    })
}`,
          c: `#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"

int main(void) {
  dvConfig((dvConfigOptions){ .output_path = "trace.toml" });

  dvGraphUndirected(g, "demo_graph") {
    dvGuAddNode(g, 0, "A");
    dvGuAddNode(g, 1, "B");
    dvGuAddNode(g, 2, "C");
    dvGuAddNode(g, 3, "D");

    dvGuAddEdge(g, 0, 1);
    dvGuAddEdge(g, 0, 2);
    dvGuAddEdge(g, 1, 2);
    dvGuAddEdge(g, 2, 3);

    dvGuRemoveEdge(g, 0, 2);
    dvGuRemoveNode(g, 3);
  }

  return 0;
}`,
        },
      },
      {
        id: 'api-graph-directed',
        title: 'GraphDirected',
        content:
          '有向图。API 与无向图一致，区别在于 addEdge 保留方向信息（from → to）。可视化时以箭头标识方向。',
        codeBlocks: {
          python: `import ds4viz as dv

with dv.graph_directed(label="demo_digraph") as g:
    g.add_node(0, "A")
    g.add_node(1, "B")
    g.add_node(2, "C")
    g.add_node(3, "D")
    g.add_edge(0, 1)
    g.add_edge(1, 2)
    g.add_edge(2, 3)
    g.add_edge(3, 0)
    g.remove_edge(3, 0)
    g.add_edge(0, 2)`,
          lua: `local ds4viz = require("ds4viz")

ds4viz.withContext(ds4viz.graphDirected("demo_digraph"), function(g)
    g:addNode(0, "A")
    g:addNode(1, "B")
    g:addNode(2, "C")
    g:addNode(3, "D")
    g:addEdge(0, 1)
    g:addEdge(1, 2)
    g:addEdge(2, 3)
    g:addEdge(3, 0)
    g:removeEdge(3, 0)
    g:addEdge(0, 2)
end)`,
          rust: `use ds4viz::prelude::*;

fn main() -> ds4viz::Result<()> {
    ds4viz::graph_directed("demo_digraph", |g| {
        g.add_node(0, "A")?;
        g.add_node(1, "B")?;
        g.add_node(2, "C")?;
        g.add_node(3, "D")?;
        g.add_edge(0, 1)?;
        g.add_edge(1, 2)?;
        g.add_edge(2, 3)?;
        g.add_edge(3, 0)?;
        g.remove_edge(3, 0)?;
        g.add_edge(0, 2)?;
        Ok(())
    })
}`,
          c: `#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"

int main(void) {
  dvConfig((dvConfigOptions){ .output_path = "trace.toml" });

  dvGraphDirected(g, "demo_digraph") {
    dvGdAddNode(g, 0, "A");
    dvGdAddNode(g, 1, "B");
    dvGdAddNode(g, 2, "C");
    dvGdAddNode(g, 3, "D");

    dvGdAddEdge(g, 0, 1);
    dvGdAddEdge(g, 1, 2);
    dvGdAddEdge(g, 2, 3);
    dvGdAddEdge(g, 3, 0);

    dvGdRemoveEdge(g, 3, 0);
    dvGdAddEdge(g, 0, 2);
  }

  return 0;
}`,
        },
      },
      {
        id: 'api-graph-weighted',
        title: 'GraphWeighted',
        content:
          '带权图。addEdge 需要额外传入 weight 参数，支持 updateWeight 更新边权重。通过 directed 参数控制有向/无向。可视化时在边上标注权值。',
        codeBlocks: {
          python: `import ds4viz as dv

with dv.graph_weighted(label="demo_weighted") as g:
    g.add_node(0, "A")
    g.add_node(1, "B")
    g.add_node(2, "C")
    g.add_node(3, "D")
    g.add_edge(0, 1, 4.0)
    g.add_edge(0, 2, 2.0)
    g.add_edge(1, 3, 5.0)
    g.add_edge(2, 3, 1.0)
    g.update_weight(0, 1, 3.0)
    g.remove_edge(2, 3)`,
          lua: `local ds4viz = require("ds4viz")

ds4viz.withContext(ds4viz.graphWeighted("demo_weighted"), function(g)
    g:addNode(0, "A")
    g:addNode(1, "B")
    g:addNode(2, "C")
    g:addNode(3, "D")
    g:addEdge(0, 1, 4.0)
    g:addEdge(0, 2, 2.0)
    g:addEdge(1, 3, 5.0)
    g:addEdge(2, 3, 1.0)
    g:updateWeight(0, 1, 3.0)
    g:removeEdge(2, 3)
end)`,
          rust: `use ds4viz::prelude::*;

fn main() -> ds4viz::Result<()> {
    ds4viz::graph_weighted("demo_weighted", false, |g| {
        g.add_node(0, "A")?;
        g.add_node(1, "B")?;
        g.add_node(2, "C")?;
        g.add_node(3, "D")?;
        g.add_edge(0, 1, 4.0)?;
        g.add_edge(0, 2, 2.0)?;
        g.add_edge(1, 3, 5.0)?;
        g.add_edge(2, 3, 1.0)?;
        g.update_weight(0, 1, 3.0)?;
        g.remove_edge(2, 3)?;
        Ok(())
    })
}`,
          c: `#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"

int main(void) {
  dvConfig((dvConfigOptions){ .output_path = "trace.toml" });

  dvGraphWeighted(g, "demo_weighted", false) {
    dvGwAddNode(g, 0, "A");
    dvGwAddNode(g, 1, "B");
    dvGwAddNode(g, 2, "C");
    dvGwAddNode(g, 3, "D");

    dvGwAddEdge(g, 0, 1, 4.0);
    dvGwAddEdge(g, 0, 2, 2.0);
    dvGwAddEdge(g, 1, 3, 5.0);
    dvGwAddEdge(g, 2, 3, 1.0);

    dvGwUpdateWeight(g, 0, 1, 3.0);
    dvGwRemoveEdge(g, 2, 3);
  }

  return 0;
}`,
        },
      },
    ],
  },
  {
    id: 'deploy',
    title: '自部署服务',
    content:
      'ds4viz 是开源的，你可以自行部署在线服务。你需要配置环境，包括安装各你要支持的语言的 ds4viz 库。详情参阅 GitHub 上的在线服务部署文档。',
    children: [
      {
        id: 'deploy-install',
        title: '库安装',
        content: '选择你需要支持的语言，安装对应的 ds4viz 库。',
        codeBlocks: {
          python: 'pip install ds4viz',
          lua: 'luarocks install ds4viz',
          rust: 'cargo add ds4viz',
          c: '下载 ds4viz.h，并使用 gcc -std=c2x 编译',
        },
      },
    ],
  },
])

/**
 * 切换节点展开状态
 *
 * @param id - 节点 id
 */
const toggleNode = (id: string): void => {
  expanded.value[id] = !expanded.value[id]
}

/**
 * 滚动到指定文档节
 *
 * @param id - 目标节点 id
 */
const scrollToSection = (id: string): void => {
  const target = document.getElementById(id)
  if (!target || !contentRef.value) return
  const container = contentRef.value
  const containerRect = container.getBoundingClientRect()
  const targetRect = target.getBoundingClientRect()
  const topOffset = 20
  const scrollPos = targetRect.top - containerRect.top + container.scrollTop - topOffset
  container.scrollTo({ top: scrollPos, behavior: 'smooth' })
}

/**
 * 获取代码块的可用语言列表
 */
const getCodeBlockLanguages = (codeBlocks: Partial<Record<Language, string>>): Language[] => {
  return Object.keys(codeBlocks) as Language[]
}

/**
 * 获取代码块的有效显示语言
 */
const getEffectiveLanguage = (codeBlocks: Partial<Record<Language, string>>): Language => {
  if (codeBlocks[selectedLanguage.value] !== undefined) {
    return selectedLanguage.value
  }
  const available = Object.keys(codeBlocks) as Language[]
  return available.length > 0 ? available[0] : 'python'
}

/**
 * 获取语言对应的色点颜色
 */
const getLanguageDotColor = (language: Language): string => {
  const colors: Record<Language, string> = {
    python: 'var(--color-lang-python)',
    lua: 'var(--color-lang-lua)',
    rust: 'var(--color-lang-rust)',
    c: 'var(--color-lang-c)',
  }
  return colors[language] ?? 'var(--color-text-tertiary)'
}

/**
 * 处理代码块语言下拉切换
 */
const handleCodeBlockLangChange = (event: Event): void => {
  const target = event.target as HTMLSelectElement
  selectedLanguage.value = target.value as Language
}

/**
 * 计算编辑器高度
 */
const getEditorHeight = (codeBlocks: Partial<Record<Language, string>>): string => {
  const code = codeBlocks[getEffectiveLanguage(codeBlocks)] ?? ''
  const lines = code.split('\n').length
  return `${Math.min(360, Math.max(60, lines * 20 + 24))}px`
}

/**
 * 复制代码到剪贴板
 */
const handleCopyCode = async (code: string, sectionId: string): Promise<void> => {
  try {
    await navigator.clipboard.writeText(code)
    copiedId.value = sectionId
    window.setTimeout(() => {
      if (copiedId.value === sectionId) {
        copiedId.value = ''
      }
    }, 2000)
  } catch {
    /* clipboard 不可用 */
  }
}

/**
 * 在编辑器中打开
 */
const handleOpenInEditor = (code: string, language: Language): void => {
  try {
    sessionStorage.setItem('ds4viz_edit_code', code)
    sessionStorage.setItem('ds4viz_edit_language', language)
  } catch {
    /* sessionStorage 不可用 */
  }
  router.push({ name: 'playground' })
}

onMounted(() => {
  const stored = localStorage.getItem('ds4viz_language') as Language | null
  if (stored && LANGUAGES.includes(stored)) {
    selectedLanguage.value = stored
  }
})
</script>

<template>
  <div class="docs-page">
    <header class="docs-page__header">
      <div class="docs-page__title">
        <MaterialIcon name="menu_book" :size="18" />
        <span>文档</span>
      </div>
    </header>

    <div class="docs-page__body">
      <aside class="docs-page__toc">
        <div v-for="section in docTree" :key="section.id" class="toc-section">
          <button class="toc-section__title" @click="toggleNode(section.id); scrollToSection(section.id)">
            <MaterialIcon :name="getToggleIcon(expanded[section.id])" :size="16" />
            <span>{{ section.title }}</span>
          </button>
          <div v-if="expanded[section.id]" class="toc-section__children">
            <button v-for="child in section.children" :key="child.id" class="toc-section__item"
              @click="scrollToSection(child.id)">
              {{ child.title }}
            </button>
          </div>
        </div>
      </aside>

      <section ref="contentRef" class="docs-page__content">
        <div v-for="section in docTree" :key="section.id" :id="section.id" class="doc-section">
          <h2 class="doc-section__title">{{ section.title }}</h2>
          <p v-if="section.content" class="doc-section__desc">{{ section.content }}</p>
          <div v-for="child in section.children" :key="child.id" :id="child.id" class="doc-subsection">
            <h3 class="doc-subsection__title">{{ child.title }}</h3>
            <p v-if="child.content" class="doc-subsection__desc">{{ child.content }}</p>
            <div v-if="child.codeBlocks && Object.keys(child.codeBlocks).length > 0" class="doc-code-block">
              <div class="doc-code-block__toolbar">
                <div class="doc-code-block__actions">
                  <button class="code-action-btn" :class="{ 'code-action-btn--copied': copiedId === child.id }"
                    @click="handleCopyCode(child.codeBlocks![getEffectiveLanguage(child.codeBlocks!)] ?? '', child.id)">
                    <svg v-if="copiedId !== child.id" viewBox="0 0 24 24" fill="currentColor">
                      <path
                        d="M16 1H4c-1.1 0-2 .9-2 2v14h2V3h12V1zm3 4H8c-1.1 0-2 .9-2 2v14c0 1.1.9 2 2 2h11c1.1 0 2-.9 2-2V7c0-1.1-.9-2-2-2zm0 16H8V7h11v14z" />
                    </svg>
                    <svg v-else viewBox="0 0 24 24" fill="currentColor">
                      <path d="M9 16.17L4.83 12l-1.42 1.41L9 19 21 7l-1.41-1.41z" />
                    </svg>
                    <span>{{ copiedId === child.id ? '已复制' : '复制' }}</span>
                  </button>
                  <button class="code-action-btn code-action-btn--primary"
                    @click="handleOpenInEditor(child.codeBlocks![getEffectiveLanguage(child.codeBlocks!)] ?? '', getEffectiveLanguage(child.codeBlocks!))">
                    <svg viewBox="0 0 24 24" fill="currentColor">
                      <path
                        d="M9.4 16.6L4.8 12l4.6-4.6L8 6l-6 6 6 6 1.4-1.4zm5.2 0l4.6-4.6-4.6-4.6L16 6l6 6-6 6-1.4-1.4z" />
                    </svg>
                    <span>在编辑器中打开</span>
                  </button>
                </div>
                <div class="doc-code-block__lang-select">
                  <span class="doc-code-block__dot"
                    :style="{ backgroundColor: getLanguageDotColor(getEffectiveLanguage(child.codeBlocks)) }" />
                  <select class="doc-code-block__select" :value="getEffectiveLanguage(child.codeBlocks)"
                    @change="handleCodeBlockLangChange">
                    <option v-for="lang in getCodeBlockLanguages(child.codeBlocks)" :key="lang" :value="lang">
                      {{ LANGUAGE_LABELS[lang as Language] ?? lang }}
                    </option>
                  </select>
                </div>
              </div>
              <div class="doc-code-block__editor" :style="{ height: getEditorHeight(child.codeBlocks) }">
                <CodeEditor :model-value="child.codeBlocks[getEffectiveLanguage(child.codeBlocks)] ?? ''"
                  :language="getEffectiveLanguage(child.codeBlocks)" :readonly="true" />
              </div>
            </div>
          </div>
        </div>
      </section>
    </div>
  </div>
</template>

<style scoped>
.docs-page {
  display: flex;
  flex-direction: column;
  gap: var(--space-2);
  padding: var(--space-3);
  height: 100%;
  overflow: hidden;
}

.docs-page__header {
  display: flex;
  align-items: center;
  justify-content: space-between;
  flex-shrink: 0;
}

.docs-page__title {
  display: flex;
  align-items: center;
  gap: 6px;
  font-size: var(--text-base);
  font-weight: var(--weight-semibold);
  color: var(--color-text-primary);
}

.docs-page__title :deep(.material-icon) {
  width: 18px;
  height: 18px;
}

.docs-page__body {
  display: grid;
  grid-template-columns: 200px 1fr;
  gap: var(--space-3);
  flex: 1;
  min-height: 0;
  overflow: hidden;
}

.docs-page__toc {
  border: 1px solid var(--color-border);
  border-radius: var(--radius-lg);
  padding: var(--space-2);
  background-color: var(--color-bg-surface);
  overflow: auto;
}

.toc-section {
  display: flex;
  flex-direction: column;
  gap: 2px;
  margin-bottom: var(--space-2);
}

.toc-section__title {
  display: flex;
  align-items: center;
  gap: 4px;
  border: none;
  background: none;
  font-size: var(--text-sm);
  font-weight: var(--weight-semibold);
  color: var(--color-text-primary);
  cursor: pointer;
  padding: 4px 0;
  border-radius: var(--radius-sm);
  transition: color var(--duration-fast) var(--ease);
}

.toc-section__title:hover {
  color: var(--color-accent);
}

.toc-section__title :deep(.material-icon) {
  width: 16px;
  height: 16px;
}

.toc-section__children {
  display: flex;
  flex-direction: column;
  gap: 2px;
  padding-left: 20px;
}

.toc-section__item {
  border: none;
  background: none;
  text-align: left;
  font-size: var(--text-xs);
  color: var(--color-text-tertiary);
  cursor: pointer;
  padding: 3px 6px;
  border-radius: var(--radius-sm);
  transition: background-color var(--duration-fast) var(--ease), color var(--duration-fast) var(--ease);
}

.toc-section__item:hover {
  background-color: var(--color-bg-hover);
  color: var(--color-accent);
}

.docs-page__content {
  border: 1px solid var(--color-border);
  border-radius: var(--radius-lg);
  padding: var(--space-3);
  background-color: var(--color-bg-surface);
  overflow: auto;
}

.doc-section {
  margin-bottom: var(--space-4);
  scroll-margin-top: var(--space-3);
}

.doc-section__title {
  margin: 0 0 var(--space-1);
  font-size: 18px;
  font-weight: var(--weight-semibold);
}

.doc-section__desc {
  margin: 0 0 var(--space-2);
  font-size: var(--text-sm);
  color: var(--color-text-body);
  line-height: var(--leading-relaxed);
}

.doc-subsection {
  margin-bottom: var(--space-3);
  scroll-margin-top: var(--space-3);
}

.doc-subsection__title {
  margin: 0 0 var(--space-1);
  font-size: 15px;
  font-weight: var(--weight-semibold);
}

.doc-subsection__desc {
  margin: 0 0 var(--space-1);
  font-size: var(--text-sm);
  color: var(--color-text-body);
  line-height: var(--leading-relaxed);
}

.doc-code-block {
  position: relative;
  border: 1px solid var(--color-border);
  border-radius: var(--radius-md);
  background-color: var(--color-bg-surface-alt);
  overflow: hidden;
}

.doc-code-block__toolbar {
  display: flex;
  align-items: center;
  justify-content: space-between;
  gap: var(--space-1);
  padding: 6px 8px;
  border-bottom: 1px solid var(--color-border);
  background-color: var(--color-bg-surface);
  flex-wrap: wrap;
}

.doc-code-block__actions {
  display: flex;
  align-items: center;
  gap: var(--space-1);
}

.doc-code-block__lang-select {
  display: inline-flex;
  align-items: center;
  gap: 6px;
  padding: 0 10px;
  height: 28px;
  border: 1px solid var(--color-border-strong);
  border-radius: var(--radius-control);
  background-color: var(--color-bg-surface);
}

.doc-code-block__dot {
  width: 7px;
  height: 7px;
  border-radius: 999px;
  flex-shrink: 0;
}

.doc-code-block__select {
  font-size: var(--text-xs);
  color: var(--color-text-primary);
  background: none;
  border: none;
  outline: none;
  cursor: pointer;
  font-family: inherit;
}

.doc-code-block__editor {
  display: flex;
  border-radius: 0;
  overflow: hidden;
}

/* ---- 统一操作按钮 ---- */
.code-action-btn {
  display: inline-flex;
  align-items: center;
  gap: 4px;
  height: 28px;
  padding: 0 8px;
  border: 1px solid var(--color-border-strong);
  border-radius: var(--radius-control);
  background-color: var(--color-bg-surface);
  color: var(--color-text-body);
  font-size: var(--text-xs);
  font-family: inherit;
  cursor: pointer;
  transition: background-color var(--duration-fast) var(--ease), border-color var(--duration-fast) var(--ease), color var(--duration-fast) var(--ease);
}

.code-action-btn:hover {
  background-color: var(--color-bg-hover);
  color: var(--color-text-primary);
}

.code-action-btn svg {
  width: 14px;
  height: 14px;
  flex-shrink: 0;
}

.code-action-btn--copied {
  border-color: var(--color-success);
  color: var(--color-success);
}

.code-action-btn--primary {
  border-color: var(--color-accent);
  background-color: var(--color-accent);
  color: var(--color-accent-contrast);
}

.code-action-btn--primary:hover {
  background-color: var(--color-accent-hover);
  border-color: var(--color-accent-hover);
}

@media (max-width: 1100px) {
  .docs-page__body {
    grid-template-columns: 1fr;
  }

  .docs-page__toc {
    order: 2;
  }
}
</style>