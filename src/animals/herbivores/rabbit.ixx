/**
 * @file
 * @brief Le lapin : le petit herbivore.
 */
export module ecosim.rabbit;

import std;
export import ecosim.herbivore;

export namespace ecosim {
    /// @brief Un petit herbivore : il se contente d'une herbe rase, que les cerfs ne peuvent pas lui prendre, se reproduit vite, vit peu et offre un petit repas.
    /// @ingroup moteur
    class Rabbit final : public Herbivore {
    public:
        /// @brief Les caractéristiques fixes du lapin ; il offre un petit repas.
        static constexpr SpeciesTraits Traits{.energyLoss = 1, .maxAge = 60, .foodEnergy = 5,
                                              .reproductionEnergy = 50, .speed = 100,
                                              .mealPercent = 50, .bodyRadius = 0.12};
        /// @brief Chance de naissance, en %, proposée au départ.
        static constexpr int DefaultBirthPercent{70};
        /// @brief À partir de cette quantité d'herbe sous lui, il reste brouter.
        static constexpr int GrazingThreshold{5};

        /// @brief Crée un lapin au centre d'une case.
        /// @throws std::invalid_argument si l'énergie n'est pas entre 1 et 100.
        Rabbit(std::uint64_t id, Position cell, int energy = StartingEnergy,
               const AnimalOptions& options = {DefaultBirthPercent});
        /// @copydoc Animal::GetKind
        AnimalKind GetKind() const override;

    protected:
        /// @brief Crée un petit lapin.
        std::unique_ptr<Animal> MakeOffspring(std::uint64_t id) const override;
    };
}
