#pragma once

#include "models.hpp"
#include <cstdint>
#include <string>

namespace arch::ui {

enum class Language : uint8_t {
    EN = 0,
    IT = 1,
    Count = 2
};

enum class StringId : uint16_t {
    AppName,
    AppTagline,
    NavWelcome,
    NavTournament,
    NavResults,
    NavHistory,
    NavSettings,
    WelcomeHeadline,
    WelcomeSubtitle,
    SelectModePrompt,
    ModeFullTitle,
    ModeFullDesc,
    ModeQuickTitle,
    ModeQuickDesc,
    BtnStartTournament,
    TournamentStepFmt,
    TournamentPrompt,
    BtnVoteLeft,
    BtnVoteRight,
    BtnUndo,
    BtnRedo,
    BtnRestart,
    DialogRestartTitle,
    DialogRestartDesc,
    ResultsTitle,
    ResultsChampionLabel,
    ResultsConfidenceBadgeFmt,
    ResultsConsistencyFmt,
    ResultsMarginFmt,
    ResultsTriadsFmt,
    ResultsRadarHeader,
    ResultsCategoryBreakdownHeader,
    ResultsStyleRankingsHeader,
    BtnSaveHistory,
    BtnViewHistory,
    BtnRetakeTournament,
    HistoryTitle,
    HistoryEmptyTitle,
    HistoryEmptyDesc,
    BtnClearHistory,
    SettingsTitle,
    SettingsThemeMode,
    ThemeLightLabel,
    ThemeDarkLabel,
    SettingsLanguage,
    BtnConfirm,
    BtnCancel,
    RadarAxisEra,
    RadarAxisOrnamentation,
    RadarAxisStructuralHonesty,
    RadarAxisGeometricOrder,
    RadarAxisMaterialWarmth,
    UserAccountLabel,
    CreateUserLabel,
    ActiveUserLabel,
    BtnNewProfile,
    ShortcutsTitle,
    ShortcutsKeyHeader,
    ShortcutsActionHeader,
    ShortcutsVoteA,
    ShortcutsVoteB,
    ShortcutsUndo,
    ShortcutsRedo,
    ShortcutsTheme,
    ShortcutsLang,
    ShortcutsBack,
    ShortcutsHelp,
    DialogClearHistoryTitle,
    DialogClearHistoryDesc,
    DialogAbandonTitle,
    DialogAbandonDesc,
    BtnResumeTournament,
    BtnAbandonTournament,
    BtnConfirmDelete,
    TotalStrings
};

class LocalizationManager {
public:
    static LocalizationManager& instance() {
        static LocalizationManager inst;
        return inst;
    }

    void set_language(Language lang) noexcept { current_lang_ = lang; }
    [[nodiscard]] Language current_language() const noexcept { return current_lang_; }
    void toggle_language() noexcept {
        current_lang_ = (current_lang_ == Language::EN) ? Language::IT : Language::EN;
    }

    // O(1) Zero-allocation lookup
    [[nodiscard]] const char* get(StringId id) const noexcept;

    // Dynamic style & category helpers
    [[nodiscard]] const char* get_style_name(const arch::domain::Style& s) const noexcept {
        return (current_lang_ == Language::IT && !s.style_it.empty()) ? s.style_it.c_str() : s.style_en.c_str();
    }

    [[nodiscard]] const char* get_category_name(const arch::domain::Category& c) const noexcept {
        return (current_lang_ == Language::IT && !c.name_it.empty()) ? c.name_it.c_str() : c.name_en.c_str();
    }

    [[nodiscard]] const char* get_category_desc(const arch::domain::Category& c) const noexcept {
        return (current_lang_ == Language::IT && !c.description_it.empty()) ? c.description_it.c_str() : c.description_en.c_str();
    }

    [[nodiscard]] const char* get_archetype_title(const arch::domain::Archetype& a) const noexcept {
        return (current_lang_ == Language::IT && !a.title_it.empty()) ? a.title_it.c_str() : a.title_en.c_str();
    }

    [[nodiscard]] const char* get_archetype_tagline(const arch::domain::Archetype& a) const noexcept {
        return (current_lang_ == Language::IT && !a.tagline_it.empty()) ? a.tagline_it.c_str() : a.tagline_en.c_str();
    }

    [[nodiscard]] const char* get_archetype_narrative(const arch::domain::Archetype& a) const noexcept {
        return (current_lang_ == Language::IT && !a.narrative_it.empty()) ? a.narrative_it.c_str() : a.narrative_en.c_str();
    }

    [[nodiscard]] const char* get_radar_axis_label(int axis_index) const noexcept;

private:
    LocalizationManager() = default;
    Language current_lang_{Language::EN};
    static const char* const kStringTable[static_cast<size_t>(Language::Count)][static_cast<size_t>(StringId::TotalStrings)];
};

// Global ergonomic helper
inline const char* tr(StringId id) noexcept {
    return LocalizationManager::instance().get(id);
}

} // namespace arch::ui
