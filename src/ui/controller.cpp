module;

#include "layout.hpp"

module ecosim.controller;

import std;

using namespace ecosim_render_detail;

namespace ecosim {
    UiController::UiController(const int width, const int height) : width{width}, height{height} {}

    bool UiController::IsShowingGraph() const { return showingGraph; }
    bool UiController::IsExitDialogOpen() const { return showingExitDialog; }
    const SettingEditor& UiController::Editor() const { return editor; }

    UiAction UiController::Handle(const RawInput& input, const bool isHomeScreen, const bool paused,
                                  SimulationSettings& settings) {
        if (isHomeScreen) HandleTyping(input.typed, settings);
        if (input.escape) {
            if (isHomeScreen) {
                // Pendant une saisie, Échap annule au lieu de quitter.
                if (!editor.Field()) return UiAction::Quit;
                editor.Cancel();
                return UiAction::None;
            }
            showingExitDialog = !showingExitDialog;
            return showingExitDialog ? UiAction::None : UiAction::CancelExit;
        }
        if (!isHomeScreen && !showingExitDialog) {
            if (input.space) return UiAction::TogglePause;
            if (input.tab) return UiAction::CycleSpeed;
            if (paused && (input.right || input.up)) return UiAction::NextGeneration;
        }
        if (!input.click) return UiAction::None;
        if (showingExitDialog) return HandleExitDialogClick(*input.click);
        if (isHomeScreen) return HandleHomeClick(*input.click, settings);
        return HandleSimulationClick(*input.click, paused);
    }

    void UiController::HandleTyping(const std::vector<KeyStroke>& typed,
                                    SimulationSettings& settings) {
        for (const KeyStroke& key : typed) {
            const auto field = editor.Field();
            if (!field) return;
            switch (key.kind) {
                case KeyStroke::Kind::Digit: editor.Type(key.digit); break;
                case KeyStroke::Kind::Erase: editor.Erase(); break;
                case KeyStroke::Kind::Confirm: editor.Commit(settings); break;
                case KeyStroke::Kind::Next:
                    editor.Commit(settings);
                    editor.Begin((*field + 1) % SettingFields().size(), settings);
                    break;
            }
        }
    }

    UiAction UiController::HandleHomeClick(const PointerPosition mouse, SimulationSettings& settings) {
        const auto fields = SettingFields();
        std::array<int, 3> rows{};
        for (std::size_t i = 0; i < fields.size(); ++i) {
            const int column = static_cast<int>(fields[i].group);
            const int row = rows[column]++;
            if (!ValueBox(width, height, column, row).Contains(mouse.x, mouse.y)) continue;
            if (editor.Field() != i) {
                editor.Commit(settings);
                editor.Begin(i, settings);
            }
            return UiAction::None;
        }
        // Un clic en dehors des cases valide la saisie en cours.
        editor.Commit(settings);
        if (StartButton(width, height).Contains(mouse.x, mouse.y)) return UiAction::StartOrRestart;
        return ResetButton(width, height).Contains(mouse.x, mouse.y)
            ? UiAction::ResetSettings : UiAction::None;
    }

    UiAction UiController::HandleExitDialogClick(const PointerPosition mouse) {
        if (ExitMenuButton(width, height).Contains(mouse.x, mouse.y)) {
            showingExitDialog = false;
            showingGraph = false;
            return UiAction::ReturnHome;
        }
        if (ExitQuitButton(width, height).Contains(mouse.x, mouse.y)) return UiAction::Quit;
        if (ExitCancelButton(width, height).Contains(mouse.x, mouse.y)) {
            showingExitDialog = false;
            return UiAction::CancelExit;
        }
        return UiAction::None;
    }

    UiAction UiController::HandleSimulationClick(const PointerPosition mouse, const bool paused) {
        if (RestartButton(width).Contains(mouse.x, mouse.y)) {
            showingGraph = false;
            return UiAction::StartOrRestart;
        }
        if (PauseButton(width, showingGraph).Contains(mouse.x, mouse.y)) return UiAction::TogglePause;
        if (paused && NextButton(width, showingGraph).Contains(mouse.x, mouse.y)) {
            return UiAction::NextGeneration;
        }
        const UiRect button = showingGraph ? BackButton() : GraphButton(width);
        if (button.Contains(mouse.x, mouse.y)) showingGraph = !showingGraph;
        return UiAction::None;
    }
}
