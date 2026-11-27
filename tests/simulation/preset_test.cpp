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

TEST_CASE("Equilibre : les quatre especes sont le plus souvent la apres 400 generations", "[preset]") {
    // Environ 9 cartes sur 10 gardent tout le monde, avec des series plus ou moins chanceuses :
    // sur 20 cartes, on en exige 15.
    int complete = 0;
    for (std::uint32_t seed = 1; seed <= 20; ++seed) {
        const Simulation simulation = Run(Preset::Balance, seed, 400);
        complete += std::ranges::all_of(SpeciesCatalog(), [&simulation](const SpeciesInfo& species) {
            return simulation.Count(species.kind) > 0;
        });
    }
    REQUIRE(complete >= 15);
}

TEST_CASE("Famine : les cerfs et les lapins disparaissent, surtout de faim", "[preset]") {
    for (const std::uint32_t seed : {1u, 2u, 3u}) {
        const Simulation simulation = Run(Preset::Famine, seed, 150);
        for (const AnimalKind kind : {AnimalKind::Deer, AnimalKind::Rabbit}) {
            const LifeEvents& events = simulation.Stats().Total(kind);
            REQUIRE(simulation.Count(kind) == 0);
            REQUIRE(events.starved > events.eaten);
        }
    }
}

TEST_CASE("Invasion : les cerfs et les lapins sont rapidement manges", "[preset]") {
    for (const std::uint32_t seed : {1u, 2u, 3u}) {
        const Simulation simulation = Run(Preset::Invasion, seed, 100);
        for (const AnimalKind kind : {AnimalKind::Deer, AnimalKind::Rabbit}) {
            const LifeEvents& events = simulation.Stats().Total(kind);
            REQUIRE(simulation.Count(kind) == 0);
            REQUIRE(events.eaten > events.starved);
        }
    }
}
