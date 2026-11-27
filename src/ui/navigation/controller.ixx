/**
 * @file
 * @brief La navigation : des entrées aux actions.
 */
export module ecosim.controller;

import std;
export import ecosim.settings_form;
export import ecosim.chart;
export import ecosim.camera;
export import ecosim.flat_view;

export namespace ecosim {
    /// @brief Ce que l'utilisateur demande à l'application.
    /// @ingroup interface
    enum class UiAction {
        None, ///< Aucune action.
        StartOrRestart, ///< Démarrer, ou recommencer avec une nouvelle carte.
        TogglePause, ///< Mettre en pause ou reprendre.
        NextGeneration, ///< Avancer d'une génération pendant la pause.
        CycleSpeed, ///< Passer à la vitesse suivante.
        ResetSettings, ///< Rétablir les réglages d'origine.
        CancelExit, ///< Fermer la confirmation de sortie.
        ReturnHome, ///< Revenir à l'accueil.
        Quit ///< Fermer l'application.
    };

    /// @brief Une touche utile à la saisie d'un réglage.
    /// @ingroup interface
    struct KeyStroke {
        /// @brief Nature de la touche : chiffre, effacement, validation ou case suivante.
        enum class Kind {
            Digit, ///< Un chiffre.
            Erase, ///< Effacer.
            Confirm, ///< Valider la saisie.
            Next ///< Valider et passer à la case suivante.
        };
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
        /// @param cellSize Côté d'une case de la carte de départ, qui remplit l'écran.
        UiController(int width, int height, int cellSize);

        /// @brief Traite les entrées d'une image.
        /// @param input Entrées.
        /// @param isHomeScreen Vrai sur l'accueil.
        /// @param paused Vrai si la simulation est en pause.
        /// @param settings Réglages, modifiés par la saisie sur l'accueil.
        /// @return L'action demandée.
        /// @param world Carte affichée, pour viser une case en relief ; nul sur l'accueil.
        UiAction Handle(const RawInput& input, bool isHomeScreen, bool paused,
                        SimulationSettings& settings, const World* world = nullptr);

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
        /// @brief La vue 2D : zoom et déplacement.
        const FlatView& Flat() const;
        /// @brief La position de la souris.
        std::optional<PointerPosition> Pointer() const;
        /// @brief La case choisie, le cas échéant.
        std::optional<Position> SelectedCell() const;
        /// @brief Côté du panneau d'information, pour ne pas cacher la case choisie.
        bool IsCellPanelOnLeft() const;

    private:
        /// @brief Transmet les touches tapées à la saisie en cours.
        void Type(const std::vector<KeyStroke>& typed, SimulationSettings& settings);
        /// @brief Avec la souris, tourne et zoome la caméra en 3D, ou déplace et zoome la carte en 2D.
        void MoveCamera(const RawInput& input);
        /// @brief Prépare les vues avant une simulation ; elles sont conservées si la taille ne change pas.
        void PrepareViews(const SimulationSettings& settings);
        /// @brief Traite un clic sur l'accueil : case de saisie, préréglage ou bouton.
        UiAction ClickHome(PointerPosition mouse, SimulationSettings& settings);
        /// @brief Traite un clic dans la confirmation de sortie.
        UiAction ClickExitDialog(PointerPosition mouse);
        /// @brief Traite un clic pendant la simulation : commandes du temps, puis carte ou graphiques.
        UiAction ClickSimulation(PointerPosition mouse, bool paused, const World* world);
        /// @brief Coche ou décoche les choix de l'écran des graphiques.
        void ClickStats(PointerPosition mouse);
        /// @brief Change de vue, recentre la caméra ou choisit une case.
        void ClickMap(PointerPosition mouse, const World* world);

        /// @brief Largeur de l'écran, en pixels.
        int width;
        /// @brief Hauteur de l'écran, en pixels.
        int height;
        /// @brief Largeur de la carte affichée, en cases.
        int mapWidth;
        /// @brief Hauteur de la carte affichée, en cases.
        int mapHeight;
        /// @brief Saisie d'un réglage sur l'accueil.
        SettingEditor editor;
        /// @brief Vrai si la confirmation de sortie est affichée.
        bool exitDialogOpen{false};
        /// @brief Vrai sur l'écran des graphiques.
        bool showingStats{false};
        /// @brief Vrai si la carte est en 3D ; conservé d'une simulation à l'autre.
        bool showing3D{false};
        /// @brief Caméra 3D ; sa position est conservée d'une simulation à l'autre de même taille.
        OrbitCamera camera;
        /// @brief Vue 2D ; son zoom est conservé d'une simulation à l'autre de même taille.
        FlatView flat;
        /// @brief Graphiques choisis ; conservés d'une simulation à l'autre.
        ChartSelection charts;
        /// @brief Case dont le panneau est ouvert.
        std::optional<Position> selectedCell;
        /// @brief Dernière position connue de la souris.
        std::optional<PointerPosition> pointer;
        /// @brief Position précédente de la souris pendant un glissement au clic droit ; vide sinon.
        std::optional<PointerPosition> dragOrigin;
    };
}
