#include <catch2/catch_all.hpp>

import std;
import ecosim.species;
import ecosim.settings_form;

using namespace ecosim;

TEST_CASE("Le catalogue decrit chaque espece de facon coherente", "[catalog]") {
    REQUIRE(SpeciesCatalog().size() == SpeciesCount);
    for (std::size_t i = 0; i < SpeciesCount; ++i) {
        const SpeciesInfo& info = SpeciesCatalog()[i];
        REQUIRE(static_cast<std::size_t>(info.kind) == i);
        REQUIRE_FALSE(info.name.empty());
        const auto animal = info.create(1, {0, 0}, 50, {info.defaultBirthPercent});
        REQUIRE(animal->GetKind() == info.kind);
        REQUIRE(animal->IsPrey() == info.canBeEaten);
        REQUIRE(animal->GetOptions().birthPercent == info.defaultBirthPercent);
    }
}

TEST_CASE("Le formulaire cree une colonne et les memes lignes pour chaque espece", "[catalog]") {
    std::array<int, 1 + SpeciesCount> rows{};
    for (const SettingField& field : SettingFields()) ++rows.at(field.column);
    for (std::size_t column = 1; column <= SpeciesCount; ++column) {
        REQUIRE(rows[column] == 3);
        REQUIRE(ColumnTitle(column) == SpeciesCatalog()[column - 1].pluralName);
    }
}
