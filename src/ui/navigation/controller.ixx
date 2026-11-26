/**
 * @file
 * @brief La navigation : des entrées aux actions.
 */
export module ecosim.controller;

import std;
export import ecosim.settings_form;
export import ecosim.chart;
export import ecosim.camera;

export namespace ecosim {
    /// @brief Ce que l'utilisateur demande à l'application.
    /// @ingroup interface
    enum class UiAction { None, StartOrRestart, TogglePause, NextGeneration,
                          CycleSpeed, ResetSettings,
                          CancelExit, ReturnHome, Quit };

    /// @brief Une touche utile à la saisie d'un réglage.
    /// @ingroup interface
    struct KeyStroke {
        /// @brief Nature de la touche : chiffre, effacement, validation ou case suivante.
        enum class Kind { Digit, Erase, Confirm, Next };
        /// @brief Nature de cette touche.
        Kind kind;
        /// @brief Chiffre tapé, pour la nature Digit.
        int digit{0};
    };

    /// @brief Position de la souris, en pixels.
    /// @ingroup interface
    struct PointerPosition {
        /// @brief Abscisse, en pixels.
        int x;
        /// @brief Ordonnée, en pixels.
        int y;
    };

    /// @brief Les entrées d'une image ; chaque appui n'y figure qu'une fois.
    /// @ingroup interface
    struct RawInput {
        /// @brief Touches de saisie.
        std::vector<KeyStroke> typed;
        /// @brief Échap.
        bool escape{false};
        /// @brief Espace.
        bool space{false};
        /// @brief Tabulation.
        bool tab{false};
        /// @brief Flèche droite.
        bool right{false};
        /// @brief Flèche haut.
        bool up{false};
        /// @brief Clic gauche.
        std::optional<PointerPosition> click;
        /// @brief Position de la souris, pour le survol.
        std::optional<PointerPosition> pointer;
        /// @brief Bouton droit maintenu.
        bool rotating{false};
        /// @brief Crans de molette, positifs vers l'avant.
        int wheel{0};
    };

    /// @brief Gère tous les écrans : accueil, carte, graphiques et confirmation.
    ///
    /// Ce code ne dessine rien et ne dépend pas de Windows.
    /// @ingroup interface
    class UiController {
    public:
        /// @brief Crée le contrôleur.
        /// @param width Largeur de l'écran, en pixels.
        /// @param height Hauteur de l'écran.
        /// @param cellSize Côté d'une case de la carte 2D.
        UiController(int width, int height, int cellSize);

        /// @brief Traite les entrées d'une image.
        /// @param input Entrées.
        /// @param isHomeScreen Vrai sur l'accueil.
        /// @param paused Vrai si la simulation est en pause.
        /// @param settings Réglages, modifiés par la saisie sur l'accueil.
        /// @return L'action demandée.
        UiAction Handle(const RawInput& input, bool isHomeScreen, bool paused,
                        SimulationSettings& settings);

        /// @brief Vrai si la confirmation de sortie est affichée.
        bool IsExitDialogOpen() const;
        /// @brief Vrai sur l'écran des graphiques.
        bool IsShowingStats() const;
        /// @brief Vrai si la carte est affichée en 3D.
        bool IsShowing3D() const;
        /// @brief La saisie en cours sur l'accueil.
        const SettingEditor& Editor() const;
        /// @brief Les graphiques choisis.
        const ChartSelection& Charts() const;
        /// @brief La caméra 3D.
        const OrbitCamera& Camera() const;
        /// @brief La position de la souris.
        std::optional<PointerPosition> Pointer() const;
        /// @brief La case choisie, le cas échéant.
        std::optional<Position> SelectedCell() const;
        /// @brief Côté du panneau d'information, pour ne pas cacher la case choisie.
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
