// Prédateur : il poursuit les proies et en capture au plus une par génération.
export module ecosim.predator;

import std;
export import ecosim.animal;

export namespace ecosim {
    class Predator final : public Animal {
    public:
        static constexpr int StartingEnergy{50};
        static constexpr AnimalTraits DefaultTraits{.foodEnergy = 35, .speed = 120};
        // Distance de vue, en cases, avant le bonus de meute.
        static constexpr double Sight{3.0};
        // Une proie à cette distance est capturée.
        static constexpr double Reach{0.45};
        // Les membres d'une meute à cette distance partagent le repas.
        static constexpr double SharingDistance{1.5};

        Predator(std::uint64_t id, Position cell, int energy = StartingEnergy,
                 const AnimalTraits& traits = DefaultTraits);
        AnimalKind GetKind() const override;
        void BeginGeneration(std::mt19937& rng) override;
        Steering Steer(const Habitat& habitat, std::mt19937& rng) const override;
        void Act(Habitat& habitat, int tick) override;

    protected:
        std::unique_ptr<Animal> MakeOffspring(std::uint64_t id) const override;

    private:
        bool hasEaten{false};
    };
}
