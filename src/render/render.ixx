// Rendu 2D avec Win32/GDI : dessine l'état sans le modifier.
export module ecosim.render;

import std;
export import ecosim.application;
export import ecosim.window;

// Outils internes au rendu (non exportés) : tout le texte passe par ces deux fonctions.
namespace ecosim {
    struct TextSize {
        int cx;
        int cy;
    };

    void WriteText(void* context, int x, int y, const std::string& text);
    TextSize MeasureText(void* context, const std::string& text);
}

export namespace ecosim {
    class Renderer2D final : public View {
    public:
        // window et controller doivent rester valides pendant la vie du rendu.
        Renderer2D(const Win32Window& window, const UiController& controller, int cellSize);

        void DrawHome(const SimulationSettings& settings) override;
        void DrawSimulation(const Simulation& simulation, const PlaybackState& playback) override;

    private:
        void DrawWorld(const Simulation& simulation, int speed, bool paused, double progress) const;
        void DrawGraph(const Simulation& simulation, int speed, bool paused) const;
        void DrawExitDialog(void* context) const;

        const UiController& controller;
        int cellSize;
        int windowWidth;
        int windowHeight;
        void* window;
    };
}
