// Navigation entre les écrans : transforme des entrées en actions, sans API graphique.
export module ecosim.controller;

import std;
export import ecosim.settings_form;

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
    };

    class UiController {
    public:
        UiController(int width, int height);

        // Sur l'accueil, la saisie modifie directement les réglages.
        UiAction Handle(const RawInput& input, bool isHomeScreen, bool paused,
                        SimulationSettings& settings);

        bool IsShowingGraph() const;
        bool IsExitDialogOpen() const;
        const SettingEditor& Editor() const;

    private:
        void HandleTyping(const std::vector<KeyStroke>& typed, SimulationSettings& settings);
        UiAction HandleHomeClick(PointerPosition mouse, SimulationSettings& settings);
        UiAction HandleExitDialogClick(PointerPosition mouse);
        UiAction HandleSimulationClick(PointerPosition mouse, bool paused);

        int width;
        int height;
        bool showingGraph{false};
        bool showingExitDialog{false};
        SettingEditor editor;
    };
}
