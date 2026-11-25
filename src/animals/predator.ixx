// Prédateur : une proie vivante sur sa case peut être capturée par génération.
export module ecosim.predator;

import std;
export import ecosim.animal;

export namespace ecosim {
    class Predator final : public Animal {
    public:
        static constexpr int StartingEnergy{50};
        static constexpr int EnergyFromPrey{35};

        Predator(std::uint64_t id, Position position, int energy = StartingEnergy);
        AnimalKind GetKind() const override;
        void Feed(World& world, std::span<const std::unique_ptr<Animal>> animals) override;
        std::unique_ptr<Animal> CreateOffspring(std::uint64_t id) const override;
    };
}
