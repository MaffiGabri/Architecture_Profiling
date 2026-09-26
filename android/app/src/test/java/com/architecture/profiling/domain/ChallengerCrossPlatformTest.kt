package com.architecture.profiling.domain

import com.architecture.profiling.domain.engine.*
import com.architecture.profiling.domain.model.*
import org.junit.Assert.*
import org.junit.Test

/**
 * Empirical Challenger Test Suite for Milestone M3:
 * 1. Exact TC1-TC5 verification from shared_tournament_fixtures.json
 * 2. Deep State Machine Invariants (0 <-> 45 navigation, branching invalidation, immutability)
 * 3. Schedule 1-factorization and spacing invariants in Kotlin engine
 */
class ChallengerCrossPlatformTest {

    private val styles = listOf(
        Style(1, "arch_01_classical.png", "The Parthenon Colonnade", "Classical Antiquity", "Antichita Classica", "classical_renaissance", "5th Century BC", 1.0, TraitRadar(0.5, 5.5, 7.0, 0.5, 8.5), emptyList(), "#D4AF37"),
        Style(2, "arch_02_gothic.png", "Notre-Dame Cathedral Nave", "Gothic Architecture", "Architettura Gotica", "historicist_sacred", "12th - 14th Century", 1.0, TraitRadar(2.0, 9.5, 8.5, 2.5, 6.5), emptyList(), "#6A1B9A"),
        Style(3, "arch_03_renaissance.png", "Basilica of Santa Maria del Fiore", "Italian Renaissance", "Rinascimento Italiano", "classical_renaissance", "15th - 16th Century", 1.0, TraitRadar(3.5, 6.5, 5.5, 1.0, 8.0), emptyList(), "#C0392B"),
        Style(4, "arch_04_baroque.png", "San Carlo alle Quattro Fontane", "Baroque Architecture", "Architettura Barocca", "historicist_sacred", "17th - 18th Century", 1.0, TraitRadar(4.5, 10.0, 3.0, 5.0, 8.5), emptyList(), "#B8860B"),
        Style(5, "arch_05_art_deco.png", "Chrysler Building Spire", "Art Deco", "Art Deco", "early_modern_industrial", "Early 20th Century", 1.0, TraitRadar(6.5, 7.0, 5.0, 2.0, 4.0), emptyList(), "#008B8B"),
        Style(6, "arch_06_bauhaus.png", "Bauhaus Dessau Studio Building", "Bauhaus & International Style", "Bauhaus e Stile Internazionale", "modernism_functionalism", "Mid 20th Century", 1.0, TraitRadar(7.0, 0.5, 9.0, 0.5, 2.0), emptyList(), "#E74C3C"),
        Style(7, "arch_07_brutalism.png", "Boston City Hall Monolith", "Brutalism", "Brutalismo", "modernism_functionalism", "Mid-Late 20th Century", 1.0, TraitRadar(7.8, 0.0, 10.0, 3.5, 4.5), emptyList(), "#708090"),
        Style(8, "arch_08_hightech.png", "Centre Pompidou", "High-Tech Architecture", "Architettura High-Tech", "early_modern_industrial", "Late 20th Century", 1.0, TraitRadar(8.2, 1.5, 10.0, 4.0, 1.0), emptyList(), "#2980B9"),
        Style(9, "arch_09_deconstructivism.png", "Guggenheim Museum Bilbao", "Deconstructivism", "Decostruttivismo", "contemporary_parametric", "Late 20th Century", 1.0, TraitRadar(9.2, 2.0, 6.0, 9.5, 3.0), emptyList(), "#8E44AD"),
        Style(10, "arch_10_parametric.png", "Heydar Aliyev Cultural Center", "Parametric & Organic", "Architettura Parametrica", "contemporary_parametric", "21st Century", 1.0, TraitRadar(9.8, 3.0, 7.5, 9.0, 4.0), emptyList(), "#16A085")
    )

    private val categories = listOf(
        Category("classical_renaissance", "Classical & Renaissance", "Classico e Rinascimentale", "", "", "#C5A059"),
        Category("historicist_sacred", "Historicist & Sacred", "Storicista e Sacro", "", "", "#800020"),
        Category("early_modern_industrial", "Early Modern & Industrial", "Primo Moderno e Industriale", "", "", "#008080"),
        Category("modernism_functionalism", "Modernism & Functionalism", "Modernismo e Funzionalismo", "", "", "#4A6984"),
        Category("contemporary_parametric", "Contemporary & Parametric", "Contemporaneo e Parametrico", "", "", "#9B59B6")
    )

