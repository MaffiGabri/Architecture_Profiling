#include "architecture/persistence.hpp"
#include <nlohmann/json.hpp>
#include <fstream>
#include <sstream>
#include <chrono>
#include <iomanip>
#include <cstdlib>
#include <random>
#include <algorithm>

#ifdef _WIN32
#include <windows.h>
#include <shlobj.h>
#endif

using json = nlohmann::json;

namespace arch::data {

namespace {

int64_t current_time_ms() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
}

std::string current_time_iso() {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm bt{};
#ifdef _WIN32
    gmtime_s(&bt, &in_time_t);
#else
    gmtime_r(&in_time_t, &bt);
#endif
    std::ostringstream ss;
    ss << std::put_time(&bt, "%Y-%m-%dT%H:%M:%SZ");
    return ss.str();
}

std::string generate_random_id(const std::string& prefix = "rec_") {
    static std::random_device rd;
    static std::mt19937_64 gen(rd());
    static std::uniform_int_distribution<uint64_t> dis;
    std::ostringstream ss;
    ss << prefix << std::hex << (dis(gen) & 0xFFFFFFFF);
    return ss.str();
}

} // anonymous namespace

// JSON conversions for UserPreferences
void to_json(json& j, const UserPreferences& p) {
    j = json{
        {"tournament_mode", (p.tournament_mode == TournamentModePref::Full45) ? "FULL" : "QUICK"},
        {"language", (p.language == LanguagePref::EN) ? "EN" : "IT"},
        {"theme", (p.theme == ThemePref::Light) ? "LIGHT" : "DARK"}
    };
}

void from_json(const json& j, UserPreferences& p) {
    if (j.contains("tournament_mode")) {
        std::string mode_str = j.at("tournament_mode").get<std::string>();
        p.tournament_mode = (mode_str == "QUICK") ? TournamentModePref::Quick15 : TournamentModePref::Full45;
    }
    if (j.contains("language")) {
        std::string lang_str = j.at("language").get<std::string>();
        p.language = (lang_str == "IT") ? LanguagePref::IT : LanguagePref::EN;
    }
    if (j.contains("theme")) {
        std::string theme_str = j.at("theme").get<std::string>();
        p.theme = (theme_str == "LIGHT") ? ThemePref::Light : ThemePref::Dark;
    }
}

// JSON conversions for UserStats
void to_json(json& j, const UserStats& s) {
    j = json{
        {"total_tournaments_completed", s.total_tournaments_completed},
        {"full_tournaments_completed", s.full_tournaments_completed},
        {"quick_tournaments_completed", s.quick_tournaments_completed},
        {"favorite_style_id", s.favorite_style_id},
        {"favorite_category_id", s.favorite_category_id},
        {"dominant_archetype_id", s.dominant_archetype_id}
    };
}

void from_json(const json& j, UserStats& s) {
    if (j.contains("total_tournaments_completed")) j.at("total_tournaments_completed").get_to(s.total_tournaments_completed);
    if (j.contains("full_tournaments_completed")) j.at("full_tournaments_completed").get_to(s.full_tournaments_completed);
    if (j.contains("quick_tournaments_completed")) j.at("quick_tournaments_completed").get_to(s.quick_tournaments_completed);
    if (j.contains("favorite_style_id")) j.at("favorite_style_id").get_to(s.favorite_style_id);
    if (j.contains("favorite_category_id")) j.at("favorite_category_id").get_to(s.favorite_category_id);
    if (j.contains("dominant_archetype_id")) j.at("dominant_archetype_id").get_to(s.dominant_archetype_id);
}

// JSON conversions for UserProfile
void to_json(json& j, const UserProfile& p) {
    j = json{
        {"id", p.id},
        {"username", p.username},
        {"created_at_ms", p.created_at_ms},
        {"last_active_ms", p.last_active_ms},
        {"preferences", p.preferences},
        {"stats", p.stats}
    };
}

void from_json(const json& j, UserProfile& p) {
    j.at("id").get_to(p.id);
    j.at("username").get_to(p.username);
    if (j.contains("created_at_ms")) j.at("created_at_ms").get_to(p.created_at_ms);
    if (j.contains("last_active_ms")) j.at("last_active_ms").get_to(p.last_active_ms);
    if (j.contains("preferences")) j.at("preferences").get_to(p.preferences);
    if (j.contains("stats")) j.at("stats").get_to(p.stats);
}

