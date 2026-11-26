/**
 * @file
 * @brief L'herbivore : il broute et peut fuir.
 */
export module ecosim.herbivore;

import std;
export import ecosim.animal;

export namespace ecosim {
    /// @brief Un animal qui mange l'herbe et sert de proie.
    /// @ingroup moteur
    class Herbivore final : public Animal {
    public:
        /// @brief Énergie de départ par défaut.
        static constexpr int StartingEnergy{50};
        /// @brief Distance, en cases, à laquelle un prédateur est repéré.
        static constexpr double ThreatSight{2.0};
        /// @brief Distance, en cases, à laquelle il cherche de l'herbe.
        static constexpr double FoodSight{2.0};
        /// @brief À partir de cette quantité d'herbe sous lui, il reste brouter.
        static constexpr int GrazingVegetation{30};

        /// @brief Crée un herbivore au centre d'une case.
        /// @throws std::invalid_argument si l'énergie n'est pas entre 1 et 100.
        Herbivore(std::uint64_t id, Position cell, int energy = StartingEnergy,
                  const AnimalTraits& traits = {});
        /// @copydoc Animal::GetKind
        AnimalKind GetKind() const override;
        /// @brief Toujours vrai.
        bool IsPrey() const override;
        /// @brief Tire au sort s'il sera sur ses gardes pendant cette génération.
        void BeginGeneration(std::mt19937& rng) override;
        /// @brief Fuit un prédateur s'il est sur ses gardes, sinon broute ou cherche de l'herbe.
        Steering Steer(const Habitat& habitat, std::mt19937& rng) const override;
        /// @brief Mange une fois par génération, au dernier petit pas, là où il se trouve.
        void Act(Habitat& habitat, int tick) override;

    protected:
        /// @brief Crée un petit herbivore.
        std::unique_ptr<Animal> MakeOffspring(std::uint64_t id) const override;

    private:
        /// @brief Vrai si l'herbivore fuira les prédateurs pendant cette génération. Tiré au sort : la fuite n'est pas automatique.
        bool alert{false};
    };
}