    // =========================================================================
    // 1. Schedule Invariants in Kotlin
    // =========================================================================
    @Test
    fun verifyScheduleInvariants_inKotlin() {
        val fullSchedule = PairingScheduler.generateSchedule(TournamentMode.FULL)
        assertEquals(45, fullSchedule.size)

        // 9 rounds of 5
        val rounds = fullSchedule.chunked(5)
        assertEquals(9, rounds.size)
        for (r in rounds) {
            val styleSet = mutableSetOf<Int>()
            for (m in r) {
                styleSet.add(m.leftStyleId)
                styleSet.add(m.rightStyleId)
            }
            assertEquals(10, styleSet.size)
        }

        // 45 unique unordered pairs
        val pairs = fullSchedule.map { minOf(it.leftStyleId, it.rightStyleId) to maxOf(it.leftStyleId, it.rightStyleId) }.toSet()
        assertEquals(45, pairs.size)

        // Quick tournament: 15 matches, 3-regular
        val quickSchedule = PairingScheduler.generateSchedule(TournamentMode.QUICK)
        assertEquals(15, quickSchedule.size)
        val counts = IntArray(11)
        for (m in quickSchedule) {
            counts[m.leftStyleId]++
            counts[m.rightStyleId]++
        }
        for (s in 1..10) {
            assertEquals("Style $s must appear 3 times in quick tournament", 3, counts[s])
        }

        // Spacing >= 4
        val lastSeen = mutableMapOf<Int, Int>()
        for ((idx, m) in fullSchedule.withIndex()) {
            for (s in listOf(m.leftStyleId, m.rightStyleId)) {
                val prev = lastSeen[s]
                if (prev != null) {
                    val dist = idx - prev
                    assertTrue("Spacing violation for style $s: dist $dist < 4", dist >= 4)
                }
                lastSeen[s] = idx
            }
        }

        // Left/Right balance 4-5
        val leftCounts = IntArray(11)
        val rightCounts = IntArray(11)
        for (m in fullSchedule) {
            leftCounts[m.leftStyleId]++
            rightCounts[m.rightStyleId]++
        }
        for (s in 1..10) {
            assertTrue("Left count for style $s must be 4 or 5, got ${leftCounts[s]}", leftCounts[s] in 4..5)
            assertTrue("Right count for style $s must be 4 or 5, got ${rightCounts[s]}", rightCounts[s] in 4..5)
        }
    }

    // =========================================================================
    // 2. State Machine: 0 <-> 45 Full Traversal & Branching Invalidation
    // =========================================================================
    @Test
    fun verifyStateMachine_fullUndoRedoCycle_andBranching() {
        var state = TournamentStateMachine.create(TournamentMode.FULL)
        val schedule = PairingScheduler.generateSchedule(TournamentMode.FULL)

        // Vote all 45 matches
        for (i in 0 until 45) {
            val match = state.currentMatch!!
            val winner = match.leftStyleId
            state = TournamentStateMachine.vote(state, winner)
            assertEquals(i + 1, state.currentIndex)
            assertEquals(i + 1, state.historyStack.size)
            assertEquals(0, state.redoStack.size)
        }
        assertTrue(state.isComplete)
        assertFalse(state.canVote)
        assertTrue(state.canUndo)
        assertFalse(state.canRedo)

        val fullWinMatrix = TournamentStateMachine.buildWinMatrix(state)
        val resultInitial = TournamentStateMachine.generateResult(state, styles, categories)

        // Undo all 45 matches down to match 0
        for (i in 44 downTo 0) {
            assertTrue("Should be able to undo at index ${state.currentIndex}", state.canUndo)
            state = TournamentStateMachine.undo(state)
            assertEquals(i, state.currentIndex)
            assertEquals(i, state.historyStack.size)
            assertEquals(45 - i, state.redoStack.size)
        }
        assertEquals(0, state.currentIndex)
        assertFalse(state.canUndo)
        assertTrue(state.canRedo)

        // Verify win matrix is completely empty at 0
        val emptyMatrix = TournamentStateMachine.buildWinMatrix(state)
        for (row in emptyMatrix) {
            for (cell in row) {
                assertEquals(0, cell)
            }
        }

        // Redo all 45 matches forward back to match 45
        for (i in 0 until 45) {
            assertTrue("Should be able to redo at index ${state.currentIndex}", state.canRedo)
            state = TournamentStateMachine.redo(state)
            assertEquals(i + 1, state.currentIndex)
            assertEquals(i + 1, state.historyStack.size)
            assertEquals(44 - i, state.redoStack.size)
        }
        assertTrue(state.isComplete)
        assertFalse(state.canRedo)

        // Verify restored win matrix and result match perfectly
        val restoredMatrix = TournamentStateMachine.buildWinMatrix(state)
        for (r in 0 until 10) {
            for (c in 0 until 10) {
                assertEquals(fullWinMatrix[r][c], restoredMatrix[r][c])
            }
        }

        val resultRestored = TournamentStateMachine.generateResult(state, styles, categories)
        assertEquals(resultInitial.championStyleId, resultRestored.championStyleId)
        assertEquals(resultInitial.dominantCategoryId, resultRestored.dominantCategoryId)
        assertEquals(resultInitial.confidencePercentage, resultRestored.confidencePercentage, 1e-6)
        assertEquals(resultInitial.consistencyZeta, resultRestored.consistencyZeta, 1e-6)

        // Now test branching: undo 10 matches, then cast a different vote
        repeat(10) { state = TournamentStateMachine.undo(state) }
        assertEquals(35, state.currentIndex)
        assertEquals(10, state.redoStack.size)
        assertTrue(state.canRedo)

        // Vote the other style (right instead of left)
        val curMatch = state.currentMatch!!
        val altWinner = curMatch.rightStyleId
        state = TournamentStateMachine.vote(state, altWinner)
        assertEquals(36, state.currentIndex)
        assertEquals(0, state.redoStack.size)
        assertFalse("Branching vote must wipe redo stack", state.canRedo)
    }

