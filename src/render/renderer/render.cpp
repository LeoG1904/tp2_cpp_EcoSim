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
          height{target.Height()}, home{cellSize},
          maps{std::make_unique<FlatMapPainter>(cellSize),
               std::make_unique<OrbitMapPainter>(controller.Map().Camera())} {}

    void Win32Renderer::DrawHome(const SimulationSettings& settings) {
        Canvas canvas{window, width, height};
        home.Draw(canvas, settings, controller.Home().Editor());
        Finish(canvas);
    }

    void Win32Renderer::DrawSimulation(const Simulation& simulation, const PlaybackState& playback) {
        Canvas canvas{window, width, height};
        // L'écran affiché est un choix de navigation, fait par le contrôleur.
        if (controller.Stats().IsOpen()) {
            stats.Draw(canvas, simulation, playback, controller.Stats().Selection(),
                       controller.Pointer());
        } else {
            DrawMap(canvas, simulation, playback);
        }
        if (controller.Dialog().IsOpen()) exitDialog.Draw(canvas);
        Finish(canvas);
    }

    void Win32Renderer::DrawMap(Canvas& canvas, const Simulation& simulation,
                                const PlaybackState& playback) const {
        const MapNavigator& map = controller.Map();
        maps.at(static_cast<std::size_t>(map.ActiveKind()))
            ->Draw(canvas, simulation, playback.progress, map.SelectedCell());
        hud.Draw(canvas, simulation, playback, map.ActiveView(), map.NextView());
        cellPanel.Draw(canvas, simulation, map.SelectedCell(), map.IsCellPanelOnLeft());
    }

    void Win32Renderer::Finish(const Canvas& canvas) {
        canvas.Present();
        Sleep(16);
    }
}
