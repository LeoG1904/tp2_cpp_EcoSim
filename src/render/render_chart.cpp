module;

#ifdef ECOSIM_USE_RAYLIB
#include <raylib.h>
#else
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#endif

#include "layout.hpp"

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
    void Renderer2D::DrawGraph(const Simulation& simulation, const int speed) const {
        const PopulationGraph graph{simulation, windowWidth, windowHeight};
#ifdef ECOSIM_USE_RAYLIB
        BeginDrawing();
        ClearBackground(Color{24, 29, 38, 255});
        const UiRect back = BackButton();
        DrawRectangle(back.x, back.y, back.width, back.height, Color{55, 64, 77, 255});
        DrawText("Retour", back.x + 20, back.y + 8, 20, RAYWHITE);
        DrawText("Evolution des populations", 155, 25, 28, RAYWHITE);
        const std::string summary = std::format("Generation {}    Herbivores {}    Predateurs {}    Vitesse x{}",
                                                simulation.GetGeneration(), simulation.HerbivoreCount(),
                                                simulation.PredatorCount(), speed);
        DrawText(summary.c_str(), 90, 78, 20, RAYWHITE);
        DrawRectangle(90, 116, 15, 15, Color{245, 210, 70, 255});
        DrawText("Herbivores", 115, 114, 20, RAYWHITE);
        DrawRectangle(270, 116, 15, 15, Color{220, 85, 75, 255});
        DrawText("Predateurs", 295, 114, 20, RAYWHITE);

        for (int tick = 0; tick <= 4; ++tick) {
            const int y = graph.plot.y + tick * graph.plot.height / 4;
            DrawLine(graph.plot.x, y, graph.plot.x + graph.plot.width, y,
                     Color{57, 66, 80, 255});
            const std::string label = std::to_string(graph.scale * (4 - tick) / 4);
            DrawText(label.c_str(), 18, y - 9, 18, RAYWHITE);
        }
        for (int tick = 0; tick <= 4; ++tick) {
            if (tick > 0 && graph.GenerationAt(tick) == graph.GenerationAt(tick - 1)) continue;
            const int x = graph.TickX(tick);
            if (tick > 0 && tick < 4) {
                DrawLine(x, graph.plot.y, x, graph.plot.y + graph.plot.height,
                         Color{57, 66, 80, 255});
            }
            const std::string label = std::to_string(graph.GenerationAt(tick));
            const int labelWidth = MeasureText(label.c_str(), 18);
            const int labelX = std::clamp(x - labelWidth / 2, graph.plot.x,
                                          std::max(graph.plot.x, graph.plot.x + graph.plot.width - labelWidth));
            DrawText(label.c_str(), labelX, graph.plot.y + graph.plot.height + 14, 18, RAYWHITE);
        }
        DrawLine(graph.plot.x, graph.plot.y, graph.plot.x,
                 graph.plot.y + graph.plot.height, RAYWHITE);
        for (int step = 1; step <= graph.segments; ++step) {
            const auto [hx1, hy1] = graph.Point(step - 1, false);
            const auto [hx2, hy2] = graph.Point(step, false);
            const auto [px1, py1] = graph.Point(step - 1, true);
            const auto [px2, py2] = graph.Point(step, true);
            DrawLineEx(Vector2{static_cast<float>(hx1), static_cast<float>(hy1)},
                       Vector2{static_cast<float>(hx2), static_cast<float>(hy2)},
                       2.5F, Color{245, 210, 70, 255});
            DrawLineEx(Vector2{static_cast<float>(px1), static_cast<float>(py1)},
                       Vector2{static_cast<float>(px2), static_cast<float>(py2)},
                       2.5F, Color{220, 85, 75, 255});
        }
        const int currentX = graph.segments > 0 ? graph.plot.x + graph.plot.width : graph.plot.x;
        DrawLine(currentX, graph.plot.y, currentX, graph.plot.y + graph.plot.height,
                 Color{125, 183, 235, 255});
        const auto& current = graph.history.back();
        DrawCircle(currentX, graph.Y(current.herbivores), 5, Color{245, 210, 70, 255});
        DrawCircle(currentX, graph.Y(current.predators), 4, Color{220, 85, 75, 255});
        const std::string currentLabel = std::format("Generation actuelle : {}", current.generation);
        DrawText(currentLabel.c_str(), graph.plot.x + graph.plot.width - MeasureText(currentLabel.c_str(), 20),
                 graph.plot.y - 28, 20, Color{125, 183, 235, 255});
        DrawText("Generations", graph.plot.x + graph.plot.width / 2 - 60,
                 graph.plot.y + graph.plot.height + 52, 18, RAYWHITE);
        EndDrawing();
#else
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
        const auto write = [&](const int x, const int y, const std::wstring& value) {
            TextOutW(buffer, x, y, value.c_str(), static_cast<int>(value.size()));
        };
        const UiRect back = BackButton();
        const RECT backRect{back.x, back.y, back.x + back.width, back.y + back.height};
        const HBRUSH buttonBrush = CreateSolidBrush(RGB(55, 64, 77));
        FillRect(buffer, &backRect, buttonBrush);
        DeleteObject(buttonBrush);
        write(back.x + 20, back.y + 9, L"Retour");
        SelectObject(buffer, headingFont);
        write(155, 25, L"Évolution des populations");
        SelectObject(buffer, labelFont);
        write(90, 78, std::format(L"Génération {}    Herbivores {}    Prédateurs {}    Vitesse x{}",
                                simulation.GetGeneration(), simulation.HerbivoreCount(),
                                simulation.PredatorCount(), speed));
        const HBRUSH herbivoreBrush = CreateSolidBrush(RGB(245, 210, 70));
        const HBRUSH predatorBrush = CreateSolidBrush(RGB(220, 85, 75));
        const RECT herbivoreLegend{90, 116, 105, 131};
        const RECT predatorLegend{270, 116, 285, 131};
        FillRect(buffer, &herbivoreLegend, herbivoreBrush);
        FillRect(buffer, &predatorLegend, predatorBrush);
        write(115, 116, L"Herbivores");
        write(295, 116, L"Prédateurs");

        const HPEN gridPen = CreatePen(PS_SOLID, 1, RGB(57, 66, 80));
        const HGDIOBJ oldPen = SelectObject(buffer, gridPen);
        for (int tick = 0; tick <= 4; ++tick) {
            const int y = graph.plot.y + tick * graph.plot.height / 4;
            MoveToEx(buffer, graph.plot.x, y, nullptr);
            LineTo(buffer, graph.plot.x + graph.plot.width, y);
            write(18, y - 9, std::to_wstring(graph.scale * (4 - tick) / 4));
        }
        for (int tick = 0; tick <= 4; ++tick) {
            if (tick > 0 && graph.GenerationAt(tick) == graph.GenerationAt(tick - 1)) continue;
            const int x = graph.TickX(tick);
            if (tick > 0 && tick < 4) {
                MoveToEx(buffer, x, graph.plot.y, nullptr);
                LineTo(buffer, x, graph.plot.y + graph.plot.height);
            }
            const std::wstring label = std::to_wstring(graph.GenerationAt(tick));
            SIZE extent{};
            GetTextExtentPoint32W(buffer, label.c_str(), static_cast<int>(label.size()), &extent);
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
        const auto currentLabel = std::format(L"Génération actuelle : {}", current.generation);
        SIZE currentExtent{};
        GetTextExtentPoint32W(buffer, currentLabel.c_str(), static_cast<int>(currentLabel.size()), &currentExtent);
        SetTextColor(buffer, RGB(125, 183, 235));
        write(graph.plot.x + graph.plot.width - currentExtent.cx, graph.plot.y - 28, currentLabel);
        SetTextColor(buffer, RGB(255, 255, 255));
        write(graph.plot.x + graph.plot.width / 2 - 55,
              graph.plot.y + graph.plot.height + 52, L"Générations");
        SelectObject(buffer, previousFont);
        DeleteObject(headingFont);
        DeleteObject(labelFont);
        DeleteObject(herbivoreBrush);
        DeleteObject(predatorBrush);
        BitBlt(screen, 0, 0, windowWidth, windowHeight, buffer, 0, 0, SRCCOPY);
        SelectObject(buffer, previous);
        DeleteObject(bitmap);
        DeleteDC(buffer);
        ReleaseDC(hwnd, screen);
        Sleep(16);
#endif
    }
}
