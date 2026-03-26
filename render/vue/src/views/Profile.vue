<!-- src/views/Profile.vue -->
<script setup lang="ts">
/**
 * 用户页面
 *
 * 顶部用户信息 + 对称信息区（收藏模板 / 执行记录）；
 * 执行记录区右上角提供“统计数据”入口，点击后弹出统计浮层。
 * 统计仅在首次访问时懒加载，并支持“统计中”占位状态。
 *
 * @file src/views/Profile.vue
 * @author WaterRun
 * @date 2026-03-26
 * @component Profile
 */

import {
  computed,
  nextTick,
  onBeforeUnmount,
  onMounted,
  ref,
  watch,
} from 'vue'
import type { Ref } from 'vue'
import { useRouter } from 'vue-router'

import { useAuthStore } from '@/stores/auth'
import { uploadAvatarApi } from '@/api/users'
import { fetchFavoritesApi, fetchExecutionHistoryApi } from '@/api/users'
import { getUserAvatarUrl } from '@/api/users'
import { unfavoriteTemplateApi } from '@/api/templates'
import { extractErrorMessage } from '@/utils/error'
import { formatDateTime, formatDuration } from '@/utils/time'
import { validatePassword } from '@/utils/validation'
import { LANGUAGE_LABELS, LANGUAGES } from '@/types/api'
import type { Language } from '@/types/api'
import Pagination from '@/components/common/Pagination.vue'
import ErrorBanner from '@/components/common/ErrorBanner.vue'
import Loading from '@/components/common/Loading.vue'
import MaterialIcon from '@/components/common/MaterialIcon.vue'
import CodeEditor from '@/components/editor/CodeEditor.vue'
import TemplateDetailPanel from '@/components/common/TemplateDetailPanel.vue'

import type { FavoriteItem, ExecutionHistoryItem } from '@/api/users'

/* ---- 常量 ---- */

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

interface StatusStyle {
  color: string
  bg: string
}

const STATUS_STYLES: Record<string, StatusStyle> = {
  active: {
    color: 'var(--color-success)',
    bg: 'var(--color-success-muted)',
  },
  inactive: {
    color: 'var(--color-warning)',
    bg: 'var(--color-warning-muted)',
  },
  banned: {
    color: 'var(--color-error)',
    bg: 'var(--color-error-muted)',
  },
  disabled: {
    color: 'var(--color-error)',
    bg: 'var(--color-error-muted)',
  },
}

const DEFAULT_STATUS_STYLE: StatusStyle = {
  color: 'var(--color-text-tertiary)',
  bg: 'var(--color-bg-hover)',
}

const EXEC_STATUS_COLORS: Record<string, string> = {
  success: 'var(--color-success)',
  error: 'var(--color-error)',
  timeout: 'var(--color-warning)',
}

const LANG_COLORS: Record<string, string> = {
  python: '#3572a5',
  lua: '#000080',
  rust: '#dea584',
  c: '#555555',
}

const STATUS_CHART_COLORS: Record<string, string> = {
  success: '#22c55e',
  error: '#ef4444',
  timeout: '#f59e0b',
}

/* ---- 工具函数 ---- */

function getAvatarColor(username: string): string {
  let hash = 0
  for (let index = 0; index < username.length; index += 1) {
    hash = ((hash << 5) - hash + username.charCodeAt(index)) | 0
  }
  return AVATAR_COLORS[Math.abs(hash) % AVATAR_COLORS.length]
}

function getAvatarInitial(username: string): string {
  return username.length > 0 ? username.charAt(0).toUpperCase() : '-'
}

function getLineCount(code: string): number {
  return code.split('\n').length
}

function getExecStatusColor(status: string): string {
  return EXEC_STATUS_COLORS[status.toLowerCase()] ?? 'var(--color-text-tertiary)'
}

function getExecStatusBg(status: string): string {
  const key = status.toLowerCase()
  if (key === 'success') return 'var(--color-success-muted)'
  if (key === 'error') return 'var(--color-error-muted)'
  if (key === 'timeout') return 'var(--color-warning-muted)'
  return 'var(--color-bg-hover)'
}

/**
 * 触发输入框振动动画
 *
 * @param target - 控制振动的状态引用
 */
async function triggerShake(target: Ref<boolean>): Promise<void> {
  target.value = false
  await nextTick()
  target.value = true
  window.setTimeout(() => {
    target.value = false
  }, 420)
}

/* ---- 饼图计算 ---- */

interface PieSlice {
  label: string
  value: number
  color: string
  percentage: number
  pathD: string
}

/**
 * 计算饼图扇区 SVG path
 *
 * @param items - 统计项
 * @returns 扇区列表
 */
