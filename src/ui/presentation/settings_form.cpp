/**
 * @file
 * @brief Liste des réglages et saisie.
 */
module ecosim.settings_form;

import std;

namespace {
    using ecosim::SettingField;
    using ecosim::SimulationSettings;

    /// @brief Construit la liste des réglages : le terrain, puis les mêmes lignes pour chaque espèce.
    std::vector<SettingField> BuildFields() {
        std::vector<SettingField> fields{
            {0, "Eau (%)", 0, 50, [](SimulationSettings& s) -> int& { return s.waterPercent; }},
            {0, "Obstacles (%)", 0, 40,
             [](SimulationSettings& s) -> int& { return s.obstaclePercent; }},
            {0, "Repousse végétation", 0, 20,
             [](SimulationSettings& s) -> int& { return s.vegetationGrowth; }},
            {0, "Largeur (X)", ecosim::MinMapSize, ecosim::InputLimit,
             [](SimulationSettings& s) -> int& { return s.mapWidth; },
             [](SimulationSettings& s, const int width) { ecosim::ResizeMap(s, width, s.mapHeight); }},
            {0, "Hauteur (Y)", ecosim::MinMapSize, ecosim::InputLimit,
             [](SimulationSettings& s) -> int& { return s.mapHeight; },
             [](SimulationSettings& s, const int height) { ecosim::ResizeMap(s, s.mapWidth, height); }},
        };
        for (const ecosim::SpeciesInfo& info : ecosim::SpeciesCatalog()) {
            const ecosim::AnimalKind kind = info.kind;
            const ecosim::SpeciesStyle& style = ecosim::StyleOf(kind);
            const std::size_t column = 1 + static_cast<std::size_t>(kind);
            fields.push_back({column, "Nombre au départ", 0, ecosim::InputLimit,
                [kind](SimulationSettings& s) -> int& { return s.species[kind].initialCount; }});
            fields.push_back({column, "Chance de naissance (%)", 0, 100,
                [kind](SimulationSettings& s) -> int& {
                    return s.species[kind].options.birthPercent;
                }});
            fields.push_back({column, style.instinctLabel, 0, style.instinctMaximum,
                [kind](SimulationSettings& s) -> int& { return s.species[kind].options.instinct; }});
        }
        return fields;
    }
}

namespace ecosim {
    const std::vector<SettingField>& SettingFields() {
        static const std::vector<SettingField> fields = BuildFields();
        return fields;
    }

    std::string ColumnTitle(const std::size_t column) {
        if (column == 0) return "Terrain";
        return SpeciesCatalog().at(column - 1).pluralName;
    }

    Color ColumnColor(const std::size_t column) {
        if (column == 0) return {120, 200, 130};
        return StyleOf(SpeciesCatalog().at(column - 1).kind).color;
    }

    std::string PresetName(const Preset preset) {
        switch (preset) {
            case Preset::Balance: return "Équilibre";
            case Preset::Famine: return "Famine";
            case Preset::Invasion: return "Invasion";
        }
        return {};
    }

    std::string PresetDescription(const Preset preset) {
        switch (preset) {
            case Preset::Balance: return "Les quatre espèces coexistent le plus souvent longtemps.";
            case Preset::Famine: return "La végétation ne repousse plus : les herbivores meurent de faim.";
            case Preset::Invasion: return "Des meutes massives prennent les herbivores par surprise et les déciment.";
        }
        return {};
    }

    std::string MapWarning(const SimulationSettings& settings) {
        const long long cells = static_cast<long long>(settings.mapWidth) * settings.mapHeight;
        if (cells < LargeMapCells) return {};
        return std::format("Grande carte ({} cases) : la simulation et la 3D risquent d'être lentes.", cells);
    }

    void SettingEditor::Begin(const std::size_t index, SimulationSettings& settings) {
        text = std::to_string(SettingFields()[index].value(settings));
        field = index;
        selected = true;
    }

    void SettingEditor::Type(const int digit) {
        if (!field || digit < 0 || digit > 9) return;
        if (selected) text.clear();
        selected = false;
        // Pas plus de chiffres que n'en a la limite haute.
        if (text.size() < std::to_string(SettingFields()[*field].maximum).size()) {
            text.push_back(static_cast<char>('0' + digit));
        }
    }

    void SettingEditor::Erase() {
        if (selected) text.clear();
        else if (!text.empty()) text.pop_back();
        selected = false;
    }

    void SettingEditor::Commit(SimulationSettings& settings) {
        if (field && !text.empty()) {
            const SettingField& target = SettingFields()[*field];
            const int value = std::clamp(std::stoi(text), target.minimum, target.maximum);
            if (target.apply) target.apply(settings, value);
            else target.value(settings) = value;
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
