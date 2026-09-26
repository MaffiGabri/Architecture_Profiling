#pragma once

#include "models.hpp"
#include <string>
#include <vector>
#include <filesystem>
#include <mutex>
#include <cstdint>
#include <optional>

namespace arch::data {

enum class TournamentModePref : uint8_t { Full45 = 0, Quick15 = 1 };
enum class LanguagePref : uint8_t { EN = 0, IT = 1 };
enum class ThemePref : uint8_t { Light = 0, Dark = 1 };

struct UserPreferences {
    TournamentModePref tournament_mode{TournamentModePref::Full45};
    LanguagePref language{LanguagePref::EN};
    ThemePref theme{ThemePref::Dark};
};

struct UserStats {
    int total_tournaments_completed{0};
    int full_tournaments_completed{0};
    int quick_tournaments_completed{0};
    int favorite_style_id{0};
    std::string favorite_category_id;
    std::string dominant_archetype_id;
};

struct UserProfile {
    std::string id;
    std::string username{"Architectural Explorer"};
    int64_t created_at_ms{0};
    int64_t last_active_ms{0};
    UserPreferences preferences;
    UserStats stats;
};

struct UserProfilesDocument {
    std::string version{"1.0.0"};
    std::string active_user_id{"usr_default_01"};
    std::vector<UserProfile> profiles;
};

struct HistoryRecord {
    std::string id;
    int64_t timestamp_ms{0};
    std::string timestamp_iso;
    std::string user_id;
    std::string username;
    std::string mode{"FULL"};
    int matches_played{0};
    int total_matches{0};
    int champion_style_id{0};
    std::string champion_style_name_en;
    std::string champion_style_name_it;
    std::string top_category_id;
    std::string top_category_name_en;
    std::string top_category_name_it;
    double confidence_pct{0.0};
    double transitivity_zeta{1.0};
    int circular_triads{0};
    double victory_margin{0.0};
    arch::domain::TraitRadar trait_coordinates;
    std::string archetype_id;
    std::string archetype_title_en;
    std::string archetype_title_it;
    std::string archetype_tagline_en;
    std::string archetype_tagline_it;
    std::string archetype_narrative_en;
    std::string archetype_narrative_it;
};

struct TournamentHistoryDocument {
    std::string version{"1.0.0"};
    std::vector<HistoryRecord> records;
};

class PathResolver {
public:
    static std::filesystem::path get_storage_directory();
    static std::filesystem::path get_user_profiles_path(const std::filesystem::path& base_dir = "");
    static std::filesystem::path get_tournament_history_path(const std::filesystem::path& base_dir = "");
};

class AtomicFileWriter {
public:
    static bool write_atomic(const std::filesystem::path& target_path, const std::string& content);
};

class ProfileRepository {
public:
    explicit ProfileRepository(std::filesystem::path storage_dir = PathResolver::get_storage_directory());
    bool load();
    bool save();

    UserProfile& get_active_profile();
    const UserProfile& get_active_profile() const;
    const std::vector<UserProfile>& get_all_profiles() const;

    bool switch_user(const std::string& user_id);
    bool create_profile(const std::string& username);
    bool delete_profile(const std::string& user_id);
    bool update_active_preferences(const UserPreferences& prefs);
    bool record_tournament_outcome(const arch::domain::TournamentResult& result, const std::string& mode);

private:
    std::filesystem::path file_path_;
    UserProfilesDocument doc_;
    mutable std::mutex mutex_;
};

class HistoryRepository {
public:
    explicit HistoryRepository(std::filesystem::path storage_dir = PathResolver::get_storage_directory());
    bool load();
    bool save();

    const std::vector<HistoryRecord>& get_records() const;
    bool add_record(const arch::domain::TournamentResult& result,
                    const std::string& user_id,
                    const std::string& username,
                    const std::string& mode,
                    const std::vector<arch::domain::Style>& styles,
                    const std::vector<arch::domain::Category>& categories);
    bool delete_record(const std::string& record_id);
    bool clear_history();

private:
    std::filesystem::path file_path_;
    TournamentHistoryDocument doc_;
    mutable std::mutex mutex_;
};

} // namespace arch::data
