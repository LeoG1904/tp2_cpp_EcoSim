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
            {{245, 210, 70}, 500, "Chance de fuir (%)", 100,
             0.28, 0.17, 0.26},
            {{232, 232, 240}, 500, "Chance de fuir (%)", 100,
             0.19, 0.12, 0.17},
            {{220, 85, 75}, 200, "Esprit de meute (vue +)", 4,
             0.34, 0.20, 0.40},
            {{190, 110, 230}, 200, "Flair (vue +)", 4, 0.25, 0.13, 0.25},
        };
        return styles.at(static_cast<std::size_t>(kind));
    }
}
