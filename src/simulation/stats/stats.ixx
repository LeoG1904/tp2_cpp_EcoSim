/**
 * @file
 * @brief Les statistiques : historique et événements.
 */
export module ecosim.stats;

import std;
export import ecosim.animal;

export namespace ecosim {
    /// @brief Naissances et morts d'une espèce.
    /// @ingroup moteur
    struct LifeEvents {
        /// @brief Naissances.
        std::size_t births{0};
        /// @brief Morts de faim.
        std::size_t starved{0};
        /// @brief Morts de vieillesse.
        std::size_t diedOfAge{0};
        /// @brief Animaux mangés.
        std::size_t eaten{0};

        /// @brief Total des morts.
        std::size_t Deaths() const { return starved + diedOfAge + eaten; }
    };

    /// @brief Mesure d'une espèce à la fin d'une génération.
    /// @ingroup moteur
    struct SpeciesSample {
        /// @brief Effectif.
        std::size_t population{0};
        /// @brief Énergie moyenne ; 0 sans animal.
        double energy{0.0};
        /// @brief Événements de cette génération.
        LifeEvents events{};
    };

    /// @brief L'état mesuré à la fin d'une génération.
    /// @ingroup moteur
    struct PopulationSample {
        /// @brief Numéro de la génération.
        int generation{0};
        /// @brief Herbe totale des prairies.
        long long vegetation{0};
        /// @brief Mesure de chaque espèce.
        PerSpecies<SpeciesSample> species{};
    };

    /// @brief Les causes de mort : faim, vieillesse ou capture.
    /// @ingroup moteur
    enum class DeathCause {
        Starvation, ///< Plus d'énergie.
        OldAge, ///< Âge maximum atteint.
        Predation ///< Capturé par un chasseur.
    };

    /// @brief Conserve l'historique d'une simulation.
    /// @ingroup moteur
    class SimulationStats {
    public:
        /// @brief Remet à zéro les événements de la génération en cours.
        void BeginGeneration();
        /// @brief Compte une naissance.
        void RecordBirth(AnimalKind kind);
        /// @brief Compte une mort et sa cause.
        void RecordDeath(AnimalKind kind, DeathCause cause);
        /// @brief Ajoute une mesure et ses événements, ou remplace celle de la même génération.
        void RecordSample(const PopulationSample& sample);

        /// @brief Une mesure par génération, la génération 0 comprise.
        const std::vector<PopulationSample>& History() const;
        /// @brief Événements de la dernière génération pour une espèce.
        const LifeEvents& LastGeneration(AnimalKind kind) const;
        /// @brief Événements depuis le début pour une espèce.
        const LifeEvents& Total(AnimalKind kind) const;

    private:
        /// @brief Une mesure par génération, dans l'ordre.
        std::vector<PopulationSample> history;
        /// @brief Événements de la génération en cours.
        PerSpecies<LifeEvents> lastGeneration{};
        /// @brief Événements cumulés depuis le début.
        PerSpecies<LifeEvents> total{};
    };
}
