#include <catch2/catch_all.hpp>

import std;
import ecosim.layout;
import ecosim.camera;
import ecosim.controller;

using namespace ecosim;

namespace {
    constexpr Viewport Screen{1920, 1080};

    RawInput Click(const int x, const int y) {
        RawInput input;
        input.click = PointerPosition{x, y};
        return input;
    }
}

TEST_CASE("Le centre de la carte est au centre de l'ecran", "[camera]") {
    const OrbitCamera camera{48, 27};
    const auto centre = camera.Project({24.0, 0.0, 13.5}, Screen);
    REQUIRE(centre.has_value());
    REQUIRE(centre->x == 960);
    REQUIRE(centre->y == 540);
}

TEST_CASE("Cliquer sur le dessin d'une case la retrouve, sous tous les angles", "[camera]") {
    OrbitCamera camera{48, 27};
    for (int turn = 0; turn < 12; ++turn) {
        for (const Position cell : {Position{0, 0}, Position{47, 26}, Position{20, 9}}) {
            const auto point = camera.Project({cell.x + 0.5, 0.0, cell.y + 0.5}, Screen);
            REQUIRE(point.has_value());
            REQUIRE(camera.PickCell(*point, Screen) == cell);
        }
        camera.Rotate(0.55, turn % 2 == 0 ? 0.2 : -0.2);
    }
}

TEST_CASE("Un clic dans le ciel ne choisit aucune case", "[camera]") {
    OrbitCamera camera{48, 27};
    camera.Rotate(0.0, -5.0);
    REQUIRE_FALSE(camera.PickCell({960, 0}, Screen).has_value());
}

TEST_CASE("L'inclinaison et le zoom restent limites, puis se reinitialisent", "[camera]") {
    OrbitCamera camera{48, 27};
    const double distance = camera.Distance();
    camera.Rotate(0.0, 10.0);
    REQUIRE(camera.Pitch() == OrbitCamera::MaxPitch);
    camera.Rotate(0.0, -10.0);
    REQUIRE(camera.Pitch() == OrbitCamera::MinPitch);

    camera.Zoom(100);
    const double closest = camera.Distance();
    REQUIRE(closest > 0.0);
    REQUIRE(closest < distance);
    camera.Zoom(-100);
    REQUIRE(camera.Distance() > distance);
    REQUIRE(camera.Position3d().y > 0.0);

    camera.Reset();
    REQUIRE(camera.Distance() == distance);
}

TEST_CASE("Le bouton bascule en 3D ; le clic droit et la molette pilotent la camera", "[camera]") {
    UiController controller{1920, 1080, 40};
    SimulationSettings settings;
    RawInput drag;
    drag.rotating = true;
    drag.pointer = PointerPosition{500, 500};
    drag.wheel = 1;
    const double distance = controller.Map().Camera().Distance();

    // En 2D, la camera ne bouge pas.
    controller.Handle(drag, false, false, settings);
    REQUIRE(controller.Map().Camera().Distance() == distance);

    const UiRect view = ViewButton(1920);
    controller.Handle(Click(view.x + 1, view.y + 1), false, false, settings);
    REQUIRE(controller.Map().ActiveKind() == MapViewKind::Orbit);

    controller.Handle(drag, false, false, settings);
    REQUIRE(controller.Map().Camera().Distance() < distance);
    REQUIRE(controller.Map().Camera().Yaw() == 0.0);
    drag.wheel = 0;
    drag.pointer = PointerPosition{400, 500};
    controller.Handle(drag, false, false, settings);
    REQUIRE(controller.Map().Camera().Yaw() > 0.0);

    const UiRect recenter = RecenterButton(1920);
    controller.Handle(Click(recenter.x + 1, recenter.y + 1), false, false, settings);
    REQUIRE(controller.Map().Camera().Yaw() == 0.0);
    REQUIRE(controller.Map().Camera().Distance() == distance);

    controller.Handle(Click(view.x + 1, view.y + 1), false, false, settings);
    REQUIRE(controller.Map().ActiveKind() == MapViewKind::Flat);
}

TEST_CASE("En 3D, un clic choisit la case visee", "[camera]") {
    UiController controller{1920, 1080, 40};
    SimulationSettings settings;
    const UiRect view = ViewButton(1920);
    controller.Handle(Click(view.x + 1, view.y + 1), false, false, settings);

    const auto point = controller.Map().Camera().Project({10.5, 0.0, 8.5}, Screen);
    REQUIRE(point.has_value());
    controller.Handle(Click(point->x, point->y), false, false, settings);
    REQUIRE(controller.Map().SelectedCell() == Position{10, 8});
    REQUIRE_FALSE(controller.Map().IsCellPanelOnLeft());
}
