/**
 * @file
 * @brief Le monde : positions, cases, grille et carte aléatoire.
 */
export module ecosim.world;

import std;

export namespace ecosim {
    /// @brief Une case de la grille, repérée par sa colonne et sa ligne.
    /// @ingroup moteur
    struct Position {
        /// @brief Colonne, à partir de 0.
        int x{0};
        /// @brief Ligne, à partir de 0.
        int y{0};

        /// @brief Vrai si les deux valeurs sont identiques.
        bool operator==(const Position&) const = default;
    };

    /// @brief Un point libre sur la carte, mesuré en cases.
    ///
    /// La case (x, y) couvre l'intervalle [x, x+1[ sur [y, y+1[.
    /// @ingroup moteur
    struct Point {
        /// @brief Abscisse, en cases.
        double x{0.0};
        /// @brief Ordonnée, en cases, vers le bas de l'écran en 2D.
        double y{0.0};

        /// @brief Vrai si les deux valeurs sont identiques.
        bool operator==(const Point&) const = default;
    };

    /// @brief La case située sous un point.
    /// @ingroup moteur
    inline Position CellAt(const Point point) {
        return {static_cast<int>(std::floor(point.x)), static_cast<int>(std::floor(point.y))};
    }
    /// @brief Le centre d'une case.
    /// @ingroup moteur
    inline Point CenterOf(const Position cell) { return {cell.x + 0.5, cell.y + 0.5}; }
    /// @brief La distance en ligne droite entre deux points, en cases.
    /// @ingroup moteur
    inline double Distance(const Point a, const Point b) { return std::hypot(a.x - b.x, a.y - b.y); }

    /// @brief La nature d'une case. Seule la prairie est accessible aux animaux.
    /// @ingroup moteur
    enum class Terrain { Prairie, Eau, Obstacle };

    /// @brief Une case : son terrain et sa quantité d'herbe.
    /// @ingroup moteur
    class Cell {
    public:
        /// @brief Quantité d'herbe maximale d'une case.
        static constexpr int MaxVegetation{100};

        /// @brief Crée une case.
        /// @param terrain Nature de la case.
        /// @param vegetation Herbe de départ, ramenée entre 0 et MaxVegetation.
        explicit Cell(Terrain terrain = Terrain::Prairie, int vegetation = 0);

        /// @brief La nature de la case.
        Terrain GetTerrain() const;
        /// @brief La quantité d'herbe.
        int GetVegetation() const;
        /// @brief Change la quantité d'herbe, ramenée entre 0 et MaxVegetation.
        void SetVegetation(int value);
        /// @brief Vrai pour une prairie.
        bool IsAccessible() const;

    private:
        Terrain terrain{Terrain::Prairie};
        int vegetation{0};
    };

    /// @brief La carte : une grille rectangulaire de cases.
    ///
    /// Elle ne contient pas les animaux : c'est le rôle de Simulation.
    /// @ingroup moteur
    class World {
    public:
        /// @brief Crée une carte de prairies sans herbe.
        /// @throws std::invalid_argument si une dimension n'est pas strictement positive.
        World(int width, int height);

        /// @brief Nombre de colonnes.
        int GetWidth() const;
        /// @brief Nombre de lignes.
        int GetHeight() const;
        /// @brief Vrai si la case existe sur la carte.
        bool IsInside(Position position) const;
        /// @brief Vrai si un animal peut se tenir en ce point : dans la carte et sur une prairie.
        bool IsWalkable(Point point) const;
        /// @brief Accède à une case pour la modifier.
        /// @throws std::out_of_range si la position est hors de la grille.
        Cell& GetCell(Position position);
        /// @brief Lit une case.
        /// @throws std::out_of_range si la position est hors de la grille.
        const Cell& GetCell(Position position) const;
        /// @brief Les cases voisines (haut, bas, gauche, droite) situées dans la carte.
        std::vector<Position> Neighbors(Position position) const;

    private:
        static int ValidDimension(int value);
        std::size_t Index(Position position) const;

        int width;
        int height;
        std::vector<Cell> cells;
    };

    /// @brief Crée une carte au hasard.
    /// @param width Nombre de colonnes.
    /// @param height Nombre de lignes.
    /// @param rng Générateur : la même graine redonne la même carte.
    /// @param waterPercent Chance, en %, qu'une case soit de l'eau.
    /// @param obstaclePercent Chance, en %, qu'une case soit un obstacle.
    /// @throws std::invalid_argument si les proportions sont négatives ou dépassent 100 au total.
    /// @ingroup moteur
    World GenerateRandomWorld(int width, int height, std::mt19937& rng,
                              int waterPercent = 10, int obstaclePercent = 10);
}
