#include <catch2/catch_all.hpp>

import std;
import ecosim.settings_form;

using namespace ecosim;

TEST_CASE("Taper une valeur remplace l'ancienne apres validation", "[settings]") {
    SimulationSettings settings;
    SettingEditor editor;
    editor.Begin(0, settings); // eau
    REQUIRE(editor.Text() == "10");
    REQUIRE(editor.IsSelected());

    editor.Type(2);
    editor.Type(5);
    REQUIRE(editor.Text() == "25");
    REQUIRE(settings.waterPercent == 10);

    editor.Commit(settings);
    REQUIRE(settings.waterPercent == 25);
    REQUIRE_FALSE(editor.Field().has_value());
}

TEST_CASE("Une saisie hors limites est ramenee a la limite", "[settings]") {
    SimulationSettings settings;
    SettingEditor editor;
    for (std::size_t i = 0; i < SettingFields().size(); ++i) {
        const SettingField& field = SettingFields()[i];
        editor.Begin(i, settings);
        for (int digit = 0; digit < 6; ++digit) editor.Type(9);
        REQUIRE(editor.Text() == "999");
        editor.Commit(settings);
        REQUIRE(field.value(settings) == field.maximum);

        editor.Begin(i, settings);
        editor.Type(0);
        editor.Commit(settings);
        REQUIRE(field.value(settings) == field.minimum);
    }
}

TEST_CASE("Effacer, laisser vide ou annuler une saisie", "[settings]") {
    SimulationSettings settings;
    SettingEditor editor;
    editor.Begin(0, settings);
    editor.Type(3);
    editor.Type(4);
    editor.Erase();
    REQUIRE(editor.Text() == "3");

    editor.Erase();
    editor.Commit(settings);
    REQUIRE(settings.waterPercent == 10);

    editor.Begin(0, settings);
    editor.Type(4);
    editor.Cancel();
    editor.Commit(settings);
    REQUIRE(settings.waterPercent == 10);
}
