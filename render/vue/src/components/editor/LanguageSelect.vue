<script setup lang="ts">
/**
 * 语言选择组件
 *
 * @component LanguageSelect
 * @example
 * ```vue
 * <LanguageSelect v-model="language" />
 * ```
 */

import { computed } from 'vue'
import type { Language } from '@/types/api'
import { LANGUAGE_LABELS } from '@/types/api'

/**
 * 组件属性定义
 */
interface Props {
  /** 当前语言 */
  modelValue: Language
}

/**
 * 组件事件定义
 */
interface Emits {
  /** 语言变更 */
  (event: 'update:modelValue', value: Language): void
}

const props = defineProps<Props>()
const emit = defineEmits<Emits>()

/**
 * 可选语言列表
 */
const options = computed<Language[]>(() => ['python', 'lua', 'rust'])

/**
 * 语言颜色映射
 */
const languageColor = computed<string>(() => {
  if (props.modelValue === 'python') return 'var(--color-lang-python)'
  if (props.modelValue === 'lua') return 'var(--color-lang-lua)'
  if (props.modelValue === 'rust') return 'var(--color-lang-rust)'
  return 'var(--color-text-tertiary)'
})

/**
 * 处理语言切换
 */
const handleChange = (event: Event): void => {
  const target = event.target as HTMLSelectElement
  emit('update:modelValue', target.value as Language)
}
</script>

<template>
  <div class="language-select">
    <span class="language-select__dot" :style="{ backgroundColor: languageColor }" />
    <select class="language-select__control" :value="modelValue" @change="handleChange">
      <option v-for="lang in options" :key="lang" :value="lang">
        {{ LANGUAGE_LABELS[lang] }}
      </option>
    </select>
  </div>
</template>

<style scoped>
.language-select {
  display: inline-flex;
  align-items: center;
  gap: 6px;
  padding: 0 10px;
  height: var(--control-height-sm);
  border: 1px solid var(--color-border-strong);
  border-radius: var(--radius-control);
  background-color: var(--color-bg-surface);
}

.language-select__dot {
  width: 8px;
  height: 8px;
  border-radius: 999px;
}

.language-select__control {
  font-size: var(--text-sm);
  color: var(--color-text-primary);
  background: none;
  border: none;
  outline: none;
  cursor: pointer;
}
</style>
