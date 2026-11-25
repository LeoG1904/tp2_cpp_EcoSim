// Données du monde et création de la carte, sans dépendance graphique.
export module ecosim.world;

import std;

export namespace ecosim {
    struct Position {
        int x{0};
        int y{0};

        bool operator==(const Position&) const = default;
    };

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
    World GenerateRandomWorld(int width, int height, std::mt19937& rng);
}
