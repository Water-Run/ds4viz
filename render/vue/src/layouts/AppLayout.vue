<script setup lang="ts">
/**
 * 应用壳布局
 *
 * @component AppLayout
 */

import { computed, onBeforeUnmount, onMounted, ref, watch } from 'vue'
import { useRouter, useRoute } from 'vue-router'

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

/** 默认头像颜色调色板（低饱和、适合白色前景文字） */
const AVATAR_COLORS: readonly string[] = [
  '#0078d4',
  '#0e7c6b',
  '#7c3aed',
  '#c2410c',
  '#0369a1',
  '#6d28d9',
  '#b45309',
  '#059669',
  '#dc2626',
  '#4f46e5',
]

/**
 * 根据用户名生成确定性头像背景色
 *
 * 使用 DJB2 变体哈希，对调色板长度取模。
 *
 * @param username - 用户名
 * @returns 十六进制颜色字符串
 */
function getAvatarColor(username: string): string {
  let hash = 0
  for (let i = 0; i < username.length; i += 1) {
    hash = ((hash << 5) - hash + username.charCodeAt(i)) | 0
  }
  return AVATAR_COLORS[Math.abs(hash) % AVATAR_COLORS.length]
}

/**
 * 获取用户名首字母（大写）
 *
 * @param username - 用户名
 * @returns 大写首字母，空值时返回 '-'
 */
function getAvatarInitial(username: string): string {
  return username.length > 0 ? username.charAt(0).toUpperCase() : '-'
}

const router = useRouter()
const route = useRoute()
const authStore = useAuthStore()

/** 侧栏收起状态 */
const collapsed = ref<boolean>(false)

/** 初始化状态 */
const initializing = ref<boolean>(true)

/** 是否为移动端视口 */
const isMobile = ref<boolean>(false)

/** MediaQueryList 引用，用于清理监听 */
let mediaQuery: MediaQueryList | null = null

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
  return getAvatarInitial(currentUser.value?.username ?? '')
})

/** 用户头像背景色（基于用户名哈希） */
const userAvatarColor = computed<string>(() => {
  return getAvatarColor(currentUser.value?.username ?? '')
})

/**
 * 响应视口变化，进入移动模式时自动收起侧栏
 *
 * @param event - 媒体查询变化事件
 */
const handleMediaChange = (event: MediaQueryListEvent): void => {
  isMobile.value = event.matches
  if (event.matches && !collapsed.value) {
    collapsed.value = true
  }
}

/** 切换侧栏 */
const toggleSidebar = (): void => {
  collapsed.value = !collapsed.value
}

/** 关闭移动端侧栏（点击遮罩层） */
const closeMobileSidebar = (): void => {
  collapsed.value = true
}

/** 退出登录 */
const handleLogout = async (): Promise<void> => {
  await authStore.logout()
  await router.push({ name: 'login' })
}

/**
 * 路由变化时，移动端自动收起侧栏
 */
watch(
  () => route.path,
  () => {
    if (isMobile.value && !collapsed.value) {
      collapsed.value = true
    }
  },
)

onMounted(async () => {
  /* ---- 响应式侧栏 ---- */
  mediaQuery = window.matchMedia('(max-width: 980px)')
  isMobile.value = mediaQuery.matches
  if (isMobile.value) {
    collapsed.value = true
  }
  mediaQuery.addEventListener('change', handleMediaChange)

  /* ---- 用户认证 ---- */
  try {
    await authStore.fetchCurrentUser()
  } catch {
    await authStore.logout()
    router.push({ name: 'login' })
    return
  }
  initializing.value = false
})

onBeforeUnmount(() => {
  mediaQuery?.removeEventListener('change', handleMediaChange)
})
</script>

<template>
  <div class="app-layout">
    <aside class="sidebar" :class="{ 'sidebar--collapsed': collapsed }">
      <div class="sidebar__header">
        <img
          v-show="!collapsed"
          src="/ds4viz/logo.png"
          alt="DS4Viz"
          class="sidebar__logo-img"
        />
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
          <div
            class="sidebar__avatar"
            :style="{ backgroundColor: userAvatarColor }"
          >
            {{ userInitial }}
          </div>
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

    <!-- 移动端遮罩层 -->
    <Transition name="fade">
      <div
        v-if="isMobile && !collapsed"
        class="sidebar-overlay"
        @click="closeMobileSidebar"
      />
    </Transition>

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
  height: 100dvh;
  overflow: hidden;
}

.app-layout__main {
  flex: 1;
  min-width: 0;
  height: 100%;
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

/* ---- 遮罩层 ---- */

.sidebar-overlay {
  position: fixed;
  inset: 0;
  z-index: 199;
  background-color: rgba(0, 0, 0, 0.3);
}

/* ---- 侧栏 ---- */

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

.sidebar__logo-img {
  width: 28px;
  height: 28px;
  flex-shrink: 0;
  border-radius: var(--radius-control);
  object-fit: contain;
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
  border-radius: 8px;
  color: #ffffff;
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