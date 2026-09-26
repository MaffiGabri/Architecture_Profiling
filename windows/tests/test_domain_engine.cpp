#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <numeric>
#include <algorithm>
#include <filesystem>
#include <iomanip>

#include "architecture/models.hpp"
#include "architecture/preference_engine.hpp"
#include "architecture/tournament.hpp"
#include "architecture/archetypes.hpp"
#include "architecture/json_loader.hpp"

#ifndef ARCH_PROJECT_ROOT_DIR
#define ARCH_PROJECT_ROOT_DIR "../.."
#endif

namespace fs = std::filesystem;
using namespace arch::domain;

static bool approx_equal(double a, double b, double eps = 1e-4) {
    return std::abs(a - b) <= eps;
}

// ==============================================================================
// Suite 1: Pairing Schedule Invariants
// ==============================================================================
bool test_pairing_invariants() {
    auto full_schedule = PairingScheduler::generate_schedule(TournamentMode::Full);
    if (full_schedule.size() != 45) {
        std::cerr << "Full schedule size is " << full_schedule.size() << ", expected 45\n";
        return false;
    }

    // Verify all 45 distinct pairs
    std::vector<std::pair<int, int>> seen;
    for (const auto& m : full_schedule) {
        if (m.left_style_id < 1 || m.left_style_id > 10 || m.right_style_id < 1 || m.right_style_id > 10) {
            std::cerr << "Style ID out of range [1, 10]\n";
            return false;
        }
        int u = std::min(m.left_style_id, m.right_style_id);
        int v = std::max(m.left_style_id, m.right_style_id);
        if (u == v) {
            std::cerr << "Self-pairing detected: " << u << " vs " << v << "\n";
            return false;
        }
        seen.push_back({u, v});
    }
    std::sort(seen.begin(), seen.end());
    seen.erase(std::unique(seen.begin(), seen.end()), seen.end());
    if (seen.size() != 45) {
        std::cerr << "Unique pairs count is " << seen.size() << ", expected 45\n";
        return false;
    }

    // Verify minimum spacing >= 4 (at least 3 intervening matches between appearances)
    for (int id = 1; id <= 10; ++id) {
        int last_idx = -100;
        for (size_t i = 0; i < full_schedule.size(); ++i) {
            if (full_schedule[i].left_style_id == id || full_schedule[i].right_style_id == id) {
                if (last_idx >= 0 && (static_cast<int>(i) - last_idx) < 4) {
                    std::cerr << "Spacing violation for style " << id << ": match "
                              << last_idx << " and match " << i << " (distance "
                              << (static_cast<int>(i) - last_idx) << " < 4)\n";
                    return false;
                }
                last_idx = static_cast<int>(i);
            }
        }
    }

    // Verify left/right spatial balance (each style appears 4-5 times on left)
    std::vector<int> left_counts(11, 0);
    for (const auto& m : full_schedule) {
        left_counts[m.left_style_id]++;
    }
    for (int id = 1; id <= 10; ++id) {
        if (left_counts[id] < 4 || left_counts[id] > 5) {
            std::cerr << "Spatial balance violation for style " << id << ": left count = "
                      << left_counts[id] << " (expected 4 or 5)\n";
            return false;
        }
    }

    // Quick tournament schedule (15 matches = 3 rounds)
    auto quick_schedule = PairingScheduler::generate_schedule(TournamentMode::Quick);
    if (quick_schedule.size() != 15) {
        std::cerr << "Quick schedule size is " << quick_schedule.size() << ", expected 15\n";
        return false;
    }
    std::vector<int> quick_counts(11, 0);
    for (const auto& m : quick_schedule) {
        quick_counts[m.left_style_id]++;
        quick_counts[m.right_style_id]++;
    }
    for (int id = 1; id <= 10; ++id) {
        if (quick_counts[id] != 3) {
            std::cerr << "Quick tournament 3-regular violation for style " << id
                      << ": appearance count = " << quick_counts[id] << " != 3\n";
            return false;
        }
    }

    return true;
}

