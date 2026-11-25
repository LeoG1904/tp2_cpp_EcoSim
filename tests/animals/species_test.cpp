#include <catch2/catch_all.hpp>

import std;
import ecosim.herbivore;
import ecosim.predator;

using namespace ecosim;

namespace {
    // Habitat de test : les espèces s'utilisent sans la simulation.
    class FakeHabitat final : public Habitat {
    public:
        World world{3, 1};
        Position prey{2, 0};
        int captures{0};

        World& GetWorld() override { return world; }
        const World& GetWorld() const override { return world; }
        int PreyCount(const Position position) const override { return position == prey ? 1 : 0; }
        bool CatchPrey(const Position position) override {
            if (position != prey) return false;
            ++captures;
            return true;
        }
    };
}

TEST_CASE("Chaque espece choisit sa destination et sa nourriture", "[species]") {
    FakeHabitat habitat;
    habitat.world.GetCell({0, 0}).SetVegetation(80);
    std::mt19937 rng{1};
    std::vector<std::unique_ptr<Animal>> animals;
    animals.push_back(std::make_unique<Herbivore>(1, Position{1, 0}));
    animals.push_back(std::make_unique<Predator>(2, Position{1, 0}));

    for (auto& animal : animals) {
        animal->MoveTo(animal->ChooseDestination(habitat, rng));
        animal->Feed(habitat);
    }

    REQUIRE(animals[0]->GetPosition() == Position{0, 0});
    REQUIRE(animals[0]->GetEnergy() == 60);
    REQUIRE(animals[1]->GetPosition() == Position{2, 0});
    REQUIRE(animals[1]->GetEnergy() == 85);
    REQUIRE(habitat.captures == 1);
}

TEST_CASE("Les proies passent avant les chasseurs", "[species]") {
    const Herbivore herbivore{1, {0, 0}};
    const Predator predator{2, {0, 0}};
    REQUIRE(herbivore.Phase() == TurnPhase::Grazing);
    REQUIRE(predator.Phase() == TurnPhase::Hunting);
    REQUIRE(TurnPhases.front() == TurnPhase::Grazing);
    REQUIRE(herbivore.IsPrey());
    REQUIRE_FALSE(predator.IsPrey());
}

TEST_CASE("Un descendant connait son parent", "[species]") {
    const Herbivore parent{7, {0, 0}, 90};
    REQUIRE_FALSE(parent.GetParentId().has_value());
    REQUIRE(parent.CreateOffspring(8)->GetParentId() == 7);
}
