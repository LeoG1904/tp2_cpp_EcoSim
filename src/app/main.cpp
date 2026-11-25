import std;
import ecosim;

int main() {
    // Taille d'une case en pixels ; la carte s'adapte à la taille de l'écran
    constexpr int CellSize{40};

    // La fenêtre prend la taille de l'écran : on l'ouvre avant de créer la carte
    ecosim::Renderer2D renderer{CellSize, "EcoSim"};
    const int cellsWide = renderer.GetWidth() / CellSize;
    const int cellsHigh = renderer.GetHeight() / CellSize;

    // Une graine aléatoire différente à chaque lancement : une carte différente à chaque fois
    std::mt19937 rng{std::random_device{}()};
    ecosim::Simulation simulation{ecosim::GenerateRandomWorld(cellsWide, cellsHigh, rng),
                                  std::mt19937{rng()}};
    simulation.SeedHerbivores(std::max(12, cellsWide * cellsHigh / 15));
    simulation.SeedPredators(std::max(3, cellsWide * cellsHigh / 150));

    // Temps de la simulation : le nombre de générations écoulées suit la vitesse choisie
    ecosim::SimulationClock clock;

    while (!renderer.ShouldClose()) {
        renderer.HandleInput();
        if (const auto speed = renderer.SpeedKeyPressed()) {
            clock.SetSpeed(*speed);
        }
        // La simulation continue pendant la consultation des courbes.
        const double elapsed = renderer.FrameTime();
        const int steps = simulation.IsFinished() ? 0 : clock.Advance(elapsed);
        for (int i = 0; i < steps; ++i) {
            simulation.Step();
            if (simulation.IsFinished()) break;
        }

        renderer.Draw(simulation, clock.GetSpeed());
    }
    return 0;
}
