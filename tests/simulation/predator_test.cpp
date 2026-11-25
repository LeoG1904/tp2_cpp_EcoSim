#include <catch2/catch_all.hpp>

import std;
import ecosim.simulation;

using namespace ecosim;

TEST_CASE("Un prédateur chasse une proie présente sur sa case", "[predator]") {
    World world{1, 1};
    world.GetCell({0, 0}).SetVegetation(10);
    Simulation simulation{std::move(world), std::mt19937{10}};
    simulation.AddHerbivore({0, 0});
    simulation.AddPredator({0, 0});

    simulation.Step();

    REQUIRE(simulation.HerbivoreCount() == 0);
    REQUIRE(simulation.PredatorCount() == 1);
    REQUIRE(simulation.Animals().front().energy == 83); // 50 + 35 pour la proie - 2 pour le temps
    REQUIRE(simulation.GetWorld().GetCell({0, 0}).GetVegetation() == 1);
}

TEST_CASE("Une proie ne peut être capturée qu'une fois", "[predator]") {
    Simulation simulation{World{1, 1}, std::mt19937{11}};
    simulation.AddHerbivore({0, 0});
    simulation.AddPredator({0, 0});
    simulation.AddPredator({0, 0});

    simulation.Step();

    REQUIRE(simulation.HerbivoreCount() == 0);
    REQUIRE(simulation.PredatorCount() == 2);
    const auto animals = simulation.Animals();
    REQUIRE(animals[0].energy == 83);
    REQUIRE(animals[1].energy == 48);
}

TEST_CASE("Un prédateur se dirige vers une proie sur la case voisine", "[predator]") {
    World world{2, 1};
    world.GetCell({1, 0}).SetVegetation(50);
    Simulation simulation{std::move(world), std::mt19937{12}};
    simulation.AddHerbivore({1, 0});
    simulation.AddPredator({0, 0});

    simulation.Step();

    REQUIRE(simulation.HerbivoreCount() == 0);
    REQUIRE(simulation.Animals().front().position == Position{1, 0});
}

TEST_CASE("Un prédateur sans proie finit par mourir", "[predator]") {
    Simulation simulation{World{1, 1}, std::mt19937{13}};
    simulation.AddPredator({0, 0}, 3);

    simulation.Step();
    REQUIRE(simulation.PredatorCount() == 1);
    simulation.Step();
    REQUIRE(simulation.PredatorCount() == 0);
}

TEST_CASE("Les prédateurs naissent sur une prairie et peuvent partager une case", "[predator]") {
    World world{2, 1};
    world.GetCell({1, 0}) = Cell{Terrain::Eau};
    Simulation simulation{std::move(world), std::mt19937{14}};
    simulation.SeedPredators(3);

    REQUIRE(simulation.PredatorCount() == 3);
    for (const auto& animal : simulation.Animals()) {
        REQUIRE(animal.kind == AnimalKind::Predator);
        REQUIRE(animal.position == Position{0, 0});
    }
    REQUIRE_THROWS_AS(simulation.AddPredator({1, 0}), std::invalid_argument);
    REQUIRE_THROWS_AS(simulation.SeedPredators(-1), std::invalid_argument);
}

TEST_CASE("Un descendant prédateur est un individu distinct", "[predator]") {
    Predator adult{1, {0, 0}, 90};
    for (int i = 0; i < 5; ++i) adult.SpendGeneration();

    REQUIRE(adult.CanReproduce());
    auto offspring = adult.CreateOffspring(2);
    adult.PayReproductionCost();

    REQUIRE(offspring->GetId() == 2);
    REQUIRE(offspring->GetKind() == AnimalKind::Predator);
    REQUIRE(offspring->GetAge() == 0);
    REQUIRE(offspring->GetEnergy() == 30);
    REQUIRE(adult.GetEnergy() == 50);
}
