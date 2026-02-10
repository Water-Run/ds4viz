<script setup lang="ts">
/**
 * 登录页
 *
 * 提供用户名 / 密码表单，登录成功后跳转编辑器（或 redirect 参数指定的页面）。
 *
 * @component Login
 */

import { ref } from 'vue'
import { useRouter, useRoute } from 'vue-router'
import { useAuthStore } from '@/stores/auth'
import { extractErrorMessage } from '@/utils/error'

const router = useRouter()
const route = useRoute()
const authStore = useAuthStore()

/** 用户名 */
const username = ref<string>('')

/** 密码 */
const password = ref<string>('')

/** 错误提示 */
const errorMessage = ref<string>('')

/** 请求中 */
const loading = ref<boolean>(false)

/**
 * 提交登录表单
 */
const handleLogin = async (): Promise<void> => {
  errorMessage.value = ''
  loading.value = true
  try {
    await authStore.login({
      username: username.value,
      password: password.value,
    })

    /* 优先跳转守卫保存的 redirect，否则去 playground */
    const redirect = typeof route.query.redirect === 'string'
      ? route.query.redirect
      : '/playground'
    await router.push(redirect)
  } catch (error: unknown) {
    errorMessage.value = extractErrorMessage(error)
  } finally {
    loading.value = false
  }
}
</script>

<template>
  <div class="auth-page">
    <div class="auth-card">
      <!-- 标识 -->
      <div class="auth-card__header">
        <img
          src="/ds4viz/logo.png"
          alt="DS4Viz"
          class="auth-card__logo"
        />
        <h1 class="auth-card__title">DS4Viz</h1>
        <p class="auth-card__subtitle">数据结构可视化平台</p>
      </div>

      <!-- 表单 -->
      <form class="auth-card__form" @submit.prevent="handleLogin">
        <div class="form-field">
          <label class="form-field__label" for="login-username">
            用户名
          </label>
          <input
            id="login-username"
            v-model="username"
            type="text"
            class="form-field__input"
            autocomplete="username"
            autofocus
            :disabled="loading"
          />
        </div>

        <div class="form-field">
          <label class="form-field__label" for="login-password">
            密码
          </label>
          <input
            id="login-password"
            v-model="password"
            type="password"
            class="form-field__input"
            autocomplete="current-password"
            :disabled="loading"
          />
        </div>

        <div v-if="errorMessage" class="auth-card__error">
          {{ errorMessage }}
        </div>

        <button
          type="submit"
          class="auth-card__action"
          :disabled="loading || username.length === 0 || password.length === 0"
        >
          {{ loading ? '登录中…' : '登录' }}
        </button>
      </form>

      <!-- 底部链接 -->
      <div class="auth-card__footer">
        <span class="auth-card__footer-text">没有账号?</span>
        <router-link :to="{ name: 'register' }" class="auth-card__footer-link">
          注册
        </router-link>
      </div>
    </div>
  </div>
</template>

<style scoped>
/* ============================================================
 *  登录页样式
 * ============================================================ */

.auth-page {
  display: flex;
  align-items: center;
  justify-content: center;
  min-height: 100dvh;
  padding: var(--space-2);
  background-color: var(--color-bg-base);
}

.auth-card {
  width: 100%;
  max-width: 360px;
  padding: var(--space-4);
  border: 1px solid var(--color-border);
  border-radius: var(--radius-lg);
  background-color: var(--color-bg-surface);
  box-shadow: var(--shadow-static);
}

.auth-card__header {
  display: flex;
  flex-direction: column;
  align-items: center;
  gap: var(--space-1);
  margin-bottom: var(--space-4);
}

.auth-card__logo {
  width: 48px;
  height: 48px;
}

.auth-card__title {
  font-size: var(--text-lg);
  font-weight: var(--weight-semibold);
  color: var(--color-text-primary);
  line-height: var(--leading-tight);
}

.auth-card__subtitle {
  font-size: var(--text-xs);
  color: var(--color-text-tertiary);
  letter-spacing: var(--tracking-panel-head);
}

.auth-card__form {
  display: flex;
  flex-direction: column;
  gap: var(--space-2);
}

.auth-card__error {
  padding: var(--space-1);
  border-radius: var(--radius-sm);
  background-color: var(--color-error-muted);
  color: var(--color-error);
  font-size: var(--text-xs);
  line-height: var(--leading-normal);
}

.auth-card__action {
  height: var(--control-height-md);
  border-radius: var(--radius-control);
  background-color: var(--color-accent);
  color: var(--color-accent-contrast);
  font-size: var(--text-sm);
  font-weight: var(--weight-medium);
  transition: background-color var(--duration-fast) var(--ease);
}

.auth-card__action:hover:not(:disabled) {
  background-color: var(--color-accent-hover);
}

.auth-card__action:active:not(:disabled) {
  background-color: var(--color-accent-pressed);
}

.auth-card__footer {
  display: flex;
  align-items: center;
  justify-content: center;
  gap: var(--space-1);
  margin-top: var(--space-3);
}

.auth-card__footer-text {
  font-size: var(--text-xs);
  color: var(--color-text-tertiary);
}

.auth-card__footer-link {
  font-size: var(--text-xs);
  font-weight: var(--weight-medium);
}

/* ---- 表单字段 ---- */

.form-field {
  display: flex;
  flex-direction: column;
  gap: var(--space-1);
}

.form-field__label {
  font-size: var(--text-xs);
  font-weight: var(--weight-medium);
  color: var(--color-text-body);
}

.form-field__input {
  height: var(--control-height-md);
  padding: 0 var(--space-2);
  border: 1px solid var(--color-border-strong);
  border-radius: var(--radius-control);
  background-color: var(--color-bg-surface);
  font-size: var(--text-sm);
  color: var(--color-text-primary);
  transition: border-color var(--duration-fast) var(--ease);
}

.form-field__input:focus {
  border-color: var(--color-accent);
}

.form-field__input:disabled {
  opacity: 0.5;
  cursor: not-allowed;
}
</style>