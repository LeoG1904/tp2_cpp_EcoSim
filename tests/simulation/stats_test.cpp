#include <catch2/catch_all.hpp>

import std;
import ecosim.simulation;

using namespace ecosim;

TEST_CASE("La generation 0 mesure les animaux et la vegetation des prairies", "[stats]") {
    World world{3, 1};
    world.GetCell({0, 0}).SetVegetation(30);
    world.GetCell({1, 0}).SetVegetation(20);
    world.GetCell({2, 0}) = Cell{Terrain::Eau, 90};
    Simulation simulation{std::move(world), std::mt19937{1}};
    simulation.AddHerbivore({0, 0}, 40);
    simulation.AddHerbivore({0, 0}, 60);

    const auto& history = simulation.Stats().History();
    REQUIRE(history.size() == 1);
    REQUIRE(history.back().vegetation == 50);
    REQUIRE(history.back().herbivores == 2);
    REQUIRE(history.back().herbivoreEnergy == 50.0);
    REQUIRE(history.back().predatorEnergy == 0.0);
    // Les animaux placés au départ ne sont pas des naissances.
    REQUIRE(simulation.Stats().Total(AnimalKind::Herbivore).births == 0);
}

TEST_CASE("Une proie capturee est comptee comme mangee", "[stats]") {
    Simulation simulation{World{1, 1}, std::mt19937{2}};
    simulation.AddHerbivore({0, 0});
    simulation.AddPredator({0, 0});
    simulation.Step();

    const LifeEvents& events = simulation.Stats().Total(AnimalKind::Herbivore);
    REQUIRE(events.eaten == 1);
    REQUIRE(events.Deaths() == 1);
}

TEST_CASE("La faim et la vieillesse sont distinguees", "[stats]") {
    Simulation hungry{World{1, 1}, std::mt19937{3}};
    hungry.AddHerbivore({0, 0}, 1);
    hungry.Step();
    REQUIRE(hungry.Stats().Total(AnimalKind::Herbivore).starved == 1);
    REQUIRE(hungry.Stats().Total(AnimalKind::Herbivore).diedOfAge == 0);

    SimulationSettings settings;
    settings.predator.maxAge = 1;
    Simulation old{World{1, 1}, std::mt19937{4}, settings};
    old.AddPredator({0, 0});
    old.Step();
    REQUIRE(old.Stats().Total(AnimalKind::Predator).diedOfAge == 1);
    REQUIRE(old.Stats().Total(AnimalKind::Predator).starved == 0);
}

TEST_CASE("Les evenements de la derniere generation sont remis a zero", "[stats]") {
    Simulation simulation{World{1, 1}, std::mt19937{5}};
    simulation.AddHerbivore({0, 0}, 1);
    simulation.AddPredator({0, 0}, 100);
    simulation.Step();
    REQUIRE(simulation.Stats().LastGeneration(AnimalKind::Herbivore).Deaths() == 1);

    simulation.Step();
    REQUIRE(simulation.Stats().LastGeneration(AnimalKind::Herbivore).Deaths() == 0);
    REQUIRE(simulation.Stats().Total(AnimalKind::Herbivore).Deaths() == 1);
}

TEST_CASE("A chaque generation, effectifs = anciens + naissances - morts", "[stats]") {
    for (const unsigned seed : {1u, 2u, 3u, 42u}) {
        std::mt19937 rng{seed};
        World world = GenerateRandomWorld(24, 14, rng);
        Simulation simulation{std::move(world), std::mt19937{rng()}};
        simulation.SeedHerbivores(60);
        simulation.SeedPredators(8);
        while (!simulation.IsFinished() && simulation.GetGeneration() < 300) {
            const std::size_t herbivores = simulation.HerbivoreCount();
            const std::size_t predators = simulation.PredatorCount();
            simulation.Step();
            const LifeEvents& h = simulation.Stats().LastGeneration(AnimalKind::Herbivore);
            const LifeEvents& p = simulation.Stats().LastGeneration(AnimalKind::Predator);
            REQUIRE(simulation.HerbivoreCount() == herbivores + h.births - h.Deaths());
            REQUIRE(simulation.PredatorCount() == predators + p.births - p.Deaths());
            REQUIRE(p.eaten == 0);
            // L'historique garde les evenements de chaque generation pour les graphiques.
            REQUIRE(simulation.PopulationHistory().back().herbivoreEvents.births == h.births);
            REQUIRE(simulation.PopulationHistory().back().predatorEvents.Deaths() == p.Deaths());
        }
    }
}
