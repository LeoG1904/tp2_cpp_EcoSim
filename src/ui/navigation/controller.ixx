// Aiguillage : confie chaque entrée à l'écran concerné, sans API graphique.
export module ecosim.controller;

import std;
export import ecosim.input;
export import ecosim.home_controller;
export import ecosim.exit_dialog;
export import ecosim.stats_screen;
export import ecosim.map_navigator;

export namespace ecosim {
    class UiController {
    public:
        // cellSize : côté d'une case de la carte, en pixels.
        UiController(int width, int height, int cellSize);

        // Sur l'accueil, la saisie modifie directement les réglages.
        UiAction Handle(const RawInput& input, bool isHomeScreen, bool paused,
                        SimulationSettings& settings);

        // État de chaque écran, en lecture seule pour le rendu.
        const HomeController& Home() const;
        const ExitDialog& Dialog() const;
        const StatsScreen& Stats() const;
        const MapNavigator& Map() const;
        std::optional<PointerPosition> Pointer() const;

    private:
        UiAction HandleEscape(bool isHomeScreen);
        UiAction HandleKeys(const RawInput& input, bool paused) const;
        UiAction HandleClick(PointerPosition mouse, bool isHomeScreen, bool paused,
                             SimulationSettings& settings);
        UiAction HandleSimulationClick(PointerPosition mouse, bool paused);

        int width;
        HomeController home;
        ExitDialog dialog;
        StatsScreen stats;
        MapNavigator map;
        std::optional<PointerPosition> pointer;
    };
}
