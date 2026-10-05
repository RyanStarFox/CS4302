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

all: mandelbrot

mandelbrot: mandelbrot.c
	$(CC) $(CFLAGS) -o $@ mandelbrot.c $(LDFLAGS)

example: mandelbrot
	./mandelbrot example

clean:
	rm -f mandelbrot
