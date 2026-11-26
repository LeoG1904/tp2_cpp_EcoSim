// Rendu Win32/GDI. Il dessine l'état sans le modifier.
export module ecosim.render;

import std;
export import ecosim.application;
export import ecosim.window;
import ecosim.camera;
import ecosim.chart;
import ecosim.settings_form;

// Outils internes au rendu (non exportés).
namespace ecosim {
    // Image préparée en mémoire puis copiée dans la fenêtre ; les ressources suivent l'objet.
    class Canvas {
    public:
        Canvas(void* window, int width, int height);
        ~Canvas();

        Canvas(const Canvas&) = delete;
        Canvas& operator=(const Canvas&) = delete;

        void* Context() const;
        int Width() const;
        int Height() const;
        void Present() const;

    private:
        void* window;
        int width;
        int height;
        void* screen;
        void* buffer;
        void* bitmap;
        void* previous;
    };

    struct TextSize {
        int cx;
        int cy;
    };

    // Tout le texte passe par ces deux fonctions.
    void WriteText(void* context, int x, int y, const std::string& text);
    TextSize MeasureText(void* context, const std::string& text);

    // Une fonction par écran ou élément ; chacune ne reçoit que ce qu'elle affiche.
    void DrawHomeScreen(Canvas& canvas, const SimulationSettings& current, const SettingEditor& editor,
                        int cellSize);
    void DrawStatsScreen(Canvas& canvas, const Simulation& simulation, const PlaybackState& playback,
                         const ChartSelection& selection, std::optional<PointerPosition> pointer);
    void DrawFlatMap(Canvas& canvas, const Simulation& simulation, double progress,
                     std::optional<Position> selected, int cellSize);
    void DrawOrbitMap(Canvas& canvas, const Simulation& simulation, double progress,
                      std::optional<Position> selected, const OrbitCamera& camera);
    // Compteurs, aide et boutons affichés par-dessus la carte.
    void DrawHud(Canvas& canvas, const Simulation& simulation, const PlaybackState& playback,
                 bool threeD);
    void DrawCellPanel(Canvas& canvas, const Simulation& simulation, std::optional<Position> selected,
                       bool onLeft);
    void DrawExitDialog(Canvas& canvas);
}

export namespace ecosim {
    class Win32Renderer final : public View {
    public:
        // window et controller doivent rester valides pendant la vie du rendu.
        Win32Renderer(const Win32Window& window, const UiController& controller, int cellSize);

        void DrawHome(const SimulationSettings& settings) override;
        void DrawSimulation(const Simulation& simulation, const PlaybackState& playback) override;

    private:
        static void Finish(const Canvas& canvas);

        const UiController& controller;
        void* window;
        int width;
        int height;
        int cellSize;
    };
}
