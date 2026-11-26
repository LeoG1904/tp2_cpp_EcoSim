module ecosim.world;

import std;

namespace ecosim {
    Cell::Cell(const Terrain terrain, const int vegetation)
        : terrain{terrain}, vegetation{std::clamp(vegetation, 0, MaxVegetation)} {}

    Terrain Cell::GetTerrain() const { return terrain; }
    int Cell::GetVegetation() const { return vegetation; }
    void Cell::SetVegetation(const int value) { vegetation = std::clamp(value, 0, MaxVegetation); }
    bool Cell::IsAccessible() const { return terrain == Terrain::Prairie; }

    World::World(const int width, const int height)
        : width{ValidDimension(width)},
          height{ValidDimension(height)},
          cells(static_cast<std::size_t>(width) * static_cast<std::size_t>(height)) {}

    int World::GetWidth() const { return width; }
    int World::GetHeight() const { return height; }

    bool World::IsInside(const Position position) const {
        return position.x >= 0 && position.x < width && position.y >= 0 && position.y < height;
    }

    bool World::IsWalkable(const Point point) const {
        const Position cell = CellAt(point);
        return IsInside(cell) && GetCell(cell).IsAccessible();
    }

    Cell& World::GetCell(const Position position) { return cells[Index(position)]; }
    const Cell& World::GetCell(const Position position) const { return cells[Index(position)]; }

    std::vector<Position> World::Neighbors(const Position position) const {
        constexpr std::array<Position, 4> offsets{{{0, -1}, {0, 1}, {-1, 0}, {1, 0}}};
        std::vector<Position> result;
        for (const auto offset : offsets) {
            const Position neighbor{position.x + offset.x, position.y + offset.y};
            if (IsInside(neighbor)) result.push_back(neighbor);
        }
        return result;
    }

    int World::ValidDimension(const int value) {
        if (value <= 0) throw std::invalid_argument{"Les dimensions du monde doivent être strictement positives"};
        return value;
    }

    std::size_t World::Index(const Position position) const {
        if (!IsInside(position)) throw std::out_of_range{"Position hors de la grille"};
        return static_cast<std::size_t>(position.y) * static_cast<std::size_t>(width)
             + static_cast<std::size_t>(position.x);
    }

    World GenerateRandomWorld(const int width, const int height, std::mt19937& rng,
                              const int waterPercent, const int obstaclePercent) {
        if (waterPercent < 0 || obstaclePercent < 0 || waterPercent + obstaclePercent > 100) {
            throw std::invalid_argument{"Proportions de terrain invalides"};
        }
        World world{width, height};
        std::uniform_int_distribution<int> percent{1, 100};
        std::uniform_int_distribution<int> vegetation{0, Cell::MaxVegetation};

        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                const int roll = percent(rng);
                Terrain terrain{Terrain::Prairie};
                if (roll <= waterPercent) terrain = Terrain::Eau;
                else if (roll <= waterPercent + obstaclePercent) terrain = Terrain::Obstacle;
                world.GetCell(Position{x, y}) = Cell{terrain, vegetation(rng)};
            }
        }
        return world;
    }
}
