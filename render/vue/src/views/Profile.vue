<script setup lang="ts">
// placeholder — 个人资料页
</script>

<template>
  <div class="page-placeholder">
    <span class="material-symbols-outlined page-placeholder__icon">person</span>
    <h2 class="page-placeholder__title">个人资料</h2>
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