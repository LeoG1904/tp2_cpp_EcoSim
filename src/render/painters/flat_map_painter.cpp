/**
 * @file
 * @brief Dessin de la carte 2D.
 */
module;

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

module ecosim.render;

import std;
import ecosim.animation;
import ecosim.species_style;
import ecosim.species;

namespace {
    /// @brief Couleur d'une case en 2D, selon le terrain et l'herbe.
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
}

namespace ecosim {
    void DrawFlatMap(Canvas& canvas, const Simulation& simulation, const double progress,
                     const std::optional<Position> selected, const int cellSize) {
        const World& world = simulation.GetWorld();
        std::vector<AnimalSnapshot> animals = simulation.Animals();
        AttachNewbornsToParents(animals);
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

        // Chaque animal est dessiné là où il se trouve : un corps, et une tête vers l'avant.
        const HGDIOBJ previousPen = SelectObject(buffer, GetStockObject(NULL_PEN));
        SetBkMode(buffer, TRANSPARENT);
        const auto disc = [buffer](const double x, const double y, const double radius) {
            Ellipse(buffer, static_cast<int>(std::lround(x - radius)),
                    static_cast<int>(std::lround(y - radius)),
                    static_cast<int>(std::lround(x + radius)) + 1,
                    static_cast<int>(std::lround(y + radius)) + 1);
        };
        for (const SpeciesInfo& species : SpeciesCatalog()) {
            const SpeciesStyle& style = StyleOf(species.kind);
            const HBRUSH body = CreateSolidBrush(RGB(style.color.red, style.color.green, style.color.blue));
            const HBRUSH head = CreateSolidBrush(RGB(style.color.red * 7 / 10, style.color.green * 7 / 10,
                                                     style.color.blue * 7 / 10));
            const double radius = cellSize * style.length * 0.72;
            const HGDIOBJ previousBrush = SelectObject(buffer, body);
            for (const AnimalSnapshot& animal : animals) {
                if (animal.kind != species.kind) continue;
                const Point point = PathPoint(animal.path, progress);
                const double angle = PathHeading(animal.path, progress);
                const double x = point.x * cellSize;
                const double y = point.y * cellSize;
                SelectObject(buffer, body);
                disc(x, y, radius);
                SelectObject(buffer, head);
                disc(x + std::cos(angle) * radius * 0.85, y + std::sin(angle) * radius * 0.85,
                     radius * 0.55);
            }
            SelectObject(buffer, previousBrush);
            DeleteObject(body);
            DeleteObject(head);
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
