#include "tournament_view.hpp"
#include "architecture/localization.hpp"
#include "architecture/theme_manager.hpp"
#include "imgui.h"
#include <cstdio>
#include <sstream>

namespace arch::ui {

static ImU32 parse_hex_color(const std::string& hex, float alpha = 1.0f) {
    if (hex.empty() || hex[0] != '#') return IM_COL32(74, 105, 132, static_cast<int>(alpha * 255.0f));
    uint32_t val = 0;
    std::stringstream ss;
    ss << std::hex << hex.substr(1);
    ss >> val;
    int r = (val >> 16) & 0xFF;
    int g = (val >> 8) & 0xFF;
    int b = val & 0xFF;
    return IM_COL32(r, g, b, static_cast<int>(alpha * 255.0f));
}

static std::function<void()> g_abandon_callback;

void set_tournament_abandon_callback(std::function<void()> cb) {
    g_abandon_callback = std::move(cb);
}

void TournamentView::render(
    arch::domain::TournamentStateMachine& tournament,
    const std::vector<arch::domain::Style>& styles,
    const std::vector<arch::domain::Category>& categories,
    const arch::platform::TextureManager& texture_manager,
    bool is_italian,
    std::function<void()> on_complete_callback
) {
    if (tournament.is_complete()) {
        if (on_complete_callback) on_complete_callback();
        return;
    }

    const auto& match = tournament.current_match();
    const arch::domain::Style* left_style = nullptr;
    const arch::domain::Style* right_style = nullptr;
    for (const auto& s : styles) {
        if (s.id == match.left_style_id) left_style = &s;
        if (s.id == match.right_style_id) right_style = &s;
    }
    if (!left_style || !right_style) return;

    const arch::domain::Category* left_cat = nullptr;
    const arch::domain::Category* right_cat = nullptr;
    for (const auto& c : categories) {
        if (c.id == left_style->category_id) left_cat = &c;
        if (c.id == right_style->category_id) right_cat = &c;
    }

    // Keyboard Shortcuts: 1 or Left Arrow for Left; 2 or Right Arrow for Right; Esc for Abandon
    if (ImGui::IsKeyPressed(ImGuiKey_1) || ImGui::IsKeyPressed(ImGuiKey_LeftArrow)) {
        tournament.vote(left_style->id);
    } else if (ImGui::IsKeyPressed(ImGuiKey_2) || ImGui::IsKeyPressed(ImGuiKey_RightArrow)) {
        tournament.vote(right_style->id);
    } else if ((ImGui::IsKeyDown(ImGuiKey_LeftCtrl) || ImGui::IsKeyDown(ImGuiKey_RightCtrl)) && ImGui::IsKeyPressed(ImGuiKey_Z)) {
        if (tournament.can_undo()) tournament.undo();
    } else if ((ImGui::IsKeyDown(ImGuiKey_LeftCtrl) || ImGui::IsKeyDown(ImGuiKey_RightCtrl)) && ImGui::IsKeyPressed(ImGuiKey_Y)) {
        if (tournament.can_redo()) tournament.redo();
    } else if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
        if (tournament.current_index() > 0) {
            ImGui::OpenPopup("ConfirmAbandonModal");
        } else {
            if (g_abandon_callback) g_abandon_callback();
        }
    }

    // 1. Navigation & Progress Bar
    render_navigation_bar(tournament, is_italian);

    ImGui::Separator();
    ImGui::Spacing();

    // 2. Responsive Side-by-Side Comparison Layout
    ImVec2 avail = ImGui::GetContentRegionAvail();
    float vs_width = 50.0f;
    float spacing = ImGui::GetStyle().ItemSpacing.x;
    float card_width = (avail.x - vs_width - spacing * 2.0f) * 0.5f;
    if (card_width < 300.0f) card_width = 300.0f;

    auto vote_callback = [&](int winner_id) {
        tournament.vote(winner_id);
    };

    // Left Card (Option A)
    render_comparison_card(
        *left_style, left_cat,
        texture_manager.get_style_texture(left_style->id),
        true, card_width, vote_callback, is_italian
    );

    ImGui::SameLine();

