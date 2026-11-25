import std;
import ecosim;

namespace {
    ecosim::Simulation CreateSimulation(const int width, const int height,
                                        const ecosim::SimulationSettings& settings) {
        std::mt19937 rng{std::random_device{}()};
        ecosim::World world = ecosim::GenerateRandomWorld(width, height, rng,
            settings.waterPercent, settings.obstaclePercent);
        ecosim::Simulation simulation{std::move(world), std::mt19937{rng()}, settings};
        simulation.SeedHerbivores(settings.initialHerbivores);
        simulation.SeedPredators(settings.initialPredators);
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

    // Choisis sur l'accueil, conservés au retour au menu et par Recommencer.
    ecosim::SimulationSettings settings = ecosim::DefaultSettings(cellsWide, cellsHigh);

    // La simulation n'existe qu'après le clic sur Démarrer.
    std::optional<ecosim::Simulation> simulation;

    // Temps de la simulation : le nombre de générations écoulées suit la vitesse choisie
    ecosim::SimulationClock clock;

    while (!renderer.ShouldClose()) {
        const ecosim::UiAction action = renderer.HandleInput(
            !simulation.has_value(), clock.IsPaused(), settings);
        if (action == ecosim::UiAction::Quit) break;
        if (action == ecosim::UiAction::ResetSettings) {
            settings = ecosim::DefaultSettings(cellsWide, cellsHigh);
        } else if (action == ecosim::UiAction::ReturnHome) {
            simulation.reset();
            clock = ecosim::SimulationClock{};
        } else if (action == ecosim::UiAction::StartOrRestart) {
            simulation = CreateSimulation(cellsWide, cellsHigh, settings);
            clock = ecosim::SimulationClock{};
        } else if (simulation && !simulation->IsFinished()) {
            if (action == ecosim::UiAction::TogglePause) clock.SetPaused(!clock.IsPaused());
            if (action == ecosim::UiAction::CycleSpeed) clock.CycleSpeed();
            if (action == ecosim::UiAction::NextGeneration && clock.IsPaused()) simulation->Step();
        }
        const double elapsed = renderer.FrameTime();
        // Lue à chaque image pour oublier les appuis faits sur l'accueil ou la confirmation.
        const auto speed = renderer.SpeedKeyPressed();
        if (!simulation) {
            renderer.DrawHome(settings);
            continue;
        }
        if (speed && !renderer.IsExitDialogOpen()) clock.SetSpeed(*speed);
        // La simulation continue pendant la consultation des courbes.
        const int steps = action != ecosim::UiAction::None || simulation->IsFinished()
                       || renderer.IsExitDialogOpen()
            ? 0 : clock.Advance(elapsed);
        for (int i = 0; i < steps; ++i) {
            simulation->Step();
            if (simulation->IsFinished()) break;
        }

        renderer.Draw(*simulation, clock.GetSpeed(), clock.IsPaused());
    }
    return 0;
}
