// Formulaire des réglages de l'accueil : libellés, limites et saisie, sans API graphique.
export module ecosim.settings_form;

import std;
export import ecosim.settings;
export import ecosim.species_style;

export namespace ecosim {
    struct SettingField {
        std::size_t column; // 0 : terrain, puis une colonne par espèce
        std::string label;
        int minimum;
        int maximum;
        std::function<int&(SimulationSettings&)> value;
    };

    std::string ColumnTitle(std::size_t column);
    Color ColumnColor(std::size_t column);
    const std::vector<SettingField>& SettingFields();
    std::string PresetName(Preset preset);
    std::string PresetDescription(Preset preset);

    // Saisie d'une valeur au clavier. Elle n'est appliquée qu'à la validation.
    class SettingEditor {
    public:
        // La valeur actuelle est sélectionnée : le premier chiffre la remplace.
        void Begin(std::size_t field, SimulationSettings& settings);
        void Type(int digit);
        void Erase();
        // Ramène la valeur dans les bornes ; une saisie vide garde l'ancienne.
        void Commit(SimulationSettings& settings);
        void Cancel();

        std::optional<std::size_t> Field() const;
        const std::string& Text() const;
        bool IsSelected() const;

    private:
        std::optional<std::size_t> field;
        std::string text;
        bool selected{false};
    };
}
