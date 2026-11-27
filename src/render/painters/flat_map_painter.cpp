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
import ecosim.layout;
import ecosim.flat_view;
import ecosim.species_style;
import ecosim.species;

namespace {
    /// @brief Couleur d'une case en 2D, selon le terrain et l'herbe.
    ecosim::Color ColorOf(const ecosim::Cell& cell) {
        switch (cell.GetTerrain()) {
            case ecosim::Terrain::Eau: return {60, 120, 200};
            case ecosim::Terrain::Obstacle: return {90, 90, 90};
            case ecosim::Terrain::Prairie: break;
        }
        const double ratio = static_cast<double>(cell.GetVegetation()) / ecosim::Cell::MaxVegetation;
        return {static_cast<int>(150 - ratio * 90), static_cast<int>(110 + ratio * 90), 70};
    }

    /// @brief En dessous de cette taille de case, en pixels, le sol est dessiné pixel par pixel.
    constexpr double SmallestDrawnCell{4.0};

    /// @brief Dessine les cases visibles.
    void DrawGround(ecosim::Canvas& canvas, const ecosim::World& world, const ecosim::FlatView& view) {
        const ecosim::CellRange visible = view.VisibleCells();
        const auto edge = [](const double value) { return static_cast<int>(std::floor(value)); };
        if (view.CellSize() >= SmallestDrawnCell) {
            for (int y = visible.top; y < visible.bottom; ++y) {
                const int top = edge(view.ScreenY(y));
                const int tall = edge(view.ScreenY(y + 1)) - top;
                for (int x = visible.left; x < visible.right; ++x) {
                    const int left = edge(view.ScreenX(x));
                    // Un pixel de retrait dessine le quadrillage.
                    canvas.Fill({left, top, edge(view.ScreenX(x + 1)) - left - 1, tall - 1},
                                ColorOf(world.GetCell(ecosim::Position{x, y})));
                }
            }
            return;
        }
        // De loin, il y a plus de cases que de pixels utiles : chaque pixel montre la case située dessous.
        const int left = std::max(0, edge(view.ScreenX(visible.left)));
        const int top = std::max(0, edge(view.ScreenY(visible.top)));
        const int right = std::min(canvas.Width(), edge(view.ScreenX(visible.right)));
        const int bottom = std::min(canvas.Height(), edge(view.ScreenY(visible.bottom)));
        const int wide = right - left;
        const int tall = bottom - top;
        if (wide <= 0 || tall <= 0) return;
        std::vector<std::uint32_t> pixels(static_cast<std::size_t>(wide) * tall, 0x1E1E1E);
        for (int y = 0; y < tall; ++y) {
            for (int x = 0; x < wide; ++x) {
                const auto cell = view.PickCell({left + x, top + y});
                if (!cell) continue;
                const ecosim::Color color = ColorOf(world.GetCell(*cell));
                pixels[static_cast<std::size_t>(y) * wide + x] =
                    static_cast<std::uint32_t>(color.red << 16 | color.green << 8 | color.blue);
            }
        }
        BITMAPINFO info{};
        info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        info.bmiHeader.biWidth = wide;
        // Une hauteur négative range les lignes de haut en bas.
        info.bmiHeader.biHeight = -tall;
        info.bmiHeader.biPlanes = 1;
        info.bmiHeader.biBitCount = 32;
        info.bmiHeader.biCompression = BI_RGB;
        SetDIBitsToDevice(static_cast<HDC>(canvas.Context()), left, top, wide, tall, 0, 0, 0, tall,
                          pixels.data(), &info, DIB_RGB_COLORS);
    }
}

namespace ecosim {
    void DrawFlatMap(Canvas& canvas, const SimulationSnapshot& simulation, const double progress,
                     const std::optional<Position> selected, const FlatView& view) {
        const World& world = simulation.world;
        // Copie modifiable : les nouveau-nés sont rattachés au trajet de leur parent.
        std::vector<AnimalSnapshot> animals = simulation.animals;
        AttachNewbornsToParents(animals);
        const HDC buffer = static_cast<HDC>(canvas.Context());
        canvas.Fill({0, 0, canvas.Width(), canvas.Height()}, {30, 30, 30});
        DrawGround(canvas, world, view);

        // Chaque animal est dessiné là où il se trouve : un corps, et une tête vers l'avant.
        const double cellSize = view.CellSize();
        const HGDIOBJ previousPen = SelectObject(buffer, GetStockObject(NULL_PEN));
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
                const double x = view.ScreenX(point.x);
                const double y = view.ScreenY(point.y);
                if (x < -cellSize || y < -cellSize || x > canvas.Width() + cellSize
                    || y > canvas.Height() + cellSize) {
                    continue;
                }
                // Vu de très loin, un animal reste un point de deux pixels.
                if (radius < 1.5) {
                    canvas.Fill({static_cast<int>(x), static_cast<int>(y), 2, 2}, style.color);
                    continue;
                }
                const double angle = PathHeading(animal.path, progress);
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

        // Cadre autour de la case choisie, assez grand pour rester visible de loin.
        if (selected) {
            const int left = static_cast<int>(std::floor(view.ScreenX(selected->x)));
            const int top = static_cast<int>(std::floor(view.ScreenY(selected->y)));
            const int side = std::max(8, static_cast<int>(cellSize) - 1);
            RECT frame{left, top, left + side, top + side};
            const HBRUSH frameBrush = CreateSolidBrush(RGB(255, 255, 255));
            for (int thickness = 0; thickness < 3; ++thickness) {
                FrameRect(buffer, &frame, frameBrush);
                InflateRect(&frame, -1, -1);
            }
            DeleteObject(frameBrush);
        }
    }
}
