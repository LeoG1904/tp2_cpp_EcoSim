module ecosim.settings;

import std;

namespace ecosim {
    const AnimalTraits& TraitsOf(const SimulationSettings& settings, const AnimalKind kind) {
        return kind == AnimalKind::Herbivore ? settings.herbivore : settings.predator;
    }

    SimulationSettings DefaultSettings(const int width, const int height) {
        SimulationSettings settings;
        settings.initialHerbivores = std::max(12, width * height / 15);
        settings.initialPredators = std::max(3, width * height / 150);
        return settings;
    }

    // Valeurs choisies après des simulations sans affichage sur de nombreuses graines.
    SimulationSettings PresetSettings(const Preset preset, const int width, const int height) {
        SimulationSettings settings = DefaultSettings(width, height);
        switch (preset) {
            case Preset::Balance:
                // Herbivores durables et prolifiques ; prédateurs qui se reproduisent peu.
                settings.herbivore.maxAge = 120;
                settings.herbivore.reproductionEnergy = 70;
                settings.herbivore.reproductionPercent = 40;
                settings.predator.energyLoss = 3;
                settings.predator.maxAge = 60;
                settings.predator.foodEnergy = 15;
                settings.predator.reproductionEnergy = 70;
                settings.predator.reproductionPercent = 5;
                break;
            case Preset::Famine:
                // Plus de repousse et peu de chasseurs : la faim domine.
                settings.vegetationGrowth = 0;
                settings.herbivore.energyLoss = 3;
                settings.initialPredators = std::max(1, settings.initialPredators / 3);
                break;
            case Preset::Invasion:
                // Les prédateurs sont bien plus nombreux au départ que les herbivores.
                settings.initialPredators = std::clamp(width * height / 7, 10, 200);
                break;
        }
        return settings;
    }
}
