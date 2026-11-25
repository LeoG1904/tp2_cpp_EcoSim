#include <catch2/catch_all.hpp>

import std;
import ecosim.application;

using namespace ecosim;

namespace {
    // Ni fenêtre ni clavier : l'application ne voit que les interfaces.
    class FakeInput final : public InputSource {
    public:
        FrameInput next;
        FrameInput Read(bool, bool, SimulationSettings&) override { return std::exchange(next, {}); }
    };

    class FakeView final : public View {
    public:
        int homeDraws{0};
        int simulationDraws{0};
        PlaybackState last{};
        void DrawHome(const SimulationSettings&) override { ++homeDraws; }
        void DrawSimulation(const Simulation&, const PlaybackState& playback) override {
            ++simulationDraws;
            last = playback;
        }
    };

    struct Scenario {
        FakeInput input;
        FakeView view;
        Application application{input, view, 20, 12, 5u};

        bool Frame(const UiAction action = UiAction::None, const double seconds = 0.0) {
            input.next.action = action;
            return application.RunFrame(seconds);
        }
        int Generation() const { return application.CurrentSimulation()->GetGeneration(); }
    };
}

TEST_CASE("L'application demarre sur l'accueil, lance puis quitte une simulation", "[application]") {
    Scenario scenario;
    REQUIRE(scenario.Frame(UiAction::None, 10.0));
    REQUIRE_FALSE(scenario.application.CurrentSimulation().has_value());
    REQUIRE(scenario.view.homeDraws == 1);

    scenario.Frame(UiAction::StartOrRestart);
    REQUIRE(scenario.Generation() == 0);
    REQUIRE(scenario.view.simulationDraws == 1);

    scenario.Frame(UiAction::None, 1.0);
    REQUIRE(scenario.Generation() == 1);

    scenario.Frame(UiAction::ReturnHome);
    REQUIRE_FALSE(scenario.application.CurrentSimulation().has_value());
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
    REQUIRE(scenario.application.Clock().GetSpeed() == 10);
    REQUIRE(scenario.Generation() == 1);
}

TEST_CASE("Recommencer remet le temps a zero et garde les reglages", "[application]") {
    Scenario scenario;
    scenario.Frame(UiAction::StartOrRestart);
    scenario.Frame(UiAction::CycleSpeed);
    scenario.Frame(UiAction::None, 1.0);
    REQUIRE(scenario.Generation() == 2);

    const int herbivores = scenario.application.Settings().species[AnimalKind::Herbivore].initialCount;
    scenario.Frame(UiAction::StartOrRestart);
    REQUIRE(scenario.Generation() == 0);
    REQUIRE(scenario.application.Clock().GetSpeed() == 1);
    REQUIRE(scenario.application.CurrentSimulation()->Count(AnimalKind::Herbivore)
            == static_cast<std::size_t>(herbivores));
}
