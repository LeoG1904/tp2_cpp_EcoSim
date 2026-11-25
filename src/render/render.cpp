module ecosim.render;

import std;

namespace ecosim {
    Renderer2D::Renderer2D(const Win32Window& target, const UiController& controller,
                           const int cellSize)
        : controller{controller}, cellSize{cellSize}, windowWidth{target.Width()},
          windowHeight{target.Height()}, window{target.Handle()} {}

    void Renderer2D::DrawSimulation(const Simulation& simulation, const PlaybackState& playback) {
        // L'écran affiché est un choix de navigation, fait par le contrôleur.
        if (controller.IsShowingGraph()) DrawGraph(simulation, playback.speed, playback.paused);
        else DrawWorld(simulation, playback.speed, playback.paused, playback.progress);
    }
}
