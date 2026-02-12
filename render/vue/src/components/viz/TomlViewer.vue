<script setup lang="ts">
/**
 * TOML 查看器
 *
 * @component TomlViewer
 */

/**
 * 组件属性定义
 */
interface Props {
  /** TOML 内容 */
  content: string
}

defineProps<Props>()
</script>

<template>
  <pre class="toml-viewer"><code>{{ content }}</code></pre>
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
</style>
