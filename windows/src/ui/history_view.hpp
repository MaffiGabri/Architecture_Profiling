#pragma once

#include "architecture/persistence.hpp"
#include <functional>

namespace arch::ui {

class HistoryView {
public:
    HistoryView() = default;

    /**
     * Renders the tournament history screen:
     * - Chronological list of saved tournament runs
     * - Key metrics per run: champion, confidence, zeta, triads, archetype, traits
     * - Controls to clear or remove records, and return to tournament
     */
    void render(
        arch::data::HistoryRepository& history_repo,
        bool is_italian,
        std::function<void()> on_back_callback
    );
};

} // namespace arch::ui
