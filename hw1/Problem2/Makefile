# 串行和并行在同一个二进制里，优化选项保持一致。
# macOS 自带的 clang++ 不支持 OpenMP。安装了 Homebrew LLVM 时改用它。
ifeq ($(origin CXX), default)
  ifneq ($(wildcard /opt/homebrew/opt/llvm/bin/clang++),)
    CXX := /opt/homebrew/opt/llvm/bin/clang++
  endif
endif

CXXFLAGS ?= -O2 -std=c++17 -Wall -Wextra -fopenmp
LDFLAGS ?= -fopenmp

# 只测一种实现时，默认跑 N=3/4、T=1/2 这四个已知例子。
# 换规模：make parallel N=64 T=20 THREADS=4 REPS=1
# 没有已知答案的规模，parallel 会和串行版对拍。
THREADS ?= 3
REPS ?= 1

.PHONY: all clean help example init serial parallel check

all: heat

heat: heat.cpp
	$(CXX) $(CXXFLAGS) -o $@ heat.cpp $(LDFLAGS)

help:
	@printf '%s\n' \
	  'make init                   只检查初始边界' \
	  'make serial                 只测串行迭代，对照四个已知小例子' \
	  'make parallel               只测并行迭代，不要求串行版已写完' \
	  'make check                  串行和并行对拍，并给出加速比' \
	  'make example                与 make check 相同' \
	  'make serial THREADS=4       小例子改用 4 个线程' \
	  'make serial N=64 T=20 THREADS=4 REPS=1'

example: check

ifdef N
T ?= 1
init: heat
	./heat init $(N)
serial: heat
	./heat serial $(N) $(T) $(THREADS) $(REPS)
parallel: heat
	./heat parallel $(N) $(T) $(THREADS) $(REPS)
check: heat
	./heat check $(N) $(T) $(THREADS) $(REPS)
else
init: heat
	./heat example init
serial: heat
	./heat example serial $(THREADS)
parallel: heat
	./heat example parallel $(THREADS)
check: heat
	./heat example check $(THREADS)
endif

clean:
	rm -f heat
