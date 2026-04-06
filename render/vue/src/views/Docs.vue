<script setup lang="ts">
/**
 * 文档页面
 *
 * @file src/views/Docs.vue
 * @author WaterRun
 * @date 2026-04-06
 * @component Docs
 */

import { computed, onMounted, onUnmounted, ref } from 'vue'
import { useRouter } from 'vue-router'

import { LANGUAGES, LANGUAGE_LABELS } from '@/types/api'
import MaterialIcon from '@/components/common/MaterialIcon.vue'
import CodeEditor from '@/components/editor/CodeEditor.vue'
import type { Language } from '@/types/api'

/**
 * 内容段落
 *
 * @interface
 */
interface ContentParagraph {
  /** 段落 HTML（支持 <b>、<code> 等标记） */
  text: string
  /** 语言归属，省略则为通用段落 */
  lang?: Language
}

/**
 * 文档节点
 *
 * @interface
 */
interface DocNode {
  /** 节点 id */
  id: string
  /** 标题 */
  title: string
  /** 语言归属，省略则为通用节点 */
  lang?: Language
  /** 内容段落（支持 HTML 标记） */
  content?: ContentParagraph[]
  /** 代码块（语言切换） */
  codeBlocks?: Partial<Record<Language, string>>
  /** 子节点 */
  children?: DocNode[]
}

const router = useRouter()

/**
 * 选中语言（全局共享）
 */
const selectedLanguage = ref<Language>('python')

/**
 * 展开状态
 */
const expanded = ref<Record<string, boolean>>({
  intro: true,
  online: true,
  syntax: true,
  linear: true,
  tree: true,
  graph: true,
  errors: true,
  examples: true,
  deploy: true,
})

/**
 * 内容区容器引用
 */
const contentRef = ref<HTMLElement | null>(null)

/**
 * 已复制的代码块 ID
 */
const copiedId = ref<string>('')

/**
* 当前可见的文档节点 id（用于 TOC 高亮）
*/
const activeId = ref<string>('')

/**
 * 切换图标名称
 *
 * @param open - 是否展开
 * @returns 图标名称
 */
const getToggleIcon = (open: boolean): string => (open ? 'expand_more' : 'chevron_right')

/**
 * 获取子节点列表（按语言过滤）
 *
 * @param node - 文档节点
 * @returns 过滤后的子节点数组
 */
const getChildren = (node: DocNode): DocNode[] => {
  const children = node.children ?? []
  return children.filter((child) => !child.lang || child.lang === selectedLanguage.value)
}

/**
 * 获取全部子节点列表（不过滤，用于 TOC）
 *
 * @param node - 文档节点
 * @returns 全部子节点数组
 */
const getAllChildren = (node: DocNode): DocNode[] => node.children ?? []

/**
 * 判断节点是否包含代码块
 *
 * @param node - 文档节点
 * @returns 是否包含可显示代码块
 */
const hasCodeBlocks = (node: DocNode): boolean => {
  return node.codeBlocks !== undefined && Object.keys(node.codeBlocks).length > 0
}

/**
 * 过滤内容段落（按当前选中语言）
 *
 * @param paragraphs - 原始段落数组
 * @returns 过滤后的段落数组
 */
const filterContent = (paragraphs?: ContentParagraph[]): ContentParagraph[] => {
  if (!paragraphs) return []
  return paragraphs.filter((p) => !p.lang || p.lang === selectedLanguage.value)
}

/* ================================================================
 *  文档树数据
 * ================================================================ */

