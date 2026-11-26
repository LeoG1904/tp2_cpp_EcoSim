/**
 * @file
 * @brief Choix des fonctions de dessin à chaque image.
 */
module;

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

module ecosim.render;

import std;

namespace ecosim {
    Win32Renderer::Win32Renderer(const Win32Window& target, const UiController& controller,
                                 const int cellSize)
        : controller{controller}, window{target.Handle()}, width{target.Width()},
          height{target.Height()}, cellSize{cellSize} {}

    void Win32Renderer::DrawHome(const SimulationSettings& settings) {
        Canvas canvas{window, width, height};
        DrawHomeScreen(canvas, settings, controller.Editor(), cellSize);
        Finish(canvas);
    }

    void Win32Renderer::DrawSimulation(const Simulation& simulation, const PlaybackState& playback) {
        Canvas canvas{window, width, height};
        // L'écran affiché est un choix de navigation, fait par le contrôleur.
        if (controller.IsShowingStats()) {
            DrawStatsScreen(canvas, simulation, playback, controller.Charts(), controller.Pointer());
        } else {
            if (controller.IsShowing3D()) {
                DrawOrbitMap(canvas, simulation, playback.progress, controller.SelectedCell(),
                             controller.Camera());
            } else {
                DrawFlatMap(canvas, simulation, playback.progress, controller.SelectedCell(), cellSize);
            }
            DrawHud(canvas, simulation, playback, controller.IsShowing3D());
            DrawCellPanel(canvas, simulation, controller.SelectedCell(), controller.IsCellPanelOnLeft());
        }
        if (controller.IsExitDialogOpen()) DrawExitDialog(canvas);
        Finish(canvas);
    }

    void Win32Renderer::Finish(const Canvas& canvas) {
        canvas.Present();
        Sleep(16);
    }
}
