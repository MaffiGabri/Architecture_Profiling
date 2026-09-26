#include "results_view.hpp"
#include "radar_chart.hpp"
#include "architecture/localization.hpp"
#include "architecture/theme_manager.hpp"
#include "imgui.h"
#include <cstdio>
#include <sstream>
#include <cmath>

namespace arch::ui {

static ImU32 parse_hex(const std::string& hex, float alpha = 1.0f) {
    if (hex.empty() || hex[0] != '#') return IM_COL32(74, 105, 132, static_cast<int>(alpha * 255.0f));
    uint32_t val = 0;
    std::stringstream ss;
    ss << std::hex << hex.substr(1);
    ss >> val;
    return IM_COL32((val >> 16) & 0xFF, (val >> 8) & 0xFF, val & 0xFF, static_cast<int>(alpha * 255.0f));
}

void ResultsView::render(
    const arch::domain::TournamentResult& result,
    const std::vector<arch::domain::Style>& styles,
    const std::vector<arch::domain::Category>& categories,
    const arch::platform::TextureManager& texture_manager,
    bool is_italian,
    std::function<void()> on_restart_callback,
    std::function<void()> on_view_history_callback
) {
    const arch::domain::Style* champion_style = nullptr;
    for (const auto& s : styles) {
        if (s.id == result.champion_style_id) {
            champion_style = &s;
            break;
        }
    }
    if (!champion_style && !styles.empty()) champion_style = &styles.front();

    const arch::domain::StyleScore* champion_score = nullptr;
    if (!result.style_rankings.empty()) {
        champion_score = &result.style_rankings.front();
    }

    // Scrollable container for the complete results overview
    ImGui::BeginChild("ResultsScrollArea", ImVec2(0, 0), false, ImGuiWindowFlags_AlwaysVerticalScrollbar);

    // 1. Multi-Factor Confidence & Consistency Badge
    render_confidence_badge(result, is_italian);
    ImGui::Spacing();

    // 2. #1 Champion Style Hero Banner
    if (champion_style && champion_score) {
        render_champion_hero(
            *champion_style,
            *champion_score,
            texture_manager.get_style_texture(champion_style->id),
            is_italian
        );
    }
    ImGui::Spacing();

    // 3. 110% Narrative Personality Archetype Card
    render_archetype_card(result.archetype, is_italian);
    ImGui::Spacing();

    // 4. Custom 5-Axis Spider/Radar Chart
    ImGui::BeginChild("RadarCard", ImVec2(0, 320.0f), ImGuiChildFlags_Border);
    {
        ImGui::TextColored(ImVec4(0.96f, 0.62f, 0.04f, 1.0f), "%s",
            is_italian ? "PROFILO ESTETICO 5-ASSI" : "5-AXIS AESTHETIC TRAIT PROFILE");
        ImGui::Separator();
        ImGui::Spacing();
        RenderRadarChart(result.user_traits, is_italian, ImVec2(0, 260.0f));
    }
    ImGui::EndChild();
    ImGui::Spacing();

    // 5. Category Affinity Breakdown Bars
    render_category_breakdown(result.category_rankings, categories, is_italian);
    ImGui::Spacing();

    // 6. Complete 1-to-10 Ranked Style Table
    render_style_rankings(result.style_rankings, styles, is_italian);
    ImGui::Spacing();

    // 7. Action Controls
    if (ImGui::Button(is_italian ? "Inizia Nuovo Torneo" : "Start New Tournament", ImVec2(220.0f, 44.0f))) {
        if (on_restart_callback) on_restart_callback();
    }
    if (ImGui::IsItemHovered()) ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
    ImGui::SetItemTooltip(is_italian
        ? "Ricomincia un nuovo torneo per profilare nuovamente i tuoi gusti"
        : "Start a new tournament to re-profile your architectural taste");

    ImGui::SameLine();
    if (ImGui::Button(is_italian ? "Cronologia Tornei" : "Tournament History", ImVec2(180.0f, 44.0f))) {
        if (on_view_history_callback) on_view_history_callback();
    }
    if (ImGui::IsItemHovered()) ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
    ImGui::SetItemTooltip(is_italian
        ? "Visualizza tutti i risultati dei tornei salvati"
        : "View all saved tournament run results");

    ImGui::Spacing();
    ImGui::EndChild();
}

void ResultsView::render_confidence_badge(const arch::domain::TournamentResult& result, bool is_italian) {
    ImGui::BeginChild("ConfidenceCard", ImVec2(0, 80.0f), ImGuiChildFlags_Border);
    {
        ImGui::Columns(4, "ConfidenceCols", false);

        // Column 1: Overall Confidence %
        ImGui::TextDisabled("%s", is_italian ? "CONFIDENZA TOTALE" : "OVERALL CONFIDENCE");
        ImGui::TextColored(ImVec4(0.96f, 0.62f, 0.04f, 1.0f), "%.1f%%", result.confidence_pct);
        if (ImGui::IsItemHovered()) {
            ImGui::SetItemTooltip(is_italian
                ? "Confidenza globale ponderata: combina coerenza decisionale, margine di vittoria e concentrazione di entropia"
                : "Overall weighted confidence: combines decision consistency, victory margin, and entropy concentration");
        }
        ImGui::NextColumn();

        // Column 2: Consistency Zeta
        ImGui::TextDisabled("%s", is_italian ? "COERENZA (ZETA)" : "CONSISTENCY (ZETA)");
        ImGui::Text("%.3f", result.consistency_zeta);
        if (ImGui::IsItemHovered()) {
            ImGui::SetItemTooltip(is_italian
                ? "Indice di transitività di Kendall (zeta in [0, 1]): 1.0 indica totale assenza di contraddizioni cicliche"
                : "Kendall transitivity index (zeta in [0, 1]): 1.0 indicates complete absence of circular contradictions");
        }
        ImGui::NextColumn();

        // Column 3: Circular Triads
        ImGui::TextDisabled("%s", is_italian ? "TRIADI CIRCOLARI" : "CIRCULAR TRIADS");
        ImGui::Text("%d", result.circular_triads);
        if (ImGui::IsItemHovered()) {
            ImGui::SetItemTooltip(is_italian
                ? "Numero di triadi circolari (A > B > C > A): indica preferenze cicliche o contrastanti"
                : "Number of circular triads (A > B > C > A): indicates conflicting or cyclic preferences");
        }
        ImGui::NextColumn();

        // Column 4: Victory Margin
        ImGui::TextDisabled("%s", is_italian ? "MARGINE VITTORIA" : "VICTORY MARGIN");
        ImGui::Text("+%.1f%%", result.victory_margin * 100.0f);
        if (ImGui::IsItemHovered()) {
            ImGui::SetItemTooltip(is_italian
                ? "Differenza di affinità tra il 1° classificato e il 2° classificato"
                : "Affinity lead of the #1 champion style over the runner-up");
        }

        ImGui::Columns(1);
    }
    ImGui::EndChild();
}

void ResultsView::render_champion_hero(
    const arch::domain::Style& champion_style,
    const arch::domain::StyleScore& champion_score,
    const arch::platform::TextureResource& texture,
    bool is_italian
) {
    ImGui::BeginChild("ChampionHero", ImVec2(0, 200.0f), ImGuiChildFlags_Border);
    {
        float t = static_cast<float>(ImGui::GetTime());
        float pulse = (std::sin(t * 3.5f) + 1.0f) * 0.5f; // [0.0, 1.0]

        ImVec2 hero_min = ImGui::GetWindowPos();
        ImVec2 hero_max(hero_min.x + ImGui::GetWindowWidth(), hero_min.y + 200.0f);

        ImDrawList* dl = ImGui::GetWindowDrawList();

        // Pulsing golden outer glow
        ImU32 gold_glow = IM_COL32(245, 158 + static_cast<int>(pulse * 40.0f), 11, 180 + static_cast<int>(pulse * 75.0f));
        dl->AddRect(hero_min, hero_max, gold_glow, 8.0f, 0, 2.5f + pulse * 1.5f);

        // Sparkle celebration particles around title badge
        ImVec2 sparkle_center(hero_min.x + 360.0f, hero_min.y + 24.0f);
        for (int i = 0; i < 8; ++i) {
            float angle = (static_cast<float>(i) * 45.0f) * (3.14159265f / 180.0f) + t * 0.9f;
            float dist = 28.0f + 8.0f * std::sin(t * 2.5f + static_cast<float>(i));
            float px = sparkle_center.x + std::cos(angle) * dist;
            float py = sparkle_center.y + std::sin(angle) * (dist * 0.35f);
            float alpha = (std::sin(t * 3.0f + static_cast<float>(i)) + 1.0f) * 0.5f;
            dl->AddCircleFilled(ImVec2(px, py), 2.2f, IM_COL32(255, 215, 0, static_cast<int>(alpha * 220.0f)));
        }

        float img_width = 240.0f;
        float img_height = img_width * (3.0f / 4.0f); // 180px

        // Left Side: 4:3 Image
        ImVec2 p = ImGui::GetCursorScreenPos();
        if (texture.is_valid()) {
            dl->AddImageRounded(
                texture.texture_id(),
                p, ImVec2(p.x + img_width, p.y + img_height),
                ImVec2(0, 0), ImVec2(1, 1),
                IM_COL32_WHITE, 8.0f
            );
        } else {
            dl->AddRectFilled(p, ImVec2(p.x + img_width, p.y + img_height), IM_COL32(50, 56, 66, 255), 8.0f);
        }

        ImGui::SetCursorPosX(img_width + 24.0f);
        ImGui::BeginGroup();
        {
            ImGui::TextColored(ImVec4(0.96f, 0.62f, 0.04f, 1.0f), "%s",
                is_italian ? "🏆 STILE ARCHITETTONICO CAMPIONE" : "🏆 CHAMPION ARCHITECTURAL STYLE");
            ImGui::Spacing();
            ImGui::TextUnformatted(champion_style.title.c_str());

            std::string sub = (is_italian && !champion_style.style_it.empty()) ? champion_style.style_it : champion_style.style_en;
            ImGui::TextDisabled("%s • %s", sub.c_str(), champion_style.era_century.c_str());
            ImGui::Spacing();

            char record_buf[128];
            std::snprintf(record_buf, sizeof(record_buf), "%dW - %dL  |  %.1f%% %s",
                champion_score.wins, champion_score.losses,
                champion_score.probability * 100.0f,
                is_italian ? "Affinità Stimata" : "Bradley-Terry Win Probability"
            );
            ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.4f, 1.0f), "%s", record_buf);
        }
        ImGui::EndGroup();
    }
    ImGui::EndChild();
}

