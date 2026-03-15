# ds4viz C 库测试构建
#
# 用法:
#   make          编译并运行测试
#   make clean    清理构建产物和测试输出

CC      = gcc
CFLAGS  = -std=c2x -Wall -Wextra -Wpedantic
LDFLAGS = -lm

.PHONY: all clean

all: test
	@./test

test: test.c ds4viz.h
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

clean:
	rm -f test t_*.toml