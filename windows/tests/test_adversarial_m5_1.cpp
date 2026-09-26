#include <iostream>
#include <cassert>
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <cstdlib>

#include "architecture/localization.hpp"
#include "architecture/persistence.hpp"
#include "architecture/json_loader.hpp"
#include "architecture/tournament.hpp"
#include "architecture/models.hpp"

using namespace arch::ui;
using namespace arch::data;
using namespace arch::domain;

// =============================================================================
// Test 1: Localization Enum & Boundary Integrity
// =============================================================================
void test_localization_integrity() {
    std::cout << "[TEST] 1. Localization Enum & Boundary Stress Testing...\n";
    auto& i18n = LocalizationManager::instance();

    size_t total_strings_val = static_cast<size_t>(StringId::TotalStrings);
    assert(total_strings_val == 74);

    // Test all enum values in English
    i18n.set_language(Language::EN);
    for (size_t i = 0; i < total_strings_val; ++i) {
        StringId id = static_cast<StringId>(i);
        const char* str = i18n.get(id);
        assert(str != nullptr);
        assert(std::string(str) != "");
        assert(std::string(str) != "<missing_string>");
    }

    // Test all enum values in Italian
    i18n.set_language(Language::IT);
    for (size_t i = 0; i < total_strings_val; ++i) {
        StringId id = static_cast<StringId>(i);
        const char* str = i18n.get(id);
        assert(str != nullptr);
        assert(std::string(str) != "");
        assert(std::string(str) != "<missing_string>");
    }

    // Test Boundary Conditions: TotalStrings and Out-Of-Bounds
    const char* oob1 = i18n.get(StringId::TotalStrings);
    assert(std::string(oob1) == "<missing_string>");

    const char* oob2 = i18n.get(static_cast<StringId>(9999));
    assert(std::string(oob2) == "<missing_string>");

    // Test Invalid Language Cast
    i18n.set_language(static_cast<Language>(99));
    const char* oob_lang = i18n.get(StringId::AppName);
    assert(std::string(oob_lang) == "<missing_string>");

    // Restore to English
    i18n.set_language(Language::EN);

    // Radar axis labels: check 0..4 and out of bounds
    for (int axis = 0; axis < 5; ++axis) {
        const char* label = i18n.get_radar_axis_label(axis);
        assert(label != nullptr && std::string(label) != "");
    }
    assert(std::string(i18n.get_radar_axis_label(-1)) == "");
    assert(std::string(i18n.get_radar_axis_label(5)) == "");
    assert(std::string(i18n.get_radar_axis_label(100)) == "");

    std::cout << "  -> PASS: All 74 StringId enum values verified in EN/IT with safe boundary fallbacks.\n";
}

// =============================================================================
// Test 2: APPDATA Profile Repository Adversarial Testing
// =============================================================================
void test_appdata_profile_resilience() {
    std::cout << "[TEST] 2. APPDATA Profile Repository Adversarial Resilience...\n";

    auto temp_base = std::filesystem::temp_directory_path() / "ArchAdv_ProfilesTest";
    std::filesystem::remove_all(temp_base);
    std::filesystem::create_directories(temp_base);

    // Case 2.1: Non-existent directory & file creation
    {
        ProfileRepository repo(temp_base);
        bool ok = repo.load();
        assert(ok);
        assert(repo.get_all_profiles().size() == 1);
        assert(repo.get_active_profile().username == "Architectural Explorer");
    }

    // Case 2.2: Corrupt / truncated JSON in user_profiles.json
    auto profiles_json = temp_base / "user_profiles.json";
    {
        std::ofstream f(profiles_json, std::ios::trunc);
        f << "{ \"version\": \"1.0.0\", \"profiles\": [ { \"id\": \"broken\", ";
    }
    {
        ProfileRepository repo(temp_base);
        bool load_res = repo.load();
        // Repository should catch JSON parse exception and return false
        assert(!load_res);
        // get_active_profile() should provide a safe in-memory fallback
        auto& active = repo.get_active_profile();
        assert(active.username == "Architectural Explorer");
        assert(active.id == "usr_default_01");
    }

    // Case 2.3: Empty JSON object in user_profiles.json
    {
        std::ofstream f(profiles_json, std::ios::trunc);
        f << "{}";
    }
    {
        ProfileRepository repo(temp_base);
        bool load_res = repo.load();
        assert(load_res);
        // doc_.profiles is empty -> repo creates default profile
        assert(repo.get_all_profiles().size() == 1);
        assert(repo.get_active_profile().username == "Architectural Explorer");
    }

    // Case 2.4: Missing fields in user_profiles.json
    {
        std::ofstream f(profiles_json, std::ios::trunc);
        f << "{ \"version\": \"1.0.0\", \"active_user_id\": \"u1\", \"profiles\": [ {\"id\": \"u1\", \"username\": \"CustomUser\"} ] }";
    }
    {
        ProfileRepository repo(temp_base);
        bool load_res = repo.load();
        assert(load_res);
        assert(repo.get_active_profile().username == "CustomUser");
        // Preferences and stats should have defaulted safely
        assert(repo.get_active_profile().preferences.tournament_mode == TournamentModePref::Full45);
        assert(repo.get_active_profile().stats.total_tournaments_completed == 0);
    }

    // Case 2.5: HistoryRepository with corrupt history file
    auto history_json = temp_base / "tournament_history.json";
    {
        std::ofstream f(history_json, std::ios::trunc);
        f << "{\"records\": [{\"broken\": ";
    }
    {
        HistoryRepository h_repo(temp_base);
        bool h_res = h_repo.load();
        assert(!h_res);
        assert(h_repo.get_records().empty());
    }

    std::filesystem::remove_all(temp_base);
    std::cout << "  -> PASS: Profile and History repositories gracefully recover from missing/corrupt JSON.\n";
}