    // Central "VS" Badge
    ImGui::BeginGroup();
    {
        float group_y = ImGui::GetCursorPosY() + 180.0f;
        ImGui::SetCursorPosY(group_y);
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImVec2 p = ImGui::GetCursorScreenPos();
        ImVec2 center(p.x + vs_width * 0.5f, p.y + 20.0f);
        draw_list->AddCircleFilled(center, 22.0f, ImGui::GetColorU32(ImGuiCol_FrameBg), 24);
        draw_list->AddCircle(center, 22.0f, ImGui::GetColorU32(ImGuiCol_Border), 24, 1.5f);
        const char* vs_text = "VS";
        ImVec2 text_sz = ImGui::CalcTextSize(vs_text);
        draw_list->AddText(ImVec2(center.x - text_sz.x * 0.5f, center.y - text_sz.y * 0.5f), ImGui::GetColorU32(ImGuiCol_TextDisabled), vs_text);
        ImGui::Dummy(ImVec2(vs_width, 44.0f));
    }
    ImGui::EndGroup();

    ImGui::SameLine();

    // Right Card (Option B)
    render_comparison_card(
        *right_style, right_cat,
        texture_manager.get_style_texture(right_style->id),
        false, card_width, vote_callback, is_italian
    );
}

void TournamentView::render_navigation_bar(
    arch::domain::TournamentStateMachine& tournament,
    bool is_italian
) {
    ImVec2 avail = ImGui::GetContentRegionAvail();
    int current_match_num = tournament.current_index() + 1;
    int total_matches = tournament.total_matches();
    float progress = (total_matches > 0)
        ? static_cast<float>(tournament.current_index()) / static_cast<float>(total_matches)
        : 0.0f;

    // Left: Exit / Abandon button & Undo / Redo buttons
    if (ImGui::Button(is_italian ? " ⮌ Esci " : " ⮌ Exit ")) {
        if (tournament.current_index() > 0) {
            ImGui::OpenPopup("ConfirmAbandonModal");
        } else {
            if (g_abandon_callback) g_abandon_callback();
        }
    }
    ImGui::SetItemTooltip(is_italian
        ? "Esci dal torneo e torna al menu iniziale"
        : "Exit tournament and return to welcome screen");

    ImGui::SameLine();
    ImGui::BeginDisabled(!tournament.can_undo());
    if (ImGui::Button(is_italian ? " ⮌ Annulla " : " ⮌ Undo ")) {
        tournament.undo();
    }
    ImGui::EndDisabled();
    ImGui::SetItemTooltip(is_italian
        ? "Annulla l'ultimo voto espresso (Ctrl+Z)"
        : "Undo last recorded match vote (Ctrl+Z)");

    ImGui::SameLine();
    ImGui::BeginDisabled(!tournament.can_redo());
    if (ImGui::Button(is_italian ? " ⮎ Ripeti " : " ⮎ Redo ")) {
        tournament.redo();
    }
    ImGui::EndDisabled();
    ImGui::SetItemTooltip(is_italian
        ? "Ripeti il voto precedentemente annullato (Ctrl+Y)"
        : "Redo previously undone match vote (Ctrl+Y)");

    ImGui::SameLine();
    ImGui::Spacing();
    ImGui::SameLine();

    // Mode Pill Badge
    const char* mode_str = (tournament.mode() == arch::domain::TournamentMode::Full)
        ? (is_italian ? "Torneo Completo (45)" : "Full Tournament (45)")
        : (is_italian ? "Torneo Rapido (15)" : "Quick Tournament (15)");
    ImGui::TextColored(ImVec4(0.96f, 0.62f, 0.04f, 1.0f), "[ %s ]", mode_str);
    ImGui::SetItemTooltip(is_italian
        ? "Formato del torneo attivo"
        : "Active tournament format");

    ImGui::SameLine();
    float counter_width = 220.0f;
    ImGui::SetCursorPosX(avail.x - counter_width);

    // Match Counter Text
    char match_buf[64];
    if (is_italian) {
        std::snprintf(match_buf, sizeof(match_buf), "Confronto %d di %d (%.0f%%)",
            current_match_num, total_matches, progress * 100.0f);
    } else {
        std::snprintf(match_buf, sizeof(match_buf), "Match %d of %d (%.0f%%)",
            current_match_num, total_matches, progress * 100.0f);
    }
    ImGui::TextUnformatted(match_buf);

    // Progress Bar
    ImGui::ProgressBar(progress, ImVec2(avail.x, 8.0f), "");

    // Abandon confirmation modal
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(480.0f, 0.0f));

    if (ImGui::BeginPopupModal("ConfirmAbandonModal", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::TextColored(ImVec4(0.96f, 0.62f, 0.04f, 1.0f), "⚠ %s", tr(StringId::DialogAbandonTitle));
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::TextWrapped("%s", tr(StringId::DialogAbandonDesc));
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::Button(tr(StringId::BtnResumeTournament), ImVec2(160.0f, 32.0f))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button(tr(StringId::BtnAbandonTournament), ImVec2(160.0f, 32.0f))) {
            ImGui::CloseCurrentPopup();
            if (g_abandon_callback) g_abandon_callback();
        }
        ImGui::EndPopup();
    }
}

