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
    const std::string& Application::Notice() const { return notice; }

    void Application::Start() {
        const std::string tooLarge{"Carte trop grande pour la mémoire de cet ordinateur : réduisez sa taille."};
        // L'ancienne simulation est libérée d'abord : sa mémoire peut servir à la nouvelle.
        simulation.reset();
        try {
            simulation = CreateSimulation();
            notice.clear();
        } catch (const std::bad_alloc&) {
            notice = tooLarge;
        } catch (const std::length_error&) {
            notice = tooLarge;
        }
    }

    bool Application::RunFrame(const double elapsedSeconds) {
        const FrameInput frame = input.Read(!simulation.has_value(), clock.IsPaused(), settings);
        if (frame.action == UiAction::Quit) return false;
        Apply(frame.action);
        if (!simulation) {
            view.DrawHome(settings, notice);
            return true;
        }
        Advance(frame, elapsedSeconds);
        view.DrawSimulation(*simulation, {clock.GetSpeed(), clock.IsPaused(), clock.Progress()});
        return true;
    }

    void Application::Apply(const UiAction action) {
        if (action == UiAction::ResetSettings) {
            settings = DefaultSettings(mapWidth, mapHeight);
            notice.clear();
        } else if (action == UiAction::ReturnHome) {
            simulation.reset();
            ResetPlayback();
        } else if (action == UiAction::StartOrRestart) {
            Start();
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
        const auto start = std::chrono::steady_clock::now();
        for (int i = 0; i < steps && !simulation->IsFinished(); ++i) {
            simulation->Step();
            // Au moins une génération est calculée ; le retard restant n'est pas rattrapé.
            const std::chrono::duration<double> spent = std::chrono::steady_clock::now() - start;
            if (spent.count() > MaxSimulationSecondsPerFrame) break;
        }
    }

    Simulation Application::CreateSimulation() {
        return CreateRandomSimulation(settings.mapWidth, settings.mapHeight, settings,
                                      static_cast<std::uint32_t>(seeds()));
    }
}
