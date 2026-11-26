/**
 * @file
 * @brief La famille des chasseurs : ils poursuivent et capturent les proies.
 */
export module ecosim.predator;

import std;
export import ecosim.animal;

export namespace ecosim {
    /// @brief Base des animaux qui chassent, avec au plus une capture par génération.
    ///
    /// Elle réunit la poursuite et la capture ; Wolf et Fox en sont les espèces. Chacune donne son régime (Animal::Eats).
    /// @ingroup moteur
    class Predator : public Animal {
    public:
        /// @brief Distance à laquelle une proie est capturée.
        static constexpr double Reach{0.45};

        /// @brief Redevient affamé.
        void BeginGeneration(std::mt19937& rng) override;
        /// @brief Flâne s'il a mangé ; sinon poursuit une proie en vue, ou en cherche une.
        Steering Steer(const Habitat& habitat, std::mt19937& rng) const override;
        /// @brief Capture une proie à portée s'il n'a pas encore mangé.
        void Act(Habitat& habitat, int tick) override;

    protected:
        /// @brief Crée un chasseur au centre d'une case.
        /// @param id Identifiant unique.
        /// @param cell Case de départ.
        /// @param energy Énergie de départ.
        /// @param traits Caractéristiques de l'espèce.
        /// @param options Réglages de l'individu.
        /// @param sight Distance de vue, en cases ; l'instinct s'y ajoute.
        /// @throws std::invalid_argument si l'énergie n'est pas entre 1 et 100.
        Predator(std::uint64_t id, Position cell, int energy, const SpeciesTraits& traits,
                 const AnimalOptions& options, double sight);
        /// @brief Direction à prendre vers une proie en vue : droit dessus, par défaut.
        /// @param habitat Entourage de l'animal.
        /// @param prey Position de la proie.
        /// @param range Distance de vue actuelle.
        virtual Steering Chase(const Habitat& habitat, Point prey, double range) const;
        /// @brief Direction à prendre sans proie en vue : errer, par défaut.
        /// @param habitat Entourage de l'animal.
        /// @param range Distance de vue actuelle.
        /// @param rng Générateur de hasard.
        virtual Steering Search(const Habitat& habitat, double range, std::mt19937& rng) const;
        /// @brief La part du repas que le chasseur garde : tout, par défaut.
        /// @param habitat Entourage de l'animal.
        /// @param meal Énergie du repas.
        virtual int KeptMeal(Habitat& habitat, int meal) const;

    private:
        /// @brief Distance de vue de l'espèce, en cases.
        double sight;
        /// @brief Vrai après une capture : une seule est permise par génération.
        bool hasEaten{false};
    };
}