void TournamentView::render_comparison_card(
    const arch::domain::Style& style,
    const arch::domain::Category* category,
    const arch::platform::TextureResource& texture,
    bool is_left,
    float card_width,
    std::function<void(int)> on_vote,
    bool is_italian
) {
    ImGui::PushID(style.id);
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    ImGuiChildFlags child_flags = ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeY;
    ImGuiWindowFlags win_flags = ImGuiWindowFlags_NoScrollbar;

    std::string child_id = is_left ? "Card_Left" : "Card_Right";
    ImGui::BeginChild(child_id.c_str(), ImVec2(card_width, 0), child_flags, win_flags);

    ImVec2 card_min = ImGui::GetWindowPos();
    ImVec2 card_size = ImGui::GetWindowSize();
    ImVec2 card_max(card_min.x + card_size.x, card_min.y + card_size.y);

    bool is_hovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows);
    if (is_hovered) {
        ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
        // Option A (Left) = Warm Amber, Option B (Right) = Royal Blue
        ImU32 glow_color = is_left ? IM_COL32(245, 158, 11, 230) : IM_COL32(59, 130, 246, 230);
        draw_list->AddRect(
            ImVec2(card_min.x - 2.0f, card_min.y - 2.0f),
            ImVec2(card_max.x + 2.0f, card_max.y + 2.0f),
            glow_color,
            ImGui::GetStyle().ChildRounding,
            0, 2.5f
        );

        ImGui::SetItemTooltip(is_left
            ? (is_italian ? "Clicca o premi [1] / [Freccia Sinistra] per votare questo stile" : "Click or press [1] / [Left Arrow] to vote for this style")
            : (is_italian ? "Clicca o premi [2] / [Freccia Destra] per votare questo stile" : "Click or press [2] / [Right Arrow] to vote for this style")
        );
    }

    // 1. Aspect-Ratio Preserved Image Thumbnail (4:3)
    float inner_width = ImGui::GetContentRegionAvail().x;
    float img_height = inner_width * (3.0f / 4.0f); // 800x600 -> 4:3
    ImVec2 img_pos = ImGui::GetCursorScreenPos();
    ImVec2 img_end(img_pos.x + inner_width, img_pos.y + img_height);

    if (texture.is_valid()) {
        draw_list->AddImageRounded(
            texture.texture_id(),
            img_pos, img_end,
            ImVec2(0, 0), ImVec2(1, 1),
            IM_COL32_WHITE,
            8.0f,
            ImDrawFlags_RoundCornersTop
        );
    } else {
        // Fallback colored rect
        draw_list->AddRectFilled(img_pos, img_end, IM_COL32(50, 56, 66, 255), 8.0f, ImDrawFlags_RoundCornersTop);
    }

    // 2. Era Century Overlay Badge (Top-Left of Image)
    {
        ImVec2 badge_pos(img_pos.x + 8.0f, img_pos.y + 8.0f);
        std::string era_str = style.era_century;
        ImVec2 text_sz = ImGui::CalcTextSize(era_str.c_str());
        ImVec2 badge_max(badge_pos.x + text_sz.x + 12.0f, badge_pos.y + text_sz.y + 6.0f);

        draw_list->AddRectFilled(badge_pos, badge_max, IM_COL32(0, 0, 0, 180), 4.0f);
        draw_list->AddText(ImVec2(badge_pos.x + 6.0f, badge_pos.y + 3.0f), IM_COL32(240, 240, 240, 255), era_str.c_str());
    }

    // 3. Option Badge (Top-Right of Image)
    {
        const char* opt_text = is_left ? (is_italian ? "OPZIONE A [1]" : "OPTION A [1]")
                                       : (is_italian ? "OPZIONE B [2]" : "OPTION B [2]");
        ImVec2 text_sz = ImGui::CalcTextSize(opt_text);
        ImVec2 badge_max(img_end.x - 8.0f, img_pos.y + 8.0f + text_sz.y + 6.0f);
        ImVec2 badge_pos(badge_max.x - text_sz.x - 12.0f, img_pos.y + 8.0f);

        ImU32 badge_col = is_left ? IM_COL32(245, 158, 11, 230) : IM_COL32(59, 130, 246, 230);
        draw_list->AddRectFilled(badge_pos, badge_max, badge_col, 4.0f);
        draw_list->AddText(ImVec2(badge_pos.x + 6.0f, badge_pos.y + 3.0f), IM_COL32(255, 255, 255, 255), opt_text);
    }

    // Advance cursor past the image
    ImGui::Dummy(ImVec2(inner_width, img_height));
    ImGui::Spacing();

    // 4. Style Title & Dual-Language Subtitle
    ImGui::TextUnformatted(style.title.c_str());
    std::string subtitle = (is_italian && !style.style_it.empty()) ? style.style_it : style.style_en;
    ImGui::TextColored(ImVec4(0.6f, 0.65f, 0.75f, 1.0f), "%s", subtitle.c_str());

    ImGui::Spacing();

    // 5. Tinted Category Pill Badge
    if (category) {
        std::string cat_name = (is_italian && !category->name_it.empty()) ? category->name_it : category->name_en;
        ImU32 cat_bg = parse_hex_color(category->color_hex, 0.22f);
        ImU32 cat_border = parse_hex_color(category->color_hex, 0.80f);
        ImU32 cat_text = parse_hex_color(category->color_hex, 1.00f);

        ImVec2 pill_pos = ImGui::GetCursorScreenPos();
        ImVec2 text_sz = ImGui::CalcTextSize(cat_name.c_str());
        ImVec2 pill_end(pill_pos.x + text_sz.x + 16.0f, pill_pos.y + text_sz.y + 8.0f);

        draw_list->AddRectFilled(pill_pos, pill_end, cat_bg, 12.0f);
        draw_list->AddRect(pill_pos, pill_end, cat_border, 12.0f, 0, 1.0f);
        draw_list->AddText(ImVec2(pill_pos.x + 8.0f, pill_pos.y + 4.0f), cat_text, cat_name.c_str());

        ImGui::Dummy(ImVec2(text_sz.x + 16.0f, text_sz.y + 8.0f));
        if (ImGui::IsItemHovered()) {
            std::string cat_desc = (is_italian && !category->description_it.empty()) ? category->description_it : category->description_en;
            ImGui::SetItemTooltip("%s", cat_desc.c_str());
        }
    }

    ImGui::Spacing();

    // 6. Trait Tag Chips
    ImGui::BeginGroup();
    int tag_count = 0;
    for (const auto& tag : style.tags) {
        if (tag_count++ >= 3) break; // Display top 3 tags
        std::string display_tag = "#" + tag;
        for (char& c : display_tag) if (c == '_') c = ' ';

        ImGui::SameLine();
        ImGui::TextDisabled("%s", display_tag.c_str());
    }
    ImGui::EndGroup();

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // 7. Large Interactive Vote Button
    std::string btn_label = is_left ? (is_italian ? "VOTA PER L'OPZIONE A" : "VOTE FOR OPTION A")
                                    : (is_italian ? "VOTA PER L'OPZIONE B" : "VOTE FOR OPTION B");
    if (ImGui::Button(btn_label.c_str(), ImVec2(inner_width, 42.0f))) {
        on_vote(style.id);
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
    }
    ImGui::SetItemTooltip(is_left
        ? (is_italian ? "Vota Opzione A [Tasto 1 / Freccia Sinistra]" : "Vote Option A [Key 1 / Left Arrow]")
        : (is_italian ? "Vota Opzione B [Tasto 2 / Freccia Destra]" : "Vote Option B [Key 2 / Right Arrow]"));

    // Clicking anywhere on the card container also registers vote
    if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
        on_vote(style.id);
    }

    ImGui::EndChild();
    ImGui::PopID();
}

} // namespace arch::ui
