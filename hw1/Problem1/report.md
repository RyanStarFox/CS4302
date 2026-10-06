# Problem 1 Report: Parallel Grayscale Histogram

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

Within one `check`, all three implementations share the same input. The `uniform` seed is fixed at 1. Thread counts are 1, 2, 4, 8, and 12. Twelve is included because it is `omp_procs` on this machine, the maximum thread count OpenMP reports. The M2 Pro has no hyper-threading, so the physical and logical counts are both 12. Those 12 processors are 8 performance cores and 4 efficiency cores.

## 2. Correctness

| Case | Command | Result | Notes |
| --- | --- | --- | --- |
| Assignment sample, length 8, not divisible by the thread count | `./histogram example` | `result=ok` | 3 threads, `N_mod_threads=2`. `example_serial_match=1`, `match_atomic=1`, `match_private=1`, `sum_ok=1` |
| Length 1 | Second block of the same run | `result=ok` | Pixel value 42. All three sums are 1 and match the serial histogram |
| `sum(hist) = N` | Table below, and all 20 runs in Section 3 | all `sum_ok=1` | The 256 counters sum to `N` |

Random input whose length is not divisible by the thread count:

```bash
./histogram check 1000 3 uniform 1 1
```

| Command | `sum_ok` | `match_atomic` | `match_private` |
| --- | --- | --- | --- |
| `./histogram check 1000 3 uniform 1 1` | 1 | 1 | 1 |

`N_mod_threads=1`. In Section 3, `N=1000000` with 12 threads has `N_mod_threads=4`. All 20 runs also have `match_atomic=1` and `match_private=1`.

## 3. Performance

`N` is \(10^6\) and \(10^7\), with both `uniform` and `constant` inputs. Times are `time_median_s` in seconds. Speedups are the printed `speedup_atomic` and `speedup_private`. The denominator is the serial median from that run, not the one-thread parallel time. The serial loop is timed again for every thread count, so \(T_{\mathrm{serial}}\) for the same `N` varies slightly.

### Uniform distribution

| N | Threads | T_serial | T_atomic | T_private | S_atomic | S_private |
| --- | --- | --- | --- | --- | --- | --- |
| 1000000 | 1 | 0.000410 | 0.001209 | 0.000396 | 0.3392 | 1.0355 |
| 1000000 | 2 | 0.000394 | 0.020858 | 0.000218 | 0.0189 | 1.8055 |
| 1000000 | 4 | 0.000309 | 0.016959 | 0.000124 | 0.0182 | 2.4923 |
| 1000000 | 8 | 0.000322 | 0.014620 | 0.000172 | 0.0220 | 1.8724 |
| 1000000 | 12 | 0.000313 | 0.012831 | 0.000217 | 0.0244 | 1.4429 |
| 10000000 | 1 | 0.003156 | 0.008829 | 0.003207 | 0.3574 | 0.9841 |
| 10000000 | 2 | 0.003186 | 0.118114 | 0.001686 | 0.0270 | 1.8896 |
| 10000000 | 4 | 0.003195 | 0.145915 | 0.000871 | 0.0219 | 3.6685 |
| 10000000 | 8 | 0.003119 | 0.138027 | 0.000640 | 0.0226 | 4.8741 |
| 10000000 | 12 | 0.003183 | 0.128709 | 0.000710 | 0.0247 | 4.4832 |

### Concentrated distribution `constant` (every pixel is 128)

| N | Threads | T_serial | T_atomic | T_private | S_atomic | S_private |
| --- | --- | --- | --- | --- | --- | --- |
| 1000000 | 1 | 0.001931 | 0.001948 | 0.001995 | 0.9912 | 0.9679 |
| 1000000 | 2 | 0.001945 | 0.009744 | 0.001058 | 0.1996 | 1.8382 |
| 1000000 | 4 | 0.001946 | 0.014719 | 0.000583 | 0.1322 | 3.3382 |
| 1000000 | 8 | 0.002037 | 0.023714 | 0.000369 | 0.0859 | 5.5194 |
| 1000000 | 12 | 0.001940 | 0.051603 | 0.000501 | 0.0376 | 3.8729 |
| 10000000 | 1 | 0.019985 | 0.019699 | 0.020174 | 1.0145 | 0.9906 |
| 10000000 | 2 | 0.020241 | 0.087802 | 0.010272 | 0.2305 | 1.9705 |
| 10000000 | 4 | 0.019687 | 0.168196 | 0.005253 | 0.1170 | 3.7477 |
| 10000000 | 8 | 0.019940 | 0.266252 | 0.002820 | 0.0749 | 7.0709 |
| 10000000 | 12 | 0.020023 | 0.545175 | 0.003540 | 0.0367 | 5.6561 |

