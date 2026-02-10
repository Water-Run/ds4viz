/**
 * App 根组件测试
 *
 * @file src/__tests__/App.spec.ts
 * @author WaterRun
 * @date 2026-02-10
 */

import { describe, it, expect } from 'vitest'
import { mount } from '@vue/test-utils'

import App from '../App.vue'

describe('App', () => {
  it('should mount without errors', () => {
    const wrapper = mount(App, {
      global: {
        stubs: {
          RouterView: true,
        },
      },
    })

    expect(wrapper.exists()).toBe(true)
  })
})