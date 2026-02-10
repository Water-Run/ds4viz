/**
 * 全局类型声明与模块增强
 *
 * @file env.d.ts
 * @author WaterRun
 * @date 2026-02-10
 */

/// <reference types="vite/client" />

declare module 'vue-router' {
    /**
     * 路由元信息类型增强
     *
     * @interface
     */
    interface RouteMeta {
        /** 需要登录才能访问 */
        requiresAuth?: boolean
        /** 仅未登录时可访问 */
        guest?: boolean
    }
}