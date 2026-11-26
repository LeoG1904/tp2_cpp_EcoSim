// Lecture des trajets pour l'affichage, sans API graphique.
export module ecosim.animation;

import std;
export import ecosim.simulation;

export namespace ecosim {
    // Position sur un trajet, progress allant de 0 (départ) à 1 (arrivée).
    Point PathPoint(const Path& path, double progress);
    // Orientation à ce moment du trajet ; un animal immobile garde sa dernière direction.
    double PathHeading(const Path& path, double progress);

    // À l'écran, un nouveau-né accompagne le dernier déplacement de son parent.
    void AttachNewbornsToParents(std::vector<AnimalSnapshot>& animals);
}
