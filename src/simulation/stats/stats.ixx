// Statistiques d'une simulation : effectifs, végétation, générations et événements.
export module ecosim.stats;

import std;
export import ecosim.animal;

export namespace ecosim {
    struct LifeEvents {
        std::size_t births{0};
        std::size_t starved{0};
        std::size_t diedOfAge{0};
        std::size_t eaten{0};

        std::size_t Deaths() const { return starved + diedOfAge + eaten; }
    };

    struct SpeciesSample {
        std::size_t population{0};
        double energy{0.0};   // énergie moyenne, 0 sans animal
        LifeEvents events{};  // survenus pendant cette génération
    };

    // État mesuré à la fin d'une génération.
    struct PopulationSample {
        int generation{0};
        long long vegetation{0};  // total sur les prairies
        PerSpecies<SpeciesSample> species{};
    };

    enum class DeathCause { Starvation, OldAge, Predation };

    class SimulationStats {
    public:
        // Remet à zéro les événements de la dernière génération.
        void BeginGeneration();
        void RecordBirth(AnimalKind kind);
        void RecordDeath(AnimalKind kind, DeathCause cause);
        // Ajoute la mesure et ses événements, ou remplace celle de la même génération.
        void RecordSample(const PopulationSample& sample);

        // Une mesure par génération, la génération 0 comprise.
        const std::vector<PopulationSample>& History() const;
        const LifeEvents& LastGeneration(AnimalKind kind) const;
        const LifeEvents& Total(AnimalKind kind) const;

    private:
        std::vector<PopulationSample> history;
        PerSpecies<LifeEvents> lastGeneration{};
        PerSpecies<LifeEvents> total{};
    };
}
