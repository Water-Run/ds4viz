import { createRouter, createWebHistory } from 'vue-router'
import type { RouteRecordRaw } from 'vue-router'
import { getToken } from '@/utils/storage'

/* -------------------------------------------------- *
 *  Route‑meta type augmentation                       *
 * -------------------------------------------------- */

declare module 'vue-router' {
  interface RouteMeta {
    requiresAuth?: boolean
    guest?: boolean
  }
}

/* -------------------------------------------------- *
 *  Route definitions                                  *
 * -------------------------------------------------- */

const routes: RouteRecordRaw[] = [
  /* ---- Guest pages ---- */
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

  /* ---- Authenticated shell ---- */
  {
    path: '/',
    component: () => import('@/layouts/AppLayout.vue'),
    meta: { requiresAuth: true },
    children: [
      {
        path: '',
        name: 'home',
        redirect: { name: 'playground' },
      },
      {
        path: 'playground',
        name: 'playground',
        component: () => import('@/views/Playground.vue'),
      },
      {
        path: 'templates',
        name: 'templates',
        component: () => import('@/views/Templates.vue'),
      },
      {
        path: 'templates/:id',
        name: 'template-detail',
        component: () => import('@/views/TemplateDetail.vue'),
      },
      {
        path: 'profile',
        name: 'profile',
        component: () => import('@/views/Profile.vue'),
      },
      {
        path: 'favorites',
        name: 'favorites',
        component: () => import('@/views/Favorites.vue'),
      },
      {
        path: 'executions',
        name: 'executions',
        component: () => import('@/views/Executions.vue'),
      },
      {
        path: 'docs',
        name: 'docs',
        component: () => import('@/views/Docs.vue'),
      },
      {
        path: 'about',
        name: 'about',
        component: () => import('@/views/About.vue'),
      },
    ],
  },

  /* ---- 404 ---- */
  {
    path: '/:pathMatch(.*)*',
    name: 'not-found',
    component: () => import('@/views/NotFound.vue'),
  },
]

/* -------------------------------------------------- *
 *  Router instance                                    *
 * -------------------------------------------------- */

const router = createRouter({
  history: createWebHistory(),
  routes,
})

/* -------------------------------------------------- *
 *  Navigation guards                                  *
 * -------------------------------------------------- */

router.beforeEach((to) => {
  const token = getToken()

  const requiresAuth = to.matched.some((r) => r.meta.requiresAuth)
  const isGuest = to.matched.some((r) => r.meta.guest)

  if (requiresAuth && !token) {
    return { name: 'login', query: { redirect: to.fullPath } }
  }

  if (isGuest && token) {
    return { name: 'playground' }
  }
})

export default router
