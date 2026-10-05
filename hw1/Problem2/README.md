# 问题 2：二维平板温度迭代

在 `heat.cpp` 里搜索 `TODO`，只填这三处：

1. `TODO(init)`：上边界中间是 100，四个角和其他边界是 0，内部初值是 0
2. `TODO(serial)`：恰好 `T` 轮。读一块缓冲，把内部点写入另一块，然后交换角色
3. `TODO(parallel)`：并行每一轮的内部点，用 reduction 求这一轮的最大绝对变化

网格按行优先存储，第 0 行是上边界。内部点下标从 1 到 `N - 2`。

`final_grid` 必须指向 `buf_a.data()` 或 `buf_b.data()`。返回值是**最后一轮**的最大变化，不是所有轮次里最大的那个。

计时只包住 `iterate_serial` / `iterate_parallel`。初始化在计时外面。

## 编译

```bash
make
make example
```

`make example` 会跑 `N=3/4`、`T=1/2`，并对照下面这张 `N=4, T=1` 的网格。最后一轮变化是 25。

```text
0 100 100 0
0  25  25 0
0   0   0 0
0   0   0 0
```

`N=3, T=2` 的最后一轮变化是 0：只有一个内部点，第二轮它的邻居仍然全是边界。这个用例用来确认你返回的是最后一轮的 delta。

## 运行

```bash
./heat example
./heat <serial|parallel|check> <N> <T> <threads> [reps]
```

| 参数 | 含义 |
| --- | --- |
| `serial` | 只计时串行版。小网格会打印温度 |
| `parallel` | 计时并行版，并给出与串行网格的最大绝对差 |
| `check` | 两边都计时，输出加速比和最大绝对差 |
| `reps` | 默认 3，输出中位数 |

性能实验：

```bash
./heat check 128 1000 4
./heat check 256 1000 4
./heat check 512 1000 4
```

线程数建议再测 1、2、4 以及程序打印的 `omp_procs`。`N=512, T=1000` 会跑一会儿。

要求 `max_abs_diff` 不超过 `1e-8`，输出里的 `diff_ok=1` 表示满足。加速比的分母是串行实现。

实验记录填进 `report.md`。
