#include "welcome_view.hpp"
#include "architecture/localization.hpp"
#include "architecture/theme_manager.hpp"
#include "imgui.h"
#include <format>
#include <cstdio>

namespace arch::ui {

void WelcomeView::render(
    arch::data::ProfileRepository& profile_repo,
    arch::domain::TournamentMode& selected_mode,
    bool is_italian,
    std::function<void(arch::domain::TournamentMode)> on_start_tournament
) {
    auto& active_profile = profile_repo.get_active_profile();
    const auto& all_profiles = profile_repo.get_all_profiles();

    ImVec2 avail = ImGui::GetContentRegionAvail();

    // 1. Header Banner
    ImGui::Spacing();
    ImGui::TextColored(ImVec4(0.96f, 0.62f, 0.04f, 1.0f), "%s", tr(StringId::AppName));
    ImGui::TextDisabled("%s", tr(StringId::AppTagline));
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // 2. User Account & Profile Management Card
    ImGui::BeginChild("UserProfileCard", ImVec2(avail.x, 150.0f), ImGuiChildFlags_Border);
    {
        ImGui::TextColored(ImVec4(0.96f, 0.62f, 0.04f, 1.0f), "%s",
            is_italian ? "PROFILO UTENTE LOCALE (%APPDATA%)" : "LOCAL USER PROFILE (%APPDATA%)");
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::Columns(2, "ProfileCols", false);
        ImGui::SetColumnWidth(0, avail.x * 0.55f);

        // Active Profile Selector
        ImGui::Text("%s:", tr(StringId::ActiveUserLabel));
        ImGui::SameLine();
        std::string current_preview = active_profile.username + " (" + active_profile.id + ")";
        if (ImGui::BeginCombo("##UserSelect", current_preview.c_str())) {
            for (const auto& prof : all_profiles) {
                bool is_selected = (prof.id == active_profile.id);
                std::string item_label = prof.username + " [" + prof.id + "]";
                if (ImGui::Selectable(item_label.c_str(), is_selected)) {
                    profile_repo.switch_user(prof.id);
                }
                if (is_selected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }
        ImGui::SetItemTooltip(is_italian ? "Seleziona un profilo utente locale salvato" : "Select a saved local user profile");

        // New profile creation controls
        if (!show_create_input_) {
            if (ImGui::Button(is_italian ? "+ Nuovo Profilo" : "+ New Profile")) {
                show_create_input_ = true;
            }
            ImGui::SetItemTooltip(is_italian ? "Crea un nuovo account locale su questo computer" : "Create a new local account on this computer");
        } else {
            ImGui::InputText("##NewUsername", new_username_buf_, sizeof(new_username_buf_));
            ImGui::SameLine();
            if (ImGui::Button(is_italian ? "Crea" : "Create")) {
                if (new_username_buf_[0] != '\0') {
                    profile_repo.create_profile(new_username_buf_);
                    show_create_input_ = false;
                }
            }
            ImGui::SameLine();
            if (ImGui::Button(is_italian ? "Annulla" : "Cancel")) {
                show_create_input_ = false;
            }
        }

        ImGui::NextColumn();

        // Profile Statistics
        ImGui::TextDisabled("%s", is_italian ? "STATISTICHE UTENTE" : "USER STATISTICS");
        ImGui::Text("%s: %d",
            is_italian ? "Tornei Completati" : "Tournaments Completed",
            active_profile.stats.total_tournaments_completed
        );
        ImGui::Text("%s: %d  |  %s: %d",
            is_italian ? "Completi" : "Full",
            active_profile.stats.full_tournaments_completed,
            is_italian ? "Rapidi" : "Quick",
            active_profile.stats.quick_tournaments_completed
        );
        if (!active_profile.stats.dominant_archetype_id.empty()) {
            ImGui::TextColored(ImVec4(0.61f, 0.35f, 0.71f, 1.0f), "%s: %s",
                is_italian ? "Archetipo Dominante" : "Dominant Archetype",
                active_profile.stats.dominant_archetype_id.c_str()
            );
        }

        ImGui::Columns(1);
    }
    ImGui::EndChild();

    ImGui::Spacing();

    // 3. Tournament Mode Selection Card
    ImGui::BeginChild("ModeSelectCard", ImVec2(avail.x, 210.0f), ImGuiChildFlags_Border);
    {
        ImGui::TextColored(ImVec4(0.96f, 0.62f, 0.04f, 1.0f), "%s", tr(StringId::SelectModePrompt));
        ImGui::Separator();
        ImGui::Spacing();

        // Mode 1: Full Tournament (45 matches)
        bool is_full = (selected_mode == arch::domain::TournamentMode::Full);
        if (ImGui::RadioButton(tr(StringId::ModeFullTitle), is_full)) {
            selected_mode = arch::domain::TournamentMode::Full;
        }
        ImGui::SetItemTooltip(is_italian
            ? "Torneo completo di 45 confronti diretti: massima accuratezza e profondità statistica"
            : "Complete 45 round-robin comparisons: maximum statistical accuracy and depth");
        ImGui::Indent(28.0f);
        ImGui::TextWrapped("%s", tr(StringId::ModeFullDesc));
        ImGui::Unindent(28.0f);

        ImGui::Spacing();

        // Mode 2: Quick Profiling (15 matches)
        bool is_quick = (selected_mode == arch::domain::TournamentMode::Quick);
        if (ImGui::RadioButton(tr(StringId::ModeQuickTitle), is_quick)) {
            selected_mode = arch::domain::TournamentMode::Quick;
        }
        ImGui::SetItemTooltip(is_italian
            ? "Torneo rapido di 15 confronti selezionati: profilazione veloce ed efficace"
            : "Fast 15 selected comparisons: quick and effective aesthetic profiling");
        ImGui::Indent(28.0f);
        ImGui::TextWrapped("%s", tr(StringId::ModeQuickDesc));
        ImGui::Unindent(28.0f);
    }
    ImGui::EndChild();

    ImGui::Spacing();
    ImGui::Spacing();

    // 4. Large Action Button to Start Tournament
    float btn_width = 300.0f;
    float btn_height = 50.0f;
    ImGui::SetCursorPosX((avail.x - btn_width) * 0.5f);

    if (ImGui::Button(tr(StringId::BtnStartTournament), ImVec2(btn_width, btn_height))) {
        if (on_start_tournament) {
            on_start_tournament(selected_mode);
        }
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
    }
    ImGui::SetItemTooltip(is_italian
        ? "Avvia il torneo di confronto tra coppie di stili architettonici"
        : "Launch the pairwise architectural style comparison tournament");
}

} // namespace arch::ui
