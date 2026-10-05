/*
 * 问题 2：二维平板温度迭代
 *
 * 只需要填写下面三处，搜索 TODO：
 *   TODO(init)      边界和初值
 *   TODO(serial)    串行迭代
 *   TODO(parallel)  每一轮的内部点并行，delta 用 reduction
 *
 * 网格按行优先存放：下标 (i, j) 是 grid[(size_t)i * n + j]。
 * 第 0 行是上边界。内部点的 i、j 都从 1 到 n - 2。
 * 计时只包住 iterate_*。初始化在计时外面。
 *
 * 用法见 README.md。先运行 make example。
 */

#include <errno.h>
#include <math.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
    MAX_THREADS = 512,
    MAX_REPS = 100
};

static const double GRID_TOL = 1e-8;

static void die(const char *msg) {
    fprintf(stderr, "error: %s\n", msg);
    exit(1);
}

static void usage(void) {
    fprintf(stderr,
            "用法:\n"
            "  ./heat example [threads]\n"
            "  ./heat <serial|parallel|check> <N> <T> <threads> [reps]\n"
            "\n"
            "  N       网格边长，N >= 3\n"
            "  T       迭代次数，T >= 1，必须刚好做 T 轮\n"
            "  threads 并行线程数；串行版会忽略它，但命令行仍要写\n"
            "  reps    默认 3，输出中位数\n"
            "  example 检查 N=3/4、T=1/2，并对照作业里的 4x4 一例\n");
    exit(1);
}

static long parse_long(const char *s, const char *name, long minv, long maxv) {
    char *end = NULL;
    errno = 0;
    long v = strtol(s, &end, 10);
    if (errno != 0 || end == s || *end != '\0' || v < minv || v > maxv) {
        fprintf(stderr, "参数 %s 无效: %s（允许 %ld..%ld）\n", name, s, minv, maxv);
        exit(1);
    }
    return v;
}

static int cmp_double(const void *a, const void *b) {
    double da = *(const double *)a;
    double db = *(const double *)b;
    return (da > db) - (da < db);
}

static double median(double *v, int n) {
    qsort(v, (size_t)n, sizeof(double), cmp_double);
    if (n % 2 == 1) {
        return v[n / 2];
    }
    return 0.5 * (v[n / 2 - 1] + v[n / 2]);
}

static double *new_grid(int n) {
    double *g = calloc((size_t)n * (size_t)n, sizeof(double));
    if (g == NULL) {
        die("内存不足");
    }
    return g;
}

static void print_grid(const char *title, const double *grid, int n) {
    printf("%s\n", title);
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            printf(" %8.3f", grid[(size_t)i * (size_t)n + (size_t)j]);
        }
        printf("\n");
    }
}

static double grid_max_abs_diff(const double *a, const double *b, int n) {
    double m = 0.0;
    size_t cells = (size_t)n * (size_t)n;
    for (size_t k = 0; k < cells; k++) {
        double d = fabs(a[k] - b[k]);
        if (d > m) {
            m = d;
        }
    }
    return m;
}

/* =====================================================================
 * 下面三个函数是你要交的计算。
 *
 * iterate_* 的约定：
 *   buf_a 和 buf_b 都已经用 init_plate 初始化。
 *   做完 steps 轮之后，把 *final_grid 指到存放最终温度的那一块。
 *   它必须是 buf_a 或 buf_b，不要指向函数里的局部数组。
 *   返回值是最后一轮的最大绝对温度变化，不是所有轮次里最大的那个。
 * ===================================================================== */

static void init_plate(double *grid, int n) {
    (void)grid;
    (void)n;
    /* TODO(init)
     * 上边界（第 0 行）温度为 100，但这一行的两个角是 0。
     * 其余边界，包括四个角，都是 0。
     * 内部点初始也是 0。
     * 每次调用 init_plate 之前，主程序都会把整块缓冲清零。
     * 你至少要把温度应为 100 的那些边界写上。内部点保持 0 即可。
     */
}

