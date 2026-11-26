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
            settings.species[info.kind].traits.instinct = info.defaultInstinct;
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
                // Des proies rapides et souvent sur leurs gardes face à des chasseurs plus lents,
                // qui profitent bien de chaque capture et vivent moins longtemps.
                herbivores.traits.speed = 120;
                herbivores.traits.reproductionPercent = 30;
                herbivores.traits.instinct = 50;
                predators.traits.speed = 80;
                predators.traits.energyLoss = 3;
                predators.traits.maxAge = 60;
                predators.traits.foodEnergy = 25;
                predators.traits.reproductionEnergy = 70;
                predators.traits.reproductionPercent = 15;
                predators.traits.instinct = 1;
                break;
            case Preset::Famine:
                // Plus de repousse et un seul chasseur lent : la faim domine.
                settings.vegetationGrowth = 0;
                herbivores.traits.energyLoss = 3;
                predators.initialCount = 1;
                predators.traits.speed = 80;
                break;
            case Preset::Invasion:
                // Les prédateurs sont bien plus nombreux au départ que les herbivores.
                predators.initialCount = std::clamp(width * height / 11, 10, 200);
                // Les proies sont prises par surprise face à de larges meutes.
                herbivores.traits.instinct = 0;
                predators.traits.instinct = 3;
                break;
        }
        return settings;
    }
}
