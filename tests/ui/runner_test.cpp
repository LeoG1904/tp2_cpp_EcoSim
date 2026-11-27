#include <catch2/catch_all.hpp>

import std;
import ecosim.runner;

using namespace ecosim;

namespace {
    Simulation Create(const std::uint32_t seed, const int width = 30, const int height = 20) {
        return CreateRandomSimulation(width, height, DefaultSettings(width, height), seed);
    }

    // Tout ce qui distingue deux photos : génération, historique et état de chaque animal.
    bool Same(const SimulationSnapshot& a, const SimulationSnapshot& b) {
        if (a.generation != b.generation || a.history.size() != b.history.size()
            || a.animals.size() != b.animals.size()) {
            return false;
        }
        for (std::size_t i = 0; i < a.animals.size(); ++i) {
            if (a.animals[i].id != b.animals[i].id || a.animals[i].energy != b.animals[i].energy
                || a.animals[i].path != b.animals[i].path) {
                return false;
            }
        }
        return a.history.back().vegetation == b.history.back().vegetation;
    }

    // Une série longue peut être écourtée : on redemande ce qui manque.
    void Reach(SimulationRunner& runner, const int generation) {
        while (runner.State().snapshot->generation < generation
               && !runner.State().snapshot->IsFinished()) {
            runner.Request(generation - runner.State().snapshot->generation);
            runner.Wait();
        }
    }
}

TEST_CASE("La premiere photo est prete avant tout calcul", "[runner]") {
    const SimulationRunner runner{Create(1)};
    const RunnerState state = runner.State();
    REQUIRE(state.snapshot->generation == 0);
    REQUIRE_FALSE(state.busy);
    REQUIRE_FALSE(state.failed);
    runner.Wait(); // rien à attendre : retour immédiat
}

TEST_CASE("Le thread calcule exactement la meme simulation", "[runner]") {
    Simulation reference = Create(7);
    SimulationRunner runner{Create(7)};
    for (int generation = 1; generation <= 25; ++generation) {
        reference.Step();
        runner.Request(1);
        runner.Wait();
        CAPTURE(generation);
        REQUIRE(Same(*runner.State().snapshot, reference.Snapshot()));
    }
}

TEST_CASE("Une photo gardee ne change plus pendant que le calcul continue", "[runner]") {
    SimulationRunner runner{Create(3)};
    Reach(runner, 5);
    const std::shared_ptr<const SimulationSnapshot> kept = runner.State().snapshot;
    const SimulationSnapshot copy = *kept;
    Reach(runner, 20);
    REQUIRE(runner.State().snapshot->generation == 20);
    REQUIRE(Same(*kept, copy));
}

TEST_CASE("Lire des photos pendant le calcul reste coherent", "[runner]") {
    SimulationRunner runner{Create(11, 60, 40)};
    int previous = 0;
    int requests = 0;
    while (previous < 40 && requests < 100000) {
        const RunnerState state = runner.State();
        // Les photos avancent sans jamais reculer, et chacune est complète.
        REQUIRE(state.snapshot->generation >= previous);
        REQUIRE(state.snapshot->history.size()
                == static_cast<std::size_t>(state.snapshot->generation) + 1);
        previous = state.snapshot->generation;
        if (!state.busy) {
            runner.Request(5);
            ++requests;
        }
        if (state.snapshot->IsFinished()) break;
    }
    REQUIRE(previous >= 40);
}

TEST_CASE("Une demande vide est ignoree", "[runner]") {
    SimulationRunner runner{Create(2)};
    runner.Request(0);
    runner.Request(-3);
    REQUIRE_FALSE(runner.State().busy);
}

TEST_CASE("Detruire le pilote pendant un calcul l'arrete sans blocage", "[runner]") {
    for (std::uint32_t seed = 1; seed <= 20; ++seed) {
        SimulationRunner runner{Create(seed, 60, 40)};
        runner.Request(1000);
    }
    SUCCEED();
}
