/**
 * @file
 * @brief Le renard : un petit chasseur solitaire.
 */
export module ecosim.fox;

import std;
export import ecosim.predator;

export namespace ecosim {
    /// @brief Un petit chasseur rapide qui ne mange que les lapins ; son instinct, le flair, étend sa vue.
    /// @ingroup moteur
    class Fox final : public Predator {
    public:
        /// @brief Les caractéristiques fixes du renard.
        static constexpr SpeciesTraits Traits{.energyLoss = 2, .maxAge = 140, .foodEnergy = 35,
                                              .reproductionEnergy = 70, .speed = 90,
                                              .bodyRadius = 0.16, .litterMin = 1, .litterMax = 3,
                                              .breedingRest = 10};
        /// @brief Chance de naissance, en %, proposée au départ.
        static constexpr int DefaultBirthPercent{8};
        /// @brief Distance de vue, en cases, avant le bonus de flair.
        static constexpr double Sight{1.5};

        /// @brief Crée un renard au centre d'une case.
        /// @throws std::invalid_argument si l'énergie n'est pas entre 1 et 100.
        Fox(std::uint64_t id, Position cell, int energy = StartingEnergy,
            const AnimalOptions& options = {DefaultBirthPercent});
        /// @copydoc Animal::GetKind
        AnimalKind GetKind() const override;
        /// @brief Vrai pour les lapins seulement.
        bool Eats(AnimalKind kind) const override;

    protected:
        /// @brief Crée un petit renard.
        std::unique_ptr<Animal> MakeOffspring(std::uint64_t id) const override;
    };
}