Raw output:

```text
./histogram check 1000000 1 uniform 1 3
serial_times_s=0.000412 0.000410 0.000409
serial_time_median_s=0.000410
atomic_times_s=0.001209 0.001215 0.001202
atomic_time_median_s=0.001209
private_times_s=0.000408 0.000396 0.000390
private_time_median_s=0.000396
sum_ok=1 match_atomic=1 match_private=1
speedup_atomic=0.3392 speedup_private=1.0355 result=ok

./histogram check 1000000 2 uniform 1 3
serial_times_s=0.000367 0.000403 0.000394
serial_time_median_s=0.000394
atomic_times_s=0.018384 0.020858 0.024001
atomic_time_median_s=0.020858
private_times_s=0.000218 0.000226 0.000186
private_time_median_s=0.000218
sum_ok=1 match_atomic=1 match_private=1
speedup_atomic=0.0189 speedup_private=1.8055 result=ok

./histogram check 1000000 4 uniform 1 3
serial_times_s=0.000318 0.000309 0.000300
serial_time_median_s=0.000309
atomic_times_s=0.017981 0.016959 0.015654
atomic_time_median_s=0.016959
private_times_s=0.000142 0.000124 0.000118
private_time_median_s=0.000124
sum_ok=1 match_atomic=1 match_private=1
speedup_atomic=0.0182 speedup_private=2.4923 result=ok

./histogram check 1000000 8 uniform 1 3
serial_times_s=0.000330 0.000321 0.000322
serial_time_median_s=0.000322
atomic_times_s=0.015743 0.014620 0.013992
atomic_time_median_s=0.014620
private_times_s=0.000185 0.000149 0.000172
private_time_median_s=0.000172
sum_ok=1 match_atomic=1 match_private=1
speedup_atomic=0.0220 speedup_private=1.8724 result=ok

./histogram check 1000000 12 uniform 1 3
serial_times_s=0.000313 0.000310 0.000316
serial_time_median_s=0.000313
atomic_times_s=0.013645 0.012550 0.012831
atomic_time_median_s=0.012831
private_times_s=0.000244 0.000217 0.000216
private_time_median_s=0.000217
sum_ok=1 match_atomic=1 match_private=1
speedup_atomic=0.0244 speedup_private=1.4429 result=ok

./histogram check 10000000 1 uniform 1 3
serial_times_s=0.003173 0.003141 0.003156
serial_time_median_s=0.003156
atomic_times_s=0.008855 0.008829 0.008760
atomic_time_median_s=0.008829
private_times_s=0.003081 0.003238 0.003207
private_time_median_s=0.003207
sum_ok=1 match_atomic=1 match_private=1
speedup_atomic=0.3574 speedup_private=0.9841 result=ok

./histogram check 10000000 2 uniform 1 3
serial_times_s=0.003186 0.003234 0.003023
serial_time_median_s=0.003186
atomic_times_s=0.128473 0.117600 0.118114
atomic_time_median_s=0.118114
private_times_s=0.001728 0.001686 0.001655
private_time_median_s=0.001686
sum_ok=1 match_atomic=1 match_private=1
speedup_atomic=0.0270 speedup_private=1.8896 result=ok

./histogram check 10000000 4 uniform 1 3
serial_times_s=0.003213 0.003195 0.003111
serial_time_median_s=0.003195
atomic_times_s=0.145915 0.119450 0.173764
atomic_time_median_s=0.145915
private_times_s=0.000871 0.000877 0.000867
private_time_median_s=0.000871
sum_ok=1 match_atomic=1 match_private=1
speedup_atomic=0.0219 speedup_private=3.6685 result=ok

./histogram check 10000000 8 uniform 1 3
serial_times_s=0.003210 0.003058 0.003119
serial_time_median_s=0.003119
atomic_times_s=0.138027 0.136338 0.140207
atomic_time_median_s=0.138027
private_times_s=0.000640 0.000604 0.000805
private_time_median_s=0.000640
sum_ok=1 match_atomic=1 match_private=1
speedup_atomic=0.0226 speedup_private=4.8741 result=ok

./histogram check 10000000 12 uniform 1 3
serial_times_s=0.003215 0.003109 0.003183
serial_time_median_s=0.003183
atomic_times_s=0.130828 0.128709 0.127177
atomic_time_median_s=0.128709
private_times_s=0.000712 0.000696 0.000710
private_time_median_s=0.000710
sum_ok=1 match_atomic=1 match_private=1
speedup_atomic=0.0247 speedup_private=4.4832 result=ok

./histogram check 1000000 1 constant 1 3
serial_times_s=0.001916 0.001931 0.001945
serial_time_median_s=0.001931
atomic_times_s=0.001920 0.001999 0.001948
atomic_time_median_s=0.001948
private_times_s=0.001995 0.001975 0.002001
private_time_median_s=0.001995
sum_ok=1 match_atomic=1 match_private=1
speedup_atomic=0.9912 speedup_private=0.9679 result=ok

./histogram check 1000000 2 constant 1 3
serial_times_s=0.001945 0.001923 0.001945
serial_time_median_s=0.001945
atomic_times_s=0.010084 0.009744 0.007572
atomic_time_median_s=0.009744
private_times_s=0.001058 0.001067 0.001045
private_time_median_s=0.001058
sum_ok=1 match_atomic=1 match_private=1
speedup_atomic=0.1996 speedup_private=1.8382 result=ok

./histogram check 1000000 4 constant 1 3
serial_times_s=0.001919 0.001946 0.001955
serial_time_median_s=0.001946
atomic_times_s=0.017463 0.014719 0.013376
atomic_time_median_s=0.014719
private_times_s=0.000583 0.000586 0.000579
private_time_median_s=0.000583
sum_ok=1 match_atomic=1 match_private=1
speedup_atomic=0.1322 speedup_private=3.3382 result=ok

./histogram check 1000000 8 constant 1 3
serial_times_s=0.002037 0.002096 0.001951
serial_time_median_s=0.002037
atomic_times_s=0.023714 0.025470 0.023521
atomic_time_median_s=0.023714
private_times_s=0.000384 0.000366 0.000369
private_time_median_s=0.000369
sum_ok=1 match_atomic=1 match_private=1
speedup_atomic=0.0859 speedup_private=5.5194 result=ok

./histogram check 1000000 12 constant 1 3
serial_times_s=0.001941 0.001929 0.001940
serial_time_median_s=0.001940
atomic_times_s=0.052970 0.051603 0.050794
atomic_time_median_s=0.051603
private_times_s=0.000513 0.000501 0.000489
private_time_median_s=0.000501
sum_ok=1 match_atomic=1 match_private=1
speedup_atomic=0.0376 speedup_private=3.8729 result=ok

./histogram check 10000000 1 constant 1 3
serial_times_s=0.019985 0.020101 0.019681
serial_time_median_s=0.019985
atomic_times_s=0.019699 0.019879 0.019596
atomic_time_median_s=0.019699
private_times_s=0.020218 0.020174 0.019982
private_time_median_s=0.020174
sum_ok=1 match_atomic=1 match_private=1
speedup_atomic=1.0145 speedup_private=0.9906 result=ok

./histogram check 10000000 2 constant 1 3
serial_times_s=0.020241 0.020234 0.020569
serial_time_median_s=0.020241
atomic_times_s=0.087802 0.093997 0.083640
atomic_time_median_s=0.087802
private_times_s=0.010373 0.010272 0.010203
private_time_median_s=0.010272
sum_ok=1 match_atomic=1 match_private=1
speedup_atomic=0.2305 speedup_private=1.9705 result=ok

./histogram check 10000000 4 constant 1 3
serial_times_s=0.019687 0.019593 0.020001
serial_time_median_s=0.019687
atomic_times_s=0.163452 0.168196 0.170660
atomic_time_median_s=0.168196
private_times_s=0.005253 0.005157 0.005265
private_time_median_s=0.005253
sum_ok=1 match_atomic=1 match_private=1
speedup_atomic=0.1170 speedup_private=3.7477 result=ok

./histogram check 10000000 8 constant 1 3
serial_times_s=0.019909 0.020037 0.019940
serial_time_median_s=0.019940
atomic_times_s=0.266252 0.353446 0.244662
atomic_time_median_s=0.266252
private_times_s=0.002886 0.002723 0.002820
private_time_median_s=0.002820
sum_ok=1 match_atomic=1 match_private=1
speedup_atomic=0.0749 speedup_private=7.0709 result=ok

./histogram check 10000000 12 constant 1 3
serial_times_s=0.019696 0.020386 0.020023
serial_time_median_s=0.020023
atomic_times_s=0.545175 0.545864 0.537600
atomic_time_median_s=0.545175
private_times_s=0.003540 0.003543 0.003534
private_time_median_s=0.003540
sum_ok=1 match_atomic=1 match_private=1
speedup_atomic=0.0367 speedup_private=5.6561 result=ok
```

