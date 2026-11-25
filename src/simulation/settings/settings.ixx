// Réglages d'une simulation, sans dépendance à l'interface.
export module ecosim.settings;

import std;
export import ecosim.predator;

export namespace ecosim {
    struct SimulationSettings {
        int waterPercent{10};
        int obstaclePercent{10};
        int vegetationGrowth{1};
        int initialHerbivores{12};
        int initialPredators{3};
        AnimalTraits herbivore{};
        AnimalTraits predator{Predator::DefaultTraits};
    };

    const AnimalTraits& TraitsOf(const SimulationSettings& settings, AnimalKind kind);
    // Les populations de départ suivent la taille de la carte.
    SimulationSettings DefaultSettings(int width, int height);
}
