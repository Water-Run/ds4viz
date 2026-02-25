<script setup lang="ts">
/**
 * 注册页
 *
 * 单页注册：用户名 + 密码 + 确认密码同屏，渐进入场动画。
 *
 * @file src/views/Register.vue
 * @author WaterRun
 * @date 2026-02-25
 * @component Register
 */

import { computed, ref } from 'vue'
import { useRouter } from 'vue-router'

import { useAuthStore } from '@/stores/auth'
import { extractErrorMessage } from '@/utils/error'

const router = useRouter()
const authStore = useAuthStore()

const username = ref<string>('')
const password = ref<string>('')
const confirmPassword = ref<string>('')
const errorMessage = ref<string>('')
const loading = ref<boolean>(false)
const showPassword = ref<boolean>(false)
const showConfirmPassword = ref<boolean>(false)

const passwordsMatch = computed<boolean>(
  () => confirmPassword.value.length > 0 && password.value === confirmPassword.value,
)

const handleRegister = async (): Promise<void> => {
  errorMessage.value = ''
  if (!username.value.trim()) {
    errorMessage.value = '请输入用户名'
    return
  }
  if (!password.value) {
    errorMessage.value = '请输入密码'
    return
  }
  if (password.value !== confirmPassword.value) {
    errorMessage.value = '两次输入的密码不一致'
    return
  }
  loading.value = true
  try {
    await authStore.register({ username: username.value, password: password.value })
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
    <div class="auth-page__dots" aria-hidden="true" />

    <main class="auth-card">
      <!-- ── 左列：品牌 ── -->
      <header class="auth-brand">
        <div class="auth-brand__top">
          <img
            src="/ds4viz/logo.png"
            alt="DS4Viz"
            class="auth-brand__logo entrance"
            :style="{ '--d': '100ms' }"
          />
          <h2 class="auth-brand__name entrance" :style="{ '--d': '200ms' }">ds4viz</h2>
          <p class="auth-brand__slogan entrance" :style="{ '--d': '300ms' }">
            通用的, 基础数据结构可视化.
          </p>
        </div>

        <a
          href="https://github.com/Water-Run/ds4viz"
          class="github-link entrance"
          :style="{ '--d': '400ms' }"
          target="_blank"
          rel="noopener noreferrer"
        >
          <svg class="github-link__icon" viewBox="0 0 16 16" fill="currentColor">
            <path
              d="M8 0C3.58 0 0 3.58 0 8c0 3.54 2.29 6.53 5.47 7.59.4.07.55-.17.55-.38 0-.19-.01-.82-.01-1.49-2.01.37-2.53-.49-2.69-.94-.09-.23-.48-.94-.82-1.13-.28-.15-.68-.52-.01-.53.63-.01 1.08.58 1.23.82.72 1.21 1.87.87 2.33.66.07-.52.28-.87.51-1.07-1.78-.2-3.64-.89-3.64-3.95 0-.87.31-1.59.82-2.15-.08-.2-.36-1.02.08-2.12 0 0 .67-.21 2.2.82.64-.18 1.32-.27 2-.27.68 0 1.36.09 2 .27 1.53-1.04 2.2-.82 2.2-.82.44 1.1.16 1.92.08 2.12.51.56.82 1.27.82 2.15 0 3.07-1.87 3.75-3.65 3.95.29.25.54.73.54 1.48 0 1.07-.01 1.93-.01 2.2 0 .21.15.46.55.38A8.013 8.013 0 0016 8c0-4.42-3.58-8-8-8z"
            />
          </svg>
          <span>在 GitHub 上查看</span>
        </a>
      </header>

      <!-- ── 右列：表单 ── -->
      <section class="auth-body entrance" :style="{ '--d': '250ms' }">
        <form class="auth-form" novalidate @submit.prevent="handleRegister">
          <div class="auth-form__header">
            <h1 class="auth-form__title">创建账号</h1>
          </div>

          <div class="auth-form__fields">
            <!-- 用户名 -->
            <div class="form-field">
              <label class="form-field__label" for="reg-username">用户名</label>
              <div class="form-field__control">
                <svg class="form-field__icon" viewBox="0 0 16 16" fill="currentColor">
                  <path
                    d="M8 8a3 3 0 1 0 0-6 3 3 0 0 0 0 6zm-5 6s-1 0-1-1 1-4 6-4 6 3 6 4-1 1-1 1H3z"
                  />
                </svg>
                <input
                  id="reg-username"
                  v-model="username"
                  type="text"
                  class="form-field__input"
                  autocomplete="username"
                  autofocus
                  placeholder="输入用户名"
                  :disabled="loading"
                />
              </div>
            </div>

            <!-- 密码 -->
            <div class="form-field">
              <label class="form-field__label" for="reg-password">密码</label>
              <div class="form-field__control">
                <svg class="form-field__icon" viewBox="0 0 16 16" fill="currentColor">
                  <path
                    d="M8 1a2 2 0 0 1 2 2v4H6V3a2 2 0 0 1 2-2zm3 6V3a3 3 0 0 0-6 0v4a2 2 0 0 0-2 2v5a2 2 0 0 0 2 2h6a2 2 0 0 0 2-2V9a2 2 0 0 0-2-2z"
                  />
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
                  class="form-field__trail"
                  tabindex="-1"
                  @click="showPassword = !showPassword"
                >
                  <svg v-if="!showPassword" viewBox="0 0 16 16" fill="currentColor">
                    <path d="M10.5 8a2.5 2.5 0 1 1-5 0 2.5 2.5 0 0 1 5 0z" />
                    <path
                      d="M0 8s3-5.5 8-5.5S16 8 16 8s-3 5.5-8 5.5S0 8 0 8zm8 3.5a3.5 3.5 0 1 0 0-7 3.5 3.5 0 0 0 0 7z"
                    />
                  </svg>
                  <svg v-else viewBox="0 0 16 16" fill="currentColor">
                    <path
                      d="M13.359 11.238C15.06 9.72 16 8 16 8s-3-5.5-8-5.5a7.028 7.028 0 0 0-2.79.588l.77.771A5.944 5.944 0 0 1 8 3.5c2.12 0 3.879 1.168 5.168 2.457A13.134 13.134 0 0 1 14.828 8c-.058.087-.122.183-.195.288-.335.48-.83 1.12-1.465 1.755-.165.165-.337.328-.513.486l.704.709z"
                    />
                    <path
                      d="M11.297 9.176a3.5 3.5 0 0 0-4.474-4.474l.823.823a2.5 2.5 0 0 1 2.829 2.829l.822.822zm-2.943 1.299.822.822a3.5 3.5 0 0 1-4.474-4.474l.823.823a2.5 2.5 0 0 0 2.829 2.829z"
                    />
                    <path
                      d="M3.35 5.47c-.18.16-.353.322-.518.487A13.134 13.134 0 0 0 1.172 8l.195.288c.335.48.83 1.12 1.465 1.755C4.121 11.332 5.881 12.5 8 12.5c.716 0 1.39-.133 2.02-.36l.77.772A7.029 7.029 0 0 1 8 13.5C3 13.5 0 8 0 8s.939-1.721 2.641-3.238l.708.709z"
                    />
                    <path
                      fill-rule="evenodd"
                      d="M13.646 14.354l-12-12 .708-.708 12 12-.708.708z"
                    />
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
                  >
                    <svg v-if="passwordsMatch" viewBox="0 0 16 16" fill="currentColor">
                      <path
                        d="M13.854 3.646a.5.5 0 0 1 0 .708l-7 7a.5.5 0 0 1-.708 0l-3.5-3.5a.5.5 0 1 1 .708-.708L6.5 10.293l6.646-6.647a.5.5 0 0 1 .708 0z"
                      />
                    </svg>
                    <svg v-else viewBox="0 0 16 16" fill="currentColor">
                      <path
                        d="M4.646 4.646a.5.5 0 0 1 .708 0L8 7.293l2.646-2.647a.5.5 0 0 1 .708.708L8.707 8l2.647 2.646a.5.5 0 0 1-.708.708L8 8.707l-2.646 2.647a.5.5 0 0 1-.708-.708L7.293 8 4.646 5.354a.5.5 0 0 1 0-.708z"
                      />
                    </svg>
                    {{ passwordsMatch ? '已匹配' : '不匹配' }}
                  </span>
                </Transition>
              </label>
              <div class="form-field__control">
                <svg class="form-field__icon" viewBox="0 0 16 16" fill="currentColor">
                  <path
                    d="M8 1a2 2 0 0 1 2 2v4H6V3a2 2 0 0 1 2-2zm3 6V3a3 3 0 0 0-6 0v4a2 2 0 0 0-2 2v5a2 2 0 0 0 2 2h6a2 2 0 0 0 2-2V9a2 2 0 0 0-2-2z"
                  />
                </svg>
                <input
                  id="reg-confirm"
                  v-model="confirmPassword"
                  :type="showConfirmPassword ? 'text' : 'password'"
                  class="form-field__input form-field__input--trail"
                  :class="{
                    'form-field__input--ok': confirmPassword.length > 0 && passwordsMatch,
                    'form-field__input--err': confirmPassword.length > 0 && !passwordsMatch,
                  }"
                  autocomplete="new-password"
                  :disabled="loading"
                  placeholder="再次输入密码"
                />
                <button
                  type="button"
                  class="form-field__trail"
                  tabindex="-1"
                  @click="showConfirmPassword = !showConfirmPassword"
                >
                  <svg v-if="!showConfirmPassword" viewBox="0 0 16 16" fill="currentColor">
                    <path d="M10.5 8a2.5 2.5 0 1 1-5 0 2.5 2.5 0 0 1 5 0z" />
                    <path
                      d="M0 8s3-5.5 8-5.5S16 8 16 8s-3 5.5-8 5.5S0 8 0 8zm8 3.5a3.5 3.5 0 1 0 0-7 3.5 3.5 0 0 0 0 7z"
                    />
                  </svg>
                  <svg v-else viewBox="0 0 16 16" fill="currentColor">
                    <path
                      d="M13.359 11.238C15.06 9.72 16 8 16 8s-3-5.5-8-5.5a7.028 7.028 0 0 0-2.79.588l.77.771A5.944 5.944 0 0 1 8 3.5c2.12 0 3.879 1.168 5.168 2.457A13.134 13.134 0 0 1 14.828 8c-.058.087-.122.183-.195.288-.335.48-.83 1.12-1.465 1.755-.165.165-.337.328-.513.486l.704.709z"
                    />
                    <path
                      d="M11.297 9.176a3.5 3.5 0 0 0-4.474-4.474l.823.823a2.5 2.5 0 0 1 2.829 2.829l.822.822zm-2.943 1.299.822.822a3.5 3.5 0 0 1-4.474-4.474l.823.823a2.5 2.5 0 0 0 2.829 2.829z"
                    />
                    <path
                      d="M3.35 5.47c-.18.16-.353.322-.518.487A13.134 13.134 0 0 0 1.172 8l.195.288c.335.48.83 1.12 1.465 1.755C4.121 11.332 5.881 12.5 8 12.5c.716 0 1.39-.133 2.02-.36l.77.772A7.029 7.029 0 0 1 8 13.5C3 13.5 0 8 0 8s.939-1.721 2.641-3.238l.708.709z"
                    />
                    <path
                      fill-rule="evenodd"
                      d="M13.646 14.354l-12-12 .708-.708 12 12-.708.708z"
                    />
                  </svg>
                </button>
              </div>
            </div>

            <Transition name="t-alert">
              <div v-if="errorMessage" class="auth-alert" role="alert">
                <svg class="auth-alert__icon" viewBox="0 0 16 16" fill="currentColor">
                  <path
                    d="M8.982 1.566a1.13 1.13 0 0 0-1.96 0L.165 13.233c-.457.778.091 1.767.98 1.767h13.713c.889 0 1.438-.99.98-1.767L8.982 1.566zM8 5c.535 0 .954.462.9.995l-.35 3.507a.552.552 0 0 1-1.1 0L7.1 5.995A.905.905 0 0 1 8 5zm.002 6a1 1 0 1 1 0 2 1 1 0 0 1 0-2z"
                  />
                </svg>
                <span>{{ errorMessage }}</span>
              </div>
            </Transition>
          </div>

          <div class="auth-form__footer">
            <router-link :to="{ name: 'login' }" class="link-btn">
              已有账号? 登录
            </router-link>
            <button
              type="submit"
              class="primary-btn"
              :disabled="loading || !username.length || !password.length || !confirmPassword.length"
            >
              <span v-if="loading" class="spinner" />
              <span>{{ loading ? '注册中…' : '注册' }}</span>
            </button>
          </div>
        </form>
      </section>
    </main>
  </div>
</template>

<style scoped lang="scss">
$ease: cubic-bezier(0.22, 0.6, 0.36, 1);
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
$base: #f4f5f6;
$error: #ef4444;
$success: #22c55e;

@keyframes fade-up {
  from { opacity: 0; transform: translateY(20px); }
  to   { opacity: 1; transform: translateY(0); }
}
@keyframes card-in {
  from { opacity: 0; transform: scale(0.97) translateY(28px); }
  to   { opacity: 1; transform: scale(1) translateY(0); }
}
@keyframes spin { to { transform: rotate(360deg); } }

.auth-page {
  min-height: 100dvh;
  display: flex; justify-content: center; align-items: center;
  padding: 40px 24px;
  background-color: $base;
  position: relative; overflow: hidden;
  font-family: Inter, system-ui, -apple-system, 'Segoe UI', sans-serif;
}
.auth-page__dots {
  position: fixed; inset: 0;
  background-image: radial-gradient(circle, rgba(0,0,0,0.03) 1px, transparent 1px);
  background-size: 24px 24px;
  pointer-events: none;
}

.auth-card {
  position: relative; z-index: 1;
  width: 100%; max-width: 960px; height: 560px;
  padding: 52px 64px;
  background-color: $surface;
  border: 1px solid $border; border-radius: 20px;
  box-shadow: 0 1px 3px rgba(0,0,0,0.04), 0 8px 32px rgba(0,0,0,0.06);
  display: grid;
  grid-template-columns: 260px 1fr;
  column-gap: 64px;
  @media (prefers-reduced-motion: no-preference) {
    animation: card-in 700ms $ease both;
  }
}

.entrance {
  @media (prefers-reduced-motion: no-preference) {
    opacity: 0;
    animation: fade-up 550ms $ease forwards;
    animation-delay: var(--d, 0ms);
  }
}

.auth-brand {
  display: flex; flex-direction: column;
  justify-content: space-between; height: 100%;
}
.auth-brand__top { display: flex; flex-direction: column; }
.auth-brand__logo {
  width: 120px; height: 120px;
  object-fit: contain; margin-bottom: 20px;
}
.auth-brand__name {
  margin: 0 0 8px;
  font-size: 28px; font-weight: 700;
  letter-spacing: -0.03em;
  color: $text-primary; line-height: 1.2;
}
.auth-brand__slogan {
  margin: 0; font-size: 15px;
  color: $text-muted; line-height: 1.6;
}

.github-link {
  display: inline-flex; align-items: center; align-self: flex-start;
  gap: 10px; height: 42px; padding: 0 18px;
  border: 1px solid $border-strong; border-radius: 12px;
  color: $text-body; font-size: 13px; font-weight: 500;
  text-decoration: none;
  transition: background-color 160ms $ease, border-color 160ms $ease,
    box-shadow 200ms $ease, color 160ms $ease;
  &:hover {
    background-color: rgba(0,0,0,0.04);
    border-color: rgba(0,0,0,0.22);
    color: $text-primary;
    box-shadow: 0 4px 16px rgba(0,0,0,0.08);
  }
  &:focus-visible { outline: 2px solid $accent; outline-offset: 2px; }
}
.github-link__icon { width: 18px; height: 18px; flex-shrink: 0; }

.auth-body { height: 100%; }
.auth-form {
  display: flex; flex-direction: column; height: 100%;
}
.auth-form__header {
  display: flex; justify-content: flex-end;
  margin-bottom: 24px; flex-shrink: 0;
}
.auth-form__title {
  margin: 0; font-size: 22px; font-weight: 600;
  letter-spacing: -0.01em; color: $text-primary; line-height: 1;
}
.auth-form__fields {
  display: flex; flex-direction: column; gap: 18px; flex: 1;
}
.auth-form__footer {
  display: flex; align-items: center;
  justify-content: space-between;
  flex-shrink: 0; padding-top: 20px;
}

.form-field { display: flex; flex-direction: column; gap: 8px; }
.form-field__label {
  font-size: 13px; font-weight: 500; color: $text-body;
  display: flex; align-items: center; gap: 8px;
}
.form-field__control {
  position: relative; display: flex; align-items: center;
  &:focus-within .form-field__icon { color: $accent; }
}
.form-field__icon {
  position: absolute; left: 14px;
  width: 18px; height: 18px;
  color: $text-muted; pointer-events: none;
  transition: color 160ms $ease;
}
.form-field__input {
  width: 100%; height: 50px;
  padding: 0 14px 0 44px;
  border: 1px solid $border-strong; border-radius: 12px;
  background-color: $surface;
  font-size: 15px; color: $text-primary;
  transition: border-color 160ms $ease, box-shadow 160ms $ease;
  &::placeholder { color: $text-muted; opacity: 0.55; }
  &:focus {
    outline: none; border-color: $accent;
    box-shadow: 0 0 0 3px $accent-wash;
  }
  &:disabled {
    opacity: 0.5; cursor: not-allowed;
    background-color: rgba(0,0,0,0.02);
  }
}
.form-field__input--trail { padding-right: 50px; }
.form-field__input--ok:not(:focus) {
  border-color: $success;
  box-shadow: 0 0 0 3px rgba(34,197,94,0.1);
}
.form-field__input--err:not(:focus) {
  border-color: $error;
  box-shadow: 0 0 0 3px rgba(239,68,68,0.08);
}
.form-field__trail {
  position: absolute; right: 8px;
  width: 34px; height: 34px;
  border: none; background: none;
  color: $text-muted; border-radius: 10px;
  cursor: pointer; display: grid; place-items: center;
  transition: color 140ms $ease, background-color 140ms $ease;
  svg { width: 17px; height: 17px; }
  &:hover { color: $text-body; background-color: rgba(0,0,0,0.05); }
  &:focus-visible { outline: 2px solid $accent; outline-offset: 1px; }
}

.match-badge {
  margin-left: auto;
  display: inline-flex; align-items: center;
  gap: 4px; font-size: 12px; font-weight: 500;
  svg { width: 13px; height: 13px; }
  &--ok { color: $success; }
  &--err { color: $error; }
}

.auth-alert {
  display: flex; align-items: flex-start;
  gap: 10px; padding: 12px 14px;
  border-radius: 12px;
  background-color: rgba(239,68,68,0.05);
  border: 1px solid rgba(239,68,68,0.14);
}
.auth-alert__icon {
  width: 15px; height: 15px; flex-shrink: 0;
  margin-top: 1px; color: $error;
}
.auth-alert span { font-size: 13px; color: $error; line-height: 1.5; }

.link-btn {
  font-size: 14px; font-weight: 500; color: $accent;
  text-decoration: none; border: none; background: none;
  cursor: pointer; padding: 0; font-family: inherit;
  transition: color 140ms $ease;
  &:hover {
    color: $accent-hover;
    text-decoration: underline; text-underline-offset: 3px;
  }
  &:focus-visible { outline: 2px solid $accent; outline-offset: 2px; border-radius: 2px; }
}
.primary-btn {
  display: inline-flex; align-items: center; justify-content: center;
  gap: 8px; height: 46px; min-width: 100px; padding: 0 28px;
  border: none; border-radius: 12px;
  background-color: $accent; color: #fff;
  font-size: 15px; font-weight: 600; cursor: pointer;
  transition: background-color 140ms $ease, box-shadow 200ms $ease,
    opacity 140ms $ease, transform 120ms $ease;
  &:hover:not(:disabled) {
    background-color: $accent-hover;
    box-shadow: 0 6px 20px rgba(0,120,212,0.22);
    transform: translateY(-1px);
  }
  &:active:not(:disabled) {
    background-color: $accent-pressed;
    box-shadow: none; transform: translateY(0);
  }
  &:disabled { opacity: 0.4; cursor: not-allowed; }
  &:focus-visible { outline: 2px solid $accent; outline-offset: 3px; }
}
.spinner {
  width: 16px; height: 16px;
  border: 2px solid rgba(255,255,255,0.35);
  border-top-color: #fff; border-radius: 50%;
  @media (prefers-reduced-motion: no-preference) {
    animation: spin 0.7s linear infinite;
  }
}

.t-alert-enter-active, .t-alert-leave-active {
  transition: opacity 180ms $ease, transform 180ms $ease;
}
.t-alert-enter-from, .t-alert-leave-to {
  opacity: 0; transform: translateY(-4px);
}
.t-match-enter-active, .t-match-leave-active {
  transition: opacity 160ms $ease, transform 160ms $ease;
}
.t-match-enter-from, .t-match-leave-to {
  opacity: 0; transform: translateY(-2px);
}

@media (max-width: 820px) {
  .auth-card {
    height: auto; display: flex; flex-direction: column;
    gap: 36px; padding: 36px 28px;
  }
  .auth-brand { height: auto; gap: 24px; }
  .auth-brand__logo { width: 88px; height: 88px; }
  .auth-brand__name { font-size: 22px; }
  .auth-body { height: auto; }
  .auth-form__header { justify-content: flex-start; margin-bottom: 24px; }
}
</style>