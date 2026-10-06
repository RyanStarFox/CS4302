# Problem 2 Report: Temperature Iteration on a Two-Dimensional Plate

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

Only the temperature iteration is timed. Grid initialization is excluded. Thread counts are 1, 2, 4, 8, and 12. Twelve is included because it is `omp_procs` on this machine, the maximum thread count OpenMP reports. The M2 Pro has no hyper-threading. Those 12 processors are 8 performance cores and 4 efficiency cores.

## 2. Correctness

`./heat example` uses 3 threads. `diff_ok=1` means the maximum absolute difference between the parallel and serial grids is at most `1e-8`. Every case below returned `result=ok`.

| Case | Final-iteration delta | `diff_ok` | Result |
| --- | --- | --- | --- |
| N=3, T=1 | 25, matches the expected 25 | 1 | `result=ok` |
| N=3, T=2 | 0, matches the expected 0 | 1 | `result=ok` |
| N=4, T=1 | 25, matches the expected 25 | 1 | `result=ok` |
| N=4, T=2 | 6.25, matches the expected 6.25 | 1 | `result=ok` |

The `N=3, T=2` case checks that the returned delta is the change in the last iteration. The only interior point stays at 25 on the second iteration, so that change is 0 rather than the first-iteration change of 25.

Final grid for `N=4, T=1`, identical for the serial and parallel runs:

```text
0 100 100 0
0  25  25 0
0   0   0 0
0   0   0 0
```

In Section 3, the final-iteration delta is the same at every thread count for a given `N`. `max_abs_diff` stays between `4.263e-14` and `5.684e-14`.

## 3. Performance

`T = 1000`. The required sizes are `N` = 128, 256, and 512. `N` = 1024 and 2048 are an extra check that a larger grid changes which thread count is fastest. Times are `time_median_s` in seconds. The speedup is the printed `speedup`, with the serial median from that run as the denominator.

| N | T | Threads | T_serial | T_parallel | Speedup | Final-iteration delta | max_abs_diff |
| --- | --- | --- | --- | --- | --- | --- | --- |
| 128 | 1000 | 1 | 0.018829 | 0.019453 | 0.9679 | 0.0239739 | 4.263e-14 |
| 128 | 1000 | 2 | 0.019225 | 0.019245 | 0.9990 | 0.0239739 | 4.263e-14 |
| 128 | 1000 | 4 | 0.018727 | 0.019000 | 0.9856 | 0.0239739 | 4.263e-14 |
| 128 | 1000 | 8 | 0.018713 | 0.063427 | 0.2950 | 0.0239739 | 4.263e-14 |
| 128 | 1000 | 12 | 0.019025 | 0.091038 | 0.2090 | 0.0239739 | 4.263e-14 |
| 256 | 1000 | 1 | 0.077207 | 0.077446 | 0.9969 | 0.0241926 | 4.263e-14 |
| 256 | 1000 | 2 | 0.077193 | 0.052475 | 1.4710 | 0.0241926 | 4.263e-14 |
| 256 | 1000 | 4 | 0.077259 | 0.037033 | 2.0862 | 0.0241926 | 4.263e-14 |
| 256 | 1000 | 8 | 0.076002 | 0.073464 | 1.0345 | 0.0241926 | 4.263e-14 |
| 256 | 1000 | 12 | 0.077172 | 0.097316 | 0.7930 | 0.0241926 | 4.263e-14 |
| 512 | 1000 | 1 | 0.315165 | 0.324711 | 0.9706 | 0.0241926 | 5.684e-14 |
| 512 | 1000 | 2 | 0.314232 | 0.180276 | 1.7431 | 0.0241926 | 5.684e-14 |
| 512 | 1000 | 4 | 0.315582 | 0.102678 | 3.0735 | 0.0241926 | 5.684e-14 |
| 512 | 1000 | 8 | 0.312389 | 0.112045 | 2.7881 | 0.0241926 | 5.684e-14 |
| 512 | 1000 | 12 | 0.311570 | 0.139596 | 2.2319 | 0.0241926 | 5.684e-14 |
| 1024 | 1000 | 1 | 1.263183 | 1.373372 | 0.9198 | 0.0241926 | 5.684e-14 |
| 1024 | 1000 | 2 | 1.259219 | 0.715421 | 1.7601 | 0.0241926 | 5.684e-14 |
| 1024 | 1000 | 4 | 1.264849 | 0.385748 | 3.2790 | 0.0241926 | 5.684e-14 |
| 1024 | 1000 | 8 | 1.263867 | 0.263694 | 4.7929 | 0.0241926 | 5.684e-14 |
| 1024 | 1000 | 12 | 1.263764 | 0.305364 | 4.1385 | 0.0241926 | 5.684e-14 |
| 2048 | 1000 | 1 | 5.105755 | 6.024341 | 0.8475 | 0.0241926 | 5.684e-14 |
| 2048 | 1000 | 2 | 5.160936 | 3.190811 | 1.6174 | 0.0241926 | 5.684e-14 |
| 2048 | 1000 | 4 | 5.165002 | 1.693770 | 3.0494 | 0.0241926 | 5.684e-14 |
| 2048 | 1000 | 8 | 5.163773 | 1.068226 | 4.8340 | 0.0241926 | 5.684e-14 |
| 2048 | 1000 | 12 | 5.182489 | 1.109612 | 4.6705 | 0.0241926 | 5.684e-14 |

