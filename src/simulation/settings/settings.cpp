/**
 * @file
 * @brief Valeurs par défaut et préréglages.
 */
module ecosim.settings;

import std;

namespace {
    /// @brief Le nombre d'animaux pour une surface : un pour tant de cases, entre un minimum et InputLimit.
    /// @param width Largeur de la carte.
    /// @param height Hauteur de la carte.
    /// @param cellsPerAnimal Cases pour un animal.
    /// @param minimum Plus petit nombre rendu.
    int AnimalsFor(const int width, const int height, const int cellsPerAnimal, const int minimum) {
        // La surface d'une très grande carte dépasse la capacité d'un int.
        const long long wanted = static_cast<long long>(width) * height / cellsPerAnimal;
        return static_cast<int>(std::clamp<long long>(wanted, minimum, ecosim::InputLimit));
    }
}

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
        settings.mapWidth = width;
        settings.mapHeight = height;
        for (const SpeciesInfo& info : SpeciesCatalog()) {
            settings.species[info.kind].initialCount =
                AnimalsFor(width, height, info.cellsPerAnimal, info.minimumAtStart);
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
        SpeciesSettings& wolves = settings.species[AnimalKind::Wolf];
        SpeciesSettings& foxes = settings.species[AnimalKind::Fox];
        switch (preset) {
            case Preset::Balance:
                // Des proies souvent sur leurs gardes, face à des chasseurs sans bonus de vue ni meute.
                deer.options.instinct = 50;
                rabbits.options.instinct = 50;
                wolves.options.instinct = 0;
                foxes.options.instinct = 0;
                break;
            case Preset::Famine:
                // Plus de repousse et un seul chasseur de chaque espèce : la faim domine.
                settings.vegetationGrowth = 0;
                wolves.initialCount = 1;
                foxes.initialCount = 1;
                break;
            case Preset::Invasion:
                // Les loups sont bien plus nombreux au départ que les herbivores.
                wolves.initialCount = AnimalsFor(width, height, 11, 10);
                // Les proies sont prises par surprise face à de larges meutes.
                deer.options.instinct = 0;
                rabbits.options.instinct = 0;
                wolves.options.instinct = 3;
                break;
        }
        return settings;
    }

    void ResizeMap(SimulationSettings& settings, const int width, const int height) {
        if (settings.mapWidth == width && settings.mapHeight == height) return;
        for (const Preset preset : Presets) {
            if (settings == PresetSettings(preset, settings.mapWidth, settings.mapHeight)) {
                settings = PresetSettings(preset, width, height);
                return;
            }
        }
        settings.mapWidth = width;
        settings.mapHeight = height;
        for (const SpeciesInfo& info : SpeciesCatalog()) {
            settings.species[info.kind].initialCount =
                AnimalsFor(width, height, info.cellsPerAnimal, info.minimumAtStart);
        }
    }
}
