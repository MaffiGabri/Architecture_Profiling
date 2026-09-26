#include "architecture/theme_manager.hpp"
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace arch::ui {

void setup_modern_imgui_style(ImGuiStyle& style) {
    // Modern Rounded Corners
    style.WindowRounding    = 8.0f;
    style.ChildRounding     = 8.0f;
    style.FrameRounding     = 6.0f;
    style.PopupRounding     = 6.0f;
    style.ScrollbarRounding = 9.0f;
    style.GrabRounding      = 6.0f;
    style.TabRounding       = 6.0f;

    // Generous Ergonomic Spacing & Padding
    style.WindowPadding     = ImVec2(16.0f, 16.0f);
    style.FramePadding      = ImVec2(12.0f, 8.0f);
    style.ItemSpacing       = ImVec2(12.0f, 10.0f);
    style.ItemInnerSpacing  = ImVec2(8.0f, 6.0f);
    style.CellPadding       = ImVec2(8.0f, 8.0f);

    // Architectural Borders & Sizing
    style.WindowBorderSize  = 1.0f;
    style.ChildBorderSize   = 1.0f;
    style.FrameBorderSize   = 1.0f;
    style.PopupBorderSize   = 1.0f;
    style.ScrollbarSize     = 12.0f;
    style.GrabMinSize       = 14.0f;

    // Sub-pixel Antialiasing flags
    style.AntiAliasedLines        = true;
    style.AntiAliasedLinesUseTex  = true;
    style.AntiAliasedFill         = true;
}

void ThemeManager::apply_theme(Theme theme) {
    current_theme_ = theme;
    ImGuiStyle& style = ImGui::GetStyle();
    setup_modern_imgui_style(style);
    ImVec4* colors = style.Colors;

    if (theme == Theme::Light) {
        // =====================================================================
        // Travertine Light Palette
        // =====================================================================
        colors[ImGuiCol_Text]                  = ImVec4(0.110f, 0.098f, 0.090f, 1.00f); // #1C1917
        colors[ImGuiCol_TextDisabled]          = ImVec4(0.471f, 0.443f, 0.424f, 1.00f); // #78716C
        colors[ImGuiCol_WindowBg]              = ImVec4(0.984f, 0.976f, 0.961f, 1.00f); // #FBF9F5
        colors[ImGuiCol_ChildBg]               = ImVec4(1.000f, 1.000f, 1.000f, 1.00f); // #FFFFFF
        colors[ImGuiCol_PopupBg]               = ImVec4(1.000f, 1.000f, 1.000f, 0.98f);
        colors[ImGuiCol_Border]                = ImVec4(0.906f, 0.898f, 0.894f, 1.00f); // #E7E5E4
        colors[ImGuiCol_BorderShadow]          = ImVec4(0.000f, 0.000f, 0.000f, 0.04f);
        colors[ImGuiCol_FrameBg]               = ImVec4(0.961f, 0.953f, 0.937f, 1.00f); // #F5F3EF
        colors[ImGuiCol_FrameBgHovered]        = ImVec4(0.929f, 0.918f, 0.890f, 1.00f);
        colors[ImGuiCol_FrameBgActive]         = ImVec4(0.898f, 0.878f, 0.843f, 1.00f);
        colors[ImGuiCol_TitleBg]               = ImVec4(0.961f, 0.953f, 0.937f, 1.00f);
        colors[ImGuiCol_TitleBgActive]         = ImVec4(0.984f, 0.976f, 0.961f, 1.00f);
        colors[ImGuiCol_TitleBgCollapsed]      = ImVec4(0.961f, 0.953f, 0.937f, 0.75f);
        colors[ImGuiCol_MenuBarBg]             = ImVec4(0.984f, 0.976f, 0.961f, 1.00f);
        colors[ImGuiCol_ScrollbarBg]           = ImVec4(0.961f, 0.953f, 0.937f, 1.00f);
        colors[ImGuiCol_ScrollbarGrab]         = ImVec4(0.839f, 0.827f, 0.820f, 1.00f); // #D6D3D1
        colors[ImGuiCol_ScrollbarGrabHovered]  = ImVec4(0.659f, 0.635f, 0.620f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabActive]   = ImVec4(0.471f, 0.443f, 0.424f, 1.00f);
        colors[ImGuiCol_CheckMark]             = ImVec4(0.600f, 0.396f, 0.082f, 1.00f); // #996515
        colors[ImGuiCol_SliderGrab]            = ImVec4(0.600f, 0.396f, 0.082f, 1.00f);
        colors[ImGuiCol_SliderGrabActive]      = ImVec4(0.490f, 0.322f, 0.067f, 1.00f);
        colors[ImGuiCol_Button]                = ImVec4(0.600f, 0.396f, 0.082f, 1.00f); // #996515 Classical Gold
        colors[ImGuiCol_ButtonHovered]         = ImVec4(0.706f, 0.471f, 0.102f, 1.00f); // #B4781A
        colors[ImGuiCol_ButtonActive]          = ImVec4(0.490f, 0.322f, 0.067f, 1.00f); // #7D5211
        colors[ImGuiCol_Header]                = ImVec4(0.992f, 0.902f, 0.541f, 0.40f);
        colors[ImGuiCol_HeaderHovered]         = ImVec4(0.992f, 0.902f, 0.541f, 0.70f);
        colors[ImGuiCol_HeaderActive]          = ImVec4(0.992f, 0.902f, 0.541f, 1.00f);
        colors[ImGuiCol_Separator]             = ImVec4(0.906f, 0.898f, 0.894f, 1.00f);
        colors[ImGuiCol_SeparatorHovered]      = ImVec4(0.600f, 0.396f, 0.082f, 0.78f);
        colors[ImGuiCol_SeparatorActive]       = ImVec4(0.600f, 0.396f, 0.082f, 1.00f);
        colors[ImGuiCol_Tab]                   = ImVec4(0.961f, 0.953f, 0.937f, 1.00f);
        colors[ImGuiCol_TabHovered]            = ImVec4(0.992f, 0.902f, 0.541f, 0.80f);
        colors[ImGuiCol_TabActive]             = ImVec4(1.000f, 1.000f, 1.000f, 1.00f);
        colors[ImGuiCol_TabUnfocused]          = ImVec4(0.961f, 0.953f, 0.937f, 1.00f);
        colors[ImGuiCol_TabUnfocusedActive]    = ImVec4(0.984f, 0.976f, 0.961f, 1.00f);
        colors[ImGuiCol_PlotHistogram]         = ImVec4(0.600f, 0.396f, 0.082f, 1.00f);
        colors[ImGuiCol_PlotHistogramHovered]  = ImVec4(0.706f, 0.471f, 0.102f, 1.00f);
        colors[ImGuiCol_TableHeaderBg]         = ImVec4(0.961f, 0.953f, 0.937f, 1.00f);
        colors[ImGuiCol_TableBorderStrong]     = ImVec4(0.839f, 0.827f, 0.820f, 1.00f);
        colors[ImGuiCol_TableBorderLight]      = ImVec4(0.906f, 0.898f, 0.894f, 1.00f);
        colors[ImGuiCol_TableRowBg]            = ImVec4(0.000f, 0.000f, 0.000f, 0.00f);
        colors[ImGuiCol_TableRowBgAlt]         = ImVec4(0.961f, 0.953f, 0.937f, 0.50f);
        colors[ImGuiCol_NavHighlight]          = ImVec4(0.600f, 0.396f, 0.082f, 1.00f);
        colors[ImGuiCol_ModalWindowDimBg]      = ImVec4(0.110f, 0.098f, 0.090f, 0.35f);
    } else {
        // =====================================================================
        // Deep Basalt Dark Palette
        // =====================================================================
        colors[ImGuiCol_Text]                  = ImVec4(0.953f, 0.957f, 0.965f, 1.00f); // #F3F4F6
        colors[ImGuiCol_TextDisabled]          = ImVec4(0.580f, 0.639f, 0.722f, 1.00f); // #94A3B8
        colors[ImGuiCol_WindowBg]              = ImVec4(0.059f, 0.067f, 0.090f, 1.00f); // #0F1117
        colors[ImGuiCol_ChildBg]               = ImVec4(0.102f, 0.114f, 0.141f, 1.00f); // #1A1D24
        colors[ImGuiCol_PopupBg]               = ImVec4(0.102f, 0.114f, 0.141f, 0.98f);
        colors[ImGuiCol_Border]                = ImVec4(0.180f, 0.204f, 0.251f, 1.00f); // #2E3440
        colors[ImGuiCol_BorderShadow]          = ImVec4(0.000f, 0.000f, 0.000f, 0.20f);
        colors[ImGuiCol_FrameBg]               = ImVec4(0.082f, 0.094f, 0.125f, 1.00f); // #151820
        colors[ImGuiCol_FrameBgHovered]        = ImVec4(0.118f, 0.133f, 0.176f, 1.00f);
        colors[ImGuiCol_FrameBgActive]         = ImVec4(0.145f, 0.169f, 0.227f, 1.00f);
        colors[ImGuiCol_TitleBg]               = ImVec4(0.082f, 0.094f, 0.125f, 1.00f);
        colors[ImGuiCol_TitleBgActive]         = ImVec4(0.102f, 0.114f, 0.141f, 1.00f);
        colors[ImGuiCol_TitleBgCollapsed]      = ImVec4(0.059f, 0.067f, 0.090f, 0.75f);
        colors[ImGuiCol_MenuBarBg]             = ImVec4(0.082f, 0.094f, 0.125f, 1.00f);
        colors[ImGuiCol_ScrollbarBg]           = ImVec4(0.071f, 0.082f, 0.110f, 1.00f);
        colors[ImGuiCol_ScrollbarGrab]         = ImVec4(0.200f, 0.255f, 0.333f, 1.00f); // #334155
        colors[ImGuiCol_ScrollbarGrabHovered]  = ImVec4(0.278f, 0.333f, 0.412f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabActive]   = ImVec4(0.392f, 0.455f, 0.545f, 1.00f);
        colors[ImGuiCol_CheckMark]             = ImVec4(0.961f, 0.620f, 0.043f, 1.00f); // #F59E0B
        colors[ImGuiCol_SliderGrab]            = ImVec4(0.961f, 0.620f, 0.043f, 1.00f);
        colors[ImGuiCol_SliderGrabActive]      = ImVec4(0.851f, 0.467f, 0.024f, 1.00f);
        colors[ImGuiCol_Button]                = ImVec4(0.961f, 0.620f, 0.043f, 1.00f); // #F59E0B Amber Gold
        colors[ImGuiCol_ButtonHovered]         = ImVec4(0.984f, 0.749f, 0.141f, 1.00f); // #FBBF24
        colors[ImGuiCol_ButtonActive]          = ImVec4(0.851f, 0.467f, 0.024f, 1.00f); // #D97706
        colors[ImGuiCol_Header]                = ImVec4(0.471f, 0.208f, 0.059f, 0.45f);
        colors[ImGuiCol_HeaderHovered]         = ImVec4(0.471f, 0.208f, 0.059f, 0.75f);
        colors[ImGuiCol_HeaderActive]          = ImVec4(0.471f, 0.208f, 0.059f, 1.00f);
        colors[ImGuiCol_Separator]             = ImVec4(0.180f, 0.204f, 0.251f, 1.00f);
        colors[ImGuiCol_SeparatorHovered]      = ImVec4(0.961f, 0.620f, 0.043f, 0.78f);
        colors[ImGuiCol_SeparatorActive]       = ImVec4(0.961f, 0.620f, 0.043f, 1.00f);
        colors[ImGuiCol_Tab]                   = ImVec4(0.082f, 0.094f, 0.125f, 1.00f);
        colors[ImGuiCol_TabHovered]            = ImVec4(0.471f, 0.208f, 0.059f, 0.80f);
        colors[ImGuiCol_TabActive]             = ImVec4(0.141f, 0.165f, 0.220f, 1.00f);
        colors[ImGuiCol_TabUnfocused]          = ImVec4(0.082f, 0.094f, 0.125f, 1.00f);
        colors[ImGuiCol_TabUnfocusedActive]    = ImVec4(0.102f, 0.114f, 0.141f, 1.00f);
        colors[ImGuiCol_PlotHistogram]         = ImVec4(0.961f, 0.620f, 0.043f, 1.00f);
        colors[ImGuiCol_PlotHistogramHovered]  = ImVec4(0.984f, 0.749f, 0.141f, 1.00f);
        colors[ImGuiCol_TableHeaderBg]         = ImVec4(0.082f, 0.094f, 0.125f, 1.00f);
        colors[ImGuiCol_TableBorderStrong]     = ImVec4(0.200f, 0.255f, 0.333f, 1.00f);
        colors[ImGuiCol_TableBorderLight]      = ImVec4(0.180f, 0.204f, 0.251f, 1.00f);
        colors[ImGuiCol_TableRowBg]            = ImVec4(0.000f, 0.000f, 0.000f, 0.00f);
        colors[ImGuiCol_TableRowBgAlt]         = ImVec4(0.141f, 0.165f, 0.220f, 0.30f);
        colors[ImGuiCol_NavHighlight]          = ImVec4(0.961f, 0.620f, 0.043f, 1.00f);
        colors[ImGuiCol_ModalWindowDimBg]      = ImVec4(0.000f, 0.000f, 0.000f, 0.60f);
    }
}

ImU32 ThemeManager::get_primary_accent_color() const noexcept {
    return (current_theme_ == Theme::Light)
        ? IM_COL32(153, 101, 21, 255)  // #996515
        : IM_COL32(245, 158, 11, 255); // #F59E0B
}

ImU32 ThemeManager::get_card_background_color() const noexcept {
    return (current_theme_ == Theme::Light)
        ? IM_COL32(255, 255, 255, 255) // #FFFFFF
        : IM_COL32(26, 29, 36, 255);   // #1A1D24
}

ImU32 ThemeManager::get_border_color() const noexcept {
    return (current_theme_ == Theme::Light)
        ? IM_COL32(231, 229, 228, 255) // #E7E5E4
        : IM_COL32(46, 52, 64, 255);   // #2E3440
}

ImU32 ThemeManager::get_text_color() const noexcept {
    return (current_theme_ == Theme::Light)
        ? IM_COL32(28, 25, 23, 255)    // #1C1917
        : IM_COL32(243, 244, 246, 255); // #F3F4F6
}

ImU32 ThemeManager::get_muted_text_color() const noexcept {
    return (current_theme_ == Theme::Light)
        ? IM_COL32(120, 113, 108, 255) // #78716C
        : IM_COL32(148, 163, 184, 255); // #94A3B8
}

ImU32 ThemeManager::parse_hex_color(const std::string& hex, float alpha) {
    if (hex.empty() || hex[0] != '#') {
        return IM_COL32(74, 105, 132, static_cast<int>(alpha * 255.0f));
    }
    uint32_t val = 0;
    std::stringstream ss;
    ss << std::hex << hex.substr(1);
    ss >> val;
    int r = (val >> 16) & 0xFF;
    int g = (val >> 8) & 0xFF;
    int b = val & 0xFF;
    return IM_COL32(r, g, b, static_cast<int>(std::clamp(alpha, 0.0f, 1.0f) * 255.0f));
}

} // namespace arch::ui