// ==============================================================================
// Suite 2: State Machine & Undo/Redo
// ==============================================================================
bool test_state_machine_undo_redo() {
    TournamentStateMachine sm(TournamentMode::Full);
    if (sm.total_matches() != 45 || sm.current_index() != 0) return false;
    if (sm.can_undo() || sm.can_redo()) return false;
    if (!sm.can_vote()) return false;
    if (sm.progress_percentage() != 0.0) return false;

    // Vote 10 matches
    for (int i = 0; i < 10; ++i) {
        const auto& m = sm.current_match();
        sm.vote(m.left_style_id);
    }
    if (sm.current_index() != 10) return false;
    if (!sm.can_undo()) return false;
    if (sm.can_redo()) return false;

    // Undo 5 matches
    for (int i = 0; i < 5; ++i) {
        if (!sm.can_undo()) return false;
        sm.undo();
    }
    if (sm.current_index() != 5) return false;
    if (!sm.can_redo()) return false;

    // Redo 2 matches
    sm.redo();
    sm.redo();
    if (sm.current_index() != 7) return false;

    // Branching vote clears redo stack
    const auto& m7 = sm.current_match();
    sm.vote(m7.right_style_id);
    if (sm.can_redo()) return false; // Redo stack must be cleared after branching
    if (sm.current_index() != 8) return false;

    // Complete tournament
    while (sm.can_vote()) {
        const auto& cur = sm.current_match();
        sm.vote(cur.left_style_id);
    }
    if (!sm.is_complete()) return false;
    if (sm.can_vote()) return false;
    if (!approx_equal(sm.progress_percentage(), 100.0, 1e-4)) return false;

    // Safety checks: voting when complete does nothing
    sm.vote(1);

    // Boundary check: invalid winner ID throws std::invalid_argument
    TournamentStateMachine sm_invalid(TournamentMode::Quick);
    bool threw_expected = false;
    try {
        sm_invalid.vote(999);
    } catch (const std::invalid_argument&) {
        threw_expected = true;
    }
    if (!threw_expected) {
        std::cerr << "Expected vote(999) to throw std::invalid_argument\n";
        return false;
    }

    return true;
}

// ==============================================================================
// Suite 3: Bradley-Terry MLE & Laplace Fallback
// ==============================================================================
bool test_bradley_terry() {
    // Case A: Dominance vector (1 > 2 > ... > 10) -> Undefeated and winless items exist
    std::vector<std::vector<int>> dom_matrix(10, std::vector<int>(10, 0));
    for (int i = 0; i < 10; ++i) {
        for (int j = i + 1; j < 10; ++j) {
            dom_matrix[i][j] = 1;
        }
    }

    auto bt_dom = PreferenceEngine::solve_bradley_terry(dom_matrix);
    if (bt_dom.algorithm != "LaplaceFallback") {
        std::cerr << "Expected LaplaceFallback for dominance matrix, got " << bt_dom.algorithm << "\n";
        return false;
    }
    if (bt_dom.iterations != 0) return false;

    // Probabilities must be strictly monotonic
    for (size_t i = 0; i < 9; ++i) {
        if (bt_dom.probabilities[i] <= bt_dom.probabilities[i + 1]) {
            std::cerr << "Probabilities not strictly monotonic: p[" << i << "] = "
                      << bt_dom.probabilities[i] << " <= p[" << (i + 1) << "] = "
                      << bt_dom.probabilities[i + 1] << "\n";
            return false;
        }
    }
    double sum = std::accumulate(bt_dom.probabilities.begin(), bt_dom.probabilities.end(), 0.0);
    if (!approx_equal(sum, 1.0, 1e-6)) return false;

    // Case B: Cyclic connected graph (TC2) -> No undefeated/winless items -> BradleyTerryMM
    std::vector<std::vector<int>> cycle_matrix(10, std::vector<int>(10, 0));
    for (int i = 0; i < 10; ++i) {
        for (int step = 1; step <= ((i < 5) ? 5 : 4); ++step) {
            cycle_matrix[i][(i + step) % 10] = 1;
        }
    }
    auto bt_cycle = PreferenceEngine::solve_bradley_terry(cycle_matrix);
    if (bt_cycle.algorithm != "BradleyTerryMM") {
        std::cerr << "Expected BradleyTerryMM for cyclic matrix, got " << bt_cycle.algorithm << "\n";
        return false;
    }
    if (bt_cycle.iterations <= 0 || bt_cycle.iterations > 50) return false;
    double sum_cycle = std::accumulate(bt_cycle.probabilities.begin(), bt_cycle.probabilities.end(), 0.0);
    if (!approx_equal(sum_cycle, 1.0, 1e-6)) return false;

    // Boundary check: invalid matrix dimension (not 10x10) throws std::invalid_argument
    std::vector<std::vector<int>> bad_matrix(4, std::vector<int>(4, 0));
    bool dim_threw_expected = false;
    try {
        PreferenceEngine::calculate_bradley_terry(bad_matrix);
    } catch (const std::invalid_argument&) {
        dim_threw_expected = true;
    }
    if (!dim_threw_expected) {
        std::cerr << "Expected 4x4 matrix to throw std::invalid_argument in calculate_bradley_terry\n";
        return false;
    }

    return true;
}