static double iterate_serial(double *buf_a, double *buf_b, int n, int steps,
                             double **final_grid) {
    (void)n;
    (void)buf_b;
    (void)steps;
    /* TODO(serial)
     * 刚好迭代 steps 次，不要因为看起来收敛了就提前停。
     * 内部点的新温度 = 上一轮上下左右四个邻居的平均值。
     * 边界不要改。
     * 读一块缓冲、写另一块，一轮结束后交换两块的角色。
     * 不要原地更新，否则邻居会混进本轮已经写过的值。
     * 返回最后一轮的 delta。*final_grid 指向最终结果。
     *
     * 交换角色可以写成：
     *   double *current = buf_a;
     *   double *next = buf_b;
     *   ... 计算 ...
     *   double *tmp = current; current = next; next = tmp;
     *   *final_grid = current;
     */
    *final_grid = buf_a;
    return 0.0;
}

static double iterate_parallel(double *buf_a, double *buf_b, int n, int steps,
                               int threads, double **final_grid) {
    (void)n;
    (void)buf_b;
    (void)steps;
    (void)threads;
    /* TODO(parallel)
     * 数值规则和串行版相同。
     * 并行的是每一轮里面的内部点，不是不同的迭代轮次。
     * 下一轮开始之前，本轮的内部点必须都已经写完。
     * 用 OpenMP reduction 计算这一轮的最大绝对温度变化。
     * 线程数用 threads。
     * 返回最后一轮的 delta，并设置 *final_grid。
     * 哪一次同步是必须的，写进报告第 3 题，不要只在代码里留一个说不清的 barrier。
     */
    *final_grid = buf_a;
    return 0.0;
}

typedef struct {
    int n;
    int steps;
    const double *grid;
    double delta;
    const char *name;
} KnownCase;

static int approx(double a, double b) {
    return fabs(a - b) <= GRID_TOL;
}

static int check_known(const KnownCase *k, const double *grid, double delta) {
    size_t cells = (size_t)k->n * (size_t)k->n;
    int grid_ok = 1;
    for (size_t i = 0; i < cells; i++) {
        if (!approx(grid[i], k->grid[i])) {
            grid_ok = 0;
            break;
        }
    }
    int delta_ok = approx(delta, k->delta);
    printf("known_%s_grid=%s\n", k->name, grid_ok ? "ok" : "fail");
    printf("known_%s_delta=%s expect=%.10g got=%.10g\n",
           k->name, delta_ok ? "ok" : "fail", k->delta, delta);
    if (!grid_ok) {
        print_grid("期望网格:", k->grid, k->n);
        print_grid("实际网格:", grid, k->n);
    }
    return grid_ok && delta_ok;
}

/*
 * N=3, T=2：只有中心一个内部点。第二轮它的四个邻居仍是边界，
 * 温度保持 25，这一轮的变化是 0。
 * 用来区分「最后一轮的 delta」和「所有轮次里最大的 delta」。
 */
static const double k_n3_t1[] = {
    0, 100, 0,
    0, 25, 0,
    0, 0, 0
};
static const double k_n3_t2[] = {
    0, 100, 0,
    0, 25, 0,
    0, 0, 0
};
static const double k_n4_t1[] = {
    0, 100, 100, 0,
    0, 25, 25, 0,
    0, 0, 0, 0,
    0, 0, 0, 0
};
static const double k_n4_t2[] = {
    0, 100, 100, 0,
    0, 31.25, 31.25, 0,
    0, 6.25, 6.25, 0,
    0, 0, 0, 0
};

static const KnownCase *lookup_known(int n, int steps) {
    static const KnownCase cases[] = {
        {3, 1, k_n3_t1, 25.0, "n3_t1"},
        {3, 2, k_n3_t2, 0.0, "n3_t2"},
        {4, 1, k_n4_t1, 25.0, "n4_t1"},
        {4, 2, k_n4_t2, 6.25, "n4_t2"},
    };
    for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
        if (cases[i].n == n && cases[i].steps == steps) {
            return &cases[i];
        }
    }
    return NULL;
}

