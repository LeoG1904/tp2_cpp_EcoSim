// Entrées de l'utilisateur et actions demandées, sans API graphique.
export module ecosim.input;

import std;

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
}
