/**
 * 认证相关类型定义
 *
 * @file src/types/auth.ts
 * @author WaterRun
 * @date 2026-02-10
 */

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
    /** 邮箱 */
    email: string
    /** 头像 URL */
    avatar: string
    /** 创建时间（ISO 8601） */
    createdAt: string
    /** 更新时间（ISO 8601） */
    updatedAt: string
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
}

/**
 * 注册请求
 *
 * @interface
 */
export interface RegisterRequest {
    /** 用户名 */
    username: string
    /** 邮箱 */
    email: string
    /** 密码 */
    password: string
}

/**
 * 注册响应
 *
 * @interface
 */
export interface RegisterResponse {
    /** 新创建的用户信息 */
    user: User
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