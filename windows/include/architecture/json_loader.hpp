#pragma once

#include "models.hpp"
#include <filesystem>
#include <string>
#include <utility>
#include <vector>

namespace arch::domain {

struct FixtureTestCase {
    std::string id;
    std::string name;
    std::string mode;
    int match_count{0};
    std::vector<int> winner_choices;

    std::string expected_algorithm;
    int expected_iterations{0};
    int expected_champion_style_id{0};
    std::string expected_dominant_category_id;
    double expected_delta{0.0};
    double expected_circular_triads{0.0};
    double expected_zeta{0.0};
    double expected_confidence_pct{0.0};
    std::string expected_archetype;
    TraitRadar expected_traits;
    std::vector<CategoryScore> expected_category_rankings;
    std::vector<StyleScore> expected_style_rankings;
};

class JsonLoader {
public:
    /**
     * Loads categories and styles from styles.json file path.
     */
    static std::pair<std::vector<Style>, std::vector<Category>> load_styles(
        const std::filesystem::path& file_path
    );

    /**
     * Parses categories and styles directly from raw JSON text.
     */
    static std::pair<std::vector<Style>, std::vector<Category>> parse_styles(
        const std::string& json_text
    );

    /**
     * Loads test fixtures from shared_tournament_fixtures.json file path.
     */
    static std::vector<FixtureTestCase> load_fixtures(
        const std::filesystem::path& file_path
    );

    /**
     * Parses test fixtures directly from raw JSON text.
     */
    static std::vector<FixtureTestCase> parse_fixtures(
        const std::string& json_text
    );
};

} // namespace arch::domain