// JSON conversions for UserProfilesDocument
void to_json(json& j, const UserProfilesDocument& doc) {
    j = json{
        {"$schema", "urn:arch:user_profiles:v1"},
        {"version", doc.version},
        {"active_user_id", doc.active_user_id},
        {"profiles", doc.profiles}
    };
}

void from_json(const json& j, UserProfilesDocument& doc) {
    if (j.contains("version")) j.at("version").get_to(doc.version);
    if (j.contains("active_user_id")) j.at("active_user_id").get_to(doc.active_user_id);
    if (j.contains("profiles")) j.at("profiles").get_to(doc.profiles);
}

// Forward-declaration of TraitRadar JSON conversions from domain
} // namespace arch::data

namespace arch::domain {
void from_json(const json& j, TraitRadar& t);
void to_json(json& j, const TraitRadar& t);
} // namespace arch::domain

namespace arch::data {

// JSON conversions for HistoryRecord
void to_json(json& j, const HistoryRecord& r) {
    j = json{
        {"id", r.id},
        {"timestamp_ms", r.timestamp_ms},
        {"timestamp_iso", r.timestamp_iso},
        {"user_id", r.user_id},
        {"username", r.username},
        {"mode", r.mode},
        {"matches_played", r.matches_played},
        {"total_matches", r.total_matches},
        {"champion_style_id", r.champion_style_id},
        {"champion_style_name_en", r.champion_style_name_en},
        {"champion_style_name_it", r.champion_style_name_it},
        {"top_category_id", r.top_category_id},
        {"top_category_name_en", r.top_category_name_en},
        {"top_category_name_it", r.top_category_name_it},
        {"confidence_pct", r.confidence_pct},
        {"transitivity_zeta", r.transitivity_zeta},
        {"circular_triads", r.circular_triads},
        {"victory_margin", r.victory_margin},
        {"trait_coordinates", r.trait_coordinates},
        {"archetype_id", r.archetype_id},
        {"archetype_title_en", r.archetype_title_en},
        {"archetype_title_it", r.archetype_title_it},
        {"archetype_tagline_en", r.archetype_tagline_en},
        {"archetype_tagline_it", r.archetype_tagline_it},
        {"archetype_narrative_en", r.archetype_narrative_en},
        {"archetype_narrative_it", r.archetype_narrative_it}
    };
}

void from_json(const json& j, HistoryRecord& r) {
    if (j.contains("id")) j.at("id").get_to(r.id);
    if (j.contains("timestamp_ms")) j.at("timestamp_ms").get_to(r.timestamp_ms);
    if (j.contains("timestamp_iso")) j.at("timestamp_iso").get_to(r.timestamp_iso);
    if (j.contains("user_id")) j.at("user_id").get_to(r.user_id);
    if (j.contains("username")) j.at("username").get_to(r.username);
    if (j.contains("mode")) j.at("mode").get_to(r.mode);
    if (j.contains("matches_played")) j.at("matches_played").get_to(r.matches_played);
    if (j.contains("total_matches")) j.at("total_matches").get_to(r.total_matches);
    if (j.contains("champion_style_id")) j.at("champion_style_id").get_to(r.champion_style_id);
    if (j.contains("champion_style_name_en")) j.at("champion_style_name_en").get_to(r.champion_style_name_en);
    if (j.contains("champion_style_name_it")) j.at("champion_style_name_it").get_to(r.champion_style_name_it);
    if (j.contains("top_category_id")) j.at("top_category_id").get_to(r.top_category_id);
    if (j.contains("top_category_name_en")) j.at("top_category_name_en").get_to(r.top_category_name_en);
    if (j.contains("top_category_name_it")) j.at("top_category_name_it").get_to(r.top_category_name_it);
    if (j.contains("confidence_pct")) j.at("confidence_pct").get_to(r.confidence_pct);
    if (j.contains("transitivity_zeta")) j.at("transitivity_zeta").get_to(r.transitivity_zeta);
    if (j.contains("circular_triads")) j.at("circular_triads").get_to(r.circular_triads);
    if (j.contains("victory_margin")) j.at("victory_margin").get_to(r.victory_margin);
    if (j.contains("trait_coordinates")) j.at("trait_coordinates").get_to(r.trait_coordinates);
    if (j.contains("archetype_id")) j.at("archetype_id").get_to(r.archetype_id);
    if (j.contains("archetype_title_en")) j.at("archetype_title_en").get_to(r.archetype_title_en);
    if (j.contains("archetype_title_it")) j.at("archetype_title_it").get_to(r.archetype_title_it);
    if (j.contains("archetype_tagline_en")) j.at("archetype_tagline_en").get_to(r.archetype_tagline_en);
    if (j.contains("archetype_tagline_it")) j.at("archetype_tagline_it").get_to(r.archetype_tagline_it);
    if (j.contains("archetype_narrative_en")) j.at("archetype_narrative_en").get_to(r.archetype_narrative_en);
    if (j.contains("archetype_narrative_it")) j.at("archetype_narrative_it").get_to(r.archetype_narrative_it);
}

// JSON conversions for TournamentHistoryDocument
void to_json(json& j, const TournamentHistoryDocument& doc) {
    j = json{
        {"$schema", "urn:arch:tournament_history:v1"},
        {"version", doc.version},
        {"records", doc.records}
    };
}

void from_json(const json& j, TournamentHistoryDocument& doc) {
    if (j.contains("version")) j.at("version").get_to(doc.version);
    if (j.contains("records")) j.at("records").get_to(doc.records);
}

// -----------------------------------------------------------------------------
// PathResolver Implementation
// -----------------------------------------------------------------------------
std::filesystem::path PathResolver::get_storage_directory() {
    std::filesystem::path resolved_path;

#ifdef _WIN32
    PWSTR ppszPath = nullptr;
    HRESULT hr = SHGetKnownFolderPath(FOLDERID_RoamingAppData, KF_FLAG_CREATE, NULL, &ppszPath);
    if (SUCCEEDED(hr) && ppszPath) {
        resolved_path = std::filesystem::path(ppszPath) / "ArchitectureProfiling";
        CoTaskMemFree(ppszPath);
    }
#endif

    if (resolved_path.empty()) {
        const char* appdata_env = std::getenv("APPDATA");
        if (appdata_env && std::string_view(appdata_env).size() > 0) {
            resolved_path = std::filesystem::path(appdata_env) / "ArchitectureProfiling";
        } else {
            resolved_path = std::filesystem::current_path() / "ArchitectureProfiling";
        }
    }

    std::error_code ec;
    std::filesystem::create_directories(resolved_path, ec);
    if (ec) {
        // Fallback to local working directory
        resolved_path = std::filesystem::current_path() / "ArchitectureProfiling";
        std::filesystem::create_directories(resolved_path, ec);
    }

    return resolved_path;
}

std::filesystem::path PathResolver::get_user_profiles_path(const std::filesystem::path& base_dir) {
    auto dir = base_dir.empty() ? get_storage_directory() : base_dir;
    return dir / "user_profiles.json";
}

std::filesystem::path PathResolver::get_tournament_history_path(const std::filesystem::path& base_dir) {
    auto dir = base_dir.empty() ? get_storage_directory() : base_dir;
    return dir / "tournament_history.json";
}

// -----------------------------------------------------------------------------
// AtomicFileWriter Implementation
// -----------------------------------------------------------------------------
bool AtomicFileWriter::write_atomic(const std::filesystem::path& target_path, const std::string& content) {
    std::error_code ec;
    auto parent_dir = target_path.parent_path();
    if (!parent_dir.empty()) {
        std::filesystem::create_directories(parent_dir, ec);
    }

    std::filesystem::path temp_path = target_path;
    temp_path += ".tmp";

    {
        std::ofstream out(temp_path, std::ios::out | std::ios::binary | std::ios::trunc);
        if (!out.is_open()) {
            return false;
        }
        out.write(content.data(), static_cast<std::streamsize>(content.size()));
        out.flush();
        if (!out.good()) {
            out.close();
            std::filesystem::remove(temp_path, ec);
            return false;
        }
        out.close();
    }

#ifdef _WIN32
    // Bulletproof atomic file swap on Windows
    BOOL ok = MoveFileExW(
        temp_path.wstring().c_str(),
        target_path.wstring().c_str(),
        MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH
    );
    if (!ok) {
        std::filesystem::rename(temp_path, target_path, ec);
        return !ec;
    }
    return true;
#else
    std::filesystem::rename(temp_path, target_path, ec);
    return !ec;
#endif
}

// -----------------------------------------------------------------------------
// ProfileRepository Implementation
// -----------------------------------------------------------------------------
ProfileRepository::ProfileRepository(std::filesystem::path storage_dir)
    : file_path_(PathResolver::get_user_profiles_path(storage_dir))
{
}

bool ProfileRepository::load() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!std::filesystem::exists(file_path_)) {
        // Create initial default profile
        UserProfile default_prof;
        default_prof.id = "usr_default_01";
        default_prof.username = "Architectural Explorer";
        default_prof.created_at_ms = current_time_ms();
        default_prof.last_active_ms = default_prof.created_at_ms;

        doc_.version = "1.0.0";
        doc_.active_user_id = default_prof.id;
        doc_.profiles.clear();
        doc_.profiles.push_back(default_prof);

        json j = doc_;
        return AtomicFileWriter::write_atomic(file_path_, j.dump(2));
    }

    try {
        std::ifstream file(file_path_);
        if (!file.is_open()) return false;
        json j;
        file >> j;
        doc_ = j.get<UserProfilesDocument>();

        if (doc_.profiles.empty()) {
            UserProfile default_prof;
            default_prof.id = "usr_default_01";
            default_prof.username = "Architectural Explorer";
            default_prof.created_at_ms = current_time_ms();
            default_prof.last_active_ms = default_prof.created_at_ms;
            doc_.profiles.push_back(default_prof);
            doc_.active_user_id = default_prof.id;
            json out_j = doc_;
            AtomicFileWriter::write_atomic(file_path_, out_j.dump(2));
        }
        return true;
    } catch (...) {
        return false;
    }
}

