// Navigation entre les écrans : transforme des entrées en actions, sans API graphique.
export module ecosim.controller;

import std;
export import ecosim.settings_form;
export import ecosim.chart;

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
    };

    class UiController {
    public:
        // cellSize : côté d'une case de la carte, en pixels.
        UiController(int width, int height, int cellSize);

        // Sur l'accueil, la saisie modifie directement les réglages.
        UiAction Handle(const RawInput& input, bool isHomeScreen, bool paused,
                        SimulationSettings& settings);

        bool IsShowingGraph() const;
        bool IsExitDialogOpen() const;
        const SettingEditor& Editor() const;
        const ChartSelection& Charts() const;
        std::optional<PointerPosition> Pointer() const;
        std::optional<Position> SelectedCell() const;
        // Le panneau évite de recouvrir la case choisie.
        bool IsCellPanelOnLeft() const;

    private:
        void HandleTyping(const std::vector<KeyStroke>& typed, SimulationSettings& settings);
        UiAction HandleHomeClick(PointerPosition mouse, SimulationSettings& settings);
        UiAction HandleExitDialogClick(PointerPosition mouse);
        UiAction HandleSimulationClick(PointerPosition mouse, bool paused);
        void HandleMapClick(PointerPosition mouse);
        void HandleChartClick(PointerPosition mouse);

        int width;
        int height;
        int cellSize;
        std::optional<Position> selectedCell;
        // Conservé d'une simulation à l'autre : c'est une préférence d'affichage.
        ChartSelection charts;
        std::optional<PointerPosition> pointer;
        bool showingGraph{false};
        bool showingExitDialog{false};
        SettingEditor editor;
    };
}
