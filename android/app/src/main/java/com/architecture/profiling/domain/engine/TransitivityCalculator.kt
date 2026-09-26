package com.architecture.profiling.domain.engine

import kotlin.math.roundToInt

object TransitivityCalculator {

    /**
     * Calculates Kendall circular triads (c) using Kendall & Babington Smith's theorem:
     * c = 142.5 - 0.5 * sum_{i=1}^{10} s_i^2 for n=10.
     * Out-degrees are scaled for quick tournaments (15 matches).
     */
    fun calculateCircularTriads(winMatrix: Array<IntArray>, totalMatchesPlayed: Int = 45): Int {
        val n = winMatrix.size
        val wins = DoubleArray(n) { i ->
            var s = 0.0
            for (j in 0 until n) {
                if (i != j && winMatrix[i][j] > winMatrix[j][i]) {
                    s += 1.0
                }
            }
            s
        }

        val sVec = if (totalMatchesPlayed == 45) {
            wins
        } else {
            val scale = 45.0 / totalMatchesPlayed.coerceAtLeast(1)
            DoubleArray(n) { i -> wins[i] * scale }
        }

        val sumSq = sVec.sumOf { it * it }
        val rawC = 142.5 - 0.5 * sumSq
        return rawC.roundToInt().coerceIn(0, 40)
    }

    /**
     * Computes Kendall's consistence coefficient zeta in [0.0, 1.0].
     * c_max = 40 for n = 10 items.
     */
    fun calculateZeta(circularTriads: Int): Double {
        val clampedC = circularTriads.coerceIn(0, 40)
        return (1.0 - (clampedC.toDouble() / 40.0)).coerceIn(0.0, 1.0)
    }
}
