module ecosim.controller;

import std;
import ecosim.layout;

namespace ecosim {
    UiController::UiController(const int width, const int height, const int cellSize)
        : width{width}, home{width, height, cellSize}, dialog{width, height}, stats{width},
          map{width, height, cellSize} {}

    const HomeController& UiController::Home() const { return home; }
    const ExitDialog& UiController::Dialog() const { return dialog; }
    const StatsScreen& UiController::Stats() const { return stats; }
    const MapNavigator& UiController::Map() const { return map; }
    std::optional<PointerPosition> UiController::Pointer() const { return pointer; }

    UiAction UiController::Handle(const RawInput& input, const bool isHomeScreen, const bool paused,
                                  SimulationSettings& settings) {
        pointer = input.pointer;
        if (isHomeScreen) home.Type(input.typed, settings);
        map.Control(input, !isHomeScreen && !stats.IsOpen() && !dialog.IsOpen());
        if (input.escape) return HandleEscape(isHomeScreen);
        if (!isHomeScreen && !dialog.IsOpen()) {
            if (const UiAction action = HandleKeys(input, paused); action != UiAction::None) {
                return action;
            }
        }
        if (!input.click) return UiAction::None;
        return HandleClick(*input.click, isHomeScreen, paused, settings);
    }

    UiAction UiController::HandleEscape(const bool isHomeScreen) {
        if (!isHomeScreen) return dialog.Toggle();
        // Pendant une saisie, Échap annule au lieu de quitter.
        return home.CancelEditing() ? UiAction::None : UiAction::Quit;
    }

    UiAction UiController::HandleKeys(const RawInput& input, const bool paused) const {
        if (input.space) return UiAction::TogglePause;
        if (input.tab) return UiAction::CycleSpeed;
        if (paused && (input.right || input.up)) return UiAction::NextGeneration;
        return UiAction::None;
    }

    UiAction UiController::HandleClick(const PointerPosition mouse, const bool isHomeScreen,
                                       const bool paused, SimulationSettings& settings) {
        if (dialog.IsOpen()) {
            const UiAction action = dialog.Click(mouse);
            if (action == UiAction::ReturnHome) {
                stats.Close();
                map.ClearSelection();
            }
            return action;
        }
        if (!isHomeScreen) return HandleSimulationClick(mouse, paused);
        const UiAction action = home.Click(mouse, settings);
        if (action == UiAction::StartOrRestart) map.ClearSelection();
        return action;
    }

    // Les commandes du temps sont communes à la carte et aux statistiques.
    UiAction UiController::HandleSimulationClick(const PointerPosition mouse, const bool paused) {
        if (RestartButton(width).Contains(mouse.x, mouse.y)) {
            stats.Close();
            map.ClearSelection();
            return UiAction::StartOrRestart;
        }
        if (PauseButton(width, stats.IsOpen()).Contains(mouse.x, mouse.y)) return UiAction::TogglePause;
        if (paused && NextButton(width, stats.IsOpen()).Contains(mouse.x, mouse.y)) {
            return UiAction::NextGeneration;
        }
        if (stats.IsOpen()) stats.Click(mouse);
        else if (GraphButton(width).Contains(mouse.x, mouse.y)) stats.Open();
        else map.Click(mouse);
        return UiAction::None;
    }
}