## 4. Discussion

### 4.1 Why can adding only a parallel-for directive to the serial loop produce incorrect results?

When multiple threads execute `hist[pixels[i]]++` concurrently, they may all read the same original value and each write back that value plus one, so only a single increment is effectively preserved rather than N increments. As a result, the final histogram is incorrect.

### 4.2 How do the two parallel versions behave under the two input distributions, and where do the differences come from?

For \(N=10^7\), serial takes about 0.0032 s on the uniform input and 0.020 s on the concentrated input, because every concentrated update hits the same counter. Atomic medians:

| Threads | uniform | constant |
| --- | --- | --- |
| 1 | 0.008829 s, S=0.3574 | 0.019699 s, S=1.0145 |
| 2 | 0.118114 s, S=0.0270 | 0.087802 s, S=0.2305 |
| 4 | 0.145915 s, S=0.0219 | 0.168196 s, S=0.1170 |
| 8 | 0.138027 s, S=0.0226 | 0.266252 s, S=0.0749 |
| 12 | 0.128709 s, S=0.0247 | 0.545175 s, S=0.0367 |

At 12 threads every atomic update of the concentrated input hits `hist[128]`, so the time grows from 0.020 s to 0.545 s. Uniform conflicts are spread over 256 bins, but 0.129 s is still about 40 times slower than the serial scan. Private medians, where each thread writes its own `local` array:

