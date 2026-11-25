module ecosim.simulation;

import std;

namespace ecosim {
    void SimulationClock::SetSpeed(const int multiplier) {
        if (std::ranges::find(Speeds, multiplier) == Speeds.end()) {
            throw std::invalid_argument{"Vitesse non supportée"};
        }
        speed = multiplier;
    }

    int SimulationClock::GetSpeed() const { return speed; }

    int SimulationClock::Advance(const double realSeconds) {
        accumulator += realSeconds * speed * BaseGenerationsPerSecond;
        const int steps = static_cast<int>(accumulator);
        accumulator -= steps;
        return steps;
    }

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
