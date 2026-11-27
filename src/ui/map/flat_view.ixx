/**
 * @file
 * @brief La vue 2D : place de la carte à l'écran, zoom et déplacement.
 */
export module ecosim.flat_view;

import std;
export import ecosim.camera;

export namespace ecosim {
    /// @brief Les cases visibles à l'écran : colonnes [left, right[ et lignes [top, bottom[.
    /// @ingroup interface
    struct CellRange {
        /// @brief Première colonne.
        int left;
        /// @brief Première ligne.
        int top;
        /// @brief Colonne qui suit la dernière.
        int right;
        /// @brief Ligne qui suit la dernière.
        int bottom;
    };

    /// @brief La carte vue de dessus. Au départ elle tient entière et centrée à l'écran ; on peut zoomer et se déplacer.
    ///
    /// Les cases restent carrées. La carte ne quitte jamais l'écran : trop petite dans une direction, elle y est centrée.
    /// @ingroup interface
    class FlatView {
    public:
        /// @brief Côté maximal d'une case en zoomant, en pixels, sauf si la vue d'ensemble est déjà plus grande.
        static constexpr double MaxCellSize{80.0};
        /// @brief Rapport de grossissement d'un cran de molette.
        static constexpr double ZoomStep{1.25};

        /// @brief Crée la vue d'ensemble d'une carte.
        FlatView(int mapWidth, int mapHeight, Viewport screen);

        /// @brief Revient à la vue d'ensemble.
        void Reset();
        /// @brief Zoome en gardant sous la souris le même endroit de la carte ; un cran positif rapproche.
        void Zoom(int notches, ScreenPoint anchor);
        /// @brief Fait glisser la carte, en pixels.
        void Pan(int dx, int dy);

        /// @brief Côté d'une case à l'écran, en pixels ; inférieur à 1 pour une carte immense.
        double CellSize() const;
        /// @brief Abscisse à l'écran d'une abscisse de la carte, en cases.
        double ScreenX(double mapX) const;
        /// @brief Ordonnée à l'écran d'une ordonnée de la carte, en cases.
        double ScreenY(double mapY) const;
        /// @brief La case située sous un point de l'écran, s'il y en a une.
        std::optional<Position> PickCell(ScreenPoint screen) const;
        /// @brief Les cases au moins en partie visibles.
        CellRange VisibleCells() const;

    private:
        /// @brief Côté d'une case qui fait tenir toute la carte.
        double FitSize() const;
        /// @brief Centre la carte ou l'empêche de laisser un vide au bord.
        void Clamp();

        /// @brief Largeur de la carte, en cases.
        int mapWidth;
        /// @brief Hauteur de la carte, en cases.
        int mapHeight;
        /// @brief Taille de l'écran.
        Viewport screen;
        /// @brief Côté actuel d'une case, en pixels.
        double cellSize{1.0};
        /// @brief Abscisse à l'écran du bord gauche de la carte.
        double originX{0.0};
        /// @brief Ordonnée à l'écran du bord haut de la carte.
        double originY{0.0};
    };
}
