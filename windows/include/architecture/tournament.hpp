#pragma once

#include "models.hpp"
#include <vector>

namespace arch::domain {

enum class TournamentMode : uint8_t {
    Full = 45,  // Complete Round-Robin (45 matches)
    Quick = 15  // Balanced 3-regular subgraph (15 matches)
};

class PairingScheduler {
public:
    /**
     * Generates a canonical 1-factorization pairing schedule for 10 styles with:
     * 1. Guaranteed minimum spacing >= 4 matches between consecutive appearances of the same item.
     * 2. Spatial left/right presentation balance (each style appears 4-5 times left, 4-5 times right in full mode).
     * 3. Quick tournament mode produces a 15-match 3-regular prefix (matches 0..14).
     */
    static std::vector<MatchPair> generate_schedule(TournamentMode mode = TournamentMode::Full);
};

class TournamentStateMachine {
public:
    explicit TournamentStateMachine(TournamentMode mode = TournamentMode::Full);

    void reset(TournamentMode mode = TournamentMode::Full);

    TournamentMode mode() const noexcept { return m_mode; }
    int current_index() const noexcept { return m_current_index; }
    int total_matches() const noexcept { return static_cast<int>(m_matches.size()); }
    int matches_played() const noexcept { return m_current_index; }
    double progress_percentage() const noexcept;
    bool is_complete() const noexcept { return m_current_index >= total_matches(); }

    const MatchRecord& current_match() const;
    const std::vector<MatchRecord>& all_matches() const noexcept { return m_matches; }

    bool can_vote() const noexcept;
    void vote(int winner_style_id);

    bool can_undo() const noexcept;
    void undo();

    bool can_redo() const noexcept;
    void redo();

    std::vector<std::vector<int>> build_win_matrix() const;

    TournamentResult calculate_result(
        const std::vector<Style>& styles,
        const std::vector<Category>& categories
    ) const;

private:
    TournamentMode m_mode;
    std::vector<MatchRecord> m_matches;
    int m_current_index{0};
    std::vector<MatchRecord> m_history_stack;
    std::vector<MatchRecord> m_redo_stack;
};

} // namespace arch::domain
