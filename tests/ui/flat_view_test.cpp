#include <catch2/catch_all.hpp>

import std;
import ecosim.flat_view;

using namespace ecosim;

namespace {
    constexpr Viewport Screen{1920, 1080};
}

TEST_CASE("La vue d'ensemble garde la carte entiere, centree et en cases carrees", "[flat_view]") {
    const FlatView exact{48, 27, Screen};
    REQUIRE(exact.CellSize() == 40.0);
    REQUIRE(exact.ScreenX(0) == 0.0);

    const FlatView square{27, 27, Screen};
    REQUIRE(square.CellSize() == 40.0);
    REQUIRE(square.ScreenX(0) == 420.0);
    REQUIRE(square.ScreenY(0) == 0.0);
    REQUIRE_FALSE(square.PickCell({100, 500}).has_value()); // bande laterale
    REQUIRE(square.PickCell({421, 1}) == Position{0, 0});
}

TEST_CASE("Une carte immense tient a l'ecran avec des cases de moins d'un pixel", "[flat_view]") {
    const FlatView view{5000, 5000, Screen};
    REQUIRE(view.CellSize() < 1.0);
    REQUIRE(view.ScreenY(5000) == Catch::Approx(1080.0));
    const CellRange visible = view.VisibleCells();
    REQUIRE(visible.right == 5000);
    REQUIRE(visible.bottom == 5000);
}

TEST_CASE("Le zoom garde l'endroit sous la souris et reste dans ses limites", "[flat_view]") {
    FlatView view{200, 200, Screen};
    const double fit = view.CellSize();
    const auto before = view.PickCell({900, 500});
    view.Zoom(5, {900, 500});
    REQUIRE(view.CellSize() > fit);
    REQUIRE(view.PickCell({900, 500}) == before);

    view.Zoom(100, {900, 500});
    REQUIRE(view.CellSize() == FlatView::MaxCellSize);
    view.Zoom(-100, {900, 500});
    REQUIRE(view.CellSize() == Catch::Approx(fit));
}

TEST_CASE("Le deplacement ne laisse pas la carte quitter l'ecran", "[flat_view]") {
    FlatView view{200, 200, Screen};
    view.Zoom(10, {960, 540});
    view.Pan(100000, 100000);
    REQUIRE(view.ScreenX(0) == 0.0);
    REQUIRE(view.ScreenY(0) == 0.0);
    view.Pan(-100000, -100000);
    REQUIRE(view.ScreenX(200) == Catch::Approx(1920.0));
    REQUIRE(view.ScreenY(200) == Catch::Approx(1080.0));

    const CellRange visible = view.VisibleCells();
    REQUIRE(visible.left > 0);
    REQUIRE(visible.right == 200);

    view.Reset();
    REQUIRE(view.ScreenY(0) == 0.0);
    REQUIRE(view.VisibleCells().left == 0);
}
