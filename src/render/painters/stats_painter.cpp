/**
 * @file
 * @brief Dessin des graphiques.
 */
module;

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

module ecosim.render;

import std;
import ecosim.layout;
import ecosim.species;
import ecosim.species_style;

namespace {
    using ecosim::UiRect;
    using ecosim::Color;

    /// @brief Couleur de la courbe de l'herbe.
    constexpr Color VegetationColor{120, 200, 130};
    /// @brief Couleur du texte secondaire.
    constexpr Color MutedInk{160, 175, 195};
    /// @brief Fond d'un graphique.
    constexpr Color ChartColor{31, 37, 48};

    /// @brief Convertit une couleur du projet en couleur Windows, pour les traits et les points.
    COLORREF ToColorRef(const Color color) { return RGB(color.red, color.green, color.blue); }

    /// @brief Couleur d'une courbe : celle de l'espèce, ou celle de l'herbe.
    Color ColorOf(const std::optional<ecosim::AnimalKind> species) {
        return species ? ecosim::StyleOf(*species).color : VegetationColor;
    }

    /// @brief Nom d'une courbe dans la légende.
    std::string NameOf(const std::optional<ecosim::AnimalKind> species) {
        return species ? ecosim::SpeciesOf(*species).pluralName : std::string{"Total"};
    }

    /// @brief Position horizontale d'une génération dans la zone de tracé.
    int PointX(const UiRect plot, const std::size_t index, const std::size_t count) {
        if (count < 2) return plot.x;
        return plot.x + static_cast<int>(static_cast<long long>(index) * plot.width
                                         / static_cast<long long>(count - 1));
    }

    /// @brief Position verticale d'une valeur, 0 en bas et maximum en haut.
    int PointY(const UiRect plot, const double value, const long long maximum) {
        return plot.y + plot.height - static_cast<int>(value / static_cast<double>(maximum) * plot.height);
    }

