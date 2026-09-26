#include "architecture/preference_engine.hpp"
#include "architecture/archetypes.hpp"
#include <cmath>
#include <algorithm>
#include <numeric>
#include <stdexcept>
#include <map>

namespace arch::domain {

BTResult PreferenceEngine::solve_bradley_terry(
    const std::vector<std::vector<int>>& win_matrix,
    double epsilon,
    int max_iterations
) {
    const size_t n = win_matrix.size();
    if (n != 10) {
        throw std::invalid_argument("Bradley-Terry solver requires a 10x10 win matrix");
    }

    std::vector<int> wins(n, 0);
    std::vector<int> total_played(n, 0);
    bool has_boundary_case = false;

    for (size_t i = 0; i < n; ++i) {
        for (size_t j = 0; j < n; ++j) {
            if (i == j) continue;
            wins[i] += win_matrix[i][j];
            total_played[i] += win_matrix[i][j] + win_matrix[j][i];
        }
        if (total_played[i] > 0 && (wins[i] == 0 || wins[i] == total_played[i])) {
            has_boundary_case = true;
        }
    }

    BTResult result;

    // Guard against degenerate tournament with zero matches played
    int total_all_matches = std::accumulate(total_played.begin(), total_played.end(), 0);
    if (total_all_matches == 0) {
        result.algorithm = "EmptyUniform";
        result.iterations = 0;
        result.probabilities.assign(n, 1.0 / static_cast<double>(n));
        return result;
    }

    if (has_boundary_case) {
        result.algorithm = "LaplaceFallback";
        result.iterations = 0;
        result.probabilities.resize(n, 0.0);

        double rate_sum = 0.0;
        for (size_t i = 0; i < n; ++i) {
            if (total_played[i] > 0) {
                result.probabilities[i] = static_cast<double>(wins[i] + 1) / static_cast<double>(total_played[i] + 2);
            } else {
                result.probabilities[i] = 0.5;
            }
            rate_sum += result.probabilities[i];
        }

        if (rate_sum > 0.0) {
            for (size_t i = 0; i < n; ++i) {
                result.probabilities[i] /= rate_sum;
            }
        }
        return result;
    }

    // Standard Bradley-Terry MM (Minorize-Maximization) algorithm
    result.algorithm = "BradleyTerryMM";
    result.probabilities.assign(n, 1.0 / static_cast<double>(n));

    std::vector<double>& p = result.probabilities;
    int iters = 0;

    for (int iter = 0; iter < max_iterations; ++iter) {
        iters++;
        std::vector<double> p_next(n, 0.0);

        for (size_t i = 0; i < n; ++i) {
            double denom = 0.0;
            for (size_t j = 0; j < n; ++j) {
                if (i == j) continue;
                int matches_ij = win_matrix[i][j] + win_matrix[j][i];
                if (matches_ij > 0) {
                    denom += static_cast<double>(matches_ij) / (p[i] + p[j]);
                }
            }
            p_next[i] = (denom > 1e-12) ? (static_cast<double>(wins[i]) / denom) : p[i];
        }

        double sum_next = std::accumulate(p_next.begin(), p_next.end(), 0.0);
        if (sum_next > 0.0) {
            for (size_t i = 0; i < n; ++i) {
                p_next[i] /= sum_next;
            }
        }

        double max_diff = 0.0;
        for (size_t i = 0; i < n; ++i) {
            max_diff = std::max(max_diff, std::abs(p_next[i] - p[i]));
        }

        p = p_next;
        if (max_diff < epsilon) {
            break;
        }
    }

    result.iterations = iters;
    return result;
}

std::vector<double> PreferenceEngine::calculate_bradley_terry(
    const std::vector<std::vector<int>>& win_matrix,
    double epsilon,
    int max_iterations
) {
    return solve_bradley_terry(win_matrix, epsilon, max_iterations).probabilities;
}

double PreferenceEngine::calculate_circular_triads(
    const std::vector<std::vector<int>>& win_matrix,
    int total_matches
) {
    const size_t n = win_matrix.size();
    std::vector<int> wins(n, 0);

    for (size_t i = 0; i < n; ++i) {
        for (size_t j = 0; j < n; ++j) {
            if (i != j) {
                wins[i] += win_matrix[i][j];
            }
        }
    }

    double scale = (total_matches == 45 || total_matches <= 0) ? 1.0 : (45.0 / static_cast<double>(total_matches));
    double sum_s_sq = 0.0;
    for (size_t i = 0; i < n; ++i) {
        double s_i = static_cast<double>(wins[i]) * scale;
        sum_s_sq += s_i * s_i;
    }

    double c = 142.5 - 0.5 * sum_s_sq;
    return std::clamp(c, 0.0, 40.0);
}

double PreferenceEngine::calculate_zeta(double circular_triads, int /*n*/) {
    return std::clamp(1.0 - (circular_triads / 40.0), 0.0, 1.0);
}

double PreferenceEngine::calculate_entropy(const std::vector<double>& probabilities) {
    double h = 0.0;
    for (double p : probabilities) {
        if (p > 1e-12) {
            h -= p * std::log(p);
        }
    }
    return h;
}

double PreferenceEngine::calculate_entropy_concentration(double entropy, int num_classes) {
    double max_h = std::log(static_cast<double>(num_classes));
    if (max_h <= 0.0) return 0.0;
    return std::clamp(1.0 - (entropy / max_h), 0.0, 1.0);
}

double PreferenceEngine::calculate_victory_margin(const std::vector<double>& sorted_probabilities) {
    if (sorted_probabilities.size() < 2) return 0.0;
    return std::clamp(sorted_probabilities[0] - sorted_probabilities[1], 0.0, 1.0);
}

double PreferenceEngine::calculate_confidence(double zeta, double victory_margin, double entropy_concentration) {
    double raw = (0.45 * zeta + 0.35 * std::tanh(3.0 * victory_margin) + 0.20 * entropy_concentration) * 100.0;
    return std::clamp(raw, 15.0, 99.0);
}

TraitRadar PreferenceEngine::calculate_user_traits(
    const std::vector<double>& style_probabilities,
    const std::vector<Style>& styles
) {
    TraitRadar total = TraitRadar::zero();
    const size_t count = std::min(style_probabilities.size(), styles.size());

    for (size_t i = 0; i < count; ++i) {
        double p = style_probabilities[i];
        const auto& t = styles[i].traits;
        total = total + (t * p);
    }

    return total;
}

std::vector<CategoryScore> PreferenceEngine::aggregate_categories(
    const std::vector<double>& style_probabilities,
    const std::vector<Style>& styles,
    const std::vector<Category>& categories
) {
    const size_t count = std::min(style_probabilities.size(), styles.size());
    std::vector<CategoryScore> scores;
    scores.reserve(categories.size());

    for (const auto& cat : categories) {
        CategoryScore cs;
        cs.category_id = cat.id;
        cs.name_en = cat.name_en;
        cs.affinity = 0.0;

        for (size_t i = 0; i < count; ++i) {
            if (styles[i].category_id == cat.id) {
                cs.affinity += style_probabilities[i];
            }
        }
        cs.percentage = cs.affinity * 100.0;
        scores.push_back(cs);
    }

    std::stable_sort(scores.begin(), scores.end(), [](const CategoryScore& a, const CategoryScore& b) {
        return a.affinity > b.affinity;
    });

    for (size_t i = 0; i < scores.size(); ++i) {
        scores[i].rank = static_cast<int>(i + 1);
    }

    return scores;
}

TournamentResult PreferenceEngine::evaluate(
    const std::vector<std::vector<int>>& win_matrix,
    const std::vector<Style>& styles,
    const std::vector<Category>& categories,
    int total_matches_scheduled
) {
    const size_t n = win_matrix.size();
    BTResult bt = solve_bradley_terry(win_matrix);

    int played_matches = 0;
    std::vector<int> wins(n, 0);
    std::vector<int> losses(n, 0);
    for (size_t i = 0; i < n; ++i) {
        for (size_t j = 0; j < n; ++j) {
            if (i != j) {
                wins[i] += win_matrix[i][j];
                losses[i] += win_matrix[j][i];
                played_matches += win_matrix[i][j];
            }
        }
    }

    double triads = calculate_circular_triads(win_matrix, total_matches_scheduled);
    double zeta = calculate_zeta(triads);

    std::vector<double> sorted_p = bt.probabilities;
    std::sort(sorted_p.begin(), sorted_p.end(), std::greater<double>());
    double delta = calculate_victory_margin(sorted_p);

    double entropy = calculate_entropy(bt.probabilities);
    double entropy_conc = calculate_entropy_concentration(entropy, static_cast<int>(n));
    double confidence = calculate_confidence(zeta, delta, entropy_conc);

    TraitRadar user_traits = calculate_user_traits(bt.probabilities, styles);
    std::vector<CategoryScore> cat_rankings = aggregate_categories(bt.probabilities, styles, categories);

    std::string dominant_cat_id = cat_rankings.empty() ? "" : cat_rankings[0].category_id;

    // Build style rankings
    std::vector<StyleScore> style_scores;
    style_scores.reserve(n);
    for (size_t i = 0; i < n; ++i) {
        StyleScore sc;
        sc.style_id = styles[i].id;
        sc.style_en = styles[i].style_en;
        sc.probability = bt.probabilities[i];
        sc.percentage = bt.probabilities[i] * 100.0;
        sc.wins = wins[i];
        sc.losses = losses[i];
        sc.total_matches = wins[i] + losses[i];
        style_scores.push_back(sc);
    }

    std::stable_sort(style_scores.begin(), style_scores.end(), [](const StyleScore& a, const StyleScore& b) {
        return a.probability > b.probability;
    });

    for (size_t i = 0; i < style_scores.size(); ++i) {
        style_scores[i].rank = static_cast<int>(i + 1);
    }

    int champ_id = style_scores.empty() ? 1 : style_scores[0].style_id;
    Archetype user_archetype = ArchetypeEngine::classify(
        dominant_cat_id, champ_id, user_traits, delta, zeta
    );

    TournamentResult result;
    result.style_rankings = style_scores;
    result.category_rankings = cat_rankings;
    result.champion_style_id = champ_id;
    result.dominant_category_id = dominant_cat_id;
    result.circular_triads = static_cast<int>(std::round(triads));
    result.consistency_zeta = zeta;
    result.victory_margin = delta;
    result.entropy = entropy;
    result.entropy_concentration = entropy_conc;
    result.confidence_pct = confidence;
    result.user_traits = user_traits;
    result.archetype = user_archetype;
    result.matches_played = played_matches;
    result.total_matches = total_matches_scheduled;
    result.algorithm = bt.algorithm;
    result.iterations = bt.iterations;

    return result;
}

} // namespace arch::domain
