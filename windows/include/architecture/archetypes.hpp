#pragma once

#include "models.hpp"
#include <vector>
#include <string>

namespace arch::domain {

class ArchetypeEngine {
public:
    /**
     * Accesses the catalog of all 8 narrative archetypes.
     */
    static const std::vector<Archetype>& all_archetypes();

    /**
     * Looks up an archetype by its unique ID (e.g., "classical_monumentalist").
     */
    static const Archetype& find_by_id(const std::string& id);

    /**
     * Looks up an archetype by its English or Italian title.
     */
    static const Archetype& find_by_title(const std::string& title);

    /**
     * Classifies user into 1 of 8 narrative archetypes based on:
     * - consistency zeta (if zeta < 0.50 -> Eclectic Synthesizer)
     * - dominant category
     * - Euclidean distance to centroid in 5D trait space
     */
    static Archetype classify(
        const std::string& dominant_category_id,
        int champion_style_id,
        const TraitRadar& user_traits,
        double victory_margin,
        double consistency_zeta = 1.0
    );
};

} // namespace arch::domain
