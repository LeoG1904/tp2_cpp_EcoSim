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
        /// @brief Chance de naissance et instinct.
        AnimalOptions options{};

        /// @brief Vrai si les deux valeurs sont identiques.
        bool operator==(const SpeciesSettings&) const = default;
    };

    /// @brief Les valeurs d'origine de chaque espèce, d'après le catalogue.
    /// @ingroup moteur
    PerSpecies<SpeciesSettings> DefaultSpeciesSettings();

    /// @brief Plus petite largeur et plus petite hauteur d'une carte, en cases.
    /// @ingroup moteur
    inline constexpr int MinMapSize{10};
    /// @brief Plus grande valeur que la saisie accepte pour une taille ou un nombre d'animaux.
    ///
    /// Ce n'est pas une taille garantie : la mémoire et le temps de calcul s'épuisent bien avant.
    /// @ingroup moteur
    inline constexpr int InputLimit{999999};

    /// @brief Tous les réglages d'une simulation.
    /// @ingroup moteur
    struct SimulationSettings {
        /// @brief Largeur de la carte, en cases.
        int mapWidth{42};
        /// @brief Hauteur de la carte, en cases.
        int mapHeight{26};
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
        Balance, ///< Les quatre espèces coexistent le plus souvent longtemps.
        Famine, ///< L'herbe ne repousse plus.
        Invasion ///< Les loups arrivent en masse.
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
    /// @brief Change la taille de la carte et adapte les populations de départ à sa surface.
    ///
    /// Un préréglage encore intact reste actif, recalculé pour la nouvelle taille. Sinon les nombres d'animaux sont recalculés et les autres réglages conservés. Sans changement de taille, rien n'est modifié.
    /// @ingroup moteur
    void ResizeMap(SimulationSettings& settings, int width, int height);
}
