module ecosim.settings;

import std;

namespace {
    using ecosim::SettingField;
    using ecosim::SettingGroup;
    using ecosim::SimulationSettings;

    // L'ordre des lignes est celui de l'affichage dans chaque colonne.
    const std::array<SettingField, 17> Fields{{
        {SettingGroup::Terrain, L"Eau (%)", 0, 50,
         [](SimulationSettings& s) -> int& { return s.waterPercent; }},
        {SettingGroup::Terrain, L"Obstacles (%)", 0, 40,
         [](SimulationSettings& s) -> int& { return s.obstaclePercent; }},
        {SettingGroup::Terrain, L"Repousse végétation", 0, 20,
         [](SimulationSettings& s) -> int& { return s.vegetationGrowth; }},
        {SettingGroup::Herbivores, L"Nombre au départ", 0, 500,
         [](SimulationSettings& s) -> int& { return s.initialHerbivores; }},
        {SettingGroup::Herbivores, L"Énergie de départ", 10, 100,
         [](SimulationSettings& s) -> int& { return s.herbivore.startingEnergy; }},
        {SettingGroup::Herbivores, L"Énergie perdue / tour", 1, 20,
         [](SimulationSettings& s) -> int& { return s.herbivore.energyLoss; }},
        {SettingGroup::Herbivores, L"Âge maximum", 10, 300,
         [](SimulationSettings& s) -> int& { return s.herbivore.maxAge; }},
        {SettingGroup::Herbivores, L"Végétation mangée max", 1, 50,
         [](SimulationSettings& s) -> int& { return s.herbivore.foodEnergy; }},
        {SettingGroup::Herbivores, L"Seuil de reproduction", 40, 100,
         [](SimulationSettings& s) -> int& { return s.herbivore.reproductionEnergy; }},
        {SettingGroup::Herbivores, L"Chance de naissance (%)", 0, 100,
         [](SimulationSettings& s) -> int& { return s.herbivore.reproductionPercent; }},
        {SettingGroup::Predators, L"Nombre au départ", 0, 200,
         [](SimulationSettings& s) -> int& { return s.initialPredators; }},
        {SettingGroup::Predators, L"Énergie de départ", 10, 100,
         [](SimulationSettings& s) -> int& { return s.predator.startingEnergy; }},
        {SettingGroup::Predators, L"Énergie perdue / tour", 1, 20,
         [](SimulationSettings& s) -> int& { return s.predator.energyLoss; }},
        {SettingGroup::Predators, L"Âge maximum", 10, 300,
         [](SimulationSettings& s) -> int& { return s.predator.maxAge; }},
        {SettingGroup::Predators, L"Énergie par proie", 5, 100,
         [](SimulationSettings& s) -> int& { return s.predator.foodEnergy; }},
        {SettingGroup::Predators, L"Seuil de reproduction", 40, 100,
         [](SimulationSettings& s) -> int& { return s.predator.reproductionEnergy; }},
        {SettingGroup::Predators, L"Chance de naissance (%)", 0, 100,
         [](SimulationSettings& s) -> int& { return s.predator.reproductionPercent; }},
    }};
}

namespace ecosim {
    SimulationSettings DefaultSettings(const int width, const int height) {
        SimulationSettings settings;
        settings.initialHerbivores = std::max(12, width * height / 15);
        settings.initialPredators = std::max(3, width * height / 150);
        return settings;
    }

    std::span<const SettingField> SettingFields() { return Fields; }

    void SettingEditor::Begin(const std::size_t index, SimulationSettings& settings) {
        text = std::to_wstring(SettingFields()[index].value(settings));
        field = index;
        selected = true;
    }

    void SettingEditor::Type(const int digit) {
        if (!field || digit < 0 || digit > 9) return;
        if (selected) text.clear();
        selected = false;
        // Trois chiffres suffisent pour la plus grande borne.
        if (text.size() < 3) text.push_back(static_cast<wchar_t>(L'0' + digit));
    }

    void SettingEditor::Erase() {
        if (selected) text.clear();
        else if (!text.empty()) text.pop_back();
        selected = false;
    }

    void SettingEditor::Commit(SimulationSettings& settings) {
        if (field && !text.empty()) {
            const SettingField& target = SettingFields()[*field];
            target.value(settings) = std::clamp(std::stoi(text), target.minimum, target.maximum);
        }
        Cancel();
    }

    void SettingEditor::Cancel() {
        field.reset();
        text.clear();
        selected = false;
    }

    std::optional<std::size_t> SettingEditor::Field() const { return field; }
    const std::wstring& SettingEditor::Text() const { return text; }
    bool SettingEditor::IsSelected() const { return selected; }
}