// =============================================================================
// Test 3: JsonLoader Adversarial & Edge Cases
// =============================================================================
void test_json_loader_adversarial() {
    std::cout << "[TEST] 3. JsonLoader Adversarial Stress Testing...\n";

    // Case 3.1: Completely malformed JSON syntax
    try {
        JsonLoader::parse_styles("{ invalid_json: [1, 2, }");
        assert(false && "Should have thrown on invalid JSON");
    } catch (const std::exception& e) {
        // Expected
    }

    // Case 3.2: Missing "styles" array
    try {
        JsonLoader::parse_styles("{\"categories\": []}");
        assert(false && "Should have thrown on missing styles array");
    } catch (const std::exception& e) {
        // Expected
    }

    // Case 3.3: Missing "categories" array
    try {
        JsonLoader::parse_styles("{\"styles\": []}");
        assert(false && "Should have thrown on missing categories array");
    } catch (const std::exception& e) {
        // Expected
    }

    // Case 3.4: Style object missing required fields (e.g. traits)
    try {
        std::string broken_style = R"({
            "categories": [{"id": "c1", "name_en": "Cat1"}],
            "styles": [{"id": 1, "filename": "1.png", "title": "T", "style_en": "E", "category_id": "c1"}]
        })";
        JsonLoader::parse_styles(broken_style);
        assert(false && "Should have thrown on style missing traits");
    } catch (const std::exception& e) {
        // Expected
    }

    // Case 3.5: Valid minimal style & category
    {
        std::string minimal = R"({
            "categories": [{"id": "c1", "name_en": "Cat1"}],
            "styles": [{
                "id": 1, "filename": "1.png", "title": "T", "style_en": "E", "category_id": "c1",
                "traits": {"era": 5.0, "ornamentation": 5.0, "structural_honesty": 5.0, "geometric_order": 5.0, "material_warmth": 5.0}
            }]
        })";
        auto [styles, categories] = JsonLoader::parse_styles(minimal);
        assert(styles.size() == 1);
        assert(categories.size() == 1);
        assert(styles[0].base_weight == 1.0); // defaulted
    }

    std::cout << "  -> PASS: JsonLoader strictly enforces schema validation and rejects invalid payloads.\n";
}

int main() {
    std::cout << "====================================================================\n";
    std::cout << " Milestone M5: Adversarial C++ Domain & Platform Stress Suite\n";
    std::cout << "====================================================================\n";

    test_localization_integrity();
    test_appdata_profile_resilience();
    test_json_loader_adversarial();

    std::cout << "\n====================================================================\n";
    std::cout << " ALL C++ ADVERSARIAL STRESS TESTS COMPLETED SUCCESSFULLY!\n";
    std::cout << "====================================================================\n";
    return 0;
}
