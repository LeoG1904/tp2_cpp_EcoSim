module ecosim.settings_form;

import std;

namespace {
    using ecosim::SettingField;
    using ecosim::SettingGroup;
    using ecosim::SimulationSettings;

    // L'ordre des lignes est celui de l'affichage dans chaque colonne.
    const std::vector<SettingField> Fields{
        {SettingGroup::Terrain, "Eau (%)", 0, 50,
         [](SimulationSettings& s) -> int& { return s.waterPercent; }},
        {SettingGroup::Terrain, "Obstacles (%)", 0, 40,
         [](SimulationSettings& s) -> int& { return s.obstaclePercent; }},
        {SettingGroup::Terrain, "Repousse végétation", 0, 20,
         [](SimulationSettings& s) -> int& { return s.vegetationGrowth; }},
        {SettingGroup::Herbivores, "Nombre au départ", 0, 500,
         [](SimulationSettings& s) -> int& { return s.initialHerbivores; }},
        {SettingGroup::Herbivores, "Énergie de départ", 10, 100,
         [](SimulationSettings& s) -> int& { return s.herbivore.startingEnergy; }},
        {SettingGroup::Herbivores, "Énergie perdue / tour", 1, 20,
         [](SimulationSettings& s) -> int& { return s.herbivore.energyLoss; }},
        {SettingGroup::Herbivores, "Âge maximum", 10, 300,
         [](SimulationSettings& s) -> int& { return s.herbivore.maxAge; }},
        {SettingGroup::Herbivores, "Végétation mangée max", 1, 50,
         [](SimulationSettings& s) -> int& { return s.herbivore.foodEnergy; }},
        {SettingGroup::Herbivores, "Seuil de reproduction", 40, 100,
         [](SimulationSettings& s) -> int& { return s.herbivore.reproductionEnergy; }},
        {SettingGroup::Herbivores, "Chance de naissance (%)", 0, 100,
         [](SimulationSettings& s) -> int& { return s.herbivore.reproductionPercent; }},
        {SettingGroup::Predators, "Nombre au départ", 0, 200,
         [](SimulationSettings& s) -> int& { return s.initialPredators; }},
        {SettingGroup::Predators, "Énergie de départ", 10, 100,
         [](SimulationSettings& s) -> int& { return s.predator.startingEnergy; }},
        {SettingGroup::Predators, "Énergie perdue / tour", 1, 20,
         [](SimulationSettings& s) -> int& { return s.predator.energyLoss; }},
        {SettingGroup::Predators, "Âge maximum", 10, 300,
         [](SimulationSettings& s) -> int& { return s.predator.maxAge; }},
        {SettingGroup::Predators, "Énergie par proie", 5, 100,
         [](SimulationSettings& s) -> int& { return s.predator.foodEnergy; }},
        {SettingGroup::Predators, "Seuil de reproduction", 40, 100,
         [](SimulationSettings& s) -> int& { return s.predator.reproductionEnergy; }},
        {SettingGroup::Predators, "Chance de naissance (%)", 0, 100,
         [](SimulationSettings& s) -> int& { return s.predator.reproductionPercent; }},
    };
}

namespace ecosim {
    const std::vector<SettingField>& SettingFields() { return Fields; }

    void SettingEditor::Begin(const std::size_t index, SimulationSettings& settings) {
        text = std::to_string(SettingFields()[index].value(settings));
        field = index;
        selected = true;
    }

    void SettingEditor::Type(const int digit) {
        if (!field || digit < 0 || digit > 9) return;
        if (selected) text.clear();
        selected = false;
        // Trois chiffres suffisent pour la plus grande borne.
        if (text.size() < 3) text.push_back(static_cast<char>('0' + digit));
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
    const std::string& SettingEditor::Text() const { return text; }
    bool SettingEditor::IsSelected() const { return selected; }
}
