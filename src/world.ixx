// Grille du monde : terrain et végétation de chaque cellule.
// Ce module ne connaît ni les animaux ni l'affichage.
export module ecosim.world;

import std;

export namespace ecosim {

    // Position d'une cellule dans la grille (x = colonne, y = ligne)
    struct Position {
        int x{0};
        int y{0};

        bool operator==(const Position&) const = default;
    };

    // Type de terrain : détermine si la végétation pousse et si l'on peut y circuler
    enum class Terrain { Prairie, Eau, Obstacle };

    // Une zone géographique de la grille
    class Cell {
    public:
        // Végétation maximale d'une cellule (sur 100, comme dans le sujet)
        static constexpr int MaxVegetation{100};

        explicit Cell(const Terrain terrain = Terrain::Prairie, const int vegetation = 0)
            : terrain{terrain}, vegetation{std::clamp(vegetation, 0, MaxVegetation)} {}

        Terrain GetTerrain() const { return terrain; }

        int GetVegetation() const { return vegetation; }

        // La valeur est ramenée dans [0, MaxVegetation]
        void SetVegetation(const int value) { vegetation = std::clamp(value, 0, MaxVegetation); }

        // Seule une prairie peut être traversée par un animal
        bool IsAccessible() const { return terrain == Terrain::Prairie; }

    private:
        Terrain terrain{Terrain::Prairie};
        int vegetation{0};
    };

    // La grille complète : width x height cellules
    class World {
    public:
        // Crée une grille de cellules de prairie sans végétation
        World(const int width, const int height)
            : width{ValidDimension(width)},
              height{ValidDimension(height)},
              cells(static_cast<std::size_t>(width) * static_cast<std::size_t>(height)) {}

        int GetWidth() const { return width; }

        int GetHeight() const { return height; }

        // Vrai si la position désigne une cellule de la grille
        bool IsInside(const Position position) const {
            return position.x >= 0 && position.x < width && position.y >= 0 && position.y < height;
        }

        // Lève std::out_of_range si la position est hors de la grille
        Cell& GetCell(const Position position) { return cells[Index(position)]; }

        const Cell& GetCell(const Position position) const { return cells[Index(position)]; }

        // Voisins directs (haut, bas, gauche, droite) qui sont dans la grille
        std::vector<Position> Neighbors(const Position position) const {
            constexpr std::array<Position, 4> offsets{{{0, -1}, {0, 1}, {-1, 0}, {1, 0}}};
            std::vector<Position> result;
            for (const auto offset : offsets) {
                const Position neighbor{position.x + offset.x, position.y + offset.y};
                if (IsInside(neighbor)) {
                    result.push_back(neighbor);
                }
            }
            return result;
        }

    private:
        static int ValidDimension(const int value) {
            if (value <= 0) {
                throw std::invalid_argument{"Les dimensions du monde doivent être strictement positives"};
            }
            return value;
        }

        // Convertit une position en indice dans le tableau à plat (ligne par ligne)
        std::size_t Index(const Position position) const {
            if (!IsInside(position)) {
                throw std::out_of_range{"Position hors de la grille"};
            }
            return static_cast<std::size_t>(position.y) * static_cast<std::size_t>(width)
                 + static_cast<std::size_t>(position.x);
        }

        int width;
        int height;
        std::vector<Cell> cells;
    };

}
