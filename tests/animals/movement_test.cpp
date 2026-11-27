#include <catch2/catch_all.hpp>

import std;
import ecosim.deer;
import ecosim.wolf;

using namespace ecosim;

namespace {
    template <typename Species>
    double Travel(const double pace) {
        const World world{12, 12};
        Species animal{1, {2, 6}};
        animal.Place({2.5, 6.5}, 0.0);
        for (int tick = 0; tick < TicksPerGeneration; ++tick) animal.Advance(world, {0.0, pace}, tick);
        return Distance({2.5, 6.5}, animal.GetLocation());
    }
}

TEST_CASE("La vitesse et l'allure fixent la distance parcourue", "[movement]") {
    REQUIRE(Travel<Deer>(1.0) == Catch::Approx(Deer::Traits.speed / 100.0));
    REQUIRE(Travel<Wolf>(1.0) == Catch::Approx(Wolf::Traits.speed / 100.0));
    REQUIRE(Travel<Deer>(0.5) == Catch::Approx(Deer::Traits.speed / 200.0));
    REQUIRE(Travel<Deer>(0.0) == 0.0);
}

TEST_CASE("Un animal tourne progressivement", "[movement]") {
    const World world{12, 12};
    Deer animal{1, {6, 6}};
    animal.Place({6.5, 6.5}, 0.0);
    animal.Advance(world, {3.0, 1.0}, 0);
    REQUIRE(animal.GetHeading() == Catch::Approx(Animal::MaxTurnPerTick));
}

TEST_CASE("Un animal contourne un obstacle sans jamais y entrer", "[movement]") {
    World world{5, 3};
    world.GetCell({2, 1}) = Cell{Terrain::Obstacle};
    Wolf animal{1, {0, 1}};
    animal.Place({1.5, 1.5}, 0.0);
    for (int generation = 0; generation < 3; ++generation) {
        animal.StartPath();
        for (int tick = 0; tick < TicksPerGeneration; ++tick) {
            animal.Advance(world, {0.0, 1.0}, tick);
            REQUIRE(world.IsWalkable(animal.GetLocation()));
        }
    }
    REQUIRE(Distance({1.5, 1.5}, animal.GetLocation()) > 0.5);
}

TEST_CASE("Monter ralentit un animal, descendre non", "[movement]") {
    // Un palier au milieu de la carte, franchi vers la droite pendant le trajet.
    const auto travel = [](const int leftLevel, const int rightLevel) {
        World world{12, 12};
        for (int y = 0; y < 12; ++y) {
            for (int x = 0; x < 12; ++x) world.GetCell({x, y}).SetElevation(x < 3 ? leftLevel : rightLevel);
        }
        Deer animal{1, {2, 6}};
        animal.Place({2.5, 6.5}, 0.0);
        for (int tick = 0; tick < TicksPerGeneration; ++tick) animal.Advance(world, {0.0, 1.0}, tick);
        return Distance({2.5, 6.5}, animal.GetLocation());
    };
    const double flat = travel(0, 0);
    REQUIRE(flat == Catch::Approx(Deer::Traits.speed / 100.0));
    REQUIRE(travel(2, 0) == Catch::Approx(flat));
    REQUIRE(travel(0, 1) < flat - 0.2);
    // La pente passée, l'animal reprend sa vitesse : il n'a perdu qu'une partie du trajet.
    REQUIRE(travel(0, 1) > flat * Animal::ClimbPace);
}
