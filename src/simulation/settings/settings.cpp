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
}
