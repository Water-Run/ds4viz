<script setup lang="ts">
/**
 * 用户页面
 *
 * 居中信息卡 + 左右分栏（收藏/执行记录）。
 *
 * @file src/views/Profile.vue
 * @author WaterRun
 * @date 2026-02-27
 * @component Profile
 */

import { computed, onMounted, ref, watch } from 'vue'
import { useRouter } from 'vue-router'

import { useAuthStore } from '@/stores/auth'
import { uploadAvatarApi } from '@/api/users'
import { fetchFavoritesApi, fetchExecutionHistoryApi } from '@/api/users'
import { getUserAvatarUrl } from '@/api/users'
import { extractErrorMessage } from '@/utils/error'
import { formatDateTime, formatDuration } from '@/utils/time'
import { LANGUAGE_LABELS } from '@/types/api'
import type { Language } from '@/types/api'
import Pagination from '@/components/common/Pagination.vue'
import ErrorBanner from '@/components/common/ErrorBanner.vue'
import Loading from '@/components/common/Loading.vue'
import MaterialIcon from '@/components/common/MaterialIcon.vue'
import CodeEditor from '@/components/editor/CodeEditor.vue'

import type { FavoriteItem, ExecutionHistoryItem } from '@/api/users'

/** 默认头像颜色调色板 */
const AVATAR_COLORS: readonly string[] = [
  '#0078d4',
  '#0e7c6b',
  '#7c3aed',
  '#c2410c',
  '#0369a1',
  '#6d28d9',
  '#b45309',
  '#059669',
  '#dc2626',
  '#4f46e5',
]

/**
 * 状态样式配置
 *
 * @interface
 */
interface StatusStyle {
  /** 文字颜色 */
  color: string
  /** 背景颜色 */
  bg: string
}

/**
 * 状态 → 颜色映射
 */
const STATUS_STYLES: Record<string, StatusStyle> = {
  active: { color: 'var(--color-success)', bg: 'var(--color-success-muted)' },
  inactive: { color: 'var(--color-warning)', bg: 'var(--color-warning-muted)' },
  banned: { color: 'var(--color-error)', bg: 'var(--color-error-muted)' },
  disabled: { color: 'var(--color-error)', bg: 'var(--color-error-muted)' },
}

/** 默认状态样式 */
const DEFAULT_STATUS_STYLE: StatusStyle = {
  color: 'var(--color-text-tertiary)',
  bg: 'var(--color-bg-hover)',
}

/**
 * 执行状态颜色类映射
 */
const EXEC_STATUS_COLORS: Record<string, string> = {
  success: 'var(--color-success)',
  error: 'var(--color-error)',
  timeout: 'var(--color-warning)',
}

function getAvatarColor(username: string): string {
  let hash = 0
  for (let i = 0; i < username.length; i += 1) {
    hash = ((hash << 5) - hash + username.charCodeAt(i)) | 0
  }
  return AVATAR_COLORS[Math.abs(hash) % AVATAR_COLORS.length]
}

function getAvatarInitial(username: string): string {
  return username.length > 0 ? username.charAt(0).toUpperCase() : '-'
}

/**
 * 计算代码行数
 *
 * @param code - 代码文本
 * @returns 行数
 */
function getLineCount(code: string): number {
  return code.split('\n').length
}

/**
 * 获取执行状态颜色
 *
 * @param status - 状态字符串
 * @returns CSS 颜色值
 */
function getExecStatusColor(status: string): string {
  return EXEC_STATUS_COLORS[status.toLowerCase()] ?? 'var(--color-text-tertiary)'
}

/**
 * 获取执行状态背景色
 *
 * @param status - 状态字符串
 * @returns CSS 颜色值
 */
function getExecStatusBg(status: string): string {
  const key = status.toLowerCase()
  if (key === 'success') return 'var(--color-success-muted)'
  if (key === 'error') return 'var(--color-error-muted)'
  if (key === 'timeout') return 'var(--color-warning-muted)'
  return 'var(--color-bg-hover)'
}

const router = useRouter()
const authStore = useAuthStore()

const avatarUploading = ref<boolean>(false)
const avatarError = ref<string>('')
const avatarLoadFailed = ref<boolean>(false)
const showPasswordForm = ref<boolean>(false)
const passwordError = ref<string>('')
const passwordSuccess = ref<string>('')
const oldPassword = ref<string>('')
const newPassword = ref<string>('')

