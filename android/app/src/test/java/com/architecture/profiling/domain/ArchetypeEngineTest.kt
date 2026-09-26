package com.architecture.profiling.domain

import com.architecture.profiling.domain.engine.ArchetypeEngine
import com.architecture.profiling.domain.model.Style
import com.architecture.profiling.domain.model.TraitRadar
import org.junit.Assert.*
import org.junit.Test

class ArchetypeEngineTest {

    private val mockStyles = listOf(
        Style(1, "arch_01.png", "Parthenon", "Classical Antiquity", "Antichita Classica", "classical_renaissance", "5th BC", 1.0, TraitRadar(0.5, 5.5, 7.0, 0.5, 8.5), emptyList(), "#D4AF37"),
        Style(2, "arch_02.png", "Notre Dame", "Gothic Architecture", "Architettura Gotica", "historicist_sacred", "12th", 1.0, TraitRadar(2.0, 9.5, 8.5, 2.5, 6.5), emptyList(), "#6A1B9A"),
        Style(3, "arch_03.png", "Florence Dome", "Italian Renaissance", "Rinascimento", "classical_renaissance", "15th", 1.0, TraitRadar(3.5, 6.5, 5.5, 1.0, 8.0), emptyList(), "#C0392B"),
        Style(4, "arch_04.png", "San Carlo", "Baroque Architecture", "Barocco", "historicist_sacred", "17th", 1.0, TraitRadar(4.5, 10.0, 3.0, 5.0, 8.5), emptyList(), "#B8860B"),
        Style(5, "arch_05.png", "Chrysler", "Art Deco", "Art Deco", "early_modern_industrial", "20th", 1.0, TraitRadar(6.5, 7.0, 5.0, 2.0, 4.0), emptyList(), "#008B8B"),
        Style(6, "arch_06.png", "Bauhaus", "Bauhaus", "Bauhaus", "modernism_functionalism", "20th", 1.0, TraitRadar(7.0, 0.5, 9.0, 0.5, 2.0), emptyList(), "#E74C3C"),
        Style(7, "arch_07.png", "City Hall", "Brutalism", "Brutalismo", "modernism_functionalism", "20th", 1.0, TraitRadar(7.8, 0.0, 10.0, 3.5, 4.5), emptyList(), "#708090"),
        Style(8, "arch_08.png", "Pompidou", "High-Tech Architecture", "High-Tech", "early_modern_industrial", "20th", 1.0, TraitRadar(8.2, 1.5, 10.0, 4.0, 1.0), emptyList(), "#2980B9"),
        Style(9, "arch_09.png", "Guggenheim", "Deconstructivism", "Decostruttivismo", "contemporary_parametric", "20th", 1.0, TraitRadar(9.2, 2.0, 6.0, 9.5, 3.0), emptyList(), "#8E44AD"),
        Style(10, "arch_10.png", "Heydar Aliyev", "Parametric & Organic", "Parametrico", "contemporary_parametric", "21st", 1.0, TraitRadar(9.8, 3.0, 7.5, 9.0, 4.0), emptyList(), "#16A085")
    )

    @Test
    fun catalog_containsAll8DistinctArchetypes() {
        assertEquals(8, ArchetypeEngine.CATALOG.size)
        val uniqueIds = ArchetypeEngine.CATALOG.map { it.id }.toSet()
        assertEquals(8, uniqueIds.size)
    }

    @Test
    fun lowConsistency_defaultsToEclecticSynthesizer() {
        val arch = ArchetypeEngine.classify(
            dominantCategoryId = "classical_renaissance",
            userTraits = TraitRadar(0.5, 5.5, 7.0, 0.5, 8.5),
            zeta = 0.40,
            styles = mockStyles
        )
        assertEquals("eclectic_synthesizer", arch.id)
    }

    @Test
    fun classicalRenaissance_classifiedAsClassicalMonumentalist() {
        val arch = ArchetypeEngine.classify(
            dominantCategoryId = "classical_renaissance",
            userTraits = TraitRadar(2.0, 6.0, 6.25, 0.75, 8.25),
            zeta = 1.0,
            styles = mockStyles
        )
        assertEquals("classical_monumentalist", arch.id)
    }

    @Test
    fun historicistSacred_classifiedAsRomanticHistorian() {
        val arch = ArchetypeEngine.classify(
            dominantCategoryId = "historicist_sacred",
            userTraits = TraitRadar(3.25, 9.75, 5.75, 3.75, 7.5),
            zeta = 1.0,
            styles = mockStyles
        )
        assertEquals("romantic_historian", arch.id)
    }

    @Test
    fun modernismSubstyles_classifiedCorrectlyViaDistance() {
        // Closer to Bauhaus (Style 6)
        val puristArch = ArchetypeEngine.classify(
            dominantCategoryId = "modernism_functionalism",
            userTraits = TraitRadar(7.0, 0.5, 9.0, 0.5, 2.0),
            zeta = 1.0,
            styles = mockStyles
        )
        assertEquals("purist_rationalist", puristArch.id)

        // Closer to Brutalism (Style 7)
        val brutalistArch = ArchetypeEngine.classify(
            dominantCategoryId = "modernism_functionalism",
            userTraits = TraitRadar(7.8, 0.0, 10.0, 3.5, 4.5),
            zeta = 1.0,
            styles = mockStyles
        )
        assertEquals("brutalist_sculptor", brutalistArch.id)
    }

    @Test
    fun contemporaryParametricSubstyles_classifiedCorrectlyViaDistance() {
        // Closer to Deconstructivism (Style 9)
        val deconArch = ArchetypeEngine.classify(
            dominantCategoryId = "contemporary_parametric",
            userTraits = TraitRadar(9.2, 2.0, 6.0, 9.5, 3.0),
            zeta = 1.0,
            styles = mockStyles
        )
        assertEquals("avantgarde_deconstructivist", deconArch.id)

        // Closer to Parametric (Style 10)
        val paramArch = ArchetypeEngine.classify(
            dominantCategoryId = "contemporary_parametric",
            userTraits = TraitRadar(9.8, 3.0, 7.5, 9.0, 4.0),
            zeta = 1.0,
            styles = mockStyles
        )
        assertEquals("parametric_visionary", paramArch.id)
    }

    @Test
    fun earlyModernSubstyles_classifiedCorrectlyViaDistance() {
        // Closer to High-Tech (Style 8)
        val hightechArch = ArchetypeEngine.classify(
            dominantCategoryId = "early_modern_industrial",
            userTraits = TraitRadar(8.2, 1.5, 10.0, 4.0, 1.0),
            zeta = 1.0,
            styles = mockStyles
        )
        assertEquals("hightech_pragmatist", hightechArch.id)

        // Closer to Art Deco / Eclectic (Style 5)
        val eclecticArch = ArchetypeEngine.classify(
            dominantCategoryId = "early_modern_industrial",
            userTraits = TraitRadar(6.5, 7.0, 5.0, 2.0, 4.0),
            zeta = 1.0,
            styles = mockStyles
        )
        assertEquals("eclectic_synthesizer", eclecticArch.id)
    }
}
