<script setup lang="ts">
/**
 * 应用根组件，挂载全局路由视图并提供路由级过渡
 *
 * @component App
 */
</script>

<template>
  <router-view v-slot="{ Component, route }">
    <Transition name="route-fade" mode="out-in">
      <component :is="Component" :key="route.matched[0]?.path ?? route.path" />
    </Transition>
  </router-view>
</template>
