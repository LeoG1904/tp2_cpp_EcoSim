#include <catch2/catch_all.hpp>

#include "../../src/ui/layout.hpp"

import std;
import ecosim.controller;

using namespace ecosim;
using namespace ecosim_render_detail;

namespace {
    constexpr int Width{1920};
    constexpr int Height{1080};
    constexpr int CellSize{40};

    RawInput Click(const UiRect button) {
        RawInput input;
        input.click = PointerPosition{button.x + 1, button.y + 1};
        return input;
    }
}

TEST_CASE("Echap ouvre puis ferme la confirmation, et quitte depuis l'accueil", "[controller]") {
    UiController controller{Width, Height, CellSize};
    SimulationSettings settings;
    RawInput escape;
    escape.escape = true;

    REQUIRE(controller.Handle(escape, true, false, settings) == UiAction::Quit);
    REQUIRE(controller.Handle(escape, false, false, settings) == UiAction::None);
    REQUIRE(controller.IsExitDialogOpen());

    RawInput space;
    space.space = true;
    REQUIRE(controller.Handle(space, false, false, settings) == UiAction::None);

    REQUIRE(controller.Handle(escape, false, false, settings) == UiAction::CancelExit);
    REQUIRE_FALSE(controller.IsExitDialogOpen());
}

TEST_CASE("Les boutons de la confirmation", "[controller]") {
    SimulationSettings settings;
    RawInput escape;
    escape.escape = true;
    const std::array<std::pair<UiRect, UiAction>, 3> buttons{{
        {ExitMenuButton(Width, Height), UiAction::ReturnHome},
        {ExitQuitButton(Width, Height), UiAction::Quit},
        {ExitCancelButton(Width, Height), UiAction::CancelExit},
    }};
    for (const auto& [button, expected] : buttons) {
        UiController controller{Width, Height, CellSize};
        controller.Handle(escape, false, false, settings);
        REQUIRE(controller.Handle(Click(button), false, false, settings) == expected);
    }
}

TEST_CASE("Clavier pendant la simulation", "[controller]") {
    UiController controller{Width, Height, CellSize};
    SimulationSettings settings;
    RawInput tab;
    tab.tab = true;
    REQUIRE(controller.Handle(tab, false, false, settings) == UiAction::CycleSpeed);
    REQUIRE(controller.Handle(tab, true, false, settings) == UiAction::None);

    RawInput arrow;
    arrow.right = true;
    REQUIRE(controller.Handle(arrow, false, false, settings) == UiAction::None);
    REQUIRE(controller.Handle(arrow, false, true, settings) == UiAction::NextGeneration);
}

TEST_CASE("Les courbes s'ouvrent, se ferment et se reinitialisent au redemarrage", "[controller]") {
    UiController controller{Width, Height, CellSize};
    SimulationSettings settings;
    controller.Handle(Click(GraphButton(Width)), false, false, settings);
    REQUIRE(controller.IsShowingGraph());
    controller.Handle(Click(BackButton()), false, false, settings);
    REQUIRE_FALSE(controller.IsShowingGraph());

    controller.Handle(Click(GraphButton(Width)), false, false, settings);
    REQUIRE(controller.Handle(Click(RestartButton(Width)), false, false, settings)
            == UiAction::StartOrRestart);
    REQUIRE_FALSE(controller.IsShowingGraph());
}

TEST_CASE("Cliquer une case, taper puis demarrer applique la valeur", "[controller]") {
    UiController controller{Width, Height, CellSize};
    SimulationSettings settings;
    controller.Handle(Click(ValueBox(Width, Height, 0, 0)), true, false, settings);
    REQUIRE(controller.Editor().Field() == 0);

    RawInput typing;
    typing.typed = {{KeyStroke::Kind::Digit, 2}, {KeyStroke::Kind::Digit, 5}};
    controller.Handle(typing, true, false, settings);
    REQUIRE(controller.Handle(Click(StartButton(Width, Height)), true, false, settings)
            == UiAction::StartOrRestart);
    REQUIRE(settings.waterPercent == 25);
}

TEST_CASE("Pendant une saisie, Echap annule et Tab passe a la case suivante", "[controller]") {
    UiController controller{Width, Height, CellSize};
    SimulationSettings settings;
    controller.Handle(Click(ValueBox(Width, Height, 0, 0)), true, false, settings);
    RawInput typing;
    typing.typed = {{KeyStroke::Kind::Digit, 3}, {KeyStroke::Kind::Next}};
    controller.Handle(typing, true, false, settings);
    REQUIRE(settings.waterPercent == 3);
    REQUIRE(controller.Editor().Field() == 1);

    RawInput escape;
    escape.escape = true;
    REQUIRE(controller.Handle(escape, true, false, settings) == UiAction::None);
    REQUIRE_FALSE(controller.Editor().Field().has_value());
}

TEST_CASE("Un clic sur la carte choisit une case, un second la libere", "[controller]") {
    UiController controller{Width, Height, CellSize};
    SimulationSettings settings;
    const UiRect cell{10 * CellSize, 8 * CellSize, CellSize, CellSize};

    REQUIRE(controller.Handle(Click(cell), false, false, settings) == UiAction::None);
    REQUIRE(controller.SelectedCell() == Position{10, 8});
    controller.Handle(Click(cell), false, false, settings);
    REQUIRE_FALSE(controller.SelectedCell().has_value());
}

TEST_CASE("Le panneau change de cote, bloque les clics et se ferme", "[controller]") {
    UiController controller{Width, Height, CellSize};
    SimulationSettings settings;
    controller.Handle(Click({40 * CellSize, 20 * CellSize, 1, 1}), false, false, settings);
    REQUIRE(controller.IsCellPanelOnLeft());
    const UiRect panel = CellPanel(Width, Height, true);

    controller.Handle(Click({panel.x + 100, panel.y + 100, 1, 1}), false, false, settings);
    REQUIRE(controller.SelectedCell() == Position{40, 20});
    controller.Handle(Click(CellPanelClose(panel)), false, false, settings);
    REQUIRE_FALSE(controller.SelectedCell().has_value());

    controller.Handle(Click({5 * CellSize, 5 * CellSize, 1, 1}), false, false, settings);
    REQUIRE_FALSE(controller.IsCellPanelOnLeft());
}

TEST_CASE("Les boutons, les courbes et le bord de l'ecran ne choisissent pas de case", "[controller]") {
    UiController controller{Width, Height + 10, CellSize};
    SimulationSettings settings;
    controller.Handle(Click({100, Height + 2, 1, 1}), false, false, settings);
    REQUIRE_FALSE(controller.SelectedCell().has_value());

    controller.Handle(Click(GraphButton(Width)), false, false, settings);
    REQUIRE_FALSE(controller.SelectedCell().has_value());
    controller.Handle(Click({400, 400, 1, 1}), false, false, settings);
    REQUIRE_FALSE(controller.SelectedCell().has_value());
}

TEST_CASE("La selection suit les courbes mais disparait au redemarrage", "[controller]") {
    UiController controller{Width, Height, CellSize};
    SimulationSettings settings;
    controller.Handle(Click({400, 400, 1, 1}), false, false, settings);
    controller.Handle(Click(GraphButton(Width)), false, false, settings);
    controller.Handle(Click(BackButton()), false, false, settings);
    REQUIRE(controller.SelectedCell() == Position{10, 10});

    controller.Handle(Click(RestartButton(Width)), false, false, settings);
    REQUIRE_FALSE(controller.SelectedCell().has_value());
}
