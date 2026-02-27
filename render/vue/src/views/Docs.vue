<script setup lang="ts">
/**
 * 文档页面
 *
 * @component Docs
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
  guides: true,
  linear: true,
  tree: true,
  graph: true,
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
      'ds4viz 是面向数据结构教学的可视化框架。用户编写操作代码，框架自动记录每一步状态快照并生成 TOML IR，前端将其渲染为可交互的步骤化动画。支持 Python、Lua、Rust 三种语言。',
  },
  {
    id: 'guides',
    title: '快速开始',
    children: [
      {
        id: 'guide-install',
        title: '安装',
        content: '选择对应语言安装依赖库。',
        codeBlocks: {
          python: 'pip install ds4viz',
          lua: 'luarocks install ds4viz',
          rust: 'cargo add ds4viz',
        },
      },
      {
        id: 'guide-config',
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
        },
      },
      {
        id: 'guide-run',
        title: '运行',
        content:
          '在编辑器中编写代码并点击运行，后端执行后生成 TOML IR 并返回前端渲染可视化。也可在本地运行脚本直接生成 trace.toml 文件。',
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
        },
      },
      { id: 'api-slist', title: 'SingleLinkedList', content: '单链表。支持 insertHead（头插）、insertTail（尾插）、insertAfter（指定节点后插入）、delete（删除节点）、deleteHead（删除头节点）、reverse（反转）操作。插入操作返回新节点 ID。', codeBlocks: { python: `import ds4viz as dv\n\nwith dv.single_linked_list(label="demo_slist") as slist:\n    node_a = slist.insert_head(10)\n    slist.insert_tail(20)\n    node_c = slist.insert_tail(30)\n    slist.insert_after(node_a, 15)\n    slist.delete(node_c)\n    slist.reverse()`, lua: `local ds4viz = require("ds4viz")\n\nds4viz.withContext(ds4viz.singleLinkedList("demo_slist"), function(slist)\n    local node_a = slist:insertHead(10)\n    slist:insertTail(20)\n    local node_c = slist:insertTail(30)\n    slist:insertAfter(node_a, 15)\n    slist:delete(node_c)\n    slist:reverse()\nend)`, rust: `use ds4viz::prelude::*;\n\nfn main() -> ds4viz::Result<()> {\n    ds4viz::single_linked_list("demo_slist", |slist| {\n        let node_a = slist.insert_head(10)?;\n        slist.insert_tail(20)?;\n        let node_c = slist.insert_tail(30)?;\n        slist.insert_after(node_a, 15)?;\n        slist.delete(node_c)?;\n        slist.reverse()?;\n        Ok(())\n    })\n}` } },
      { id: 'api-dlist', title: 'DoubleLinkedList', content: '双向链表。在单链表基础上增加 insertBefore（指定节点前插入）、deleteTail（删除尾节点）操作，并支持双向遍历。', codeBlocks: { python: `import ds4viz as dv\n\nwith dv.double_linked_list(label="demo_dlist") as dlist:\n    node_a = dlist.insert_head(10)\n    node_b = dlist.insert_tail(30)\n    dlist.insert_after(node_a, 20)\n    dlist.insert_before(node_b, 25)\n    dlist.delete_tail()\n    dlist.reverse()`, lua: `local ds4viz = require("ds4viz")\n\nds4viz.withContext(ds4viz.doubleLinkedList("demo_dlist"), function(dlist)\n    local node_a = dlist:insertHead(10)\n    local node_b = dlist:insertTail(30)\n    dlist:insertAfter(node_a, 20)\n    dlist:insertBefore(node_b, 25)\n    dlist:deleteTail()\n    dlist:reverse()\nend)`, rust: `use ds4viz::prelude::*;\n\nfn main() -> ds4viz::Result<()> {\n    ds4viz::double_linked_list("demo_dlist", |dlist| {\n        let node_a = dlist.insert_head(10)?;\n        let node_b = dlist.insert_tail(30)?;\n        dlist.insert_after(node_a, 20)?;\n        dlist.insert_before(node_b, 25)?;\n        dlist.delete_tail()?;\n        dlist.reverse()?;\n        Ok(())\n    })\n}` } },
    ],
  },
  {
    id: 'tree',
    title: '树结构',
    children: [
      { id: 'api-binary-tree', title: 'BinaryTree', content: '二叉树。支持 insertRoot（插入根节点）、insertLeft / insertRight（插入左/右子节点）、delete（删除节点及其子树）、updateValue（更新节点值）操作。插入操作返回节点 ID。', codeBlocks: { python: `import ds4viz as dv\n\nwith dv.binary_tree(label="demo_tree") as tree:\n    root = tree.insert_root(1)\n    left = tree.insert_left(root, 2)\n    right = tree.insert_right(root, 3)\n    tree.insert_left(left, 4)\n    tree.insert_right(left, 5)\n    tree.insert_left(right, 6)\n    tree.update_value(right, 30)\n    tree.delete(left)`, lua: `local ds4viz = require("ds4viz")\n\nds4viz.withContext(ds4viz.binaryTree("demo_tree"), function(tree)\n    local root = tree:insertRoot(1)\n    local left = tree:insertLeft(root, 2)\n    local right = tree:insertRight(root, 3)\n    tree:insertLeft(left, 4)\n    tree:insertRight(left, 5)\n    tree:insertLeft(right, 6)\n    tree:updateValue(right, 30)\n    tree:delete(left)\nend)`, rust: `use ds4viz::prelude::*;\n\nfn main() -> ds4viz::Result<()> {\n    ds4viz::binary_tree("demo_tree", |tree| {\n        let root = tree.insert_root(1)?;\n        let left = tree.insert_left(root, 2)?;\n        let right = tree.insert_right(root, 3)?;\n        tree.insert_left(left, 4)?;\n        tree.insert_right(left, 5)?;\n        tree.insert_left(right, 6)?;\n        tree.update_value(right, 30)?;\n        tree.delete(left)?;\n        Ok(())\n    })\n}` } },
      { id: 'api-bst', title: 'BinarySearchTree', content: '二叉搜索树。支持 insert（插入，自动维护 BST 性质）和 delete（删除，自动处理叶子、单子、双子三种情况）操作。值类型限制为数值。', codeBlocks: { python: `import ds4viz as dv\n\nwith dv.binary_search_tree(label="demo_bst") as bst:\n    bst.insert(50)\n    bst.insert(30)\n    bst.insert(70)\n    bst.insert(20)\n    bst.insert(40)\n    bst.insert(60)\n    bst.insert(80)\n    bst.delete(30)\n    bst.insert(35)`, lua: `local ds4viz = require("ds4viz")\n\nds4viz.withContext(ds4viz.binarySearchTree("demo_bst"), function(bst)\n    bst:insert(50)\n    bst:insert(30)\n    bst:insert(70)\n    bst:insert(20)\n    bst:insert(40)\n    bst:insert(60)\n    bst:insert(80)\n    bst:delete(30)\n    bst:insert(35)\nend)`, rust: `use ds4viz::prelude::*;\n\nfn main() -> ds4viz::Result<()> {\n    ds4viz::binary_search_tree("demo_bst", |bst| {\n        bst.insert(50)?;\n        bst.insert(30)?;\n        bst.insert(70)?;\n        bst.insert(20)?;\n        bst.insert(40)?;\n        bst.insert(60)?;\n        bst.insert(80)?;\n        bst.delete(30)?;\n        bst.insert(35)?;\n        Ok(())\n    })\n}` } },
      { id: 'api-heap', title: 'Heap', content: '堆（优先队列）。支持 min（最小堆）和 max（最大堆）两种模式。insert 插入元素并自动上浮，extract 提取堆顶并自动下沉。内部以二叉树形式渲染。', codeBlocks: { python: `import ds4viz as dv\n\n# 最小堆\nwith dv.heap(label="demo_min_heap", heap_type="min") as h:\n    h.insert(15)\n    h.insert(10)\n    h.insert(20)\n    h.insert(5)\n    h.insert(30)\n    h.extract()\n    h.insert(2)\n    h.extract()`, lua: `local ds4viz = require("ds4viz")\n\n-- 最小堆\nds4viz.withContext(ds4viz.heap("demo_min_heap", "min"), function(h)\n    h:insert(15)\n    h:insert(10)\n    h:insert(20)\n    h:insert(5)\n    h:insert(30)\n    h:extract()\n    h:insert(2)\n    h:extract()\nend)`, rust: `use ds4viz::prelude::*;\n\nfn main() -> ds4viz::Result<()> {\n    // 最小堆\n    ds4viz::heap("demo_min_heap", HeapType::Min, |h| {\n        h.insert(15)?;\n        h.insert(10)?;\n        h.insert(20)?;\n        h.insert(5)?;\n        h.insert(30)?;\n        h.extract()?;\n        h.insert(2)?;\n        h.extract()?;\n        Ok(())\n    })\n}` } },
    ],
  },
  {
    id: 'graph',
    title: '图结构',
    children: [
      { id: 'api-graph-undirected', title: 'GraphUndirected', content: '无向图。支持 addNode（添加节点）、addEdge（添加无向边，内部自动规范化方向）、removeNode（删除节点及关联边）、removeEdge（删除边）、updateNodeLabel（更新节点标签）操作。不允许自环。', codeBlocks: { python: `import ds4viz as dv\n\nwith dv.graph_undirected(label="demo_graph") as g:\n    g.add_node(0, "A")\n    g.add_node(1, "B")\n    g.add_node(2, "C")\n    g.add_node(3, "D")\n    g.add_edge(0, 1)\n    g.add_edge(0, 2)\n    g.add_edge(1, 2)\n    g.add_edge(2, 3)\n    g.remove_edge(0, 2)\n    g.remove_node(3)`, lua: `local ds4viz = require("ds4viz")\n\nds4viz.withContext(ds4viz.graphUndirected("demo_graph"), function(g)\n    g:addNode(0, "A")\n    g:addNode(1, "B")\n    g:addNode(2, "C")\n    g:addNode(3, "D")\n    g:addEdge(0, 1)\n    g:addEdge(0, 2)\n    g:addEdge(1, 2)\n    g:addEdge(2, 3)\n    g:removeEdge(0, 2)\n    g:removeNode(3)\nend)`, rust: `use ds4viz::prelude::*;\n\nfn main() -> ds4viz::Result<()> {\n    ds4viz::graph_undirected("demo_graph", |g| {\n        g.add_node(0, "A")?;\n        g.add_node(1, "B")?;\n        g.add_node(2, "C")?;\n        g.add_node(3, "D")?;\n        g.add_edge(0, 1)?;\n        g.add_edge(0, 2)?;\n        g.add_edge(1, 2)?;\n        g.add_edge(2, 3)?;\n        g.remove_edge(0, 2)?;\n        g.remove_node(3)?;\n        Ok(())\n    })\n}` } },
      { id: 'api-graph-directed', title: 'GraphDirected', content: '有向图。API 与无向图一致，区别在于 addEdge 保留方向信息（from → to）。可视化时以箭头标识方向。', codeBlocks: { python: `import ds4viz as dv\n\nwith dv.graph_directed(label="demo_digraph") as g:\n    g.add_node(0, "A")\n    g.add_node(1, "B")\n    g.add_node(2, "C")\n    g.add_node(3, "D")\n    g.add_edge(0, 1)\n    g.add_edge(1, 2)\n    g.add_edge(2, 3)\n    g.add_edge(3, 0)\n    g.remove_edge(3, 0)\n    g.add_edge(0, 2)`, lua: `local ds4viz = require("ds4viz")\n\nds4viz.withContext(ds4viz.graphDirected("demo_digraph"), function(g)\n    g:addNode(0, "A")\n    g:addNode(1, "B")\n    g:addNode(2, "C")\n    g:addNode(3, "D")\n    g:addEdge(0, 1)\n    g:addEdge(1, 2)\n    g:addEdge(2, 3)\n    g:addEdge(3, 0)\n    g:removeEdge(3, 0)\n    g:addEdge(0, 2)\nend)`, rust: `use ds4viz::prelude::*;\n\nfn main() -> ds4viz::Result<()> {\n    ds4viz::graph_directed("demo_digraph", |g| {\n        g.add_node(0, "A")?;\n        g.add_node(1, "B")?;\n        g.add_node(2, "C")?;\n        g.add_node(3, "D")?;\n        g.add_edge(0, 1)?;\n        g.add_edge(1, 2)?;\n        g.add_edge(2, 3)?;\n        g.add_edge(3, 0)?;\n        g.remove_edge(3, 0)?;\n        g.add_edge(0, 2)?;\n        Ok(())\n    })\n}` } },
      { id: 'api-graph-weighted', title: 'GraphWeighted', content: '带权图。addEdge 需要额外传入 weight 参数，支持 updateWeight 更新边权重。通过 directed 参数控制有向/无向。可视化时在边上标注权值。', codeBlocks: { python: `import ds4viz as dv\n\nwith dv.graph_weighted(label="demo_weighted") as g:\n    g.add_node(0, "A")\n    g.add_node(1, "B")\n    g.add_node(2, "C")\n    g.add_node(3, "D")\n    g.add_edge(0, 1, 4.0)\n    g.add_edge(0, 2, 2.0)\n    g.add_edge(1, 3, 5.0)\n    g.add_edge(2, 3, 1.0)\n    g.update_weight(0, 1, 3.0)\n    g.remove_edge(2, 3)`, lua: `local ds4viz = require("ds4viz")\n\nds4viz.withContext(ds4viz.graphWeighted("demo_weighted"), function(g)\n    g:addNode(0, "A")\n    g:addNode(1, "B")\n    g:addNode(2, "C")\n    g:addNode(3, "D")\n    g:addEdge(0, 1, 4.0)\n    g:addEdge(0, 2, 2.0)\n    g:addEdge(1, 3, 5.0)\n    g:addEdge(2, 3, 1.0)\n    g:updateWeight(0, 1, 3.0)\n    g:removeEdge(2, 3)\nend)`, rust: `use ds4viz::prelude::*;\n\nfn main() -> ds4viz::Result<()> {\n    ds4viz::graph_weighted("demo_weighted", false, |g| {\n        g.add_node(0, "A")?;\n        g.add_node(1, "B")?;\n        g.add_node(2, "C")?;\n        g.add_node(3, "D")?;\n        g.add_edge(0, 1, 4.0)?;\n        g.add_edge(0, 2, 2.0)?;\n        g.add_edge(1, 3, 5.0)?;\n        g.add_edge(2, 3, 1.0)?;\n        g.update_weight(0, 1, 3.0)?;\n        g.remove_edge(2, 3)?;\n        Ok(())\n    })\n}` } },
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
            <button
              v-for="child in section.children"
              :key="child.id"
              class="toc-section__item"
              @click="scrollToSection(child.id)"
            >
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
            <div
              v-if="child.codeBlocks && Object.keys(child.codeBlocks).length > 0"
              class="doc-code-block"
            >
              <div class="doc-code-block__toolbar">
                <div class="doc-code-block__actions">
                  <button
                    class="code-action-btn"
                    :class="{ 'code-action-btn--copied': copiedId === child.id }"
                    @click="handleCopyCode(child.codeBlocks![getEffectiveLanguage(child.codeBlocks!)] ?? '', child.id)"
                  >
                    <svg v-if="copiedId !== child.id" viewBox="0 0 24 24" fill="currentColor"><path d="M16 1H4c-1.1 0-2 .9-2 2v14h2V3h12V1zm3 4H8c-1.1 0-2 .9-2 2v14c0 1.1.9 2 2 2h11c1.1 0 2-.9 2-2V7c0-1.1-.9-2-2-2zm0 16H8V7h11v14z" /></svg>
                    <svg v-else viewBox="0 0 24 24" fill="currentColor"><path d="M9 16.17L4.83 12l-1.42 1.41L9 19 21 7l-1.41-1.41z" /></svg>
                    <span>{{ copiedId === child.id ? '已复制' : '复制' }}</span>
                  </button>
                  <button
                    class="code-action-btn code-action-btn--primary"
                    @click="handleOpenInEditor(child.codeBlocks![getEffectiveLanguage(child.codeBlocks!)] ?? '', getEffectiveLanguage(child.codeBlocks!))"
                  >
                    <svg viewBox="0 0 24 24" fill="currentColor"><path d="M9.4 16.6L4.8 12l4.6-4.6L8 6l-6 6 6 6 1.4-1.4zm5.2 0l4.6-4.6-4.6-4.6L16 6l6 6-6 6-1.4-1.4z" /></svg>
                    <span>在编辑器中打开</span>
                  </button>
                </div>
                <div class="doc-code-block__lang-select">
                  <span
                    class="doc-code-block__dot"
                    :style="{ backgroundColor: getLanguageDotColor(getEffectiveLanguage(child.codeBlocks)) }"
                  />
                  <select
                    class="doc-code-block__select"
                    :value="getEffectiveLanguage(child.codeBlocks)"
                    @change="handleCodeBlockLangChange"
                  >
                    <option
                      v-for="lang in getCodeBlockLanguages(child.codeBlocks)"
                      :key="lang"
                      :value="lang"
                    >
                      {{ LANGUAGE_LABELS[lang as Language] ?? lang }}
                    </option>
                  </select>
                </div>
              </div>
              <div
                class="doc-code-block__editor"
                :style="{ height: getEditorHeight(child.codeBlocks) }"
              >
                <CodeEditor
                  :model-value="child.codeBlocks[getEffectiveLanguage(child.codeBlocks)] ?? ''"
                  :language="getEffectiveLanguage(child.codeBlocks)"
                  :readonly="true"
                />
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

.docs-page__header { display: flex; align-items: center; justify-content: space-between; flex-shrink: 0; }
.docs-page__title { display: flex; align-items: center; gap: 6px; font-size: var(--text-base); font-weight: var(--weight-semibold); color: var(--color-text-primary); }
.docs-page__title :deep(.material-icon) { width: 18px; height: 18px; }

.docs-page__body { display: grid; grid-template-columns: 200px 1fr; gap: var(--space-3); flex: 1; min-height: 0; overflow: hidden; }

.docs-page__toc { border: 1px solid var(--color-border); border-radius: var(--radius-lg); padding: var(--space-2); background-color: var(--color-bg-surface); overflow: auto; }
.toc-section { display: flex; flex-direction: column; gap: 2px; margin-bottom: var(--space-2); }
.toc-section__title { display: flex; align-items: center; gap: 4px; border: none; background: none; font-size: var(--text-sm); font-weight: var(--weight-semibold); color: var(--color-text-primary); cursor: pointer; padding: 4px 0; border-radius: var(--radius-sm); transition: color var(--duration-fast) var(--ease); }
.toc-section__title:hover { color: var(--color-accent); }
.toc-section__title :deep(.material-icon) { width: 16px; height: 16px; }
.toc-section__children { display: flex; flex-direction: column; gap: 2px; padding-left: 20px; }
.toc-section__item { border: none; background: none; text-align: left; font-size: var(--text-xs); color: var(--color-text-tertiary); cursor: pointer; padding: 3px 6px; border-radius: var(--radius-sm); transition: background-color var(--duration-fast) var(--ease), color var(--duration-fast) var(--ease); }
.toc-section__item:hover { background-color: var(--color-bg-hover); color: var(--color-accent); }

.docs-page__content { border: 1px solid var(--color-border); border-radius: var(--radius-lg); padding: var(--space-3); background-color: var(--color-bg-surface); overflow: auto; }
.doc-section { margin-bottom: var(--space-4); scroll-margin-top: var(--space-3); }
.doc-section__title { margin: 0 0 var(--space-1); font-size: 18px; font-weight: var(--weight-semibold); }
.doc-section__desc { margin: 0 0 var(--space-2); font-size: var(--text-sm); color: var(--color-text-body); line-height: var(--leading-relaxed); }
.doc-subsection { margin-bottom: var(--space-3); scroll-margin-top: var(--space-3); }
.doc-subsection__title { margin: 0 0 var(--space-1); font-size: 15px; font-weight: var(--weight-semibold); }
.doc-subsection__desc { margin: 0 0 var(--space-1); font-size: var(--text-sm); color: var(--color-text-body); line-height: var(--leading-relaxed); }

.doc-code-block { position: relative; border: 1px solid var(--color-border); border-radius: var(--radius-md); background-color: var(--color-bg-surface-alt); overflow: hidden; }
.doc-code-block__toolbar { display: flex; align-items: center; justify-content: space-between; gap: var(--space-1); padding: 6px 8px; border-bottom: 1px solid var(--color-border); background-color: var(--color-bg-surface); flex-wrap: wrap; }
.doc-code-block__actions { display: flex; align-items: center; gap: var(--space-1); }
.doc-code-block__lang-select { display: inline-flex; align-items: center; gap: 6px; padding: 0 10px; height: 28px; border: 1px solid var(--color-border-strong); border-radius: var(--radius-control); background-color: var(--color-bg-surface); }
.doc-code-block__dot { width: 7px; height: 7px; border-radius: 999px; flex-shrink: 0; }
.doc-code-block__select { font-size: var(--text-xs); color: var(--color-text-primary); background: none; border: none; outline: none; cursor: pointer; font-family: inherit; }
.doc-code-block__editor { display: flex; border-radius: 0; overflow: hidden; }

/* ---- 统一操作按钮 ---- */
.code-action-btn { display: inline-flex; align-items: center; gap: 4px; height: 28px; padding: 0 8px; border: 1px solid var(--color-border-strong); border-radius: var(--radius-control); background-color: var(--color-bg-surface); color: var(--color-text-body); font-size: var(--text-xs); font-family: inherit; cursor: pointer; transition: background-color var(--duration-fast) var(--ease), border-color var(--duration-fast) var(--ease), color var(--duration-fast) var(--ease); }
.code-action-btn:hover { background-color: var(--color-bg-hover); color: var(--color-text-primary); }
.code-action-btn svg { width: 14px; height: 14px; flex-shrink: 0; }
.code-action-btn--copied { border-color: var(--color-success); color: var(--color-success); }
.code-action-btn--primary { border-color: var(--color-accent); background-color: var(--color-accent); color: var(--color-accent-contrast); }
.code-action-btn--primary:hover { background-color: var(--color-accent-hover); border-color: var(--color-accent-hover); }

@media (max-width: 1100px) {
  .docs-page__body { grid-template-columns: 1fr; }
  .docs-page__toc { order: 2; }
}
</style>
