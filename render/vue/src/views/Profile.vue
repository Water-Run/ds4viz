<script setup lang="ts">
/**
 * 用户页面
 *
 * @component Profile
 */

import { computed, ref, watch } from 'vue'

import { useAuthStore } from '@/stores/auth'
import { uploadAvatarApi } from '@/api/users'
import { fetchFavoritesApi, fetchExecutionHistoryApi } from '@/api/users'
import { getUserAvatarUrl } from '@/api/users'
import { extractErrorMessage } from '@/utils/error'
import { formatDateTime } from '@/utils/time'
import Pagination from '@/components/common/Pagination.vue'
import ErrorBanner from '@/components/common/ErrorBanner.vue'
import Loading from '@/components/common/Loading.vue'
import MaterialIcon from '@/components/common/MaterialIcon.vue'

import type { FavoriteItem, ExecutionHistoryItem } from '@/api/users'

/** 默认头像颜色调色板（低饱和、适合白色前景文字） */
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
 * @returns 大写首字母，空值时返回 '-'
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
 * 收藏展开状态
 */
const favoritesExpanded = ref<boolean>(false)

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
 * 执行记录展开状态
 */
const executionsExpanded = ref<boolean>(false)

/**
 * 当前用户
 */
const currentUser = computed(() => authStore.currentUser)

/**
 * 是否拥有自定义头像（avatarUrl 非 null 且图片未加载失败）
 */
const hasCustomAvatar = computed<boolean>(() => {
  return currentUser.value !== null
    && currentUser.value.avatarUrl !== null
    && !avatarLoadFailed.value
})

/**
 * 头像图片地址（仅当有自定义头像时使用）
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
 * 默认头像背景色（基于用户名哈希）
 */
const userAvatarColor = computed<string>(() => {
  return getAvatarColor(currentUser.value?.username ?? '')
})

/**
 * 收藏总页数
 */
const favoritesTotalPages = computed(() => Math.max(1, Math.ceil(favoritesTotal.value / 10)))

/**
 * 执行记录总页数
 */
const executionsTotalPages = computed(() =>
  Math.max(1, Math.ceil(executionsTotal.value / 10)),
)

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
 * 头像图片加载失败，切换到默认字母头像
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
    passwordSuccess.value = '更新成功'
    oldPassword.value = ''
    newPassword.value = ''
  } catch (error: unknown) {
    passwordError.value = extractErrorMessage(error)
  }
}

/**
 * 切换收藏展开
 */
const toggleFavorites = async (): Promise<void> => {
  favoritesExpanded.value = !favoritesExpanded.value
  if (favoritesExpanded.value) {
    await loadFavorites()
  }
}

/**
 * 切换执行记录展开
 */
