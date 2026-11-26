#include <catch2/catch_all.hpp>

import std;
import ecosim.simulation;

using namespace ecosim;

TEST_CASE("La simulation s'arrete a la generation ou le dernier animal meurt", "[simulation]") {
    Simulation simulation{World{1, 1}, std::mt19937{3}};
    simulation.AddAnimal(AnimalKind::Deer, {0, 0}, 1);
    REQUIRE_FALSE(simulation.IsFinished());

    simulation.Step();
    REQUIRE(simulation.IsFinished());
    REQUIRE(simulation.GetGeneration() == 1);
    REQUIRE(simulation.Count(AnimalKind::Deer) == 0);
    REQUIRE(simulation.Count(AnimalKind::Predator) == 0);
    REQUIRE(simulation.Stats().History().size() == 2);
    REQUIRE(simulation.Stats().History().back().species[AnimalKind::Deer].population == 0);
    REQUIRE(simulation.Stats().History().back().species[AnimalKind::Predator].population == 0);

    const int vegetation = simulation.GetWorld().GetCell({0, 0}).GetVegetation();
    for (int i = 0; i < 10; ++i) simulation.Step();
    REQUIRE(simulation.GetGeneration() == 1);
    REQUIRE(simulation.Stats().History().size() == 2);
    REQUIRE(simulation.GetWorld().GetCell({0, 0}).GetVegetation() == vegetation);
}
