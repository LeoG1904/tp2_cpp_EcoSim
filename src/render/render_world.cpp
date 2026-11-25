module;

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include "layout.hpp"

module ecosim.render;

import std;
import ecosim.simulation;

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
}

namespace ecosim {
    void Renderer2D::DrawWorld(const Simulation& simulation, const int speed) const {
        const World& world = simulation.GetWorld();
        std::vector<int> herbivores(static_cast<std::size_t>(world.GetWidth()) * world.GetHeight());
        std::vector<int> predators(herbivores.size());
        for (const auto& animal : simulation.Animals()) {
            const auto index = static_cast<std::size_t>(animal.position.y) * world.GetWidth()
                             + static_cast<std::size_t>(animal.position.x);
            if (animal.kind == AnimalKind::Herbivore) ++herbivores[index];
            else if (animal.kind == AnimalKind::Predator) ++predators[index];
        }
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
        for (int y = 0; y < world.GetHeight(); ++y) {
            for (int x = 0; x < world.GetWidth(); ++x) {
                const auto index = static_cast<std::size_t>(y) * world.GetWidth() + x;
                const int herbivoreCount = herbivores[index];
                const int predatorCount = predators[index];
                if (herbivoreCount == 0 && predatorCount == 0) continue;
                const bool shared = herbivoreCount > 0 && predatorCount > 0;
                const int centerY = y * cellSize + cellSize / 2;
                const int radius = shared ? cellSize / 5 : cellSize * 36 / 100;
                const auto drawAnimal = [&](const int count, const int centerX,
                                            const HBRUSH brush) {
                    if (count == 0) return;
                    SelectObject(buffer, brush);
                    Ellipse(buffer, centerX - radius, centerY - radius,
                            centerX + radius, centerY + radius);
                    if (count > 1) {
                        const std::wstring label = std::to_wstring(count);
                        SIZE extent{};
                        GetTextExtentPoint32W(buffer, label.c_str(), static_cast<int>(label.size()), &extent);
                        SetTextColor(buffer, RGB(30, 30, 30));
                        TextOutW(buffer, centerX - extent.cx / 2, centerY - extent.cy / 2,
                                 label.c_str(), static_cast<int>(label.size()));
                    }
                };
                drawAnimal(herbivoreCount, x * cellSize + (shared ? cellSize / 4 : cellSize / 2),
                           animalBrush);
                drawAnimal(predatorCount, x * cellSize + (shared ? 3 * cellSize / 4 : cellSize / 2),
                           predatorBrush);
            }
        }
        SelectObject(buffer, previousPen);
        SelectObject(buffer, previousBrush);
        DeleteObject(animalBrush);
        DeleteObject(predatorBrush);

        const std::wstring status = simulation.IsFinished()
            ? L"Fin de la simulation : plus aucun animal"
            : std::format(L"Vitesse x{}   (touches 1 à 4 : x1 x2 x5 x10 ; Échap : quitter)", speed);
        const auto hud = std::format(L"Génération {}   Herbivores {}   Prédateurs {}",
                                     simulation.GetGeneration(), simulation.HerbivoreCount(),
                                     simulation.PredatorCount());
        SetTextColor(buffer, RGB(255, 255, 255));
        TextOutW(buffer, 10, 10, hud.c_str(), static_cast<int>(hud.size()));
        TextOutW(buffer, 10, 30, status.c_str(), static_cast<int>(status.size()));
        const UiRect button = GraphButton(windowWidth);
        const RECT buttonRect{button.x, button.y, button.x + button.width, button.y + button.height};
        const HBRUSH buttonBrush = CreateSolidBrush(RGB(55, 64, 77));
        FillRect(buffer, &buttonRect, buttonBrush);
        const UiRect restart = RestartButton(windowWidth);
        const RECT restartRect{restart.x, restart.y, restart.x + restart.width, restart.y + restart.height};
        FillRect(buffer, &restartRect, buttonBrush);
        DeleteObject(buttonBrush);
        TextOutW(buffer, button.x + 20, button.y + 9, L"Courbes", 7);
        TextOutW(buffer, restart.x + 20, restart.y + 9, L"Recommencer", 11);
        BitBlt(screen, 0, 0, windowWidth, windowHeight, buffer, 0, 0, SRCCOPY);
        SelectObject(buffer, previous);
        DeleteObject(bitmap);
        DeleteDC(buffer);
        ReleaseDC(hwnd, screen);
        Sleep(16);
    }
}
