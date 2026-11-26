#include <catch2/catch_all.hpp>

import std;
import ecosim.layout;
import ecosim.chart;
import ecosim.controller;

using namespace ecosim;

namespace {
    std::vector<PopulationSample> History() {
        std::vector<PopulationSample> history{{0}, {1}, {2}};
        const std::array<std::size_t, 3> herbivores{10, 12, 0};
        const std::array<std::size_t, 3> predators{2, 2, 1};
        for (std::size_t i = 0; i < history.size(); ++i) {
            history[i].species[AnimalKind::Herbivore].population = herbivores[i];
            history[i].species[AnimalKind::Predator].population = predators[i];
        }
        history[0].species[AnimalKind::Herbivore].energy = 50.0;
        history[1].species[AnimalKind::Herbivore].energy = 55.5;
        history[1].species[AnimalKind::Herbivore].events.births = 3;
        history[2].species[AnimalKind::Herbivore].events.births = 4;
        history[2].vegetation = 900;
        return history;
    }

    RawInput Click(const UiRect area) {
        RawInput input;
        input.click = PointerPosition{area.x + 1, area.y + 1};
        return input;
    }
}

TEST_CASE("Les evenements s'affichent en total ou par generation", "[chart]") {
    ChartSelection selection;
    auto series = BuildSeries(History(), ChartMetric::Births, selection);
    REQUIRE(series.size() == 2);
    REQUIRE(series[0].values == std::vector<std::optional<double>>{0.0, 3.0, 7.0});

    selection.cumulative = false;
    series = BuildSeries(History(), ChartMetric::Births, selection);
    REQUIRE(series[0].values == std::vector<std::optional<double>>{0.0, 3.0, 4.0});

    // Les effectifs ne sont jamais cumules.
    selection.cumulative = true;
    series = BuildSeries(History(), ChartMetric::Population, selection);
    REQUIRE(series[0].values == std::vector<std::optional<double>>{10.0, 12.0, 0.0});
}

TEST_CASE("Le filtre d'espece, les mangés et la vegetation", "[chart]") {
    ChartSelection selection;
    REQUIRE(BuildSeries(History(), ChartMetric::Eaten, selection).size() == 1);

    selection.hidden[AnimalKind::Herbivore] = true;
    const auto population = BuildSeries(History(), ChartMetric::Population, selection);
    REQUIRE(population.size() == 1);
    REQUIRE(population[0].species == AnimalKind::Predator);
    REQUIRE(BuildSeries(History(), ChartMetric::Eaten, selection).empty());

    selection.hidden[AnimalKind::Predator] = true;
    const auto vegetation = BuildSeries(History(), ChartMetric::Vegetation, selection);
    REQUIRE(vegetation.size() == 1);
    REQUIRE(vegetation[0].values.back() == 900.0);
}

TEST_CASE("Sans animal, l'energie moyenne n'a pas de valeur", "[chart]") {
    const auto series = BuildSeries(History(), ChartMetric::Energy, ChartSelection{});
    REQUIRE(series[0].values[1] == 55.5);
    REQUIRE_FALSE(series[0].values[2].has_value());
    REQUIRE(FormatValue(ChartMetric::Energy, series[0].values[1]) == "55.5");
    REQUIRE(FormatValue(ChartMetric::Energy, series[0].values[2]) == "-");
}

TEST_CASE("L'axe monte jusqu'a un nombre rond", "[chart]") {
    const auto maximum = [](const double value) {
        return AxisMaximum({{AnimalKind::Herbivore, {value}}});
    };
    REQUIRE(maximum(0.0) == 4);
    REQUIRE(maximum(5.0) == 8);
    REQUIRE(maximum(100.0) == 100);
    REQUIRE(maximum(893.0) == 1000);
    REQUIRE(maximum(690.0) == 800);
    REQUIRE(maximum(66098.0) == 80000);
}

TEST_CASE("La souris designe la generation la plus proche", "[chart]") {
    REQUIRE(IndexAt(100, 100, 400, 5) == 0);
    REQUIRE(IndexAt(290, 100, 400, 5) == 2);
    REQUIRE(IndexAt(900, 100, 400, 5) == 4);
    REQUIRE(IndexAt(300, 100, 400, 1) == 0);
}

TEST_CASE("Les cases a cocher choisissent les graphiques", "[chart]") {
    UiController controller{1920, 1080, 40};
    SimulationSettings settings;
    // Sur la carte, ces positions ne changent rien.
    controller.Handle(Click(MetricChip(1920, 1)), false, false, settings);
    REQUIRE(controller.Charts().VisibleMetrics().size() == MetricCount);

    controller.Handle(Click(GraphButton(1920)), false, false, settings);
    controller.Handle(Click(MetricChip(1920, 1)), false, false, settings);
    REQUIRE(controller.Charts().VisibleMetrics().size() == MetricCount - 1);
    REQUIRE_FALSE(controller.Charts().metrics[1]);

    controller.Handle(Click(SpeciesChip(1)), false, false, settings);
    controller.Handle(Click(CumulativeButton()), false, false, settings);
    REQUIRE(controller.Charts().hidden[AnimalKind::Predator]);
    REQUIRE_FALSE(controller.Charts().cumulative);

    // Le choix est conserve apres un redemarrage.
    controller.Handle(Click(RestartButton(1920)), false, false, settings);
    REQUIRE_FALSE(controller.Charts().metrics[1]);
}

TEST_CASE("Les graphiques restent dans l'ecran sans se chevaucher", "[chart]") {
    for (const auto& [width, height] : {std::pair{1280, 720}, std::pair{1920, 1080}}) {
        for (int count = 1; count <= static_cast<int>(MetricCount); ++count) {
            for (int i = 0; i < count; ++i) {
                const UiRect panel = ChartPanel(width, height, i, count);
                REQUIRE(panel.x >= 0);
                REQUIRE(panel.x + panel.width <= width);
                REQUIRE(panel.y + panel.height <= height);
                REQUIRE(ChartPlot(panel).height >= 40);
                for (int other = 0; other < i; ++other) {
                    const UiRect b = ChartPanel(width, height, other, count);
                    const bool apart = panel.x >= b.x + b.width || b.x >= panel.x + panel.width
                                    || panel.y >= b.y + b.height || b.y >= panel.y + panel.height;
                    REQUIRE(apart);
                }
            }
        }
        REQUIRE(MetricChip(width, 6).x + MetricChip(width, 6).width <= width);
    }
}
