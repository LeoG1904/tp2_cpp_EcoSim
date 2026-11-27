/**
 * @file
 * @brief Le loup : un chasseur qui peut agir en meute.
 */
export module ecosim.wolf;

import std;
export import ecosim.predator;

export namespace ecosim {
    /// @brief Un chasseur qui mange les cerfs et les lapins ; avec son instinct, il chasse en meute.
    /// @ingroup moteur
    class Wolf final : public Predator {
    public:
        /// @brief Les caractéristiques fixes du loup.
        static constexpr SpeciesTraits Traits{.energyLoss = 2, .maxAge = 80, .foodEnergy = 25,
                                              .reproductionEnergy = 80, .speed = 80,
                                              .bodyRadius = 0.20, .litterMin = 2, .litterMax = 4,
                                              .breedingRest = 12, .waterLoss = 2};
        /// @brief Chance de naissance, en %, proposée au départ.
        static constexpr int DefaultBirthPercent{4};
        /// @brief Distance de vue, en cases, avant le bonus de meute.
        static constexpr double Sight{3.0};
        /// @brief Distance dans laquelle les loups partagent le repas.
        static constexpr double SharingDistance{1.5};

        /// @brief Crée un loup au centre d'une case.
        /// @throws std::invalid_argument si l'énergie n'est pas entre 1 et 100.
        Wolf(std::uint64_t id, Position cell, int energy = StartingEnergy,
             const AnimalOptions& options = {DefaultBirthPercent});
        /// @copydoc Animal::GetKind
        AnimalKind GetKind() const override;
        /// @brief Vrai pour les cerfs et les lapins.
        bool Eats(AnimalKind kind) const override;

    protected:
        /// @brief En meute, vise un côté de la proie ; seul, fonce droit dessus.
        Steering Chase(const Habitat& habitat, Point prey, double range) const override;
        /// @brief En meute, rejoint le groupe ; sinon erre.
        Steering Search(const Habitat& habitat, double range, std::mt19937& rng) const override;
        /// @brief En meute, partage le repas avec les loups proches.
        int KeptMeal(Habitat& habitat, int meal) const override;
        /// @brief Crée un petit loup.
        std::unique_ptr<Animal> MakeOffspring(std::uint64_t id) const override;

    private:
        /// @brief Les loups en vue, ou aucun quand l'esprit de meute est désactivé.
        std::vector<Point> Pack(const Habitat& habitat, double range) const;
    };
}
