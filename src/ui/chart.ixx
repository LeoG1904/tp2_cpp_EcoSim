// Choix et données des graphiques de statistiques, sans API graphique.
export module ecosim.chart;

import std;
export import ecosim.stats;

export namespace ecosim {
    enum class ChartMetric { Population, Births, Starved, OldAge, Eaten, Energy, Vegetation };
    inline constexpr std::size_t MetricCount{7};

    // Noms des boutons, dans l'ordre de ChartMetric.
    const std::vector<std::string>& MetricNames();

    // Ce que l'utilisateur a choisi d'afficher.
    struct ChartSelection {
        std::array<bool, MetricCount> metrics{true, true, true, true, true, true, true};
        bool herbivores{true};
        bool predators{true};
        bool cumulative{true}; // événements : total depuis le début, ou par génération

        std::vector<ChartMetric> VisibleMetrics() const;
    };

    enum class SeriesSource { Herbivores, Predators, Vegetation };

    struct ChartSeries {
        SeriesSource source;
        // Une valeur par génération ; vide quand la mesure n'existe pas.
        std::vector<std::optional<double>> values;
    };

    // Les naissances et les morts sont les seules mesures qui se cumulent.
    bool IsEventMetric(ChartMetric metric);
    std::string ChartTitle(ChartMetric metric, bool cumulative);
    std::vector<ChartSeries> BuildSeries(const std::vector<PopulationSample>& history,
                                         ChartMetric metric, const ChartSelection& selection);
    // Haut de l'axe : un nombre rond, divisible en quatre graduations entières.
    long long AxisMaximum(const std::vector<ChartSeries>& series);
    std::string FormatValue(ChartMetric metric, std::optional<double> value);
    // Génération la plus proche d'une position horizontale du graphique.
    std::size_t IndexAt(int x, int plotX, int plotWidth, std::size_t count);
}
