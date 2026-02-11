<script setup lang="ts">
/**
 * 通用错误提示条
 *
 * 显示错误消息，支持手动关闭。
 *
 * @component ErrorBanner
 *
 * @example
 * ```vue
 * <ErrorBanner :message="errorMsg" @dismiss="errorMsg = ''" />
 * ```
 */

/**
 * 组件属性定义
 */
interface Props {
    /** 错误消息文本，空字符串时隐藏 */
    message: string
}

/**
 * 组件事件定义
 */
interface Emits {
    /** 用户点击关闭按钮 */
    (event: 'dismiss'): void
}

defineProps<Props>()
defineEmits<Emits>()
</script>

<template>
    <Transition name="error-banner">
        <div v-if="message.length > 0" class="error-banner" role="alert">
            <span class="material-symbols-outlined error-banner__icon">
                error
            </span>
            <span class="error-banner__text">{{ message }}</span>
            <button
                class="error-banner__close"
                aria-label="关闭"
                @click="$emit('dismiss')"
            >
                <span class="material-symbols-outlined">close</span>
            </button>
        </div>
    </Transition>
</template>

<style scoped>
.error-banner {
    display: flex;
    align-items: center;
    gap: var(--space-1);
    padding: var(--space-1) var(--space-2);
    border: 1px solid var(--color-error);
    border-radius: var(--radius-control);
    background-color: var(--color-error-muted);
    color: var(--color-error);
    font-size: var(--text-sm);
    line-height: var(--leading-normal);
}

.error-banner__icon {
    font-size: 18px;
    flex-shrink: 0;
}

.error-banner__text {
    flex: 1;
    min-width: 0;
}

.error-banner__close {
    display: flex;
    align-items: center;
    justify-content: center;
    width: 24px;
    height: 24px;
    flex-shrink: 0;
    border: none;
    background: none;
    color: var(--color-error);
    border-radius: var(--radius-sm);
    cursor: pointer;
    transition: background-color var(--duration-fast) var(--ease);
}

.error-banner__close:hover {
    background-color: rgba(239, 68, 68, 0.12);
}

.error-banner__close .material-symbols-outlined {
    font-size: 16px;
}

.error-banner-enter-active,
.error-banner-leave-active {
    transition:
        opacity var(--duration-normal) var(--ease),
        transform var(--duration-normal) var(--ease);
}

.error-banner-enter-from {
    opacity: 0;
    transform: translateY(-8px);
}

.error-banner-leave-to {
    opacity: 0;
    transform: translateY(-8px);
}
</style>