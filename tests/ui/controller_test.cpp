#include <catch2/catch_all.hpp>

import std;
import ecosim.layout;
import ecosim.controller;

using namespace ecosim;

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
    REQUIRE(controller.IsShowingStats());
    controller.Handle(Click(BackButton()), false, false, settings);
    REQUIRE_FALSE(controller.IsShowingStats());

    controller.Handle(Click(GraphButton(Width)), false, false, settings);
    REQUIRE(controller.Handle(Click(RestartButton(Width)), false, false, settings)
            == UiAction::StartOrRestart);
    REQUIRE_FALSE(controller.IsShowingStats());
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
    // La carte est centree : il reste une bande de 5 pixels en haut et en bas.
    controller.Handle(Click({100, Height + 6, 1, 1}), false, false, settings);
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

TEST_CASE("Un prereglage remplace les reglages, y compris une saisie en cours", "[controller]") {
    UiController controller{Width, Height, CellSize};
    SimulationSettings settings;
    controller.Handle(Click(ValueBox(Width, Height, 0, 0)), true, false, settings);
    RawInput typing;
    typing.typed = {{KeyStroke::Kind::Digit, 3}};
    controller.Handle(typing, true, false, settings);

    for (std::size_t i = 0; i < Presets.size(); ++i) {
        const UiRect button = PresetButton(Width, Height, static_cast<int>(i));
        REQUIRE(controller.Handle(Click(button), true, false, settings) == UiAction::None);
        REQUIRE(settings == PresetSettings(Presets[i], settings.mapWidth, settings.mapHeight));
        REQUIRE_FALSE(controller.Editor().Field().has_value());
    }
}

TEST_CASE("Les prereglages respectent les limites du formulaire", "[controller]") {
    for (const auto& [width, height] : {std::pair{16, 10}, std::pair{42, 26}, std::pair{96, 54}}) {
        for (const Preset preset : Presets) {
            SimulationSettings settings = PresetSettings(preset, width, height);
            for (const SettingField& field : SettingFields()) {
                REQUIRE(field.value(settings) >= field.minimum);
                REQUIRE(field.value(settings) <= field.maximum);
            }
        }
    }
}

TEST_CASE("En 2D, la molette zoome, le clic droit deplace et Recentrer revient", "[controller]") {
    UiController controller{Width, Height, CellSize};
    SimulationSettings settings;
    RawInput wheel;
    wheel.wheel = 3;
    wheel.pointer = PointerPosition{960, 540};
    controller.Handle(wheel, false, false, settings);
    REQUIRE(controller.Flat().CellSize() > CellSize);

    const double before = controller.Flat().ScreenX(0);
    RawInput drag;
    drag.rotating = true;
    drag.pointer = PointerPosition{900, 500};
    controller.Handle(drag, false, false, settings);
    drag.pointer = PointerPosition{950, 500};
    controller.Handle(drag, false, false, settings);
    REQUIRE(controller.Flat().ScreenX(0) == Catch::Approx(before + 50));

    controller.Handle(Click(RecenterButton(Width)), false, false, settings);
    REQUIRE(controller.Flat().CellSize() == CellSize);
}

TEST_CASE("Demarrer adapte les vues a la taille choisie", "[controller]") {
    UiController controller{Width, Height, CellSize};
    SimulationSettings settings = DefaultSettings(27, 27);
    controller.Handle(Click(StartButton(Width, Height)), true, false, settings);
    // La carte carree est centree entre deux bandes de 420 pixels.
    REQUIRE(controller.Flat().ScreenX(0) == 420.0);
    controller.Handle(Click({100, 500, 1, 1}), false, false, settings);
    REQUIRE_FALSE(controller.SelectedCell().has_value());
    controller.Handle(Click({1480, 1060, 1, 1}), false, false, settings);
    REQUIRE(controller.SelectedCell() == Position{26, 26});
}

namespace {
    // Le centre d'une case en 2D, avec la vue de depart.
    RawInput ClickCell(const UiController& controller, const Position cell) {
        RawInput input;
        input.click = PointerPosition{static_cast<int>(controller.Flat().ScreenX(cell.x + 0.5)),
                                      static_cast<int>(controller.Flat().ScreenY(cell.y + 0.5))};
        return input;
    }
}