bool ProfileRepository::save() {
    std::lock_guard<std::mutex> lock(mutex_);
    json j = doc_;
    return AtomicFileWriter::write_atomic(file_path_, j.dump(2));
}

UserProfile& ProfileRepository::get_active_profile() {
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto& p : doc_.profiles) {
        if (p.id == doc_.active_user_id) {
            return p;
        }
    }
    if (doc_.profiles.empty()) {
        UserProfile def;
        def.id = "usr_default_01";
        def.username = "Architectural Explorer";
        def.created_at_ms = current_time_ms();
        def.last_active_ms = def.created_at_ms;
        doc_.profiles.push_back(def);
        doc_.active_user_id = def.id;
    }
    return doc_.profiles.front();
}

const UserProfile& ProfileRepository::get_active_profile() const {
    std::lock_guard<std::mutex> lock(mutex_);
    for (const auto& p : doc_.profiles) {
        if (p.id == doc_.active_user_id) {
            return p;
        }
    }
    static UserProfile fallback{"usr_default_01", "Architectural Explorer", 0, 0, {}, {}};
    if (!doc_.profiles.empty()) return doc_.profiles.front();
    return fallback;
}

const std::vector<UserProfile>& ProfileRepository::get_all_profiles() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return doc_.profiles;
}

bool ProfileRepository::switch_user(const std::string& user_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto& p : doc_.profiles) {
        if (p.id == user_id) {
            doc_.active_user_id = user_id;
            p.last_active_ms = current_time_ms();
            json j = doc_;
            return AtomicFileWriter::write_atomic(file_path_, j.dump(2));
        }
    }
    return false;
}

