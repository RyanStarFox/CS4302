/*
 * 问题 1：灰度直方图
 *
 * 只需要填写下面三处，搜索 TODO：
 *   TODO(serial)   串行直方图
 *   TODO(atomic)   共享直方图，更新计数器用 atomic
 *   TODO(private)  每个线程一份局部直方图，最后再合并
 *
 * 参数解析、造数据、计时、与串行结果对拍都已经接好。
 * 计时只包住你的函数。造数据和对拍在计时外面。
 *
 * 用法见 README.md。先运行 make example。
 */

#include <errno.h>
#include <omp.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
    HIST_BINS = 256,
    MAX_THREADS = 512,
    MAX_REPS = 100
};

typedef void (*HistFn)(const uint8_t *pixels, size_t n, uint64_t *hist, int threads);

typedef struct {
    double samples[MAX_REPS];
    double median_s;
    int reps;
    uint64_t hist[HIST_BINS];
} HistRun;

static void die(const char *msg) {
    fprintf(stderr, "error: %s\n", msg);
    exit(1);
}

static void usage(void) {
    fprintf(stderr,
            "用法:\n"
            "  ./histogram example [threads]\n"
            "  ./histogram <serial|atomic|private|check> <N> <threads> <uniform|constant> [seed] [reps]\n"
            "\n"
            "  example   作业中的小例子，以及长度为 1 的边界\n"
            "  serial    只跑串行版并计时\n"
            "  atomic    共享直方图版本，并与串行版对拍\n"
            "  private   线程私有直方图版本，并与串行版对拍\n"
            "  check     三个版本用同一次输入计时，并给出加速比\n"
            "  uniform   每个像素在 0..255 上均匀随机，种子固定\n"
            "  constant  每个像素都是 128\n"
            "  seed      默认 1；reps 默认 3，输出中位数\n");
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

static uint32_t parse_u32(const char *s) {
    char *end = NULL;
    errno = 0;
    unsigned long v = strtoul(s, &end, 10);
    if (errno != 0 || end == s || *end != '\0' || v > 4294967295ul) {
        fprintf(stderr, "seed 无效: %s\n", s);
        exit(1);
    }
    return (uint32_t)v;
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

static uint32_t lcg_next(uint32_t *state) {
    *state = *state * 1664525u + 1013904223u;
    return *state;
}

static void generate_pixels(uint8_t *pixels, size_t n, const char *dist, uint32_t seed) {
    if (strcmp(dist, "constant") == 0) {
        memset(pixels, 128, n);
        return;
    }
    if (strcmp(dist, "uniform") != 0) {
        die("分布只能是 uniform 或 constant");
    }
    uint32_t state = seed;
    for (size_t i = 0; i < n; i++) {
        pixels[i] = (uint8_t)(lcg_next(&state) >> 24);
    }
}

static uint64_t hist_sum(const uint64_t *hist) {
    uint64_t sum = 0;
    for (int v = 0; v < HIST_BINS; v++) {
        sum += hist[v];
    }
    return sum;
}

static int hist_equal(const uint64_t *a, const uint64_t *b) {
    for (int v = 0; v < HIST_BINS; v++) {
        if (a[v] != b[v]) {
            return 0;
        }
    }
    return 1;
}

static int first_mismatch(const uint64_t *a, const uint64_t *b) {
    for (int v = 0; v < HIST_BINS; v++) {
        if (a[v] != b[v]) {
            return v;
        }
    }
    return -1;
}

static int hist_all_zero(const uint64_t *hist) {
    for (int v = 0; v < HIST_BINS; v++) {
        if (hist[v] != 0) {
            return 0;
        }
    }
    return 1;
}

/* =====================================================================
 * 下面三个函数是你要交的计算。其余代码不要改，除非你发现脚手架有错。
 * hist 在函数里是指针，长度固定为 HIST_BINS（256）。
 * 清零时写循环，或 memset(hist, 0, sizeof(uint64_t) * HIST_BINS)。
 * 不要写 sizeof(hist)，那只是指针的大小。
 * parallel for 的循环变量用 long。不少编译器不接受 size_t。
 * 主程序在计时前会把 hist 清零，但清零发生在计时之外。
 * 函数每次调用都应得到完整、正确的直方图；计时会重复调用。
 * ===================================================================== */

static void histogram_serial(const uint8_t *pixels, size_t n, uint64_t *hist) {
    (void)pixels;
    (void)n;
    (void)hist;
    /* TODO(serial)
     * hist[v] = 像素值等于 v 的个数，v = 0..255。
     * 计数器用 64 位整数。先把 256 个计数清零，再遍历 pixels。
     */
}

static void histogram_atomic(const uint8_t *pixels, size_t n, uint64_t *hist, int threads) {
    (void)pixels;
    (void)n;
    (void)hist;
    (void)threads;
    /* TODO(atomic)  Version A：所有线程更新同一份 hist
     * 1. 清零 hist。清零完成之前不要开始自增。
     * 2. 用 parallel for 按像素下标划分循环，线程数用 threads。
     * 3. hist[pixels[i]]++ 必须放在 atomic 里。多个线程会写同一个 bin。
     * 不要用 critical 把整个循环包起来。
     */
}

static void histogram_private(const uint8_t *pixels, size_t n, uint64_t *hist, int threads) {
    (void)pixels;
    (void)n;
    (void)hist;
    (void)threads;
    /* TODO(private)  Version B：线程私有直方图
     * 1. 每个线程自己准备 256 个计数，并清零。
     *    可以在并行区域内用栈上的数组，也可以事先分配 threads 行。
     * 2. 线程只把分到的像素累加进自己的局部直方图。
     *    这一步不要写全局 hist。
     * 3. 输入处理完之后，再把各线程的局部直方图加进 hist。
     * 局部直方图的分配、清零和最后的合并都要留在这个函数里，因为它们计入时间。
     */
}

static void serial_adapter(const uint8_t *pixels, size_t n, uint64_t *hist, int threads) {
    (void)threads;
    histogram_serial(pixels, n, hist);
}

static void run_timed(HistFn fn, const uint8_t *pixels, size_t n, int threads,
                      int reps, HistRun *out) {
    out->reps = reps;
    for (int r = 0; r < reps; r++) {
        memset(out->hist, 0, sizeof(out->hist));
        double t0 = omp_get_wtime();
        fn(pixels, n, out->hist, threads);
        out->samples[r] = omp_get_wtime() - t0;
    }
    double sorted[MAX_REPS];
    memcpy(sorted, out->samples, (size_t)reps * sizeof(double));
    out->median_s = median(sorted, reps);
}

static void print_times(const char *name, const HistRun *run) {
    printf("%s_times_s=", name);
    for (int r = 0; r < run->reps; r++) {
        printf("%s%.6f", r ? " " : "", run->samples[r]);
    }
    printf("\n%s_time_median_s=%.6f\n", name, run->median_s);
}

static void print_speedup(const char *name, double t_serial, double t_parallel) {
    if (t_serial > 0.0 && t_parallel > 0.0) {
        printf("speedup_%s=%.4f\n", name, t_serial / t_parallel);
    } else {
        printf("speedup_%s=na\n", name);
    }
}

static void report_match(const char *name, const HistRun *got, const uint64_t *ref,
                         size_t n, int *ok) {
    int bin = first_mismatch(got->hist, ref);
    int match = bin < 0;
    printf("match_%s=%d\n", name, match);
    if (!match) {
        *ok = 0;
        printf("mismatch_%s_bin=%d serial=%llu got=%llu\n",
               name, bin,
               (unsigned long long)ref[bin],
               (unsigned long long)got->hist[bin]);
        if (hist_all_zero(got->hist)) {
            printf("hint_%s=直方图仍全是 0，对应的 TODO 还没有写出累加\n", name);
        }
    } else if (hist_sum(ref) != (uint64_t)n) {
        printf("hint_%s=和串行版相同，但串行结果还不正确，请先写 TODO(serial)\n", name);
    }
}

static int run_case(const char *mode, const uint8_t *pixels, size_t n, int threads,
                    const char *dist, uint32_t seed, int reps, const uint64_t *expect) {
    int want_serial = strcmp(mode, "serial") == 0 || strcmp(mode, "check") == 0;
    int want_atomic = strcmp(mode, "atomic") == 0 || strcmp(mode, "check") == 0;
    int want_private = strcmp(mode, "private") == 0 || strcmp(mode, "check") == 0;
    int compare_atomic = strcmp(mode, "atomic") == 0 || strcmp(mode, "check") == 0;
    int compare_private = strcmp(mode, "private") == 0 || strcmp(mode, "check") == 0;

    printf("problem=1\n");
    printf("mode=%s\n", mode);
    printf("N=%zu\n", n);
    printf("threads=%d\n", threads);
    printf("N_mod_threads=%zu\n", n % (size_t)threads);
    printf("dist=%s\n", dist);
    printf("seed=%u\n", seed);
    printf("reps=%d\n", reps);
    printf("omp_procs=%d\n", omp_get_num_procs());

    omp_set_dynamic(0);
    omp_set_num_threads(threads);

    HistRun serial_run;
    HistRun atomic_run;
    HistRun private_run;
    memset(&serial_run, 0, sizeof(serial_run));
    memset(&atomic_run, 0, sizeof(atomic_run));
    memset(&private_run, 0, sizeof(private_run));

    if (want_serial || compare_atomic || compare_private || expect != NULL) {
        run_timed(serial_adapter, pixels, n, threads, want_serial ? reps : 1, &serial_run);
        if (want_serial) {
            print_times("serial", &serial_run);
        }
    }
    if (want_atomic) {
        run_timed(histogram_atomic, pixels, n, threads, reps, &atomic_run);
        print_times("atomic", &atomic_run);
    }
    if (want_private) {
        run_timed(histogram_private, pixels, n, threads, reps, &private_run);
        print_times("private", &private_run);
    }

    int ok = 1;
    uint64_t serial_sum = hist_sum(serial_run.hist);
    printf("serial_sum=%llu\n", (unsigned long long)serial_sum);
    printf("sum_ok=%d\n", serial_sum == (uint64_t)n);
    if (serial_sum != (uint64_t)n) {
        ok = 0;
        if (hist_all_zero(serial_run.hist)) {
            printf("hint_serial=串行直方图仍全是 0，请先完成 TODO(serial)\n");
        }
    }

    if (expect != NULL && !hist_equal(serial_run.hist, expect)) {
        ok = 0;
        int bin = first_mismatch(serial_run.hist, expect);
        printf("example_serial_match=0\n");
        printf("example_mismatch_bin=%d expect=%llu got=%llu\n",
               bin,
               (unsigned long long)expect[bin],
               (unsigned long long)serial_run.hist[bin]);
    } else if (expect != NULL) {
        printf("example_serial_match=1\n");
    }

    if (compare_atomic) {
        report_match("atomic", &atomic_run, serial_run.hist, n, &ok);
        uint64_t sum = hist_sum(atomic_run.hist);
        printf("atomic_sum=%llu\n", (unsigned long long)sum);
        printf("atomic_sum_ok=%d\n", sum == (uint64_t)n);
        if (sum != (uint64_t)n) {
            ok = 0;
        }
    }
    if (compare_private) {
        report_match("private", &private_run, serial_run.hist, n, &ok);
        uint64_t sum = hist_sum(private_run.hist);
        printf("private_sum=%llu\n", (unsigned long long)sum);
        printf("private_sum_ok=%d\n", sum == (uint64_t)n);
        if (sum != (uint64_t)n) {
            ok = 0;
        }
    }

    if (strcmp(mode, "check") == 0) {
        print_speedup("atomic", serial_run.median_s, atomic_run.median_s);
        print_speedup("private", serial_run.median_s, private_run.median_s);
        printf("speedup_note=分母是串行实现的中位时间，不是并行版开 1 个线程的时间\n");
    }

    printf("result=%s\n", ok ? "ok" : "fail");
    return ok ? 0 : 1;
}

static int run_example(int threads) {
    int status = 0;

    const uint8_t sample[] = {0, 1, 1, 2, 2, 2, 255, 255};
    uint64_t expect[HIST_BINS];
    memset(expect, 0, sizeof(expect));
    expect[0] = 1;
    expect[1] = 2;
    expect[2] = 3;
    expect[255] = 2;

    printf("===== example: 作业给出的 8 个像素，threads=%d =====\n", threads);
    status |= run_case("check", sample, sizeof(sample), threads, "given", 0, 1, expect);

    uint8_t one[] = {42};
    uint64_t expect_one[HIST_BINS];
    memset(expect_one, 0, sizeof(expect_one));
    expect_one[42] = 1;

    printf("\n===== example: 长度为 1 =====\n");
    status |= run_case("check", one, 1, threads, "given", 0, 1, expect_one);
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

    if (argc < 5 || argc > 7) {
        usage();
    }

    const char *mode = argv[1];
    if (strcmp(mode, "serial") != 0 && strcmp(mode, "atomic") != 0 &&
        strcmp(mode, "private") != 0 && strcmp(mode, "check") != 0) {
        usage();
    }

    size_t n = (size_t)parse_long(argv[2], "N", 1, 2000000000L);
    int threads = (int)parse_long(argv[3], "threads", 1, MAX_THREADS);
    const char *dist = argv[4];
    uint32_t seed = 1;
    int reps = 3;
    if (argc >= 6) {
        seed = parse_u32(argv[5]);
    }
    if (argc >= 7) {
        reps = (int)parse_long(argv[6], "reps", 1, MAX_REPS);
    }
    if (strcmp(dist, "uniform") != 0 && strcmp(dist, "constant") != 0) {
        die("分布只能是 uniform 或 constant");
    }

    uint8_t *pixels = malloc(n);
    if (pixels == NULL) {
        die("内存不足");
    }
    generate_pixels(pixels, n, dist, seed);

    int status = run_case(mode, pixels, n, threads, dist, seed, reps, NULL);
    free(pixels);
    return status;
}
