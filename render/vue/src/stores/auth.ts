/**
 * 认证状态管理
 *
 * 管理用户登录态、token 持久化及当前用户信息。
 * API 调用方法预留接口，待 HTTP 客户端实现后接入。
 *
 * @file src/stores/auth.ts
 * @author WaterRun
 * @date 2026-02-10
 */

import { ref, computed } from 'vue'
import { defineStore } from 'pinia'

import type { User, LoginRequest, RegisterRequest, ChangePasswordRequest } from '@/types/auth'
import { getToken, setToken, removeToken } from '@/utils/storage'

/**
 * 认证 Store
 *
 * @example
 * ```typescript
 * const authStore = useAuthStore()
 * await authStore.login({ username: 'admin', password: '123456' })
 * ```
 */
export const useAuthStore = defineStore('auth', () => {
  /* ---- State ---- */

  /** 当前登录用户 */
  const currentUser = ref<User | null>(null)

  /** JWT token，初始化时从 sessionStorage 恢复 */
  const token = ref<string | null>(getToken())

  /* ---- Getters ---- */

  /**
   * 是否已认证（持有 token 即视为已认证）
   */
  const isAuthenticated = computed<boolean>(() => token.value !== null)

  /* ---- Actions ---- */

  /**
   * 登录
   *
   * @param _request - 登录凭证
   * @throws {Error} API 未实现或请求失败时抛出
   */
  const login = async (_request: LoginRequest): Promise<void> => {
    // TODO: 接入 HTTP 客户端后替换
    // const response = await api.post<LoginResponse>('/auth/login', _request)
    // token.value = response.token
    // setToken(response.token)
    // currentUser.value = response.user
    throw new Error('API client not implemented')
  }

  /**
   * 注册
   *
   * @param _request - 注册信息
   * @throws {Error} API 未实现或请求失败时抛出
   */
  const register = async (_request: RegisterRequest): Promise<void> => {
    // TODO: 接入 HTTP 客户端后替换
    throw new Error('API client not implemented')
  }

  /**
   * 登出，清除 token 与用户信息
   */
  const logout = (): void => {
    currentUser.value = null
    token.value = null
    removeToken()
  }

  /**
   * 拉取当前用户信息
   *
   * @throws {Error} API 未实现或 token 无效时抛出
   */
  const fetchCurrentUser = async (): Promise<void> => {
    if (token.value === null) {
      return
    }
    // TODO: 接入 HTTP 客户端后替换
    // const user = await api.get<User>('/auth/me')
    // currentUser.value = user
    throw new Error('API client not implemented')
  }

  /**
   * 修改密码
   *
   * @param _request - 新旧密码
   * @throws {Error} API 未实现或请求失败时抛出
   */
  const changePassword = async (_request: ChangePasswordRequest): Promise<void> => {
    // TODO: 接入 HTTP 客户端后替换
    throw new Error('API client not implemented')
  }

  /**
   * 更新内存中的 token 并同步至 sessionStorage
   *
   * @param newToken - 新 token
   */
  const updateToken = (newToken: string): void => {
    token.value = newToken
    setToken(newToken)
  }

  return {
    currentUser,
    token,
    isAuthenticated,
    login,
    register,
    logout,
    fetchCurrentUser,
    changePassword,
    updateToken,
  }
})