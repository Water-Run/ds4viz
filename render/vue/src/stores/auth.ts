/**
 * 认证状态管理
 *
 * 管理用户登录态、token 持久化及当前用户信息。
 *
 * @file src/stores/auth.ts
 * @author WaterRun
 * @date 2026-02-10
 */

import { ref, computed } from 'vue'
import { defineStore } from 'pinia'

import type { User, LoginRequest, RegisterRequest, ChangePasswordRequest } from '@/types/auth'
import { getToken, setToken, removeToken } from '@/utils/storage'
import { loginApi, registerApi, logoutApi, fetchMeApi } from '@/api/auth'

/**
 * 认证 Store
 *
 * @example
 * ```typescript
 * const auth = useAuthStore()
 * await auth.login({ username: 'admin', password: '123456' })
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
   * 登录并持久化 token
   *
   * @param request - 登录凭证
   * @throws {ApiError} 凭证错误或账号被封禁
   */
  const login = async (request: LoginRequest): Promise<void> => {
    const response = await loginApi(request)
    token.value = response.token
    setToken(response.token)
    currentUser.value = response.user
  }

  /**
   * 注册新账号（注册后不自动登录）
   *
   * @param request - 注册信息
   * @throws {ApiError} 用户名冲突或参数校验失败
   */
  const register = async (request: RegisterRequest): Promise<void> => {
    await registerApi(request)
  }

  /**
   * 登出，清除远程会话与本地状态
   *
   * 无论 API 调用是否成功均清除本地 token。
   */
  const logout = async (): Promise<void> => {
    try {
      if (token.value !== null) {
        await logoutApi()
      }
    } catch {
      /* 退出时忽略网络 / 令牌失效错误 */
    } finally {
      currentUser.value = null
      token.value = null
      removeToken()
    }
  }

  /**
   * 更新当前用户信息
   *
   * @param user - 新的用户信息
   */
  const setCurrentUser = (user: User | null): void => {
    currentUser.value = user
  }

  /**
   * 拉取当前用户信息（token 不存在时静默跳过）
   *
   * @throws {ApiError} token 无效时抛出
   */
  const fetchCurrentUser = async (): Promise<void> => {
    if (token.value === null) {
      return
    }
    currentUser.value = await fetchMeApi()
  }

  /**
   * 修改密码（待 users API 模块实现后接入）
   *
   * @param _request - 新旧密码
   * @throws {Error} 尚未实现
   */
  const changePassword = async (_request: ChangePasswordRequest): Promise<void> => {
    throw new Error('Not implemented — pending users API module')
  }

  /**
   * 手动更新 token 并同步至 sessionStorage
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
    setCurrentUser,
  }
})