function computePieSlices(
  items: Array<{ label: string; value: number; color: string }>,
): PieSlice[] {
  const filtered = items.filter((item) => item.value > 0)
  const total = filtered.reduce((sum, item) => sum + item.value, 0)

  if (total === 0) return []

  const centerX = 50
  const centerY = 50
  const radius = 40
  const slices: PieSlice[] = []
  let startAngle = -Math.PI / 2

  for (const item of filtered) {
    const ratio = item.value / total
    const percentage = Math.round(ratio * 100)
    const angle = ratio * Math.PI * 2
    const endAngle = startAngle + angle

    let pathD = ''
    if (filtered.length === 1) {
      pathD = `M ${centerX} ${centerY - radius} A ${radius} ${radius} 0 1 1 ${centerX - 0.001} ${centerY - radius} Z`
    } else {
      const x1 = centerX + radius * Math.cos(startAngle)
      const y1 = centerY + radius * Math.sin(startAngle)
      const x2 = centerX + radius * Math.cos(endAngle)
      const y2 = centerY + radius * Math.sin(endAngle)
      const largeArc = angle > Math.PI ? 1 : 0
      pathD = `M ${centerX} ${centerY} L ${x1} ${y1} A ${radius} ${radius} 0 ${largeArc} 1 ${x2} ${y2} Z`
    }

    slices.push({
      label: item.label,
      value: item.value,
      color: item.color,
      percentage,
      pathD,
    })

    startAngle = endAngle
  }

  return slices
}

/* ---- 核心状态 ---- */

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
const shakeNewPassword = ref<boolean>(false)

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

const expandedExecId = ref<number | null>(null)
const copiedExecId = ref<number | null>(null)
const selectedFavoriteId = ref<number | null>(null)

/* ---- 统计相关状态（懒加载） ---- */

const statsPopoverOpen = ref<boolean>(false)
const statsRequested = ref<boolean>(false)
const statsLoading = ref<boolean>(false)
const statsError = ref<string>('')

const allExecItems = ref<ExecutionHistoryItem[]>([])
const statsTotalFavorites = ref<number>(0)

const statsPopoverRef = ref<HTMLElement | null>(null)

/* ---- 派生状态 ---- */

const currentUser = computed(() => authStore.currentUser)

const hasCustomAvatar = computed<boolean>(() => {
  return (
    currentUser.value !== null
    && currentUser.value.avatarUrl !== null
    && !avatarLoadFailed.value
  )
})

const avatarSrc = computed<string>(() => {
  if (!currentUser.value || currentUser.value.avatarUrl === null) return ''
  return getUserAvatarUrl(currentUser.value.id)
})

const userInitial = computed<string>(() => getAvatarInitial(currentUser.value?.username ?? ''))
const userAvatarColor = computed<string>(() => getAvatarColor(currentUser.value?.username ?? ''))

const statusStyle = computed<StatusStyle>(() => {
  const status = (currentUser.value?.status ?? '').toLowerCase()
  return STATUS_STYLES[status] ?? DEFAULT_STATUS_STYLE
})

const currentPlaygroundLanguage = computed<Language>(() => {
  const stored = localStorage.getItem('ds4viz_language') as Language | null
  if (stored && LANGUAGES.includes(stored)) return stored
  return 'python'
})

const favoritesTotalPages = computed<number>(() => {
  return Math.max(1, Math.ceil(favoritesTotal.value / 10))
})

const executionsTotalPages = computed<number>(() => {
  return Math.max(1, Math.ceil(executionsTotal.value / 10))
})

const newPwdValidation = computed(() => validatePassword(newPassword.value))

/* ---- 统计派生 ---- */

const langDistributionData = computed(() => {
  const counter = new Map<string, number>()
  for (const item of allExecItems.value) {
    counter.set(item.language, (counter.get(item.language) ?? 0) + 1)
  }
  return Array.from(counter.entries()).map(([lang, count]) => ({
    label: LANGUAGE_LABELS[lang as Language] ?? lang,
    value: count,
    color: LANG_COLORS[lang] ?? '#999',
  }))
})

const statusDistributionData = computed(() => {
  const counter = new Map<string, number>()
  for (const item of allExecItems.value) {
    const status = item.status.toLowerCase()
    counter.set(status, (counter.get(status) ?? 0) + 1)
  }
  return Array.from(counter.entries()).map(([status, count]) => ({
    label: status.charAt(0).toUpperCase() + status.slice(1),
    value: count,
    color: STATUS_CHART_COLORS[status] ?? '#999',
  }))
})

const langPieSlices = computed(() => computePieSlices(langDistributionData.value))
const statusPieSlices = computed(() => computePieSlices(statusDistributionData.value))

const avgExecTime = computed<string>(() => {
  const valid = allExecItems.value.filter((item) => item.executionTime !== null)
  if (valid.length === 0) return '--'
  const sum = valid.reduce((acc, item) => acc + (item.executionTime ?? 0), 0)
  return formatDuration(Math.round(sum / valid.length))
})

const statsReady = computed<boolean>(() => {
  return !statsLoading.value && statsRequested.value && statsError.value.length === 0
})

/* ---- 监听 ---- */

watch(
  () => currentUser.value?.avatarUrl,
  () => {
    avatarLoadFailed.value = false
  },
)

/* ---- 数据加载 ---- */

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

/**
 * 统计懒加载：仅在用户访问统计浮层时执行
 */
