<script setup lang="ts">
/**
 * 文档页面
 *
 * @file src/views/Docs.vue
 * @author WaterRun
 * @date 2026-03-26
 * @component Docs
 */

import { computed, onMounted, ref } from 'vue'
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
        text: 'ds4viz 提供<b>在线服务</b>，这是使用 ds4viz 最方便的方式。当前页面即 ds4viz 网页端，使用 <code>Vue SPA</code> + <code>FastAPI</code> + <code>PostgreSQL</code> 技术栈，将代码编辑、远程执行、<code>TOML IR</code> 解析、可视化渲染整合到同一工作流中。在线服务需要登录才可访问。',
      },
    ],
    children: [
      {
        id: 'online-pages',
        title: '页面架构',
        content: [
          {
            text: '进入系统后，左侧为可折叠导航栏（图标模式 / 完全展开），包含以下页面:<br><b>编辑器</b> — 核心工作区，左右分栏的可视化 + 代码编辑器<br><b>模板库</b> — 卡片列表，支持分类筛选、搜索与收藏<br><b>文档</b> — 当前页面<br><b>个人主页</b> — 头像更新、密码修改、收藏模板与执行记录管理<br><b>关于</b> — 项目信息与许可协议',
          },
          {
            text: '所有登录后的页面共享左侧导航栏。导航栏底部显示当前用户头像，点击进入个人主页。',
          },
        ],
      },
      {
        id: 'online-editor',
        title: '编辑器',
        content: [
          { text: '编辑器页采用<b>左右分栏布局</b>。' },
          {
            text: '左侧为<b>可视化渲染器</b>，展示数据结构的当前状态快照。支持步进控制（首步 / 上一步 / 下一步 / 末步）和自动播放。画布支持鼠标滚轮缩放、拖拽平移，双击还原视图。',
          },
          {
            text: '右侧为基于 <code>Monaco</code> 的代码编辑区域。上方下拉框切换语言，内置 ds4viz API 的语法补全与基础高亮。点击「运行」按钮后，代码提交至后端执行，后端返回 <code>TOML IR</code>，前端解析并刷新可视化面板。',
          },
          {
            text: '支持下载当前 <code>TOML</code> 文件和上传本地 <code>TOML</code> 文件。上传本地 TOML 时，代码行高亮联动不可用（因无对应源码）。',
          },
          {
            text: '当可视化设置中的「<b>联动代码高亮</b>」开启时，步进切换会自动高亮编辑器中对应的源码行。',
          },
        ],
      },
      {
        id: 'online-renderer',
        title: '可视化渲染器',
        content: [
          {
            text: '渲染器按 <code>object.kind</code> 识别结构类型并绘制对应图形。渲染过程基于 <code>states</code> 完整快照，<code>steps</code> 用于展示操作名、参数、返回值和源码行号。',
          },
          {
            text: '可视化面板左下角的齿轮按钮展开运行时配置面板，包含以下开关:<br><b>显示元数据</b> — 展示 IR 的 <code>object.label</code>、<code>remarks</code>、<code>meta</code> 等信息<br><b>平滑动画衔接</b> — 帧间 PPT 风格平滑过渡<br><b>联动代码高亮</b> — 步进时高亮编辑器中对应源码行<br><b>自适应画布</b> — 自动调整缩放与平移以适应内容<br><b>变更强化</b> — 新增元素渐入动画 + 移除元素红色虚线渐出<br><b>控制台调试</b> — 在浏览器控制台输出 ds4viz 内部日志<br><b>跳过初始页</b> — 生成后直接进入可视化，不显示就绪提示<br><b>生成后自动播放</b> — 代码运行完成后自动开始播放<br><b>播放间隔</b> — 自动播放的帧间隔，范围 0–10000ms',
          },
          {
            text: '渲染器支持鼠标悬浮 Tooltip，显示元素的详细信息（值、索引、ID、角色等）。',
          },
          {
            text: '渲染器目前支持以下结构的可视化:<br><b>线性结构</b> — <code>stack</code>（圆柱堆叠）、<code>queue</code>（横向方块链）<br><b>链表</b> — <code>slist</code> / <code>dlist</code>（带箭头的节点链）<br><b>树</b> — <code>binary_tree</code> / <code>bst</code>（分层布局）<br><b>图</b> — <code>graph_undirected</code> / <code>graph_directed</code> / <code>graph_weighted</code>（圆形布局，箭头标识方向，权值标注于边上）',
          },
        ],
      },
      {
        id: 'online-templates',
        title: '模板系统',
        content: [
          {
            text: '模板库页面提供卡片式列表，支持按分类筛选和关键词搜索。每张卡片显示标题、描述、分类、收藏数和创建时间。',
          },
          {
            text: '点击卡片展开详情面板，包含只读代码编辑器、语言切换、描述文本和收藏按钮。点击「<b>在编辑器中打开</b>」将代码导入编辑器页面。编辑器已有代码时，会提示覆盖确认。',
          },
          {
            text: '收藏的模板可在个人主页中管理。',
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

onMounted(() => {
  const stored = localStorage.getItem('ds4viz_language') as Language | null
  if (stored && LANGUAGES.includes(stored)) {
    selectedLanguage.value = stored
  }
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
          <button class="toc-section__title" @click="toggleNode(section.id); scrollToSection(section.id)">
            <MaterialIcon :name="getToggleIcon(expanded[section.id])" :size="16" />
            <span>{{ section.title }}</span>
          </button>
          <div v-if="expanded[section.id]" class="toc-section__children">
            <button v-for="child in getAllChildren(section)" :key="child.id" class="toc-section__item"
              :class="{ 'toc-section__item--hidden': !!child.lang && child.lang !== selectedLanguage }"
              @click="scrollToSection(child.id)">
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
