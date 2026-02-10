/**
 * 路由配置
 *
 * 管理所有页面路由及导航守卫（认证拦截）。
 *
 * @file src/router/index.ts
 * @author WaterRun
 * @date 2026-02-10
 */

import { createRouter, createWebHistory, type RouteRecordRaw } from 'vue-router'
import { useAuthStore } from '@/stores/auth'

/**
 * 路由表
 *
 * meta.requiresAuth — 需要登录才能访问
 * meta.guest        — 仅未登录时可访问（已登录则重定向到编辑器）
 */
const routes: RouteRecordRaw[] = [
  /* ---------- 认证页 ---------- */
  {
    path: '/login',
    name: 'login',
    component: () => import('@/views/Login.vue'),
    meta: { guest: true },
  },
  {
    path: '/register',
    name: 'register',
    component: () => import('@/views/Register.vue'),
    meta: { guest: true },
  },

  /* ---------- 主功能页 ---------- */
  {
    path: '/',
    name: 'home',
    component: () => import('@/views/Home.vue'),
  },
  {
    path: '/playground',
    name: 'playground',
    component: () => import('@/views/Playground.vue'),
    meta: { requiresAuth: true },
  },
  {
    path: '/profile',
    name: 'profile',
    component: () => import('@/views/Profile.vue'),
    meta: { requiresAuth: true },
  },
]

/**
 * 路由实例
 */
const router = createRouter({
  history: createWebHistory(import.meta.env.BASE_URL),
  routes,
})

/**
 * 全局前置守卫 — 认证拦截
 *
 * 1. requiresAuth 页面未登录 → 重定向到 /login
 * 2. guest 页面已登录 → 重定向到 /playground
 */
router.beforeEach((to) => {
  const auth = useAuthStore()

  if (to.meta.requiresAuth === true && !auth.isAuthenticated) {
    return { name: 'login', query: { redirect: to.fullPath } }
  }

  if (to.meta.guest === true && auth.isAuthenticated) {
    return { name: 'playground' }
  }
})

export default router