    // =========================================================================
    // 3. Exact TC3 from shared_tournament_fixtures.json
    // =========================================================================
    @Test
    fun verifyTC3_actualSharedFixture() {
        val tc3Winners = listOf(10, 9, 8, 7, 6, 2, 3, 9, 8, 6, 10, 4, 5, 6, 7)
        var state = TournamentStateMachine.create(TournamentMode.QUICK)
        for (w in tc3Winners) {
            state = TournamentStateMachine.vote(state, w)
        }
        assertTrue(state.isComplete)

        val res = TournamentStateMachine.generateResult(state, styles, categories)

        // Assertions from shared_tournament_fixtures.json
        assertEquals(6, res.championStyleId)
        assertEquals("modernism_functionalism", res.dominantCategoryId)
        assertEquals(12, res.circularTriads)
        assertEquals(0.7, res.consistencyZeta, 1e-4)
        assertEquals(0.04, res.victoryMargin, 1e-4)
        assertEquals(36.1547, res.confidencePercentage, 0.01)
        assertEquals("brutalist_sculptor", res.archetype.id)
        assertEquals("The Brutalist Sculptor", res.archetype.titleEn)

        // Trait assertions
        assertEquals(6.66, res.userTraits.era, 0.01)
        assertEquals(3.72, res.userTraits.ornamentation, 0.01)
        assertEquals(7.50, res.userTraits.structuralHonesty, 0.01)
        assertEquals(4.06, res.userTraits.geometricOrder, 0.01)
        assertEquals(4.32, res.userTraits.materialWarmth, 0.01)
    }

    // =========================================================================
    // 4. Exact TC4 and TC5 from shared_tournament_fixtures.json
    // =========================================================================
    @Test
    fun verifyTC4_actualSharedFixture() {
        val tc4Winners = listOf(
            1, 2, 3, 4, 5,
            2, 1, 4, 5, 6,
            3, 2, 1, 9, 8,
            4, 3, 2, 1, 8,
            5, 4, 3, 2, 1,
            10, 5, 4, 3, 1,
            10, 8, 5, 1, 3,
            8, 9, 1, 2, 3,
            9, 1, 2, 3, 4
        )
        var state = TournamentStateMachine.create(TournamentMode.FULL)
        for (w in tc4Winners) {
            state = TournamentStateMachine.vote(state, w)
        }
        val res = TournamentStateMachine.generateResult(state, styles, categories)

        assertEquals(1, res.championStyleId)
        assertEquals("classical_renaissance", res.dominantCategoryId)
        assertEquals(0, res.circularTriads)
        assertEquals(1.0, res.consistencyZeta, 1e-4)
        assertEquals(0.018182, res.victoryMargin, 1e-4)
        assertEquals(48.2214, res.confidencePercentage, 0.01)
        assertEquals("classical_monumentalist", res.archetype.id)
    }

    @Test
    fun verifyTC5_actualSharedFixture() {
        val tc5Winners = listOf(
            1, 9, 3, 7, 5,
            2, 3, 4, 8, 6,
            10, 4, 1, 9, 7,
            10, 5, 2, 7, 8,
            5, 6, 3, 2, 1,
            6, 7, 4, 9, 1,
            10, 8, 5, 4, 2,
            10, 9, 1, 5, 3,
            9, 8, 2, 6, 4
        )
        var state = TournamentStateMachine.create(TournamentMode.FULL)
        for (w in tc5Winners) {
            state = TournamentStateMachine.vote(state, w)
        }
        val res = TournamentStateMachine.generateResult(state, styles, categories)

        assertEquals(1, res.championStyleId)
        assertEquals("historicist_sacred", res.dominantCategoryId)
        assertEquals(40, res.circularTriads)
        assertEquals(0.0, res.consistencyZeta, 1e-4)
        assertEquals(15.0, res.confidencePercentage, 0.01)
        assertEquals("eclectic_synthesizer", res.archetype.id)
    }
}