const docTree = computed<DocNode[]>(() => [
  /* ============================================================
   *  1. 概览
   * ============================================================ */
  {
    id: 'intro',
    title: '概览',
    content: [
      {
        text: '<b>ds4viz</b>（data-structure-for-visualization）是一个面向教学场景的基础数据结构可视化框架。核心架构通过提供对应语言库，将代码中的结构操作过程记录为<b>可回放、可追踪、可复现</b>的状态序列，并存储在通用的 <code>TOML IR</code> 中。由于各层解耦的设计，ds4viz 支持多种渲染器。用户可以在单个页面完成从编写代码到观察结构变化的全过程。',
      },
      {
        text: 'ds4viz 遵循 <b>GPL v3</b> 协议开放源代码。完整文档、语言库源码与渲染器实现见 GitHub 仓库。',
      },
    ],
    children: [
      {
        id: 'intro-principle',
        title: '基础原理',
        content: [
          {
            text: 'ds4viz 采用<b>代码 → 中间语言 → 渲染</b>三层解耦架构。',
          },
          {
            text: '<b>第一层是语言库</b>。用户在目标语言中以指定语法调用 ds4viz API，每次操作都会记录状态变化。语言库负责将操作序列序列化为 <code>TOML</code> 格式的中间表示文件。',
          },
          {
            text: '<b>第二层是中间语言 IR</b>。所有语言统一产出 <code>.toml</code> 文件，该文件描述一次完整 trace。渲染器不需要读取源码，只需读取 TOML 即可复原整个过程。IR 是平台无关、完全独立的。',
          },
          {
            text: '<b>第三层是渲染器</b>。渲染器按 <code>object.kind</code> 解析结构类型，按 <code>states</code> 和 <code>steps</code> 进行展示。同一份 TOML 可在不同渲染端复用。一类特定的、不设计用于交互的渲染器在 ds4viz 中被称为<b>编译器</b>，例如输出到 PowerPoint 演示文稿等。',
          },
        ],
      },
      {
        id: 'intro-ir',
        title: 'IR 中间语言',
        content: [
          {
            text: '<code>TOML IR</code> 是 ds4viz 的核心中间表示。一个 <code>.toml</code> 文件必须且仅描述<b>一个数据结构实例</b>的完整状态变迁。渲染器仅依赖 <code>.toml</code> 文件即可完整回放操作流。',
          },
          {
            text: '一个完整的 <code>.toml</code> 文件包含以下顶层节:<br><code>[meta]</code> — 生成时间与语言上下文<br><code>[package]</code> — ds4viz 库版本信息<br><code>[remarks]</code> — 可选，标题、作者、备注<br><code>[object]</code> — 数据结构类型（<code>kind</code>）与标签（<code>label</code>）<br><code>[[states]]</code> — 1..N 个完整状态快照，每个 state 可独立渲染<br><code>[[steps]]</code> — 0..N 个操作步骤，通过 <code>before</code>/<code>after</code> 引用 states<br><code>[result]</code> 或 <code>[error]</code> — 成功或失败的结局标记（二选一）',
          },
          {
            text: '<code>states</code> 是<b>完整快照语义</b>——渲染器可以在不依赖其他 state 的情况下渲染任意 state。<code>steps</code> 描述每次操作，携带操作名（<code>op</code>）、参数（<code>args</code>）、源码位置（<code>code.line</code>）、阶段标记（<code>phase</code>）以及高亮标记（<code>highlights</code>）。',
          },
          {
            text: '<code>steps.highlights</code> 也是<b>全量快照语义</b>：每个 step 的 highlights 完整描述该步骤的所有高亮，渲染器无需回溯之前的 step。',
          },
          {
            text: '文档级约束要求 <code>result</code> 与 <code>error</code> 有且仅有一个存在，<code>states</code> 的 <code>id</code> 从 0 连续递增，节点 <code>id</code> 在所有 states 中表示同一逻辑节点。已删除节点的 <code>id</code> 不得被后续新节点复用。',
          },
        ],
      },
    ],
  },

  /* ============================================================
  *  2. 在线服务
  * ============================================================ */
  {
    id: 'online',
    title: '在线服务',
    content: [
      {
        text: 'ds4viz 在线服务将代码编辑、远程执行、<code>TOML IR</code> 解析与可视化渲染整合到同一工作流。使用前需注册账号并登录。登录后左侧导航栏包含三个入口：<b>编辑器</b>（核心工作区）、<b>模板库</b>（示例代码）、<b>文档</b>（当前页面）。导航栏可在图标模式与完全展开之间切换，底部显示当前用户头像，点击进入个人主页。左上角品牌标识可访问关于页面。',
      },
    ],
    children: [
      /* ---- 2.1 编辑器布局 ---- */
      {
        id: 'online-editor-layout',
        title: '编辑器布局',
        content: [
          {
            text: '编辑器页面采用<b>左右分栏布局</b>：左侧为可视化渲染器，右侧为代码编辑器。两个面板之间支持三种模式切换。',
          },
          {
            text: '<b>分屏模式</b>（默认）：左右各占一半空间。<b>可视化全屏</b>：可视化面板占满宽度，编辑器折叠为一根窄竖条。<b>编辑器全屏</b>：编辑器占满宽度，可视化折叠为窄竖条。',
          },
          {
            text: '切换方式有两种。方式一：将鼠标移至面板边缘，会出现一个半透明箭头条，点击即可向对应方向展开该面板。再次点击箭头条恢复分屏。方式二：面板被折叠后变为带图标和文字的竖条（"切换渲染器"或"切换编辑器"），点击竖条可重新展开该面板。',
          },
          {
            text: '在窄屏设备（≤1100px）上，两个面板自动切换为上下堆叠排列，折叠触发条隐藏。',
          },
        ],
      },

      /* ---- 2.2 代码编辑 ---- */
      {
        id: 'online-editor-code',
        title: '代码编辑',
        content: [
          {
            text: '编辑器基于 <code>Monaco Editor</code>（VS Code 同款内核），提供语法高亮、括号匹配、多光标编辑等能力。字体为 <code>JetBrains Mono</code>，默认 13px，行高 20px，Tab 缩进 4 空格。',
          },
          {
            text: '编辑器上方的下拉框可切换编程语言。当前支持 <code>Python</code> 和 <code>C</code>。如果编辑器中已有自行编写的代码（非默认模板），切换语言时会弹出覆盖确认对话框。',
          },
          {
            text: '每种语言预置一段可直接运行的默认模板代码。打开编辑器时自动加载上次使用的语言及其模板。在模板库或文档中点击"在编辑器中打开"后，代码和语言会自动替换至编辑器。',
          },
          {
            text: '编辑器内置 ds4viz 专属补全。在 Python 中，输入 <code>dv.</code> 后会弹出结构创建函数（<code>dv.stack</code>、<code>dv.binary_tree</code> 等）、通用方法（<code>step</code>、<code>phase</code>、<code>alias</code>）、高亮辅助（<code>dv.node</code>、<code>dv.item</code>、<code>dv.edge</code>）以及 Python 内置关键字和函数。C 语言中同样提供 <code>dvStack</code>、<code>dvStep</code>、<code>dvNode</code> 等宏和函数补全。补全支持 Snippet 语法，会自动填入参数占位符并定位光标。',
          },
          {
            text: '当可视化设置中"<b>联动代码高亮</b>"开启时，切换可视化步骤会自动在编辑器中高亮对应的源码行（橙色背景）。这要求代码是通过"运行"按钮产生的（上传本地 TOML 时无对应源码，此功能不可用）。',
          },
        ],
      },

      /* ---- 2.3 运行与执行 ---- */
      {
        id: 'online-editor-run',
        title: '运行与执行',
        content: [
          {
            text: '点击编辑器区域上方的<b>「运行」</b>按钮，当前代码将提交至后端沙箱执行。执行完成后，后端返回 <code>TOML IR</code> 文件内容。前端自动解析 TOML 并更新可视化面板。',
          },
          {
            text: '运行期间，按钮显示"运行中"并禁用重复点击。编辑器仍可编辑代码，但建议等待当前运行完成。运行结束后，编辑器下方的执行信息栏会显示状态与耗时，例如"success / 128ms"。',
          },
          {
            text: '如果代码执行失败（语法错误、运行时异常、超时等），页面顶部会出现红色错误横幅，展示后端返回的错误消息。可视化面板仍会渲染至出错前的最后有效状态（如果 TOML 中包含 <code>[error]</code>）。点击横幅右侧的关闭按钮可手动消除。',
          },
          {
            text: '如果 TOML 解析本身失败（格式损坏、字段缺失等），错误横幅会显示具体的解析错误位置和原因。可视化面板保持之前的状态不变。',
          },
        ],
      },

      /* ---- 2.4 TOML 文件操作 ---- */
      {
        id: 'online-editor-toml',
        title: 'TOML 文件操作',
        content: [
          {
            text: '编辑器页面标题栏右侧提供<b>上传</b>和<b>下载</b>两个按钮。',
          },
          {
            text: '<b>上传</b>：点击上传按钮选择本地 <code>.toml</code> 文件。文件读取后自动解析并渲染可视化。上传的 TOML 文件独立于编辑器代码——因为没有对应的源码，代码行高亮联动不可用。这适用于从其他渠道（命令行执行、其他渲染器导出等）获得的 TOML 文件。',
          },
          {
            text: '<b>下载</b>：点击下载按钮将当前的 TOML IR 文件保存到本地，文件名为 <code>ds4viz.toml</code>。只有在至少执行过一次代码或上传过一次 TOML 后，下载按钮才可用。',
          },
          {
            text: '在可视化面板下方有一个可折叠的 <b>TOML IR</b> 查看器。点击展开后，以基础语法高亮显示当前 TOML 文件的全文内容。高亮覆盖了 Section 标头（蓝色）、键名（黑色加粗）、字符串值（绿色）、数值（蓝色）、布尔值（紫色）、注释（灰色斜体）。最大显示高度 240px，超出部分可滚动。',
          },
        ],
      },

      /* ---- 2.5 LLM 代码生成 ---- */
      {
        id: 'online-editor-llm',
        title: 'LLM 代码生成',
        content: [
          {
            text: '编辑器面板右下角有一个圆形按钮，将鼠标悬停或点击即可展开 <b>LLM 代码生成面板</b>。该面板允许通过自然语言描述生成 ds4viz 代码。',
          },
          {
            text: '在文本框中输入需求描述（例如"构建一棵包含 7 个节点的 BST 并演示中序遍历过程"），按 <code>Enter</code> 或点击「生成」按钮提交。<code>Shift+Enter</code> 可换行输入多行提示。生成期间按钮显示旋转动画。',
          },
          {
            text: '<b>修改当前代码</b>开关：默认关闭。勾选后，当前编辑器中的代码会作为上下文注入 LLM 提示，LLM 将基于现有代码做增量修改而非从零生成。适用于"在现有代码基础上增加遍历步骤"等场景。',
          },
          {
            text: '如果编辑器中已有自行编写的代码且未勾选"修改当前代码"，点击生成时会先弹出覆盖确认提示条（黄色背景），需手动确认后才执行生成。',
          },
          {
            text: '点击生成按钮旁的齿轮图标可展开 <b>LLM 配置区</b>，包含以下设置：<br><code>API 地址</code> — OpenAI 兼容的 Chat Completions 端点<br><code>模型</code> — 模型名称<br><code>API Key</code> — 认证密钥（密码输入框）<br><code>超时</code> — 请求超时秒数（1–600）<br><code>最大 Token</code> — 响应最大 Token 数（1–131072）',
          },
          {
            text: '所有 LLM 配置存储在浏览器本地（<code>localStorage</code>），不会上传至 ds4viz 服务器。点击"恢复默认"可重置为初始值。鼠标离开面板区域 300ms 后面板自动关闭。',
          },
        ],
      },

      /* ---- 2.6 渲染器概述 ---- */
      {
        id: 'online-viz-overview',
        title: '渲染器概述',
        content: [
          {
            text: '可视化渲染器是编辑器页面的左侧面板，负责将 <code>TOML IR</code> 中描述的数据结构状态序列渲染为可交互的 SVG 图形。',
          },
          {
            text: '渲染器基于<b>完整快照语义</b>：IR 中每个 <code>state</code> 都是一个独立的完整状态描述，渲染任意一帧不需要依赖其他帧。<code>steps</code> 提供操作名称、参数、源码行号和高亮标记等辅助信息。',
          },
          {
            text: '渲染器内部使用<b>帧序列</b>模型：帧 0 对应初始状态（state 0，无 step），此后每个 step 对应一帧。因此总帧数 = 1 + steps 数量。即使多个 step 指向同一 state（如纯观察步骤 <code>observe</code>），也各自生成独立帧。步进控制、自动播放、阶段导航等功能均以帧为最小导航单位。',
          },
          {
            text: '渲染器按 <code>object.kind</code> 识别结构类型，当前支持：<br><b>线性结构</b> — <code>stack</code>（圆柱堆叠）、<code>queue</code>（横向方块链）<br><b>链表</b> — <code>slist</code>（单链表节点链）、<code>dlist</code>（双链表双向箭头链）<br><b>树</b> — <code>binary_tree</code>（二叉树）、<code>bst</code>（二叉搜索树）<br><b>图</b> — <code>graph_undirected</code>（无向图）、<code>graph_directed</code>（有向图）、<code>graph_weighted</code>（带权有向图）',
          },
        ],
      },

      /* ---- 2.7 步进控制与播放 ---- */
      {
        id: 'online-viz-controls',
        title: '步进控制与播放',
        content: [
          {
            text: '渲染器面板上方工具栏包含五个导航按钮和一个帧计数器。从左到右依次为：<b>首步</b>（跳至帧 0）、<b>上一步</b>、帧计数器（如"3 / 12"）、<b>下一步</b>、<b>末步</b>（跳至最后一帧）、<b>播放/暂停</b>。到达边界时对应按钮自动变灰禁用。',
          },
          {
            text: '点击<b>播放</b>按钮后，渲染器将以设定的间隔自动逐帧前进。播放到末帧时自动停止。播放期间再次点击按钮变为暂停。播放间隔可在渲染器设置面板中调整（0–10000ms，默认 768ms）。修改间隔时如果正在播放，会立即以新间隔重启。',
          },
          {
            text: '手动执行上一步、下一步、首步或末步操作时，如果正在自动播放，播放会自动暂停。',
          },
          {
            text: 'TOML 解析成功后，默认显示<b>就绪页</b>：一个带播放图标的居中提示，显示"可视化结果已生成"及总帧数，下方两个按钮分别为"下一步"（进入第 1 帧）和"自动播放"（从第 1 帧开始连续播放）。就绪页的目的是让用户选择观察方式。',
          },
          {
            text: '可通过渲染器设置关闭就绪页：<br><b>跳过初始页</b> — 开启后生成 TOML 后直接显示可视化（停在帧 0），不显示就绪提示<br><b>生成后自动播放</b> — 开启后代码运行完成即自动进入播放，跳过就绪页',
          },
          {
            text: '工具栏右侧还显示当前步骤信息：操作名称标签（如 <code>push</code>、<code>observe</code>）以及对应源码行号（如 <code>L7</code>）。空状态或初始帧时不显示。',
          },
        ],
      },

      /* ---- 2.8 画布交互 ---- */
      {
        id: 'online-viz-canvas',
        title: '画布交互',
        content: [
          {
            text: '可视化画布支持缩放和平移，用于观察大型结构的细节或全局。',
          },
          {
            text: '<b>缩放</b>：在画布上滚动鼠标滚轮即可缩放。缩放以鼠标当前位置为锚点，向上滚为放大，向下滚为缩小。每次缩放倍率为 1.1 倍。',
          },
          {
            text: '<b>平移</b>：在画布上按住鼠标左键拖拽即可平移视图。拖拽超过 4 像素后才识别为平移（避免误触）。拖拽时光标变为抓取形状。',
          },
          {
            text: '<b>双击还原</b>：双击画布任意位置，视图将以 200ms 动画平滑回到自适应计算的最佳位置和缩放级别。',
          },
          {
            text: '<b>自适应画布</b>：当渲染器设置中"自适应画布"开启时（默认开启），每次数据变化或帧切换后，画布会自动调整缩放和位置，使全部内容完整可见并居中显示。动画时长 180ms。',
          },
          {
            text: '手动缩放或平移后，自适应暂时挂起，画布底部中央会浮现半透明提示"双击画布还原缩放定位"。双击后恢复自适应。当展开变更卡片时，自适应会自动扩展视野以包含卡片区域。',
          },
        ],
      },

      /* ---- 2.9 栈与队列渲染 ---- */
      {
        id: 'online-viz-linear',
        title: '栈与队列渲染',
        content: [
          {
            text: '<b>栈（Stack）</b>以圆柱堆叠形式渲染。每个元素是一个水平的椭圆层，最底部元素在最下方，最顶部元素在最上方。椭圆的上表面用实线绘制，侧面用连接线表示。栈顶（TOP）元素有蓝色强调填充，右侧标注 <code>TOP</code> 标签。',
          },
          {
            text: '<b>队列（Queue）</b>以横向方块链形式渲染。每个元素是一个圆角矩形，从左到右排列，相邻元素之间用箭头连接。<code>FRONT</code>（队首）标签显示在对应元素上方，<code>REAR</code>（队尾）标签显示在对应元素下方（通过虚线引出）。当同一元素同时为 FRONT 和 REAR 时（队列仅有一个元素），两个标签分别在上下方显示。',
          },
          {
            text: '栈和队列的值文本居中渲染在元素内部。如果值文本过长（超出基准宽度），元素会自动按比例放大；超长文本会分行显示（最多 2 行，末行超限追加 <code>…</code>）。中文字符按 2 宽度单位计算。',
          },
          {
            text: '鼠标悬浮在任意元素上会弹出 Tooltip，显示索引（如 <code>[0]</code>）、完整值（截断的文本在此处完整展示）、角色（TOP/FRONT/REAR）。',
          },
        ],
      },

      /* ---- 2.10 链表渲染 ---- */
      {
        id: 'online-viz-linked',
        title: '链表渲染',
        content: [
          {
            text: '<b>单链表（SList）</b>按照 <code>head→next→...→-1</code> 的遍历顺序从左到右排列节点。每个节点是一个圆角矩形，内部上方显示值、下方显示节点 ID（如 <code>#3</code>）。相邻节点之间用带箭头的连线表示 <code>next</code> 指针。最后一个节点（<code>next = -1</code>）右侧显示 <code>⊘</code> 空指针标记。',
          },
          {
            text: '<b>双链表（DList）</b>布局与单链表相同，区别在于相邻节点之间有<b>两条</b>箭头线——上方一条从左到右表示 <code>next</code>，下方一条从右到左表示 <code>prev</code>。两条线垂直偏移 4px 避免重叠。',
          },
          {
            text: '<code>HEAD</code> 标签显示在头节点上方（蓝色）。双链表的 <code>TAIL</code> 标签显示在尾节点下方（琥珀色），通过虚线引出。',
          },
          {
            text: '鼠标悬浮节点时 Tooltip 显示：节点 ID、完整值、角色（HEAD/TAIL）、后继节点 ID、前驱节点 ID（仅双链表）。',
          },
          {
            text: '链表节点支持点击展开<b>节点变更卡片</b>（见后文），可追踪节点在整个操作序列中的变更历史。',
          },
        ],
      },

      /* ---- 2.11 树渲染 ---- */
      {
        id: 'online-viz-tree',
        title: '二叉树与 BST 渲染',
        content: [
          {
            text: '<b>二叉树（BinaryTree）</b>和<b>二叉搜索树（BST）</b>使用相同的渲染方式。树采用<b>分层布局</b>：通过中序遍历确定节点的水平坐标（保证左子在左、右子在右且不交叉），深度确定垂直坐标。',
          },
          {
            text: '每个节点渲染为圆形，圆内显示节点值。根节点用蓝色淡底填充（accent wash）并以蓝色描边与其他节点区分。父子节点之间用直线连接。',
          },
          {
            text: '节点半径会根据最长值自动缩放。值文本最多显示 1 行，超长则截断加 <code>…</code>。层间距为 72px，节点间水平最小距离由半径决定。',
          },
          {
            text: '鼠标悬浮节点时 Tooltip 显示：节点 ID、完整值、角色（ROOT）、左子节点 ID、右子节点 ID。',
          },
          {
            text: '树节点支持点击展开<b>节点变更卡片</b>，可追踪每个节点从创建到删除的完整变更历史。',
          },
        ],
      },

      /* ---- 2.12 图渲染 ---- */
      {
        id: 'online-viz-graph',
        title: '图渲染',
        content: [
          {
            text: '<b>无向图</b>、<b>有向图</b>和<b>带权有向图</b>均使用<b>圆形布局</b>：所有节点均匀分布在圆周上，每个节点渲染为带颜色的圆形。颜色从 12 色调色板中按节点 ID 循环分配，圆内显示节点标签（<code>label</code>）。',
          },
          {
            text: '边渲染为从源节点圆周边缘到目标节点圆周边缘的直线（自动裁剪，不穿过圆心区域）。<b>有向图</b>和<b>带权图</b>的边末端带三角形箭头标记方向。<b>带权图</b>在边的中点上方标注权值数字（蓝色加粗）。',
          },
          {
            text: '圆形布局的半径随节点数量自动调整。节点标签过长时自动截断。鼠标悬浮节点时 Tooltip 显示节点 ID 和完整标签。',
          },
          {
            text: '图节点同样支持点击展开<b>节点变更卡片</b>，可追踪节点和关联边的变更历史。',
          },
        ],
      },

      /* ---- 2.13 高亮系统 ---- */
      {
        id: 'online-viz-highlight',
        title: '高亮系统',
        content: [
          {
            text: '当步骤携带 <code>highlights</code> 标记时，渲染器会对指定元素施加视觉强调。高亮采用<b>全量快照语义</b>：每个步骤的 highlights 完整描述该帧的所有高亮，切换帧时自动替换而非累积。',
          },
          {
            text: '渲染器支持 6 种预定义高亮样式，每种对应不同的填充色和描边色：<br><code>focus</code>（当前焦点） — 蓝色填充 + 蓝色描边<br><code>visited</code>（已访问） — 极浅灰填充 + 灰色描边<br><code>active</code>（参与计算） — 青色填充 + 青色描边<br><code>comparing</code>（正在比较） — 琥珀色填充 + 琥珀色描边<br><code>found</code>（查找命中） — 绿色填充 + 绿色描边<br><code>error</code>（异常状态） — 红色填充 + 红色描边',
          },
          {
            text: '<code>level</code>（1–9）控制视觉强度。数值越大，描边越粗（基础 1px + level × 0.22px）。同一帧内可以通过不同 level 区分主次——例如当前焦点用 <code>level=3</code>，已访问节点用 <code>level=1</code>。',
          },
          {
            text: '高亮变化通过 CSS 过渡平滑衔接（140ms），不会突然跳变。切换帧时，上一帧的高亮平滑褪去，新帧的高亮平滑出现。',
          },
          {
            text: '高亮可应用于三类对象：<br><b>节点</b>（<code>node</code>类型） — 适用于链表、树、图，覆盖节点的 fill 和 stroke<br><b>元素</b>（<code>item</code>类型） — 仅适用于栈和队列，覆盖对应层/方块的颜色<br><b>边</b>（<code>edge</code>类型） — 仅适用于图结构，覆盖边的 stroke 和宽度',
          },
        ],
      },

      /* ---- 2.14 平滑过渡与变更强化 ---- */
      {
        id: 'online-viz-transitions',
        title: '平滑过渡与变更强化',
        content: [
          {
            text: '渲染器提供两项视觉衔接机制，使帧间切换更具连续感。两者可在渲染器设置中独立开关。',
          },
          {
            text: '<b>平滑动画衔接</b>（默认开启）：帧间切换时，树节点和图节点的位置变化以 200ms CSS 过渡实现平滑移动，而非瞬间跳变。边的端点同步移动。这在树结构删除/插入节点导致布局重排时效果尤为明显——节点会"滑动"到新位置而非闪现。',
          },
          {
            text: '<b>变更强化</b>（默认开启）：帧间切换时，渲染器自动检测新增和移除的元素。<b>新增元素</b>以渐入动画出现（从 88% 缩放 + 透明渐入到正常大小）。<b>移除元素</b>以红色虚线轮廓渐出（从 45% 透明度缩小到 86% 并消失）。',
          },
          {
            text: '变更强化的时序分为三个阶段：首先渲染移除元素的红色虚线 ghost；ghost 渐出完成后提交新数据；新增元素以渐入动画出现。自动播放时 ghost 时长为播放间隔的一半，手动操作时为 500ms。',
          },
          {
            text: '首帧从空到非空时（例如第一次运行代码后），平滑过渡自动抑制一帧，避免全部元素从左上角原点"飞入"的异常效果。',
          },
          {
            text: '如果浏览器启用了"减少动画"偏好（<code>prefers-reduced-motion: reduce</code>），所有位移和缩放动画会被系统级禁用。',
          },
        ],
      },

      /* ---- 2.15 节点变更卡片 ---- */
      {
        id: 'online-viz-cards',
        title: '节点变更卡片',
        content: [
          {
            text: '对于含 <code>nodes</code> 的结构（链表、树、图），<b>点击任意节点</b>可展开该节点的<b>变更卡片</b>——一个 SVG 内渲染的横向时间线，完整展示该节点在所有帧中的变更历史。再次点击同一节点可收起卡片。',
          },
          {
            text: '卡片由左侧的<b>标题单元格</b>和右侧的<b>变更单元格序列</b>组成。标题单元格显示节点 ID（如 <code>#3</code>）、别名（如果有）和当前值。每个变更单元格对应一个发生了变化的帧。连续未变化的帧会自动跳过，只保留实际发生变更的条目。',
          },
          {
            text: '每个变更单元格顶部有一个<b>变更类型图标</b>，共 7 种：<br>绿色圆圈 + 十字 — <code>created</code>（节点被创建）<br>红色圆圈 + 叉号 — <code>deleted</code>（节点被删除）<br>蓝色圆圈 + 右箭头 — <code>value_changed</code>（值变化）<br>青色圆圈 + 双箭头 — <code>structural</code>（结构关系变化，如 next/left/right 指针改变、边增删）<br>琥珀色圆圈 + 圆点 — <code>highlighted</code>（被高亮标记）<br>紫色圆圈 + 横线 — <code>alias_changed</code>（别名变化）<br>深蓝圆圈 + W — <code>weight_changed</code>（边权重变化）',
          },
          {
            text: '图标下方依次显示：变化后的值（created 显示新值，deleted 显示 <code>✕</code>，value_changed 显示新值）、帧号（如 <code>F3</code>）。当前帧对应的单元格有蓝色描边和浅蓝背景突出显示。',
          },
          {
            text: '卡片有两种显示模式，通过标题单元格底部的 <code>+</code>/<code>−</code> 按钮切换：<br><b>简洁模式</b>（默认）：每个单元格仅显示图标、值和帧号<br><b>详细模式</b>：额外显示变更详情（如值变化的"旧值→新值"、结构变化的"字段: 旧→新"、alias 变化的"旧→新"）以及操作名',
          },
          {
            text: '帧范围可通过标题单元格的 <code>▸</code>/<code>◂</code> 按钮切换：<br><b>当前帧之前</b>（默认）：只显示当前帧及其之前的变更<br><b>全部帧</b>：显示节点在整个 IR 中的全部变更。尚未到达的帧单元格以 35% 透明度显示',
          },
          {
            text: '当变更条目超过 7 个时，右侧/左侧会出现翻页箭头按钮，可滚动查看更多帧。展开卡片时会自动滚动到当前帧附近。',
          },
          {
            text: '<b>点击任意单元格</b>可直接跳转到对应帧。这是快速定位特定操作的便捷方式。',
          },
          {
            text: '卡片通过虚线<b>引线</b>（Leader line）连接到对应节点。引线从卡片最近边缘出发，终止于节点圆周边缘（而非中心，避免遮挡值文本）。卡片位置由评分算法自动选择：为每个节点在 24 个方向 × 5 个距离级别中选取最优位置，评分考虑距离惩罚、引线穿越其他节点的惩罚、与已放置卡片的拥挤惩罚。',
          },
          {
            text: '画布右上角提供<b>展开全部/折叠全部</b>按钮。点击"展开"会同时展开当前可见的所有节点的变更卡片；点击"折叠"关闭所有卡片。展开卡片后，如果自适应画布开启，视野会自动扩展以包含所有卡片。',
          },
        ],
      },

      /* ---- 2.16 消逝节点 ---- */
      {
        id: 'online-viz-departed',
        title: '消逝节点',
        content: [
          {
            text: '在当前帧中已被删除的节点称为<b>消逝节点</b>。渲染器在它们最后已知的位置渲染一个<b>红色虚线圆</b>，圆内显示截断的最后已知值（灰红色文字）。',
          },
          {
            text: '当存在消逝节点时，画布左上角出现一个带计数的小按钮。将鼠标悬停到按钮上会展开<b>消逝节点列表</b>，每项显示节点名称和存活帧范围（如 <code>F0–F5</code>，表示该节点在帧 0 创建、帧 5 消逝）。',
          },
          {
            text: '<b>点击列表中的条目</b>或<b>点击画布上的虚线圆</b>，可展开该消逝节点的变更卡片。消逝节点的卡片有红色虚线外框和红色引线以区分。卡片标题右下角有专属的 <code>✕</code> 关闭按钮。',
          },
          {
            text: '消逝节点功能的用途是回溯被删除节点的完整生命周期——从创建到每次值变化、结构变化，直到最终删除。这在分析树的旋转、图的节点删除等操作时尤为有用。',
          },
        ],
      },

      /* ---- 2.17 阶段导航 ---- */
      {
        id: 'online-viz-phases',
        title: '阶段导航',
        content: [
          {
            text: '当 IR 的 steps 中使用了 <code>phase</code> 字段时，渲染器底部中央会出现<b>阶段导航条</b>。阶段由连续使用相同 <code>phase</code> 值的 steps 合并而成。',
          },
          {
            text: '导航条中央是当前阶段名称的圆角胶囊按钮，左右各有一个箭头按钮。点击左箭头跳转到上一阶段的起始帧，点击右箭头跳转到下一阶段的起始帧。到达首/末阶段时对应箭头禁用。',
          },
          {
            text: '点击中央的阶段名称按钮会弹出<b>阶段列表</b>浮层，列出所有阶段的名称和对应步骤范围。点击任一阶段可直接跳转。当前所在阶段以蓝色背景突出显示。',
          },
          {
            text: '自动播放期间，阶段导航条的箭头和弹出层均禁用，中央按钮变为只读状态并显示播放图标。',
          },
          {
            text: '阶段导航可在渲染器设置中通过"显示阶段导航"开关关闭。阶段不是必需的——没有 <code>phase</code> 的 IR 不会显示导航条。',
          },
        ],
      },

      /* ---- 2.18 标题、元数据与 Tooltip ---- */
      {
        id: 'online-viz-display',
        title: '标题、元数据与信息提示',
        content: [
          {
            text: '<b>可视化方案名</b>（渲染器设置"可视化方案名"开关控制，默认开启）：在可视化内容区上方居中显示以下信息——<code>object.label</code> 以蓝色小字渲染，<code>remarks.title</code> 以较大加粗标题渲染，<code>remarks.comment</code> 以斜体辅助文字渲染，<code>remarks.author</code> 以斜体辅助文字渲染。缺失的字段不显示。',
          },
          {
            text: '<b>元数据面板</b>（渲染器设置"显示元数据"开关控制，默认关闭）：在渲染器面板 header 下方显示结构化标签行。包含以下行：<br><code>Object</code> — 显示 label 和 kind<br><code>Remarks</code> — 显示 title、author、comment<br><code>IR</code> — 显示 created_at<br><code>Step</code> — 显示当前步骤的 op、line、note 以及全部 args 键值对',
          },
          {
            text: '<b>Tooltip</b>：鼠标悬浮在任意可视化元素（栈层、队列方块、链表节点、树节点、图节点）上时，光标旁边会出现一个白色浮层，以键值对形式显示该元素的详细信息。浮层跟随鼠标移动，鼠标离开后消失。Tooltip 不遮挡元素本身（偏移 12px）。',
          },
          {
            text: '<b>空状态</b>：当数据结构已创建但内容为空时（如空栈、空树、空图），渲染器不显示占位提示，而是渲染一组虚线示意图形 + 文案。栈显示虚线圆柱轮廓 + "空栈"，队列显示虚线方块链 + "空队列"，链表显示虚线矩形 + 箭头 + "空链表"，树显示虚线圆 + 连线 + "空树"，图显示虚线圆 + 连线 + "空图"。',
          },
          {
            text: '在尚未运行任何代码且未上传 TOML 时，渲染器面板显示居中占位提示："等待渲染——运行代码或上传 TOML 查看结果"。',
          },
        ],
      },

      /* ---- 2.19 渲染器设置 ---- */
      {
        id: 'online-viz-settings',
        title: '渲染器设置',
        content: [
          {
            text: '可视化面板左下角的齿轮按钮展开<b>渲染器设置面板</b>。鼠标悬停自动展开，离开 300ms 后自动关闭，也可点击按钮手动切换。',
          },
          {
            text: '设置面板包含以下<b>常规开关</b>：<br><b>显示元数据</b> — 展示 IR 的 object.label、remarks、meta 等结构化信息<br><b>自适应画布</b> — 数据变化时自动调整缩放与位置<br><b>生成后自动播放</b> — 代码运行完成后自动开始播放<br><b>显示阶段导航</b> — 显示底部阶段导航条（需 IR 含 phase）<br><b>可视化方案名</b> — 显示 label/title/comment/author 标题信息',
          },
          {
            text: '面板底部有一个<b>调试</b>入口（带虫子图标），鼠标悬停展开二级面板，包含以下开关：<br><b>平滑动画衔接</b> — 帧间节点位移的 CSS 过渡<br><b>联动代码高亮</b> — 步进时高亮编辑器对应源码行<br><b>变更强化</b> — 新增渐入 + 移除红色虚线渐出<br><b>跳过初始页</b> — 不显示就绪提示，直接进入可视化<br><b>控制台调试</b> — 在浏览器控制台输出 ds4viz 内部日志',
          },
          {
            text: '<b>播放间隔滑块</b>：范围 0–10000ms，步进 100ms。数值越小播放越快。当前值以等宽字体显示在滑块右侧。默认 768ms。',
          },
          {
            text: '"<b>恢复默认</b>"按钮可一键重置全部设置到初始值。所有设置自动保存到浏览器 <code>localStorage</code>，页面刷新或重新登录后保留。',
          },
        ],
      },

      /* ---- 2.20 模板库 ---- */
      {
        id: 'online-templates',
        title: '模板库',
        content: [
          {
            text: '模板库以卡片网格展示预置的 ds4viz 示例代码。每张卡片显示标题、描述摘要（最多 2 行）、分类标签和收藏数。',
          },
          {
            text: '左侧分类栏可按类别筛选模板。点击"全部"显示所有分类。右上角搜索框支持关键词实时搜索（300ms 防抖，兼容中文输入法）。搜索结果页显示匹配条数和"清除搜索"链接。',
          },
          {
            text: '列表使用<b>无限滚动</b>加载。向下滚动接近底部时自动加载下一页。在"全部"分类下，如果首屏未显示任何收藏模板，系统会自动预取后续页，确保"我收藏的"区域尽快出现。',
          },
          {
            text: '收藏的模板在列表顶部的<b>"我收藏的"</b>独立区域集中显示，与其余模板以分隔线区分。每张卡片的右下角有心形按钮，点击可收藏或取消收藏。收藏数实时更新。',
          },
          {
            text: '点击卡片后，列表顶部展开一个<b>模板详情面板</b>。面板左侧是只读 Monaco 代码编辑器（高度根据代码行数自适应），右侧显示描述、代码说明（explanation）、分类、语言、创建时间和收藏按钮。顶部有语言下拉框可切换展示不同语言的实现。',
          },
          {
            text: '面板提供两个操作按钮：<br><b>复制</b> — 将当前语言的代码复制到剪贴板，按钮短暂变为"已复制"✓<br><b>在编辑器中打开</b> — 将代码和语言导入编辑器页面并跳转。编辑器已有自行编写的代码时，会弹出覆盖确认',
          },
        ],
      },

      /* ---- 2.21 个人主页 ---- */
      {
        id: 'online-profile',
        title: '个人主页',
        content: [
          {
            text: '个人主页分为三个区域：顶部用户信息、左下收藏模板、右下执行记录。',
          },
          {
            text: '<b>用户信息区</b>居中显示头像、用户名、ID 和账号状态标签（Active 绿色/Suspended 黄色/Banned 红色）。头像右下角有相机图标按钮，点击可上传新头像。未上传头像时显示用户名首字母 + 确定性背景色（相同用户名始终得到相同颜色）。',
          },
          {
            text: '用户信息区下方有"<b>修改密码</b>"链接。点击展开折叠表单，包含旧密码和新密码两个输入框。新密码输入时实时显示五项复杂度指示：8–32 字符、大写字母、小写字母、数字、特殊字符。每项满足时变为绿色 ✓，未满足为灰色。提交时若新密码不达标，输入框会短暂振动提示。',
          },
          {
            text: '<b>收藏模板区</b>以卡片列表展示已收藏的模板（分页，每页 10 条）。每张卡片显示标题、分类、描述、收藏数和收藏时间。点击卡片跳转到模板库页面并自动展开对应模板详情。卡片右下角有红色实心心形按钮可取消收藏。',
          },
          {
            text: '<b>执行记录区</b>以列表形式展示历史执行记录（分页，每页 10 条）。每条记录显示语言、状态标签（success 绿/error 红/timeout 黄）、耗时、代码行数和创建时间。点击记录可展开代码详情：只读 Monaco 编辑器显示代码，下方提供"复制代码"和"在编辑器中打开"按钮。',
          },
          {
            text: '执行记录区标题右侧有"<b>统计数据</b>"入口。鼠标悬停展开统计浮层，首次访问时延迟加载（显示"统计中…"）。统计内容包括：<br>摘要数字卡片：总执行次数、平均执行耗时、收藏总数<br>状态分布饼图：success/error/timeout 各占比<br>语言分布饼图：Python/C 等各占比',
          },
        ],
      },
    ],
  },

  /* ============================================================
   *  3. 语法基础
   * ============================================================ */
  {
    id: 'syntax',
    title: '语法基础',
    content: [
      {
        text: 'ds4viz 当前支持 Python 和 C 两种语言，遵循一致的语义设计：创建结构对象，在受控上下文中执行操作，自动生成 <code>trace.toml</code>。正常退出时包含 <code>[result]</code>，发生错误时包含 <code>[error]</code>。',
      },
      {
        text: '值类型仅支持 <code>integer</code>、<code>float</code>、<code>string</code>、<code>boolean</code> 四种标量。BST 的 <code>insert</code>/<code>delete</code> 仅接受数值类型（<code>int</code> / <code>float</code>）。',
      },
      {
        text: 'Python 使用 <code>with</code> 上下文管理器确保 TOML 始终写入。',
        lang: 'python',
      },
      {
        text: 'C 使用结构作用域宏实现相同保证，但不要在作用域内使用 <code>return</code> 或 <code>goto</code> 跳出。',
        lang: 'c',
      },
    ],
    children: [
      {
        id: 'syntax-config',
        title: '全局配置',
        content: [
          {
            text: '全局配置是可选的，用于设定输出路径、标题、作者和备注等元信息。不调用配置时，输出路径默认为 <code>trace.toml</code>，<code>remarks</code> 节不生成。',
          },
          {
            text: '通过 <code>dv.config()</code> 进行全局配置。',
            lang: 'python',
          },
          {
            text: '通过 <code>dvConfig()</code> 并传入 <code>dvConfigOptions</code> 结构体进行全局配置，未指定的字段自动零初始化。',
            lang: 'c',
          },
          {
            text: '参数说明:<br><code>output_path</code> — 输出文件路径，默认 <code>"trace.toml"</code><br><code>title</code> — 可视化标题，对应 IR <code>[remarks].title</code><br><code>author</code> — 作者信息，对应 IR <code>[remarks].author</code><br><code>comment</code> — 注释说明，对应 IR <code>[remarks].comment</code>',
          },
        ],
        codeBlocks: {
          python: `import ds4viz as dv

dv.config(
    output_path="trace.toml",
    title="Demo",
    author="WaterRun",
    comment="示例"
)`,
          c: `#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"

int main(void) {
    dvConfig((dvConfigOptions){
        .output_path = "trace.toml",
        .title       = "Demo",
        .author      = "WaterRun",
        .comment     = "示例"
    });

    return 0;
}`,
        },
      },
      {
        id: 'syntax-context',
        title: '上下文管理',
        content: [
          {
            text: '所有数据结构实例通过<b>上下文管理模式</b>使用，确保无论成功或异常都会写入 <code>.toml</code> 文件。',
          },
          {
            text: '使用 <code>with</code> 语句创建上下文。进入 <code>with</code> 时创建对象并写入初始状态；每次 API 调用写入一个 step 与对应新 state；<code>with</code> 正常退出时输出 <code>[result]</code>；<code>with</code> 内异常时输出 <code>[error]</code>。',
            lang: 'python',
          },
          {
            text: '使用结构作用域宏（如 <code>dvStack(s) { ... }</code>）创建上下文。进入作用域时创建对象并写入初始状态；每次 API 调用写入一个 step 与对应新 state；作用域正常退出时输出 <code>[result]</code>；操作异常时输出 <code>[error]</code>。注意不要在作用域内使用 <code>return</code> 或 <code>goto</code> 跳出，否则会跳过 TOML 写入及资源清理。',
            lang: 'c',
          },
          {
            text: '每个结构创建时可指定 <code>label</code> 参数（对应 IR <code>[object].label</code>），不传时使用默认值（如 Stack 默认 <code>"stack"</code>）。',
          },
        ],
        codeBlocks: {
          python: `import ds4viz as dv

# label 可选，默认值因结构而异
with dv.stack(label="my_stack") as s:
    s.push(10)
    s.push(20)
    val = s.pop()   # val = 20

# with 退出时自动写入 trace.toml`,
          c: `#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"

int main(void) {
    dvConfig((dvConfigOptions){ .output_path = "trace.toml" });

    // 第二个参数为 label，可省略
    dvStack(s, "my_stack") {
        dvStackPush(s, 10);
        dvStackPush(s, 20);
        dvValue val = dvStackPop(s);  // dvInt(val) == 20
    }
    // 作用域退出时自动写入 trace.toml

    return 0;
}`,
        },
      },
      {
        id: 'syntax-value-types',
        title: '值类型与约束',
        content: [
          {
            text: 'ds4viz 的 <code>value</code> 字段仅支持 TOML 标量类型。',
          },
          {
            text: '对应 <code>int</code>、<code>float</code>、<code>str</code>、<code>bool</code> 四种原生类型。<code>pop</code> / <code>dequeue</code> 等操作直接返回原始 Python 值。',
            lang: 'python',
          },
          {
            text: '<code>push</code> / <code>enqueue</code> 等操作通过 <code>_Generic</code> 自动推断值类型：<code>int</code> 及其他整型序列化为 TOML 整数，<code>float</code> / <code>double</code> 序列化为浮点数，<code>const char*</code> 序列化为字符串，<code>bool</code> 序列化为布尔值。',
            lang: 'c',
          },
          {
            text: '<code>pop</code> / <code>dequeue</code> 等操作返回 <code>dvValue</code> 联合体。使用 <code>dvInt()</code>、<code>dvFloat()</code>、<code>dvStr()</code>、<code>dvBool()</code> 提取具体类型的值。debug 模式下，类型不匹配会触发 <code>assert</code>。返回的字符串指针生命周期与所属结构作用域相同，作用域退出后失效。',
            lang: 'c',
          },
          {
            text: '类型约束:<br><code>stack</code> / <code>queue</code> / <code>slist</code> / <code>dlist</code> / <code>binary_tree</code> 的 value 接受全部四种标量<br><code>bst</code> 的 <code>insert</code> / <code>delete</code> 仅接受 <code>int</code> / <code>float</code>（因需要维护有序性）<br><code>graph</code> 的节点使用 <code>label</code>（string，长度 1..32）而非 value<br><code>graph_weighted</code> 的边使用 <code>weight</code>（<code>int</code> / <code>float</code>）',
          },
        ],
        codeBlocks: {
          python: `import ds4viz as dv

with dv.stack() as s:
    s.push(42)          # int
    s.push(3.14)        # float
    s.push("hello")     # str
    s.push(True)        # bool

    val = s.pop()       # val = True (原始类型)
    val = s.pop()       # val = "hello"`,
          c: `dvStack(s) {
    dvStackPush(s, 42);        // int -> TOML integer
    dvStackPush(s, 3.14);      // double -> TOML float
    dvStackPush(s, "hello");   // const char* -> TOML string
    dvStackPush(s, true);      // bool -> TOML boolean

    dvValue v1 = dvStackPop(s);       // v1.tag == DV_BOOL
    bool b = dvBool(v1);              // true

    dvValue v2 = dvStackPop(s);       // v2.tag == DV_STR
    const char* str = dvStr(v2);      // "hello"

    dvValue v3 = dvStackPop(s);       // v3.tag == DV_FLOAT
    double f = dvFloat(v3);           // 3.14

    dvValue v4 = dvStackPop(s);       // v4.tag == DV_INT
    int64_t n = dvInt(v4);            // 42
}`,
        },
      },
      {
        id: 'syntax-common',
        title: '通用实例方法',
        content: [
          { text: '所有数据结构实例均支持以下通用方法。' },
          {
            text: '<b><code>step(note, highlights)</code></b><br>记录一次观察步骤，不改变数据结构的状态。在 IR 中生成 <code>op = "observe"</code>、<code>before == after</code> 的 step。适用于遍历、查找等需要逐步展示但不修改结构的场景。<code>note</code> 为步骤说明（可选），<code>highlights</code> 为高亮标记列表（可选）。',
          },
          {
            text: '<b><code>amend(note, highlights)</code></b><br>修改上一步的展示属性。传入 <code>None</code> 的参数不修改对应字段。适用于在变更操作后补充高亮或说明。传入 <code>highlights=[]</code> 可清除高亮。不存在上一步时抛出异常。',
            lang: 'python',
          },
          {
            text: '<b><code>dvAmend(s, note, ...)</code></b><br>修改上一步的展示属性。传入 <code>NULL</code> 的参数不修改对应字段。适用于在变更操作后补充高亮或说明。使用 <code>dvAmendHL()</code> 仅修改高亮，使用 <code>dvAmendClearHL()</code> 清除高亮。不存在上一步时记录错误。',
            lang: 'c',
          },
          {
            text: '<b><code>phase(name)</code></b><br>阶段标记。上下文内产生的所有步骤（包括变更操作和手动 <code>step()</code>）都会标记为指定阶段（对应 IR <code>steps.phase</code>）。支持嵌套，内层优先。<code>phase</code> 不是必须的；当操作流程存在明确的逻辑划分时推荐使用，便于渲染器分段展示。',
          },
          {
            text: '使用 <code>with obj.phase("name"):</code> 语法进入阶段上下文。',
            lang: 'python',
          },
          {
            text: '使用 <code>dvPhase(s, "name") { ... }</code> 宏语法进入阶段作用域。',
            lang: 'c',
          },
          {
            text: '<b><code>alias(node_id, name)</code></b><br>仅适用于含 <code>nodes</code> 的结构（链表、树、图）。设置或清除节点别名，别名出现在后续状态快照中供渲染器展示。不产生新步骤。<code>name</code> 传入 <code>None</code> 表示清除。别名在同一 state 内必须唯一，仅允许字母、数字、下划线、连字符，长度 1..64。',
            lang: 'python',
          },
          {
            text: '<b><code>dvAlias(s, node_id, name)</code></b><br>仅适用于含 <code>nodes</code> 的结构（链表、树、图）。设置或清除节点别名，别名出现在后续状态快照中供渲染器展示。不产生新步骤。<code>name</code> 传入 <code>NULL</code> 表示清除。别名在同一 state 内必须唯一，仅允许字母、数字、下划线、连字符，长度 1..64。',
            lang: 'c',
          },
        ],
        codeBlocks: {
          python: `import ds4viz as dv

with dv.binary_tree(label="遍历") as tree:
    with tree.phase("构建"):
        root = tree.insert_root(10)
        tree.alias(root, "root")
        left = tree.insert_left(root, 5)
        right = tree.insert_right(root, 15)

    with tree.phase("中序遍历"):
        tree.step(note="访问 5", highlights=[
            dv.node(left, "focus", level=3)
        ])
        tree.step(note="访问 10", highlights=[
            dv.node(left, "visited"),
            dv.node(root, "focus", level=3)
        ])
        tree.step(note="访问 15", highlights=[
            dv.node(left, "visited"),
            dv.node(root, "visited"),
            dv.node(right, "focus", level=3)
        ])`,
          c: `dvBinaryTree(tree, "遍历") {
    int root, left, right;

    dvPhase(tree, "构建") {
        root  = dvBtInsertRoot(tree, 10);
        dvAlias(tree, root, "root");
        left  = dvBtInsertLeft(tree, root, 5);
        right = dvBtInsertRight(tree, root, 15);
    }

    dvPhase(tree, "中序遍历") {
        dvStep(tree, "访问 5",
            dvNode(left, DV_FOCUS, 3));

        dvStep(tree, "访问 10",
            dvNode(left, DV_VISITED),
            dvNode(root, DV_FOCUS, 3));

        dvStep(tree, "访问 15",
            dvNode(left, DV_VISITED),
            dvNode(root, DV_VISITED),
            dvNode(right, DV_FOCUS, 3));
    }
}`,
        },
      },
      {
        id: 'syntax-highlights',
        title: '高亮标记',
        content: [
          {
            text: '高亮标记用于标记步骤中需要视觉强调的元素，通过辅助函数构造后传入 <code>step()</code> 或 <code>amend()</code> 的 <code>highlights</code> 参数。',
          },
          {
            text: '<b><code>dv.node(target, style, level)</code></b> — 节点高亮。适用于含 <code>nodes</code> 的结构：链表、树、图。<code>target</code> 为节点 id。',
            lang: 'python',
          },
          {
            text: '<b><code>dv.item(target, style, level)</code></b> — 元素高亮。仅适用于 <code>stack</code> 和 <code>queue</code>。<code>target</code> 为 <code>items</code> 数组中的有效索引。',
            lang: 'python',
          },
          {
            text: '<b><code>dv.edge(from_id, to_id, style, level)</code></b> — 边高亮。仅适用于图结构。<code>(from_id, to_id)</code> 必须匹配关联 state 中存在的一条边。无向图时须满足 <code>from_id &lt; to_id</code>。',
            lang: 'python',
          },
          {
            text: '<b><code>dvNode(target, style, level)</code></b> — 节点高亮。适用于含 <code>nodes</code> 的结构：链表、树、图。<code>target</code> 为节点 id。',
            lang: 'c',
          },
          {
            text: '<b><code>dvItem(target, style, level)</code></b> — 元素高亮。仅适用于 <code>stack</code> 和 <code>queue</code>。<code>target</code> 为 <code>items</code> 数组中的有效索引。',
            lang: 'c',
          },
          {
            text: '<b><code>dvEdge(from_id, to_id, style, level)</code></b> — 边高亮。仅适用于图结构。<code>(from_id, to_id)</code> 必须匹配关联 state 中存在的一条边。无向图时须满足 <code>from_id &lt; to_id</code>。',
            lang: 'c',
          },
          {
            text: '<code>style</code> 为高亮样式字符串，<code>level</code> 为视觉强度等级（1..9，默认 1，数值越大越醒目）。<code>style</code> 和 <code>level</code> 均为可选参数，默认 <code>style = "focus"</code>、<code>level = 1</code>。',
          },
          {
            text: '预定义 <code>style</code> 值（渲染器必须支持，自定义值回退为 <code>focus</code> 表现）:<br><code>focus</code> — 当前焦点<br><code>visited</code> — 已访问<br><code>active</code> — 参与当前计算<br><code>comparing</code> — 正在比较<br><code>found</code> — 查找命中<br><code>error</code> — 异常状态',
          },
          {
            text: '<code>level</code> 是无颜色语义的视觉分级，渲染器自行决定映射方式（透明度、边框粗细、动画强度等）。同一 step 内同一 <code>style</code> 的不同 <code>level</code> 用于区分主次。',
          },
        ],
        codeBlocks: {
          python: `import ds4viz as dv

with dv.stack(label="高亮演示") as s:
    s.push(10)
    s.push(20)
    s.push(30)

    # level=3 表示高强调，level=1 为默认
    s.amend(highlights=[
        dv.item(0, "visited"),
        dv.item(1, "visited"),
        dv.item(2, "focus", level=3)
    ])

    s.pop()
    s.amend(note="弹出 30", highlights=[
        dv.item(0, "visited"),
        dv.item(1, "focus", level=3)
    ])`,
          c: `dvStack(s, "高亮演示") {
    dvStackPush(s, 10);
    dvStackPush(s, 20);
    dvStackPush(s, 30);

    dvAmendHL(s,
        dvItem(0, DV_VISITED),
        dvItem(1, DV_VISITED),
        dvItem(2, DV_FOCUS, 3));

    dvStackPop(s);
    dvAmend(s, "弹出 30",
        dvItem(0, DV_VISITED),
        dvItem(1, DV_FOCUS, 3));
}`,
        },
      },
    ],
  },

  /* ============================================================
   *  4. 线性结构
   * ============================================================ */
  {
    id: 'linear',
    title: '线性结构',
    children: [
      {
        id: 'api-stack',
        title: 'Stack',
        content: [
          { text: '栈，<b>后进先出（LIFO）</b>线性结构。' },
          {
            text: '通过 <code>dv.stack(label="stack")</code> 创建实例。',
            lang: 'python',
          },
          {
            text: '通过 <code>dvStack(s)</code> 或 <code>dvStack(s, "stack")</code> 创建实例。',
            lang: 'c',
          },
          {
            text: '支持操作:<br><code>push(value)</code> — 压栈，将 value 压入栈顶。<br><code>pop()</code> — 弹栈，移除并返回栈顶元素。栈为空时抛出异常。<br><code>clear()</code> — 清空栈中所有元素。',
            lang: 'python',
          },
          {
            text: '支持操作:<br><code>dvStackPush(s, value)</code> — 压栈，将 value 压入栈顶。<br><code>dvStackPop(s)</code> — 弹栈，移除并返回栈顶元素（<code>dvValue</code>）。栈为空时记录错误。<br><code>dvStackClear(s)</code> — 清空栈中所有元素。',
            lang: 'c',
          },
        ],
        codeBlocks: {
          python: `import ds4viz as dv

with dv.stack(label="demo_stack") as s:
    s.push(10)
    s.amend(note="压入 10", highlights=[dv.item(0, "focus", level=3)])

    s.push(20)
    s.push(30)
    s.amend(highlights=[
        dv.item(0, "visited"),
        dv.item(1, "visited"),
        dv.item(2, "focus", level=3)
    ])

    val = s.pop()    # val = 30
    s.amend(note=f"弹出 {val}")

    val = s.pop()    # val = 20
    s.pop()          # val = 10

    s.push(99)
    s.amend(note="压入 99", highlights=[dv.item(0, "focus", level=3)])`,
          c: `#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"

int main(void) {
    dvConfig((dvConfigOptions){ .output_path = "trace.toml" });

    dvStack(s, "demo_stack") {
        dvStackPush(s, 10);
        dvAmend(s, "压入 10", dvItem(0, DV_FOCUS, 3));

        dvStackPush(s, 20);
        dvStackPush(s, 30);
        dvAmendHL(s,
            dvItem(0, DV_VISITED),
            dvItem(1, DV_VISITED),
            dvItem(2, DV_FOCUS, 3));

        dvValue val = dvStackPop(s);   // dvInt(val) == 30
        dvAmend(s, "弹出 30");

        dvStackPop(s);
        dvStackPop(s);

        dvStackPush(s, 99);
        dvAmend(s, "压入 99", dvItem(0, DV_FOCUS, 3));
    }

    return 0;
}`,
        },
      },
      {
        id: 'api-queue',
        title: 'Queue',
        content: [
          { text: '队列，<b>先进先出（FIFO）</b>线性结构。' },
          {
            text: '通过 <code>dv.queue(label="queue")</code> 创建实例。',
            lang: 'python',
          },
          {
            text: '通过 <code>dvQueue(q)</code> 或 <code>dvQueue(q, "queue")</code> 创建实例。',
            lang: 'c',
          },
          {
            text: '支持操作:<br><code>enqueue(value)</code> — 入队，将 value 添加到队尾。<br><code>dequeue()</code> — 出队，移除并返回队首元素。队列为空时抛出异常。<br><code>clear()</code> — 清空队列中所有元素。',
            lang: 'python',
          },
          {
            text: '支持操作:<br><code>dvQueueEnqueue(q, value)</code> — 入队，将 value 添加到队尾。<br><code>dvQueueDequeue(q)</code> — 出队，移除并返回队首元素（<code>dvValue</code>）。队列为空时记录错误。<br><code>dvQueueClear(q)</code> — 清空队列中所有元素。',
            lang: 'c',
          },
        ],
        codeBlocks: {
          python: `import ds4viz as dv

with dv.queue(label="demo_queue") as q:
    with q.phase("入队"):
        q.enqueue("甲")
        q.enqueue("乙")
        q.enqueue("丙")

    with q.phase("处理"):
        val = q.dequeue()   # val = "甲"
        q.amend(note=f"处理 {val}", highlights=[
            dv.item(0, "focus", level=3)
        ])

        q.enqueue("丁")
        q.amend(note="新任务入队")

        val = q.dequeue()   # val = "乙"
        q.amend(note=f"处理 {val}")`,
          c: `#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"

int main(void) {
    dvConfig((dvConfigOptions){ .output_path = "trace.toml" });

    dvQueue(q, "demo_queue") {
        dvPhase(q, "入队") {
            dvQueueEnqueue(q, "甲");
            dvQueueEnqueue(q, "乙");
            dvQueueEnqueue(q, "丙");
        }

        dvPhase(q, "处理") {
            dvValue val = dvQueueDequeue(q);
            dvAmend(q, "处理 甲",
                dvItem(0, DV_FOCUS, 3));

            dvQueueEnqueue(q, "丁");
            dvAmend(q, "新任务入队");

            val = dvQueueDequeue(q);
            dvAmend(q, "处理 乙");
        }
    }

    return 0;
}`,
        },
      },
      {
        id: 'api-slist',
        title: 'SingleLinkedList',
        content: [
          {
            text: '单链表。每个节点持有 <code>value</code> 和 <code>next</code> 指针。',
          },
          {
            text: '通过 <code>dv.single_linked_list(label="slist")</code> 创建实例。',
            lang: 'python',
          },
          {
            text: '通过 <code>dvSingleLinkedList(l)</code> 或 <code>dvSingleLinkedList(l, "slist")</code> 创建实例。',
            lang: 'c',
          },
          {
            text: '支持操作:<br><code>insert_head(value)</code> — 头插，返回新节点 id。<br><code>insert_tail(value)</code> — 尾插，返回新节点 id。<br><code>insert_after(node_id, value)</code> — 在指定节点后插入，返回新节点 id。节点不存在时抛出异常。<br><code>delete(node_id)</code> — 删除指定节点。节点不存在时抛出异常。<br><code>delete_head()</code> — 删除头节点。链表为空时抛出异常。<br><code>reverse()</code> — 反转链表。',
            lang: 'python',
          },
          {
            text: '支持操作:<br><code>dvSlInsertHead(l, value)</code> — 头插，返回新节点 id。<br><code>dvSlInsertTail(l, value)</code> — 尾插，返回新节点 id。<br><code>dvSlInsertAfter(l, node_id, value)</code> — 在指定节点后插入，返回新节点 id。节点不存在时记录错误。<br><code>dvSlDelete(l, node_id)</code> — 删除指定节点。节点不存在时记录错误。<br><code>dvSlDeleteHead(l)</code> — 删除头节点。链表为空时记录错误。<br><code>dvSlReverse(l)</code> — 反转链表。',
            lang: 'c',
          },
        ],
        codeBlocks: {
          python: `import ds4viz as dv

with dv.single_linked_list(label="demo_slist") as slist:
    a = slist.insert_head(10)
    slist.alias(a, "head")
    b = slist.insert_tail(20)
    c = slist.insert_tail(30)

    d = slist.insert_after(a, 15)
    slist.amend(note="在 10 后插入 15", highlights=[
        dv.node(a, "active"),
        dv.node(d, "focus", level=3)
    ])

    slist.delete(c)
    slist.amend(note="删除节点 30")

    slist.reverse()
    slist.amend(note="反转链表")`,
          c: `#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"

int main(void) {
    dvConfig((dvConfigOptions){ .output_path = "trace.toml" });

    dvSingleLinkedList(slist, "demo_slist") {
        int a = dvSlInsertHead(slist, 10);
        dvAlias(slist, a, "head");
        int b = dvSlInsertTail(slist, 20);
        int c = dvSlInsertTail(slist, 30);

        int d = dvSlInsertAfter(slist, a, 15);
        dvAmend(slist, "在 10 后插入 15",
            dvNode(a, DV_ACTIVE),
            dvNode(d, DV_FOCUS, 3));

        dvSlDelete(slist, c);
        dvAmend(slist, "删除节点 30");

        dvSlReverse(slist);
        dvAmend(slist, "反转链表");
    }

    return 0;
}`,
        },
      },
      {
        id: 'api-dlist',
        title: 'DoubleLinkedList',
        content: [
          {
            text: '双向链表。每个节点持有 <code>value</code>、<code>prev</code> 和 <code>next</code> 指针。',
          },
          {
            text: '通过 <code>dv.double_linked_list(label="dlist")</code> 创建实例。',
            lang: 'python',
          },
          {
            text: '通过 <code>dvDoubleLinkedList(l)</code> 或 <code>dvDoubleLinkedList(l, "dlist")</code> 创建实例。',
            lang: 'c',
          },
          {
            text: '在单链表基础上增加:<br><code>insert_before(node_id, value)</code> — 在指定节点前插入，返回新节点 id。<br><code>delete_tail()</code> — 删除尾节点。链表为空时抛出异常。',
            lang: 'python',
          },
          {
            text: '在单链表基础上增加:<br><code>dvDlInsertBefore(l, node_id, value)</code> — 在指定节点前插入，返回新节点 id。<br><code>dvDlDeleteTail(l)</code> — 删除尾节点。链表为空时记录错误。',
            lang: 'c',
          },
          {
            text: '其余操作（<code>insert_head</code>、<code>insert_tail</code>、<code>insert_after</code>、<code>delete</code>、<code>delete_head</code>、<code>reverse</code>）与单链表一致。',
          },
        ],
        codeBlocks: {
          python: `import ds4viz as dv

with dv.double_linked_list(label="demo_dlist") as dlist:
    with dlist.phase("构建"):
        a = dlist.insert_head(10)
        dlist.alias(a, "head")
        b = dlist.insert_tail(30)
        c = dlist.insert_before(b, 20)
        dlist.amend(note="在 30 前插入 20", highlights=[
            dv.node(c, "focus", level=3),
            dv.node(b, "active")
        ])

    with dlist.phase("修改"):
        dlist.insert_after(a, 15)
        dlist.delete_tail()
        dlist.reverse()`,
          c: `#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"

int main(void) {
    dvConfig((dvConfigOptions){ .output_path = "trace.toml" });

    dvDoubleLinkedList(dlist, "demo_dlist") {
        int a, b, c;

        dvPhase(dlist, "构建") {
            a = dvDlInsertHead(dlist, 10);
            dvAlias(dlist, a, "head");
            b = dvDlInsertTail(dlist, 30);
            c = dvDlInsertBefore(dlist, b, 20);
            dvAmend(dlist, "在 30 前插入 20",
                dvNode(c, DV_FOCUS, 3),
                dvNode(b, DV_ACTIVE));
        }

        dvPhase(dlist, "修改") {
            dvDlInsertAfter(dlist, a, 15);
            dvDlDeleteTail(dlist);
            dvDlReverse(dlist);
        }
    }

    return 0;
}`,
        },
      },
    ],
  },

  /* ============================================================
   *  5. 树结构
   * ============================================================ */
  {
    id: 'tree',
    title: '树结构',
    children: [
      {
        id: 'api-binary-tree',
        title: 'BinaryTree',
        content: [
          {
            text: '二叉树。每个节点持有 <code>value</code>、<code>left</code> 和 <code>right</code> 子指针。',
          },
          {
            text: '通过 <code>dv.binary_tree(label="binary_tree")</code> 创建实例。',
            lang: 'python',
          },
          {
            text: '通过 <code>dvBinaryTree(t)</code> 或 <code>dvBinaryTree(t, "binary_tree")</code> 创建实例。',
            lang: 'c',
          },
          {
            text: '支持操作:<br><code>insert_root(value)</code> — 插入根节点，返回节点 id。根已存在时抛出异常。<br><code>insert_left(parent_id, value)</code> — 插入左子节点，返回节点 id。父节点不存在或左子已存在时抛出异常。<br><code>insert_right(parent_id, value)</code> — 插入右子节点，返回节点 id。父节点不存在或右子已存在时抛出异常。<br><code>delete(node_id)</code> — 删除指定节点及其所有子树。节点不存在时抛出异常。<br><code>update_value(node_id, value)</code> — 更新节点值。节点不存在时抛出异常。',
            lang: 'python',
          },
          {
            text: '支持操作:<br><code>dvBtInsertRoot(t, value)</code> — 插入根节点，返回节点 id。根已存在时记录错误。<br><code>dvBtInsertLeft(t, parent_id, value)</code> — 插入左子节点，返回节点 id。父节点不存在或左子已存在时记录错误。<br><code>dvBtInsertRight(t, parent_id, value)</code> — 插入右子节点，返回节点 id。父节点不存在或右子已存在时记录错误。<br><code>dvBtDelete(t, node_id)</code> — 删除指定节点及其所有子树。节点不存在时记录错误。<br><code>dvBtUpdateValue(t, node_id, value)</code> — 更新节点值。节点不存在时记录错误。',
            lang: 'c',
          },
        ],
        codeBlocks: {
          python: `import ds4viz as dv

with dv.binary_tree(label="demo_tree") as tree:
    root = tree.insert_root(10)
    tree.alias(root, "root")
    left = tree.insert_left(root, 5)
    right = tree.insert_right(root, 15)
    tree.insert_left(left, 3)
    tree.insert_right(left, 7)
    tree.insert_left(right, 12)

    tree.update_value(right, 20)
    tree.amend(note="将 15 更新为 20", highlights=[
        dv.node(right, "focus", level=3)
    ])

    tree.delete(left)
    tree.amend(note="删除节点 5 及其子树", highlights=[
        dv.node(root, "active")
    ])`,
          c: `#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"

int main(void) {
    dvConfig((dvConfigOptions){ .output_path = "trace.toml" });

    dvBinaryTree(tree, "demo_tree") {
        int root  = dvBtInsertRoot(tree, 10);
        dvAlias(tree, root, "root");
        int left  = dvBtInsertLeft(tree, root, 5);
        int right = dvBtInsertRight(tree, root, 15);
        dvBtInsertLeft(tree, left, 3);
        dvBtInsertRight(tree, left, 7);
        dvBtInsertLeft(tree, right, 12);

        dvBtUpdateValue(tree, right, 20);
        dvAmend(tree, "将 15 更新为 20",
            dvNode(right, DV_FOCUS, 3));

        dvBtDelete(tree, left);
        dvAmend(tree, "删除节点 5 及其子树",
            dvNode(root, DV_ACTIVE));
    }

    return 0;
}`,
        },
      },
      {
        id: 'api-bst',
        title: 'BinarySearchTree',
        content: [
          {
            text: '二叉搜索树。结构与 BinaryTree 一致，但 <code>insert</code> / <code>delete</code> 操作自动维护 BST 有序性。值类型限制为数值（<code>int</code> / <code>float</code>）。',
          },
          {
            text: '通过 <code>dv.binary_search_tree(label="bst")</code> 创建实例。',
            lang: 'python',
          },
          {
            text: '通过 <code>dvBinarySearchTree(b)</code> 或 <code>dvBinarySearchTree(b, "bst")</code> 创建实例。',
            lang: 'c',
          },
          {
            text: '支持操作:<br><code>insert(value)</code> — 插入节点，自动寻找合适位置，返回节点 id。<br><code>delete(value)</code> — 删除值等于 value 的节点，自动处理叶子、单子、双子三种情况。值不存在时抛出异常。',
            lang: 'python',
          },
          {
            text: '支持操作:<br><code>dvBstInsert(b, value)</code> — 插入节点，自动寻找合适位置，返回节点 id。<br><code>dvBstDelete(b, value)</code> — 删除值等于 value 的节点，自动处理叶子、单子、双子三种情况。值不存在时记录错误。',
            lang: 'c',
          },
          {
            text: 'BST 同样支持 <code>alias()</code>、<code>step()</code>、<code>amend()</code>、<code>phase()</code> 等通用方法。',
          },
        ],
        codeBlocks: {
          python: `import ds4viz as dv

with dv.binary_search_tree(label="demo_bst") as bst:
    with bst.phase("插入"):
        n10 = bst.insert(10)
        bst.alias(n10, "root")
        bst.insert(5)
        bst.insert(15)
        bst.insert(3)
        bst.insert(7)
        bst.insert(12)
        bst.insert(20)

    with bst.phase("删除"):
        bst.delete(5)
        bst.amend(note="删除 5，用中序后继替代")

        n6 = bst.insert(6)
        bst.amend(note="插入 6", highlights=[
            dv.node(n6, "focus", level=3)
        ])`,
          c: `#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"

int main(void) {
    dvConfig((dvConfigOptions){ .output_path = "trace.toml" });

    dvBinarySearchTree(bst, "demo_bst") {
        dvPhase(bst, "插入") {
            int n10 = dvBstInsert(bst, 10);
            dvAlias(bst, n10, "root");
            dvBstInsert(bst, 5);
            dvBstInsert(bst, 15);
            dvBstInsert(bst, 3);
            dvBstInsert(bst, 7);
            dvBstInsert(bst, 12);
            dvBstInsert(bst, 20);
        }

        dvPhase(bst, "删除") {
            dvBstDelete(bst, 5);
            dvAmend(bst, "删除 5，用中序后继替代");

            int n6 = dvBstInsert(bst, 6);
            dvAmend(bst, "插入 6",
                dvNode(n6, DV_FOCUS, 3));
        }
    }

    return 0;
}`,
        },
      },
    ],
  },

  /* ============================================================
   *  6. 图结构
   * ============================================================ */
  {
    id: 'graph',
    title: '图结构',
    children: [
      {
        id: 'api-graph-undirected',
        title: 'GraphUndirected',
        content: [
          {
            text: '无向图。节点持有 <code>id</code> 和 <code>label</code>，边由 <code>(from, to)</code> 对表示，内部自动规范化为 <code>from &lt; to</code>。',
          },
          {
            text: '通过 <code>dv.graph_undirected(label="graph")</code> 创建实例。',
            lang: 'python',
          },
          {
            text: '通过 <code>dvGraphUndirected(g)</code> 或 <code>dvGraphUndirected(g, "graph")</code> 创建实例。',
            lang: 'c',
          },
          {
            text: '支持操作:<br><code>add_node(node_id, label)</code> — 添加节点。节点已存在时抛出异常。<code>label</code> 长度 1..32。<br><code>add_edge(from_id, to_id)</code> — 添加无向边。内部自动规范化方向。节点不存在、边已存在或形成自环时抛出异常。<br><code>remove_node(node_id)</code> — 删除节点及其所有关联边。节点不存在时抛出异常。<br><code>remove_edge(from_id, to_id)</code> — 删除边。边不存在时抛出异常。<br><code>update_node_label(node_id, label)</code> — 更新节点标签。节点不存在时抛出异常。',
            lang: 'python',
          },
          {
            text: '支持操作:<br><code>dvGuAddNode(g, node_id, label)</code> — 添加节点。节点已存在时记录错误。<code>label</code> 长度 1..32。<br><code>dvGuAddEdge(g, from_id, to_id)</code> — 添加无向边。内部自动规范化方向。节点不存在、边已存在或形成自环时记录错误。<br><code>dvGuRemoveNode(g, node_id)</code> — 删除节点及其所有关联边。节点不存在时记录错误。<br><code>dvGuRemoveEdge(g, from_id, to_id)</code> — 删除边。边不存在时记录错误。<br><code>dvGuUpdateNodeLabel(g, node_id, label)</code> — 更新节点标签。节点不存在时记录错误。',
            lang: 'c',
          },
        ],
        codeBlocks: {
          python: `import ds4viz as dv

with dv.graph_undirected(label="demo_graph") as g:
    g.add_node(0, "甲")
    g.add_node(1, "乙")
    g.add_node(2, "丙")
    g.add_node(3, "丁")

    g.add_edge(0, 1)
    g.add_edge(0, 2)
    g.add_edge(1, 3)
    g.add_edge(2, 3)

    g.update_node_label(1, "乙*")
    g.remove_edge(0, 1)
    g.remove_node(3)`,
          c: `#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"

int main(void) {
    dvConfig((dvConfigOptions){ .output_path = "trace.toml" });

    dvGraphUndirected(g, "demo_graph") {
        dvGuAddNode(g, 0, "甲");
        dvGuAddNode(g, 1, "乙");
        dvGuAddNode(g, 2, "丙");
        dvGuAddNode(g, 3, "丁");

        dvGuAddEdge(g, 0, 1);
        dvGuAddEdge(g, 0, 2);
        dvGuAddEdge(g, 1, 3);
        dvGuAddEdge(g, 2, 3);

        dvGuUpdateNodeLabel(g, 1, "乙*");
        dvGuRemoveEdge(g, 0, 1);
        dvGuRemoveNode(g, 3);
    }

    return 0;
}`,
        },
      },
      {
        id: 'api-graph-directed',
        title: 'GraphDirected',
        content: [
          {
            text: '有向图。API 与无向图一致，区别在于 <code>add_edge</code> 保留方向信息（from → to），可视化时以箭头标识方向。不允许自环，不允许重复边。',
          },
          {
            text: '通过 <code>dv.graph_directed(label="graph")</code> 创建实例。',
            lang: 'python',
          },
          {
            text: '通过 <code>dvGraphDirected(g)</code> 或 <code>dvGraphDirected(g, "graph")</code> 创建实例。',
            lang: 'c',
          },
          {
            text: '支持操作:<br><code>add_node(node_id, label)</code> — 添加节点。<br><code>add_edge(from_id, to_id)</code> — 添加有向边。<br><code>remove_node(node_id)</code> — 删除节点及其入边与出边。<br><code>remove_edge(from_id, to_id)</code> — 删除有向边。<br><code>update_node_label(node_id, label)</code> — 更新节点标签。',
            lang: 'python',
          },
          {
            text: '支持操作:<br><code>dvGdAddNode(g, node_id, label)</code> — 添加节点。<br><code>dvGdAddEdge(g, from_id, to_id)</code> — 添加有向边。<br><code>dvGdRemoveNode(g, node_id)</code> — 删除节点及其入边与出边。<br><code>dvGdRemoveEdge(g, from_id, to_id)</code> — 删除有向边。<br><code>dvGdUpdateNodeLabel(g, node_id, label)</code> — 更新节点标签。',
            lang: 'c',
          },
        ],
        codeBlocks: {
          python: `import ds4viz as dv

with dv.graph_directed(label="demo_digraph") as g:
    with g.phase("构建"):
        g.add_node(0, "甲")
        g.add_node(1, "乙")
        g.add_node(2, "丙")

        g.add_edge(0, 1)    # 甲→乙
        g.add_edge(1, 2)    # 乙→丙
        g.add_edge(2, 0)    # 丙→甲

    with g.phase("修改"):
        g.remove_edge(2, 0)
        g.amend(note="断开 丙→甲 的环", highlights=[
            dv.node(0, "active"),
            dv.node(2, "active")
        ])

        g.add_edge(0, 2)    # 甲→丙`,
          c: `#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"

int main(void) {
    dvConfig((dvConfigOptions){ .output_path = "trace.toml" });

    dvGraphDirected(g, "demo_digraph") {
        dvPhase(g, "构建") {
            dvGdAddNode(g, 0, "甲");
            dvGdAddNode(g, 1, "乙");
            dvGdAddNode(g, 2, "丙");

            dvGdAddEdge(g, 0, 1);
            dvGdAddEdge(g, 1, 2);
            dvGdAddEdge(g, 2, 0);
        }

        dvPhase(g, "修改") {
            dvGdRemoveEdge(g, 2, 0);
            dvAmend(g, "断开 丙→甲 的环",
                dvNode(0, DV_ACTIVE),
                dvNode(2, DV_ACTIVE));

            dvGdAddEdge(g, 0, 2);
        }
    }

    return 0;
}`,
        },
      },
      {
        id: 'api-graph-weighted',
        title: 'GraphWeighted',
        content: [
          {
            text: '带权有向图。<code>add_edge</code> 需要额外传入 <code>weight</code> 参数（<code>int</code> / <code>float</code>），支持 <code>update_weight</code> 更新边权重。可视化时在边上标注权值。',
          },
          {
            text: '通过 <code>dv.graph_weighted(label="graph")</code> 创建实例。',
            lang: 'python',
          },
          {
            text: '通过 <code>dvGraphWeighted(g)</code> 或 <code>dvGraphWeighted(g, "graph")</code> 创建实例。',
            lang: 'c',
          },
          {
            text: '在有向图基础上增加:<br><code>add_edge(from_id, to_id, weight)</code> — 添加带权有向边。<br><code>update_weight(from_id, to_id, weight)</code> — 更新边权重。边不存在时抛出异常。',
            lang: 'python',
          },
          {
            text: '在有向图基础上增加:<br><code>dvGwAddEdge(g, from_id, to_id, weight)</code> — 添加带权有向边。<br><code>dvGwUpdateWeight(g, from_id, to_id, weight)</code> — 更新边权重。边不存在时记录错误。',
            lang: 'c',
          },
          {
            text: '其余操作（<code>add_node</code>、<code>remove_node</code>、<code>remove_edge</code>、<code>update_node_label</code>）与有向图一致。',
          },
        ],
        codeBlocks: {
          python: `import ds4viz as dv

with dv.graph_weighted(label="demo_weighted") as g:
    g.add_node(0, "甲")
    g.alias(0, "source")
    g.add_node(1, "乙")
    g.add_node(2, "丙")

    g.add_edge(0, 1, 3.5)
    g.amend(highlights=[
        dv.node(0, "active"),
        dv.node(1, "active"),
        dv.edge(0, 1, "focus", level=3)
    ])

    g.add_edge(1, 2, 2.0)
    g.add_edge(0, 2, 10.0)

    g.update_weight(0, 2, 5.5)
    g.amend(note="松弛权重: 10.0 → 5.5", highlights=[
        dv.edge(0, 2, "focus", level=3)
    ])`,
          c: `#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"

int main(void) {
    dvConfig((dvConfigOptions){ .output_path = "trace.toml" });

    dvGraphWeighted(g, "demo_weighted") {
        dvGwAddNode(g, 0, "甲");
        dvAlias(g, 0, "source");
        dvGwAddNode(g, 1, "乙");
        dvGwAddNode(g, 2, "丙");

        dvGwAddEdge(g, 0, 1, 3.5);
        dvAmendHL(g,
            dvNode(0, DV_ACTIVE),
            dvNode(1, DV_ACTIVE),
            dvEdge(0, 1, DV_FOCUS, 3));

        dvGwAddEdge(g, 1, 2, 2.0);
        dvGwAddEdge(g, 0, 2, 10.0);

        dvGwUpdateWeight(g, 0, 2, 5.5);
        dvAmend(g, "松弛权重: 10.0 → 5.5",
            dvEdge(0, 2, DV_FOCUS, 3));
    }

    return 0;
}`,
        },
      },
    ],
  },

  /* ============================================================
   *  7. 错误处理
   * ============================================================ */
  {
    id: 'errors',
    title: '错误处理',
    content: [
      {
        text: '所有数据结构在遇到非法操作时会进入错误状态。上下文管理器 / 作用域宏保证即使发生错误，<code>.toml</code> 文件仍然会生成，包含 <code>[error]</code> 部分而非 <code>[result]</code>。',
      },
    ],
    children: [
      {
        id: 'errors-python',
        title: 'Python 错误处理',
        lang: 'python',
        content: [
          {
            text: '非法操作抛出 <code>RuntimeError</code>。上下文管理器捕获异常，在退出时自动写入包含 <code>[error]</code> 的 <code>.toml</code> 文件。',
          },
          {
            text: '异常发生后，<code>with</code> 块内后续代码不再执行（Python 正常异常传播行为）。<code>.toml</code> 文件中 <code>[error]</code> 包含 <code>type</code>、<code>message</code>、以及可选的 <code>line</code>、<code>step</code>、<code>last_state</code> 字段。',
          },
          {
            text: '<code>[error].type</code> 的可能取值：<code>runtime</code>、<code>timeout</code>、<code>validation</code>、<code>sandbox</code>、<code>unknown</code>。',
          },
        ],
        codeBlocks: {
          python: `import ds4viz as dv

try:
    with dv.stack(label="错误演示") as s:
        s.push(10)
        s.push(20)
        s.pop()
        s.pop()
        s.pop()     # 栈已空，抛出 RuntimeError
        s.push(30)  # 不执行
except RuntimeError as e:
    print(f"捕获错误: {e}")

# trace.toml 已生成，包含前 4 步的正常记录 + [error]`,
        },
      },
      {
        id: 'errors-c',
        title: 'C 错误处理',
        lang: 'c',
        content: [
          {
            text: '非法操作记录错误信息但不终止程序。错误发生后，同一作用域内后续的所有操作被静默跳过：返回 ID 的操作返回 <code>-1</code>，返回 <code>dvValue</code> 的操作返回零值。',
          },
          {
            text: '作用域结束时，<code>.toml</code> 文件包含 <code>[error]</code> 部分。这种设计确保 C 程序不会因为 ds4viz 操作异常而崩溃。',
          },
        ],
        codeBlocks: {
          c: `#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"

int main(void) {
    dvConfig((dvConfigOptions){ .title = "错误处理演示" });

    dvStack(s, "错误演示") {
        dvStackPush(s, 10);
        dvStackPush(s, 20);
        dvStackPop(s);
        dvStackPop(s);
        dvStackPop(s);         // 栈已空，记录错误
        dvStackPush(s, 30);    // 静默跳过，不执行
    }
    // trace.toml 已生成，包含 [error]

    return 0;
}`,
        },
      },
    ],
  },

  /* ============================================================
   *  8. 使用示例
   * ============================================================ */
  {
    id: 'examples',
    title: '使用示例',
    content: [
      {
        text: '以下示例演示常见的数据结构操作场景，展示 <code>step()</code>、<code>highlights</code> 和 <code>phase</code> 的典型用法。',
      },
    ],
    children: [
      {
        id: 'example-preorder',
        title: '二叉树前序遍历',
        content: [
          {
            text: '构建二叉树后，使用 <code>step()</code> 逐步记录前序遍历（根 → 左 → 右）的访问过程。每一步标记已访问节点为 <code>visited</code>，当前访问节点为 <code>focus</code>。',
          },
        ],
        codeBlocks: {
          python: `import ds4viz as dv

dv.config(title="二叉树前序遍历")

with dv.binary_tree(label="前序遍历") as tree:
    with tree.phase("构建"):
        root = tree.insert_root(10)
        tree.alias(root, "root")
        n5  = tree.insert_left(root, 5)
        n15 = tree.insert_right(root, 15)
        n3  = tree.insert_left(n5, 3)
        n7  = tree.insert_right(n5, 7)
        n12 = tree.insert_left(n15, 12)

    visited = []

    def visit(node_id, label):
        tree.step(
            note=f"访问节点 {label}",
            highlights=[
                *[dv.node(v, "visited") for v in visited],
                dv.node(node_id, "focus", level=3)
            ]
        )
        visited.append(node_id)

    with tree.phase("前序遍历"):
        visit(root, 10)
        visit(n5, 5)
        visit(n3, 3)
        visit(n7, 7)
        visit(n15, 15)
        visit(n12, 12)`,
          c: `#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"

int main(void) {
    dvConfig((dvConfigOptions){ .title = "二叉树前序遍历" });

    dvBinaryTree(tree, "前序遍历") {
        int root, n5, n15, n3, n7, n12;

        dvPhase(tree, "构建") {
            root = dvBtInsertRoot(tree, 10);
            dvAlias(tree, root, "root");
            n5   = dvBtInsertLeft(tree, root, 5);
            n15  = dvBtInsertRight(tree, root, 15);
            n3   = dvBtInsertLeft(tree, n5, 3);
            n7   = dvBtInsertRight(tree, n5, 7);
            n12  = dvBtInsertLeft(tree, n15, 12);
        }

        dvPhase(tree, "前序遍历") {
            dvStep(tree, "访问节点 10",
                dvNode(root, DV_FOCUS, 3));
            dvStep(tree, "访问节点 5",
                dvNode(root, DV_VISITED),
                dvNode(n5, DV_FOCUS, 3));
            dvStep(tree, "访问节点 3",
                dvNode(root, DV_VISITED),
                dvNode(n5, DV_VISITED),
                dvNode(n3, DV_FOCUS, 3));
            dvStep(tree, "访问节点 7",
                dvNode(root, DV_VISITED),
                dvNode(n5, DV_VISITED),
                dvNode(n3, DV_VISITED),
                dvNode(n7, DV_FOCUS, 3));
            dvStep(tree, "访问节点 15",
                dvNode(root, DV_VISITED),
                dvNode(n5, DV_VISITED),
                dvNode(n3, DV_VISITED),
                dvNode(n7, DV_VISITED),
                dvNode(n15, DV_FOCUS, 3));
            dvStep(tree, "访问节点 12",
                dvNode(root, DV_VISITED),
                dvNode(n5, DV_VISITED),
                dvNode(n3, DV_VISITED),
                dvNode(n7, DV_VISITED),
                dvNode(n15, DV_VISITED),
                dvNode(n12, DV_FOCUS, 3));
        }
    }

    return 0;
}`,
        },
      },
      {
        id: 'example-bst-search',
        title: 'BST 查找',
        content: [
          {
            text: '使用 <code>step()</code> 记录在 BST 中查找目标值的比较路径。每一步标记比较节点为 <code>comparing</code>，已排除节点为 <code>visited</code>，命中节点为 <code>found</code>。',
          },
        ],
        codeBlocks: {
          python: `import ds4viz as dv

dv.config(title="BST 查找 7")

with dv.binary_search_tree(label="BST查找") as bst:
    with bst.phase("构建"):
        n10 = bst.insert(10)
        bst.alias(n10, "root")
        n5  = bst.insert(5)
        n15 = bst.insert(15)
        n3  = bst.insert(3)
        n7  = bst.insert(7)
        n12 = bst.insert(12)
        n20 = bst.insert(20)

    with bst.phase("查找"):
        bst.step(note="7 < 10, 向左", highlights=[
            dv.node(n10, "comparing", level=3)
        ])
        bst.step(note="7 > 5, 向右", highlights=[
            dv.node(n10, "visited"),
            dv.node(n5, "comparing", level=3)
        ])
        bst.step(note="7 == 7, 找到!", highlights=[
            dv.node(n10, "visited"),
            dv.node(n5, "visited"),
            dv.node(n7, "found", level=3)
        ])`,
          c: `#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"

int main(void) {
    dvConfig((dvConfigOptions){ .title = "BST 查找 7" });

    dvBinarySearchTree(bst, "BST查找") {
        int n10, n5, n15, n3, n7, n12, n20;

        dvPhase(bst, "构建") {
            n10 = dvBstInsert(bst, 10);
            dvAlias(bst, n10, "root");
            n5  = dvBstInsert(bst, 5);
            n15 = dvBstInsert(bst, 15);
            n3  = dvBstInsert(bst, 3);
            n7  = dvBstInsert(bst, 7);
            n12 = dvBstInsert(bst, 12);
            n20 = dvBstInsert(bst, 20);
        }

        dvPhase(bst, "查找") {
            dvStep(bst, "7 < 10, 向左",
                dvNode(n10, DV_COMPARING, 3));
            dvStep(bst, "7 > 5, 向右",
                dvNode(n10, DV_VISITED),
                dvNode(n5, DV_COMPARING, 3));
            dvStep(bst, "7 == 7, 找到!",
                dvNode(n10, DV_VISITED),
                dvNode(n5, DV_VISITED),
                dvNode(n7, DV_FOUND, 3));
        }
    }

    return 0;
}`,
        },
      },
      {
        id: 'example-bfs',
        title: '无向图 BFS',
        content: [
          {
            text: '从节点甲出发，广度优先遍历无向图。使用 <code>step()</code> 记录每次出队与邻居发现过程。边高亮用于标示当前探索的边。',
          },
        ],
        codeBlocks: {
          python: `import ds4viz as dv

dv.config(title="无向图 BFS")

with dv.graph_undirected(label="BFS") as g:
    g.add_node(0, "甲")
    g.add_node(1, "乙")
    g.add_node(2, "丙")
    g.add_node(3, "丁")
    g.add_node(4, "戊")

    g.add_edge(0, 1)
    g.add_edge(0, 2)
    g.add_edge(1, 3)
    g.add_edge(2, 3)
    g.add_edge(3, 4)

    g.step(note="起点 甲 入队", highlights=[
        dv.node(0, "focus", level=3)
    ])
    g.step(note="出队 甲, 发现 乙/丙", highlights=[
        dv.node(0, "visited"),
        dv.node(1, "focus", level=2),
        dv.node(2, "focus", level=2),
        dv.edge(0, 1, "active"),
        dv.edge(0, 2, "active")
    ])
    g.step(note="出队 乙, 发现 丁", highlights=[
        dv.node(0, "visited"),
        dv.node(1, "visited"),
        dv.node(2, "active"),
        dv.node(3, "focus", level=2),
        dv.edge(1, 3, "active")
    ])
    g.step(note="出队 丁, 发现 戊", highlights=[
        dv.node(0, "visited"),
        dv.node(1, "visited"),
        dv.node(2, "visited"),
        dv.node(3, "visited"),
        dv.node(4, "focus", level=2),
        dv.edge(3, 4, "active")
    ])
    g.step(note="BFS 完成", highlights=[
        dv.node(0, "visited"),
        dv.node(1, "visited"),
        dv.node(2, "visited"),
        dv.node(3, "visited"),
        dv.node(4, "visited")
    ])`,
          c: `#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"

int main(void) {
    dvConfig((dvConfigOptions){ .title = "无向图 BFS" });

    dvGraphUndirected(g, "BFS") {
        dvGuAddNode(g, 0, "甲");
        dvGuAddNode(g, 1, "乙");
        dvGuAddNode(g, 2, "丙");
        dvGuAddNode(g, 3, "丁");
        dvGuAddNode(g, 4, "戊");

        dvGuAddEdge(g, 0, 1);
        dvGuAddEdge(g, 0, 2);
        dvGuAddEdge(g, 1, 3);
        dvGuAddEdge(g, 2, 3);
        dvGuAddEdge(g, 3, 4);

        dvStep(g, "起点 甲 入队",
            dvNode(0, DV_FOCUS, 3));
        dvStep(g, "出队 甲, 发现 乙/丙",
            dvNode(0, DV_VISITED),
            dvNode(1, DV_FOCUS, 2),
            dvNode(2, DV_FOCUS, 2),
            dvEdge(0, 1, DV_ACTIVE),
            dvEdge(0, 2, DV_ACTIVE));
        dvStep(g, "出队 乙, 发现 丁",
            dvNode(0, DV_VISITED),
            dvNode(1, DV_VISITED),
            dvNode(2, DV_ACTIVE),
            dvNode(3, DV_FOCUS, 2),
            dvEdge(1, 3, DV_ACTIVE));
        dvStep(g, "出队 丁, 发现 戊",
            dvNode(0, DV_VISITED),
            dvNode(1, DV_VISITED),
            dvNode(2, DV_VISITED),
            dvNode(3, DV_VISITED),
            dvNode(4, DV_FOCUS, 2),
            dvEdge(3, 4, DV_ACTIVE));
        dvStep(g, "BFS 完成",
            dvNode(0, DV_VISITED),
            dvNode(1, DV_VISITED),
            dvNode(2, DV_VISITED),
            dvNode(3, DV_VISITED),
            dvNode(4, DV_VISITED));
    }

    return 0;
}`,
        },
      },
      {
        id: 'example-dijkstra',
        title: 'Dijkstra 松弛',
        content: [
          {
            text: '使用带权图演示 Dijkstra 最短路径的边松弛过程。每一步标记当前松弛节点和边，最终标记最短路径。',
          },
        ],
        codeBlocks: {
          python: `import ds4viz as dv

dv.config(title="Dijkstra 松弛过程")

with dv.graph_weighted(label="最短路径") as g:
    with g.phase("构建"):
        g.add_node(0, "起点")
        g.alias(0, "source")
        g.add_node(1, "甲")
        g.add_node(2, "乙")
        g.add_node(3, "终点")
        g.add_edge(0, 1, 1)
        g.add_edge(0, 2, 4)
        g.add_edge(1, 2, 2)
        g.add_edge(1, 3, 6)
        g.add_edge(2, 3, 3)

    with g.phase("松弛"):
        g.step(note="从起点松弛: dist[甲]=1, dist[乙]=4", highlights=[
            dv.node(0, "visited"),
            dv.node(1, "focus", level=2),
            dv.node(2, "focus", level=2),
            dv.edge(0, 1, "active", level=3),
            dv.edge(0, 2, "active")
        ])
        g.step(note="从甲松弛: dist[乙]=3, dist[终点]=7", highlights=[
            dv.node(0, "visited"),
            dv.node(1, "visited"),
            dv.node(2, "focus", level=2),
            dv.node(3, "focus", level=2),
            dv.edge(1, 2, "active", level=3),
            dv.edge(1, 3, "active")
        ])
        g.step(note="从乙松弛: dist[终点]=6", highlights=[
            dv.node(0, "visited"),
            dv.node(1, "visited"),
            dv.node(2, "visited"),
            dv.node(3, "focus", level=3),
            dv.edge(2, 3, "active", level=3)
        ])
        g.step(note="最短路径: 起点→甲→乙→终点, 距离=6", highlights=[
            dv.node(0, "found"), dv.node(1, "found"),
            dv.node(2, "found"), dv.node(3, "found"),
            dv.edge(0, 1, "found", level=3),
            dv.edge(1, 2, "found", level=3),
            dv.edge(2, 3, "found", level=3)
        ])`,
          c: `#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"

int main(void) {
    dvConfig((dvConfigOptions){ .title = "Dijkstra 松弛过程" });

    dvGraphWeighted(g, "最短路径") {
        dvPhase(g, "构建") {
            dvGwAddNode(g, 0, "起点");
            dvAlias(g, 0, "source");
            dvGwAddNode(g, 1, "甲");
            dvGwAddNode(g, 2, "乙");
            dvGwAddNode(g, 3, "终点");
            dvGwAddEdge(g, 0, 1, 1);
            dvGwAddEdge(g, 0, 2, 4);
            dvGwAddEdge(g, 1, 2, 2);
            dvGwAddEdge(g, 1, 3, 6);
            dvGwAddEdge(g, 2, 3, 3);
        }

        dvPhase(g, "松弛") {
            dvStep(g, "从起点松弛: dist[甲]=1, dist[乙]=4",
                dvNode(0, DV_VISITED),
                dvNode(1, DV_FOCUS, 2),
                dvNode(2, DV_FOCUS, 2),
                dvEdge(0, 1, DV_ACTIVE, 3),
                dvEdge(0, 2, DV_ACTIVE));
            dvStep(g, "从甲松弛: dist[乙]=3, dist[终点]=7",
                dvNode(0, DV_VISITED),
                dvNode(1, DV_VISITED),
                dvNode(2, DV_FOCUS, 2),
                dvNode(3, DV_FOCUS, 2),
                dvEdge(1, 2, DV_ACTIVE, 3),
                dvEdge(1, 3, DV_ACTIVE));
            dvStep(g, "从乙松弛: dist[终点]=6",
                dvNode(0, DV_VISITED),
                dvNode(1, DV_VISITED),
                dvNode(2, DV_VISITED),
                dvNode(3, DV_FOCUS, 3),
                dvEdge(2, 3, DV_ACTIVE, 3));
            dvStep(g, "最短路径: 起点→甲→乙→终点, 距离=6",
                dvNode(0, DV_FOUND),
                dvNode(1, DV_FOUND),
                dvNode(2, DV_FOUND),
                dvNode(3, DV_FOUND),
                dvEdge(0, 1, DV_FOUND, 3),
                dvEdge(1, 2, DV_FOUND, 3),
                dvEdge(2, 3, DV_FOUND, 3));
        }
    }

    return 0;
}`,
        },
      },
    ],
  },

  /* ============================================================
   *  9. 自部署服务
   * ============================================================ */
  {
    id: 'deploy',
    title: '自部署服务',
    content: [
      {
        text: 'ds4viz 是开源的，可自行部署在线服务。需要配置运行环境，安装对应语言的 ds4viz 库，并部署 <code>FastAPI</code> 后端与 <code>Vue</code> 前端。详情参阅 GitHub 仓库中的部署文档。',
      },
    ],
    children: [
      {
        id: 'deploy-install',
        title: '库安装',
        content: [
          { text: '按需安装语言库。' },
          {
            text: '通过 <code>pip</code> 安装。',
            lang: 'python',
          },
          {
            text: '下载 <code>ds4viz.h</code> 单头文件，在一个 <code>.c</code> 文件中定义 <code>DS4VIZ_IMPLEMENTATION</code> 后 <code>include</code> 即可。可选定义 <code>DS4VIZ_SHORT_NAMES</code> 启用 <code>dv</code> 短名前缀。',
            lang: 'c',
          },
        ],
        codeBlocks: {
          python: `# 安装 Python 库
pip install ds4viz

# 验证安装
python -c "import ds4viz; print('ok')"`,
          c: `// 下载 ds4viz.h 后放入项目目录
// 编译命令 (需要 C23 支持):
// gcc -std=c2x main.c -o main

#define DS4VIZ_IMPLEMENTATION
#define DS4VIZ_SHORT_NAMES
#include "ds4viz.h"

int main(void) {
    dvConfig((dvConfigOptions){
        .title = "安装验证"
    });

    dvStack(s) {
        dvStackPush(s, 42);
    }

    return 0;
}`,
        },
      },
    ],
  },
])

