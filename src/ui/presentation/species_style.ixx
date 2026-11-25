// Présentation de chaque espèce : couleur, silhouette et limites de saisie.
export module ecosim.species_style;

import std;
export import ecosim.species;

export namespace ecosim {
    struct Color {
        int red;
        int green;
        int blue;
    };

    struct SpeciesStyle {
        Color color;
        // Formulaire de l'accueil.
        int initialMaximum;
        std::string foodLabel;
        int foodMinimum;
        int foodMaximum;
        // Corps du modèle 3D, en fractions de case.
        double length;
        double thickness;
        double height;
    };

    const SpeciesStyle& StyleOf(AnimalKind kind);
}
