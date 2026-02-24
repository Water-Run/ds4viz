<script setup lang="ts">
/**
 * 注册页
 *
 * 提供用户名 / 密码 / 确认密码表单，注册成功后跳转登录页。
 * 实时检测两次密码是否一致，采用 IDE / 开发者工具视觉风格。
 *
 * @file src/views/Register.vue
 * @author WaterRun
 * @date 2026-02-24
 * @updated 2026-02-24
 * @component Register
 */

import { computed, ref } from 'vue'
import { useRouter } from 'vue-router'

import { useAuthStore } from '@/stores/auth'
import { extractErrorMessage } from '@/utils/error'

const router = useRouter()
const authStore = useAuthStore()

/** 用户名输入 */
const username = ref<string>('')

/** 密码输入 */
const password = ref<string>('')

/** 确认密码输入 */
const confirmPassword = ref<string>('')

/** 错误提示文本 */
const errorMessage = ref<string>('')

/** 请求进行中标志 */
const loading = ref<boolean>(false)

/** 是否以明文显示密码 */
const showPassword = ref<boolean>(false)

/** 是否以明文显示确认密码 */
const showConfirmPassword = ref<boolean>(false)

/**
 * 密码与确认密码是否一致（仅在确认密码非空时激活校验）
 *
 * @returns boolean
 */
const passwordsMatch = computed<boolean>(() =>
  confirmPassword.value.length > 0 && password.value === confirmPassword.value,
)

