// Transforme l'état de la simulation en faces 3D déjà projetées, sans API graphique.
export module ecosim.scene3d;

import std;
export import ecosim.camera;
export import ecosim.simulation;
export import ecosim.species_style;

export namespace ecosim {
    enum class FaceKind { Ground, Selection, Obstacle, Animal };

    // Quadrilatère à l'écran.
    struct Face {
        std::array<ScreenPoint, 4> points;
        Color color;
        FaceKind kind;
        std::optional<AnimalKind> species; // pour les faces d'un animal
    };

    // Au-delà, les animaux d'une même espèce sur une case ne sont plus dessinés.
    inline constexpr int MaxModelsPerCell{4};

    // Les faces sont rangées dans l'ordre de dessin : le sol, puis les volumes du plus loin au plus près.
    std::vector<Face> BuildScene(const World& world, const std::vector<AnimalSnapshot>& animals,
                                 double progress, const OrbitCamera& camera, Viewport viewport,
                                 std::optional<Position> selected);
}
