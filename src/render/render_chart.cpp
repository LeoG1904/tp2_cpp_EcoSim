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
    constexpr COLORREF HerbivoreColor{RGB(245, 210, 70)};
    constexpr COLORREF PredatorColor{RGB(220, 85, 75)};
    constexpr COLORREF VegetationColor{RGB(120, 200, 130)};
    constexpr COLORREF PrimaryInk{RGB(255, 255, 255)};
    constexpr COLORREF MutedInk{RGB(160, 175, 195)};
    constexpr COLORREF PanelColor{RGB(31, 37, 48)};
    constexpr COLORREF ButtonColor{RGB(55, 64, 77)};
    constexpr COLORREF CursorColor{RGB(125, 183, 235)};

    COLORREF ColorOf(const ecosim::SeriesSource source) {
        if (source == ecosim::SeriesSource::Herbivores) return HerbivoreColor;
        if (source == ecosim::SeriesSource::Predators) return PredatorColor;
        return VegetationColor;
    }

    std::string NameOf(const ecosim::SeriesSource source) {
        if (source == ecosim::SeriesSource::Herbivores) return "Herbivores";
        if (source == ecosim::SeriesSource::Predators) return "Prédateurs";
        return "Total";
    }

    void Fill(const HDC dc, const UiRect area, const COLORREF color) {
        const RECT rect{area.x, area.y, area.x + area.width, area.y + area.height};
        const HBRUSH brush = CreateSolidBrush(color);
        FillRect(dc, &rect, brush);
        DeleteObject(brush);
    }

    void Line(const HDC dc, const int x1, const int y1, const int x2, const int y2,
              const COLORREF color) {
        const HPEN pen = CreatePen(PS_SOLID, 1, color);
        const HGDIOBJ previous = SelectObject(dc, pen);
        MoveToEx(dc, x1, y1, nullptr);
        LineTo(dc, x2, y2);
        SelectObject(dc, previous);
        DeleteObject(pen);
    }

    int PointX(const UiRect plot, const std::size_t index, const std::size_t count) {
        if (count < 2) return plot.x;
        return plot.x + static_cast<int>(static_cast<long long>(index) * plot.width
                                         / static_cast<long long>(count - 1));
    }

    int PointY(const UiRect plot, const double value, const long long maximum) {
        return plot.y + plot.height - static_cast<int>(value / static_cast<double>(maximum) * plot.height);
    }

    // Une valeur absente coupe la courbe au lieu de la faire tomber à zéro.
    void DrawCurve(const HDC dc, const UiRect plot, const ecosim::ChartSeries& series,
                   const long long maximum) {
        const HPEN pen = CreatePen(PS_SOLID, 2, ColorOf(series.source));
        const HGDIOBJ previous = SelectObject(dc, pen);
        bool drawing = false;
        for (std::size_t i = 0; i < series.values.size(); ++i) {
            if (!series.values[i]) {
                drawing = false;
                continue;
            }
            const int x = PointX(plot, i, series.values.size());
            const int y = PointY(plot, *series.values[i], maximum);
            if (drawing) LineTo(dc, x, y);
            else MoveToEx(dc, x, y, nullptr);
            drawing = true;
        }
        SelectObject(dc, previous);
        DeleteObject(pen);
    }

    void DrawPoint(const HDC dc, const int x, const int y, const COLORREF color) {
        const HBRUSH brush = CreateSolidBrush(color);
        const HGDIOBJ previousBrush = SelectObject(dc, brush);
        const HGDIOBJ previousPen = SelectObject(dc, GetStockObject(NULL_PEN));
        Ellipse(dc, x - 5, y - 5, x + 6, y + 6);
        SelectObject(dc, previousPen);
        SelectObject(dc, previousBrush);
        DeleteObject(brush);
    }
}

