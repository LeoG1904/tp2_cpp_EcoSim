/**
 * @file
 * @brief Liste des réglages et saisie.
 */
module ecosim.settings_form;

import std;

namespace {
    using ecosim::SettingField;
    using ecosim::SimulationSettings;

    // Les mêmes lignes sont créées pour chaque espèce du catalogue.
    std::vector<SettingField> BuildFields() {
        std::vector<SettingField> fields{
            {0, "Eau (%)", 0, 50, [](SimulationSettings& s) -> int& { return s.waterPercent; }},
            {0, "Obstacles (%)", 0, 40,
             [](SimulationSettings& s) -> int& { return s.obstaclePercent; }},
            {0, "Repousse végétation", 0, 20,
             [](SimulationSettings& s) -> int& { return s.vegetationGrowth; }},
        };
        for (const ecosim::SpeciesInfo& info : ecosim::SpeciesCatalog()) {
            const ecosim::AnimalKind kind = info.kind;
            const ecosim::SpeciesStyle& style = ecosim::StyleOf(kind);
            const std::size_t column = 1 + static_cast<std::size_t>(kind);
            const auto trait = [&](const std::string& label, const int minimum, const int maximum,
                                   int ecosim::AnimalTraits::* member) {
                fields.push_back({column, label, minimum, maximum,
                    [kind, member](SimulationSettings& s) -> int& {
                        return s.species[kind].traits.*member;
                    }});
            };
            fields.push_back({column, "Nombre au départ", 0, style.initialMaximum,
                [kind](SimulationSettings& s) -> int& { return s.species[kind].initialCount; }});
            trait("Vitesse (% case / tour)", 20, 300, &ecosim::AnimalTraits::speed);
            trait("Énergie de départ", 10, 100, &ecosim::AnimalTraits::startingEnergy);
            trait("Énergie perdue / tour", 1, 20, &ecosim::AnimalTraits::energyLoss);
            trait("Âge maximum", 10, 300, &ecosim::AnimalTraits::maxAge);
            trait(style.foodLabel, style.foodMinimum, style.foodMaximum,
                  &ecosim::AnimalTraits::foodEnergy);
            trait("Seuil de reproduction", 40, 100, &ecosim::AnimalTraits::reproductionEnergy);
            trait("Chance de naissance (%)", 0, 100, &ecosim::AnimalTraits::reproductionPercent);
            trait(style.instinctLabel, 0, style.instinctMaximum, &ecosim::AnimalTraits::instinct);
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
            case Preset::Balance: return "Les deux espèces coexistent longtemps.";
            case Preset::Famine: return "La végétation ne repousse plus : les herbivores meurent de faim.";
            case Preset::Invasion: return "Des meutes massives prennent les herbivores par surprise et les déciment.";
        }
        return {};
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
