/*
 * 问题 2：二维平板温度迭代
 *
 * 只需要填写下面三处，搜索 TODO：
 *   TODO(init)      边界和初值
 *   TODO(serial)    串行迭代
 *   TODO(parallel)  每一轮的内部点并行，delta 用 reduction
 *
 * 网格按行优先存放在 vector 里：下标 (i, j) 是 grid[(size_t)i * n + j]。
 * 第 0 行是上边界。内部点的 i、j 都从 1 到 n - 2。
 * 计时只包住 iterate_*。初始化在计时外面。
 *
 * 用法见 README.md。写完一个版本就可以 make init、make serial 或 make parallel。
 */

#include <omp.h>

#include <algorithm>
#include <array>
#include <charconv>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>
#include <system_error>
#include <vector>

enum {
    MAX_THREADS = 512,
    MAX_REPS = 100
};

static const double GRID_TOL = 1e-8;

[[noreturn]] static void usage() {
    std::cerr
        << "用法:\n"
        << "  ./heat example [init|serial|parallel|check] [threads]\n"
        << "  ./heat init <N>\n"
        << "  ./heat <serial|parallel|check> <N> <T> <threads> [reps]\n"
        << "\n"
        << "  N       网格边长，N >= 3\n"
        << "  T       迭代次数，T >= 1，必须刚好做 T 轮\n"
        << "  threads 并行线程数；串行版会忽略它，但命令行仍要写\n"
        << "  reps    默认 3，输出中位数\n"
        << "  init     只检查初始边界，不跑迭代\n"
        << "  serial   只跑串行迭代。N=3/4、T=1/2 时对照已知网格\n"
        << "  parallel 只跑并行迭代。有已知网格时不要求串行版已写完\n"
        << "  check    串行和并行对拍，并给出加速比\n"
        << "  example  可只测一种实现。默认 check，覆盖 N=3/4、T=1/2\n";
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

static std::vector<double> new_grid(int n) {
    return std::vector<double>(static_cast<size_t>(n) * static_cast<size_t>(n), 0.0);
}

static void print_grid(const char *title, const double *grid, int n) {
    std::cout << title << '\n';
    std::cout.setf(std::ios::fixed);
    std::cout.precision(3);
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            std::cout << ' ' << grid[static_cast<size_t>(i) * static_cast<size_t>(n) + static_cast<size_t>(j)];
        }
        std::cout << '\n';
    }
    std::cout.unsetf(std::ios::fixed);
}

