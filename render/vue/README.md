# `ds4viz` Vue 渲染器

Vue 3 + TypeScript 单页应用, 解析 `.toml` IR 并以交互式方式回放数据结构操作.
本仓库的在线服务前端 (`server/release`) 与本地一体化编码-渲染环境均使用此渲染器.

- 仓库: <https://github.com/Water-Run/ds4viz>
- 协议: **GPL-3.0-or-later** (与根 `LICENSE` 一致)
- 测试于: Fedora 43 / bun 1.3.9 / Node 22 LTS / pnpm 9

## 技术栈

| 项 | 说明 |
|---|---|
| 框架 | Vue 3.5 (`<script setup>` + Composition API) |
| 语言 | TypeScript 5.9 (strict) |
| 状态 | Pinia 3 |
| 路由 | vue-router 5 |
| 编辑器 | Monaco Editor (via `@guolao/vue-monaco-editor`) |
| TOML 解析 | `@iarna/toml` (客户端校验 + 解析) |
| 构建 | Vite 7 |
| 单元测试 | Vitest 4 + `@vue/test-utils` + jsdom |
| 静态检查 | `vue-tsc` (类型) + oxlint + ESLint |
| 格式化 | Prettier 3 (`semi: false`, `singleQuote: true`, `printWidth: 100`) |
| 样式 | SCSS (`sass-embedded`) + CSS 变量主题 |

## 环境要求

- Node.js **20.19+** 或 **22.12+** (`package.json:engines`)
- 包管理器: `pnpm` (推荐) / `npm` / `yarn` / `bun` 均可

```bash
cp .env.example .env       # 默认指向 http://127.0.0.1:8000
pnpm install                # 或 npm i / bun i / yarn
```

## 常用命令

```bash
pnpm dev                    # 启动 Vite dev server, 默认 http://localhost:5173
pnpm build                  # vue-tsc 严格类型检查 + Vite 生产构建
pnpm preview                # 预览生产构建产物
pnpm test:unit              # 跑 vitest 全量
pnpm test:unit -- -t "name" # 按测试名过滤
pnpm test:unit -- src/__tests__/App.spec.ts  # 单文件
pnpm type-check             # vue-tsc, 必跑 (strict)
pnpm lint                   # oxlint + ESLint (--fix)
pnpm format                 # Prettier --write
```

## 目录结构

```text
src/
├── main.ts                 # 入口; 调用 setupMonacoWorkers (main.ts:20)
├── App.vue                 # 根组件
├── api/                    # 后端 HTTP 客户端 (auth/users/templates/executions)
├── components/             # 组件
│   ├── common/             # 通用 (TemplateDetailPanel 等)
│   ├── editor/             # CodeEditor (Monaco) / LlmPanel
│   └── viz/                # VizPanel (核心可视化, 3539 行)
├── layouts/                # AppLayout 等
├── router/                 # 路由表; 守卫在 router/index.ts:110-123
├── stores/                 # Pinia (auth/executions/templates)
├── styles/                 # base.css / theme.css (主题与全局样式)
├── types/                  # TypeScript 类型 (ir/api/auth/viz/...)
├── utils/                  # ir.ts (746 行) / viz.ts / monaco.ts / llm.ts / ...
└── views/                  # 路由级页面
```

## 关键页面与组件

| 路径 | 行数级别 | 职责 |
|---|---|---|
| `views/Playground.vue` | 1121 | 编辑器主页面: Monaco + 可视化 + 运行/步进 |
| `views/Docs.vue` | 2949 | 文档页 (项目内嵌文档) |
| `views/Templates.vue` / `TemplateDetail.vue` | - | 模板库 / 模板详情 |
| `views/Profile.vue` / `Favorites.vue` / `Executions.vue` | - | 个人中心 |
| `views/Login.vue` / `Register.vue` | - | 认证 |
| `components/viz/VizPanel.vue` | 3539 | 9 种结构的统一可视化组件, 内联 SVG |
| `components/editor/CodeEditor.vue` | - | Monaco 封装, shallowRef + rAF 合帧 |
| `components/editor/LlmPanel.vue` | - | LLM 代码生成浮动面板 |
| `utils/ir.ts` | 746 | TOML IR 解析 + 校验 + 字段映射 |
| `utils/viz.ts` | - | 帧序列与 phase 段落构建 |
| `utils/node-timeline.ts` | 490 | 节点变更时间线 (消失/出现追踪) |
| `utils/monaco.ts` | 388 | Monaco worker + 自动补全定义 |
| `utils/llm.ts` / `llm-prompts.ts` | - | LLM 集成 (默认 endpoint `https://api.poe.com/v1`) |

