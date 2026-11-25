module;

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include "../ui/layout.hpp"

module ecosim.render;

import std;
import ecosim.simulation;

using namespace ecosim_render_detail;

namespace {
    struct PopulationGraph {
        // Projette l'historique sur l'écran sans modifier les données de simulation.
        const std::vector<ecosim::PopulationSample>& history;
        UiRect plot;
        std::size_t scale{10};
        int segments;

        PopulationGraph(const ecosim::Simulation& simulation, const int width, const int height)
            : history{simulation.PopulationHistory()},
              plot{PopulationPlot(width, height)},
              segments{static_cast<int>(std::min<std::size_t>(plot.width, history.size() - 1))} {
            for (const auto& sample : history) {
                scale = std::max({scale, sample.herbivores, sample.predators});
            }
            scale = (scale + 9) / 10 * 10;
        }

        std::pair<int, int> Point(const int step, const bool predator) const {
            const std::size_t index = static_cast<std::size_t>(step) * (history.size() - 1)
                                    / static_cast<std::size_t>(segments);
            const std::size_t count = predator ? history[index].predators : history[index].herbivores;
            const int x = plot.x + step * plot.width / segments;
            return {x, Y(count)};
        }

        int Y(const std::size_t count) const {
            return plot.y + plot.height
                 - static_cast<int>(static_cast<double>(count) * plot.height / scale);
        }

        int GenerationAt(const int tick) const {
            return static_cast<int>(static_cast<long long>(history.back().generation) * tick / 4);
        }

        int TickX(const int tick) const { return plot.x + tick * plot.width / 4; }
    };

}

