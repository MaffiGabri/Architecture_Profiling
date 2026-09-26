#include "history_view.hpp"
#include "architecture/localization.hpp"
#include "architecture/theme_manager.hpp"
#include "imgui.h"
#include <cstdio>
#include <string>

namespace arch::ui {

void HistoryView::render(
    arch::data::HistoryRepository& history_repo,
    bool is_italian,
    std::function<void()> on_back_callback
) {
    const auto& records = history_repo.get_records();
    ImVec2 avail = ImGui::GetContentRegionAvail();

    // 1. Navigation / Action Header Bar
    ImGui::Spacing();
    if (ImGui::Button(is_italian ? " ⮌ Torna al Torneo" : " ⮌ Back to Tournament", ImVec2(180.0f, 36.0f))) {
        if (on_back_callback) on_back_callback();
    }
    if (ImGui::IsItemHovered()) ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
    ImGui::SetItemTooltip(is_italian ? "Ritorna alla schermata iniziale o al torneo" : "Return to welcome screen or tournament");

    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.96f, 0.62f, 0.04f, 1.0f), "  |  %s (%zu %s)",
        tr(StringId::HistoryTitle),
        records.size(),
        is_italian ? "registrazioni" : "runs"
    );

    if (!records.empty()) {
        ImGui::SameLine();
        float clear_btn_w = 170.0f;
        ImGui::SetCursorPosX(avail.x - clear_btn_w);
        if (ImGui::Button(tr(StringId::BtnClearHistory), ImVec2(clear_btn_w, 36.0f))) {
            ImGui::OpenPopup("ConfirmClearHistoryModal");
        }
        if (ImGui::IsItemHovered()) ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
        ImGui::SetItemTooltip(is_italian
            ? "Elimina tutti i tornei registrati (richiede conferma)"
            : "Delete all saved tournament records (requires confirmation)");
    }

    // Modal Confirmation Dialog for Clearing History
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(480.0f, 0.0f));

    if (ImGui::BeginPopupModal("ConfirmClearHistoryModal", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::TextColored(ImVec4(0.96f, 0.62f, 0.04f, 1.0f), "⚠ %s", tr(StringId::DialogClearHistoryTitle));
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::TextWrapped("%s", tr(StringId::DialogClearHistoryDesc));
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::Button(tr(StringId::BtnConfirmDelete), ImVec2(170.0f, 32.0f))) {
            history_repo.clear_history();
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button(tr(StringId::BtnCancel), ImVec2(120.0f, 32.0f)) || ImGui::IsKeyPressed(ImGuiKey_Escape)) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // 2. Empty State Handling
    if (records.empty()) {
        ImGui::BeginChild("EmptyHistoryCard", ImVec2(0, 160.0f), ImGuiChildFlags_Border);
        {
            ImGui::Spacing();
            ImGui::TextColored(ImVec4(0.96f, 0.62f, 0.04f, 1.0f), "%s", tr(StringId::HistoryEmptyTitle));
            ImGui::Spacing();
            ImGui::TextWrapped("%s", tr(StringId::HistoryEmptyDesc));
            ImGui::Spacing();
            if (ImGui::Button(is_italian ? "Inizia un Nuovo Torneo" : "Start a New Tournament", ImVec2(220.0f, 40.0f))) {
                if (on_back_callback) on_back_callback();
            }
            if (ImGui::IsItemHovered()) ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
            ImGui::SetItemTooltip(is_italian ? "Avvia un nuovo torneo" : "Launch a new tournament");
        }
        ImGui::EndChild();
        return;
    }

    // 3. Chronological Records List
    ImGui::BeginChild("HistoryScrollArea", ImVec2(0, 0), false, ImGuiWindowFlags_AlwaysVerticalScrollbar);
    {
        std::string record_to_delete;

        for (size_t i = 0; i < records.size(); ++i) {
            const auto& rec = records[i];
            ImGui::PushID(static_cast<int>(i));

            std::string child_id = "HistRecord_" + rec.id;
            ImGui::BeginChild(child_id.c_str(), ImVec2(0, 175.0f), ImGuiChildFlags_Border);
            {
                // Line 1: Timestamp, Mode, Champion, Delete Button
                std::string champ_name = (is_italian && !rec.champion_style_name_it.empty())
                    ? rec.champion_style_name_it : rec.champion_style_name_en;
                std::string cat_name = (is_italian && !rec.top_category_name_it.empty())
                    ? rec.top_category_name_it : rec.top_category_name_en;

                ImGui::TextColored(ImVec4(0.96f, 0.62f, 0.04f, 1.0f), "🏆 %s", champ_name.c_str());
                ImGui::SameLine();
                ImGui::TextDisabled("• %s • [%s]", rec.timestamp_iso.c_str(), rec.mode.c_str());

                // Delete button on the right
                ImGui::SameLine(ImGui::GetContentRegionAvail().x - 70.0f);
                if (ImGui::SmallButton(is_italian ? "Elimina" : "Delete")) {
                    record_to_delete = rec.id;
                }
                if (ImGui::IsItemHovered()) ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
                ImGui::SetItemTooltip(is_italian
                    ? "Rimuovi questo torneo dalla cronologia locale"
                    : "Remove this tournament run from local history");

                ImGui::Separator();
                ImGui::Spacing();

                // Line 2: Archetype & Tagline
                std::string arch_title = (is_italian && !rec.archetype_title_it.empty())
                    ? rec.archetype_title_it : rec.archetype_title_en;
                std::string arch_tagline = (is_italian && !rec.archetype_tagline_it.empty())
                    ? rec.archetype_tagline_it : rec.archetype_tagline_en;

                ImGui::TextColored(ImVec4(0.61f, 0.35f, 0.71f, 1.0f), "%s", arch_title.c_str());
                ImGui::SameLine();
                ImGui::TextDisabled("\"%s\"", arch_tagline.c_str());

                ImGui::Spacing();

                // Line 3: Metrics summary
                ImGui::Text("%s: %.1f%%  |  %s: %.3f  |  %s: %d  |  %s: +%.1f%%",
                    is_italian ? "Confidenza" : "Confidence", rec.confidence_pct,
                    is_italian ? "Coerenza (ζ)" : "Consistency (ζ)", rec.transitivity_zeta,
                    is_italian ? "Triadi" : "Triads", rec.circular_triads,
                    is_italian ? "Margine" : "Margin", rec.victory_margin * 100.0f
                );

                ImGui::Spacing();

                // Line 4: 5-Axis Coordinates
                ImGui::TextDisabled("%s: [%.1f, %.1f, %.1f, %.1f, %.1f] (%s)",
                    is_italian ? "Tratti 5D" : "5D Traits",
                    rec.trait_coordinates.era,
                    rec.trait_coordinates.ornamentation,
                    rec.trait_coordinates.structural_honesty,
                    rec.trait_coordinates.geometric_order,
                    rec.trait_coordinates.material_warmth,
                    cat_name.c_str()
                );
            }
            ImGui::EndChild();

            ImGui::PopID();
            ImGui::Spacing();
        }

        if (!record_to_delete.empty()) {
            history_repo.delete_record(record_to_delete);
        }
    }
    ImGui::EndChild();
}

} // namespace arch::ui
