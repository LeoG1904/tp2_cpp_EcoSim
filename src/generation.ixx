// Création de cartes aléatoires. Le générateur est fourni par l'appelant
// pour que le résultat soit reproductible à partir d'une graine.
export module ecosim.generation;

import std;
import ecosim.world;

export namespace ecosim {

    // Crée une carte où environ 10 % des cellules sont de l'eau, 10 % des obstacles
    // et le reste des prairies, chacune avec une végétation aléatoire.
    World GenerateRandomWorld(const int width, const int height, std::mt19937& rng) {
        World world{width, height};

        std::uniform_int_distribution<int> percent{1, 100};
        std::uniform_int_distribution<int> vegetation{0, Cell::MaxVegetation};

        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                const int roll = percent(rng);
                Terrain terrain{Terrain::Prairie};
                if (roll <= 10) {
                    terrain = Terrain::Eau;
                } else if (roll <= 20) {
                    terrain = Terrain::Obstacle;
                }
                world.GetCell(Position{x, y}) = Cell{terrain, vegetation(rng)};
            }
        }
        return world;
    }

}
