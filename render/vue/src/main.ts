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
import { setupMonacoWorkers } from './utils/monaco'

const app = createApp(App)

setupMonacoWorkers()

app.use(createPinia())
app.use(router)

app.mount('#app')
