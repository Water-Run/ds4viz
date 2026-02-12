<script setup lang="ts">
/**
 * App header placeholder
 *
 * @component AppHeader
 */
</script>

<template>
  <header class="app-header">
    <div class="app-header__title">DS4Viz</div>
  </header>
</template>

<style scoped>
.app-header {
  display: flex;
  align-items: center;
  justify-content: space-between;
  height: var(--header-height);
  padding: 0 var(--space-3);
  border-bottom: 1px solid var(--color-border);
  background-color: var(--color-bg-surface);
}

.app-header__title {
  font-size: var(--text-base);
  font-weight: var(--weight-semibold);
  color: var(--color-text-primary);
}
</style>
