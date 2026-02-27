<script setup lang="ts">
/**
 * TOML 查看器（带基础语法高亮）
 *
 * 对 Section、Key、String、Number、Boolean、Comment 进行着色。
 *
 * @file src/components/viz/TomlViewer.vue
 * @author WaterRun
 * @date 2026-02-27
 * @component TomlViewer
 */

import { computed } from 'vue'

/**
 * 组件属性定义
 */
interface Props {
  /** TOML 内容 */
  content: string
}

const props = defineProps<Props>()

/**
 * HTML 实体转义
 *
 * @param text - 原始文本
 * @returns 转义后的安全文本
 */
function escapeHtml(text: string): string {
  return text
    .replace(/&/g, '&amp;')
    .replace(/</g, '&lt;')
    .replace(/>/g, '&gt;')
    .replace(/"/g, '&quot;')
}

/**
 * 对值部分着色
 *
 * @param raw - 等号后的原始文本（含前导空格）
 * @returns 带 span 的 HTML
 */
function colorizeValue(raw: string): string {
  const trimmed = raw.trim()

  if (trimmed.startsWith('"') || trimmed.startsWith("'")) {
    return `<span class="toml-hl-string">${escapeHtml(raw)}</span>`
  }

  if (trimmed === 'true' || trimmed === 'false') {
    return `<span class="toml-hl-boolean">${escapeHtml(raw)}</span>`
  }

  if (/^-?[0-9]/.test(trimmed) || /^-?inf$/.test(trimmed) || trimmed === 'nan') {
    return `<span class="toml-hl-number">${escapeHtml(raw)}</span>`
  }

  if (trimmed.startsWith('[') || trimmed.startsWith('{')) {
    return `<span class="toml-hl-value">${escapeHtml(raw)}</span>`
  }

  return escapeHtml(raw)
}

/**
 * 按行解析并生成高亮 HTML
 */
const highlightedHtml = computed<string>(() => {
  return props.content
    .split('\n')
    .map((line) => {
      const trimmed = line.trim()

      if (trimmed.length === 0) return ''

      if (trimmed.startsWith('#')) {
        return `<span class="toml-hl-comment">${escapeHtml(line)}</span>`
      }

      if (trimmed.startsWith('[')) {
        return `<span class="toml-hl-section">${escapeHtml(line)}</span>`
      }

      const eqIndex = line.indexOf('=')
      if (eqIndex > 0) {
        const keyPart = escapeHtml(line.substring(0, eqIndex))
        const valuePart = line.substring(eqIndex + 1)
        return `<span class="toml-hl-key">${keyPart}</span><span class="toml-hl-eq">=</span>${colorizeValue(valuePart)}`
      }

      return escapeHtml(line)
    })
    .join('\n')
})
</script>

<template>
  <!-- eslint-disable vue/no-v-html -->
  <pre class="toml-viewer"><code v-html="highlightedHtml"></code></pre>
</template>

<style scoped>
.toml-viewer {
  margin: 0;
  padding: var(--space-2);
  border: 1px solid var(--color-border);
  border-radius: var(--radius-lg);
  background-color: var(--color-bg-surface);
  font-family: var(--font-mono);
  font-size: var(--text-xs);
  line-height: var(--leading-code);
  color: var(--color-text-body);
  overflow: auto;
  max-height: 240px;
}

.toml-viewer :deep(.toml-hl-section) {
  color: var(--color-accent);
  font-weight: var(--weight-semibold);
}

.toml-viewer :deep(.toml-hl-key) {
  color: var(--color-text-primary);
  font-weight: var(--weight-medium);
}

.toml-viewer :deep(.toml-hl-eq) {
  color: var(--color-text-tertiary);
}

.toml-viewer :deep(.toml-hl-string) {
  color: #22863a;
}

.toml-viewer :deep(.toml-hl-number) {
  color: #005cc5;
}

.toml-viewer :deep(.toml-hl-boolean) {
  color: #6f42c1;
}

.toml-viewer :deep(.toml-hl-value) {
  color: var(--color-text-body);
}

.toml-viewer :deep(.toml-hl-comment) {
  color: var(--color-text-tertiary);
  font-style: italic;
}
</style>
