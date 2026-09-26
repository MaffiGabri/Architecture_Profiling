#include "radar_chart.hpp"
#include <cmath>
#include <array>
#include <algorithm>
#include <cstdio>

namespace arch::ui {

constexpr int NUM_AXES = 5;
constexpr double PI = 3.14159265358979323846;
constexpr double ANGLE_STEP = 2.0 * PI / 5.0; // 72 degrees in radians

// Trigonometric vertex angles in radians:
// -90°, -18°, +54°, +126°, +198°
static constexpr std::array<double, NUM_AXES> AXIS_ANGLES = {
    -PI / 2.0,                  // -90°  (Axis 0: Era / Epoca)
    -PI / 2.0 + ANGLE_STEP,      // -18°  (Axis 1: Ornamentation / Ornamentazione)
    -PI / 2.0 + 2 * ANGLE_STEP,  // +54°  (Axis 2: Structural Honesty / Onestà Strutturale)
    -PI / 2.0 + 3 * ANGLE_STEP,  // +126° (Axis 3: Geometric Order / Ordine Geometrico)
    -PI / 2.0 + 4 * ANGLE_STEP   // +198° (Axis 4: Material Warmth / Calore Materiali)
};

static const std::array<const char*, NUM_AXES> LABELS_EN = {
    "Era",
    "Ornamentation",
    "Structural Honesty",
    "Geometric Order",
    "Material Warmth"
};

static const std::array<const char*, NUM_AXES> LABELS_IT = {
    "Epoca",
    "Ornamentazione",
    "Onestà Strutturale",
    "Ordine Geometrico",
    "Calore Materiali"
};

void RenderRadarChart(
    const arch::domain::TraitRadar& traits,
    bool is_italian,
    ImVec2 size,
    ImU32 accent_color,
    ImU32 grid_color,
    ImU32 text_color
) {
    ImVec2 avail = ImGui::GetContentRegionAvail();
    if (size.x <= 0.0f) size.x = avail.x;

    // Reserve layout space in the ImGui layout stream
    ImVec2 cursor_pos = ImGui::GetCursorScreenPos();
    ImGui::Dummy(size);

    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 center(cursor_pos.x + size.x * 0.5f, cursor_pos.y + size.y * 0.5f);

    // Padding for outer text labels
    float label_padding = 54.0f;
    float radius = (std::min(size.x, size.y) * 0.5f) - label_padding;
    if (radius < 20.0f) return;

    // Resolve color themes dynamically if not explicitly passed
    if (accent_color == 0) accent_color = ImGui::GetColorU32(ImGuiCol_CheckMark);
    if (grid_color == 0) grid_color = ImGui::GetColorU32(ImGuiCol_Border);
    if (text_color == 0) text_color = ImGui::GetColorU32(ImGuiCol_Text);

    // 1. Draw 5-Level Concentric Pentagonal Web Grid (Levels: 0.2, 0.4, 0.6, 0.8, 1.0)
    constexpr std::array<float, 5> levels = {0.2f, 0.4f, 0.6f, 0.8f, 1.0f};
    for (float lvl : levels) {
        float r_lvl = radius * lvl;
        ImVec2 grid_pts[NUM_AXES];
        for (int i = 0; i < NUM_AXES; ++i) {
            double a = AXIS_ANGLES[i];
            grid_pts[i] = ImVec2(
                center.x + static_cast<float>(r_lvl * std::cos(a)),
                center.y + static_cast<float>(r_lvl * std::sin(a))
            );
        }
        float thickness = (lvl == 1.0f) ? 1.5f : 1.0f;
        draw_list->AddPolyline(grid_pts, NUM_AXES, grid_color, ImDrawFlags_Closed, thickness);
    }

    // 2. Draw Radial Spokes and Outer Trait Labels
    const auto& labels = is_italian ? LABELS_IT : LABELS_EN;
    std::array<double, NUM_AXES> trait_vals = traits.to_array();

    for (int i = 0; i < NUM_AXES; ++i) {
        double a = AXIS_ANGLES[i];
        float cos_a = static_cast<float>(std::cos(a));
        float sin_a = static_cast<float>(std::sin(a));

        ImVec2 spoke_end(center.x + radius * cos_a, center.y + radius * sin_a);
        draw_list->AddLine(center, spoke_end, grid_color, 1.0f);

        // Outer text label positioning
        float label_dist = radius + 22.0f;
        float label_x = center.x + label_dist * cos_a;
        float label_y = center.y + label_dist * sin_a;

        char val_buf[32];
        std::snprintf(val_buf, sizeof(val_buf), "%.1f", trait_vals[i]);

        ImVec2 title_sz = ImGui::CalcTextSize(labels[i]);
        ImVec2 val_sz = ImGui::CalcTextSize(val_buf);

        // Center multi-line text block
        float block_h = title_sz.y + val_sz.y + 2.0f;
        ImVec2 pos_title(label_x - title_sz.x * 0.5f, label_y - block_h * 0.5f);
        ImVec2 pos_val(label_x - val_sz.x * 0.5f, pos_title.y + title_sz.y + 2.0f);

        draw_list->AddText(pos_title, text_color, labels[i]);
        draw_list->AddText(pos_val, accent_color, val_buf);
    }

    // 3. User Trait Polygon Fill and Stroke
    ImVec2 data_pts[NUM_AXES];
    for (int i = 0; i < NUM_AXES; ++i) {
        double a = AXIS_ANGLES[i];
        float norm_val = std::clamp(static_cast<float>(trait_vals[i] / 10.0), 0.0f, 1.0f);
        float r_data = radius * norm_val;
        data_pts[i] = ImVec2(
            center.x + static_cast<float>(r_data * std::cos(a)),
            center.y + static_cast<float>(r_data * std::sin(a))
        );
    }

    // Translucent polygon fill (alpha = 0.25f / ~0x40)
    ImU32 fill_color = (accent_color & 0x00FFFFFF) | (0x40000000);
    draw_list->AddConvexPolyFilled(data_pts, NUM_AXES, fill_color);

    // Solid border outline (thickness = 2.5f)
    draw_list->AddPolyline(data_pts, NUM_AXES, accent_color, ImDrawFlags_Closed, 2.5f);

    // 4. Circular Vertex Markers
    for (int i = 0; i < NUM_AXES; ++i) {
        // Outer white halo circle
        draw_list->AddCircleFilled(data_pts[i], 4.5f, IM_COL32(255, 255, 255, 255), 16);
        // Inner accent dot
        draw_list->AddCircleFilled(data_pts[i], 3.0f, accent_color, 16);
    }

    // 5. Interactive Hover Tooltips on Radar Chart Vertices
    ImVec2 mouse = ImGui::GetMousePos();
    int hovered_vertex = -1;
    for (int i = 0; i < NUM_AXES; ++i) {
        float dx = mouse.x - data_pts[i].x;
        float dy = mouse.y - data_pts[i].y;
        if (dx * dx + dy * dy <= 16.0f * 16.0f) { // 16px hit target
            hovered_vertex = i;
            break;
        }
    }

    if (hovered_vertex >= 0) {
        // Enlarge vertex marker on hover
        draw_list->AddCircleFilled(data_pts[hovered_vertex], 7.0f, IM_COL32(255, 255, 255, 255), 16);
        draw_list->AddCircleFilled(data_pts[hovered_vertex], 5.0f, accent_color, 16);

        static const char* const AXIS_DESC_EN[NUM_AXES] = {
            "Chronological preference: classical/historical (0) to contemporary/avant-garde (10)",
            "Decorative intensity: minimalist/functional (0) to ornate/sculptural (10)",
            "Tectonic expression: masked/decorated (0) to exposed structure/materials (10)",
            "Compositional logic: organic/deconstructed (0) to symmetrical/modular (10)",
            "Tactile sensation: cold concrete/steel/glass (0) to warm stone/timber/brick (10)"
        };
        static const char* const AXIS_DESC_IT[NUM_AXES] = {
            "Preferenza temporale: classico/storico (0) a contemporaneo/avanguardia (10)",
            "Intensità decorativa: minimalista/funzionale (0) a ornato/scultoreo (10)",
            "Espressione tettonica: mascherata/decorata (0) a struttura a vista (10)",
            "Logica compositiva: organica/decostruita (0) a simmetrica/modulare (10)",
            "Sensazione tattile: freddo calcestruzzo/acciaio (0) a pietra/legno caldo (10)"
        };

        ImGui::BeginTooltip();
        ImGui::TextColored(ImVec4(0.96f, 0.62f, 0.04f, 1.0f), "%s", labels[hovered_vertex]);
        ImGui::Separator();
        ImGui::Text("%s: %.1f / 10.0", is_italian ? "Punteggio Estetico" : "Aesthetic Score", trait_vals[hovered_vertex]);
        ImGui::Spacing();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 25.0f);
        ImGui::TextDisabled("%s", is_italian ? AXIS_DESC_IT[hovered_vertex] : AXIS_DESC_EN[hovered_vertex]);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

} // namespace arch::ui
