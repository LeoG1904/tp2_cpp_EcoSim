import std;
import ecosim;

namespace {
    ecosim::Simulation CreateSimulation(const int width, const int height) {
        std::mt19937 rng{std::random_device{}()};
        ecosim::Simulation simulation{ecosim::GenerateRandomWorld(width, height, rng),
                                      std::mt19937{rng()}};
        simulation.SeedHerbivores(std::max(12, width * height / 15));
        simulation.SeedPredators(std::max(3, width * height / 150));
        return simulation;
    }
}

int main() {
    // Taille d'une case en pixels ; la carte s'adapte à la taille de l'écran
    constexpr int CellSize{40};

    // La fenêtre prend la taille de l'écran : on l'ouvre avant de créer la carte
    ecosim::Renderer2D renderer{CellSize, "EcoSim"};
    const int cellsWide = renderer.GetWidth() / CellSize;
    const int cellsHigh = renderer.GetHeight() / CellSize;

    // Une nouvelle carte et de nouvelles populations sont créées à chaque redémarrage.
    ecosim::Simulation simulation = CreateSimulation(cellsWide, cellsHigh);

    // Temps de la simulation : le nombre de générations écoulées suit la vitesse choisie
    ecosim::SimulationClock clock;

    while (!renderer.ShouldClose()) {
        const bool restart = renderer.HandleInput();
        if (restart) {
            simulation = CreateSimulation(cellsWide, cellsHigh);
            clock = ecosim::SimulationClock{};
        }
        if (const auto speed = renderer.SpeedKeyPressed()) {
            clock.SetSpeed(*speed);
        }
        // La simulation continue pendant la consultation des courbes.
        const double elapsed = renderer.FrameTime();
        const int steps = restart || simulation.IsFinished() ? 0 : clock.Advance(elapsed);
        for (int i = 0; i < steps; ++i) {
            simulation.Step();
            if (simulation.IsFinished()) break;
        }

        renderer.Draw(simulation, clock.GetSpeed());
    }
    return 0;
}
