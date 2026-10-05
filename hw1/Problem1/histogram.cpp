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

#include <omp.h>

#include <algorithm>
#include <array>
#include <charconv>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <string>
#include <system_error>
#include <vector>

using std::uint32_t;
using std::uint64_t;
using std::uint8_t;

enum {
    HIST_BINS = 256,
    MAX_THREADS = 512,
    MAX_REPS = 100
};

using HistFn = void (*)(const std::vector<uint8_t> &, std::vector<uint64_t> &, int);

struct HistRun {
    std::array<double, MAX_REPS> samples{};
    double median_s = 0.0;
    int reps = 0;
    std::vector<uint64_t> hist;
};

[[noreturn]] static void die(const std::string &msg) {
    std::cerr << "error: " << msg << '\n';
    std::exit(1);
}

[[noreturn]] static void usage() {
    std::cerr
        << "用法:\n"
        << "  ./histogram example [threads]\n"
        << "  ./histogram <serial|atomic|private|check> <N> <threads> <uniform|constant> [seed] [reps]\n"
        << "\n"
        << "  example   作业中的小例子，以及长度为 1 的边界\n"
        << "  serial    只跑串行版并计时\n"
        << "  atomic    共享直方图版本，并与串行版对拍\n"
        << "  private   线程私有直方图版本，并与串行版对拍\n"
        << "  check     三个版本用同一次输入计时，并给出加速比\n"
        << "  uniform   每个像素在 0..255 上均匀随机，种子固定\n"
        << "  constant  每个像素都是 128\n"
        << "  seed      默认 1；reps 默认 3，输出中位数\n";
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

static uint32_t parse_u32(const std::string &s) {
    uint32_t v = 0;
    const char *begin = s.data();
    const char *end = begin + s.size();
    auto [ptr, ec] = std::from_chars(begin, end, v);
    if (ec != std::errc() || ptr != end) {
        std::cerr << "seed 无效: " << s << '\n';
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

static uint32_t lcg_next(uint32_t &state) {
    state = state * 1664525u + 1013904223u;
    return state;
}

static void generate_pixels(std::vector<uint8_t> &pixels, const std::string &dist, uint32_t seed) {
    if (dist == "constant") {
        std::fill(pixels.begin(), pixels.end(), static_cast<uint8_t>(128));
        return;
    }
    if (dist != "uniform") {
        die("分布只能是 uniform 或 constant");
    }
    uint32_t state = seed;
    for (uint8_t &pixel : pixels) {
        pixel = static_cast<uint8_t>(lcg_next(state) >> 24);
    }
}

static uint64_t hist_sum(const std::vector<uint64_t> &hist) {
    uint64_t sum = 0;
    for (uint64_t bin : hist) {
        sum += bin;
    }
    return sum;
}

static bool hist_equal(const std::vector<uint64_t> &a, const std::vector<uint64_t> &b) {
    return a == b;
}

static int first_mismatch(const std::vector<uint64_t> &a, const std::vector<uint64_t> &b) {
    const int n = static_cast<int>(std::min(a.size(), b.size()));
    for (int v = 0; v < n; v++) {
        if (a[v] != b[v]) {
            return v;
        }
    }
    return -1;
}

static bool hist_all_zero(const std::vector<uint64_t> &hist) {
    return std::all_of(hist.begin(), hist.end(), [](uint64_t v) { return v == 0; });
}

/* =====================================================================
 * 下面三个函数是你要交的计算。其余代码不要改，除非你发现脚手架有错。
 * hist 的长度固定为 HIST_BINS（256），不要 resize。
 * 清零可以写循环，或 std::fill(hist.begin(), hist.end(), 0)。
 * parallel for 的循环变量用 long。不少编译器不接受 size_t。
 * 主程序在计时前会把 hist 清零，但清零发生在计时之外。
 * 函数每次调用都应得到完整、正确的直方图；计时会重复调用。
 * ===================================================================== */

static void histogram_serial(const std::vector<uint8_t> &pixels, std::vector<uint64_t> &hist) {
    (void)pixels;
    (void)hist;
    /* TODO(serial)
     * hist[v] = 像素值等于 v 的个数，v = 0..255。
     * 计数器用 64 位整数。先把 256 个计数清零，再遍历 pixels。
     */
}

static void histogram_atomic(const std::vector<uint8_t> &pixels, std::vector<uint64_t> &hist,
                             int threads) {
    (void)pixels;
    (void)hist;
    (void)threads;
    /* TODO(atomic)  Version A：所有线程更新同一份 hist
     * 1. 清零 hist。清零完成之前不要开始自增。
     * 2. 用 parallel for 按像素下标划分循环，线程数用 threads。
     * 3. hist[pixels[i]]++ 必须放在 atomic 里。多个线程会写同一个 bin。
     * 不要用 critical 把整个循环包起来。
     */
}

static void histogram_private(const std::vector<uint8_t> &pixels, std::vector<uint64_t> &hist,
                              int threads) {
    (void)pixels;
    (void)hist;
    (void)threads;
    /* TODO(private)  Version B：线程私有直方图
     * 1. 每个线程自己准备 256 个计数，并清零。
     *    可以在并行区域内用 std::array<uint64_t, 256>，也可以事先分配 threads 行。
     * 2. 线程只把分到的像素累加进自己的局部直方图。
     *    这一步不要写全局 hist。
     * 3. 输入处理完之后，再把各线程的局部直方图加进 hist。
     * 局部直方图的分配、清零和最后的合并都要留在这个函数里，因为它们计入时间。
     */
}

static void serial_adapter(const std::vector<uint8_t> &pixels, std::vector<uint64_t> &hist,
                           int threads) {
    (void)threads;
    histogram_serial(pixels, hist);
}

static void run_timed(HistFn fn, const std::vector<uint8_t> &pixels, int threads, int reps,
                      HistRun &out) {
    out.reps = reps;
    out.hist.assign(HIST_BINS, 0);
    for (int r = 0; r < reps; r++) {
        std::fill(out.hist.begin(), out.hist.end(), 0);
        double t0 = omp_get_wtime();
        fn(pixels, out.hist, threads);
        out.samples[r] = omp_get_wtime() - t0;
    }
    out.median_s = median(out.samples, reps);
}

static void print_times(const char *name, const HistRun &run) {
    std::cout.setf(std::ios::fixed);
    std::cout.precision(6);
    std::cout << name << "_times_s=";
    for (int r = 0; r < run.reps; r++) {
        std::cout << (r ? " " : "") << run.samples[r];
    }
    std::cout << '\n' << name << "_time_median_s=" << run.median_s << '\n';
    std::cout.unsetf(std::ios::fixed);
}

static void print_speedup(const char *name, double t_serial, double t_parallel) {
    std::cout << "speedup_" << name << '=';
    if (t_serial > 0.0 && t_parallel > 0.0) {
        std::cout.setf(std::ios::fixed);
        std::cout.precision(4);
        std::cout << (t_serial / t_parallel) << '\n';
        std::cout.unsetf(std::ios::fixed);
    } else {
        std::cout << "na\n";
    }
}

static void report_match(const char *name, const HistRun &got, const std::vector<uint64_t> &ref,
                         size_t n, int &ok) {
    int bin = first_mismatch(got.hist, ref);
    bool match = bin < 0;
    std::cout << "match_" << name << '=' << (match ? 1 : 0) << '\n';
    if (!match) {
        ok = 0;
        std::cout << "mismatch_" << name << "_bin=" << bin
                  << " serial=" << ref[bin] << " got=" << got.hist[bin] << '\n';
        if (hist_all_zero(got.hist)) {
            std::cout << "hint_" << name << "=直方图仍全是 0，对应的 TODO 还没有写出累加\n";
        }
    } else if (hist_sum(ref) != n) {
        std::cout << "hint_" << name << "=和串行版相同，但串行结果还不正确，请先写 TODO(serial)\n";
    }
}

static int run_case(const std::string &mode, const std::vector<uint8_t> &pixels, int threads,
                    const std::string &dist, uint32_t seed, int reps,
                    const std::vector<uint64_t> *expect) {
    const bool want_serial = mode == "serial" || mode == "check";
    const bool want_atomic = mode == "atomic" || mode == "check";
    const bool want_private = mode == "private" || mode == "check";
    const bool compare_atomic = mode == "atomic" || mode == "check";
    const bool compare_private = mode == "private" || mode == "check";
    const size_t n = pixels.size();

    std::cout << "problem=1\n"
              << "mode=" << mode << '\n'
              << "N=" << n << '\n'
              << "threads=" << threads << '\n'
              << "N_mod_threads=" << (n % static_cast<size_t>(threads)) << '\n'
              << "dist=" << dist << '\n'
              << "seed=" << seed << '\n'
              << "reps=" << reps << '\n'
              << "omp_procs=" << omp_get_num_procs() << '\n';

    omp_set_dynamic(0);
    omp_set_num_threads(threads);

    HistRun serial_run;
    HistRun atomic_run;
    HistRun private_run;

    if (want_serial || compare_atomic || compare_private || expect != nullptr) {
        run_timed(serial_adapter, pixels, threads, want_serial ? reps : 1, serial_run);
        if (want_serial) {
            print_times("serial", serial_run);
        }
    }
    if (want_atomic) {
        run_timed(histogram_atomic, pixels, threads, reps, atomic_run);
        print_times("atomic", atomic_run);
    }
    if (want_private) {
        run_timed(histogram_private, pixels, threads, reps, private_run);
        print_times("private", private_run);
    }

    int ok = 1;
    if (serial_run.hist.empty()) {
        serial_run.hist.assign(HIST_BINS, 0);
    }
    const uint64_t serial_sum = hist_sum(serial_run.hist);
    std::cout << "serial_sum=" << serial_sum << '\n';
    std::cout << "sum_ok=" << (serial_sum == n ? 1 : 0) << '\n';
    if (serial_sum != n) {
        ok = 0;
        if (hist_all_zero(serial_run.hist)) {
            std::cout << "hint_serial=串行直方图仍全是 0，请先完成 TODO(serial)\n";
        }
    }

    if (expect != nullptr && !hist_equal(serial_run.hist, *expect)) {
        ok = 0;
        const int bin = first_mismatch(serial_run.hist, *expect);
        std::cout << "example_serial_match=0\n";
        std::cout << "example_mismatch_bin=" << bin << " expect=" << (*expect)[bin]
                  << " got=" << serial_run.hist[bin] << '\n';
    } else if (expect != nullptr) {
        std::cout << "example_serial_match=1\n";
    }

    if (compare_atomic) {
        report_match("atomic", atomic_run, serial_run.hist, n, ok);
        const uint64_t sum = hist_sum(atomic_run.hist);
        std::cout << "atomic_sum=" << sum << '\n';
        std::cout << "atomic_sum_ok=" << (sum == n ? 1 : 0) << '\n';
        if (sum != n) {
            ok = 0;
        }
    }
    if (compare_private) {
        report_match("private", private_run, serial_run.hist, n, ok);
        const uint64_t sum = hist_sum(private_run.hist);
        std::cout << "private_sum=" << sum << '\n';
        std::cout << "private_sum_ok=" << (sum == n ? 1 : 0) << '\n';
        if (sum != n) {
            ok = 0;
        }
    }

    if (mode == "check") {
        print_speedup("atomic", serial_run.median_s, atomic_run.median_s);
        print_speedup("private", serial_run.median_s, private_run.median_s);
        std::cout << "speedup_note=分母是串行实现的中位时间，不是并行版开 1 个线程的时间\n";
    }

    std::cout << "result=" << (ok ? "ok" : "fail") << '\n';
    return ok ? 0 : 1;
}

static int run_example(int threads) {
    int status = 0;

    const std::vector<uint8_t> sample = {0, 1, 1, 2, 2, 2, 255, 255};
    std::vector<uint64_t> expect(HIST_BINS, 0);
    expect[0] = 1;
    expect[1] = 2;
    expect[2] = 3;
    expect[255] = 2;

    std::cout << "===== example: 作业给出的 8 个像素，threads=" << threads << " =====\n";
    status |= run_case("check", sample, threads, "given", 0, 1, &expect);

    const std::vector<uint8_t> one = {42};
    std::vector<uint64_t> expect_one(HIST_BINS, 0);
    expect_one[42] = 1;

    std::cout << "\n===== example: 长度为 1 =====\n";
    status |= run_case("check", one, threads, "given", 0, 1, &expect_one);
    return status == 0 ? 0 : 1;
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

    if (argc < 5 || argc > 7) {
        usage();
    }

    const std::string mode = argv[1];
    if (mode != "serial" && mode != "atomic" && mode != "private" && mode != "check") {
        usage();
    }

    const auto n = static_cast<size_t>(parse_long(argv[2], "N", 1, 2000000000L));
    const int threads = static_cast<int>(parse_long(argv[3], "threads", 1, MAX_THREADS));
    const std::string dist = argv[4];
    uint32_t seed = 1;
    int reps = 3;
    if (argc >= 6) {
        seed = parse_u32(argv[5]);
    }
    if (argc >= 7) {
        reps = static_cast<int>(parse_long(argv[6], "reps", 1, MAX_REPS));
    }
    if (dist != "uniform" && dist != "constant") {
        die("分布只能是 uniform 或 constant");
    }

    std::vector<uint8_t> pixels(n);
    generate_pixels(pixels, dist, seed);
    return run_case(mode, pixels, threads, dist, seed, reps, nullptr);
}
