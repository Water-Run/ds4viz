/**
 * LLM 系统 Prompt 构建
 *
 * 从本地 library 目录读取 ds4viz README 文档，动态构建系统 Prompt。
 *
 * @file src/utils/llm-prompts.ts
 * @author WaterRun
 * @date 2026-03-27
 */

import type { Language } from '@/types/api'
import pythonDocs from '../../../../library/python/README.md?raw'
import cDocs from '../../../../library/c/README.md?raw'

/** 各语言对应的本地文档内容 */
const DOCS: Record<Language, string> = {
    python: pythonDocs,
    c: cDocs,
}

/**
 * 构建 LLM 系统 Prompt
 *
 * @param language - 当前编辑器语言
 * @returns 系统提示文本
 */
export function buildSystemPrompt(language: Language): string {
    const langName = language === 'python' ? 'Python' : 'C'
    const docs = DOCS[language]

    const docsSection =
        docs.length > 0
            ? `以下是 ds4viz 库的完整 README 文档:\n\n---\n${docs}\n---`
            : '(文档加载失败，请根据 ds4viz 常规 API 模式生成代码)'

    return `你是 ds4viz 代码生成助手。ds4viz 是一个面向教学的基础数据结构可视化框架。用户编写代码调用 ds4viz API，程序运行后生成 TOML IR 文件用于可视化回放。

当前语言: ${langName}

${docsSection}

输出规则:
1. 仅输出可直接运行的 ${langName} 代码，不包含任何解释、注释说明或 Markdown 格式标记
2. 代码必须使用 ds4viz API 创建数据结构并执行操作
3. 合理使用 step()、amend()、phase()、alias() 和 highlights 使可视化效果清晰、步骤分明
4. 代码应当完整、可直接执行，包含必要的 import/include
5. 不要输出代码块标记（如 \`\`\`python 或 \`\`\`c），只输出纯代码
6. 若用户请求与 ds4viz 无关的内容，仍然生成使用 ds4viz 的演示代码`
}
