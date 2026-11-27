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
    enum class Terrain {
        Prairie, ///< Accessible aux animaux ; l'herbe y pousse.
        Eau, ///< Inaccessible.
        Obstacle ///< Inaccessible.
    };

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
        /// @brief Nature de la case.
        Terrain terrain{Terrain::Prairie};
        /// @brief Quantité d'herbe, de 0 à MaxVegetation.
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
        /// @brief Vérifie qu'une dimension est strictement positive.
        /// @return La valeur reçue.
        /// @throws std::invalid_argument sinon.
        static int ValidDimension(int value);
        /// @brief Rang d'une case dans le tableau, rangé ligne par ligne.
        /// @throws std::out_of_range si la position est hors de la grille.
        std::size_t Index(Position position) const;

        /// @brief Nombre de colonnes.
        int width;
        /// @brief Nombre de lignes.
        int height;
        /// @brief Toutes les cases, ligne par ligne.
        std::vector<Cell> cells;
    };

    /// @brief Crée une carte cohérente à partir de deux reliefs de bruit tirés au hasard.
    ///
    /// L'altitude place l'eau dans les creux et les obstacles sur les hauteurs ; l'humidité fixe l'herbe de départ. Les prairies restantes sont toutes reliées entre elles ; si une carte est trop morcelée, d'autres reliefs sont essayés.
    /// @param width Nombre de colonnes.
    /// @param height Nombre de lignes.
    /// @param rng Générateur : la même graine redonne la même carte.
    /// @param waterPercent Part de la carte couverte d'eau, en %, avant de combler les prairies isolées.
    /// @param obstaclePercent Part de la carte couverte d'obstacles, en %, avant de combler les prairies isolées.
    /// @throws std::invalid_argument si les proportions sont négatives ou dépassent 100 au total.
    /// @ingroup moteur
    World GenerateRandomWorld(int width, int height, std::mt19937& rng,
                              int waterPercent = 10, int obstaclePercent = 10);
}
