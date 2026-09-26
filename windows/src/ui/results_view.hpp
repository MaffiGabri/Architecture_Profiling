#pragma once

#include "architecture/models.hpp"
#include "architecture/texture_manager.hpp"
#include <functional>

namespace arch::ui {

class ResultsView {
public:
    ResultsView() = default;

    /**
     * Renders the tournament completion showcase:
     * 1. Multi-factor Confidence & Consistency Badge
     * 2. #1 Champion Style Hero Banner
     * 3. 110% Narrative Personality Archetype Card
     * 4. Custom 5-Axis Radar Chart
     * 5. Ranked Category Affinity Breakdown Bars
     * 6. Complete 1-to-10 Ranked Style Table
     * 7. Action controls (Restart / View History)
     */
    void render(
        const arch::domain::TournamentResult& result,
        const std::vector<arch::domain::Style>& styles,
        const std::vector<arch::domain::Category>& categories,
        const arch::platform::TextureManager& texture_manager,
        bool is_italian,
        std::function<void()> on_restart_callback,
        std::function<void()> on_view_history_callback
    );

private:
    void render_confidence_badge(const arch::domain::TournamentResult& result, bool is_italian);
    void render_champion_hero(
        const arch::domain::Style& champion_style,
        const arch::domain::StyleScore& champion_score,
        const arch::platform::TextureResource& texture,
        bool is_italian
    );
    void render_archetype_card(const arch::domain::Archetype& archetype, bool is_italian);
    void render_category_breakdown(
        const std::vector<arch::domain::CategoryScore>& category_rankings,
        const std::vector<arch::domain::Category>& categories,
        bool is_italian
    );
    void render_style_rankings(
        const std::vector<arch::domain::StyleScore>& style_rankings,
        const std::vector<arch::domain::Style>& styles,
        bool is_italian
    );
};

} // namespace arch::ui
