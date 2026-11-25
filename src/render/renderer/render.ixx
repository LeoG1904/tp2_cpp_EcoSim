// Rendu Win32/GDI : un dessinateur par écran. Aucun ne modifie l'état qu'il affiche.
export module ecosim.render;

import std;
export import ecosim.application;
export import ecosim.window;
import ecosim.camera;
import ecosim.map_navigator;
import ecosim.chart;
import ecosim.settings_form;

// Classes internes au rendu (non exportées).
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

    class HomePainter {
    public:
        explicit HomePainter(int cellSize);
        void Draw(Canvas& canvas, const SimulationSettings& current, const SettingEditor& editor) const;

    private:
        int cellSize;
    };

    class StatsPainter {
    public:
        void Draw(Canvas& canvas, const Simulation& simulation, const PlaybackState& playback,
                  const ChartSelection& selection, std::optional<PointerPosition> pointer) const;
    };

    // Dessine la carte. Une nouvelle vue ajoute une classe sans modifier les autres.
    class MapPainter {
    public:
        virtual ~MapPainter() = default;
        virtual void Draw(Canvas& canvas, const Simulation& simulation, double progress,
                          std::optional<Position> selected) const = 0;
    };

    class FlatMapPainter final : public MapPainter {
    public:
        explicit FlatMapPainter(int cellSize);
        void Draw(Canvas& canvas, const Simulation& simulation, double progress,
                  std::optional<Position> selected) const override;

    private:
        int cellSize;
    };

    class OrbitMapPainter final : public MapPainter {
    public:
        // camera doit rester valide pendant la vie du dessinateur.
        explicit OrbitMapPainter(const OrbitCamera& camera);
        void Draw(Canvas& canvas, const Simulation& simulation, double progress,
                  std::optional<Position> selected) const override;

    private:
        const OrbitCamera& camera;
    };

    // Compteurs, aide et boutons affichés par-dessus la carte.
    class HudPainter {
    public:
        void Draw(Canvas& canvas, const Simulation& simulation, const PlaybackState& playback,
                  const MapView& view, const MapView& nextView) const;
    };

    class CellPanelPainter {
    public:
        void Draw(Canvas& canvas, const Simulation& simulation, std::optional<Position> selected,
                  bool onLeft) const;
    };

    class ExitDialogPainter {
    public:
        void Draw(Canvas& canvas) const;
    };
}

export namespace ecosim {
    // Assemble les dessinateurs et choisit l'écran d'après la navigation.
    class Win32Renderer final : public View {
    public:
        // window et controller doivent rester valides pendant la vie du rendu.
        Win32Renderer(const Win32Window& window, const UiController& controller, int cellSize);

        void DrawHome(const SimulationSettings& settings) override;
        void DrawSimulation(const Simulation& simulation, const PlaybackState& playback) override;

    private:
        void DrawMap(Canvas& canvas, const Simulation& simulation, const PlaybackState& playback) const;
        static void Finish(const Canvas& canvas);

        const UiController& controller;
        void* window;
        int width;
        int height;
        HomePainter home;
        StatsPainter stats;
        // Dans l'ordre de MapViewKind.
        std::array<std::unique_ptr<MapPainter>, MapViewCount> maps;
        HudPainter hud;
        CellPanelPainter cellPanel;
        ExitDialogPainter exitDialog;
    };
}