// ==============================================================================
// Suite 4: Kendall Circular Triads & Consistence Zeta
// ==============================================================================
bool test_kendall_triads() {
    // Transitive matrix: c = 0, zeta = 1.0
    std::vector<std::vector<int>> trans_matrix(10, std::vector<int>(10, 0));
    for (int i = 0; i < 10; ++i) {
        for (int j = i + 1; j < 10; ++j) {
            trans_matrix[i][j] = 1;
        }
    }
    double c_trans = PreferenceEngine::calculate_circular_triads(trans_matrix, 45);
    double z_trans = PreferenceEngine::calculate_zeta(c_trans);
    if (!approx_equal(c_trans, 0.0, 1e-6) || !approx_equal(z_trans, 1.0, 1e-6)) {
        std::cerr << "Transitive triads expected (0, 1.0), got (" << c_trans << ", " << z_trans << ")\n";
        return false;
    }

    // Cyclic matrix: 5 with 5 wins, 5 with 4 wins -> sum(s^2) = 205 -> c = 142.5 - 102.5 = 40.0, zeta = 0.0
    std::vector<std::vector<int>> cycle_matrix(10, std::vector<int>(10, 0));
    for (int i = 0; i < 10; ++i) {
        for (int step = 1; step <= ((i < 5) ? 5 : 4); ++step) {
            cycle_matrix[i][(i + step) % 10] = 1;
        }
    }
    double c_cycle = PreferenceEngine::calculate_circular_triads(cycle_matrix, 45);
    double z_cycle = PreferenceEngine::calculate_zeta(c_cycle);
    if (!approx_equal(c_cycle, 40.0, 1e-6) || !approx_equal(z_cycle, 0.0, 1e-6)) {
        std::cerr << "Cyclic triads expected (40, 0.0), got (" << c_cycle << ", " << z_cycle << ")\n";
        return false;
    }

    return true;
}