/* ================================================================
 *  代码块辅助
 * ================================================================ */

/**
 * 获取代码块有效显示语言
 *
 * @param codeBlocks - 代码块映射
 * @returns 当前应展示语言
 */
const getEffectiveLanguage = (
  codeBlocks: Partial<Record<Language, string>>,
): Language => {
  if (codeBlocks[selectedLanguage.value] !== undefined) {
    return selectedLanguage.value
  }
  const available = Object.keys(codeBlocks) as Language[]
  return available.length > 0 ? available[0] : 'python'
}

/**
 * 获取语言对应色点颜色
 *
 * @param language - 语言
 * @returns 颜色值
 */
const getLanguageDotColor = (language: Language): string => {
  const colors: Record<Language, string> = {
    python: 'var(--color-lang-python)',
    c: 'var(--color-lang-c)',
  }
  return colors[language] ?? 'var(--color-text-tertiary)'
}

/**
 * 处理全局语言切换
 *
 * @param event - change 事件
 */
const handleLanguageChange = (event: Event): void => {
  const target = event.target as HTMLSelectElement
  selectedLanguage.value = target.value as Language
}

/**
 * 计算编辑器高度
 *
 * @param codeBlocks - 代码块映射
 * @returns CSS 高度
 */
const getEditorHeight = (
  codeBlocks: Partial<Record<Language, string>>,
): string => {
  const code = codeBlocks[getEffectiveLanguage(codeBlocks)] ?? ''
  const lines = code.split('\n').length
  return `${Math.min(480, Math.max(60, lines * 20 + 24))}px`
}

