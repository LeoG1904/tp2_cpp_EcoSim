/**
 * @file
 * @brief Traitement des entrées de chaque écran.
 */
module ecosim.controller;

import std;
import ecosim.layout;
import ecosim.species;

namespace ecosim {
    UiController::UiController(const int width, const int height, const int cellSize)
        : width{width}, height{height}, mapWidth{std::max(MinMapSize, width / cellSize)},
          mapHeight{std::max(MinMapSize, height / cellSize)}, camera{mapWidth, mapHeight},
          flat{mapWidth, mapHeight, {width, height}} {}

    const FlatView& UiController::Flat() const { return flat; }

    void UiController::PrepareViews(const SimulationSettings& settings) {
        if (settings.mapWidth == mapWidth && settings.mapHeight == mapHeight) return;
        mapWidth = settings.mapWidth;
        mapHeight = settings.mapHeight;
        camera = OrbitCamera{mapWidth, mapHeight};
        flat = FlatView{mapWidth, mapHeight, {width, height}};
    }

    bool UiController::IsExitDialogOpen() const { return exitDialogOpen; }
    bool UiController::IsShowingStats() const { return showingStats; }
    bool UiController::IsShowing3D() const { return showing3D; }
    const SettingEditor& UiController::Editor() const { return editor; }
    const ChartSelection& UiController::Charts() const { return charts; }
    const OrbitCamera& UiController::Camera() const { return camera; }
    std::optional<PointerPosition> UiController::Pointer() const { return pointer; }
    std::optional<Position> UiController::SelectedCell() const { return selectedCell; }

    bool UiController::IsCellPanelOnLeft() const {
        if (!selectedCell) return false;
        if (!showing3D) return flat.ScreenX(selectedCell->x + 0.5) >= width / 2;
        const auto centre = camera.Project({selectedCell->x + 0.5, 0.0, selectedCell->y + 0.5},
                                           {width, height});
        return centre && centre->x >= width / 2;
    }

    UiAction UiController::Handle(const RawInput& input, const bool isHomeScreen, const bool paused,
                                  SimulationSettings& settings, const World* world) {
        pointer = input.pointer;
        if (isHomeScreen) Type(input.typed, settings);
        if (!isHomeScreen && !showingStats && !exitDialogOpen) MoveCamera(input);
        else dragOrigin.reset();

        if (input.escape) {
            if (!isHomeScreen) {
                exitDialogOpen = !exitDialogOpen;
                return exitDialogOpen ? UiAction::None : UiAction::CancelExit;
            }
            // Pendant une saisie, Échap annule au lieu de quitter.
            if (!editor.Field()) return UiAction::Quit;
            editor.Cancel();
            return UiAction::None;
        }
        if (!isHomeScreen && !exitDialogOpen) {
            if (input.space) return UiAction::TogglePause;
            if (input.tab) return UiAction::CycleSpeed;
            if (paused && (input.right || input.up)) return UiAction::NextGeneration;
        }
        if (!input.click) return UiAction::None;
        if (exitDialogOpen) return ClickExitDialog(*input.click);
        if (isHomeScreen) return ClickHome(*input.click, settings);
        return ClickSimulation(*input.click, paused, world);
    }

    void UiController::Type(const std::vector<KeyStroke>& typed, SimulationSettings& settings) {
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

    void UiController::MoveCamera(const RawInput& input) {
        if (input.wheel != 0) {
            if (showing3D) camera.Zoom(input.wheel);
            else if (input.pointer) flat.Zoom(input.wheel, {input.pointer->x, input.pointer->y});
        }
        if (!input.rotating || !input.pointer) {
            dragOrigin.reset();
            return;
        }
        // La carte suit la souris : glisser vers le bas fait passer la caméra au-dessus.
        if (dragOrigin && showing3D) {
            constexpr double radiansPerPixel{0.006};
            camera.Rotate((dragOrigin->x - input.pointer->x) * radiansPerPixel,
                          (input.pointer->y - dragOrigin->y) * radiansPerPixel);
        } else if (dragOrigin) {
            flat.Pan(input.pointer->x - dragOrigin->x, input.pointer->y - dragOrigin->y);
        }
        dragOrigin = input.pointer;
    }

    UiAction UiController::ClickHome(const PointerPosition mouse, SimulationSettings& settings) {
        const auto& fields = SettingFields();
        std::array<int, SettingColumns> rows{};
        for (std::size_t i = 0; i < fields.size(); ++i) {
            const int column = static_cast<int>(fields[i].column);
            const int row = rows.at(fields[i].column)++;
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
                settings = PresetSettings(Presets[i], settings.mapWidth, settings.mapHeight);
                return UiAction::None;
            }
        }
        if (StartButton(width, height).Contains(mouse.x, mouse.y)) {
            selectedCell.reset();
            PrepareViews(settings);
            return UiAction::StartOrRestart;
        }
        return ResetButton(width, height).Contains(mouse.x, mouse.y)
            ? UiAction::ResetSettings : UiAction::None;
    }

