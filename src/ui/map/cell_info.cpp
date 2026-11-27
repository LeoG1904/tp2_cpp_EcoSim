/**
 * @file
 * @brief Rédaction du panneau d'une case.
 */
module ecosim.cell_info;

import std;

namespace {
    /// @brief Le nom français d'un terrain.
    std::string TerrainName(const ecosim::Terrain terrain) {
        switch (terrain) {
            case ecosim::Terrain::Eau: return "Eau";
            case ecosim::Terrain::Obstacle: return "Obstacle";
            case ecosim::Terrain::Prairie: break;
        }
        return "Prairie";
    }

}

namespace ecosim {
    std::vector<std::string> DescribeCell(const SimulationSnapshot& simulation, const Position position) {
        const Cell& cell = simulation.world.GetCell(position);
        // Seuls les animaux de la case sont copiés.
        std::vector<AnimalSnapshot> present;
        std::ranges::copy_if(simulation.animals, std::back_inserter(present),
                             [position](const AnimalSnapshot& animal) {
                                 return animal.position == position;
                             });
        std::string counts;
        for (const SpeciesInfo& species : SpeciesCatalog()) {
            const auto number = std::ranges::count(present, species.kind, &AnimalSnapshot::kind);
            if (!counts.empty()) counts += "    ";
            counts += std::format("{} : {}", species.pluralName, number);
        }

        std::vector<std::string> lines{
            // Colonnes et lignes numérotées à partir de 1 pour l'affichage.
            std::format("Case : colonne {}, ligne {}", position.x + 1, position.y + 1),
            std::format("Terrain : {}", TerrainName(cell.GetTerrain())),
            cell.IsAccessible()
                ? std::format("Végétation : {} / {}", cell.GetVegetation(), Cell::MaxVegetation)
                : std::string{"Inaccessible aux animaux"},
            counts,
        };
        for (std::size_t i = 0; i < std::min(present.size(), MaxListedAnimals); ++i) {
            const AnimalSnapshot& animal = present[i];
            lines.push_back(std::format("{} {}{} n°{} : âge {}, énergie {}", SpeciesOf(animal.kind).name,
                                        animal.age < Animal::AdultAge ? "jeune " : "",
                                        animal.sex == Sex::Female ? "femelle" : "mâle", animal.id,
                                        animal.age, animal.energy));
        }
        if (present.size() > MaxListedAnimals) {
            lines.push_back(std::format("Autres animaux : {}", present.size() - MaxListedAnimals));
        }
        return lines;
    }
}
