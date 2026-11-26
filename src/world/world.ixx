// Données du monde et création de la carte, sans dépendance graphique.
export module ecosim.world;

import std;

export namespace ecosim {
    struct Position {
        int x{0};
        int y{0};

        bool operator==(const Position&) const = default;
    };

    // Point libre sur la carte, en cases : la case (x, y) couvre [x, x+1[ x [y, y+1[.
    struct Point {
        double x{0.0};
        double y{0.0};

        bool operator==(const Point&) const = default;
    };

    // Case située sous un point.
    inline Position CellAt(const Point point) {
        return {static_cast<int>(std::floor(point.x)), static_cast<int>(std::floor(point.y))};
    }
    inline Point CenterOf(const Position cell) { return {cell.x + 0.5, cell.y + 0.5}; }
    inline double Distance(const Point a, const Point b) { return std::hypot(a.x - b.x, a.y - b.y); }

    enum class Terrain { Prairie, Eau, Obstacle };

    class Cell {
    public:
        static constexpr int MaxVegetation{100};

        explicit Cell(Terrain terrain = Terrain::Prairie, int vegetation = 0);

        Terrain GetTerrain() const;
        int GetVegetation() const;
        void SetVegetation(int value);
        bool IsAccessible() const;

    private:
        Terrain terrain{Terrain::Prairie};
        int vegetation{0};
    };

    class World {
    public:
        World(int width, int height);

        int GetWidth() const;
        int GetHeight() const;
        bool IsInside(Position position) const;
        // Un animal peut se tenir en ce point : dans la carte et sur une prairie.
        bool IsWalkable(Point point) const;
        Cell& GetCell(Position position);
        const Cell& GetCell(Position position) const;
        std::vector<Position> Neighbors(Position position) const;

    private:
        static int ValidDimension(int value);
        std::size_t Index(Position position) const;

        int width;
        int height;
        std::vector<Cell> cells;
    };

    // Le générateur appartient au monde ; la graine reste choisie par l'appelant.
    World GenerateRandomWorld(int width, int height, std::mt19937& rng,
                              int waterPercent = 10, int obstaclePercent = 10);
}
