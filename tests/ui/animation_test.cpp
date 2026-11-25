#include <catch2/catch_all.hpp>

import std;
import ecosim.animation;

using namespace ecosim;

TEST_CASE("Un pas manuel s'anime en pause puis s'arrete a l'arrivee", "[animation]") {
    SimulationClock clock;
    clock.SetPaused(true);
    ManualStepAnimation animation;
    animation.Start(clock);
    REQUIRE(clock.Progress() == 0.0);

    animation.Update(clock, 0.125);
    REQUIRE(clock.Progress() == 0.5);
    animation.Update(clock, 5.0);
    REQUIRE(clock.Progress() == 1.0);

    // A la reprise, la generation suivante commence sans retour en arriere.
    clock.SetPaused(false);
    REQUIRE(clock.Advance(0.125) == 1);
    REQUIRE(clock.Progress() == 0.125);
}

TEST_CASE("Sans pas manuel, la pause reste figee", "[animation]") {
    SimulationClock clock;
    clock.Advance(0.25);
    clock.SetPaused(true);
    ManualStepAnimation animation;
    animation.Update(clock, 1.0);
    REQUIRE(clock.Progress() == 0.25);

    animation.Start(clock);
    animation.Cancel();
    animation.Update(clock, 1.0);
    REQUIRE(clock.Progress() == 0.0);
}

TEST_CASE("A l'ecran, un nouveau-ne accompagne son parent", "[animation]") {
    std::vector<AnimalSnapshot> animals{
        {1, {1, 0}, AnimalKind::Herbivore, 9, 50, {0, 0}, std::nullopt},
        {2, {1, 0}, AnimalKind::Herbivore, 0, 30, {1, 0}, 1},
        {3, {2, 0}, AnimalKind::Herbivore, 4, 40, {3, 0}, 1},
    };

    AttachNewbornsToParents(animals);

    REQUIRE(animals[1].previousPosition == Position{0, 0});
    REQUIRE(animals[2].previousPosition == Position{3, 0});
}