const favorites = ref<FavoriteItem[]>([])
const favoritesTotal = ref<number>(0)
const favoritesPage = ref<number>(1)
const favoritesLoading = ref<boolean>(false)
const favoritesError = ref<string>('')

const executions = ref<ExecutionHistoryItem[]>([])
const executionsTotal = ref<number>(0)
const executionsPage = ref<number>(1)
const executionsLoading = ref<boolean>(false)
const executionsError = ref<string>('')

/**
 * 当前展开的执行记录 ID
 */
const expandedExecId = ref<number | null>(null)

/**
 * 复制成功的执行记录 ID
 */
const copiedExecId = ref<number | null>(null)

const currentUser = computed(() => authStore.currentUser)

const hasCustomAvatar = computed<boolean>(() => {
  return currentUser.value !== null
    && currentUser.value.avatarUrl !== null
    && !avatarLoadFailed.value
})

const avatarSrc = computed<string>(() => {
  if (!currentUser.value || currentUser.value.avatarUrl === null) return ''
  return getUserAvatarUrl(currentUser.value.id)
})

const userInitial = computed<string>(() => {
  return getAvatarInitial(currentUser.value?.username ?? '')
})

const userAvatarColor = computed<string>(() => {
  return getAvatarColor(currentUser.value?.username ?? '')
})

const statusStyle = computed<StatusStyle>(() => {
  const status = (currentUser.value?.status ?? '').toLowerCase()
  return STATUS_STYLES[status] ?? DEFAULT_STATUS_STYLE
})

const favoritesTotalPages = computed(() => Math.max(1, Math.ceil(favoritesTotal.value / 10)))
const executionsTotalPages = computed(() => Math.max(1, Math.ceil(executionsTotal.value / 10)))

watch(
  () => currentUser.value?.avatarUrl,
  () => {
    avatarLoadFailed.value = false
  },
)

const loadFavorites = async (): Promise<void> => {
  if (!currentUser.value) return
  favoritesLoading.value = true
  favoritesError.value = ''
  try {
    const result = await fetchFavoritesApi(currentUser.value.id, {
      page: favoritesPage.value,
      limit: 10,
    })
    favorites.value = result.items
    favoritesTotal.value = result.total
  } catch (error: unknown) {
    favoritesError.value = extractErrorMessage(error)
  } finally {
    favoritesLoading.value = false
  }
}

const loadExecutions = async (): Promise<void> => {
  if (!currentUser.value) return
  executionsLoading.value = true
  executionsError.value = ''
  try {
    const result = await fetchExecutionHistoryApi(currentUser.value.id, {
      page: executionsPage.value,
      limit: 10,
    })
    executions.value = result.items
    executionsTotal.value = result.total
  } catch (error: unknown) {
    executionsError.value = extractErrorMessage(error)
  } finally {
    executionsLoading.value = false
  }
}

const handleAvatarChange = async (event: Event): Promise<void> => {
  const input = event.target as HTMLInputElement
  const file = input.files?.[0]
  if (!file || !currentUser.value) return
  avatarUploading.value = true
  avatarError.value = ''
  try {
    const result = await uploadAvatarApi(currentUser.value.id, file)
    authStore.setCurrentUser({
      ...currentUser.value,
      avatarUrl: result.avatarUrl,
    })
  } catch (error: unknown) {
    avatarError.value = extractErrorMessage(error)
  } finally {
    avatarUploading.value = false
    input.value = ''
  }
}

const handleAvatarError = (): void => {
  avatarLoadFailed.value = true
}

const handlePasswordChange = async (): Promise<void> => {
  passwordError.value = ''
  passwordSuccess.value = ''
  try {
    await authStore.changePassword({
      oldPassword: oldPassword.value,
      newPassword: newPassword.value,
    })
    passwordSuccess.value = '密码已更新'
    oldPassword.value = ''
    newPassword.value = ''
  } catch (error: unknown) {
    passwordError.value = extractErrorMessage(error)
  }
}

const handleFavoritesPage = async (page: number): Promise<void> => {
  favoritesPage.value = page
  await loadFavorites()
}

const handleExecutionsPage = async (page: number): Promise<void> => {
  executionsPage.value = page
  expandedExecId.value = null
  await loadExecutions()
}

