/**
 * 认证相关 API
 *
 * 对接后端 /api/auth/* 端点，负责 snake_case ↔ camelCase 映射。
 *
 * @file src/api/auth.ts
 * @author WaterRun
 * @date 2026-02-10
 */

import { get, post } from './http'
import type {
  User,
  UserStatus,
  LoginRequest,
  LoginResponse,
  RegisterRequest,
} from '@/types/auth'

/* ----------------------------------------------------------------
 *  后端 JSON 原始结构（snake_case）
 * ---------------------------------------------------------------- */

/**
 * 后端用户 JSON
 *
 * @interface
 */
interface RawUser {
  /** 用户 ID */
  id: number
  /** 用户名 */
  username: string
  /** 头像路径（可为 null） */
  avatar_url: string | null
  /** 账号状态 */
  status: UserStatus
  /** 创建时间 */
  created_at: string
}

/**
 * 后端登录响应 JSON
 *
 * @interface
 */
interface RawLoginResponse {
  /** JWT */
  token: string
  /** 用户信息 */
  user: RawUser
  /** 过期时间 */
  expires_at: string
}

/* ----------------------------------------------------------------
 *  映射函数
 * ---------------------------------------------------------------- */

/**
 * 将后端用户对象映射为前端 User
 *
 * @param raw - 后端原始 JSON
 * @returns 前端 User 对象
 */
function mapUser(raw: RawUser): User {
  return {
    id: raw.id,
    username: raw.username,
    avatarUrl: raw.avatar_url,
    status: raw.status,
    createdAt: raw.created_at,
  }
}

/**
 * 将后端登录响应映射为前端 LoginResponse
 *
 * @param raw - 后端原始 JSON
 * @returns 前端 LoginResponse 对象
 */
function mapLoginResponse(raw: RawLoginResponse): LoginResponse {
  return {
    token: raw.token,
    user: mapUser(raw.user),
    expiresAt: raw.expires_at,
  }
}

/* ----------------------------------------------------------------
 *  API 函数
 * ---------------------------------------------------------------- */

/**
 * 登录
 *
 * @param request - 用户名与密码
 * @returns 令牌及用户信息
 * @throws {ApiError} 401 凭证错误 / 403 账号被封禁
 */
export async function loginApi(request: LoginRequest): Promise<LoginResponse> {
  const raw = await post<RawLoginResponse>('/auth/login', request)
  return mapLoginResponse(raw)
}

/**
 * 注册
 *
 * @param request - 用户名与密码
 * @returns 新创建的用户信息
 * @throws {ApiError} 409 用户名已存在 / 422 参数校验失败
 */
export async function registerApi(request: RegisterRequest): Promise<User> {
  const raw = await post<RawUser>('/auth/register', request)
  return mapUser(raw)
}

/**
 * 登出，销毁当前会话
 *
 * @throws {ApiError} 401 令牌无效
 */
export async function logoutApi(): Promise<void> {
  await post<{ message: string }>('/auth/logout')
}

/**
 * 获取当前登录用户信息
 *
 * @returns 当前用户
 * @throws {ApiError} 401 令牌无效或过期
 */
export async function fetchMeApi(): Promise<User> {
  const raw = await get<RawUser>('/auth/me')
  return mapUser(raw)
}
