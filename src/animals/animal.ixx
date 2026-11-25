// Données et comportement communs aux animaux, indépendants du rendu.
export module ecosim.animal;

import std;
export import ecosim.world;

export namespace ecosim {
    enum class AnimalKind { Herbivore, Predator };
    inline constexpr std::size_t SpeciesCount{2};

    // Une valeur pour chaque espèce, désignée par son nom plutôt que par un indice.
    template <typename T>
    struct PerSpecies {
        std::array<T, SpeciesCount> values{};

        T& operator[](const AnimalKind kind) { return values.at(static_cast<std::size_t>(kind)); }
        const T& operator[](const AnimalKind kind) const {
            return values.at(static_cast<std::size_t>(kind));
        }
        bool operator==(const PerSpecies&) const = default;
    };

    // Les phases d'une génération sont jouées dans l'ordre de ce tableau.
    enum class TurnPhase { Grazing, Hunting };
    inline constexpr std::array<TurnPhase, 2> TurnPhases{TurnPhase::Grazing, TurnPhase::Hunting};

    // Statistiques réglables d'une espèce. Les valeurs initiales sont celles des herbivores.
    struct AnimalTraits {
        int startingEnergy{50};
        int energyLoss{2};            // énergie perdue à chaque génération
        int maxAge{80};
        int foodEnergy{10};           // végétation mangée au maximum, ou énergie par proie
        int reproductionEnergy{80};   // énergie minimale pour se reproduire
        int reproductionPercent{20};  // chance de naissance quand les conditions sont réunies
        // Instinct propre à l'espèce, 0 pour le désactiver. Proie : chance (%) de fuir un
        // chasseur repéré. Chasseur : rayon de meute, en cases.
        int instinct{0};

        bool operator==(const AnimalTraits&) const = default;
    };

    class Animal;

    // Ce qu'un animal peut percevoir et modifier pendant son tour, sans connaître la simulation.
    class Habitat {
    public:
        virtual ~Habitat() = default;
        virtual World& GetWorld() = 0;
        virtual const World& GetWorld() const = 0;
        // Nombre de proies sur la case au début de la phase en cours.
        virtual int PreyCount(Position position) const = 0;
        // Capture une proie encore vivante sur la case, s'il en reste une.
        virtual bool CatchPrey(Position position) = 0;
        // Nombre de chasseurs sur la case au début de la phase en cours.
        virtual int HunterCount(Position position) const = 0;
        // Partage un repas avec les congénères proches ; renvoie la part du chasseur.
        virtual int ShareMeal(const Animal& hunter, int energy) = 0;
    };

    class Animal {
    public:
        static constexpr int MaxEnergy{100};

        virtual ~Animal() = default;
        Animal(const Animal&) = delete;
        Animal& operator=(const Animal&) = delete;

        std::uint64_t GetId() const;
        std::optional<std::uint64_t> GetParentId() const;
        Position GetPosition() const;
        // Case occupée avant le dernier déplacement.
        Position GetPreviousPosition() const;
        int GetAge() const;
        int GetEnergy() const;
        const AnimalTraits& GetTraits() const;
        bool IsAlive() const;

        void MoveTo(Position destination);
        void SpendGeneration();
        void Die();
        bool CanReproduce() const;
        void PayReproductionCost();
        std::unique_ptr<Animal> CreateOffspring(std::uint64_t id) const;

        // Comportement propre à chaque espèce.
        virtual AnimalKind GetKind() const = 0;
        virtual TurnPhase Phase() const = 0;
        virtual bool IsPrey() const;
        virtual bool IsHunter() const;
        // Énergie reçue d'un autre animal.
        void ReceiveMeal(int energy);
        virtual Position ChooseDestination(const Habitat& habitat, std::mt19937& rng) const = 0;
        virtual void Feed(Habitat& habitat) = 0;

    protected:
        Animal(std::uint64_t id, Position position, int energy, const AnimalTraits& traits);
        void GainEnergy(int amount);
        // Case accessible, sur place ou voisine, au meilleur score ; égalités tirées au sort.
        Position BestAccessiblePosition(const World& world, const std::function<int(Position)>& score,
                                        std::mt19937& rng) const;
        // Distance (en pas sur la grille) jusqu'à la case la plus proche qui vérifie found,
        // dans la limite de radius ; radius + 1 si aucune. Les obstacles ne bloquent pas la vue.
        static int NearestDistance(const World& world, Position from, int radius,
                                   const std::function<bool(Position)>& found);
        virtual std::unique_ptr<Animal> MakeOffspring(std::uint64_t id) const = 0;

    private:
        std::uint64_t id;
        std::optional<std::uint64_t> parentId;
        Position position;
        Position previousPosition;
        int age{0};
        int energy;
        AnimalTraits traits;
    };
}
