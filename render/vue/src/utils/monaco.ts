/**
 * Monaco 编辑器运行时配置
 *
 * @file src/utils/monaco.ts
 * @author WaterRun
 * @date 2026-02-12
 * @updated 2026-02-24
 */

/**
 * 初始化 Monaco web worker (Vite)
 *
 * 为 Monaco 编辑器注册 getWorker 工厂函数。
 * 当前仅提供通用编辑器 Worker；JSON / TypeScript 等语言
 * 不在业务范围内，统一回退至 editor.worker。
 */
export function setupMonacoWorkers(): void {
  const globalScope = self as unknown as {
    MonacoEnvironment?: {
      getWorker: (_moduleId: string, _label: string) => Worker
    }
  }
  globalScope.MonacoEnvironment = {
    getWorker: (_moduleId: string, _label: string): Worker =>
      new Worker(
        new URL('monaco-editor/esm/vs/editor/editor.worker?worker', import.meta.url),
        { type: 'module' },
      ),
  }
}