/**
 * 复制代码到剪贴板
 *
 * @param code - 代码
 * @param sectionId - 子节点 id
 */
const handleCopyCode = async (code: string, sectionId: string): Promise<void> => {
  try {
    await navigator.clipboard.writeText(code)
    copiedId.value = sectionId
    window.setTimeout(() => {
      if (copiedId.value === sectionId) {
        copiedId.value = ''
      }
    }, 2000)
  } catch {
    /* clipboard 不可用 */
  }
}

/**
 * 在编辑器中打开
 *
 * @param code - 代码
 * @param language - 语言
 */
const handleOpenInEditor = (code: string, language: Language): void => {
  try {
    sessionStorage.setItem('ds4viz_edit_code', code)
    sessionStorage.setItem('ds4viz_edit_language', language)
  } catch {
    /* sessionStorage 不可用 */
  }
  router.push({ name: 'playground' })
}

/* ================================================================
 *  TOC 交互
 * ================================================================ */

/**
 * 切换节点展开状态
 *
 * @param id - 节点 id
 */
const toggleNode = (id: string): void => {
  expanded.value[id] = !expanded.value[id]
}

/**
 * 滚动到指定文档节
 *
 * @param id - 目标节点 id
 */
const scrollToSection = (id: string): void => {
  const target = document.getElementById(id)
  if (!target || !contentRef.value) return
  const container = contentRef.value
  const containerRect = container.getBoundingClientRect()
  const targetRect = target.getBoundingClientRect()
  const topOffset = 20
  const scrollPos = targetRect.top - containerRect.top + container.scrollTop - topOffset
  container.scrollTo({ top: scrollPos, behavior: 'smooth' })
}

