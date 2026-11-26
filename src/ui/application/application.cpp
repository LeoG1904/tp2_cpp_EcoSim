/**
 * @file
 * @brief Enchaînement des actions et du temps à chaque image.
 */
module ecosim.application;

import std;

namespace ecosim {
    Application::Application(InputSource& input, View& view, const int mapWidth, const int mapHeight,
                             const std::uint32_t seed)
        : input{input}, view{view}, mapWidth{mapWidth}, mapHeight{mapHeight}, seeds{seed},
          settings{DefaultSettings(mapWidth, mapHeight)} {}

    const std::optional<Simulation>& Application::CurrentSimulation() const { return simulation; }
    const SimulationClock& Application::Clock() const { return clock; }
    const SimulationSettings& Application::Settings() const { return settings; }

    bool Application::RunFrame(const double elapsedSeconds) {
        const FrameInput frame = input.Read(!simulation.has_value(), clock.IsPaused(), settings);
        if (frame.action == UiAction::Quit) return false;
        Apply(frame.action);
        if (!simulation) {
            view.DrawHome(settings);
            return true;
        }
        Advance(frame, elapsedSeconds);
        view.DrawSimulation(*simulation, {clock.GetSpeed(), clock.IsPaused(), clock.Progress()});
        return true;
    }

    void Application::Apply(const UiAction action) {
        if (action == UiAction::ResetSettings) {
            settings = DefaultSettings(mapWidth, mapHeight);
        } else if (action == UiAction::ReturnHome) {
            simulation.reset();
            ResetPlayback();
        } else if (action == UiAction::StartOrRestart) {
            simulation = CreateSimulation();
            ResetPlayback();
        } else if (simulation && !simulation->IsFinished()) {
            if (action == UiAction::TogglePause) {
                clock.SetPaused(!clock.IsPaused());
                animatingStep = false;
            }
            if (action == UiAction::CycleSpeed) clock.CycleSpeed();
            if (action == UiAction::NextGeneration && clock.IsPaused()) {
                simulation->Step();
                clock.SetProgress(0.0);
                animatingStep = true;
            }
        }
    }

    void Application::ResetPlayback() {
        clock = SimulationClock{};
        animatingStep = false;
    }

    // La simulation continue pendant la consultation des courbes.
    void Application::Advance(const FrameInput& frame, const double elapsedSeconds) {
        if (frame.speed && !frame.exitDialogOpen) clock.SetSpeed(*frame.speed);
        if (frame.action != UiAction::None || frame.exitDialogOpen || simulation->IsFinished()) return;
        if (animatingStep && clock.IsPaused()) {
            // L'horloge garde la position atteinte : la reprise ne fait pas reculer les animaux.
            clock.SetProgress(clock.Progress() + elapsedSeconds * ManualStepsPerSecond);
            animatingStep = clock.Progress() < 1.0;
        }
        const int steps = clock.Advance(elapsedSeconds);
        for (int i = 0; i < steps && !simulation->IsFinished(); ++i) simulation->Step();
    }

    Simulation Application::CreateSimulation() {
        return CreateRandomSimulation(mapWidth, mapHeight, settings,
                                      static_cast<std::uint32_t>(seeds()));
    }
}