namespace ecosim {
    void Renderer2D::DrawGraph(const Simulation& simulation, const int speed, const bool paused) const {
        const PopulationGraph graph{simulation, windowWidth, windowHeight};
        const auto hwnd = static_cast<HWND>(window);
        const HDC screen = GetDC(hwnd);
        const HDC buffer = CreateCompatibleDC(screen);
        const HBITMAP bitmap = CreateCompatibleBitmap(screen, windowWidth, windowHeight);
        const HGDIOBJ previous = SelectObject(buffer, bitmap);
        const RECT bounds{0, 0, windowWidth, windowHeight};
        const HBRUSH background = CreateSolidBrush(RGB(24, 29, 38));
        FillRect(buffer, &bounds, background);
        DeleteObject(background);
        SetBkMode(buffer, TRANSPARENT);
        SetTextColor(buffer, RGB(255, 255, 255));
        const HFONT headingFont = CreateFontW(30, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                                              DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                              CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe UI");
        const HFONT labelFont = CreateFontW(20, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                            CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe UI");
        const HGDIOBJ previousFont = SelectObject(buffer, labelFont);
        const auto write = [&](const int x, const int y, const std::string& value) {
            WriteText(buffer, x, y, value);
        };
        const UiRect back = BackButton();
        const RECT backRect{back.x, back.y, back.x + back.width, back.y + back.height};
        const HBRUSH buttonBrush = CreateSolidBrush(RGB(55, 64, 77));
        FillRect(buffer, &backRect, buttonBrush);
        const UiRect restart = RestartButton(windowWidth);
        const RECT restartRect{restart.x, restart.y, restart.x + restart.width, restart.y + restart.height};
        FillRect(buffer, &restartRect, buttonBrush);
        if (!simulation.IsFinished()) {
            const UiRect pause = PauseButton(windowWidth, true);
            const RECT pauseRect{pause.x, pause.y, pause.x + pause.width, pause.y + pause.height};
            FillRect(buffer, &pauseRect, buttonBrush);
            write(pause.x + 20, pause.y + 9, paused ? "Reprendre" : "Pause");
            if (paused) {
                const UiRect next = NextButton(windowWidth, true);
                const RECT nextRect{next.x, next.y, next.x + next.width, next.y + next.height};
                FillRect(buffer, &nextRect, buttonBrush);
                write(next.x + 12, next.y + 9, "Génération +1");
            }
        }
        DeleteObject(buttonBrush);
        write(back.x + 20, back.y + 9, "Retour");
        write(restart.x + 20, restart.y + 9, "Recommencer");
        SelectObject(buffer, headingFont);
        write(155, 25, "Évolution des populations");
        SelectObject(buffer, labelFont);
        std::string status;
        if (simulation.IsFinished()) status = "Fin de la simulation : plus aucun animal";
        else if (paused) status = std::format("En pause   Vitesse x{}", speed);
        else status = std::format("Vitesse x{}", speed);
        write(90, 78, std::format("Génération {}    Herbivores {}    Prédateurs {}    {}",
                                simulation.GetGeneration(), simulation.HerbivoreCount(),
                                simulation.PredatorCount(), status));
        const HBRUSH herbivoreBrush = CreateSolidBrush(RGB(245, 210, 70));
        const HBRUSH predatorBrush = CreateSolidBrush(RGB(220, 85, 75));
        const RECT herbivoreLegend{90, 116, 105, 131};
        const RECT predatorLegend{270, 116, 285, 131};
        FillRect(buffer, &herbivoreLegend, herbivoreBrush);
        FillRect(buffer, &predatorLegend, predatorBrush);
        write(115, 116, "Herbivores");
        write(295, 116, "Prédateurs");

        const HPEN gridPen = CreatePen(PS_SOLID, 1, RGB(57, 66, 80));
        const HGDIOBJ oldPen = SelectObject(buffer, gridPen);
        for (int tick = 0; tick <= 4; ++tick) {
            const int y = graph.plot.y + tick * graph.plot.height / 4;
            MoveToEx(buffer, graph.plot.x, y, nullptr);
            LineTo(buffer, graph.plot.x + graph.plot.width, y);
            write(18, y - 9, std::to_string(graph.scale * (4 - tick) / 4));
        }
        for (int tick = 0; tick <= 4; ++tick) {
            if (tick > 0 && graph.GenerationAt(tick) == graph.GenerationAt(tick - 1)) continue;
            const int x = graph.TickX(tick);
            if (tick > 0 && tick < 4) {
                MoveToEx(buffer, x, graph.plot.y, nullptr);
                LineTo(buffer, x, graph.plot.y + graph.plot.height);
            }
            const std::string label = std::to_string(graph.GenerationAt(tick));
            const TextSize extent = MeasureText(buffer, label);
            write(std::clamp(x - static_cast<int>(extent.cx) / 2, graph.plot.x,
                             std::max(graph.plot.x, graph.plot.x + graph.plot.width - static_cast<int>(extent.cx))),
                  graph.plot.y + graph.plot.height + 14, label);
        }
        SelectObject(buffer, oldPen);
        DeleteObject(gridPen);
        const auto drawCurve = [&](const bool predator, const COLORREF color) {
            const HPEN pen = CreatePen(PS_SOLID, 2, color);
            const HGDIOBJ old = SelectObject(buffer, pen);
            if (graph.segments > 0) {
                const auto [startX, startY] = graph.Point(0, predator);
                MoveToEx(buffer, startX, startY, nullptr);
                for (int step = 1; step <= graph.segments; ++step) {
                    const auto [x, y] = graph.Point(step, predator);
                    LineTo(buffer, x, y);
                }
            }
            SelectObject(buffer, old);
            DeleteObject(pen);
        };
        drawCurve(false, RGB(245, 210, 70));
        drawCurve(true, RGB(220, 85, 75));
        const int currentX = graph.segments > 0 ? graph.plot.x + graph.plot.width : graph.plot.x;
        const HPEN currentPen = CreatePen(PS_SOLID, 2, RGB(125, 183, 235));
        const HGDIOBJ previousCurrentPen = SelectObject(buffer, currentPen);
        MoveToEx(buffer, currentX, graph.plot.y, nullptr);
        LineTo(buffer, currentX, graph.plot.y + graph.plot.height);
        SelectObject(buffer, previousCurrentPen);
        DeleteObject(currentPen);
        const auto& current = graph.history.back();
        const auto drawCurrentPoint = [&](const std::size_t count, const HBRUSH brush) {
            const int y = graph.Y(count);
            const HGDIOBJ oldBrush = SelectObject(buffer, brush);
            const HGDIOBJ oldPointPen = SelectObject(buffer, GetStockObject(NULL_PEN));
            Ellipse(buffer, currentX - 5, y - 5, currentX + 5, y + 5);
            SelectObject(buffer, oldPointPen);
            SelectObject(buffer, oldBrush);
        };
        drawCurrentPoint(current.herbivores, herbivoreBrush);
        drawCurrentPoint(current.predators, predatorBrush);
        const auto currentLabel = std::format("Génération actuelle : {}", current.generation);
        const TextSize currentExtent = MeasureText(buffer, currentLabel);
        SetTextColor(buffer, RGB(125, 183, 235));
        write(graph.plot.x + graph.plot.width - currentExtent.cx, graph.plot.y - 28, currentLabel);
        SetTextColor(buffer, RGB(255, 255, 255));
        write(graph.plot.x + graph.plot.width / 2 - 55,
              graph.plot.y + graph.plot.height + 52, "Générations");
        SelectObject(buffer, previousFont);
        DeleteObject(headingFont);
        DeleteObject(labelFont);
        DeleteObject(herbivoreBrush);
        DeleteObject(predatorBrush);
        if (controller.IsExitDialogOpen()) DrawExitDialog(buffer);
        BitBlt(screen, 0, 0, windowWidth, windowHeight, buffer, 0, 0, SRCCOPY);
        SelectObject(buffer, previous);
        DeleteObject(bitmap);
        DeleteDC(buffer);
        ReleaseDC(hwnd, screen);
        Sleep(16);
    }
}
