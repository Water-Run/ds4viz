# ds4viz C Library Makefile
#
# @author WaterRun
# @date 2025-12-24

# 编译器和标志
CC = gcc
CFLAGS = -std=c17 -Wall -Wextra -Wpedantic -Werror
CFLAGS_DEBUG = $(CFLAGS) -g -O0 -DDEBUG
CFLAGS_RELEASE = $(CFLAGS) -O2 -DNDEBUG

# 目标文件
TEST_TARGET = test_ds4viz
TEST_SRC = test_ds4viz.c

# 默认目标
.PHONY: all
all: test

# 测试目标
.PHONY: test
test: $(TEST_TARGET)
	@echo "Running tests..."
	@./$(TEST_TARGET)
	@echo "Cleaning up test files..."
	@rm -f *.toml

# 编译测试程序
$(TEST_TARGET): $(TEST_SRC) ds4viz.h
	$(CC) $(CFLAGS_DEBUG) -o $@ $(TEST_SRC)

# 发布版本编译
.PHONY: release
release: $(TEST_SRC) ds4viz.h
	$(CC) $(CFLAGS_RELEASE) -o $(TEST_TARGET) $(TEST_SRC)

# 清理
.PHONY: clean
clean:
	rm -f $(TEST_TARGET)
	rm -f *.toml
	rm -f *.o

# 检查内存泄漏（需要安装valgrind）
.PHONY: memcheck
memcheck: $(TEST_TARGET)
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(TEST_TARGET)

# 静态分析（需要安装clang）
.PHONY: analyze
analyze:
	clang --analyze $(TEST_SRC)

# 格式检查
.PHONY: format-check
format-check:
	@echo "Format check not implemented - use clang-format manually"

# 帮助信息
.PHONY: help
help:
	@echo "Available targets:"
	@echo "  all       - Build and run tests (default)"
	@echo "  test      - Build and run tests"
	@echo "  release   - Build with optimization"
	@echo "  clean     - Remove build artifacts"
	@echo "  memcheck  - Run with valgrind"
	@echo "  analyze   - Run static analysis"
	@echo "  help      - Show this help"
	