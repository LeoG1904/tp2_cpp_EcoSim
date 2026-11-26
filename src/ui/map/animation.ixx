/**
 * @file
 * @brief La lecture des trajets pour l'affichage.
 */
export module ecosim.animation;

import std;
export import ecosim.simulation;

export namespace ecosim {
    /// @brief La position sur un trajet.
    /// @param path Trajet d'une génération.
    /// @param progress De 0 (départ) à 1 (arrivée).
    /// @ingroup interface
    Point PathPoint(const Path& path, double progress);
    /// @brief L'orientation à ce moment du trajet ; un animal arrêté garde sa dernière direction.
    /// @ingroup interface
    double PathHeading(const Path& path, double progress);

    /// @brief Fait accompagner à chaque nouveau-né, à l'écran, le trajet de son parent.
    /// @ingroup interface
    void AttachNewbornsToParents(std::vector<AnimalSnapshot>& animals);
}
