<script setup lang="ts">
/**
 * 文档页面
 *
 * @component Docs
 */

import { computed, ref } from 'vue'

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
 * 选中语言
 */
const selectedLanguage = ref<Language>('python')

/**
 * 展开状态
 */
const expanded = ref<Record<string, boolean>>({
  intro: true,
  guides: true,
  api: true,
})

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
      'ds4viz 是面向数据结构教学的可视化框架。通过执行示例代码，自动生成 TOML IR 并渲染为步骤化动画。',
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
        id: 'guide-run',
        title: '运行',
        content: '在编辑器中运行代码，生成 TOML 并进入可视化。',
      },
    ],
  },
  {
    id: 'api',
    title: '核心 API',
    children: [
      {
        id: 'api-stack',
        title: 'Stack',
        content: '演示栈的 push / pop 操作。',
        codeBlocks: {
          python:
            'import ds4viz as dv\n\nwith dv.stack(label="demo") as s:\n    s.push(1)\n    s.pop()',
          lua:
            'local dv = require("ds4viz")\n\ndv.withContext(dv.stack("demo"), function(s)\n  s:push(1)\n  s:pop()\nend)',
          rust:
            'use ds4viz::prelude::*;\n\nfn main() -> ds4viz::Result<()> {\n  ds4viz::stack("demo", |s| {\n    s.push(1)?;\n    s.pop()?;\n    Ok(())\n  })\n}',
        },
      },
      {
        id: 'api-queue',
        title: 'Queue',
        content: '演示队列的 enqueue / dequeue 操作。',
        codeBlocks: {
          python:
            'import ds4viz as dv\n\nwith dv.queue(label="demo") as q:\n    q.enqueue(1)\n    q.dequeue()',
          lua:
            'local dv = require("ds4viz")\n\ndv.withContext(dv.queue("demo"), function(q)\n  q:enqueue(1)\n  q:dequeue()\nend)',
          rust:
            'use ds4viz::prelude::*;\n\nfn main() -> ds4viz::Result<()> {\n  ds4viz::queue("demo", |q| {\n    q.enqueue(1)?;\n    q.dequeue()?;\n    Ok(())\n  })\n}',
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
 * 切换语言
 *
 * @param language - 语言标识
 */
const handleLanguageChange = (language: Language): void => {
  selectedLanguage.value = language
}
</script>

<template>
  <div class="docs-page">
    <header class="docs-page__header">
      <div class="docs-page__title">
        <MaterialIcon name="menu_book" :size="18" />
        <span>文档</span>
      </div>
      <div class="docs-page__lang">
        <button
          v-for="lang in LANGUAGES"
          :key="lang"
          class="docs-page__lang-btn"
          :class="{ 'docs-page__lang-btn--active': selectedLanguage === lang }"
          @click="handleLanguageChange(lang)"
        >
          {{ LANGUAGE_LABELS[lang] }}
        </button>
      </div>
    </header>

    <div class="docs-page__body">
      <aside class="docs-page__toc">
        <div v-for="section in docTree" :key="section.id" class="toc-section">
          <button class="toc-section__title" @click="toggleNode(section.id)">
            <MaterialIcon :name="getToggleIcon(expanded[section.id])" :size="18" />
            <span>{{ section.title }}</span>
          </button>
          <div v-if="expanded[section.id]" class="toc-section__children">
            <button
              v-for="child in section.children"
              :key="child.id"
              class="toc-section__item"
            >
              {{ child.title }}
            </button>
          </div>
        </div>
      </aside>

      <section class="docs-page__content">
        <div v-for="section in docTree" :key="section.id" class="doc-section">
          <h2 class="doc-section__title">{{ section.title }}</h2>
          <p v-if="section.content" class="doc-section__desc">{{ section.content }}</p>
          <div v-for="child in section.children" :key="child.id" class="doc-subsection">
            <h3 class="doc-subsection__title">{{ child.title }}</h3>
            <p v-if="child.content" class="doc-subsection__desc">{{ child.content }}</p>
            <pre
              v-if="child.codeBlocks && child.codeBlocks[selectedLanguage]"
              class="doc-code"
            ><code>{{ child.codeBlocks[selectedLanguage] }}</code></pre>
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

.docs-page__lang {
  display: inline-flex;
  gap: var(--space-1);
}

.docs-page__lang-btn {
  height: var(--control-height-sm);
  padding: 0 12px;
  border-radius: var(--radius-control);
  border: 1px solid var(--color-border-strong);
  background-color: var(--color-bg-surface);
  color: var(--color-text-body);
  font-size: var(--text-xs);
  transition:
    border-color var(--duration-fast) var(--ease),
    background-color var(--duration-fast) var(--ease),
    color var(--duration-fast) var(--ease);
}

.docs-page__lang-btn--active {
  border-color: var(--color-accent);
  color: var(--color-accent);
  background-color: var(--color-accent-wash);
}

.docs-page__body {
  display: grid;
  grid-template-columns: 220px 1fr;
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
  gap: 4px;
  margin-bottom: var(--space-2);
}

.toc-section__title {
  display: flex;
  align-items: center;
  gap: 6px;
  border: none;
  background: none;
  font-size: var(--text-sm);
  font-weight: var(--weight-semibold);
  color: var(--color-text-primary);
  cursor: pointer;
}

.toc-section__title :deep(.material-icon) {
  width: 18px;
  height: 18px;
}

.toc-section__children {
  display: flex;
  flex-direction: column;
  gap: 4px;
  padding-left: 20px;
}

.toc-section__item {
  border: none;
  background: none;
  text-align: left;
  font-size: var(--text-xs);
  color: var(--color-text-tertiary);
  cursor: pointer;
  padding: 2px 4px;
  border-radius: var(--radius-sm);
  transition: background-color var(--duration-fast) var(--ease), color var(--duration-fast) var(--ease);
}

.toc-section__item:hover {
  background-color: var(--color-bg-hover);
  color: var(--color-text-primary);
}

.docs-page__content {
  border: 1px solid var(--color-border);
  border-radius: var(--radius-lg);
  padding: var(--space-3);
  background-color: var(--color-bg-surface);
  overflow: auto;
}

.doc-section {
  margin-bottom: var(--space-3);
}

.doc-section__title {
  margin: 0 0 var(--space-1);
  font-size: var(--text-base);
  font-weight: var(--weight-semibold);
}

.doc-section__desc {
  margin: 0 0 var(--space-2);
  font-size: var(--text-sm);
  color: var(--color-text-body);
  line-height: var(--leading-relaxed);
}

.doc-subsection {
  margin-bottom: var(--space-2);
}

.doc-subsection__title {
  margin: 0 0 var(--space-1);
  font-size: var(--text-sm);
  font-weight: var(--weight-semibold);
}

.doc-subsection__desc {
  margin: 0 0 var(--space-1);
  font-size: var(--text-sm);
  color: var(--color-text-body);
}

.doc-code {
  margin: 0;
  padding: var(--space-2);
  border-radius: var(--radius-md);
  background-color: var(--color-bg-surface-alt);
  border: 1px solid var(--color-border);
  font-family: var(--font-mono);
  font-size: var(--text-xs);
  line-height: var(--leading-code);
}

@media (max-width: 1100px) {
  .docs-page__body {
    grid-template-columns: 1fr;
  }

  .docs-page__toc {
    order: 2;
  }
}

.docs-page__lang-btn:hover {
  border-color: var(--color-accent);
}
</style>
