// Formulaire des réglages de l'accueil : libellés, limites et saisie, sans API graphique.
export module ecosim.settings_form;

import std;
export import ecosim.settings;

export namespace ecosim {
    enum class SettingGroup { Terrain, Herbivores, Predators };

    struct SettingField {
        SettingGroup group;
        std::string label;
        int minimum;
        int maximum;
        int& (*value)(SimulationSettings&);
    };

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
