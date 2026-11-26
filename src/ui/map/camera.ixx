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
        struct Basis {
            Vec3 forward;
            Vec3 right;
            Vec3 up;
        };
        Basis Axes() const;
        static double FocalLength(Viewport viewport);

        int mapWidth;
        int mapHeight;
        double yaw{0.0};
        double pitch{0.0};
        double distance{0.0};
    };
}
