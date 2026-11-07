#include <iostream>
#include <vector>
#include <chrono>
#include <iomanip>
#include <atomic>
#include <cstdint>
#include <string>
#include <tbb/tbb.h>

struct Subproblem {
    std::vector<int8_t> board;
    int next_k;
};

// 1. Однопроходный базовый рекурсивный поиск (1 цикл)
uint64_t solve_seq(std::vector<int8_t>& board, int k, int n) {
    if (k == 0) return 1;

    uint64_t count = 0;
    const int len = 2 * n;
    const int max_i = (k == n) ? (n - 2) / 2 : (len - k - 2);

    for (int i = 0; i <= max_i; ++i) {
        const int j = i + k + 1;
        if (board[i] == 0 && board[j] == 0) {
            board[i] = board[j] = static_cast<int8_t>(k);
            count += solve_seq(board, k - 1, n);
            board[i] = board[j] = 0;
        }
    }
    return count;
}

// 2. Вариант 1: Task Group с Cut-Off
void solve_task_group(std::vector<int8_t> board, int k, int n, int cutoff_k,
                      tbb::task_group& tg, std::atomic<uint64_t>& counter) {
    if (k <= cutoff_k) {
        uint64_t local = solve_seq(board, k, n);
        counter.fetch_add(local, std::memory_order_relaxed);
        return;
    }

    const int len = 2 * n;
    const int max_i = (k == n) ? (n - 2) / 2 : (len - k - 2);

    for (int i = 0; i <= max_i; ++i) {
        const int j = i + k + 1;
        if (board[i] == 0 && board[j] == 0) {
            auto next_board = board;
            next_board[i] = next_board[j] = static_cast<int8_t>(k);
            tg.run([next_board, k, n, cutoff_k, &tg, &counter]() {
                solve_task_group(next_board, k - 1, n, cutoff_k, tg, counter);
            });
        }
    }
}

// 3. Вариант 2: Предварительное разрешение рекурсии
void expand_recursion(std::vector<int8_t>& board, int k, int n, int target_k, std::vector<Subproblem>& tasks) {
    if (k == target_k) {
        tasks.push_back({board, k});
        return;
    }

    const int len = 2 * n;
    const int max_i = (k == n) ? (n - 2) / 2 : (len - k - 2);

    for (int i = 0; i <= max_i; ++i) {
        const int j = i + k + 1;
        if (board[i] == 0 && board[j] == 0) {
            board[i] = board[j] = static_cast<int8_t>(k);
            expand_recursion(board, k - 1, n, target_k, tasks);
            board[i] = board[j] = 0;
        }
    }
}

uint64_t solve_parallel_reduce(const std::vector<Subproblem>& tasks, int n) {
    return tbb::parallel_reduce(
        tbb::blocked_range<size_t>(0, tasks.size()),
        uint64_t(0),
        [&](const tbb::blocked_range<size_t>& r, uint64_t init) -> uint64_t {
            for (size_t idx = r.begin(); idx < r.end(); ++idx) {
                auto local_board = tasks[idx].board;
                init += solve_seq(local_board, tasks[idx].next_k, n);
            }
            return init;
        },
        std::plus<uint64_t>()
    );
}

int main(int argc, char* argv[]) {
    int n = 12;
    if (argc > 1) {
        n = std::stoi(argv[1]);
    }

    const unsigned int p = tbb::info::default_concurrency();

    std::cout << "======================================================================\n";
    std::cout << "PVA Aufgabe 4: Langford-Sequenzen (Vergleichs-Benchmark)\n";
    std::cout << "Parameter n = " << n << " | Threads = " << p << "\n";
    std::cout << "======================================================================\n";

    if (n % 4 != 0 && n % 4 != 3) {
        std::cerr << "Fuer n = " << n << " existieren mathematisch keine Loesungen!\n";
        return 1;
    }

    // --- 1. Sequentiell Baseline ---
    std::vector<int8_t> seq_board(2 * n, 0);
    auto t1 = std::chrono::high_resolution_clock::now();
    uint64_t seq_res = solve_seq(seq_board, n, n);
    auto t2 = std::chrono::high_resolution_clock::now();
    double t_seq = std::chrono::duration<double, std::milli>(t2 - t1).count();

    // --- 2. Variante 1: task_group ---
    std::vector<int8_t> tg_board(2 * n, 0);
    std::atomic<uint64_t> tg_res{0};
    tbb::task_group tg;
    const int cutoff_k = n - 3;

    auto t3 = std::chrono::high_resolution_clock::now();
    solve_task_group(tg_board, n, n, cutoff_k, tg, tg_res);
    tg.wait();
    auto t4 = std::chrono::high_resolution_clock::now();
    double t_v1 = std::chrono::duration<double, std::milli>(t4 - t3).count();

    // --- 3. Variante 2: Rekursionsaufloesung + parallel_reduce ---
    std::vector<int8_t> exp_board(2 * n, 0);
    std::vector<Subproblem> subproblems;

    auto t5 = std::chrono::high_resolution_clock::now();
    expand_recursion(exp_board, n, n, cutoff_k, subproblems);
    uint64_t red_res = solve_parallel_reduce(subproblems, n);
    auto t6 = std::chrono::high_resolution_clock::now();
    double t_v2 = std::chrono::duration<double, std::milli>(t6 - t5).count();

    // Вывод форматированной таблицы результатов
    std::cout << std::left << std::setw(32) << "Verfahren"
              << std::setw(16) << "Loesungen"
              << std::setw(14) << "Zeit (ms)"
              << std::setw(12) << "Speedup (S)"
              << std::setw(12) << "Effizienz (E)" << "\n";
    std::cout << std::string(86, '-') << "\n";

    auto print_row = [&](const std::string& name, uint64_t res, double time_ms) {
        double speedup = t_seq / time_ms;
        double eff = (speedup / p) * 100.0;
        std::cout << std::left << std::setw(32) << name
                  << std::setw(16) << res
                  << std::fixed << std::setprecision(2)
                  << std::setw(14) << time_ms
                  << std::setw(12) << speedup
                  << std::setprecision(1)
                  << std::setw(11) << eff << "%\n";
    };

    print_row("1. Sequentiell (Baseline)", seq_res, t_seq);
    print_row("2. TBB task_group (V1)", tg_res.load(), t_v1);
    print_row("3. TBB parallel_reduce (V2)", red_res, t_v2);

    std::cout << std::string(86, '-') << "\n";
    bool valid = (seq_res == tg_res.load()) && (seq_res == red_res);
    std::cout << "Status: " << (valid ? "ALLE ERGEBNISSE KORREKT (VALIDIERT)" : "FEHLER IN BERECHNUNG") << "\n";

    return 0;
}
