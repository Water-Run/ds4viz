# Agent Guide for ds4viz

## Project Overview

**ds4viz** is a multi-language educational platform for data structure visualization. It's a polyglot monorepo with:
- **library/**: Client libraries in 12+ languages (TypeScript, Python, Rust, PHP, Lua, C, JavaScript, etc.)
- **render/**: Visualization renderers (Vue.js SPA, standalone web renderer, planned: CLI, WinUI3, Flutter)
- **server/**: Backend execution services (Rust production server, Lua demo server)
- **prompt/**: Documentation and specifications
- **assets/**: Project images and assets

**Architecture**: Three-layer decoupled design: `Code (any language) → TOML IR → Multi-platform Renderer`

## Build, Lint, and Test Commands

### TypeScript Library (`library/typescript/`)

```bash
# Build
npm run build              # Compile TypeScript to dist/
npm run clean             # Remove dist/ folder
npm run typecheck         # Type check without emitting files

# Lint & Format
npm run lint              # Run ESLint on src/ and test/

# Test
npm test                  # Run all tests with Jest
npm run test:unit         # Run unit tests only
npm run test:integration  # Run integration tests only
npm run test:types        # Run type definition tests with tsd
npm run test:coverage     # Run tests with coverage report
npm run test:watch        # Run tests in watch mode

# Run a single test file
node --experimental-vm-modules node_modules/jest/bin/jest.js test/unit.test.ts
```

### Python Library (`library/python/`)

```bash
# Install dev dependencies
pip install -e ".[dev]"

# Test
pytest                    # Run all tests in test/
pytest test/stack_test.py # Run a single test file
pytest -v                 # Verbose output
pytest --cov              # Run with coverage

# Lint & Type Check
mypy ds4viz               # Type check with strict mode
ruff check ds4viz         # Lint with ruff
ruff format ds4viz        # Format with ruff
```

### Vue Renderer (`render/vue/`)

```bash
# Development
npm run dev               # Start Vite dev server

# Build
npm run build             # Type check + production build
npm run build-only        # Build without type checking
npm run preview           # Preview production build

# Type Check
npm run type-check        # Run vue-tsc

# Lint & Format
npm run lint              # ESLint with auto-fix and cache
npm run format            # Prettier format src/

# Test
npm run test:unit         # Run Vitest tests
```

### PHP Library (`library/php/`)

```bash
# Install dependencies
composer install

# Test
composer test             # Run PHPUnit tests
composer phpstan          # Run PHPStan static analysis (level 9)
composer phpcs            # Check PSR12 code style
composer check            # Run all checks (phpcs + phpstan + test)

# Run a single test file
vendor/bin/phpunit test/StackTest.php
```

### Rust Library (`library/rust/`)

```bash
# Build
cargo build               # Debug build
cargo build --release     # Release build with optimizations

# Test
cargo test                # Run all tests
cargo test stack          # Run tests matching "stack"
cargo test -- --nocapture # Show println! output

# Lint
cargo clippy              # Run clippy lints (all, pedantic, nursery)
cargo fmt                 # Format code
cargo check               # Fast compile check
```

## Code Style Guidelines

### TypeScript

**Imports**
- Use ESM imports with `.js` extensions: `import { foo } from './module.js'`
- Group imports: external packages first, then internal modules
- Use `type` imports for type-only: `import type { Foo } from './types.js'`

**Formatting**
- Semicolons: Required (standard TypeScript convention)
- Quotes: Single quotes preferred in Vue projects, double quotes in TS library
- Indentation: 2 spaces (Vue), 4 spaces (TS library)
- Line length: 100 characters (Vue Prettier), no hard limit (TS library)

**Types**
- Always use explicit types for function parameters and return values
- Enable all strict TypeScript flags (see tsconfig.json)
- Use `unknown` instead of `any` in catch clauses
- Use exact optional property types where possible
- Mark index signatures as potentially undefined

**Naming Conventions**
- Variables/Functions: `camelCase`
- Classes/Interfaces/Types: `PascalCase`
- Constants: `UPPER_SNAKE_CASE` for true constants
- Private class members: prefix with `_` (e.g., `_internalState`)
- Factory functions: lowercase (e.g., `stack()`, `queue()`)

**Error Handling**
- Use custom error classes extending `Error`
- Document error types in JSDoc with `@throws`
- Validate inputs and throw descriptive errors
- Use `unknown` in catch clauses, narrow type before use

**Documentation**
- JSDoc for all public APIs
- Include `@param`, `@returns`, `@throws` tags
- Code is documentation: prefer clear names over comments

### Python

**Imports**
- Standard library first, third-party second, local modules last
- Use absolute imports: `from ds4viz.structures import Stack`
- Type imports: `from typing import TYPE_CHECKING` for circular dependencies

**Formatting**
- Follow PEP 8, enforced by ruff
- Line length: 120 characters
- Indentation: 4 spaces
- Quotes: Double quotes preferred (ruff default)

**Types**
- Type hint ALL function signatures (enforced by mypy strict mode)
- Use modern syntax: `str | None` instead of `Optional[str]`
- Use `typing` module for complex types
- Raw strings for docstrings: `r"""`

**Naming Conventions**
- Variables/Functions: `snake_case`
- Classes: `PascalCase`
- Constants: `UPPER_SNAKE_CASE`
- Private members: prefix with `_` (e.g., `_internal_state`)

**Error Handling**
- Raise specific exceptions: `ValueError`, `TypeError`, etc.
- Document exceptions in docstrings
- Use type narrowing to avoid `None` errors

**Documentation**
- Docstrings for all public APIs (Google or reStructuredText style)
- Include `:param`, `:return`, `:raise` sections
- Use `__all__` to define public API

### Rust

**Formatting**
- Use `cargo fmt` (rustfmt) - always format before commit
- Indentation: 4 spaces
- Line length: 100 characters (rustfmt default)

**Error Handling**
- Use `Result<T, E>` for recoverable errors
- Use `thiserror` for custom error types
- Document errors in doc comments
- Prefer `?` operator for error propagation

**Naming Conventions**
- Variables/Functions: `snake_case`
- Types/Traits: `PascalCase`
- Constants: `UPPER_SNAKE_CASE`
- Lifetimes: short lowercase: `'a`, `'b`

**Documentation**
- Doc comments (`///`) for all public items
- Include examples in doc tests
- Document panics, errors, and safety invariants

**Lints**
- Enable all clippy lints (all, pedantic, nursery)
- Warn on missing docs
- Use `#[must_use]` for types that should not be ignored

### Vue/Frontend

**Component Style**
- Use `<script setup lang="ts">` for composition API
- Props: define with `defineProps<T>()` using TypeScript
- Emits: define with `defineEmits<T>()`
- Single-file components: `<script>`, `<template>`, `<style scoped>`

**Formatting (Prettier)**
- No semicolons
- Single quotes
- Print width: 100 characters
- Run `npm run format` before commit

**State Management**
- Use Pinia for global state
- Composables for reusable logic
- Keep component state local when possible

## General Practices

### Testing
- Write tests for all new features and bug fixes
- Test file naming: `*_test.py` (Python), `*.test.ts` (TS), `*Test.php` (PHP), `tests/*.rs` (Rust)
- Aim for high coverage but prioritize meaningful tests
- Use descriptive test names that explain what is being tested

### Git Workflow
- Write clear, concise commit messages (1-2 sentences focusing on "why")
- Follow existing commit style in `git log`
- No force push to main/master
- Never commit secrets (.env, credentials, etc.)

### Error Messages
- Write clear, actionable error messages
- Include context: what failed and why
- Suggest fixes when possible

### Comments
- Code should be self-documenting through clear names
- Comment "why" not "what"
- Update comments when code changes
- Avoid redundant comments

### File Organization
- Group related functionality together
- Keep files focused and cohesive
- Use directory structure to indicate architecture
- Follow existing patterns in the codebase

---

**Last Updated**: 2025-01-15
**Project Repository**: https://github.com/Water-Run/ds4viz
