// Données et comportement communs aux animaux, indépendants du rendu.
export module ecosim.animal;

import std;
export import ecosim.world;

export namespace ecosim {
    enum class AnimalKind { Herbivore, Predator };

    // Statistiques réglables d'une espèce. Les valeurs initiales sont celles des herbivores.
    struct AnimalTraits {
        int startingEnergy{50};
        int energyLoss{2};            // énergie perdue à chaque génération
        int maxAge{80};
        int foodEnergy{10};           // végétation mangée au maximum, ou énergie par proie
        int reproductionEnergy{80};   // énergie minimale pour se reproduire
        int reproductionPercent{20};  // chance de naissance quand les conditions sont réunies
    };

    class Animal {
    public:
        static constexpr int MaxEnergy{100};

        virtual ~Animal() = default;
        Animal(const Animal&) = delete;
        Animal& operator=(const Animal&) = delete;

        std::uint64_t GetId() const;
        Position GetPosition() const;
        int GetAge() const;
        int GetEnergy() const;
        const AnimalTraits& GetTraits() const;
        bool IsAlive() const;

        void MoveTo(Position destination);
        void SpendGeneration();
        void Die();
        bool CanReproduce() const;
        void PayReproductionCost();

        virtual AnimalKind GetKind() const = 0;
        virtual std::unique_ptr<Animal> CreateOffspring(std::uint64_t id) const = 0;

    protected:
        Animal(std::uint64_t id, Position position, int energy, const AnimalTraits& traits);
        void GainEnergy(int amount);

    private:
        std::uint64_t id;
        Position position;
        int age{0};
        int energy;
        AnimalTraits traits;
    };
}