// ==============================================================================
// Suite 5: Multi-Objective Confidence Bounds
// ==============================================================================
bool test_confidence() {
    // Clamped minimum (noise/contradiction): 15.0%
    double c_min = PreferenceEngine::calculate_confidence(0.0, 0.0, 0.0);
    if (!approx_equal(c_min, 15.0, 1e-6)) {
        std::cerr << "Confidence min clamp expected 15.0, got " << c_min << "\n";
        return false;
    }

    // Clamped maximum: 99.0%
    double c_max = PreferenceEngine::calculate_confidence(1.0, 1.0, 1.0);
    if (!approx_equal(c_max, 99.0, 1e-6)) {
        std::cerr << "Confidence max clamp expected 99.0, got " << c_max << "\n";
        return false;
    }

    // Typical transitive dominance case:
    // zeta = 1.0, delta = 0.018182, entropy_conc = 0.06571 -> ~48.2214%
    double c_mid = PreferenceEngine::calculate_confidence(1.0, 0.018182, 0.06571);
    if (!approx_equal(c_mid, 48.2214, 0.01)) {
        std::cerr << "Confidence mid expected ~48.22, got " << c_mid << "\n";
        return false;
    }

    return true;
}

// ==============================================================================
// Suite 6: 5D Trait Projection & 8 Archetypes Classification
// ==============================================================================
bool test_archetypes() {
    const auto& catalog = ArchetypeEngine::all_archetypes();
    if (catalog.size() != 8) {
        std::cerr << "Expected 8 archetypes, found " << catalog.size() << "\n";
        return false;
    }

    // Rule 1: Contradictory votes (zeta < 0.50) -> The Eclectic Synthesizer
    auto a_inconsistent = ArchetypeEngine::classify("classical_renaissance", 1, TraitRadar{2.0, 6.0, 6.0, 1.0, 8.0}, 0.5, 0.40);
    if (a_inconsistent.id != "eclectic_synthesizer") {
        std::cerr << "Expected eclectic_synthesizer for zeta < 0.50, got " << a_inconsistent.id << "\n";
        return false;
    }

    // Rule 2: Classical Renaissance
    auto a_classic = ArchetypeEngine::classify("classical_renaissance", 1, TraitRadar{2.0, 6.0, 6.0, 1.0, 8.0}, 0.2, 1.0);
    if (a_classic.id != "classical_monumentalist") return false;

    // Rule 3: Historicist Sacred
    auto a_romantic = ArchetypeEngine::classify("historicist_sacred", 2, TraitRadar{3.0, 9.0, 5.0, 3.0, 7.0}, 0.2, 1.0);
    if (a_romantic.id != "romantic_historian") return false;

    // Rule 4: Modernism - Brutalist vs Purist
    TraitRadar brutalist_traits{7.8, 0.0, 10.0, 3.5, 4.5};
    auto a_brutalist = ArchetypeEngine::classify("modernism_functionalism", 7, brutalist_traits, 0.2, 1.0);
    if (a_brutalist.id != "brutalist_sculptor") return false;

    TraitRadar purist_traits{7.0, 0.5, 9.0, 0.5, 2.0};
    auto a_purist = ArchetypeEngine::classify("modernism_functionalism", 6, purist_traits, 0.2, 1.0);
    if (a_purist.id != "purist_rationalist") return false;

    // Rule 5: Contemporary Parametric - Deconstructivist vs Parametric
    TraitRadar decon_traits{9.2, 2.0, 6.0, 9.5, 3.0};
    auto a_decon = ArchetypeEngine::classify("contemporary_parametric", 9, decon_traits, 0.2, 1.0);
    if (a_decon.id != "avantgarde_deconstructivist") return false;

    TraitRadar param_traits{9.8, 3.0, 7.5, 9.0, 4.0};
    auto a_param = ArchetypeEngine::classify("contemporary_parametric", 10, param_traits, 0.2, 1.0);
    if (a_param.id != "parametric_visionary") return false;

    // Rule 6: Early Modern Industrial - High-Tech vs Eclectic
    TraitRadar hightech_traits{8.2, 1.5, 10.0, 4.0, 1.0};
    auto a_hightech = ArchetypeEngine::classify("early_modern_industrial", 8, hightech_traits, 0.2, 1.0);
    if (a_hightech.id != "hightech_pragmatist") return false;

    return true;
}

