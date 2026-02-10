/**
 * 认证相关类型定义
 *
 * 与后端 /api/auth/* 接口一一对应，字段名使用 camelCase，
 * API 层负责 snake_case ↔ camelCase 的双向映射。
 *
 * @file src/types/auth.ts
 * @author WaterRun
 * @date 2026-02-10
 */

/**
 * 用户账号状态
 *
 * @typedef
 */
export type UserStatus = 'Active' | 'Suspended' | 'Banned'

/**
 * 用户信息
 *
 * @interface
 */
export interface User {
    /** 用户 ID */
    id: number
    /** 用户名 */
    username: string
    /** 头像 URL（未设置时为 null） */
    avatarUrl: string | null
    /** 账号状态 */
    status: UserStatus
    /** 创建时间（ISO 8601） */
    createdAt: string
}

/**
 * 登录请求
 *
 * @interface
 */
export interface LoginRequest {
    /** 用户名 */
    username: string
    /** 密码 */
    password: string
}

/**
 * 登录响应
 *
 * @interface
 */
export interface LoginResponse {
    /** JWT token */
    token: string
    /** 用户信息 */
    user: User
    /** token 过期时间（ISO 8601） */
    expiresAt: string
}

/**
 * 注册请求
 *
 * @interface
 */
export interface RegisterRequest {
    /** 用户名（3–32 字符） */
    username: string
    /** 密码（1–64 字符，UTF-8 字节不超过 64） */
    password: string
}

/**
 * 修改密码请求
 *
 * @interface
 */
export interface ChangePasswordRequest {
    /** 原密码 */
    oldPassword: string
    /** 新密码 */
    newPassword: string
}