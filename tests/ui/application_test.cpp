#include <catch2/catch_all.hpp>

import std;
import ecosim.application;

using namespace ecosim;

namespace {
    // Ni fenêtre ni clavier : l'application ne voit que les interfaces.
    class FakeInput final : public InputSource {
    public:
        FrameInput next;
        FrameInput Read(bool, bool, SimulationSettings&, const World*) override {
            return std::exchange(next, {});
        }
    };

    class FakeView final : public View {
    public:
        int homeDraws{0};
        int simulationDraws{0};
        PlaybackState last{};
        void DrawHome(const SimulationSettings&, const std::string&) override { ++homeDraws; }
        void DrawSimulation(const SimulationSnapshot&, const PlaybackState& playback) override {
            ++simulationDraws;
            last = playback;
        }
    };

    struct Scenario {
        FakeInput input;
        FakeView view;
        Application application{input, view, 20, 12, 5u};

        // Une image, puis on attend le thread : les tests restent reproductibles.
        bool Frame(const UiAction action = UiAction::None, const double seconds = 0.0) {
            input.next.action = action;
            const bool running = application.RunFrame(seconds);
            application.WaitForSimulation();
            return running;
        }
        int Generation() const { return application.CurrentSimulation()->generation; }
    };
}

TEST_CASE("L'application demarre sur l'accueil, lance puis quitte une simulation", "[application]") {
    Scenario scenario;
    REQUIRE(scenario.Frame(UiAction::None, 10.0));
    REQUIRE_FALSE(scenario.application.CurrentSimulation());
    REQUIRE(scenario.view.homeDraws == 1);

    scenario.Frame(UiAction::StartOrRestart);
    REQUIRE(scenario.Generation() == 0);
    REQUIRE(scenario.view.simulationDraws == 1);

    scenario.Frame(UiAction::None, 1.0);
    REQUIRE(scenario.Generation() == 1);

    scenario.Frame(UiAction::ReturnHome);
    REQUIRE_FALSE(scenario.application.CurrentSimulation());
    REQUIRE_FALSE(scenario.Frame(UiAction::Quit));
}

TEST_CASE("Pause, pas manuel anime et reprise", "[application]") {
    Scenario scenario;
    scenario.Frame(UiAction::StartOrRestart);
    scenario.Frame(UiAction::TogglePause);
    scenario.Frame(UiAction::None, 10.0);
    REQUIRE(scenario.Generation() == 0);

    scenario.Frame(UiAction::NextGeneration);
    REQUIRE(scenario.Generation() == 1);
    // L'image qui demande le calcul montre encore la fin de l'ancien trajet.
    REQUIRE(scenario.view.last.progress == 1.0);
    scenario.Frame();
    REQUIRE(scenario.view.last.progress == 0.0);
    scenario.Frame(UiAction::None, 1.0);
    REQUIRE(scenario.view.last.progress == 1.0);
    REQUIRE(scenario.Generation() == 1);
}

TEST_CASE("La confirmation fige la simulation et ignore les vitesses", "[application]") {
    Scenario scenario;
    scenario.Frame(UiAction::StartOrRestart);
    scenario.input.next = {UiAction::None, 10, true};
    scenario.application.RunFrame(5.0);
    REQUIRE(scenario.Generation() == 0);
    REQUIRE(scenario.application.Clock().GetSpeed() == 1);

    scenario.input.next = {UiAction::None, 10, false};
    scenario.application.RunFrame(0.1);
    scenario.application.WaitForSimulation();
    REQUIRE(scenario.application.Clock().GetSpeed() == 10);
    REQUIRE(scenario.Generation() == 1);
}

TEST_CASE("Recommencer remet le temps a zero et garde les reglages", "[application]") {
    Scenario scenario;
    scenario.Frame(UiAction::StartOrRestart);
    scenario.Frame(UiAction::CycleSpeed);
    scenario.Frame(UiAction::None, 1.0);
    REQUIRE(scenario.Generation() == 2);

    const int herbivores = scenario.application.Settings().species[AnimalKind::Deer].initialCount;
    scenario.Frame(UiAction::StartOrRestart);
    REQUIRE(scenario.Generation() == 0);
    REQUIRE(scenario.application.Clock().GetSpeed() == 1);
    REQUIRE(scenario.application.CurrentSimulation()->Count(AnimalKind::Deer)
            == static_cast<std::size_t>(herbivores));
}

TEST_CASE("Apres un pas manuel, la reprise enchaine sans retour en arriere", "[application]") {
    Scenario scenario;
    scenario.Frame(UiAction::StartOrRestart);
    scenario.Frame(UiAction::TogglePause);
    scenario.Frame(UiAction::NextGeneration);
    scenario.Frame(UiAction::None, 0.125);
    REQUIRE(scenario.view.last.progress == 0.5);
    scenario.Frame(UiAction::None, 5.0);
    scenario.Frame(UiAction::TogglePause);
    scenario.Frame(UiAction::None, 0.125);
    REQUIRE(scenario.Generation() == 2);
    scenario.Frame();
    REQUIRE(scenario.view.last.progress == 0.125);
}

TEST_CASE("La simulation prend la taille choisie", "[application]") {
    Scenario scenario;
    REQUIRE(scenario.application.Settings().mapWidth == 20);
    scenario.Frame(UiAction::StartOrRestart);
    REQUIRE(scenario.application.CurrentSimulation()->world.GetWidth() == 20);
    REQUIRE(scenario.application.CurrentSimulation()->world.GetHeight() == 12);
}

TEST_CASE("Une carte trop grande pour la memoire laisse sur l'accueil avec un message", "[application]") {
    // Mille milliards de cases : aucun ordinateur de bureau ne peut les reserver.
    class HugeInput final : public InputSource {
    public:
        bool first{true};
        FrameInput Read(bool, bool, SimulationSettings& settings, const World*) override {
            if (!std::exchange(first, false)) return {UiAction::ResetSettings, std::nullopt, false};
            ResizeMap(settings, InputLimit, InputLimit);
            return {UiAction::StartOrRestart, std::nullopt, false};
        }
    };
    HugeInput input;
    FakeView view;
    Application application{input, view, 20, 12, 5u};

    REQUIRE(application.RunFrame(0.0));
    REQUIRE_FALSE(application.CurrentSimulation());
    REQUIRE_FALSE(application.Notice().empty());
    REQUIRE(view.homeDraws == 1);

    // « Valeurs par defaut » rend la taille de depart et efface le message.
    application.RunFrame(0.0);
    REQUIRE(application.Settings().mapWidth == 20);
    REQUIRE(application.Notice().empty());
}
