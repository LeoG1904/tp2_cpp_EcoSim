/**
 * @file
 * @brief Enchaînement des actions et du temps à chaque image ; le calcul est confié au thread.
 */
module ecosim.application;

import std;

namespace ecosim {
    Application::Application(InputSource& input, View& view, const int mapWidth, const int mapHeight,
                             const std::uint32_t seed)
        : input{input}, view{view}, mapWidth{mapWidth}, mapHeight{mapHeight}, seeds{seed},
          settings{DefaultSettings(mapWidth, mapHeight)} {}

    std::shared_ptr<const SimulationSnapshot> Application::CurrentSimulation() const {
        return runner ? runner->State().snapshot : nullptr;
    }
    void Application::WaitForSimulation() const {
        if (runner) runner->Wait();
    }
    const SimulationClock& Application::Clock() const { return clock; }
    const SimulationSettings& Application::Settings() const { return settings; }
    const std::string& Application::Notice() const { return notice; }

    void Application::Start() {
        const std::string tooLarge{"Carte trop grande pour la mémoire de cet ordinateur : réduisez sa taille."};
        // L'ancienne simulation est libérée d'abord : sa mémoire peut servir à la nouvelle.
        runner.reset();
        try {
            runner = std::make_unique<SimulationRunner>(CreateSimulation());
            notice.clear();
        } catch (const std::bad_alloc&) {
            notice = tooLarge;
        } catch (const std::length_error&) {
            notice = tooLarge;
        }
    }

    void Application::Abandon() {
        runner.reset();
        ResetPlayback();
        notice = "Le calcul a échoué, sans doute faute de mémoire : réduisez la taille de la carte.";
    }

    bool Application::RunFrame(const double elapsedSeconds) {
        // La photo est gardée le temps de la lecture : sa carte reste valable.
        const std::shared_ptr<const SimulationSnapshot> shown = CurrentSimulation();
        const FrameInput frame = input.Read(!runner, clock.IsPaused(), settings,
                                            shown ? &shown->world : nullptr);
        if (frame.action == UiAction::Quit) return false;
        Apply(frame.action);
        if (runner && runner->State().failed) Abandon();
        if (!runner) {
            view.DrawHome(settings, notice);
            return true;
        }
        Advance(frame, elapsedSeconds, runner->State());
        Dispatch();
        // La photo et l'activité sont lues ensemble : le dessin reste cohérent.
        const RunnerState state = runner->State();
        // En attendant la suite, les animaux restent au bout du trajet connu au lieu de le rejouer.
        const double progress = state.busy || due > 0 ? 1.0 : clock.Progress();
        view.DrawSimulation(*state.snapshot, {clock.GetSpeed(), clock.IsPaused(), progress});
        return true;
    }

    void Application::Apply(const UiAction action) {
        if (action == UiAction::ResetSettings) {
            settings = DefaultSettings(mapWidth, mapHeight);
            notice.clear();
        } else if (action == UiAction::ReturnHome) {
            runner.reset();
            ResetPlayback();
        } else if (action == UiAction::StartOrRestart) {
            Start();
            ResetPlayback();
        } else if (runner && !runner->State().snapshot->IsFinished()) {
            if (action == UiAction::TogglePause) {
                clock.SetPaused(!clock.IsPaused());
                animatingStep = false;
                // La pause oublie le retard ; la génération déjà commencée se termine.
                due = 0;
            }
            if (action == UiAction::CycleSpeed) clock.CycleSpeed();
            if (action == UiAction::NextGeneration && clock.IsPaused()) {
                due = 1;
                clock.SetProgress(0.0);
                animatingStep = true;
            }
        }
    }

    void Application::ResetPlayback() {
        clock = SimulationClock{};
        animatingStep = false;
        due = 0;
    }

    // La simulation continue pendant la consultation des courbes.
    void Application::Advance(const FrameInput& frame, const double elapsedSeconds,
                              const RunnerState& state) {
        if (frame.speed && !frame.exitDialogOpen) clock.SetSpeed(*frame.speed);
        if (frame.action != UiAction::None || frame.exitDialogOpen) return;
        if (state.snapshot->IsFinished()) {
            due = 0;
            return;
        }
        if (animatingStep && clock.IsPaused() && !state.busy && due == 0) {
            // L'horloge garde la position atteinte : la reprise ne fait pas reculer les animaux.
            clock.SetProgress(clock.Progress() + elapsedSeconds * ManualStepsPerSecond);
            animatingStep = clock.Progress() < 1.0;
        }
        due += clock.Advance(elapsedSeconds);
        // Pendant un calcul, au plus une génération attend : le retard ne s'accumule pas.
        if (state.busy) due = std::min(due, 1);
    }

    void Application::Dispatch() {
        if (due == 0 || runner->State().busy) return;
        runner->Request(std::exchange(due, 0));
    }

    Simulation Application::CreateSimulation() {
        return CreateRandomSimulation(settings.mapWidth, settings.mapHeight, settings,
                                      static_cast<std::uint32_t>(seeds()));
    }
}
