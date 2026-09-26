#pragma once

#include "architecture/tournament.hpp"
#include "architecture/models.hpp"
#include "architecture/texture_manager.hpp"
#include <functional>

namespace arch::ui {

class TournamentView {
public:
    TournamentView() = default;

    /**
     * Renders the tournament screen: navigation bar, progress bar,
     * and the two side-by-side comparison cards (Option A vs Option B).
     */
    void render(
        arch::domain::TournamentStateMachine& tournament,
        const std::vector<arch::domain::Style>& styles,
        const std::vector<arch::domain::Category>& categories,
        const arch::platform::TextureManager& texture_manager,
        bool is_italian,
        std::function<void()> on_complete_callback
    );

private:
    void render_navigation_bar(
        arch::domain::TournamentStateMachine& tournament,
        bool is_italian
    );

    void render_comparison_card(
        const arch::domain::Style& style,
        const arch::domain::Category* category,
        const arch::platform::TextureResource& texture,
        bool is_left,
        float card_width,
        std::function<void(int)> on_vote,
        bool is_italian
    );
};

} // namespace arch::ui
