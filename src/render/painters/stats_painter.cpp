module;

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include "../gdi/gdi.hpp"

module ecosim.render;

import std;
import ecosim.layout;
import ecosim.species;
import ecosim.species_style;

namespace {
    using ecosim::UiRect;

    constexpr COLORREF VegetationColor{RGB(120, 200, 130)};
    constexpr COLORREF PrimaryInk{RGB(255, 255, 255)};
    constexpr COLORREF MutedInk{RGB(160, 175, 195)};
    constexpr COLORREF PanelColor{RGB(31, 37, 48)};
    constexpr COLORREF ButtonColor{RGB(55, 64, 77)};
    constexpr COLORREF CursorColor{RGB(125, 183, 235)};

    COLORREF ToColorRef(const ecosim::Color color) { return RGB(color.red, color.green, color.blue); }

    // Une série sans espèce est une mesure du monde : la végétation.
    COLORREF ColorOf(const std::optional<ecosim::AnimalKind> species) {
        return species ? ToColorRef(ecosim::StyleOf(*species).color) : VegetationColor;
    }

    std::string NameOf(const std::optional<ecosim::AnimalKind> species) {
        return species ? ecosim::SpeciesOf(*species).pluralName : std::string{"Total"};
    }

    void Fill(const HDC dc, const UiRect area, const COLORREF color) {
        ecosim_gdi::FillArea(dc, area.x, area.y, area.width, area.height, color);
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
        const HPEN pen = CreatePen(PS_SOLID, 2, ColorOf(series.species));
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

    // Écrit avec les polices et couleurs de l'écran des statistiques.
    using Write = std::function<void(int, int, const std::string&, COLORREF)>;
    // Un graphique : titre, légende, axes puis courbes.
    // WriteText et MeasureText sont les outils de texte du rendu.
    void DrawPanel(const HDC buffer, const Write& write, const UiRect panel,
                   const std::vector<ecosim::PopulationSample>& history,
                   const ecosim::ChartMetric metric, const ecosim::ChartSelection& selection,
                   const std::size_t shown, const bool hovered,
                   const std::function<int(const std::string&)>& textWidth) {
        const UiRect plot = ecosim::ChartPlot(panel);
        const std::vector<ecosim::ChartSeries> series = ecosim::BuildSeries(history, metric, selection);
        const long long maximum = ecosim::AxisMaximum(series);
        Fill(buffer, panel, PanelColor);
        write(panel.x + 12, panel.y + 6, ecosim::ChartTitle(metric, selection.cumulative), PrimaryInk);

        // Légende avec la valeur actuelle, ou celle de la génération survolée.
        int legendX = panel.x + 12;
        for (const ecosim::ChartSeries& one : series) {
            Fill(buffer, {legendX, panel.y + 35, 11, 11}, ColorOf(one.species));
            const std::string text = std::format("{} : {}", NameOf(one.species),
                                                 ecosim::FormatValue(metric, one.values[shown]));
            write(legendX + 17, panel.y + 30, text, MutedInk);
            legendX += 17 + textWidth(text) + 22;
        }
        if (series.empty()) write(legendX, panel.y + 30, "Aucune espèce à afficher", MutedInk);

        for (int tick = 0; tick <= 4; ++tick) {
            const int y = plot.y + tick * plot.height / 4;
            Line(buffer, plot.x, y, plot.x + plot.width, y, RGB(57, 66, 80));
            // Sur un petit graphique, une graduation sur deux évite les chevauchements.
            if (plot.height < 100 && tick % 2 != 0) continue;
            const std::string label = std::to_string(maximum * (4 - tick) / 4);
            write(plot.x - 8 - textWidth(label), y - 9, label, MutedInk);
        }
        const std::string first = std::to_string(history.front().generation);
        const std::string last = std::format("génération {}", history.back().generation);
        write(plot.x, plot.y + plot.height + 5, first, MutedInk);
        write(plot.x + plot.width - textWidth(last), plot.y + plot.height + 5, last,
              MutedInk);

        for (const ecosim::ChartSeries& one : series) DrawCurve(buffer, plot, one, maximum);
        if (hovered) {
            const int x = PointX(plot, shown, history.size());
            Line(buffer, x, plot.y, x, plot.y + plot.height, CursorColor);
        }
        for (const ecosim::ChartSeries& one : series) {
            if (!one.values[shown]) continue;
            DrawPoint(buffer, PointX(plot, shown, history.size()),
                      PointY(plot, *one.values[shown], maximum), ColorOf(one.species));
        }
    }
}

namespace ecosim {
    void DrawStatsScreen(Canvas& canvas, const Simulation& simulation, const PlaybackState& playback,
                         const ChartSelection& selection, const std::optional<PointerPosition> pointer) {
        const std::vector<PopulationSample>& history = simulation.Stats().History();
        const HDC buffer = static_cast<HDC>(canvas.Context());
        const int windowWidth = canvas.Width();
        const int windowHeight = canvas.Height();
        const int speed = playback.speed;
        const bool paused = playback.paused;
        Fill(buffer, {0, 0, windowWidth, windowHeight}, RGB(24, 29, 38));
        SetBkMode(buffer, TRANSPARENT);
        const HFONT headingFont = ecosim_gdi::UiFont(30, FW_BOLD);
        const HFONT labelFont = ecosim_gdi::UiFont(20, FW_NORMAL);
        const HFONT chartFont = ecosim_gdi::UiFont(17, FW_NORMAL);
        const HGDIOBJ previousFont = SelectObject(buffer, labelFont);
        const Write write = [&](const int x, const int y, const std::string& value,
                                const COLORREF color) {
            SetTextColor(buffer, color);
            WriteText(buffer, x, y, value);
        };
        const auto textWidth = [buffer](const std::string& value) {
            return MeasureText(buffer, value).cx;
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
        std::string counts = std::format("Génération {}", simulation.GetGeneration());
        for (const SpeciesInfo& species : SpeciesCatalog()) {
            counts += std::format("    {} {}", species.pluralName, simulation.Count(species.kind));
        }
        write(20, 78, std::format("{}    {}", counts, status), PrimaryInk);

        SelectObject(buffer, chartFont);
        for (std::size_t i = 0; i < MetricCount; ++i) {
            toggle(MetricChip(windowWidth, static_cast<int>(i)), MetricNames()[i],
                   selection.metrics[i], CursorColor);
        }
        for (const SpeciesInfo& species : SpeciesCatalog()) {
            toggle(SpeciesChip(static_cast<int>(species.kind)), species.pluralName,
                   !selection.hidden[species.kind], ColorOf(species.kind));
        }
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
        if (pointer) {
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
            DrawPanel(buffer, write, ChartPanel(windowWidth, windowHeight, i, count), history,
                      visible[static_cast<std::size_t>(i)], selection, shown, hovered.has_value(),
                      textWidth);
        }

        SelectObject(buffer, previousFont);
        DeleteObject(headingFont);
        DeleteObject(labelFont);
        DeleteObject(chartFont);
    }
}
