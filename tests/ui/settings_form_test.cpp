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
        // La saisie s'arrete a la longueur de la limite haute.
        for (int digit = 0; digit < 9; ++digit) editor.Type(9);
        REQUIRE(editor.Text() == std::string(std::to_string(field.maximum).size(), '9'));
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

TEST_CASE("La taille de la carte a un minimum de 10 et accepte de grandes valeurs", "[settings]") {
    SimulationSettings settings = DefaultSettings(42, 26);
    const auto type = [&settings](const std::size_t field, const std::string& digits) {
        SettingEditor editor;
        editor.Begin(field, settings);
        for (const char digit : digits) editor.Type(digit - '0');
        editor.Commit(settings);
    };
    // Les champs 3 et 4 du terrain : largeur puis hauteur.
    REQUIRE(SettingFields()[3].label == "Largeur (X)");
    REQUIRE(SettingFields()[4].label == "Hauteur (Y)");
    type(3, "5");
    type(4, "2000");
    REQUIRE(settings.mapWidth == MinMapSize);
    REQUIRE(settings.mapHeight == 2000);
    REQUIRE(settings.species[AnimalKind::Deer].initialCount == MinMapSize * 2000 / 15);
}

TEST_CASE("Seule une grande carte declenche un avertissement", "[settings]") {
    REQUIRE(MapWarning(DefaultSettings(48, 27)).empty());
    REQUIRE_FALSE(MapWarning(DefaultSettings(200, 100)).empty());
    // La surface depasse la capacite d'un int sans la fausser.
    REQUIRE(MapWarning(DefaultSettings(999999, 999999)).find("999998000001") != std::string::npos);
}
