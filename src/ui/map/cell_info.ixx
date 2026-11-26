/**
 * @file
 * @brief Le texte du panneau d'une case.
 */
export module ecosim.cell_info;

import std;
export import ecosim.simulation;

export namespace ecosim {
    /// @brief Nombre d'animaux détaillés ; les autres sont seulement comptés.
    /// @ingroup interface
    inline constexpr std::size_t MaxListedAnimals{7};

    /// @brief Décrit une case, une chaîne par ligne du panneau.
    /// @param simulation Simulation observée.
    /// @param position Case, qui doit être dans la carte.
    /// @ingroup interface
    std::vector<std::string> DescribeCell(const Simulation& simulation, Position position);
}