Raw output:

```text
./heat check 128 1000 1 3
serial_times_s=0.018756 0.018829 0.019082
serial_time_median_s=0.018829
parallel_times_s=0.019575 0.019340 0.019453
parallel_time_median_s=0.019453
serial_final_delta=0.0239739 parallel_final_delta=0.0239739
max_abs_diff=4.263e-14 diff_ok=1 speedup=0.9679 result=ok

./heat check 128 1000 2 3
serial_times_s=0.019129 0.019299 0.019225
serial_time_median_s=0.019225
parallel_times_s=0.019245 0.019152 0.019296
parallel_time_median_s=0.019245
serial_final_delta=0.0239739 parallel_final_delta=0.0239739
max_abs_diff=4.263e-14 diff_ok=1 speedup=0.9990 result=ok

./heat check 128 1000 4 3
serial_times_s=0.018897 0.018727 0.018657
serial_time_median_s=0.018727
parallel_times_s=0.020064 0.018971 0.019000
parallel_time_median_s=0.019000
serial_final_delta=0.0239739 parallel_final_delta=0.0239739
max_abs_diff=4.263e-14 diff_ok=1 speedup=0.9856 result=ok

./heat check 128 1000 8 3
serial_times_s=0.018833 0.018647 0.018713
serial_time_median_s=0.018713
parallel_times_s=0.065466 0.063427 0.063124
parallel_time_median_s=0.063427
serial_final_delta=0.0239739 parallel_final_delta=0.0239739
max_abs_diff=4.263e-14 diff_ok=1 speedup=0.2950 result=ok

./heat check 128 1000 12 3
serial_times_s=0.019296 0.019025 0.018875
serial_time_median_s=0.019025
parallel_times_s=0.091054 0.090192 0.091038
parallel_time_median_s=0.091038
serial_final_delta=0.0239739 parallel_final_delta=0.0239739
max_abs_diff=4.263e-14 diff_ok=1 speedup=0.2090 result=ok

./heat check 256 1000 1 3
serial_times_s=0.076955 0.077462 0.077207
serial_time_median_s=0.077207
parallel_times_s=0.077242 0.077447 0.077446
parallel_time_median_s=0.077446
serial_final_delta=0.0241926 parallel_final_delta=0.0241926
max_abs_diff=4.263e-14 diff_ok=1 speedup=0.9969 result=ok

./heat check 256 1000 2 3
serial_times_s=0.077193 0.077264 0.077188
serial_time_median_s=0.077193
parallel_times_s=0.053388 0.052475 0.052455
parallel_time_median_s=0.052475
serial_final_delta=0.0241926 parallel_final_delta=0.0241926
max_abs_diff=4.263e-14 diff_ok=1 speedup=1.4710 result=ok

./heat check 256 1000 4 3
serial_times_s=0.077259 0.102370 0.076814
serial_time_median_s=0.077259
parallel_times_s=0.038646 0.037033 0.036722
parallel_time_median_s=0.037033
serial_final_delta=0.0241926 parallel_final_delta=0.0241926
max_abs_diff=4.263e-14 diff_ok=1 speedup=2.0862 result=ok

./heat check 256 1000 8 3
serial_times_s=0.076002 0.075086 0.076179
serial_time_median_s=0.076002
parallel_times_s=0.073848 0.073464 0.072693
parallel_time_median_s=0.073464
serial_final_delta=0.0241926 parallel_final_delta=0.0241926
max_abs_diff=4.263e-14 diff_ok=1 speedup=1.0345 result=ok

./heat check 256 1000 12 3
serial_times_s=0.077172 0.077296 0.076976
serial_time_median_s=0.077172
parallel_times_s=0.099335 0.097316 0.096424
parallel_time_median_s=0.097316
serial_final_delta=0.0241926 parallel_final_delta=0.0241926
max_abs_diff=4.263e-14 diff_ok=1 speedup=0.7930 result=ok

./heat check 512 1000 1 3
serial_times_s=0.309389 0.315165 0.318380
serial_time_median_s=0.315165
parallel_times_s=0.325679 0.323533 0.324711
parallel_time_median_s=0.324711
serial_final_delta=0.0241926 parallel_final_delta=0.0241926
max_abs_diff=5.684e-14 diff_ok=1 speedup=0.9706 result=ok

./heat check 512 1000 2 3
serial_times_s=0.312470 0.314232 0.315223
serial_time_median_s=0.314232
parallel_times_s=0.180583 0.180276 0.179781
parallel_time_median_s=0.180276
serial_final_delta=0.0241926 parallel_final_delta=0.0241926
max_abs_diff=5.684e-14 diff_ok=1 speedup=1.7431 result=ok

./heat check 512 1000 4 3
serial_times_s=0.315582 0.320536 0.311998
serial_time_median_s=0.315582
parallel_times_s=0.102678 0.102945 0.102391
parallel_time_median_s=0.102678
serial_final_delta=0.0241926 parallel_final_delta=0.0241926
max_abs_diff=5.684e-14 diff_ok=1 speedup=3.0735 result=ok

./heat check 512 1000 8 3
serial_times_s=0.312407 0.309044 0.312389
serial_time_median_s=0.312389
parallel_times_s=0.114152 0.109912 0.112045
parallel_time_median_s=0.112045
serial_final_delta=0.0241926 parallel_final_delta=0.0241926
max_abs_diff=5.684e-14 diff_ok=1 speedup=2.7881 result=ok

./heat check 512 1000 12 3
serial_times_s=0.308179 0.311570 0.312206
serial_time_median_s=0.311570
parallel_times_s=0.142008 0.139596 0.139580
parallel_time_median_s=0.139596
serial_final_delta=0.0241926 parallel_final_delta=0.0241926
max_abs_diff=5.684e-14 diff_ok=1 speedup=2.2319 result=ok
```

