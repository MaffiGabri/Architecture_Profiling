#include "architecture/json_loader.hpp"
#include <nlohmann/json.hpp>
#include <fstream>
#include <sstream>
#include <stdexcept>

using json = nlohmann::json;

namespace arch::domain {

void from_json(const json& j, TraitRadar& t) {
    j.at("era").get_to(t.era);
    j.at("ornamentation").get_to(t.ornamentation);
    j.at("structural_honesty").get_to(t.structural_honesty);
    j.at("geometric_order").get_to(t.geometric_order);
    j.at("material_warmth").get_to(t.material_warmth);
}

void to_json(json& j, const TraitRadar& t) {
    j = json{
        {"era", t.era},
        {"ornamentation", t.ornamentation},
        {"structural_honesty", t.structural_honesty},
        {"geometric_order", t.geometric_order},
        {"material_warmth", t.material_warmth}
    };
}

void from_json(const json& j, Category& c) {
    j.at("id").get_to(c.id);
    j.at("name_en").get_to(c.name_en);
    if (j.contains("name_it")) {
        j.at("name_it").get_to(c.name_it);
    }
    if (j.contains("description_en")) {
        j.at("description_en").get_to(c.description_en);
    }
    if (j.contains("description_it")) {
        j.at("description_it").get_to(c.description_it);
    }
    if (j.contains("color_hex")) {
        j.at("color_hex").get_to(c.color_hex);
    }
}

void from_json(const json& j, Style& s) {
    j.at("id").get_to(s.id);
    j.at("filename").get_to(s.filename);
    j.at("title").get_to(s.title);
    j.at("style_en").get_to(s.style_en);
    if (j.contains("style_it")) {
        j.at("style_it").get_to(s.style_it);
    }
    j.at("category_id").get_to(s.category_id);
    if (j.contains("era_century")) {
        j.at("era_century").get_to(s.era_century);
    }
    if (j.contains("base_weight")) {
        j.at("base_weight").get_to(s.base_weight);
    } else {
        s.base_weight = 1.0;
    }
    j.at("traits").get_to(s.traits);
    if (j.contains("tags")) {
        j.at("tags").get_to(s.tags);
    }
    if (j.contains("color_hex")) {
        j.at("color_hex").get_to(s.color_hex);
    }
}

void from_json(const json& j, CategoryScore& cs) {
    j.at("category_id").get_to(cs.category_id);
    if (j.contains("name_en")) {
        j.at("name_en").get_to(cs.name_en);
    }
    if (j.contains("affinity")) {
        j.at("affinity").get_to(cs.affinity);
    } else if (j.contains("score")) {
        j.at("score").get_to(cs.affinity);
    }
    if (j.contains("percentage")) {
        j.at("percentage").get_to(cs.percentage);
    }
    if (j.contains("rank")) {
        j.at("rank").get_to(cs.rank);
    }
}

void from_json(const json& j, StyleScore& ss) {
    j.at("style_id").get_to(ss.style_id);
    if (j.contains("style_en")) {
        j.at("style_en").get_to(ss.style_en);
    }
    if (j.contains("probability")) {
        j.at("probability").get_to(ss.probability);
    } else if (j.contains("score")) {
        j.at("score").get_to(ss.probability);
    }
    if (j.contains("percentage")) {
        j.at("percentage").get_to(ss.percentage);
    }
    if (j.contains("rank")) {
        j.at("rank").get_to(ss.rank);
    }
}

std::pair<std::vector<Style>, std::vector<Category>> JsonLoader::parse_styles(
    const std::string& json_text
) {
    json root = json::parse(json_text);
    std::vector<Category> categories = root.at("categories").get<std::vector<Category>>();
    std::vector<Style> styles = root.at("styles").get<std::vector<Style>>();
    return {styles, categories};
}

std::pair<std::vector<Style>, std::vector<Category>> JsonLoader::load_styles(
    const std::filesystem::path& file_path
) {
    std::ifstream file(file_path);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open styles JSON file: " + file_path.string());
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return parse_styles(buffer.str());
}

std::vector<FixtureTestCase> JsonLoader::parse_fixtures(const std::string& json_text) {
    json root = json::parse(json_text);
    std::vector<FixtureTestCase> fixtures;

    const std::string array_key = root.contains("test_cases") ? "test_cases" : "fixtures";
    if (!root.contains(array_key)) {
        throw std::runtime_error("Fixtures JSON does not contain test_cases or fixtures array");
    }

    for (const auto& item : root.at(array_key)) {
        FixtureTestCase tc;
        tc.id = item.at("id").get<std::string>();
        tc.name = item.at("name").get<std::string>();
        tc.mode = item.value("mode", "full");
        tc.match_count = item.value("match_count", 45);

        if (item.contains("match_winners")) {
            tc.winner_choices = item.at("match_winners").get<std::vector<int>>();
        } else if (item.contains("winner_choices")) {
            tc.winner_choices = item.at("winner_choices").get<std::vector<int>>();
        }

        const auto& exp = item.at("expected");
        tc.expected_algorithm = exp.value("algorithm", "");
        tc.expected_iterations = exp.value("iterations", 0);
        tc.expected_champion_style_id = exp.contains("dominant_style_id")
            ? exp.at("dominant_style_id").get<int>()
            : exp.value("champion_style_id", 0);

        tc.expected_dominant_category_id = exp.at("dominant_category").get<std::string>();
        tc.expected_delta = exp.value("delta", 0.0);
        tc.expected_circular_triads = exp.value("circular_triads", 0.0);
        tc.expected_zeta = exp.value("zeta", 0.0);

        if (exp.contains("confidence_pct")) {
            tc.expected_confidence_pct = exp.at("confidence_pct").get<double>();
        } else if (exp.contains("confidence_percentage")) {
            tc.expected_confidence_pct = exp.at("confidence_percentage").get<double>();
        }

        tc.expected_archetype = exp.contains("archetype")
            ? exp.at("archetype").get<std::string>()
            : exp.value("archetype_id", "");

        if (exp.contains("traits_5d")) {
            exp.at("traits_5d").get_to(tc.expected_traits);
        } else if (exp.contains("user_traits")) {
            exp.at("user_traits").get_to(tc.expected_traits);
        }

        if (exp.contains("category_rankings")) {
            tc.expected_category_rankings = exp.at("category_rankings").get<std::vector<CategoryScore>>();
        }

        if (exp.contains("style_rankings")) {
            tc.expected_style_rankings = exp.at("style_rankings").get<std::vector<StyleScore>>();
        }

        fixtures.push_back(tc);
    }

    return fixtures;
}

std::vector<FixtureTestCase> JsonLoader::load_fixtures(const std::filesystem::path& file_path) {
    std::ifstream file(file_path);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open fixtures JSON file: " + file_path.string());
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return parse_fixtures(buffer.str());
}

} // namespace arch::domain
