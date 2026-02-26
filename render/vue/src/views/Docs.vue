<script setup lang="ts">
/**
 * 文档页面
 *
 * @component Docs
 */

import { computed, onMounted, ref } from 'vue'

import { LANGUAGES, LANGUAGE_LABELS } from '@/types/api'
import MaterialIcon from '@/components/common/MaterialIcon.vue'
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
 * 滚动到指定文档节（顶部留出间距）
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
 *
 * @param codeBlocks - 代码块映射
 * @returns 可用语言数组
 */
const getCodeBlockLanguages = (codeBlocks: Partial<Record<Language, string>>): Language[] => {
  return Object.keys(codeBlocks) as Language[]
}

/**
 * 获取代码块的有效显示语言
 *
 * @param codeBlocks - 代码块映射
 * @returns 有效语言标识
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
 *
 * @param language - 语言标识
 * @returns CSS 颜色值
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
 * 处理代码块语言下拉切换（全局同步）
 *
 * @param event - 原生 change 事件
 */
const handleCodeBlockLangChange = (event: Event): void => {
  const target = event.target as HTMLSelectElement
  selectedLanguage.value = target.value as Language
}

/**
 * 初始化时同步编辑器页的语言选择
 */
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
              <pre class="doc-code"><code>{{ child.codeBlocks[getEffectiveLanguage(child.codeBlocks)] }}</code></pre>
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

/* ---- 主体 ---- */

.docs-page__body {
  display: grid;
  grid-template-columns: 200px 1fr;
  gap: var(--space-3);
  flex: 1;
  min-height: 0;
  overflow: hidden;
}

/* ---- 目录侧栏 ---- */

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
  transition:
    background-color var(--duration-fast) var(--ease),
    color var(--duration-fast) var(--ease);
}

.toc-section__item:hover {
  background-color: var(--color-bg-hover);
  color: var(--color-accent);
}

/* ---- 内容区 ---- */

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

/* ---- 代码块容器 ---- */

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
  justify-content: flex-end;
  padding: 6px 8px;
  border-bottom: 1px solid var(--color-border);
  background-color: var(--color-bg-surface);
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

.doc-code {
  margin: 0;
  padding: var(--space-2);
  font-family: var(--font-mono);
  font-size: var(--text-sm);
  line-height: 1.65;
  color: var(--color-text-body);
  overflow: auto;
}

/* ---- 响应式 ---- */

@media (max-width: 1100px) {
  .docs-page__body {
    grid-template-columns: 1fr;
  }

  .docs-page__toc {
    order: 2;
  }
}
</style>
