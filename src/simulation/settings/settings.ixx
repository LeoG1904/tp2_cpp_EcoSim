// Réglages d'une simulation et bornes autorisées, sans dépendance graphique.
export module ecosim.settings;

import std;
export import ecosim.predator;

export namespace ecosim {
    struct SimulationSettings {
        int waterPercent{10};
        int obstaclePercent{10};
        int vegetationGrowth{1};
        int initialHerbivores{12};
        int initialPredators{3};
        AnimalTraits herbivore{};
        AnimalTraits predator{Predator::DefaultTraits};
    };

    enum class SettingGroup { Terrain, Herbivores, Predators };

    struct SettingField {
        SettingGroup group;
        std::wstring_view label;
        int minimum;
        int maximum;
        int& (*value)(SimulationSettings&);
    };

    // Les populations de départ suivent la taille de la carte.
    SimulationSettings DefaultSettings(int width, int height);
    std::span<const SettingField> SettingFields();

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
        const std::wstring& Text() const;
        bool IsSelected() const;

    private:
        std::optional<std::size_t> field;
        std::wstring text;
        bool selected{false};
    };
}
