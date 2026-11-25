import std;
import ecosim.clock;
import ecosim.generation;
import ecosim.growth;
import ecosim.render;

int main() {
    // Taille d'une case en pixels ; la carte s'adapte à la taille de l'écran
    constexpr int CellSize{40};

    // La fenêtre prend la taille de l'écran : on l'ouvre avant de créer la carte
    ecosim::Renderer2D renderer{CellSize, "EcoSim"};
    const int cellsWide = renderer.GetWidth() / CellSize;
    const int cellsHigh = renderer.GetHeight() / CellSize;

    // Une graine aléatoire différente à chaque lancement : une carte différente à chaque fois
    std::mt19937 rng{std::random_device{}()};
    auto world = ecosim::GenerateRandomWorld(cellsWide, cellsHigh, rng);

    // Temps de la simulation : le nombre de générations écoulées suit la vitesse choisie
    ecosim::SimulationClock clock;
    int generation = 0;

    while (!renderer.ShouldClose()) {
        if (const auto speed = renderer.SpeedKeyPressed()) {
            clock.SetSpeed(*speed);
        }
        // Chaque génération jouée fait repousser la végétation
        const int steps = clock.Advance(renderer.FrameTime());
        for (int i = 0; i < steps; ++i) {
            ecosim::GrowVegetation(world);
            ++generation;
        }

        renderer.Draw(world, generation, clock.GetSpeed());
    }
    return 0;
}
