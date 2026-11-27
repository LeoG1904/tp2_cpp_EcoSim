#include <catch2/catch_all.hpp>

import std;
import ecosim.simulation;
import ecosim.grid_habitat;

using namespace ecosim;

TEST_CASE("Deux herbivores partagent une case et mangent la végétation disponible", "[herbivore]") {
    World world{1, 1};
    world.GetCell({0, 0}).SetVegetation(20);
    Simulation simulation{std::move(world), std::mt19937{1}};
    simulation.AddAnimal(AnimalKind::Deer, {0, 0});
    simulation.AddAnimal(AnimalKind::Deer, {0, 0});

    simulation.Step();

    REQUIRE(simulation.GetGeneration() == 1);
    REQUIRE(simulation.Count(AnimalKind::Deer) == 2);
    // 21 points d'herbe : un cerf en mange 12, l'autre 4, et les 5 derniers restent au sol.
    REQUIRE(simulation.GetWorld().GetCell({0, 0}).GetVegetation() == 5);
    const auto animals = simulation.Animals();
    REQUIRE(animals[0].energy + animals[1].energy == 60 + 52);
    for (const auto& animal : animals) {
        REQUIRE(animal.position == Position{0, 0});
        REQUIRE(animal.age == 1);
    }
}

TEST_CASE("Un herbivore rejoint une prairie voisine plus riche", "[herbivore]") {
    World world{2, 1};
    world.GetCell({1, 0}).SetVegetation(50);
    Simulation simulation{std::move(world), std::mt19937{2}};
    simulation.AddAnimal(AnimalKind::Deer, {0, 0});

    for (int i = 0; i < 4 && simulation.Animals().front().position != Position{1, 0}; ++i) {
        simulation.Step();
    }

    REQUIRE(simulation.Animals().front().position == Position{1, 0});
}

TEST_CASE("Un herbivore meurt quand son énergie est épuisée", "[herbivore]") {
    Simulation simulation{World{1, 1}, std::mt19937{3}};
    simulation.AddAnimal(AnimalKind::Deer, {0, 0}, 1);

    simulation.Step();

    REQUIRE(simulation.Count(AnimalKind::Deer) == 0);
}

TEST_CASE("La case doit être accessible même si plusieurs animaux peuvent l'occuper", "[herbivore]") {
    World world{1, 1};
    world.GetCell({0, 0}) = Cell{Terrain::Eau};
    Simulation simulation{std::move(world), std::mt19937{4}};

    REQUIRE_THROWS_AS(simulation.AddAnimal(AnimalKind::Deer, {0, 0}), std::invalid_argument);
    REQUIRE_THROWS_AS(simulation.AddAnimal(AnimalKind::Deer, {2, 0}), std::invalid_argument);
}

TEST_CASE("La naissance cree des petits distincts qui attendent la generation suivante", "[herbivore]") {
    World world{1, 1};
    world.GetCell({0, 0}).SetVegetation(100);
    SimulationSettings settings;
    settings.vegetationGrowth = 20;
    settings.species[AnimalKind::Deer].options.birthPercent = 100;
    Simulation simulation{std::move(world), std::mt19937{7}, settings};
    simulation.AddAnimal(AnimalKind::Deer, {0, 0}, 95);
    simulation.AddAnimal(AnimalKind::Deer, {0, 0}, 95);

    for (int i = 0; i < 30 && simulation.Count(AnimalKind::Deer) == 2; ++i) {
        simulation.Step();
    }

    const auto animals = simulation.Animals();
    REQUIRE(animals.size() > 2);
    std::set<std::uint64_t> ids;
    for (const auto& animal : animals) ids.insert(animal.id);
    REQUIRE(ids.size() == animals.size());
    for (std::size_t i = 2; i < animals.size(); ++i) {
        REQUIRE(animals[i].age == 0);
        // La mere est la femelle placee en premier.
        REQUIRE(animals[i].parentId == animals[0].id);
        REQUIRE(animals[i].position == animals[0].position);
    }
}

TEST_CASE("Le trajet d'une generation est continu et reste sur les prairies", "[herbivore]") {
    std::mt19937 rng{9};
    World world = GenerateRandomWorld(20, 14, rng);
    Simulation simulation{std::move(world), std::mt19937{10}};
    simulation.Seed(AnimalKind::Deer, 30);
    simulation.Seed(AnimalKind::Wolf, 6);
    std::map<std::uint64_t, Point> previous;
    for (const auto& animal : simulation.Animals()) previous[animal.id] = animal.path.back();

    for (int generation = 0; generation < 40 && !simulation.IsFinished(); ++generation) {
        simulation.Step();
        for (const auto& animal : simulation.Animals()) {
            // Un trajet commence où le précédent s'est arrêté.
            if (previous.contains(animal.id)) REQUIRE(animal.path.front() == previous[animal.id]);
            for (std::size_t i = 1; i < animal.path.size(); ++i) {
                // Le pas de l'animal, plus au maximum la poussée d'une collision.
                REQUIRE(Distance(animal.path[i - 1], animal.path[i])
                        <= Deer::Traits.speed / 100.0 / TicksPerGeneration + MaxPushPerTick + 1e-9);
                REQUIRE(simulation.GetWorld().IsWalkable(animal.path[i]));
            }
            REQUIRE(animal.position == CellAt(animal.path.back()));
            previous[animal.id] = animal.path.back();
        }
    }
}

TEST_CASE("Les animaux ne partent pas tous du centre des cases", "[herbivore]") {
    Simulation simulation{World{6, 6}, std::mt19937{12}};
    simulation.Seed(AnimalKind::Deer, 10);
    const auto animals = simulation.Animals();
    REQUIRE(std::ranges::any_of(animals, [](const auto& animal) {
        return animal.path.back() != CenterOf(animal.position);
    }));
}
