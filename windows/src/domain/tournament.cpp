#include "architecture/tournament.hpp"
#include "architecture/preference_engine.hpp"
#include <array>
#include <algorithm>
#include <stdexcept>

namespace arch::domain {

std::vector<MatchPair> PairingScheduler::generate_schedule(TournamentMode mode) {
    // 9 canonical rounds of 1-factorization on K_10
    const std::vector<std::vector<std::pair<int, int>>> canonical_rounds = {
        {{1, 10}, {2, 9}, {3, 8}, {4, 7}, {5, 6}},
        {{2, 10}, {1, 3}, {4, 9}, {5, 8}, {6, 7}},
        {{3, 10}, {2, 4}, {1, 5}, {6, 9}, {7, 8}},
        {{4, 10}, {3, 5}, {2, 6}, {1, 7}, {8, 9}},
        {{5, 10}, {4, 6}, {3, 7}, {2, 8}, {1, 9}},
        {{6, 10}, {5, 7}, {4, 8}, {3, 9}, {1, 2}},
        {{7, 10}, {6, 8}, {5, 9}, {1, 4}, {2, 3}},
        {{8, 10}, {7, 9}, {1, 6}, {2, 5}, {3, 4}},
        {{9, 10}, {1, 8}, {2, 7}, {3, 6}, {4, 5}}
    };

    std::array<int, 11> left_counts{};
    std::array<int, 11> right_counts{};
    std::vector<MatchPair> schedule;
    schedule.reserve(45);

    int match_idx = 0;
    for (size_t r = 0; r < canonical_rounds.size(); ++r) {
        for (const auto& [u, v] : canonical_rounds[r]) {
            int left, right;
            if (left_counts[u] < left_counts[v]) {
                left = u;
                right = v;
            } else if (left_counts[v] < left_counts[u]) {
                left = v;
                right = u;
            } else {
                if ((left_counts[u] + right_counts[v]) % 2 == 0) {
                    left = u;
                    right = v;
                } else {
                    left = v;
                    right = u;
                }
            }
            left_counts[left]++;
            right_counts[right]++;
            schedule.push_back(MatchPair{match_idx++, static_cast<int>(r), left, right});
        }
    }

    if (mode == TournamentMode::Quick) {
        schedule.resize(15);
    }

    return schedule;
}

TournamentStateMachine::TournamentStateMachine(TournamentMode mode) {
    reset(mode);
}

void TournamentStateMachine::reset(TournamentMode mode) {
    m_mode = mode;
    m_current_index = 0;
    m_history_stack.clear();
    m_redo_stack.clear();

    auto pairs = PairingScheduler::generate_schedule(mode);
    m_matches.clear();
    m_matches.reserve(pairs.size());

    for (const auto& p : pairs) {
        MatchRecord mr;
        mr.match_index = p.match_index;
        mr.round_index = p.round_index;
        mr.left_style_id = p.left_style_id;
        mr.right_style_id = p.right_style_id;
        mr.outcome = MatchOutcome::Undecided;
        mr.winner_style_id = 0;
        m_matches.push_back(mr);
    }
}

double TournamentStateMachine::progress_percentage() const noexcept {
    if (m_matches.empty()) return 0.0;
    return (static_cast<double>(m_current_index) / static_cast<double>(m_matches.size())) * 100.0;
}

const MatchRecord& TournamentStateMachine::current_match() const {
    if (m_matches.empty()) {
        throw std::out_of_range("No matches in tournament");
    }
    if (m_current_index >= static_cast<int>(m_matches.size())) {
        return m_matches.back();
    }
    return m_matches[m_current_index];
}

bool TournamentStateMachine::can_vote() const noexcept {
    return m_current_index < static_cast<int>(m_matches.size());
}

void TournamentStateMachine::vote(int winner_style_id) {
    if (!can_vote()) {
        return;
    }

    auto& current = m_matches[m_current_index];
    if (winner_style_id != current.left_style_id && winner_style_id != current.right_style_id) {
        throw std::invalid_argument("Winner ID must be one of the competing styles in this match");
    }

    current.winner_style_id = winner_style_id;
    current.outcome = (winner_style_id == current.left_style_id)
        ? MatchOutcome::LeftWon
        : MatchOutcome::RightWon;

    m_history_stack.push_back(current);
    m_redo_stack.clear();
    m_current_index++;
}

bool TournamentStateMachine::can_undo() const noexcept {
    return !m_history_stack.empty() && m_current_index > 0;
}

void TournamentStateMachine::undo() {
    if (!can_undo()) {
        return;
    }

    m_current_index--;
    m_redo_stack.push_back(m_matches[m_current_index]);
    m_history_stack.pop_back();

    m_matches[m_current_index].outcome = MatchOutcome::Undecided;
    m_matches[m_current_index].winner_style_id = 0;
}

bool TournamentStateMachine::can_redo() const noexcept {
    return !m_redo_stack.empty() && m_current_index < static_cast<int>(m_matches.size());
}

void TournamentStateMachine::redo() {
    if (!can_redo()) {
        return;
    }

    auto redo_match = m_redo_stack.back();
    m_redo_stack.pop_back();

    m_matches[m_current_index] = redo_match;
    m_history_stack.push_back(redo_match);
    m_current_index++;
}

std::vector<std::vector<int>> TournamentStateMachine::build_win_matrix() const {
    std::vector<std::vector<int>> win_matrix(10, std::vector<int>(10, 0));

    for (const auto& m : m_matches) {
        if (m.is_played() && m.winner_style_id > 0) {
            int winner = m.winner_style_id;
            int loser = (winner == m.left_style_id) ? m.right_style_id : m.left_style_id;
            if (winner >= 1 && winner <= 10 && loser >= 1 && loser <= 10) {
                win_matrix[winner - 1][loser - 1] += 1;
            }
        }
    }

    return win_matrix;
}

TournamentResult TournamentStateMachine::calculate_result(
    const std::vector<Style>& styles,
    const std::vector<Category>& categories
) const {
    auto matrix = build_win_matrix();
    return PreferenceEngine::evaluate(matrix, styles, categories, total_matches());
}

} // namespace arch::domain
