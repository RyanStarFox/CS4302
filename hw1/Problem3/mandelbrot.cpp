/*
 * 问题 3：Mandelbrot 迭代次数与 OpenMP 调度
 *
 * 只需要填写下面五处，搜索 TODO：
 *   TODO(pixel)    单个复平面点的迭代次数
 *   TODO(serial)   按行计算整张图，统计写入 stats[0]
 *   TODO(static)   行循环 schedule(static)
 *   TODO(dynamic)  行循环 schedule(dynamic, 1)
 *   TODO(guided)   行循环 schedule(guided, 1)
 *
 * 三种并行版本必须各自写出 schedule 子句。不要只靠环境变量 OMP_SCHEDULE。
 * 每个并行迭代计算完整的一行，这样三种调度的工作单位相同。
 * 计时只包住 compute_*。坐标公式和像素函数都算在计算里面。
 *
 * 用法见 README.md。先运行 make example。
 */

#include <omp.h>

#include <algorithm>
#include <array>
#include <charconv>
#include <cstdlib>
#include <iostream>
#include <string>
#include <system_error>
#include <vector>

enum {
    MAX_THREADS = 512,
    MAX_REPS = 100
};

struct ThreadStat {
    long pixels = 0;
    long iter_sum = 0;
};

using ComputeFn = void (*)(int, int, int, int, std::vector<int> &, std::vector<ThreadStat> &);

[[noreturn]] static void usage() {
    std::cerr
        << "用法:\n"
        << "  ./mandelbrot example [threads]\n"
        << "  ./mandelbrot <serial|static|dynamic|guided|check> <W> <H> <K> <threads> [reps]\n"
        << "\n"
        << "  W, H    图像宽和高，都 >= 2\n"
        << "  K       每个像素最多迭代次数，K >= 1\n"
        << "  threads 并行线程数\n"
        << "  reps    默认 3，输出中位数\n"
        << "  check   三种调度都与串行版逐像素比较，并打印每个线程的工作量\n"
        << "  example 先检查 c=0 和 c=1，再用一个高度不能被线程数整除的小图对拍\n";
    std::exit(1);
}

static long parse_long(const std::string &s, const char *name, long minv, long maxv) {
    long v = 0;
    const char *begin = s.data();
    const char *end = begin + s.size();
    auto [ptr, ec] = std::from_chars(begin, end, v);
    if (ec != std::errc() || ptr != end || v < minv || v > maxv) {
        std::cerr << "参数 " << name << " 无效: " << s << "（允许 " << minv << ".." << maxv << "）\n";
        std::exit(1);
    }
    return v;
}

static double median(std::array<double, MAX_REPS> v, int n) {
    std::sort(v.begin(), v.begin() + n);
    if (n % 2 == 1) {
        return v[n / 2];
    }
    return 0.5 * (v[n / 2 - 1] + v[n / 2]);
}

/* =====================================================================
 * 下面五个函数是你要交的计算。
 * out 按行优先：第 i 行第 j 列是 out[(long)i * w + j]。
 * stats 的长度：串行版只用 stats[0]；并行版有 threads 项，下标用线程号。
 * 统计在线程本地累加，算完再写入 stats。不要为了统计给每个像素加全局锁。
 * ===================================================================== */

static int pixel_iterations(double x, double y, int k) {
    (void)x;
    (void)y;
    (void)k;
    /* TODO(pixel)
     * z 从 0 开始，反复做 z = z^2 + c，c = x + y i。
     * 不使用复数库。令 z = a + b i，则一步是：
     *   a_new = a*a - b*b + x
     *   b_new = 2*a*b + y
     * 两个新值都用更新前的 a 和 b。
     * 当 a*a + b*b > 4，或已经做完 k 次时停止。
     * 返回实际做了多少次。
     * 作业给出的两个点：c = 0 时应返回 k；c = 1 且 k >= 3 时应返回 3。
     * 还没写的时候保持 return -1，主程序会直接停下来。
     */
    return -1;
}

static void compute_serial(int w, int h, int k, std::vector<int> &out, std::vector<ThreadStat> &stats) {
    (void)w;
    (void)h;
    (void)k;
    (void)out;
    (void)stats;
    /* TODO(serial)
     * 像素 (i, j) 对应
     *   x = -2 + 3.0 * j / static_cast<double>(w - 1)
     *   y = -1.5 + 3.0 * i / static_cast<double>(h - 1)
     * 用浮点除法。把 pixel_iterations 的结果写入 out。
     * stats[0].pixels 是像素总数，stats[0].iter_sum 是所有像素迭代次数之和。
     * 用 long，不要用 int 做 iter_sum。
     */
}

static void compute_static(int w, int h, int k, int threads, std::vector<int> &out,
                           std::vector<ThreadStat> &stats) {
    (void)w;
    (void)h;
    (void)k;
    (void)threads;
    (void)out;
    (void)stats;
    /* TODO(static)
     * 只并行最外层的行循环，schedule(static)。
     * 一个迭代负责一整行。
     * 每个线程用私有变量累加自己处理的像素数和迭代次数之和，
     * 再写入 stats[omp_get_thread_num()]。每个线程只写自己的槽位。
     */
}

