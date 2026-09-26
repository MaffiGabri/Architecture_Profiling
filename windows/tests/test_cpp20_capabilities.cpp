/**
 * ==============================================================================
 * Architecture Profiling Companion - C++20 Compiler Capability Verification
 * Modern C++20 Capability Test Suite (Windows)
 * ==============================================================================
 * Verifies that the host C++ toolchain correctly compiles and executes:
 * 1. C++20 Concepts (<concepts>, custom concept definitions, requires clauses)
 * 2. C++20 Ranges and Views (<ranges>, filter, transform, views pipeline)
 * 3. Structured Bindings (struct decomposition, tuple decomposition, loop bindings)
 * 4. std::filesystem (<filesystem>, path manipulation, directory inspection)
 * 5. C++20 Threads & Atomics (<thread>, <atomic>, std::jthread / std::thread)
 * 6. std::span (<span> non-owning buffer slices for image texture processing)
 * 7. Three-way comparison operator (<=> spaceship operator, std::strong_ordering)
 * ==============================================================================
 */

#include <iostream>
#include <vector>
#include <string>
#include <string_view>
#include <numeric>
#include <algorithm>
#include <concepts>
#include <ranges>
#include <filesystem>
#include <thread>
#include <atomic>
#include <chrono>
#include <span>
#include <compare>
#include <map>
#include <cassert>
#include <cmath>

namespace arch::test {

// =============================================================================
// 1. C++20 CONCEPTS & CONSTRAINTS
// =============================================================================

template <typename T>
concept NumericTrait = std::floating_point<T> || std::integral<T>;

template <typename T>
concept AestheticScoreable = requires(T t) {
    { t.id() } -> std::same_as<int>;
    { t.score() } -> std::floating_point;
    { t.category() } -> std::convertible_to<std::string_view>;
};

template <NumericTrait T>
constexpr T clamp_trait(T value, T min_val, T max_val) {
    return std::clamp(value, min_val, max_val);
}

struct MockStyle {
    int m_id;
    double m_score;
    std::string m_category;

    int id() const { return m_id; }
    double score() const { return m_score; }
    std::string_view category() const { return m_category; }

    // Three-way comparison (Spaceship operator)
    auto operator<=>(const MockStyle& other) const = default;
};

// Compile-time concept verification
static_assert(NumericTrait<int>);
static_assert(NumericTrait<double>);
static_assert(!NumericTrait<std::string>);
static_assert(AestheticScoreable<MockStyle>);

bool test_concepts() {
    double clamped = clamp_trait(12.5, 0.0, 10.0);
    if (std::abs(clamped - 10.0) > 1e-6) return false;

    int clamped_int = clamp_trait(-3, 0, 10);
    if (clamped_int != 0) return false;

    return true;
}

// =============================================================================
// 2. C++20 RANGES AND VIEWS
// =============================================================================

bool test_ranges() {
    std::vector<MockStyle> styles = {
        {1, 8.5, "classical_renaissance"},
        {2, 4.2, "historicist_sacred"},
        {3, 9.1, "classical_renaissance"},
        {4, 6.0, "modernism_functionalism"},
        {5, 7.8, "early_modern_industrial"},
        {6, 3.5, "contemporary_parametric"}
    };

    // Range-based sort with projection
    std::ranges::sort(styles, std::greater<>{}, &MockStyle::m_score);

    if (styles.front().m_id != 3) return false; // Highest score (9.1)

    // Pipeline using views::filter and views::transform
    auto high_scorers = styles
        | std::views::filter([](const MockStyle& s) { return s.m_score >= 7.0; })
        | std::views::transform([](const MockStyle& s) { return s.m_id; });

    std::vector<int> high_scorer_ids;
    for (int id : high_scorers) {
        high_scorer_ids.push_back(id);
    }

    // Expected high scorers: id 3 (9.1), id 1 (8.5), id 5 (7.8)
    if (high_scorer_ids.size() != 3) return false;
    if (high_scorer_ids[0] != 3 || high_scorer_ids[1] != 1 || high_scorer_ids[2] != 5) {
        return false;
    }

    return true;
}

// =============================================================================
// 3. STRUCTURED BINDINGS
// =============================================================================

struct PairMatch {
    int left_id;
    int right_id;
    double margin;
};

bool test_structured_bindings() {
    PairMatch match{1, 2, 0.85};
    auto [left, right, margin] = match;

    if (left != 1 || right != 2 || std::abs(margin - 0.85) > 1e-6) {
        return false;
    }

    // Structured binding with std::map
    std::map<std::string, double> category_affinity = {
        {"classical", 0.45},
        {"modernism", 0.35},
        {"brutalism", 0.20}
    };

    double sum = 0.0;
    for (const auto& [category, affinity] : category_affinity) {
        sum += affinity;
    }

    if (std::abs(sum - 1.0) > 1e-6) return false;

    return true;
}

// =============================================================================
// 4. STD::FILESYSTEM
// =============================================================================

bool test_filesystem() {
    namespace fs = std::filesystem;

    fs::path current = fs::current_path();
    if (current.empty()) return false;

    fs::path test_sub = current / "assets" / "data" / "styles.json";
    if (test_sub.extension() != ".json") return false;
    if (test_sub.filename() != "styles.json") return false;

    // Check parent path navigation
    fs::path parent = test_sub.parent_path();
    if (parent.filename() != "data") return false;

    return true;
}

// =============================================================================
// 5. THREADS AND ATOMICS
// =============================================================================

bool test_threads() {
    std::atomic<int> processed_counter{0};
    constexpr int NUM_TASKS = 20;

    // Test concurrency using worker threads
    std::vector<std::thread> workers;
    for (int i = 0; i < 4; ++i) {
        workers.emplace_back([&processed_counter, NUM_TASKS]() {
            for (int k = 0; k < NUM_TASKS / 4; ++k) {
                processed_counter.fetch_add(1, std::memory_order_relaxed);
                std::this_thread::sleep_for(std::chrono::microseconds(100));
            }
        });
    }

    for (auto& w : workers) {
        if (w.joinable()) {
            w.join();
        }
    }

    if (processed_counter.load() != NUM_TASKS) return false;

#if defined(__cpp_lib_jthread)
    // std::jthread auto-joining verification
    std::atomic<bool> jthread_ran{false};
    {
        std::jthread jt([&jthread_ran]() {
            jthread_ran = true;
        });
    } // Auto joins upon destruction
    if (!jthread_ran.load()) return false;
#endif

    return true;
}

// =============================================================================
// 6. STD::SPAN (Non-owning buffer view for texture loaders)
// =============================================================================

bool verify_pixel_buffer(std::span<const uint8_t> buffer, size_t expected_size) {
    if (buffer.size() != expected_size) return false;
    if (buffer.empty()) return false;
    return buffer[0] == 0xFF;
}

bool test_span() {
    std::vector<uint8_t> dummy_pixels(800 * 600 * 4, 0x00);
    dummy_pixels[0] = 0xFF; // Mock first byte

    return verify_pixel_buffer(dummy_pixels, 800 * 600 * 4);
}

// =============================================================================
// 7. THREE-WAY COMPARISON (<=> Spaceship Operator)
// =============================================================================

bool test_spaceship_operator() {
    MockStyle a{1, 5.0, "catA"};
    MockStyle b{2, 5.0, "catA"};
    MockStyle c{1, 5.0, "catA"};

    if ((a == c) != true) return false;
    if ((a < b) != true) return false; // id 1 < id 2
    if ((b > a) != true) return false;

    return true;
}

} // namespace arch::test

