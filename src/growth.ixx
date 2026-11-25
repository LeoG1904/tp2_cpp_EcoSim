// Repousse de la végétation : la première phase d'une génération.
export module ecosim.growth;

import std;
import ecosim.world;

export namespace ecosim {

    // Végétation gagnée par chaque prairie à chaque génération
    constexpr int VegetationGrowthPerGeneration{1};

    // Chaque prairie gagne de la végétation, sans dépasser Cell::MaxVegetation.
    // Eau et obstacles ne repoussent pas.
    void GrowVegetation(World& world) {
        for (int y = 0; y < world.GetHeight(); ++y) {
            for (int x = 0; x < world.GetWidth(); ++x) {
                Cell& cell = world.GetCell(Position{x, y});
                if (cell.GetTerrain() == Terrain::Prairie) {
                    cell.SetVegetation(cell.GetVegetation() + VegetationGrowthPerGeneration);
                }
            }
        }
    }

}
