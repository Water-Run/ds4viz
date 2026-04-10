<script setup lang="ts">
/**
 * 可视化占位状态
 *
 * @component VizPlaceholder
 */

import MaterialIcon from '@/components/common/MaterialIcon.vue'
</script>

<template>
  <div class="viz-placeholder">
    <MaterialIcon name="data_array" class="viz-placeholder__icon" :size="40" />
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
  background-color: var(--color-bg-surface-alt);
  color: var(--color-text-tertiary);
}

.viz-placeholder__icon {
  width: 40px;
  height: 40px;
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