const toggleExecutions = async (): Promise<void> => {
  executionsExpanded.value = !executionsExpanded.value
  if (executionsExpanded.value) {
    await loadExecutions()
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
</script>

<template>
  <div class="profile-page">
    <header class="profile-page__header">
      <div class="profile-page__title">
        <MaterialIcon name="person" :size="18" />
        <span>用户</span>
      </div>
    </header>

    <section class="profile-page__card">
      <div class="profile-card__avatar">
        <img
          v-if="hasCustomAvatar"
          :src="avatarSrc"
          alt="avatar"
          class="profile-card__avatar-img"
          @error="handleAvatarError"
        />
        <div
          v-else
          class="profile-card__default-avatar"
          :style="{ backgroundColor: userAvatarColor }"
        >
          {{ userInitial }}
        </div>
        <label class="avatar-upload">
          <MaterialIcon name="photo_camera" :size="18" />
          <input
            class="sr-only"
            type="file"
            accept="image/*"
            @change="handleAvatarChange"
          />
        </label>
      </div>
      <div class="profile-card__info">
        <div class="profile-card__name">{{ currentUser?.username }}</div>
        <div class="profile-card__meta">ID {{ currentUser?.id }}</div>
        <div class="profile-card__meta">状态 {{ currentUser?.status }}</div>
      </div>
      <div class="profile-card__status">
        <span v-if="avatarUploading" class="profile-card__hint">上传中</span>
        <span v-else class="profile-card__hint">更新头像</span>
        <ErrorBanner :message="avatarError" @dismiss="avatarError = ''" />
      </div>
    </section>

    <section class="profile-page__card">
      <h3 class="card-title">更新密码</h3>
      <div class="form-row">
        <label>旧密码</label>
        <input v-model="oldPassword" type="password" />
      </div>
      <div class="form-row">
        <label>新密码</label>
        <input v-model="newPassword" type="password" />
      </div>
      <button class="primary-btn" @click="handlePasswordChange">更新密码</button>
      <ErrorBanner :message="passwordError" @dismiss="passwordError = ''" />
      <p v-if="passwordSuccess" class="success-text">{{ passwordSuccess }}</p>
    </section>

    <section class="profile-page__card">
      <div class="card-header">
        <h3 class="card-title">收藏模板</h3>
        <button class="text-btn" @click="toggleFavorites">
          {{ favoritesExpanded ? '收起' : '展开' }}
        </button>
      </div>
      <Loading v-if="favoritesLoading" message="加载中" />
      <ErrorBanner :message="favoritesError" @dismiss="favoritesError = ''" />
      <div v-if="favoritesExpanded" class="list">
        <div v-for="item in favorites" :key="item.templateId" class="list-item">
          <div class="list-item__main">
            <div class="list-item__title">{{ item.title }}</div>
            <div class="list-item__desc">{{ item.description }}</div>
          </div>
          <div class="list-item__meta">收藏时间 {{ formatDateTime(item.favoritedAt) }}</div>
        </div>
        <Pagination
          :page="favoritesPage"
          :total-pages="favoritesTotalPages"
          @change="handleFavoritesPage"
        />
      </div>
    </section>

    <section class="profile-page__card">
      <div class="card-header">
        <h3 class="card-title">执行记录</h3>
        <button class="text-btn" @click="toggleExecutions">
          {{ executionsExpanded ? '收起' : '展开' }}
        </button>
      </div>
      <Loading v-if="executionsLoading" message="加载中" />
      <ErrorBanner :message="executionsError" @dismiss="executionsError = ''" />
      <div v-if="executionsExpanded" class="list">
        <div v-for="item in executions" :key="item.id" class="list-item">
          <div class="list-item__main">
            <div class="list-item__title">{{ item.language }}</div>
            <div class="list-item__desc">{{ item.code }}</div>
          </div>
          <div class="list-item__meta">{{ formatDateTime(item.createdAt) }}</div>
        </div>
        <Pagination
          :page="executionsPage"
          :total-pages="executionsTotalPages"
          @change="handleExecutionsPage"
        />
      </div>
    </section>
  </div>
</template>

<style scoped>
.profile-page {
  display: flex;
  flex-direction: column;
  gap: var(--space-2);
  padding: var(--space-3);
  height: 100%;
  overflow: auto;
}

.profile-page__header {
  display: flex;
  align-items: center;
  justify-content: space-between;
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

.profile-page__card {
  border: 1px solid var(--color-border);
  border-radius: var(--radius-lg);
  padding: var(--space-3);
  background-color: var(--color-bg-surface);
  display: flex;
  flex-direction: column;
  gap: var(--space-2);
}

/* ---- 头像区域 ---- */

.profile-card__avatar {
  position: relative;
  width: 88px;
  height: 88px;
}

.profile-card__avatar-img {
  width: 88px;
  height: 88px;
  border-radius: 16px;
  object-fit: cover;
  border: 1px solid var(--color-border-strong);
}

.profile-card__default-avatar {
  width: 88px;
  height: 88px;
  border-radius: 16px;
  display: flex;
  align-items: center;
  justify-content: center;
  color: #ffffff;
  font-size: 36px;
  font-weight: 600;
  line-height: 1;
  user-select: none;
}

.avatar-upload {
  position: absolute;
  right: -6px;
  bottom: -6px;
  width: 28px;
  height: 28px;
  display: flex;
  align-items: center;
  justify-content: center;
  border-radius: 10px;
  border: 1px solid var(--color-border);
  background-color: var(--color-bg-surface);
  cursor: pointer;
  transition: background-color var(--duration-fast) var(--ease);
}

.avatar-upload:hover {
  background-color: var(--color-bg-hover);
}

.avatar-upload :deep(.material-icon) {
  width: 18px;
  height: 18px;
}

/* ---- 信息区域 ---- */

.profile-card__info {
  display: flex;
  flex-direction: column;
  gap: 4px;
}

.profile-card__name {
  font-size: var(--text-base);
  font-weight: var(--weight-semibold);
}

.profile-card__meta {
  font-size: var(--text-xs);
  color: var(--color-text-tertiary);
}

.profile-card__status {
  display: flex;
  flex-direction: column;
  gap: 4px;
}

.profile-card__hint {
  font-size: var(--text-xs);
  color: var(--color-text-tertiary);
}

/* ---- 通用卡片元素 ---- */

.card-title {
  margin: 0;
  font-size: var(--text-sm);
  font-weight: var(--weight-semibold);
}

.card-header {
  display: flex;
  align-items: center;
  justify-content: space-between;
}

.form-row {
  display: flex;
  flex-direction: column;
  gap: 6px;
}

.form-row input {
  height: var(--control-height-md);
  border: 1px solid var(--color-border-strong);
  border-radius: var(--radius-control);
  padding: 0 12px;
}

.primary-btn {
  height: var(--control-height-md);
  border-radius: var(--radius-control);
  border: none;
  background-color: var(--color-accent);
  color: var(--color-accent-contrast);
  font-weight: var(--weight-medium);
  transition:
    background-color var(--duration-fast) var(--ease),
    transform var(--duration-fast) var(--ease);
}

.primary-btn:hover {
  background-color: var(--color-accent-hover);
  transform: translateY(-1px);
}

.primary-btn:disabled {
  background-color: var(--color-border-strong);
  color: var(--color-text-tertiary);
  cursor: not-allowed;
  transform: none;
}

.text-btn {
  border: none;
  background: none;
  color: var(--color-accent);
  font-size: var(--text-xs);
  cursor: pointer;
  transition: color var(--duration-fast) var(--ease);
}

.text-btn:hover {
  color: var(--color-accent-hover);
}

.success-text {
  font-size: var(--text-xs);
  color: var(--color-success);
}

/* ---- 列表 ---- */

.list {
  display: flex;
  flex-direction: column;
  gap: var(--space-1);
}

.list-item {
  display: flex;
  align-items: flex-start;
  justify-content: space-between;
  padding: var(--space-2);
  border: 1px solid var(--color-border);
  border-radius: var(--radius-md);
  background-color: var(--color-bg-surface-alt);
  transition:
    border-color var(--duration-fast) var(--ease),
    box-shadow var(--duration-fast) var(--ease),
    transform var(--duration-fast) var(--ease);
}

.list-item:hover {
  border-color: var(--color-border-strong);
  box-shadow: 0 10px 26px rgba(0, 0, 0, 0.1);
  transform: translateY(-1px);
}

.list-item__title {
  font-size: var(--text-sm);
  font-weight: var(--weight-semibold);
}

.list-item__desc {
  font-size: var(--text-xs);
  color: var(--color-text-tertiary);
}

.list-item__meta {
  font-size: var(--text-xs);
  color: var(--color-text-tertiary);
  white-space: nowrap;
}

.sr-only {
  position: absolute;
  width: 1px;
  height: 1px;
  padding: 0;
  margin: -1px;
  overflow: hidden;
  clip: rect(0, 0, 0, 0);
  border: 0;
}
</style>