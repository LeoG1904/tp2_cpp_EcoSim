module ecosim.simulation;

import std;

namespace ecosim {
    void GrowVegetation(World& world) {
        constexpr int growthPerGeneration{1};
        for (int y = 0; y < world.GetHeight(); ++y) {
            for (int x = 0; x < world.GetWidth(); ++x) {
                Cell& cell = world.GetCell(Position{x, y});
                if (cell.GetTerrain() == Terrain::Prairie) {
                    cell.SetVegetation(cell.GetVegetation() + growthPerGeneration);
                }
            }
        }
    }
}
