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

TEST_CASE("La simulation applique la repousse et les caracteristiques de l'espece", "[settings]") {
    SimulationSettings settings;
    settings.vegetationGrowth = 20;
    Simulation simulation{World{1, 1}, std::mt19937{2}, settings};
    simulation.AddAnimal(AnimalKind::Deer, {0, 0});

    simulation.Step();

    // 50 au départ, 10 mangés, 2 perdus.
    REQUIRE(simulation.Animals().front().energy == 58);
    REQUIRE(simulation.GetWorld().GetCell({0, 0}).GetVegetation() == 10);
}

TEST_CASE("Chaque proie nourrit le predateur selon sa taille", "[settings]") {
    const auto energyAfterHunt = [](const AnimalKind prey) {
        Simulation hunt{World{1, 1}, std::mt19937{4}};
        hunt.AddAnimal(prey, {0, 0});
        hunt.AddAnimal(AnimalKind::Wolf, {0, 0});
        hunt.Step();
        return hunt.Animals().front().energy;
    };
    REQUIRE(energyAfterHunt(AnimalKind::Deer) == 72);   // 50 + 25 - 3
    REQUIRE(energyAfterHunt(AnimalKind::Rabbit) == 59); // 50 + 12 - 3
}

TEST_CASE("La chance de naissance peut etre nulle ou certaine, et s'herite", "[settings]") {
    SimulationSettings settings;
    settings.species[AnimalKind::Deer].options.birthPercent = 0;
    Simulation sterile{World{1, 1}, std::mt19937{5}, settings};
    sterile.AddAnimal(AnimalKind::Deer, {0, 0}, 100);
    for (int i = 0; i < 8; ++i) sterile.Step();
    REQUIRE(sterile.Count(AnimalKind::Deer) == 1);

    settings.species[AnimalKind::Deer].options.birthPercent = 100;
    Simulation fertile{World{1, 1}, std::mt19937{5}, settings};
    fertile.AddAnimal(AnimalKind::Deer, {0, 0}, 100);
    for (int i = 0; i < 5; ++i) fertile.Step();
    REQUIRE(fertile.Count(AnimalKind::Deer) == 2);

    const Deer parent{1, {0, 0}, 90, settings.species[AnimalKind::Deer].options};
    REQUIRE(parent.CreateOffspring(2)->GetOptions().birthPercent == 100);
}
