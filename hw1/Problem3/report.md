# Problem 3 Report: Mandelbrot Computation and OpenMP Scheduling

- Name: Shaoyan
- Student ID: 523031910224

## 1. Experimental setup

| Item | Record |
| --- | --- |
| CPU | Apple M2 Pro |
| Physical cores | 12 |
| Logical processors / `omp_procs` | 12 |
| Compiler | Homebrew clang++ 20.1.7 (`/opt/homebrew/opt/llvm/bin/clang++`) |
| Flags | `-O2 -std=c++17 -Wall -Wextra -fopenmp` |
| Timing | 3 runs per configuration; the median is reported. Speedup uses the serial implementation from the same `check` |

The M2 Pro has no hyper-threading. Those 12 processors are 8 performance cores and 4 efficiency cores. Thread counts are 1, 2, 4, 8, and 12. Twelve is included because it is `omp_procs` on this machine. Only `compute_serial`, `compute_static`, `compute_dynamic`, and `compute_guided` are timed. Each parallel version parallelizes the outer row loop and writes one row per iteration. The schedule clauses are `schedule(static)`, `schedule(dynamic, 1)`, and `schedule(guided, 1)`.

## 2. Correctness

`c = 0` returns `K`. `c = 1` with `K >= 3` returns 3. `match_*=1` means that schedule produced the same iteration image as the serial implementation.

| Check | Expected | Actual | Result |
| --- | --- | --- | --- |
| `c = 0`, K=100 | iteration count = 100 | `pixel_c0=100` | pass |
| `c = 1`, K=100 | iteration count = 3 | `pixel_c1=3` | pass |
| `./mandelbrot example check 3` (8×5, height not divisible by 3) | `match_*=1` | all three schedules match, iter_sum 699 | `result=ok` |
| Sum of per-thread pixel counts | equal to W × H | 40 on the example, 600 on the extra check, 262144 and 1048576 on the performance runs | pass |

Extra command: `./mandelbrot check 30 20 100 6 1`. Height 20 is not divisible by 6. Every schedule matched the serial image and the same total iteration count, 11490.

| Schedule | match | sum of pixels | sum of iter_sum |
| --- | --- | --- | --- |
| static | 1 | 600 | 11490 |
| dynamic | 1 | 600 | 11490 |
| guided | 1 | 600 | 11490 |

All performance runs below also have `match_static=1`, `match_dynamic=1`, and `match_guided=1`. The 512×512 image has `iter_sum_total=23158183`. The 1024×1024 image has `iter_sum_total=92760047`.

## 3. Performance and per-thread workload

`K = 500`. Times are `time_median_s` in seconds. Each speedup uses the serial median from that same run.

### Total time

| W | H | K | Threads | T_serial | T_static | T_dynamic | T_guided | S_static | S_dynamic | S_guided |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 512 | 512 | 500 | 1 | 0.088593 | 0.090850 | 0.070884 | 0.067523 | 0.9752 | 1.2498 | 1.3120 |
| 512 | 512 | 500 | 2 | 0.067388 | 0.057525 | 0.058848 | 0.034519 | 1.1715 | 1.1451 | 1.9522 |
| 512 | 512 | 500 | 4 | 0.067516 | 0.034342 | 0.018036 | 0.019468 | 1.9660 | 3.7434 | 3.4680 |
| 512 | 512 | 500 | 8 | 0.068670 | 0.023247 | 0.009163 | 0.009475 | 2.9539 | 7.4942 | 7.2475 |
| 512 | 512 | 500 | 12 | 0.068397 | 0.017648 | 0.007447 | 0.008656 | 3.8756 | 9.1845 | 7.9017 |
| 1024 | 1024 | 500 | 1 | 0.273107 | 0.274066 | 0.272901 | 0.278750 | 0.9965 | 1.0008 | 0.9798 |
| 1024 | 1024 | 500 | 2 | 0.292600 | 0.137661 | 0.136883 | 0.146885 | 2.1255 | 2.1376 | 1.9920 |
| 1024 | 1024 | 500 | 4 | 0.270303 | 0.132555 | 0.073020 | 0.077605 | 2.0392 | 3.7018 | 3.4831 |
| 1024 | 1024 | 500 | 8 | 0.271038 | 0.091887 | 0.039688 | 0.044654 | 2.9497 | 6.8292 | 6.0697 |
| 1024 | 1024 | 500 | 12 | 0.275650 | 0.073087 | 0.037323 | 0.039877 | 3.7715 | 7.3855 | 6.9125 |

### Per-thread workload

Configuration: W = 512, H = 512, K = 500, threads = 4. One run of `./mandelbrot check 512 512 500 4 1`.

**static**

| Thread | pixels | iter_sum |
| --- | --- | --- |
| 0 | 65536 | 425146 |
| 1 | 65536 | 11153933 |
| 2 | 65536 | 11153958 |
| 3 | 65536 | 425146 |

**dynamic, 1**

| Thread | pixels | iter_sum |
| --- | --- | --- |
| 0 | 101376 | 5836983 |
| 1 | 46592 | 5782486 |
| 2 | 22016 | 5687874 |
| 3 | 92160 | 5850840 |

**guided, 1**

| Thread | pixels | iter_sum |
| --- | --- | --- |
| 0 | 62976 | 6385724 |
| 1 | 58368 | 5439283 |
| 2 | 34304 | 5805552 |
| 3 | 106496 | 5527624 |

```text
static_thread=0 pixels=65536 iter_sum=425146
static_thread=1 pixels=65536 iter_sum=11153933
static_thread=2 pixels=65536 iter_sum=11153958
static_thread=3 pixels=65536 iter_sum=425146
dynamic_thread=0 pixels=101376 iter_sum=5836983
dynamic_thread=1 pixels=46592 iter_sum=5782486
dynamic_thread=2 pixels=22016 iter_sum=5687874
dynamic_thread=3 pixels=92160 iter_sum=5850840
guided_thread=0 pixels=62976 iter_sum=6385724
guided_thread=1 pixels=58368 iter_sum=5439283
guided_thread=2 pixels=34304 iter_sum=5805552
guided_thread=3 pixels=106496 iter_sum=5527624
```

## 4. Discussion

### 4.1 Why can threads that process the same number of rows perform different amounts of computation?

At 512×512 with 4 threads, `static` gives every thread 65536 pixels, but threads 1 and 2 do about 1.12×10^7 iterations while threads 0 and 3 do about 4.25×10^5. Points inside the set run all `K = 500` iterations and points outside escape quickly, and static hands the middle rows, where the set sits, to the same threads.

### 4.2 Which schedule is fastest in your experiments? Explain using the per-thread workloads and the execution times.

`dynamic, 1` is fastest from 4 threads up. At 512×512 and 4 threads it takes 0.018036 s, against 0.034342 s for static and 0.019468 s for guided. Static leaves two threads with about 26 times the iteration work of the other two. Dynamic hands out one row at a time, so the four `iter_sum` values stay within 5.69×10^6 to 5.85×10^6. Guided is close but its early chunks are larger, so the spread is wider.

### 4.3 Does a more flexible schedule always reduce execution time? Why or why not?

No. On 512×512 with 2 threads, guided takes 0.034519 s, while dynamic takes 0.058848 s and is slightly slower than static at 0.057525 s. Asking for a new row after every row costs more than it saves when only two threads are waiting. With one thread on 1024×1024, guided (0.278750 s) is slower than serial (0.273107 s), because there is no other thread to take the remaining rows.
