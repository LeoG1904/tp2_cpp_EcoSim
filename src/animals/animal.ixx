// Données et comportement communs aux animaux, indépendants du rendu.
export module ecosim.animal;

import std;
export import ecosim.world;

export namespace ecosim {
    enum class AnimalKind { Herbivore };

    class Animal {
    public:
        static constexpr int MaxEnergy{100};
        static constexpr int MaxAge{80};

        virtual ~Animal() = default;
        Animal(const Animal&) = delete;
        Animal& operator=(const Animal&) = delete;

        std::uint64_t GetId() const;
        Position GetPosition() const;
        int GetAge() const;
        int GetEnergy() const;
        bool IsAlive() const;

        void MoveTo(Position destination);
        void SpendGeneration();
        bool CanReproduce() const;
        void PayReproductionCost();

        virtual AnimalKind GetKind() const = 0;
        virtual void Feed(World& world) = 0;
        virtual std::unique_ptr<Animal> CreateOffspring(std::uint64_t id) const = 0;

    protected:
        Animal(std::uint64_t id, Position position, int energy);
        void GainEnergy(int amount);

    private:
        std::uint64_t id;
        Position position;
        int age{0};
        int energy;
    };
}
