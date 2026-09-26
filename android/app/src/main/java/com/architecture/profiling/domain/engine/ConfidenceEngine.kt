package com.architecture.profiling.domain.engine

import kotlin.math.ln
import kotlin.math.tanh

object ConfidenceEngine {

    fun calculateEntropy(probabilities: DoubleArray): Double {
        var h = 0.0
        for (p in probabilities) {
            if (p > 1e-12) {
                h -= p * ln(p)
            }
        }
        return h
    }

    fun calculateEntropyConcentration(entropy: Double, numClasses: Int = 10): Double {
        val maxH = ln(numClasses.toDouble())
        return (1.0 - (entropy / maxH)).coerceIn(0.0, 1.0)
    }

    fun calculateVictoryMargin(probabilities: DoubleArray): Double {
        val sorted = probabilities.sortedDescending()
        if (sorted.size < 2) return 0.0
        return (sorted[0] - sorted[1]).coerceIn(0.0, 1.0)
    }

    /**
     * Multi-objective confidence formula:
     * clamp(15.0, 99.0, (0.45 * zeta + 0.35 * tanh(3.0 * Delta) + 0.20 * C_H) * 100.0)
     */
    fun calculateConfidence(
        zeta: Double,
        victoryMargin: Double,
        entropyConcentration: Double
    ): Double {
        val marginFactor = tanh(3.0 * victoryMargin)
        val rawConfidence = (0.45 * zeta + 0.35 * marginFactor + 0.20 * entropyConcentration) * 100.0
        return rawConfidence.coerceIn(15.0, 99.0)
    }
}
