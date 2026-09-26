package com.architecture.profiling.domain

import com.architecture.profiling.domain.engine.PreferenceScorer
import com.architecture.profiling.domain.model.Category
import com.architecture.profiling.domain.model.Style
import com.architecture.profiling.domain.model.TraitRadar
import org.junit.Assert.*
import org.junit.Test

class PreferenceScorerTest {

    @Test
    fun bradleyTerry_convergesOnSymmetricTournament() {
        // Dense cyclic tournament: item i beats item (i+step)%10 for step 1..4
        val winMatrix = Array(10) { IntArray(10) }
        for (i in 0 until 10) {
            for (step in 1..4) {
                winMatrix[i][(i + step) % 10] = 1
            }
        }

        val probabilities = PreferenceScorer.calculateBradleyTerry(winMatrix)
        assertEquals(10, probabilities.size)
        assertEquals("Probabilities must sum to 1.0", 1.0, probabilities.sum(), 1e-6)

        for (p in probabilities) {
            assertEquals("Symmetric tournament yields uniform 0.10 probability", 0.10, p, 1e-3)
        }
    }

    @Test
    fun bradleyTerry_handlesBoundaryConditionViaLaplaceFallback() {
        // Style 1 beats all (9 wins, 0 losses), Style 10 loses all (0 wins, 9 losses)
        val winMatrix = Array(10) { IntArray(10) }
        for (i in 0 until 10) {
            for (j in i + 1 until 10) {
                winMatrix[i][j] = 1
            }
        }

        val probabilities = PreferenceScorer.calculateBradleyTerry(winMatrix)
        assertEquals(10, probabilities.size)
        assertEquals("Probabilities must sum to 1.0", 1.0, probabilities.sum(), 1e-6)

        // Strictly decreasing order
        for (i in 0 until 9) {
            assertTrue(
                "p[$i] (${probabilities[i]}) must be > p[${i+1}] (${probabilities[i+1]})",
                probabilities[i] > probabilities[i + 1]
            )
        }

        assertEquals(
            "Top style probability for 9-0 dominance with Laplace smoothing",
            10.0 / 55.0, probabilities[0], 1e-5
        )
    }

    @Test
    fun allProbabilities_areStrictlyPositive() {
        val winMatrix = Array(10) { IntArray(10) }
        winMatrix[0][1] = 1 // only 1 match played

        val probabilities = PreferenceScorer.calculateBradleyTerry(winMatrix)
        for (p in probabilities) {
            assertTrue("Probability must be strictly positive", p > 0.0)
        }
    }

    @Test
    fun rankStyles_ordersDescendingByProbability() {
        val probabilities = doubleArrayOf(0.05, 0.25, 0.15, 0.05, 0.10, 0.05, 0.05, 0.10, 0.15, 0.05)
        val winMatrix = Array(10) { IntArray(10) }
        val rankings = PreferenceScorer.rankStyles(probabilities, winMatrix)

        assertEquals(10, rankings.size)
        assertEquals(2, rankings[0].styleId) // 0.25
        assertEquals(1, rankings[0].rank)
        for (i in 0 until rankings.size - 1) {
            assertTrue(rankings[i].probability >= rankings[i + 1].probability)
            assertEquals(i + 1, rankings[i].rank)
        }
    }

    @Test
    fun aggregateCategories_groupsStylesCorrectly() {
        val styles = listOf(
            Style(1, "1.png", "S1", "S1", "S1", "catA", "20", 1.0, TraitRadar.ZERO, emptyList(), "#000"),
            Style(2, "2.png", "S2", "S2", "S2", "catA", "20", 1.0, TraitRadar.ZERO, emptyList(), "#000"),
            Style(3, "3.png", "S3", "S3", "S3", "catB", "20", 1.0, TraitRadar.ZERO, emptyList(), "#000")
        )
        val categories = listOf(
            Category("catA", "Category A", "Categoria A"),
            Category("catB", "Category B", "Categoria B")
        )
        val probabilities = doubleArrayOf(0.3, 0.2, 0.5)

        val catScores = PreferenceScorer.aggregateCategories(probabilities, styles, categories)
        assertEquals(2, catScores.size)
        // catA = 0.3 + 0.2 = 0.5, catB = 0.5
        assertEquals(0.5, catScores[0].affinity, 1e-6)
        assertEquals(0.5, catScores[1].affinity, 1e-6)
    }
}