bool ProfileRepository::create_profile(const std::string& username) {
    std::lock_guard<std::mutex> lock(mutex_);
    UserProfile new_prof;
    new_prof.id = generate_random_id("usr_");
    new_prof.username = username.empty() ? "Architect" : username;
    new_prof.created_at_ms = current_time_ms();
    new_prof.last_active_ms = new_prof.created_at_ms;

    doc_.profiles.push_back(new_prof);
    doc_.active_user_id = new_prof.id;

    json j = doc_;
    return AtomicFileWriter::write_atomic(file_path_, j.dump(2));
}

bool ProfileRepository::delete_profile(const std::string& user_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (doc_.profiles.size() <= 1) {
        // Do not allow deleting the last remaining profile
        return false;
    }

    auto it = std::remove_if(doc_.profiles.begin(), doc_.profiles.end(),
        [&](const UserProfile& p) { return p.id == user_id; });

    if (it == doc_.profiles.end()) return false;
    doc_.profiles.erase(it, doc_.profiles.end());

    if (doc_.active_user_id == user_id) {
        doc_.active_user_id = doc_.profiles.front().id;
    }

    json j = doc_;
    return AtomicFileWriter::write_atomic(file_path_, j.dump(2));
}

bool ProfileRepository::update_active_preferences(const UserPreferences& prefs) {
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto& p : doc_.profiles) {
        if (p.id == doc_.active_user_id) {
            p.preferences = prefs;
            p.last_active_ms = current_time_ms();
            json j = doc_;
            return AtomicFileWriter::write_atomic(file_path_, j.dump(2));
        }
    }
    return false;
}

