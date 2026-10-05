# 串行和并行在同一个二进制里，优化选项保持一致。
# macOS 自带的 clang++ 不支持 OpenMP。安装了 Homebrew LLVM 时改用它。
ifeq ($(origin CXX), default)
  ifneq ($(wildcard /opt/homebrew/opt/llvm/bin/clang++),)
    CXX := /opt/homebrew/opt/llvm/bin/clang++
  endif
endif

CXXFLAGS ?= -O2 -std=c++17 -Wall -Wextra -fopenmp
LDFLAGS ?= -fopenmp

# 只测一种实现时，默认跑作业里的 8 个像素和长度为 1 的数组。
# 换规模：make atomic N=100000 THREADS=4 DIST=uniform SEED=1 REPS=1
THREADS ?= 3
REPS ?= 1
DIST ?= uniform
SEED ?= 1

.PHONY: all clean help example serial atomic private check

all: histogram

histogram: histogram.cpp
	$(CXX) $(CXXFLAGS) -o $@ histogram.cpp $(LDFLAGS)

help:
	@printf '%s\n' \
	  'make serial                 只测串行，用作业小例子' \
	  'make atomic                 只测共享直方图，不要求串行版已写完' \
	  'make private                只测线程私有直方图，不要求串行版已写完' \
	  'make check                  三种一起对拍，并给出加速比' \
	  'make example                与 make check 相同' \
	  'make serial THREADS=4       小例子改用 4 个线程' \
	  'make atomic N=100000 THREADS=4 DIST=uniform REPS=1' \
	  '    指定 N 时，atomic/private 会和串行版对拍，所以串行版要先正确'

example: check

ifdef N
serial: histogram
	./histogram serial $(N) $(THREADS) $(DIST) $(SEED) $(REPS)
atomic: histogram
	./histogram atomic $(N) $(THREADS) $(DIST) $(SEED) $(REPS)
private: histogram
	./histogram private $(N) $(THREADS) $(DIST) $(SEED) $(REPS)
check: histogram
	./histogram check $(N) $(THREADS) $(DIST) $(SEED) $(REPS)
else
serial: histogram
	./histogram example serial $(THREADS)
atomic: histogram
	./histogram example atomic $(THREADS)
private: histogram
	./histogram example private $(THREADS)
check: histogram
	./histogram example check $(THREADS)
endif

clean:
	rm -f histogram
