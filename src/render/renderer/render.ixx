/**
 * @file
 * @brief Le rendu Win32/GDI : il dessine l'état sans le modifier.
 */
export module ecosim.render;

import std;
export import ecosim.application;
export import ecosim.window;
import ecosim.camera;
import ecosim.chart;
import ecosim.settings_form;

// Outils internes au rendu (non exportés).
namespace ecosim {
    /// @brief Une image préparée en mémoire puis copiée d'un coup dans la fenêtre, pour éviter le scintillement.
    ///
    /// Ses ressources sont libérées avec l'objet (RAII).
    /// @ingroup affichage
    class Canvas {
    public:
        /// @brief Prépare une image pour la fenêtre.
        Canvas(void* window, int width, int height);
        ~Canvas();

        Canvas(const Canvas&) = delete;
        Canvas& operator=(const Canvas&) = delete;

        /// @brief Surface de dessin GDI.
        void* Context() const;
        /// @brief Largeur de l'image.
        int Width() const;
        /// @brief Hauteur de l'image.
        int Height() const;
        /// @brief Affiche l'image dans la fenêtre.
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

    /// @brief Dimensions d'un texte, en pixels.
    /// @ingroup affichage
    struct TextSize {
        /// @brief Largeur.
        int cx;
        /// @brief Hauteur.
        int cy;
    };

    /// @brief Écrit un texte UTF-8.
    /// @ingroup affichage
    void WriteText(void* context, int x, int y, const std::string& text);
    /// @brief Mesure un texte UTF-8.
    /// @ingroup affichage
    TextSize MeasureText(void* context, const std::string& text);

    /// @brief Dessine l'accueil et ses réglages.
    /// @ingroup affichage
    void DrawHomeScreen(Canvas& canvas, const SimulationSettings& current, const SettingEditor& editor,
                        int cellSize);
    /// @brief Dessine l'écran des graphiques.
    /// @ingroup affichage
    void DrawStatsScreen(Canvas& canvas, const Simulation& simulation, const PlaybackState& playback,
                         const ChartSelection& selection, std::optional<PointerPosition> pointer);
    /// @brief Dessine la carte vue de dessus.
    /// @ingroup affichage
    void DrawFlatMap(Canvas& canvas, const Simulation& simulation, double progress,
                     std::optional<Position> selected, int cellSize);
    /// @brief Dessine la carte en 3D.
    /// @ingroup affichage
    void DrawOrbitMap(Canvas& canvas, const Simulation& simulation, double progress,
                      std::optional<Position> selected, const OrbitCamera& camera);
    /// @brief Dessine les compteurs, l'aide et les boutons par-dessus la carte.
    /// @ingroup affichage
    void DrawHud(Canvas& canvas, const Simulation& simulation, const PlaybackState& playback,
                 bool threeD);
    /// @brief Dessine le panneau d'information de la case choisie.
    /// @ingroup affichage
    void DrawCellPanel(Canvas& canvas, const Simulation& simulation, std::optional<Position> selected,
                       bool onLeft);
    /// @brief Dessine la confirmation de sortie.
    /// @ingroup affichage
    void DrawExitDialog(Canvas& canvas);
}

export namespace ecosim {
    /// @brief L'affichage de l'application : il choisit ce qu'il faut dessiner.
    /// @ingroup affichage
    class Win32Renderer final : public View {
    public:
        /// @brief Crée le rendu. La fenêtre et le contrôleur doivent vivre aussi longtemps que lui.
        Win32Renderer(const Win32Window& window, const UiController& controller, int cellSize);

        /// @copydoc View::DrawHome
        void DrawHome(const SimulationSettings& settings) override;
        /// @copydoc View::DrawSimulation
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
