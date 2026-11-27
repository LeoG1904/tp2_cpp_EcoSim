/**
 * @file
 * @brief Les données des graphiques de statistiques.
 */
export module ecosim.chart;

import std;
export import ecosim.stats;
import ecosim.species;

export namespace ecosim {
    /// @brief Les statistiques que l'on peut afficher.
    /// @ingroup interface
    enum class ChartMetric {
        Population, ///< Effectifs.
        Births, ///< Naissances.
        Starved, ///< Morts de faim.
        Parched, ///< Morts de soif.
        OldAge, ///< Morts de vieillesse.
        Eaten, ///< Proies mangées.
        Energy, ///< Énergie moyenne.
        Vegetation ///< Herbe totale des prairies.
    };
    /// @brief Nombre de statistiques.
    /// @ingroup interface
    inline constexpr std::size_t MetricCount{8};

    /// @brief Les noms des boutons, dans l'ordre de ChartMetric.
    /// @ingroup interface
    const std::vector<std::string>& MetricNames();

    /// @brief Ce que l'utilisateur a choisi d'afficher.
    /// @ingroup interface
    struct ChartSelection {
        /// @brief Graphiques affichés.
        std::array<bool, MetricCount> metrics{true, true, true, true, true, true, true, true};
        /// @brief Espèces masquées.
        PerSpecies<bool> hidden{};
        /// @brief Naissances et morts : total depuis le début, ou par génération.
        bool cumulative{true};

        /// @brief Les statistiques affichées.
        std::vector<ChartMetric> VisibleMetrics() const;
    };

    /// @brief Une courbe.
    /// @ingroup interface
    struct ChartSeries {
        /// @brief Espèce ; vide pour une mesure du monde, comme l'herbe.
        std::optional<AnimalKind> species;
        /// @brief Une valeur par génération ; vide quand la mesure n'existe pas.
        std::vector<std::optional<double>> values;
    };

    /// @brief Vrai pour les naissances et les morts, les seules mesures cumulables.
    /// @ingroup interface
    bool IsEventMetric(ChartMetric metric);
    /// @brief Le titre d'un graphique.
    /// @ingroup interface
    std::string ChartTitle(ChartMetric metric, bool cumulative);
    /// @brief Prépare les courbes d'une statistique à partir de l'historique.
    /// @ingroup interface
    std::vector<ChartSeries> BuildSeries(const std::vector<PopulationSample>& history,
                                         ChartMetric metric, const ChartSelection& selection);
    /// @brief Le haut de l'axe : un nombre rond, divisible en quatre graduations entières.
    /// @ingroup interface
    long long AxisMaximum(const std::vector<ChartSeries>& series);
    /// @brief Met une valeur en texte ; « - » si elle n'existe pas.
    /// @ingroup interface
    std::string FormatValue(ChartMetric metric, std::optional<double> value);
    /// @brief La génération la plus proche d'une position horizontale de la souris.
    /// @ingroup interface
    std::size_t IndexAt(int x, int plotX, int plotWidth, std::size_t count);
}