// ==============================================================================
// Suite 7: Shared Test Fixtures Verification (Cross-Platform Parity)
// ==============================================================================
bool test_shared_fixtures() {
    fs::path root = ARCH_PROJECT_ROOT_DIR;
    fs::path styles_path = root / "shared" / "data" / "styles.json";
    fs::path fixtures_path = root / "tests" / "test_fixtures" / "shared_tournament_fixtures.json";

    if (!fs::exists(styles_path)) {
        std::cerr << "styles.json not found at " << styles_path << "\n";
        return false;
    }
    if (!fs::exists(fixtures_path)) {
        std::cerr << "shared_tournament_fixtures.json not found at " << fixtures_path << "\n";
        return false;
    }

    auto [styles, categories] = JsonLoader::load_styles(styles_path);
    if (styles.size() != 10 || categories.size() != 5) {
        std::cerr << "Loaded " << styles.size() << " styles and " << categories.size() << " categories\n";
        return false;
    }

    auto fixtures = JsonLoader::load_fixtures(fixtures_path);
    if (fixtures.size() != 5) {
        std::cerr << "Expected 5 test fixtures, got " << fixtures.size() << "\n";
        return false;
    }

    for (const auto& fx : fixtures) {
        TournamentMode mode = (fx.mode == "quick") ? TournamentMode::Quick : TournamentMode::Full;
        TournamentStateMachine sm(mode);

        for (int winner_id : fx.winner_choices) {
            sm.vote(winner_id);
        }

        auto result = sm.calculate_result(styles, categories);

        // Algorithm check
        if (!fx.expected_algorithm.empty() && result.algorithm != fx.expected_algorithm) {
            std::cerr << "[" << fx.id << "] Algorithm mismatch: got " << result.algorithm
                      << ", expected " << fx.expected_algorithm << "\n";
            return false;
        }

        // Iteration check
        if (result.iterations != fx.expected_iterations) {
            std::cerr << "[" << fx.id << "] Iterations mismatch: got " << result.iterations
                      << ", expected " << fx.expected_iterations << "\n";
            return false;
        }

        // Champion style check
        if (result.champion_style_id != fx.expected_champion_style_id) {
            std::cerr << "[" << fx.id << "] Champion style mismatch: got " << result.champion_style_id
                      << ", expected " << fx.expected_champion_style_id << "\n";
            return false;
        }

        // Dominant category check
        if (result.dominant_category_id != fx.expected_dominant_category_id) {
            std::cerr << "[" << fx.id << "] Dominant category mismatch: got " << result.dominant_category_id
                      << ", expected " << fx.expected_dominant_category_id << "\n";
            return false;
        }

        // Circular triads check
        if (!approx_equal(result.circular_triads, fx.expected_circular_triads, 0.01)) {
            std::cerr << "[" << fx.id << "] Circular triads mismatch: got " << result.circular_triads
                      << ", expected " << fx.expected_circular_triads << "\n";
            return false;
        }

        // Zeta check
        if (!approx_equal(result.consistency_zeta, fx.expected_zeta, 1e-4)) {
            std::cerr << "[" << fx.id << "] Zeta mismatch: got " << result.consistency_zeta
                      << ", expected " << fx.expected_zeta << "\n";
            return false;
        }

        // Margin Delta check
        if (!approx_equal(result.victory_margin, fx.expected_delta, 1e-4)) {
            std::cerr << "[" << fx.id << "] Margin Delta mismatch: got " << result.victory_margin
                      << ", expected " << fx.expected_delta << "\n";
            return false;
        }

        // Confidence % check
        if (!approx_equal(result.confidence_pct, fx.expected_confidence_pct, 0.01)) {
            std::cerr << "[" << fx.id << "] Confidence % mismatch: got " << result.confidence_pct
                      << ", expected " << fx.expected_confidence_pct << "\n";
            return false;
        }

        // Archetype check
        if (!fx.expected_archetype.empty()) {
            if (result.archetype.title_en != fx.expected_archetype &&
                result.archetype.id != fx.expected_archetype) {
                std::cerr << "[" << fx.id << "] Archetype mismatch: got " << result.archetype.title_en
                          << ", expected " << fx.expected_archetype << "\n";
                return false;
            }
        }

        // Trait Radar check
        if (!approx_equal(result.user_traits.era, fx.expected_traits.era, 0.01) ||
            !approx_equal(result.user_traits.ornamentation, fx.expected_traits.ornamentation, 0.01) ||
            !approx_equal(result.user_traits.structural_honesty, fx.expected_traits.structural_honesty, 0.01) ||
            !approx_equal(result.user_traits.geometric_order, fx.expected_traits.geometric_order, 0.01) ||
            !approx_equal(result.user_traits.material_warmth, fx.expected_traits.material_warmth, 0.01)) {
            std::cerr << "[" << fx.id << "] 5D Trait radar mismatch\n";
            return false;
        }

        // Category rankings check
        if (!fx.expected_category_rankings.empty()) {
            if (result.category_rankings.size() != fx.expected_category_rankings.size()) {
                std::cerr << "[" << fx.id << "] Category rankings size mismatch\n";
                return false;
            }
            for (size_t i = 0; i < result.category_rankings.size(); ++i) {
                if (result.category_rankings[i].category_id != fx.expected_category_rankings[i].category_id ||
                    !approx_equal(result.category_rankings[i].percentage, fx.expected_category_rankings[i].percentage, 0.05)) {
                    std::cerr << "[" << fx.id << "] Category ranking mismatch at rank " << (i + 1)
                              << ": got (" << result.category_rankings[i].category_id << ", "
                              << result.category_rankings[i].percentage << "%), expected ("
                              << fx.expected_category_rankings[i].category_id << ", "
                              << fx.expected_category_rankings[i].percentage << "%)\n";
                    return false;
                }
            }
        }
    }

    return true;
}

