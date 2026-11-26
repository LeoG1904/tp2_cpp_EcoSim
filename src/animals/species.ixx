/**
 * @file
 * @brief Le catalogue des espèces.
 */
export module ecosim.species;

import std;
export import ecosim.herbivore;
export import ecosim.predator;

export namespace ecosim {
    /// @brief La fiche d'une espèce.
    /// @ingroup moteur
    struct SpeciesInfo {
        /// @brief Espèce décrite.
        AnimalKind kind;
        /// @brief Nom au singulier.
        std::string name;
        /// @brief Nom au pluriel.
        std::string pluralName;
        /// @brief Statistiques d'origine.
        AnimalTraits defaultTraits;
        /// @brief Au départ, un animal pour ce nombre de cases.
        int cellsPerAnimal;
        /// @brief Population de départ minimale.
        int minimumAtStart;
        /// @brief Vrai pour une proie.
        bool canBeEaten;
        /// @brief Instinct activé dans l'application.
        int defaultInstinct;
        /// @brief Fonction qui crée un animal de cette espèce.
        std::unique_ptr<Animal> (*create)(std::uint64_t id, Position position, int energy,
                                          const AnimalTraits& traits);
    };

    /// @brief Toutes les espèces, dans l'ordre d'AnimalKind.
    /// @ingroup moteur
    const std::vector<SpeciesInfo>& SpeciesCatalog();
    /// @brief La fiche d'une espèce.
    /// @ingroup moteur
    const SpeciesInfo& SpeciesOf(AnimalKind kind);
}
