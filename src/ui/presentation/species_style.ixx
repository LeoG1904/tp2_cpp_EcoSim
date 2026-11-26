/**
 * @file
 * @brief L'apparence de chaque espèce.
 */
export module ecosim.species_style;

import std;
export import ecosim.species;

export namespace ecosim {
    /// @brief Une couleur : rouge, vert et bleu, de 0 à 255.
    /// @ingroup interface
    struct Color {
        /// @brief Part de rouge.
        int red;
        /// @brief Part de vert.
        int green;
        /// @brief Part de bleu.
        int blue;
    };

    /// @brief La présentation d'une espèce.
    /// @ingroup interface
    struct SpeciesStyle {
        /// @brief Couleur à l'écran.
        Color color;
        /// @brief Population de départ maximale.
        int initialMaximum;
        /// @brief Nom du réglage de nourriture.
        std::string foodLabel;
        /// @brief Limite basse de ce réglage.
        int foodMinimum;
        /// @brief Limite haute de ce réglage.
        int foodMaximum;
        /// @brief Nom du réglage d'instinct.
        std::string instinctLabel;
        /// @brief Limite haute de l'instinct.
        int instinctMaximum;
        /// @brief Longueur du corps, en fraction de case.
        double length;
        /// @brief Largeur du corps.
        double thickness;
        /// @brief Hauteur du modèle 3D.
        double height;
    };

    /// @brief L'apparence d'une espèce.
    /// @ingroup interface
    const SpeciesStyle& StyleOf(AnimalKind kind);
}
