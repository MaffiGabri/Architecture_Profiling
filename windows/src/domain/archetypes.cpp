#include "architecture/archetypes.hpp"
#include <algorithm>

namespace arch::domain {

const std::vector<Archetype>& ArchetypeEngine::all_archetypes() {
    static const std::vector<Archetype> catalog = {
        {
            "purist_rationalist",
            "The Purist Rationalist", "Il Razionalista Puro",
            "Form follows function without compromise.", "La forma segue la funzione senza compromessi.",
            "You value absolute spatial clarity, geometric honesty, and unornamented elegance. For you, architecture reaches its zenith when every element has purpose and excess is stripped away.",
            "Valuti l'assoluta chiarezza spaziale, l'onestà geometrica e l'eleganza priva di ornamenti. Per te l'architettura raggiunge il suo apice quando ogni elemento ha uno scopo e il superfluo viene eliminato.",
            "modernism_functionalism"
        },
        {
            "classical_monumentalist",
            "The Classical Monumentalist", "Il Monumentalista Classico",
            "Harmony in eternal proportion and golden ratios.", "Armonia in eterne proporzioni e sezioni auree.",
            "You find sanctuary in eternal geometries, tripartite orders, and the dignity of ancient civic spaces. Proportion, symmetry, and noble materials define your architectural ideal.",
            "Trovi rifugio nelle geometrie eterne, negli ordini tripartiti e nella dignità degli spazi civici antichi. Proporzione, simmetria e materiali nobili definiscono il tuo ideale architettonico.",
            "classical_renaissance"
        },
        {
            "brutalist_sculptor",
            "The Brutalist Sculptor", "Lo Scultore Brutalista",
            "The poetry of raw concrete and unvarnished mass.", "La poesia del cemento a vista e della massa tettonica.",
            "You celebrate tectonic truth. Massive cantilevers, exposed aggregate, and bold geometric silhouettes command your respect through their sheer material presence and permanence.",
            "Celebri la verità tettonica. Sbalzi possenti, aggregato a vista e audaci silhouette geometriche esigono il tuo rispetto attraverso la loro presenza materica e permanenza.",
            "modernism_functionalism"
        },
        {
            "romantic_historian",
            "The Romantic Historian", "Lo Storico Romantico",
            "Architecture as sacred narrative, mystery, and drama.", "L'architettura come narrazione sacra, mistero e dramma.",
            "For you, architecture is emotional theatre and spiritual resonance. Vaulted arches, intricate stonework, and dramatic light create spaces charged with memory and transcendent atmosphere.",
            "Per te l'architettura è teatro emozionale e risonanza spirituale. Volte slanciate, pietra scolpita e chiaroscuri drammatici creano ambienti carichi di memoria e atmosfera trascendente.",
            "historicist_sacred"
        },
        {
            "parametric_visionary",
            "The Parametric Visionary", "Il Visionario Parametrico",
            "Fluid computational biology translated into built form.", "Biologia computazionale fluida tradotta in forma costruita.",
            "You inhabit the cutting edge of spatial design, where algorithms generate continuous topological surfaces, organic curves, and structures that seem to grow rather than be built.",
            "Abiti la frontiera del design spaziale, dove algoritmi generano superfici topologiche continue, curvature organiche e strutture che sembrano germogliare anziché essere costruite.",
            "contemporary_parametric"
        },
        {
            "hightech_pragmatist",
            "The High-Tech Pragmatist", "Il Pragmatico High-Tech",
            "The building as an expressive, living machine.", "L'edificio come macchina espressiva e vivente.",
            "You celebrate engineering honesty and exposed infrastructure. External trusses, glass membranes, and articulated mechanical systems turn functionality into high art.",
            "Celebri l'onestà ingegneristica e l'infrastruttura a vista. Reticolari d'acciaio esterne, membrane di vetro e sistemi meccanici articolati trasformano la funzione in pura arte.",
            "early_modern_industrial"
        },
        {
            "avantgarde_deconstructivist",
            "The Avant-Garde Deconstructivist", "Il Decostruttivista d'Avanguardia",
            "Controlled chaos and subversive geometric distortion.", "Caos controllato e distorsione geometrica sovversiva.",
            "You challenge spatial complacency through non-orthogonal intersecting planes, fragmented volumes, and deliberate visual instability that awakens the senses.",
            "Sfidi l'abitudine spaziale attraverso piani intersecati non ortogonali, volumi frammentati e una deliberata instabilità visiva che risveglia i sensi.",
            "contemporary_parametric"
        },
        {
            "eclectic_synthesizer",
            "The Eclectic Synthesizer", "Il Sintetizzatore Eclettico",
            "Harmonizing historical depth with modern audacity.", "Armonizzazione tra profondità storica e audacia moderna.",
            "You refuse to be bound by a single architectural dogma. You synthesize classic ornament with modern materials, celebrating hybridity, cosmopolitan taste, and rich dialogue between eras.",
            "Rifiuti di farti vincolare da un singolo dogma architettonico. Sintetizzi ornamento classico e materiali moderni, celebrando ibridazione, gusto cosmopolita e dialogo fecondo tra le epoche.",
            "eclectic"
        }
    };
    return catalog;
}

const Archetype& ArchetypeEngine::find_by_id(const std::string& id) {
    const auto& catalog = all_archetypes();
    for (const auto& a : catalog) {
        if (a.id == id) {
            return a;
        }
    }
    return catalog.back(); // Fallback to eclectic synthesizer
}

const Archetype& ArchetypeEngine::find_by_title(const std::string& title) {
    const auto& catalog = all_archetypes();
    for (const auto& a : catalog) {
        if (a.title_en == title || a.title_it == title) {
            return a;
        }
    }
    return catalog.back(); // Fallback
}

Archetype ArchetypeEngine::classify(
    const std::string& dominant_category_id,
    int /*champion_style_id*/,
    const TraitRadar& user_traits,
    double /*victory_margin*/,
    double consistency_zeta
) {
    // 5D trait centroids defined in Explorer 2 & simulation
    static constexpr TraitRadar centroid_purist{7.00, 0.50, 9.00, 0.50, 2.00};
    static constexpr TraitRadar centroid_brutalist{7.80, 0.00, 10.00, 3.50, 4.50};
    static constexpr TraitRadar centroid_deconstructivist{9.20, 2.00, 6.00, 9.50, 3.00};
    static constexpr TraitRadar centroid_parametric{9.80, 3.00, 7.50, 9.00, 4.00};
    static constexpr TraitRadar centroid_hightech{8.20, 1.50, 10.00, 4.00, 1.00};
    static constexpr TraitRadar centroid_eclectic{6.50, 7.00, 5.00, 2.00, 4.00};

    // Rule 1: Inconsistent votes (zeta < 0.50) indicate eclectic / ambivalent preference
    if (consistency_zeta < 0.50) {
        return find_by_id("eclectic_synthesizer");
    }

    // Rule 2: Classical & Renaissance
    if (dominant_category_id == "classical_renaissance") {
        return find_by_id("classical_monumentalist");
    }

    // Rule 3: Historicist & Sacred
    if (dominant_category_id == "historicist_sacred") {
        return find_by_id("romantic_historian");
    }

    // Rule 4: Modernism & Functionalism (Purist vs Brutalist)
    if (dominant_category_id == "modernism_functionalism") {
        double d_purist = user_traits.distance_squared(centroid_purist);
        double d_brutalist = user_traits.distance_squared(centroid_brutalist);
        return (d_brutalist < d_purist)
            ? find_by_id("brutalist_sculptor")
            : find_by_id("purist_rationalist");
    }

    // Rule 5: Contemporary & Parametric (Deconstructivist vs Parametric)
    if (dominant_category_id == "contemporary_parametric") {
        double d_decon = user_traits.distance_squared(centroid_deconstructivist);
        double d_param = user_traits.distance_squared(centroid_parametric);
        return (d_decon < d_param)
            ? find_by_id("avantgarde_deconstructivist")
            : find_by_id("parametric_visionary");
    }

    // Rule 6: Early Modern & Industrial (High-Tech vs Eclectic)
    if (dominant_category_id == "early_modern_industrial") {
        double d_hightech = user_traits.distance_squared(centroid_hightech);
        double d_eclectic = user_traits.distance_squared(centroid_eclectic);
        return (d_hightech < d_eclectic)
            ? find_by_id("hightech_pragmatist")
            : find_by_id("eclectic_synthesizer");
    }

    return find_by_id("eclectic_synthesizer");
}

} // namespace arch::domain
