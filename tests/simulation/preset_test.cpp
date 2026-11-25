#include <catch2/catch_all.hpp>

import std;
import ecosim.simulation;

using namespace ecosim;

namespace {
    // Même création que l'application, avec une graine fixe.
    Simulation Run(const Preset preset, const std::uint32_t seed, const int generations) {
        constexpr int width{42};
        constexpr int height{26};
        Simulation simulation = CreateRandomSimulation(width, height,
            PresetSettings(preset, width, height), seed);
        while (!simulation.IsFinished() && simulation.GetGeneration() < generations) simulation.Step();
        return simulation;
    }
}

TEST_CASE("Equilibre : les deux especes sont encore la apres 400 generations", "[preset]") {
    for (const std::uint32_t seed : {1u, 2u, 3u}) {
        const Simulation simulation = Run(Preset::Balance, seed, 400);
        REQUIRE(simulation.Count(AnimalKind::Herbivore) > 0);
        REQUIRE(simulation.Count(AnimalKind::Predator) > 0);
    }
}

TEST_CASE("Famine : les herbivores disparaissent, surtout de faim", "[preset]") {
    for (const std::uint32_t seed : {1u, 2u, 3u}) {
        const Simulation simulation = Run(Preset::Famine, seed, 150);
        const LifeEvents& herbivores = simulation.Stats().Total(AnimalKind::Herbivore);
        REQUIRE(simulation.Count(AnimalKind::Herbivore) == 0);
        REQUIRE(herbivores.starved > herbivores.eaten);
    }
}

TEST_CASE("Invasion : les herbivores sont rapidement manges", "[preset]") {
    for (const std::uint32_t seed : {1u, 2u, 3u}) {
        const Simulation simulation = Run(Preset::Invasion, seed, 100);
        const LifeEvents& herbivores = simulation.Stats().Total(AnimalKind::Herbivore);
        REQUIRE(simulation.Count(AnimalKind::Herbivore) == 0);
        REQUIRE(herbivores.eaten > herbivores.starved);
    }
}
