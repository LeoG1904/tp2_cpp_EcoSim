// Texte décrivant une case de la simulation, sans API graphique.
export module ecosim.cell_info;

import std;
export import ecosim.simulation;

export namespace ecosim {
    // Au-delà, les animaux restants sont seulement comptés.
    inline constexpr std::size_t MaxListedAnimals{7};

    // Une chaîne par ligne du panneau ; la position doit être dans le monde.
    std::vector<std::string> DescribeCell(const Simulation& simulation, Position position);
}