/**
 * 收集当前 DOM 中实际存在的所有文档节点 id（按文档顺序）
 *
 * @returns id 数组
 */
const getAllRenderedSectionIds = (): string[] => {
  const ids: string[] = []
  for (const section of docTree.value) {
    ids.push(section.id)
    for (const child of getChildren(section)) {
      ids.push(child.id)
    }
  }
  return ids
}

/**
 * 根据当前滚动位置更新 activeId
 */
const updateActiveSection = (): void => {
  if (!contentRef.value) return
  const container = contentRef.value
  const containerRect = container.getBoundingClientRect()
  const threshold = 48 // px，距容器顶部多少以内算"已进入"

  const ids = getAllRenderedSectionIds()
  let current = ids[0] ?? ''

  for (const id of ids) {
    const el = document.getElementById(id)
    if (!el) continue
    const top = el.getBoundingClientRect().top - containerRect.top
    if (top <= threshold) {
      current = id
    }
  }

  activeId.value = current
}

/** 清除滚动监听的回调 */
let _scrollUnlisten: (() => void) | null = null

onMounted(() => {
  const stored = localStorage.getItem('ds4viz_language') as Language | null
  if (stored && LANGUAGES.includes(stored)) {
    selectedLanguage.value = stored
  }

  if (contentRef.value) {
    contentRef.value.addEventListener('scroll', updateActiveSection, { passive: true })
    _scrollUnlisten = () => contentRef.value?.removeEventListener('scroll', updateActiveSection)
    updateActiveSection()
  }
})