    UiAction UiController::ClickExitDialog(const PointerPosition mouse) {
        if (ExitMenuButton(width, height).Contains(mouse.x, mouse.y)) {
            exitDialogOpen = false;
            showingStats = false;
            selectedCell.reset();
            return UiAction::ReturnHome;
        }
        if (ExitQuitButton(width, height).Contains(mouse.x, mouse.y)) return UiAction::Quit;
        if (ExitCancelButton(width, height).Contains(mouse.x, mouse.y)) {
            exitDialogOpen = false;
            return UiAction::CancelExit;
        }
        return UiAction::None;
    }

    // Les commandes du temps sont communes à la carte et aux statistiques.
    UiAction UiController::ClickSimulation(const PointerPosition mouse, const bool paused,
                                           const World* world) {
        if (RestartButton(width).Contains(mouse.x, mouse.y)) {
            showingStats = false;
            selectedCell.reset();
            return UiAction::StartOrRestart;
        }
        if (PauseButton(width, showingStats).Contains(mouse.x, mouse.y)) return UiAction::TogglePause;
        if (paused && NextButton(width, showingStats).Contains(mouse.x, mouse.y)) {
            return UiAction::NextGeneration;
        }
        if (showingStats) ClickStats(mouse);
        else if (GraphButton(width).Contains(mouse.x, mouse.y)) showingStats = true;
        else ClickMap(mouse, world);
        return UiAction::None;
    }

    void UiController::ClickStats(const PointerPosition mouse) {
        if (BackButton().Contains(mouse.x, mouse.y)) showingStats = false;
        for (std::size_t i = 0; i < MetricCount; ++i) {
            if (MetricChip(width, static_cast<int>(i)).Contains(mouse.x, mouse.y)) {
                charts.metrics[i] = !charts.metrics[i];
            }
        }
        for (const SpeciesInfo& species : SpeciesCatalog()) {
            if (SpeciesChip(static_cast<int>(species.kind)).Contains(mouse.x, mouse.y)) {
                charts.hidden[species.kind] = !charts.hidden[species.kind];
            }
        }
        if (CumulativeButton().Contains(mouse.x, mouse.y)) charts.cumulative = !charts.cumulative;
    }

    void UiController::ClickMap(const PointerPosition mouse, const World* world) {
        if (ViewButton(width).Contains(mouse.x, mouse.y)) {
            showing3D = !showing3D;
            return;
        }
        if (RecenterButton(width).Contains(mouse.x, mouse.y)) {
            if (showing3D) camera.Reset();
            else flat.Reset();
            return;
        }
        if (selectedCell) {
            const UiRect panel = CellPanel(width, height, IsCellPanelOnLeft());
            if (panel.Contains(mouse.x, mouse.y)) {
                if (CellPanelClose(panel).Contains(mouse.x, mouse.y)) selectedCell.reset();
                return;
            }
        }
        // Les bandes autour de la carte ne contiennent pas de case.
        const std::optional<Position> cell = showing3D
            ? camera.PickCell({mouse.x, mouse.y}, {width, height}, world)
            : flat.PickCell({mouse.x, mouse.y});
        if (!cell) return;
        // Un second clic sur la case choisie ferme le panneau.
        if (selectedCell == cell) selectedCell.reset();
        else selectedCell = cell;
    }
}
