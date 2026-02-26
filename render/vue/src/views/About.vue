<script setup lang="ts">
/**
 * 关于页面
 *
 * 展示 ds4viz 项目信息、许可协议与免责声明。
 *
 * @file src/views/About.vue
 * @author WaterRun
 * @date 2026-02-26
 * @component About
 */
</script>

<template>
  <div class="about-page">
    <div class="about-page__content">
      <img
        src="/ds4viz/logo.png"
        alt="ds4viz"
        class="about-page__logo"
      />

      <h1 class="about-page__title">ds4viz</h1>
      <p class="about-page__slogan">通用的, 基础数据结构可视化.</p>

      <p class="about-page__license">
        ds4viz 是自由软件, 遵循 GPL v3 协议开放源代码. 本软件按 "AS IS" 提供, 不提供任何明示或暗示的担保, 包括但不限于适销性, 特定用途适用性. 在适用法律允许的范围内, 作者及贡献者不对因使用或无法使用本软件而产生的任何直接或间接损失承担责任.
      </p>

      <a
        href="https://github.com/Water-Run/ds4viz"
        class="about-page__github"
        target="_blank"
        rel="noopener noreferrer"
      >
        <svg class="about-page__github-icon" viewBox="0 0 16 16" fill="currentColor">
          <path
            d="M8 0C3.58 0 0 3.58 0 8c0 3.54 2.29 6.53 5.47 7.59.4.07.55-.17.55-.38 0-.19-.01-.82-.01-1.49-2.01.37-2.53-.49-2.69-.94-.09-.23-.48-.94-.82-1.13-.28-.15-.68-.52-.01-.53.63-.01 1.08.58 1.23.82.72 1.21 1.87.87 2.33.66.07-.52.28-.87.51-1.07-1.78-.2-3.64-.89-3.64-3.95 0-.87.31-1.59.82-2.15-.08-.2-.36-1.02.08-2.12 0 0 .67-.21 2.2.82.64-.18 1.32-.27 2-.27.68 0 1.36.09 2 .27 1.53-1.04 2.2-.82 2.2-.82.44 1.1.16 1.92.08 2.12.51.56.82 1.27.82 2.15 0 3.07-1.87 3.75-3.65 3.95.29.25.54.73.54 1.48 0 1.07-.01 1.93-.01 2.2 0 .21.15.46.55.38A8.013 8.013 0 0016 8c0-4.42-3.58-8-8-8z"
          />
        </svg>
        <span>在 GitHub 上查看</span>
      </a>

      <p class="about-page__footer">此渲染器是 Vue SPA 渲染器, 提供在线服务.</p>
    </div>
  </div>
</template>

<style scoped>
.about-page {
  display: flex;
  align-items: center;
  justify-content: center;
  height: 100%;
  padding: var(--space-3);
  padding-bottom: 10%;
}

.about-page__content {
  display: flex;
  flex-direction: column;
  align-items: center;
  max-width: 520px;
  width: 100%;
}

.about-page__logo {
  width: 192px;
  height: 192px;
  object-fit: contain;
  margin-bottom: var(--space-3);
}

.about-page__title {
  margin: 0;
  font-size: 26px;
  font-weight: 700;
  letter-spacing: -0.03em;
  color: var(--color-text-primary);
  line-height: 1.2;
}

.about-page__slogan {
  margin: 6px 0 0;
  font-size: 15px;
  font-weight: 600;
  color: var(--color-text-body);
  line-height: 1.5;
}

.about-page__license {
  margin: var(--space-3) 0 0;
  font-size: var(--text-xs);
  color: var(--color-text-tertiary);
  line-height: var(--leading-relaxed);
  text-align: center;
  max-width: 460px;
}

.about-page__github {
  display: inline-flex;
  align-items: center;
  gap: 10px;
  height: 42px;
  padding: 0 18px;
  margin-top: var(--space-3);
  border: 1px solid var(--color-border-strong);
  border-radius: 12px;
  color: var(--color-text-body);
  font-size: 13px;
  font-weight: 500;
  text-decoration: none;
  transition:
    background-color var(--duration-fast) var(--ease),
    border-color var(--duration-fast) var(--ease),
    box-shadow var(--duration-normal) var(--ease),
    color var(--duration-fast) var(--ease);
}

.about-page__github:hover {
  background-color: var(--color-bg-hover);
  border-color: var(--color-border-strong);
  color: var(--color-text-primary);
  box-shadow: 0 4px 16px rgba(0, 0, 0, 0.08);
}

.about-page__github:focus-visible {
  outline: 2px solid var(--color-accent);
  outline-offset: 2px;
}

.about-page__github-icon {
  width: 18px;
  height: 18px;
  flex-shrink: 0;
}

.about-page__footer {
  margin: var(--space-3) 0 0;
  font-size: var(--text-xs);
  color: var(--color-text-tertiary);
  text-align: center;
}
</style>
