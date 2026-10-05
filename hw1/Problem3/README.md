# 问题 3：Mandelbrot 与 OpenMP 调度

在 `mandelbrot.cpp` 里搜索 `TODO`，只填这五处：

1. `TODO(pixel)`：单个点的迭代次数
2. `TODO(serial)`：按行计算整张图，统计写入 `stats[0]`
3. `TODO(static)`：行循环使用 `schedule(static)`
4. `TODO(dynamic)`：行循环使用 `schedule(dynamic, 1)`
5. `TODO(guided)`：行循环使用 `schedule(guided, 1)`

三种调度都要写在源码的 schedule 子句里。并行单位是一整行。

像素 `(i, j)` 的坐标：

```text
x = -2 + 3.0 * j / (double)(W - 1)
y = -1.5 + 3.0 * i / (double)(H - 1)
```

线程统计在本地累加，结束再写入 `stats[线程号]`。不要给每个像素加一把全局锁。`iter_sum` 用 `long`。

程序启动时会检查两个点：`c = 0` 的迭代次数是 `K`；`K >= 3` 时 `c = 1` 的迭代次数是 3。这两个点不对，后面的大图不会跑。

## 编译

```bash
make
make help
make pixel
make serial
make static
make dynamic
make guided
make check
```

`make pixel` 只检查 `c = 0` 和 `c = 1`。`make serial` 只跑串行整图。三种调度各自一个目标，都会和串行结果逐像素比较，所以要先让 `make serial` 通过。`make check` 和 `make example` 会把三种调度一起测。

默认是 8×5 的图和 3 个线程，高度不能被线程数整除。它会打印每个线程的 `pixels` 和 `iter_sum`。某个线程的 `pixels=0` 可以出现，只要像素数之和等于 `W * H`，并且 `match_*=1`。

换规模：

```bash
make static THREADS=5
make guided W=30 H=20 K=100 THREADS=6 REPS=1
```

## 运行

```bash
./mandelbrot example [pixel|serial|static|dynamic|guided|check] [threads]
./mandelbrot <serial|static|dynamic|guided|check> <W> <H> <K> <threads> [reps]
```

| 参数 | 含义 |
| --- | --- |
| `check` | 三种调度都与串行结果逐像素比较，并输出工作量和加速比 |
| `reps` | 默认 3，输出中位数 |

正确性还要另测一张高度不能被线程数整除的图，`example` 已经覆盖一种。换线程数可以再测一次：

```bash
./mandelbrot check 30 20 100 6 1
```

性能实验：

```bash
./mandelbrot check 512 512 500 4
./mandelbrot check 1024 1024 500 4
```

`1024×1024、K=500` 会比较慢。先让 `example` 通过，再跑大图。线程数建议包含 1、2、4 以及 `omp_procs`。

加速比的分母是串行实现。每个线程的像素数和迭代次数之和都要抄进 `report.md`。
