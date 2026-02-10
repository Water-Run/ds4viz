/**
 * 路由配置及全局导航守卫
 *
 * 认证页（login / register）标记 guest meta，
 * 应用页由 AppLayout 包裹并标记 requiresAuth。
 *
 * 需要创建以下视图/布局文件后路由方可正常工作：
 *   src/layouts/AppLayout.vue
 *   src/views/LoginView.vue
 *   src/views/RegisterView.vue
 *   src/views/EditorView.vue
 *   src/views/TemplatesView.vue
 *   src/views/DocsView.vue
 *   src/views/UserView.vue
 *
 * @file src/router/index.ts
 * @author WaterRun
 * @date 2026-02-10
 */

import { createRouter, createWebHistory } from 'vue-router'
import type { RouteRecordRaw } from 'vue-router'
import { hasToken } from '@/utils/storage'

/** 路由表 */
const routes: RouteRecordRaw[] = [
  {
    path: '/login',
    name: 'login',
    component: () => import('@/views/LoginView.vue'),
    meta: { guest: true },
  },
  {
    path: '/register',
    name: 'register',
    component: () => import('@/views/RegisterView.vue'),
    meta: { guest: true },
  },
  {
    path: '/',
    component: () => import('@/layouts/AppLayout.vue'),
    meta: { requiresAuth: true },
    children: [
      {
        path: '',
        redirect: '/editor',
      },
      {
        path: 'editor',
        name: 'editor',
        component: () => import('@/views/EditorView.vue'),
      },
      {
        path: 'templates',
        name: 'templates',
        component: () => import('@/views/TemplatesView.vue'),
      },
      {
        path: 'docs',
        name: 'docs',
        component: () => import('@/views/DocsView.vue'),
      },
      {
        path: 'user',
        name: 'user',
        component: () => import('@/views/UserView.vue'),
      },
    ],
  },
]

const router = createRouter({
  history: createWebHistory(import.meta.env.BASE_URL),
  routes,
})

/**
 * 全局前置守卫
 *
 * 未认证用户访问 requiresAuth 路由 → 重定向至登录页；
 * 已认证用户访问 guest 路由 → 重定向至编辑器页。
 */
router.beforeEach((to) => {
  const authenticated = hasToken()

  if (to.meta.requiresAuth === true && !authenticated) {
    return { name: 'login' }
  }

  if (to.meta.guest === true && authenticated) {
    return { name: 'editor' }
  }
})

export default router