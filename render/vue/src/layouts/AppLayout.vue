<script setup lang="ts">
/**
 * 应用壳布局
 *
 * @component AppLayout
 */

import { computed, onMounted, ref } from 'vue'
import { useRouter } from 'vue-router'

import Loading from '@/components/common/Loading.vue'
import MaterialIcon from '@/components/common/MaterialIcon.vue'
import { useAuthStore } from '@/stores/auth'

/**
 * 导航项
 *
 * @interface
 */
interface NavItem {
  /** 标题 */
  label: string
  /** 图标名称 */
  icon: string
  /** 路由路径 */
  to: string
}

const router = useRouter()
const authStore = useAuthStore()

/** 侧栏收起状态 */
const collapsed = ref<boolean>(false)

/** 初始化状态 */
const initializing = ref<boolean>(true)

/** 导航列表 */
const navItems: NavItem[] = [
  { label: '编辑器', icon: 'code', to: '/playground' },
  { label: '模板库', icon: 'dashboard', to: '/templates' },
  { label: '文档', icon: 'menu_book', to: '/docs' },
]

/** 当前用户 */
const currentUser = computed(() => authStore.currentUser)

/** 用户头像初始字母 */
const userInitial = computed<string>(() => {
  const name = currentUser.value?.username ?? ''
  return name.length > 0 ? name.charAt(0).toUpperCase() : '-'
})

/** 切换侧栏 */
const toggleSidebar = (): void => {
  collapsed.value = !collapsed.value
}

/** 退出登录 */
const handleLogout = async (): Promise<void> => {
  await authStore.logout()
  await router.push({ name: 'login' })
}

onMounted(async () => {
  try {
    await authStore.fetchCurrentUser()
  } catch {
    await authStore.logout()
    router.push({ name: 'login' })
    return
  }
  initializing.value = false
})
</script>

<template>
  <div class="app-layout">
    <aside class="sidebar" :class="{ 'sidebar--collapsed': collapsed }">
      <div class="sidebar__header">
        <div v-show="!collapsed" class="sidebar__logo-mark">
          <MaterialIcon name="insights" :size="18" />
        </div>
        <span v-show="!collapsed" class="sidebar__brand">DS4Viz</span>
        <button
          class="sidebar__toggle"
          :aria-label="collapsed ? '展开侧边栏' : '折叠侧边栏'"
          @click="toggleSidebar"
        >
          <MaterialIcon :name="collapsed ? 'menu' : 'menu_open'" :size="20" />
        </button>
      </div>

      <nav class="sidebar__nav">
        <router-link
          v-for="item in navItems"
          :key="item.to"
          :to="item.to"
          class="nav-item"
          :title="collapsed ? item.label : undefined"
        >
          <MaterialIcon :name="item.icon" class="nav-item__icon" :size="20" />
          <span v-show="!collapsed" class="nav-item__label">{{ item.label }}</span>
        </router-link>
      </nav>

      <div class="sidebar__footer">
        <router-link to="/profile" class="sidebar__user" title="个人资料">
          <div class="sidebar__avatar">{{ userInitial }}</div>
          <span v-show="!collapsed" class="sidebar__username">
            {{ currentUser?.username ?? '' }}
          </span>
        </router-link>
        <button class="sidebar__logout" title="退出登录" @click="handleLogout">
          <MaterialIcon name="logout" :size="20" />
          <span v-show="!collapsed" class="sidebar__logout-label">退出</span>
        </button>
      </div>
    </aside>

    <main class="app-layout__main">
      <div v-if="initializing" class="app-layout__loading">
        <Loading message="加载中..." />
      </div>
      <router-view v-else />
    </main>
  </div>
</template>

<style scoped>
.app-layout {
  display: flex;
  min-height: 100dvh;
}

.app-layout__main {
  flex: 1;
  min-width: 0;
  display: flex;
  flex-direction: column;
  overflow-y: auto;
  background: var(--color-bg-base);
}

.app-layout__loading {
  flex: 1;
  display: flex;
  align-items: center;
  justify-content: center;
}

.sidebar {
  position: sticky;
  top: 0;
  height: 100dvh;
  width: var(--sidebar-width);
  flex-shrink: 0;
  display: flex;
  flex-direction: column;
  background: var(--color-bg-surface);
  border-right: 1px solid var(--color-border);
  overflow: hidden;
  transition: width var(--duration-normal) var(--ease);
}

.sidebar--collapsed {
  width: var(--sidebar-width-collapsed);
}

