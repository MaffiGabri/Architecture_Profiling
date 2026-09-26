#pragma once

#include "models.hpp"
#include <vector>
#include <string>

namespace arch::domain {

struct BTResult {
    std::vector<double> probabilities;
    std::string algorithm; // "BradleyTerryMM", "LaplaceFallback", or "EmptyUniform"
    int iterations{0};
};

class PreferenceEngine {
public:
    /**
     * Detailed Bradley-Terry solver returning probabilities, algorithm used, and iteration count.
     */
    static BTResult solve_bradley_terry(
        const std::vector<std::vector<int>>& win_matrix,
        double epsilon = 1e-6,
        int max_iterations = 50
    );

    /**
     * Solves Bradley-Terry latent preference probabilities using Minorize-Maximization (MM).
     * Falls back to regularized Laplace smoothing win-rate when undefeated or 0-win items exist.
     * @param win_matrix 10x10 matrix where win_matrix[i-1][j-1] is wins of style i over j.
     * @param epsilon Convergence threshold (default 1e-6).
     * @param max_iterations Maximum iterations (default 50).
     * @return 10-element vector of normalized probabilities summing to 1.0.
     */
    static std::vector<double> calculate_bradley_terry(
        const std::vector<std::vector<int>>& win_matrix,
        double epsilon = 1e-6,
        int max_iterations = 50
    );

    /**
     * Calculates Kendall circular triads (c) using Kendall & Babington Smith's theorem:
     * c = 142.5 - 0.5 * sum_{i=1}^{10} s_i^2, clamped to [0.0, 40.0].
     * For non-45 matches (e.g. quick mode 15 matches), out-degrees are scaled by (45.0 / total_matches).
     */
    static double calculate_circular_triads(
        const std::vector<std::vector<int>>& win_matrix,
        int total_matches = 45
    );

    /**
     * Calculates Kendall's coefficient of consistence zeta in [0.0, 1.0].
     * zeta = 1.0 - c / c_max, where c_max = 40.0 for n=10.
     */
    static double calculate_zeta(double circular_triads, int n = 10);

    /**
     * Calculates Shannon entropy H = -sum(p_i * ln(p_i)) for p_i > 1e-12.
     */
    static double calculate_entropy(const std::vector<double>& probabilities);

    /**
     * Calculates entropy concentration C_H = clamp(1.0 - H / ln(num_classes), 0.0, 1.0).
     */
    static double calculate_entropy_concentration(double entropy, int num_classes = 10);

    /**
     * Calculates margin of victory Delta = p_(1) - p_(2).
     */
    static double calculate_victory_margin(const std::vector<double>& sorted_probabilities);

    /**
     * Multi-objective confidence formula:
     * clamp((0.45 * zeta + 0.35 * tanh(3.0 * Delta) + 0.20 * C_H) * 100.0, 15.0, 99.0)
     */
    static double calculate_confidence(double zeta, double victory_margin, double entropy_concentration);

    /**
     * Projects style probabilities onto the 5-axis TraitRadar.
     * T_k = sum_{i=1}^{10} p_i * trait_{i,k}
     */
    static TraitRadar calculate_user_traits(
        const std::vector<double>& style_probabilities,
        const std::vector<Style>& styles
    );

    /**
     * Aggregates style probabilities into normalized category affinity percentages.
     */
    static std::vector<CategoryScore> aggregate_categories(
        const std::vector<double>& style_probabilities,
        const std::vector<Style>& styles,
        const std::vector<Category>& categories
    );

    /**
     * Evaluates complete tournament results from win matrix and metadata.
     */
    static TournamentResult evaluate(
        const std::vector<std::vector<int>>& win_matrix,
        const std::vector<Style>& styles,
        const std::vector<Category>& categories,
        int total_matches_scheduled
    );
};

} // namespace arch::domain