static void compute_dynamic(int w, int h, int k, int threads, std::vector<int> &out,
                            std::vector<ThreadStat> &stats) {
    (void)w;
    (void)h;
    (void)k;
    (void)threads;
    (void)out;
    (void)stats;
    /* TODO(dynamic)
     * 与 static 相同，但 schedule(dynamic, 1)：每次分发 1 行。
     */
}

static void compute_guided(int w, int h, int k, int threads, std::vector<int> &out,
                           std::vector<ThreadStat> &stats) {
    (void)w;
    (void)h;
    (void)k;
    (void)threads;
    (void)out;
    (void)stats;
    /* TODO(guided)
     * 与 static 相同，但 schedule(guided, 1)：最小块大小是 1 行。
     */
}

static bool self_check_pixel(int k) {
    bool ok = true;
    const int c0 = pixel_iterations(0.0, 0.0, k);
    const int c1 = pixel_iterations(1.0, 0.0, k);
    std::cout << "pixel_c0=" << c0 << " expect=" << k << '\n';
    if (c0 != k) {
        std::cerr << "c = 0 的迭代次数应为 " << k << "，实际 " << c0
                  << "。请先完成 TODO(pixel)。\n";
        ok = false;
    }
    if (k >= 3) {
        std::cout << "pixel_c1=" << c1 << " expect=3\n";
        if (c1 != 3) {
            std::cerr << "c = 1 且 K >= 3 时迭代次数应为 3，实际 " << c1 << "。\n";
            ok = false;
        }
    }
    return ok;
}

static void print_matrix(const std::vector<int> &out, int w, int h) {
    std::cout << "iterations:\n";
    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            std::cout << ' ' << out[static_cast<long>(i) * w + j];
        }
        std::cout << '\n';
    }
}

static long find_mismatch(const std::vector<int> &a, const std::vector<int> &b, long cells) {
    for (long i = 0; i < cells; i++) {
        if (a[i] != b[i]) {
            return i;
        }
    }
    return -1;
}

static void print_stats(const char *name, const std::vector<ThreadStat> &stats) {
    long pixels = 0;
    long iter_sum = 0;
    for (int t = 0; t < static_cast<int>(stats.size()); t++) {
        std::cout << name << "_thread=" << t << " pixels=" << stats[t].pixels
                  << " iter_sum=" << stats[t].iter_sum << '\n';
        pixels += stats[t].pixels;
        iter_sum += stats[t].iter_sum;
    }
    std::cout << name << "_pixels_total=" << pixels << '\n';
    std::cout << name << "_iter_sum_total=" << iter_sum << '\n';
}

static bool stats_cover_image(const char *name, const std::vector<ThreadStat> &stats, long cells) {
    long pixels = 0;
    for (const ThreadStat &s : stats) {
        pixels += s.pixels;
    }
    if (pixels != cells) {
        std::cerr << name << " 的像素统计之和是 " << pixels << "，图像有 " << cells
                  << " 个像素。请把每个线程的 pixels / iter_sum 写入 stats。\n";
        return false;
    }
    return true;
}

static void serial_adapter(int w, int h, int k, int threads, std::vector<int> &out,
                           std::vector<ThreadStat> &stats) {
    (void)threads;
    compute_serial(w, h, k, out, stats);
}

static void time_compute(ComputeFn fn, int w, int h, int k, int threads, int reps,
                         std::vector<int> &out, std::vector<ThreadStat> &stats,
                         std::array<double, MAX_REPS> &samples, double &median_s) {
    for (int r = 0; r < reps; r++) {
        std::fill(out.begin(), out.end(), 0);
        std::fill(stats.begin(), stats.end(), ThreadStat{});
        const double t0 = omp_get_wtime();
        fn(w, h, k, threads, out, stats);
        samples[r] = omp_get_wtime() - t0;
    }
    median_s = median(samples, reps);
}

static void print_times(const char *name, const std::array<double, MAX_REPS> &samples, int reps,
                        double median_s) {
    std::cout.setf(std::ios::fixed);
    std::cout.precision(6);
    std::cout << name << "_times_s=";
    for (int r = 0; r < reps; r++) {
        std::cout << (r ? " " : "") << samples[r];
    }
    std::cout << '\n' << name << "_time_median_s=" << median_s << '\n';
    std::cout.unsetf(std::ios::fixed);
}