.sidebar__header {
  display: flex;
  align-items: center;
  gap: 8px;
  height: 56px;
  padding: 0 12px;
  flex-shrink: 0;
}

.sidebar--collapsed .sidebar__header {
  justify-content: center;
}

.sidebar__logo-mark {
  width: 28px;
  height: 28px;
  flex-shrink: 0;
  display: flex;
  align-items: center;
  justify-content: center;
  background: var(--color-accent);
  color: #fff;
  border-radius: var(--radius-control);
}

.sidebar__brand {
  flex: 1;
  font-size: 16px;
  font-weight: 600;
  color: var(--color-text-primary);
  white-space: nowrap;
  overflow: hidden;
}

.sidebar__toggle {
  display: flex;
  align-items: center;
  justify-content: center;
  width: var(--control-height-sm);
  height: var(--control-height-sm);
  flex-shrink: 0;
  border: none;
  background: none;
  color: var(--color-text-secondary);
  border-radius: var(--radius-control);
  cursor: pointer;
  transition:
    color var(--duration-fast) var(--ease),
    background var(--duration-fast) var(--ease);
}

.sidebar__toggle:hover {
  background: var(--color-bg-hover);
  color: var(--color-text-primary);
}

.sidebar__nav {
  flex: 1;
  display: flex;
  flex-direction: column;
  gap: 2px;
  padding: 8px;
  overflow-y: auto;
  overflow-x: hidden;
}

.nav-item {
  display: flex;
  align-items: center;
  gap: 10px;
  height: var(--control-height-md);
  padding: 0 10px;
  border-radius: var(--radius-control);
  color: var(--color-text-secondary);
  text-decoration: none;
  white-space: nowrap;
  cursor: pointer;
  transition:
    background var(--duration-fast) var(--ease),
    color var(--duration-fast) var(--ease);
}

.nav-item:hover {
  background: var(--color-bg-hover);
  color: var(--color-text-primary);
}

.nav-item.router-link-active {
  background: var(--color-accent-wash);
  color: var(--color-accent);
}

.nav-item.router-link-active:hover {
  background: var(--color-accent-wash);
}

.nav-item__icon {
  width: 20px;
  height: 20px;
  flex-shrink: 0;
}

.nav-item__label {
  font-size: 14px;
  font-weight: 500;
  overflow: hidden;
}

.sidebar--collapsed .nav-item {
  justify-content: center;
}

.sidebar__footer {
  flex-shrink: 0;
  display: flex;
  flex-direction: column;
  gap: 2px;
  padding: 8px;
  border-top: 1px solid var(--color-border);
}

.sidebar__user {
  display: flex;
  align-items: center;
  gap: 10px;
  padding: 6px 10px;
  border-radius: var(--radius-control);
  text-decoration: none;
  color: var(--color-text-primary);
  white-space: nowrap;
  cursor: pointer;
  transition: background var(--duration-fast) var(--ease);
}

.sidebar__user:hover {
  background: var(--color-bg-hover);
}

.sidebar__user.router-link-active {
  background: var(--color-accent-wash);
}

.sidebar__avatar {
  width: 28px;
  height: 28px;
  flex-shrink: 0;
  display: flex;
  align-items: center;
  justify-content: center;
  border-radius: 50%;
  background: var(--color-accent-wash);
  color: var(--color-accent);
  font-size: 13px;
  font-weight: 600;
  line-height: 1;
}

.sidebar__username {
  font-size: 14px;
  font-weight: 500;
  overflow: hidden;
  text-overflow: ellipsis;
}

.sidebar--collapsed .sidebar__user {
  justify-content: center;
  padding: 6px;
}

.sidebar__logout {
  display: flex;
  align-items: center;
  gap: 10px;
  height: var(--control-height-md);
  padding: 0 10px;
  border: none;
  background: none;
  border-radius: var(--radius-control);
  color: var(--color-text-secondary);
  font-family: inherit;
  cursor: pointer;
  white-space: nowrap;
  transition:
    background var(--duration-fast) var(--ease),
    color var(--duration-fast) var(--ease);
}

.sidebar__logout:hover {
  background: var(--color-bg-hover);
  color: var(--color-error);
}

.sidebar__logout-label {
  font-size: 14px;
  font-weight: 500;
}

.sidebar--collapsed .sidebar__logout {
  justify-content: center;
}

@media (max-width: 980px) {
  .sidebar {
    position: fixed;
    left: 0;
    top: 0;
    z-index: var(--z-sticky);
  }

  .app-layout__main {
    margin-left: var(--sidebar-width-collapsed);
  }
}
</style>