const loadStats = async (): Promise<void> => {
  if (!currentUser.value) return

  statsRequested.value = true
  statsLoading.value = true
  statsError.value = ''

  try {
    const fullExecutions: ExecutionHistoryItem[] = []
    let page = 1
    const limit = 100
    let hasMore = true

    while (hasMore) {
      const result = await fetchExecutionHistoryApi(currentUser.value.id, { page, limit })
      fullExecutions.push(...result.items)
      hasMore = fullExecutions.length < result.total
      page += 1
    }

    allExecItems.value = fullExecutions

    const favoriteResult = await fetchFavoritesApi(currentUser.value.id, {
      page: 1,
      limit: 1,
    })
    statsTotalFavorites.value = favoriteResult.total
  } catch (error: unknown) {
    statsError.value = extractErrorMessage(error)
  } finally {
    statsLoading.value = false
  }
}

/* ---- 事件 ---- */

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

  if (!newPwdValidation.value.valid) {
    await triggerShake(shakeNewPassword)
    passwordError.value = '新密码不满足复杂度要求'
    return
  }

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

const handleFavoritesPage = async (targetPage: number): Promise<void> => {
  favoritesPage.value = targetPage
  selectedFavoriteId.value = null
  await loadFavorites()
}

const handleExecutionsPage = async (targetPage: number): Promise<void> => {
  executionsPage.value = targetPage
  expandedExecId.value = null
  await loadExecutions()
}

const toggleExecExpand = (id: number): void => {
  expandedExecId.value = expandedExecId.value === id ? null : id
}

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
    /* clipboard 不可用 */
  }
}

const handleEditInPlayground = (item: ExecutionHistoryItem): void => {
  try {
    sessionStorage.setItem('ds4viz_edit_code', item.code)
    sessionStorage.setItem('ds4viz_edit_language', item.language)
  } catch {
    /* sessionStorage 不可用 */
  }
  router.push({ name: 'playground' })
}

const handleSelectFavorite = (templateId: number): void => {
  selectedFavoriteId.value = selectedFavoriteId.value === templateId ? null : templateId
}

const handleCloseFavoritePanel = (): void => {
  selectedFavoriteId.value = null
}

const handleFavoritePanelToggle = async (templateId: number): Promise<void> => {
  try {
    await unfavoriteTemplateApi(templateId)
    favorites.value = favorites.value.filter((item) => item.templateId !== templateId)
    favoritesTotal.value = Math.max(0, favoritesTotal.value - 1)
    selectedFavoriteId.value = null
  } catch (error: unknown) {
    favoritesError.value = extractErrorMessage(error)
  }
}

const handleUnfavoriteFromCard = async (
  templateId: number,
  event: Event,
): Promise<void> => {
  event.stopPropagation()
  try {
    await unfavoriteTemplateApi(templateId)
    favorites.value = favorites.value.filter((item) => item.templateId !== templateId)
    favoritesTotal.value = Math.max(0, favoritesTotal.value - 1)
    if (selectedFavoriteId.value === templateId) {
      selectedFavoriteId.value = null
    }
  } catch (error: unknown) {
    favoritesError.value = extractErrorMessage(error)
  }
}

/**
 * 打开/关闭统计浮层（点击触发）
 */
const handleToggleStatsPopover = async (): Promise<void> => {
  statsPopoverOpen.value = !statsPopoverOpen.value

  if (statsPopoverOpen.value && !statsRequested.value) {
    await loadStats()
  }
}

/**
 * 点击外部关闭统计浮层
 */
const handleClickOutside = (event: MouseEvent): void => {
  if (!statsPopoverOpen.value) return
  const target = event.target as Node
  if (statsPopoverRef.value && !statsPopoverRef.value.contains(target)) {
    statsPopoverOpen.value = false
  }
}

onMounted(async () => {
  await Promise.all([loadFavorites(), loadExecutions()])
  document.addEventListener('mousedown', handleClickOutside)
})

