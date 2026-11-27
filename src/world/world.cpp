/**
 * @file
 * @brief Implémentation du monde et de la génération de carte par bruit.
 */
module ecosim.world;

import std;

namespace {
    /// @brief Écart, en cases, entre deux valeurs tirées au sort pour les grandes formes.
    constexpr int ReliefSpacing{8};
    /// @brief Écart, en cases, pour les détails qui rendent les contours irréguliers.
    constexpr int DetailSpacing{4};
    /// @brief Poids des détails par rapport aux grandes formes.
    constexpr double DetailWeight{0.5};
    /// @brief Part des prairies que la plus grande zone doit réunir pour que la carte soit acceptée.
    constexpr double ConnectedShare{0.95};
    /// @brief Nombre de reliefs essayés avant de garder le moins morcelé.
    constexpr int MaxAttempts{20};

    /// @brief Courbe douce de 0 à 1, plate aux extrémités : elle évite les cassures entre deux valeurs.
    double Smooth(const double t) { return t * t * (3.0 - 2.0 * t); }

    /// @brief Bruit de valeur : des valeurs au sort sur une grille espacée, lissées entre elles.
    /// @return Une valeur par case, ligne par ligne.
    std::vector<double> ValueNoise(const int width, const int height, const int spacing,
                                   std::mt19937& rng) {
        const int columns = width / spacing + 2;
        const int rows = height / spacing + 2;
        std::uniform_real_distribution<double> draw{0.0, 1.0};
        std::vector<double> corners(static_cast<std::size_t>(columns) * rows);
        for (double& corner : corners) corner = draw(rng);
        const auto corner = [&](const int x, const int y) {
            return corners[static_cast<std::size_t>(y) * columns + x];
        };

        std::vector<double> noise;
        noise.reserve(static_cast<std::size_t>(width) * height);
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                const int left = x / spacing;
                const int top = y / spacing;
                const double across = Smooth(static_cast<double>(x % spacing) / spacing);
                const double down = Smooth(static_cast<double>(y % spacing) / spacing);
                const double above = std::lerp(corner(left, top), corner(left + 1, top), across);
                const double below = std::lerp(corner(left, top + 1), corner(left + 1, top + 1), across);
                noise.push_back(std::lerp(above, below, down));
            }
        }
        return noise;
    }

    /// @brief Un relief : de grandes formes, plus des détails.
    std::vector<double> Landscape(const int width, const int height, std::mt19937& rng) {
        std::vector<double> result = ValueNoise(width, height, ReliefSpacing, rng);
        const std::vector<double> details = ValueNoise(width, height, DetailSpacing, rng);
        for (std::size_t i = 0; i < result.size(); ++i) result[i] += DetailWeight * details[i];
        return result;
    }

    /// @brief Les rangs des cases, de la valeur la plus basse à la plus haute.
    std::vector<std::size_t> Ascending(const std::vector<double>& values) {
        std::vector<std::size_t> order(values.size());
        std::iota(order.begin(), order.end(), std::size_t{0});
        std::ranges::stable_sort(order, {}, [&values](const std::size_t i) { return values[i]; });
        return order;
    }

    /// @brief Les groupes de prairies reliées par un côté, chacun sous forme de liste de cases.
    std::vector<std::vector<ecosim::Position>> PrairieRegions(const ecosim::World& world) {
        const int width = world.GetWidth();
        std::vector<bool> seen(static_cast<std::size_t>(width) * world.GetHeight(), false);
        const auto visit = [&](const ecosim::Position cell, std::vector<ecosim::Position>& region) {
            const auto index = static_cast<std::size_t>(cell.y) * width + cell.x;
            if (seen[index] || !world.GetCell(cell).IsAccessible()) return;
            seen[index] = true;
            region.push_back(cell);
        };

        std::vector<std::vector<ecosim::Position>> regions;
        for (int y = 0; y < world.GetHeight(); ++y) {
            for (int x = 0; x < width; ++x) {
                std::vector<ecosim::Position> region;
                visit({x, y}, region);
                // La liste s'allonge pendant son parcours : elle sert aussi de file d'attente.
                for (std::size_t next = 0; next < region.size(); ++next) {
                    for (const ecosim::Position neighbor : world.Neighbors(region[next])) {
                        visit(neighbor, region);
                    }
                }
                if (!region.empty()) regions.push_back(std::move(region));
            }
        }
        return regions;
    }

    /// @brief La part des prairies réunie dans la plus grande zone ; 1 sans prairie.
    double LargestShare(const ecosim::World& world) {
        std::size_t largest = 0;
        std::size_t total = 0;
        for (const auto& region : PrairieRegions(world)) {
            largest = std::max(largest, region.size());
            total += region.size();
        }
        return total == 0 ? 1.0 : static_cast<double>(largest) / static_cast<double>(total);
    }

    /// @brief Ne garde que la plus grande zone de prairies : aucun animal ne peut naître enfermé.
    /// @param world Carte à corriger.
    /// @param altitude Altitude de chaque case : une poche basse devient de l'eau, une poche haute un obstacle.
    /// @param waterPercent Part d'eau demandée.
    /// @param obstaclePercent Part d'obstacles demandée.
    void FillIsolatedPrairies(ecosim::World& world, const std::vector<double>& altitude,
                              const int waterPercent, const int obstaclePercent) {
        std::vector<std::vector<ecosim::Position>> regions = PrairieRegions(world);
        if (regions.size() < 2) return;
        const auto largest = std::ranges::max_element(regions, {}, [](const auto& region) {
            return region.size();
        });
        std::vector<double> sorted = altitude;
        std::ranges::sort(sorted);
        const double median = sorted[sorted.size() / 2];
        for (auto region = regions.begin(); region != regions.end(); ++region) {
            if (region == largest) continue;
            for (const ecosim::Position cell : *region) {
                bool low = altitude[static_cast<std::size_t>(cell.y) * world.GetWidth() + cell.x] < median;
                // Un terrain que l'utilisateur a mis à 0 % n'est jamais ajouté.
                if (waterPercent == 0) low = false;
                if (obstaclePercent == 0) low = true;
                world.GetCell(cell) = ecosim::Cell{low ? ecosim::Terrain::Eau : ecosim::Terrain::Obstacle};
            }
        }
    }

    /// @brief Une carte proposée, avant de combler ses prairies isolées.
    struct Draft {
        /// @brief La carte.
        ecosim::World world;
        /// @brief Altitude de chaque case, ligne par ligne.
        std::vector<double> altitude;
        /// @brief Part des prairies réunie dans la plus grande zone.
        double connectedShare;
    };

    /// @brief Tire deux reliefs et en déduit le terrain et l'herbe de chaque case.
    Draft Sculpt(const int width, const int height, std::mt19937& rng, const int waterPercent,
                 const int obstaclePercent) {
        using ecosim::Cell;
        using ecosim::Position;
        using ecosim::Terrain;
        ecosim::World world{width, height};
        std::vector<double> altitude = Landscape(width, height, rng);
        const std::vector<double> humidity = Landscape(width, height, rng);
        const std::size_t count = altitude.size();
        const auto cellAt = [width](const std::size_t index) {
            return Position{static_cast<int>(index % width), static_cast<int>(index / width)};
        };

        // L'herbe de départ suit l'humidité. Le classement répartit toutes les quantités à parts égales.
        const std::vector<std::size_t> dryToWet = Ascending(humidity);
        for (std::size_t rank = 0; rank < count; ++rank) {
            const int vegetation = count < 2 ? Cell::MaxVegetation / 2
                : static_cast<int>(rank * Cell::MaxVegetation / (count - 1));
            world.GetCell(cellAt(dryToWet[rank])) = Cell{Terrain::Prairie, vegetation};
        }
        // Les cases les plus basses sont noyées, les plus hautes sont des obstacles.
        const std::vector<std::size_t> lowToHigh = Ascending(altitude);
        const std::size_t water = (count * waterPercent + 50) / 100;
        const std::size_t obstacles = std::min(count - water, (count * obstaclePercent + 50) / 100);
        for (std::size_t i = 0; i < water; ++i) {
            world.GetCell(cellAt(lowToHigh[i])) = Cell{Terrain::Eau};
        }
        for (std::size_t i = count - obstacles; i < count; ++i) {
            world.GetCell(cellAt(lowToHigh[i])) = Cell{Terrain::Obstacle};
        }
        const double share = LargestShare(world);
        return {std::move(world), std::move(altitude), share};
    }
}

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
        std::optional<Draft> best;
        for (int attempt = 0; attempt < MaxAttempts; ++attempt) {
            Draft draft = Sculpt(width, height, rng, waterPercent, obstaclePercent);
            if (!best || draft.connectedShare > best->connectedShare) best = std::move(draft);
            if (best->connectedShare >= ConnectedShare) break;
        }
        FillIsolatedPrairies(best->world, best->altitude, waterPercent, obstaclePercent);
        return std::move(best->world);
    }
}
