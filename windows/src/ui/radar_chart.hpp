#pragma once

#include "architecture/models.hpp"
#include "imgui.h"

namespace arch::ui {

/**
 * Pure ImDrawList 5-Axis Spider/Radar Chart.
 * Matches Android native geometry with regular pentagonal symmetry and zero external charting dependencies.
 *
 * Axes:
 * 0: Era (-90 deg / 12 o'clock)
 * 1: Ornamentation (-18 deg / top-right)
 * 2: Structural Honesty (+54 deg / bottom-right)
 * 3: Geometric Order (+126 deg / bottom-left)
 * 4: Material Warmth (+198 deg / top-left)
 */
void RenderRadarChart(
    const arch::domain::TraitRadar& traits,
    bool is_italian,
    ImVec2 size = ImVec2(0.0f, 280.0f),
    ImU32 accent_color = 0,
    ImU32 grid_color = 0,
    ImU32 text_color = 0
);

} // namespace arch::ui