onUnmounted(() => {
  _scrollUnlisten?.()
})
</script>

<template>
  <div class="docs-page">
    <header class="docs-page__header">
      <div class="docs-page__title">
        <MaterialIcon name="menu_book" :size="18" />
        <span>文档</span>
      </div>

      <div class="docs-page__lang-switch">
        <span class="docs-page__lang-dot" :style="{ backgroundColor: getLanguageDotColor(selectedLanguage) }" />
        <select class="docs-page__lang-dropdown" :value="selectedLanguage" @change="handleLanguageChange">
          <option v-for="lang in LANGUAGES" :key="lang" :value="lang">
            {{ LANGUAGE_LABELS[lang] ?? lang }}
          </option>
        </select>
      </div>
    </header>

    <div class="docs-page__body">
      <aside class="docs-page__toc">
        <div v-for="section in docTree" :key="section.id" class="toc-section">
          <button class="toc-section__title" :class="{ 'toc-section__title--active': activeId === section.id }"
            @click="toggleNode(section.id); scrollToSection(section.id)">
            <MaterialIcon :name="getToggleIcon(expanded[section.id])" :size="16" />
            <span>{{ section.title }}</span>
          </button>
          <div v-if="expanded[section.id]" class="toc-section__children">
            <button v-for="child in getAllChildren(section)" :key="child.id" class="toc-section__item" :class="{
              'toc-section__item--hidden': !!child.lang && child.lang !== selectedLanguage,
              'toc-section__item--active': activeId === child.id
            }" @click="scrollToSection(child.id)">
              {{ child.title }}
            </button>
          </div>
        </div>
      </aside>

      <section ref="contentRef" class="docs-page__content">
        <!-- GitHub 引导 -->
        <div class="docs-page__github-banner">
          <svg class="docs-page__github-icon" viewBox="0 0 16 16" fill="currentColor">
            <path
              d="M8 0C3.58 0 0 3.58 0 8c0 3.54 2.29 6.53 5.47 7.59.4.07.55-.17.55-.38 0-.19-.01-.82-.01-1.49-2.01.37-2.53-.49-2.69-.94-.09-.23-.48-.94-.82-1.13-.28-.15-.68-.52-.01-.53.63-.01 1.08.58 1.23.82.72 1.21 1.87.87 2.33.66.07-.52.28-.87.51-1.07-1.78-.2-3.64-.89-3.64-3.95 0-.87.31-1.59.82-2.15-.08-.2-.36-1.02.08-2.12 0 0 .67-.21 2.2.82.64-.18 1.32-.27 2-.27.68 0 1.36.09 2 .27 1.53-1.04 2.2-.82 2.2-.82.44 1.1.16 1.92.08 2.12.51.56.82 1.27.82 2.15 0 3.07-1.87 3.75-3.65 3.95.29.25.54.73.54 1.48 0 1.07-.01 1.93-.01 2.2 0 .21.15.46.55.38A8.013 8.013 0 0016 8c0-4.42-3.58-8-8-8z" />
          </svg>
          <span>完整文档、IR 定义、语言库源码与渲染器实现见</span>
          <a href="https://github.com/Water-Run/ds4viz" target="_blank" rel="noopener noreferrer">GitHub 仓库</a>
        </div>

        <!-- 文档正文 -->
        <div v-for="section in docTree" :key="section.id" :id="section.id" class="doc-section">
          <h2 class="doc-section__title">{{ section.title }}</h2>
          <p v-for="(para, pIdx) in filterContent(section.content)" :key="`${section.id}-sp-${pIdx}`"
            class="doc-section__desc" v-html="para.text" />

          <div v-for="child in getChildren(section)" :key="child.id" :id="child.id" class="doc-subsection">
            <h3 class="doc-subsection__title">{{ child.title }}</h3>
            <p v-for="(para, pIdx) in filterContent(child.content)" :key="`${child.id}-cp-${pIdx}`"
              class="doc-subsection__desc" v-html="para.text" />

            <div v-if="hasCodeBlocks(child)" class="doc-code-block">
              <div class="doc-code-block__toolbar">
                <div class="doc-code-block__actions">
                  <button class="code-action-btn" :class="{ 'code-action-btn--copied': copiedId === child.id }"
                    @click="handleCopyCode(child.codeBlocks![getEffectiveLanguage(child.codeBlocks!)] ?? '', child.id)">
                    <svg v-if="copiedId !== child.id" viewBox="0 0 24 24" fill="currentColor">
                      <path
                        d="M16 1H4c-1.1 0-2 .9-2 2v14h2V3h12V1zm3 4H8c-1.1 0-2 .9-2 2v14c0 1.1.9 2 2 2h11c1.1 0 2-.9 2-2V7c0-1.1-.9-2-2-2zm0 16H8V7h11v14z" />
                    </svg>
                    <svg v-else viewBox="0 0 24 24" fill="currentColor">
                      <path d="M9 16.17L4.83 12l-1.42 1.41L9 19 21 7l-1.41-1.41z" />
                    </svg>
                    <span>{{ copiedId === child.id ? '已复制' : '复制' }}</span>
                  </button>

                  <button class="code-action-btn code-action-btn--primary" @click="handleOpenInEditor(
                    child.codeBlocks![getEffectiveLanguage(child.codeBlocks!)] ?? '',
                    getEffectiveLanguage(child.codeBlocks!)
                  )">
                    <svg viewBox="0 0 24 24" fill="currentColor">
                      <path
                        d="M9.4 16.6L4.8 12l4.6-4.6L8 6l-6 6 6 6 1.4-1.4zm5.2 0l4.6-4.6-4.6-4.6L16 6l6 6-6 6-1.4-1.4z" />
                    </svg>
                    <span>在编辑器中打开</span>
                  </button>
                </div>

                <div class="doc-code-block__lang-badge">
                  <span class="doc-code-block__dot"
                    :style="{ backgroundColor: getLanguageDotColor(getEffectiveLanguage(child.codeBlocks!)) }" />
                  <span>{{ LANGUAGE_LABELS[getEffectiveLanguage(child.codeBlocks!)] ??
                    getEffectiveLanguage(child.codeBlocks!) }}</span>
                </div>
              </div>

              <div class="doc-code-block__editor" :style="{ height: getEditorHeight(child.codeBlocks!) }">
                <CodeEditor :model-value="child.codeBlocks![getEffectiveLanguage(child.codeBlocks!)] ?? ''"
                  :language="getEffectiveLanguage(child.codeBlocks!)" :readonly="true" />
              </div>
            </div>
          </div>
        </div>
      </section>
    </div>
  </div>
