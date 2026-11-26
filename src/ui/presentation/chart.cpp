/**
 * @file
 * @brief Description de chaque statistique et calcul des courbes.
 */
module ecosim.chart;

import std;
import ecosim.species;

namespace {
    using ecosim::PopulationSample;
    using ecosim::SpeciesSample;
    using Reading = std::optional<double>;

    // Tout ce qui définit une statistique : l'ajouter revient à ajouter une ligne.
    struct MetricInfo {
        std::string button;
        std::string title;
        bool isEvent;      // les naissances et les morts peuvent se cumuler
        bool onlyEaten;    // n'a de sens que pour les espèces qui sont des proies
        // Mesure d'une espèce, ou nullptr pour une mesure du monde.
        Reading (*ofSpecies)(const SpeciesSample&);
        Reading (*ofWorld)(const PopulationSample&);
    };

    Reading Number(const std::size_t value) { return static_cast<double>(value); }

    // Dans l'ordre de ChartMetric.
    const MetricInfo& InfoOf(const ecosim::ChartMetric metric) {
        static const std::vector<MetricInfo> metrics{
            {"Effectifs", "Effectifs", false, false,
             [](const SpeciesSample& s) { return Number(s.population); }, nullptr},
            {"Naissances", "Naissances", true, false,
             [](const SpeciesSample& s) { return Number(s.events.births); }, nullptr},
            {"Morts de faim", "Morts de faim", true, false,
             [](const SpeciesSample& s) { return Number(s.events.starved); }, nullptr},
            {"Vieillesse", "Morts de vieillesse", true, false,
             [](const SpeciesSample& s) { return Number(s.events.diedOfAge); }, nullptr},
            {"Mangés", "mangés", true, true,
             [](const SpeciesSample& s) { return Number(s.events.eaten); }, nullptr},
            // Sans animal, il n'y a pas d'énergie moyenne : la courbe s'interrompt.
            {"Énergie", "Énergie moyenne", false, false,
             [](const SpeciesSample& s) { return s.population == 0 ? Reading{} : Reading{s.energy}; },
             nullptr},
            {"Végétation", "Végétation des prairies", false, false, nullptr,
             [](const PopulationSample& s) { return Reading{static_cast<double>(s.vegetation)}; }},
        };
        return metrics.at(static_cast<std::size_t>(metric));
    }
}

namespace ecosim {
    const std::vector<std::string>& MetricNames() {
        static const std::vector<std::string> names = [] {
            std::vector<std::string> result;
            for (std::size_t i = 0; i < MetricCount; ++i) {
                result.push_back(InfoOf(static_cast<ChartMetric>(i)).button);
            }
            return result;
        }();
        return names;
    }

    std::vector<ChartMetric> ChartSelection::VisibleMetrics() const {
        std::vector<ChartMetric> visible;
        for (std::size_t i = 0; i < MetricCount; ++i) {
            if (metrics[i]) visible.push_back(static_cast<ChartMetric>(i));
        }
        return visible;
    }

    bool IsEventMetric(const ChartMetric metric) { return InfoOf(metric).isEvent; }

    std::string ChartTitle(const ChartMetric metric, const bool cumulative) {
        const MetricInfo& info = InfoOf(metric);
        std::string title = info.title;
        if (info.onlyEaten) {
            // Le titre nomme les proies d'après le catalogue.
            std::string prey;
            for (const SpeciesInfo& species : SpeciesCatalog()) {
                if (species.canBeEaten) prey += (prey.empty() ? "" : ", ") + species.pluralName;
            }
            title = prey + " " + title;
        }
        if (!info.isEvent) return title;
        return title + (cumulative ? " (total)" : " (par génération)");
    }

    std::vector<ChartSeries> BuildSeries(const std::vector<PopulationSample>& history,
                                         const ChartMetric metric, const ChartSelection& selection) {
        const MetricInfo& info = InfoOf(metric);
        std::vector<ChartSeries> result;
        const auto add = [&](const std::optional<AnimalKind> kind) {
            ChartSeries series{kind, {}};
            series.values.reserve(history.size());
            double total = 0.0;
            for (const PopulationSample& sample : history) {
                Reading value = kind ? info.ofSpecies(sample.species[*kind]) : info.ofWorld(sample);
                if (value && selection.cumulative && info.isEvent) {
                    total += *value;
                    value = total;
                }
                series.values.push_back(value);
            }
            result.push_back(std::move(series));
        };
        if (!info.ofSpecies) {
            // Une mesure du monde n'appartient à aucune espèce : le filtre ne la masque pas.
            add(std::nullopt);
            return result;
        }
        for (const SpeciesInfo& species : SpeciesCatalog()) {
            if (selection.hidden[species.kind] || (info.onlyEaten && !species.canBeEaten)) continue;
            add(species.kind);
        }
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