void ResultsView::render_archetype_card(const arch::domain::Archetype& archetype, bool is_italian) {
    ImGui::BeginChild("ArchetypeCard", ImVec2(0, 160.0f), ImGuiChildFlags_Border);
    {
        ImGui::TextColored(ImVec4(0.61f, 0.35f, 0.71f, 1.0f), "%s",
            is_italian ? "ARCHETIPO DELLA PERSONALITÀ (FUNZIONALITÀ 110%)" : "PERSONALITY ARCHETYPE (110% FEATURE)");
        ImGui::Separator();
        ImGui::Spacing();

        std::string title = (is_italian && !archetype.title_it.empty()) ? archetype.title_it : archetype.title_en;
        std::string tagline = (is_italian && !archetype.tagline_it.empty()) ? archetype.tagline_it : archetype.tagline_en;
        std::string narrative = (is_italian && !archetype.narrative_it.empty()) ? archetype.narrative_it : archetype.narrative_en;

        ImGui::TextUnformatted(title.c_str());
        ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "\"%s\"", tagline.c_str());
        ImGui::Spacing();
        ImGui::TextWrapped("%s", narrative.c_str());
    }
    ImGui::EndChild();
}

void ResultsView::render_category_breakdown(
    const std::vector<arch::domain::CategoryScore>& category_rankings,
    const std::vector<arch::domain::Category>& categories,
    bool is_italian
) {
    ImGui::BeginChild("CategoryBreakdown", ImVec2(0, 0), ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeY);
    {
        ImGui::TextColored(ImVec4(0.96f, 0.62f, 0.04f, 1.0f), "%s",
            is_italian ? "DISTRIBUZIONE DI AFFINITÀ PER CATEGORIA" : "CATEGORY AFFINITY BREAKDOWN");
        ImGui::Separator();
        ImGui::Spacing();

        ImDrawList* dl = ImGui::GetWindowDrawList();
        float bar_width = ImGui::GetContentRegionAvail().x;

        for (const auto& cs : category_rankings) {
            const arch::domain::Category* cat = nullptr;
            for (const auto& c : categories) {
                if (c.id == cs.category_id) { cat = &c; break; }
            }
            std::string name = cat ? ((is_italian && !cat->name_it.empty()) ? cat->name_it : cat->name_en) : cs.category_id;
            std::string hex_col = cat ? cat->color_hex : "#4A6984";

            ImGui::Text("%s", name.c_str());
            ImGui::SameLine(bar_width - 60.0f);
            ImGui::Text("%.1f%%", cs.percentage);

            ImVec2 bar_p = ImGui::GetCursorScreenPos();
            float fill_w = bar_width * static_cast<float>(cs.percentage / 100.0);

            // Track background
            dl->AddRectFilled(bar_p, ImVec2(bar_p.x + bar_width, bar_p.y + 8.0f), IM_COL32(40, 45, 55, 255), 4.0f);
            // Filled bar with category color
            dl->AddRectFilled(bar_p, ImVec2(bar_p.x + fill_w, bar_p.y + 8.0f), parse_hex(hex_col), 4.0f);

            ImGui::Dummy(ImVec2(bar_width, 14.0f));
        }
    }
    ImGui::EndChild();
}

