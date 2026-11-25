module;

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

module ecosim.render;

import std;
import ecosim.animation;
import ecosim.species_style;

namespace {
    COLORREF ColorOf(const ecosim::Cell& cell) {
        switch (cell.GetTerrain()) {
            case ecosim::Terrain::Eau: return RGB(60, 120, 200);
            case ecosim::Terrain::Obstacle: return RGB(90, 90, 90);
            case ecosim::Terrain::Prairie: break;
        }
        const double ratio = static_cast<double>(cell.GetVegetation()) / ecosim::Cell::MaxVegetation;
        return RGB(static_cast<int>(150 - ratio * 90),
                   static_cast<int>(110 + ratio * 90), 70);
    }

    struct Marker {
        int x;
        int y;
        int radius;
        ecosim::AnimalKind kind;
        int count;
    };

    // Un rond par groupe d'animaux, placé entre l'ancienne et la nouvelle case.
    std::vector<Marker> AnimalMarkers(const std::vector<ecosim::AnimalSnapshot>& animals,
                                      const ecosim::World& world, const int cellSize,
                                      const double progress) {
        const auto indexOf = [&](const ecosim::Position position) {
            return static_cast<std::size_t>(position.y) * world.GetWidth()
                 + static_cast<std::size_t>(position.x);
        };
        const std::size_t cellCount = static_cast<std::size_t>(world.GetWidth()) * world.GetHeight();
        // Un bit par espèce présente, au départ et à l'arrivée : elles se partagent la case.
        std::vector<std::uint32_t> before(cellCount);
        std::vector<std::uint32_t> after(cellCount);
        const auto bit = [](const ecosim::AnimalKind kind) {
            return std::uint32_t{1} << static_cast<unsigned>(kind);
        };
        for (const auto& animal : animals) {
            before[indexOf(animal.previousPosition)] |= bit(animal.kind);
            after[indexOf(animal.position)] |= bit(animal.kind);
        }

        const auto anchor = [&](const ecosim::Position position, const ecosim::AnimalKind kind,
                                const std::vector<std::uint32_t>& occupancy) {
            const std::uint32_t present = occupancy[indexOf(position)];
            const int sharing = std::popcount(present);
            const int rank = std::popcount(present & (bit(kind) - 1));
            const int offset = cellSize * (2 * rank + 1) / (2 * sharing);
            return Marker{position.x * cellSize + offset, position.y * cellSize + cellSize / 2,
                          sharing > 1 ? cellSize * 2 / (5 * sharing) : cellSize * 36 / 100, kind, 1};
        };
        const auto between = [&](const int from, const int to) {
            return from + static_cast<int>(std::lround((to - from) * progress));
        };

        // Les animaux qui font le même trajet avancent ensemble.
        std::map<std::tuple<std::size_t, std::size_t, ecosim::AnimalKind>, Marker> trips;
        for (const auto& animal : animals) {
            const auto key = std::tuple{indexOf(animal.previousPosition), indexOf(animal.position),
                                        animal.kind};
            if (const auto found = trips.find(key); found != trips.end()) {
                ++found->second.count;
                continue;
            }
            const Marker from = anchor(animal.previousPosition, animal.kind, before);
            const Marker to = anchor(animal.position, animal.kind, after);
            trips.emplace(key, Marker{between(from.x, to.x), between(from.y, to.y),
                                      between(from.radius, to.radius), animal.kind, 1});
        }

        // Des ronds presque superposés n'en forment qu'un, pour garder un compte lisible.
        std::vector<Marker> markers;
        std::vector<std::vector<std::size_t>> nearby(cellCount);
        for (const auto& [key, trip] : trips) {
            const int cellX = std::clamp(trip.x / cellSize, 0, world.GetWidth() - 1);
            const int cellY = std::clamp(trip.y / cellSize, 0, world.GetHeight() - 1);
            Marker* match = nullptr;
            for (int y = std::max(0, cellY - 1); y <= std::min(world.GetHeight() - 1, cellY + 1); ++y) {
                for (int x = std::max(0, cellX - 1); x <= std::min(world.GetWidth() - 1, cellX + 1); ++x) {
                    for (const std::size_t i : nearby[indexOf({x, y})]) {
                        Marker& other = markers[i];
                        if (!match && other.kind == trip.kind
                            && std::abs(other.x - trip.x) <= trip.radius / 2
                            && std::abs(other.y - trip.y) <= trip.radius / 2) {
                            match = &other;
                        }
                    }
                }
            }
            if (match) {
                match->count += trip.count;
            } else {
                nearby[indexOf({cellX, cellY})].push_back(markers.size());
                markers.push_back(trip);
            }
        }
        return markers;
    }
}

namespace ecosim {
    FlatMapPainter::FlatMapPainter(const int cellSize) : cellSize{cellSize} {}

    void FlatMapPainter::Draw(Canvas& canvas, const Simulation& simulation, const double progress,
                              const std::optional<Position> selected) const {
        const World& world = simulation.GetWorld();
        std::vector<AnimalSnapshot> animals = simulation.Animals();
        AttachNewbornsToParents(animals);
        const std::vector<Marker> markers = AnimalMarkers(animals, world, cellSize, progress);
        const HDC buffer = static_cast<HDC>(canvas.Context());
        const int windowWidth = canvas.Width();
        const int windowHeight = canvas.Height();
        const RECT bounds{0, 0, windowWidth, windowHeight};
        const HBRUSH background = CreateSolidBrush(RGB(30, 30, 30));
        FillRect(buffer, &bounds, background);
        DeleteObject(background);

        for (int y = 0; y < world.GetHeight(); ++y) {
            for (int x = 0; x < world.GetWidth(); ++x) {
                const HBRUSH brush = CreateSolidBrush(ColorOf(world.GetCell(Position{x, y})));
                const RECT cell{x * cellSize, y * cellSize,
                                (x + 1) * cellSize - 1, (y + 1) * cellSize - 1};
                FillRect(buffer, &cell, brush);
                DeleteObject(brush);
            }
        }

        const HGDIOBJ previousPen = SelectObject(buffer, GetStockObject(NULL_PEN));
        SetBkMode(buffer, TRANSPARENT);
        for (const Marker& marker : markers) {
            const Color color = StyleOf(marker.kind).color;
            const HBRUSH brush = CreateSolidBrush(RGB(color.red, color.green, color.blue));
            const HGDIOBJ previousBrush = SelectObject(buffer, brush);
            Ellipse(buffer, marker.x - marker.radius, marker.y - marker.radius,
                    marker.x + marker.radius, marker.y + marker.radius);
            SelectObject(buffer, previousBrush);
            DeleteObject(brush);
            if (marker.count > 1) {
                const std::string label = std::to_string(marker.count);
                const TextSize extent = MeasureText(buffer, label);
                SetTextColor(buffer, RGB(30, 30, 30));
                WriteText(buffer, marker.x - extent.cx / 2, marker.y - extent.cy / 2, label);
            }
        }
        SelectObject(buffer, previousPen);

        // Cadre autour de la case choisie.
        if (selected) {
            RECT frame{selected->x * cellSize, selected->y * cellSize,
                       (selected->x + 1) * cellSize - 1, (selected->y + 1) * cellSize - 1};
            const HBRUSH frameBrush = CreateSolidBrush(RGB(255, 255, 255));
            for (int thickness = 0; thickness < 3; ++thickness) {
                FrameRect(buffer, &frame, frameBrush);
                InflateRect(&frame, -1, -1);
            }
            DeleteObject(frameBrush);
        }
    }
}