    /// @brief Trace une courbe, interrompue là où la valeur n'existe pas.
    void DrawCurve(const HDC dc, const UiRect plot, const ecosim::ChartSeries& series,
                   const long long maximum) {
        const HPEN pen = CreatePen(PS_SOLID, 2, ToColorRef(ColorOf(series.species)));
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

    /// @brief Dessine un point rond sur une courbe.
    void DrawPoint(const HDC dc, const int x, const int y, const Color color) {
        const HBRUSH brush = CreateSolidBrush(ToColorRef(color));
        const HGDIOBJ previousBrush = SelectObject(dc, brush);
        const HGDIOBJ previousPen = SelectObject(dc, GetStockObject(NULL_PEN));
        Ellipse(dc, x - 5, y - 5, x + 6, y + 6);
        SelectObject(dc, previousPen);
        SelectObject(dc, previousBrush);
        DeleteObject(brush);
    }

    /// @brief Dessine un graphique complet : titre, légende, axes et courbes.
    /// @param canvas Image à compléter.
    /// @param panel Emplacement du graphique.
    /// @param history Mesures de toutes les générations.
    /// @param metric Statistique à tracer.
    /// @param selection Choix de l'utilisateur.
    /// @param shown Génération dont la légende donne les valeurs.
    /// @param hovered Vrai pour tracer le repère de survol.
    void DrawPanel(ecosim::Canvas& canvas, const UiRect panel,
                   const std::vector<ecosim::PopulationSample>& history,
                   const ecosim::ChartMetric metric, const ecosim::ChartSelection& selection,
                   const std::size_t shown, const bool hovered) {
        const HDC buffer = static_cast<HDC>(canvas.Context());
        const UiRect plot = ecosim::ChartPlot(panel);
        const std::vector<ecosim::ChartSeries> series = ecosim::BuildSeries(history, metric, selection);
        const long long maximum = ecosim::AxisMaximum(series);
        canvas.Fill(panel, ChartColor);
        canvas.Write(panel.x + 12, panel.y + 6, ecosim::ChartTitle(metric, selection.cumulative),
                     ecosim::WhiteText);

        // Légende avec la valeur actuelle, ou celle de la génération survolée.
        const auto entry = [&](const ecosim::ChartSeries& one, const bool named) {
            const std::string value = ecosim::FormatValue(metric, one.values[shown]);
            return named ? std::format("{} : {}", NameOf(one.species), value) : value;
        };
        // Sur un graphique étroit, les noms sont omis : la couleur désigne l'espèce.
        int needed = 12;
        for (const ecosim::ChartSeries& one : series) needed += 17 + canvas.Measure(entry(one, true)).cx + 22;
        const bool named = needed - 22 <= panel.width;
        int legendX = panel.x + 12;
        for (const ecosim::ChartSeries& one : series) {
            canvas.Fill({legendX, panel.y + 35, 11, 11}, ColorOf(one.species));
            const std::string label = entry(one, named);
            canvas.Write(legendX + 17, panel.y + 30, label, MutedInk);
            legendX += 17 + canvas.Measure(label).cx + 22;
        }
        if (series.empty()) canvas.Write(legendX, panel.y + 30, "Aucune espèce à afficher", MutedInk);

        for (int tick = 0; tick <= 4; ++tick) {
            const int y = plot.y + tick * plot.height / 4;
            canvas.Fill({plot.x, y, plot.width, 1}, {57, 66, 80});
            // Sur un petit graphique, une graduation sur deux évite les chevauchements.
            if (plot.height < 100 && tick % 2 != 0) continue;
            const std::string label = std::to_string(maximum * (4 - tick) / 4);
            canvas.Write(plot.x - 8 - canvas.Measure(label).cx, y - 9, label, MutedInk);
        }
        const std::string first = std::to_string(history.front().generation);
        const std::string last = std::format("génération {}", history.back().generation);
        canvas.Write(plot.x, plot.y + plot.height + 5, first, MutedInk);
        canvas.Write(plot.x + plot.width - canvas.Measure(last).cx, plot.y + plot.height + 5, last,
                     MutedInk);

        for (const ecosim::ChartSeries& one : series) DrawCurve(buffer, plot, one, maximum);
        if (hovered) {
            const int x = PointX(plot, shown, history.size());
            canvas.Fill({x, plot.y, 1, plot.height}, ecosim::HighlightColor);
        }
        for (const ecosim::ChartSeries& one : series) {
            if (!one.values[shown]) continue;
            DrawPoint(buffer, PointX(plot, shown, history.size()),
                      PointY(plot, *one.values[shown], maximum), ColorOf(one.species));
        }
    }
}

namespace ecosim {
    void DrawStatsScreen(Canvas& canvas, const SimulationSnapshot& simulation, const PlaybackState& playback,
                         const ChartSelection& selection, const std::optional<PointerPosition> pointer) {
        const std::vector<PopulationSample>& history = simulation.history;
        const int windowWidth = canvas.Width();
        const int windowHeight = canvas.Height();
        canvas.Fill({0, 0, windowWidth, windowHeight}, {24, 29, 38});
        canvas.SetFont(20);

        const auto button = [&](const UiRect area, const std::string& label) {
            canvas.SideButton(area, label, 7);
        };
        // Case à cocher : pleine et de la couleur de la série quand elle est affichée.
        const auto toggle = [&](const UiRect area, const std::string& label, const bool selected,
                                const Color color) {
            canvas.Fill(area, selected ? ButtonColor : ChartColor);
            const UiRect box{area.x + 10, area.y + 10, 14, 14};
            canvas.Fill(box, selected ? color : MutedInk);
            if (!selected) canvas.Fill({box.x + 2, box.y + 2, 10, 10}, ChartColor);
            canvas.Write(area.x + 32, area.y + 7, label, selected ? WhiteText : MutedInk);
        };

        button(BackButton(), "Retour");
        button(RestartButton(windowWidth), "Recommencer");
        if (!simulation.IsFinished()) {
            button(PauseButton(windowWidth, true), playback.paused ? "Reprendre" : "Pause");
            if (playback.paused) button(NextButton(windowWidth, true), "Génération +1");
        }
        canvas.SetFont(30, true);
        canvas.Write(155, 25, "Statistiques", WhiteText);
        canvas.SetFont(20);
        std::string status;
        if (simulation.IsFinished()) status = "Fin de la simulation : plus aucun animal";
        else if (playback.paused) status = std::format("En pause   Vitesse x{}", playback.speed);
        else status = std::format("Vitesse x{}", playback.speed);
        std::string counts = std::format("Génération {}", simulation.generation);
        for (const SpeciesInfo& species : SpeciesCatalog()) {
            counts += std::format("    {} {}", species.pluralName, simulation.Count(species.kind));
        }
        canvas.Write(20, 78, std::format("{}    {}", counts, status), WhiteText);

        canvas.SetFont(17);
        for (std::size_t i = 0; i < MetricCount; ++i) {
            toggle(MetricChip(windowWidth, static_cast<int>(i)), MetricNames()[i],
                   selection.metrics[i], HighlightColor);
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
            canvas.Write(area.x + 20, area.y + 40,
                         "Aucune statistique sélectionnée : cochez-en une ci-dessus.", MutedInk);
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
            canvas.Write(CumulativeButton().x + CumulativeButton().width + 24, CumulativeButton().y + 7,
                         std::format("Valeurs à la génération {}", history[shown].generation),
                         HighlightColor);
        }

        for (int i = 0; i < count; ++i) {
            DrawPanel(canvas, ChartPanel(windowWidth, windowHeight, i, count), history,
                      visible[static_cast<std::size_t>(i)], selection, shown, hovered.has_value());
        }
    }
}
