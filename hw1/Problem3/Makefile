# 串行和并行在同一个二进制里，优化选项保持一致。
# macOS 自带的 clang++ 不支持 OpenMP。安装了 Homebrew LLVM 时改用它。
ifeq ($(origin CXX), default)
  ifneq ($(wildcard /opt/homebrew/opt/llvm/bin/clang++),)
    CXX := /opt/homebrew/opt/llvm/bin/clang++
  endif
endif

CXXFLAGS ?= -O2 -std=c++17 -Wall -Wextra -fopenmp
LDFLAGS ?= -fopenmp

# 默认用 8x5 的小图，高度不能被 3 个线程整除。
# 换规模：make static W=30 H=20 K=100 THREADS=6 REPS=1
# static/dynamic/guided 会和串行结果比较，所以要先让 make serial 通过。
THREADS ?= 3
REPS ?= 1

.PHONY: all clean help example pixel serial static dynamic guided check

all: mandelbrot

mandelbrot: mandelbrot.cpp
	$(CXX) $(CXXFLAGS) -o $@ mandelbrot.cpp $(LDFLAGS)

help:
	@printf '%s\n' \
	  'make pixel                  只检查 c=0 和 c=1' \
	  'make serial                 只测串行整图' \
	  'make static                 只测 schedule(static)' \
	  'make dynamic                只测 schedule(dynamic, 1)' \
	  'make guided                 只测 schedule(guided, 1)' \
	  'make check                  三种调度都和串行版对拍' \
	  'make example                与 make check 相同' \
	  'make static THREADS=5       小图改用 5 个线程' \
	  'make guided W=30 H=20 K=100 THREADS=6 REPS=1'

example: check

ifdef W
H ?= 20
K ?= 100
pixel: mandelbrot
	./mandelbrot example pixel
serial: mandelbrot
	./mandelbrot serial $(W) $(H) $(K) $(THREADS) $(REPS)
static: mandelbrot
	./mandelbrot static $(W) $(H) $(K) $(THREADS) $(REPS)
dynamic: mandelbrot
	./mandelbrot dynamic $(W) $(H) $(K) $(THREADS) $(REPS)
guided: mandelbrot
	./mandelbrot guided $(W) $(H) $(K) $(THREADS) $(REPS)
check: mandelbrot
	./mandelbrot check $(W) $(H) $(K) $(THREADS) $(REPS)
else
pixel: mandelbrot
	./mandelbrot example pixel
serial: mandelbrot
	./mandelbrot example serial $(THREADS)
static: mandelbrot
	./mandelbrot example static $(THREADS)
dynamic: mandelbrot
	./mandelbrot example dynamic $(THREADS)
guided: mandelbrot
	./mandelbrot example guided $(THREADS)
check: mandelbrot
	./mandelbrot example check $(THREADS)
endif

clean:
	rm -f mandelbrot
