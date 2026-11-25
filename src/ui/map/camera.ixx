// Caméra 3D qui tourne autour de la carte, sans API graphique.
export module ecosim.camera;

import std;
export import ecosim.world;

export namespace ecosim {
    // La case (x, y) occupe le carré [x, x+1] x [y, y+1] du sol ; y3d est la hauteur.
    struct Vec3 {
        double x;
        double y;
        double z;
    };

    struct ScreenPoint {
        int x;
        int y;
    };

    struct Viewport {
        int width;
        int height;
    };

    class OrbitCamera {
    public:
        static constexpr double MinPitch{0.30};  // presque au ras du sol
        static constexpr double MaxPitch{1.45};  // presque à la verticale

        OrbitCamera(int mapWidth, int mapHeight);

        // Revient à la vue d'ensemble.
        void Reset();
        // Angles en radians : autour de la carte, puis au-dessus du sol.
        void Rotate(double yawDelta, double pitchDelta);
        // Un cran positif rapproche la caméra.
        void Zoom(int notches);

        double Yaw() const;
        double Pitch() const;
        double Distance() const;
        Vec3 Position3d() const;

        // Position à l'écran ; vide si le point est derrière la caméra.
        std::optional<ScreenPoint> Project(Vec3 point, Viewport viewport) const;
        // Case du sol située sous un point de l'écran.
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
