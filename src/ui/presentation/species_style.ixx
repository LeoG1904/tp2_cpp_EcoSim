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

    /// @brief L'apparence propre à un sexe.
    /// @ingroup interface
    struct SexLook {
        /// @brief Couleur du corps : le mâle est plus foncé, la femelle plus claire.
        Color body;
        /// @brief Couleur du détail qui distingue : bois, oreilles, crinière, museau ou bout de queue.
        Color accent;
    };

    /// @brief Taille d'un jeune à l'écran, en part de celle d'un adulte.
    ///
    /// Elle ne concerne que le dessin : les collisions utilisent le même corps à tout âge.
    /// @ingroup interface
    inline constexpr double YoungScale{0.65};

    /// @brief La présentation d'une espèce.
    /// @ingroup interface
    struct SpeciesStyle {
        /// @brief Couleur de l'espèce, pour les titres et les courbes.
        Color color;
        /// @brief Apparence de la femelle.
        SexLook female;
        /// @brief Apparence du mâle.
        SexLook male;
        /// @brief Apparence d'un jeune, la même pour les deux sexes ; le détail est celui des jeunes de l'espèce.
        SexLook young;
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
    /// @brief Vrai à cet âge pour un jeune, qui ne peut pas encore se reproduire (Animal::AdultAge).
    /// @ingroup interface
    bool IsYoung(int age);
    /// @brief L'apparence d'un animal : celle des jeunes de l'espèce, sinon celle de son sexe.
    /// @ingroup interface
    const SexLook& LookOf(AnimalKind kind, Sex sex, int age);
}