</template>

<style scoped>
.docs-page {
  display: flex;
  flex-direction: column;
  gap: var(--space-2);
  padding: var(--space-3);
  height: 100%;
  overflow: hidden;
}

.docs-page__header {
  display: flex;
  align-items: center;
  justify-content: space-between;
  flex-shrink: 0;
}

.docs-page__title {
  display: flex;
  align-items: center;
  gap: 6px;
  font-size: var(--text-base);
  font-weight: var(--weight-semibold);
  color: var(--color-text-primary);
}

.docs-page__title :deep(.material-icon) {
  width: 20px;
  height: 20px;
}

/* ---- 全局语言选择器 ---- */

.docs-page__lang-switch {
  display: inline-flex;
  align-items: center;
  gap: 6px;
  padding: 0 10px;
  height: 30px;
  border: 1px solid var(--color-border-strong);
  border-radius: var(--radius-control);
  background-color: var(--color-bg-surface);
}

.docs-page__lang-dot {
  width: 7px;
  height: 7px;
  border-radius: 999px;
  flex-shrink: 0;
}

.docs-page__lang-dropdown {
  font-size: var(--text-sm);
  color: var(--color-text-primary);
  background: none;
  border: none;
  outline: none;
  cursor: pointer;
  font-family: inherit;
}

