/**
 * 应用入口，初始化 Vue 实例及全局插件
 *
 * @file src/main.ts
 * @author WaterRun
 * @date 2026-02-10
 */

import { createApp } from 'vue'
import { createPinia } from 'pinia'

import App from './App.vue'
import router from './router'

import './styles/base.css'

const app = createApp(App)

app.use(createPinia())
app.use(router)

app.mount('#app')