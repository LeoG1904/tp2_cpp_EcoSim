// Prédateur : une proie vivante sur sa case peut être capturée par génération.
export module ecosim.predator;

import std;
export import ecosim.animal;

export namespace ecosim {
    class Predator final : public Animal {
    public:
        static constexpr int StartingEnergy{50};
        static constexpr AnimalTraits DefaultTraits{.foodEnergy = 35};

        Predator(std::uint64_t id, Position position, int energy = StartingEnergy,
                 const AnimalTraits& traits = DefaultTraits);
        AnimalKind GetKind() const override;
        TurnPhase Phase() const override;
        // Va vers la case qui compte le plus de proies.
        Position ChooseDestination(const Habitat& habitat, std::mt19937& rng) const override;
        void Feed(Habitat& habitat) override;

    protected:
        std::unique_ptr<Animal> MakeOffspring(std::uint64_t id) const override;
    };
}
