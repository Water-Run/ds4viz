<script setup lang="ts">
/**
 * 可视化占位状态
 *
 * @component VizPlaceholder
 */
</script>

<template>
  <div class="viz-placeholder">
    <span class="material-symbols-outlined viz-placeholder__icon">data_array</span>
    <div class="viz-placeholder__content">
      <p class="viz-placeholder__title">等待渲染</p>
      <p class="viz-placeholder__desc">运行代码或上传 TOML 查看结果</p>
    </div>
  </div>
</template>

<style scoped>
.viz-placeholder {
  display: flex;
  align-items: center;
  justify-content: center;
  flex-direction: column;
  gap: var(--space-1);
  width: 100%;
  height: 100%;
  border: 1px dashed var(--color-border);
  border-radius: var(--radius-lg);
  background-color: var(--color-bg-surface-alt);
  color: var(--color-text-tertiary);
}

.viz-placeholder__icon {
  font-size: 40px;
}

.viz-placeholder__content {
  display: flex;
  flex-direction: column;
  align-items: center;
  gap: 2px;
}

.viz-placeholder__title {
  margin: 0;
  font-size: var(--text-sm);
  font-weight: var(--weight-medium);
  color: var(--color-text-primary);
}

.viz-placeholder__desc {
  margin: 0;
  font-size: var(--text-xs);
}
</style>
