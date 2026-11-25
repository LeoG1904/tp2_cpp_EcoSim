// Accueil : saisie des réglages, préréglages et démarrage.
export module ecosim.home_controller;

import std;
export import ecosim.input;
export import ecosim.settings_form;

export namespace ecosim {
    class HomeController {
    public:
        HomeController(int width, int height, int cellSize);

        void Type(const std::vector<KeyStroke>& typed, SimulationSettings& settings);
        // Annule la saisie en cours ; false s'il n'y en avait pas.
        bool CancelEditing();
        UiAction Click(PointerPosition mouse, SimulationSettings& settings);
        const SettingEditor& Editor() const;

    private:
        // Renvoie true si le clic visait une case de valeur.
        bool ClickValue(PointerPosition mouse, SimulationSettings& settings);

        int width;
        int height;
        int cellSize;
        SettingEditor editor;
    };
}
