// Navigation entre les écrans : transforme les entrées en actions, sans API graphique.
export module ecosim.controller;

import std;
export import ecosim.settings_form;
export import ecosim.chart;
export import ecosim.camera;

export namespace ecosim {
    enum class UiAction { None, StartOrRestart, TogglePause, NextGeneration,
                          CycleSpeed, ResetSettings,
                          CancelExit, ReturnHome, Quit };

    // Touche utile à la saisie d'une valeur.
    struct KeyStroke {
        enum class Kind { Digit, Erase, Confirm, Next };
        Kind kind;
        int digit{0};
    };

    struct PointerPosition {
        int x;
        int y;
    };

    // Entrées d'une image : chaque appui n'y figure qu'une fois.
    struct RawInput {
        std::vector<KeyStroke> typed;
        bool escape{false};
        bool space{false};
        bool tab{false};
        bool right{false};
        bool up{false};
        std::optional<PointerPosition> click;
        std::optional<PointerPosition> pointer; // position de la souris, pour le survol
        bool rotating{false};                   // bouton droit maintenu
        int wheel{0};                           // crans de molette, positifs vers l'avant
    };

    class UiController {
    public:
        // cellSize : côté d'une case de la carte, en pixels.
        UiController(int width, int height, int cellSize);

        // Sur l'accueil, la saisie modifie directement les réglages.
        UiAction Handle(const RawInput& input, bool isHomeScreen, bool paused,
                        SimulationSettings& settings);

        // État lu par le rendu.
        bool IsExitDialogOpen() const;
        bool IsShowingStats() const;
        bool IsShowing3D() const;
        const SettingEditor& Editor() const;
        const ChartSelection& Charts() const;
        const OrbitCamera& Camera() const;
        std::optional<PointerPosition> Pointer() const;
        std::optional<Position> SelectedCell() const;
        // Le panneau évite de recouvrir la case choisie.
        bool IsCellPanelOnLeft() const;

    private:
        void Type(const std::vector<KeyStroke>& typed, SimulationSettings& settings);
        void MoveCamera(const RawInput& input);
        UiAction ClickHome(PointerPosition mouse, SimulationSettings& settings);
        UiAction ClickExitDialog(PointerPosition mouse);
        UiAction ClickSimulation(PointerPosition mouse, bool paused);
        void ClickStats(PointerPosition mouse);
        void ClickMap(PointerPosition mouse);

        int width;
        int height;
        int cellSize;
        SettingEditor editor;
        bool exitDialogOpen{false};
        bool showingStats{false};
        bool showing3D{false};
        // La vue, la caméra et les graphiques choisis sont conservés d'une simulation à l'autre.
        OrbitCamera camera;
        ChartSelection charts;
        std::optional<Position> selectedCell;
        std::optional<PointerPosition> pointer;
        std::optional<PointerPosition> dragOrigin;
    };
}
