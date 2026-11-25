#include <catch2/catch_all.hpp>

import std;
import ecosim.simulation;

using namespace ecosim;

TEST_CASE("L'historique des populations suit les générations et les ajouts", "[population]") {
    Simulation simulation{World{1, 1}, std::mt19937{42}};
    REQUIRE(simulation.PopulationHistory().size() == 1);
    REQUIRE(simulation.PopulationHistory().front().generation == 0);

    simulation.AddHerbivore({0, 0});
    simulation.AddHerbivore({0, 0});
    simulation.AddPredator({0, 0});
    REQUIRE(simulation.PopulationHistory().front().herbivores == 2);
    REQUIRE(simulation.PopulationHistory().front().predators == 1);

    simulation.Step();
    REQUIRE(simulation.PopulationHistory().size() == 2);
    REQUIRE(simulation.PopulationHistory().back().generation == 1);
    REQUIRE(simulation.PopulationHistory().back().herbivores == simulation.HerbivoreCount());
    REQUIRE(simulation.PopulationHistory().back().predators == simulation.PredatorCount());

    simulation.AddPredator({0, 0});
    REQUIRE(simulation.PopulationHistory().back().predators == 2);
    REQUIRE(simulation.PopulationHistory().front().predators == 1);

    simulation.Step();
    REQUIRE(simulation.PopulationHistory().size() == 3);
    REQUIRE(simulation.PopulationHistory().back().generation == 2);
    REQUIRE(simulation.PopulationHistory().back().herbivores == simulation.HerbivoreCount());
    REQUIRE(simulation.PopulationHistory().back().predators == simulation.PredatorCount());

    for (int i = 0; i < 8; ++i) simulation.Step();
    const auto& history = simulation.PopulationHistory();
    REQUIRE(history.size() == static_cast<std::size_t>(simulation.GetGeneration() + 1));
    for (std::size_t i = 0; i < history.size(); ++i) {
        REQUIRE(history[i].generation == static_cast<int>(i));
    }
}