static void reset_plate(double *a, double *b, int n) {
    size_t bytes = (size_t)n * (size_t)n * sizeof(double);
    memset(a, 0, bytes);
    memset(b, 0, bytes);
    init_plate(a, n);
    init_plate(b, n);
}

static void time_serial(double *a, double *b, int n, int steps, int reps,
                        double *samples, double *median_s, double **final_grid,
                        double *delta) {
    *delta = 0.0;
    *final_grid = NULL;
    for (int r = 0; r < reps; r++) {
        reset_plate(a, b, n);
        double t0 = omp_get_wtime();
        *delta = iterate_serial(a, b, n, steps, final_grid);
        samples[r] = omp_get_wtime() - t0;
    }
    double sorted[MAX_REPS];
    memcpy(sorted, samples, (size_t)reps * sizeof(double));
    *median_s = median(sorted, reps);
}

static void time_parallel(double *a, double *b, int n, int steps, int threads, int reps,
                          double *samples, double *median_s, double **final_grid,
                          double *delta) {
    *delta = 0.0;
    *final_grid = NULL;
    for (int r = 0; r < reps; r++) {
        reset_plate(a, b, n);
        double t0 = omp_get_wtime();
        *delta = iterate_parallel(a, b, n, steps, threads, final_grid);
        samples[r] = omp_get_wtime() - t0;
    }
    double sorted[MAX_REPS];
    memcpy(sorted, samples, (size_t)reps * sizeof(double));
    *median_s = median(sorted, reps);
}

static void print_times(const char *name, const double *samples, int reps, double median_s) {
    printf("%s_times_s=", name);
    for (int r = 0; r < reps; r++) {
        printf("%s%.6f", r ? " " : "", samples[r]);
    }
    printf("\n%s_time_median_s=%.6f\n", name, median_s);
}

