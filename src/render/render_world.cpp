module;

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include "../ui/layout.hpp"

module ecosim.render;

import std;
import ecosim.simulation;
import ecosim.animation;
import ecosim.cell_info;

using namespace ecosim_render_detail;

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
        // Un bit par espèce, au départ et à l'arrivée : les deux espèces se partagent la case.
        std::vector<unsigned char> before(cellCount);
        std::vector<unsigned char> after(cellCount);
        const auto bit = [](const ecosim::AnimalKind kind) {
            return static_cast<unsigned char>(kind == ecosim::AnimalKind::Herbivore ? 1 : 2);
        };
        for (const auto& animal : animals) {
            before[indexOf(animal.previousPosition)] |= bit(animal.kind);
            after[indexOf(animal.position)] |= bit(animal.kind);
        }

        const auto anchor = [&](const ecosim::Position position, const ecosim::AnimalKind kind,
                                const std::vector<unsigned char>& occupancy) {
            const bool shared = occupancy[indexOf(position)] == 3;
            const int offset = !shared ? cellSize / 2
                : kind == ecosim::AnimalKind::Herbivore ? cellSize / 4 : 3 * cellSize / 4;
            return Marker{position.x * cellSize + offset, position.y * cellSize + cellSize / 2,
                          shared ? cellSize / 5 : cellSize * 36 / 100, kind, 1};
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
    void Renderer2D::DrawWorld(const Simulation& simulation, const int speed, const bool paused,
                               const double progress) const {
        const World& world = simulation.GetWorld();
        std::vector<AnimalSnapshot> animals = simulation.Animals();
        AttachNewbornsToParents(animals);
        const std::vector<Marker> markers = AnimalMarkers(animals, world, cellSize, progress);
        const auto hwnd = static_cast<HWND>(window);
        const HDC screen = GetDC(hwnd);
        const HDC buffer = CreateCompatibleDC(screen);
        const HBITMAP bitmap = CreateCompatibleBitmap(screen, windowWidth, windowHeight);
        const HGDIOBJ previous = SelectObject(buffer, bitmap);
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

        const HBRUSH animalBrush = CreateSolidBrush(RGB(245, 210, 70));
        const HBRUSH predatorBrush = CreateSolidBrush(RGB(220, 85, 75));
        const HGDIOBJ previousBrush = SelectObject(buffer, animalBrush);
        const HGDIOBJ previousPen = SelectObject(buffer, GetStockObject(NULL_PEN));
        SetBkMode(buffer, TRANSPARENT);
        for (const Marker& marker : markers) {
            SelectObject(buffer, marker.kind == AnimalKind::Herbivore ? animalBrush : predatorBrush);
            Ellipse(buffer, marker.x - marker.radius, marker.y - marker.radius,
                    marker.x + marker.radius, marker.y + marker.radius);
            if (marker.count > 1) {
                const std::string label = std::to_string(marker.count);
                const TextSize extent = MeasureText(buffer, label);
                SetTextColor(buffer, RGB(30, 30, 30));
                WriteText(buffer, marker.x - extent.cx / 2, marker.y - extent.cy / 2, label);
            }
        }
        SelectObject(buffer, previousPen);
        SelectObject(buffer, previousBrush);
        DeleteObject(animalBrush);
        DeleteObject(predatorBrush);

        std::string status;
        if (simulation.IsFinished()) status = "Fin de la simulation : plus aucun animal";
        else if (paused) status = std::format("En pause   Vitesse x{}   (flèche droite ou haut : avancer ; Tab : vitesse)", speed);
        else status = std::format("Vitesse x{}   (Tab ou touches 1 à 4 : x1 x2 x5 x10 ; Échap : options ; clic : détails d'une case)", speed);
        const auto hud = std::format("Génération {}   Herbivores {}   Prédateurs {}",
                                     simulation.GetGeneration(), simulation.HerbivoreCount(),
                                     simulation.PredatorCount());
        SetTextColor(buffer, RGB(255, 255, 255));
        WriteText(buffer, 10, 10, hud);
        WriteText(buffer, 10, 30, status);
        const UiRect button = GraphButton(windowWidth);
        const RECT buttonRect{button.x, button.y, button.x + button.width, button.y + button.height};
        const HBRUSH buttonBrush = CreateSolidBrush(RGB(55, 64, 77));
        FillRect(buffer, &buttonRect, buttonBrush);
        const UiRect restart = RestartButton(windowWidth);
        const RECT restartRect{restart.x, restart.y, restart.x + restart.width, restart.y + restart.height};
        FillRect(buffer, &restartRect, buttonBrush);
        if (!simulation.IsFinished()) {
            const UiRect pause = PauseButton(windowWidth, false);
            const RECT pauseRect{pause.x, pause.y, pause.x + pause.width, pause.y + pause.height};
            FillRect(buffer, &pauseRect, buttonBrush);
            const std::string label = paused ? "Reprendre" : "Pause";
            WriteText(buffer, pause.x + 20, pause.y + 9, label);
            if (paused) {
                const UiRect next = NextButton(windowWidth, false);
                const RECT nextRect{next.x, next.y, next.x + next.width, next.y + next.height};
                FillRect(buffer, &nextRect, buttonBrush);
                const std::string nextLabel = "Génération +1";
                WriteText(buffer, next.x + 20, next.y + 9, nextLabel);
            }
        }
        DeleteObject(buttonBrush);
        WriteText(buffer, button.x + 20, button.y + 9, "Courbes");
        WriteText(buffer, restart.x + 20, restart.y + 9, "Recommencer");
        DrawSelection(buffer, simulation);
        if (controller.IsExitDialogOpen()) DrawExitDialog(buffer);
        BitBlt(screen, 0, 0, windowWidth, windowHeight, buffer, 0, 0, SRCCOPY);
        SelectObject(buffer, previous);
        DeleteObject(bitmap);
        DeleteDC(buffer);
        ReleaseDC(hwnd, screen);
        Sleep(16);
    }
}
