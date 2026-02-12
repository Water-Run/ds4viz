/**
 * Monaco 编辑器运行时配置
 *
 * @file src/utils/monaco.ts
 * @author WaterRun
 * @date 2026-02-12
 * @updated 2026-02-12
 */

/**
 * 初始化 Monaco web worker (Vite)
 */
export function setupMonacoWorkers(): void {
  const globalScope = self as unknown as { MonacoEnvironment?: { getWorker: () => Worker } }
  globalScope.MonacoEnvironment = {
    getWorker: () =>
      new Worker(
        new URL('monaco-editor/esm/vs/editor/editor.worker?worker', import.meta.url),
        { type: 'module' },
      ),
  }
}
