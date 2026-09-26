package com.architecture.profiling.domain.engine

import com.architecture.profiling.domain.model.Category
import com.architecture.profiling.domain.model.CategoryScore
import com.architecture.profiling.domain.model.Style
import com.architecture.profiling.domain.model.StyleScore
import kotlin.math.abs
import kotlin.math.max

object PreferenceScorer {

    /**
     * Solves Bradley-Terry latent preference probabilities using Minorize-Maximization (MM).
     * Falls back to regularized Laplace smoothing win-rate when undefeated or 0-win items exist.
     */
    fun calculateBradleyTerry(
        winMatrix: Array<IntArray>,
        epsilon: Double = 1e-6,
        maxIterations: Int = 50
    ): DoubleArray {
        val n = winMatrix.size
        val wins = IntArray(n)
        val totalMatches = IntArray(n)
        var hasBoundaryCondition = false

        for (i in 0 until n) {
            for (j in 0 until n) {
                if (i == j) continue
                wins[i] += winMatrix[i][j]
                totalMatches[i] += winMatrix[i][j] + winMatrix[j][i]
            }
            if (totalMatches[i] > 0 && (wins[i] == 0 || wins[i] == totalMatches[i])) {
                hasBoundaryCondition = true
            }
        }

        // Boundary case: undefeated (100% wins) or beaten (0 wins) items.
        // Use regularized Laplace smoothing win-rate to guarantee stability and prevent MLE divergence.
        if (hasBoundaryCondition) {
            val rates = DoubleArray(n)
            for (i in 0 until n) {
                rates[i] = if (totalMatches[i] > 0) {
                    (wins[i] + 1.0) / (totalMatches[i] + 2.0)
                } else {
                    0.5
                }
            }
            val sumRates = rates.sum()
            return DoubleArray(n) { i -> rates[i] / sumRates }
        }

        // Minorize-Maximization (MM) Iterative Convergence
        var p = DoubleArray(n) { 1.0 / n }

        for (iter in 0 until maxIterations) {
            val pNext = DoubleArray(n)
            var sumNext = 0.0

            for (i in 0 until n) {
                var denominator = 0.0
                for (j in 0 until n) {
                    if (i == j) continue
                    val matchesIj = winMatrix[i][j] + winMatrix[j][i]
                    if (matchesIj > 0) {
                        denominator += matchesIj / (p[i] + p[j])
                    }
                }
                pNext[i] = if (denominator > 1e-12) wins[i] / denominator else p[i]
                sumNext += pNext[i]
            }

            if (sumNext > 1e-12) {
                for (i in 0 until n) pNext[i] /= sumNext
            }

            var maxDiff = 0.0
            for (i in 0 until n) {
                maxDiff = max(maxDiff, abs(pNext[i] - p[i]))
            }
            p = pNext

            if (maxDiff < epsilon) break
        }

        return p
    }

    fun rankStyles(probabilities: DoubleArray, winMatrix: Array<IntArray>): List<StyleScore> {
        val n = probabilities.size
        val indexed = (0 until n).map { i ->
            val wins = (0 until n).sumOf { j -> winMatrix[i][j] }
            val total = (0 until n).sumOf { j -> winMatrix[i][j] + winMatrix[j][i] }
            Triple(i + 1, probabilities[i], Pair(wins, total))
        }.sortedWith(
            compareByDescending<Triple<Int, Double, Pair<Int, Int>>> { it.second }
                .thenByDescending { it.third.first }
                .thenBy { it.first }
        )

        return indexed.mapIndexed { rankIdx, item ->
            StyleScore(
                styleId = item.first,
                probability = item.second,
                rank = rankIdx + 1,
                wins = item.third.first,
                totalPlayed = item.third.second
            )
        }
    }

    fun aggregateCategories(
        probabilities: DoubleArray,
        styles: List<Style>,
        categories: List<Category>
    ): List<CategoryScore> {
        val styleMap = styles.associateBy { it.id }
        val categoryScores = categories.associate { it.id to 0.0 }.toMutableMap()

        for (i in probabilities.indices) {
            val styleId = i + 1
            val catId = styleMap[styleId]?.categoryId ?: continue
            categoryScores[catId] = (categoryScores[catId] ?: 0.0) + probabilities[i]
        }

        return categoryScores.entries
            .sortedWith(compareByDescending<Map.Entry<String, Double>> { it.value }.thenBy { it.key })
            .mapIndexed { rankIdx, entry ->
                CategoryScore(
                    categoryId = entry.key,
                    affinity = entry.value,
                    rank = rankIdx + 1
                )
            }
    }
}
