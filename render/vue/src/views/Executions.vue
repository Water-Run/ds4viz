<script setup lang="ts">
// placeholder — 执行记录页
</script>

<template>
  <div class="page-placeholder">
    <span class="material-symbols-outlined page-placeholder__icon">history</span>
    <h2 class="page-placeholder__title">执行记录</h2>
    <p class="page-placeholder__desc">此页面正在开发中</p>
  </div>
</template>

<style scoped>
.page-placeholder {
  flex: 1;
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  gap: 12px;
  color: var(--color-text-muted);
}

.page-placeholder__icon {
  font-size: 48px;
}

.page-placeholder__title {
  margin: 0;
  font-size: 20px;
  font-weight: 600;
  color: var(--color-text-secondary);
}

.page-placeholder__desc {
  margin: 0;
  font-size: 14px;
}
</style>