onBeforeUnmount(() => {
  document.removeEventListener('mousedown', handleClickOutside)
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

    <!-- 用户信息 -->
    <section class="profile-hero">
      <div class="profile-hero__avatar">
        <img v-if="hasCustomAvatar" :src="avatarSrc" alt="avatar" class="profile-hero__avatar-img"
          @error="handleAvatarError" />
        <div v-else class="profile-hero__avatar-default" :style="{ backgroundColor: userAvatarColor }">
          {{ userInitial }}
        </div>
        <label class="profile-hero__avatar-upload" :class="{ 'profile-hero__avatar-upload--busy': avatarUploading }">
          <MaterialIcon name="photo_camera" :size="16" />
          <input class="sr-only" type="file" accept="image/*" :disabled="avatarUploading"
            @change="handleAvatarChange" />
        </label>
      </div>

      <ErrorBanner :message="avatarError" @dismiss="avatarError = ''" />

      <div class="profile-hero__name">{{ currentUser?.username }}</div>
      <div class="profile-hero__id">ID {{ currentUser?.id }}</div>
      <span class="profile-hero__status-badge" :style="{ color: statusStyle.color, backgroundColor: statusStyle.bg }">
        {{ currentUser?.status }}
      </span>

      <button class="profile-hero__pwd-link" @click="showPasswordForm = !showPasswordForm">
        {{ showPasswordForm ? '取消' : '修改密码' }}
      </button>

      <Transition name="slide-fade">
        <div v-if="showPasswordForm" class="pwd-form">
          <input v-model="oldPassword" type="password" class="pwd-form__input" placeholder="旧密码" />
          <input v-model="newPassword" type="password" class="pwd-form__input" :class="{
            'pwd-form__input--shake': shakeNewPassword,
            'pwd-form__input--err': shakeNewPassword,
          }" placeholder="新密码" />
          <Transition name="slide-fade">
            <div v-if="newPassword.length > 0" class="pwd-rules">
              <span :class="newPwdValidation.lengthOk ? 'pwd-rules__ok' : 'pwd-rules__pending'">
                8–32字符
              </span>
              <span :class="newPwdValidation.hasUppercase ? 'pwd-rules__ok' : 'pwd-rules__pending'">
                大写
              </span>
              <span :class="newPwdValidation.hasLowercase ? 'pwd-rules__ok' : 'pwd-rules__pending'">
                小写
              </span>
              <span :class="newPwdValidation.hasDigit ? 'pwd-rules__ok' : 'pwd-rules__pending'">
                数字
              </span>
              <span :class="newPwdValidation.hasSpecial ? 'pwd-rules__ok' : 'pwd-rules__pending'">
                特殊字符
              </span>
            </div>
          </Transition>
          <button class="pwd-form__submit" :disabled="!oldPassword.length || !newPwdValidation.valid"
            @click="handlePasswordChange">
            确认修改
          </button>
          <ErrorBanner :message="passwordError" @dismiss="passwordError = ''" />
          <p v-if="passwordSuccess" class="pwd-form__success">{{ passwordSuccess }}</p>
        </div>
      </Transition>
    </section>

    <!-- 收藏展开详情 -->
    <Transition name="slide-fade">
      <TemplateDetailPanel v-if="selectedFavoriteId !== null" :template-id="selectedFavoriteId"
        :default-language="currentPlaygroundLanguage" @close="handleCloseFavoritePanel"
        @toggle-favorite="handleFavoritePanelToggle" />
    </Transition>

    <!-- 对称信息区 -->
    <section class="profile-sections">
      <!-- 收藏模板 -->
      <article class="profile-section">
        <header class="profile-section__header">
          <h3 class="profile-section__title">
            <MaterialIcon name="favorite" :size="16" />
            <span>收藏模板</span>
          </h3>
        </header>

        <ErrorBanner :message="favoritesError" @dismiss="favoritesError = ''" />

        <div v-if="favoritesLoading" class="profile-section__center">
          <Loading />
        </div>

        <div v-else-if="favorites.length === 0" class="profile-section__center profile-section__empty">
          <MaterialIcon name="bookmark" :size="24" />
          <p>暂无收藏</p>
        </div>

        <div v-else class="profile-section__list">
          <div v-for="item in favorites" :key="item.templateId" class="fav-card"
            :class="{ 'fav-card--selected': selectedFavoriteId === item.templateId }"
            @click="handleSelectFavorite(item.templateId)">
            <div class="fav-card__header">
              <span class="fav-card__title">{{ item.title }}</span>
              <span class="fav-card__category">{{ item.category }}</span>
            </div>
            <div class="fav-card__desc">{{ item.description }}</div>
            <div class="fav-card__footer">
              <span class="fav-card__meta">
                收藏 {{ item.favoriteCount }} · {{ formatDateTime(item.favoritedAt) }}
              </span>
              <button class="fav-card__unfav" aria-label="取消收藏"
                @click="handleUnfavoriteFromCard(item.templateId, $event)">
                <svg viewBox="0 0 24 24" fill="currentColor">
                  <path
                    d="M12 21.35l-1.45-1.32C5.4 15.36 2 12.28 2 8.5 2 5.42 4.42 3 7.5 3c1.74 0 3.41.81 4.5 2.09C13.09 3.81 14.76 3 16.5 3 19.58 3 22 5.42 22 8.5c0 3.78-3.4 6.86-8.55 11.54L12 21.35z" />
                </svg>
              </button>
            </div>
          </div>
        </div>

        <Pagination v-if="favoritesTotal > 10" :page="favoritesPage" :total-pages="favoritesTotalPages"
          @change="handleFavoritesPage" />
      </article>

      <!-- 执行记录 -->
      <article class="profile-section profile-section--execution" ref="statsPopoverRef">
        <header class="profile-section__header">
          <h3 class="profile-section__title">
            <MaterialIcon name="history" :size="16" />
            <span>执行记录</span>
          </h3>
          <button class="stats-entry" @click.stop="handleToggleStatsPopover">
            统计数据
          </button>
        </header>

        <!-- 统计浮层 -->
        <Transition name="fade">
          <div v-if="statsPopoverOpen" class="stats-popover" @click.stop>
            <div class="stats-popover__header">
              <span>统计数据</span>
              <button class="stats-popover__close" @click="statsPopoverOpen = false">
                <MaterialIcon name="close" :size="14" />
              </button>
            </div>

            <ErrorBanner :message="statsError" @dismiss="statsError = ''" />

            <div v-if="statsLoading" class="stats-popover__loading">
              <Loading message="统计中…" />
            </div>

            <div v-else-if="statsRequested && allExecItems.length === 0" class="stats-popover__empty">
              <MaterialIcon name="analytics" :size="20" />
              <span>暂无可统计数据</span>
            </div>

            <div v-else-if="statsReady" class="stats-popover__body">
              <div class="stats-summary">
                <div class="stats-summary__item">
                  <span class="stats-summary__value">{{ allExecItems.length }}</span>
                  <span class="stats-summary__label">总执行次数</span>
                </div>
                <div class="stats-summary__item">
                  <span class="stats-summary__value">{{ avgExecTime }}</span>
                  <span class="stats-summary__label">平均执行耗时</span>
                </div>
                <div class="stats-summary__item">
                  <span class="stats-summary__value">{{ statsTotalFavorites }}</span>
                  <span class="stats-summary__label">收藏总数</span>
                </div>
              </div>

              <div class="stats-charts">
                <div class="stats-chart">
                  <div class="stats-chart__title">状态分布</div>
                  <svg v-if="statusPieSlices.length > 1" class="stats-chart__svg" viewBox="0 0 100 100">
                    <path v-for="(slice, index) in statusPieSlices" :key="`sp-${index}`" :d="slice.pathD"
                      :fill="slice.color" />
                  </svg>
                  <div v-else-if="statusPieSlices.length === 1" class="stats-chart__single"
                    :style="{ color: statusPieSlices[0].color }">
                    <span class="stats-chart__single-dot" :style="{ backgroundColor: statusPieSlices[0].color }" />
                    {{ statusPieSlices[0].label }} 100%
                  </div>
                  <div class="stats-chart__legend">
                    <span v-for="(slice, index) in statusPieSlices" :key="`sl-${index}`" class="legend-item">
                      <span class="legend-item__dot" :style="{ backgroundColor: slice.color }" />
                      {{ slice.label }} {{ slice.percentage }}%
                    </span>
                  </div>
                </div>

                <div class="stats-chart">
                  <div class="stats-chart__title">语言分布</div>
                  <svg v-if="langPieSlices.length > 1" class="stats-chart__svg" viewBox="0 0 100 100">
                    <path v-for="(slice, index) in langPieSlices" :key="`lp-${index}`" :d="slice.pathD"
                      :fill="slice.color" />
                  </svg>
                  <div v-else-if="langPieSlices.length === 1" class="stats-chart__single"
                    :style="{ color: langPieSlices[0].color }">
                    <span class="stats-chart__single-dot" :style="{ backgroundColor: langPieSlices[0].color }" />
                    {{ langPieSlices[0].label }} 100%
                  </div>
                  <div class="stats-chart__legend">
                    <span v-for="(slice, index) in langPieSlices" :key="`ll-${index}`" class="legend-item">
                      <span class="legend-item__dot" :style="{ backgroundColor: slice.color }" />
                      {{ slice.label }} {{ slice.percentage }}%
                    </span>
                  </div>
                </div>
              </div>
            </div>
          </div>
        </Transition>

        <ErrorBanner :message="executionsError" @dismiss="executionsError = ''" />

        <div v-if="executionsLoading" class="profile-section__center">
          <Loading />
        </div>

        <div v-else-if="executions.length === 0" class="profile-section__center profile-section__empty">
          <MaterialIcon name="schedule" :size="24" />
          <p>暂无记录</p>
        </div>

        <div v-else class="profile-section__list">
          <div v-for="item in executions" :key="item.id" class="exec-item"
            :class="{ 'exec-item--expanded': expandedExecId === item.id }">
            <button class="exec-item__summary" @click="toggleExecExpand(item.id)">
              <span class="exec-item__lang">{{ LANGUAGE_LABELS[item.language as Language] ?? item.language }}</span>
              <span class="exec-item__status" :style="{
                color: getExecStatusColor(item.status),
                backgroundColor: getExecStatusBg(item.status),
              }">
                {{ item.status }}
              </span>
              <span class="exec-item__duration">
                {{ item.executionTime !== null ? formatDuration(item.executionTime) : '--' }}
              </span>
              <span class="exec-item__lines">{{ getLineCount(item.code) }} 行</span>
              <span class="exec-item__time">{{ formatDateTime(item.createdAt) }}</span>
              <MaterialIcon :name="expandedExecId === item.id ? 'expand_less' : 'expand_more'" :size="16"
                class="exec-item__chevron" />
            </button>

            <Transition name="slide-fade">
              <div v-if="expandedExecId === item.id" class="exec-item__detail">
                <div class="exec-item__editor">
                  <CodeEditor :model-value="item.code" :language="item.language as Language" :readonly="true" />
                </div>
                <div class="exec-item__actions">
                  <button class="code-action-btn" :class="{ 'code-action-btn--copied': copiedExecId === item.id }"
                    @click.stop="handleCopyCode(item)">
                    <svg v-if="copiedExecId !== item.id" viewBox="0 0 24 24" fill="currentColor">
                      <path
                        d="M16 1H4c-1.1 0-2 .9-2 2v14h2V3h12V1zm3 4H8c-1.1 0-2 .9-2 2v14c0 1.1.9 2 2 2h11c1.1 0 2-.9 2-2V7c0-1.1-.9-2-2-2zm0 16H8V7h11v14z" />
                    </svg>
                    <svg v-else viewBox="0 0 24 24" fill="currentColor">
                      <path d="M9 16.17L4.83 12l-1.42 1.41L9 19 21 7l-1.41-1.41z" />
                    </svg>
                    <span>{{ copiedExecId === item.id ? '已复制' : '复制代码' }}</span>
                  </button>
                  <button class="code-action-btn code-action-btn--primary" @click.stop="handleEditInPlayground(item)">
                    <svg viewBox="0 0 24 24" fill="currentColor">
                      <path
                        d="M9.4 16.6L4.8 12l4.6-4.6L8 6l-6 6 6 6 1.4-1.4zm5.2 0l4.6-4.6-4.6-4.6L16 6l6 6-6 6-1.4-1.4z" />
                    </svg>
                    <span>在编辑器中打开</span>
                  </button>
                </div>
              </div>
            </Transition>
          </div>
        </div>

        <Pagination v-if="executionsTotal > 10" :page="executionsPage" :total-pages="executionsTotalPages"
          @change="handleExecutionsPage" />
      </article>
    </section>
  </div>
