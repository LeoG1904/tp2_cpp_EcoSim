/**
 * @file
 * @brief La construction de la scène 3D.
 */
export module ecosim.scene3d;

import std;
export import ecosim.camera;
export import ecosim.simulation;
export import ecosim.species_style;

export namespace ecosim {
    /// @brief Ce que représente un polygone de la scène.
    /// @ingroup interface
    enum class FaceKind {
        Ground, ///< Sol d'une case.
        Cliff, ///< Flanc d'une case plus haute que sa voisine.
        Selection, ///< Cadre de la case choisie.
        Obstacle, ///< Bloc d'un obstacle.
        Grass, ///< Touffe d'herbe.
        Animal ///< Partie d'un animal.
    };

    /// @brief Nombre maximal de sommets d'un polygone (une touffe d'herbe).
    /// @ingroup interface
    inline constexpr std::size_t MaxFacePoints{7};

    /// @brief Un polygone déjà projeté sur l'écran.
    /// @ingroup interface
    struct Face {
        /// @brief Sommets ; seuls les pointCount premiers servent.
        std::array<ScreenPoint, MaxFacePoints> points;
        /// @brief Nombre de sommets.
        std::size_t pointCount;
        /// @brief Couleur.
        Color color;
        /// @brief Nature de l'élément.
        FaceKind kind;
        /// @brief Espèce, pour un animal.
        std::optional<AnimalKind> species;
    };

    /// @brief Nombre maximal de touffes d'herbe par case.
    /// @ingroup interface
    inline constexpr int MaxGrassTufts{5};
    /// @brief Hauteur maximale de l'herbe, assez basse pour laisser voir les animaux.
    /// @ingroup interface
    inline constexpr double MaxGrassHeight{0.24};

    /// @brief Nombre de touffes pour une quantité d'herbe.
    /// @ingroup interface
    int GrassTuftCount(int vegetation);
    /// @brief Hauteur de l'herbe pour une quantité donnée.
    /// @ingroup interface
    double GrassHeight(int vegetation);

    /// @brief Transforme la simulation en polygones.
    ///
    /// Ils sont rangés dans l'ordre de dessin. Sur une carte plate : le sol, puis les volumes du plus loin au plus près. Avec du relief, les cases sont dessinées de la plus lointaine à la plus proche, chacune suivie de ce qu'elle porte : une hauteur cache ce qui est derrière elle.
    /// @ingroup interface
    std::vector<Face> BuildScene(const World& world, const std::vector<AnimalSnapshot>& animals,
                                 double progress, const OrbitCamera& camera, Viewport viewport,
                                 std::optional<Position> selected);
}