TEST_CASE("Un bouton d'ajout choisit l'espece, puis chaque clic sur une prairie en place une", "[controller]") {
    UiController controller{Width, Height, CellSize};
    SimulationSettings settings;
    World world{Width / CellSize, Height / CellSize};
    world.GetCell({3, 4}) = Cell{Terrain::Eau};
    world.GetCell({4, 4}) = Cell{Terrain::Obstacle};
    const auto click = [&](const RawInput& input) {
        return controller.Handle(input, false, false, settings, &world);
    };
    for (const SpeciesInfo& species : SpeciesCatalog()) {
        const int index = static_cast<int>(species.kind);
        REQUIRE(click(Click(AddAnimalButton(Width, index))) == UiAction::None);
        REQUIRE(controller.PlacingKind() == species.kind);
        REQUIRE(click(ClickCell(controller, {10, 8})) == UiAction::AddAnimal);
        REQUIRE(controller.Placement()->kind == species.kind);
        REQUIRE(controller.Placement()->cell == Position{10, 8});
        // On place sans ouvrir le panneau de la case, et la demande ne dure qu'une image.
        REQUIRE_FALSE(controller.SelectedCell().has_value());
        REQUIRE(click(RawInput{}) == UiAction::None);
        REQUIRE_FALSE(controller.Placement().has_value());
    }
    // Ni l'eau ni un obstacle n'accueillent un animal, et l'ajout reste actif.
    for (const Position closed : {Position{3, 4}, Position{4, 4}}) {
        REQUIRE(click(ClickCell(controller, closed)) == UiAction::None);
        REQUIRE_FALSE(controller.Placement().has_value());
    }
    REQUIRE(controller.PlacingKind().has_value());
}

TEST_CASE("Le meme bouton ou Echap termine l'ajout", "[controller]") {
    UiController controller{Width, Height, CellSize};
    SimulationSettings settings;
    const World world{Width / CellSize, Height / CellSize};
    const auto click = [&](const RawInput& input) {
        return controller.Handle(input, false, false, settings, &world);
    };
    // Choisir une espece ferme le panneau ouvert.
    click(ClickCell(controller, {5, 5}));
    REQUIRE(controller.SelectedCell().has_value());
    const UiRect wolf = AddAnimalButton(Width, static_cast<int>(AnimalKind::Wolf));
    click(Click(wolf));
    REQUIRE_FALSE(controller.SelectedCell().has_value());
    click(Click(AddAnimalButton(Width, static_cast<int>(AnimalKind::Fox))));
    REQUIRE(controller.PlacingKind() == AnimalKind::Fox);
    click(Click(wolf));
    click(Click(wolf));
    REQUIRE_FALSE(controller.PlacingKind().has_value());
    click(ClickCell(controller, {5, 5}));
    REQUIRE(controller.SelectedCell() == Position{5, 5});

    click(Click(wolf));
    RawInput escape;
    escape.escape = true;
    REQUIRE(click(escape) == UiAction::None);
    REQUIRE_FALSE(controller.PlacingKind().has_value());
    REQUIRE_FALSE(controller.IsExitDialogOpen());
    click(escape);
    REQUIRE(controller.IsExitDialogOpen());
}

TEST_CASE("L'ajout s'arrete au redemarrage, et les courbes n'ajoutent rien", "[controller]") {
    UiController controller{Width, Height, CellSize};
    SimulationSettings settings;
    const World world{Width / CellSize, Height / CellSize};
    const auto click = [&](const RawInput& input) {
        return controller.Handle(input, false, false, settings, &world);
    };
    click(Click(AddAnimalButton(Width, 0)));
    click(Click(GraphButton(Width)));
    REQUIRE(click(ClickCell(controller, {10, 8})) == UiAction::None);
    REQUIRE_FALSE(controller.Placement().has_value());
    REQUIRE(click(Click(RestartButton(Width))) == UiAction::StartOrRestart);
    REQUIRE_FALSE(controller.PlacingKind().has_value());
}

TEST_CASE("En 3D aussi, un clic place l'animal sur la case visee, meme en hauteur", "[controller]") {
    UiController controller{Width, Height, CellSize};
    SimulationSettings settings;
    World world{Width / CellSize, Height / CellSize};
    const Position hill{20, 12};
    world.GetCell(hill).SetElevation(3);
    const auto click = [&](const RawInput& input) {
        return controller.Handle(input, false, false, settings, &world);
    };
    click(Click(ViewButton(Width)));
    click(Click(AddAnimalButton(Width, 1)));
    const auto top = controller.Camera().Project({20.5, 3 * LevelHeight, 12.5}, {Width, Height});
    REQUIRE(top.has_value());
    RawInput input;
    input.click = PointerPosition{top->x, top->y};
    REQUIRE(click(input) == UiAction::AddAnimal);
    REQUIRE(controller.Placement()->cell == hill);
}

TEST_CASE("Les boutons d'ajout tiennent au-dessus du panneau d'une case", "[controller]") {
    for (const auto& [width, height] : {std::pair{1280, 720}, std::pair{1920, 1080}}) {
        const UiRect panel = CellPanel(width, height, false);
        const UiRect recenter = RecenterButton(width);
        for (int i = 0; i < static_cast<int>(SpeciesCount); ++i) {
            const UiRect button = AddAnimalButton(width, i);
            REQUIRE(button.y >= recenter.y + recenter.height);
            REQUIRE(button.y + button.height <= panel.y);
            REQUIRE(button.x + button.width <= width);
        }
    }
}
