#include <catch2/catch_all.hpp>

import std;
import ecosim.deer;
import ecosim.predator;

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
    REQUIRE(Travel<Predator>(1.0) == Catch::Approx(Predator::Traits.speed / 100.0));
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
    Predator animal{1, {0, 1}};
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
