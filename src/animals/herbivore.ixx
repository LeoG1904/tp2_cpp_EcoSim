// Spécialisation de l'animal qui se nourrit de végétation.
export module ecosim.herbivore;

import std;
export import ecosim.animal;

export namespace ecosim {
    class Herbivore final : public Animal {
    public:
        static constexpr int StartingEnergy{50};
        // Distances, en cases, auxquelles il repère un chasseur ou de l'herbe.
        static constexpr double ThreatSight{2.0};
        static constexpr double FoodSight{2.0};
        // Au-dessus de cette végétation, il reste brouter au lieu de chercher ailleurs.
        static constexpr int GrazingVegetation{30};

        Herbivore(std::uint64_t id, Position cell, int energy = StartingEnergy,
                  const AnimalTraits& traits = {});
        AnimalKind GetKind() const override;
        bool IsPrey() const override;
        void BeginGeneration(std::mt19937& rng) override;
        // Fuit un chasseur s'il est sur ses gardes, sinon cherche l'herbe la plus riche.
        Steering Steer(const Habitat& habitat, std::mt19937& rng) const override;
        // Mange une fois par génération, là où il se trouve à la fin.
        void Act(Habitat& habitat, int tick) override;

    protected:
        std::unique_ptr<Animal> MakeOffspring(std::uint64_t id) const override;

    private:
        // La fuite n'est pas automatique : elle est tirée au sort à chaque génération.
        bool alert{false};
    };
}
