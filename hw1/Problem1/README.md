# 问题 1：灰度直方图

在 `histogram.c` 里搜索 `TODO`，只填这三处：

1. `TODO(serial)`：串行直方图
2. `TODO(atomic)`：所有线程更新同一份 `hist`，计数器用 `atomic`
3. `TODO(private)`：每个线程一份局部直方图，处理完输入后再合并

参数、造数据、计时和对拍已经写好。计时只包住这三个函数，不包含造数据和对拍。

## 编译

```bash
make
make example
```

macOS 自带的 clang 没有 OpenMP。本目录的 Makefile 在检测到 Homebrew LLVM 时会改用它。Linux 上一般直接 `make` 即可。

调试时可以临时关掉优化：

```bash
make clean
make CFLAGS="-O0 -g -std=c11 -Wall -Wextra -fopenmp"
```

交作业和做性能实验时改回默认的 `-O2`。串行和并行在同一个程序里，优化等级是一样的。

## 运行

```bash
./histogram example
./histogram <serial|atomic|private|check> <N> <threads> <uniform|constant> [seed] [reps]
```

| 参数 | 含义 |
| --- | --- |
| `example` | 作业里的 8 个像素，以及长度为 1 的数组。默认 3 个线程，8 不能被 3 整除 |
| `serial` | 只计时串行版 |
| `atomic` | 计时共享直方图版本，并和串行结果逐项比较 |
| `private` | 计时线程私有版本，并和串行结果逐项比较 |
| `check` | 三个版本用同一份输入，输出中位时间和加速比 |
| `uniform` | 每个像素在 0..255 上均匀随机 |
| `constant` | 每个像素都是 128 |
| `seed` | 默认 1。同一种子得到同一份输入 |
| `reps` | 默认 3。`time_median_s` 是这几次的中位数 |

`example` 的时间没有参考价值，只看 `result=ok`。

性能实验至少做这四组，线程数自己再展开（建议包含 1、2、4 以及程序打印的 `omp_procs`）：

```bash
./histogram check 1000000 4 uniform 1 3
./histogram check 1000000 4 constant 1 3
./histogram check 10000000 4 uniform 1 3
./histogram check 10000000 4 constant 1 3
```

## 输出

最后一行是 `result=ok` 或 `result=fail`。

- `sum_ok`：256 个计数之和是否等于 `N`
- `match_atomic` / `match_private`：是否和串行结果逐项相同
- `speedup_atomic` / `speedup_private`：串行中位时间除以该并行版本的中位时间

加速比的分母是串行实现，不是并行版开 1 个线程的时间。

实验记录填进 `report.md`。
