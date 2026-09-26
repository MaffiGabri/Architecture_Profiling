package com.architecture.profiling.domain.model

data class TournamentState(
    val mode: TournamentMode = TournamentMode.FULL,
    val matches: List<MatchRecord>,
    val currentIndex: Int = 0,
    val historyStack: List<MatchRecord> = emptyList(),
    val redoStack: List<MatchRecord> = emptyList()
) {
    val totalMatches: Int get() = matches.size
    val matchesPlayed: Int get() = currentIndex
    val currentMatch: MatchRecord? get() = matches.getOrNull(currentIndex)
    val isComplete: Boolean get() = currentIndex >= matches.size
    val canVote: Boolean get() = !isComplete
    val canUndo: Boolean get() = historyStack.isNotEmpty()
    val canRedo: Boolean get() = redoStack.isNotEmpty()
    val progressPercentage: Double get() = if (matches.isEmpty()) 0.0 else (currentIndex.toDouble() / matches.size) * 100.0
}
