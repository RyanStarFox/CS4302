# 串行和并行在同一个二进制里，优化选项保持一致。
# macOS 自带 clang 不支持 OpenMP。安装了 Homebrew LLVM 时改用它。
ifeq ($(origin CC), default)
  ifneq ($(wildcard /opt/homebrew/opt/llvm/bin/clang),)
    CC := /opt/homebrew/opt/llvm/bin/clang
  endif
endif

CFLAGS ?= -O2 -std=c11 -Wall -Wextra -fopenmp
LDFLAGS ?= -fopenmp -lm

.PHONY: all clean example

all: histogram

histogram: histogram.c
	$(CC) $(CFLAGS) -o $@ histogram.c $(LDFLAGS)

example: histogram
	./histogram example

clean:
	rm -f histogram
