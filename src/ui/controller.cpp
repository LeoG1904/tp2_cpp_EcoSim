module;

#include "layout.hpp"

module ecosim.controller;

import std;

using namespace ecosim_render_detail;

namespace ecosim {
    UiController::UiController(const int width, const int height, const int cellSize)
        : width{width}, height{height}, cellSize{cellSize} {}

    std::optional<Position> UiController::SelectedCell() const { return selectedCell; }
    const ChartSelection& UiController::Charts() const { return charts; }
    std::optional<PointerPosition> UiController::Pointer() const { return pointer; }

    bool UiController::IsCellPanelOnLeft() const {
        return selectedCell && selectedCell->x * cellSize >= width / 2;
    }

    bool UiController::IsShowingGraph() const { return showingGraph; }
    bool UiController::IsExitDialogOpen() const { return showingExitDialog; }
    const SettingEditor& UiController::Editor() const { return editor; }

    UiAction UiController::Handle(const RawInput& input, const bool isHomeScreen, const bool paused,
                                  SimulationSettings& settings) {
        pointer = input.pointer;
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
        for (std::size_t i = 0; i < Presets.size(); ++i) {
            if (PresetButton(width, height, static_cast<int>(i)).Contains(mouse.x, mouse.y)) {
                settings = PresetSettings(Presets[i], width / cellSize, height / cellSize);
                return UiAction::None;
            }
        }
        if (StartButton(width, height).Contains(mouse.x, mouse.y)) {
            selectedCell.reset();
            return UiAction::StartOrRestart;
        }
        return ResetButton(width, height).Contains(mouse.x, mouse.y)
            ? UiAction::ResetSettings : UiAction::None;
    }

    UiAction UiController::HandleExitDialogClick(const PointerPosition mouse) {
        if (ExitMenuButton(width, height).Contains(mouse.x, mouse.y)) {
            showingExitDialog = false;
            showingGraph = false;
            selectedCell.reset();
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
            selectedCell.reset();
            return UiAction::StartOrRestart;
        }
        if (PauseButton(width, showingGraph).Contains(mouse.x, mouse.y)) return UiAction::TogglePause;
        if (paused && NextButton(width, showingGraph).Contains(mouse.x, mouse.y)) {
            return UiAction::NextGeneration;
        }
        const UiRect button = showingGraph ? BackButton() : GraphButton(width);
        if (button.Contains(mouse.x, mouse.y)) showingGraph = !showingGraph;
        else if (showingGraph) HandleChartClick(mouse);
        else HandleMapClick(mouse);
        return UiAction::None;
    }

    void UiController::HandleChartClick(const PointerPosition mouse) {
        for (std::size_t i = 0; i < MetricCount; ++i) {
            if (MetricChip(width, static_cast<int>(i)).Contains(mouse.x, mouse.y)) {
                charts.metrics[i] = !charts.metrics[i];
            }
        }
        if (SpeciesChip(0).Contains(mouse.x, mouse.y)) charts.herbivores = !charts.herbivores;
        if (SpeciesChip(1).Contains(mouse.x, mouse.y)) charts.predators = !charts.predators;
        if (CumulativeButton().Contains(mouse.x, mouse.y)) charts.cumulative = !charts.cumulative;
    }

    void UiController::HandleMapClick(const PointerPosition mouse) {
        if (selectedCell) {
            const UiRect panel = CellPanel(width, height, IsCellPanelOnLeft());
            if (panel.Contains(mouse.x, mouse.y)) {
                if (CellPanelClose(panel).Contains(mouse.x, mouse.y)) selectedCell.reset();
                return;
            }
        }
        const Position cell{mouse.x / cellSize, mouse.y / cellSize};
        // La bande restante au bord de l'écran ne contient pas de case.
        if (mouse.x < 0 || mouse.y < 0 || cell.x >= width / cellSize || cell.y >= height / cellSize) {
            return;
        }
        // Un second clic sur la case choisie ferme le panneau.
        if (selectedCell == cell) selectedCell.reset();
        else selectedCell = cell;
    }
}