namespace ecosim {
    void Renderer2D::DrawGraph(const Simulation& simulation, const int speed, const bool paused) const {
        const std::vector<PopulationSample>& history = simulation.PopulationHistory();
        const ChartSelection& selection = controller.Charts();
        const auto hwnd = static_cast<HWND>(window);
        const HDC screen = GetDC(hwnd);
        const HDC buffer = CreateCompatibleDC(screen);
        const HBITMAP bitmap = CreateCompatibleBitmap(screen, windowWidth, windowHeight);
        const HGDIOBJ previous = SelectObject(buffer, bitmap);
        Fill(buffer, {0, 0, windowWidth, windowHeight}, RGB(24, 29, 38));
        SetBkMode(buffer, TRANSPARENT);
        const auto createFont = [](const int height, const int weight) {
            return CreateFontW(height, 0, 0, 0, weight, FALSE, FALSE, FALSE,
                               DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                               CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe UI");
        };
        const HFONT headingFont = createFont(30, FW_BOLD);
        const HFONT labelFont = createFont(20, FW_NORMAL);
        const HFONT chartFont = createFont(17, FW_NORMAL);
        const HGDIOBJ previousFont = SelectObject(buffer, labelFont);
        const auto write = [&](const int x, const int y, const std::string& value,
                               const COLORREF color) {
            SetTextColor(buffer, color);
            WriteText(buffer, x, y, value);
        };
        const auto button = [&](const UiRect area, const std::string& label) {
            Fill(buffer, area, ButtonColor);
            write(area.x + 20, area.y + 7, label, PrimaryInk);
        };
        // Case à cocher : pleine et de la couleur de la série quand elle est affichée.
        const auto toggle = [&](const UiRect area, const std::string& label, const bool selected,
                                const COLORREF color) {
            Fill(buffer, area, selected ? ButtonColor : PanelColor);
            const UiRect box{area.x + 10, area.y + 10, 14, 14};
            Fill(buffer, box, selected ? color : MutedInk);
            if (!selected) Fill(buffer, {box.x + 2, box.y + 2, 10, 10}, PanelColor);
            write(area.x + 32, area.y + 7, label, selected ? PrimaryInk : MutedInk);
        };

        button(BackButton(), "Retour");
        button(RestartButton(windowWidth), "Recommencer");
        if (!simulation.IsFinished()) {
            button(PauseButton(windowWidth, true), paused ? "Reprendre" : "Pause");
            if (paused) button(NextButton(windowWidth, true), "Génération +1");
        }
        SelectObject(buffer, headingFont);
        write(155, 25, "Statistiques", PrimaryInk);
        SelectObject(buffer, labelFont);
        std::string status;
        if (simulation.IsFinished()) status = "Fin de la simulation : plus aucun animal";
        else if (paused) status = std::format("En pause   Vitesse x{}", speed);
        else status = std::format("Vitesse x{}", speed);
        write(20, 78, std::format("Génération {}    Herbivores {}    Prédateurs {}    {}",
                                  simulation.GetGeneration(), simulation.HerbivoreCount(),
                                  simulation.PredatorCount(), status), PrimaryInk);

        SelectObject(buffer, chartFont);
        for (std::size_t i = 0; i < MetricCount; ++i) {
            toggle(MetricChip(windowWidth, static_cast<int>(i)), MetricNames()[i],
                   selection.metrics[i], CursorColor);
        }
        toggle(SpeciesChip(0), "Herbivores", selection.herbivores, HerbivoreColor);
        toggle(SpeciesChip(1), "Prédateurs", selection.predators, PredatorColor);
        button(CumulativeButton(), selection.cumulative ? "Événements : total"
                                                        : "Événements : par génération");

        const std::vector<ChartMetric> visible = selection.VisibleMetrics();
        const int count = static_cast<int>(visible.size());
        if (count == 0) {
            const UiRect area = ChartArea(windowWidth, windowHeight);
            write(area.x + 20, area.y + 40, "Aucune statistique sélectionnée : cochez-en une ci-dessus.",
                  MutedInk);
        }

        // La génération survolée est la même sur tous les graphiques.
        std::optional<std::size_t> hovered;
        if (const auto pointer = controller.Pointer()) {
            for (int i = 0; i < count; ++i) {
                const UiRect plot = ChartPlot(ChartPanel(windowWidth, windowHeight, i, count));
                if (plot.Contains(pointer->x, pointer->y)) {
                    hovered = IndexAt(pointer->x, plot.x, plot.width, history.size());
                }
            }
        }
        const std::size_t shown = hovered.value_or(history.size() - 1);
        if (hovered) {
            write(CumulativeButton().x + CumulativeButton().width + 24, CumulativeButton().y + 7,
                  std::format("Valeurs à la génération {}", history[shown].generation), CursorColor);
        }

        for (int i = 0; i < count; ++i) {
            const ChartMetric metric = visible[static_cast<std::size_t>(i)];
            const UiRect panel = ChartPanel(windowWidth, windowHeight, i, count);
            const UiRect plot = ChartPlot(panel);
            const std::vector<ChartSeries> series = BuildSeries(history, metric, selection);
            const long long maximum = AxisMaximum(series);
            Fill(buffer, panel, PanelColor);
            write(panel.x + 12, panel.y + 6, ChartTitle(metric, selection.cumulative), PrimaryInk);

            // Légende avec la valeur actuelle, ou celle de la génération survolée.
            int legendX = panel.x + 12;
            for (const ChartSeries& one : series) {
                Fill(buffer, {legendX, panel.y + 35, 11, 11}, ColorOf(one.source));
                const std::string text = std::format("{} : {}", NameOf(one.source),
                                                     FormatValue(metric, one.values[shown]));
                write(legendX + 17, panel.y + 30, text, MutedInk);
                legendX += 17 + MeasureText(buffer, text).cx + 22;
            }
            if (series.empty()) write(legendX, panel.y + 30, "Aucune espèce à afficher", MutedInk);

            for (int tick = 0; tick <= 4; ++tick) {
                const int y = plot.y + tick * plot.height / 4;
                Line(buffer, plot.x, y, plot.x + plot.width, y, RGB(57, 66, 80));
                // Sur un petit graphique, une graduation sur deux évite les chevauchements.
                if (plot.height < 100 && tick % 2 != 0) continue;
                const std::string label = std::to_string(maximum * (4 - tick) / 4);
                write(plot.x - 8 - MeasureText(buffer, label).cx, y - 9, label, MutedInk);
            }
            const std::string first = std::to_string(history.front().generation);
            const std::string last = std::format("génération {}", history.back().generation);
            write(plot.x, plot.y + plot.height + 5, first, MutedInk);
            write(plot.x + plot.width - MeasureText(buffer, last).cx, plot.y + plot.height + 5, last,
                  MutedInk);

            for (const ChartSeries& one : series) DrawCurve(buffer, plot, one, maximum);
            if (hovered) {
                const int x = PointX(plot, shown, history.size());
                Line(buffer, x, plot.y, x, plot.y + plot.height, CursorColor);
            }
            for (const ChartSeries& one : series) {
                if (!one.values[shown]) continue;
                DrawPoint(buffer, PointX(plot, shown, history.size()),
                          PointY(plot, *one.values[shown], maximum), ColorOf(one.source));
            }
        }

        SelectObject(buffer, previousFont);
        DeleteObject(headingFont);
        DeleteObject(labelFont);
        DeleteObject(chartFont);
        if (controller.IsExitDialogOpen()) DrawExitDialog(buffer);
        BitBlt(screen, 0, 0, windowWidth, windowHeight, buffer, 0, 0, SRCCOPY);
        SelectObject(buffer, previous);
        DeleteObject(bitmap);
        DeleteDC(buffer);
        ReleaseDC(hwnd, screen);
        Sleep(16);
    }
}