## 关键约定

- 命名空间: `@/` → `src/`
- 复杂对象 (`Monaco` 实例, IR 文档) 用 `shallowRef` + `markRaw` 避免深响应式
- HTTP 401 自动清 token (在 `api/http.ts:125-127`)
- VizPanel 用 **内联 SVG** 渲染, 9 种结构各自独立布局; **禁止** 改用 canvas / D3
- 高亮色硬编码在 `VizPanel.vue` 的 `HL_COLORS`, 主题色在 `styles/theme.css`
- 节点 ID 编码: `s-{idx}` (stack) / `q-{idx}` (queue) / `n-{id}` (slist/dlist) / `t-{id}` (tree) / `g-{id}` (graph) / `e-{from}-{to}` (edge)

## 环境变量

| 变量 | 默认 | 说明 |
|---|---|---|
| `VITE_API_BASE_URL` | `http://127.0.0.1:8000` | 后端 API 根地址, 走 Vite proxy 转发 |

Docker / 反代部署时由 `docker/.env.example` 统一配置, 通过 nginx `/api/` 反代.

## 与后端 / 沙箱的协作

- 本前端仅渲染 `.toml` IR, 不直接执行用户代码.
- `views/Playground.vue` 提交代码到后端 `/api/execute` (`server/release/src/api/execution.py`).
- 后端在 systemd-run 沙箱内运行代码, 返回 `.toml`.
- 用户也可跳过执行, 在 `Playground.vue` 直接上传本地 `.toml` 文件渲染.

## 调试 / 排错速查

| 现象 | 优先排查 |
|---|---|
| Monaco worker 404 | `main.ts:20` 是否调用了 `setupMonacoWorkers` |
| IR 解析报 `null` | 校验 `utils/ir.ts:ALLOWED_KINDS` 与 `document/IR定义.md` 一致; 检查 `meta.lang` 是否为 `python` / `c` |
| VizPanel 节点位置抖动 | 检查 `committedData` 是否随数据变更触发更新 |
| 切换语言不更新默认代码 | `Playground.vue:218` 递增 `languageSelectKey` |
| LLM 调不通 | `utils/llm.ts:32-43` localStorage 配置 + `utils/llm-prompts.ts` 系统提示 |
| `pnpm build` 报类型错 | `src/types/ir.ts` 是否与 `document/IR定义.md` 同步 |

## 开发小贴士

- **修改 IR** → 同步 `src/types/ir.ts` + `src/utils/ir.ts` + `VizPanel.vue` 的对应 kind 分支, 然后跑 `pnpm type-check`.
- **新增可视化结构** → 改完 `VizPanel.vue` 后, 同步 `utils/ir.ts:ALLOWED_KINDS` 与 `utils/viz.ts` 的布局.
- **样式调整** → 优先改 `styles/theme.css` 的 CSS 变量; 节点调色板改 `VizPanel.vue:NODE_PALETTE`; 尺寸常量改顶部 `BASE_*` 系列.
- **拉起后端** → `cd server/release && python src/main.py --test`, 然后在前端 `.env` 把 `VITE_API_BASE_URL` 改成 `http://127.0.0.1:10000`.

## 一键拉起 (推荐)

不必分别启动前后端, 仓库根目录有 Docker 一键脚本:

```bash
cp docker/.env.example .env
./docker/manage.sh up          # postgres + backend + frontend
./docker/manage.sh logs        # 跟踪日志
./docker/manage.sh down        # 停止
./docker/manage.sh clean       # 清理 (含数据卷)
```

详见 [根 README](../../README.md) 与 [document/在线服务部署.md](../../document/在线服务部署.md).
