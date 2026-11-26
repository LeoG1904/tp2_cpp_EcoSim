/**
 * @file
 * @brief Le prédateur : il poursuit et capture les proies.
 */
export module ecosim.predator;

import std;
export import ecosim.animal;

export namespace ecosim {
    /// @brief Un animal qui chasse les proies, au plus une capture par génération.
    /// @ingroup moteur
    class Predator final : public Animal {
    public:
        /// @brief Les caractéristiques fixes du prédateur.
        static constexpr SpeciesTraits Traits{.energyLoss = 3, .maxAge = 70, .foodEnergy = 25,
                                              .reproductionEnergy = 70, .speed = 80};
        /// @brief Chance de naissance, en %, proposée au départ.
        static constexpr int DefaultBirthPercent{10};
        /// @brief Distance de vue, en cases, avant le bonus de meute.
        static constexpr double Sight{3.0};
        /// @brief Distance à laquelle une proie est capturée.
        static constexpr double Reach{0.45};
        /// @brief Distance dans laquelle les congénères partagent le repas.
        static constexpr double SharingDistance{1.5};

        /// @brief Crée un prédateur au centre d'une case.
        /// @throws std::invalid_argument si l'énergie n'est pas entre 1 et 100.
        Predator(std::uint64_t id, Position cell, int energy = StartingEnergy,
                 const AnimalOptions& options = {DefaultBirthPercent});
        /// @copydoc Animal::GetKind
        AnimalKind GetKind() const override;
        /// @brief Redevient affamé.
        void BeginGeneration(std::mt19937& rng) override;
        /// @brief Fonce sur la proie la plus proche ; en meute, vise un côté ou rejoint le groupe ; sinon erre.
        Steering Steer(const Habitat& habitat, std::mt19937& rng) const override;
        /// @brief Capture une proie à portée s'il n'a pas encore mangé.
        void Act(Habitat& habitat, int tick) override;

    protected:
        /// @brief Crée un petit prédateur.
        std::unique_ptr<Animal> MakeOffspring(std::uint64_t id) const override;

    private:
        /// @brief Vrai après une capture : une seule est permise par génération.
        bool hasEaten{false};
    };
}
