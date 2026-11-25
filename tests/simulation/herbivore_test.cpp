#include <catch2/catch_all.hpp>

import std;
import ecosim.simulation;

using namespace ecosim;

TEST_CASE("Deux herbivores partagent une case et mangent la végétation disponible", "[herbivore]") {
    World world{1, 1};
    world.GetCell({0, 0}).SetVegetation(20);
    Simulation simulation{std::move(world), std::mt19937{1}};
    simulation.AddAnimal(AnimalKind::Herbivore, {0, 0});
    simulation.AddAnimal(AnimalKind::Herbivore, {0, 0});

    simulation.Step();

    REQUIRE(simulation.GetGeneration() == 1);
    REQUIRE(simulation.Count(AnimalKind::Herbivore) == 2);
    REQUIRE(simulation.GetWorld().GetCell({0, 0}).GetVegetation() == 1);
    for (const auto& animal : simulation.Animals()) {
        REQUIRE(animal.position == Position{0, 0});
        REQUIRE(animal.age == 1);
        REQUIRE(animal.energy == 58);
    }
}

TEST_CASE("Un herbivore rejoint une prairie voisine plus riche", "[herbivore]") {
    World world{2, 1};
    world.GetCell({1, 0}).SetVegetation(50);
    Simulation simulation{std::move(world), std::mt19937{2}};
    simulation.AddAnimal(AnimalKind::Herbivore, {0, 0});

    simulation.Step();

    REQUIRE(simulation.Animals().front().position == Position{1, 0});
    REQUIRE(simulation.GetWorld().GetCell({1, 0}).GetVegetation() == 41);
}

TEST_CASE("Un herbivore meurt quand son énergie est épuisée", "[herbivore]") {
    Simulation simulation{World{1, 1}, std::mt19937{3}};
    simulation.AddAnimal(AnimalKind::Herbivore, {0, 0}, 1);

    simulation.Step();

    REQUIRE(simulation.Count(AnimalKind::Herbivore) == 0);
}

TEST_CASE("La case doit être accessible même si plusieurs animaux peuvent l'occuper", "[herbivore]") {
    World world{1, 1};
    world.GetCell({0, 0}) = Cell{Terrain::Eau};
    Simulation simulation{std::move(world), std::mt19937{4}};

    REQUIRE_THROWS_AS(simulation.AddAnimal(AnimalKind::Herbivore, {0, 0}), std::invalid_argument);
    REQUIRE_THROWS_AS(simulation.AddAnimal(AnimalKind::Herbivore, {2, 0}), std::invalid_argument);
}

TEST_CASE("La naissance crée un animal distinct qui attend la génération suivante", "[herbivore]") {
    World world{1, 1};
    world.GetCell({0, 0}).SetVegetation(100);
    Simulation simulation{std::move(world), std::mt19937{7}};
    simulation.AddAnimal(AnimalKind::Herbivore, {0, 0}, 95);

    for (int i = 0; i < 30 && simulation.Count(AnimalKind::Herbivore) == 1; ++i) {
        simulation.Step();
    }

    REQUIRE(simulation.Count(AnimalKind::Herbivore) > 1);
    const auto animals = simulation.Animals();
    REQUIRE(animals[0].id != animals[1].id);
    REQUIRE(animals[1].age == 0);
    REQUIRE(animals[1].position == animals[0].position);
}

TEST_CASE("Chaque animal garde sa case precedente pour l'animation", "[herbivore]") {
    World world{2, 1};
    world.GetCell({1, 0}).SetVegetation(50);
    Simulation simulation{std::move(world), std::mt19937{2}};
    simulation.AddAnimal(AnimalKind::Herbivore, {0, 0});
    REQUIRE(simulation.Animals().front().previousPosition == Position{0, 0});

    simulation.Step();
    REQUIRE(simulation.Animals().front().previousPosition == Position{0, 0});
    REQUIRE(simulation.Animals().front().position == Position{1, 0});
}
