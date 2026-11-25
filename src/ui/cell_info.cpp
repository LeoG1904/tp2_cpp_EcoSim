module ecosim.cell_info;

import std;

namespace {
    std::string TerrainName(const ecosim::Terrain terrain) {
        switch (terrain) {
            case ecosim::Terrain::Eau: return "Eau";
            case ecosim::Terrain::Obstacle: return "Obstacle";
            case ecosim::Terrain::Prairie: break;
        }
        return "Prairie";
    }

    std::string SpeciesName(const ecosim::AnimalKind kind) {
        return kind == ecosim::AnimalKind::Herbivore ? "Herbivore" : "Prédateur";
    }
}

namespace ecosim {
    std::vector<std::string> DescribeCell(const Simulation& simulation, const Position position) {
        const Cell& cell = simulation.GetWorld().GetCell(position);
        std::vector<AnimalSnapshot> present = simulation.Animals();
        std::erase_if(present, [position](const AnimalSnapshot& animal) {
            return animal.position != position;
        });
        const auto herbivores = std::ranges::count_if(present, [](const AnimalSnapshot& animal) {
            return animal.kind == AnimalKind::Herbivore;
        });

        std::vector<std::string> lines{
            // Colonnes et lignes numérotées à partir de 1 pour l'affichage.
            std::format("Case : colonne {}, ligne {}", position.x + 1, position.y + 1),
            std::format("Terrain : {}", TerrainName(cell.GetTerrain())),
            cell.IsAccessible()
                ? std::format("Végétation : {} / {}", cell.GetVegetation(), Cell::MaxVegetation)
                : std::string{"Inaccessible aux animaux"},
            std::format("Herbivores : {}    Prédateurs : {}", herbivores,
                        std::ssize(present) - herbivores),
        };
        for (std::size_t i = 0; i < std::min(present.size(), MaxListedAnimals); ++i) {
            const AnimalSnapshot& animal = present[i];
            lines.push_back(std::format("{} n°{} : âge {}, énergie {}", SpeciesName(animal.kind),
                                        animal.id, animal.age, animal.energy));
        }
        if (present.size() > MaxListedAnimals) {
            lines.push_back(std::format("Autres animaux : {}", present.size() - MaxListedAnimals));
        }
        return lines;
    }
}
