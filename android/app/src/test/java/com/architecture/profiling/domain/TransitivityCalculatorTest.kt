package com.architecture.profiling.domain

import com.architecture.profiling.domain.engine.TransitivityCalculator
import org.junit.Assert.*
import org.junit.Test

class TransitivityCalculatorTest {

    @Test
    fun perfectTransitivity_yieldsZeroTriadsAndZetaOne() {
        val winMatrix = Array(10) { IntArray(10) }
        for (i in 0 until 10) {
            for (j in i + 1 until 10) {
                winMatrix[i][j] = 1
            }
        }

        val c = TransitivityCalculator.calculateCircularTriads(winMatrix, totalMatchesPlayed = 45)
        val zeta = TransitivityCalculator.calculateZeta(c)

        assertEquals("Zero circular triads under complete transitivity", 0, c)
        assertEquals("Zeta must be exactly 1.0", 1.0, zeta, 1e-6)
    }

    @Test
    fun extremeCyclicParadox_yields40TriadsAndZetaZero() {
        // 5 items with 5 wins, 5 items with 4 wins -> maximum circular triads for n=10
        val winMatrix = Array(10) { IntArray(10) }
        for (i in 0 until 10) {
            val winsForThis = if (i < 5) 5 else 4
            for (step in 1..winsForThis) {
                winMatrix[i][(i + step) % 10] = 1
            }
        }

        val c = TransitivityCalculator.calculateCircularTriads(winMatrix, totalMatchesPlayed = 45)
        val zeta = TransitivityCalculator.calculateZeta(c)

        assertEquals("Maximum circular triads for n=10 is 40", 40, c)
        assertEquals("Zeta must be exactly 0.0 under max triads", 0.0, zeta, 1e-6)
    }

    @Test
    fun quickTournamentScaling_preservesZetaScale() {
        // In 15 matches (quick mode), simulate transitive outcomes
        val winMatrix = Array(10) { IntArray(10) }
        // Let style 1 beat styles 2, 3, 4, etc.
        val quickWinners = listOf(1, 2, 3, 7, 6, 2, 1, 4, 5, 6, 3, 2, 1, 6, 7)
        val schedule = com.architecture.profiling.domain.engine.PairingScheduler.generateSchedule(com.architecture.profiling.domain.model.TournamentMode.QUICK)
        for (idx in 0 until 15) {
            val pair = schedule[idx]
            val w = quickWinners[idx]
            val l = if (w == pair.leftStyleId) pair.rightStyleId else pair.leftStyleId
            winMatrix[w - 1][l - 1]++
        }

        val c = TransitivityCalculator.calculateCircularTriads(winMatrix, totalMatchesPlayed = 15)
        val zeta = TransitivityCalculator.calculateZeta(c)

        assertEquals(0, c)
        assertEquals(1.0, zeta, 1e-6)
    }

    @Test
    fun clamping_protectsAgainstOutOfBounds() {
        assertEquals(1.0, TransitivityCalculator.calculateZeta(-5), 1e-6)
        assertEquals(0.0, TransitivityCalculator.calculateZeta(50), 1e-6)
    }
}
