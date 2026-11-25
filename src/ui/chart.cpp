module ecosim.chart;

import std;

namespace {
    // Valeur d'une espèce à une génération, avant un éventuel cumul.
    std::optional<double> Measure(const ecosim::PopulationSample& sample,
                                  const ecosim::ChartMetric metric, const bool herbivore) {
        using ecosim::ChartMetric;
        const ecosim::LifeEvents& events = herbivore ? sample.herbivoreEvents : sample.predatorEvents;
        const std::size_t population = herbivore ? sample.herbivores : sample.predators;
        switch (metric) {
            case ChartMetric::Population: return static_cast<double>(population);
            case ChartMetric::Births: return static_cast<double>(events.births);
            case ChartMetric::Starved: return static_cast<double>(events.starved);
            case ChartMetric::OldAge: return static_cast<double>(events.diedOfAge);
            case ChartMetric::Eaten: return static_cast<double>(events.eaten);
            case ChartMetric::Energy:
                // Sans animal, il n'y a pas d'énergie moyenne : la courbe s'interrompt.
                if (population == 0) return std::nullopt;
                return herbivore ? sample.herbivoreEnergy : sample.predatorEnergy;
            case ChartMetric::Vegetation: return static_cast<double>(sample.vegetation);
        }
        return std::nullopt;
    }
}

namespace ecosim {
    const std::vector<std::string>& MetricNames() {
        static const std::vector<std::string> names{
            "Effectifs", "Naissances", "Morts de faim", "Vieillesse", "Mangés", "Énergie", "Végétation"};
        return names;
    }

    std::vector<ChartMetric> ChartSelection::VisibleMetrics() const {
        std::vector<ChartMetric> visible;
        for (std::size_t i = 0; i < MetricCount; ++i) {
            if (metrics[i]) visible.push_back(static_cast<ChartMetric>(i));
        }
        return visible;
    }

    bool IsEventMetric(const ChartMetric metric) {
        return metric == ChartMetric::Births || metric == ChartMetric::Starved
            || metric == ChartMetric::OldAge || metric == ChartMetric::Eaten;
    }

    std::string ChartTitle(const ChartMetric metric, const bool cumulative) {
        const std::string suffix = cumulative ? " (total)" : " (par génération)";
        switch (metric) {
            case ChartMetric::Population: return "Effectifs";
            case ChartMetric::Births: return "Naissances" + suffix;
            case ChartMetric::Starved: return "Morts de faim" + suffix;
            case ChartMetric::OldAge: return "Morts de vieillesse" + suffix;
            case ChartMetric::Eaten: return "Herbivores mangés" + suffix;
            case ChartMetric::Energy: return "Énergie moyenne";
            case ChartMetric::Vegetation: return "Végétation des prairies";
        }
        return {};
    }

    std::vector<ChartSeries> BuildSeries(const std::vector<PopulationSample>& history,
                                         const ChartMetric metric, const ChartSelection& selection) {
        std::vector<ChartSeries> result;
        const auto add = [&](const SeriesSource source) {
            ChartSeries series{source, {}};
            series.values.reserve(history.size());
            double total = 0.0;
            for (const PopulationSample& sample : history) {
                std::optional<double> value = Measure(sample, metric, source != SeriesSource::Predators);
                if (value && selection.cumulative && IsEventMetric(metric)) {
                    total += *value;
                    value = total;
                }
                series.values.push_back(value);
            }
            result.push_back(std::move(series));
        };
        if (metric == ChartMetric::Vegetation) {
            // La végétation n'appartient à aucune espèce : le filtre ne la masque pas.
            add(SeriesSource::Vegetation);
            return result;
        }
        if (selection.herbivores) add(SeriesSource::Herbivores);
        // Aucun animal ne mange les prédateurs.
        if (selection.predators && metric != ChartMetric::Eaten) add(SeriesSource::Predators);
        return result;
    }

    long long AxisMaximum(const std::vector<ChartSeries>& series) {
        double highest = 0.0;
        for (const ChartSeries& one : series) {
            for (const auto& value : one.values) {
                if (value) highest = std::max(highest, *value);
            }
        }
        // Pas autorisés : 1, 2, 2,5 ou 5 fois une puissance de dix, toujours entiers.
        for (long long power = 1;; power *= 10) {
            for (const long long tenths : {10LL, 20LL, 25LL, 50LL}) {
                if (tenths * power % 10 != 0) continue;
                const long long maximum = 4 * (tenths * power / 10);
                if (static_cast<double>(maximum) >= highest) return maximum;
            }
        }
    }

    std::string FormatValue(const ChartMetric metric, const std::optional<double> value) {
        if (!value) return "-";
        if (metric == ChartMetric::Energy) return std::format("{:.1f}", *value);
        return std::to_string(std::llround(*value));
    }

    std::size_t IndexAt(const int x, const int plotX, const int plotWidth, const std::size_t count) {
        if (count < 2 || plotWidth <= 0) return 0;
        const double ratio = std::clamp(static_cast<double>(x - plotX) / plotWidth, 0.0, 1.0);
        return static_cast<std::size_t>(std::llround(ratio * static_cast<double>(count - 1)));
    }
}
