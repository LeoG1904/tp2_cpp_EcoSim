/**
 * @file
 * @brief Le cerf : le grand herbivore.
 */
export module ecosim.deer;

import std;
export import ecosim.herbivore;

export namespace ecosim {
    /// @brief Un grand herbivore : il cherche une herbe haute et offre un repas complet.
    /// @ingroup moteur
    class Deer final : public Herbivore {
    public:
        /// @brief Les caractéristiques fixes du cerf.
        static inline SpeciesTraits Traits{.energyLoss = 2, .maxAge = 80, .foodEnergy = 10,
                                              .reproductionEnergy = 80, .speed = 110};
        /// @brief Chance de naissance, en %, proposée au départ.
        static constexpr int DefaultBirthPercent{20};
        /// @brief À partir de cette quantité d'herbe sous lui, il reste brouter.
        static inline int GrazingThreshold{30};

        /// @brief Crée un cerf au centre d'une case.
        /// @throws std::invalid_argument si l'énergie n'est pas entre 1 et 100.
        Deer(std::uint64_t id, Position cell, int energy = StartingEnergy,
             const AnimalOptions& options = {DefaultBirthPercent});
        /// @copydoc Animal::GetKind
        AnimalKind GetKind() const override;

    protected:
        /// @brief Crée un petit cerf.
        std::unique_ptr<Animal> MakeOffspring(std::uint64_t id) const override;
    };
}
