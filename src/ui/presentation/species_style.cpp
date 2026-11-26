/**
 * @file
 * @brief Couleurs, silhouettes et limites de chaque espèce.
 */
module ecosim.species_style;

import std;

namespace ecosim {
    const SpeciesStyle& StyleOf(const AnimalKind kind) {
        // Une entrée par espèce, dans l'ordre du catalogue.
        static const std::vector<SpeciesStyle> styles{
            {{245, 210, 70}, 500, "Végétation mangée max", 1, 50, "Chance de fuir (%)", 100,
             0.28, 0.17, 0.26},
            {{220, 85, 75}, 200, "Énergie par proie", 5, 100, "Esprit de meute (vue +)", 4,
             0.34, 0.20, 0.40},
        };
        return styles.at(static_cast<std::size_t>(kind));
    }
}