static int run_case(const std::string &mode, int w, int h, int k, int threads, int reps,
                    bool show_matrix) {
    const bool want_serial = mode == "serial" || mode == "check";
    const bool want_static = mode == "static" || mode == "check";
    const bool want_dynamic = mode == "dynamic" || mode == "check";
    const bool want_guided = mode == "guided" || mode == "check";
    const bool compare = mode == "check" || want_static || want_dynamic || want_guided;

    std::cout << "problem=3\n"
              << "mode=" << mode << '\n'
              << "W=" << w << '\n'
              << "H=" << h << '\n'
              << "K=" << k << '\n'
              << "threads=" << threads << '\n'
              << "H_mod_threads=" << (h % threads) << '\n'
              << "reps=" << reps << '\n'
              << "omp_procs=" << omp_get_num_procs() << '\n';

    if (!self_check_pixel(k)) {
        std::cout << "result=fail\n";
        return 1;
    }

    omp_set_dynamic(0);
    omp_set_num_threads(threads);

    const long cells = static_cast<long>(w) * static_cast<long>(h);
    std::vector<int> serial_out(static_cast<size_t>(cells), 0);
    std::vector<int> trial_out(static_cast<size_t>(cells), 0);
    std::vector<ThreadStat> serial_stats(1);
    std::vector<ThreadStat> trial_stats(static_cast<size_t>(threads));

    std::array<double, MAX_REPS> samples{};
    double serial_median = 0.0;
    int ok = 1;

    if (want_serial || compare) {
        const int serial_reps = want_serial ? reps : 1;
        time_compute(serial_adapter, w, h, k, threads, serial_reps, serial_out, serial_stats,
                     samples, serial_median);
        if (want_serial) {
            print_times("serial", samples, serial_reps, serial_median);
        }
        print_stats("serial", serial_stats);
        if (!stats_cover_image("serial", serial_stats, cells)) {
            ok = 0;
        }
        const long untouched = std::count(serial_out.begin(), serial_out.end(), 0);
        if (untouched > 0) {
            ok = 0;
            std::cout << "hint_serial=有 " << untouched
                      << " 个像素仍是 0。z 从 0 出发，正确实现里每个像素至少迭代 1 次。\n";
        }
        if (show_matrix && cells <= 64) {
            print_matrix(serial_out, w, h);
        }
    }

    struct Version {
        const char *name;
        ComputeFn fn;
        bool enabled;
    };
    const Version versions[] = {
        {"static", compute_static, want_static},
        {"dynamic", compute_dynamic, want_dynamic},
        {"guided", compute_guided, want_guided},
    };

    for (const Version &version : versions) {
        if (!version.enabled) {
            continue;
        }
        double med = 0.0;
        time_compute(version.fn, w, h, k, threads, reps, trial_out, trial_stats, samples, med);
        print_times(version.name, samples, reps, med);
        print_stats(version.name, trial_stats);
        if (!stats_cover_image(version.name, trial_stats, cells)) {
            ok = 0;
        }
        const long bad = find_mismatch(serial_out, trial_out, cells);
        std::cout << "match_" << version.name << '=' << (bad < 0 ? 1 : 0) << '\n';
        if (bad >= 0) {
            ok = 0;
            std::cout << "mismatch_" << version.name << "_index=" << bad << " row=" << (bad / w)
                      << " col=" << (bad % w) << " serial=" << serial_out[bad]
                      << " got=" << trial_out[bad] << '\n';
        }
        if (mode == "check") {
            if (serial_median > 0.0 && med > 0.0) {
                std::cout.setf(std::ios::fixed);
                std::cout.precision(4);
                std::cout << "speedup_" << version.name << '=' << (serial_median / med) << '\n';
                std::cout.unsetf(std::ios::fixed);
            } else {
                std::cout << "speedup_" << version.name << "=na\n";
            }
        }
    }

    if (mode == "check") {
        std::cout << "speedup_note=分母是串行实现的中位时间，不是并行版开 1 个线程的时间\n";
    }

    std::cout << "result=" << (ok ? "ok" : "fail") << '\n';
    return ok ? 0 : 1;
}

static int run_example(int threads) {
    const int w = 8;
    const int h = 5;
    const int k = 100;
    std::cout << "===== example: 单像素，以及 " << w << "x" << h
              << " 小图（高度不能被 " << threads << " 整除）=====\n";
    if (h % threads == 0) {
        std::cerr << "example 希望高度不能被线程数整除。当前 H=" << h << ", threads=" << threads
                  << "。\n换一个不能整除的线程数，例如 ./mandelbrot example 3\n";
        return 1;
    }
    return run_case("check", w, h, k, threads, 1, true);
}

int main(int argc, char **argv) {
    if (argc >= 2 && std::string(argv[1]) == "example") {
        if (argc > 3) {
            usage();
        }
        int threads = 3;
        if (argc == 3) {
            threads = static_cast<int>(parse_long(argv[2], "threads", 1, MAX_THREADS));
        }
        return run_example(threads);
    }

    if (argc < 6 || argc > 7) {
        usage();
    }

    const std::string mode = argv[1];
    if (mode != "serial" && mode != "static" && mode != "dynamic" && mode != "guided" &&
        mode != "check") {
        usage();
    }

    const int w = static_cast<int>(parse_long(argv[2], "W", 2, 20000));
    const int h = static_cast<int>(parse_long(argv[3], "H", 2, 20000));
    const int k = static_cast<int>(parse_long(argv[4], "K", 1, 1000000));
    const int threads = static_cast<int>(parse_long(argv[5], "threads", 1, MAX_THREADS));
    int reps = 3;
    if (argc == 7) {
        reps = static_cast<int>(parse_long(argv[6], "reps", 1, MAX_REPS));
    }

    return run_case(mode, w, h, k, threads, reps, false);
}
