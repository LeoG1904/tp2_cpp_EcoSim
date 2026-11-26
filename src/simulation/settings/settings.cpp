/**
 * @file
 * @brief Valeurs par défaut et préréglages.
 */
module ecosim.settings;

import std;

namespace ecosim {
    PerSpecies<SpeciesSettings> DefaultSpeciesSettings() {
        PerSpecies<SpeciesSettings> result;
        for (const SpeciesInfo& info : SpeciesCatalog()) {
            result[info.kind] = {info.minimumAtStart, {info.defaultBirthPercent}};
        }
        return result;
    }

    SimulationSettings DefaultSettings(const int width, const int height) {
        SimulationSettings settings;
        for (const SpeciesInfo& info : SpeciesCatalog()) {
            settings.species[info.kind].initialCount =
                std::max(info.minimumAtStart, width * height / info.cellsPerAnimal);
            settings.species[info.kind].options.instinct = info.defaultInstinct;
        }
        return settings;
    }

    // Valeurs choisies après des simulations sans affichage sur de nombreuses graines.
    // Un scénario décrit un équilibre précis : il nomme les espèces qu'il modifie.
    SimulationSettings PresetSettings(const Preset preset, const int width, const int height) {
        SimulationSettings settings = DefaultSettings(width, height);
        SpeciesSettings& deer = settings.species[AnimalKind::Deer];
        SpeciesSettings& rabbits = settings.species[AnimalKind::Rabbit];
        SpeciesSettings& predators = settings.species[AnimalKind::Predator];
        switch (preset) {
            case Preset::Balance:
                // Des cerfs souvent sur leurs gardes, face à des prédateurs qui chassent seuls.
                deer.options.instinct = 50;
                predators.options.instinct = 0;
                break;
            case Preset::Famine:
                // Plus de repousse et un seul chasseur : la faim domine.
                settings.vegetationGrowth = 0;
                predators.initialCount = 1;
                break;
            case Preset::Invasion:
                // Les prédateurs sont bien plus nombreux au départ que les herbivores.
                predators.initialCount = std::clamp(width * height / 11, 10, 200);
                // Les proies sont prises par surprise face à de larges meutes.
                deer.options.instinct = 0;
                rabbits.options.instinct = 0;
                predators.options.instinct = 3;
                break;
        }
        return settings;
    }
}
