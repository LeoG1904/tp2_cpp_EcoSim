#include <catch2/catch_all.hpp>

import std;
import ecosim.layout;
import ecosim.controller;

using namespace ecosim;

namespace {
    PointerPosition Inside(const UiRect area) { return {area.x + 1, area.y + 1}; }
}

// Chaque role se teste sans les autres.
TEST_CASE("La confirmation fonctionne seule", "[navigation]") {
    ExitDialog dialog{1920, 1080};
    REQUIRE(dialog.Toggle() == UiAction::None);
    REQUIRE(dialog.IsOpen());
    REQUIRE(dialog.Click({0, 0}) == UiAction::None);
    REQUIRE(dialog.Click(Inside(ExitMenuButton(1920, 1080))) == UiAction::ReturnHome);
    REQUIRE_FALSE(dialog.IsOpen());
}

TEST_CASE("Chaque vue de la carte annonce son bouton et ses capacites", "[navigation]") {
    MapNavigator map{1920, 1080, 40};
    REQUIRE(map.ActiveKind() == MapViewKind::Flat);
    REQUIRE(map.NextView().ButtonLabel() == "Vue 3D");
    REQUIRE_FALSE(map.ActiveView().CanRecenter());

    map.Click(Inside(ViewButton(1920)));
    REQUIRE(map.ActiveKind() == MapViewKind::Orbit);
    REQUIRE(map.NextView().ButtonLabel() == "Vue 2D");
    REQUIRE(map.ActiveView().CanRecenter());
}