/**
 * 提交注册表单，成功后跳转登录页
 *
 * @returns Promise<void>
 * @throws 注册失败异常
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
    <div class="auth-page__grid" aria-hidden="true" />

    <main class="auth-card" aria-label="注册">
      <!-- 品牌头部 -->
      <header class="auth-header">
        <div class="auth-header__logo-ring">
          <img src="/ds4viz/logo.png" alt="DS4Viz 图标" class="auth-header__logo" />
        </div>
        <h1 class="auth-header__title">创建新账号</h1>
        <p class="auth-header__tagline">通用的, 基础数据结构可视化.</p>
      </header>

      <!-- 注册表单 -->
      <form class="auth-form" novalidate @submit.prevent="handleRegister">
        <!-- 用户名 -->
        <div class="form-field">
          <label class="form-field__label" for="reg-username">用户名</label>
          <div class="form-field__control">
            <svg class="form-field__lead" viewBox="0 0 16 16" fill="currentColor" aria-hidden="true">
              <path d="M8 8a3 3 0 1 0 0-6 3 3 0 0 0 0 6zm-5 6s-1 0-1-1 1-4 6-4 6 3 6 4-1 1-1 1H3z" />
            </svg>
            <input
              id="reg-username"
              v-model="username"
              type="text"
              class="form-field__input"
              autocomplete="username"
              autofocus
              :disabled="loading"
              placeholder="输入用户名"
            />
          </div>
        </div>

        <!-- 密码 -->
        <div class="form-field">
          <label class="form-field__label" for="reg-password">密码</label>
          <div class="form-field__control">
            <svg class="form-field__lead" viewBox="0 0 16 16" fill="currentColor" aria-hidden="true">
              <path d="M8 1a2 2 0 0 1 2 2v4H6V3a2 2 0 0 1 2-2zm3 6V3a3 3 0 0 0-6 0v4a2 2 0 0 0-2 2v5a2 2 0 0 0 2 2h6a2 2 0 0 0 2-2V9a2 2 0 0 0-2-2z" />
            </svg>
            <input
              id="reg-password"
              v-model="password"
              :type="showPassword ? 'text' : 'password'"
              class="form-field__input form-field__input--trail"
              autocomplete="new-password"
              :disabled="loading"
              placeholder="设置密码"
            />
            <button
              type="button"
              class="form-field__trail-btn"
              :aria-label="showPassword ? '隐藏密码' : '显示密码'"
              :aria-pressed="showPassword"
              @click="showPassword = !showPassword"
            >
              <svg v-if="!showPassword" viewBox="0 0 16 16" fill="currentColor" aria-hidden="true">
                <path d="M10.5 8a2.5 2.5 0 1 1-5 0 2.5 2.5 0 0 1 5 0z" />
                <path d="M0 8s3-5.5 8-5.5S16 8 16 8s-3 5.5-8 5.5S0 8 0 8zm8 3.5a3.5 3.5 0 1 0 0-7 3.5 3.5 0 0 0 0 7z" />
              </svg>
              <svg v-else viewBox="0 0 16 16" fill="currentColor" aria-hidden="true">
                <path d="M13.359 11.238C15.06 9.72 16 8 16 8s-3-5.5-8-5.5a7.028 7.028 0 0 0-2.79.588l.77.771A5.944 5.944 0 0 1 8 3.5c2.12 0 3.879 1.168 5.168 2.457A13.134 13.134 0 0 1 14.828 8c-.058.087-.122.183-.195.288-.335.48-.83 1.12-1.465 1.755-.165.165-.337.328-.513.486l.704.709z" />
                <path d="M11.297 9.176a3.5 3.5 0 0 0-4.474-4.474l.823.823a2.5 2.5 0 0 1 2.829 2.829l.822.822zm-2.943 1.299.822.822a3.5 3.5 0 0 1-4.474-4.474l.823.823a2.5 2.5 0 0 0 2.829 2.829z" />
                <path d="M3.35 5.47c-.18.16-.353.322-.518.487A13.134 13.134 0 0 0 1.172 8l.195.288c.335.48.83 1.12 1.465 1.755C4.121 11.332 5.881 12.5 8 12.5c.716 0 1.39-.133 2.02-.36l.77.772A7.029 7.029 0 0 1 8 13.5C3 13.5 0 8 0 8s.939-1.721 2.641-3.238l.708.709z" />
                <path fill-rule="evenodd" d="M13.646 14.354l-12-12 .708-.708 12 12-.708.708z" />
              </svg>
            </button>
          </div>
        </div>

        <!-- 确认密码 -->
        <div class="form-field">
          <label class="form-field__label" for="reg-confirm">
            确认密码
            <Transition name="t-match">
              <span
                v-if="confirmPassword.length > 0"
                class="match-badge"
                :class="passwordsMatch ? 'match-badge--ok' : 'match-badge--err'"
                aria-live="polite"
              >
                <svg v-if="passwordsMatch" viewBox="0 0 16 16" fill="currentColor" aria-hidden="true">
                  <path d="M13.854 3.646a.5.5 0 0 1 0 .708l-7 7a.5.5 0 0 1-.708 0l-3.5-3.5a.5.5 0 1 1 .708-.708L6.5 10.293l6.646-6.647a.5.5 0 0 1 .708 0z" />
                </svg>
                <svg v-else viewBox="0 0 16 16" fill="currentColor" aria-hidden="true">
                  <path d="M4.646 4.646a.5.5 0 0 1 .708 0L8 7.293l2.646-2.647a.5.5 0 0 1 .708.708L8.707 8l2.647 2.646a.5.5 0 0 1-.708.708L8 8.707l-2.646 2.647a.5.5 0 0 1-.708-.708L7.293 8 4.646 5.354a.5.5 0 0 1 0-.708z" />
                </svg>
                {{ passwordsMatch ? '已匹配' : '不匹配' }}
              </span>
            </Transition>
          </label>
          <div class="form-field__control">
            <svg class="form-field__lead" viewBox="0 0 16 16" fill="currentColor" aria-hidden="true">
              <path d="M8 1a2 2 0 0 1 2 2v4H6V3a2 2 0 0 1 2-2zm3 6V3a3 3 0 0 0-6 0v4a2 2 0 0 0-2 2v5a2 2 0 0 0 2 2h6a2 2 0 0 0 2-2V9a2 2 0 0 0-2-2z" />
            </svg>
            <input
              id="reg-confirm"
              v-model="confirmPassword"
              :type="showConfirmPassword ? 'text' : 'password'"
              class="form-field__input form-field__input--trail"
              :class="{
                'form-field__input--match-ok':  confirmPassword.length > 0 && passwordsMatch,
                'form-field__input--match-err': confirmPassword.length > 0 && !passwordsMatch,
              }"
              autocomplete="new-password"
              :disabled="loading"
              placeholder="再次输入密码"
            />
            <button
              type="button"
              class="form-field__trail-btn"
              :aria-label="showConfirmPassword ? '隐藏密码' : '显示密码'"
              :aria-pressed="showConfirmPassword"
              @click="showConfirmPassword = !showConfirmPassword"
            >
              <svg v-if="!showConfirmPassword" viewBox="0 0 16 16" fill="currentColor" aria-hidden="true">
                <path d="M10.5 8a2.5 2.5 0 1 1-5 0 2.5 2.5 0 0 1 5 0z" />
                <path d="M0 8s3-5.5 8-5.5S16 8 16 8s-3 5.5-8 5.5S0 8 0 8zm8 3.5a3.5 3.5 0 1 0 0-7 3.5 3.5 0 0 0 0 7z" />
              </svg>
              <svg v-else viewBox="0 0 16 16" fill="currentColor" aria-hidden="true">
                <path d="M13.359 11.238C15.06 9.72 16 8 16 8s-3-5.5-8-5.5a7.028 7.028 0 0 0-2.79.588l.77.771A5.944 5.944 0 0 1 8 3.5c2.12 0 3.879 1.168 5.168 2.457A13.134 13.134 0 0 1 14.828 8c-.058.087-.122.183-.195.288-.335.48-.83 1.12-1.465 1.755-.165.165-.337.328-.513.486l.704.709z" />
                <path d="M11.297 9.176a3.5 3.5 0 0 0-4.474-4.474l.823.823a2.5 2.5 0 0 1 2.829 2.829l.822.822zm-2.943 1.299.822.822a3.5 3.5 0 0 1-4.474-4.474l.823.823a2.5 2.5 0 0 0 2.829 2.829z" />
                <path d="M3.35 5.47c-.18.16-.353.322-.518.487A13.134 13.134 0 0 0 1.172 8l.195.288c.335.48.83 1.12 1.465 1.755C4.121 11.332 5.881 12.5 8 12.5c.716 0 1.39-.133 2.02-.36l.77.772A7.029 7.029 0 0 1 8 13.5C3 13.5 0 8 0 8s.939-1.721 2.641-3.238l.708.709z" />
                <path fill-rule="evenodd" d="M13.646 14.354l-12-12 .708-.708 12 12-.708.708z" />
              </svg>
            </button>
          </div>
        </div>

        <!-- 错误提示 -->
        <Transition name="t-error">
          <div v-if="errorMessage" class="auth-alert" role="alert" aria-live="assertive">
            <svg class="auth-alert__icon" viewBox="0 0 16 16" fill="currentColor" aria-hidden="true">
              <path d="M8.982 1.566a1.13 1.13 0 0 0-1.96 0L.165 13.233c-.457.778.091 1.767.98 1.767h13.713c.889 0 1.438-.99.98-1.767L8.982 1.566zM8 5c.535 0 .954.462.9.995l-.35 3.507a.552.552 0 0 1-1.1 0L7.1 5.995A.905.905 0 0 1 8 5zm.002 6a1 1 0 1 1 0 2 1 1 0 0 1 0-2z" />
            </svg>
            <span>{{ errorMessage }}</span>
          </div>
        </Transition>

        <!-- 提交 -->
        <button
          type="submit"
          class="auth-submit"
          :disabled="
            loading
              || !username.length
              || !password.length
              || !confirmPassword.length
          "
        >
          <span v-if="loading" class="auth-submit__spinner" aria-hidden="true" />
          <span>{{ loading ? '注册中…' : '注册' }}</span>
        </button>
      </form>

      <!-- 底部导航 -->
      <footer class="auth-footer">
        <span class="auth-footer__text">已有账号?</span>
        <router-link :to="{ name: 'login' }" class="auth-footer__link">
          登录
        </router-link>
      </footer>
    </main>
  </div>
</template>

<style scoped lang="scss">
// 与 Login.vue 保持一致的排版系统
$ease: cubic-bezier(0.2, 0, 0, 1);
$accent: #0078d4;
$accent-hover: #106ebe;
$accent-pressed: #005a9e;
$accent-wash: rgba(0, 120, 212, 0.08);
$border: rgba(0, 0, 0, 0.08);
$border-strong: rgba(0, 0, 0, 0.14);
$text-primary: #101010;
$text-body: #3f3f3f;
$text-muted: #737373;
$surface: #ffffff;
$surface-soft: #fbfbfc;
$base: #f4f5f6;
$error: #ef4444;
$success: #22c55e;

@keyframes card-rise {
  from { opacity: 0; transform: translateY(6px); }
  to { opacity: 1; transform: translateY(0); }
}

@keyframes spin {
  to { transform: rotate(360deg); }
}

.auth-page {
  min-height: 100dvh;
  padding: 48px 24px;
  background-color: $base;
  position: relative;
  overflow: hidden;
  display: flex;
  justify-content: center;
  align-items: flex-start;
  font-family: Inter, system-ui, -apple-system, 'Segoe UI', sans-serif;
}

.auth-page__grid {
  position: fixed;
  inset: 0;
  background-image: radial-gradient(circle, rgba(0, 0, 0, 0.04) 1px, transparent 1px);
  background-size: 24px 24px;
  pointer-events: none;
  user-select: none;
}

.auth-card {
  position: relative;
  z-index: 1;
  width: 100%;
  max-width: 960px;
  padding: 32px;
  background-color: $surface;
  border: 1px solid $border;
  border-radius: 14px;
  box-shadow: 0 1px 2px rgba(0, 0, 0, 0.04);
  display: grid;
  grid-template-columns: minmax(200px, 280px) minmax(360px, 1fr);
  column-gap: 40px;
  row-gap: 16px;

  @media (prefers-reduced-motion: no-preference) {
    animation: card-rise 240ms $ease both;
  }
}

@media (max-width: 900px) {
  .auth-card {
    display: flex;
    flex-direction: column;
    gap: 24px;
  }
}

.auth-header {
  grid-column: 1;
  display: flex;
  flex-direction: column;
  align-items: flex-start;
  gap: 12px;
}

.auth-header__logo-ring {
  width: 64px;
  height: 64px;
  border-radius: 14px;
  background-color: $surface-soft;
  border: 1px solid $border;
  display: grid;
  place-items: center;
}

.auth-header__logo {
  width: 34px;
  height: 34px;
  object-fit: contain;
}

.auth-header__title {
  font-size: 18px;
  font-weight: 600;
  letter-spacing: -0.01em;
  color: $text-primary;
  line-height: 1.2;
  margin: 0;
}

.auth-header__tagline {
  font-size: 13px;
  color: $text-body;
  line-height: 1.6;
  margin: 0;
}

.auth-form {
  grid-column: 2;
  display: flex;
  flex-direction: column;
  gap: 16px;
}

.form-field {
  display: flex;
  flex-direction: column;
  gap: 8px;
}

.form-field__label {
  font-size: 12px;
  font-weight: 500;
  color: $text-body;
  display: flex;
  align-items: center;
  gap: 8px;
}

.form-field__control {
  position: relative;
  display: flex;
  align-items: center;

  &:focus-within .form-field__lead {
    color: $accent;
  }
}

.form-field__lead {
  position: absolute;
  left: 12px;
  width: 16px;
  height: 16px;
  color: $text-muted;
  pointer-events: none;
  transition: color 140ms $ease;
}

.form-field__input {
  width: 100%;
  height: 40px;
  padding: 0 12px 0 40px;
  border: 1px solid $border-strong;
  border-radius: 10px;
  background-color: $surface;
  font-size: 13px;
  color: $text-primary;
  transition:
    border-color 140ms $ease,
    box-shadow 140ms $ease;

  &::placeholder {
    color: $text-muted;
    opacity: 0.7;
  }

  &:focus {
    outline: none;
    border-color: $accent;
    box-shadow: 0 0 0 3px $accent-wash;
  }

  &:disabled {
    opacity: 0.5;
    cursor: not-allowed;
    background-color: rgba(0, 0, 0, 0.02);
  }
}

.form-field__input--trail {
  padding-right: 44px;
}

.form-field__input--match-ok:not(:focus) {
  border-color: $success;
  box-shadow: 0 0 0 3px rgba(34, 197, 94, 0.12);
}

.form-field__input--match-err:not(:focus) {
  border-color: $error;
  box-shadow: 0 0 0 3px rgba(239, 68, 68, 0.10);
}

.form-field__trail-btn {
  position: absolute;
  right: 8px;
  width: 28px;
  height: 28px;
  border: none;
  background: none;
  color: $text-muted;
  border-radius: 8px;
  cursor: pointer;
  display: grid;
  place-items: center;
  transition:
    color 140ms $ease,
    background-color 140ms $ease;

  svg {
    width: 16px;
    height: 16px;
  }

  &:hover {
    color: $text-body;
    background-color: rgba(0, 0, 0, 0.05);
  }

  &:focus-visible {
    outline: 2px solid $accent;
    outline-offset: 1px;
  }
}

.match-badge {
  margin-left: auto;
  display: inline-flex;
  align-items: center;
  gap: 4px;
  font-size: 11px;
  font-weight: 500;

  &--ok  { color: $success; }
  &--err { color: $error; }
}

.t-match-enter-active,
.t-match-leave-active {
  transition: opacity 160ms $ease, transform 160ms $ease;
}

.t-match-enter-from,
.t-match-leave-to {
  opacity: 0;
  transform: translateY(-2px);
}

.auth-alert {
  display: flex;
  align-items: flex-start;
  gap: 8px;
  padding: 12px;
  border-radius: 10px;
  background-color: rgba(239, 68, 68, 0.08);
  border: 1px solid rgba(239, 68, 68, 0.18);

  span {
    font-size: 12px;
    color: $error;
    line-height: 1.6;
  }
}

.t-error-enter-active,
.t-error-leave-active {
  transition:
    opacity 160ms $ease,
    transform 160ms $ease;
}

.t-error-enter-from,
.t-error-leave-to {
  opacity: 0;
  transform: translateY(-4px);
}

.auth-submit {
  display: inline-flex;
  align-items: center;
  justify-content: center;
  gap: 8px;
  height: 40px;
  border: none;
  border-radius: 10px;
  background-color: $accent;
  color: #ffffff;
  font-size: 13px;
  font-weight: 600;
  cursor: pointer;
  transition:
    background-color 140ms $ease,
    box-shadow 160ms $ease,
    opacity 140ms $ease,
    transform 120ms $ease;

  &:hover:not(:disabled) {
    background-color: $accent-hover;
    box-shadow: 0 10px 26px rgba(0, 0, 0, 0.10);
    transform: translateY(-1px);
  }

  &:active:not(:disabled) {
    background-color: $accent-pressed;
    box-shadow: none;
    transform: translateY(0);
  }

  &:disabled {
    opacity: 0.45;
    cursor: not-allowed;
  }

  &:focus-visible {
    outline: 2px solid $accent;
    outline-offset: 3px;
  }
}

.auth-submit__spinner {
  width: 14px;
  height: 14px;
  border: 2px solid rgba(255, 255, 255, 0.35);
  border-top-color: #ffffff;
  border-radius: 50%;

  @media (prefers-reduced-motion: no-preference) {
    animation: spin 0.7s linear infinite;
  }
}

.auth-footer {
  grid-column: 2;
  display: flex;
  align-items: center;
  gap: 8px;
  justify-content: center;
}

.auth-footer__text {
  font-size: 12px;
  color: $text-muted;
}

.auth-footer__link {
  font-size: 12px;
  font-weight: 500;
  color: $accent;
  text-decoration: none;
  transition: color 140ms $ease;

  &:hover {
    color: $accent-hover;
    text-decoration: underline;
    text-underline-offset: 2px;
  }

  &:focus-visible {
    outline: 2px solid $accent;
    outline-offset: 2px;
    border-radius: 2px;
  }
}
</style>
