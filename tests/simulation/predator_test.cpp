#include <catch2/catch_all.hpp>

import std;
import ecosim.simulation;

using namespace ecosim;

TEST_CASE("Un prédateur chasse une proie présente sur sa case", "[predator]") {
    World world{1, 1};
    world.GetCell({0, 0}).SetVegetation(10);
    Simulation simulation{std::move(world), std::mt19937{10}};
    simulation.AddAnimal(AnimalKind::Deer, {0, 0});
    simulation.AddAnimal(AnimalKind::Wolf, {0, 0});

    simulation.Step();

    REQUIRE(simulation.Count(AnimalKind::Deer) == 0);
    REQUIRE(simulation.Count(AnimalKind::Wolf) == 1);
    REQUIRE(simulation.Animals().front().energy == 73); // 50 + 25 pour la proie - 2 pour le temps
    // Capturée en chemin, la proie n'a pas eu le temps de manger.
    REQUIRE(simulation.GetWorld().GetCell({0, 0}).GetVegetation() == 11);
}

TEST_CASE("Une proie ne peut être capturée qu'une fois", "[predator]") {
    Simulation simulation{World{1, 1}, std::mt19937{11}};
    simulation.AddAnimal(AnimalKind::Deer, {0, 0});
    simulation.AddAnimal(AnimalKind::Wolf, {0, 0});
    simulation.AddAnimal(AnimalKind::Wolf, {0, 0});

    simulation.Step();

    REQUIRE(simulation.Count(AnimalKind::Deer) == 0);
    REQUIRE(simulation.Count(AnimalKind::Wolf) == 2);
    const auto animals = simulation.Animals();
    REQUIRE(animals[0].energy == 73);
    REQUIRE(animals[1].energy == 48);
}

TEST_CASE("Un prédateur se dirige vers une proie sur la case voisine", "[predator]") {
    World world{2, 1};
    world.GetCell({1, 0}).SetVegetation(50);
    Simulation simulation{std::move(world), std::mt19937{12}};
    simulation.AddAnimal(AnimalKind::Deer, {1, 0});
    simulation.AddAnimal(AnimalKind::Wolf, {0, 0});

    simulation.Step();

    REQUIRE(simulation.Count(AnimalKind::Deer) == 0);
    REQUIRE(simulation.Animals().front().position == Position{1, 0});
}

TEST_CASE("Un prédateur sans proie finit par mourir", "[predator]") {
    Simulation simulation{World{1, 1}, std::mt19937{13}};
    simulation.AddAnimal(AnimalKind::Wolf, {0, 0}, 4);

    simulation.Step();
    REQUIRE(simulation.Count(AnimalKind::Wolf) == 1);
    simulation.Step();
    REQUIRE(simulation.Count(AnimalKind::Wolf) == 0);
}

TEST_CASE("Les prédateurs naissent sur une prairie et peuvent partager une case", "[predator]") {
    World world{2, 1};
    world.GetCell({1, 0}) = Cell{Terrain::Eau};
    Simulation simulation{std::move(world), std::mt19937{14}};
    simulation.Seed(AnimalKind::Wolf, 3);

    REQUIRE(simulation.Count(AnimalKind::Wolf) == 3);
    for (const auto& animal : simulation.Animals()) {
        REQUIRE(animal.kind == AnimalKind::Wolf);
        REQUIRE(animal.position == Position{0, 0});
    }
    REQUIRE_THROWS_AS(simulation.AddAnimal(AnimalKind::Wolf, {1, 0}), std::invalid_argument);
    REQUIRE_THROWS_AS(simulation.Seed(AnimalKind::Wolf, -1), std::invalid_argument);
}

TEST_CASE("Un descendant prédateur est un individu distinct", "[predator]") {
    Wolf adult{1, {0, 0}, 90};
    for (int i = 0; i < 5; ++i) adult.SpendGeneration();

    REQUIRE(adult.CanReproduce());
    auto offspring = adult.CreateOffspring(2, Sex::Male);
    adult.PayReproductionCost();

    REQUIRE(offspring->GetId() == 2);
    REQUIRE(offspring->GetKind() == AnimalKind::Wolf);
    REQUIRE(offspring->GetAge() == 0);
    REQUIRE(offspring->GetEnergy() == 30);
    // 90, moins 5 générations à 2, moins le coût d'une portée.
    REQUIRE(adult.GetEnergy() == 90 - 5 * Wolf::Traits.energyLoss - Animal::ReproductionCost);
    // Après une portée, il faut se reposer.
    REQUIRE_FALSE(adult.CanReproduce());
}

TEST_CASE("Un renard ne capture pas un cerf, mais un lapin", "[predator]") {
    const auto survivors = [](const AnimalKind prey) {
        Simulation simulation{World{1, 1}, std::mt19937{15}};
        simulation.AddAnimal(prey, {0, 0});
        simulation.AddAnimal(AnimalKind::Fox, {0, 0});
        simulation.Step();
        return simulation.Count(prey);
    };
    REQUIRE(survivors(AnimalKind::Deer) == 1);
    REQUIRE(survivors(AnimalKind::Rabbit) == 0);
}

TEST_CASE("Seuls les chasseurs qui le mangent menacent un herbivore", "[predator]") {
    // Un cerf sur ses gardes ignore un renard : il broute et ne perd que son metabolisme.
    SimulationSettings settings;
    settings.vegetationGrowth = 0;
    settings.species[AnimalKind::Deer].options.instinct = 100;
    const auto path = [&settings](const bool withFox) {
        World world{12, 12};
        for (int y = 0; y < 12; ++y) {
            for (int x = 0; x < 12; ++x) world.GetCell({x, y}).SetVegetation(100);
        }
        Simulation simulation{std::move(world), std::mt19937{16}, settings};
        simulation.AddAnimal(AnimalKind::Deer, {5, 5});
        if (withFox) simulation.AddAnimal(AnimalKind::Fox, {6, 5});
        simulation.Step();
        return simulation.Animals().front().path;
    };
    const Path alone = path(false);
    const Path watched = path(true);
    REQUIRE(Distance(alone.back(), watched.back()) < 0.5);
}