// ==============================================================================
// Suite 8: Degenerate Tournament Evaluation (0 Matches Played)
// ==============================================================================
bool test_empty_tournament_evaluation() {
    fs::path root = ARCH_PROJECT_ROOT_DIR;
    fs::path styles_path = root / "shared" / "data" / "styles.json";
    auto [styles, categories] = JsonLoader::load_styles(styles_path);

    // Direct evaluation of 0-match win matrix
    std::vector<std::vector<int>> empty_matrix(10, std::vector<int>(10, 0));
    auto result = PreferenceEngine::evaluate(empty_matrix, styles, categories, 45);

    // 1. Style probabilities all equal to 0.10 (10.0%) and sum to 1.0
    double sum_p = 0.0;
    for (size_t i = 0; i < result.style_rankings.size(); ++i) {
        const auto& s = result.style_rankings[i];
        if (!approx_equal(s.probability, 0.10, 1e-6)) {
            std::cerr << "Style " << s.style_id << " probability is " << s.probability << ", expected 0.10\n";
            return false;
        }
        if (!approx_equal(s.percentage, 10.0, 1e-4)) {
            std::cerr << "Style " << s.style_id << " percentage is " << s.percentage << ", expected 10.0%\n";
            return false;
        }
        sum_p += s.probability;
    }
    if (!approx_equal(sum_p, 1.0, 1e-6)) {
        std::cerr << "Sum of probabilities is " << sum_p << ", expected 1.0\n";
        return false;
    }

    // 2. Entropy H = ln(10) ~ 2.302585
    double expected_h = std::log(10.0);
    if (!approx_equal(result.entropy, expected_h, 1e-5)) {
        std::cerr << "Entropy is " << result.entropy << ", expected " << expected_h << "\n";
        return false;
    }

    // 3. Entropy concentration C_H = 0.0
    if (!approx_equal(result.entropy_concentration, 0.0, 1e-6)) {
        std::cerr << "Entropy concentration is " << result.entropy_concentration << ", expected 0.0\n";
        return false;
    }

    // 4. Victory margin delta = 0.0
    if (!approx_equal(result.victory_margin, 0.0, 1e-6)) {
        std::cerr << "Victory margin delta is " << result.victory_margin << ", expected 0.0\n";
        return false;
    }

    // 5. Consistency zeta = 0.0
    if (!approx_equal(result.consistency_zeta, 0.0, 1e-6)) {
        std::cerr << "Consistency zeta is " << result.consistency_zeta << ", expected 0.0\n";
        return false;
    }

    // 6. Bounded confidence % == 15.0% (minimum clamp floor)
    if (!approx_equal(result.confidence_pct, 15.0, 1e-4)) {
        std::cerr << "Confidence % is " << result.confidence_pct << ", expected 15.0%\n";
        return false;
    }

    // 7. Matches played == 0
    if (result.matches_played != 0) {
        std::cerr << "Matches played is " << result.matches_played << ", expected 0\n";
        return false;
    }

    // 8. Category affinity balance (each category has 2 styles = 20.0%)
    for (const auto& cat : result.category_rankings) {
        if (!approx_equal(cat.percentage, 20.0, 1e-4)) {
            std::cerr << "Category " << cat.category_id << " percentage is " << cat.percentage << ", expected 20.0%\n";
            return false;
        }
    }

    // 9. Archetype classification
    if (result.archetype.id != "eclectic_synthesizer") {
        std::cerr << "Archetype is " << result.archetype.id << ", expected eclectic_synthesizer\n";
        return false;
    }

    return true;
}

