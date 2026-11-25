// Transforme l'état de la simulation en faces 3D déjà projetées, sans API graphique.
export module ecosim.scene3d;

import std;
export import ecosim.camera;
export import ecosim.simulation;
export import ecosim.species_style;

export namespace ecosim {
    enum class FaceKind { Ground, Selection, Obstacle, Grass, Animal };

    // Une touffe d'herbe compte le plus de sommets.
    inline constexpr std::size_t MaxFacePoints{7};

    // Polygone à l'écran ; seuls les pointCount premiers sommets sont utilisés.
    struct Face {
        std::array<ScreenPoint, MaxFacePoints> points;
        std::size_t pointCount;
        Color color;
        FaceKind kind;
        std::optional<AnimalKind> species; // pour les faces d'un animal
    };

    // Au-delà, les animaux d'une même espèce sur une case ne sont plus dessinés.
    inline constexpr int MaxModelsPerCell{4};

    // Herbe d'une prairie : la hauteur et le nombre de touffes suivent la végétation.
    inline constexpr int MaxGrassTufts{5};
    // Assez basse pour laisser voir les animaux.
    inline constexpr double MaxGrassHeight{0.24};

    int GrassTuftCount(int vegetation);
    double GrassHeight(int vegetation);

    // Les faces sont rangées dans l'ordre de dessin : le sol, puis les volumes du plus loin au plus près.
    std::vector<Face> BuildScene(const World& world, const std::vector<AnimalSnapshot>& animals,
                                 double progress, const OrbitCamera& camera, Viewport viewport,
                                 std::optional<Position> selected);
}
