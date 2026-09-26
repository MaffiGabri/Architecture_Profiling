package com.architecture.profiling.domain

import com.architecture.profiling.domain.engine.*
import com.architecture.profiling.domain.model.*
import org.junit.Assert.*
import org.junit.Test

class SharedFixturesTest {

    private val mockStyles = listOf(
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

    private val mockCategories = listOf(
        Category("classical_renaissance", "Classical & Renaissance", "Classico e Rinascimentale", "", "", "#C5A059"),
        Category("historicist_sacred", "Historicist & Sacred", "Storicista e Sacro", "", "", "#800020"),
        Category("early_modern_industrial", "Early Modern & Industrial", "Primo Moderno e Industriale", "", "", "#008080"),
        Category("modernism_functionalism", "Modernism & Functionalism", "Modernismo e Funzionalismo", "", "", "#4A6984"),
        Category("contemporary_parametric", "Contemporary & Parametric", "Contemporaneo e Parametrico", "", "", "#9B59B6")
    )

    private fun runTournament(mode: TournamentMode, winners: List<Int>): TournamentResult {
        var state = TournamentStateMachine.create(mode)
        for (w in winners) {
            state = TournamentStateMachine.vote(state, w)
        }
        return TournamentStateMachine.generateResult(state, mockStyles, mockCategories)
    }

    @Test
    fun testCase1_transitiveDominance() {
        val schedule = PairingScheduler.generateSchedule(TournamentMode.FULL)
        val winners = schedule.map { minOf(it.leftStyleId, it.rightStyleId) }
        val res = runTournament(TournamentMode.FULL, winners)

        assertEquals(1, res.championStyleId)
        assertEquals("classical_renaissance", res.dominantCategoryId)
        assertEquals(0, res.circularTriads)
        assertEquals(1.0, res.consistencyZeta, 1e-4)
        assertEquals(0.018182, res.victoryMargin, 1e-4)
        assertEquals(48.2214, res.confidencePercentage, 0.01)
        assertEquals("classical_monumentalist", res.archetype.id)
    }

    @Test
    fun testCase2_cyclicContradiction() {
        val schedule = PairingScheduler.generateSchedule(TournamentMode.FULL)
        val winners = schedule.map { pair ->
            val diff = (pair.rightStyleId - pair.leftStyleId + 10) % 10
            when {
                diff in 1..4 -> pair.leftStyleId
                diff in 6..9 -> pair.rightStyleId
                else -> minOf(pair.leftStyleId, pair.rightStyleId)
            }
        }
        val res = runTournament(TournamentMode.FULL, winners)

        assertEquals(40, res.circularTriads)
        assertEquals(0.0, res.consistencyZeta, 1e-4)
        assertEquals(15.0, res.confidencePercentage, 0.01)
        assertEquals("eclectic_synthesizer", res.archetype.id)
    }

    @Test
    fun testCase3_quickTournament15() {
        val winners = listOf(1, 2, 3, 7, 6, 2, 1, 4, 5, 6, 3, 2, 1, 6, 7)
        val res = runTournament(TournamentMode.QUICK, winners)

        assertEquals(1, res.championStyleId)
        assertEquals(0, res.circularTriads)
        assertEquals(1.0, res.consistencyZeta, 1e-4)
        assertEquals(46.0745, res.confidencePercentage, 0.01)
        assertEquals("classical_monumentalist", res.archetype.id)
    }

    @Test
    fun testCase4_categoryPolarizedClassical() {
        val schedule = PairingScheduler.generateSchedule(TournamentMode.FULL)
        val winners = schedule.map { pair ->
            val u = pair.leftStyleId
            val v = pair.rightStyleId
            val isClassicalU = u in listOf(1, 3)
            val isClassicalV = v in listOf(1, 3)
            val isModernU = u in listOf(6, 7)
            val isModernV = v in listOf(6, 7)
            when {
                isClassicalU && !isClassicalV -> u
                isClassicalV && !isClassicalU -> v
                isModernU && !isModernV -> v
                isModernV && !isModernU -> u
                else -> minOf(u, v)
            }
        }
        val res = runTournament(TournamentMode.FULL, winners)

        assertEquals(1, res.championStyleId)
        assertEquals("classical_renaissance", res.dominantCategoryId)
        assertEquals(0.345455, res.categoryRankings[0].affinity, 1e-4)
        assertEquals(0, res.circularTriads)
        assertEquals(1.0, res.consistencyZeta, 1e-4)
        assertEquals(48.2214, res.confidencePercentage, 0.01)
        assertEquals("classical_monumentalist", res.archetype.id)
    }

    @Test
    fun testCase5_uniformNoise() {
        val schedule = PairingScheduler.generateSchedule(TournamentMode.FULL)
        val winners = schedule.mapIndexed { idx, pair ->
            if (idx % 2 == 0) pair.leftStyleId else pair.rightStyleId
        }
        val res = runTournament(TournamentMode.FULL, winners)

        assertEquals(40, res.circularTriads)
        assertEquals(0.0, res.consistencyZeta, 1e-4)
        assertEquals(15.0, res.confidencePercentage, 0.01)
        assertEquals("eclectic_synthesizer", res.archetype.id)
    }
}
