# AGENTS.md

This repository is a multi-language monorepo for ds4viz libraries and renderers.
Use the section that matches the subproject you are modifying.

## Repository map
- `library/python`: Python library (pytest, ruff, mypy)
- `library/typescript`: TypeScript library (tsc, eslint, jest, tsd)
- `library/c`: C header-only library (Makefile + test harness)
- `render/vue`: Vue 3 renderer (Vite, Vitest, ESLint, Prettier, oxlint)

## Cursor/Copilot rules
- No Cursor rules found in `.cursor/rules/` or `.cursorrules`.
- No Copilot rules found in `.github/copilot-instructions.md`.

## Build, lint, test commands

### Python library (`library/python`)
- Install dev deps: `python -m pip install -e .[dev]`
- Tests (all): `pytest -v`
- Single test file: `pytest -v test/test_example.py`
- Single test by name: `pytest -v -k "test_name"`
- Coverage: `pytest --cov`
- Lint (ruff): `ruff check .`
- Format (ruff): `ruff format .`
- Type check (mypy): `mypy ds4viz`

Notes:
- Pytest config lives in `library/python/pyproject.toml`.
- Python requires 3.12+ and strict mypy settings.

### TypeScript library (`library/typescript`)
- Install deps: `pnpm install` (or `npm install`)
- Build: `pnpm build` (runs `tsc`)
- Lint: `pnpm lint` (ESLint)
- Type check: `pnpm typecheck` (tsc --noEmit)
- Tests (all): `pnpm test`
- Single test file: `pnpm test -- test/unit.test.ts`
- Single test by name: `pnpm test -- -t "test name"`
- Coverage: `pnpm test:coverage`
- Type tests: `pnpm test:types` (tsd)

Notes:
- Jest runs via Node with `--experimental-vm-modules` per `package.json`.
- Node >= 18 required for this package.

### C library (`library/c`)
- Build + test (default): `make test`
- Build release: `make release`
- Clean: `make clean`
- Memcheck (valgrind): `make memcheck`
- Static analysis (clang): `make analyze`

Notes:
- Test harness is `test_ds4viz.c` and runs as a single binary.
- No built-in single-test selector; edit the test file or add a new target if needed.
- C17 required; warnings are treated as errors.

### Vue renderer (`render/vue`)
- Install deps: `pnpm install` (or `npm install`)
- Dev server: `pnpm dev`
- Build: `pnpm build`
- Preview: `pnpm preview`
- Unit tests: `pnpm test:unit`
- Single test by name: `pnpm test:unit -- -t "test name"`
- Single test file: `pnpm test:unit -- path/to/test.spec.ts`
- Lint (all): `pnpm lint`
- Lint (oxlint): `pnpm lint:oxlint`
- Lint (eslint): `pnpm lint:eslint`
- Format: `pnpm format`
- Type check: `pnpm type-check`

Notes:
- Node >= 20.19 or >= 22.12 for this package.
- `lint` and `format` run with `--fix`/`--write` in scripts.

## Code style guidelines

### General
- Prefer minimal, targeted changes; avoid unrelated reformatting.
- Match the existing file style when no formatter is configured.
- Keep public APIs stable; update docs when changing behavior.
- Preserve the IR contract: `.toml` output must remain compatible.

### Python (`library/python`)
- Formatting: ruff formatter, line length 120.
- Linting: ruff with isort enabled; organize imports by standard/third-party/first-party.
- Typing: mypy strict, no implicit Optional, no untyped defs.
- Naming: `snake_case` functions/variables, `PascalCase` classes, `UPPER_SNAKE_CASE` constants.
- Error handling: raise `RuntimeError` for invalid operations; errors are recorded to `.toml` on context exit.
- API constraints: value types are `int | float | str | bool`.
- Tests: place under `test/` and use `test_*.py` naming.

### TypeScript (`library/typescript`)
- Formatting: no explicit Prettier config here; follow existing file formatting.
- Linting: ESLint on `src` and `test` with `.ts` extensions.
- Typing: `strict: true`, no unused locals/params, explicit returns when needed.
- Imports: prefer explicit named imports; keep type-only imports using `import type`.
- Naming: `camelCase` functions/variables, `PascalCase` classes/types, `UPPER_SNAKE_CASE` constants.
- Error handling: use `StructureError`/`ValidationError` from library; ensure `.toml` error output still generated.
- API constraints: value types are `number | string | boolean`.

### C (`library/c`)
- Standard: C17, compile with `-Wall -Wextra -Wpedantic -Werror`.
- Formatting: keep brace style and indentation consistent with existing files.
- Naming: `dv_` prefix for public symbols; `snake_case` for functions and variables.
- Error handling: functions return `dv_error_t`; check and propagate errors consistently.
- Resource lifecycle: create -> operate -> commit -> destroy; always commit before destroy.

### Vue (`render/vue`)
- Formatting: Prettier with `semi: false`, `singleQuote: true`, `printWidth: 100`.
- Indentation: 2 spaces per `.editorconfig`.
- Linting: oxlint + ESLint; both run with `--fix` in scripts.
- Vue: prefer `<script setup>` and Composition API patterns when adding components.
- Types: keep `vue-tsc` type checks green; avoid `any` unless unavoidable.

## Tests to run for changes
- Python changes: `pytest -v`, `ruff check .`, `mypy ds4viz`.
- TypeScript changes: `pnpm lint`, `pnpm test`, `pnpm typecheck`.
- C changes: `make test` (and `make analyze` if touching core logic).
- Vue changes: `pnpm lint`, `pnpm test:unit`, `pnpm type-check`.

## Tips for agentic edits
- Keep changes scoped to the subproject you are working on.
- If you update public APIs, also update the corresponding README in that subproject.
- Avoid modifying generated files under `dist/` or `node_modules/`.
- Do not commit secrets or credentials.