static int run_case(const char *mode, int n, int steps, int threads, int reps, int show_grid) {
    int want_serial_time = strcmp(mode, "serial") == 0 || strcmp(mode, "check") == 0;
    int want_parallel_time = strcmp(mode, "parallel") == 0 || strcmp(mode, "check") == 0;
    int want_diff = strcmp(mode, "parallel") == 0 || strcmp(mode, "check") == 0;

    printf("problem=2\n");
    printf("mode=%s\n", mode);
    printf("N=%d\n", n);
    printf("T=%d\n", steps);
    printf("threads=%d\n", threads);
    printf("reps=%d\n", reps);
    printf("omp_procs=%d\n", omp_get_num_procs());

    omp_set_dynamic(0);
    omp_set_num_threads(threads);

    double *sa = new_grid(n);
    double *sb = new_grid(n);
    double *pa = new_grid(n);
    double *pb = new_grid(n);

    double serial_samples[MAX_REPS];
    double parallel_samples[MAX_REPS];
    double serial_median = 0.0;
    double parallel_median = 0.0;
    double *serial_final = NULL;
    double *parallel_final = NULL;
    double serial_delta = 0.0;
    double parallel_delta = 0.0;

    if (want_serial_time || want_diff) {
        int serial_reps = want_serial_time ? reps : 1;
        time_serial(sa, sb, n, steps, serial_reps, serial_samples, &serial_median,
                    &serial_final, &serial_delta);
        if (want_serial_time) {
            print_times("serial", serial_samples, serial_reps, serial_median);
        }
        printf("serial_final_delta=%.10g\n", serial_delta);
        if (serial_final == NULL) {
            printf("hint_serial=没有设置最终网格指针\n");
        } else if (show_grid && n <= 8) {
            print_grid("serial_grid:", serial_final, n);
        }
    }

    if (want_parallel_time) {
        time_parallel(pa, pb, n, steps, threads, reps, parallel_samples, &parallel_median,
                      &parallel_final, &parallel_delta);
        print_times("parallel", parallel_samples, reps, parallel_median);
        printf("parallel_final_delta=%.10g\n", parallel_delta);
        if (parallel_final != NULL && show_grid && n <= 8) {
            print_grid("parallel_grid:", parallel_final, n);
        }
    }

    int ok = 1;
    int serial_moved = 0;
    if (serial_final != NULL) {
        double *fresh = new_grid(n);
        init_plate(fresh, n);
        serial_moved = grid_max_abs_diff(fresh, serial_final, n) > GRID_TOL;
        if (!serial_moved) {
            ok = 0;
            printf("hint_serial=最终网格仍是初值，内部点没有被迭代更新\n");
        }
        free(fresh);
    }

    const KnownCase *known = lookup_known(n, steps);
    if (known != NULL && serial_final != NULL) {
        if (!check_known(known, serial_final, serial_delta)) {
            ok = 0;
            printf("hint=先改 TODO(init) 和 TODO(serial)\n");
        }
    }

    if (want_diff) {
        if (serial_final == NULL || parallel_final == NULL) {
            printf("max_abs_diff=na\n");
            ok = 0;
        } else if (!serial_moved) {
            printf("max_abs_diff=na\n");
            printf("diff_ok=na\n");
        } else {
            double diff = grid_max_abs_diff(serial_final, parallel_final, n);
            printf("max_abs_diff=%.3e\n", diff);
            printf("diff_ok=%d\n", diff <= GRID_TOL);
            if (diff > GRID_TOL) {
                ok = 0;
            }
            if (!approx(serial_delta, parallel_delta)) {
                printf("delta_mismatch serial=%.10g parallel=%.10g\n",
                       serial_delta, parallel_delta);
                ok = 0;
            }
        }
    }

    if (strcmp(mode, "check") == 0) {
        if (serial_median > 0.0 && parallel_median > 0.0) {
            printf("speedup=%.4f\n", serial_median / parallel_median);
        } else {
            printf("speedup=na\n");
        }
        printf("speedup_note=分母是串行实现的中位时间，不是并行版开 1 个线程的时间\n");
    }

    printf("result=%s\n", ok ? "ok" : "fail");

    free(sa);
    free(sb);
    free(pa);
    free(pb);
    return ok ? 0 : 1;
}

static int run_example(int threads) {
    struct {
        int n;
        int steps;
    } cases[] = {
        {3, 1},
        {3, 2},
        {4, 1},
        {4, 2},
    };
    int status = 0;
    for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
        printf("===== example: N=%d T=%d threads=%d =====\n",
               cases[i].n, cases[i].steps, threads);
        status |= run_case("check", cases[i].n, cases[i].steps, threads, 1, 1);
        printf("\n");
    }
    return status == 0 ? 0 : 1;
}

int main(int argc, char **argv) {
    if (argc >= 2 && strcmp(argv[1], "example") == 0) {
        if (argc > 3) {
            usage();
        }
        int threads = 3;
        if (argc == 3) {
            threads = (int)parse_long(argv[2], "threads", 1, MAX_THREADS);
        }
        return run_example(threads);
    }

    if (argc < 5 || argc > 6) {
        usage();
    }

    const char *mode = argv[1];
    if (strcmp(mode, "serial") != 0 && strcmp(mode, "parallel") != 0 &&
        strcmp(mode, "check") != 0) {
        usage();
    }

    int n = (int)parse_long(argv[2], "N", 3, 4000);
    int steps = (int)parse_long(argv[3], "T", 1, 1000000);
    int threads = (int)parse_long(argv[4], "threads", 1, MAX_THREADS);
    int reps = 3;
    if (argc == 6) {
        reps = (int)parse_long(argv[5], "reps", 1, MAX_REPS);
    }

    return run_case(mode, n, steps, threads, reps, n <= 8);
}
