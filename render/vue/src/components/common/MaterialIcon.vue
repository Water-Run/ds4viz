<script setup lang="ts">
/**
 * 本地图标组件（Material Icons SVG）
 *
 * @file src/components/common/MaterialIcon.vue
 * @author WaterRun
 * @date 2026-02-13
 * @updated 2026-02-13
 */

import { computed } from 'vue'

/**
 * 图标组件属性定义
 */
interface Props {
  /** 图标名称（对应 /public/material-icons 下的 SVG 文件名） */
  name: string
  /** 图标尺寸（像素） */
  size?: number
  /** 辅助说明（为空则 aria-hidden） */
  label?: string
}

const props = withDefaults(defineProps<Props>(), {
  size: 18,
  label: '',
})

/**
 * 图标资源 URL
 */
const iconUrl = computed<string>(() => `/material-icons/${props.name}.svg`)

/**
 * 是否隐藏无障碍文本
 */
const isAriaHidden = computed<boolean>(() => props.label.length === 0)
</script>

<template>
  <span
    class="material-icon"
    :style="{
      width: `${size}px`,
      height: `${size}px`,
      WebkitMaskImage: `url(${iconUrl})`,
      maskImage: `url(${iconUrl})`,
    }"
    :aria-hidden="isAriaHidden"
    :aria-label="isAriaHidden ? undefined : label"
    role="img"
  />
</template>

<style scoped>
.material-icon {
  display: inline-block;
  flex-shrink: 0;
  background-color: currentColor;
  mask-repeat: no-repeat;
  mask-position: center;
  mask-size: contain;
  -webkit-mask-repeat: no-repeat;
  -webkit-mask-position: center;
  -webkit-mask-size: contain;
}
</style>
