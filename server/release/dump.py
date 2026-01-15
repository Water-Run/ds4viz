#!/usr/bin/env python3
import sys
from pathlib import Path

def detect_lang(path: Path) -> str:
    ext = path.suffix.lower()
    return {
        ".rs": "rust",
        ".toml": "toml",
    }.get(ext, "")

def gather_files(repo_root: Path):
    files = []
    cargo = repo_root / "Cargo.toml"
    if cargo.is_file():
        files.append(cargo)
    src_dir = repo_root / "src"
    tests_dir = repo_root / "tests"
    # 收集 src 下的 .rs
    if src_dir.is_dir():
        files += sorted(src_dir.rglob("*.rs"))
    # 收集 tests 下的 .rs
    if tests_dir.is_dir():
        files += sorted(tests_dir.rglob("*.rs"))
    return files

def main():
    repo_root = Path.cwd()
    files = gather_files(repo_root)
    if not files:
        print("No files found (Cargo.toml, src/*.rs, tests/*.rs).", file=sys.stderr)
        sys.exit(1)

    for path in files:
        rel = path.relative_to(repo_root)
        lang = detect_lang(path)
        print(f"{rel}:")
        print(f"```{lang}")
        try:
            content = path.read_text(encoding="utf-8")
        except Exception as e:
            print(f"[读取文件出错: {e}]")
        else:
            print(content.rstrip("\n"))
        print("```")
        print()  # 空行分隔

if __name__ == "__main__":
    main()
