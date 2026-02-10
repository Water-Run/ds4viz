# `ds4viz`: CSS 编码规范

> 适用于 Vue/TypeScript SPA 项目中的样式编写（推荐使用 SCSS）。遵循该规范可确保样式可读、可维护、可扩展，并与组件化开发一致。

## 1. 语言与工具

- **推荐语言**：`SCSS`（优先）或 `CSS`  
- **编码工具**：`Stylelint` + `Prettier`（最严格模式，零 Warning）
- **命名风格**：统一使用 **kebab-case**
- **优先级策略**：避免权重战，不使用 `!important`（除非有明确例外规则）

---

## 2. 文件与目录规范

### 2.1 文件命名

- 使用 **kebab-case**  
  例：`user-profile.scss`、`data-table.scss`

### 2.2 目录建议

```
src/
  styles/
    base/
      reset.scss
      variables.scss
      mixins.scss
      typography.scss
    themes/
      light.scss
      dark.scss
    components/
      user-profile.scss
      data-table.scss
    layout/
      header.scss
      sidebar.scss
```

---

## 3. 命名规范

### 3.1 类名（Class）

- 统一使用 **kebab-case**
- **不使用驼峰、不使用下划线**
- 避免语义不清的命名（如 `.box`, `.item`, `.blue`）

✅ 推荐：
- `.user-profile`
- `.data-table__row`
- `.is-active`

❌ 禁止：
- `.userProfile`
- `.user_profile`
- `.box1`

### 3.2 BEM 规范（推荐）

```
.block
.block__element
.block--modifier
```

例：
- `.data-table`
- `.data-table__row`
- `.data-table--compact`

---

## 4. 书写顺序与结构

### 4.1 声明顺序

按 **布局 → 盒模型 → 排版 → 视觉 → 交互** 顺序编写：

1. **布局与定位**：`display`, `position`, `top/right/bottom/left`, `z-index`
2. **盒模型**：`box-sizing`, `width/height`, `margin`, `padding`, `border`
3. **排版**：`font`, `line-height`, `text-align`, `white-space`
4. **视觉**：`color`, `background`, `box-shadow`, `opacity`
5. **交互**：`cursor`, `transition`, `animation`

---

## 5. 变量与主题

### 5.1 统一使用 CSS 变量

在 `:root` 或主题容器中定义：

- 颜色
- 间距
- 圆角
- 阴影
- 动画时间

例：

```scss
:root {
  --color-primary: #3b82f6;
  --color-text: #111827;
  --radius-md: 8px;
  --space-md: 16px;
  --shadow-sm: 0 1px 2px rgba(0, 0, 0, 0.08);
}
```

### 5.2 主题隔离

推荐使用属性选择器：

```scss
[data-theme='dark'] {
  --color-text: #f9fafb;
}
```

---

## 6. 组件化样式规范

- **组件样式就近**：组件样式尽量与组件文件同级
- **禁止全局污染**：局部样式必须限定作用域
- **避免深层嵌套**：嵌套不超过 3 层

---

## 7. 选择器规范

### 7.1 禁止使用过度权重选择器

❌ 禁止：

```scss
.page .wrapper .content .item span {}
```

✅ 推荐：

```scss
.data-table__cell {}
```

### 7.2 避免 ID 选择器

- ID 选择器权重过高，不利于复用与覆盖

---

## 8. 响应式规范

- 使用 `@media` 时遵循 **移动优先**
- 断点统一配置，避免散乱

推荐断点（示例）：

- `sm`: 640px
- `md`: 768px
- `lg`: 1024px
- `xl`: 1280px

---

## 9. 禁止与限制

### 9.1 禁止 `!important`

除非：
- 第三方样式无法覆盖
- 明确写在 `exceptions.scss` 记录理由

### 9.2 禁止内联样式

除非用于动态样式绑定（并且必须注释说明原因）

---

## 10. 注释规范（仅在必要时使用）

- 样式需尽可能自描述
- 只在复杂逻辑或 Hack 时添加注释

---

## 11. 代码格式规范

- 使用 **2 空格缩进**
- 选择器与 `{` 之间保留 1 空格
- 每条属性独立一行
- 文件末尾保留 1 个空行

---

## 12. 样式重用与工具类

- 推荐使用 `mixins`、`functions`、`variables`
- 常用布局优先封装为工具类或 mixin
- 禁止重复定义相同样式块

---

## 13. 示例模板（SCSS）

```scss
/**
 * 用户卡片样式
 *
 * @file src/styles/components/user-card.scss
 * @author WaterRun
 * @date 2026-02-10
 */

.user-card {
  display: flex;
  align-items: center;
  gap: var(--space-md);

  width: 100%;
  padding: var(--space-md);
  border: 1px solid var(--color-border);
  border-radius: var(--radius-md);

  color: var(--color-text);
  background: var(--color-bg);
  box-shadow: var(--shadow-sm);

  &__avatar {
    width: 48px;
    height: 48px;
    border-radius: 50%;
  }

  &__name {
    font-size: 16px;
    font-weight: 600;
  }

  &--compact {
    padding: var(--space-sm);
  }
}
```

---

## 14. 推荐 Stylelint 规则要点

- `color-hex-length: short`
- `selector-class-pattern: ^[a-z][a-z0-9-]*$`
- `max-nesting-depth: 3`
- `declaration-no-important: true`
