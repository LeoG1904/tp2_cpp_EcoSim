#include <catch2/catch_all.hpp>

import std;
import ecosim.simulation;

using namespace ecosim;

TEST_CASE("L'historique des populations suit les générations et les ajouts", "[population]") {
    Simulation simulation{World{1, 1}, std::mt19937{42}};
    REQUIRE(simulation.Stats().History().size() == 1);
    REQUIRE(simulation.Stats().History().front().generation == 0);

    simulation.AddAnimal(AnimalKind::Deer, {0, 0});
    simulation.AddAnimal(AnimalKind::Deer, {0, 0});
    simulation.AddAnimal(AnimalKind::Predator, {0, 0});
    REQUIRE(simulation.Stats().History().front().species[AnimalKind::Deer].population == 2);
    REQUIRE(simulation.Stats().History().front().species[AnimalKind::Predator].population == 1);

    simulation.Step();
    REQUIRE(simulation.Stats().History().size() == 2);
    REQUIRE(simulation.Stats().History().back().generation == 1);
    REQUIRE(simulation.Stats().History().back().species[AnimalKind::Deer].population == simulation.Count(AnimalKind::Deer));
    REQUIRE(simulation.Stats().History().back().species[AnimalKind::Predator].population == simulation.Count(AnimalKind::Predator));

    simulation.AddAnimal(AnimalKind::Predator, {0, 0});
    REQUIRE(simulation.Stats().History().back().species[AnimalKind::Predator].population == 2);
    REQUIRE(simulation.Stats().History().front().species[AnimalKind::Predator].population == 1);

    simulation.Step();
    REQUIRE(simulation.Stats().History().size() == 3);
    REQUIRE(simulation.Stats().History().back().generation == 2);
    REQUIRE(simulation.Stats().History().back().species[AnimalKind::Deer].population == simulation.Count(AnimalKind::Deer));
    REQUIRE(simulation.Stats().History().back().species[AnimalKind::Predator].population == simulation.Count(AnimalKind::Predator));

    for (int i = 0; i < 8; ++i) simulation.Step();
    const auto& history = simulation.Stats().History();
    REQUIRE(history.size() == static_cast<std::size_t>(simulation.GetGeneration() + 1));
    for (std::size_t i = 0; i < history.size(); ++i) {
        REQUIRE(history[i].generation == static_cast<int>(i));
    }
}
