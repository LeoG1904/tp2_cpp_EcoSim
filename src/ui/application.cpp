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
                animation.Cancel();
            }
            if (action == UiAction::CycleSpeed) clock.CycleSpeed();
            if (action == UiAction::NextGeneration && clock.IsPaused()) {
                simulation->Step();
                animation.Start(clock);
            }
        }
    }

    void Application::ResetPlayback() {
        clock = SimulationClock{};
        animation.Cancel();
    }

    // La simulation continue pendant la consultation des courbes.
    void Application::Advance(const FrameInput& frame, const double elapsedSeconds) {
        if (frame.speed && !frame.exitDialogOpen) clock.SetSpeed(*frame.speed);
        if (frame.action != UiAction::None || frame.exitDialogOpen || simulation->IsFinished()) return;
        animation.Update(clock, elapsedSeconds);
        const int steps = clock.Advance(elapsedSeconds);
        for (int i = 0; i < steps && !simulation->IsFinished(); ++i) simulation->Step();
    }

    Simulation Application::CreateSimulation() {
        std::mt19937 rng{static_cast<std::uint32_t>(seeds())};
        World world = GenerateRandomWorld(mapWidth, mapHeight, rng,
                                          settings.waterPercent, settings.obstaclePercent);
        Simulation result{std::move(world), std::mt19937{static_cast<std::uint32_t>(rng())}, settings};
        result.SeedHerbivores(settings.initialHerbivores);
        result.SeedPredators(settings.initialPredators);
        return result;
    }
}
