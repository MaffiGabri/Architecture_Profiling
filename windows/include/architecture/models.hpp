#pragma once

#include <string>
#include <vector>
#include <array>
#include <cmath>
#include <optional>
#include <compare>
#include <string_view>
#include <cstdint>

namespace arch::domain {

/**
 * 5-Axis Aesthetic Trait Vector [0.0, 10.0]^5
 */
struct TraitRadar {
    double era{5.0};
    double ornamentation{5.0};
    double structural_honesty{5.0};
    double geometric_order{5.0};
    double material_warmth{5.0};

    static constexpr TraitRadar zero() noexcept {
        return TraitRadar{0.0, 0.0, 0.0, 0.0, 0.0};
    }

    constexpr TraitRadar operator+(const TraitRadar& other) const noexcept {
        return TraitRadar{
            era + other.era,
            ornamentation + other.ornamentation,
            structural_honesty + other.structural_honesty,
            geometric_order + other.geometric_order,
            material_warmth + other.material_warmth
        };
    }

    constexpr TraitRadar operator*(double scalar) const noexcept {
        return TraitRadar{
            era * scalar,
            ornamentation * scalar,
            structural_honesty * scalar,
            geometric_order * scalar,
            material_warmth * scalar
        };
    }

    constexpr double distance_squared(const TraitRadar& other) const noexcept {
        double d1 = era - other.era;
        double d2 = ornamentation - other.ornamentation;
        double d3 = structural_honesty - other.structural_honesty;
        double d4 = geometric_order - other.geometric_order;
        double d5 = material_warmth - other.material_warmth;
        return d1 * d1 + d2 * d2 + d3 * d3 + d4 * d4 + d5 * d5;
    }

    double distance(const TraitRadar& other) const noexcept {
        return std::sqrt(distance_squared(other));
    }

    constexpr std::array<double, 5> to_array() const noexcept {
        return {era, ornamentation, structural_honesty, geometric_order, material_warmth};
    }

    auto operator<=>(const TraitRadar&) const = default;
};

/**
 * Architectural Category Metadata
 */
struct Category {
    std::string id;
    std::string name_en;
    std::string name_it;
    std::string description_en;
    std::string description_it;
    std::string color_hex;

    auto operator<=>(const Category&) const = default;
};

/**
 * Architectural Style Item Metadata
 */
struct Style {
    int id{0};
    std::string filename;
    std::string title;
    std::string style_en;
    std::string style_it;
    std::string category_id;
    std::string era_century;
    double base_weight{1.0};
    TraitRadar traits;
    std::vector<std::string> tags;
    std::string color_hex;

    auto operator<=>(const Style&) const = default;
};

/**
 * Match Outcome Enum
 */
enum class MatchOutcome : uint8_t {
    Undecided = 0,
    LeftWon = 1,
    RightWon = 2
};

/**
 * Pairwise Match Definition
 */
struct MatchPair {
    int match_index{0};
    int round_index{0};
    int left_style_id{0};
    int right_style_id{0};

    auto operator<=>(const MatchPair&) const = default;
};

/**
 * Match State Record with Outcome
 */
struct MatchRecord {
    int match_index{0};
    int round_index{0};
    int left_style_id{0};
    int right_style_id{0};
    MatchOutcome outcome{MatchOutcome::Undecided};
    int winner_style_id{0};

    constexpr bool is_played() const noexcept { return outcome != MatchOutcome::Undecided; }
    auto operator<=>(const MatchRecord&) const = default;
};

/**
 * Computed Style Score & Ranking
 */
struct StyleScore {
    int style_id{0};
    std::string style_en;
    double probability{0.0};
    double percentage{0.0};
    int rank{0};
    int wins{0};
    int losses{0};
    int total_matches{0};

    auto operator<=>(const StyleScore&) const = default;
};

/**
 * Computed Category Score & Ranking
 */
struct CategoryScore {
    std::string category_id;
    std::string name_en;
    double affinity{0.0};
    double percentage{0.0};
    int rank{0};

    auto operator<=>(const CategoryScore&) const = default;
};

/**
 * Narrative Architectural Archetype Profile
 */
struct Archetype {
    std::string id;
    std::string title_en;
    std::string title_it;
    std::string tagline_en;
    std::string tagline_it;
    std::string narrative_en;
    std::string narrative_it;
    std::string dominant_category_id;

    auto operator<=>(const Archetype&) const = default;
};

/**
 * Comprehensive Tournament Result Evaluation
 */
struct TournamentResult {
    std::vector<StyleScore> style_rankings;
    std::vector<CategoryScore> category_rankings;
    int champion_style_id{0};
    std::string dominant_category_id;
    int circular_triads{0};
    double consistency_zeta{1.0};
    double victory_margin{0.0};
    double entropy{0.0};
    double entropy_concentration{0.0};
    double confidence_pct{0.0};
    TraitRadar user_traits;
    Archetype archetype;
    int matches_played{0};
    int total_matches{0};
    std::string algorithm{"LaplaceFallback"};
    int iterations{0};

    auto operator<=>(const TournamentResult&) const = default;
};

} // namespace arch::domain
