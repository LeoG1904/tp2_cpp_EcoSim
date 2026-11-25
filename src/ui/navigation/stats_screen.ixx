// Écran des statistiques : ouverture et choix des graphiques.
export module ecosim.stats_screen;

import std;
export import ecosim.input;
export import ecosim.chart;

export namespace ecosim {
    class StatsScreen {
    public:
        explicit StatsScreen(int width);

        bool IsOpen() const;
        void Open();
        void Close();
        // Clic à l'intérieur de l'écran ouvert.
        void Click(PointerPosition mouse);
        // Conservé d'une simulation à l'autre : c'est une préférence d'affichage.
        const ChartSelection& Selection() const;

    private:
        int width;
        bool open{false};
        ChartSelection selection;
    };
}
