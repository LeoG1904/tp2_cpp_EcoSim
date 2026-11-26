/**
 * @file
 * @brief Les réglages d'une simulation et les préréglages.
 */
export module ecosim.settings;

import std;
export import ecosim.species;

export namespace ecosim {
    /// @brief Les réglages d'une espèce.
    /// @ingroup moteur
    struct SpeciesSettings {
        /// @brief Nombre d'animaux au départ.
        int initialCount{0};
        /// @brief Statistiques de l'espèce.
        AnimalTraits traits{};

        /// @brief Vrai si les deux valeurs sont identiques.
        bool operator==(const SpeciesSettings&) const = default;
    };

    /// @brief Les valeurs d'origine de chaque espèce, d'après le catalogue.
    /// @ingroup moteur
    PerSpecies<SpeciesSettings> DefaultSpeciesSettings();

    /// @brief Tous les réglages d'une simulation.
    /// @ingroup moteur
    struct SimulationSettings {
        /// @brief Part d'eau, en %.
        int waterPercent{10};
        /// @brief Part d'obstacles, en %.
        int obstaclePercent{10};
        /// @brief Herbe gagnée par prairie à chaque génération.
        int vegetationGrowth{1};
        /// @brief Réglages de chaque espèce.
        PerSpecies<SpeciesSettings> species{DefaultSpeciesSettings()};

        /// @brief Vrai si les deux valeurs sont identiques.
        bool operator==(const SimulationSettings&) const = default;
    };

    /// @brief Scénarios prêts à l'emploi : équilibre, famine et invasion.
    /// @ingroup moteur
    enum class Preset {
        Balance, ///< Les deux espèces coexistent longtemps.
        Famine, ///< L'herbe ne repousse plus.
        Invasion ///< Les prédateurs arrivent en masse.
    };
    /// @brief Les préréglages, dans l'ordre des boutons.
    /// @ingroup moteur
    inline constexpr std::array<Preset, 3> Presets{Preset::Balance, Preset::Famine, Preset::Invasion};

    /// @brief Réglages de l'application pour une carte de cette taille.
    ///
    /// Les populations de départ suivent la taille ; fuite et meutes sont activées.
    /// @ingroup moteur
    SimulationSettings DefaultSettings(int width, int height);
    /// @brief Réglages d'un scénario pour une carte de cette taille.
    /// @ingroup moteur
    SimulationSettings PresetSettings(Preset preset, int width, int height);
}
