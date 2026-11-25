#include <catch2/catch_all.hpp>

import std;
import ecosim.simulation;

using namespace ecosim;

TEST_CASE("La simulation s'arrete a la generation ou le dernier animal meurt", "[simulation]") {
    Simulation simulation{World{1, 1}, std::mt19937{3}};
    simulation.AddHerbivore({0, 0}, 1);
    REQUIRE_FALSE(simulation.IsFinished());

    simulation.Step();
    REQUIRE(simulation.IsFinished());
    REQUIRE(simulation.GetGeneration() == 1);
    REQUIRE(simulation.HerbivoreCount() == 0);
    REQUIRE(simulation.PredatorCount() == 0);
    REQUIRE(simulation.PopulationHistory().size() == 2);
    REQUIRE(simulation.PopulationHistory().back().herbivores == 0);
    REQUIRE(simulation.PopulationHistory().back().predators == 0);

    const int vegetation = simulation.GetWorld().GetCell({0, 0}).GetVegetation();
    for (int i = 0; i < 10; ++i) simulation.Step();
    REQUIRE(simulation.GetGeneration() == 1);
    REQUIRE(simulation.PopulationHistory().size() == 2);
    REQUIRE(simulation.GetWorld().GetCell({0, 0}).GetVegetation() == vegetation);
}
