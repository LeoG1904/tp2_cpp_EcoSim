#include <catch2/catch_all.hpp>

// Le main des tests est fourni par Catch2WithMain (voir CMakeLists.txt)
import std;
import ecosim.clock;

using namespace ecosim;

TEST_CASE("Vitesse x1 : une génération par seconde", "[clock]") {
    SimulationClock clock;
    REQUIRE(clock.GetSpeed() == 1);
    REQUIRE(clock.Advance(1.0) == 1);
}

TEST_CASE("Vitesses x2, x5 et x10", "[clock]") {
    SimulationClock clock;

    clock.SetSpeed(2);
    REQUIRE(clock.Advance(1.0) == 2);

    clock.SetSpeed(5);
    REQUIRE(clock.Advance(1.0) == 5);

    clock.SetSpeed(10);
    REQUIRE(clock.Advance(1.0) == 10);
}

TEST_CASE("Les fractions de génération sont conservées", "[clock]") {
    SimulationClock clock;
    clock.SetSpeed(2);

    // 0,25 s à x2 = 0,5 génération : rien pour l'instant
    REQUIRE(clock.Advance(0.25) == 0);
    // 0,25 s de plus : 1 génération complète
    REQUIRE(clock.Advance(0.25) == 1);
}

TEST_CASE("Vitesse non supportée refusée", "[clock]") {
    SimulationClock clock;
    REQUIRE_THROWS_AS(clock.SetSpeed(3), std::invalid_argument);
    REQUIRE(clock.GetSpeed() == 1);
}

TEST_CASE("La pause ignore le temps ecoule sans perdre la fraction deja accumulee", "[clock]") {
    SimulationClock clock;
    clock.SetSpeed(2);
    REQUIRE(clock.Advance(0.25) == 0);

    clock.SetPaused(true);
    REQUIRE(clock.IsPaused());
    REQUIRE(clock.Advance(30.0) == 0);

    clock.SetPaused(false);
    REQUIRE_FALSE(clock.IsPaused());
    REQUIRE(clock.Advance(0.25) == 1);
    REQUIRE(clock.Advance(0.25) == 0);
}

TEST_CASE("Tab parcourt les vitesses puis revient a x1", "[clock]") {
    SimulationClock clock;
    for (const int expected : {2, 5, 10, 1}) {
        clock.CycleSpeed();
        REQUIRE(clock.GetSpeed() == expected);
    }
}
