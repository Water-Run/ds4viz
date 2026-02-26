<script setup lang="ts">
/**
 * 用户页面
 *
 * 居中信息卡 + 左右分栏（收藏/执行记录）。
 *
 * @component Profile
 */

import { computed, onMounted, ref, watch } from 'vue'

import { useAuthStore } from '@/stores/auth'
import { uploadAvatarApi } from '@/api/users'
import { fetchFavoritesApi, fetchExecutionHistoryApi } from '@/api/users'
import { getUserAvatarUrl } from '@/api/users'
import { extractErrorMessage } from '@/utils/error'
import { formatDateTime, formatDuration } from '@/utils/time'
import Pagination from '@/components/common/Pagination.vue'
import ErrorBanner from '@/components/common/ErrorBanner.vue'
import Loading from '@/components/common/Loading.vue'
import MaterialIcon from '@/components/common/MaterialIcon.vue'

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
 * 根据用户名生成确定性头像背景色
 *
 * @param username - 用户名
 * @returns 十六进制颜色字符串
 */
function getAvatarColor(username: string): string {
  let hash = 0
  for (let i = 0; i < username.length; i += 1) {
    hash = ((hash << 5) - hash + username.charCodeAt(i)) | 0
  }
  return AVATAR_COLORS[Math.abs(hash) % AVATAR_COLORS.length]
}

/**
 * 获取用户名首字母（大写）
 *
 * @param username - 用户名
 * @returns 大写首字母
 */
function getAvatarInitial(username: string): string {
  return username.length > 0 ? username.charAt(0).toUpperCase() : '-'
}

/**
 * 认证状态
 */
const authStore = useAuthStore()

/**
 * 头像上传中
 */
const avatarUploading = ref<boolean>(false)

/**
 * 头像错误提示
 */
const avatarError = ref<string>('')

/**
 * 头像图片加载失败标志
 */
const avatarLoadFailed = ref<boolean>(false)

/**
 * 密码表单展开状态
 */
const showPasswordForm = ref<boolean>(false)

/**
 * 密码错误提示
 */
const passwordError = ref<string>('')

/**
 * 密码更新成功提示
 */
const passwordSuccess = ref<string>('')

/**
 * 旧密码
 */
const oldPassword = ref<string>('')

/**
 * 新密码
 */
const newPassword = ref<string>('')

/**
 * 收藏列表
 */
const favorites = ref<FavoriteItem[]>([])

/**
 * 收藏总数
 */
const favoritesTotal = ref<number>(0)

/**
 * 收藏页码
 */
const favoritesPage = ref<number>(1)

/**
 * 收藏加载
 */
const favoritesLoading = ref<boolean>(false)

/**
 * 收藏错误
 */
const favoritesError = ref<string>('')

/**
 * 执行记录列表
 */
const executions = ref<ExecutionHistoryItem[]>([])

/**
 * 执行记录总数
 */
const executionsTotal = ref<number>(0)

/**
 * 执行记录页码
 */
const executionsPage = ref<number>(1)

/**
 * 执行记录加载
 */
const executionsLoading = ref<boolean>(false)

/**
 * 执行记录错误
 */
const executionsError = ref<string>('')

/**
 * 当前用户
 */
const currentUser = computed(() => authStore.currentUser)

/**
 * 是否拥有自定义头像
 */
const hasCustomAvatar = computed<boolean>(() => {
  return currentUser.value !== null
    && currentUser.value.avatarUrl !== null
    && !avatarLoadFailed.value
})

/**
 * 头像图片地址
 */
const avatarSrc = computed<string>(() => {
  if (!currentUser.value || currentUser.value.avatarUrl === null) return ''
  return getUserAvatarUrl(currentUser.value.id)
})

/**
 * 用户名首字母
 */
const userInitial = computed<string>(() => {
  return getAvatarInitial(currentUser.value?.username ?? '')
})

/**
 * 默认头像背景色
 */
const userAvatarColor = computed<string>(() => {
  return getAvatarColor(currentUser.value?.username ?? '')
})

/**
 * 用户状态样式（根据 status 值自动匹配颜色）
 */
const statusStyle = computed<StatusStyle>(() => {
  const status = (currentUser.value?.status ?? '').toLowerCase()
  return STATUS_STYLES[status] ?? DEFAULT_STATUS_STYLE
})

/**
 * 收藏总页数
 */
const favoritesTotalPages = computed(() => Math.max(1, Math.ceil(favoritesTotal.value / 10)))

/**
 * 执行记录总页数
 */