static double grid_max_abs_diff(const double *a, const double *b, int n) {
    double m = 0.0;
    const size_t cells = static_cast<size_t>(n) * static_cast<size_t>(n);
    for (size_t k = 0; k < cells; k++) {
        const double d = std::fabs(a[k] - b[k]);
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
 *   buf_a 和 buf_b 都已经用 init_plate 初始化，长度都是 n * n。
 *   做完 steps 轮之后，把 final_grid 指到存放最终温度的那一块。
 *   它必须是 buf_a.data() 或 buf_b.data()，不要指向函数里的局部数组。
 *   返回值是最后一轮的最大绝对温度变化，不是所有轮次里最大的那个。
 * ===================================================================== */

static void init_plate(std::vector<double> &grid, int n) {
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

static double iterate_serial(std::vector<double> &buf_a, std::vector<double> &buf_b, int n, int steps,
                             double *&final_grid) {
    (void)n;
    (void)buf_b;
    (void)steps;
    /* TODO(serial)
     * 刚好迭代 steps 次，不要因为看起来收敛了就提前停。
     * 内部点的新温度 = 上一轮上下左右四个邻居的平均值。
     * 边界不要改。
     * 读一块缓冲、写另一块，一轮结束后交换两块的角色。
     * 不要原地更新，否则邻居会混进本轮已经写过的值。
     * 返回最后一轮的 delta。final_grid 指向最终结果。
     *
     * 交换角色可以写成：
     *   double *current = buf_a.data();
     *   double *next = buf_b.data();
     *   ... 计算 ...
     *   double *tmp = current; current = next; next = tmp;
     *   final_grid = current;
     */
    final_grid = buf_a.data();
    return 0.0;
}

static double iterate_parallel(std::vector<double> &buf_a, std::vector<double> &buf_b, int n,
                               int steps, int threads, double *&final_grid) {
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
     * 返回最后一轮的 delta，并设置 final_grid。
     * 哪一次同步是必须的，写进报告第 3 题，不要只在代码里留一个说不清的 barrier。
     */
    final_grid = buf_a.data();
    return 0.0;
}

struct KnownCase {
    int n;
    int steps;
    const double *grid;
    double delta;
    const char *name;
};

static bool approx(double a, double b) {
    return std::fabs(a - b) <= GRID_TOL;
}

static bool check_known(const KnownCase &k, const char *who, const double *grid, double delta) {
    const size_t cells = static_cast<size_t>(k.n) * static_cast<size_t>(k.n);
    bool grid_ok = true;
    for (size_t i = 0; i < cells; i++) {
        if (!approx(grid[i], k.grid[i])) {
            grid_ok = false;
            break;
        }
    }
    const bool delta_ok = approx(delta, k.delta);
    std::cout << who << "_known_" << k.name << "_grid=" << (grid_ok ? "ok" : "fail") << '\n';
    std::cout << who << "_known_" << k.name << "_delta=" << (delta_ok ? "ok" : "fail")
              << " expect=" << k.delta << " got=" << delta << '\n';
    if (!grid_ok) {
        print_grid("期望网格:", k.grid, k.n);
        print_grid("实际网格:", grid, k.n);
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
    for (const KnownCase &k : cases) {
        if (k.n == n && k.steps == steps) {
            return &k;
        }
    }
    return nullptr;
}

static void reset_plate(std::vector<double> &a, std::vector<double> &b, int n) {
    std::fill(a.begin(), a.end(), 0.0);
    std::fill(b.begin(), b.end(), 0.0);
    init_plate(a, n);
    init_plate(b, n);
}

static void time_serial(std::vector<double> &a, std::vector<double> &b, int n, int steps, int reps,
                        std::array<double, MAX_REPS> &samples, double &median_s, double *&final_grid,
                        double &delta) {
    delta = 0.0;
    final_grid = nullptr;
    for (int r = 0; r < reps; r++) {
        reset_plate(a, b, n);
        const double t0 = omp_get_wtime();
        delta = iterate_serial(a, b, n, steps, final_grid);
        samples[r] = omp_get_wtime() - t0;
    }
    median_s = median(samples, reps);
}

static void time_parallel(std::vector<double> &a, std::vector<double> &b, int n, int steps,
                          int threads, int reps, std::array<double, MAX_REPS> &samples,
                          double &median_s, double *&final_grid, double &delta) {
    delta = 0.0;
    final_grid = nullptr;
    for (int r = 0; r < reps; r++) {
        reset_plate(a, b, n);
        const double t0 = omp_get_wtime();
        delta = iterate_parallel(a, b, n, steps, threads, final_grid);
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

static bool plate_moved(const double *grid, int n) {
    std::vector<double> fresh = new_grid(n);
    init_plate(fresh, n);
    return grid_max_abs_diff(fresh.data(), grid, n) > GRID_TOL;
}

static bool check_result(const char *who, const double *grid, double delta, int n, int steps, int &ok) {
    if (grid == nullptr) {
        std::cout << "hint_" << who << "=没有设置最终网格指针\n";
        ok = 0;
        return false;
    }
    if (!plate_moved(grid, n)) {
        std::cout << "hint_" << who << "=最终网格仍是初值，内部点没有被迭代更新\n";
        ok = 0;
        return false;
    }
    const KnownCase *known = lookup_known(n, steps);
    if (known != nullptr && !check_known(*known, who, grid, delta)) {
        std::cout << "hint_" << who << "=网格或最后一轮 delta 与已知例子不一致\n";
        ok = 0;
        return false;
    }
    return true;
}

static int run_case(const std::string &mode, int n, int steps, int threads, int reps, bool show_grid) {
    const bool known_parallel = mode == "parallel" && lookup_known(n, steps) != nullptr;
    const bool want_serial = mode == "serial" || mode == "check" || (mode == "parallel" && !known_parallel);
    const bool want_parallel = mode == "parallel" || mode == "check";
    const bool want_diff = mode == "check" || (mode == "parallel" && !known_parallel);

    std::cout << "problem=2\n"
              << "mode=" << mode << '\n'
              << "N=" << n << '\n'
              << "T=" << steps << '\n'
              << "threads=" << threads << '\n'
              << "reps=" << reps << '\n'
              << "omp_procs=" << omp_get_num_procs() << '\n';

    omp_set_dynamic(0);
    omp_set_num_threads(threads);

    std::vector<double> sa = new_grid(n);
    std::vector<double> sb = new_grid(n);
    std::vector<double> pa = new_grid(n);
    std::vector<double> pb = new_grid(n);

    std::array<double, MAX_REPS> serial_samples{};
    std::array<double, MAX_REPS> parallel_samples{};
    double serial_median = 0.0;
    double parallel_median = 0.0;
    double *serial_final = nullptr;
    double *parallel_final = nullptr;
    double serial_delta = 0.0;
    double parallel_delta = 0.0;

    if (want_serial) {
        const int serial_reps = (mode == "serial" || mode == "check") ? reps : 1;
        time_serial(sa, sb, n, steps, serial_reps, serial_samples, serial_median, serial_final,
                    serial_delta);
        if (mode == "serial" || mode == "check") {
            print_times("serial", serial_samples, serial_reps, serial_median);
        }
        std::cout << "serial_final_delta=" << serial_delta << '\n';
        if (serial_final != nullptr && show_grid && n <= 8) {
            print_grid("serial_grid:", serial_final, n);
        }
    }

    if (want_parallel) {
        time_parallel(pa, pb, n, steps, threads, reps, parallel_samples, parallel_median,
                      parallel_final, parallel_delta);
        print_times("parallel", parallel_samples, reps, parallel_median);
        std::cout << "parallel_final_delta=" << parallel_delta << '\n';
        if (parallel_final != nullptr && show_grid && n <= 8) {
            print_grid("parallel_grid:", parallel_final, n);
        }
    }

    int ok = 1;
    if (want_serial) {
        check_result("serial", serial_final, serial_delta, n, steps, ok);
    }
    if (want_parallel) {
        check_result("parallel", parallel_final, parallel_delta, n, steps, ok);
    }

    if (want_diff) {
        if (ok == 0) {
            std::cout << "max_abs_diff=na\n";
            std::cout << "diff_ok=na\n";
        } else {
            const double diff = grid_max_abs_diff(serial_final, parallel_final, n);
            std::cout.setf(std::ios::scientific);
            std::cout.precision(3);
            std::cout << "max_abs_diff=" << diff << '\n';
            std::cout.unsetf(std::ios::scientific);
            std::cout << "diff_ok=" << (diff <= GRID_TOL ? 1 : 0) << '\n';
            if (diff > GRID_TOL) {
                ok = 0;
            }
            if (!approx(serial_delta, parallel_delta)) {
                std::cout << "delta_mismatch serial=" << serial_delta
                          << " parallel=" << parallel_delta << '\n';
                ok = 0;
            }
        }
    }

    if (mode == "check") {
        if (serial_median > 0.0 && parallel_median > 0.0) {
            std::cout.setf(std::ios::fixed);
            std::cout.precision(4);
            std::cout << "speedup=" << (serial_median / parallel_median) << '\n';
            std::cout.unsetf(std::ios::fixed);
        } else {
            std::cout << "speedup=na\n";
        }
        std::cout << "speedup_note=分母是串行实现的中位时间，不是并行版开 1 个线程的时间\n";
    }

    std::cout << "result=" << (ok ? "ok" : "fail") << '\n';
    return ok ? 0 : 1;
}

static int run_init_case(int n) {
    std::cout << "===== init: N=" << n << " =====\n";
    std::vector<double> grid = new_grid(n);
    init_plate(grid, n);
    bool ok = true;
    int reported = 0;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            const double expect = (i == 0 && j > 0 && j + 1 < n) ? 100.0 : 0.0;
            const double got =
                grid[static_cast<size_t>(i) * static_cast<size_t>(n) + static_cast<size_t>(j)];
            if (!approx(got, expect)) {
                ok = false;
                if (reported < 4) {
                    std::cout << "init_mismatch i=" << i << " j=" << j << " expect=" << expect
                              << " got=" << got << '\n';
                    reported++;
                }
            }
        }
    }
    std::cout << "init_ok=" << (ok ? 1 : 0) << '\n';
    if (!ok) {
        std::cout << "hint_init=上边界中间应为 100，四个角和其他位置应为 0\n";
        if (n <= 8) {
            print_grid("init_grid:", grid.data(), n);
        }
    }
    std::cout << "result=" << (ok ? "ok" : "fail") << '\n';
    return ok ? 0 : 1;
}

static int run_init_suite() {
    int status = 0;
    status |= run_init_case(3);
    std::cout << '\n';
    status |= run_init_case(4);
    return status == 0 ? 0 : 1;
}

static int run_example(const std::string &mode, int threads) {
    if (mode == "init") {
        return run_init_suite();
    }
    const struct {
        int n;
        int steps;
    } cases[] = {
        {3, 1},
        {3, 2},
        {4, 1},
        {4, 2},
    };
    int status = 0;
    for (const auto &c : cases) {
        std::cout << "===== example: N=" << c.n << " T=" << c.steps << " mode=" << mode
                  << " threads=" << threads << " =====\n";
        status |= run_case(mode, c.n, c.steps, threads, 1, true);
        std::cout << '\n';
    }
    return status == 0 ? 0 : 1;
}

int main(int argc, char **argv) {
    if (argc >= 2 && std::string(argv[1]) == "example") {
        std::string mode = "check";
        int threads = 3;
        int idx = 2;
        if (idx < argc) {
            const std::string arg = argv[idx];
            if (arg == "init" || arg == "serial" || arg == "parallel" || arg == "check") {
                mode = arg;
                idx++;
            }
        }
        if (idx < argc) {
            threads = static_cast<int>(parse_long(argv[idx], "threads", 1, MAX_THREADS));
            idx++;
        }
        if (idx != argc) {
            usage();
        }
        return run_example(mode, threads);
    }

    if (argc >= 2 && std::string(argv[1]) == "init") {
        if (argc != 3) {
            usage();
        }
        const int n = static_cast<int>(parse_long(argv[2], "N", 3, 4000));
        return run_init_case(n);
    }

    if (argc < 5 || argc > 6) {
        usage();
    }

    const std::string mode = argv[1];
    if (mode != "serial" && mode != "parallel" && mode != "check") {
        usage();
    }

    const int n = static_cast<int>(parse_long(argv[2], "N", 3, 4000));
    const int steps = static_cast<int>(parse_long(argv[3], "T", 1, 1000000));
    const int threads = static_cast<int>(parse_long(argv[4], "threads", 1, MAX_THREADS));
    int reps = 3;
    if (argc == 6) {
        reps = static_cast<int>(parse_long(argv[5], "reps", 1, MAX_REPS));
    }

    return run_case(mode, n, steps, threads, reps, n <= 8);
}