bool ProfileRepository::record_tournament_outcome(const arch::domain::TournamentResult& result, const std::string& mode) {
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto& p : doc_.profiles) {
        if (p.id == doc_.active_user_id) {
            p.stats.total_tournaments_completed++;
            if (mode == "FULL" || mode == "Full") {
                p.stats.full_tournaments_completed++;
            } else {
                p.stats.quick_tournaments_completed++;
            }
            p.stats.favorite_style_id = result.champion_style_id;
            p.stats.favorite_category_id = result.dominant_category_id;
            p.stats.dominant_archetype_id = result.archetype.id;
            p.last_active_ms = current_time_ms();

            json j = doc_;
            return AtomicFileWriter::write_atomic(file_path_, j.dump(2));
        }
    }
    return false;
}

// -----------------------------------------------------------------------------
// HistoryRepository Implementation
// -----------------------------------------------------------------------------
HistoryRepository::HistoryRepository(std::filesystem::path storage_dir)
    : file_path_(PathResolver::get_tournament_history_path(storage_dir))
{
}

bool HistoryRepository::load() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!std::filesystem::exists(file_path_)) {
        doc_.version = "1.0.0";
        doc_.records.clear();
        json j = doc_;
        return AtomicFileWriter::write_atomic(file_path_, j.dump(2));
    }

    try {
        std::ifstream file(file_path_);
        if (!file.is_open()) return false;
        json j;
        file >> j;
        doc_ = j.get<TournamentHistoryDocument>();
        return true;
    } catch (...) {
        return false;
    }
}

bool HistoryRepository::save() {
    std::lock_guard<std::mutex> lock(mutex_);
    json j = doc_;
    return AtomicFileWriter::write_atomic(file_path_, j.dump(2));
}

const std::vector<HistoryRecord>& HistoryRepository::get_records() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return doc_.records;
}

bool HistoryRepository::add_record(
    const arch::domain::TournamentResult& result,
    const std::string& user_id,
    const std::string& username,
    const std::string& mode,
    const std::vector<arch::domain::Style>& styles,
    const std::vector<arch::domain::Category>& categories
) {
    std::lock_guard<std::mutex> lock(mutex_);
    HistoryRecord rec;
    rec.id = generate_random_id("rec_");
    rec.timestamp_ms = current_time_ms();
    rec.timestamp_iso = current_time_iso();
    rec.user_id = user_id;
    rec.username = username;
    rec.mode = mode;
    rec.matches_played = result.matches_played;
    rec.total_matches = result.total_matches;
    rec.champion_style_id = result.champion_style_id;

    for (const auto& s : styles) {
        if (s.id == result.champion_style_id) {
            rec.champion_style_name_en = s.style_en;
            rec.champion_style_name_it = s.style_it;
            break;
        }
    }

    rec.top_category_id = result.dominant_category_id;
    for (const auto& c : categories) {
        if (c.id == result.dominant_category_id) {
            rec.top_category_name_en = c.name_en;
            rec.top_category_name_it = c.name_it;
            break;
        }
    }

    rec.confidence_pct = result.confidence_pct;
    rec.transitivity_zeta = result.consistency_zeta;
    rec.circular_triads = result.circular_triads;
    rec.victory_margin = result.victory_margin;
    rec.trait_coordinates = result.user_traits;

    rec.archetype_id = result.archetype.id;
    rec.archetype_title_en = result.archetype.title_en;
    rec.archetype_title_it = result.archetype.title_it;
    rec.archetype_tagline_en = result.archetype.tagline_en;
    rec.archetype_tagline_it = result.archetype.tagline_it;
    rec.archetype_narrative_en = result.archetype.narrative_en;
    rec.archetype_narrative_it = result.archetype.narrative_it;

    // Prepend so the newest tournament appears first
    doc_.records.insert(doc_.records.begin(), std::move(rec));

    json j = doc_;
    return AtomicFileWriter::write_atomic(file_path_, j.dump(2));
}

bool HistoryRepository::delete_record(const std::string& record_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = std::remove_if(doc_.records.begin(), doc_.records.end(),
        [&](const HistoryRecord& r) { return r.id == record_id; });
    if (it == doc_.records.end()) return false;
    doc_.records.erase(it, doc_.records.end());

    json j = doc_;
    return AtomicFileWriter::write_atomic(file_path_, j.dump(2));
}

bool HistoryRepository::clear_history() {
    std::lock_guard<std::mutex> lock(mutex_);
    doc_.records.clear();
    json j = doc_;
    return AtomicFileWriter::write_atomic(file_path_, j.dump(2));
}

} // namespace arch::data
