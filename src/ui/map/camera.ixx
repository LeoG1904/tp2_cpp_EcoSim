/**
 * @file
 * @brief La caméra 3D et ses calculs de projection.
 */
export module ecosim.camera;

import std;
export import ecosim.world;

export namespace ecosim {
    /// @brief Un point de l'espace 3D. La case (x, y) de la carte occupe [x, x+1] sur x et [y, y+1] sur z ; y est la hauteur.
    /// @ingroup interface
    struct Vec3 {
        /// @brief Axe des colonnes.
        double x;
        /// @brief Hauteur.
        double y;
        /// @brief Axe des lignes.
        double z;
    };

    /// @brief Un point de l'écran, en pixels.
    /// @ingroup interface
    struct ScreenPoint {
        /// @brief Abscisse, en pixels.
        int x;
        /// @brief Ordonnée, en pixels.
        int y;
    };

    /// @brief La taille de l'image.
    /// @ingroup interface
    struct Viewport {
        /// @brief Largeur, en pixels.
        int width;
        /// @brief Hauteur, en pixels.
        int height;
    };

    /// @brief Une caméra qui tourne autour du centre de la carte.
    /// @ingroup interface
    class OrbitCamera {
    public:
        /// @brief Inclinaison minimale : presque au ras du sol.
        static constexpr double MinPitch{0.30};
        /// @brief Inclinaison maximale : presque à la verticale.
        static constexpr double MaxPitch{1.45};

        /// @brief Crée la caméra en vue d'ensemble.
        OrbitCamera(int mapWidth, int mapHeight);

        /// @brief Revient à la vue d'ensemble.
        void Reset();
        /// @brief Tourne autour de la carte et change l'inclinaison, en radians.
        void Rotate(double yawDelta, double pitchDelta);
        /// @brief Zoome ; un cran positif rapproche.
        void Zoom(int notches);

        /// @brief Angle autour de la carte.
        double Yaw() const;
        /// @brief Inclinaison.
        double Pitch() const;
        /// @brief Distance au centre.
        double Distance() const;
        /// @brief Position de la caméra dans l'espace.
        Vec3 Position3d() const;

        /// @brief Projette un point de l'espace sur l'écran ; vide s'il est derrière la caméra.
        std::optional<ScreenPoint> Project(Vec3 point, Viewport viewport) const;
        /// @brief La case du sol située sous un point de l'écran.
        std::optional<Position> PickCell(ScreenPoint screen, Viewport viewport) const;

    private:
        /// @brief Les trois axes de la caméra, de longueur 1.
        struct Basis {
            /// @brief Direction du regard.
            Vec3 forward;
            /// @brief Droite de l'image.
            Vec3 right;
            /// @brief Haut de l'image.
            Vec3 up;
        };
        /// @brief Calcule les axes de la caméra, qui regarde le centre de la carte.
        Basis Axes() const;
        /// @brief Distance focale en pixels, d'après le champ de vision et la hauteur de l'image.
        static double FocalLength(Viewport viewport);

        /// @brief Largeur de la carte, en cases.
        int mapWidth;
        /// @brief Profondeur de la carte, en cases.
        int mapHeight;
        /// @brief Angle autour de la carte, en radians.
        double yaw{0.0};
        /// @brief Inclinaison au-dessus du sol, en radians.
        double pitch{0.0};
        /// @brief Distance au centre de la carte.
        double distance{0.0};
    };
}