</template>

<style scoped>
@keyframes shake {

  0%,
  100% {
    transform: translateX(0);
  }

  12% {
    transform: translateX(-6px);
  }

  28% {
    transform: translateX(5px);
  }

  44% {
    transform: translateX(-4px);
  }

  60% {
    transform: translateX(3px);
  }

  76% {
    transform: translateX(-2px);
  }

  90% {
    transform: translateX(1px);
  }
}

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

.profile-hero {
  display: flex;
  flex-direction: column;
  align-items: center;
  gap: var(--space-1);
  flex-shrink: 0;
  padding: var(--space-4) var(--space-2) var(--space-3);
  width: 100%;
}

.profile-hero__avatar {
  position: relative;
  width: 96px;
  height: 96px;
}

.profile-hero__avatar-img {
  width: 96px;
  height: 96px;
  border-radius: 50%;
  object-fit: cover;
  border: 2px solid var(--color-border-strong);
}

.profile-hero__avatar-default {
  width: 96px;
  height: 96px;
  border-radius: 50%;
  display: flex;
  align-items: center;
  justify-content: center;
  color: #ffffff;
  font-size: 36px;
  font-weight: 600;
  line-height: 1;
  user-select: none;
}

.profile-hero__avatar-upload {
  position: absolute;
  right: -2px;
  bottom: -2px;
  width: 30px;
  height: 30px;
  display: flex;
  align-items: center;
  justify-content: center;
  border-radius: 50%;
  border: 1px solid var(--color-border);
  background-color: var(--color-bg-surface);
  cursor: pointer;
  transition: background-color var(--duration-fast) var(--ease);
}

