/**
 * 编辑器模板管理
 *
 * @file src/utils/editor-templates.ts
 * @author WaterRun
 * @date 2026-02-12
 * @updated 2026-02-12
 */

import type { Language } from '@/types/api'

/**
 * 获取默认代码模板
 *
 * @param language - 语言标识
 * @returns 默认代码模板
 */
export function getDefaultTemplate(language: Language): string {
  const templates: Record<Language, string> = {
    python: `import ds4viz as dv

def main():
    with dv.stack(label="demo_stack") as s:
        s.push(10)
        s.push(20)
        s.push(30)
        s.pop()
        s.push(40)

if __name__ == "__main__":
    main()
`,
    lua: `local ds4viz = require("ds4viz")

ds4viz.withContext(ds4viz.stack("demo_stack"), function(s)
    s:push(10)
    s:push(20)
    s:push(30)
    s:pop()
    s:push(40)
end)
`,
    rust: `use ds4viz::prelude::*;

fn main() -> ds4viz::Result<()> {
    ds4viz::stack("demo_stack", |s| {
        s.push(10)?;
        s.push(20)?;
        s.push(30)?;
        s.pop()?;
        s.push(40)?;
        Ok(())
    })
}
`,
  }

  return templates[language]
}
