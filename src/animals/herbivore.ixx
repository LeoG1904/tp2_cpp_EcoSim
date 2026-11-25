// Spécialisation de l'animal qui se nourrit de végétation.
export module ecosim.herbivore;

import std;
export import ecosim.animal;

export namespace ecosim {
    class Herbivore final : public Animal {
    public:
        static constexpr int StartingEnergy{50};
        // Distance à laquelle un chasseur peut être repéré.
        static constexpr int ThreatSight{2};

        Herbivore(std::uint64_t id, Position position, int energy = StartingEnergy,
                  const AnimalTraits& traits = {});
        AnimalKind GetKind() const override;
        TurnPhase Phase() const override;
        bool IsPrey() const override;
        // Va vers la végétation la plus abondante.
        Position ChooseDestination(const Habitat& habitat, std::mt19937& rng) const override;
        void Feed(Habitat& habitat) override;

    protected:
        std::unique_ptr<Animal> MakeOffspring(std::uint64_t id) const override;
    };
}