/**
 * 切换展开执行记录
 *
 * @param id - 执行记录 ID
 */
const toggleExecExpand = (id: number): void => {
  expandedExecId.value = expandedExecId.value === id ? null : id
}

/**
 * 复制执行代码到剪切板
 *
 * @param item - 执行记录
 */
const handleCopyCode = async (item: ExecutionHistoryItem): Promise<void> => {
  try {
    await navigator.clipboard.writeText(item.code)
    copiedExecId.value = item.id
    window.setTimeout(() => {
      if (copiedExecId.value === item.id) {
        copiedExecId.value = null
      }
    }, 2000)
  } catch {
    /* clipboard API 不可用 */
  }
}

/**
 * 在编辑器中编辑执行代码
 *
 * @param item - 执行记录
 */
const handleEditInPlayground = (item: ExecutionHistoryItem): void => {
  try {
    sessionStorage.setItem('ds4viz_edit_code', item.code)
    sessionStorage.setItem('ds4viz_edit_language', item.language)
  } catch {
    /* sessionStorage 不可用 */
  }
  router.push({ name: 'playground' })
}

onMounted(async () => {
  await Promise.all([loadFavorites(), loadExecutions()])
})
</script>

<template>
  <div class="profile-page">
    <header class="profile-page__header">
      <div class="profile-page__title">
        <MaterialIcon name="person" :size="18" />
        <span>用户</span>
      </div>
    </header>

    <!-- 信息卡 -->
    <section class="profile-info">
      <div class="profile-info__avatar">
        <img
          v-if="hasCustomAvatar"
          :src="avatarSrc"
          alt="avatar"
          class="profile-info__avatar-img"
          @error="handleAvatarError"
        />
        <div
          v-else
          class="profile-info__avatar-default"
          :style="{ backgroundColor: userAvatarColor }"
        >
          {{ userInitial }}
        </div>
        <label class="profile-info__avatar-upload" :class="{ 'profile-info__avatar-upload--busy': avatarUploading }">
          <MaterialIcon name="photo_camera" :size="14" />
          <input
            class="sr-only"
            type="file"
            accept="image/*"
            :disabled="avatarUploading"
            @change="handleAvatarChange"
          />
        </label>
      </div>
      <ErrorBanner :message="avatarError" @dismiss="avatarError = ''" />
      <div class="profile-info__name">{{ currentUser?.username }}</div>
      <div class="profile-info__meta">
        <span
          class="profile-info__status-badge"
          :style="{ color: statusStyle.color, backgroundColor: statusStyle.bg }"
        >
          {{ currentUser?.status }}
        </span>
        <span class="profile-info__meta-sep">·</span>
        <span>ID {{ currentUser?.id }}</span>
      </div>
      <button class="profile-info__pwd-link" @click="showPasswordForm = !showPasswordForm">
        {{ showPasswordForm ? '取消' : '修改密码' }}
      </button>
      <Transition name="slide-fade">
        <div v-if="showPasswordForm" class="pwd-form">
          <input v-model="oldPassword" type="password" class="pwd-form__input" placeholder="旧密码" />
          <input v-model="newPassword" type="password" class="pwd-form__input" placeholder="新密码" />
          <button class="pwd-form__submit" :disabled="!oldPassword.length || !newPassword.length" @click="handlePasswordChange">确认修改</button>
          <ErrorBanner :message="passwordError" @dismiss="passwordError = ''" />
          <p v-if="passwordSuccess" class="pwd-form__success">{{ passwordSuccess }}</p>
        </div>
      </Transition>
    </section>

    <!-- 左右分栏 -->
    <div class="profile-columns">
      <!-- 收藏 -->
      <section class="profile-col">
        <h3 class="profile-col__title">
          <MaterialIcon name="favorite" :size="16" />
          <span>收藏的模板</span>
        </h3>
        <ErrorBanner :message="favoritesError" @dismiss="favoritesError = ''" />
        <div v-if="favoritesLoading" class="profile-col__center"><Loading /></div>
        <div v-else-if="favorites.length === 0" class="profile-col__center profile-col__empty">
          <MaterialIcon name="bookmark" :size="24" />
          <p>暂无收藏</p>
        </div>
        <div v-else class="profile-col__list">
          <div v-for="item in favorites" :key="item.templateId" class="col-item">
            <div class="col-item__title">{{ item.title }}</div>
            <div class="col-item__desc">{{ item.description }}</div>
            <div class="col-item__meta">
              {{ item.category }} · 收藏 {{ item.favoriteCount }} · {{ formatDateTime(item.favoritedAt) }}
            </div>
          </div>
        </div>
        <Pagination
          v-if="favoritesTotal > 10"
          :page="favoritesPage"
          :total-pages="favoritesTotalPages"
          @change="handleFavoritesPage"
        />
      </section>

      <!-- 执行记录 -->
      <section class="profile-col">
        <h3 class="profile-col__title">
          <MaterialIcon name="history" :size="16" />
          <span>执行记录</span>
        </h3>
        <ErrorBanner :message="executionsError" @dismiss="executionsError = ''" />
        <div v-if="executionsLoading" class="profile-col__center"><Loading /></div>
        <div v-else-if="executions.length === 0" class="profile-col__center profile-col__empty">
          <MaterialIcon name="schedule" :size="24" />
          <p>暂无记录</p>
        </div>
        <div v-else class="profile-col__list">
          <div
            v-for="item in executions"
            :key="item.id"
            class="exec-item"
            :class="{ 'exec-item--expanded': expandedExecId === item.id }"
          >
            <button class="exec-item__summary" @click="toggleExecExpand(item.id)">
              <span class="exec-item__lang">
                {{ LANGUAGE_LABELS[item.language as Language] ?? item.language }}
              </span>
              <span
                class="exec-item__status"
                :style="{ color: getExecStatusColor(item.status), backgroundColor: getExecStatusBg(item.status) }"
              >
                {{ item.status }}
              </span>
              <span class="exec-item__duration">
                {{ item.executionTime !== null ? formatDuration(item.executionTime) : '--' }}
              </span>
              <span class="exec-item__lines">{{ getLineCount(item.code) }} 行</span>
              <span class="exec-item__time">{{ formatDateTime(item.createdAt) }}</span>
              <MaterialIcon
                :name="expandedExecId === item.id ? 'expand_less' : 'expand_more'"
                :size="16"
                class="exec-item__chevron"
              />
            </button>
            <Transition name="slide-fade">
              <div v-if="expandedExecId === item.id" class="exec-item__detail">
                <div class="exec-item__editor">
                  <CodeEditor
                    :model-value="item.code"
                    :language="(item.language as Language)"
                    :readonly="true"
                  />
                </div>
                <div class="exec-item__actions">
                  <button class="exec-action-btn" @click.stop="handleCopyCode(item)">
                    <MaterialIcon :name="copiedExecId === item.id ? 'check' : 'content_copy'" :size="16" />
                    <span>{{ copiedExecId === item.id ? '已复制' : '复制代码' }}</span>
                  </button>
                  <button class="exec-action-btn exec-action-btn--primary" @click.stop="handleEditInPlayground(item)">
                    <MaterialIcon name="edit" :size="16" />
                    <span>在编辑器中编辑</span>
                  </button>
                </div>
              </div>
            </Transition>
          </div>
        </div>
        <Pagination
          v-if="executionsTotal > 10"
          :page="executionsPage"
          :total-pages="executionsTotalPages"
          @change="handleExecutionsPage"
        />
      </section>
    </div>
  </div>
