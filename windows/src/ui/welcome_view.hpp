#pragma once

#include "architecture/models.hpp"
#include "architecture/tournament.hpp"
#include "architecture/persistence.hpp"
#include <functional>

namespace arch::ui {

class WelcomeView {
public:
    WelcomeView() = default;

    /**
     * Renders the welcome view:
     * - Profile manager (selection, new profile creation, stats)
     * - Tournament mode selection (Full 45 vs Quick 15)
     * - Theme & language controls
     * - Action button to begin tournament
     */
    void render(
        arch::data::ProfileRepository& profile_repo,
        arch::domain::TournamentMode& selected_mode,
        bool is_italian,
        std::function<void(arch::domain::TournamentMode)> on_start_tournament
    );

private:
    char new_username_buf_[64]{"New Explorer"};
    bool show_create_input_{false};
};

} // namespace arch::ui
