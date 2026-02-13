<script setup lang="ts">
/**
 * 404 页面
 *
 * @component NotFound
 */

import { useRouter } from 'vue-router'
import MaterialIcon from '@/components/common/MaterialIcon.vue'

const router = useRouter()

/**
 * 返回首页
 */
function goHome(): void {
  router.push({ name: 'playground' })
}
</script>

<template>
  <div class="not-found">
    <MaterialIcon name="explore_off" class="not-found__icon" />
    <h1 class="not-found__code">404</h1>
    <p class="not-found__message">页面未找到</p>
    <button class="not-found__action" @click="goHome">返回首页</button>
  </div>
</template>

<style scoped>
.not-found {
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  min-height: 100dvh;
  gap: 12px;
  color: var(--color-text-tertiary);
  padding: 24px;
}

.not-found__icon {
  width: 64px;
  height: 64px;
}

.not-found__code {
  margin: 0;
  font-size: 48px;
  font-weight: 700;
  color: var(--color-text-primary);
  line-height: 1;
}

.not-found__message {
  margin: 0;
  font-size: 16px;
}

.not-found__action {
  margin-top: 12px;
  height: var(--control-height-md);
  padding: 0 20px;
  border: none;
  border-radius: var(--radius-control);
  background: var(--color-accent);
  color: #fff;
  font-size: 14px;
  font-weight: 500;
  font-family: inherit;
  cursor: pointer;
  transition:
    background var(--duration-fast) var(--ease);
}

.not-found__action:hover {
  background: var(--color-accent-hover);
}
</style>