void ResultsView::render_style_rankings(
    const std::vector<arch::domain::StyleScore>& style_rankings,
    const std::vector<arch::domain::Style>& styles,
    bool is_italian
) {
    ImGui::BeginChild("RankingsCard", ImVec2(0, 0), ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeY);
    {
        ImGui::TextColored(ImVec4(0.96f, 0.62f, 0.04f, 1.0f), "%s",
            is_italian ? "CLASSIFICA COMPLETA DEGLI STILI" : "COMPLETE STYLE RANKINGS");
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::BeginTable("StylesRankTable", 5, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerH)) {
            ImGui::TableSetupColumn(is_italian ? "Pos" : "Rank", ImGuiTableColumnFlags_WidthFixed, 55.0f);
            ImGui::TableSetupColumn(is_italian ? "Stile" : "Style Title", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn(is_italian ? "Epoca" : "Era", ImGuiTableColumnFlags_WidthFixed, 140.0f);
            ImGui::TableSetupColumn(is_italian ? "Record" : "Score", ImGuiTableColumnFlags_WidthFixed, 80.0f);
            ImGui::TableSetupColumn(is_italian ? "Probabilità" : "Affinity", ImGuiTableColumnFlags_WidthFixed, 80.0f);
            ImGui::TableHeadersRow();

            for (const auto& score : style_rankings) {
                const arch::domain::Style* s = nullptr;
                for (const auto& item : styles) {
                    if (item.id == score.style_id) { s = &item; break; }
                }

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                if (score.rank == 1) {
                    ImGui::TextColored(ImVec4(0.96f, 0.62f, 0.04f, 1.0f), "🥇 #1");
                } else if (score.rank == 2) {
                    ImGui::TextColored(ImVec4(0.75f, 0.75f, 0.80f, 1.0f), "🥈 #2");
                } else if (score.rank == 3) {
                    ImGui::TextColored(ImVec4(0.80f, 0.50f, 0.20f, 1.0f), "🥉 #3");
                } else {
                    ImGui::Text("#%d", score.rank);
                }

                ImGui::TableSetColumnIndex(1);
                ImGui::TextUnformatted(s ? s->title.c_str() : "Unknown");

                ImGui::TableSetColumnIndex(2);
                ImGui::TextDisabled("%s", s ? s->era_century.c_str() : "-");

                ImGui::TableSetColumnIndex(3);
                ImGui::Text("%dW - %dL", score.wins, score.losses);

                ImGui::TableSetColumnIndex(4);
                ImGui::TextColored(ImVec4(0.96f, 0.62f, 0.04f, 1.0f), "%.1f%%", score.percentage);
                if (ImGui::IsItemHovered()) {
                    ImGui::SetItemTooltip(is_italian
                        ? "Affinità percentuale calcolata tramite massima verosimiglianza Bradley-Terry"
                        : "Percentage affinity calculated via Bradley-Terry maximum likelihood estimation");
                }
            }
            ImGui::EndTable();
        }
    }
    ImGui::EndChild();
}

} // namespace arch::ui
