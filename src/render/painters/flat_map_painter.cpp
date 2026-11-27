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
    /// @brief Couleur d'une case sans tenir compte de sa hauteur, selon le terrain et l'herbe.
    ecosim::Color TerrainColor(const ecosim::Cell& cell) {
        switch (cell.GetTerrain()) {
            case ecosim::Terrain::Eau: return {60, 120, 200};
            case ecosim::Terrain::Obstacle: return {90, 90, 90};
            case ecosim::Terrain::Prairie: break;
        }
        const double ratio = static_cast<double>(cell.GetVegetation()) / ecosim::Cell::MaxVegetation;
        return {static_cast<int>(150 - ratio * 90), static_cast<int>(110 + ratio * 90), 70};
    }

    /// @brief Couleur d'une case en 2D : son terrain, éclairci selon son palier.
    ecosim::Color ColorOf(const ecosim::Cell& cell) {
        return ecosim::LevelTint(TerrainColor(cell), cell.GetElevation());
    }

    /// @brief Dessine l'ombre au pied d'une case plus haute, à l'intérieur de la case basse.
    /// @param canvas Image à compléter.
    /// @param world Carte.
    /// @param cell La case basse.
    /// @param area Rectangle de la case basse, en pixels.
    void DrawSlopeShadows(ecosim::Canvas& canvas, const ecosim::World& world, const ecosim::Position cell,
                          const ecosim::UiRect area) {
        const int elevation = world.GetCell(cell).GetElevation();
        const int thickness = std::max(1, area.width / 9);
        // La couleur de la case assombrie : une ombre, et non un mur.
        const ecosim::Color base = ColorOf(world.GetCell(cell));
        const ecosim::Color shade{base.red * 3 / 5, base.green * 3 / 5, base.blue * 3 / 5};
        for (const ecosim::Position neighbor : world.Neighbors(cell)) {
            if (world.GetCell(neighbor).GetElevation() <= elevation) continue;
            ecosim::UiRect shadow = area;
            if (neighbor.x != cell.x) {
                shadow.width = thickness;
                if (neighbor.x > cell.x) shadow.x = area.x + area.width - thickness;
            } else {
                shadow.height = thickness;
                if (neighbor.y > cell.y) shadow.y = area.y + area.height - thickness;
            }
            canvas.Fill(shadow, shade);
        }
    }

    /// @brief Rayon, en pixels, à partir duquel les détails d'un animal sont dessinés.
    constexpr double SmallestDetailedRadius{3.0};

    /// @brief Convertit une couleur du projet en couleur Windows.
    COLORREF Paint(const ecosim::Color color) { return RGB(color.red, color.green, color.blue); }

    /// @brief Dessine des disques et des traits placés par rapport à un animal, en unités de son rayon.
    struct AnimalBrush {
        /// @brief Surface de dessin, avec le pinceau système sélectionné.
        HDC dc;
        /// @brief Centre de l'animal, en pixels.
        double x;
        /// @brief Centre de l'animal, en pixels.
        double y;
        /// @brief Rayon du corps, en pixels.
        double radius;
        /// @brief Direction de la marche, en radians.
        double heading;

        /// @brief Le point situé en avant du centre et sur le côté.
        POINT At(const double forward, const double side) const {
            return {std::lround(x + (std::cos(heading) * forward - std::sin(heading) * side) * radius),
                    std::lround(y + (std::sin(heading) * forward + std::cos(heading) * side) * radius)};
        }

        /// @brief Un disque.
        void Disc(const double forward, const double side, const double size, const ecosim::Color color) const {
            const POINT center = At(forward, side);
            const int half = std::max(1, static_cast<int>(std::lround(size * radius)));
            SetDCBrushColor(dc, Paint(color));
            Ellipse(dc, center.x - half, center.y - half, center.x + half + 1, center.y + half + 1);
        }

        /// @brief Un trait épais aux bouts ronds, d'un point à un autre ; le stylo donne sa couleur et son épaisseur.
        void Stroke(const HPEN pen, const POINT from, const POINT to) const {
            const HGDIOBJ previous = SelectObject(dc, pen);
            MoveToEx(dc, from.x, from.y, nullptr);
            LineTo(dc, to.x, to.y);
            SelectObject(dc, previous);
        }

        /// @brief Un stylo dont l'épaisseur suit la taille de l'animal. L'appelant doit le détruire.
        HPEN Pen(const double width, const ecosim::Color color) const {
            return CreatePen(PS_SOLID, std::max(1, static_cast<int>(std::lround(width * radius))), Paint(color));
        }
    };

    /// @brief Les détails des jeunes : taches du faon, oreilles courtes du lapereau et du louveteau.
    void DrawYoungDetails(const AnimalBrush& brush, const ecosim::AnimalKind kind, const ecosim::SexLook& look) {
        if (kind == ecosim::AnimalKind::Deer) {
            brush.Disc(-0.45, -0.4, 0.2, look.accent);
            brush.Disc(-0.5, 0.35, 0.2, look.accent);
            brush.Disc(0.05, 0.0, 0.2, look.accent);
        } else if (kind == ecosim::AnimalKind::Rabbit) {
            const HPEN ear = brush.Pen(0.5, look.accent);
            for (const double side : {-1.0, 1.0}) {
                brush.Stroke(ear, brush.At(0.75, side * 0.45), brush.At(0.35, side * 0.95));
            }
            DeleteObject(ear);
        } else if (kind == ecosim::AnimalKind::Wolf) {
            brush.Disc(0.7, -0.75, 0.33, look.accent);
            brush.Disc(0.7, 0.75, 0.33, look.accent);
        }
    }

    /// @brief Dessine un animal : corps, tête et, s'il est assez grand, les détails de son âge ou de son sexe.
    void DrawAnimal(const AnimalBrush& brush, const ecosim::AnimalKind kind, const ecosim::Sex sex,
                    const int age) {
        const ecosim::SexLook& look = ecosim::LookOf(kind, sex, age);
        const bool young = ecosim::IsYoung(age);
        const bool male = sex == ecosim::Sex::Male;
        const bool detailed = brush.radius >= SmallestDetailedRadius;
        const ecosim::Color headColor{look.body.red * 7 / 10, look.body.green * 7 / 10,
                                      look.body.blue * 7 / 10};
        // La queue passe sous le corps.
        if (detailed && kind == ecosim::AnimalKind::Fox) {
            // La queue d'un renardeau est courte.
            const HPEN tail = brush.Pen(0.55, look.body);
            brush.Stroke(tail, brush.At(-0.7, 0.0), brush.At(young ? -1.3 : -1.75, 0.0));
            DeleteObject(tail);
            if (male && !young) brush.Disc(-1.8, 0.0, 0.34, look.accent);
        }
        brush.Disc(0.0, 0.0, 1.0, look.body);
        if (detailed && kind == ecosim::AnimalKind::Wolf && male && !young) brush.Disc(0.45, 0.0, 0.8, look.accent);
        // Un jeune a la tête grosse pour son corps.
        brush.Disc(0.85, 0.0, young ? 0.72 : 0.55, headColor);
        if (!detailed) return;
        if (young) {
            DrawYoungDetails(brush, kind, look);
            return;
        }

        if (kind == ecosim::AnimalKind::Wolf && !male) brush.Disc(1.25, 0.0, 0.3, look.accent);
        if (kind == ecosim::AnimalKind::Rabbit) {
            // Deux longues oreilles couchées vers l'arrière.
            const HPEN ear = brush.Pen(0.42, look.accent);
            for (const double side : {-1.0, 1.0}) {
                brush.Stroke(ear, brush.At(0.65, side * 0.35), brush.At(-1.05, side * 0.95));
            }
            DeleteObject(ear);
        }
        if (kind == ecosim::AnimalKind::Deer && male) {
            // Des bois à deux pointes de chaque côté de la tête.
            const HPEN antler = brush.Pen(0.24, look.accent);
            for (const double side : {-1.0, 1.0}) {
                brush.Stroke(antler, brush.At(0.9, side * 0.3), brush.At(1.45, side * 1.15));
                brush.Stroke(antler, brush.At(1.15, side * 0.7), brush.At(1.85, side * 0.6));
            }
            DeleteObject(antler);
        }
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
                    const ecosim::UiRect area{left, top, edge(view.ScreenX(x + 1)) - left - 1, tall - 1};
                    canvas.Fill(area, ColorOf(world.GetCell(ecosim::Position{x, y})));
                    DrawSlopeShadows(canvas, world, {x, y}, area);
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

        // Chaque animal est dessiné là où il se trouve. L'ordre des espèces garde les chasseurs au-dessus.
        const double cellSize = view.CellSize();
        const HGDIOBJ previousPen = SelectObject(buffer, GetStockObject(NULL_PEN));
        // Le pinceau système change de couleur sans créer un objet par disque.
        const HGDIOBJ previousBrush = SelectObject(buffer, GetStockObject(DC_BRUSH));
        for (const SpeciesInfo& species : SpeciesCatalog()) {
            // Le disque d'un adulte a la taille du corps utilisée par les collisions.
            const double adultRadius = cellSize * species.bodyRadius;
            for (const AnimalSnapshot& animal : animals) {
                if (animal.kind != species.kind) continue;
                const Point point = PathPoint(animal.path, progress);
                const double x = view.ScreenX(point.x);
                const double y = view.ScreenY(point.y);
                if (x < -cellSize || y < -cellSize || x > canvas.Width() + cellSize
                    || y > canvas.Height() + cellSize) {
                    continue;
                }
                const double radius = adultRadius * (IsYoung(animal.age) ? YoungScale : 1.0);
                // Vu de très loin, un animal reste un point de deux pixels.
                if (radius < 1.5) {
                    canvas.Fill({static_cast<int>(x), static_cast<int>(y), 2, 2},
                                LookOf(animal.kind, animal.sex, animal.age).body);
                    continue;
                }
                DrawAnimal({buffer, x, y, radius, PathHeading(animal.path, progress)}, animal.kind,
                           animal.sex, animal.age);
            }
        }
        SelectObject(buffer, previousBrush);
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