.profile-hero__avatar-upload:hover {
  background-color: var(--color-bg-hover);
}

.profile-hero__avatar-upload--busy {
  opacity: 0.5;
  cursor: not-allowed;
}

.profile-hero__avatar-upload :deep(.material-icon) {
  width: 16px;
  height: 16px;
}

.profile-hero__name {
  font-size: var(--text-xl);
  font-weight: var(--weight-semibold);
  color: var(--color-text-primary);
  margin-top: var(--space-1);
}

.profile-hero__id {
  font-size: var(--text-sm);
  color: var(--color-text-tertiary);
  font-family: var(--font-mono);
}

.profile-hero__status-badge {
  display: inline-block;
  padding: 2px 12px;
  border-radius: var(--radius-full);
  font-size: var(--text-xs);
  font-weight: var(--weight-medium);
  line-height: 1.5;
}

.profile-hero__pwd-link {
  border: none;
  background: none;
  color: var(--color-accent);
  font-size: var(--text-xs);
  font-family: inherit;
  font-weight: var(--weight-medium);
  cursor: pointer;
  transition: color var(--duration-fast) var(--ease);
  margin-top: var(--space-1);
}

.profile-hero__pwd-link:hover {
  color: var(--color-accent-hover);
}

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
  transition: border-color var(--duration-fast) var(--ease), box-shadow var(--duration-fast) var(--ease);
}

.pwd-form__input:focus {
  border-color: var(--color-accent);
}

.pwd-form__input--err {
  border-color: var(--color-error);
  box-shadow: 0 0 0 3px rgba(239, 68, 68, 0.08);
}

