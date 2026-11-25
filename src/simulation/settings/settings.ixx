// Réglages d'une simulation, sans dépendance à l'interface.
export module ecosim.settings;

import std;
export import ecosim.species;

export namespace ecosim {
    struct SpeciesSettings {
        int initialCount{0};
        AnimalTraits traits{};

        bool operator==(const SpeciesSettings&) const = default;
    };

    // Valeurs d'origine de chaque espèce, d'après le catalogue.
    PerSpecies<SpeciesSettings> DefaultSpeciesSettings();

    struct SimulationSettings {
        int waterPercent{10};
        int obstaclePercent{10};
        int vegetationGrowth{1};
        PerSpecies<SpeciesSettings> species{DefaultSpeciesSettings()};

        bool operator==(const SimulationSettings&) const = default;
    };

    // Scénarios prêts à l'emploi, dans l'ordre des boutons de l'accueil.
    enum class Preset { Balance, Famine, Invasion };
    inline constexpr std::array<Preset, 3> Presets{Preset::Balance, Preset::Famine, Preset::Invasion};

    // Les populations de départ suivent la taille de la carte.
    SimulationSettings DefaultSettings(int width, int height);
    // Réglages d'un scénario pour une carte de cette taille.
    SimulationSettings PresetSettings(Preset preset, int width, int height);
}