## 4. Discussion

### 4.1 Why can different iterations not be executed directly in parallel?

Each interior value is the average of its four neighbors from the previous iteration, so iteration `t + 1` cannot start until iteration `t` has been written. Running both at once would mix old and new neighbors, and the result would depend on the schedule.

### 4.2 Why might updating the temperature grid in place change the result?

An in-place update lets a later point read a neighbor that already belongs to the current iteration. The Jacobi step needs all four neighbors from the previous iteration, so the code reads `current`, writes `next`, and swaps only after the whole interior is done.

### 4.3 Which operations require synchronization at the end of each iteration?

`parallel for` ends with an implicit barrier, so every interior write is visible before the buffers are swapped. `reduction(max:delta)` combines each thread's private maximum at that same barrier, so an extra `barrier` would only wait twice.

### 4.4 How does parallel overhead affect execution time when the grid is small?

Each of the 1000 iterations creates a new team, so one parallel thread is already slightly slower than serial (0.324711 s versus 0.315165 s at `N=512`). `N=128` never beats serial; at 12 threads the speedup is only 0.209.

On `N=256` and `N=512` the best count is 4 threads (S=2.09 and S=3.07), because one iteration is too small for 8 threads. Once the grid is large enough, the best count is 8: `N=1024` gives S=4.79 and `N=2048` gives S=4.83. This machine has 8 performance cores and 4 efficiency cores. Twelve threads also run on the efficiency cores, and the barrier at the end of every iteration waits for the slowest of them, so 12 is slower than 8 on both large grids (S=4.14 and S=4.67).
