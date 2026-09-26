#include <cassert>
#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include <cmath>

#include "architecture/persistence.hpp"
#include "architecture/localization.hpp"
#include "architecture/models.hpp"

using namespace arch::data;
using namespace arch::ui;

void test_path_resolver() {
    auto storage_dir = PathResolver::get_storage_directory();
    assert(!storage_dir.empty());
    assert(std::filesystem::exists(storage_dir));

    auto profiles_path = PathResolver::get_user_profiles_path(storage_dir);
    auto history_path = PathResolver::get_tournament_history_path(storage_dir);

    assert(profiles_path.filename() == "user_profiles.json");
    assert(history_path.filename() == "tournament_history.json");

    std::cout << "[PASS] test_path_resolver: " << storage_dir << "\n";
}

void test_atomic_file_writer() {
    auto test_dir = std::filesystem::temp_directory_path() / "ArchProfilingTest_Atomic";
    std::filesystem::create_directories(test_dir);
    auto test_file = test_dir / "atomic_test.txt";

    std::string payload = "Hello Atomic World with Win32 MoveFileExW!";
    bool write_ok = AtomicFileWriter::write_atomic(test_file, payload);
    assert(write_ok);
    (void)write_ok;
    assert(std::filesystem::exists(test_file));

    // Overwrite test
    std::string updated_payload = "Updated Atomic Content";
    bool overwrite_ok = AtomicFileWriter::write_atomic(test_file, updated_payload);
    assert(overwrite_ok);
    (void)overwrite_ok;

    std::filesystem::remove_all(test_dir);
    std::cout << "[PASS] test_atomic_file_writer\n";
}

void test_profile_repository() {
    auto test_dir = std::filesystem::temp_directory_path() / "ArchProfilingTest_Profiles";
    std::filesystem::create_directories(test_dir);

    // Initial load creates default profile
    ProfileRepository repo(test_dir);
    assert(repo.load() == true);
    assert(repo.get_all_profiles().size() == 1);
    assert(repo.get_active_profile().username == "Architectural Explorer");

    // Create new profile
    assert(repo.create_profile("Alice In Wonderland") == true);
    assert(repo.get_all_profiles().size() == 2);
    assert(repo.get_active_profile().username == "Alice In Wonderland");

    // Update preferences
    UserPreferences prefs;
    prefs.tournament_mode = TournamentModePref::Quick15;
    prefs.language = LanguagePref::IT;
    prefs.theme = ThemePref::Light;
    assert(repo.update_active_preferences(prefs) == true);
    assert(repo.get_active_profile().preferences.language == LanguagePref::IT);

    // Record outcome
    arch::domain::TournamentResult result;
    result.champion_style_id = 4;
    result.dominant_category_id = "classical_renaissance";
    result.archetype.id = "classical_monumentalist";
    assert(repo.record_tournament_outcome(result, "QUICK") == true);

    const auto& updated_prof = repo.get_active_profile();
    assert(updated_prof.stats.total_tournaments_completed == 1);
    assert(updated_prof.stats.quick_tournaments_completed == 1);
    assert(updated_prof.stats.favorite_style_id == 4);
    (void)updated_prof;

    // Reload from disk to verify atomic persistence
    ProfileRepository repo2(test_dir);
    assert(repo2.load() == true);
    assert(repo2.get_all_profiles().size() == 2);
    assert(repo2.get_active_profile().preferences.theme == ThemePref::Light);
    assert(repo2.get_active_profile().stats.favorite_style_id == 4);

    std::filesystem::remove_all(test_dir);
    std::cout << "[PASS] test_profile_repository\n";
}

void test_history_repository() {
    auto test_dir = std::filesystem::temp_directory_path() / "ArchProfilingTest_History";
    std::filesystem::create_directories(test_dir);

    HistoryRepository repo(test_dir);
    assert(repo.load() == true);
    assert(repo.get_records().empty());

    // Prepare mock styles & categories
    std::vector<arch::domain::Style> styles;
    arch::domain::Style s1;
    s1.id = 1;
    s1.style_en = "Classical Antiquity";
    s1.style_it = "Antichità Classica";
    styles.push_back(s1);

    std::vector<arch::domain::Category> categories;
    arch::domain::Category c1;
    c1.id = "classical_renaissance";
    c1.name_en = "Classical & Renaissance";
    c1.name_it = "Classico e Rinascimentale";
    categories.push_back(c1);

    arch::domain::TournamentResult result;
    result.matches_played = 45;
    result.total_matches = 45;
    result.champion_style_id = 1;
    result.dominant_category_id = "classical_renaissance";
    result.confidence_pct = 92.5;
    result.consistency_zeta = 0.95;
    result.circular_triads = 1;
    result.victory_margin = 0.18;
    result.user_traits = arch::domain::TraitRadar{2.0, 7.0, 8.0, 2.0, 8.0};
    result.archetype.id = "classical_monumentalist";
    result.archetype.title_en = "The Classical Monumentalist";
    result.archetype.title_it = "Il Monumentalista Classico";

    assert(repo.add_record(result, "usr_01", "Bob", "FULL", styles, categories) == true);
    assert(repo.get_records().size() == 1);
    assert(repo.get_records().front().champion_style_name_en == "Classical Antiquity");
    assert(repo.get_records().front().confidence_pct == 92.5);

    // Verify reload from disk
    HistoryRepository repo2(test_dir);
    assert(repo2.load() == true);
    assert(repo2.get_records().size() == 1);

    // Clear history
    assert(repo2.clear_history() == true);
    assert(repo2.get_records().empty());

    std::filesystem::remove_all(test_dir);
    std::cout << "[PASS] test_history_repository\n";
}

void test_localization() {
    auto& i18n = LocalizationManager::instance();

    // English checks
    i18n.set_language(Language::EN);
    assert(std::string(tr(StringId::AppName)) == "Architecture Profiling");
    assert(std::string(tr(StringId::BtnStartTournament)) == "Begin Tournament");
    assert(std::string(i18n.get_radar_axis_label(0)) == "Era");
    assert(std::string(i18n.get_radar_axis_label(4)) == "Material Warmth");

    // Italian instant toggle checks
    i18n.toggle_language();
    assert(i18n.current_language() == Language::IT);
    assert(std::string(tr(StringId::AppName)) == "Profilazione Architettonica");
    assert(std::string(tr(StringId::BtnStartTournament)) == "Inizia il Torneo");
    assert(std::string(i18n.get_radar_axis_label(0)) == "Epoca");
    assert(std::string(i18n.get_radar_axis_label(4)) == "Calore dei Materiali");

    // Dynamic style helper checks
    arch::domain::Style s;
    s.style_en = "Gothic Architecture";
    s.style_it = "Architettura Gotica";
    assert(std::string(i18n.get_style_name(s)) == "Architettura Gotica");

    i18n.set_language(Language::EN);
    assert(std::string(i18n.get_style_name(s)) == "Gothic Architecture");

    std::cout << "[PASS] test_localization\n";
}

int main(int argc, char** argv) {
    std::cout << "Running Persistence and Localization Unit Tests...\n";
    test_path_resolver();
    test_atomic_file_writer();
    test_profile_repository();
    test_history_repository();
    test_localization();
    std::cout << "\nAll 5 Persistence and Localization Unit Tests PASSED successfully!\n";
    return 0;
}