const executionsTotalPages = computed(() => Math.max(1, Math.ceil(executionsTotal.value / 10)))

/**
 * avatarUrl 变化时重置加载失败标志
 */
watch(
  () => currentUser.value?.avatarUrl,
  () => {
    avatarLoadFailed.value = false
  },
)

/**
 * 加载收藏
 */
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

/**
 * 加载执行记录
 */
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

/**
 * 上传头像
 */
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

/**
 * 头像图片加载失败回退
 */
const handleAvatarError = (): void => {
  avatarLoadFailed.value = true
}

/**
 * 更新密码
 */
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

/**
 * 收藏分页
 */
const handleFavoritesPage = async (page: number): Promise<void> => {
  favoritesPage.value = page
  await loadFavorites()
}

/**
 * 执行记录分页
 */
const handleExecutionsPage = async (page: number): Promise<void> => {
  executionsPage.value = page
  await loadExecutions()
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

    <!-- 信息卡（与下方列区同宽） -->
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
          <input
            v-model="oldPassword"
            type="password"
            class="pwd-form__input"
            placeholder="旧密码"
          />
          <input
            v-model="newPassword"
            type="password"
            class="pwd-form__input"
            placeholder="新密码"
          />
          <button
            class="pwd-form__submit"
            :disabled="!oldPassword.length || !newPassword.length"
            @click="handlePasswordChange"
          >
            确认修改
          </button>
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
        <div v-if="favoritesLoading" class="profile-col__center">
          <Loading />
        </div>
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
        <div v-if="executionsLoading" class="profile-col__center">
          <Loading />
        </div>
        <div v-else-if="executions.length === 0" class="profile-col__center profile-col__empty">
          <MaterialIcon name="schedule" :size="24" />
          <p>暂无记录</p>
        </div>
        <div v-else class="profile-col__list">
          <div v-for="item in executions" :key="item.id" class="col-item">
            <div class="col-item__title">
              {{ item.language }}
              <span class="col-item__status">{{ item.status }}</span>
            </div>
            <div class="col-item__desc">{{ item.code }}</div>
            <div class="col-item__meta">
              {{ formatDateTime(item.createdAt) }} · 耗时 {{ item.executionTime !== null ? formatDuration(item.executionTime) : '--' }}
            </div>
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

/* ---- 信息卡（全宽，内容居中） ---- */

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

.profile-info__avatar {
  position: relative;
  width: 72px;
  height: 72px;
}

.profile-info__avatar-img {
  width: 72px;
  height: 72px;
  border-radius: 50%;
  object-fit: cover;
  border: 1px solid var(--color-border-strong);
}

.profile-info__avatar-default {
  width: 72px;
  height: 72px;
  border-radius: 50%;
  display: flex;
  align-items: center;
  justify-content: center;
  color: #ffffff;
  font-size: 28px;
  font-weight: 600;
  line-height: 1;
  user-select: none;
}

.profile-info__avatar-upload {
  position: absolute;
  right: -4px;
  bottom: -4px;
  width: 26px;
  height: 26px;
  display: flex;
  align-items: center;
  justify-content: center;
  border-radius: 50%;
  border: 1px solid var(--color-border);
  background-color: var(--color-bg-surface);
  cursor: pointer;
  transition: background-color var(--duration-fast) var(--ease);
}

.profile-info__avatar-upload:hover {
  background-color: var(--color-bg-hover);
}

.profile-info__avatar-upload--busy {
  opacity: 0.5;
  cursor: not-allowed;
}

.profile-info__avatar-upload :deep(.material-icon) {
  width: 14px;
  height: 14px;
}

.profile-info__name {
  font-size: var(--text-lg);
  font-weight: var(--weight-semibold);
  color: var(--color-text-primary);
}

.profile-info__meta {
  display: flex;
  align-items: center;
  gap: 6px;
  font-size: var(--text-xs);
  color: var(--color-text-tertiary);
}

.profile-info__meta-sep {
  color: var(--color-text-disabled);
}

.profile-info__status-badge {
  display: inline-block;
  padding: 2px 10px;
  border-radius: var(--radius-full);
  font-size: var(--text-xs);
  font-weight: var(--weight-medium);
  line-height: 1.5;
}

.profile-info__pwd-link {
  border: none;
  background: none;
  color: var(--color-accent);
  font-size: var(--text-xs);
  font-family: inherit;
  font-weight: var(--weight-medium);
  cursor: pointer;
  transition: color var(--duration-fast) var(--ease);
}

.profile-info__pwd-link:hover {
  color: var(--color-accent-hover);
}

/* ---- 密码表单 ---- */

.pwd-form {
  display: flex;
  flex-direction: column;
  gap: var(--space-1);
  width: 100%;
  max-width: 280px;
}

.pwd-form__input {
  height: var(--control-height-md);
  border: 1px solid var(--color-border-strong);
  border-radius: var(--radius-control);
  padding: 0 12px;
  font-size: var(--text-sm);
  color: var(--color-text-primary);
  transition: border-color var(--duration-fast) var(--ease);
}

.pwd-form__input:focus {
  border-color: var(--color-accent);
}

.pwd-form__submit {
  height: var(--control-height-md);
  border-radius: var(--radius-control);
  border: none;
  background-color: var(--color-accent);
  color: var(--color-accent-contrast);
  font-size: var(--text-sm);
  font-weight: var(--weight-medium);
  cursor: pointer;
  transition:
    background-color var(--duration-fast) var(--ease),
    transform var(--duration-fast) var(--ease);
}

.pwd-form__submit:hover:not(:disabled) {
  background-color: var(--color-accent-hover);
  transform: translateY(-1px);
}

.pwd-form__submit:disabled {
  opacity: 0.4;
  cursor: not-allowed;
  transform: none;
}

.pwd-form__success {
  margin: 0;
  font-size: var(--text-xs);
  color: var(--color-success);
  text-align: center;
}

/* ---- 左右分栏 ---- */

.profile-columns {
  display: grid;
  grid-template-columns: 1fr 1fr;
  gap: var(--space-2);
  flex: 1;
  min-height: 0;
  width: 100%;
}

.profile-col {
  display: flex;
  flex-direction: column;
  min-height: 0;
  min-width: 0;
  border: 1px solid var(--color-border);
  border-radius: var(--radius-lg);
  background-color: var(--color-bg-surface);
  padding: var(--space-2);
  gap: var(--space-1);
}

.profile-col__title {
  margin: 0;
  display: flex;
  align-items: center;
  gap: 6px;
  font-size: var(--text-sm);
  font-weight: var(--weight-semibold);
  color: var(--color-text-primary);
  flex-shrink: 0;
}

.profile-col__title :deep(.material-icon) {
  width: 16px;
  height: 16px;
}

.profile-col__center {
  flex: 1;
  display: flex;
  align-items: center;
  justify-content: center;
}

.profile-col__empty {
  flex-direction: column;
  gap: var(--space-1);
  color: var(--color-text-tertiary);
}

.profile-col__empty p {
  margin: 0;
  font-size: var(--text-xs);
}

.profile-col__empty :deep(.material-icon) {
  width: 24px;
  height: 24px;
}

.profile-col__list {
  flex: 1;
  display: flex;
  flex-direction: column;
  gap: var(--space-1);
  overflow-y: auto;
  min-height: 0;
}

.col-item {
  padding: var(--space-1);
  border: 1px solid var(--color-border);
  border-radius: var(--radius-md);
  background-color: var(--color-bg-surface-alt);
  flex-shrink: 0;
  transition:
    border-color var(--duration-fast) var(--ease),
    box-shadow var(--duration-fast) var(--ease);
}

.col-item:hover {
  border-color: var(--color-border-strong);
  box-shadow: var(--shadow-static);
}

.col-item__title {
  display: flex;
  align-items: center;
  gap: 6px;
  font-size: var(--text-sm);
  font-weight: var(--weight-semibold);
  color: var(--color-text-primary);
  min-width: 0;
}

.col-item__status {
  font-size: var(--text-xs);
  font-weight: var(--weight-normal);
  color: var(--color-text-tertiary);
  padding: 1px 6px;
  border-radius: var(--radius-sm);
  border: 1px solid var(--color-border);
  white-space: nowrap;
  flex-shrink: 0;
}

.col-item__desc {
  font-size: var(--text-xs);
  color: var(--color-text-tertiary);
  margin-top: 2px;
  overflow: hidden;
  text-overflow: ellipsis;
  white-space: nowrap;
  min-width: 0;
}

.col-item__meta {
  font-size: var(--text-xs);
  color: var(--color-text-tertiary);
  margin-top: 2px;
  white-space: nowrap;
  overflow: hidden;
  text-overflow: ellipsis;
}

.sr-only {
  position: absolute;
  width: 1px;
  height: 1px;
  padding: 0;
  margin: -1px;
  overflow: hidden;
  clip: rect(0, 0, 0, 0);
  white-space: nowrap;
  border: 0;
}

@media (max-width: 860px) {
  .profile-columns {
    grid-template-columns: 1fr;
  }
}
</style>
