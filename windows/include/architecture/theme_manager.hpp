#pragma once

#include "imgui.h"
#include <cstdint>
#include <string>

namespace arch::ui {

enum class Theme : uint8_t {
    Light = 0,
    Dark = 1
};

class ThemeManager {
public:
    static ThemeManager& instance() {
        static ThemeManager inst;
        return inst;
    }

    [[nodiscard]] Theme current_theme() const noexcept { return current_theme_; }

    void toggle_theme() {
        apply_theme((current_theme_ == Theme::Light) ? Theme::Dark : Theme::Light);
    }

    void apply_theme(Theme theme);

    // Dynamic color helpers for UI components
    [[nodiscard]] ImU32 get_primary_accent_color() const noexcept;
    [[nodiscard]] ImU32 get_card_background_color() const noexcept;
    [[nodiscard]] ImU32 get_border_color() const noexcept;
    [[nodiscard]] ImU32 get_text_color() const noexcept;
    [[nodiscard]] ImU32 get_muted_text_color() const noexcept;

    // Helper for category hex colors
    static ImU32 parse_hex_color(const std::string& hex, float alpha = 1.0f);

private:
    ThemeManager() : current_theme_(Theme::Dark) {}
    Theme current_theme_{Theme::Dark};
};

void setup_modern_imgui_style(ImGuiStyle& style);

} // namespace arch::ui
