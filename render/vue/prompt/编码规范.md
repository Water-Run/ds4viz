# `ds4viz`: Vue/TypeScript SPA 编码规范

在编写Vue + TypeScript单页应用时，你需要遵循以下编码规范：

**语言级别:** 最新稳定版 `TypeScript`, `Vue 3` (Composition API)  
**类型系统:** 使用严格的TypeScript配置(`strict: true`)，所有变量、参数、返回值都必须有明确的类型注解。善用联合类型、交叉类型、泛型、类型守卫、类型断言、条件类型等TypeScript高级特性  
**Vue风格:** 强制使用Composition API和`<script setup>`语法，禁止使用Options API。使用`defineProps`、`defineEmits`、`defineExpose`等编译器宏。优先使用组合式函数(Composables)进行逻辑复用  
**编码规范:** 遵循Vue官方风格指南(优先级A/B规则)和Airbnb TypeScript风格。代码需要在ESLint和Prettier最严格模式下无Warning。采用现代写法：使用可选链(`?.`)、空值合并(`??`)、顶层await等现代特性。善用`match-case`等现代优雅写法  
**文档注释:** 在文件开头、所有组件、组合式函数、工具函数、类型定义上都要有遵循TSDoc/JSDoc标准的文档注释。代码即文档的编码规范：代码需要尽可能的自描述、易懂，减少非文档注释之外的注释内容

## 命名约定

- **PascalCase**: 组件名、类型名、接口名、枚举名 (如 `UserProfile.vue`, `DataProcessor`, `UserInterface`)
- **camelCase**: 变量名、函数名、组合式函数、方法名 (如 `userData`, `getUserInfo`, `useDataFetcher`)
- **UPPER_SNAKE_CASE**: 常量、环境变量 (如 `API_BASE_URL`, `MAX_RETRY_COUNT`)
- **kebab-case**: CSS类名、组件标签、props、events (如 `user-profile`, `@update-value`)
- **组合式函数**: 以`use`开头 (如 `useAuth`, `useDataTable`)
- **类型后缀**: Interface用`I`前缀或无前缀，Type用`Type`后缀，Enum用`Enum`后缀 (如 `IUser`/`User`, `UserType`, `StatusEnum`)

## 单文件组件结构顺序

```vue
<script setup lang="ts">
// 1. 导入语句（按类型分组，每组内按字母顺序）
// 2. 类型定义
// 3. Props定义
// 4. Emits定义
// 5. 响应式状态
// 6. 计算属性
// 7. 监听器
// 8. 生命周期钩子
// 9. 方法
// 10. Expose
</script>

<template>
  <!-- 模板内容 -->
</template>

<style scoped lang="scss">
/* 样式 */
</style>
```

## 文档注释规范

### 文件开头

```typescript
/**
 * 文件功能描述，使用中文
 *
 * @file 文件路径（从src开始）
 * @author WaterRun
 * @date 创建日期 YYYY-MM-DD
 * @updated 更新日期 YYYY-MM-DD
 */
```

### 组件文档

```vue
<script setup lang="ts">
/**
 * 组件功能描述，使用中文
 *
 * @component 组件名
 * @example 简单使用示例代码
 */
```

### 类型定义

```typescript
/**
 * 接口/类型/枚举描述，使用中文
 */
export interface IUser {
  /** 字段说明 */
  id: string
  /** 字段说明 */
  username: string
}

/**
 * 枚举描述
 */
export enum UserRole {
  /** 枚举值说明 */
  ADMIN = 'admin',
  /** 枚举值说明 */
  USER = 'user',
}
```

### 组合式函数

```typescript
/**
 * 组合式函数功能描述，使用中文
 *
 * @template T - 泛型参数说明
 * @param paramName - 参数说明
 * @param options - 配置选项说明
 * @returns 返回值说明
 * @example 使用示例代码
 */
export function useDataFetcher<T>(
  fetcher: () => Promise<T>,
  options?: FetchOptions
): UseFetcherReturn<T> {
  // 实现
}
```

### Props和Emits定义

```typescript
/**
 * 组件属性定义
 */
interface Props {
  /** 属性说明 */
  user: User
  /** 属性说明，默认值说明 */
  showActions?: boolean
}

/**
 * 组件事件定义
 */
interface Emits {
  /** 事件说明 */
  (event: 'edit', userId: string): void
  /** 事件说明 */
  (event: 'delete', userId: string): void
}
```

### 响应式状态

```typescript
/**
 * 状态变量说明
 */
const userData = ref<User | null>(null)

/**
 * 加载状态
 */
const loading = ref<boolean>(false)
```

### 计算属性

```typescript
/**
 * 计算属性功能描述
 * 详细说明计算逻辑
 */
const displayName = computed<string>(() => {
  return user.value?.nickname || user.value?.username || '未命名'
})
```

### 方法

```typescript
/**
 * 方法功能描述，使用中文
 *
 * @param paramName - 参数说明
 * @returns 返回值说明
 * @throws 异常说明
 */
const handleSubmit = async (formData: FormData): Promise<void> => {
  // 实现
}
```

### Pinia Store

```typescript
/**
 * Store功能描述，使用中文
 * 管理的状态和功能说明
 *
 * @example 使用示例代码
 */
export const useUserStore = defineStore('user', () => {
  // State
  /**
   * 状态说明
   */
  const currentUser = ref<IUser | null>(null)

  // Getters
  /**
   * Getter功能说明
   */
  const isAuthenticated = computed<boolean>(() => currentUser.value !== null)

  // Actions
  /**
   * Action功能描述
   *
   * @param username - 参数说明
   * @param password - 参数说明
   * @throws 异常说明
   */
  const login = async (username: string, password: string): Promise<void> => {
    // 实现
  }

  return {
    currentUser,
    isAuthenticated,
    login,
  }
})
```

### 工具函数

```typescript
/**
 * 工具函数功能描述，使用中文
 *
 * @param paramName - 参数说明
 * @param options - 配置选项说明
 * @returns 返回值说明
 * @example 使用示例代码
 */
export function formatDate(
  date: Date,
  options?: FormatOptions
): string {
  // 实现
}
```

## 最佳实践

1. **Composition API优先**: 始终使用`<script setup>`，禁用Options API
2. **TypeScript严格模式**: 开启所有strict选项，避免any类型
3. **Props验证**: 使用TypeScript接口定义Props，提供默认值
4. **事件类型化**: 使用接口定义Emits，明确事件参数类型
5. **响应式解构**: 使用`storeToRefs`解构Pinia store，保持响应性
6. **组合式函数**: 将可复用逻辑提取为composables，遵循`use`命名约定
7. **计算属性纯净**: 计算属性不应有副作用，只做数据转换
8. **避免深层嵌套**: 组件嵌套不超过3层，使用组合式函数复用逻辑
9. **样式隔离**: 使用`scoped`样式，配合CSS变量实现主题
10. **性能优化**: 善用`v-memo`、`v-once`、异步组件、虚拟滚动等优化手段
11. **错误边界**: 使用`onErrorCaptured`捕获组件错误
12. **无障碍性**: 注意语义化HTML和ARIA属性