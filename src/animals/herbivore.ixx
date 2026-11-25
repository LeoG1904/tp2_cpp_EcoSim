// Spécialisation de l'animal qui se nourrit de végétation.
export module ecosim.herbivore;

import std;
export import ecosim.animal;

export namespace ecosim {
    class Herbivore final : public Animal {
    public:
        static constexpr int StartingEnergy{50};

        Herbivore(std::uint64_t id, Position position, int energy = StartingEnergy);
        AnimalKind GetKind() const override;
        void Feed(World& world, std::span<const std::unique_ptr<Animal>> animals) override;
        std::unique_ptr<Animal> CreateOffspring(std::uint64_t id) const override;
    };
}
