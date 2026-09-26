package com.architecture.profiling.domain.engine

import com.architecture.profiling.domain.model.MatchPair
import com.architecture.profiling.domain.model.TournamentMode

object PairingScheduler {

    /**
     * Generates a 1-factorization schedule for K_10 round-robin tournament.
     * Properties:
     * 1. 45 distinct pairs for Full mode, 15 for Quick mode.
     * 2. Spacing >= 4 matches between re-appearances of the same item.
     * 3. Left/right balance: each style appears 4-5 times on the left, 4-5 times on the right.
     */
    fun generateSchedule(mode: TournamentMode = TournamentMode.FULL): List<MatchPair> {
        // Base 1-factorization for K_10: Vertex 10 at center, vertices 1..9 on regular 9-gon
        val canonicalRounds = listOf(
            listOf(Pair(1, 10), Pair(2, 9), Pair(3, 8), Pair(4, 7), Pair(5, 6)),
            listOf(Pair(2, 10), Pair(1, 3), Pair(4, 9), Pair(5, 8), Pair(6, 7)),
            listOf(Pair(3, 10), Pair(2, 4), Pair(1, 5), Pair(6, 9), Pair(7, 8)),
            listOf(Pair(4, 10), Pair(3, 5), Pair(2, 6), Pair(1, 7), Pair(8, 9)),
            listOf(Pair(5, 10), Pair(4, 6), Pair(3, 7), Pair(2, 8), Pair(1, 9)),
            listOf(Pair(6, 10), Pair(5, 7), Pair(4, 8), Pair(3, 9), Pair(1, 2)),
            listOf(Pair(7, 10), Pair(6, 8), Pair(5, 9), Pair(1, 4), Pair(2, 3)),
            listOf(Pair(8, 10), Pair(7, 9), Pair(1, 6), Pair(2, 5), Pair(3, 4)),
            listOf(Pair(9, 10), Pair(1, 8), Pair(2, 7), Pair(3, 6), Pair(4, 5))
        )

        val fullScheduleRaw = mutableListOf<Pair<Int, Int>>()
        for (round in canonicalRounds) {
            fullScheduleRaw.addAll(round)
        }

        // Greedy left/right spatial balancer
        val leftCounts = IntArray(11)
        val rightCounts = IntArray(11)
        val fullPairs = mutableListOf<Pair<Int, Int>>()

        for ((u, v) in fullScheduleRaw) {
            val (left, right) = when {
                leftCounts[u] < leftCounts[v] -> Pair(u, v)
                leftCounts[v] < leftCounts[u] -> Pair(v, u)
                else -> {
                    if ((leftCounts[u] + rightCounts[v]) % 2 == 0) Pair(u, v) else Pair(v, u)
                }
            }
            leftCounts[left]++
            rightCounts[right]++
            fullPairs.add(Pair(left, right))
        }

        val matchCount = if (mode == TournamentMode.FULL) 45 else 15
        return (0 until matchCount).map { idx ->
            val pair = fullPairs[idx]
            val roundIdx = idx / 5
            MatchPair(
                matchIndex = idx,
                roundIndex = roundIdx,
                leftStyleId = pair.first,
                rightStyleId = pair.second
            )
        }
    }
}