// ==============================================================================
// Main Entry Point & CTest Dispatcher
// ==============================================================================
int main(int argc, char* argv[]) {
    std::string test_filter = "all";
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if ((arg == "--test" || arg == "-t") && i + 1 < argc) {
            test_filter = argv[++i];
        } else if (arg == "--all") {
            test_filter = "all";
        }
    }

    std::cout << "========================================================\n";
    std::cout << " Architecture Profiling Domain Engine Unit Test Suite    \n";
    std::cout << " Milestone M3: Modern C++20 Parity & CTest Verification \n";
    std::cout << " Filter: " << test_filter << "\n";
    std::cout << "========================================================\n";

    bool all_passed = true;
    auto run = [&](const char* name, const char* id, auto fn) {
        if (test_filter != "all" && test_filter != id) return;
        std::cout << "Running " << name << " ... " << std::flush;
        try {
            if (fn()) {
                std::cout << "[PASS]\n";
            } else {
                std::cout << "[FAIL: assertion returned false]\n";
                all_passed = false;
            }
        } catch (const std::exception& e) {
            std::cout << "[FAIL: exception " << e.what() << "]\n";
            all_passed = false;
        }
    };

    run("1. 1-Factorization & Spacing Invariants", "pairing",       test_pairing_invariants);
    run("2. Tournament State Machine Undo/Redo   ", "state_machine", test_state_machine_undo_redo);
    run("3. Bradley-Terry MLE & Laplace Fallback ", "bradley_terry", test_bradley_terry);
    run("4. Kendall Triads & Consistence Zeta    ", "triads",        test_kendall_triads);
    run("5. Multi-Objective Confidence Bounds   ", "confidence",    test_confidence);
    run("6. 8 Archetype Classification Engine    ", "archetypes",    test_archetypes);
    run("7. Shared Test Fixtures Verification    ", "fixtures",      test_shared_fixtures);
    run("8. Degenerate Tournament Evaluation     ", "empty_tournament", test_empty_tournament_evaluation);

    std::cout << "--------------------------------------------------------\n";
    if (all_passed) {
        std::cout << "VERDICT: All Domain Engine unit tests PASSED.\n";
        return 0;
    } else {
        std::cout << "VERDICT: Some tests FAILED.\n";
        return 1;
    }
}
