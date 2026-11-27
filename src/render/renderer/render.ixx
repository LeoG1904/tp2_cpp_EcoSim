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
import ecosim.layout;

// Outils internes au rendu (non exportés).
namespace ecosim {
    /// @brief Texte blanc, sur la carte et les courbes.
    inline constexpr Color WhiteText{255, 255, 255};
    /// @brief Texte clair, sur l'accueil et les panneaux.
    inline constexpr Color LightText{239, 246, 242};
    /// @brief Texte secondaire de l'accueil.
    inline constexpr Color SoftText{177, 203, 198};
    /// @brief Fond d'un panneau.
    inline constexpr Color PanelColor{32, 45, 53};
    /// @brief Fond d'un bouton.
    inline constexpr Color ButtonColor{55, 64, 77};
    /// @brief Fond d'un élément choisi.
    inline constexpr Color ActiveColor{52, 96, 140};
    /// @brief Fond du bouton qui lance ou confirme.
    inline constexpr Color ConfirmColor{62, 148, 105};
    /// @brief Contour de saisie, repère de survol et cases cochées.
    inline constexpr Color HighlightColor{125, 183, 235};

    /// @brief Dimensions d'un texte, en pixels.
    /// @ingroup affichage
    struct TextSize {
        /// @brief Largeur.
        int cx;
        /// @brief Hauteur.
        int cy;
    };

    /// @brief Une image préparée en mémoire puis copiée d'un coup dans la fenêtre, pour éviter le scintillement.
    ///
    /// Elle réunit les dessins courants : fonds, textes et boutons. Ses ressources, polices comprises,
    /// sont libérées avec l'objet (RAII).
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

        /// @brief Remplit un rectangle d'une couleur.
        void Fill(UiRect area, Color color);
        /// @brief Choisit la police de l'interface pour les textes suivants.
        /// @param size Hauteur des caractères, en pixels.
        /// @param bold Vrai pour des caractères gras.
        void SetFont(int size, bool bold = false);
        /// @brief Écrit un texte UTF-8, sans fond.
        void Write(int x, int y, const std::string& text, Color color);
        /// @brief Écrit un texte centré horizontalement sur une abscisse.
        void WriteCentered(int centerX, int y, const std::string& text, Color color);
        /// @brief Mesure un texte UTF-8 avec la police choisie.
        TextSize Measure(const std::string& text) const;
        /// @brief Un bouton au texte centré.
        void Button(UiRect area, const std::string& label, Color background, Color ink = LightText);
        /// @brief Un bouton de commande, au texte blanc aligné à gauche.
        /// @param area Emplacement.
        /// @param label Texte.
        /// @param textTop Distance entre le haut du bouton et celui du texte.
        void SideButton(UiRect area, const std::string& label, int textTop);

    private:
        /// @brief Fenêtre de destination.
        void* window;
        /// @brief Largeur de l'image.
        int width;
        /// @brief Hauteur de l'image.
        int height;
        /// @brief Surface de la fenêtre.
        void* screen;
        /// @brief Surface en mémoire où l'on dessine.
        void* buffer;
        /// @brief Image associée à la surface en mémoire.
        void* bitmap;
        /// @brief Image à restaurer avant de libérer les ressources.
        void* previous;
        /// @brief Police présente avant le premier choix, à restaurer à la fin.
        void* initialFont{nullptr};
        /// @brief Polices créées pour cette image.
        std::vector<void*> fonts;
    };

    /// @brief Dessine l'accueil et ses réglages.
    /// @ingroup affichage
    void DrawHomeScreen(Canvas& canvas, const SimulationSettings& current, const SettingEditor& editor,
                        int cellSize, const std::string& notice);
    /// @brief Dessine l'écran des graphiques.
    /// @ingroup affichage
    void DrawStatsScreen(Canvas& canvas, const SimulationSnapshot& simulation, const PlaybackState& playback,
                         const ChartSelection& selection, std::optional<PointerPosition> pointer);
    /// @brief Dessine la carte vue de dessus.
    /// @ingroup affichage
    void DrawFlatMap(Canvas& canvas, const SimulationSnapshot& simulation, double progress,
                     std::optional<Position> selected, const FlatView& view);
    /// @brief Dessine la carte en 3D.
    /// @ingroup affichage
    void DrawOrbitMap(Canvas& canvas, const SimulationSnapshot& simulation, double progress,
                      std::optional<Position> selected, const OrbitCamera& camera);
    /// @brief Dessine les compteurs, l'aide et les boutons par-dessus la carte.
    /// @param canvas Image à compléter.
    /// @param simulation Photo de la simulation.
    /// @param playback État de la lecture.
    /// @param threeD Vrai pour la vue 3D.
    /// @param placing Espèce en cours d'ajout : son bouton est surligné et l'aide l'explique.
    /// @ingroup affichage
    void DrawHud(Canvas& canvas, const SimulationSnapshot& simulation, const PlaybackState& playback,
                 bool threeD,
                 std::optional<AnimalKind> placing);
    /// @brief Dessine le panneau d'information de la case choisie.
    /// @ingroup affichage
    void DrawCellPanel(Canvas& canvas, const SimulationSnapshot& simulation, std::optional<Position> selected,
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
        void DrawHome(const SimulationSettings& settings, const std::string& notice) override;
        /// @copydoc View::DrawSimulation
        void DrawSimulation(const SimulationSnapshot& simulation, const PlaybackState& playback) override;

    private:
        /// @brief Affiche l'image, puis attend un court instant avant la suivante.
        static void Finish(const Canvas& canvas);

        /// @brief Navigation à afficher.
        const UiController& controller;
        /// @brief Identifiant de la fenêtre.
        void* window;
        /// @brief Largeur de l'écran.
        int width;
        /// @brief Hauteur de l'écran.
        int height;
        /// @brief Pas du quadrillage décoratif de l'accueil, en pixels.
        int cellSize;
    };
}
