<script setup lang="ts">
/**
 * 登录页
 *
 * 提供用户名 / 密码表单，登录成功后跳转编辑器（或 redirect 参数指定的页面）。
 * 集成 GitHub 项目链接，采用 IDE / 开发者工具视觉风格。
 *
 * @file src/views/Login.vue
 * @author WaterRun
 * @date 2026-02-24
 * @updated 2026-02-24
 * @component Login
 */

import { ref } from 'vue'
import { useRoute, useRouter } from 'vue-router'

import { useAuthStore } from '@/stores/auth'
import { extractErrorMessage } from '@/utils/error'

const router = useRouter()
const route = useRoute()
const authStore = useAuthStore()

/** 用户名输入 */
const username = ref<string>('')

/** 密码输入 */
const password = ref<string>('')

/** 错误提示文本 */
const errorMessage = ref<string>('')

/** 请求进行中标志 */
const loading = ref<boolean>(false)

/** 是否以明文显示密码 */
const showPassword = ref<boolean>(false)

/**
 * 提交登录表单，成功后跳转目标页
 *
 * @returns Promise<void>
 * @throws 登录失败异常
 */
const handleLogin = async (): Promise<void> => {
  errorMessage.value = ''
  loading.value = true

  try {
    await authStore.login({
      username: username.value,
      password: password.value,
    })

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
    <div class="auth-page__grid" aria-hidden="true" />

    <main class="auth-card" aria-label="登录">
      <!-- 品牌头部 -->
      <header class="auth-header">
        <div class="auth-header__logo-ring">
          <img src="/ds4viz/logo.png" alt="DS4Viz 图标" class="auth-header__logo" />
        </div>
        <h1 class="auth-header__title">DS4Viz</h1>
        <p class="auth-header__tagline">通用的, 基础数据结构可视化.</p>
      </header>

      <!-- GitHub 项目入口 -->
      <a
        href="https://github.com/Water-Run/ds4viz"
        class="github-link"
        target="_blank"
        rel="noopener noreferrer"
        aria-label="在 GitHub 上查看 DS4Viz 源码"
      >
        <svg class="github-link__icon" viewBox="0 0 16 16" fill="currentColor" aria-hidden="true">
          <path d="M8 0C3.58 0 0 3.58 0 8c0 3.54 2.29 6.53 5.47 7.59.4.07.55-.17.55-.38 0-.19-.01-.82-.01-1.49-2.01.37-2.53-.49-2.69-.94-.09-.23-.48-.94-.82-1.13-.28-.15-.68-.52-.01-.53.63-.01 1.08.58 1.23.82.72 1.21 1.87.87 2.33.66.07-.52.28-.87.51-1.07-1.78-.2-3.64-.89-3.64-3.95 0-.87.31-1.59.82-2.15-.08-.2-.36-1.02.08-2.12 0 0 .67-.21 2.2.82.64-.18 1.32-.27 2-.27.68 0 1.36.09 2 .27 1.53-1.04 2.2-.82 2.2-.82.44 1.1.16 1.92.08 2.12.51.56.82 1.27.82 2.15 0 3.07-1.87 3.75-3.65 3.95.29.25.54.73.54 1.48 0 1.07-.01 1.93-.01 2.2 0 .21.15.46.55.38A8.013 8.013 0 0016 8c0-4.42-3.58-8-8-8z" />
        </svg>
        <span>在 GitHub 上查看</span>
      </a>

      <!-- 分隔线 -->
      <div class="auth-divider" role="separator" aria-hidden="true">
        <span class="auth-divider__text">账号登录</span>
      </div>

      <!-- 登录表单 -->
      <form class="auth-form" novalidate @submit.prevent="handleLogin">
        <!-- 用户名 -->
        <div class="form-field">
          <label class="form-field__label" for="login-username">用户名</label>
          <div class="form-field__control">
            <svg class="form-field__lead" viewBox="0 0 16 16" fill="currentColor" aria-hidden="true">
              <path d="M8 8a3 3 0 1 0 0-6 3 3 0 0 0 0 6zm-5 6s-1 0-1-1 1-4 6-4 6 3 6 4-1 1-1 1H3z" />
            </svg>
            <input
              id="login-username"
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
          <label class="form-field__label" for="login-password">密码</label>
          <div class="form-field__control">
            <svg class="form-field__lead" viewBox="0 0 16 16" fill="currentColor" aria-hidden="true">
              <path d="M8 1a2 2 0 0 1 2 2v4H6V3a2 2 0 0 1 2-2zm3 6V3a3 3 0 0 0-6 0v4a2 2 0 0 0-2 2v5a2 2 0 0 0 2 2h6a2 2 0 0 0 2-2V9a2 2 0 0 0-2-2z" />
            </svg>
            <input
              id="login-password"
              v-model="password"
              :type="showPassword ? 'text' : 'password'"
              class="form-field__input form-field__input--trail"
              autocomplete="current-password"
              :disabled="loading"
              placeholder="输入密码"
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
          :disabled="loading || !username.length || !password.length"
        >
          <span v-if="loading" class="auth-submit__spinner" aria-hidden="true" />
          <span>{{ loading ? '登录中…' : '登录' }}</span>
        </button>
      </form>

      <!-- 底部导航 -->
      <footer class="auth-footer">
        <span class="auth-footer__text">没有账号?</span>
        <router-link :to="{ name: 'register' }" class="auth-footer__link">
          注册
        </router-link>
      </footer>
    </main>
  </div>
</template>

<style scoped lang="scss">
// ── Local design tokens ───────────────────────────────────────────────────────
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

// ── Keyframes ────────────────────────────────────────────────────────────────
@keyframes card-rise {
  from { opacity: 0; transform: translateY(6px); }
  to { opacity: 1; transform: translateY(0); }
}

@keyframes spin {
  to { transform: rotate(360deg); }
}

// ── Page ──────────────────────────────────────────────────────────────────────
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

// ── Card ──────────────────────────────────────────────────────────────────────
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

// ── Header ────────────────────────────────────────────────────────────────────
.auth-header {
  grid-column: 1;
  display: flex;
  flex-direction: column;
  align-items: flex-start;
  gap: 12px;
  text-align: left;
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

// ── GitHub link ───────────────────────────────────────────────────────────────
.github-link {
  grid-column: 1;
  display: inline-flex;
  align-items: center;
  gap: 8px;
  height: 40px;
  padding: 0 14px;
  border: 1px solid $border-strong;
  border-radius: 10px;
  background-color: transparent;
  color: $text-body;
  font-size: 13px;
  font-weight: 500;
  text-decoration: none;
  transition:
    background-color 140ms $ease,
    border-color 140ms $ease,
    box-shadow 160ms $ease,
    color 140ms $ease,
    transform 120ms $ease;

  &:hover {
    background-color: rgba(0, 0, 0, 0.04);
    border-color: rgba(0, 0, 0, 0.22);
    color: $text-primary;
    box-shadow: 0 10px 26px rgba(0, 0, 0, 0.10);
  }

  &:active {
    background-color: rgba(0, 0, 0, 0.07);
    box-shadow: none;
    transform: translateY(1px);
  }

  &:focus-visible {
    outline: 2px solid $accent;
    outline-offset: 2px;
  }
}

.github-link__icon {
  width: 16px;
  height: 16px;
  flex-shrink: 0;
}

// ── Divider ────────────────────────────────────────────────────────────────
.auth-divider {
  grid-column: 2;
  display: flex;
  align-items: center;
  gap: 12px;
  margin-top: 4px;
}

.auth-divider::before,
.auth-divider::after {
  content: '';
  flex: 1;
  height: 1px;
  background-color: $border;
}

.auth-divider__text {
  font-size: 12px;
  font-weight: 600;
  color: $text-muted;
  letter-spacing: 0.06em;
  text-transform: uppercase;
  white-space: nowrap;
}

// ── Form ────────────────────────────────────────────────────────────────────
.auth-form {
  grid-column: 2;
  display: flex;
  flex-direction: column;
  gap: 16px;
}

// ── Form field ───────────────────────────────────────────────────────────────
.form-field {
  display: flex;
  flex-direction: column;
  gap: 8px;
}

.form-field__label {
  font-size: 12px;
  font-weight: 500;
  color: $text-body;
}

.form-field__control {
  position: relative;
  display: flex;
  align-items: center;
}

.form-field__control:focus-within .form-field__lead {
  color: $accent;
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

// ── Error alert ──────────────────────────────────────────────────────────────
.auth-alert {
  display: flex;
  align-items: flex-start;
  gap: 8px;
  padding: 12px;
  border-radius: 10px;
  background-color: rgba(239, 68, 68, 0.08);
  border: 1px solid rgba(239, 68, 68, 0.18);
}

.auth-alert__icon {
  width: 14px;
  height: 14px;
  flex-shrink: 0;
  margin-top: 2px;
  color: $error;
}

.auth-alert span {
  font-size: 12px;
  color: $error;
  line-height: 1.6;
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

// ── Submit button ─────────────────────────────────────────────────────────────
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

// ── Footer ────────────────────────────────────────────────────────────────────
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