| Threads | uniform | constant |
| --- | --- | --- |
| 1 | 0.003207 s, S=0.9841 | 0.020174 s, S=0.9906 |
| 2 | 0.001686 s, S=1.8896 | 0.010272 s, S=1.9705 |
| 4 | 0.000871 s, S=3.6685 | 0.005253 s, S=3.7477 |
| 8 | 0.000640 s, S=4.8741 | 0.002820 s, S=7.0709 |
| 12 | 0.000710 s, S=4.4832 | 0.003540 s, S=5.6561 |

Private beats serial from 2 threads on. For \(N=10^7\) the best count is 8, not 12. This machine has 8 performance cores and 4 efficiency cores. Twelve threads also run on the efficiency cores, and the barrier waits for the slowest of them, so 12 threads are slower than 8: uniform 0.000710 s versus 0.000640 s, constant 0.003540 s versus 0.002820 s. On the smaller uniform input \(N=10^6\), thread creation and the merge dominate, and 4 threads (0.000124 s) beat both 8 and 12.

### 4.3 What extra work does the thread-private histogram introduce?

Relative to the serial loop, the private version does the following inside the timed region.

1. Allocate `threads` local histograms and zero the global `hist`.
2. Each thread creates and zeros another 256-bin `local` array on the stack. The thread id is read once.
3. The pixel loop writes only `local[pixels[i]]`, not the global `hist`, so the scan has no atomic operations.
4. After the scan, `local` is copied to `locals[tid]`, and the per-thread counters are added into `hist` bin by bin. The merge is \(256 \times p\) additions, plus a copy of the same size.

Next to \(10^7\) pixels this extra work is small, so one thread nearly matches serial (0.003207 s versus 0.003156 s on the uniform input). On the uniform \(N=10^6\) input it is not: 12 threads take 0.000217 s and 4 threads take 0.000124 s.
