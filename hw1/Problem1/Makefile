# 串行和并行在同一个二进制里，优化选项保持一致。
# macOS 自带的 clang++ 不支持 OpenMP。安装了 Homebrew LLVM 时改用它。
ifeq ($(origin CXX), default)
  ifneq ($(wildcard /opt/homebrew/opt/llvm/bin/clang++),)
    CXX := /opt/homebrew/opt/llvm/bin/clang++
  endif
endif

CXXFLAGS ?= -O2 -std=c++17 -Wall -Wextra -fopenmp
LDFLAGS ?= -fopenmp

.PHONY: all clean example

all: histogram

histogram: histogram.cpp
	$(CXX) $(CXXFLAGS) -o $@ histogram.cpp $(LDFLAGS)

example: histogram
	./histogram example

clean:
	rm -f histogram
