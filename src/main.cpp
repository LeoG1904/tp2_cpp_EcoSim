import std;
import ecosim.generation;
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
    const auto world = ecosim::GenerateRandomWorld(cellsWide, cellsHigh, rng);

    while (!renderer.ShouldClose()) {
        renderer.Draw(world);
    }
    return 0;
}
