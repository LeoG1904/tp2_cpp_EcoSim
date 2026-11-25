#include <catch2/catch_all.hpp>

import std;
import ecosim.simulation;

using namespace ecosim;

TEST_CASE("Les proportions de terrain sont respectees", "[settings]") {
    std::mt19937 rng{1};
    const World world = GenerateRandomWorld(20, 20, rng, 0, 0);
    for (int y = 0; y < 20; ++y) {
        for (int x = 0; x < 20; ++x) {
            REQUIRE(world.GetCell({x, y}).GetTerrain() == Terrain::Prairie);
        }
    }
    const World lake = GenerateRandomWorld(5, 5, rng, 100, 0);
    REQUIRE(lake.GetCell({2, 2}).GetTerrain() == Terrain::Eau);
    REQUIRE_THROWS_AS(GenerateRandomWorld(5, 5, rng, 60, 50), std::invalid_argument);
}

TEST_CASE("La simulation applique les statistiques choisies", "[settings]") {
    SimulationSettings settings;
    settings.vegetationGrowth = 20;
    settings.herbivore.startingEnergy = 60;
    settings.herbivore.energyLoss = 7;
    settings.herbivore.foodEnergy = 4;
    Simulation simulation{World{1, 1}, std::mt19937{2}, settings};
    simulation.AddHerbivore({0, 0});

    simulation.Step();

    REQUIRE(simulation.Animals().front().energy == 57); // 60 + 4 mangés - 7 perdus
    REQUIRE(simulation.GetWorld().GetCell({0, 0}).GetVegetation() == 16);
}

TEST_CASE("L'age maximum et le gain par proie sont reglables", "[settings]") {
    SimulationSettings settings;
    settings.herbivore.maxAge = 1;
    Simulation old{World{1, 1}, std::mt19937{3}, settings};
    old.AddHerbivore({0, 0});
    old.Step();
    REQUIRE(old.IsFinished());

    settings = SimulationSettings{};
    settings.predator.foodEnergy = 10;
    Simulation hunt{World{1, 1}, std::mt19937{4}, settings};
    hunt.AddHerbivore({0, 0});
    hunt.AddPredator({0, 0});
    hunt.Step();
    REQUIRE(hunt.Animals().front().energy == 58); // 50 + 10 - 2
}

TEST_CASE("La chance de naissance peut etre nulle ou certaine, et s'herite", "[settings]") {
    SimulationSettings settings;
    settings.herbivore.reproductionEnergy = 40;
    settings.herbivore.reproductionPercent = 0;
    Simulation sterile{World{1, 1}, std::mt19937{5}, settings};
    sterile.AddHerbivore({0, 0}, 100);
    for (int i = 0; i < 8; ++i) sterile.Step();
    REQUIRE(sterile.HerbivoreCount() == 1);

    settings.herbivore.reproductionPercent = 100;
    Simulation fertile{World{1, 1}, std::mt19937{5}, settings};
    fertile.AddHerbivore({0, 0}, 100);
    for (int i = 0; i < 5; ++i) fertile.Step();
    REQUIRE(fertile.HerbivoreCount() == 2);

    const Herbivore parent{1, {0, 0}, 90, settings.herbivore};
    REQUIRE(parent.CreateOffspring(2)->GetTraits().reproductionPercent == 100);
}
