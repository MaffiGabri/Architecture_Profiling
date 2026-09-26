package com.architecture.profiling.domain.engine

import com.architecture.profiling.domain.model.ArchitecturalArchetype
import com.architecture.profiling.domain.model.Style
import com.architecture.profiling.domain.model.TraitRadar

object ArchetypeEngine {

    val CATALOG: List<ArchitecturalArchetype> = listOf(
        ArchitecturalArchetype(
            id = "purist_rationalist",
            titleEn = "The Purist Rationalist",
            titleIt = "Il Razionalista Puro",
            taglineEn = "Form follows function without compromise.",
            taglineIt = "La forma segue la funzione senza compromessi.",
            narrativeEn = "You value absolute spatial clarity, geometric honesty, and unornamented elegance. For you, architecture is a calibrated instrument of living where structure itself becomes beauty.",
            narrativeIt = "Valuti l'assoluta chiarezza spaziale, l'onestà geometrica e l'eleganza priva di ornamenti. Per te l'architettura è uno strumento calibrato dell'abitare dove la struttura stessa diventa bellezza.",
            dominantCategoryId = "modernism_functionalism"
        ),
        ArchitecturalArchetype(
            id = "classical_monumentalist",
            titleEn = "The Classical Monumentalist",
            titleIt = "Il Monumentalista Classico",
            taglineEn = "Harmony in eternal proportion and golden ratios.",
            taglineIt = "Armonia in eterne proporzioni e sezioni auree.",
            narrativeEn = "You find sanctuary in eternal geometries, tripartite orders, and the dignity of ancient civic spaces. Architecture is an enduring testament to humanistic harmony and timeless order.",
            narrativeIt = "Trovi rifugio nelle geometrie eterne, negli ordini tripartiti e nella dignità degli spazi civici antichi. L'architettura è una testimonianza duratura di armonia umanistica e ordine senza tempo.",
            dominantCategoryId = "classical_renaissance"
        ),
        ArchitecturalArchetype(
            id = "brutalist_sculptor",
            titleEn = "The Brutalist Sculptor",
            titleIt = "Lo Scultore Brutalista",
            taglineEn = "The poetry of raw concrete and unvarnished mass.",
            taglineIt = "La poesia del cemento a vista e della massa tettonica.",
            narrativeEn = "You celebrate tectonic truth. Massive cantilevers, exposed timber-grained concrete, and monolithic permanence command your deep aesthetic respect.",
            narrativeIt = "Celebri la verità tettonica. Sbalzi possenti, cemento armato a vista e permanenza monolitica esigono il tuo profondo rispetto estetico.",
            dominantCategoryId = "modernism_functionalism"
        ),
        ArchitecturalArchetype(
            id = "romantic_historian",
            titleEn = "The Romantic Historian",
            titleIt = "Lo Storico Romantico",
            taglineEn = "Architecture as sacred narrative, mystery, and drama.",
            taglineIt = "L'architettura come narrazione sacra, mistero e dramma.",
            narrativeEn = "For you, architecture is emotional theatre and spiritual resonance. You are drawn to soaring ribbed vaults, chiaroscuro lighting, and intricate sculptural symbolism.",
            narrativeIt = "Per te l'architettura è teatro emozionale e risonanza spirituale. Sei attratto da volte a costoloni slanciate, chiaroscuri teatrali e intricato simbolismo scultoreo.",
            dominantCategoryId = "historicist_sacred"
        ),
        ArchitecturalArchetype(
            id = "parametric_visionary",
            titleEn = "The Parametric Visionary",
            titleIt = "Il Visionario Parametrico",
            taglineEn = "Fluid computational biology translated into built form.",
            taglineIt = "Biologia computazionale fluida tradotta in forma costruita.",
            narrativeEn = "You inhabit the future of spatial design. Double-curved continuous envelopes, topological fluidity, and seamless transitions define your avant-garde vision.",
            narrativeIt = "Abiti il futuro del design spaziale. Involucri continui a doppia curvatura, fluidità topologica e transizioni senza soluzione di continuità definiscono la tua visione d'avanguardia.",
            dominantCategoryId = "contemporary_parametric"
        ),
        ArchitecturalArchetype(
            id = "hightech_pragmatist",
            titleEn = "The High-Tech Pragmatist",
            titleIt = "Il Pragmatico High-Tech",
            taglineEn = "The building as an expressive, living machine.",
            taglineIt = "L'edificio come macchina espressiva e vivente.",
            narrativeEn = "You celebrate engineering honesty and exposed external steel exoskeletons. Structure and mechanical servicing become the primary artistic expression.",
            narrativeIt = "Celebri l'onestà ingegneristica e gli esoscheletri d'acciaio a vista. La struttura e gli impianti tecnologici diventano l'espressione artistica primaria.",
            dominantCategoryId = "early_modern_industrial"
        ),
        ArchitecturalArchetype(
            id = "avantgarde_deconstructivist",
            titleEn = "The Avant-Garde Deconstructivist",
            titleIt = "Il Decostruttivista d'Avanguardia",
            taglineEn = "Controlled chaos and subversive geometric distortion.",
            taglineIt = "Caos controllato e distorsione geometrica sovversiva.",
            narrativeEn = "You challenge spatial complacency through non-orthogonal intersecting volumes, fragmented planes, and dynamic kinetic tension.",
            narrativeIt = "Sfidi l'abitudine spaziale attraverso volumi intersecati non ortogonali, piani frammentati e dinamica tensione cinetica.",
            dominantCategoryId = "contemporary_parametric"
        ),
        ArchitecturalArchetype(
            id = "eclectic_synthesizer",
            titleEn = "The Eclectic Synthesizer",
            titleIt = "Il Sintetizzatore Eclettico",
            taglineEn = "Harmonizing historical depth with modern audacity.",
            taglineIt = "Armonizzazione tra profondità storica e audacia moderna.",
            narrativeEn = "You refuse to be bound by a single dogma. Your aesthetic synthesizes timeless historical proportion with bold modern innovation into a nuanced, multi-layered sensibility.",
            narrativeIt = "Rifiuti di farti vincolare da un singolo dogma. La tua estetica sintetizza l'armonia storica classica con l'audacia moderna in una sensibilità sfaccettata ed elegante.",
            dominantCategoryId = "eclectic"
        )
    )