.pwd-form__input--shake {
  animation: shake 420ms cubic-bezier(0.2, 0, 0, 1);
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
  transition: background-color var(--duration-fast) var(--ease), transform var(--duration-fast) var(--ease);
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

.pwd-rules {
  display: flex;
  flex-wrap: wrap;
  gap: 4px 8px;
}

.pwd-rules span {
  font-size: 11px;
  font-weight: 500;
}

.pwd-rules__ok {
  color: var(--color-success);
}

.pwd-rules__pending {
  color: var(--color-text-tertiary);
}

/* 对称信息区 */
.profile-sections {
  display: grid;
  grid-template-columns: 1fr 1fr;
  gap: var(--space-2);
  flex: 1;
  min-height: 360px;
}

.profile-section {
  position: relative;
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

.profile-section__header {
  display: flex;
  align-items: center;
  justify-content: space-between;
  min-height: 24px;
}

.profile-section__title {
  margin: 0;
  display: inline-flex;
  align-items: center;
  gap: 6px;
  font-size: var(--text-sm);
  font-weight: var(--weight-semibold);
  color: var(--color-text-primary);
}

.profile-section__title :deep(.material-icon) {
  width: 16px;
  height: 16px;
}

.profile-section__center {
  flex: 1;
  display: flex;
  align-items: center;
  justify-content: center;
}

.profile-section__empty {
  flex-direction: column;
  gap: var(--space-1);
  color: var(--color-text-tertiary);
}

.profile-section__empty p {
  margin: 0;
  font-size: var(--text-xs);
}

.profile-section__empty :deep(.material-icon) {
  width: 24px;
  height: 24px;
}

.profile-section__list {
  flex: 1;
  display: flex;
  flex-direction: column;
  gap: var(--space-1);
  overflow-y: auto;
  min-height: 0;
}

/* 统计入口 */
.stats-entry {
  border: none;
  background: none;
  color: var(--color-accent);
  font-size: var(--text-xs);
  font-family: inherit;
  font-weight: var(--weight-medium);
  cursor: pointer;
  padding: 0;
  transition: color var(--duration-fast) var(--ease);
}

.stats-entry:hover {
  color: var(--color-accent-hover);
  text-decoration: underline;
  text-underline-offset: 3px;
}

/* 统计浮层 */
.stats-popover {
  position: absolute;
  top: 40px;
  right: var(--space-2);
  width: min(460px, calc(100% - 32px));
  z-index: var(--z-dropdown);
  border: 1px solid var(--color-border-strong);
  border-radius: var(--radius-lg);
  background-color: var(--color-bg-surface);
  box-shadow: var(--shadow-hover);
  padding: var(--space-2);
  display: flex;
  flex-direction: column;
  gap: var(--space-1);
}

.stats-popover__header {
  display: flex;
  align-items: center;
  justify-content: space-between;
  font-size: var(--text-xs);
  font-weight: var(--weight-semibold);
  color: var(--color-text-primary);
  letter-spacing: var(--tracking-panel-head);
  text-transform: uppercase;
}

.stats-popover__close {
  width: 22px;
  height: 22px;
  border: none;
  background: none;
  color: var(--color-text-tertiary);
  border-radius: var(--radius-sm);
  display: inline-flex;
  align-items: center;
  justify-content: center;
  cursor: pointer;
  transition: background-color var(--duration-fast) var(--ease), color var(--duration-fast) var(--ease);
}

.stats-popover__close:hover {
  background-color: var(--color-bg-hover);
  color: var(--color-text-primary);
}

.stats-popover__loading,
.stats-popover__empty {
  min-height: 120px;
  display: flex;
  align-items: center;
  justify-content: center;
  color: var(--color-text-tertiary);
  gap: 6px;
  font-size: var(--text-xs);
}

.stats-popover__body {
  display: flex;
  flex-direction: column;
  gap: var(--space-2);
}

.stats-summary {
  display: grid;
  grid-template-columns: repeat(3, 1fr);
  gap: var(--space-1);
}

.stats-summary__item {
  border: 1px solid var(--color-border);
  border-radius: var(--radius-md);
  background-color: var(--color-bg-surface-alt);
  padding: 8px;
  display: flex;
  flex-direction: column;
  align-items: center;
  gap: 2px;
}

.stats-summary__value {
  font-size: var(--text-base);
  font-family: var(--font-mono);
  font-weight: var(--weight-semibold);
  color: var(--color-text-primary);
}

.stats-summary__label {
  font-size: 11px;
  color: var(--color-text-tertiary);
}

.stats-charts {
  display: grid;
  grid-template-columns: 1fr 1fr;
  gap: var(--space-1);
}

.stats-chart {
  border: 1px solid var(--color-border);
  border-radius: var(--radius-md);
  background-color: var(--color-bg-surface-alt);
  padding: 8px;
  display: flex;
  flex-direction: column;
  align-items: center;
  gap: 6px;
}

.stats-chart__title {
  font-size: 11px;
  font-weight: var(--weight-semibold);
  color: var(--color-text-primary);
}

.stats-chart__svg {
  width: 72px;
  height: 72px;
  flex-shrink: 0;
}

.stats-chart__single {
  height: 72px;
  display: inline-flex;
  align-items: center;
  gap: 6px;
  font-size: var(--text-xs);
  font-weight: var(--weight-semibold);
}

.stats-chart__single-dot {
  width: 10px;
  height: 10px;
  border-radius: 999px;
  flex-shrink: 0;
}

.stats-chart__legend {
  display: flex;
  flex-wrap: wrap;
  justify-content: center;
  gap: 2px 8px;
}

.legend-item {
  display: inline-flex;
  align-items: center;
  gap: 4px;
  font-size: 11px;
  color: var(--color-text-body);
  white-space: nowrap;
}

.legend-item__dot {
  width: 7px;
  height: 7px;
  border-radius: 999px;
  flex-shrink: 0;
}

/* 收藏卡片 */
.fav-card {
  padding: var(--space-1);
  border: 1px solid var(--color-border);
  border-radius: var(--radius-md);
  background-color: var(--color-bg-surface-alt);
  flex-shrink: 0;
  cursor: pointer;
  transition: border-color var(--duration-fast) var(--ease), box-shadow var(--duration-fast) var(--ease), transform var(--duration-fast) var(--ease);
}

.fav-card:hover {
  border-color: var(--color-border-strong);
  box-shadow: var(--shadow-static);
  transform: translateY(-1px);
}

.fav-card--selected {
  border-color: var(--color-accent);
  box-shadow: 0 0 0 2px var(--color-accent-wash);
}

.fav-card__header {
  display: flex;
  align-items: baseline;
  justify-content: space-between;
  gap: var(--space-1);
}

.fav-card__title {
  font-size: var(--text-sm);
  font-weight: var(--weight-semibold);
  color: var(--color-text-primary);
  overflow: hidden;
  text-overflow: ellipsis;
  white-space: nowrap;
  min-width: 0;
}

.fav-card__category {
  flex-shrink: 0;
  padding: 1px 6px;
  border: 1px solid var(--color-border);
  border-radius: var(--radius-full);
  font-size: var(--text-xs);
  color: var(--color-text-tertiary);
}

.fav-card__desc {
  font-size: var(--text-xs);
  color: var(--color-text-tertiary);
  margin-top: 2px;
  display: -webkit-box;
  -webkit-line-clamp: 2;
  -webkit-box-orient: vertical;
  overflow: hidden;
}

.fav-card__footer {
  display: flex;
  align-items: center;
  justify-content: space-between;
  margin-top: 4px;
}

.fav-card__meta {
  font-size: var(--text-xs);
  color: var(--color-text-tertiary);
  white-space: nowrap;
  overflow: hidden;
  text-overflow: ellipsis;
}

.fav-card__unfav {
  display: flex;
  align-items: center;
  justify-content: center;
  width: 24px;
  height: 24px;
  border: none;
  background: none;
  color: var(--color-error);
  border-radius: var(--radius-sm);
  cursor: pointer;
  flex-shrink: 0;
  transition: background-color var(--duration-fast) var(--ease);
}

.fav-card__unfav:hover {
  background-color: var(--color-error-muted);
}

.fav-card__unfav svg {
  width: 16px;
  height: 16px;
}

/* 执行项 */
.exec-item {
  border: 1px solid var(--color-border);
  border-radius: var(--radius-md);
  background-color: var(--color-bg-surface-alt);
  flex-shrink: 0;
  transition: border-color var(--duration-fast) var(--ease), box-shadow var(--duration-fast) var(--ease);
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

.exec-item__duration,
.exec-item__lines,
.exec-item__time {
  font-size: var(--text-xs);
  color: var(--color-text-tertiary);
  white-space: nowrap;
}

.exec-item__duration {
  font-family: var(--font-mono);
}

.exec-item__time {
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

/* 操作按钮 */
.code-action-btn {
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
  transition: background-color var(--duration-fast) var(--ease), border-color var(--duration-fast) var(--ease), color var(--duration-fast) var(--ease);
}

.code-action-btn:hover {
  background-color: var(--color-bg-hover);
  color: var(--color-text-primary);
}

.code-action-btn svg {
  width: 16px;
  height: 16px;
  flex-shrink: 0;
}

.code-action-btn--copied {
  border-color: var(--color-success);
  color: var(--color-success);
}

.code-action-btn--primary {
  border-color: var(--color-accent);
  background-color: var(--color-accent);
  color: var(--color-accent-contrast);
}

.code-action-btn--primary:hover {
  background-color: var(--color-accent-hover);
  border-color: var(--color-accent-hover);
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

@media (prefers-reduced-motion: reduce) {
  .pwd-form__input--shake {
    animation: none;
  }
}

@media (max-width: 980px) {
  .profile-sections {
    grid-template-columns: 1fr;
  }

  .stats-popover {
    left: var(--space-2);
    right: var(--space-2);
    width: auto;
  }

  .stats-summary {
    grid-template-columns: 1fr;
  }

  .stats-charts {
    grid-template-columns: 1fr;
  }
}
</style>