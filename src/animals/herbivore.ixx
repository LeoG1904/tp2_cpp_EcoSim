/**
 * @file
 * @brief La famille des herbivores : ils broutent et peuvent fuir.
 */
export module ecosim.herbivore;

import std;
export import ecosim.animal;

export namespace ecosim {
    /// @brief Base des animaux qui mangent l'herbe et servent de proie.
    ///
    /// Elle réunit la fuite et le broutage ; Deer et Rabbit en sont les espèces.
    /// @ingroup moteur
    class Herbivore : public Animal {
    public:
        /// @brief Distance, en cases, à laquelle un prédateur est repéré.
        static constexpr double ThreatSight{2.0};
        /// @brief Distance, en cases, à laquelle il cherche de l'herbe.
        static constexpr double FoodSight{2.0};

        /// @brief Toujours vrai.
        bool IsPrey() const override;
        /// @brief Tire au sort s'il sera sur ses gardes pendant cette génération.
        void BeginGeneration(std::mt19937& rng) override;
        /// @brief Fuit un prédateur s'il est sur ses gardes, sinon broute ou cherche de l'herbe.
        Steering Steer(const Habitat& habitat, std::mt19937& rng) const override;
        /// @brief Mange une fois par génération, au dernier petit pas, là où il se trouve.
        void Act(Habitat& habitat, int tick) override;

    protected:
        /// @brief Crée un herbivore au centre d'une case.
        /// @param id Identifiant unique.
        /// @param cell Case de départ.
        /// @param energy Énergie de départ.
        /// @param traits Caractéristiques de l'espèce.
        /// @param options Réglages de l'individu.
        /// @param grazingVegetation À partir de cette quantité d'herbe sous lui, il reste brouter.
        /// @throws std::invalid_argument si l'énergie n'est pas entre 1 et 100.
        Herbivore(std::uint64_t id, Position cell, int energy, const SpeciesTraits& traits,
                  const AnimalOptions& options, int grazingVegetation);

    private:
        /// @brief Quantité d'herbe qui suffit à l'espèce pour rester sur place.
        int grazingVegetation;
        /// @brief Vrai si l'herbivore fuira les prédateurs pendant cette génération. Tiré au sort : la fuite n'est pas automatique.
        bool alert{false};
    };
}
