package com.architecture.profiling.domain.model

data class MatchRecord(
    val matchIndex: Int,
    val roundIndex: Int,
    val leftStyleId: Int,
    val rightStyleId: Int,
    val outcome: MatchOutcome = MatchOutcome.UNDECIDED,
    val winnerStyleId: Int? = null
) {
    val isPlayed: Boolean get() = outcome != MatchOutcome.UNDECIDED
}
