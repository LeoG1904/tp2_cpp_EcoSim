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
            {{245, 210, 70},
             {{255, 234, 140}, {255, 234, 140}},
             {{218, 160, 40}, {245, 232, 200}},
             {{214, 166, 104}, {255, 250, 236}},
             "Chance de fuir (%)", 100,
             0.28, 0.17, 0.26},
            {{232, 232, 240},
             {{252, 228, 234}, {238, 120, 165}},
             {{178, 196, 228}, {78, 100, 158}},
             {{208, 205, 214}, {128, 122, 138}},
             "Chance de fuir (%)", 100,
             0.19, 0.12, 0.17},
            {{220, 85, 75},
             {{242, 140, 118}, {255, 232, 214}},
             {{182, 52, 48}, {88, 20, 24}},
             {{200, 128, 112}, {96, 40, 36}},
             "Esprit de meute (vue +)", 4,
             0.34, 0.20, 0.40},
            {{190, 110, 230},
             {{224, 170, 248}, {224, 170, 248}},
             {{150, 72, 204}, {255, 255, 255}},
             {{200, 160, 232}, {200, 160, 232}},
             "Flair (vue +)", 4, 0.25, 0.13, 0.25},
        };
        return styles.at(static_cast<std::size_t>(kind));
    }

    bool IsYoung(const int age) { return age < Animal::AdultAge; }

    const SexLook& LookOf(const AnimalKind kind, const Sex sex, const int age) {
        const SpeciesStyle& style = StyleOf(kind);
        if (IsYoung(age)) return style.young;
        return sex == Sex::Female ? style.female : style.male;
    }
}