/* ---- Body ---- */

.docs-page__body {
  display: grid;
  grid-template-columns: 200px 1fr;
  gap: var(--space-3);
  flex: 1;
  min-height: 0;
  overflow: hidden;
}

/* ---- TOC ---- */

.docs-page__toc {
  border: 1px solid var(--color-border);
  border-radius: var(--radius-lg);
  padding: var(--space-2);
  background-color: var(--color-bg-surface);
  overflow-y: auto;
}

.toc-section {
  display: flex;
  flex-direction: column;
  gap: 2px;
  margin-bottom: var(--space-2);
}

.toc-section__title {
  display: flex;
  align-items: center;
  gap: 4px;
  border: none;
  background: none;
  font-size: 0.934rem;
  font-weight: var(--weight-semibold);
  color: var(--color-text-primary);
  cursor: pointer;
  padding: 4px 0;
  border-radius: var(--radius-sm);
  transition: color var(--duration-fast) var(--ease);
}

.toc-section__title:hover {
  color: var(--color-accent);
}

.toc-section__title :deep(.material-icon) {
  width: 16px;
  height: 16px;
}

.toc-section__children {
  display: flex;
  flex-direction: column;
  gap: 2px;
  padding-left: 20px;
}

.toc-section__item {
  border: none;
  background: none;
  text-align: left;
  font-size: 0.8625rem;
  color: var(--color-text-tertiary);
  cursor: pointer;
  padding: 3px 6px;
  border-radius: var(--radius-sm);
  transition:
    background-color var(--duration-fast) var(--ease),
    color var(--duration-fast) var(--ease),
    opacity var(--duration-fast) var(--ease);
}

.toc-section__item:hover {
  background-color: var(--color-bg-hover);
  color: var(--color-accent);
}

.toc-section__item--hidden {
  display: none;
}

.toc-section__title--active {
  color: var(--color-accent);
}

.toc-section__title--active :deep(.material-icon) {
  color: var(--color-accent);
}

.toc-section__item--active {
  background-color: var(--color-bg-hover);
  color: var(--color-accent);
  font-weight: var(--weight-medium);
}

/* ---- Content ---- */

.docs-page__content {
  border: 1px solid var(--color-border);
  border-radius: var(--radius-lg);
  padding: var(--space-4) var(--space-4);
  background-color: var(--color-bg-surface);
  overflow-y: auto;
}

/* ---- GitHub banner ---- */

.docs-page__github-banner {
  display: flex;
  align-items: center;
  gap: 8px;
  padding: 10px 16px;
  margin-bottom: var(--space-3);
  border: 1px solid var(--color-border);
  border-radius: var(--radius-md);
  background-color: var(--color-bg-surface-alt);
  font-size: 0.934rem;
  color: var(--color-text-body);
}

.docs-page__github-icon {
  width: 18px;
  height: 18px;
  flex-shrink: 0;
  color: var(--color-text-primary);
}

.docs-page__github-banner a {
  color: var(--color-accent);
  font-weight: var(--weight-medium);
  text-decoration: none;
  transition: color var(--duration-fast) var(--ease);
}

.docs-page__github-banner a:hover {
  color: var(--color-accent-hover);
  text-decoration: underline;
  text-underline-offset: 3px;
}

/* ---- Section ---- */

.doc-section {
  margin-bottom: var(--space-5);
  scroll-margin-top: var(--space-3);
}

.doc-section__title {
  margin: 0 0 var(--space-2);
  font-size: 1.38rem;
  font-weight: var(--weight-semibold);
  padding-bottom: var(--space-1);
  border-bottom: 1px solid var(--color-border);
}

.doc-section__desc {
  margin: 0 0 var(--space-2);
  font-size: 0.934rem;
  color: var(--color-text-body);
  line-height: var(--leading-relaxed);
}

.doc-section__desc :deep(code) {
  font-family: var(--font-mono, monospace);
  font-size: 0.875em;
  padding: 1px 5px;
  border-radius: var(--radius-sm);
  background-color: var(--color-bg-surface-alt);
  color: var(--color-text-primary);
  white-space: nowrap;
}

.doc-section__desc :deep(b) {
  font-weight: var(--weight-semibold);
  color: var(--color-text-primary);
}

/* ---- Subsection ---- */

.doc-subsection {
  margin-bottom: var(--space-4);
  scroll-margin-top: var(--space-3);
}

.doc-subsection__title {
  margin: 0 0 var(--space-1);
  font-size: 1.1rem;
  font-weight: var(--weight-semibold);
}

.doc-subsection__desc {
  margin: 0 0 var(--space-2);
  font-size: 0.934rem;
  color: var(--color-text-body);
  line-height: 1.75;
}

.doc-subsection__desc :deep(code) {
  font-family: var(--font-mono, monospace);
  font-size: 0.875em;
  padding: 1px 5px;
  border-radius: var(--radius-sm);
  background-color: var(--color-bg-surface-alt);
  color: var(--color-text-primary);
  white-space: nowrap;
}

.doc-subsection__desc :deep(b) {
  font-weight: var(--weight-semibold);
  color: var(--color-text-primary);
}

/* ---- Code block ---- */

.doc-code-block {
  position: relative;
  border: 1px solid var(--color-border);
  border-radius: var(--radius-md);
  background-color: var(--color-bg-surface-alt);
  overflow: hidden;
  margin-top: var(--space-2);
  margin-bottom: var(--space-1);
}

.doc-code-block__toolbar {
  display: flex;
  align-items: center;
  justify-content: space-between;
  gap: var(--space-1);
  padding: 6px 8px;
  border-bottom: 1px solid var(--color-border);
  background-color: var(--color-bg-surface);
  flex-wrap: wrap;
}

.doc-code-block__actions {
  display: flex;
  align-items: center;
  gap: var(--space-1);
}

.doc-code-block__lang-badge {
  display: inline-flex;
  align-items: center;
  gap: 6px;
  padding: 0 10px;
  height: 28px;
  font-size: var(--text-xs);
  color: var(--color-text-tertiary);
}

.doc-code-block__dot {
  width: 7px;
  height: 7px;
  border-radius: 999px;
  flex-shrink: 0;
}

.doc-code-block__editor {
  display: flex;
  border-radius: 0;
  overflow: hidden;
}

/* ---- Action buttons ---- */

.code-action-btn {
  display: inline-flex;
  align-items: center;
  gap: 4px;
  height: 28px;
  padding: 0 8px;
  border: 1px solid var(--color-border-strong);
  border-radius: var(--radius-control);
  background-color: var(--color-bg-surface);
  color: var(--color-text-body);
  font-size: var(--text-xs);
  font-family: inherit;
  cursor: pointer;
  transition:
    background-color var(--duration-fast) var(--ease),
    border-color var(--duration-fast) var(--ease),
    color var(--duration-fast) var(--ease);
}

.code-action-btn:hover {
  background-color: var(--color-bg-hover);
  color: var(--color-text-primary);
}

.code-action-btn svg {
  width: 14px;
  height: 14px;
  flex-shrink: 0;
}

.code-action-btn--copied {
  border-color: var(--color-success);
  color: var(--color-success);
}

.code-action-btn--primary {
  border-color: var(--color-accent);
  background-color: var(--color-accent);
  color: var(--color-accent-contrast);
}

.code-action-btn--primary:hover {
  background-color: var(--color-accent-hover);
  border-color: var(--color-accent-hover);
}

/* ---- Responsive ---- */

@media (max-width: 1100px) {
  .docs-page__body {
    grid-template-columns: 1fr;
  }

  .docs-page__toc {
    order: 2;
  }
}
</style>
