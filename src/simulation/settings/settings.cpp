module ecosim.settings;

import std;

namespace ecosim {
    PerSpecies<SpeciesSettings> DefaultSpeciesSettings() {
        PerSpecies<SpeciesSettings> result;
        for (const SpeciesInfo& info : SpeciesCatalog()) {
            result[info.kind] = {info.minimumAtStart, info.defaultTraits};
        }
        return result;
    }

    SimulationSettings DefaultSettings(const int width, const int height) {
        SimulationSettings settings;
        for (const SpeciesInfo& info : SpeciesCatalog()) {
            settings.species[info.kind].initialCount =
                std::max(info.minimumAtStart, width * height / info.cellsPerAnimal);
        }
        return settings;
    }

    // Valeurs choisies après des simulations sans affichage sur de nombreuses graines.
    // Un scénario décrit un équilibre précis : il nomme les espèces qu'il modifie.
    SimulationSettings PresetSettings(const Preset preset, const int width, const int height) {
        SimulationSettings settings = DefaultSettings(width, height);
        SpeciesSettings& herbivores = settings.species[AnimalKind::Herbivore];
        SpeciesSettings& predators = settings.species[AnimalKind::Predator];
        switch (preset) {
            case Preset::Balance:
                // Herbivores durables et prolifiques ; prédateurs qui se reproduisent peu.
                herbivores.traits.maxAge = 120;
                herbivores.traits.reproductionEnergy = 70;
                herbivores.traits.reproductionPercent = 40;
                predators.traits.energyLoss = 3;
                predators.traits.maxAge = 60;
                predators.traits.foodEnergy = 15;
                predators.traits.reproductionEnergy = 70;
                predators.traits.reproductionPercent = 5;
                break;
            case Preset::Famine:
                // Plus de repousse et peu de chasseurs : la faim domine.
                settings.vegetationGrowth = 0;
                herbivores.traits.energyLoss = 3;
                predators.initialCount = std::max(1, predators.initialCount / 3);
                break;
            case Preset::Invasion:
                // Les prédateurs sont bien plus nombreux au départ que les herbivores.
                predators.initialCount = std::clamp(width * height / 7, 10, 200);
                break;
        }
        return settings;
    }
}