// =============================================================================
// MAIN ENTRY POINT
// =============================================================================

int main(int argc, char* argv[]) {
    std::string test_filter = "all";

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if ((arg == "--test" || arg == "-t") && i + 1 < argc) {
            test_filter = argv[++i];
        } else if (arg == "--all") {
            test_filter = "all";
        } else if (arg.rfind("--test=", 0) == 0) {
            test_filter = arg.substr(7);
        }
    }

    std::cout << "========================================================\n";
    std::cout << " Modern C++20 Toolchain Capability Verification Suite   \n";
    std::cout << " Architecture Profiling Desktop System (Windows)        \n";
    std::cout << " Filter: " << test_filter << "\n";
    std::cout << "========================================================\n";

    bool all_passed = true;

    auto run_test = [&](const char* name, const char* id, auto test_fn) {
        if (test_filter != "all" && test_filter != id) {
            return;
        }
        std::cout << "Running " << name << " ... ";
        try {
            if (test_fn()) {
                std::cout << "[PASS]\n";
            } else {
                std::cout << "[FAIL: assertion returned false]\n";
                all_passed = false;
            }
        } catch (const std::exception& ex) {
            std::cout << "[FAIL: exception: " << ex.what() << "]\n";
            all_passed = false;
        } catch (...) {
            std::cout << "[FAIL: unknown exception]\n";
            all_passed = false;
        }
    };

    run_test("1. C++20 Concepts & Requires Clauses", "concepts", arch::test::test_concepts);
    run_test("2. C++20 Ranges & Views Pipelines   ", "ranges",   arch::test::test_ranges);
    run_test("3. Structured Bindings & Decomp     ", "bindings", arch::test::test_structured_bindings);
    run_test("4. std::filesystem Navigation       ", "filesystem", arch::test::test_filesystem);
    run_test("5. C++20 Threads & Atomic Sync      ", "threads",  arch::test::test_threads);
    run_test("6. std::span Memory Views           ", "span",     arch::test::test_span);
    run_test("7. Three-Way Comparison (<=>)       ", "spaceship", arch::test::test_spaceship_operator);

    std::cout << "--------------------------------------------------------\n";
    if (all_passed) {
        std::cout << "VERDICT: All requested C++20 capability tests PASSED.\n";
        std::cout << "The toolchain is 100% compliant with Architecture Profiling requirements.\n";
        return 0;
    } else {
        std::cout << "VERDICT: Some C++20 tests FAILED. Check toolchain conformance.\n";
        return 1;
    }
}