</template>

<style scoped>
.profile-page {
  display: flex;
  flex-direction: column;
  gap: var(--space-2);
  padding: var(--space-3);
  height: 100%;
  overflow-y: auto;
}

.profile-page__header {
  display: flex;
  align-items: center;
  flex-shrink: 0;
}

.profile-page__title {
  display: flex;
  align-items: center;
  gap: 6px;
  font-size: var(--text-base);
  font-weight: var(--weight-semibold);
  color: var(--color-text-primary);
}

.profile-page__title :deep(.material-icon) {
  width: 18px;
  height: 18px;
}

/* ---- 信息卡 ---- */

.profile-info {
  display: flex;
  flex-direction: column;
  align-items: center;
  gap: var(--space-1);
  flex-shrink: 0;
  padding: var(--space-3) var(--space-2);
  width: 100%;
  border: 1px solid var(--color-border);
  border-radius: var(--radius-lg);
  background-color: var(--color-bg-surface);
}

.profile-info__avatar { position: relative; width: 72px; height: 72px; }
.profile-info__avatar-img { width: 72px; height: 72px; border-radius: 50%; object-fit: cover; border: 1px solid var(--color-border-strong); }
.profile-info__avatar-default { width: 72px; height: 72px; border-radius: 50%; display: flex; align-items: center; justify-content: center; color: #ffffff; font-size: 28px; font-weight: 600; line-height: 1; user-select: none; }
.profile-info__avatar-upload { position: absolute; right: -4px; bottom: -4px; width: 26px; height: 26px; display: flex; align-items: center; justify-content: center; border-radius: 50%; border: 1px solid var(--color-border); background-color: var(--color-bg-surface); cursor: pointer; transition: background-color var(--duration-fast) var(--ease); }
.profile-info__avatar-upload:hover { background-color: var(--color-bg-hover); }
.profile-info__avatar-upload--busy { opacity: 0.5; cursor: not-allowed; }
.profile-info__avatar-upload :deep(.material-icon) { width: 14px; height: 14px; }
.profile-info__name { font-size: var(--text-lg); font-weight: var(--weight-semibold); color: var(--color-text-primary); }
.profile-info__meta { display: flex; align-items: center; gap: 6px; font-size: var(--text-xs); color: var(--color-text-tertiary); }
.profile-info__meta-sep { color: var(--color-text-disabled); }
.profile-info__status-badge { display: inline-block; padding: 2px 10px; border-radius: var(--radius-full); font-size: var(--text-xs); font-weight: var(--weight-medium); line-height: 1.5; }
.profile-info__pwd-link { border: none; background: none; color: var(--color-accent); font-size: var(--text-xs); font-family: inherit; font-weight: var(--weight-medium); cursor: pointer; transition: color var(--duration-fast) var(--ease); }
.profile-info__pwd-link:hover { color: var(--color-accent-hover); }

/* ---- 密码表单 ---- */

.pwd-form { display: flex; flex-direction: column; gap: var(--space-1); width: 100%; max-width: 280px; }
.pwd-form__input { height: var(--control-height-md); border: 1px solid var(--color-border-strong); border-radius: var(--radius-control); padding: 0 12px; font-size: var(--text-sm); color: var(--color-text-primary); transition: border-color var(--duration-fast) var(--ease); }
.pwd-form__input:focus { border-color: var(--color-accent); }
.pwd-form__submit { height: var(--control-height-md); border-radius: var(--radius-control); border: none; background-color: var(--color-accent); color: var(--color-accent-contrast); font-size: var(--text-sm); font-weight: var(--weight-medium); cursor: pointer; transition: background-color var(--duration-fast) var(--ease), transform var(--duration-fast) var(--ease); }
.pwd-form__submit:hover:not(:disabled) { background-color: var(--color-accent-hover); transform: translateY(-1px); }
.pwd-form__submit:disabled { opacity: 0.4; cursor: not-allowed; transform: none; }
.pwd-form__success { margin: 0; font-size: var(--text-xs); color: var(--color-success); text-align: center; }

/* ---- 左右分栏 ---- */

.profile-columns { display: grid; grid-template-columns: 1fr 1fr; gap: var(--space-2); flex: 1; min-height: 0; width: 100%; }
.profile-col { display: flex; flex-direction: column; min-height: 0; min-width: 0; border: 1px solid var(--color-border); border-radius: var(--radius-lg); background-color: var(--color-bg-surface); padding: var(--space-2); gap: var(--space-1); }
.profile-col__title { margin: 0; display: flex; align-items: center; gap: 6px; font-size: var(--text-sm); font-weight: var(--weight-semibold); color: var(--color-text-primary); flex-shrink: 0; }
.profile-col__title :deep(.material-icon) { width: 16px; height: 16px; }
.profile-col__center { flex: 1; display: flex; align-items: center; justify-content: center; }
.profile-col__empty { flex-direction: column; gap: var(--space-1); color: var(--color-text-tertiary); }
.profile-col__empty p { margin: 0; font-size: var(--text-xs); }
.profile-col__empty :deep(.material-icon) { width: 24px; height: 24px; }
.profile-col__list { flex: 1; display: flex; flex-direction: column; gap: var(--space-1); overflow-y: auto; min-height: 0; }

/* ---- 收藏项 ---- */

.col-item { padding: var(--space-1); border: 1px solid var(--color-border); border-radius: var(--radius-md); background-color: var(--color-bg-surface-alt); flex-shrink: 0; transition: border-color var(--duration-fast) var(--ease), box-shadow var(--duration-fast) var(--ease); }
.col-item:hover { border-color: var(--color-border-strong); box-shadow: var(--shadow-static); }
.col-item__title { display: flex; align-items: center; gap: 6px; font-size: var(--text-sm); font-weight: var(--weight-semibold); color: var(--color-text-primary); min-width: 0; }
.col-item__desc { font-size: var(--text-xs); color: var(--color-text-tertiary); margin-top: 2px; overflow: hidden; text-overflow: ellipsis; white-space: nowrap; min-width: 0; }
.col-item__meta { font-size: var(--text-xs); color: var(--color-text-tertiary); margin-top: 2px; white-space: nowrap; overflow: hidden; text-overflow: ellipsis; }

/* ---- 执行记录项 ---- */

.exec-item {
  border: 1px solid var(--color-border);
  border-radius: var(--radius-md);
  background-color: var(--color-bg-surface-alt);
  flex-shrink: 0;
  transition:
    border-color var(--duration-fast) var(--ease),
    box-shadow var(--duration-fast) var(--ease);
}

.exec-item:hover {
  border-color: var(--color-border-strong);
}

.exec-item--expanded {
  border-color: var(--color-accent);
}

.exec-item__summary {
  display: flex;
  align-items: center;
  gap: 8px;
  width: 100%;
  padding: var(--space-1);
  border: none;
  background: none;
  cursor: pointer;
  font-family: inherit;
  text-align: left;
  border-radius: var(--radius-md);
  transition: background-color var(--duration-fast) var(--ease);
}

.exec-item__summary:hover {
  background-color: var(--color-bg-hover);
}

.exec-item__lang {
  font-size: var(--text-sm);
  font-weight: var(--weight-semibold);
  color: var(--color-text-primary);
  white-space: nowrap;
}

.exec-item__status {
  font-size: var(--text-xs);
  font-weight: var(--weight-medium);
  padding: 1px 8px;
  border-radius: var(--radius-full);
  white-space: nowrap;
}

.exec-item__duration {
  font-size: var(--text-xs);
  color: var(--color-text-tertiary);
  font-family: var(--font-mono);
  white-space: nowrap;
}

.exec-item__lines {
  font-size: var(--text-xs);
  color: var(--color-text-tertiary);
  white-space: nowrap;
}

.exec-item__time {
  font-size: var(--text-xs);
  color: var(--color-text-tertiary);
  white-space: nowrap;
  margin-left: auto;
}

.exec-item__chevron {
  width: 16px;
  height: 16px;
  flex-shrink: 0;
  color: var(--color-text-tertiary);
}

.exec-item__detail {
  display: flex;
  flex-direction: column;
  gap: var(--space-1);
  padding: 0 var(--space-1) var(--space-1);
}

.exec-item__editor {
  height: 180px;
  display: flex;
  border-radius: var(--radius-md);
  overflow: hidden;
}

.exec-item__actions {
  display: flex;
  align-items: center;
  gap: var(--space-1);
}

.exec-action-btn {
  display: inline-flex;
  align-items: center;
  gap: 4px;
  height: var(--control-height-sm);
  padding: 0 10px;
  border: 1px solid var(--color-border-strong);
  border-radius: var(--radius-control);
  background-color: var(--color-bg-surface);
  color: var(--color-text-body);
  font-size: var(--text-xs);
  font-family: inherit;
  cursor: pointer;
  transition:
    background-color var(--duration-fast) var(--ease),
    border-color var(--duration-fast) var(--ease);
}

.exec-action-btn:hover {
  background-color: var(--color-bg-hover);
  border-color: var(--color-border-strong);
}

.exec-action-btn :deep(.material-icon) {
  width: 16px;
  height: 16px;
}

.exec-action-btn--primary {
  border-color: var(--color-accent);
  background-color: var(--color-accent);
  color: var(--color-accent-contrast);
}

.exec-action-btn--primary:hover {
  background-color: var(--color-accent-hover);
  border-color: var(--color-accent-hover);
}

.sr-only { position: absolute; width: 1px; height: 1px; padding: 0; margin: -1px; overflow: hidden; clip: rect(0, 0, 0, 0); white-space: nowrap; border: 0; }

@media (max-width: 860px) {
  .profile-columns { grid-template-columns: 1fr; }
}
</style>
