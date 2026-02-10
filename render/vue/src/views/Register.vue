<script setup lang="ts">
/**
 * 注册页
 *
 * 提供用户名 / 密码 / 确认密码表单，注册成功后跳转登录页。
 *
 * @component Register
 */

import { ref } from 'vue'
import { useRouter } from 'vue-router'
import { useAuthStore } from '@/stores/auth'
import { extractErrorMessage } from '@/utils/error'

const router = useRouter()
const authStore = useAuthStore()

/** 用户名 */
const username = ref<string>('')

/** 密码 */
const password = ref<string>('')

/** 确认密码 */
const confirmPassword = ref<string>('')

/** 错误提示 */
const errorMessage = ref<string>('')

/** 请求中 */
const loading = ref<boolean>(false)

/**
 * 提交注册表单
 */
const handleRegister = async (): Promise<void> => {
  errorMessage.value = ''

  if (password.value !== confirmPassword.value) {
    errorMessage.value = '两次输入的密码不一致'
    return
  }

  loading.value = true
  try {
    await authStore.register({
      username: username.value,
      password: password.value,
    })
    await router.push({ name: 'login' })
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
        <p class="auth-card__subtitle">创建账号</p>
      </div>

      <!-- 表单 -->
      <form class="auth-card__form" @submit.prevent="handleRegister">
        <div class="form-field">
          <label class="form-field__label" for="reg-username">
            用户名
          </label>
          <input
            id="reg-username"
            v-model="username"
            type="text"
            class="form-field__input"
            autocomplete="username"
            autofocus
            :disabled="loading"
          />
        </div>

        <div class="form-field">
          <label class="form-field__label" for="reg-password">
            密码
          </label>
          <input
            id="reg-password"
            v-model="password"
            type="password"
            class="form-field__input"
            autocomplete="new-password"
            :disabled="loading"
          />
        </div>

        <div class="form-field">
          <label class="form-field__label" for="reg-confirm">
            确认密码
          </label>
          <input
            id="reg-confirm"
            v-model="confirmPassword"
            type="password"
            class="form-field__input"
            autocomplete="new-password"
            :disabled="loading"
          />
        </div>

        <div v-if="errorMessage" class="auth-card__error">
          {{ errorMessage }}
        </div>

        <button
          type="submit"
          class="auth-card__action"
          :disabled="
            loading
              || username.length === 0
              || password.length === 0
              || confirmPassword.length === 0
          "
        >
          {{ loading ? '注册中…' : '注册' }}
        </button>
      </form>

      <!-- 底部链接 -->
      <div class="auth-card__footer">
        <span class="auth-card__footer-text">已有账号?</span>
        <router-link :to="{ name: 'login' }" class="auth-card__footer-link">
          登录
        </router-link>
      </div>
    </div>
  </div>
</template>

<style scoped>
/* ============================================================
 *  注册页样式（与登录页共用 auth-card / form-field 规则）
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