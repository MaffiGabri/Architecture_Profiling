#include <windows.h>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>
#include <memory>

#include "architecture/models.hpp"
#include "architecture/tournament.hpp"
#include "architecture/json_loader.hpp"
#include "architecture/persistence.hpp"
#include "architecture/localization.hpp"
#include "architecture/theme_manager.hpp"
#include "architecture/texture_manager.hpp"

#include "platform/d3d11_context.hpp"
#include "ui/welcome_view.hpp"
#include "ui/tournament_view.hpp"
#include "ui/results_view.hpp"
#include "ui/history_view.hpp"

namespace arch::ui {
void set_tournament_abandon_callback(std::function<void()> cb);
}

namespace {

enum class AppScreen {
    Welcome,
    Tournament,
    Results,
    History
};

std::filesystem::path resolve_styles_json_path() {
    wchar_t exe_buf[MAX_PATH];
    if (GetModuleFileNameW(nullptr, exe_buf, MAX_PATH) > 0) {
        std::filesystem::path exe_dir = std::filesystem::path(exe_buf).parent_path();
        std::array<std::filesystem::path, 4> exe_candidates = {
            exe_dir / "assets" / "data" / "styles.json",
            exe_dir / "data" / "styles.json",
            exe_dir / ".." / "shared" / "data" / "styles.json",
            exe_dir.parent_path() / "shared" / "data" / "styles.json"
        };
        for (const auto& c : exe_candidates) {
            if (std::filesystem::exists(c)) return c;
        }
    }

    std::array<std::filesystem::path, 6> local_candidates = {
        std::filesystem::path("assets/data/styles.json"),
        std::filesystem::path("data/styles.json"),
        std::filesystem::path("shared/data/styles.json"),
        std::filesystem::path("../shared/data/styles.json"),
        std::filesystem::path("../../shared/data/styles.json"),
        std::filesystem::path("../../../shared/data/styles.json")
    };
    for (const auto& c : local_candidates) {
        if (std::filesystem::exists(c)) return c;
    }

    return "assets/data/styles.json";
}

std::filesystem::path resolve_asset_root_path() {
    wchar_t exe_buf[MAX_PATH];
    if (GetModuleFileNameW(nullptr, exe_buf, MAX_PATH) > 0) {
        std::filesystem::path exe_dir = std::filesystem::path(exe_buf).parent_path();
        if (std::filesystem::exists(exe_dir / "assets")) {
            return exe_dir / "assets";
        }
        if (std::filesystem::exists(exe_dir / ".." / "shared")) {
            return exe_dir / ".." / "shared";
        }
    }
    if (std::filesystem::exists("assets")) return "assets";
    if (std::filesystem::exists("shared")) return "shared";
    if (std::filesystem::exists("../shared")) return "../shared";
    return "";
}

} // namespace

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    std::ofstream log("debug_log.txt");
    log << "App started" << std::endl;
    // 1. Persistence & User Profiles
    arch::data::ProfileRepository profile_repo;
    profile_repo.load();
    log << "Profiles loaded" << std::endl;


    arch::data::HistoryRepository history_repo;
    log << "Calling history_repo.load()..." << std::endl;
    history_repo.load();
    log << "History loaded" << std::endl;

    log << "Calling profile_repo.get_active_profile()..." << std::endl;
    const auto& active_profile = profile_repo.get_active_profile();
    log << "Active profile fetched" << std::endl;

    // 2. Localization & Theme Managers
    log << "Initializing LocalizationManager..." << std::endl;
    auto& i18n = arch::ui::LocalizationManager::instance();
    log << "Setting language..." << std::endl;
    i18n.set_language(
        (active_profile.preferences.language == arch::data::LanguagePref::IT)
            ? arch::ui::Language::IT
            : arch::ui::Language::EN
    );
    log << "Language set" << std::endl;

    // 3. DirectX 11 & Win32 Window Context
    arch::platform::D3D11Context context;
    if (!context.initialize(1280, 840, L"Architecture Profiling - Windows Desktop Companion")) {
        log << "Context initialization failed!" << std::endl;
        MessageBoxW(nullptr, L"Failed to initialize DirectX 11 rendering context.", L"Error", MB_ICONERROR);
        return 1;
    }
    log << "Context initialized" << std::endl;

    log << "Initializing ThemeManager..." << std::endl;
    auto& theme_mgr = arch::ui::ThemeManager::instance();
    log << "Setting theme..." << std::endl;
    theme_mgr.apply_theme(
        (active_profile.preferences.theme == arch::data::ThemePref::Light)
            ? arch::ui::Theme::Light
            : arch::ui::Theme::Dark
    );
    log << "Theme set" << std::endl;

    // 4. Load Architectural Styles & Categories Metadata
    auto styles_path = resolve_styles_json_path();
    log << "Styles path resolved to: " << styles_path << std::endl;
    auto [styles, categories] = arch::domain::JsonLoader::load_styles(styles_path);
    log << "Styles loaded count: " << styles.size() << std::endl;
    if (styles.empty()) {
        std::cerr << "[Main] Warning: No styles loaded from " << styles_path << std::endl;
    }

    // 5. Pre-warm DirectX 11 Image Textures via stb_image
    arch::platform::TextureManager texture_manager;
    auto asset_root = resolve_asset_root_path();
    texture_manager.initialize(context.device(), asset_root);
    texture_manager.prewarm_style_images(styles);

    // 6. Application Views & State Router
    AppScreen current_screen = AppScreen::Welcome;
    arch::domain::TournamentMode selected_mode = (active_profile.preferences.tournament_mode == arch::data::TournamentModePref::Quick15)
        ? arch::domain::TournamentMode::Quick
        : arch::domain::TournamentMode::Full;

    std::unique_ptr<arch::domain::TournamentStateMachine> tournament;
    arch::domain::TournamentResult current_result;

    arch::ui::WelcomeView welcome_view;
    arch::ui::TournamentView tournament_view;
    arch::ui::ResultsView results_view;
    arch::ui::HistoryView history_view;

    arch::ui::set_tournament_abandon_callback([&]() {
        tournament.reset();
        current_screen = AppScreen::Welcome;
    });

    // 7. Interactive Message & Rendering Loop
    while (context.process_messages()) {
        context.begin_frame();

        bool is_it = (i18n.current_language() == arch::ui::Language::IT);
        bool open_shortcuts_modal = false;

        // Global hotkeys (active only when user is not typing in a text input field)
        if (!ImGui::GetIO().WantTextInput) {
            if (ImGui::IsKeyPressed(ImGuiKey_F1)) {
                open_shortcuts_modal = true;
            }
            if (ImGui::IsKeyPressed(ImGuiKey_T)) {
                theme_mgr.toggle_theme();
                arch::data::UserPreferences prefs = profile_repo.get_active_profile().preferences;
                prefs.theme = (theme_mgr.current_theme() == arch::ui::Theme::Light)
                    ? arch::data::ThemePref::Light
                    : arch::data::ThemePref::Dark;
                profile_repo.update_active_preferences(prefs);
            }
            if (ImGui::IsKeyPressed(ImGuiKey_L)) {
                i18n.toggle_language();
                arch::data::UserPreferences prefs = profile_repo.get_active_profile().preferences;
                prefs.language = (i18n.current_language() == arch::ui::Language::IT)
                    ? arch::data::LanguagePref::IT
                    : arch::data::LanguagePref::EN;
                profile_repo.update_active_preferences(prefs);
            }
            if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
                if (current_screen == AppScreen::History || current_screen == AppScreen::Results) {
                    current_screen = AppScreen::Welcome;
                }
            }
        }

        // Render Top Utility & Navigation Header Bar
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
        ImGuiWindowFlags main_win_flags = ImGuiWindowFlags_NoDecoration |
                                          ImGuiWindowFlags_NoMove |
                                          ImGuiWindowFlags_NoResize |
                                          ImGuiWindowFlags_NoSavedSettings |
                                          ImGuiWindowFlags_NoBringToFrontOnFocus;

        ImGui::Begin("MainWindowHost", nullptr, main_win_flags);
        {
            // Global Header Bar: Title, User Profile, Language & Theme Toggles
            ImGui::TextColored(ImVec4(0.96f, 0.62f, 0.04f, 1.0f), "🏛 %s", arch::ui::tr(arch::ui::StringId::AppName));
            ImGui::SameLine();
            ImGui::TextDisabled(" |  v1.0.0");

            // Navigation tabs
            ImGui::SameLine();
            ImGui::Spacing();
            ImGui::SameLine();
            if (ImGui::SmallButton(arch::ui::tr(arch::ui::StringId::NavWelcome))) {
                current_screen = AppScreen::Welcome;
            }
            if (ImGui::IsItemHovered()) ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
            ImGui::SetItemTooltip(is_it ? "Torna alla schermata iniziale" : "Go to welcome screen");

            ImGui::SameLine();
            if (ImGui::SmallButton(arch::ui::tr(arch::ui::StringId::NavHistory))) {
                current_screen = AppScreen::History;
            }
            if (ImGui::IsItemHovered()) ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
            ImGui::SetItemTooltip(is_it ? "Visualizza la cronologia dei tornei" : "View tournament history");

            // Right-aligned controls: Language toggle, Theme toggle, Help button
            float right_controls_w = 300.0f;
            ImGui::SameLine(ImGui::GetContentRegionAvail().x - right_controls_w);

            // Language Toggle Button (Instant 0-restart re-render)
            const char* lang_btn_text = (is_it ? "IT [Lingua]" : "EN [Language]");
            if (ImGui::Button(lang_btn_text, ImVec2(95.0f, 26.0f))) {
                i18n.toggle_language();
                arch::data::UserPreferences prefs = profile_repo.get_active_profile().preferences;
                prefs.language = (i18n.current_language() == arch::ui::Language::IT)
                    ? arch::data::LanguagePref::IT
                    : arch::data::LanguagePref::EN;
                profile_repo.update_active_preferences(prefs);
            }
            if (ImGui::IsItemHovered()) ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
            ImGui::SetItemTooltip(is_it ? "Cambia lingua (L)" : "Toggle language (L)");

            ImGui::SameLine();

            // Theme Toggle Button (Light / Dark)
            bool is_light = (theme_mgr.current_theme() == arch::ui::Theme::Light);
            const char* theme_btn_text = is_light ? "☀ Chiaro" : "🌙 Scuro";
            if (ImGui::Button(theme_btn_text, ImVec2(75.0f, 26.0f))) {
                theme_mgr.toggle_theme();
                arch::data::UserPreferences prefs = profile_repo.get_active_profile().preferences;
                prefs.theme = (theme_mgr.current_theme() == arch::ui::Theme::Light)
                    ? arch::data::ThemePref::Light
                    : arch::data::ThemePref::Dark;
                profile_repo.update_active_preferences(prefs);
            }
            if (ImGui::IsItemHovered()) ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
            ImGui::SetItemTooltip(is_it ? "Cambia tema visivo (T)" : "Toggle visual theme (T)");

            ImGui::SameLine();

            // Help Button
            if (ImGui::Button("? Help [F1]", ImVec2(90.0f, 26.0f)) || open_shortcuts_modal) {
                ImGui::OpenPopup("KeyboardShortcutsModal");
            }
            if (ImGui::IsItemHovered()) ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
            ImGui::SetItemTooltip(is_it
                ? "Visualizza la guida alle scorciatoie da tastiera (F1)"
                : "Open keyboard shortcuts cheat sheet (F1)");

            ImGui::Separator();
            ImGui::Spacing();

            // Screen Dispatch
            switch (current_screen) {
                case AppScreen::Welcome: {
                    welcome_view.render(
                        profile_repo,
                        selected_mode,
                        is_it,
                        [&](arch::domain::TournamentMode mode) {
                            selected_mode = mode;
                            tournament = std::make_unique<arch::domain::TournamentStateMachine>(mode);
                            current_screen = AppScreen::Tournament;
                        }
                    );
                    break;
                }

                case AppScreen::Tournament: {
                    if (!tournament) {
                        current_screen = AppScreen::Welcome;
                        break;
                    }

                    tournament_view.render(
                        *tournament,
                        styles,
                        categories,
                        texture_manager,
                        is_it,
                        [&]() {
                            // Tournament Completed Callback
                            current_result = tournament->calculate_result(styles, categories);
                            const auto& prof = profile_repo.get_active_profile();
                            std::string mode_str = (tournament->mode() == arch::domain::TournamentMode::Full) ? "FULL" : "QUICK";

                            history_repo.add_record(
                                current_result,
                                prof.id,
                                prof.username,
                                mode_str,
                                styles,
                                categories
                            );

                            profile_repo.record_tournament_outcome(current_result, mode_str);
                            current_screen = AppScreen::Results;
                        }
                    );
                    break;
                }

                case AppScreen::Results: {
                    results_view.render(
                        current_result,
                        styles,
                        categories,
                        texture_manager,
                        is_it,
                        [&]() {
                            // On Restart
                            tournament.reset();
                            current_screen = AppScreen::Welcome;
                        },
                        [&]() {
                            // On View History
                            current_screen = AppScreen::History;
                        }
                    );
                    break;
                }

                case AppScreen::History: {
                    history_view.render(
                        history_repo,
                        is_it,
                        [&]() {
                            current_screen = AppScreen::Welcome;
                        }
                    );
                    break;
                }
            }

            // KeyboardShortcutsModal
            ImVec2 center = ImGui::GetMainViewport()->GetCenter();
            ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
            ImGui::SetNextWindowSize(ImVec2(520.0f, 0.0f));

            if (ImGui::BeginPopupModal("KeyboardShortcutsModal", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
                ImGui::TextColored(ImVec4(0.96f, 0.62f, 0.04f, 1.0f), "⌨ %s", arch::ui::tr(arch::ui::StringId::ShortcutsTitle));
                ImGui::Separator();
                ImGui::Spacing();

                if (ImGui::BeginTable("ShortcutsGuideTable", 2, ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_RowBg)) {
                    ImGui::TableSetupColumn(arch::ui::tr(arch::ui::StringId::ShortcutsKeyHeader), ImGuiTableColumnFlags_WidthFixed, 150.0f);
                    ImGui::TableSetupColumn(arch::ui::tr(arch::ui::StringId::ShortcutsActionHeader), ImGuiTableColumnFlags_WidthStretch);
                    ImGui::TableHeadersRow();

                    auto add_shortcut_row = [](const char* key, const char* desc) {
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::TextColored(ImVec4(0.96f, 0.62f, 0.04f, 1.0f), "%s", key);
                        ImGui::TableSetColumnIndex(1);
                        ImGui::TextUnformatted(desc);
                    };

                    add_shortcut_row("1  /  Left Arrow", arch::ui::tr(arch::ui::StringId::ShortcutsVoteA));
                    add_shortcut_row("2  /  Right Arrow", arch::ui::tr(arch::ui::StringId::ShortcutsVoteB));
                    add_shortcut_row("Ctrl + Z", arch::ui::tr(arch::ui::StringId::ShortcutsUndo));
                    add_shortcut_row("Ctrl + Y", arch::ui::tr(arch::ui::StringId::ShortcutsRedo));
                    add_shortcut_row("T", arch::ui::tr(arch::ui::StringId::ShortcutsTheme));
                    add_shortcut_row("L", arch::ui::tr(arch::ui::StringId::ShortcutsLang));
                    add_shortcut_row("Esc", arch::ui::tr(arch::ui::StringId::ShortcutsBack));
                    add_shortcut_row("F1", arch::ui::tr(arch::ui::StringId::ShortcutsHelp));

                    ImGui::EndTable();
                }

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                if (ImGui::Button(is_it ? "Chiudi" : "Close", ImVec2(120.0f, 32.0f)) || ImGui::IsKeyPressed(ImGuiKey_Escape)) {
                    ImGui::CloseCurrentPopup();
                }
                if (ImGui::IsItemHovered()) ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
                ImGui::EndPopup();
            }
        }
        ImGui::End();

        // Resolve frame clear color from theme
        float clear_color[4];
        if (theme_mgr.current_theme() == arch::ui::Theme::Light) {
            clear_color[0] = 0.984f; clear_color[1] = 0.976f; clear_color[2] = 0.961f; clear_color[3] = 1.0f; // #FBF9F5
        } else {
            clear_color[0] = 0.059f; clear_color[1] = 0.067f; clear_color[2] = 0.090f; clear_color[3] = 1.0f; // #0F1117
        }

        context.end_frame(true, clear_color);
    }

    // Persist active preferences on exit
    arch::data::UserPreferences final_prefs;
    final_prefs.tournament_mode = (selected_mode == arch::domain::TournamentMode::Quick)
        ? arch::data::TournamentModePref::Quick15
        : arch::data::TournamentModePref::Full45;
    final_prefs.language = (i18n.current_language() == arch::ui::Language::IT)
        ? arch::data::LanguagePref::IT
        : arch::data::LanguagePref::EN;
    final_prefs.theme = (theme_mgr.current_theme() == arch::ui::Theme::Light)
        ? arch::data::ThemePref::Light
        : arch::data::ThemePref::Dark;
    profile_repo.update_active_preferences(final_prefs);

    texture_manager.shutdown();
    context.shutdown();

    return 0;
}