    fun findById(id: String): ArchitecturalArchetype =
        CATALOG.find { it.id == id } ?: CATALOG.last()

    fun calculateUserTraits(probabilities: DoubleArray, styles: List<Style>): TraitRadar {
        val styleMap = styles.associateBy { it.id }
        var result = TraitRadar.ZERO
        for (i in probabilities.indices) {
            val style = styleMap[i + 1] ?: continue
            result += style.traits * probabilities[i]
        }
        return result
    }

    /**
     * Classifies user into 1 of 8 narrative archetypes based on transitivity zeta,
     * dominant category, and 5D trait Euclidean distance centroids.
     */
    fun classify(
        dominantCategoryId: String,
        userTraits: TraitRadar,
        zeta: Double,
        styles: List<Style>
    ): ArchitecturalArchetype {
        // Inconsistent or contradictory preferences default to Eclectic Synthesizer
        if (zeta < 0.50) {
            return findById("eclectic_synthesizer")
        }

        val styleMap = styles.associateBy { it.id }

        return when (dominantCategoryId) {
            "classical_renaissance" -> findById("classical_monumentalist")
            "historicist_sacred" -> findById("romantic_historian")
            "modernism_functionalism" -> {
                val puristTraits = styleMap[6]?.traits ?: TraitRadar.ZERO
                val brutalistTraits = styleMap[7]?.traits ?: TraitRadar.ZERO
                val dPurist = userTraits.distanceSquared(puristTraits)
                val dBrutalist = userTraits.distanceSquared(brutalistTraits)
                if (dBrutalist < dPurist) findById("brutalist_sculptor") else findById("purist_rationalist")
            }
            "contemporary_parametric" -> {
                val deconTraits = styleMap[9]?.traits ?: TraitRadar.ZERO
                val paramTraits = styleMap[10]?.traits ?: TraitRadar.ZERO
                val dDecon = userTraits.distanceSquared(deconTraits)
                val dParam = userTraits.distanceSquared(paramTraits)
                if (dDecon < dParam) findById("avantgarde_deconstructivist") else findById("parametric_visionary")
            }
            "early_modern_industrial" -> {
                val hightechTraits = styleMap[8]?.traits ?: TraitRadar.ZERO
                val eclecticTraits = styleMap[5]?.traits ?: TraitRadar.ZERO
                val dHighTech = userTraits.distanceSquared(hightechTraits)
                val dEclectic = userTraits.distanceSquared(eclecticTraits)
                if (dHighTech < dEclectic) findById("hightech_pragmatist") else findById("eclectic_synthesizer")
            }
            else -> findById("eclectic_synthesizer")
        }
    }
}
